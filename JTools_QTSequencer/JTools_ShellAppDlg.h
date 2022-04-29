// JTools_ShellAppDlg.h : header file
//
#pragma once

#include "JTNX_NxFeedHandler.h"     // Defines and forwards for Logging/NxCore functionality
#include "QTSeqCharterPrice.h"
#include "QTSeqCharterVol.h"
#include "QTSeqCharterLegend.h"
#include "afxwin.h"

// CJTools_ShellAppDlg dialog
class CJTools_ShellAppDlg : public CDialog
{
// Construction
public:
	CJTools_ShellAppDlg(CWnd* pParent = NULL);	// standard constructor
    
// Dialog Data
	enum { IDD = IDD_JTOOLS_SHELLAPP_DIALOG };

    unsigned char CompactMode;
	WINDOWPLACEMENT GVwp;      
       
    unsigned char NxCoreIsRunning;
    void *JTNX_NxGlobalUserData;
    unsigned char StopFromUser;
	
	int RTSimDelay;
	char TapeFileName[256];
	char DLLFilename[256];
	unsigned char StartCoreOnStartup;		
	unsigned char HistTimeMode;	
	unsigned char FWXNxCoreMode;
	unsigned char ExcludeQuotes;
	unsigned char ExcludeL2Quotes;
	unsigned char ExcludeOPRA;
    unsigned char ExcludeCRC;
    
	unsigned char IsDelayed;
	int DelayMin;
	int DelayPause;

    unsigned char ViewInitiated,ConnectOnStartup;		
    unsigned char ExitOnTapeEnd,SaveScreenPos;
	
    JTLogStruct AppLog;
    unsigned char ShowMajorLog;
    unsigned char FileMajorLog;
    unsigned char MajorLoggingOn;
    unsigned char ShowNxCoreMS;
    unsigned char ResetLogOnNXDate;
    int NxClockGUIInterval;
		
	CEdit NxCoreModeControl;		
	CButton NextStepButton;
	
	COLORREF BackgroundColor;    	
	COLORREF TextColor; 
	COLORREF WinBG1,WinFG1;
	COLORREF WinBG2,WinFG2;
    COLORREF WinBG3,WinFG3;
	CBrush WinBrush1;
	CBrush WinBrush2;
    CBrush WinBrush3;
	CBrush BackgroundBrush;
			
    int DPI100;
    double DPIScale;
    void GetDPI();
    int DPIpx(int num);
	
	void WriteINIFile(char *FileName);
	unsigned char ReadINIFile(char *FileName);	    


    char Symbol[32];        
    char TradeDateString[255];
    char NameString[255];
    char NameString2[255];

    int LastMouseX,ThisMouseX;
    unsigned char MouseGrabbed;
        
    unsigned char ViewIsMinimized;

    unsigned char UpdateChartRT,UserInRT;
    unsigned char ShowGrids,ShowVol,ShowPrice,ShowLegend,ShowISO;
            
    int PriceLenOffest1,VolLenOffset1,LegendLenOffset1;
    int PriceLenOffest2,VolLenOffset2,LegendLenOffset2;
    char PriceStr1[32],VolStr1[32],LegendStr1[32];
    char PriceStr2[32],VolStr2[32],LegendStr2[32];

    int PriceY1,PriceHeight;    
    int VolY1,VolHeight;    
    int LegendY1,LegendHeight;      
    unsigned char StartFromOnOK;

    CEdit SysStatusControl;
    CEdit SymbolControl;
    CEdit InterestStatusControl;    
    CEdit HardSizeControl;    
    CEdit TestControl;
    
    CEdit CompWidthControl;
    CEdit OtherWidthControl;
    CEdit HardPriceMaxControl;
    CEdit HardPriceMinControl;
    CEdit HardPriceScaleControl;
    CEdit VolumeHardScaleControl;
    CEdit VolumeHardMaxControl;
    CEdit BandingControl;

    CButton TradeButton[MAXEXCHANGES],BidButton[MAXEXCHANGES],AskButton[MAXEXCHANGES];
    CStatic TradeText[MAXEXCHANGES],BidText[MAXEXCHANGES],AskText[MAXEXCHANGES];    
    CButton LineButton[3],PointButton[3];
    CStatic LineText[3],PointText[3];
    CButton OffButton[3],OnButton[3],AutoButton[3];
    CStatic LabelText[3];
    CButton CLineButton[3],CPointButton[3];
    CStatic CLineText[3],CPointText[3];
    CStatic CLabel[4];
    
    COLORREF ClearColor,GraphBckgrnd,ChartOutlines,StudyTextColor,ScaleTextColor;
    COLORREF DescripTextColor,GridColor,TradeOutlineColor;
    COLORREF TimeTickHighColor,TimeTickLowColor;
    CFont ChartFont;

    CDateTimeCtrl StartTimeControl;
    CDateTimeCtrl StopTimeControl;
       
    CStatic GraphControl,VolGraphControl,LegendGraphControl;
    CEdit DataPointsControl;
    CEdit StartPointControl;
    CEdit RExgControl;

    QTSeqCharterPrice Chart;		
    QTSeqCharterVol VolChart;
    QTSeqCharterLegend LegendChart;

    BarIndicatorType ChartData;
    ColorIndicatorType ChartColors;
      
    CRITICAL_SECTION DisplaySection;		

    int CompPenWidth;
    int OtherPenWidths;   
    unsigned char TradePriceType;
    unsigned char QuotePriceType;

    int CurrChartBar,ChartStartBar,ChartStopBar,ChartNumBars;
    int ViewStartBar,ViewStopBar,ViewDataPoints;
    unsigned char DataPointForceSet;
    
