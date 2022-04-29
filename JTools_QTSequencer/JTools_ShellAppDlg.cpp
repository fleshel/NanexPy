/////////////////////////////////////////////////////
// JTools_ShellAppDlg.cpp : implementation file    //
// Modified specifically for the                   //
// JTools_QTSequencer app                          //
/////////////////////////////////////////////////////
// Author: Jeffrey Donovan                         //
// Copyright (C) 2012 Nanex, LLC.                  //
// All Rights Reserved.                            //
/////////////////////////////////////////////////////
//	If you are a Subscriber to NxCore, you may	   //
//  copy, modify, and distribute the contents	   //
//	of this file without royalty, provided that    //
//	you assume any and all responsibilty, and	   //
//	forever release Jeffrey Donovan and Eric       //
//  Scott Hunsader from	any damage claims, real,   //
//  perceived, intentional, or imagined.	       //
/////////////////////////////////////////////////////
// LDM: 08-19-16                                   //
/////////////////////////////////////////////////////

#include "stdafx.h"
#include "JTools_ShellApp.h"
#include "JTools_ShellAppDlg.h"
#include "..\JT_Common\JT_Parsers.h"           
#include "..\JT_CommonGUI\JT_StockPreferences.h"           // App preferences dialog 
#include "..\JT_CommonGUI\JT_NumFormatters.h"          
#include "ColorPrefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CORECONFIGNAME "JTools_QTSequencer.ini"
#define LOGNAMEPREFIX "JTools_QTSequencer"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Global handle to the Dialog, for use in callback functions.
// Set in OnInitialDialog function
CJTools_ShellAppDlg *pView=NULL;

extern NxCoreStateGetMMQuotes	  pfNxCoreStateGetMMQuotes;
extern NxCoreStateGetLastTrade	  pfNxCoreStateGetLastTrade;
extern NxCorePriceToDouble		  pfNxCorePriceToDouble;

#define IDC_TRADEBUTTONS 16000
#define IDC_BIDBUTTONS 16030
#define IDC_ASKBUTTONS 16060

#define IDC_LINEBUTTONS 16090
#define IDC_DOTBUTTONS 16095

#define IDC_ONBUTTONS 17000
#define IDC_OFFBUTTONS 17005
#define IDC_AUTOBUTTONS 17010

#define IDC_CLINEBUTTONS 17015
#define IDC_CDOTBUTTONS 17020

ExchangeMapType ExchangeMap[]={
  {0,"COMP",RGB(180,180,180),RGB(0,255,0),RGB(255,0,0),2},
  {4,"AMEX",RGB(153,0,204),RGB(51,204,103),RGB(181,145,0),1},
  {60,"BATS",RGB(255,0,255),RGB(100,200,255),RGB(255,200,100),1},
  {63,"BATY",RGB(255,153,204),RGB(59,118,177),RGB(204,51,0),1},
  {11,"BOST",RGB(255,102,0),RGB(100,200,100),RGB(200,100,100),1},
  {68,"IEX",RGB(155,155,0),RGB(0,64,150),RGB(164,110,0),1},
  {17,"CHIC",RGB(204,204,255),RGB(0,150,64),RGB(150,64,0),1},
  { 8,"CINC",RGB(51,102,153),RGB(0,140,0),RGB(150,150,0),1},
  {64,"EDGE",RGB(0,153,51),RGB(70,170,84),RGB(170,84,70),1},
  {65,"EDGX",RGB(153,153,255),RGB(50,150,64),RGB(150,64,50),1},
  { 2,"ADF",RGB(153,255,153),RGB(150,200,150),RGB(200,150,150),1},
  { 1,"NSDQ",RGB(0,118,245),RGB(0,154,255),RGB(255,154,0),1},
  {57,"NQNX",RGB(0,58,165),RGB(0,184,255),RGB(255,184,0),1},
  { 3,"NYSE",RGB(0,255,0),RGB(0,90,0),RGB(255,150,150),1},
  {59,"NTRF",RGB(0,180,0),RGB(0,60,0),RGB(255,120,120),1},
  { 7,"ARCA",RGB(0,255,255),RGB(0,255,255),RGB(255,255,0),1},
  { 9,"PHIL",RGB(153,102,51),RGB(102,153,51),RGB(153,51,102),1}    
};

int ReverseExchangeMap[256];

#define IDM_COLORPREFS 0x0110


// Callback functions for NxCore section of code. You can detach the GUI 
// from NxCore code by simply not setting the callbacks (see OnInitDialog)
extern "C"
{

// Functions callbacks - Called within JWX_NxFeedHandler.cpp
// for displaying data
//
// Modified 9-18-13 to take pNxCoreSys, pNxCoreMsg as opposed to hour,min,sec,millisec,
// AND to call CheckNXTime() 
//-----------------------------------------------------------
int _cdecl WriteNxCoreTime(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg,unsigned char ShowMS)
{
	if ((!pView)||(!pView->m_hWnd)) return 0;
	char str[64];

	if (!ShowMS)	
      sprintf(str,"NxCore Time: %02d:%02d:%02d",pNxCoreSys->nxTime.Hour,pNxCoreSys->nxTime.Minute,pNxCoreSys->nxTime.Second);	
	else	
      sprintf(str,"NxCore Time: %02d:%02d:%02d:%03d",pNxCoreSys->nxTime.Hour,pNxCoreSys->nxTime.Minute,pNxCoreSys->nxTime.Second,pNxCoreSys->nxTime.Millisecond);

    pView->SysStatusControl.SetWindowTextA(str);

    pView->CheckNXTime(pNxCoreSys,pNxCoreMsg);
	return 1;
}
//----------------------------------------
int _cdecl WriteStatusLine(char *str)
{ 
	if ((!pView)||(!pView->m_hWnd)) return 0;
    pView->SysStatusControl.SetWindowTextA(str);		
	return 1;
}
//----------------------------------------
int _cdecl EndOfTape(void)
{
	if ((!pView)||(!pView->m_hWnd)) return 0;		

    if (pView->ExitOnTapeEnd)
      pView->SetTimer(3,100,NULL);	
    else if (!pView->StopFromUser)
	  pView->SetTimer(1,1000,NULL);	   
	return 1;
}
//----------------------------------------
int _cdecl ErrorCallback(int ErrorCode)
{ 
	if ((!pView)||(!pView->m_hWnd)) return 0;
	switch(ErrorCode)
	{        
	    // Could not load DLL
	    case NXCORE_DLLLOADERROR:
		     JTNX_Interface_Stop(pView->JTNX_NxGlobalUserData);
			 pView->SysStatusControl.SetWindowTextA("NxCore DLL Load Error");		
		     pView->NxCoreIsRunning=0;
	         break;
	    // Could not allocate additional user data
		case NXCORE_MEMALLOCATIONERROR:
		     JTNX_Interface_Stop(pView->JTNX_NxGlobalUserData);
			 pView->SysStatusControl.SetWindowTextA("Memory Allocation Error");		
		     pView->NxCoreIsRunning=0;
		     break;

		// Other errors...
		default:
		    // If error code is negative it is a process tape error
		    if (ErrorCode<0)
			{		
		       JTNX_Interface_Stop(pView->JTNX_NxGlobalUserData);
			   char str[64];
			   sprintf(str,"Process Tape Error: %d",ErrorCode);
			   pView->SysStatusControl.SetWindowTextA(str);		
		       pView->NxCoreIsRunning=0;
			}
		    break;
	}
	return 1;
}

int WaitCount=0;

//------------------------------------------------------------
int NxS_RTDataCallBack(int Msg,const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{
  
  switch(Msg)
  {
     case NxMSG_TRADE:
          if (pView->InterestWait) return 1;
          pView->BreakTrade(pNxCoreSys,pNxCoreMsg);
          break;
     case NxMSG_EXGQUOTE:
          if (pView->InterestWait) return 1;
          pView->BreakQuote(pNxCoreSys,pNxCoreMsg);
          break;

    case 200:
         pView->InterestStatusControl.SetWindowTextA("Symbol Does Not Exist!");   
         pView->OnBnClickedStopcore();
         break;
    case 201:
         pView->InterestStatusControl.SetWindowTextA("Forwarding to Start Time...");                          
         pView->EnableRTControl(0);
         break;
     
    case 203:
          pView->InterestWait=0;          
          break;
    case 204:
          sprintf(pView->TradeDateString," on %02d/%02d/%d",pNxCoreSys->nxDate.Month,pNxCoreSys->nxDate.Day,pNxCoreSys->nxDate.Year);
          sprintf(pView->NameString,"Prices for %s %s",pView->Symbol,pView->TradeDateString);
          sprintf(pView->NameString2,"Sizes for %s %s",pView->Symbol,pView->TradeDateString);
          break;
    case 205:
          break;    
  }
  return 1;
}


} // extern "C"


// Dialog constructor
//-------------------------------------------------------------
CJTools_ShellAppDlg::CJTools_ShellAppDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CJTools_ShellAppDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    StartCoreOnStartup=NxCoreIsRunning=StopFromUser=0;
    ExitOnTapeEnd=0;SaveScreenPos=1;
    JTNX_NxGlobalUserData=NULL;
	
	HistTimeMode=TIMEMODE_FULLTIME;
	RTSimDelay=20;

	BackgroundColor=RGB(54,54,54);//GetSysColor(COLOR_BTNFACE);//COLOR_WINDOW);
    TextColor=RGB(255,255,255);//GetSysColor(COLOR_WINDOWTEXT);	

	WinBG2=RGB(255,255,255);
	WinFG2=RGB(0,0,0);
	WinBG1=RGB(110,110,110);;//GetSysColor(COLOR_BTNFACE);
	WinFG1=RGB(255,255,255);;//GetSysColor(COLOR_WINDOWTEXT);    
    WinBG3=GetSysColor(COLOR_WINDOW);
	WinFG3=GetSysColor(COLOR_WINDOWTEXT);    
		
	NxCoreIsRunning=0;
	FWXNxCoreMode=NXCORE_REALTIME;	
	TapeFileName[0]=0;

// Load (when not specified in INI file) 32bit or 64bit NxCoreAPI DLL initially
#ifdef _M_IX86
	strcpy(DLLFilename,"NxCoreAPI.dll");
#else
	strcpy(DLLFilename,"NxCoreAPI64.dll");
#endif
    CreateDirectory("JTools_RunningINIFiles",NULL);
		
	ShowMajorLog=0;
    FileMajorLog=MajorLoggingOn=1;    
    ShowNxCoreMS=0;
	CompactMode=0;
	ExcludeQuotes=0;
    ExcludeL2Quotes=1;
    ExcludeOPRA=1;
    ExcludeCRC=0;
    
	IsDelayed=0;
	DelayMin=20;
	DelayPause=250;	

    DPI100=96;
    DPIScale=1.0;
    ViewInitiated=0;
    ResetLogOnNXDate=0;

    NxClockGUIInterval=NxCLOCK_SECOND;
    JTLog_InitStruct(&AppLog);    


    Symbol[0]=0;
                
    UserInRT=1;
    UpdateChartRT=0;    
    ShowGrids=1;            
    ShowVol=1;    
    ShowPrice=1;
    ShowLegend=1;
                  
    ViewIsMinimized=0;

    strcpy(PriceStr1,"Price");    
    strcpy(PriceStr2,"Price");
    strcpy(VolStr1,"Size");
    strcpy(VolStr2,"Size");
    strcpy(LegendStr1,"Legend");
    strcpy(LegendStr2,"Lgnd");
   
    ClearColor=BackgroundColor;
    GraphBckgrnd=RGB(0,0,0);
    ChartOutlines=RGB(255,255,255);
    StudyTextColor=RGB(255,255,255);
    ScaleTextColor=RGB(255,255,255);
    DescripTextColor=RGB(255,255,255);   
    GridColor=RGB(80,80,80);
    TradeOutlineColor=RGB(255,255,255);        
    
    LastMouseX=ThisMouseX=0;
    MouseGrabbed=0;
    
    TradePriceType=2;
    CurrChartBar=0;    
    ChartStartBar=0;
    ChartStopBar=0;
    ViewStartBar=0;
    ViewStopBar=0;
    ViewDataPoints=0;
    
    DoPriceRecalc=1;
    DoVolRecalc=1;
    TradeDateString[0]=NameString[0]=NameString2[0]=0;
    
    InterestWait=1;  // Pause stream?        
    StartFromOnOK=0;

    CompPenWidth=1;
    OtherPenWidths=1;

    ChartColors.BBOBidBandColor=RGB(153,255,0);
    ChartColors.BBOAskBandColor=RGB(255,204,204);
    ChartColors.BBOBidBandPen.CreatePen(PS_SOLID,ExchangeMap[0].PenWidth,ChartColors.BBOBidBandColor);  
    ChartColors.BBOAskBandPen.CreatePen(PS_SOLID,ExchangeMap[0].PenWidth,ChartColors.BBOAskBandColor);
   
    RoundLotAdj=1;
    AutoTradesOn=1;

    StartTime=34200000;
    StopTime=34260000;    
    memset(&ChartData,0,sizeof(BarIndicatorType));

    StartHour=9;StartMin=30;StartSec=0;
    StopHour=9;StopMin=31;StopSec=0;

    ShowTimeCounts=ShowTimeTime=1;
    ShowReverseTime=0;

    DoCountReCalc=1;
    DataPointForceSet=0;

    ExcludeQTTrades=ExcludeQTQuotes=0;
    RExgFilterStr[0]=0;

    TimeTickHighColor=RGB(255,255,255);
    TimeTickLowColor=RGB(155,155,155);

    ShowISO=0;
    PointExpand=4,PointExpand2=3;

    MAXBARS=86400;    
    MaxBarError=0;
    SplitSizeChart=0;
}

// Data exchange - Called by framework
//-------------------------------------------------------------
void CJTools_ShellAppDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  // ALL RELATED TO NXCORE ENGINE
  DDX_Control(pDX, IDC_SYSTEMMODE, NxCoreModeControl);
  DDX_Control(pDX, IDC_STEP, NextStepButton);        
  DDX_Control(pDX, IDC_SYSSTATUS, SysStatusControl);
  DDX_Control(pDX, IDC_SYMBOL, SymbolControl);
  DDX_Control(pDX, IDC_INTSTATUS, InterestStatusControl);    
  DDX_Control(pDX, IDC_GRAPH, GraphControl);  
  DDX_Control(pDX, IDC_VOLGRAPH, VolGraphControl);    
  DDX_Control(pDX, IDC_LEGGRAPH, LegendGraphControl);    

  DDX_Control(pDX, IDC_COMPWIDTH, CompWidthControl);
  DDX_Control(pDX, IDC_OTHERWIDTH, OtherWidthControl);
  DDX_Control(pDX, IDC_HARDMAX, HardPriceMaxControl);
  DDX_Control(pDX, IDC_HARDMIN, HardPriceMinControl);
  DDX_Control(pDX, IDC_HARDSCALE, HardPriceScaleControl);
  DDX_Control(pDX, IDC_VOLHARDMAX, VolumeHardMaxControl);
  DDX_Control(pDX, IDC_VOLHARDSCALE, VolumeHardScaleControl);
  DDX_Control(pDX, IDC_BANDPERCENT, BandingControl);
  DDX_Control(pDX, IDC_STARTTIME, StartTimeControl);
  DDX_Control(pDX, IDC_STOPTIME, StopTimeControl);
  DDX_Control(pDX, IDC_NUMDATAPOINTS, DataPointsControl);
  DDX_Control(pDX, IDC_STARTPOINT, StartPointControl);
  DDX_Control(pDX, IDC_REXCHANGEFILTER, RExgControl);
}

BEGIN_MESSAGE_MAP(CJTools_ShellAppDlg, CDialog)
    ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
    ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()
    ON_WM_CTLCOLOR()

    ON_WM_MOUSEMOVE()	
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()

    ON_BN_CLICKED(IDC_STARTCORE, &CJTools_ShellAppDlg::OnBnClickedStartcore)
	ON_BN_CLICKED(IDC_STOPCORE, &CJTools_ShellAppDlg::OnBnClickedStopcore)	
	ON_BN_CLICKED(IDC_SETTINGS, &CJTools_ShellAppDlg::OnBnClickedSettings)
	ON_BN_CLICKED(IDC_TIMEFULL, &CJTools_ShellAppDlg::OnBnClickedTimefull)
	ON_BN_CLICKED(IDC_TIMESIM, &CJTools_ShellAppDlg::OnBnClickedTimesim)
	ON_BN_CLICKED(IDC_TIMESTEP, &CJTools_ShellAppDlg::OnBnClickedTimestep)
	ON_BN_CLICKED(IDC_STEP, &CJTools_ShellAppDlg::OnBnClickedStep)		

    ON_EN_CHANGE(IDC_SYMBOL, &CJTools_ShellAppDlg::OnEnChangeSymbol)
    ON_BN_CLICKED(IDC_GRIDS, &CJTools_ShellAppDlg::OnBnClickedGrids)        
    ON_BN_CLICKED(IDC_CHARTPAUSE, &CJTools_ShellAppDlg::OnBnClickedChartpause)
    ON_BN_CLICKED(IDC_VOLUMEON, &CJTools_ShellAppDlg::OnBnClickedVolumeon)                 
      
    
    ON_NOTIFY(UDN_DELTAPOS, IDC_COMPWIDTHSPIN, &CJTools_ShellAppDlg::OnDeltaposCompwidthspin)
    ON_NOTIFY(UDN_DELTAPOS, IDC_OTHERWIDTHSPIN, &CJTools_ShellAppDlg::OnDeltaposOtherwidthspin)
        
    ON_BN_CLICKED(IDC_PRICEON, &CJTools_ShellAppDlg::OnBnClickedPriceon)
    ON_EN_CHANGE(IDC_COMPWIDTH, &CJTools_ShellAppDlg::OnEnChangeCompwidth)
    ON_EN_CHANGE(IDC_OTHERWIDTH, &CJTools_ShellAppDlg::OnEnChangeOtherwidth)
    ON_BN_CLICKED(IDC_CIRCUITSTYLE, &CJTools_ShellAppDlg::OnBnClickedCircuitstyle)
    
    ON_BN_CLICKED(IDC_SCALETOPRICE, &CJTools_ShellAppDlg::OnBnClickedScaletoprice)
    ON_BN_CLICKED(IDC_SCALETONBBO, &CJTools_ShellAppDlg::OnBnClickedScaletonbbo)
    ON_BN_CLICKED(IDC_SCALETOALL, &CJTools_ShellAppDlg::OnBnClickedScaletoall)
    ON_BN_CLICKED(IDC_SCALETOHARD, &CJTools_ShellAppDlg::OnBnClickedScaletohard)
    ON_EN_CHANGE(IDC_HARDMAX, &CJTools_ShellAppDlg::OnEnChangeHardmax)
    ON_EN_CHANGE(IDC_HARDMIN, &CJTools_ShellAppDlg::OnEnChangeHardmin)
    ON_EN_CHANGE(IDC_HARDSCALE, &CJTools_ShellAppDlg::OnEnChangeHardscale)
    ON_BN_CLICKED(IDC_SCALEVOLAUTO, &CJTools_ShellAppDlg::OnBnClickedScalevolauto)
    ON_BN_CLICKED(IDC_SCALEVOLHARD, &CJTools_ShellAppDlg::OnBnClickedScalevolhard)
    ON_EN_CHANGE(IDC_VOLHARDMAX, &CJTools_ShellAppDlg::OnEnChangeVolhardmax)
    ON_EN_CHANGE(IDC_VOLHARDSCALE, &CJTools_ShellAppDlg::OnEnChangeVolhardscale)
    ON_BN_CLICKED(IDC_USEBBOBAND, &CJTools_ShellAppDlg::OnBnClickedUsebboband)
    ON_EN_CHANGE(IDC_BANDPERCENT, &CJTools_ShellAppDlg::OnEnChangeBandpercent)
    ON_BN_CLICKED(IDC_ROUNDLOTADJ, &CJTools_ShellAppDlg::OnBnClickedRoundlotadj)
    ON_BN_CLICKED(IDC_AUTOTRADE, &CJTools_ShellAppDlg::OnBnClickedNoautotrade)
    ON_BN_CLICKED(IDC_LEGENDON, &CJTools_ShellAppDlg::OnBnClickedLegendon)

    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_STARTTIME, &CJTools_ShellAppDlg::OnDtnDatetimechangeStarttime)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_STOPTIME, &CJTools_ShellAppDlg::OnDtnDatetimechangeStoptime)
    ON_BN_CLICKED(IDC_REVTIME, &CJTools_ShellAppDlg::OnBnClickedRevtime)
    ON_BN_CLICKED(IDC_CHARTREFRESH, &CJTools_ShellAppDlg::OnBnClickedChartrefresh)
    ON_EN_CHANGE(IDC_NUMDATAPOINTS, &CJTools_ShellAppDlg::OnEnChangeNumdatapoints)
    ON_EN_CHANGE(IDC_STARTPOINT, &CJTools_ShellAppDlg::OnEnChangeStartpoint)
    ON_BN_CLICKED(IDC_EXCLUDETRADES, &CJTools_ShellAppDlg::OnBnClickedExcludetrades)
    ON_BN_CLICKED(IDC_EXCLUDEQUOTES, &CJTools_ShellAppDlg::OnBnClickedExcludequotes)
    ON_EN_CHANGE(IDC_REXCHANGEFILTER, &CJTools_ShellAppDlg::OnEnChangeRexchangefilter)

    ON_NOTIFY(UDN_DELTAPOS, IDC_DATAPOINTSPINNER, &CJTools_ShellAppDlg::OnDeltaposDatapointspin)
    ON_NOTIFY(UDN_DELTAPOS, IDC_STARTPOINTSPINNER, &CJTools_ShellAppDlg::OnDeltaposStartpointspin)
    ON_BN_CLICKED(IDC_SHOWISO, &CJTools_ShellAppDlg::OnBnClickedShowiso)
    ON_BN_CLICKED(IDC_SPLITSIZECHART, &CJTools_ShellAppDlg::OnBnClickedSplitsizechart)
