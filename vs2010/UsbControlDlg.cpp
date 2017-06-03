
// UsbControlDlg.cpp : ʵ���ļ�
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

int g_width=640; 
int g_height=480;
HWND m_hwnd;
//cv::VideoWriter h_vw;
volatile bool snap;
volatile bool save_all;
volatile bool b_timer1s;
extern CUsbControlDlg* mainwindow;
CDisplay *g_pdisplay;
byte * imgBuf;
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
enum ReqValue
{
	TRIGMODE=0xD0,IMGDISP,EXPOGAIN,GAIN,EXPO,MIRROR,RCEXTR,TRIGPERIOD,RSTHW,SOFTTRIG,RSTSENSOR,WRS1DATA=0xDB,WRS2DATA,IMUSAMRAT=0xDE,
	MAXEXPO,oEEADDR=0xE9,WREE,RDEE,SADDR,RDS1DATA,RDS2DATA
};
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	// ʵ��
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


// CUsbControlDlg �Ի���




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

	m_bCloseWnd=FALSE;
	m_lBytePerSecond=0;
	m_CyDriver=NEW_DRIVER;
	m_pBrush=NULL;
	m_pVideoDataFile=NULL;
	m_bReview=FALSE;
	m_bSave=FALSE;
	m_Init = FALSE;
	snap=false;
	g_pdisplay=new CDisplay();
}

