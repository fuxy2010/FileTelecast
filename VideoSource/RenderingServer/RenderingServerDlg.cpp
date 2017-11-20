
// RenderingServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RenderingServer.h"
#include "RenderingServerDlg.h"
#include "afxdialogex.h"
#include "Sizescale.h"

#include "ScheduleServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//for AAC
#ifndef INP_BUFFER_SIZE
#define INP_BUFFER_SIZE 960//2048
#endif

using namespace ScheduleServer;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRenderingServerDlg dialog
CRenderingServerDlg::CRenderingServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRenderingServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CRenderingServerDlg::~CRenderingServerDlg()
{
}

void CRenderingServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_RTMP_START, m_rtmp_start_btn);
	DDX_Control(pDX, IDC_BUTTON_RTMP_STOP, m_rtmp_stop_btn);
	DDX_Control(pDX, IDC_EDIT_HTTP_SERVICE, m_HttpServiceStatus);
}

BEGIN_MESSAGE_MAP(CRenderingServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CRenderingServerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CRenderingServerDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_RTMP_START, &CRenderingServerDlg::OnBnClickedButtonRTMPStart)
	ON_BN_CLICKED(IDC_BUTTON_RTMP_STOP, &CRenderingServerDlg::OnBnClickedButtonRTMPStop)
	ON_BN_CLICKED(IDC_BUTTON_DH, &CRenderingServerDlg::OnBnClickedButtonDh)
	ON_BN_CLICKED(IDC_BUTTON_H264, &CRenderingServerDlg::OnBnClickedButtonH264)
END_MESSAGE_MAP()


// CRenderingServerDlg message handlers
void CRenderingServerDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//CDialogEx::OnOK();
}

void CRenderingServerDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

void CRenderingServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRenderingServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRenderingServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CRenderingServerDlg::OnInitDialog()  //�����ڳ�ʼ��
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_rtmp_start_btn.EnableWindow(TRUE);
	m_rtmp_stop_btn.EnableWindow(FALSE);
	
	SetWindowText("RenderingServer");

	//����������·����Ϊ��ǰ·��
	char cur_path[MAX_PATH];

	::ZeroMemory(cur_path, sizeof(cur_path));
	::GetModuleFileName(NULL, cur_path, sizeof(cur_path));

	std::string cur_path_str(cur_path);
	cur_path_str = cur_path_str.substr(0, cur_path_str.rfind("\\")) + "\\";

	::SetCurrentDirectory(cur_path_str.c_str());

	//������־
	std::map<int, std::string> log_file_map;
	log_file_map[0] = "RenderingServer.log";	
	SINGLETON(CLogger).open("", log_file_map, cur_path_str + "log");
	LOG_SET_LEVEL(HIGHEST_LOG_LEVEL);

	//��ȡ�����ļ�
	SINGLETON(CConfigBox).load("config.ini");

	//��������
	try
	{
		if(SS_NoErr != SINGLETON(CScheduleServer).start(cur_path_str))//��������
		{
			AfxMessageBox("Fail in initializing UserAgent!");
		}
	}
	catch(...)
	{
		LOG_WRITE("<FAIL> ScheduleServer failed in stopping!", 1, true);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRenderingServerDlg::OnClose()  //�������������Ͻǡ��رա���ť�Ĳ���
{
	// TODO: Add your message handler code here and/or call default
	//isRuning = FALSE;
	//releaseResource();

	SINGLETON(CScheduleServer).shutdown();

	//shutdown_audio_sampler();

	CDialogEx::OnClose();
}

void CRenderingServerDlg::OnBnClickedButtonRTMPStart()  //����(�����豸)
{
	if(false)
	{
		SINGLETON(CScheduleServer).add_video_pull_task(111, "720p.yuv");
		return;
	}

	//遍历当前窗口
	string caption = SINGLETON(CConfigBox).get_property("Window", "");

	if(false == caption.empty())
	{
		CWnd* pDesktopWnd = CWnd::GetDesktopWindow();

		//2.获得一个子窗口
		CWnd* pWnd = pDesktopWnd->GetWindow(GW_CHILD);
		//3.循环取得桌面下的所有子窗口
		while(pWnd != NULL)
		{
			//获得窗口类名
			CString strClassName = _T("");
			::GetClassName(pWnd->GetSafeHwnd(),strClassName.GetBuffer(256),256);

			//获得窗口标题
			CString strWindowText = _T("");
			::GetWindowText(pWnd->GetSafeHwnd(),strWindowText.GetBuffer(256),256);

			//AfxMessageBox(strWindowText);
			if(-1 != strWindowText.Find(caption.c_str()))
			{
				CRect rect;
				pWnd->GetWindowRect(&rect);
				TRACE("\n%s", strWindowText);
				TRACE("\n%s %d %d", strWindowText, rect.Width(), rect.Height());

				caption = strWindowText;

				//pWnd->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);//窗口置顶
				//::SetWindowPos(pWnd->GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
				//::SetForegroundWindow(pWnd->GetSafeHwnd());//激活窗口但不置顶

				pWnd->GetWindowRect(&rect);
				TRACE("\n%s %d %d", strWindowText, rect.Width(), rect.Height());

				break;
			}

			//继续下一个子窗口
			pWnd = pWnd->GetWindow(GW_HWNDNEXT);
		}

		if(NULL == pWnd) caption = "";
	}
	//SINGLETON(CScheduleServer).add_video_pull_task(111, "test.yuv");
	//SINGLETON(CScheduleServer).add_video_pull_task(222, "720p.yuv");
	SINGLETON(CScheduleServer).add_capture_screen_task(111, caption);

	m_rtmp_start_btn.EnableWindow(FALSE);
	m_rtmp_stop_btn.EnableWindow(TRUE);
}

void CRenderingServerDlg::OnBnClickedButtonRTMPStop()  //�Ͽ�����
{
	if(false)
	{
		//窗口置顶
		string caption = SINGLETON(CConfigBox).get_property("Window", "");

		if(false == caption.empty())
		{
			CWnd* pWnd = CWnd::GetDesktopWindow()->GetWindow(GW_CHILD);
		
			while(pWnd != NULL)
			{
				CString strClassName = _T("");
				::GetClassName(pWnd->GetSafeHwnd(),strClassName.GetBuffer(256),256);

				CString strWindowText = _T("");
				::GetWindowText(pWnd->GetSafeHwnd(),strWindowText.GetBuffer(256),256);

				if(-1 != strWindowText.Find(caption.c_str()))
				{
					//pWnd->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);//窗口置顶
					//::SetWindowPos(pWnd->GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
					::SetForegroundWindow(pWnd->GetSafeHwnd());//激活窗口但不置顶
					break;
				}

				pWnd = pWnd->GetWindow(GW_HWNDNEXT);
			}
		}

		//光标居中
		SetCursorPos(GetSystemMetrics(SM_CXSCREEN) / 2,  GetSystemMetrics(SM_CYSCREEN) / 2);

		//mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		//mouse_event(MOUSEEVENTF_MOVE, 0, -100, 0, 0);
		//mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

		mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -50, 0);
		return;
	}

	//SINGLETON(CScheduleServer).stop_rtmp_push(123);

	//shutdown_audio_sampler();

	SINGLETON(CScheduleServer).remove_capture_screen_task();

	m_rtmp_start_btn.EnableWindow(TRUE);
	m_rtmp_stop_btn.EnableWindow(FALSE);
}

void CRenderingServerDlg::OnBnClickedButtonDh()
{
	// TODO: Add your control notification handler code here
	//SINGLETON(CScheduleServer).dh_test();
	//SINGLETON(CScheduleServer).rtsp_test();
}


void CRenderingServerDlg::OnBnClickedButtonH264()
{
	// TODO: Add your control notification handler code here
	SINGLETON(CScheduleServer).h264_test();
	//SINGLETON(CScheduleServer).rtsp_test();
	//SINGLETON(CScheduleServer).rtsp_test2();
}
