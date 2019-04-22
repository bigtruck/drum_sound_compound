
// MIDI2CDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MIDI2C.h"
#include "MIDI2CDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_MYMSG                (WM_USER + 1)
#define WM_MYMSG_CREATE_START   (WM_MYMSG + 1)
#define WM_MYMSG_CREATE_END     (WM_MYMSG + 2)

#define WAV_DATA_BUFFSIZE	(1024*5)
#define BIN_FILE_HEAD_LEN	(512)

typedef struct
{
	CMIDI2CDlg *myDlg;
	//UINT32    headBuff[BIN_FILE_HEAD_LEN / 4];
	UINT32    dataBuff[1024 * 1024];
	UINT32    dataBuffOut[1024 * 1024];
	UINT32    binLength;
	CString   outPath;
}T_threadParamCreateFile;


T_threadParamCreateFile threadParamCreateFile;

bool addMapInfo;


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

														// ʵ��
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


// CMIDI2CDlg �Ի���



CMIDI2CDlg::CMIDI2CDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MIDI2C_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMIDI2CDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTFILE, m_CtrlList);
	DDX_Control(pDX, IDC_CHECK_ADDMAP, check_box);
}

BEGIN_MESSAGE_MAP(CMIDI2CDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUSCANFILE, &CMIDI2CDlg::OnBnClickedBuscanfile)
	ON_BN_CLICKED(IDC_BUCREATE, &CMIDI2CDlg::OnBnClickedBucreate)
	ON_BN_CLICKED(IDC_BUOPENPATH, &CMIDI2CDlg::OnBnClickedBuopenpath)
	ON_BN_CLICKED(IDC_BUADDFILE, &CMIDI2CDlg::OnBnClickedBuaddfile)
	ON_BN_CLICKED(IDC_BUCLEARLIST, &CMIDI2CDlg::OnBnClickedBuclearlist)
	ON_MESSAGE(WM_MYMSG, OnUserThreadend)
	ON_BN_CLICKED(IDC_CHECK_ADDMAP, &CMIDI2CDlg::OnBnClickedCheckAddmap)
	ON_BN_CLICKED(IDC_CHECK_CREAD_C, &CMIDI2CDlg::OnBnClickedCheckCreadC)
END_MESSAGE_MAP()


// CMIDI2CDlg ��Ϣ�������

BOOL CMIDI2CDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

									//ShowWindow(SW_MAXIMIZE);
	ShowWindow(SW_SHOWNORMAL);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	GetDlgItem(IDC_STATIC)->SetWindowText(_T(""));

	LVCOLUMN lvColumn;

	m_CtrlList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);

	lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvColumn.fmt = LVCFMT_LEFT;


	lvColumn.cx = 50;
	lvColumn.pszText = _T("���");
	m_CtrlList.InsertColumn(0, &lvColumn);

	lvColumn.cx = 100;
	lvColumn.pszText = _T("�ļ���");
	m_CtrlList.InsertColumn(1, &lvColumn);

	lvColumn.cx = 500;
	lvColumn.pszText = _T("�ļ�·��");
	m_CtrlList.InsertColumn(2, &lvColumn);

	//GetDlgItem(IDC_COMBO_SAMP)
	CComboBox *cBox;
	cBox = (CComboBox*)GetDlgItem(IDC_COMBO_SAMP);
	cBox->AddString(_T("192000"));
	cBox->AddString(_T("96000"));
	cBox->AddString(_T("48000"));
	cBox->AddString(_T("44100"));
	cBox->AddString(_T("32000"));
	cBox->AddString(_T("22050"));
	cBox->AddString(_T("16000"));
	cBox->AddString(_T("8000"));
	//cBox->EnableScrollBar(SB_VERT,ESB_ENABLE_BOTH);
	cBox->SetCurSel(3);

	CButton *mRadioBu = (CButton *)GetDlgItem(IDC_RADIO_16);
	mRadioBu->SetCheck(TRUE);
	mRadioBu = (CButton *)GetDlgItem(IDC_RADIO_24);
	mRadioBu->SetCheck(FALSE);

	addMapInfo = 0;

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMIDI2CDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMIDI2CDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMIDI2CDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		break;
		//case BFFM_SELCHANGED:
		//		TCHAR szPath[MAX_PATH];
		//		SHGetPathFromIDList(PCIDLIST_ABSOLUTE(lParam), szPath);
		//		CString strPath = szPath ;
		//		CString ext = strPath.Right(4) ;
		//		if ( ext == ".bin")
		//			SendMessage(hwnd, BFFM_ENABLEOK, 0, 1);
		//		else
		//			SendMessage(hwnd, BFFM_ENABLEOK, 0, 0);
		//	break ;
	}
	return 0;
}



