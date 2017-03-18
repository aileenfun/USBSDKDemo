
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
HWND m_hwnd;
cv::VideoWriter h_vw;
volatile bool snap;
volatile bool save_all;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
enum ReqValue
{
	TRIGMODE=0xD0,IMGDISP,EXPOGAIN,GAIN,EXPO,MIRROR,RCEXTR,TRIGPERIOD,RSTHW,SOFTTRIG,RSTSENSOR
};
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
	DDX_Control(pDX, IDC_COMBOTrigMode, cbTrigMode);
	DDX_Control(pDX, IDC_EDITHwTrigFreq, eFpgaFreq);
	DDX_Control(pDX, IDC_EDITGainValue, eGainValue);
	DDX_Control(pDX, IDC_EDITExpoValue, eExpoValue);
	DDX_Control(pDX, IDC_CHECKAutoGain, cbAutoGain);
	DDX_Control(pDX, IDC_CHECKAutoExpo, cbAutoExpo);
	DDX_Control(pDX, IDC_CHECK_SAVEALL, m_chk_save_all);
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
	ON_EN_CHANGE(IDC_EDITGainValue, &CUsbControlDlg::setGainValue)
	ON_BN_CLICKED(IDC_CHECKAutoGain, &CUsbControlDlg::setAutoGainExpo)
	ON_BN_CLICKED(IDC_CHECKAutoExpo, &CUsbControlDlg::setAutoGainExpo)
	ON_EN_CHANGE(IDC_EDITExpoValue, &CUsbControlDlg::setExpoValue)
	ON_EN_CHANGE(IDC_EDITHwTrigFreq, &CUsbControlDlg::setFpgaFreq)
	ON_BN_CLICKED(IDC_BUTTONSoftTrig, &CUsbControlDlg::OnBnClickedButtonsofttrig)
	ON_BN_CLICKED(IDC_CHECK_SAVEALL, &CUsbControlDlg::OnBnClickedCheckSaveall)
END_MESSAGE_MAP()