END_MESSAGE_MAP()


// OnInitDialog - Called by framework
//-------------------------------------------------------------
BOOL CJTools_ShellAppDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    GetDPI();

    ::InitializeCriticalSection( &DisplaySection );

    CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{      
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, IDM_COLORPREFS, "Color Prefs");
	}

    memset(&ReverseExchangeMap,0,sizeof(int)*256);
    memset(&ChartData,0,sizeof(BarIndicatorType));
    AllocateColorMem(&ChartColors);

    ChartData.DrawTradeLine=1;
    ChartData.DrawCTradeLine=0;
    ChartData.DrawBidLine=1;
    ChartData.DrawCBidLine=0;
    ChartData.DrawAskLine=1;
    ChartData.DrawCAskLine=0;

    ChartData.ScaleTo=1;
    ChartData.BBOBandAmount=0.20;
    ChartData.UseBBOBands=0;

        
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	pView=this;
		
	JTNX_NxGlobalUserData=(void*)JTNX_Interface_AllocateGlobalUserData();	
    JTNX_Interface_SetChartDataPtr(JTNX_NxGlobalUserData,&ChartData);
    JTLog_ResetFileName(&AppLog,LOGNAMEPREFIX);    
        
	JTNX_Interface_SetStatusDisplayCallback(JTNX_NxGlobalUserData,WriteStatusLine);
	JTNX_Interface_SetEndOfTapeCallback(JTNX_NxGlobalUserData,EndOfTape);
	JTNX_Interface_SetErrorCallback(JTNX_NxGlobalUserData,ErrorCallback);
    
    ChartFont.CreateFontA(-MulDiv(8, GetDeviceCaps(GetDC()->m_hDC, LOGPIXELSY), 72),0,1,1,FW_REGULAR,0,0,0,0,0,0,0,0,"Arial");

	if (ReadINIFile(CORECONFIGNAME))
	{
	  if (CompactMode)
	    SetWindowPos(NULL,0,0,415,175, SWP_NOMOVE);              
	}

    JTNX_Interface_SetMaxBars(JTNX_NxGlobalUserData,MAXBARS);

    for (int loop=0;loop<MAXEXCHANGES;loop++)    
    {
      SetColorMem(&ChartColors,loop,ExchangeMap[loop].LastColor,ExchangeMap[loop].BidColor,ExchangeMap[loop].AskColor,ExchangeMap[loop].PenWidth);
      if (ExchangeMap[loop].Exchange>=0)
         ReverseExchangeMap[ExchangeMap[loop].Exchange]=loop;      
    }
    SetBandingColor();

    JTNX_Interface_SetNxClockGUIInterval(JTNX_NxGlobalUserData,NxClockGUIInterval);
    if (NxClockGUIInterval!=(NxCLOCK_HOUR+1))
        JTNX_Interface_SetTimeDisplayCallback(JTNX_NxGlobalUserData,WriteNxCoreTime);    
    else    
        JTNX_Interface_SetTimeDisplayCallback(JTNX_NxGlobalUserData,NULL);     

    JTLog_SetParms(&AppLog,ShowMajorLog,FileMajorLog,MajorLoggingOn,ShowNxCoreMS);		

    JTNX_Interface_SetFeedDelay(JTNX_NxGlobalUserData,IsDelayed,DelayMin,DelayPause);    	
    JTNX_Interface_SetAppLog(JTNX_NxGlobalUserData,&AppLog);			
    JTNX_Interface_SetLogResetNxDate(JTNX_NxGlobalUserData,ResetLogOnNXDate);
	
	BackgroundBrush.CreateSolidBrush(BackgroundColor);	
	WinBrush1.CreateSolidBrush(WinBG1);	
	WinBrush2.CreateSolidBrush(WinBG2);			
    WinBrush3.CreateSolidBrush(WinBG3);			
		    
	switch(FWXNxCoreMode)
	{
	    case NXCORE_REALTIME:
			 NxCoreModeControl.SetWindowTextA("Realtime");
			 break;

		case NXCORE_TAPE:
			 char str[1064];
			 sprintf(str,"Historical - %s",TapeFileName);
			 NxCoreModeControl.SetWindowTextA(str);
			 break;
	}

    NextStepButton.EnableWindow(FALSE);
    switch(HistTimeMode)
	{
	    case TIMEMODE_FULLTIME:CheckDlgButton(IDC_TIMEFULL,1);break;
		case TIMEMODE_SIMTIME: CheckDlgButton(IDC_TIMESIM,1);break;
	    case TIMEMODE_STEPTIME:CheckDlgButton(IDC_TIMESTEP,1); 
             NextStepButton.EnableWindow(TRUE);
             break;
	}
	JTNX_Interface_SetHistMode(JTNX_NxGlobalUserData,HistTimeMode,RTSimDelay);
	JTNX_Interface_SetExcludeFlags(JTNX_NxGlobalUserData,ExcludeQuotes,ExcludeL2Quotes,ExcludeOPRA,ExcludeCRC);
    		
	CheckDlgButton(IDC_COMPACT,(BOOL)CompactMode);
	CheckDlgButton(IDC_MAJORLOG,(BOOL)MajorLoggingOn);	
	CheckDlgButton(IDC_MAJORLOGSHOW,(BOOL)ShowMajorLog);	
	CheckDlgButton(IDC_MAJORLOGFILE,(BOOL)FileMajorLog);
    CheckDlgButton(IDC_SHOWMS,(BOOL)ShowNxCoreMS);
    CheckDlgButton(IDC_RESETLOGNX,(BOOL)ResetLogOnNXDate);
    CheckDlgButton(IDC_CIRCUITSTYLE,(BOOL)ChartData.CircuitStyle);
    CheckDlgButton(IDC_ROUNDLOTADJ,(BOOL)RoundLotAdj);
    CheckDlgButton(IDC_AUTOTRADE,(BOOL)AutoTradesOn);
    CheckDlgButton(IDC_REVTIME,(BOOL)ShowReverseTime);
    CheckDlgButton(IDC_EXCLUDETRADES,(BOOL)ExcludeQTTrades);
    CheckDlgButton(IDC_EXCLUDEQUOTES,(BOOL)ExcludeQTQuotes);
    CheckDlgButton(IDC_CHARTPAUSE,(BOOL)!UpdateChartRT);
    CheckDlgButton(IDC_SHOWISO,(BOOL)ShowISO);

    CheckDlgButton(IDC_SPLITSIZECHART,(BOOL)SplitSizeChart);
          

    BandingControl.EnableWindow(0);
    ::EnableWindow(GetDlgItem(IDC_BANDINGLABEL)->m_hWnd,0);

    EnableHardScale(0);
    EnableHardVolScale(0);
    switch(ChartData.ScaleTo)
    {
      case 0:CheckDlgButton(IDC_SCALETOPRICE,(BOOL)1);break;
      case 1:CheckDlgButton(IDC_SCALETONBBO,(BOOL)1);break;
      case 2:CheckDlgButton(IDC_SCALETOALL,(BOOL)1);break;
      case 3:CheckDlgButton(IDC_SCALETOHARD,(BOOL)1);
             EnableHardScale(1);
             break;
    }
    switch(ChartData.VolScaleTo)
    {
      case 0:CheckDlgButton(IDC_SCALEVOLAUTO,(BOOL)1);break;
      case 1:CheckDlgButton(IDC_SCALEVOLHARD,(BOOL)1);break;
             EnableHardVolScale(1);
             break;
    }
    

    if (!SplitSizeChart)
    {
      strcpy(VolStr1,"Size");
      strcpy(VolStr2,"Size");               
    }
    else
    {
      strcpy(VolStr1,"QSize Splt");
      strcpy(VolStr2,"QSze");               
    }

    VolLenOffset1=(int)(strlen(VolStr1)*Chart.FontHeight/2);
    VolLenOffset2=(int)(strlen(VolStr2)*Chart.FontHeight/2);
    
    RECT Rect;
    RECT Rect2;
    GetClientRect(&Rect);     
    
    JTNX_Interface_SetMessageCallback(JTNX_NxGlobalUserData,NxS_RTDataCallBack);

    SysStatusControl.GetClientRect(&Rect2);
    SysStatusControl.SetWindowPos(NULL,Rect.right/2-(Rect2.right-Rect2.left)/2,Rect.bottom-25,0,0,SWP_NOSIZE);
   
    ClearColor=BackgroundColor;
    StudyTextColor=TextColor;
    ScaleTextColor=TextColor;
    DescripTextColor=TextColor;
  
    PriceY1=DPIpx(288);
    int Remaining;
        
    Remaining=(Rect.bottom-PriceY1-DPIpx(30))/3;
    if ((ShowPrice)&&(ShowVol))
    {
      PriceHeight=Remaining+DPIpx(40);
      VolY1=PriceY1+Remaining+DPIpx(40);
      VolHeight=LegendHeight=Remaining-DPIpx(20);      
      LegendY1=VolY1+Remaining-DPIpx(20);
    }
    else if (ShowPrice)
    {
      PriceHeight=Remaining*2;
      LegendHeight=Remaining;
      LegendY1=PriceY1+Remaining*2;
    }
    else if (ShowVol)
    {
      VolY1=PriceY1;
      VolHeight=Remaining*2;
      LegendHeight=Remaining;
      LegendY1=VolY1+Remaining*2;
    }
             
	GraphControl.SetWindowPos(NULL,0,PriceY1,Rect.right,PriceHeight,SWP_NOZORDER);              
    GraphControl.GetClientRect(&Rect);  
	Chart.InitStudyChart(GraphControl.GetDC(),Rect.right,Rect.bottom);	
    Chart.SetChartFormatting(20,60,15,28,0,5,10,10);
	Chart.SetChartFont(&ChartFont,&ChartFont,&ChartFont);       
    Chart.SetChartColors(ClearColor,ScaleTextColor,
                        GraphBckgrnd,StudyTextColor,ChartOutlines,
                        DescripTextColor,GridColor);
    Chart.SetGridColor(GridColor);
    Chart.SetTimeTickColor(TimeTickHighColor,TimeTickLowColor);
    Chart.SetTradeOutlineColor(TradeOutlineColor);

    VolGraphControl.GetClientRect(&Rect);  
    VolGraphControl.SetWindowPos(NULL,0,VolY1,Rect.right,VolHeight,SWP_NOZORDER);                  	
    VolGraphControl.GetClientRect(&Rect);  
	VolChart.InitStudyChart(VolGraphControl.GetDC(),Rect.right,Rect.bottom);	
        
    if (ShowPrice) VolChart.SetChartFormatting(20,60,5,28,0,5,5,0);
    else VolChart.SetChartFormatting(20,60,20,28,0,5,5,0);
   
    VolChart.SetChartFont(&ChartFont,&ChartFont,&ChartFont);
	VolChart.SetChartColors(ClearColor,ScaleTextColor,
                        GraphBckgrnd,StudyTextColor,ChartOutlines,
                        DescripTextColor,GridColor);
    VolChart.SetGridColor(GridColor);
    VolChart.SetTimeTickColor(TimeTickHighColor,TimeTickLowColor);
	
    LegendGraphControl.GetClientRect(&Rect);  
    LegendGraphControl.SetWindowPos(NULL,0,LegendY1,Rect.right,LegendHeight,SWP_NOZORDER);                  	
    LegendGraphControl.GetClientRect(&Rect);  
	LegendChart.InitStudyChart(LegendGraphControl.GetDC(),Rect.right,Rect.bottom);	
    LegendChart.SetChartFormatting(20,60,5,13,0,5,5,0);
	LegendChart.SetChartFont(&ChartFont,&ChartFont,&ChartFont);
	LegendChart.SetChartColors(ClearColor,ScaleTextColor,
                        GraphBckgrnd,StudyTextColor,ChartOutlines,
                        DescripTextColor,GridColor);
    
    VolChart.SetTradeOutlineColor(TradeOutlineColor);
    VolChart.SetStudyType(SplitSizeChart);
        
    CheckDlgButton(IDC_GRIDS,ShowGrids);   
    CheckDlgButton(IDC_VOLUMEON,ShowVol);    
    CheckDlgButton(IDC_PRICEON,ShowPrice);    
    CheckDlgButton(IDC_LEGENDON,ShowLegend);    
    
    CheckDlgButton(IDC_TIMEFULL,1);              
            
    PriceLenOffest1=(int)(strlen(PriceStr1)*Chart.FontHeight/2);    
    VolLenOffset1=(int)(strlen(VolStr1)*Chart.FontHeight/2);
    PriceLenOffest2=(int)(strlen(PriceStr2)*Chart.FontHeight/2);
    VolLenOffset2=(int)(strlen(VolStr2)*Chart.FontHeight/2);
    LegendLenOffset1=(int)(strlen(LegendStr1)*Chart.FontHeight/2);
    LegendLenOffset2=(int)(strlen(LegendStr2)*Chart.FontHeight/2);

    char str[32];
    sprintf(str,"%d",PointExpand2);
    CompWidthControl.SetWindowTextA(str);
    sprintf(str,"%d",PointExpand);
    OtherWidthControl.SetWindowTextA(str);
    sprintf(str,"%0.1f",ChartData.BBOBandAmount*100.0);
    BandingControl.SetWindowTextA(str);
        
    int StartY=0;
    int StartX;
    if (DPIScale<=1.0)
      StartX=DPIpx(350);
    else
      StartX=DPIpx(375);

    Rect.right=DPIpx(40);Rect.bottom=DPIpx(16);
    for (int i=0;i<3;i++)
    {      
       LabelText[i].Create("",SS_LEFT,Rect,this);
       LabelText[i].SetWindowPos(NULL,StartX+(DPIpx(60)*i),StartY,0,0,SWP_NOSIZE);
       LabelText[i].SetFont(GetFont(),0);
       LabelText[i].ShowWindow(SW_SHOWNORMAL);
    }
    LabelText[0].SetWindowTextA("Trades");
    LabelText[1].SetWindowTextA("Bids");
    LabelText[2].SetWindowTextA("Asks");

    StartY+=DPIpx(16);
    Rect.left=0;Rect.top=0;
    Rect.right=DPIpx(16);Rect.bottom=DPIpx(16);
    for (int i=0;i<3;i++)
    {
      OnButton[i].Create("O",BS_PUSHBUTTON  | WS_TABSTOP,Rect,this,IDC_ONBUTTONS+i);
      OnButton[i].SetWindowPos(NULL,StartX+(DPIpx(60)*i),StartY,0,0,SWP_NOSIZE);
      OnButton[i].ShowWindow(SW_SHOWNORMAL);
      OnButton[i].SetFont(GetFont(),0);
      OffButton[i].Create("X",BS_PUSHBUTTON  | WS_TABSTOP,Rect,this,IDC_OFFBUTTONS+i);
      OffButton[i].SetWindowPos(NULL,StartX+(DPIpx(60)*i)+DPIpx(16),StartY,0,0,SWP_NOSIZE);
      OffButton[i].ShowWindow(SW_SHOWNORMAL);      
      OffButton[i].SetFont(GetFont(),0);
      AutoButton[i].Create("A",BS_PUSHBUTTON  | WS_TABSTOP,Rect,this,IDC_AUTOBUTTONS+i);
      AutoButton[i].SetWindowPos(NULL,StartX+(DPIpx(60)*i)+DPIpx(32),StartY,0,0,SWP_NOSIZE);
      AutoButton[i].ShowWindow(SW_SHOWNORMAL);
      AutoButton[i].SetFont(GetFont(),0);
    }
   
    StartY+=DPIpx(4);   
    for (int i=0;i<MAXEXCHANGES;i++)
    {    
       Rect.left=0;Rect.top=0;
       Rect.right=DPIpx(12);Rect.bottom=DPIpx(12);
       TradeButton[i].Create("",BS_AUTOCHECKBOX | WS_TABSTOP,Rect,this,IDC_TRADEBUTTONS+i);
       TradeButton[i].SetWindowPos(NULL,StartX,StartY,0,0,SWP_NOSIZE);
       TradeButton[i].ShowWindow(SW_SHOWNORMAL);

       if (i==0) Rect.right=DPIpx(16);else Rect.right=DPIpx(32);
       Rect.bottom=14;
       TradeText[i].Create(ExchangeMap[i].Title,SS_LEFT,Rect,this);
       TradeText[i].SetWindowPos(NULL,StartX+DPIpx(20),StartY,0,0,SWP_NOSIZE);
       TradeText[i].SetFont(GetFont(),0);
       TradeText[i].ShowWindow(SW_SHOWNORMAL);
       
       Rect.left=0;Rect.top=0;
       Rect.right=DPIpx(12);Rect.bottom=DPIpx(12);
       BidButton[i].Create("",BS_AUTOCHECKBOX | WS_TABSTOP,Rect,this,IDC_BIDBUTTONS+i);
       BidButton[i].SetWindowPos(NULL,StartX+DPIpx(60),StartY,0,0,SWP_NOSIZE);
       BidButton[i].ShowWindow(SW_SHOWNORMAL);

       if (i==0) Rect.right=DPIpx(16);else Rect.right=DPIpx(32);
       Rect.bottom=DPIpx(14);
       BidText[i].Create(ExchangeMap[i].Title,SS_LEFT ,Rect,this);
       BidText[i].SetWindowPos(NULL,StartX+DPIpx(80),StartY,0,0,SWP_NOSIZE);
       BidText[i].SetFont(GetFont(),0);
       BidText[i].ShowWindow(SW_SHOWNORMAL);

       Rect.left=0;Rect.top=0;
       Rect.right=DPIpx(12);Rect.bottom=DPIpx(12);
       AskButton[i].Create("",BS_AUTOCHECKBOX | WS_TABSTOP,Rect,this,IDC_ASKBUTTONS+i);
       AskButton[i].SetWindowPos(NULL,StartX+DPIpx(120),StartY,0,0,SWP_NOSIZE);
       AskButton[i].ShowWindow(SW_SHOWNORMAL);

       if (i==0) Rect.right=DPIpx(16);else Rect.right=DPIpx(32);
       Rect.bottom=DPIpx(14);
       AskText[i].Create(ExchangeMap[i].Title,SS_LEFT ,Rect,this);
       AskText[i].SetWindowPos(NULL,StartX+DPIpx(140),StartY,0,0,SWP_NOSIZE);
       AskText[i].SetFont(GetFont(),0);
       AskText[i].ShowWindow(SW_SHOWNORMAL);
       
       StartY+=DPIpx(14);
    }
    TradeText[0].SetWindowTextA("On");
    BidText[0].SetWindowTextA("On");
    AskText[0].SetWindowTextA("On");
        
    Rect.left=0;Rect.top=0;
    Rect.right=DPIpx(13);Rect.bottom=DPIpx(13);

    for (int i=0;i<3;i++)
    {
      PointButton[i].Create("",BS_AUTORADIOBUTTON | WS_TABSTOP,Rect,this,IDC_DOTBUTTONS+i);
      PointButton[i].SetWindowPos(NULL,StartX+DPIpx(28),StartY,0,0,SWP_NOSIZE);
      PointButton[i].ShowWindow(SW_SHOWNORMAL);

      LineButton[i].Create("",BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP,Rect,this,IDC_LINEBUTTONS+i);
      LineButton[i].SetWindowPos(NULL,StartX,StartY,0,0,SWP_NOSIZE);
      LineButton[i].ShowWindow(SW_SHOWNORMAL);
     
      LineText[i].Create("L",SS_LEFT ,Rect,this);
      LineText[i].SetWindowPos(NULL,StartX+DPIpx(14),StartY,0,0,SWP_NOSIZE);
      LineText[i].SetFont(GetFont(),0);
      LineText[i].ShowWindow(SW_SHOWNORMAL);      

      PointText[i].Create("P",SS_LEFT ,Rect,this);
      PointText[i].SetWindowPos(NULL,StartX+DPIpx(42),StartY,0,0,SWP_NOSIZE);
      PointText[i].SetFont(GetFont(),0);
      PointText[i].ShowWindow(SW_SHOWNORMAL);
      StartX+=DPIpx(60);
    }

    StartY=DPIpx(155);
    if (DPIScale<=1.0)
      StartX=DPIpx(225);
    else
      StartX=DPIpx(250);

    TradeButton[0].SetWindowPos(NULL,StartX,StartY,0,0,SWP_NOSIZE);
    TradeText[0].SetWindowPos(NULL,StartX+DPIpx(14),StartY,0,0,SWP_NOSIZE);
    BidButton[0].SetWindowPos(NULL,StartX,StartY+DPIpx(14),0,0,SWP_NOSIZE);
    BidText[0].SetWindowPos(NULL,StartX+DPIpx(14),StartY+DPIpx(14),0,0,SWP_NOSIZE);
    AskButton[0].SetWindowPos(NULL,StartX,StartY+DPIpx(28),0,0,SWP_NOSIZE);
    AskText[0].SetWindowPos(NULL,StartX+DPIpx(14),StartY+DPIpx(28),0,0,SWP_NOSIZE);

    PointButton[0].EnableWindow(FALSE);
    PointButton[0].ShowWindow(SW_HIDE);
    LineButton[0].EnableWindow(FALSE);
    LineButton[0].ShowWindow(SW_HIDE);
    PointText[0].ShowWindow(SW_HIDE);
    LineText[0].ShowWindow(SW_HIDE);


    Rect.left=0;Rect.top=0;
    for (int i=0;i<3;i++)
    {
      Rect.right=DPIpx(13);Rect.bottom=DPIpx(13);

      CPointButton[i].Create("",BS_AUTORADIOBUTTON | WS_TABSTOP,Rect,this,IDC_CDOTBUTTONS+i);
      CPointButton[i].SetWindowPos(NULL,StartX+DPIpx(64),StartY,0,0,SWP_NOSIZE);
      CPointButton[i].ShowWindow(SW_SHOWNORMAL);

      CLineButton[i].Create("",BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP,Rect,this,IDC_CLINEBUTTONS+i);
      CLineButton[i].SetWindowPos(NULL,StartX+DPIpx(36),StartY,0,0,SWP_NOSIZE);
      CLineButton[i].ShowWindow(SW_SHOWNORMAL);     

      CLineText[i].Create("L",SS_LEFT ,Rect,this);
      CLineText[i].SetWindowPos(NULL,StartX+DPIpx(50),StartY,0,0,SWP_NOSIZE);
      CLineText[i].SetFont(GetFont(),0);
      CLineText[i].ShowWindow(SW_SHOWNORMAL);
      
      CPointText[i].Create("P",SS_LEFT ,Rect,this);
      CPointText[i].SetWindowPos(NULL,StartX+DPIpx(78),StartY,0,0,SWP_NOSIZE);
      CPointText[i].SetFont(GetFont(),0);
      CPointText[i].ShowWindow(SW_SHOWNORMAL);

      Rect.right=DPIpx(60);Rect.bottom=DPIpx(12);
      CLabel[i].Create("",SS_RIGHT ,Rect,this);
      CLabel[i].SetWindowPos(NULL,StartX-DPIpx(65),StartY,0,0,SWP_NOSIZE);
      CLabel[i].SetFont(GetFont(),0);
      CLabel[i].ShowWindow(SW_SHOWNORMAL);

      StartY+=DPIpx(14);
    }

    StartY=DPIpx(141);
    Rect.right=DPIpx(88);Rect.bottom=DPIpx(12);
    CLabel[3].Create("Composite/NBBO",SS_RIGHT ,Rect,this);
    CLabel[3].SetWindowPos(NULL,StartX-DPIpx(10),StartY,0,0,SWP_NOSIZE);
    CLabel[3].SetFont(GetFont(),0);
    CLabel[3].ShowWindow(SW_SHOWNORMAL);

    CLabel[0].SetWindowTextA("Trade:");
    CLabel[1].SetWindowTextA("Bid:");
    CLabel[2].SetWindowTextA("Ask:");

    if (ChartData.DrawTradeLine)  LineButton[0].SetCheck(1);
    else PointButton[0].SetCheck(1);
    if (ChartData.DrawBidLine)  LineButton[1].SetCheck(1);
    else PointButton[1].SetCheck(1);
    if (ChartData.DrawAskLine)  LineButton[2].SetCheck(1);
    else PointButton[2].SetCheck(1);

    if (ChartData.DrawCTradeLine)  CLineButton[0].SetCheck(1);
    else CPointButton[0].SetCheck(1);
    if (ChartData.DrawCBidLine)  CLineButton[1].SetCheck(1);
    else CPointButton[1].SetCheck(1);
    if (ChartData.DrawCAskLine)  CLineButton[2].SetCheck(1);
    else CPointButton[2].SetCheck(1);

    LegendChart.FindMaxBar(&ChartData);
    RepositionChart(0,0);
    DisableExchanges();


    time_t ltime;
    time( &ltime ); 
    _localtime64_s( &TodaysDate, &ltime ); 

	StartTimeControl.SetFormat("HH:mm:ss");
	StopTimeControl.SetFormat("HH:mm:ss");
	CTime Start=CTime(TodaysDate.tm_year+1900,TodaysDate.tm_mon+1,TodaysDate.tm_mday,StartHour,StartMin,StartSec,-1);	
	StartTimeControl.SetTime(&Start);
	CTime Stop=CTime(TodaysDate.tm_year+1900,TodaysDate.tm_mon+1,TodaysDate.tm_mday,StopHour,StopMin,StopSec,-1);		
	StopTimeControl.SetTime(&Stop);


    if (StartCoreOnStartup)
	{
		NxCoreIsRunning=1;		
		unsigned char OkToGo=0;
		if (FWXNxCoreMode==NXCORE_REALTIME)
		{
			if (JTNX_Interface_Start(JTNX_NxGlobalUserData,NULL,DLLFilename,1)==0)	OkToGo=1;
		}
		else
		{
			if (JTNX_Interface_Start(JTNX_NxGlobalUserData,TapeFileName,DLLFilename,1)==0)	OkToGo=1;
		}

	    if (!OkToGo) SysStatusControl.SetWindowTextA("Cound not start NxCore.");	
	}
	else
	    SysStatusControl.SetWindowTextA("Waiting to Start.");	

    SymbolControl.SetFocus();    
    ViewInitiated=1;   

    SetTimer(4,100,NULL);
        
	return FALSE;  // return TRUE  unless you set the focus to a control
}

