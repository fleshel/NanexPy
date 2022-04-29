/////////////////////////////////////////////////////////////////////////////
// QTSeqCharterLegend
// Author: Jeffrey Donovan
// Copyright 2012 by Nanex, LLC 
// LDM: 08-05-2012
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "QTSeqCharterLegend.h"
#include "..\JT_CommonGUI\JT_NumFormatters.h"          
#include "math.h"

//------------------------------------------------------------
QTSeqCharterLegend::QTSeqCharterLegend()
{
    LastMaxSizeResize=0;        
    AnyTradesOn=AnyBidsOn=AnyAsksOn=0;
}
//------------------------------------------------------------
QTSeqCharterLegend::~QTSeqCharterLegend()
{
}
//------------------------------------------------------------
void QTSeqCharterLegend::ResetZeroRange(void)
{
   JTools_SimpleChartTemplate::ResetZeroRange();
   LastMaxSizeResize=0;
}
//------------------------------------------------------------
void QTSeqCharterLegend::FindMaxBar(BarIndicatorType *Indicator)
{
  LegendStudyHeight=FontHeight*3;
  AnyTradesOn=AnyBidsOn=AnyAsksOn=0;
  for (int loop=1;loop<MAXEXCHANGES;loop++)
  {
    if (Indicator->Array[loop])
    {
       if (((Indicator->Array[loop]->TradeOn)&&(!Indicator->Array[loop]->UToggleTradeOff))||
          ((Indicator->Array[loop]->BidOn)&&(!Indicator->Array[loop]->UToggleBidOff))||
          ((Indicator->Array[loop]->AskOn)&&(!Indicator->Array[loop]->UToggleAskOff)))
          LegendStudyHeight+=FontHeight;

       if ((Indicator->Array[loop]->TradeOn)&&(!Indicator->Array[loop]->UToggleTradeOff)) AnyTradesOn=1;
       if ((Indicator->Array[loop]->BidOn)&&(!Indicator->Array[loop]->UToggleBidOff)) AnyBidsOn=1;
       if ((Indicator->Array[loop]->AskOn)&&(!Indicator->Array[loop]->UToggleAskOff)) AnyAsksOn=1;
    }
  }

  if (Indicator->Array[0])
  {
    int CompositeStudyHeight=FontHeight*3;
    if ((Indicator->Array[0]->TradeOn)&&(!Indicator->Array[0]->UToggleTradeOff)) CompositeStudyHeight+=FontHeight;
    if ((Indicator->Array[0]->BidOn)&&(!Indicator->Array[0]->UToggleBidOff)) CompositeStudyHeight+=FontHeight;
    if ((Indicator->Array[0]->AskOn)&&(!Indicator->Array[0]->UToggleAskOff)) CompositeStudyHeight+=FontHeight;
  
    if (CompositeStudyHeight>LegendStudyHeight)
      LegendStudyHeight=CompositeStudyHeight;
  }
}
//------------------------------------------------------------
void QTSeqCharterLegend::DrawStudy(BarIndicatorType *Indicator,
                                         ColorIndicatorType *Colors,
                                         ExchangeMapType *ExchangeMap,
                                         unsigned char UseISO,int PointExpand,int PointExpand2)
{  
  CFont *OldFont=NULL;
  CPen *OldPen=NewDC.SelectObject(&ChartOutlinePen);
  CBrush *OldBrush=NewDC.SelectObject(&WindowBKBrush);
  if (ChartFont) OldFont=NewDC.SelectObject(ChartFont);
  NewDC.SelectClipRgn(&ClipRgn);               

  int BidOffset=0,AskOffset=0,ExgOffset=0,StatOffset1=0,StatOffset2=0,BandingOffset=0;
  
  IncAmount=FontWidth*5;
  int StartX=StudyX+10;
  int StartY=StudyY+FontHeight+4;
  int FY2=FontHeight/2-2;
  int StatsOn=0;

  StatOffset1+=FontWidth*15;
  StatOffset2+=FontWidth*20;

  NewDC.SetTextColor(ChartTextColor);
  if ((AnyTradesOn)&&((!Indicator->Array[0]->TradeOn)||(Indicator->DrawCTradeLine)))
  {    
    NewDC.SetBkColor(ChartBKColor);
    NewDC.TextOutA(StartX,StartY,"Trd",strlen("Trd"));    
    BidOffset+=IncAmount;AskOffset+=IncAmount;ExgOffset+=IncAmount;StatOffset1+=IncAmount;StatOffset2+=IncAmount;

    if (UseISO)
    {
      NewDC.SetBkColor(ChartBKColor);
      NewDC.TextOutA(StartX+IncAmount,StartY,"ISO",strlen("ISO"));    
      BidOffset+=IncAmount;AskOffset+=IncAmount;ExgOffset+=IncAmount;StatOffset1+=IncAmount;StatOffset2+=IncAmount;
    }
  }
  if (AnyBidsOn)
  {   
    NewDC.SetBkColor(ChartBKColor);
    NewDC.TextOutA(StartX+BidOffset,StartY,"Bid",strlen("Bid"));    
    AskOffset+=IncAmount;ExgOffset+=IncAmount;StatOffset1+=IncAmount;StatOffset2+=IncAmount;
  }
  if (AnyAsksOn)
  {   
    NewDC.SetBkColor(ChartBKColor);
    NewDC.TextOutA(StartX+AskOffset,StartY,"Ask",strlen("Ask"));    
    ExgOffset+=IncAmount;StatOffset1+=IncAmount;StatOffset2+=IncAmount;
  }
  if ((AnyTradesOn)||(AnyBidsOn)||(AnyAsksOn))
  { 
    if ((AnyBidsOn)||(AnyAsksOn))
    {
      NewDC.TextOutA(StartX+StatOffset1-(FontWidth*3),StartY,"# Quotes",strlen("# Quotes"));    
      StatOffset2+=FontWidth*5;
    }
    if ((AnyTradesOn)||(Indicator->Array[0]->TradeOn))
      NewDC.TextOutA(StartX+StatOffset2-(FontWidth*3),StartY,"# Trades",strlen("# Trades"));    
     
    NewDC.TextOutA(StartX+StatOffset1-(FontWidth*3),StudyY,"Reporting Exg Stats",strlen("Reporting Exg Stats"));        
    
    StatOffset1+=IncAmount;
    StatOffset2+=IncAmount;

    NewDC.SetBkColor(ChartBKColor);
    NewDC.TextOutA(StartX,StudyY,"ColorLgnd",strlen("ColorLgnd"));        
    NewDC.TextOutA(StartX+ExgOffset,StartY,"Exg",strlen("Exg"));    
  }
 
  int CurrY=StartY+(FontHeight*2);  
  char Stat[256];
  for (int loop=1;loop<MAXEXCHANGES;loop++)
  {
    if (Indicator->Array[loop])
    {
      if (Indicator->Array[loop]->TradeOn)
      {
        NewDC.SelectObject(&ChartOutlinePen);
        NewDC.SelectObject(&Colors->Colors[loop]->LastBrush);
        if (PointExpand<4)
          NewDC.Rectangle(StartX,
                     CurrY,
                     StartX+10,
                     CurrY+FontHeight-2);    
        else
          NewDC.Ellipse(StartX,CurrY,StartX+10,CurrY+FontHeight-2);            

        if (UseISO)
        {
          POINT TriPoints[3];
          TriPoints[0].x=StartX+IncAmount+5;
          TriPoints[0].y=(int) CurrY;
          TriPoints[1].x=StartX+IncAmount;
          TriPoints[1].y=(int) CurrY+(FontHeight-4);
          TriPoints[2].x=StartX+IncAmount+10;
          TriPoints[2].y=(int) CurrY+(FontHeight-4);
          NewDC.Polygon((const POINT *)&TriPoints,3);
        }
      }
      if (Indicator->Array[loop]->BidOn)
      {
        NewDC.FillSolidRect(StartX+BidOffset,CurrY,10,FontHeight-2,ExchangeMap[loop].BidColor);            
      }
      if (Indicator->Array[loop]->AskOn)
      {
        NewDC.FillSolidRect(StartX+AskOffset,CurrY,10,FontHeight-2,ExchangeMap[loop].AskColor);            
      }
      if ((Indicator->Array[loop]->TradeOn)||(Indicator->Array[loop]->BidOn)||(Indicator->Array[loop]->AskOn))
      {
        NewDC.SetBkColor(ChartBKColor);
        NewDC.TextOutA(StartX+ExgOffset,CurrY-FY2,ExchangeMap[loop].Title,strlen(ExchangeMap[loop].Title));    
        
        if ((AnyTradesOn)||(Indicator->Array[0]->TradeOn))
        {
          SetTextAlign(NewDC,TA_RIGHT);    
          FormatIntWCommas(Indicator->Array[loop]->TotalTradeCount,Stat);
          NewDC.TextOutA(StartX+StatOffset2,CurrY-FY2,Stat,strlen(Stat));    
          SetTextAlign(NewDC,TA_LEFT);          
          StatsOn=1;
        }

        if ((AnyBidsOn)||(AnyAsksOn))
        {
          SetTextAlign(NewDC,TA_RIGHT);    
          FormatIntWCommas(Indicator->Array[loop]->TotalQuoteCount,Stat);
          NewDC.TextOutA(StartX+StatOffset1,CurrY-FY2,Stat,strlen(Stat));    
          SetTextAlign(NewDC,TA_LEFT);    
          StatsOn=1;
        }
        CurrY+=FontHeight;        
      }      
    }
  }
  int StatOffset=0;
  if (StatsOn) 
  {
    if ((AnyTradesOn)||(Indicator->Array[0]->TradeOn)) StatOffset=StatOffset2+10;
    else StatOffset=StatOffset1+10;
  }
  else StatOffset=0;
  
  if (Indicator->Array[0])
  {   
    if ((AnyTradesOn)||(AnyBidsOn)||(AnyAsksOn)) 
    {
      CurrY=StudyY+FontHeight+4;
      StartX=StudyX+StatOffset+60;
    }
    else
    {
      CurrY=StudyY+4;
      StartX=StudyX+10;
    }
    BandingOffset=StartX+130;

    if ((Indicator->Array[0]->TradeOn)||(Indicator->Array[0]->BidOn)||(Indicator->Array[0]->AskOn))
    {           
      NewDC.SetBkColor(ChartBKColor);
      NewDC.TextOutA(StartX,CurrY,"Composite/NBBO",strlen("Composite/NBBO"));    
      CurrY+=(FontHeight*2);

      if ((Indicator->Array[0]->TradeOn)&&(!Indicator->Array[0]->UToggleTradeOff)) 
      {                   
        if ((PointExpand<4)||(Indicator->DrawCTradeLine))
        {
          if (Indicator->DrawCTradeLine)
            NewDC.FillSolidRect(StartX,CurrY,10,FontHeight-2,ExchangeMap[0].LastColor);
          else
          {
            NewDC.SelectObject(&ChartOutlinePen);
            NewDC.SelectObject(&Colors->Colors[0]->LastBrush);
            NewDC.Rectangle(StartX,CurrY,StartX+10,CurrY+FontHeight-2);    
          }
        }
        else
        {
          NewDC.SelectObject(&ChartOutlinePen);
          NewDC.SelectObject(&Colors->Colors[0]->LastBrush);
          NewDC.Ellipse(StartX,CurrY,StartX+10,CurrY+FontHeight-2);            
        }

        NewDC.SetBkColor(ChartBKColor);
        NewDC.TextOutA(StartX+15,CurrY-FY2,"Composite Trades",strlen("Composite Trades"));    
        CurrY+=FontHeight;

        if (UseISO)
        {
          POINT TriPoints[3];
          TriPoints[0].x=StartX+5;
          TriPoints[0].y=(int) CurrY;
          TriPoints[1].x=StartX;
          TriPoints[1].y=(int) CurrY+(FontHeight-4);
          TriPoints[2].x=StartX+10;
          TriPoints[2].y=(int) CurrY+(FontHeight-4);
          NewDC.Polygon((const POINT *)&TriPoints,3);
          NewDC.SetBkColor(ChartBKColor);
          if (Indicator->UseBBOBands)
            NewDC.TextOutA(StartX+15,CurrY-FY2,"Composite ISO Trds",strlen("Composite ISO Trds"));    
          else
            NewDC.TextOutA(StartX+15,CurrY-FY2,"Composite ISO Trades",strlen("Composite ISO Trades"));    
          CurrY+=FontHeight;
        }
      }
      if ((Indicator->Array[0]->BidOn)&&(!Indicator->Array[0]->UToggleBidOff)) 
      {           
        if (Indicator->DrawCBidLine)
          NewDC.FillSolidRect(StartX,CurrY,10,FontHeight-2,ExchangeMap[0].BidColor);
        else
        {
          NewDC.SelectObject(&Colors->Colors[0]->BidPen);
          NewDC.SelectObject(&Colors->Colors[0]->BidBrush);
          NewDC.Ellipse(StartX,CurrY,StartX+10,CurrY+FontHeight-2);            
        }
        
        NewDC.SetBkColor(ChartBKColor);
        NewDC.TextOutA(StartX+15,CurrY-FY2,"National Best Bid",strlen("National Best Bid"));    
        CurrY+=FontHeight;
      }
      if ((Indicator->Array[0]->AskOn)&&(!Indicator->Array[0]->UToggleAskOff)) 
      {            
        if (Indicator->DrawCAskLine)
          NewDC.FillSolidRect(StartX,CurrY,10,FontHeight-2,ExchangeMap[0].AskColor);
        else
        {
          NewDC.SelectObject(&Colors->Colors[0]->AskPen);
          NewDC.SelectObject(&Colors->Colors[0]->AskBrush);
          NewDC.Ellipse(StartX,CurrY,StartX+10,CurrY+FontHeight-2);            
        }

        NewDC.SetBkColor(ChartBKColor);
        NewDC.TextOutA(StartX+15,CurrY-FY2,"National Best Ask",strlen("National Best Ask"));    
      }
    }

    if (Indicator->UseBBOBands)
    {
      if ((AnyTradesOn)||(AnyBidsOn)||(AnyAsksOn))     
         CurrY=StudyY+FontHeight+4;          
      else    
        CurrY=StudyY+4;
      
      StartX=BandingOffset+IncAmount;

      char str[32];
      NewDC.SetBkColor(ChartBKColor);
      NewDC.TextOutA(StartX,CurrY,"NBBO Banding",strlen("NBBO Banding"));    
      CurrY+=(FontHeight*2);

      NewDC.FillSolidRect(StartX,CurrY,10,FontHeight-2,Colors->BBOAskBandColor);            
      NewDC.SetBkColor(ChartBKColor);
      sprintf(str,"%0.1f %% NBBO Ask Band",Indicator->BBOBandAmount*100.0);
      NewDC.TextOutA(StartX+15,CurrY-FY2,str,strlen(str));    
      CurrY+=FontHeight;
      NewDC.FillSolidRect(StartX,CurrY,10,FontHeight-2,Colors->BBOBidBandColor);            
      NewDC.SetBkColor(ChartBKColor);
      sprintf(str,"%0.1f %% NBBO Bid Band",Indicator->BBOBandAmount*100.0);
      NewDC.TextOutA(StartX+15,CurrY-FY2,str,strlen(str));    
      CurrY+=FontHeight;
    }
  }
  
  if (OldFont) NewDC.SelectObject(OldFont);
  if (OldPen)  NewDC.SelectObject(OldPen);
  if (OldBrush)  NewDC.SelectObject(OldBrush);
  NewDC.SelectClipRgn(NULL);               
}