void CMIDI2CDlg::mFindFile(CString dir)
//void mFindFile(CString dir)
{
	if (dir.IsEmpty())
	{
		MessageBox(_T("·������Ϊ�� "), _T("����"), MB_OK | MB_ICONWARNING);
		return;
	}

	CFileFind ff;
	BOOL bWork = ff.FindFile(dir + _T("\\*.*"));
	while (bWork)
	{
		bWork = ff.FindNextFile();
		if (ff.IsDots())
		{
			continue;
		}
		else if (ff.IsDirectory())
		{
			CString DirPath = ff.GetFilePath();
			mFindFile(DirPath);
		}
		else
		{
			CString FileName = ff.GetFileName();
			//int len = FileName.GetLength() - ff.GetFileTitle().GetLength();
			//CString ext = FileName.Right(len);
			CString fPath = ff.GetFilePath();
			CFile cf;
			cf.Open(fPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite);
			UINT32 head;
			cf.Read((UINT8 *)&head, 4);
			cf.Close();
			//if (ext == ".snd")
			if (head == 0x46464952)//"RIFF"
			{
				//	����ļ���Ϣ���б�
#ifdef UNICODE
				WCHAR *fn;
				WCHAR buff[20];
#else
				char* fn;
				char buff[20];
#endif
				g_lvItem.mask = LVIF_TEXT;
				g_lvItem.iItem = m_CtrlList.GetItemCount();
				g_lvItem.iSubItem = 0;
				wsprintf(buff, _T("%u"), g_lvItem.iItem);
				g_lvItem.pszText = buff;						//	���
				g_nItem = m_CtrlList.InsertItem(&g_lvItem);

				CString str = FileName;
				str = str.Left(str.GetLength() - 4);
				//int len = str.Find(" - 01 - �ϳ��� - AD Drums Master", 0);
				//if (len >= 0)
				//{
				//	str.Delete(len, str.GetLength() - len);
				//}
				str.Replace(_T(" "), _T("_"));
				fn = (char *)str.GetBuffer(str.GetLength());
				m_CtrlList.SetItemText(g_nItem, 1, fn);			//	�ļ���

				fn = (char *)fPath.GetBuffer(fPath.GetLength());
				m_CtrlList.SetItemText(g_nItem, 2, fn);			//	�ļ�·��

				CString FullPath = dir + _T("\\") + FileName;
				mFindFile(FullPath + _T("\n"));	//	�ݹ����
			}
		}
	}
	ff.Close();
}


void CMIDI2CDlg::OnBnClickedBuscanfile()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CEdit *m_EDPath;
	CString path;
	m_EDPath = (CEdit *)GetDlgItem(IDC_EDFOLDER);
	m_EDPath->GetWindowText(path);
	if (path.IsEmpty())
	{
		MessageBox(_T("·������Ϊ��"), _T(" "), MB_OK | MB_ICONWARNING);
		return;
	}

	mFindFile(path);
	//CWinThread *mThread;
	//threadParamScanFile.fileDir = path;
	//mThread = AfxBeginThread((AFX_THREADPROC)ScanFile, &threadParamScanFile, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
	//mThread->ResumeThread();

	path.Empty();
	path.Format(_T("��%d���ļ�"), m_CtrlList.GetItemCount());
	GetDlgItem(IDC_STATIC)->SetWindowText(path);
}