BOOL CUsbControlDlg::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN )
	{
		if(pMsg->wParam == VK_RETURN)// || pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;                // Do not process further
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

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
	CRect cRect,wRect,mRect;
	GetDesktopWindow()->GetWindowRect(wRect);
	GetWindowRect(cRect);
	mRect.right=wRect.right-50;
	mRect.bottom=wRect.bottom-50;
	mRect.left=mRect.right-cRect.Width();
	mRect.top=mRect.bottom-cRect.Height();
	MoveWindow(mRect);
	//------------------------------------
	m_pFileRbf=new CFile();
	m_pVideoDlg=new CVideoDlg();

	m_pVideoDlg->Create(IDD_DLG_VIDEO,this);
	m_pVideoDlg->ShowWindow(FALSE);
	m_hDisplayDC=m_pVideoDlg->GetDisplayDC()->m_hDC;

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
	//icct_factory fac_func=reinterpret_cast<icct_factory>(
	//	::GetProcAddress(dll_handle,"create_CCTAPI"));

	cbTrigMode.AddString(L"AutoTrig");
	
	cbTrigMode.InsertString(1,L"SoftTrig");
	cbTrigMode.InsertString(2,L"FpgaTrig");
	cbTrigMode.InsertString(3,L"FromOutSide");
	cbTrigMode.SetCurSel(0);
	cbTrigMode.SetMinVisibleItems(4);
	cbAutoExpo.SetCheck(1);
	cbAutoGain.SetCheck(1);
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
void BMPHeader(int lWidth, int lHeight,byte* m_buf)
{
	int mlBpp=8;
	bool lReverse=true;
	BITMAPFILEHEADER bhh;
	BITMAPINFOHEADER bih;
	memset(&bhh,0,sizeof(BITMAPFILEHEADER));
	memset(&bih,0,sizeof(BITMAPINFOHEADER));

	int widthStep				=	(((lWidth * mlBpp) + 31) & (~31)) / 8; //每行实际占用的大小（每行都被填充到一个4字节边界）
	int QUADSize 				= 	mlBpp==8?sizeof(RGBQUAD)*256:0;

	//构造彩色图的文件头
	bhh.bfOffBits				=	(DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + QUADSize; 
	bhh.bfSize					=	(DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + QUADSize + widthStep*lHeight;  
	bhh.bfReserved1				=	0; 
	bhh.bfReserved2				=	0;
	bhh.bfType					=	0x4d42;  

	//构造彩色图的信息头
	bih.biBitCount				=	mlBpp;
	bih.biSize					=	sizeof(BITMAPINFOHEADER);
	bih.biHeight				=	(lReverse?-1:1)*lHeight;
	bih.biWidth					=	lWidth;  
	bih.biPlanes				=	1;
	bih.biCompression			=	BI_RGB;
	bih.biSizeImage				=	widthStep*lHeight;  
	bih.biXPelsPerMeter			=	0;  
	bih.biYPelsPerMeter			=	0;  
	bih.biClrUsed				=	0;  
	bih.biClrImportant			=	0;  
	
	//构造灰度图的调色版
	RGBQUAD rgbquad[256];
	if(mlBpp==8)
	{
		for(int i=0;i<256;i++)
		{
			rgbquad[i].rgbBlue=i;
			rgbquad[i].rgbGreen=i;
			rgbquad[i].rgbRed=i;
			rgbquad[i].rgbReserved=0;
		}
	}

	int DIBSize = widthStep * lHeight;
	//TRACE(_T("DIBSIze is %d"),DIBSize);

	bool b_save_file	=true;
	if(b_save_file)
	{
		CString strName;
		CString camFolder;
		camFolder.Format(L"d:\\c6UDP\\cam%d",0);
		if(CreateDirectory(camFolder,NULL)||ERROR_ALREADY_EXISTS == GetLastError())
		{
			int iFileIndex=1;
			do 
			{
				strName.Format(L"d:\\c6UDP\\cam%d\\V_%d.bmp",0,iFileIndex);
				++iFileIndex;
			} while (_waccess(strName,0)==0);
			CT2CA pszConvertedAnsiString (strName);
			std::string cvfilename(pszConvertedAnsiString);
			
			CFile file;  
	if(file.Open(strName,CFile::modeWrite | CFile::modeCreate))  
	{
		file.Write((LPSTR)&bhh,sizeof(BITMAPFILEHEADER));  
		file.Write((LPSTR)&bih,sizeof(BITMAPINFOHEADER));  
		if(mlBpp==8) file.Write(&rgbquad,sizeof(RGBQUAD)*256);
		file.Write(m_buf,DIBSize);  
		file.Close();  
		return ;  
	}  
		}
	}

	
}

void _stdcall RawCallBack(LPVOID lpParam,LPVOID lpUser)
{
	BYTE *pDataBuffer = (BYTE*)lpParam;
	CUsbControlDlg *pDlg=(CUsbControlDlg*)lpUser;
	cv::Mat frame(g_height,g_width,CV_8UC1,pDataBuffer);
	cv::imshow("disp",frame);
	cv::waitKey(1);
	//BMPHeader(g_width,g_height,pDataBuffer);
		if(snap||save_all)
	{
		CString strName;
		CString camFolder;
		//camFolder.Format(L"cam%d",0);
		if(1)//CreateDirectory(camFolder,NULL)||ERROR_ALREADY_EXISTS == GetLastError())
		{
			int iFileIndex=1;
			do 
			{
				strName.Format(L"V_%d.bmp",iFileIndex);
				++iFileIndex;
			} while (_waccess(strName,0)==0);
			CT2CA pszConvertedAnsiString (strName);
			std::string cvfilename(pszConvertedAnsiString);

		cv::imwrite(cvfilename,frame);
		snap=false;
		}
		}
	//h_vw.write(frame);//save video operation
}
void CUsbControlDlg::OnBnClickedBtnVideocapture()
{
	// TODO: 在此添加控件通知处理程序代码
	//m_pVideoDlg->ShowWindow(TRUE);
	
	cv::namedWindow("disp");
	CyUsb_Init();
	m_byData[0]=(g_width&0xff<<8)>>8;
	m_byData[1]=(g_width&0xff);
	m_byData[2]=(g_height&0xff<<8)>>8;
	m_byData[3]=(g_height&0xff);
	m_sUsbOrder.ReqCode=IMGDISP;
	m_sUsbOrder.DataBytes=4;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
	if(h_cctapi->startCap(g_height,g_width,RawCallBack)<0)
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"USB设备打开失败！");
		return;
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"采集中...");
		CheckRadioButton(IDC_RADIO_NORMAL,IDC_RADIO_XYMIRROR,IDC_RADIO_NORMAL);
		m_bUsbOpen=TRUE;
	}
	//std::string filename="videofile.avi";
	//cv::Size videosize=cv::Size(g_width,g_height);
	////h_vw.open(filename,CV_FOURCC('X','V','I','D'),15,videosize,0);
	//if(!h_vw.isOpened())
	//{
	//	SetDlgItemText(IDC_STATIC_TEXT,L"保存视频失败。");
	//	return;
	//}

}


