﻿
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "StatusDlg.h"


// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	/*
	1 查看磁盘分区
	2 查看指定目录下文件
	3 打开文件
	4 下载文件
	9 删除文件
	5 鼠标操作
	6 发送屏幕内容
	7 锁机
	8 解锁
	1981 测试连接
	返回值：命令号，小于0=错误
	*/
	//int SendCommandPacket(int nCmd,bool bAutoClose=true,BYTE* pData=NULL,size_t nLength = 0);

	CString GetPath(HTREEITEM hTree);
	void DeleteTreeChildItem(HTREEITEM hTreeSelect);
	void LoadFileInfo();
	void LoadFileCurrent();
	//static void threadEntryForDownFile(void*);
	//void threadDownFile();

	//static void threadEntryForWatchData(void*);
	//void threadWatchData();
// 实现
protected:
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnBnClickedButtonTest();
	DWORD m_server_address;
	CString m_port;
	afx_msg void OnBnClickedButtonFileinfo();
	CTreeCtrl m_tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// //显示文件
	CListCtrl m_list;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDelteteFile();
	afx_msg void OnRunFile();
	afx_msg void OnBnClickedButtonStartwatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnIpnFieldchangedIpaddressServer(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEdit1Port();
};
