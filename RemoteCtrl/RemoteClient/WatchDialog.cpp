// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "ClientController.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_WATCH, pParent)
{
	m_isFull = false;
	m_nObjHeight = -1;
	m_nObjWidth = -1;
}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
	ON_BN_CLICKED(IDC_BUTTON_LOCK, &CWatchDialog::OnBnClickedButtonLock)
	ON_BN_CLICKED(IDC_BUTTON_UNLOCK, &CWatchDialog::OnBnClickedButtonUnlock)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::USerPoint2RemoteScreenPoint(CPoint& point,bool isScreen )
{

	CRect clientRect;
	if (isScreen)
	{
		ScreenToClient(&point);
	}
	
	m_picture.GetWindowRect(&clientRect);
	int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	int x = point.x * m_nObjWidth / width0;
	int y = point.y * m_nObjHeight / height0;

	return CPoint(x,y);
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_isFull = false;
	SetTimer(0, 45, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0)
	{
		if (m_isFull)
		{

			CRect rect;
			m_picture.GetWindowRect(rect);
			m_nObjWidth = m_image.GetWidth();
			m_nObjHeight = m_image.GetHeight();
			//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0,
				rect.Width(),rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);
			m_image.Destroy();
			SetImageStatus();



		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) 
	{
		CPoint remote = USerPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 1;//双击
		TRACE("左键双击\r\n");
		CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));

	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = USerPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 2;//按下
		TRACE("左键按下\r\n");
		CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = USerPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 3;//抬起
		TRACE("左键松开\r\n");
		CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}

	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = USerPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 1;
		TRACE("右键双击");
		CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = USerPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 2;
		TRACE("右键按下");
		CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}

	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = USerPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 3;
		TRACE("右键松开");
		CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}

	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	{
		CPoint remote = USerPoint2RemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 8;
		event.nAction = 0;//移动
		TRACE("移动\r\n");
		CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
	}

	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()
{
	// TODO: 在此添加控件通知处理程序代码
	//if ((m_nObjHeight != -1) && (m_nObjWidth != -1))
	//{
	//	CPoint point;
	//	GetCursorPos(&point);
	//	CPoint remote = USerPoint2RemoteScreenPoint(point,true);
	//	MOUSEEV event;
	//	event.ptXY = remote;
	//	event.nButton = 0;
	//	event.nAction = 0;//d单机
	//	TRACE("左键单机\r\n");
	//	(CClientSocket*)GetParent()->SendMessage(WM_SEND_PACKET, 5 << 1 | 1,
	//		(WPARAM) & event);
	//}
	
}


void CWatchDialog::OnBnClickedButtonLock()
{
	// TODO: 在此添加控件通知处理程序代码

	CClientController::getInstance()->SendCommandPacket(7);
}


void CWatchDialog::OnBnClickedButtonUnlock()
{
	// TODO: 在此添加控件通知处理程序代码

	CClientController::getInstance()->SendCommandPacket(8);
}
