
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include "ClientSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框  提交测试1111
//main分支测试
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_port(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERVER, m_server_address);
	DDX_Text(pDX, IDC_EDIT1_PORT, m_port);
	DDX_Control(pDX, IDC_TREE_DIR, m_tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_list);
}

int CRemoteClientDlg::SendCommandPacket(int nCmd,bool bAutoClose, BYTE* pData, size_t nLength)
{
	UpdateData();
	// TODO: 在此添加控件通知处理程序代码
	CClientSocket* pClient = CClientSocket::getInstance();
	bool ret = pClient->InitSocket(m_server_address, atoi(m_port));
	if (!ret)
	{
		AfxMessageBox("网络初始化失败");
		return -1;
	}
	CPacket pack(nCmd, pData, nLength);
	ret = pClient->SendData(pack);
	TRACE("send ret=%d\r\n", ret);
	int cmd = pClient->DealCommand();
	TRACE("ack:%d\r\n", cmd);
	if(bAutoClose)pClient->CloseSocket();
	return cmd;
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CRemoteClientDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_FILEINFO, &CRemoteClientDlg::OnBnClickedButtonFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELTETE_FILE, &CRemoteClientDlg::OnDelteteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteClientDlg::OnRunFile)
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	m_port = _T("9999");
	m_server_address = 0x7F000001;
	UpdateData(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRemoteClientDlg::OnBnClickedButtonTest()
{
	SendCommandPacket(1981);
}


void CRemoteClientDlg::OnBnClickedButtonFileinfo()
{
	int ret = SendCommandPacket(1);
	if (ret == -1)
	{
		AfxMessageBox("命令处理失败");
		return;
	}
	m_tree.DeleteAllItems();
	CClientSocket* pClient = CClientSocket::getInstance();
	std::string drivers = pClient->GetPacket().strData;
	std::string dr;
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',')
		{
			dr += ":";
			HTREEITEM hTmp = m_tree.InsertItem(dr.c_str(),TVI_ROOT,TVI_LAST);
			m_tree.InsertItem("", hTmp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}

}

CString CRemoteClientDlg::GetPath(HTREEITEM hTree)
{
	CString strRet, strTmp;
	do
	{
		strTmp = m_tree.GetItemText(hTree);
	    strRet = strTmp + '\\' + strRet;
		hTree = m_tree.GetParentItem(hTree);
	} while (hTree!=NULL);
	return strRet;
}

void CRemoteClientDlg::DeleteTreeChildItem(HTREEITEM hTreeSelect)
{
	HTREEITEM  hSub = NULL;
	do
	{
		hSub = m_tree.GetChildItem(hTreeSelect);
		if (hSub!=NULL)
		{
			m_tree.DeleteItem(hSub);
		}
	} while (hSub!=NULL);
}

void CRemoteClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);//全局坐标
	m_tree.ScreenToClient(&ptMouse);

	HTREEITEM hTreeSelect = m_tree.HitTest(ptMouse, 0);
	if (hTreeSelect == NULL)
	{
		return;
	}
	if (m_tree.GetChildItem(hTreeSelect) == NULL)
	{
		return;
	}

	DeleteTreeChildItem(hTreeSelect);
	m_list.DeleteAllItems();
	CString strPath = GetPath(hTreeSelect);
	TRACE("[path=%s]\r\n", strPath);
	SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	CClientSocket* pClient = CClientSocket::getInstance();
	while (pInfo->HasNext)
	{
		TRACE("[%s] isdir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (pInfo->IsDirectory)
		{
			if (CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == "..") {
				int cmd = pClient->DealCommand();
				TRACE("ack%d\r\n", cmd);
				if (cmd < 0)break;
				pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTmp = m_tree.InsertItem(pInfo->szFileName, hTreeSelect, TVI_LAST);
			m_tree.InsertItem("", hTmp, TVI_LAST);
		}
		else
		{
			m_list.InsertItem(0, pInfo->szFileName);
		}

		int cmd = pClient->DealCommand();
		TRACE("ack%d\r\n", cmd);
		if (cmd < 0)break;
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}

	pClient->CloseSocket();
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse,ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_list.ScreenToClient(&ptList);
	int ListSelected = m_list.HitTest(ptList);
	if (ListSelected <0)
	{
		return;
	}
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu* pPupup = menu.GetSubMenu(0);
	if (pPupup!=NULL)
	{
		pPupup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,ptMouse.x,ptMouse.y,this);
	}
}


void CRemoteClientDlg::OnDownloadFile()
{
	// TODO: 下载文件
	int nListSelected = m_list.GetSelectionMark();
	CString strFile = m_list.GetItemText(nListSelected, 0);
	
	CFileDialog dlg(FALSE,"*", strFile,OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,NULL,this);
	
	if (dlg.DoModal() == IDOK)
	{
		FILE* pFile = fopen(dlg.GetPathName(), "wb+");
		if (pFile == NULL)
		{
			AfxMessageBox("本地没有权限打开文件或者无法创建文件");
			return;
		}
		HTREEITEM hSelected = m_tree.GetSelectedItem();
		strFile = GetPath(hSelected) + strFile;
		TRACE("OnDownloadFile() strFile=%s\r\n", strFile);
		int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
		if (ret < 0)
		{
			AfxMessageBox("执行下载失败");
			return;
		}
		CClientSocket* pCleint = CClientSocket::getInstance();
		long long nLength = *(long long*)pCleint->GetPacket().strData.c_str();
		if (nLength == 0)
		{
			AfxMessageBox("文件长度为0或者无法读取文件");
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
		pCleint->CloseSocket();
	}
	
}


void CRemoteClientDlg::OnDelteteFile()
{
	// TODO: 删除文件
}


void CRemoteClientDlg::OnRunFile()
{
	// TODO: 打开文件
}
