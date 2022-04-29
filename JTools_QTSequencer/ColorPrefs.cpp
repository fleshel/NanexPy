// ColorPrefs.cpp : implementation file
//
#include "stdafx.h"
#include "ColorPrefs.h"

// CColorPrefs dialog
IMPLEMENT_DYNAMIC(CColorPrefs, CDialog)

CColorPrefs::CColorPrefs(CWnd* pParent /*=NULL*/)
	: CDialog(CColorPrefs::IDD, pParent)
{
    ChartBKColor=RGB(255,255,255);
    GridColor=RGB(0,0,0);    
    BackgroundColor=GetSysColor(COLOR_BTNFACE);//COLOR_WINDOW);
    TextColor=GetSysColor(COLOR_WINDOWTEXT);	
    StaticBackgroundColor=RGB(110,110,110);
    StaticTextColor=RGB(255,255,255);    
    ChartOutlines=RGB(255,255,255);    
    BBOAskBandColor=RGB(255,120,0);
    BBOBidBandColor=RGB(0,120,255);
    MaxBars=86400;
}

CColorPrefs::~CColorPrefs()
{
}

void CColorPrefs::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);    
  DDX_Control(pDX, IDC_MAINCOLORCONTROL1, MainColor1Control);
  DDX_Control(pDX, IDC_MAINCOLORCONTROL2, MainColor2Control);
  DDX_Control(pDX, IDC_MAINCOLORCONTROL3, MainColor3Control);
    
  DDX_Control(pDX, IDC_MAINCOLORCONTROL7, MainColor7Control);
  DDX_Control(pDX, IDC_MAINCOLORCONTROL8, MainColor8Control);
  DDX_Control(pDX, IDC_MAINCOLORCONTROL9, MainColor9Control);
  DDX_Control(pDX, IDC_MAINCOLORCONTROL10, MainColor10Control);
  DDX_Control(pDX, IDC_MAINCOLORCONTROL11, MainColor11Control);
  DDX_Control(pDX, IDC_MAINCOLORCONTROL12, MainColor12Control);
  
  DDX_Control(pDX, IDC_WINSHOW5, FontControl);  
  DDX_Control(pDX, IDC_EXCHANGELIST, ExchangeList);  
  DDX_Control(pDX, IDC_MAXBARS, MaxBarsControl);  
}

BEGIN_MESSAGE_MAP(CColorPrefs, CDialog)
  ON_WM_CTLCOLOR()  
  ON_BN_CLICKED(IDC_MAINCOLORPICK1, &CColorPrefs::OnBnClickedMaincolorpick1)
  ON_BN_CLICKED(IDC_MAINCOLORPICK2, &CColorPrefs::OnBnClickedMaincolorpick2)
  ON_BN_CLICKED(IDC_MAINFGCOLORPICK1, &CColorPrefs::OnBnClickedMainfgcolorpick1)
  ON_BN_CLICKED(IDC_MAINFGCOLORPICK2, &CColorPrefs::OnBnClickedMainfgcolorpick2)
  ON_BN_CLICKED(IDC_MAINCOLORPICK3, &CColorPrefs::OnBnClickedMaincolorpick3)
  ON_BN_CLICKED(IDC_MAINFGCOLORPICK3, &CColorPrefs::OnBnClickedMainfgcolorpick3)
  
  ON_BN_CLICKED(IDC_MAINCOLORPICK7, &CColorPrefs::OnBnClickedMaincolorpick7)  
  ON_BN_CLICKED(IDC_LISTFONT, &CColorPrefs::OnBnClickedListfont)
  ON_BN_CLICKED(IDC_DEFAULT, &CColorPrefs::OnBnClickedDefault)
  ON_BN_CLICKED(IDC_WHITESCHEME, &CColorPrefs::OnBnClickedWhitescheme)
  ON_BN_CLICKED(IDC_BLACKSCHEME, &CColorPrefs::OnBnClickedBlackscheme)
  ON_BN_CLICKED(IDC_MAINCOLORPICK8, &CColorPrefs::OnBnClickedMaincolorpick8)  
  ON_BN_CLICKED(IDC_MAINCOLORPICK9, &CColorPrefs::OnBnClickedMaincolorpick9)  
  ON_BN_CLICKED(IDC_MAINCOLORPICK10, &CColorPrefs::OnBnClickedMaincolorpick10)  
  ON_BN_CLICKED(IDC_MAINCOLORPICK11, &CColorPrefs::OnBnClickedMaincolorpick11)  
  ON_BN_CLICKED(IDC_MAINCOLORPICK12, &CColorPrefs::OnBnClickedMaincolorpick12)  
  
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_EXCHANGELIST, &CColorPrefs::OnLvnItemchangedExchangelist)
  ON_EN_CHANGE(IDC_MAXBARS, &CColorPrefs::OnEnChangeMaxbars)
  
