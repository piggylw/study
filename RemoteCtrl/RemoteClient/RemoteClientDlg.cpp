﻿
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
}

int CRemoteClientDlg::SendCommandPacket(int nCmd, BYTE* pData, size_t nLength)
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
	pClient->CloseSocket();
	return cmd;
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CRemoteClientDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_FILEINFO, &CRemoteClientDlg::OnBnClickedButtonFileinfo)
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
			m_tree.InsertItem(dr.c_str(),TVI_ROOT,TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}

}
