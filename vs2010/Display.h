#pragma once
class CDisplay
{
public:
	CDisplay(void);
	~CDisplay(void);

public:
	void Open(CDC* pDC,CRect rt );
	void Close();
	void Display();
	BOOL SetRect(CRect rt );
	CDC *GetMemDC();
	CDC		*m_pMemDC;
private:
	CDC		*m_pDcDisplay;
	CRect	m_rtDisplay;

	CBitmap *m_pMemBitmap;
	HDC		m_hDc;
};

