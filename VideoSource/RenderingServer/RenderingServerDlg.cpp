
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

#include "RTMPPushTask.h"
#include "VideoPullTask.h"
void CRenderingServerDlg::OnBnClickedButtonRTMPStart()  //����(�����豸)
{
	//SINGLETON(CScheduleServer).start_rtmp_push(123);

	//m_rtmp_start_btn.EnableWindow(FALSE);
	//m_rtmp_stop_btn.EnableWindow(TRUE);

	LiveCastRequest req;
	//req.url = url;

	LiveCastResponse res = SINGLETON(CScheduleServer).query_livecast_response(req);

	/*if(false == res.num.empty())
	{
		//Json::Value json_value;
		json_value["StreamNo"] = Json::Value(res.num);
		json_value["StreamUrl"] = Json::Value(res.url);
		json_value["ExpireTime"] = Json::Value(res.expire);

		Json::FastWriter fast_writer;
		return return_response(0, "success", fast_writer.write(json_value));//return fast_writer.write(json_value);
	}
	else*/
	{
		unsigned long id = 123;//timeGetTime() & 0xffffff;
		//string hls_url = "";
		//string rtmp_url = "";
		CRTMPPushTask* rtmp_push_task = NULL;
		CDataRecvTask* sdk_recv_task = NULL;//CDPSDKLivePullTask* sdk_recv_task = NULL;

		//RTMP
		{
			RTMP_PUSH_TASK_INFO task_info;

			task_info.task_id = id;
			task_info.ua_id = id;

			task_info.rtmp_url = "rtmp://";
			task_info.rtmp_url += SINGLETON(CConfigBox).get_property("HLSServer", "localhost");
			task_info.rtmp_url += ":" + SINGLETON(CConfigBox).get_property("RTMPServerPort", "1935");
			task_info.rtmp_url += "/hls/";
			task_info.rtmp_url += MiscTools::parse_type_to_string<unsigned long>(task_info.ua_id);
			
			rtmp_push_task = new CRTMPPushTask(task_info);

			if(false == rtmp_push_task->is_initialized())
			{
				return;
			}

			if(SS_NoErr != SINGLETON(CScheduleServer).add_task(rtmp_push_task, task_info.task_id))
			{
				delete rtmp_push_task;
				rtmp_push_task = NULL;

				return;
			}
		}

		//SDK
		{
			SDK_RECV_TASK_INFO task_info;

			task_info.task_id = id + 1;
			task_info.ua_id = id;

			task_info.data_url = "720p.yuv";
			//task_info.data_url = "cif.yuv";
			task_info.fps = 30;
			task_info.video_width = 1280;//352;
			task_info.video_height = 720;//288;

			sdk_recv_task = new CVideoPullTask(task_info);

			if(false == sdk_recv_task->is_initialized())
			{
				rtmp_push_task->shutdown();
				return;
			}

			if(SS_NoErr != SINGLETON(CScheduleServer).add_task(sdk_recv_task, task_info.task_id))
			{
				delete sdk_recv_task;
				sdk_recv_task = NULL;

				rtmp_push_task->shutdown();

				return;
			}

			/*hls_url = "http://";
			hls_url += SINGLETON(CConfigBox).get_property("HLSServer", "localhost");
			hls_url += ":";
			hls_url += SINGLETON(CConfigBox).get_property("HLSServerPort", "8080");
			hls_url += "/hls/";
			hls_url += MiscTools::parse_type_to_string<unsigned long>(id);
			hls_url += ".m3u8";

			rtmp_url = "rtmp://";
			rtmp_url += SINGLETON(CConfigBox).get_property("HLSServer", "localhost");
			rtmp_url += ":";
			rtmp_url += SINGLETON(CConfigBox).get_property("RTMPServerPort", "1935");
			rtmp_url += "/hls/";
			rtmp_url += MiscTools::parse_type_to_string<unsigned long>(id);*/
		}

		//success////////////////////////////////////////////////////////////////////////
		SINGLETON(CScheduleServer)._push_task_map[id] = rtmp_push_task;
		SINGLETON(CScheduleServer)._pull_task_map[id] = sdk_recv_task;

		res.num = MiscTools::parse_type_to_string<unsigned long>(id);
		//res.url = hls_url;
		//res.url2 = rtmp_url;
		res.expire = "300";
		SINGLETON(CScheduleServer).insert_livecast_map(req, res);

		//Json::Value json_value;
		//json_value["StreamNo"] = Json::Value(MiscTools::parse_type_to_string<unsigned long>(id));
		//json_value["StreamUrl"] = Json::Value(hls_url);
		//json_value["StreamUrl2"] = Json::Value(rtmp_url);
		//json_value["ExpireTime"] = Json::Value("300");

		//Json::FastWriter fast_writer;
		return;//return return_response(0, "success", fast_writer.write(json_value), 6000);//return fast_writer.write(json_value);
	}
}

void CRenderingServerDlg::OnBnClickedButtonRTMPStop()  //�Ͽ�����
{
	SINGLETON(CScheduleServer).stop_rtmp_push(123);

	//shutdown_audio_sampler();

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
