
// UsbControlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UsbControl.h"	
#include "UsbControlDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

int g_width=752;
int g_height=480;
cv::VideoWriter h_vw;
volatile bool snap;
	volatile bool b_timer1s;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
	HWND hwnd_mainwindow;
	extern CUsbControlDlg* mainwindow;
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
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
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CUsbControlDlg 对话框

CUsbControlDlg::CUsbControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUsbControlDlg::IDD, pParent)
	, m_iProcType(0)
	, m_sEdit_Width(_T(""))
	, m_sEdit_Height(_T(""))
	//, c_AutoExpo(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pFileRbf=NULL;
	m_bUsbOpen=FALSE;
	m_bSendData=FALSE;
	memset(m_byData,0,sizeof(BYTE)*64);
	m_sUsbOrder.pData=m_byData;
	m_pVideoDlg=NULL;

	m_hDisplayDC=NULL;
	m_hThread=NULL;
	m_pReadBuff=new char[ReadDataBytes];
	memset(m_pReadBuff,0,sizeof(char)*ReadDataBytes);

	m_bCloseWnd=FALSE;
	m_lBytePerSecond=0;
	m_CyDriver=NEW_DRIVER;
	m_pBrush=NULL;
	m_pVideoDataFile=NULL;
	m_bReview=FALSE;
	m_bSave=FALSE;
	m_Init = FALSE;
	snap=false;
	char temp[4];
}

CUsbControlDlg::~CUsbControlDlg()
{
	if(m_pReadBuff!=NULL)
	{
		delete[] m_pReadBuff;
		m_pReadBuff=NULL;
	}
}

void CUsbControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_NORMAL, m_iProcType);
	DDX_Control(pDX, IDC_EDIT1, m_Edit_Width);
	DDX_Control(pDX, IDC_EDIT2, m_Edit_Height);
	DDX_Text(pDX, IDC_EDIT1, m_sEdit_Width);
	DDX_Text(pDX, IDC_EDIT2, m_sEdit_Height);
	DDX_Control(pDX, IDC_COMBODev, m_comboDevNum);
	DDX_Control(pDX,IDC_COMBOTrigMode,m_ComboTrigMode);
	DDX_Control(pDX, IDC_EDITFPGAFreq, m_ceFpgaFreq);
	DDX_Control(pDX, IDC_CHECK2, c_AutoExpo);
	DDX_Control(pDX, IDC_CHECK3, c_AutoGain);
	DDX_Control(pDX, IDC_EDITExpoValue, ce_ExpoValue);
	DDX_Control(pDX, IDC_EDITGainValue, ce_GainValue);
}

BEGIN_MESSAGE_MAP(CUsbControlDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_VIDEOCAPTURE, &CUsbControlDlg::OnBnClickedBtnVideocapture)
	ON_BN_CLICKED(IDC_BTN_STOPCAPTURE, &CUsbControlDlg::OnBnClickedBtnStopcapture)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_RADIO_NORMAL, &CUsbControlDlg::OnBnClickedRadioProcType)
	ON_BN_CLICKED(IDC_RADIO_XMIRROR, &CUsbControlDlg::OnBnClickedRadioProcType)
	ON_BN_CLICKED(IDC_RADIO_YMIRROR, &CUsbControlDlg::OnBnClickedRadioProcType)
	ON_BN_CLICKED(IDC_RADIO_XYMIRROR, &CUsbControlDlg::OnBnClickedRadioProcType)
	ON_EN_CHANGE(IDC_EDIT1, &CUsbControlDlg::OnEnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT2, &CUsbControlDlg::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_BTN_SNAP, &CUsbControlDlg::OnBnClickedBnSnap)
ON_CBN_SELCHANGE(IDC_COMBOTrigMode, &CUsbControlDlg::setTrigMode)
ON_EN_UPDATE(IDC_EDITFPGAFreq, &CUsbControlDlg::setFpgaFreq)
ON_BN_CLICKED(IDC_CHECK2, &CUsbControlDlg::setExpoGain)
ON_BN_CLICKED(IDC_CHECK3, &CUsbControlDlg::setExpoGain)
ON_EN_UPDATE(IDC_EDITExpoValue, &CUsbControlDlg::setExpoValue)
ON_EN_CHANGE(IDC_EDITGainValue, &CUsbControlDlg::setGainValue)
ON_BN_CLICKED(IDC_BtnSoftTrig, &CUsbControlDlg::OnBnClickedBtnsofttrig)
END_MESSAGE_MAP()