void CMIDI2CDlg::OnBnClickedBuopenpath()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//char def[]="C:\\" ;
	PIDLIST_ABSOLUTE pIDLIST;			//�������ص�ֵ��������  
	BROWSEINFO brsInfo;
	char dirPath[MAX_PATH];			//����ѡ��Ŀ¼���ַ�����  
	CString curDir;

	GetModuleFileName(NULL, dirPath, MAX_PATH);
	_tcsrchr(dirPath, _T('\\'))[1] = 0;
	curDir = dirPath;

	brsInfo.hwndOwner = this->m_hWnd;	//ӵ���ߴ���  
	brsInfo.pidlRoot = NULL;			//��Ŀ¼Ϊ����  
	brsInfo.pszDisplayName = dirPath;	//���صı�ѡ���·��  
	brsInfo.lpszTitle = _T("��ѡ��Ŀ¼");	//�Ի������  
	brsInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN | BIF_NEWDIALOGSTYLE; //����ʾ����·��  
	brsInfo.lpfn = BrowseCallbackProc;				//�ص�
	brsInfo.lParam = (LPARAM)(LPCTSTR)curDir.GetBuffer(curDir.GetLength());				//Ĭ��·��

																						// ��ʼ��COM����������ǵ��ͷ�  
	CoInitialize(NULL);
	pIDLIST = SHBrowseForFolder(&brsInfo);
	if (pIDLIST)
	{
		//	�õ�����·��  
		SHGetPathFromIDList(pIDLIST, dirPath);
		CEdit *m_EDPath;
		m_EDPath = (CEdit *)GetDlgItem(IDC_EDFOLDER);
		m_EDPath->SetWindowText(dirPath);
		//mFindFile(dirPath);
		//wsprintf(dirPath, "�� %u ���ļ�", m_CtrlList.GetItemCount());
		//m_StaticText.SetWindowText(dirPath);
	}
	CoUninitialize();
	// �ͷ�IDLIST
	CoTaskMemFree((LPVOID)pIDLIST);

}

//const char c_head[] = "#include <stdio.h>\r\nint main(void)\r\n{\r\n\r\n}\r\n";
#define	STR_TAB			("\t\t")
#define STR_TAB_SIZE	(sizeof(STR_TAB))

#define STR_LINEFEED	("\r\n")
#define STR_LINEFEED_SIZE	(sizeof(STR_LINEFEED) - 1)

void CMIDI2CDlg::OnBnClickedBucreate()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//CHAR curDir[MAX_PATH];
	CWinThread *mThread;

	//GetCurrentDirectory(MAX_PATH, curDir);		
	GetDlgItem(IDC_EDFOLDER)->GetWindowText(threadParamCreateFile.outPath);
	//threadParamCreateFile.outPath.Format("%s", curDir);
	threadParamCreateFile.myDlg = this;
	mThread = AfxBeginThread((AFX_THREADPROC)CreateFile, &threadParamCreateFile, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
	mThread->ResumeThread();
}



void CMIDI2CDlg::OnBnClickedBuaddfile()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CString fPath = _T("");

	CFileDialog    dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Describe Files (*.wav)|*.wav|All Files (*.*)|*.*||"), NULL);

	if (dlgFile.DoModal())
	{
		fPath = dlgFile.GetPathName();
		if (!fPath.IsEmpty())
		{

			CFile cf;
			cf.Open(fPath, CFile::modeRead | CFile::typeBinary);
			CString FileName = cf.GetFileName();
			UINT32 head;
			cf.Read((UINT8 *)&head, 4);
			cf.Close();
			//if (ext == ".snd")
			if (head == 0x46464952)//"MThd"
			{
				//	����ļ���Ϣ���б�
				char *fn;
				char buff[20];
				g_lvItem.mask = LVIF_TEXT;
				g_lvItem.iItem = m_CtrlList.GetItemCount();
				g_lvItem.iSubItem = 0;
				wsprintf(buff, _T("%u"), g_lvItem.iItem);
				g_lvItem.pszText = buff;						//	���
				int itemNo;
				itemNo = m_CtrlList.InsertItem(&g_lvItem);

				CString str = FileName;
				str = str.Left(str.GetLength() - 4);
				//int len = str.Find(" - 01 - �ϳ��� - AD Drums Master", 0);
				//if (len >= 0)
				//{
				//	str.Delete(len, str.GetLength() - len);
				//}
				str.Replace(_T(" "), _T("_"));
				fn = (char *)str.GetBuffer(str.GetLength());
				m_CtrlList.SetItemText(itemNo, 1, fn);			//	�ļ���

				fn = (char *)fPath.GetBuffer(fPath.GetLength());
				m_CtrlList.SetItemText(itemNo, 2, fn);			//	�ļ�·��
				str.Empty();
				str.Format(_T("��%d���ļ�"), m_CtrlList.GetItemCount());
				GetDlgItem(IDC_STATIC)->SetWindowText(str);
			}
		}
	}
}


