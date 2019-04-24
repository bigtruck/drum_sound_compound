
// MIDI2CDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"



// CMIDI2CDlg �Ի���
class CMIDI2CDlg : public CDialogEx
{
// ����
public:
	CMIDI2CDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MIDI2C_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
