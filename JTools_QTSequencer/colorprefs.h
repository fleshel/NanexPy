#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "Resource.h"
#include "QTSeqChartTypes.h"
// CColorPrefs dialog

class CColorPrefs : public CDialog
{
	DECLARE_DYNAMIC(CColorPrefs)

public:
	CColorPrefs(CWnd* pParent = NULL);   // standard constructor
	virtual ~CColorPrefs();
	
	CStatic MainColor1Control;
    CStatic MainColor2Control;
    CStatic MainColor3Control;        
    CStatic MainColor7Control;
    CStatic MainColor8Control;
    CStatic MainColor9Control;
    CStatic MainColor10Control;
    CStatic MainColor11Control;
    CStatic MainColor12Control;
    CEdit MaxBarsControl;
    CEdit FontControl;
    CFont *NewFont;
    CFont *DefaultFont;  
    CListCtrl ExchangeList;

	COLORREF BackgroundColor,StaticBackgroundColor;    
	CBrush BackgroundBrush,StaticBrush;
	COLORREF TextColor,StaticTextColor;
    COLORREF ChartBKColor,GridColor;    
    COLORREF TradeOutlineColor,SeperatorColor;
    CBrush ChartBKBrush;
    COLORREF LastColor,BidColor,AskColor;
    int CurrentItem;
    COLORREF BBOAskBandColor,BBOBidBandColor;

    int MaxBars;

    ExchangeMapType ExchangeMap[MAXEXCHANGES];
    COLORREF ChartOutlines;

    void SetCurrentItem(int Item);
    
        
// Dialog Data
	enum { IDD = IDD_COLORPREFS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP() 

public:
  afx_msg void OnBnClickedListfont();
  afx_msg void OnBnClickedDefault();
  afx_msg void OnBnClickedBlackscheme();
  afx_msg void OnBnClickedWhitescheme();
  afx_msg void OnBnClickedMaincolorpick1();
  afx_msg void OnBnClickedMaincolorpick2();
  afx_msg void OnBnClickedMainfgcolorpick1();
  afx_msg void OnBnClickedMainfgcolorpick2();
  afx_msg void OnBnClickedMaincolorpick3();
  afx_msg void OnBnClickedMainfgcolorpick3();
    
  afx_msg void OnBnClickedMaincolorpick7();

  afx_msg void OnBnClickedMaincolorpick8();
  afx_msg void OnBnClickedMaincolorpick9();
  afx_msg void OnBnClickedMaincolorpick10();
  afx_msg void OnBnClickedMaincolorpick11();
  afx_msg void OnBnClickedMaincolorpick12();

  afx_msg void OnLvnItemchangedExchangelist(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnEnChangeMaxbars();
  
};
