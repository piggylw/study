#pragma once
#include "ClientSocket.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include <map>
#include "resource.h"
#include "ClientSocket.h"
#include "CTool.h"

//#define WM_SEND_PACK (WM_USER+1)
//#define WM_SEND_DATA (WM_USER+2)
#define WM_SHOW_STATUS (WM_USER+3) //չʾ״̬
#define WM_SHOW_WATCH (WM_USER+4)  //Զ�̼��
#define WM_SEND_MESSAGE (WM_USER+0x1000) //�Զ�����Ϣ����


class CClientController
{
public:
	static CClientController* getInstance();
	//��ʼ��
	int InitController();
	//����
	int Invoke(CWnd*& pMainWnd);

	LRESULT SendMessage(MSG msg);

	void UpdateAddress(int nIP, short nPort)
	{
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}

	int DealCommand()
	{
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket()
	{
		CClientSocket::getInstance()->CloseSocket();
	}


	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nLength = 0, std::list<CPacket>* lstPacks=NULL);

	int GetImage(CImage& image)
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		return CTool::Bytes2Image(image, pClient->GetPacket().strData);
	}

	int DownLoadFile(CString strPath);
	void StartWatchScreen();

protected:
	void threadWatchSreen();
	static void threadWatchSreenEntry(void* arg);
	void threadDownloadFile();
	static void threadDownloadEntry(void* arg);
	CClientController():m_statusdlg(&m_remotedlg),m_watchdlg(&m_remotedlg)
	{
		m_hThread = INVALID_HANDLE_VALUE;
		m_hThreadDownload = INVALID_HANDLE_VALUE;
		m_hThreadWatch = INVALID_HANDLE_VALUE;
		m_isClosed = true;
		m_nThreadID = -1;
	}
	~CClientController()
	{
		WaitForSingleObject(m_hThread, 100);
	}
	void threadFunc();
	static unsigned __stdcall threadEntry(void* arg);
	static void releaseInstance()
	{
		if (m_instance!=NULL)
		{
			delete m_instance;
			m_instance = NULL;
		}
	}
	//LRESULT OnSendPack(
	//	UINT nMsg,
	//	WPARAM wParam,
	//	LPARAM lParam);

	//LRESULT OnSendData(
	//	UINT nMsg,
	//	WPARAM wParam,
	//	LPARAM lParam);

	LRESULT OnShowStatus(
		UINT nMsg,
		WPARAM wParam,
		LPARAM lParam);

	LRESULT OnShowWatch(
		UINT nMsg,
		WPARAM wParam,
		LPARAM lParam);

private:
	typedef struct MsgInfo{
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m)
		{
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		
		MsgInfo(const MsgInfo& m)
		{
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& m)
		{
			if (this!=&m)
			{
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}

	}MSGINFO;
	typedef LRESULT(CClientController::* MSGFUNC)(
		UINT nMsg, 
		WPARAM wParam, 
		LPARAM lParam);
	static std::map<UINT, MSGFUNC> m_mapFunc;
	static CClientController* m_instance;
	CWatchDialog m_watchdlg;
	CRemoteClientDlg m_remotedlg;
	CStatusDlg m_statusdlg;
	HANDLE m_hThread;
	unsigned m_nThreadID;
	HANDLE m_hThreadDownload;
	HANDLE m_hThreadWatch;
	bool m_isClosed;//����Ƿ�ر�
	CString m_strRemote;//�����ļ�Զ��·��
	CString m_strLocal;//�����ļ��ı���·��
	class CHelper {
	public:
		CHelper()
		{
			//CClientController::getInstance();
		}
		~CHelper()
		{
			CClientController::releaseInstance();
		}
	};
	static CHelper m_helper;
};

