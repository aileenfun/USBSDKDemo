��sdk����������ֱ������bin�е��ļ����в������У������������������ķֱ��ʣ�Ĭ��Ϊ1280��1024�����VideoCapture����ʼ�ɼ���SnapShot�����ͼ����Ŀ¼�е�snap.jpg.
�������������ʱ���ᱣ��¼��Ϊh.264���롣

�������vs2010��д��ʹ����OpenCV���е���ʾ������ͼƬ��������Ƶ�ļ��Ĺ��ܡ�

�����CyUsb_Init()�󣬼��ɽ���SendOrder������������Ϳ����������StartCap��������ɼ��Ĳ�����
API������CCTAPI.h�С�

CCT_API int startCap(int height,int width,LPMV_CALLBACK2 CallBackFunc);
��ʼ�ɼ������У�
int height���ɼ�ͼ��ĸ߶�
int width:�ɼ�ͼ��Ŀ��
LPMV_CALLBACK2 CallBackFunc���ص�����,�õ�ͼ�����ݺ��û�����ʹ�ã������н���ʾ
������ʹ�÷����ڴ����У�

CCT_API int stopCap();
ֹͣͼ��ɼ���

CCT_API int setMirrorType(DataProcessType mirrortype);
����ͼ��ľ���ʽ������
DataProcessTypeΪ�Զ���ö������
enum DataProcessType
{
	Normal_Proc,Xmirror_Proc,Ymirror_Proc,XYmirror_Proc
};


���֧�ֵ�Э������Ϊenum�ͣ�
enum ReqValue
{
	TRIGMODE=0xD0,IMGDISP,EXPOGAIN,GAIN,EXPO,MIRROR,RCEXTR,TRIGPERIOD,RSTHW,SOFTTRIG,RSTSENSOR
};

//TRIGMODE��������ʽ
//IMGDISP���ֱ�������
//EXPOGAIN���Զ�������ع�
//GAIN������ֵ����
//EXPO���ع�ֵ����
//MIRROR������
//�м伸���ֶα�������δ����
//SOFTTRIG������һ��


�������ع�ʱ�亯������������������
void CUsbControlDlg::setExpoValue()
{
	if(!m_bUsbOpen)
		return;
	CString s_temp;
	UpdateData(true);
	eExpoValue.GetWindowText(s_temp);//���ı��������ı����ݣ�ת����int����ֵ
	int ExpoValue= _tstoi(s_temp);
	s_temp.ReleaseBuffer();
	if(cbAutoExpo.GetCheck()==false&&ExpoValue>0)//�ж��ع���ֵ��0������ȡ�����Զ��ع�
	{
		m_byData[0]=(ExpoValue&0xff<<8)>>8;//��Ҫ���͵���ֵ��䵽m_byData�ڣ����m_byData[15]
		m_byData[1]=ExpoValue&0xff;
		m_sUsbOrder.DataBytes=2;//m_byData����Ч���ݳ��ȣ�����ExpoValueΪ2�ֽڣ�����Ϊ2�������m_byData����4���ֽڵ���Ч������Ϊ4
		m_sUsbOrder.ReqCode=EXPO;//Э��ֵ��enum���ͣ�
		m_sUsbOrder.Direction=0;//���򣬴���λ����USB����ķ���Ϊ�����ֵ0��
		SendOrder(&m_sUsbOrder);//�������
	}
	else
	{
		SetDlgItemText(IDC_STATIC_TEXT,L"Check Expo?");
	}
}

