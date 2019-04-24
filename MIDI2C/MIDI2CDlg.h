
// MIDI2CDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"



// CMIDI2CDlg 对话框
class CMIDI2CDlg : public CDialogEx
{
// 构造
public:
	CMIDI2CDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MIDI2C_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	LVITEM g_lvItem;
	int g_nItem;
	CListCtrl m_CtrlList;
	afx_msg void OnBnClickedBuscanfile();
	afx_msg void OnBnClickedBucreate();
	afx_msg void OnBnClickedBuopenpath();
	void mFindFile(CString dir);
	afx_msg void OnBnClickedBuaddfile();
	afx_msg void OnBnClickedBuclearlist();
	static UINT WINAPI CreateFile(LPVOID lParam);
	
	afx_msg LRESULT OnUserThreadend(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	void DlgCtrlEnable(BOOL enable);

	void ViewLastError(LPTSTR lpMsg);
	
};