// CUsbControlDlg 消息处理程序

BOOL CUsbControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//------------------------------------
	//mainwindow=GetDesktopWindow();
	hwnd_mainwindow=::GetActiveWindow();
	CRect cRect,wRect,mRect;
	GetDesktopWindow()->GetWindowRect(wRect);
	GetWindowRect(cRect);
	mRect.right=wRect.right-50;
	mRect.bottom=wRect.bottom-50;
	mRect.left=mRect.right-cRect.Width();
	mRect.top=mRect.bottom-cRect.Height();
	MoveWindow(mRect);
	//------------------------------------
	SetTimer(1,1000,NULL);
	m_iRdoDriver=(int)m_CyDriver;

	m_pBrush=new CBrush[2];
	m_pBrush[0].CreateSolidBrush(RGB(99,208,242));
	m_pBrush[1].CreateSolidBrush(RGB(174,238,250));

	HINSTANCE dll_handle=::LoadLibraryA("CCTAPI.dll");
	if(!dll_handle)
	{
		std::cerr<<"unable to load dll\n";
		return TRUE;
	}
	icct_factory fac_func=reinterpret_cast<icct_factory>(
		::GetProcAddress(dll_handle,"create_CCTAPI"));
	if(!fac_func)
	{
		std::cerr<<"unable to load create_CCTAPI from dll\n";
		//return TRUE;
	}
	//h_cctapi=fac_func();
	//h_cctapi=::create_CCTAPI();
	m_ComboTrigMode.AddString(L"AutoTrig");
	m_ComboTrigMode.InsertString(1,L"FpgaTrig");
	m_ComboTrigMode.InsertString(2,L"SoftTrig");
	m_ComboTrigMode.InsertString(3,L"FromOutSide");
	m_ComboTrigMode.SetCurSel(0);
	c_AutoExpo.SetCheck(1);
	UpdateData(TRUE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUsbControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();//加载控制台窗口模型
		int temp1 = 1;
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUsbControlDlg::OnPaint()
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
HCURSOR CUsbControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void _stdcall RawCallBack(LPVOID lpParam,LPVOID lpUser)
{
	//BYTE *pDataBuffer = (BYTE*)lpParam;
	DFrameStruct *imData=(DFrameStruct*)lpParam;
	//CUsbControlDlg *pDlg=(CUsbControlDlg*)lpUser;
	cv::Mat frame(imData->height,imData->width,CV_8UC1,imData->leftData.get());
	cv::Mat frame1(imData->height,imData->width,CV_8UC1,imData->rightData.get());
	//cv::Mat colored(g_height,g_width,CV_8UC3);
	//cv::applyColorMap(frame,colored,cv::COLORMAP_JET)
	cv::imshow("disp",frame);
	cv::imshow("disp1",frame1);
	//cv::imshow("color",colored);
	cv::waitKey(10);
	if (b_timer1s==TRUE)
	{
		CString csIMU;
		IMUDataStruct *m_IMU=imData->IMUData.get();
		csIMU.Format(L"Accel       Gyro    \n x: %7d| %7d\n y:%7d|%7d\n z: %7d|%7d",
			m_IMU[0].accelData[0],m_IMU[0].gyroData[0],
			m_IMU[0].accelData[1],m_IMU[0].gyroData[1],
			m_IMU[0].accelData[2],m_IMU[0].gyroData[2]);
		mainwindow->setStatusText(csIMU);
		
	b_timer1s=FALSE;
	}

	if(snap==true)
	{
		cv::imwrite("snap.jpg",frame);
		snap=false;
	}
	//h_vw.write(frame);//save video operation
}
void CUsbControlDlg::setStatusText(CString cs)
{
	SetDlgItemText(IDC_STATIC_TEXT,cs);
	//UpdateData(TRUE);
}
void CUsbControlDlg::OnBnClickedBtnVideocapture()
{
	// TODO: 在此添加控件通知处理程序代码
	//m_pVideoDlg->ShowWindow(TRUE);
	cv::namedWindow("disp");
	cv::namedWindow("disp1");
	if(h_cctapi->startCap(g_height,g_width,RawCallBack,0)<0)
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"USB设备打开失败！");
		return;
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"采集中...");
		CheckRadioButton(IDC_RADIO_NORMAL,IDC_RADIO_XYMIRROR,IDC_RADIO_NORMAL);
	}
	/*std::string filename="videofile.avi";
	cv::Size videosize=cv::Size(g_width,g_height);
	h_vw.open(filename,CV_FOURCC('X','V','I','D'),15,videosize,0);
	if(!h_vw.isOpened())
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"保存视频失败。");
		return;
	}*/

}