// Called by framework when user presses Enter Key
// Capture so window does not close when enter pressed.
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnOK()
{
  InterestWait=1;
  StopFromUser=0;

  SetRExchanges(RExgFilterStr);

  DataPointForceSet=1;
  DataPointsControl.SetWindowTextA("");  
  DataPointForceSet=1;
  StartPointControl.SetWindowTextA("");

  DisableExchanges();
  InterestStatusControl.SetWindowTextA("");   
  TradeDateString[0]=0;
  NameString[0]=0;
  NameString2[0]=0;
  CountStr[0]=0;

  CurrChartBar=0;
  ChartData.PriceGridDivisor=0.0;    
  ChartStartBar=0;
  ChartStopBar=0;
  ViewStartBar=0;
  ViewStopBar=0;
  ViewDataPoints=0;

  TotalTradeCount=0;
  TotalQuoteCount=0;
  
  Chart.ResetZeroRange();
  VolChart.ResetZeroRange();
  DoPriceRecalc=DoVolRecalc=1;
  UserInRT=1;
  MaxBarError=0;
     
  NextStepButton.EnableWindow(FALSE);
  HistTimeMode=TIMEMODE_FULLTIME;
  JTNX_Interface_SetHistMode(JTNX_NxGlobalUserData,HistTimeMode,RTSimDelay);
  CheckDlgButton(IDC_TIMEFULL,1);
  CheckDlgButton(IDC_TIMESIM,0);
  CheckDlgButton(IDC_TIMESTEP,0); 

  ClearChart();
  
  char NxSymbol[32];
  sprintf(NxSymbol,"e%s",strupr(Symbol));
  
  unsigned char ListedExg=0;  
  if (strstr(NxSymbol,":"))
  {      
      char *sp=NxSymbol;
      sp=JTParser_ReadToDelim(sp,NxSymbol,':');
      *(sp-1)=0;
      ListedExg=atoi(sp);
  }
   
  JTNX_Interface_SetUserSymbol(JTNX_NxGlobalUserData,NxSymbol,ListedExg);

  if (!NxCoreIsRunning) 
  {    
    StartFromOnOK=1;
    OnBnClickedStartcore();
  }
}

// Called by framework when user presses Close or Esc to exit the app.
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnCancel()
{
  unsigned char OkToClose=1;
  if (NxCoreIsRunning)
  {
    if (MessageBox("The slave is currently running! Shutting down the feed slave will stop all processing.\nDo you really want to stop the NxFeed Slave?","Warning!",MB_YESNO | MB_ICONQUESTION)==IDNO)
    {
      OkToClose=0;
    }
    // If NxCore is running and we allow the app to close here, bad things will happen so...
    else
    {
      // Trigger the stop in NxCore interface
      StopFromUser=1;
      JTNX_Interface_TriggerStop(JTNX_NxGlobalUserData);		
      
      // Give NxCore a full second to shut down. Timer 3 will then call this function again.
      SetTimer(3,1000,NULL);
      return;
    }
  }
  if (OkToClose)
  {
    NxCoreIsRunning=0;
	KillTimer(1);
	KillTimer(4);
	WriteINIFile(CORECONFIGNAME);		
    JTNX_Interface_FreeGlobalUserData(JTNX_NxGlobalUserData);  

    BackgroundBrush.DeleteObject();
	WinBrush1.DeleteObject();
	WinBrush2.DeleteObject();
    WinBrush3.DeleteObject();

    FreeChartMem(&ChartData);
    FreeColorMem(&ChartColors);
    ::DeleteCriticalSection( &DisplaySection );

    CDialog::OnCancel();
  }
}

// On Timer Function. Called once a second for timer (1) and
// at the end of a tape for timer (2)
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnTimer(UINT_PTR idEvent) 
{
    switch(idEvent)
	{	    
		// Timer set by EndOfTape function
		case 1:			 
			 KillTimer(1);

			 // If system is running in real time buffers, restart
			 // on the next day, after the last tape ended.
	         if (FWXNxCoreMode==NXCORE_REALTIME)
			 {
			     NxCoreIsRunning=1;
                 JTLog_ResetFileName(&AppLog,LOGNAMEPREFIX);
		         JTNX_Interface_Start(JTNX_NxGlobalUserData,NULL,DLLFilename,1);		             						 

                 if (Symbol[0]) OnOK();
			 }
			 // Otherwise we are running from historical tape, so 
			 // shut everything down properly.
	         else	
			 {
				 NxCoreIsRunning=0;
				 JTNX_Interface_Stop(JTNX_NxGlobalUserData);
				 SysStatusControl.SetWindowTextA("NxCore has been stopped.");			    							                    
		 	 }			 
			 break;     		

       // Timer 3 is set only when app is closed while NxCore is running.
       case 3:KillTimer(3);
             NxCoreIsRunning=0;
             OnCancel();
             break;

       case 4:KillTimer(4);
             RefreshChart();
             RefreshLegendChart();
             break;    
	}
}


////////////////////////////////////
// CHART DRAWING/SIZING FUNCTIONS //
////////////////////////////////////

void CJTools_ShellAppDlg::CountEm(void)
{
  if (!ChartStopBar) return;
  TotalTradeCount=0;
  TotalQuoteCount=0;

  for (int loop2=0;loop2<MAXEXCHANGES;loop2++)
  {
    ChartData.Array[loop2]->TotalTradeCount=0;
    ChartData.Array[loop2]->TotalQuoteCount=0;
  }

  for (int loop=ViewStartBar;loop<=ViewStopBar;loop++)
  {
    // Start at Index #1....We don't want to count the composite entries (which is index 0)
    for (int loop2=1;loop2<MAXEXCHANGES;loop2++)
    {
      if (ChartData.Array[loop2]->Volume[loop]) 
      {
        ++TotalTradeCount;
        ++ChartData.Array[loop2]->TotalTradeCount;
      }
      if ((ChartData.Array[loop2]->BidVolume[loop])||(ChartData.Array[loop2]->AskVolume[loop])) 
      {
        ++TotalQuoteCount;
        ++ChartData.Array[loop2]->TotalQuoteCount;
      }
    }
  }  
}

void CJTools_ShellAppDlg::MakeTitleString(void)
{  
  CountStr[0]=0;

  if (!ChartStopBar) return;

  ChartNumBars=ChartStopBar;

  char TradeCString[50],TradeSCString[50],QuoteCString[50];
  FormatIntWCommas(TotalTradeCount,TradeCString);
  FormatIntWCommas(TotalQuoteCount,QuoteCString);

  int StartMS=ChartData.MSTOD[ViewStartBar];
  int StopMS=ChartData.MSTOD[ViewStopBar];;

  int Seconds=0;
  int Millis=0;
  int Minutes=0;
  int Hours=0;

  if (StartMS>0)
  {
    int TotalMS=StopMS-StartMS;
    
    Seconds=TotalMS/1000;
    Millis=TotalMS-(Seconds*1000);
    Minutes=Seconds/60;
    Hours=Minutes/60;
    Minutes=Minutes-(Hours*60);
    Seconds=Seconds-(Minutes*60);
        
    char HourStr[25],MinStr[25],SecStr[25],MSStr[25];
    CountStr[0]=0;    
    if (1)
    {
       HourStr[0]=MinStr[0]=SecStr[0]=MSStr[0]=0;
    
       if (Hours>0)
       {
         if (Hours>1)
           sprintf(HourStr,"%d Hours, ",Hours);
         else
           sprintf(HourStr,"%d Hour, ",Hours);
       }
       if ((Minutes>0)||(Hours>0))
       {
         if (Minutes==1)
            sprintf(MinStr,"%d Minute, ",Minutes);
         else
            sprintf(MinStr,"%d Minutes, ",Minutes);
       }
       if ((Seconds>0)||(Minutes>0)||(Hours>0))
       {
         if (Seconds==1)
            sprintf(SecStr,"%d Second, ",Seconds);
         else
            sprintf(SecStr,"%d Seconds, ",Seconds);
       }     
       sprintf(MSStr,"%d Milliseconds",Millis);
       
       if (1) strcat(MSStr,"  -  ");

       sprintf(CountStr,"Elapsed Time: %s%s%s%s",HourStr,MinStr,SecStr,MSStr);      
     }
   }

  unsigned char AnyTradesOn=0,AnyQuotesOn=0;
  for (int loop2=0;loop2<MAXEXCHANGES;loop2++)
  {
    if (ChartData.Array[loop2]->TradeOn) AnyTradesOn=1;
    if (ChartData.Array[loop2]->BidOn) AnyQuotesOn=1;
    if (ChartData.Array[loop2]->AskOn) AnyQuotesOn=1;
  }

  char str[64];
  if ((!ExcludeQTQuotes)&&(AnyQuotesOn))
  {
       sprintf(str,"Total Quotes: %s  ",QuoteCString);
       strcat(CountStr,str);
  }
  if ((!ExcludeQTTrades)&&(AnyTradesOn))
  {
       sprintf(str,"Total Trades: %s  ",TradeCString);
       strcat(CountStr,str);
  }
  
}

//----------------------------------------
void CJTools_ShellAppDlg::ClearChart(void)
{
    if ((IsIconic())||(!ViewInitiated)) return;  

    ::EnterCriticalSection( &DisplaySection );
    Chart.ChartPrepToDraw();
    Chart.ClearChart();
    VolChart.ChartPrepToDraw();
    VolChart.ClearChart();
    LegendChart.ChartPrepToDraw();
    LegendChart.ClearChart();
    CDC *hdc=GraphControl.GetDC();		       
    Chart.ChartToDisplay(hdc);    
    ReleaseDC(hdc);
    hdc=VolGraphControl.GetDC();	
    VolChart.ChartToDisplay(hdc);
    ReleaseDC(hdc);
    hdc=LegendGraphControl.GetDC();	
    LegendChart.ChartToDisplay(hdc);
    ReleaseDC(hdc);
       
    ::LeaveCriticalSection( &DisplaySection );
}

//----------------------------------------
void CJTools_ShellAppDlg::RefreshChart(void)
{
    if ((IsIconic())||(!ViewInitiated)) return;  

    ::EnterCriticalSection( &DisplaySection );
    
    CDC *hdc;

    if ((DoCountReCalc)&&(CurrChartBar)) 
    {
      DoCountReCalc=0;
      CountEm();
      MakeTitleString();      
    }

    if (ShowPrice)
    {
      Chart.ChartPrepToDraw();
      Chart.ClearChart();
    }
    if (ShowVol)
    {
      VolChart.ChartPrepToDraw();
      VolChart.ClearChart();
    }

    if (ShowPrice)
    {
      hdc=GraphControl.GetDC();		       
      
      if (CurrChartBar)
      {
        if (DoPriceRecalc)
        {
          Chart.FindMaxBar(&ChartData);
          DoPriceRecalc=0;      
        }
        Chart.DrawScale(&ChartData,TradePriceType,ShowGrids);
        Chart.DrawTimeLine(&ChartData,2,ShowGrids,ShowTimeTime,ShowTimeCounts,ShowReverseTime);

        Chart.DrawStudy(&ChartData,&ChartColors,ShowISO,PointExpand,PointExpand2);
      }
      
      if (Chart.ChartingHeight>175)
        Chart.DrawLeftVertLabel(10,Chart.ChartingHeight/2-PriceLenOffest1,PriceStr1);
      else if (Chart.ChartingHeight>75)
        Chart.DrawLeftVertLabel(10,Chart.ChartingHeight/2-PriceLenOffest2,PriceStr2);
      else
        Chart.DrawLeftVertLabel(10,-4,"Price");
      
      Chart.DrawNanexLogo(5,5,RGB(140,140,180));

      if (CurrChartBar)
      {
        char TitleStr[256];
        sprintf(TitleStr,"%s  -  %s",NameString,CountStr);
        Chart.DrawTitle(TitleStr);//NameString);
      }

      Chart.DrawProductTitle("Nanex QT Sequencer G4");

      Chart.ChartToDisplay(hdc);    
      ReleaseDC(hdc);
    }

    if (ShowVol)
    {      
      hdc=VolGraphControl.GetDC();	

      if (CurrChartBar)
      {
        if (DoVolRecalc)
        {        
          VolChart.FindMaxBar(&ChartData);
          DoVolRecalc=0;
        }
        
        if (!SplitSizeChart)
           VolChart.DrawScale(&ChartData,ShowGrids);       
        else
          VolChart.DrawScale2(&ChartData,ShowGrids);       
        VolChart.DrawTimeLine(&ChartData,2,ShowGrids,ShowTimeTime,ShowTimeCounts,ShowReverseTime);

        if (!SplitSizeChart)
          VolChart.DrawStudy(&ChartData,&ChartColors,ExchangeMap,ShowISO,PointExpand,PointExpand2);
        else
          VolChart.DrawStudy2(&ChartData,&ChartColors,ExchangeMap,ShowISO,PointExpand,PointExpand2);

        if (!ShowPrice)
        {
          VolChart.DrawNanexLogo(5,5,RGB(140,140,180));

          if (CurrChartBar)
          {
            char TitleStr[256];
            sprintf(TitleStr,"%s  -  %s",NameString2,CountStr);
            VolChart.DrawTitle(TitleStr);
          }
          VolChart.DrawProductTitle("Nanex QT Sequencer G4");
        }
      }

      if (VolChart.ChartingHeight>80)
        VolChart.DrawLeftVertLabel(10,VolChart.ChartingHeight/2-VolLenOffset1,VolStr1);
      else if (VolChart.ChartingHeight>30)
        VolChart.DrawLeftVertLabel(10,VolChart.ChartingHeight/2-VolLenOffset2,VolStr2);
      else
        VolChart.DrawLeftVertLabel(10,-4,"Vl");
      VolChart.ChartToDisplay(hdc);

      ReleaseDC(hdc);
    }
   
    ::LeaveCriticalSection( &DisplaySection );
}

