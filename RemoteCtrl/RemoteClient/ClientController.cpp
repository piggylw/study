#include "pch.h"
#include "ClientController.h"


std::map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;
CClientController::CHelper CClientController::m_helper;

CClientController* CClientController::getInstance()
{
	if (m_instance==NULL)
	{
		m_instance = new CClientController();
		struct {
			UINT nMsg;
			MSGFUNC func;
		}MsgFuncs[] = {
			//{WM_SEND_PACK,&CClientController::OnSendPack},
			//{WM_SEND_DATA,&CClientController::OnSendData},
			{WM_SHOW_STATUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH,&CClientController::OnShowWatch},
			{(UINT) - 1,NULL}
		};
		for (int  i = 0; MsgFuncs[i].func!=NULL; i++)
		{
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(MsgFuncs[i].nMsg,MsgFuncs[i].func));
		}
	}
	return m_instance;
}

int CClientController::InitController()
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, threadEntry, this, 0, &m_nThreadID);
	m_statusdlg.Create(IDD_DIALOG_STATUS,&m_remotedlg);
	return 0;
}

int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remotedlg;
	return m_remotedlg.DoModal();
}

LRESULT CClientController::SendMessage(MSG msg)
{
	
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent==NULL)
	{
		return -2;
	}
	MSGINFO info(msg);
	PostThreadMessage(m_nThreadID,WM_SEND_MESSAGE,(WPARAM)&info, (LPARAM)hEvent);

	WaitForSingleObject(hEvent,INFINITE);
	CloseHandle(hEvent);
	return info.result;
}

int CClientController::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nLength,
	std::list<CPacket>* plstPacks)
{
	CClientSocket* pClient = CClientSocket::getInstance();
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	std::list<CPacket> lstPacks;
	if (plstPacks == NULL)
	{
		plstPacks = &lstPacks;
	}
	pClient->SendPacket(CPacket(nCmd, pData, nLength,hEvent),*plstPacks,bAutoClose);
	CloseHandle(hEvent);
	if (plstPacks->size()>0)
	{
		return plstPacks->front().sCmd;
	}
	return -1;
}

int CClientController::DownLoadFile(CString strPath)
{
	CFileDialog dlg(FALSE, "*", strPath, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, NULL, &m_remotedlg);

	if (dlg.DoModal() == IDOK)
	{
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();
		m_hThreadDownload = (HANDLE)_beginthread(&CClientController::threadDownloadEntry, 0, this);
		if (WaitForSingleObject(m_hThreadDownload, 0) != WAIT_TIMEOUT)
		{
			return -1;
		}
		m_remotedlg.BeginWaitCursor();
		m_statusdlg.m_info.SetWindowText("命令正在执行");
		m_statusdlg.ShowWindow(SW_SHOW);
		m_statusdlg.CenterWindow(&m_remotedlg);
		m_statusdlg.SetActiveWindow();
	}
	return 0;

}

void CClientController::StartWatchScreen()
{
	m_isClosed = false;
	//m_watchdlg.SetParent(&m_remotedlg);
	m_hThreadWatch = (HANDLE)_beginthread(&CClientController::threadWatchSreenEntry, 0, this);
	m_watchdlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch, 500);
}

void CClientController::threadWatchSreen()
{
	Sleep(50);
	while (!m_isClosed)
	{

		if (m_watchdlg.isFull() == false)
		{
			std::list<CPacket> lstPacks;
			int ret = SendCommandPacket(6,true,NULL,0,&lstPacks);
			if (ret == 6)
			{
				if (CTool::Bytes2Image(m_watchdlg.GetImage(), lstPacks.front().strData) ==0)
				{
					m_watchdlg.SetImageStatus(true);
				}
				else
				{
					TRACE("获取失败:ret=%d\r\n", ret);
				}

			}
		}
		Sleep(1);
	}
}

void CClientController::threadWatchSreenEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadWatchSreen();
	_endthread();
}

void CClientController::threadDownloadFile()
{
	FILE* pFile = fopen(m_strLocal, "wb+");
	if (pFile==NULL)
	{
		AfxMessageBox("本地没有权限打开文件或者无法创建文件");
		m_statusdlg.ShowWindow(SW_HIDE);
		m_statusdlg.EndWaitCursor();
		return;
	}
	CClientSocket* pCleint = CClientSocket::getInstance();
	int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength());
	if (ret < 0)
	{
		AfxMessageBox("执行下载失败");
		fclose(pFile);
		pCleint->CloseSocket();
		return;
	}

	long long nLength = *(long long*)pCleint->GetPacket().strData.c_str();
	if (nLength == 0)
	{
		AfxMessageBox("文件长度为0或者无法读取文件");
		fclose(pFile);
		pCleint->CloseSocket();
		return;
	}
	long long nCount = 0;

	while (nCount < nLength)
	{
		ret = pCleint->DealCommand();
		if (ret < 0)
		{
			AfxMessageBox("传输失败");
			TRACE("传输失败：ret=%d\r\n", ret);
			break;
		}

		fwrite(pCleint->GetPacket().strData.c_str(), 1, pCleint->GetPacket().strData.size(), pFile);
		nCount += pCleint->GetPacket().strData.size();
	}

	fclose(pFile);
	CloseSocket();
	m_statusdlg.ShowWindow(SW_HIDE);
	m_remotedlg.EndWaitCursor();
	m_remotedlg.MessageBox("下载完成", "完成");
}

void CClientController::threadDownloadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadDownloadFile();
	_endthread();
}

void CClientController::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE)
		{
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(pmsg->msg.message);
			if (it != m_mapFunc.end())
			{
				pmsg->result = (this->*it->second)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
			}
			else
			{
				pmsg->result = -1;
			}
			SetEvent(hEvent);
		}
		else
		{
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end())
			{
				(this->*it->second)(msg.message,msg.wParam,msg.lParam);
			}
		}

	}
}

unsigned __stdcall CClientController::threadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();
	_endthreadex(0);
	return 0;
}

//LRESULT CClientController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	CPacket* packet = (CPacket*)wParam;
//	return pClient->SendData(*packet);
//	
//}
//}

//LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	char* pbuffer = (char*)wParam;
//	return pClient->SendData(pbuffer,(int)lParam);
//}

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusdlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_watchdlg.DoModal();
}