CUsbControlDlg::~CUsbControlDlg()
{
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
	DDX_Control(pDX, IDC_EDITMaxExpo, MaxExpo);
	DDX_Control(pDX, IDC_EDITIMUSMPRate, IMUSampleRate);
	DDX_Control(pDX, IDC_EDITS1ADDR, S1ADDR);
	DDX_Control(pDX, IDC_EDITS1Data, S1DATA);
	DDX_Control(pDX, IDC_EDITS2Addr, S2ADDR);
	DDX_Control(pDX, IDC_EDITS2Data, S2DATA);
	DDX_Control(pDX, IDC_EDITIMUSMPRate7, EEADDR);
	DDX_Control(pDX, IDC_EDITIMUSMPRate6, EEDATA);
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
	ON_EN_CHANGE(IDC_EDITMaxExpo, &CUsbControlDlg::OnEnChangeEditmaxexpo)
	ON_EN_CHANGE(IDC_EDITIMUSMPRate, &CUsbControlDlg::OnEnChangeEditimusmprate)
	ON_BN_CLICKED(IDC_BUTTONRDS1, &CUsbControlDlg::OnBnClickedButtonrds1)
	ON_BN_CLICKED(IDC_BUTTONWRS1, &CUsbControlDlg::OnBnClickedButtonwrs1)
	ON_BN_CLICKED(IDC_BUTTONRDS2, &CUsbControlDlg::OnBnClickedButtonrds2)
	ON_BN_CLICKED(IDC_BUTTONWRS2, &CUsbControlDlg::OnBnClickedButtonwrs2)
	ON_BN_CLICKED(IDC_BUTTONEERD, &CUsbControlDlg::OnBnClickedButtoneerd)
	ON_BN_CLICKED(IDC_BUTTONEEWR, &CUsbControlDlg::OnBnClickedButtoneewr)
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

// CUsbControlDlg ��Ϣ�������

BOOL CUsbControlDlg::OnInitDialog()
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
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
	//m_pDisplay=new CDisplay();
	//m_pDisplay->Open(CDC::FromHandle(m_hDisplayDC),CRect(0,0,g_width,g_height));
	g_pdisplay->Open(CDC::FromHandle(m_hDisplayDC),CRect(0,0,g_width,g_height));
	SetTimer(1,1000,NULL);
	m_iRdoDriver=(int)m_CyDriver;

	m_pBrush=new CBrush[2];
	m_pBrush[0].CreateSolidBrush(RGB(99,208,242));
	m_pBrush[1].CreateSolidBrush(RGB(174,238,250));

	HINSTANCE dll_handle=::LoadLibraryA("CCTAPI.dll");

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
	imgBuf=new byte[g_height*g_width];
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CUsbControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();//���ؿ���̨����ģ��
		int temp1 = 1;
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CUsbControlDlg::OnPaint()
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
HCURSOR CUsbControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CUsbControlDlg::initBMPHeader(int lWidth, int lHeight,BITMAPINFO*bmi)
{
	int mlBpp=8;
	bool lReverse=true;
	BITMAPFILEHEADER bhh;
	BITMAPINFOHEADER bih;
	memset(&bhh,0,sizeof(BITMAPFILEHEADER));
	memset(&bih,0,sizeof(BITMAPINFOHEADER));

	int widthStep				=	(((lWidth * mlBpp) + 31) & (~31)) / 8; //ÿ��ʵ��ռ�õĴ�С��ÿ�ж�����䵽һ��4�ֽڱ߽磩
	int QUADSize 				= 	mlBpp==8?sizeof(RGBQUAD)*256:0;

	//�����ɫͼ���ļ�ͷ
	bhh.bfOffBits				=	(DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + QUADSize; 
	bhh.bfSize					=	(DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + QUADSize + widthStep*lHeight;  
	bhh.bfReserved1				=	0; 
	bhh.bfReserved2				=	0;
	bhh.bfType					=	0x4d42;  

	//�����ɫͼ����Ϣͷ
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

	
	//����Ҷ�ͼ�ĵ�ɫ��
	RGBQUAD rgbquad[256];
	if(mlBpp==8)
	{
		for(int i=0;i<256;i++)
		{
			rgbquad[i].rgbBlue=i;
			rgbquad[i].rgbGreen=i;
			rgbquad[i].rgbRed=i;
			rgbquad[i].rgbReserved=0;
			bmi->bmiColors[i].rgbRed=i;
			bmi->bmiColors[i].rgbGreen=i;
			bmi->bmiColors[i].rgbBlue=i;
			bmi->bmiColors[i].rgbReserved=0;
		}
	}

	//int DIBSize = widthStep * lHeight;
	memcpy(&(bmi->bmiHeader),&bih,sizeof(bih));
	//m_bmi->bmiHeader=bih;

}
void CUsbControlDlg::BMPHeader(int lWidth, int lHeight,byte* m_buf)
{
	int mlBpp=8;
	bool lReverse=true;
	BITMAPFILEHEADER bhh;
	BITMAPINFOHEADER bih;
	memset(&bhh,0,sizeof(BITMAPFILEHEADER));
	memset(&bih,0,sizeof(BITMAPINFOHEADER));

	int widthStep				=	(((lWidth * mlBpp) + 31) & (~31)) / 8; //ÿ��ʵ��ռ�õĴ�С��ÿ�ж�����䵽һ��4�ֽڱ߽磩
	int QUADSize 				= 	mlBpp==8?sizeof(RGBQUAD)*256:0;

	//�����ɫͼ���ļ�ͷ
	bhh.bfOffBits				=	(DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + QUADSize; 
	bhh.bfSize					=	(DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + QUADSize + widthStep*lHeight;  
	bhh.bfReserved1				=	0; 
	bhh.bfReserved2				=	0;
	bhh.bfType					=	0x4d42;  

	//�����ɫͼ����Ϣͷ
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
	
	//����Ҷ�ͼ�ĵ�ɫ��
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
	DFrameStruct *imData=(DFrameStruct*)lpParam;
	CUsbControlDlg *pDlg=(CUsbControlDlg*)lpUser;

	
	//memcpy(imgBuf,imData->leftData.get(),imData->height*imData->width);
	//StretchDIBits(mainwindow->m_pDisplay->m_pMemDC->m_hDC,0,0,g_width,g_height,0,0,g_width,g_height,imgBuf,&(pDlg->m_bmi),DIB_RGB_COLORS,SRCCOPY);
	//checkcolum(imgBuf,imData->width);
//	StretchDIBits(g_pdisplay->m_pMemDC->m_hDC,0,0,g_width,g_height,0,0,g_width,g_height,imgBuf,&(pDlg->m_bmi),DIB_RGB_COLORS,SRCCOPY);
//	g_pdisplay->Display();
	//delete imgBuf;
	
	cv::Mat frame(imData->height,imData->width,CV_8UC1,imData->leftData.get());
	cv::Mat frame1(imData->height,imData->width,CV_8UC1,imData->rightData.get());
	cv::imshow("disp",frame);
	cv::imshow("disp1",frame1);
	cv::waitKey(1);
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

		//cv::imwrite(cvfilename,frame);
		snap=false;
		}
		}
		
	
	if (b_timer1s==TRUE)
	{
		CString csIMU;
		IMUDataStruct *m_IMU=imData->IMUData.get();
		csIMU.Format(L"Accel       Gyro    \n x: %7d| %7d\n y:%7d|%7d\n z: %7d|%7d\n expotime:%d",
			m_IMU[0].accelData[0],m_IMU[0].gyroData[0],
			m_IMU[0].accelData[1],m_IMU[0].gyroData[1],
			m_IMU[0].accelData[2],m_IMU[0].gyroData[2],
			imData->expotime);
		mainwindow->setStatusText(csIMU);
		
	b_timer1s=FALSE;
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//m_pVideoDlg->ShowWindow(TRUE);
	cv::namedWindow("disp");
	cv::namedWindow("disp1");
	CyUsb_Init();
	m_byData[0]=(g_width&0xff<<8)>>8;
	m_byData[1]=(g_width&0xff);
	m_byData[2]=(g_height&0xff<<8)>>8;
	m_byData[3]=(g_height&0xff);
	m_sUsbOrder.ReqCode=IMGDISP;
	m_sUsbOrder.DataBytes=4;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
	initBMPHeader(g_width,g_height,&m_bmi);
	if(h_cctapi->startCap(g_height,g_width,RawCallBack,(LPVOID*)this,0)<0)
	//if(h_cctapi->startCap(g_height,g_width,RawCallBack,0)<0)
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"USB�豸��ʧ�ܣ�");
		return;
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"�ɼ���...");
		CheckRadioButton(IDC_RADIO_NORMAL,IDC_RADIO_XYMIRROR,IDC_RADIO_NORMAL);
		m_bUsbOpen=TRUE;
	}
	//std::string filename="videofile.avi";
	//cv::Size videosize=cv::Size(g_width,g_height);
	////h_vw.open(filename,CV_FOURCC('X','V','I','D'),15,videosize,0);
	//if(!h_vw.isOpened())
	//{
	//	SetDlgItemText(IDC_STATIC_TEXT,L"������Ƶʧ�ܡ�");
	//	return;
	//}

}


void CUsbControlDlg::OnBnClickedBtnStopcapture()
{
	if(h_cctapi->stopCap()!=0)
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"��δ�ɼ�");
		return;
	}
	//cv::destroyWindow("disp");
	SetDlgItemText(IDC_STATIC_TEXT,L" ");
	m_bUsbOpen=FALSE;
}

void CUsbControlDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	// TODO: �ڴ˴������Ϣ����������
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
	//h_cctapi->setMirrorType(DataProcessType(m_iProcType));
}


HBRUSH CUsbControlDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	// TODO:  �ڴ˸��� DC ���κ�����
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
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
	/*-------�����ַ����룬������Ϣ��*/
	/*
	CString s;
	s.GetBufferSetLength(1024);
	m_Edit_Width.GetWindowTextW(s.GetBuffer(),s.GetLength());
	MessageBox(s,_T("��ȡͼ����"),MB_OK);
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
	if(cbTrigMode.GetCurSel()==2&&fpgafreq>10&&fpgafreq<60)
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
		SetDlgItemText(IDC_STATIC_TEXT,L"Check Trig Mode && fpga freq 10~50");
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


void CUsbControlDlg::OnEnChangeEditmaxexpo()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	if(!m_bUsbOpen)
		return;
	CString s_temp;
	UpdateData(true);
	MaxExpo.GetWindowText(s_temp);
	int iMaxExpo= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	if(cbAutoExpo.GetCheck()==true&&iMaxExpo>0)
	{
		iMaxExpo=iMaxExpo/27.185;
		m_byData[0]=(iMaxExpo&0xff<<8)>>8;
		m_byData[1]=iMaxExpo&0xff;
		m_sUsbOrder.DataBytes=2;
		m_sUsbOrder.ReqCode=MAXEXPO;
		m_sUsbOrder.Direction=0;
		SendOrder(&m_sUsbOrder);
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"Check Expo?");
	}
}


void CUsbControlDlg::OnEnChangeEditimusmprate()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	//if(!m_bUsbOpen)
		//return;
	CString s_temp;
	UpdateData(true);
	IMUSampleRate.GetWindowText(s_temp);
	int iIMUSR= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	if(iIMUSR>0&&iIMUSR<21)
	{
		m_byData[0]=iIMUSR;
		//m_byData[1]=iIMUSR&0xff;
		m_sUsbOrder.DataBytes=1;
		m_sUsbOrder.ReqCode=IMUSAMRAT;
		m_sUsbOrder.Direction=0;
		SendOrder(&m_sUsbOrder);
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"imu sample rate 1~20");
	}
	
}


void CUsbControlDlg::OnBnClickedButtonrds1()
{
	// TODO: Add your control notification handler code here
	CString s_temp;
	UpdateData(true);
	S1ADDR.GetWindowText(s_temp);
	int is1addr= _tstoi(s_temp);
	//s_temp.ReleaseBuffer();

	//write address
	m_byData[0]=is1addr;
	m_sUsbOrder.ReqCode=SADDR;
	m_sUsbOrder.DataBytes=1;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);

	//read
	m_sUsbOrder.ReqCode=RDS1DATA;
	m_sUsbOrder.DataBytes=2;
	m_sUsbOrder.Direction=1;
	SendOrder(&m_sUsbOrder);
	UINT8 rxval[2];
	memcpy(rxval,m_byData,2);
	int irxval=rxval[1]<<8;
	irxval+=rxval[0];
	s_temp.Format(_T("%x"),irxval);
	S1DATA.SetWindowTextW(s_temp);

}


void CUsbControlDlg::OnBnClickedButtonwrs1()
{
	// TODO: Add your control notification handler code here
	CString s_temp;
	UpdateData(true);
	S1ADDR.GetWindowText(s_temp);
	int is1addr= _tstoi(s_temp);
	//s_temp.ReleaseBuffer();

	//write address
	m_byData[0]=is1addr;
	m_sUsbOrder.ReqCode=SADDR;
	m_sUsbOrder.DataBytes=1;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);

	//write data
	S1DATA.GetWindowText(s_temp);
	int is1data= _tstoi(s_temp);
	m_byData[0]=(is1data&0xff<<8)>>8;
	m_byData[1]=is1data&0xff;
	m_sUsbOrder.DataBytes=2;
	m_sUsbOrder.ReqCode=WRS1DATA;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
}


void CUsbControlDlg::OnBnClickedButtonrds2()
{
	// TODO: Add your control notification handler code here
	CString s_temp;
	UpdateData(true);
	S2ADDR.GetWindowText(s_temp);
	int is1addr= _tstoi(s_temp);
	//s_temp.ReleaseBuffer();

	//write address
	m_byData[0]=is1addr;
	m_sUsbOrder.ReqCode=SADDR;
	m_sUsbOrder.DataBytes=1;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);

	//read
	m_sUsbOrder.ReqCode=RDS2DATA;
	m_sUsbOrder.DataBytes=2;
	m_sUsbOrder.Direction=1;
	SendOrder(&m_sUsbOrder);
	UINT8 rxval[2];
	memcpy(rxval,m_byData,2);
	int irxval=rxval[1]<<8;
	irxval+=rxval[0];
	s_temp.Format(_T("%x"),irxval);
	S2DATA.SetWindowTextW(s_temp);
}


void CUsbControlDlg::OnBnClickedButtonwrs2()
{
	// TODO: Add your control notification handler code here
	CString s_temp;
	UpdateData(true);
	S2ADDR.GetWindowText(s_temp);
	int is1addr= _tstoi(s_temp);
	//s_temp.ReleaseBuffer();

	//write address
	m_byData[0]=is1addr;
	m_sUsbOrder.ReqCode=SADDR;
	m_sUsbOrder.DataBytes=1;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);

	//write data
	S2DATA.GetWindowText(s_temp);
	int is1data= _tstoi(s_temp);
	m_byData[0]=(is1data&0xff<<8)>>8;
	m_byData[1]=is1data&0xff;
	m_sUsbOrder.DataBytes=2;
	m_sUsbOrder.ReqCode=WRS2DATA;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
}


void CUsbControlDlg::OnBnClickedButtoneerd()
{
	// TODO: Add your control notification handler code here
	CString s_temp;
	UpdateData(true);
	EEADDR.GetWindowText(s_temp);
	int is1addr= _tstoi(s_temp);
	//s_temp.ReleaseBuffer();

	//write address
	m_byData[0]=(is1addr&0xff<<8)>>8;
	m_byData[1]=is1addr&0xff;
	m_sUsbOrder.ReqCode=oEEADDR;
	m_sUsbOrder.DataBytes=2;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);

	//read
	m_sUsbOrder.ReqCode=RDEE;
	m_sUsbOrder.DataBytes=1;
	m_sUsbOrder.Direction=1;
	SendOrder(&m_sUsbOrder);
	UINT8 rxval[1];
	memcpy(rxval,m_byData,1);
	int irxval=rxval[0];
	s_temp.Format(_T("%x"),irxval);
	EEDATA.SetWindowTextW(s_temp);
}


void CUsbControlDlg::OnBnClickedButtoneewr()
{
	// TODO: Add your control notification handler code here
	CString s_temp;
	UpdateData(true);
	EEADDR.GetWindowText(s_temp);
	int is1addr= _tstoi(s_temp);
	//s_temp.ReleaseBuffer();

	//write address
	m_byData[0]=(is1addr&0xff<<8)>>8;
	m_byData[1]=is1addr&0xff;
	m_sUsbOrder.ReqCode=oEEADDR;
	m_sUsbOrder.DataBytes=2;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);

	//write data
	EEDATA.GetWindowText(s_temp);
	int is1data= _tstoi(s_temp);
	m_byData[0]=is1data;
	m_sUsbOrder.DataBytes=1;
	m_sUsbOrder.ReqCode=WREE;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
}