//----------------------------------------
void CJTools_ShellAppDlg::RefreshLegendChart(void)
{
    if ((IsIconic())||(!ShowLegend)||(!ViewInitiated)/*||(!CurrChartBar)*/) return;  

    ::EnterCriticalSection( &DisplaySection );
    CDC *hdc;
    LegendChart.ChartPrepToDraw();
    LegendChart.ClearChart();
    if (LegendChart.ChartingHeight>80)
      LegendChart.DrawLeftVertLabel(10,LegendChart.ChartingHeight/2-LegendLenOffset1,LegendStr1);
    else if (LegendChart.ChartingHeight>30)
      LegendChart.DrawLeftVertLabel(10,LegendChart.ChartingHeight/2-LegendLenOffset2,LegendStr2);
    else
      LegendChart.DrawLeftVertLabel(10,-4,"lgn");

    if (CurrChartBar)
      LegendChart.DrawStudy(&ChartData,&ChartColors,ExchangeMap,ShowISO,PointExpand,PointExpand2);    

    hdc=LegendGraphControl.GetDC();	
    LegendChart.ChartToDisplay(hdc);
    ReleaseDC(hdc);
    ::LeaveCriticalSection( &DisplaySection );
}

//-------------------------------------------------------------
void CJTools_ShellAppDlg::RepositionChart(int cx,int cy)
{
    if ((cx==0)||(cy==0))
    {
       RECT Rect;
       GetClientRect(&Rect);
       cx=Rect.right;
       cy=Rect.bottom;
    }

    int Remaining;
    CDC *hdc;    
    if (ShowLegend)
    {
      LegendChart.FindMaxBar(&ChartData);      
      Remaining=(cy-PriceY1-30-(LegendChart.LegendStudyHeight+DPIpx(35)))/2;
      LegendHeight=LegendChart.LegendStudyHeight+DPIpx(35);
      LegendY1=cy-DPIpx(30)-(LegendChart.LegendStudyHeight+DPIpx(35));      
    }
    else
      Remaining=(cy-PriceY1-DPIpx(30))/2;

    if ((ShowPrice)&&(ShowVol))
    {
      if (ShowLegend)
      {
        PriceHeight=(int)((double)Remaining*1.20+DPIpx(40));
        VolY1=PriceY1+PriceHeight;
        VolHeight=((double)Remaining*0.80)-DPIpx(40);      
      }
      else
      {
        PriceHeight=(int)((double)Remaining*1.25+DPIpx(40));
        VolY1=PriceY1+PriceHeight;
        VolHeight=((double)Remaining*0.75)-DPIpx(40);      
      }
    }
    else if (ShowPrice)
    {
      PriceHeight=Remaining*2;
    }
    else if (ShowVol)
    {
      VolY1=PriceY1;
      VolHeight=Remaining*2;
    }
           
    //::EnterCriticalSection( &DisplaySection );
    GraphControl.SetWindowPos(NULL,0,PriceY1,cx,PriceHeight,SWP_NOZORDER);              
    VolGraphControl.SetWindowPos(NULL,0,VolY1,cx,VolHeight,SWP_NOZORDER);           
    LegendGraphControl.SetWindowPos(NULL,0,LegendY1,cx,LegendHeight,SWP_NOZORDER);           
    
    hdc=GraphControl.GetDC();
    Chart.ResizeChart(this,hdc,cx,PriceHeight);
    ReleaseDC(hdc);
    hdc=VolGraphControl.GetDC();
    VolChart.ResizeChart(this,hdc,cx,VolHeight);
    ReleaseDC(hdc);
       
    hdc=LegendGraphControl.GetDC();
    LegendChart.ResizeChart(this,hdc,cx,LegendHeight);
    ReleaseDC(hdc);
        
    DoPriceRecalc=DoVolRecalc=1;
    //::LeaveCriticalSection( &DisplaySection );

    Chart.SetStartStopBars(ViewStartBar,ViewStopBar);
    VolChart.SetStartStopBars(ViewStartBar,ViewStopBar);

    SetTimer(4,5,NULL);    
}


///////////////////////
// NXCORE START/STOP //
///////////////////////

// User clicked "Start NxCore"
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedStartcore()
{
	if (!NxCoreIsRunning)
	{	    
		JTNX_Interface_SetHistMode(JTNX_NxGlobalUserData,HistTimeMode,RTSimDelay);
        JTLog_ResetFileName(&AppLog,LOGNAMEPREFIX);
        
		unsigned char OkToGo=0;
        		
		if (FWXNxCoreMode==NXCORE_REALTIME)		
		{
			if (JTNX_Interface_Start(JTNX_NxGlobalUserData,NULL,DLLFilename,1)==0) OkToGo=1;		
		}
		else
		{					    
			if (JTNX_Interface_Start(JTNX_NxGlobalUserData,TapeFileName,DLLFilename,1)==0) OkToGo=1;
		}

	    if (OkToGo) 
        {
          NxCoreIsRunning=1;		
          if ((!StartFromOnOK)&&(Symbol[0])) 
          {            
            OnOK();
          }
          StartFromOnOK=0;
        }
	    else
          SysStatusControl.SetWindowTextA("Cound not start NxCore.");	
	}
}

// User clicked "Stop NxCore"
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedStopcore()
{	
	if (NxCoreIsRunning)
	{
	    StopFromUser=1;

		unsigned char OkToGo=1;
		if (FWXNxCoreMode==NXCORE_REALTIME)
		{
			if (MessageBox("WARNING! You are running from the NxCore Real-Time Buffers!\nStopping the Slave will force you to restart from the start of the day's buffer!\nAre you sure you want to do this?","Warning!",MB_YESNO | MB_ICONQUESTION)!=IDYES)
				OkToGo=0;
		}
		if (OkToGo)
		{							
			if (HistTimeMode!=TIMEMODE_FULLTIME)
			{
				JTNX_Interface_SetHistMode(JTNX_NxGlobalUserData,TIMEMODE_FULLTIME,RTSimDelay);	
				JTNX_Interface_TriggerStop(JTNX_NxGlobalUserData);			    
			}
			else
			{				
				JTNX_Interface_TriggerStop(JTNX_NxGlobalUserData);			    
			}
            NxCoreIsRunning=0;		

            NextStepButton.EnableWindow(FALSE);
            HistTimeMode=TIMEMODE_FULLTIME;
            JTNX_Interface_SetHistMode(JTNX_NxGlobalUserData,HistTimeMode,RTSimDelay);
            CheckDlgButton(IDC_TIMEFULL,1);
	        CheckDlgButton(IDC_TIMESIM,0);
	        CheckDlgButton(IDC_TIMESTEP,0); 

            EnableRTControl(1);
            InterestStatusControl.SetWindowTextA("");                          
            
		}		
	}
}