void CMIDI2CDlg::OnBnClickedBuclearlist()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_CtrlList.DeleteAllItems();
	GetDlgItem(IDC_STATIC)->SetWindowText(_T("�����"));
}

#define DRUM_OUT_FILE_NAME	"drum_sound_info.h"
#define DRUM_OUT_FILE_END	"#endif\r\n"
#define STR_LINEFEED	_T("\r\n")
#define STR_LINEFEED_SIZE	(sizeof(STR_LINEFEED) - 1)


UINT CMIDI2CDlg::CreateFile(LPVOID lParam)
{	
	T_threadParamCreateFile *param = (T_threadParamCreateFile *)lParam;
	//CWnd      *cWnd;
	CListCtrl *mList;
	CStatic   *mStatic;
	CComboBox *mBox;
	CButton   *mRadioBu;
	INT       totalFile,cnt;
	ULONGLONG curPoint;
	UINT32    ChunkID, ChunkSize, Format, SubchunkID, SubchunkSize, SampleRate, ByteRate;
	UINT16    PCMFormat, NumChannels, BlockAlign, BitsPerSample;
	BOOL      brn;
	UINT32    dataAddr;
	CFile     binFile, wavFile, mapFile,datFile;
	CString   str,str2,arrLen,extFileName;
	UINT32    sample;
	UINT16    sbit;

	CMIDI2CDlg *myDlg = (CMIDI2CDlg *)(param->myDlg);
	
	::PostMessage(::AfxGetMainWnd()->m_hWnd, WM_MYMSG, WM_MYMSG_CREATE_START, 0);

	bool creatCfile = ((CButton*)myDlg->GetDlgItem(IDC_CHECK_ADDMAP))->GetCheck();

	mList = (CListCtrl *)myDlg->GetDlgItem(IDC_LISTFILE);
	mStatic = (CStatic *)myDlg->GetDlgItem(IDC_STATIC);
	mBox = (CComboBox *)myDlg->GetDlgItem(IDC_COMBO_SAMP);
	mRadioBu = (CButton *)myDlg->GetDlgItem(IDC_RADIO_16);
	int sel = mBox->GetCurSel();
	mBox->GetLBText(sel, str);
	sample = atoi(str.GetBuffer());

	if (addMapInfo)
	{

	}


	sbit = 24;
	if (mRadioBu->GetCheck() > 0)
	{
		sbit = 16;
	}

	totalFile = mList->GetItemCount();
	if (totalFile < 1)
	{
		AfxMessageBox(_T("���ļ�"),MB_ICONERROR);
		goto __LAB_EXIT;
	}
	
	str = param->outPath + _T("\\SoundSource.bin");
	brn = binFile.Open(str, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
	if (!brn)
	{
		AfxMessageBox(_T("�޷���������ļ�"), MB_ICONERROR);
		goto __LAB_EXIT;
	}
	str = param->outPath + _T("\\") + _T(DRUM_OUT_FILE_NAME);
	brn = mapFile.Open(str, CFile::modeCreate | CFile::modeWrite);
	if (!brn)
	{
		AfxMessageBox(_T("�޷�����map����ļ�"), MB_ICONERROR);
		goto __LAB_EXIT;
	}

	str.Format(_T("#ifndef __DRUM_SOUND_INFO_H\r\n#define __DRUM_SOUND_INFO_H\r\n\r\n#define DRUM_SOUND_SOURCE_MAX %d"),totalFile);
	mapFile.Write(str.GetBuffer(), str.GetLength());
	str.Empty();
	str.Format(_T("\r\n\r\nconst unsigned long soundSourceAddr[SOUNDSOURCE_MAX][4]=\r\n{\r\n"));
	mapFile.Write(str.GetBuffer(), str.GetLength());
	str.Empty();
	cnt = 0;
	dataAddr = 0;
	arrLen.Empty();
	extFileName.Empty();
	while (totalFile)
	{
		CString fPath;
		if (creatCfile)
		{
			str.Empty();
			str2 = mList->GetItemText(cnt, 1);
			str = param->outPath + _T("\\") + str2 + _T(".c");
			brn = datFile.Open(str, CFile::modeCreate | CFile::modeWrite);
			if (!brn)
			{
				myDlg->ViewLastError(_T("create faill"));
				goto __LAB_EXIT;
			}
			str.Empty();
			str = _T("const short int Drum_") + str2 + _T("[]={\r\n");
			datFile.Write(str, str.GetLength());
			arrLen += _T("#define DrumLen_") + str2 + _T("    ");
			extFileName += _T("extern const short int Drum_") + str2 + _T("[];\r\n");
		}
		fPath.Empty();
		fPath = mList->GetItemText(cnt, 2);
		mStatic->SetWindowText(fPath);
		brn = wavFile.Open(fPath, CFile::modeRead );
		if (brn)
		{
			wavFile.Read((UINT8 *)&ChunkID, 4);
			if (ChunkID != 0x46464952)//RIFF
			{
				AfxMessageBox(_T("�ļ���ʽ����\r\n") + mList->GetItemText(cnt, 2), MB_ICONERROR);
				goto __LAB_EXIT;
			}
			wavFile.Read((UINT8 *)&ChunkSize, 4);
			wavFile.Read((UINT8 *)&Format, 4);
			if (Format != 0x45564157)//WAVE
			{
				AfxMessageBox(_T("�ļ���ʽ����\r\n") + mList->GetItemText(cnt, 2), MB_ICONERROR);
				goto __LAB_EXIT;
			}
			while (1)
			{
				wavFile.Read((UINT8 *)&SubchunkID, 4);
				wavFile.Read((UINT8 *)&SubchunkSize, 4);
				if (wavFile.GetPosition() >= wavFile.GetLength())//�ļ��Ƿ����
				{
					AfxMessageBox(_T("�޷�ʶ����ļ�\r\n") + mList->GetItemText(cnt, 2), MB_ICONERROR);
					goto __LAB_EXIT;
				}
				if (SubchunkID == 0x20746D66)//"fmt "
				{
					curPoint = wavFile.GetPosition();
					wavFile.Read((UINT8 *)&PCMFormat, 2);
					wavFile.Read((UINT8 *)&NumChannels, 2);
					wavFile.Read((UINT8 *)&SampleRate, 4);
					wavFile.Read((UINT8 *)&ByteRate, 4);
					wavFile.Read((UINT8 *)&BlockAlign, 2);
					wavFile.Read((UINT8 *)&BitsPerSample, 2);
					if ( PCMFormat != 0x0001 )
					{
						AfxMessageBox(_T("�ļ���PCM���ݸ�ʽ\r\n") + mList->GetItemText(cnt, 2), MB_ICONERROR);
						goto __LAB_EXIT;
					}
					if (((BlockAlign / NumChannels) * 8) != sbit)
					{
						str.Empty();
						str.Format(_T("��%dbit����\r\n"), sbit);
						AfxMessageBox(str + mList->GetItemText(cnt, 2), MB_ICONERROR);
						goto __LAB_EXIT;
					}
					//if (SampleRate != 44100)
					if (SampleRate != sample)
					{
						str.Empty();
						str.Format(_T("��%dHZ������\r\n"), sample);
						AfxMessageBox(str + mList->GetItemText(cnt, 2), MB_ICONERROR);
						goto __LAB_EXIT;
					}
				}
				else if (SubchunkID == 0x61746164)//"data"
				{
					UINT32    dataCount;
					INT16    *pIn,*pOut;

					dataCount = 0;
					while (1)
					{
						wavFile.Read((UINT8 *)(&param->dataBuff[0]), SubchunkSize);
						if (NumChannels == 1)
						{
							binFile.Write((UINT8 *)(&param->dataBuff[0]), SubchunkSize);
							if (creatCfile)
							{
								pIn = (INT16*)(&param->dataBuff[0]);
								UINT32 sampLen;
								if (sbit == 16)
								{
									sampLen = SubchunkSize / 2;
									str.Format(_T("%d\r\n"), sampLen);
									arrLen += str;
									for (UINT32 n = 0; n < sampLen; n++)
									{
										str.Format(_T("%d,\r\n"), *pIn++);
										datFile.Write(str, str.GetLength());
									}
									str = _T("};\r\n");
									datFile.Write(str, str.GetLength());
								}
								else if (sbit == 24)
								{

								}
							}
						}
						else
						{
							if (creatCfile)
							{
								pIn = (INT16*)(&param->dataBuff[0]);
								pOut = (INT16*)(&param->dataBuffOut[0]);
								str.Format(_T("%d\r\n"), SubchunkSize / NumChannels);
								arrLen += str;
								for (UINT32 i = 0; i < (SubchunkSize / NumChannels); i++)
								{
									pOut[i] = *pIn;
									str.Format(_T("%d,\r\n"), *pIn);
									datFile.Write(str, str.GetLength());
									pIn += 2;
								}
								str = _T("};\r\n");
								datFile.Write(str, str.GetLength());
							}
							binFile.Write((UINT8 *)(&param->dataBuffOut[0]), SubchunkSize / NumChannels);
						}
						break;
					}
					dataCount = SubchunkSize / NumChannels;
					str = wavFile.GetFileName();
					char buf[256], buf1[256];
					int n = str.Find(_T(" "), 0);
					if (n != -1)
					{
						memcpy(buf, str.GetBuffer(n), n);
						buf[n] = 0;
						n++;
						int j = str.Find(_T(" "), n);
						if (j != -1)
						{
							str2 = str.Mid(n, j - n);
							memcpy(buf1, str2.GetBuffer(), n);
							buf1[str2.GetLength()] = 0;
							str.Empty();
							str2.Empty();
						}
						else
						{
							buf1[0] = _T('0');
							buf1[1] = 0;
						}
					}
					else
					{
						buf[0] = _T('0');
						buf[1] = 0;
						buf1[0] = _T('0');
						buf1[1] = 0;
					}
					str.Format(_T("%s,%s,0x%x,0x%x,\r\n"), buf, buf1, dataAddr, dataCount);
					mapFile.Write(str.GetBuffer(), str.GetLength());
					dataAddr += dataCount;
					break;
				}
				else
				{
					curPoint = wavFile.GetPosition();
					wavFile.Seek(curPoint + SubchunkSize, SEEK_SET);
				}
				
			}			
			wavFile.Close();
		}
		else
		{
			AfxMessageBox(_T("�޷���\r\n") + mList->GetItemText(cnt, 2), MB_ICONERROR);
			goto __LAB_EXIT;
		}
		cnt++;
		totalFile--;
		if (creatCfile)
		{
			datFile.Close();
		}
		Sleep(5);
	}
	//binFile.Seek(0, SEEK_SET);
	//binFile.Write(param->headBuff, BIN_FILE_HEAD_LEN);//д��һ��ͷ��Ϣ
	param->binLength = binFile.GetLength();
	binFile.Close();
	str = _T("};\r\n");
	mapFile.Write(str.GetBuffer(), str.GetLength());
	mapFile.Write(DRUM_OUT_FILE_END,sizeof(DRUM_OUT_FILE_END)-1);
	mapFile.Write(STR_LINEFEED, STR_LINEFEED_SIZE);
	mapFile.Close();

	if (creatCfile)
	{
		str = param->outPath + _T("\\DrumDatLen.h");
		brn = mapFile.Open(str, CFile::modeCreate | CFile::modeWrite);
		if (!brn)
		{
			AfxMessageBox(_T("�޷������Ļ���������ļ�"), MB_ICONERROR);
			goto __LAB_EXIT;
		}
		mapFile.Write(arrLen.GetBuffer(), arrLen.GetLength());
		mapFile.Write(_T("\r\n"), 2);
		mapFile.Write(extFileName.GetBuffer(), extFileName.GetLength());
		mapFile.Close();
	}

	::PostMessage(::AfxGetMainWnd()->m_hWnd, WM_MYMSG, WM_MYMSG_CREATE_END, param->binLength);
__LAB_EXIT:
	str.Empty();
	//wavFile.Close();
	//binFile.Close();
	//mapFile.Close();
	myDlg->DlgCtrlEnable(TRUE);
	AfxEndThread(0);
	return 0;
}

LRESULT CMIDI2CDlg::OnUserThreadend(WPARAM wParam, LPARAM lParam)
{
	UINT32 msg = wParam;
	CString str;
	//T_threadParamCreateFile *param = (T_threadParamCreateFile *)lParam;

	switch (msg)
	{
	case WM_MYMSG_CREATE_START:
		DlgCtrlEnable(false);
		break;
	case WM_MYMSG_CREATE_END:
		GetDlgItem(IDC_STATIC)->SetWindowText(_T("���"));
		DlgCtrlEnable(true);
		str.Format(_T("���ݳ���Ϊ:%d�ֽ�"), lParam);
		MessageBox(str, _T("���"), MB_OK | MB_ICONMASK);
		break;
	default:
		//AfxMessageBox("δ֪��Ϣ", MB_ICONERROR);
		MessageBox(_T("δ֪��Ϣ"), _T("����"), MB_OK | MB_ICONERROR);
		break;
	}
	return LRESULT();
}

BOOL CMIDI2CDlg::PreTranslateMessage(MSG * pMsg)
{
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}

	return  CDialog::PreTranslateMessage(pMsg);
}

