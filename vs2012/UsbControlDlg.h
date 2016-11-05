
// UsbControlDlg.h : 头文件
//

#pragma once
#include "../../CCTAPI/CCTAPI/DataProcess.h"
#include "CCTAPI.h"
#include "../../CCTAPI/CCTAPI/CyUsb.h"
#include "VideoDlg.h"
#include "afxwin.h"
#include <cv.hpp>
#include <opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
// CUsbControlDlg 对话框
typedef
	VOID
	(WINAPI * LPMV_CALLBACK2)(LPVOID lpParam, LPVOID lpUser);

typedef ICCTAPI*(APIENTRY  *icct_factory)();//factory of ICCTAPI implementing objects

class CUsbControlDlg : public CDialogEx
{
// 构造
	HICON m_hIcon;
	enum { IDD = IDD_USBCONTROL_DIALOG };
	DECLARE_MESSAGE_MAP()

public:
	CUsbControlDlg(CWnd* pParent = NULL);	
	~CUsbControlDlg();
		
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	
	virtual BOOL OnInitDialog();

private:
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedBtnOpenusb();
	afx_msg void OnBnClickedBtnOpenfile();
	afx_msg void OnBnClickedBtnVideocapture();
	afx_msg void OnBnClickedBtnStopcapture();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCbnSelchangeComboDriver();
	afx_msg void OnBnClickedRadioDriver();
	afx_msg void OnBnClickedRadioProcType();
	afx_msg void OnBnClickedRadioChangeType();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnBnClickedBtnReview();
	static unsigned int __stdcall ThreadProcess(void* pParam);
	void ThreadProcessFunction();
	BOOL CloseRbfFile();
	BOOL OpenRbfFile();
	void SendData();
	BOOL OpenDataFile();
	BOOL CloseDataFile();

	//void _stdcall RawCallBack(LPVOID lpParam,LPVOID lpUser);
	void Split(CString in, CString *out, int &outcnt, CString spliter,int maxn);
private:
	BOOL	m_bUsbOpen;
	CFile*	m_pFileRbf;					//下位机程序文件
	CString m_strRbfFileName;
	BYTE          m_byData[64];
	USB_ORDER     m_sUsbOrder;
	BOOL		  m_bSendData;
	CVideoDlg*    m_pVideoDlg;
	HDC			  m_hDisplayDC;
	HANDLE        m_hThread;
	char*         m_pReadBuff;
	BOOL		  m_bCloseWnd;
	long          m_lBytePerSecond;
	CYUSB_DRIVER  m_CyDriver;			//驱动类型：新、旧
	int			  m_iRdoDriver;
	int			  m_iProcType;
	CBrush*       m_pBrush;	
	CFile*        m_pVideoDataFile;
	BOOL          m_bReview;
	BOOL          m_bSave;
	BOOL          m_Init;
	CString       m_strDataFileName;
	CCCTAPIApp *h_cctapi;
	

public:
	afx_msg void OnBnClickedBtnCreatebmp();
	CEdit m_Edit_Width;
	afx_msg void OnEnChangeEdit1();
	CEdit m_Edit_Height;
	afx_msg void OnEnChangeEdit2();
	CString m_sEdit_Width;
	CString m_sEdit_Height;
	afx_msg void OnBnClickedButton2();
	void saveVideoFun(cv::Mat frame);
	
	int test;
	int test2;
	afx_msg void OnBnClickedBnSnap();
	afx_msg void OnCbnSelchangeCombo1();
	CComboBox m_comboDevNum;
	void setStatusText(CString cs);
};
