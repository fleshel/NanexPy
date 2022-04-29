/////////////////////////////////////////////////////////////////////////////
//
// LDM: 07-31-2012
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __QTSEQCHARTTYPE_H
#define __QTSEQCHARTTYPE_H
/////////////////////////////////////////////////////////////////////////////

#define MAXEXCHANGES 17

typedef struct
{  
  double *Last,*BidLast,*AskLast;
  unsigned char *TCondition;
  unsigned int *Volume,*BidVolume,*AskVolume;  
  unsigned char TradeOn,BidOn,AskOn;
  unsigned char UToggleTradeOff,UToggleBidOff,UToggleAskOff;    
  unsigned long TotalTradeCount,TotalQuoteCount;
}QTRecordType;

typedef struct
{
  QTRecordType *Array[MAXEXCHANGES];      
  unsigned char *Hour,*Min,*Sec;
  unsigned short *MSec;   
  unsigned long *MSTOD;
  double PriceGridDivisor;
  
  unsigned char DrawTradeLine;
  unsigned char DrawBidLine;
  unsigned char DrawAskLine;

  unsigned char DrawCTradeLine;
  unsigned char DrawCBidLine;
  unsigned char DrawCAskLine;

  unsigned char ScaleTo;
  unsigned char CircuitStyle;

  unsigned char VolScaleTo;
  double HardPriceMax,HardPriceMin,HardPriceScale;
  unsigned long HardVolMax,HardVolScale;

  double BBOBandAmount;
  unsigned char UseBBOBands;
  double *BBOAskBand,*BBOBidBand;

}BarIndicatorType;

typedef struct
{
  CPen LastPen,BidPen,AskPen;
  CBrush LastBrush,BidBrush,AskBrush;
}ColorRecordType;

typedef struct
{
  ColorRecordType *Colors[MAXEXCHANGES];
  COLORREF BBOAskBandColor,BBOBidBandColor;
  CPen BBOAskBandPen,BBOBidBandPen;
}ColorIndicatorType;

typedef struct
{  
  int Exchange;
  char Title[8];
  COLORREF LastColor;
  COLORREF BidColor;
  COLORREF AskColor;
  int PenWidth;
}ExchangeMapType;

#endif // __FWXPRICECHART_H