    unsigned char DoPriceRecalc,DoVolRecalc,DoCountReCalc;    
    unsigned char InterestWait; 

    char BreakStr[64];
    unsigned short LExg,RExg;
    unsigned char RoundLotAdj;
    unsigned char AutoTradesOn;

    struct tm TodaysDate;
    unsigned int StartTime;
    unsigned int StopTime;
    unsigned char StartHour,StartMin,StartSec;
    unsigned char StopHour,StopMin,StopSec;

    unsigned char ShowTimeCounts,ShowTimeTime,ShowReverseTime,SplitSizeChart;
    int PointExpand,PointExpand2;

    unsigned long TotalTradeCount;
    unsigned long TotalQuoteCount;
    char CountStr[255];
    
    unsigned char ExcludeQTTrades,ExcludeQTQuotes;
    char RExgFilterStr[512];
    unsigned char  RExchangeToggles[256];
    int RNumExchanges;

    int MAXBARS;
    unsigned char MaxBarError;
            
   
    void RefreshChart(void);
    void RefreshLegendChart(void);
    void ClearChart(void);
        
    void RepositionChart(int cx,int cy);
    
    void BreakTrade(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg);
    void BreakQuote(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg);
    void FreeChartMem(BarIndicatorType *ChartData);
        
    void AllocateColorMem(ColorIndicatorType *ChartColors);
    void FreeColorMem(ColorIndicatorType *ChartColors);
    void SetColorMem(ColorIndicatorType *ChartColors,
                     int Index,
                     COLORREF LastColor,COLORREF BidColor,COLORREF AskColor,
                     int PenWidth);
    void SetBandingColor(void);

    void DisableExchanges(void);
    void EnableTradeExchange(int Index);
    void EnableBidExchange(int Index);
    void EnableAskExchange(int Index);
    void EnableHardScale(unsigned char Enable);
    void EnableHardVolScale(unsigned char Enable);

    void CountEm(void);
    void MakeTitleString(void);
    int SetRExchanges(char *buff);
    void EnableRTControl(unsigned char Enable);

    void CheckNXTime(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg);
        
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;


	// Generated message map functions
	virtual BOOL OnInitDialog();   
	virtual void OnTimer(UINT_PTR idEvent);    
    virtual void OnCancel();
    virtual void OnOK();
    virtual HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    virtual void OnLButtonDown(UINT nFlags,CPoint point);
	virtual void OnLButtonUp(UINT nFlags,CPoint point);
	virtual void OnMouseMove(UINT nFlags,CPoint point);
    LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);
    
	afx_msg void OnPaint();
    virtual void OnSize(UINT nType,int cx,int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedStartcore();
	afx_msg void OnBnClickedStopcore();	
	afx_msg void OnBnClickedSettings();
	afx_msg void OnBnClickedTimefull();
	afx_msg void OnBnClickedTimesim();
	afx_msg void OnBnClickedTimestep();
	afx_msg void OnBnClickedStep();	
    afx_msg void OnEnChangeSymbol();    
    afx_msg void OnBnClickedGrids();        
    afx_msg void OnBnClickedChartpause();
    afx_msg void OnBnClickedVolumeon();
    
    afx_msg void OnDtnDatetimechangeStarttime(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDtnDatetimechangeStoptime(NMHDR *pNMHDR, LRESULT *pResult);   
    afx_msg void OnDeltaposCompwidthspin(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposOtherwidthspin(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposDatapointspin(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposStartpointspin(NMHDR *pNMHDR, LRESULT *pResult);        
    afx_msg void OnBnClickedPriceon();
    afx_msg void OnBnClickedTradesoff();
    afx_msg void OnBnClickedTradeson();
    afx_msg void OnBnClickedTradesauto();
    afx_msg void OnBnClickedBidsoff();
    afx_msg void OnBnClickedBidson();
    afx_msg void OnBnClickedBidsauto();
    afx_msg void OnBnClickedAsksoff();
    afx_msg void OnBnClickedAskson();
    afx_msg void OnBnClickedAsksauto();
    afx_msg void OnEnChangeCompwidth();
    afx_msg void OnEnChangeOtherwidth();
    afx_msg void OnBnClickedCircuitstyle();

    afx_msg void OnBnClickedScaletoprice();
    afx_msg void OnBnClickedScaletonbbo();
    afx_msg void OnBnClickedScaletoall();
    afx_msg void OnBnClickedScaletohard();
    afx_msg void OnEnChangeHardmax();
    afx_msg void OnEnChangeHardmin();
    afx_msg void OnEnChangeHardscale();
    afx_msg void OnBnClickedScalevolauto();
    afx_msg void OnBnClickedScalevolhard();
    afx_msg void OnEnChangeVolhardmax();
    afx_msg void OnEnChangeVolhardscale();
    afx_msg void OnBnClickedUsebboband();
    afx_msg void OnEnChangeBandpercent();
    afx_msg void OnBnClickedRoundlotadj();
    afx_msg void OnBnClickedNoautotrade();
    afx_msg void OnBnClickedLegendon();
    afx_msg void OnBnClickedRevtime();
    afx_msg void OnBnClickedChartrefresh();
    afx_msg void OnEnChangeNumdatapoints();
    afx_msg void OnEnChangeStartpoint();
    afx_msg void OnBnClickedExcludetrades();
    afx_msg void OnBnClickedExcludequotes();
    afx_msg void OnEnChangeRexchangefilter();
    afx_msg void OnBnClickedShowiso();
    afx_msg void OnBnClickedSplitsizechart();
};