///////////////////////
// SUPPORT FUNCTIONS //
///////////////////////
//------------------------------------------------------------
void CJTools_ShellAppDlg::FreeChartMem(BarIndicatorType *ChartData)
{
  for (int loop=0;loop<MAXEXCHANGES;loop++)
  {
     if (ChartData->Array[loop]) free(ChartData->Array[loop]);    
  }
  if (ChartData->BBOBidBand) free(ChartData->BBOBidBand);
  if (ChartData->BBOAskBand) free(ChartData->BBOAskBand);

  if (ChartData->Hour) free(ChartData->Hour);
  if (ChartData->Min) free(ChartData->Min);
  if (ChartData->Sec) free(ChartData->Sec);
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::AllocateColorMem(ColorIndicatorType *ChartColors)
{
  for (int loop=0;loop<MAXEXCHANGES;loop++)
      ChartColors->Colors[loop]=(ColorRecordType *)calloc(1,sizeof(ColorRecordType));    
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::FreeColorMem(ColorIndicatorType *ChartColors)
{
  for (int loop=0;loop<MAXEXCHANGES;loop++)
  {
     if (ChartColors->Colors[loop]) 
     {
       ChartColors->Colors[loop]->LastPen.DeleteObject();
       ChartColors->Colors[loop]->BidPen.DeleteObject();
       ChartColors->Colors[loop]->AskPen.DeleteObject();
       ChartColors->Colors[loop]->LastBrush.DeleteObject();
       ChartColors->Colors[loop]->BidBrush.DeleteObject();
       ChartColors->Colors[loop]->AskBrush.DeleteObject();
       free(ChartColors->Colors[loop]);    
     }
  }
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::SetBandingColor(void)
{
  ChartColors.BBOBidBandPen.DeleteObject();
  ChartColors.BBOBidBandPen.CreatePen(PS_SOLID,ExchangeMap[0].PenWidth,ChartColors.BBOBidBandColor);
  ChartColors.BBOAskBandPen.DeleteObject();
  ChartColors.BBOAskBandPen.CreatePen(PS_SOLID,ExchangeMap[0].PenWidth,ChartColors.BBOAskBandColor);
  
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::SetColorMem(ColorIndicatorType *ChartColors,
                                      int Index,
                                      COLORREF LastColor,COLORREF BidColor,COLORREF AskColor,
                                      int PenWidth)
{
  if (ChartColors->Colors[Index]) 
  {
    ChartColors->Colors[Index]->LastPen.DeleteObject();
    ChartColors->Colors[Index]->BidPen.DeleteObject();
    ChartColors->Colors[Index]->AskPen.DeleteObject();
    ChartColors->Colors[Index]->LastBrush.DeleteObject();
    ChartColors->Colors[Index]->BidBrush.DeleteObject();
    ChartColors->Colors[Index]->AskBrush.DeleteObject();
    ChartColors->Colors[Index]->LastPen.CreatePen(PS_SOLID,1,LastColor);
    ChartColors->Colors[Index]->BidPen.CreatePen(PS_SOLID,1,BidColor);
    ChartColors->Colors[Index]->AskPen.CreatePen(PS_SOLID,1,AskColor);
    ChartColors->Colors[Index]->LastBrush.CreateSolidBrush(LastColor);
    ChartColors->Colors[Index]->BidBrush.CreateSolidBrush(BidColor);
    ChartColors->Colors[Index]->AskBrush.CreateSolidBrush(AskColor);
  }
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::EnableTradeExchange(int Index)
{
    if (((AutoTradesOn)&&(Index!=0))||((!AutoTradesOn)&&(Index==0))) TradeButton[Index].SetCheck(1);
    TradeButton[Index].EnableWindow(TRUE);
    TradeText[Index].EnableWindow(TRUE);
    OffButton[0].EnableWindow(TRUE);
    OnButton[0].EnableWindow(TRUE);
    AutoButton[0].EnableWindow(TRUE);
    LabelText[0].EnableWindow(TRUE);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::EnableBidExchange(int Index)
{
    BidButton[Index].SetCheck(1);
    BidButton[Index].EnableWindow(TRUE);
    BidText[Index].EnableWindow(TRUE);
    OffButton[1].EnableWindow(TRUE);
    OnButton[1].EnableWindow(TRUE);
    AutoButton[1].EnableWindow(TRUE);
    LabelText[1].EnableWindow(TRUE);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::EnableAskExchange(int Index)
{
    AskButton[Index].SetCheck(1);
    AskButton[Index].EnableWindow(TRUE);
    AskText[Index].EnableWindow(TRUE);
    OffButton[2].EnableWindow(TRUE);
    OnButton[2].EnableWindow(TRUE);
    AutoButton[2].EnableWindow(TRUE);
    LabelText[2].EnableWindow(TRUE);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::DisableExchanges(void)
{
  for (int loop=0;loop<MAXEXCHANGES;loop++)
  {
    TradeButton[loop].SetCheck(0);BidButton[loop].SetCheck(0);AskButton[loop].SetCheck(0);
    TradeButton[loop].EnableWindow(0);BidButton[loop].EnableWindow(0);AskButton[loop].EnableWindow(0);
    TradeText[loop].EnableWindow(0);BidText[loop].EnableWindow(0);AskText[loop].EnableWindow(0);
    
    if (loop<3)
    {
      OffButton[loop].EnableWindow(0);
      OnButton[loop].EnableWindow(0);
      AutoButton[loop].EnableWindow(0);
      LabelText[loop].EnableWindow(0);
    }
  }  
  if (ViewInitiated)
  {
    LegendChart.FindMaxBar(&ChartData);    
  }
}
//-----------------------------------------------------------
void CJTools_ShellAppDlg::EnableRTControl(unsigned char Enable)
{
  ::EnableWindow(GetDlgItem(IDC_ROUNDLOTADJ)->m_hWnd,Enable);
  ::EnableWindow(GetDlgItem(IDC_EXCLUDETRADES)->m_hWnd,Enable);
  ::EnableWindow(GetDlgItem(IDC_EXCLUDEQUOTES)->m_hWnd,Enable);  
  ::EnableWindow(GetDlgItem(IDC_RTLABEL1)->m_hWnd,Enable);
  ::EnableWindow(GetDlgItem(IDC_RTLABEL2)->m_hWnd,Enable);
  ::EnableWindow(GetDlgItem(IDC_RTLABEL3)->m_hWnd,Enable);
  ::EnableWindow(GetDlgItem(IDC_RTLABEL4)->m_hWnd,Enable);
  RExgControl.EnableWindow(Enable);
}

//-----------------------------------------------------------
int CJTools_ShellAppDlg::SetRExchanges(char *buff)
{
    
    RNumExchanges=0;
    if ((!buff)||(!buff[0]))
    {
	    memset(RExchangeToggles,1,sizeof(unsigned char)*256);	    
	    return 0;
    }

    memset(RExchangeToggles,0,sizeof(unsigned char)*256);
    char cat[10];
    char *c=cat;
    char *p=buff;
    int cnum=0;
   
    while ((*p)&&(p))
    {
	  if ((*p!=',')&&(*p))     
        *c++=*p++;
	  else
	  {
	    p++;
	    *c=0;
	    cnum=atoi(cat);
	    if ((cnum>=0)&&(cnum<256))
	    {
           RExchangeToggles[cnum]=1;		   
           ++RNumExchanges;
	    }
	    c=cat;
	  }
    }
    *c=0;
    cnum=atoi(cat);
    if ((cnum>=0)&&(cnum<256))
    {
       RExchangeToggles[cnum]=1;
       ++RNumExchanges;
    }
    return 0;
}


//////////////////
// USER ACTIONS //
//////////////////

//-------------------------------------------------------------
BOOL CJTools_ShellAppDlg::OnCommand(WPARAM wParam,LPARAM lParam)
{
  if ((wParam>=IDC_TRADEBUTTONS)&&(wParam<=IDC_TRADEBUTTONS+(MAXEXCHANGES-1)))
  {
    int ButtonID=wParam-IDC_TRADEBUTTONS;
    ChartData.Array[ButtonID]->TradeOn=!ChartData.Array[ButtonID]->TradeOn;
    if (!ChartData.Array[ButtonID]->TradeOn) ChartData.Array[ButtonID]->UToggleTradeOff=1;
    else ChartData.Array[ButtonID]->UToggleTradeOff=0;  
    DoCountReCalc=DoPriceRecalc=DoVolRecalc=1;
    RepositionChart(0,0);
  }
  else if ((wParam>=IDC_BIDBUTTONS)&&(wParam<=IDC_BIDBUTTONS+(MAXEXCHANGES-1)))
  {
    int ButtonID=wParam-IDC_BIDBUTTONS;
    ChartData.Array[ButtonID]->BidOn=!ChartData.Array[ButtonID]->BidOn;
    if (!ChartData.Array[ButtonID]->BidOn) ChartData.Array[ButtonID]->UToggleBidOff=1;
    else ChartData.Array[ButtonID]->UToggleBidOff=0;  
    DoCountReCalc=DoPriceRecalc=DoVolRecalc=1;
    RepositionChart(0,0);
  }
  else if ((wParam>=IDC_ASKBUTTONS)&&(wParam<=IDC_ASKBUTTONS+(MAXEXCHANGES-1)))
  {
    int ButtonID=wParam-IDC_ASKBUTTONS;
    ChartData.Array[ButtonID]->AskOn=!ChartData.Array[ButtonID]->AskOn;
    if (!ChartData.Array[ButtonID]->AskOn) ChartData.Array[ButtonID]->UToggleAskOff=1;
    else ChartData.Array[ButtonID]->UToggleAskOff=0;  
    DoCountReCalc=DoPriceRecalc=DoVolRecalc=1;
    RepositionChart(0,0);
  }
  else if ((wParam>=IDC_ONBUTTONS)&&(wParam<=IDC_ONBUTTONS+2))
  {
    int ButtonID=wParam-IDC_ONBUTTONS;    
    switch(ButtonID)
    {
    case 0:OnBnClickedTradeson();break;
    case 1:OnBnClickedBidson();break;
    case 2:OnBnClickedAskson();break;
    }    
    DoCountReCalc=DoPriceRecalc=DoVolRecalc=1;
    RepositionChart(0,0);
  }
  else if ((wParam>=IDC_OFFBUTTONS)&&(wParam<=IDC_OFFBUTTONS+2))
  {
    int ButtonID=wParam-IDC_OFFBUTTONS;    
    switch(ButtonID)
    {
    case 0:OnBnClickedTradesoff();break;
    case 1:OnBnClickedBidsoff();break;
    case 2:OnBnClickedAsksoff();break;
    }    
    DoCountReCalc=DoPriceRecalc=DoVolRecalc=1;
    RepositionChart(0,0);
  }
  else if ((wParam>=IDC_AUTOBUTTONS)&&(wParam<=IDC_AUTOBUTTONS+2))
  {
    int ButtonID=wParam-IDC_AUTOBUTTONS;    
    switch(ButtonID)
    {
    case 0:OnBnClickedTradesauto();break;
    case 1:OnBnClickedBidsauto();break;
    case 2:OnBnClickedAsksauto();break;
    }    
    DoCountReCalc=DoPriceRecalc=DoVolRecalc=1;
    RepositionChart(0,0);
  }
  else if ((wParam>=IDC_LINEBUTTONS)&&(wParam<=IDC_LINEBUTTONS+2))
  {
    int ButtonID=wParam-IDC_LINEBUTTONS;    
    switch(ButtonID)
    {
    case 0:ChartData.DrawTradeLine=1;break;
    case 1:ChartData.DrawBidLine=1;break;
    case 2:ChartData.DrawAskLine=1;break;
    }    
    RefreshChart();
  }
  else if ((wParam>=IDC_DOTBUTTONS)&&(wParam<=IDC_DOTBUTTONS+2))
  {
    int ButtonID=wParam-IDC_DOTBUTTONS;    
    switch(ButtonID)
    {
    case 0:ChartData.DrawTradeLine=0;break;
    case 1:ChartData.DrawBidLine=0;break;
    case 2:ChartData.DrawAskLine=0;break;
    }    
    RefreshChart();
  }
  else if ((wParam>=IDC_CLINEBUTTONS)&&(wParam<=IDC_CLINEBUTTONS+2))
  {
    int ButtonID=wParam-IDC_CLINEBUTTONS;    
    switch(ButtonID)
    {
    case 0:ChartData.DrawCTradeLine=1;break;
    case 1:ChartData.DrawCBidLine=1;break;
    case 2:ChartData.DrawCAskLine=1;break;
    }    
    RefreshChart();
    RefreshLegendChart();
  }
  else if ((wParam>=IDC_CDOTBUTTONS)&&(wParam<=IDC_CDOTBUTTONS+2))
  {
    int ButtonID=wParam-IDC_CDOTBUTTONS;    
    switch(ButtonID)
    {
    case 0:ChartData.DrawCTradeLine=0;break;
    case 1:ChartData.DrawCBidLine=0;break;
    case 2:ChartData.DrawCAskLine=0;break;
    }    
    RefreshChart();
    RefreshLegendChart();
  }

  CDialog::OnCommand(wParam,lParam);
  return TRUE;
}
// User clicked "Full"
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedTimefull()
{	
    if (HistTimeMode!=TIMEMODE_FULLTIME)
    {
      NextStepButton.EnableWindow(FALSE);
	  HistTimeMode=TIMEMODE_FULLTIME;
	  JTNX_Interface_SetHistMode(JTNX_NxGlobalUserData,HistTimeMode,RTSimDelay);			
      if ((Symbol[0])&&(ViewInitiated)) 
      {
        InterestStatusControl.SetWindowTextA("Forwarding to Start Time...");                          
        OnOK();
      }
    }
}
// User clicked "RT-Sim"
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedTimesim()
{
    NextStepButton.EnableWindow(FALSE);
    HistTimeMode=TIMEMODE_SIMTIME;
	JTNX_Interface_SetHistMode(JTNX_NxGlobalUserData,HistTimeMode,RTSimDelay);			
}
// User clicked "Next"
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedTimestep()
{
    NextStepButton.EnableWindow(TRUE);
    HistTimeMode=TIMEMODE_STEPTIME;
	JTNX_Interface_SetHistMode(JTNX_NxGlobalUserData,HistTimeMode,RTSimDelay);			
}
// User clicked "Step" button
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedStep()
{
    JTNX_Interface_PingRelease(JTNX_NxGlobalUserData);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeSymbol()
{
  CString String;
  SymbolControl.GetWindowText(String);
  strcpy(Symbol,String.GetBuffer());    
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedGrids()
{
  ShowGrids=!ShowGrids;
  RefreshChart();
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedChartpause()
{
   UpdateChartRT=!UpdateChartRT;
   if (UpdateChartRT) 
   {
     ChartStopBar=CurrChartBar;
     DoPriceRecalc=DoVolRecalc=1;

     UserInRT=1;
   }
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedVolumeon()
{
  ShowVol=!ShowVol;

  if (ShowVol)
  {
    if (ShowPrice) VolChart.SetChartFormatting(20,60,5,28,0,5,5,0);
    else VolChart.SetChartFormatting(20,60,20,28,0,5,5,0);
  }

  CRect Rect;    
  GetClientRect(&Rect);
  RepositionChart(Rect.right,Rect.bottom);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedPriceon()
{
  ShowPrice=!ShowPrice;

  if ((!ShowPrice)&&(ShowVol)) VolChart.SetChartFormatting(20,60,20,28,0,5,5,0);
  else VolChart.SetChartFormatting(20,60,5,28,0,5,5,0);

  CRect Rect;    
  GetClientRect(&Rect);
  RepositionChart(Rect.right,Rect.bottom);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedTradesoff()
{
  ::EnterCriticalSection( &DisplaySection );
  for (int loop=1;loop<MAXEXCHANGES;loop++)
  {
    if (ChartData.Array[loop]->TradeOn)
    {
      ChartData.Array[loop]->TradeOn=0;
      TradeButton[loop].SetCheck(0);      
    }
    ChartData.Array[loop]->UToggleTradeOff=1;      
  }
  ::LeaveCriticalSection( &DisplaySection );
  RepositionChart(0,0);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedTradeson()
{
  ::EnterCriticalSection( &DisplaySection );
  for (int loop=1;loop<MAXEXCHANGES;loop++)
  {
    if ((!ChartData.Array[loop]->TradeOn)&&(ChartData.Array[loop]->UToggleTradeOff))
    {
      ChartData.Array[loop]->TradeOn=1;
      TradeButton[loop].SetCheck(1);      
    }
    ChartData.Array[loop]->UToggleTradeOff=0;      
  }
  ::LeaveCriticalSection( &DisplaySection );
  RepositionChart(0,0);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedTradesauto()
{  
  ::EnterCriticalSection( &DisplaySection );
  for (int loop=1;loop<MAXEXCHANGES;loop++)
  {
    if (ChartData.Array[loop]->TradeOn)
    {
      ChartData.Array[loop]->TradeOn=0;
      TradeButton[loop].SetCheck(0);      
    } 
    ChartData.Array[loop]->UToggleTradeOff=1;      

    for (int loop2=ChartStartBar;loop2<=ChartStopBar;loop2++)
    {    
      if ((!ChartData.Array[loop]->TradeOn)&&(ChartData.Array[loop]->Last[loop2]>0.0))
      {
        ChartData.Array[loop]->TradeOn=1;
        TradeButton[loop].SetCheck(1);
        ChartData.Array[loop]->UToggleTradeOff=0;      
      }
    }
  }
  ::LeaveCriticalSection( &DisplaySection );
  RepositionChart(0,0);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedBidsoff()
{
  ::EnterCriticalSection( &DisplaySection );
  for (int loop=1;loop<MAXEXCHANGES;loop++)
  {
    if (ChartData.Array[loop]->BidOn)
    {
      ChartData.Array[loop]->BidOn=0;
      BidButton[loop].SetCheck(0);      
    }
    ChartData.Array[loop]->UToggleBidOff=1;      
  }
  ::LeaveCriticalSection( &DisplaySection );
  RepositionChart(0,0);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedBidson()
{
  ::EnterCriticalSection( &DisplaySection );
  for (int loop=1;loop<MAXEXCHANGES;loop++)
  {
    if ((!ChartData.Array[loop]->BidOn)&&(ChartData.Array[loop]->UToggleBidOff))
    {
      ChartData.Array[loop]->BidOn=1;
      BidButton[loop].SetCheck(1);      
    }
    ChartData.Array[loop]->UToggleBidOff=0;      
  }
  ::LeaveCriticalSection( &DisplaySection );
  RepositionChart(0,0);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedBidsauto()
{ 
  ::EnterCriticalSection( &DisplaySection );
  for (int loop=1;loop<MAXEXCHANGES;loop++)
  {
    if (ChartData.Array[loop]->BidOn)
    {
      ChartData.Array[loop]->BidOn=0;
      BidButton[loop].SetCheck(0);      
    }  
    ChartData.Array[loop]->UToggleBidOff=1;      
    for (int loop2=ChartStartBar;loop2<=ChartStopBar;loop2++)
    {    
      if ((!ChartData.Array[loop]->BidOn)&&(ChartData.Array[loop]->BidLast[loop2]>0.0))
      {
        ChartData.Array[loop]->BidOn=1;
        BidButton[loop].SetCheck(1);
        ChartData.Array[loop]->UToggleBidOff=0;      
      }
    }
  }
  ::LeaveCriticalSection( &DisplaySection );
  RepositionChart(0,0);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedAsksoff()
{
  ::EnterCriticalSection( &DisplaySection );
  for (int loop=1;loop<MAXEXCHANGES;loop++)
  {
    if (ChartData.Array[loop]->AskOn)
    {
      ChartData.Array[loop]->AskOn=0;
      AskButton[loop].SetCheck(0);      
    }
    ChartData.Array[loop]->UToggleAskOff=1;      
  }
  ::LeaveCriticalSection( &DisplaySection );
  RepositionChart(0,0);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedAskson()
{
  ::EnterCriticalSection( &DisplaySection );
  for (int loop=1;loop<MAXEXCHANGES;loop++)
  {
    if ((!ChartData.Array[loop]->AskOn)&&(ChartData.Array[loop]->UToggleAskOff))
    {
        ChartData.Array[loop]->AskOn=1;
        AskButton[loop].SetCheck(1);        
    }
    ChartData.Array[loop]->UToggleAskOff=0;      
  }
  ::LeaveCriticalSection( &DisplaySection );
  RepositionChart(0,0);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedAsksauto()
{  
  ::EnterCriticalSection( &DisplaySection );
  for (int loop=1;loop<MAXEXCHANGES;loop++)
  {
    if (ChartData.Array[loop]->AskOn)
    {
      ChartData.Array[loop]->AskOn=0;
      AskButton[loop].SetCheck(0);      
    }  
    ChartData.Array[loop]->UToggleAskOff=1;      
    for (int loop2=ChartStartBar;loop2<=ChartStopBar;loop2++)
    {    
      if ((!ChartData.Array[loop]->AskOn)&&(ChartData.Array[loop]->AskLast[loop2]>0.0))
      {
        ChartData.Array[loop]->AskOn=1;
        AskButton[loop].SetCheck(1);
        ChartData.Array[loop]->UToggleAskOff=0;      
      }
    }
  }
  ::LeaveCriticalSection( &DisplaySection );
  RepositionChart(0,0);
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeCompwidth()
{
  CString String;

  CompWidthControl.GetWindowText(String);	           
  PointExpand2=atoi((char *)String.GetString());    

  if ((PointExpand2>6)||(PointExpand2<1))
  {
    if (PointExpand2>20) PointExpand2=20;
    if (PointExpand2<1) PointExpand2=1;

    char str[32];
    sprintf(str,"%d",PointExpand2);
    CompWidthControl.SetWindowTextA(str);
  }

  ::EnterCriticalSection( &DisplaySection );
 
  ExchangeMap[0].PenWidth=PointExpand2;
  SetColorMem(&ChartColors,0,ExchangeMap[0].LastColor,ExchangeMap[0].BidColor,ExchangeMap[0].AskColor,ExchangeMap[0].PenWidth);                                      
    
  ::LeaveCriticalSection( &DisplaySection );  

  if (UpdateChartRT)
    RefreshChart();
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnDeltaposCompwidthspin(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
  
  if (pNMUpDown->iDelta<0.0) 
  {
    ++PointExpand2;
    if (PointExpand2>20) PointExpand2=20;
  }
  else           
  {
    --PointExpand2;
     if (PointExpand2<1) PointExpand2=1;
  }

  char str[32];
  sprintf(str,"%d",PointExpand2);
  CompWidthControl.SetWindowTextA(str);

  ::EnterCriticalSection( &DisplaySection );
 
  ExchangeMap[0].PenWidth=PointExpand2;
  SetColorMem(&ChartColors,0,ExchangeMap[0].LastColor,ExchangeMap[0].BidColor,ExchangeMap[0].AskColor,ExchangeMap[0].PenWidth);                                      
    
  ::LeaveCriticalSection( &DisplaySection );  

  if (UpdateChartRT)
    RefreshChart();

  *pResult = 0;
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeOtherwidth()
{
  CString String;

  OtherWidthControl.GetWindowText(String);	           
  PointExpand=atoi((char *)String.GetString());    

  if ((PointExpand>6)||(PointExpand<1))
  {
    if (PointExpand>6) PointExpand=6;
    if (PointExpand<1) PointExpand=1;

    char str[32];
    sprintf(str,"%d",PointExpand);
    OtherWidthControl.SetWindowTextA(str);
  }

  if (UpdateChartRT)
    RefreshChart();
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnDeltaposOtherwidthspin(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
  
  if (pNMUpDown->iDelta<0.0) 
  {
    ++PointExpand;
    if (PointExpand>6) PointExpand=6;
  }
  else           
  {
    --PointExpand;
     if (PointExpand<1) PointExpand=1;
  }

  char str[32];
  sprintf(str,"%d",PointExpand);
  OtherWidthControl.SetWindowTextA(str);

  if (UpdateChartRT)
    RefreshChart();

  *pResult = 0;
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedCircuitstyle()
{
  ChartData.CircuitStyle=!ChartData.CircuitStyle;
  RefreshChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedScaletoprice()
{
  ChartData.ScaleTo=0;
  DoPriceRecalc=1;
  EnableHardScale(0);
  RefreshChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedScaletonbbo()
{
  ChartData.ScaleTo=1;
  DoPriceRecalc=1;
  EnableHardScale(0);
  RefreshChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedScaletoall()
{
  ChartData.ScaleTo=2;
  DoPriceRecalc=1;
  EnableHardScale(0);
  RefreshChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedScaletohard()
{
  ChartData.ScaleTo=3;
  EnableHardScale(1);  
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::EnableHardScale(unsigned char Enable)
{
  HardPriceMaxControl.EnableWindow(Enable);
  HardPriceMinControl.EnableWindow(Enable);
  HardPriceScaleControl.EnableWindow(Enable);
  ::EnableWindow(GetDlgItem(IDC_HARDLABEL1)->m_hWnd,Enable);
  ::EnableWindow(GetDlgItem(IDC_HARDLABEL2)->m_hWnd,Enable);
  ::EnableWindow(GetDlgItem(IDC_HARDLABEL3)->m_hWnd,Enable);
  if (Enable)
  {
    char str1[16],str2[16],str3[16];
    sprintf(str1,"%0.2f",Chart.MaxBarHeight);
    ChartData.HardPriceMax=Chart.MaxBarHeight;    

    sprintf(str2,"%0.2f",Chart.MinBarHeight);
    ChartData.HardPriceMin=Chart.MinBarHeight;    

    sprintf(str3,"%0.2f",Chart.IncAmount);
    ChartData.HardPriceScale=Chart.IncAmount;

    HardPriceMaxControl.SetWindowTextA(str1);	  
    HardPriceMinControl.SetWindowTextA(str2);	  
    HardPriceScaleControl.SetWindowTextA(str3);	  
  }
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeHardmax()
{
  char str[16];
  HardPriceMaxControl.GetWindowText(str,16);	        
  if (strlen(str)>0) ChartData.HardPriceMax=atof(str);                       
  else               ChartData.HardPriceMax=1.0;
  DoPriceRecalc=0;
  Chart.FindMaxBar(&ChartData);
  RefreshChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeHardmin()
{
  char str[16];
  HardPriceMinControl.GetWindowText(str,16);	        
  if (strlen(str)>0) ChartData.HardPriceMin=atof(str);                       
  else               ChartData.HardPriceMin=0.0;
  DoPriceRecalc=0;
  Chart.FindMaxBar(&ChartData);
  RefreshChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeHardscale()
{
  char str[16];
  HardPriceScaleControl.GetWindowText(str,16);	        
  if (strlen(str)>0) ChartData.HardPriceScale=atof(str);                       
  else               ChartData.HardPriceScale=0.0;
  RefreshChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedScalevolauto()
{
  EnableHardVolScale(0);
  ChartData.VolScaleTo=0;
  DoVolRecalc=1;
  RefreshChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedScalevolhard()
{
  DoVolRecalc=0;
  EnableHardVolScale(1);
  ChartData.VolScaleTo=1;
  RefreshChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::EnableHardVolScale(unsigned char Enable)
{
  VolumeHardMaxControl.EnableWindow(Enable);  
  VolumeHardScaleControl.EnableWindow(Enable);
  ::EnableWindow(GetDlgItem(IDC_HARDLABEL4)->m_hWnd,Enable);
  ::EnableWindow(GetDlgItem(IDC_HARDLABEL5)->m_hWnd,Enable);
  if (Enable)
  {
    char str1[16],str2[16];
    sprintf(str1,"%0.0f",VolChart.MaxBarHeight);
    ChartData.HardVolScale=Chart.MaxBarHeight;    

    sprintf(str2,"%0.0f",VolChart.IncAmount);
    ChartData.HardVolScale=(unsigned long)VolChart.IncAmount;

    VolumeHardMaxControl.SetWindowTextA(str1);	  
    VolumeHardScaleControl.SetWindowTextA(str2);	  
  }
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeVolhardmax()
{
  char str[16];
  VolumeHardMaxControl.GetWindowText(str,16);	        
  if (strlen(str)>0) ChartData.HardVolMax=atof(str);                       
  else               ChartData.HardVolMax=1.0;
  DoVolRecalc=0;
  VolChart.FindMaxBar(&ChartData);
  RefreshChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeVolhardscale()
{
  char str[16];
  VolumeHardScaleControl.GetWindowText(str,16);	        
  if (strlen(str)>0) ChartData.HardVolScale=atof(str);                       
  else               ChartData.HardVolScale=1.0;
  DoVolRecalc=0;  
  RefreshChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedUsebboband()
{
  ChartData.UseBBOBands=!ChartData.UseBBOBands;
  if (ChartData.UseBBOBands)
  {  
    BandingControl.EnableWindow(1);
    ::EnableWindow(GetDlgItem(IDC_BANDINGLABEL)->m_hWnd,1);
    if (ChartData.ScaleTo!=1)
    {
      switch(ChartData.ScaleTo)
      {
        case 0:CheckDlgButton(IDC_SCALETOPRICE,(BOOL)0);break;
        case 2:CheckDlgButton(IDC_SCALETOALL,(BOOL)0);break;
        case 3:CheckDlgButton(IDC_SCALETOHARD,(BOOL)0);
               EnableHardScale(0);
               break;
      }
      ChartData.ScaleTo=1;
      CheckDlgButton(IDC_SCALETONBBO,(BOOL)1);
    }
  }
  else
  {
    BandingControl.EnableWindow(0);
    ::EnableWindow(GetDlgItem(IDC_BANDINGLABEL)->m_hWnd,0);
  }

  DoPriceRecalc=1;
  RefreshChart();
  RefreshLegendChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeBandpercent()
{
  char str[16];
  double Band=10.0;
  BandingControl.GetWindowText(str,16);	        
  if (strlen(str)>0) Band=atof(str);                       
  ChartData.BBOBandAmount=(Band/100.0);
  
  RefreshChart();
  RefreshLegendChart();
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedRoundlotadj()
{
  RoundLotAdj=!RoundLotAdj;
}
//---------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedNoautotrade()
{
  AutoTradesOn=!AutoTradesOn;
}
//---------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedLegendon()
{
  ShowLegend=!ShowLegend;
  RepositionChart(0,0);
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnDtnDatetimechangeStarttime(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

    CTime Start,Stop;
    StartTimeControl.GetTime(Start);
    unsigned long TempStopTime;
   
    StartHour=Start.GetHour();
    StartMin=Start.GetMinute();
    StartSec=Start.GetSecond();

    StartTime=((StartHour*3600)+(StartMin*60)+StartSec)*1000;   
    StopTime=((StartHour*3600)+((StartMin+1)*60)+StartSec);
    
    StopHour=StopTime / 3600;
    StopMin=(StopTime-(StopHour*3600)) / 60;
    StopSec=StopTime-((StopHour*3600)+(StopMin*60));

    time_t ltime;
    time( &ltime ); 
    _localtime64_s( &TodaysDate, &ltime ); 
	Stop=CTime(TodaysDate.tm_year+1900,TodaysDate.tm_mon+1,TodaysDate.tm_mday,StopHour,StopMin,StopSec,-1);		
	StopTimeControl.SetTime(&Stop);

    StopTime=((StopHour*3600)+(StopMin*60)+StopSec)*1000;

    *pResult = 0;
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::OnDtnDatetimechangeStoptime(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

    CTime Start,Stop;    
    StopTimeControl.GetTime(Stop);  
    
    StopHour=Stop.GetHour();
    StopMin=Stop.GetMinute();
    StopSec=Stop.GetSecond();

    StopTime=((StopHour*3600)+(StopMin*60)+StopSec)*1000;
  
    *pResult = 0;
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedRevtime()
{
  ShowReverseTime=!ShowReverseTime;
  RefreshChart();
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedChartrefresh()
{
  RefreshChart();
  RefreshLegendChart();
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeNumdatapoints()
{
  if (DataPointForceSet)
  {
    DataPointForceSet=0;
    return;
  }
  char str[16];
  DataPointsControl.GetWindowText(str,16);	        
  ViewDataPoints=atoi(str);                       
  if (ViewDataPoints>ChartStopBar) ViewDataPoints=ChartStopBar;
  ViewStartBar=ChartStartBar;
  ViewStopBar=ViewStartBar+ViewDataPoints;
  DoPriceRecalc=1;
  DoVolRecalc=1;  
  DoCountReCalc=1;

  Chart.SetStartStopBars(ViewStartBar,ViewStopBar);
  VolChart.SetStartStopBars(ViewStartBar,ViewStopBar);

  if (UpdateChartRT)
    RefreshChart();
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnDeltaposDatapointspin(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
  
  if (pNMUpDown->iDelta<0.0) 
  {
    ViewDataPoints+=100;
    if (ViewDataPoints>ChartStopBar) ViewDataPoints=ChartStopBar;
  }
  else           
  {
    ViewDataPoints-=100;
     if (ViewDataPoints<100) ViewDataPoints=100;
  }

  char str[32];
  sprintf(str,"%d",ViewDataPoints);
  DataPointsControl.SetWindowTextA(str);

  ViewStartBar=ChartStartBar;
  ViewStopBar=ViewStartBar+ViewDataPoints;
  DoPriceRecalc=1;
  DoVolRecalc=1;  
  Chart.SetStartStopBars(ViewStartBar,ViewStopBar);
  VolChart.SetStartStopBars(ViewStartBar,ViewStopBar);

  if (UpdateChartRT)
    RefreshChart();

  *pResult = 0;
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeStartpoint()
{
  if (DataPointForceSet)
  {
    DataPointForceSet=0;
    return;
  }
  char str[16];
  StartPointControl.GetWindowText(str,16);	        
  if (strlen(str)>0) ViewStartBar=atoi(str);                       
  else               ViewStartBar=ChartStartBar;
  if (ViewStartBar>ChartStopBar) ViewStartBar=ChartStopBar-1;
  if (ViewStartBar<ChartStartBar) ViewStartBar=ChartStartBar;

  ViewStopBar=ViewStartBar+ViewDataPoints;

  if (ViewStopBar>ChartStopBar)
  {
    ViewStopBar=ChartStopBar;
    ViewStartBar=ViewStopBar-ViewDataPoints+1;
    if (ViewStartBar<ChartStartBar) ViewStartBar=ChartStartBar;
  }

  DoPriceRecalc=1;
  DoVolRecalc=1;  
  Chart.SetStartStopBars(ViewStartBar,ViewStopBar);
  VolChart.SetStartStopBars(ViewStartBar,ViewStopBar);

  if (UpdateChartRT)
    RefreshChart();
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnDeltaposStartpointspin(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
  
  if (pNMUpDown->iDelta<0.0) 
  {
    ViewStartBar+=100;
    if (ViewStartBar>ChartStopBar) ViewStartBar=ChartStopBar-1;
  }
  else           
  {
    ViewStartBar-=100;
     if (ViewStartBar<ChartStartBar) ViewStartBar=ChartStartBar;
  }

  char str[32];
  sprintf(str,"%d",ViewStartBar);
  StartPointControl.SetWindowTextA(str);

  ViewStopBar=ViewStartBar+ViewDataPoints;

  if (ViewStopBar>ChartStopBar)
  {
    ViewStopBar=ChartStopBar;
    ViewStartBar=ViewStopBar-ViewDataPoints+1;
    if (ViewStartBar<ChartStartBar) ViewStartBar=ChartStartBar;
  }

  DoPriceRecalc=1;
  DoVolRecalc=1;  
  Chart.SetStartStopBars(ViewStartBar,ViewStopBar);
  VolChart.SetStartStopBars(ViewStartBar,ViewStopBar);

  if (UpdateChartRT)
    RefreshChart();

  *pResult = 0;
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedExcludetrades()
{
  ExcludeQTTrades=!ExcludeQTTrades;
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedExcludequotes()
{
  ExcludeQTQuotes=!ExcludeQTQuotes;
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnEnChangeRexchangefilter()
{
   RExgControl.GetWindowText(RExgFilterStr,512);	       
   if (!strcmp(RExgFilterStr,"0")) RExgFilterStr[0]=0;
}
//---------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedShowiso()
{
  ShowISO=!ShowISO;
  RefreshChart();
  RefreshLegendChart();
}
//---------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedSplitsizechart()
{
  SplitSizeChart=!SplitSizeChart;
  VolChart.SetStudyType(SplitSizeChart);
  DoVolRecalc=1;

  if (!SplitSizeChart)
  {
    strcpy(VolStr1,"Size");
    strcpy(VolStr2,"Size");               
  }
  else
  {
    strcpy(VolStr1,"QSize Splt");
    strcpy(VolStr2,"QSze");               
  }

  VolLenOffset1=(int)(strlen(VolStr1)*Chart.FontHeight/2);
  VolLenOffset2=(int)(strlen(VolStr2)*Chart.FontHeight/2);

  RefreshChart();
}


/////////////////////
// MOUSE FUNCTIONS //
/////////////////////

//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnLButtonDown(UINT nFlags,CPoint point)
{
    int x1=Chart.ChartingX;
    int x2=Chart.ChartingX+Chart.ChartingWidth+16;
    int y1=Chart.ChartingY+PriceY1;
    int y2=y1+Chart.ChartingHeight+16;
    if ((point.x>=x1)&&(point.x<=x2)&&(point.y>=y1)&&(point.y<=y2))
    {
      LastMouseX=point.x;
      MouseGrabbed=1;
    }
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnLButtonUp(UINT nFlags,CPoint point)
{
    MouseGrabbed=0;
	ClientToScreen(&point);
}
//-------------------------------------------------------------
LRESULT CJTools_ShellAppDlg::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
  MouseGrabbed=0;
  return 1;
}
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnMouseMove(UINT nFlags,CPoint point)
{
  TRACKMOUSEEVENT tme;
  tme.cbSize = sizeof(tme);
  tme.hwndTrack = this->m_hWnd;
  tme.dwFlags = TME_LEAVE;
  tme.dwHoverTime=1;
  TrackMouseEvent(&tme);

  if ((MouseGrabbed)&&(CurrChartBar>0)&&(ThisMouseX!=point.x))
  {
    ThisMouseX=point.x;

    int x1=Chart.ChartingX;
    int x2=Chart.ChartingX+Chart.ChartingWidth+16;
    int y1=Chart.ChartingY+PriceY1;
    int y2=y1+Chart.ChartingHeight+16;

    if ((point.x>=x1)&&(point.x<=x2)&&(point.y>=y1)&&(point.y<=y2))
    {   
      ::EnterCriticalSection( &DisplaySection );
      ViewStopBar+=(LastMouseX-ThisMouseX);
      ViewStartBar+=(LastMouseX-ThisMouseX);

      if (ViewStopBar<0) ViewStopBar=0;
      else if (ViewStopBar>ChartStopBar) 
      {
        ViewStopBar=ChartStopBar;
        ViewStartBar=ViewStopBar-ViewDataPoints;
      }
      if (ViewStartBar<ChartStartBar) 
      {
        ViewStartBar=ChartStartBar;
        ViewStopBar=ViewStartBar+ViewDataPoints;
      }
          
      Chart.SetStartStopBars(ViewStartBar,ViewStopBar);
      VolChart.SetStartStopBars(ViewStartBar,ViewStopBar);
      DoPriceRecalc=DoVolRecalc=DoCountReCalc=1;

      ::LeaveCriticalSection( &DisplaySection );
      
      RefreshChart();
      RefreshLegendChart();
    }
    else
      MouseGrabbed=0;

    LastMouseX=ThisMouseX;
  }
}


/////////////////////////
// NXCORE FEED UPDATES //
/////////////////////////

//------------------------------------------------------------
void CJTools_ShellAppDlg::BreakTrade(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{            
   if (ExcludeQTTrades) return;
   if ((RNumExchanges)&&(!RExchangeToggles[pNxCoreMsg->coreHeader.ReportingExg])) return;

   LExg=pNxCoreMsg->coreHeader.ListedExg;   
   RExg=pNxCoreMsg->coreHeader.ReportingExg;      
   TradePriceType=pNxCoreMsg->coreData.Trade.PriceType;
        
   if (((!(pNxCoreMsg->coreData.Trade.ConditionFlags & NxTCF_NOLAST))||
       (pNxCoreMsg->coreData.Trade.TradeCondition==1))&&(pNxCoreMsg->coreData.Trade.Price>0))
   {              
     unsigned char NewEnabled=0;
     
     if (pNxCoreMsg->coreHeader.nxExgTimestamp.MsOfDay<StartTime) return;
     if ((pNxCoreMsg->coreHeader.nxExgTimestamp.MsOfDay>=StopTime)||(CurrChartBar+1>=MAXBARS))
     {       
       InterestWait=1;       
       if (CurrChartBar+1>=MAXBARS) MaxBarError=1;
       pView->InterestStatusControl.SetWindowTextA("Calculating Chart");                          

       --CurrChartBar;
       ChartStopBar=CurrChartBar;
       ViewStartBar=ChartStartBar;
       ViewStopBar=ChartStopBar;
       ViewDataPoints=ViewStopBar-ViewStartBar+1;

       Chart.SetStartStopBars(ViewStartBar,ViewStopBar);
       VolChart.SetStartStopBars(ViewStartBar,ViewStopBar);
       DoPriceRecalc=DoVolRecalc=1;

       OnBnClickedTimestep();
       CheckDlgButton(IDC_TIMEFULL,0);
	   CheckDlgButton(IDC_TIMESIM,0);
	   CheckDlgButton(IDC_TIMESTEP,1); 

       DoCountReCalc=1;
       RefreshChart();       
       RefreshLegendChart();

       char str[32];
       DataPointForceSet=1;
       sprintf(str,"%d",ViewDataPoints);
       DataPointsControl.SetWindowTextA(str);

       DataPointForceSet=1;
       sprintf(str,"%d",ViewStartBar);
       StartPointControl.SetWindowTextA(str);     

       if (!MaxBarError)
         pView->InterestStatusControl.SetWindowTextA("Complete");                          
       else
         pView->InterestStatusControl.SetWindowTextA("Complete - More Data Exists");                          
       return;
     }

     if (CurrChartBar==0)
       pView->InterestStatusControl.SetWindowTextA("Retrieving Data");                          
     
     int Bar=CurrChartBar;
     ++CurrChartBar;
     
     ChartData.Hour[Bar]  =pNxCoreMsg->coreHeader.nxExgTimestamp.Hour;
     ChartData.Min[Bar]   =pNxCoreMsg->coreHeader.nxExgTimestamp.Minute;
     ChartData.Sec[Bar]   =pNxCoreMsg->coreHeader.nxExgTimestamp.Second;
     ChartData.MSec[Bar]  =pNxCoreMsg->coreHeader.nxExgTimestamp.Millisecond;
     ChartData.MSTOD[Bar] =pNxCoreSys->nxTime.MsOfDay;

     // First do the composite prices     
     ChartData.Array[0]->Last[Bar]=pfNxCorePriceToDouble(pNxCoreMsg->coreData.Trade.Price,TradePriceType);;
     ChartData.Array[0]->Volume[Bar]+=pNxCoreMsg->coreData.Trade.Size;
     ChartData.Array[0]->TCondition[Bar]=pNxCoreMsg->coreData.Trade.TradeCondition;

     if (UpdateChartRT)
     {
       if ((ChartData.ScaleTo<3)&&(ChartData.Array[0]->Last[Bar] > Chart.MaxBarHeight)) DoPriceRecalc=1;
       else if ((pView->ChartData.ScaleTo<3)&&(ChartData.Array[0]->Last[Bar] < Chart.MinBarHeight)) DoPriceRecalc=1;

       if (ChartData.VolScaleTo<1)
       {
         if (ChartData.Array[0]->Volume[Bar] > VolChart.MaxBarHeight) DoVolRecalc=1;
       }
     }

     if ((!ChartData.Array[0]->TradeOn)&&(!ChartData.Array[0]->UToggleTradeOff))
     {
        EnableTradeExchange(0);

        if (!AutoTradesOn)
          ChartData.Array[0]->TradeOn=1;
        else
          ChartData.Array[0]->UToggleTradeOff=1;
        NewEnabled=1;
     }

     // Next do the exchange specific prices;
     int MapIndex=ReverseExchangeMap[pNxCoreMsg->coreHeader.ReportingExg];
     if (MapIndex<=0) return;

     if ((!ChartData.Array[MapIndex]->TradeOn)&&(!ChartData.Array[MapIndex]->UToggleTradeOff))
     {
        EnableTradeExchange(MapIndex);
        if (AutoTradesOn)
          ChartData.Array[MapIndex]->TradeOn=1;
        else
          ChartData.Array[MapIndex]->UToggleTradeOff=1;
        NewEnabled=1;
     }

     ChartData.Array[MapIndex]->Last[Bar]=ChartData.Array[0]->Last[Bar];     
     ChartData.Array[MapIndex]->Volume[Bar]+=pNxCoreMsg->coreData.Trade.Size;
     ChartData.Array[MapIndex]->TCondition[Bar]=pNxCoreMsg->coreData.Trade.TradeCondition;
     
     if (UpdateChartRT)
     {
       if ((ChartData.ScaleTo<3)&&(ChartData.Array[MapIndex]->Last[Bar] > Chart.MaxBarHeight)) DoPriceRecalc=1;
       else if ((ChartData.ScaleTo<3)&&(ChartData.Array[MapIndex]->Last[Bar] < Chart.MinBarHeight)) DoPriceRecalc=1;
       if ((ChartData.VolScaleTo<1)&&(ChartData.Array[MapIndex]->Volume[Bar] > VolChart.MaxBarHeight)) DoVolRecalc=1;
     }

     if (NewEnabled) 
     {
       LegendChart.FindMaxBar(&ChartData);
       RepositionChart(0,0);    
     }

     if (UpdateChartRT)
     {      
       ChartStopBar=CurrChartBar;
       ViewStartBar=ChartStartBar;
       ViewStopBar=ChartStopBar;
       ViewDataPoints=ViewStopBar-ViewStartBar+1;

       Chart.SetStartStopBars(ViewStartBar,ViewStopBar);
       VolChart.SetStartStopBars(ViewStartBar,ViewStopBar);
       DoPriceRecalc=DoVolRecalc=DoCountReCalc=1;
      
       RefreshChart();
       RefreshLegendChart();
     }
   }
}
//------------------------------------------------------------
void CJTools_ShellAppDlg::BreakQuote(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{              
   if (ExcludeQTQuotes) return;
   if ((RNumExchanges)&&(!RExchangeToggles[pNxCoreMsg->coreHeader.ReportingExg])) return;

   
   LExg=pNxCoreMsg->coreHeader.ListedExg;   
   RExg=pNxCoreMsg->coreHeader.ReportingExg;      
   QuotePriceType=pNxCoreMsg->coreData.ExgQuote.coreQuote.PriceType;
        
   if (pNxCoreMsg->coreData.ExgQuote.coreQuote.QuoteCondition==0)
   { 
     if (pNxCoreMsg->coreHeader.nxExgTimestamp.MsOfDay<StartTime) 
     {
       return;
     }
     
     if ((pNxCoreMsg->coreHeader.nxExgTimestamp.MsOfDay>=StopTime)||(CurrChartBar+1>=MAXBARS))
     {       
       InterestWait=1;       
       if (CurrChartBar+1>=MAXBARS) MaxBarError=1;
       pView->InterestStatusControl.SetWindowTextA("Calculating Chart");                          

       --CurrChartBar;
       ChartStopBar=CurrChartBar;
       ViewStartBar=ChartStartBar;
       ViewStopBar=ChartStopBar;
       ViewDataPoints=ViewStopBar-ViewStartBar+1;

       Chart.SetStartStopBars(ViewStartBar,ViewStopBar);
       VolChart.SetStartStopBars(ViewStartBar,ViewStopBar);
       DoPriceRecalc=DoVolRecalc=1;
       
       OnBnClickedTimestep();       
       CheckDlgButton(IDC_TIMEFULL,0);
	   CheckDlgButton(IDC_TIMESIM,0);
	   CheckDlgButton(IDC_TIMESTEP,1); 

       DoCountReCalc=1;
       RefreshChart();
       RefreshLegendChart();
       
       char str[32];
       DataPointForceSet=1;
       sprintf(str,"%d",ViewDataPoints);
       DataPointsControl.SetWindowTextA(str);

       DataPointForceSet=1;
       sprintf(str,"%d",ViewStartBar);
       StartPointControl.SetWindowTextA(str);

       if (!MaxBarError)
         pView->InterestStatusControl.SetWindowTextA("Complete");                          
       else
         pView->InterestStatusControl.SetWindowTextA("Complete - More Data Exists");                          
       return;
     }

     if (CurrChartBar==0)
       pView->InterestStatusControl.SetWindowTextA("Retrieving Data");                          
          
     unsigned char NewEnabled=0;

     if (CurrChartBar+1>=MAXBARS) return;     
     int Bar=CurrChartBar;
     ++CurrChartBar;
     
     ChartData.Hour[Bar]  =pNxCoreMsg->coreHeader.nxExgTimestamp.Hour;
     ChartData.Min[Bar]   =pNxCoreMsg->coreHeader.nxExgTimestamp.Minute;
     ChartData.Sec[Bar]   =pNxCoreMsg->coreHeader.nxExgTimestamp.Second;
     ChartData.MSec[Bar]  =pNxCoreMsg->coreHeader.nxExgTimestamp.Millisecond;
     ChartData.MSTOD[Bar] =pNxCoreSys->nxTime.MsOfDay;

     // First do the composite prices     
     if (pNxCoreMsg->coreData.ExgQuote.BestBidPrice>0)
     {
       ChartData.Array[0]->BidLast[Bar]=pfNxCorePriceToDouble(pNxCoreMsg->coreData.ExgQuote.BestBidPrice,QuotePriceType);;
       ChartData.BBOBidBand[Bar]=ChartData.Array[0]->BidLast[Bar]-(ChartData.Array[0]->BidLast[Bar]*ChartData.BBOBandAmount);
       if ((UpdateChartRT)&&(ChartData.ScaleTo<3)&&(ChartData.Array[0]->BidLast[Bar] > Chart.MaxBarHeight)) DoPriceRecalc=1;
       else if ((UpdateChartRT)&&(ChartData.ScaleTo<3)&&(ChartData.Array[0]->BidLast[Bar] < Chart.MinBarHeight)) DoPriceRecalc=1;

       if (RoundLotAdj)
          ChartData.Array[0]->BidVolume[Bar]=pNxCoreMsg->coreData.ExgQuote.BestBidSize*100;
       else
          ChartData.Array[0]->BidVolume[Bar]=pNxCoreMsg->coreData.ExgQuote.BestBidSize;

       if ((!ChartData.Array[0]->BidOn)&&(!ChartData.Array[0]->UToggleBidOff))
       {
         EnableBidExchange(0);
         ChartData.Array[0]->BidOn=1;
         NewEnabled=1;
       }
     }
     if (pNxCoreMsg->coreData.ExgQuote.BestAskPrice>0)
     {
       ChartData.Array[0]->AskLast[Bar]=pfNxCorePriceToDouble(pNxCoreMsg->coreData.ExgQuote.BestAskPrice,QuotePriceType);;
       ChartData.BBOAskBand[Bar]=ChartData.Array[0]->AskLast[Bar]+(ChartData.Array[0]->AskLast[Bar]*ChartData.BBOBandAmount);
       if ((UpdateChartRT)&&(ChartData.ScaleTo<3)&&(ChartData.Array[0]->AskLast[Bar] > Chart.MaxBarHeight)) DoPriceRecalc=1;
       else if (((UpdateChartRT)&&(ChartData.ScaleTo<3))&&(ChartData.Array[0]->AskLast[Bar] < Chart.MinBarHeight)) DoPriceRecalc=1;

       if (RoundLotAdj)
          ChartData.Array[0]->AskVolume[Bar]=pNxCoreMsg->coreData.ExgQuote.BestAskSize*100;
       else
          ChartData.Array[0]->AskVolume[Bar]=pNxCoreMsg->coreData.ExgQuote.BestAskSize;

       if ((!ChartData.Array[0]->AskOn)&&(!ChartData.Array[0]->UToggleAskOff))
       {
          EnableAskExchange(0);
          ChartData.Array[0]->AskOn=1;
          NewEnabled=1;
       }
     }
     
          
     // Next do the exchange specific prices;
     int MapIndex=ReverseExchangeMap[pNxCoreMsg->coreHeader.ReportingExg];
     if (MapIndex<=0) return;
     
     if (pNxCoreMsg->coreData.ExgQuote.coreQuote.BidPrice>0)
     {
       ChartData.Array[MapIndex]->BidLast[Bar]=pfNxCorePriceToDouble(pNxCoreMsg->coreData.ExgQuote.coreQuote.BidPrice,QuotePriceType);;
       if ((UpdateChartRT)&&(ChartData.ScaleTo<3)&&(ChartData.Array[MapIndex]->BidLast[Bar] > Chart.MaxBarHeight)) DoPriceRecalc=1;
       else if ((UpdateChartRT)&&(ChartData.ScaleTo<3)&&(ChartData.Array[MapIndex]->BidLast[Bar] < Chart.MinBarHeight)) DoPriceRecalc=1;
       if ((!ChartData.Array[MapIndex]->BidOn)&&(!ChartData.Array[MapIndex]->UToggleBidOff))
       {
          EnableBidExchange(MapIndex);
          ChartData.Array[MapIndex]->BidOn=1;
          NewEnabled=1;
       }
       if (pNxCoreMsg->coreData.ExgQuote.coreQuote.BidSize>ChartData.Array[MapIndex]->BidVolume[Bar])
       {
         if (RoundLotAdj)
           ChartData.Array[MapIndex]->BidVolume[Bar]=pNxCoreMsg->coreData.ExgQuote.coreQuote.BidSize*100;
         else
           ChartData.Array[MapIndex]->BidVolume[Bar]=pNxCoreMsg->coreData.ExgQuote.coreQuote.BidSize;
       }
     }
     
     if (pNxCoreMsg->coreData.ExgQuote.coreQuote.AskPrice>0)
     {
       ChartData.Array[MapIndex]->AskLast[Bar]=pfNxCorePriceToDouble(pNxCoreMsg->coreData.ExgQuote.coreQuote.AskPrice,QuotePriceType);;
       if ((UpdateChartRT)&&(ChartData.ScaleTo<3)&&(ChartData.Array[MapIndex]->AskLast[Bar] > Chart.MaxBarHeight)) DoPriceRecalc=1;
       else if ((UpdateChartRT)&&(ChartData.ScaleTo<3)&&(ChartData.Array[MapIndex]->AskLast[Bar] < Chart.MinBarHeight)) DoPriceRecalc=1;     
       if ((!ChartData.Array[MapIndex]->AskOn)&&(!ChartData.Array[MapIndex]->UToggleAskOff))
       {
          EnableAskExchange(MapIndex);
          ChartData.Array[MapIndex]->AskOn=1;
          NewEnabled=1;
       }
       if (pNxCoreMsg->coreData.ExgQuote.coreQuote.AskSize>ChartData.Array[MapIndex]->AskVolume[Bar])
       {
         if (RoundLotAdj)
           ChartData.Array[MapIndex]->AskVolume[Bar]=pNxCoreMsg->coreData.ExgQuote.coreQuote.AskSize*100;
         else
           ChartData.Array[MapIndex]->AskVolume[Bar]=pNxCoreMsg->coreData.ExgQuote.coreQuote.AskSize;
       }
     }
     

     if (NewEnabled) 
     {
       LegendChart.FindMaxBar(&ChartData);
       RepositionChart(0,0);
     }
     if (UpdateChartRT)
     {       
       ChartStopBar=CurrChartBar;
       ViewStartBar=ChartStartBar;
       ViewStopBar=ChartStopBar;
       ViewDataPoints=ViewStopBar-ViewStartBar+1;

       Chart.SetStartStopBars(ViewStartBar,ViewStopBar);
       VolChart.SetStartStopBars(ViewStartBar,ViewStopBar);
       DoPriceRecalc=DoVolRecalc=DoCountReCalc=1;
      
       RefreshChart();
       RefreshLegendChart();
     }
   }
}

//---------------------------------------------
void CJTools_ShellAppDlg::CheckNXTime(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{
     if (InterestWait) return;

     if ((pNxCoreSys->nxTime.MsOfDay>60000)&&((pNxCoreSys->nxTime.MsOfDay-60000>=StopTime)||(CurrChartBar+1>=MAXBARS)))
     {       
       InterestWait=1;       
       if (CurrChartBar+1>=MAXBARS) MaxBarError=1;
       pView->InterestStatusControl.SetWindowTextA("Calculating Chart");                          

       --CurrChartBar;
       ChartStopBar=CurrChartBar;
       ViewStartBar=ChartStartBar;
       ViewStopBar=ChartStopBar;
       ViewDataPoints=ViewStopBar-ViewStartBar+1;

       Chart.SetStartStopBars(ViewStartBar,ViewStopBar);
       VolChart.SetStartStopBars(ViewStartBar,ViewStopBar);
       DoPriceRecalc=DoVolRecalc=1;
       
       OnBnClickedTimestep();       
       CheckDlgButton(IDC_TIMEFULL,0);
	   CheckDlgButton(IDC_TIMESIM,0);
	   CheckDlgButton(IDC_TIMESTEP,1); 

       DoCountReCalc=1;
       RefreshChart();
       RefreshLegendChart();
       
       char str[32];
       DataPointForceSet=1;
       sprintf(str,"%d",ViewDataPoints);
       DataPointsControl.SetWindowTextA(str);

       DataPointForceSet=1;
       sprintf(str,"%d",ViewStartBar);
       StartPointControl.SetWindowTextA(str);

       if (!MaxBarError)
         pView->InterestStatusControl.SetWindowTextA("Complete");                          
       else
         pView->InterestStatusControl.SetWindowTextA("Complete - More Data Exists");                          
     }
     
}

//////////////////
// SYSTEM CALLS //
//////////////////

// Color the view window components
//-------------------------------------------------------------
HBRUSH CJTools_ShellAppDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{	  
	switch (nCtlColor) {
	case CTLCOLOR_EDIT:	
         SetTextColor(pDC->m_hDC,WinFG3);
	     SetBkColor( pDC->m_hDC,WinBG3 );
	     return (HBRUSH)(WinBrush3.GetSafeHandle());	          	     

	case CTLCOLOR_STATIC:			 
		 if ((pWnd==&SysStatusControl)||
             (pWnd==&SymbolControl)||             
             (pWnd==&InterestStatusControl))
         {
		     SetTextColor(pDC->m_hDC,WinFG1);
		     SetBkColor( pDC->m_hDC,WinBG1 );
		     return (HBRUSH)(WinBrush1.GetSafeHandle());	
         }
         else
         {
           SetTextColor(pDC->m_hDC,TextColor);
		   SetBkColor( pDC->m_hDC,BackgroundColor );
           return (HBRUSH)(BackgroundBrush.GetSafeHandle());	          	     
         }
         // Let remaining statics fall through to next case clause..._DLG handler
		
	case CTLCOLOR_DLG:
	case CTLCOLOR_BTN:    
	     SetTextColor(pDC->m_hDC,TextColor);
	     SetBkColor( pDC->m_hDC,BackgroundColor );         
         return (HBRUSH)(BackgroundBrush.GetSafeHandle());

    }
    return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);    
}

// Window painting - Called by framework
//---------------------------------------
void CJTools_ShellAppDlg::OnPaint()
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
		CDialog::OnPaint();

        SetTimer(4,100,NULL);                 
	}
}
// OnSize
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnSize(UINT nType,int cx,int cy)
{      
    //CDialog::OnSize(nType,cx,cy);
    if (nType==SIZE_MINIMIZED)
    {
      ViewIsMinimized=1;    
      return;
    }
  
    ViewIsMinimized=0;
    if (ViewInitiated)
    {
      RECT Rect;
      SysStatusControl.GetClientRect(&Rect);
      SysStatusControl.SetWindowPos(NULL,cx/2-(Rect.right-Rect.left)/2,cy-DPIpx(25),0,0,SWP_NOSIZE);
      RepositionChart(cx,cy);
    }    
}
// OnQueryDragIcon
//-------------------------------------------------------------
HCURSOR CJTools_ShellAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//////////////////////////////
// PREFERENCES AND SETTINGS //
//////////////////////////////

// Call the preferences dialog 
//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnBnClickedSettings()
{
	if (!NxCoreIsRunning)
	{
	    JT_StockPreferences SettingsDLG;

	    strcpy(SettingsDLG.TapeFileName,TapeFileName);	
		strcpy(SettingsDLG.DLLFilename,DLLFilename);	
		
	    SettingsDLG.RunningMode=FWXNxCoreMode;	    
		SettingsDLG.StartCoreOnStartup=StartCoreOnStartup;
        SettingsDLG.RTSimPause=RTSimDelay;

		SettingsDLG.WinBG1=BackgroundColor;
	    SettingsDLG.WinFG1=TextColor;			
		SettingsDLG.WinBG2=WinBG2;
	    SettingsDLG.WinFG2=WinFG2;
		SettingsDLG.WinBG3=WinBG1;
	    SettingsDLG.WinFG3=WinFG1;
        SettingsDLG.WinBG4=WinBG3;
	    SettingsDLG.WinFG4=WinFG3;

		SettingsDLG.ExcludeQuotes=ExcludeQuotes;
	    SettingsDLG.ExcludeL2Quotes=ExcludeL2Quotes;
	    SettingsDLG.ExcludeOPRA=ExcludeOPRA;
        SettingsDLG.ExcludeCRC=ExcludeCRC;
        
		SettingsDLG.IsDelayed=IsDelayed;
	    SettingsDLG.DelayMin=DelayMin;
	    SettingsDLG.DelayPause=DelayPause;

        SettingsDLG.ExitOnTapeEnd=ExitOnTapeEnd;
        SettingsDLG.SaveScreenPos=SaveScreenPos;

        SettingsDLG.NxClockGUIInterval=NxClockGUIInterval;
        SettingsDLG.ActivityClearThreshold=0;//ActivityClearThreshold;

	    if (SettingsDLG.DoModal()==IDOK)
	    {
		    FWXNxCoreMode=SettingsDLG.RunningMode;		    
			StartCoreOnStartup=SettingsDLG.StartCoreOnStartup;
		    strcpy(TapeFileName,SettingsDLG.TapeFileName);
			strcpy(DLLFilename,SettingsDLG.DLLFilename);	

			IsDelayed=SettingsDLG.IsDelayed;
	        DelayMin=SettingsDLG.DelayMin;
	        DelayPause=SettingsDLG.DelayPause;
			
			ExcludeQuotes=SettingsDLG.ExcludeQuotes;
	        ExcludeL2Quotes=SettingsDLG.ExcludeL2Quotes;
	        ExcludeOPRA=SettingsDLG.ExcludeOPRA;
            ExcludeCRC=SettingsDLG.ExcludeCRC;

            ExitOnTapeEnd=SettingsDLG.ExitOnTapeEnd;
            SaveScreenPos=SettingsDLG.SaveScreenPos;

			BackgroundBrush.DeleteObject();
			BackgroundColor=SettingsDLG.WinBG1;
	        TextColor=SettingsDLG.WinFG1;
			BackgroundBrush.CreateSolidBrush(BackgroundColor);	

			RTSimDelay=SettingsDLG.RTSimPause;
			WinBG1=SettingsDLG.WinBG3;
	        WinFG1=SettingsDLG.WinFG3;
		    WinBG2=SettingsDLG.WinBG2;
	        WinFG2=SettingsDLG.WinFG2;
            WinBG3=SettingsDLG.WinBG4;
	        WinFG3=SettingsDLG.WinFG4;
			WinBrush1.DeleteObject();	
			WinBrush2.DeleteObject();	
            WinBrush3.DeleteObject();	
			WinBrush1.CreateSolidBrush(WinBG1);	
	        WinBrush2.CreateSolidBrush(WinBG2);	
            WinBrush3.CreateSolidBrush(WinBG3);	

            NxClockGUIInterval=SettingsDLG.NxClockGUIInterval;
            								
			WriteINIFile(CORECONFIGNAME);
			    
		    switch(FWXNxCoreMode)
	        {
	        case NXCORE_REALTIME:
			     NxCoreModeControl.SetWindowTextA("Realtime");
			     break;

		    case NXCORE_TAPE:
			     char str[1064];
			     sprintf(str,"Historical - %s",TapeFileName);
			     NxCoreModeControl.SetWindowTextA(str);
			     break;
	        }

            JTNX_Interface_SetFeedDelay(JTNX_NxGlobalUserData,IsDelayed,DelayMin,DelayPause);
			JTNX_Interface_SetHistMode(JTNX_NxGlobalUserData,HistTimeMode,RTSimDelay);
			JTNX_Interface_SetExcludeFlags(JTNX_NxGlobalUserData,ExcludeQuotes,ExcludeL2Quotes,ExcludeOPRA,ExcludeCRC);
            
            JTNX_Interface_SetNxClockGUIInterval(JTNX_NxGlobalUserData,NxClockGUIInterval);
            if (NxClockGUIInterval!=(NxCLOCK_HOUR+1))
              JTNX_Interface_SetTimeDisplayCallback(JTNX_NxGlobalUserData,WriteNxCoreTime);
            else
            {
              JTNX_Interface_SetTimeDisplayCallback(JTNX_NxGlobalUserData,NULL);
            }
			
			JTNX_Interface_PingRelease(JTNX_NxGlobalUserData);			
			Invalidate();
		}
	}
	else
		MessageBox("Cannot change settings while processing NxCore Data!\nPlease stop NxCore processing first.","Warning!",MB_OK);
}

//-------------------------------------------------------------
void CJTools_ShellAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_COLORPREFS)
	{                
		CColorPrefs ColorDialog;

        ColorDialog.BackgroundColor=BackgroundColor;
        ColorDialog.TextColor=TextColor;
        ColorDialog.StaticBackgroundColor=WinBG1;
        ColorDialog.StaticTextColor=WinFG1;
        
        ColorDialog.ChartBKColor=GraphBckgrnd;
        ColorDialog.GridColor=GridColor;
        ColorDialog.TradeOutlineColor=TradeOutlineColor;
                
        ColorDialog.ChartOutlines=ChartOutlines;
        ColorDialog.NewFont=&ChartFont;

        ColorDialog.BBOBidBandColor=ChartColors.BBOBidBandColor;
        ColorDialog.BBOAskBandColor=ChartColors.BBOAskBandColor;

        ColorDialog.MaxBars=MAXBARS;

        memcpy(ColorDialog.ExchangeMap,ExchangeMap,sizeof(ExchangeMapType)*MAXEXCHANGES);
        
		if (ColorDialog.DoModal()==IDOK)
        {
          memcpy(ExchangeMap,ColorDialog.ExchangeMap,sizeof(ExchangeMapType)*MAXEXCHANGES);
          for (int loop=0;loop<MAXEXCHANGES;loop++)    
          {
            SetColorMem(&ChartColors,loop,ExchangeMap[loop].LastColor,ExchangeMap[loop].BidColor,ExchangeMap[loop].AskColor,ExchangeMap[loop].PenWidth);                                      
          }
          ChartColors.BBOBidBandColor=ColorDialog.BBOBidBandColor;
          ChartColors.BBOAskBandColor=ColorDialog.BBOAskBandColor;
          SetBandingColor();

          BackgroundColor=ColorDialog.BackgroundColor;
          TextColor=ColorDialog.TextColor;
          WinBG1=ColorDialog.StaticBackgroundColor;
          WinFG1=ColorDialog.StaticTextColor;
          
          GraphBckgrnd=ColorDialog.ChartBKColor;
          GridColor=ColorDialog.GridColor;
          
          BackgroundBrush.DeleteObject();	
          BackgroundBrush.CreateSolidBrush(BackgroundColor);	
          
          ClearColor=BackgroundColor;
          StudyTextColor=TextColor;
          ScaleTextColor=TextColor;
          DescripTextColor=TextColor;

          TradeOutlineColor=ColorDialog.TradeOutlineColor;
          
          ChartOutlines=ColorDialog.ChartOutlines;
          
          Chart.SetChartColors(ClearColor,ScaleTextColor,
                        GraphBckgrnd,StudyTextColor,ChartOutlines,
                        DescripTextColor,GridColor);
          VolChart.SetChartColors(ClearColor,ScaleTextColor,
                        GraphBckgrnd,StudyTextColor,ChartOutlines,
                        DescripTextColor,GridColor);
          LegendChart.SetChartColors(ClearColor,ScaleTextColor,
                        GraphBckgrnd,StudyTextColor,ChartOutlines,
                        DescripTextColor,GridColor);

          VolChart.SetTradeOutlineColor(TradeOutlineColor);
          Chart.SetTradeOutlineColor(TradeOutlineColor);
          Chart.SetChartFont(&ChartFont,&ChartFont,&ChartFont);
          VolChart.SetChartFont(&ChartFont,&ChartFont,&ChartFont);
          LegendChart.SetChartFont(&ChartFont,&ChartFont,&ChartFont);

          PriceLenOffest1=(int)(strlen(PriceStr1)*Chart.FontHeight/2);          
          VolLenOffset1=(int)(strlen(VolStr1)*Chart.FontHeight/2);
          PriceLenOffest2=(int)(strlen(PriceStr2)*Chart.FontHeight/2);
          VolLenOffset2=(int)(strlen(VolStr2)*Chart.FontHeight/2);

          if (ColorDialog.MaxBars!=MAXBARS)
            OnBnClickedStopcore();           
          
          MAXBARS=ColorDialog.MaxBars;

          WriteINIFile(CORECONFIGNAME);
                
          Invalidate(1);
          RepositionChart(0,0);      
        }
	}    
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// Write the system preferences to file
//-------------------------------------------------------------
void CJTools_ShellAppDlg::WriteINIFile(char *FileName)
{    	
	char str[256];
    sprintf(str,"JTools_RunningINIFiles\\%s",FileName);
	FILE *out;
	out=fopen(str,"wt");	
   	
	if (out)
    {		
        if (TapeFileName[0]) fprintf(out,"TAPENAME=%s  ;Tape Filename\n",TapeFileName);		
        if (DLLFilename[0])  fprintf(out,"DLLNAME=%s  ;DLL Filename\n",DLLFilename);				             
        fprintf(out,";-------------------------------------------------------------------------\n");       
        fprintf(out,";The following items are used to control various aspects  of the application.\n");       
        fprintf(out,";-------------------------------------------------------------------------\n");       
        fprintf(out,"COREMODE=%d  ;NxCore Mode (1=Realtime, 2=Historical from tape)\n",FWXNxCoreMode);
        fprintf(out,"FMAJOR=%d  ;File Major Log (1=YES,0=NO)\n",FileMajorLog);
        fprintf(out,"MLOGON=%d  ;Enable Major Log (1=YES,0=NO)\n",MajorLoggingOn);
        fprintf(out,"RLOGNXDATE=%d  ;Reset Logfile baded on NxTape Date (1=YES,0=NO)\n",ResetLogOnNXDate);                
        fprintf(out,"EOPRA=%d  ;Exclude OPRA mode (1=YES,0=NO)\n",ExcludeOPRA);
        fprintf(out,"ECRC=%d  ;Exclude CRC (1=YES,0=NO)\n",ExcludeCRC);        
        fprintf(out,"RTSIMD=%d  ;RT Sim Delay (specify in Milliseconds)\n",RTSimDelay);
        fprintf(out,"ISDELAYED=%d  ;Delayed feed (1=YES,0=NO)\n",IsDelayed);
        fprintf(out,"DELAYMIN=%d  ;Delay minutes (specify in Minutes)\n",DelayMin);
        fprintf(out,"DELAYPAUSE=%d  ;Delay Pause (specify in Milliseconds)\n",DelayPause);               
        fprintf(out,";-------------------------------------------------------------------------\n");       
        fprintf(out,";The following items are usually related to the GUI driven application\n");       
        fprintf(out,";but might also be utilized in a console or service type application.\n");       
        fprintf(out,";-------------------------------------------------------------------------\n");               
        fprintf(out,"CORELAUNCH=%d  ;Launch Nx on Startup (1=YES,0=NO)\n",StartCoreOnStartup);
        fprintf(out,"COREEXIT=%d  ;Exit Application on Tape End (1=YES,0=NO)\n",ExitOnTapeEnd);
        fprintf(out,";-------------------------------------------------------------------------\n");       
        fprintf(out,";The following items are only related to the GUI driven application.\n");       
        fprintf(out,";They can be safely ignored in a console or service type of application.\n");       
        fprintf(out,";-------------------------------------------------------------------------\n");       
        fprintf(out,"COMPACT=%d  ;Compact mode (1=YES,0=NO)\n",CompactMode);        
        fprintf(out,"BKCOLOR=%06x  ;Background Color (RRGGBB)\n",JTParser_ReverseColor(BackgroundColor));
        fprintf(out,"TXCOLOR=%06x  ;Text Color (RRGGBB)\n",JTParser_ReverseColor(TextColor));        
        fprintf(out,"ACTIVITYBG=%06x  ;Activity BG (RRGGBB)\n",JTParser_ReverseColor(WinBG2));
        fprintf(out,"ACTIVITYFG=%06x  ;Activity FG (RRGGBB)\n",JTParser_ReverseColor(WinFG2));
        fprintf(out,"READONLYBG=%06x  ;ReadOnly BG (RRGGBB)\n",JTParser_ReverseColor(WinBG1));
        fprintf(out,"READONLYFG=%06x  ;ReadOnly FG (RRGGBB)\n",JTParser_ReverseColor(WinFG1));
        fprintf(out,"EDITABLEBG=%06x  ;Editable BG (RRGGBB)\n",JTParser_ReverseColor(WinBG3));
        fprintf(out,"EDITABLEFG=%06x  ;Editable FG (RRGGBB)\n",JTParser_ReverseColor(WinFG3));
        
        if (SaveScreenPos)
        {
          GetWindowPlacement(&GVwp);	
          fprintf(out,"PLACEMENT=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            GVwp.flags,GVwp.length,
            GVwp.ptMaxPosition.x,GVwp.ptMaxPosition.y,
            GVwp.ptMinPosition.x,GVwp.ptMinPosition.y,
            GVwp.rcNormalPosition.bottom,GVwp.rcNormalPosition.left,GVwp.rcNormalPosition.right,GVwp.rcNormalPosition.top,
            GVwp.showCmd);
        }

        fprintf(out,";-------------------------------------------------------------------------\n");       
        fprintf(out,";The following items are only related to the JTools_DepthMapper app.\n");       
        fprintf(out,";-------------------------------------------------------------------------\n");              

        fprintf(out,"GRIDCOLOR=%06x  ; (RRGGBB)\n",JTParser_ReverseColor(GridColor));
        fprintf(out,"TRADEOUTLINE=%06x  ; (RRGGBB)\n",JTParser_ReverseColor(TradeOutlineColor));        
        fprintf(out,"CHARTBK=%06x  ; (RRGGBB)\n",JTParser_ReverseColor(GraphBckgrnd));
        fprintf(out,"CHARTOUTLINE=%06x  ; (RRGGBB)\n",JTParser_ReverseColor(ChartOutlines));
        fprintf(out,"SHOWGRIDS=%d  ;Show Grids (1=YES,0=NO)\n",ShowGrids);
        fprintf(out,"SHOWPRICE=%d  ;Show Price (1=YES,0=NO)\n",ShowPrice);
        fprintf(out,"SHOWVOL=%d  ;Show Volume (1=YES,0=NO)\n",ShowVol);   
        fprintf(out,"SHOWLEGEND=%d  ;Show Legend (1=YES,0=NO)\n",ShowLegend);        
        fprintf(out,"BBOUPPERBAND=%06x  ; (RRGGBB)\n",JTParser_ReverseColor(ChartColors.BBOAskBandColor));
        fprintf(out,"BBOLOWERBAND=%06x  ; (RRGGBB)\n",JTParser_ReverseColor(ChartColors.BBOBidBandColor));
        fprintf(out,"TRADEPTSIZE=%d  ; Trade Point Size\n",PointExpand);
        fprintf(out,"QUOTEPTSIZE=%d  ; Quote Point Size\n",PointExpand2);
        fprintf(out,"BBOBANDAMOUNT=%0.2f  ; BBO Banding Amount (As %)\n",ChartData.BBOBandAmount);
        fprintf(out,"CIRCUITSTYLE=%d  ; Circuit Style (1=YES, 0=NO)\n",ChartData.CircuitStyle);
        fprintf(out,"TRADELINE=%d  ; Trade Line (1=YES, 0=NO)\n",ChartData.DrawTradeLine);
        fprintf(out,"BIDLINE=%d  ; Bid Line (1=YES, 0=NO)\n",ChartData.DrawBidLine);
        fprintf(out,"ASKLINE=%d  ; Bid Line (1=YES, 0=NO)\n",ChartData.DrawAskLine);
        fprintf(out,"CTRADELINE=%d  ; CompTrade Line (1=YES, 0=NO)\n",ChartData.DrawCTradeLine);
        fprintf(out,"CBIDLINE=%d  ; CompBid Line (1=YES, 0=NO)\n",ChartData.DrawCBidLine);
        fprintf(out,"CASKLINE=%d  ; CompBid Line (1=YES, 0=NO)\n",ChartData.DrawCAskLine);
        fprintf(out,"ROUNDLOTADJ=%d  ; Round Lot Adjustment (1=YES, 0=NO)\n",RoundLotAdj);                      
        fprintf(out,"AUTOTRADE=%d  ; AutoTradesOn (1=YES,0=NO)\n",AutoTradesOn);
        fprintf(out,"TIMETICKHIGH=%06x  ;TIMETICKHIGH (RRGGBB)\n",JTParser_ReverseColor(TimeTickHighColor));
        fprintf(out,"TIMETICKLOW=%06x  ;TIMETICKHIGH (RRGGBB)\n",JTParser_ReverseColor(TimeTickLowColor));
        fprintf(out,"MAXBARS=%d  ;MAXBARS (86400 is default)\n",MAXBARS);
        fprintf(out,"SPLITSIZE=%d  ;SplitSizeChart (1=YES,0=NO)\n",SplitSizeChart);
                
        LOGFONT Font;	
		ChartFont.GetLogFont(&Font);        
        char newname[32];
        strcpy(newname,Font.lfFaceName);
        int len=strlen(newname);
        for (int loop=0;loop<len;loop++) if (newname[loop]==' ') newname[loop]='|';
        fprintf(out,"FONTSPEC=%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
          newname,
          Font.lfCharSet,Font.lfClipPrecision,Font.lfEscapement,Font.lfHeight,Font.lfItalic,
          Font.lfOrientation,Font.lfOutPrecision,Font.lfPitchAndFamily,Font.lfQuality,
          Font.lfStrikeOut,Font.lfUnderline,Font.lfWeight,Font.lfWidth);

        for (int loop=0;loop<MAXEXCHANGES;loop++)
        {          
          fprintf(out,"EMAP%d=%d,%s,%06x,%06x,%06x\n",                  
                  loop+1,
                  ExchangeMap[loop].Exchange,ExchangeMap[loop].Title,
                  JTParser_ReverseColor(ExchangeMap[loop].LastColor),
                  JTParser_ReverseColor(ExchangeMap[loop].BidColor),
                  JTParser_ReverseColor(ExchangeMap[loop].AskColor));
        }
          
		// Close er up
		fflush(out);
		fclose(out);
	}    	
}

// Read the system preferences from file
//-------------------------------------------------------------
unsigned char CJTools_ShellAppDlg::ReadINIFile(char *FileName)
{  
  LOGFONT Font;	
  char str[256];
  sprintf(str,"JTools_RunningINIFiles\\%s",FileName);
  FILE *in;  
  in=fopen(str,"rt");
  if (in)
  {
    char str[1024];  
    char Label[256],Value[256];
    int EMapCount=1;

    SaveScreenPos=0;

    while (!feof(in))
    {      
      char *sp=str;
      char ch=1;

      // Scan full line from file
      while ((!feof(in))&&(ch!='\n')&&(ch!='\r')&&(ch)) { fscanf(in,"%c",&ch); if (ch) *sp++=ch;}
      *sp=0;

      if ((!feof(in))&&(str[0]!=';'))
      {
        // Parse the string
        sp=str;
        sp=JTParser_ParseINIString(sp,Label,Value);
      
        if (!strcmp(Label,"TAPENAME")) 
        {
          if ((Value[0]=='"')&&(Value[1]=='"')) TapeFileName[0]=0;
          else strcpy(TapeFileName,Value);
        }        
        else if (!strcmp(Label,"DLLNAME")) strcpy(DLLFilename,Value);
        else if (!strcmp(Label,"RTSIMD")) RTSimDelay=atoi(Value);
        else if (!strcmp(Label,"CORELAUNCH")) StartCoreOnStartup=atoi(Value);
        else if (!strcmp(Label,"COREEXIT")) ExitOnTapeEnd=atoi(Value);
        else if (!strcmp(Label,"COREMODE")) FWXNxCoreMode=atoi(Value);        
        else if (!strcmp(Label,"FMAJOR")) FileMajorLog=atoi(Value);
        else if (!strcmp(Label,"MLOGON")) MajorLoggingOn=atoi(Value);
        else if (!strcmp(Label,"RLOGNXDATE")) ResetLogOnNXDate=atoi(Value);
        else if (!strcmp(Label,"COMPACT")) CompactMode=atoi(Value);
        else if (!strcmp(Label,"EOPRA")) ExcludeOPRA=atoi(Value);
        else if (!strcmp(Label,"ECRC")) ExcludeCRC=atoi(Value);        
        else if (!strcmp(Label,"ISDELAYED")) IsDelayed=atoi(Value);
        else if (!strcmp(Label,"DELAYMIN")) DelayMin=atoi(Value);
        else if (!strcmp(Label,"DELAYPAUSE")) DelayPause=atoi(Value);        
        else if (!strcmp(Label,"BKCOLOR")) 
        {
          sscanf(Value,"%x",&BackgroundColor);BackgroundColor=JTParser_ReverseColor(BackgroundColor);
        }
        else if (!strcmp(Label,"TXCOLOR")) 
        {
          sscanf(Value,"%x",&TextColor);TextColor=JTParser_ReverseColor(TextColor);
        }
        else if (!strcmp(Label,"READONLYBG")) 
        {
          sscanf(Value,"%x",&WinBG1);WinBG1=JTParser_ReverseColor(WinBG1);
        }
        else if (!strcmp(Label,"READONLYFG")) 
        {
          sscanf(Value,"%x",&WinFG1);WinFG1=JTParser_ReverseColor(WinFG1);
        }
        else if (!strcmp(Label,"EDITABLEBG")) 
        {
          sscanf(Value,"%x",&WinBG3);WinBG3=JTParser_ReverseColor(WinBG3);
        }
        else if (!strcmp(Label,"EDITABLEFG")) 
        {
          sscanf(Value,"%x",&WinFG3);WinFG3=JTParser_ReverseColor(WinFG3);
        }
        else if (!strcmp(Label,"ACTIVITYBG")) 
        {
          sscanf(Value,"%x",&WinBG2);WinBG2=JTParser_ReverseColor(WinBG2);
        }
        else if (!strcmp(Label,"ACTIVITYFG")) 
        {
          sscanf(Value,"%x",&WinFG2);WinFG2=JTParser_ReverseColor(WinFG2);
        }
        else if (!strcmp(Label,"PLACEMENT")) 
        {
          char *sp2=Value;
          char PVal[32];
          sp2=JTParser_ReadToDelim(sp2,PVal,',');GVwp.flags=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');GVwp.length=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');GVwp.ptMaxPosition.x=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');GVwp.ptMaxPosition.y=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');GVwp.ptMinPosition.x=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');GVwp.ptMinPosition.y=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');GVwp.rcNormalPosition.bottom=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');GVwp.rcNormalPosition.left=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');GVwp.rcNormalPosition.right=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');GVwp.rcNormalPosition.top=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');GVwp.showCmd=atoi(PVal);          
          SetWindowPlacement(&GVwp);	          
          SaveScreenPos=1;
        }        
        else if (!strcmp(Label,"GRIDCOLOR")) 
        {
          sscanf(Value,"%x",&GridColor);GridColor=JTParser_ReverseColor(GridColor);
        }
        else if (!strcmp(Label,"TRADEOUTLINE")) 
        {
          sscanf(Value,"%x",&TradeOutlineColor);TradeOutlineColor=JTParser_ReverseColor(TradeOutlineColor);
        }
        else if (!strcmp(Label,"CHARTBK")) 
        {
          sscanf(Value,"%x",&GraphBckgrnd);GraphBckgrnd=JTParser_ReverseColor(GraphBckgrnd);
        }
        else if (!strcmp(Label,"CHARTOUTLINE")) 
        {
          sscanf(Value,"%x",&ChartOutlines);ChartOutlines=JTParser_ReverseColor(ChartOutlines);
        }
        else if (!strcmp(Label,"BBOUPPERBAND")) 
        {
          sscanf(Value,"%x",&ChartColors.BBOAskBandColor);ChartColors.BBOAskBandColor=JTParser_ReverseColor(ChartColors.BBOAskBandColor);
        }
        else if (!strcmp(Label,"BBOLOWERBAND")) 
        {
          sscanf(Value,"%x",&ChartColors.BBOBidBandColor);ChartColors.BBOBidBandColor=JTParser_ReverseColor(ChartColors.BBOBidBandColor);
        }
        
        else if (!strcmp(Label,"SHOWGRIDS")) ShowGrids=atoi(Value);
        else if (!strcmp(Label,"SHOWPRICE")) ShowPrice=atoi(Value);
        else if (!strcmp(Label,"SHOWLEGEND")) ShowLegend=atoi(Value);
        else if (!strcmp(Label,"SHOWVOL")) ShowVol=atoi(Value);                
        else if (!strcmp(Label,"TRADEPTSIZE")) PointExpand=atoi(Value);        
        else if (!strcmp(Label,"QUOTEPTSIZE")) PointExpand2=atoi(Value);        
        else if (!strcmp(Label,"BBOBANDAMOUNT")) ChartData.BBOBandAmount=atof(Value);        
        else if (!strcmp(Label,"CIRCUITSTYLE")) ChartData.CircuitStyle=atoi(Value);                
        else if (!strcmp(Label,"TRADELINE")) ChartData.DrawTradeLine=atoi(Value);        
        else if (!strcmp(Label,"BIDLINE")) ChartData.DrawBidLine=atoi(Value);        
        else if (!strcmp(Label,"ASKLINE")) ChartData.DrawAskLine=atoi(Value);        
        else if (!strcmp(Label,"CTRADELINE")) ChartData.DrawCTradeLine=atoi(Value);        
        else if (!strcmp(Label,"CBIDLINE")) ChartData.DrawCBidLine=atoi(Value);        
        else if (!strcmp(Label,"CASKLINE")) ChartData.DrawCAskLine=atoi(Value);        
        else if (!strcmp(Label,"ROUNDLOTADJ")) RoundLotAdj=atoi(Value);                        
        else if (!strcmp(Label,"AUTOTRADE")) AutoTradesOn=atoi(Value);   
        else if (!strcmp(Label,"MAXBARS")) MAXBARS=atoi(Value);           
        else if (!strcmp(Label,"SPLITSIZE")) SplitSizeChart=atoi(Value);           
        else if (!strcmp(Label,"TIMETICKHIGH")) 
        {
          sscanf(Value,"%x",&TimeTickHighColor);TimeTickHighColor=JTParser_ReverseColor(TimeTickHighColor);
        }
        else if (!strcmp(Label,"TIMETICKLOW")) 
        {
          sscanf(Value,"%x",&TimeTickLowColor);TimeTickLowColor=JTParser_ReverseColor(TimeTickLowColor);
        }
        else if (!strcmp(Label,"FONTSPEC"))
        {
          char *sp2=Value;
          char PVal[32];
          sp2=JTParser_ReadToDelim(sp2,PVal,',');strcpy(Font.lfFaceName,PVal);
          int len=strlen(Font.lfFaceName);
          for (int loop=0;loop<len;loop++) if (Font.lfFaceName[loop]=='|') Font.lfFaceName[loop]=' ';
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfCharSet=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfClipPrecision=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfEscapement=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfHeight=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfItalic=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfOrientation=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfOutPrecision=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfPitchAndFamily=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfQuality=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfStrikeOut=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfUnderline=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfWeight=atoi(PVal);
          sp2=JTParser_ReadToDelim(sp2,PVal,',');Font.lfWidth=atoi(PVal);

          ChartFont.DeleteObject();
	      ChartFont.CreateFont(Font.lfHeight,Font.lfWidth,Font.lfEscapement,
		        Font.lfOrientation,Font.lfWeight,Font.lfItalic,Font.lfUnderline,
		        Font.lfStrikeOut,Font.lfCharSet,Font.lfOutPrecision,Font.lfClipPrecision,
		        Font.lfQuality,Font.lfPitchAndFamily,Font.lfFaceName);		          
        }
        else
        {
          char EComp[32];
          sprintf(EComp,"EMAP%d",EMapCount);
          if (!strcmp(Label,EComp))
          {
            char *sp2=Value;
            char PVal[32];
            sp2=JTParser_ReadToDelim(sp2,PVal,',');ExchangeMap[EMapCount-1].Exchange=atoi(PVal);
            sp2=JTParser_ReadToDelim(sp2,PVal,',');strcpy(ExchangeMap[EMapCount-1].Title,PVal);
            sp2=JTParser_ReadToDelim(sp2,PVal,',');
            {
              sscanf(PVal,"%x",&ExchangeMap[EMapCount-1].LastColor);
              ExchangeMap[EMapCount-1].LastColor=JTParser_ReverseColor(ExchangeMap[EMapCount-1].LastColor);
            }
            sp2=JTParser_ReadToDelim(sp2,PVal,',');
            {
              sscanf(PVal,"%x",&ExchangeMap[EMapCount-1].BidColor);
              ExchangeMap[EMapCount-1].BidColor=JTParser_ReverseColor(ExchangeMap[EMapCount-1].BidColor);
            }
            sp2=JTParser_ReadToDelim(sp2,PVal,',');
            {
              sscanf(PVal,"%x",&ExchangeMap[EMapCount-1].AskColor);
              ExchangeMap[EMapCount-1].AskColor=JTParser_ReverseColor(ExchangeMap[EMapCount-1].AskColor);
            }
            
            ++EMapCount;
          }
        }
      }           
    }
    fclose(in);

    if (!TapeFileName[0]) FWXNxCoreMode=NXCORE_REALTIME;
    if (NxClockGUIInterval<NxCLOCK_SECOND) NxClockGUIInterval=NxCLOCK_SECOND;
    return 1;
  }
  return 0;
}


// DPI Stuff for proper sizing of GUI elements
//---------------------------------------------
void CJTools_ShellAppDlg::GetDPI()
{
    int ly=GetDeviceCaps(::GetDC(NULL),LOGPIXELSY);
	int lx=GetDeviceCaps(::GetDC(NULL),LOGPIXELSX);
	DPIScale=(double)(lx)/(double)DPI100;
}
//---------------------------------------------
int CJTools_ShellAppDlg::DPIpx(int num)
{
	double newnum=(double)num*DPIScale;
	return((int)newnum);
}