void CUsbControlDlg::OnBnClickedBtnStopcapture()
{
	if(h_cctapi->stopCap()!=0)
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"尚未采集");
		return;
	}
	cv::destroyWindow("disp");
	cv::destroyWindow("disp1");
	SetDlgItemText(IDC_STATIC_TEXT,L" ");
}

void CUsbControlDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	// TODO: 在此处添加消息处理程序代码
	KillTimer(1);

	m_bCloseWnd=TRUE;

	Sleep(100);
	if(m_pVideoDlg!=NULL)
	{
		delete m_pVideoDlg;
		m_pVideoDlg=NULL;
	}
	//CloseUsb();
	//CyUsb_Destroy();
	if(m_pBrush!=NULL)
	{
		for(int i=0;i<2;++i)
		{
			if(m_pBrush[i].m_hObject!=NULL)
			{
				m_pBrush[i].DeleteObject();
			}
		}
		delete[] m_pBrush;
		m_pBrush=NULL;
	}
}

void CUsbControlDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int iFrame=0;
	CString str;
	switch(nIDEvent)
	{
	case 1:
		{
			//if(m_pDataProcess!=NULL)
			//{
			//	m_pDataProcess->GetFrameCount(iFrame);
			//	str.Format(L"%d Fps     %0.4f MBs",iFrame,float(m_lBytePerSecond)/1024.0/1024.0);
			//	m_lBytePerSecond=0;
			//	if(m_pVideoDlg!=NULL)
			//	{
			//		m_pVideoDlg->SetWindowText(str);
			//	}
			//}
			b_timer1s=true;
			UpdateData(TRUE);
		}
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CUsbControlDlg::OnBnClickedRadioProcType()
{
	UpdateData(TRUE);
	h_cctapi->setMirrorType(DataProcessType(m_iProcType));
}


HBRUSH CUsbControlDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	// TODO:  在此更改 DC 的任何特性
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	int ID=pWnd->GetDlgCtrlID();
	if(ID==IDC_STATIC_TEXT)
	{
		pDC->SetTextColor(RGB(0,0,255));
		pDC->SetBkMode(TRANSPARENT);
	}
	switch(nCtlColor)
	{
	case CTLCOLOR_DLG:
	case CTLCOLOR_BTN:
		return m_pBrush[0];
	case CTLCOLOR_STATIC:
		return m_pBrush[0];
	default:
		return hbr;
	}
}


void CUsbControlDlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	/*-------单个字符输入，弹出信息框*/
	/*
	CString s;
	s.GetBufferSetLength(1024);
	m_Edit_Width.GetWindowTextW(s.GetBuffer(),s.GetLength());
	MessageBox(s,_T("获取图像宽度"),MB_OK);
	s.ReleaseBuffer();
	*/	
	CString s_temp;
	UpdateData(true);
	s_temp = m_sEdit_Width.GetString();
	g_width = _tstoi(s_temp);
	s_temp.ReleaseBuffer();
}


void CUsbControlDlg::OnEnChangeEdit2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CString s_temp;
	UpdateData(true);
	s_temp = m_sEdit_Height.GetString();
	g_height = _tstoi(s_temp);
	s_temp.ReleaseBuffer();
}

void CUsbControlDlg::Split(CString in, CString *out, int &outcnt, CString spliter,int maxn)
{

	int d_len=spliter.GetLength();
	int j=0;
	int n=0;
	int m_pos;
	while(1)
	{
		m_pos= in.Find(spliter,j);
		if(m_pos==-1 && j==0)
		{
			out[0]=in.Mid(0);
			outcnt=0;//-1
			break;
		}
		if((m_pos==-1 && j!=0)||(n==maxn))
		{
			out[n]=in.Mid(j,in.GetLength()-j);
			outcnt=n;
			break;
		}

		if(j==0)
		{
			out[n]=in.Mid(0,m_pos);
			j=m_pos+d_len;
		}
		else
		{
			out[n]=in.Mid(j,m_pos-j);
			j=m_pos+d_len;
		}
		n++;
	}
}