void CMIDI2CDlg::DlgCtrlEnable(BOOL enable)
{
	GetDlgItem(IDC_EDFOLDER)->EnableWindow(enable);
	GetDlgItem(IDC_BUOPENPATH)->EnableWindow(enable);
	GetDlgItem(IDC_BUADDFILE)->EnableWindow(enable);
	GetDlgItem(IDC_BUSCANFILE)->EnableWindow(enable);
	GetDlgItem(IDC_BUCREATE)->EnableWindow(enable);
	GetDlgItem(IDC_BUCLEARLIST)->EnableWindow(enable);
	GetDlgItem(IDC_COMBO_SAMP)->EnableWindow(enable);
	GetDlgItem(IDC_RADIO_16)->EnableWindow(enable);
	GetDlgItem(IDC_RADIO_24)->EnableWindow(enable);
	GetDlgItem(IDC_CHECK_ADDMAP)->EnableWindow(enable);
	//GetDlgItem(IDC_LISTFILE)->EnableWindow(enable);
}

void CMIDI2CDlg::ViewLastError(LPTSTR lpMsg)
{
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();//��ȡ�������

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	CString strErr;
	strErr.Format(_T("%s-%s"), lpMsg, lpMsgBuf);
	MessageBox(strErr, _T("����"), MB_OK | MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

void CMIDI2CDlg::OnBnClickedCheckAddmap()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if(check_box.GetCheck())
	{
		addMapInfo = 1;
	}
	else
	{
		addMapInfo = 0;
	}

	
}




void CMIDI2CDlg::OnBnClickedCheckCreadC()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CButton* cb;
	cb = (CButton*)GetDlgItem(IDC_CHECK_ADDMAP);
	cb->GetCheck();

	
}
