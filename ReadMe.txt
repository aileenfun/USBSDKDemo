本sdk开发包可以直接运行bin中的文件进行测试运行，在输入框中输入相机的分辨率，默认为1280×1024，点击VideoCapture即开始采集。SnapShot保存截图，在目录中的snap.jpg.
另外在软件运行时还会保存录像，为h.264编码。

软件采用vs2010编写，使用了OpenCV库中的显示、保存图片、保存视频文件的功能。

在完成CyUsb_Init()后，即可进行SendOrder（）向相机发送控制命令，或者StartCap启动相机采集的操作。
API函数在CCTAPI.h中。

CCT_API int startCap(int height,int width,LPMV_CALLBACK2 CallBackFunc);
开始采集。其中：
int height：采集图像的高度
int width:采集图像的宽度
LPMV_CALLBACK2 CallBackFunc：回调函数,拿到图像数据后用户可以使用，本例中仅显示
（具体使用方法在代码中）

CCT_API int stopCap();
停止图像采集。

CCT_API int setMirrorType(DataProcessType mirrortype);
设置图像的镜像方式。其中
DataProcessType为自定义枚举类型
enum DataProcessType
{
	Normal_Proc,Xmirror_Proc,Ymirror_Proc,XYmirror_Proc
};


相机支持的协议命令为enum型：
enum ReqValue
{
	TRIGMODE=0xD0,IMGDISP,EXPOGAIN,GAIN,EXPO,MIRROR,RCEXTR,TRIGPERIOD,RSTHW,SOFTTRIG,RSTSENSOR
};

//TRIGMODE，触发方式
//IMGDISP，分辨率设置
//EXPOGAIN，自动增益和曝光
//GAIN，增益值设置
//EXPO，曝光值设置
//MIRROR，镜像
//中间几个字段保留，尚未开发
//SOFTTRIG，软触发一次


以设置曝光时间函数举例向相机发送命令：
void CUsbControlDlg::setExpoValue()
{
	if(!m_bUsbOpen)
		return;
	CString s_temp;
	UpdateData(true);
	eExpoValue.GetWindowText(s_temp);//从文本框内拿文本数据，转换成int型数值
	int ExpoValue= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	if(cbAutoExpo.GetCheck()==false&&ExpoValue>0)//判断曝光数值〉0，并且取消了自动曝光
	{
		m_byData[0]=(ExpoValue&0xff<<8)>>8;//将要发送的数值填充到m_byData内，最大到m_byData[15]
		m_byData[1]=ExpoValue&0xff;
		m_sUsbOrder.DataBytes=2;//m_byData内有效数据长度，这里ExpoValue为2字节，所以为2。如果在m_byData填了4个字节的有效数据则为4
		m_sUsbOrder.ReqCode=EXPO;//协议值，enum类型，
		m_sUsbOrder.Direction=0;//方向，从上位机到USB相机的方向为输出，值0。
		SendOrder(&m_sUsbOrder);//发送命令。
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"Check Expo?");
	}
}

