// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_WATCH, pParent)
{

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
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::USerPoint2RemoteScreenPoint(CPoint& point,bool isScreen )
{
	//800 450   监控屏幕
	//2880 1800 客户屏幕
	CRect clientRect;
	if (isScreen)
	{
		ScreenToClient(&point);
	}
	
	m_picture.GetWindowRect(&clientRect);
	int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	int width = 2880;
	int height = 1800;
	int x = point.x * width / width0;
	int y = point.y * height / height0;

	return CPoint(x,y);
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(0, 45, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0)
	{
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->isFull())
		{
			if (!pParent->GetImage().IsNull())
			{
				CRect rect;
				m_picture.GetWindowRect(rect);
				//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
				pParent->GetImage().StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0,
					rect.Width(),rect.Height(), SRCCOPY);
				//m_picture.InvalidateRect(NULL);
				pParent->GetImage().Destroy();
				pParent->SetImageStatus();
			}


		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CPoint remote = USerPoint2RemoteScreenPoint(point);
	MOUSEEV event;
	event.nButton = 0;//左键
	event.nAction = 1;//双击
	(CClientSocket*)GetParent()->SendMessage(WM_SEND_PACKET, 5 << 1 | 1,
		(WPARAM) & event);
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	CPoint remote = USerPoint2RemoteScreenPoint(point);
	MOUSEEV event;
	event.nButton = 0;
	event.nAction = 2;//按下
	(CClientSocket*)GetParent()->SendMessage(WM_SEND_PACKET, 5 << 1 | 1,
		(WPARAM) & event);

	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint remote = USerPoint2RemoteScreenPoint(point);
	MOUSEEV event;
	event.nButton = 0;
	event.nAction = 3;//抬起
	(CClientSocket*)GetParent()->SendMessage(WM_SEND_PACKET, 5 << 1 | 1,
		(WPARAM) & event);
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	CPoint remote = USerPoint2RemoteScreenPoint(point);
	MOUSEEV event;
	event.nButton = 1;
	event.nAction = 1;
	(CClientSocket*)GetParent()->SendMessage(WM_SEND_PACKET, 5 << 1 | 1,
		(WPARAM) & event);

	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint remote = USerPoint2RemoteScreenPoint(point);
	MOUSEEV event;
	event.nButton = 1;
	event.nAction = 2;
	(CClientSocket*)GetParent()->SendMessage(WM_SEND_PACKET, 5 << 1 | 1,
		(WPARAM) & event);
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint remote = USerPoint2RemoteScreenPoint(point);
	MOUSEEV event;
	event.nButton = 1;
	event.nAction = 3;
	(CClientSocket*)GetParent()->SendMessage(WM_SEND_PACKET, 5 << 1 | 1,
		(WPARAM) & event);
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint remote = USerPoint2RemoteScreenPoint(point);
	MOUSEEV event;
	event.nButton = 8;
	event.nAction = 0;//移动
	//(CClientSocket*)GetParent()->SendMessage(WM_SEND_PACKET, 5 << 1 | 1,
	//	(WPARAM) & event);
	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()
{
	// TODO: 在此添加控件通知处理程序代码
	CPoint point;
	GetCursorPos(&point);
	CPoint remote = USerPoint2RemoteScreenPoint(point,true);
	MOUSEEV event;
	event.nButton = 0;
	event.nAction = 0;//d单机
	(CClientSocket*)GetParent()->SendMessage(WM_SEND_PACKET, 5 << 1 | 1,
		(WPARAM) & event);
}