END_MESSAGE_MAP()

//-------------------------------------------------------------
BOOL CColorPrefs::OnInitDialog()
{
	CDialog::OnInitDialog();
    FontControl.SetFont(NewFont,1);
	FontControl.SetWindowTextA("CHART FONT");   		
	BackgroundBrush.CreateSolidBrush(BackgroundColor);	
    StaticBrush.CreateSolidBrush(StaticBackgroundColor);
    ChartBKBrush.CreateSolidBrush(ChartBKColor);	        

    ::SetWindowTextA(GetDlgItem(IDC_MAINCOLORCONTROL3)->m_hWnd,"GRID");
    ::SetWindowTextA(GetDlgItem(IDC_MAINCOLORCONTROL1)->m_hWnd,"WINDOW");
    ::SetWindowTextA(GetDlgItem(IDC_MAINCOLORCONTROL2)->m_hWnd,"CONTROLS");       
    ::SetWindowTextA(GetDlgItem(IDC_MAINCOLORCONTROL7)->m_hWnd,"TOUTLINE");
    ::SetWindowTextA(GetDlgItem(IDC_MAINCOLORCONTROL8)->m_hWnd,"TRADE");
    ::SetWindowTextA(GetDlgItem(IDC_MAINCOLORCONTROL9)->m_hWnd,"BID");
    ::SetWindowTextA(GetDlgItem(IDC_MAINCOLORCONTROL10)->m_hWnd,"ASK");
    ::SetWindowTextA(GetDlgItem(IDC_MAINCOLORCONTROL11)->m_hWnd,"ASK BAND");
    ::SetWindowTextA(GetDlgItem(IDC_MAINCOLORCONTROL12)->m_hWnd,"BID BAND");

    CurrentItem=0;
    LastColor=ExchangeMap[0].LastColor;
    BidColor=ExchangeMap[0].BidColor;
    AskColor=ExchangeMap[0].AskColor;

    ExchangeList.SetExtendedStyle(ExchangeList.GetExtendedStyle()|LVS_EX_FULLROWSELECT );	
    ExchangeList.InsertColumn(0,"Exchange",0,250);

    for (int loop=0;loop<MAXEXCHANGES;loop++)
      ExchangeList.InsertItem(loop,ExchangeMap[loop].Title);

    char str[12];
    sprintf(str,"%d",MaxBars);
    MaxBarsControl.SetWindowTextA(str);
        
    return 1;
}
//-------------------------------------------------------------
void CColorPrefs::SetCurrentItem(int Item)
{
  LastColor=ExchangeMap[Item].LastColor;
  BidColor=ExchangeMap[Item].BidColor;
  AskColor=ExchangeMap[Item].AskColor;
  CurrentItem=Item; 
  Invalidate();  
}
//-------------------------------------------------------------
void CColorPrefs::OnLvnItemchangedExchangelist(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
  SetCurrentItem(pNMLV->iItem);
  *pResult = 0;
}
//-------------------------------------------------------------
HBRUSH CColorPrefs::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{	  
    switch (nCtlColor) 
	{
	case CTLCOLOR_STATIC: 
		 if ((pWnd==&MainColor1Control)||(pWnd==&FontControl))
		 {
		     SetTextColor(pDC->m_hDC,TextColor);
		     SetBkColor( pDC->m_hDC,BackgroundColor );
		     return (HBRUSH)(BackgroundBrush.GetSafeHandle());	          	     
		 }
		 else if (pWnd==&MainColor2Control)
		 {
		     SetTextColor(pDC->m_hDC,StaticTextColor);
		     SetBkColor( pDC->m_hDC,StaticBackgroundColor );
		     return (HBRUSH)(StaticBrush.GetSafeHandle());	          	     
		 }
         else if (pWnd==&MainColor3Control)
		 {
		     SetTextColor(pDC->m_hDC,GridColor);
		     SetBkColor( pDC->m_hDC,ChartBKColor );
		     return (HBRUSH)(ChartBKBrush.GetSafeHandle());	          	     
		 }
         
         else if (pWnd==&MainColor7Control)
		 {
		     SetTextColor(pDC->m_hDC,TradeOutlineColor);
		     SetBkColor( pDC->m_hDC,ChartBKColor );
		     return (HBRUSH)(ChartBKBrush.GetSafeHandle());	          	     
		 }
         else if (pWnd==&MainColor8Control)
		 {
		     SetTextColor(pDC->m_hDC,LastColor);
		     SetBkColor( pDC->m_hDC,ChartBKColor );
		     return (HBRUSH)(ChartBKBrush.GetSafeHandle());	          	     
		 }
         else if (pWnd==&MainColor9Control)
		 {
		     SetTextColor(pDC->m_hDC,BidColor);
		     SetBkColor( pDC->m_hDC,ChartBKColor );
		     return (HBRUSH)(ChartBKBrush.GetSafeHandle());	          	     
		 }
         else if (pWnd==&MainColor10Control)
		 {
		     SetTextColor(pDC->m_hDC,AskColor);
		     SetBkColor( pDC->m_hDC,ChartBKColor );
		     return (HBRUSH)(ChartBKBrush.GetSafeHandle());	          	     
		 }
         else if (pWnd==&MainColor11Control)
		 {
		     SetTextColor(pDC->m_hDC,BBOAskBandColor);
		     SetBkColor( pDC->m_hDC,ChartBKColor );
		     return (HBRUSH)(ChartBKBrush.GetSafeHandle());	          	     
		 }
         else if (pWnd==&MainColor12Control)
		 {
		     SetTextColor(pDC->m_hDC,BBOBidBandColor);
		     SetBkColor( pDC->m_hDC,ChartBKColor );
		     return (HBRUSH)(ChartBKBrush.GetSafeHandle());	          	     
		 }
         else
         {
             SetTextColor(pDC->m_hDC,TextColor);
		     SetBkColor( pDC->m_hDC,BackgroundColor );
		     return (HBRUSH)(BackgroundBrush.GetSafeHandle());	
         }
	     break;
    case CTLCOLOR_DLG:	      
    case CTLCOLOR_BTN:	      
	     return (HBRUSH)(BackgroundBrush.GetSafeHandle());	         
    } 
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);    
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMaincolorpick1()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = BackgroundColor;
  if (CDial.DoModal()==IDOK)
  {
	BackgroundColor=CDial.GetColor();	
    BackgroundBrush.DeleteObject();		
    BackgroundBrush.CreateSolidBrush(BackgroundColor);	

	Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMainfgcolorpick1()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = TextColor;
  if (CDial.DoModal()==IDOK)
  {
	TextColor=CDial.GetColor();	        
	Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMaincolorpick2()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = StaticBackgroundColor;
  if (CDial.DoModal()==IDOK)
  {
	StaticBackgroundColor=CDial.GetColor();	
    StaticBrush.DeleteObject();		
    StaticBrush.CreateSolidBrush(StaticBackgroundColor);	
	Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMainfgcolorpick2()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = StaticTextColor;
  if (CDial.DoModal()==IDOK)
  {
	StaticTextColor=CDial.GetColor();	        
	Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMaincolorpick3()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = ChartBKColor;
  if (CDial.DoModal()==IDOK)
  {
	ChartBKColor=CDial.GetColor();	        
    ChartBKBrush.DeleteObject();		
    ChartBKBrush.CreateSolidBrush(ChartBKColor);
	Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMainfgcolorpick3()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = GridColor;
  if (CDial.DoModal()==IDOK)
  {
	GridColor=CDial.GetColor();	        
	Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMaincolorpick7()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = TradeOutlineColor;
  if (CDial.DoModal()==IDOK)
  {
	TradeOutlineColor=CDial.GetColor();	        
	Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMaincolorpick8()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = LastColor;
  if (CDial.DoModal()==IDOK)
  {
	LastColor=CDial.GetColor();	        
    ExchangeMap[CurrentItem].LastColor=LastColor;
	Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMaincolorpick9()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = BidColor;
  if (CDial.DoModal()==IDOK)
  {
	BidColor=CDial.GetColor();	        
    ExchangeMap[CurrentItem].BidColor=BidColor;
	Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMaincolorpick10()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = AskColor;
  if (CDial.DoModal()==IDOK)
  {
	AskColor=CDial.GetColor();	        
    ExchangeMap[CurrentItem].AskColor=AskColor;
	Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMaincolorpick11()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = BBOAskBandColor;
  if (CDial.DoModal()==IDOK)
  {
	BBOAskBandColor=CDial.GetColor();	            
	Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedMaincolorpick12()
{
  CColorDialog CDial;
  CDial.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
  CDial.m_cc.rgbResult = BBOBidBandColor;
  if (CDial.DoModal()==IDOK)
  {
	BBOBidBandColor=CDial.GetColor();	            
	Invalidate();
  }
}

//-------------------------------------------------------------
void CColorPrefs::OnBnClickedListfont()
{
    LOGFONT Font;
	NewFont->GetLogFont(&Font);
	CFontDialog FontDial(&Font,CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS);
	if (FontDial.DoModal()==IDOK)
	{	
	    FontDial.GetCurrentFont(&Font);	
	    NewFont->CreateFont(Font.lfHeight,Font.lfWidth,Font.lfEscapement,
		    Font.lfOrientation,Font.lfWeight,Font.lfItalic,Font.lfUnderline,
		    Font.lfStrikeOut,Font.lfCharSet,Font.lfOutPrecision,Font.lfClipPrecision,
		    Font.lfQuality,Font.lfPitchAndFamily,Font.lfFaceName);
	    FontControl.SetFont(NewFont,1);
	}
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedDefault()
{
  if (MessageBox("Really Reset to Default?","Message",MB_YESNO)==IDYES)
  {
    MaxBars=86400;

    NewFont->DeleteObject();
    NewFont->CreateFontA(-MulDiv(8, GetDeviceCaps(GetDC()->m_hDC, LOGPIXELSY), 72),0,1,1,FW_REGULAR,0,0,0,0,0,0,0,0,"Arial");    
    FontControl.SetFont(NewFont,1);

    BackgroundBrush.DeleteObject();	
    StaticBrush.DeleteObject();	
    ChartBKBrush.DeleteObject();

	ChartBKColor=RGB(0,0,0);
    GridColor=RGB(80,80,80);
    TradeOutlineColor=RGB(255,255,255);

    BackgroundColor=RGB(54,54,54);
    TextColor=RGB(255,255,255);
    StaticBackgroundColor=RGB(110,110,110);
    StaticTextColor=RGB(255,255,255);    
     
    BackgroundBrush.CreateSolidBrush(BackgroundColor);	
    StaticBrush.CreateSolidBrush(StaticBackgroundColor);
    ChartBKBrush.CreateSolidBrush(ChartBKColor);	

    SeperatorColor=RGB(255,255,255);
    ChartOutlines=RGB(255,255,255);
  
    ExchangeMap[0].LastColor=RGB(255,255,255);
    ExchangeMap[1].LastColor=RGB(200,200,200);
    ExchangeMap[2].LastColor=RGB(190,190,190);
    ExchangeMap[3].LastColor=RGB(180,180,180);
    ExchangeMap[4].LastColor=RGB(170,170,170);
    ExchangeMap[5].LastColor=RGB(160,160,160);
    ExchangeMap[6].LastColor=RGB(150,150,150);
    ExchangeMap[7].LastColor=RGB(140,140,140);
    ExchangeMap[8].LastColor=RGB(130,130,130);
    ExchangeMap[9].LastColor=RGB(120,120,120);
    ExchangeMap[10].LastColor=RGB(110,110,110);
    ExchangeMap[11].LastColor=RGB(100,100,100);
    ExchangeMap[12].LastColor=RGB(90,90,90);
    ExchangeMap[13].LastColor=RGB(80,80,80);
    ExchangeMap[14].LastColor=RGB(70,70,70);

    ExchangeMap[0].BidColor=RGB(0,255,0);
    ExchangeMap[1].BidColor=RGB(51,204,103);
    ExchangeMap[2].BidColor=RGB(100,200,255);
    ExchangeMap[3].BidColor=RGB(59,118,177);
    ExchangeMap[4].BidColor=RGB(100,200,100);
    ExchangeMap[5].BidColor=RGB(0,64,150);
    ExchangeMap[6].BidColor=RGB(0,150,64);
    ExchangeMap[7].BidColor=RGB(0,140,0);
    ExchangeMap[8].BidColor=RGB(70,170,84);
    ExchangeMap[9].BidColor=RGB(50,150,64);
    ExchangeMap[10].BidColor=RGB(150,200,150);
    ExchangeMap[11].BidColor=RGB(0,154,255);
    ExchangeMap[12].BidColor=RGB(0,90,0);
    ExchangeMap[13].BidColor=RGB(0,255,255);
    ExchangeMap[14].BidColor=RGB(102,153,51);

    ExchangeMap[0].AskColor=RGB(255,0,0);
    ExchangeMap[1].AskColor=RGB(181,145,0);
    ExchangeMap[2].AskColor=RGB(255,200,100);
    ExchangeMap[3].AskColor=RGB(204,51,0);
    ExchangeMap[4].AskColor=RGB(200,100,100);
    ExchangeMap[5].AskColor=RGB(164,110,0);
    ExchangeMap[6].AskColor=RGB(150,64,0);
    ExchangeMap[7].AskColor=RGB(150,150,0);
    ExchangeMap[8].AskColor=RGB(170,84,70);
    ExchangeMap[9].AskColor=RGB(150,64,50);
    ExchangeMap[10].AskColor=RGB(200,150,150);
    ExchangeMap[11].AskColor=RGB(255,154,0);
    ExchangeMap[12].AskColor=RGB(255,150,150);
    ExchangeMap[13].AskColor=RGB(255,255,0);
    ExchangeMap[14].AskColor=RGB(153,51,102);
    
    Invalidate();
  }
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedBlackscheme()
{
    BackgroundBrush.DeleteObject();	
    StaticBrush.DeleteObject();	

    ChartBKBrush.DeleteObject();

	ChartBKColor=RGB(0,0,0);
    GridColor=RGB(80,80,80);
    TradeOutlineColor=RGB(255,255,255);
    BackgroundColor=RGB(54,54,54);
    TextColor=RGB(255,255,255);

    StaticBackgroundColor=RGB(110,110,110);
    StaticTextColor=RGB(255,255,255);         
    BackgroundBrush.CreateSolidBrush(BackgroundColor);	
    StaticBrush.CreateSolidBrush(StaticBackgroundColor);
    ChartBKBrush.CreateSolidBrush(ChartBKColor);	

    SeperatorColor=RGB(255,255,255);
    ChartOutlines=RGB(255,255,255);
    
    Invalidate();
}
//-------------------------------------------------------------
void CColorPrefs::OnBnClickedWhitescheme()
{
    BackgroundBrush.DeleteObject();	
    StaticBrush.DeleteObject();	
    ChartBKBrush.DeleteObject();

	ChartBKColor=RGB(255,255,255);
    GridColor=RGB(200,200,200);
    TradeOutlineColor=RGB(0,0,0);
    BackgroundColor=RGB(255,255,255);
    TextColor=RGB(0,0,0);
    StaticBackgroundColor=RGB(110,110,110);
    StaticTextColor=RGB(255,255,255);    
     
    BackgroundBrush.CreateSolidBrush(BackgroundColor);	
    StaticBrush.CreateSolidBrush(StaticBackgroundColor);
    ChartBKBrush.CreateSolidBrush(ChartBKColor);	

    SeperatorColor=RGB(0,0,0);
    ChartOutlines=RGB(25,25,25);
    
    Invalidate();
}


//-------------------------------------------------------------
void CColorPrefs::OnEnChangeMaxbars()
{  
  char str[12];
  MaxBarsControl.GetWindowText(str,12);	        
  if (strlen(str)>0) MaxBars=atoi(str);                       
  else               MaxBars=86400;
}