void CUsbControlDlg::OnBnClickedBnSnap()
{
	snap=true;
}


//BOOL CUsbControlDlg::PreTranslateMessage(MSG* pMsg)
//{
//	if (pMsg->message == WM_KEYDOWN &&
//		pMsg->wParam == VK_RETURN &&
//		GetFocus() == m_ceFpgaFreq)
//	{
//		switch(GetFocus())
//		{
//		case m_ceFpgaFreq:
//		default:
//		}
//		// handle return pressed in edit control
//		return TRUE; // this doesn't need processing anymore
//	}
//	return FALSE; // all other cases still need default processing
//}

void CUsbControlDlg::setFpgaFreq()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	
	CString s_temp;
	UpdateData(true);
	m_ceFpgaFreq.GetWindowText(s_temp);
	int fpgafreq= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	if(m_ComboTrigMode.GetCurSel()==2&&fpgafreq>0)
	{
		m_byData[0]=2;
		m_byData[1]=fpgafreq&0xff;
	m_sUsbOrder.DataBytes=2;
	m_sUsbOrder.ReqCode=TRIGMODE;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"Check Trig Mode");
	}

}

void CUsbControlDlg::setTrigMode()
{
	// TODO: Add your control notification handler code here

	CString s_temp;
	UpdateData(true);
	m_ceFpgaFreq.GetWindowText(s_temp);
	int fpgafreq= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	m_byData[0]=m_ComboTrigMode.GetCurSel()&0xff;
	m_byData[1]=fpgafreq&0xff;
	m_sUsbOrder.DataBytes=2;
	m_sUsbOrder.ReqCode=TRIGMODE;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
	if(m_ComboTrigMode.GetCurSel()==2&&fpgafreq<=0)
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"Neet Fpga Freq");
	}
	return;
}




void CUsbControlDlg::setExpoGain()
{
	// TODO: Add your control notification handler code here
	m_byData[0]=c_AutoExpo.GetCheck()==true?1:0;
	m_byData[0]+=(c_AutoGain.GetCheck()==true?1:0)<<1;
	m_sUsbOrder.DataBytes=1;
	m_sUsbOrder.ReqCode=EXPOGAIN;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
}


void CUsbControlDlg::setExpoValue()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here
	CString s_temp;
	UpdateData(true);
	ce_ExpoValue.GetWindowText(s_temp);
	int ExpoValue= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	if(c_AutoExpo.GetCheck()==false&&ExpoValue>0)
	{
		m_byData[0]=(ExpoValue&0xff<<8)>>8;
		m_byData[1]=ExpoValue&0xff;
		m_sUsbOrder.DataBytes=2;
		m_sUsbOrder.ReqCode=EXPO;
		m_sUsbOrder.Direction=0;
		SendOrder(&m_sUsbOrder);
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"Check Expo?");
	}

}


void CUsbControlDlg::setGainValue()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CString s_temp;
	UpdateData(true);
	ce_GainValue.GetWindowText(s_temp);
	int GainValue= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	if(c_AutoExpo.GetCheck()==false&&GainValue>0)
	{
		m_byData[0]=(GainValue&0xff<<8)>>8;
		m_byData[1]=GainValue&0xff;
		m_sUsbOrder.DataBytes=2;
		m_sUsbOrder.ReqCode=GAIN;
		m_sUsbOrder.Direction=0;
		SendOrder(&m_sUsbOrder);
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"Check Expo?");
	}
}


void CUsbControlDlg::OnBnClickedBtnsofttrig()
{
	// TODO: Add your control notification handler code her
	if(m_ComboTrigMode.GetCurSel()==3)
	{
		m_sUsbOrder.ReqCode=SOFTTRIG;
		m_sUsbOrder.DataBytes=0;
		m_sUsbOrder.Direction=0;
		SendOrder(&m_sUsbOrder);
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"Triger Mode?");
	}
}