void CUsbControlDlg::OnBnClickedBtnStopcapture()
{
	if(h_cctapi->stopCap()!=0)
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"尚未采集");
		return;
	}
	cv::destroyWindow("disp");
	SetDlgItemText(IDC_STATIC_TEXT,L" ");
	m_bUsbOpen=FALSE;
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
	//h_cctapi->setMirrorType(DataProcessType(m_iProcType));
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


void CUsbControlDlg::setTrigMode()
{
	if(!m_bUsbOpen)
		return;
	CString s_temp;
	UpdateData(true);
	eFpgaFreq.GetWindowText(s_temp);
	int fpgafreq= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	m_byData[0]=cbTrigMode.GetCurSel()&0xff;
	m_byData[1]=fpgafreq&0xff;
	m_sUsbOrder.DataBytes=2;
	m_sUsbOrder.ReqCode=TRIGMODE;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
	if(cbTrigMode.GetCurSel()==2&&fpgafreq<=0)
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"Need Fpga Freq");
	}
	return;
}


void CUsbControlDlg::setFpgaFreq()
{
	if(!m_bUsbOpen)
		return;
	CString s_temp;
	UpdateData(true);
	eFpgaFreq.GetWindowText(s_temp);
	int fpgafreq= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	if(cbTrigMode.GetCurSel()==2&&fpgafreq>0)
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


void CUsbControlDlg::setGainValue()
{
	if(!m_bUsbOpen)
		return;
	CString s_temp;
	UpdateData(true);
	eGainValue.GetWindowText(s_temp);
	int GainValue= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	if(cbAutoExpo.GetCheck()==false&&GainValue>0)
	{
		m_byData[0]=GainValue&0xff;
		m_sUsbOrder.DataBytes=1;
		m_sUsbOrder.ReqCode=GAIN;
		m_sUsbOrder.Direction=0;
		SendOrder(&m_sUsbOrder);
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"Check Gain?");
	}
}


void CUsbControlDlg::setAutoGainExpo()
{
	if(!m_bUsbOpen)
		return;
	m_byData[0]=cbAutoExpo.GetCheck()==true?1:0;
	m_byData[0]+=(cbAutoGain.GetCheck()==true?1:0)<<1;
	m_sUsbOrder.DataBytes=1;
	m_sUsbOrder.ReqCode=EXPOGAIN;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
}


void CUsbControlDlg::setExpoValue()
{
	if(!m_bUsbOpen)
		return;
	CString s_temp;
	UpdateData(true);
	eExpoValue.GetWindowText(s_temp);
	int ExpoValue= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	if(cbAutoExpo.GetCheck()==false&&ExpoValue>0)
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




void CUsbControlDlg::OnBnClickedButtonsofttrig()
{
	if(cbTrigMode.GetCurSel()==1)
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


void CUsbControlDlg::OnBnClickedCheckSaveall()
{
	// TODO: Add your control notification handler code here
	save_all=m_chk_save_all.GetCheck();
}
