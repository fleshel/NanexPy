/////////////////////////////////////////////////////////////////////////////
// QTSeqCharterPrice
// Author: Jeffrey Donovan
// Copyright 2012 by Nanex, LLC 
// LDM: 08-06-2012
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "QTSeqCharterPrice.h"
#include "math.h"

//------------------------------------------------------------
QTSeqCharterPrice::QTSeqCharterPrice()
{
    LastMaxPriceResize=0;
    LastMinPriceResize=0;    
    TradeOutlinePen.CreatePen(PS_SOLID,1,RGB(255,255,255));        
    TradeOutlineColor=RGB(255,255,255); 
}
//------------------------------------------------------------
QTSeqCharterPrice::~QTSeqCharterPrice()
{
  TradeOutlinePen.DeleteObject();
}
//------------------------------------------------------------
void QTSeqCharterPrice::SetTradeOutlineColor(COLORREF Color)
{
  TradeOutlinePen.DeleteObject();
  TradeOutlinePen.CreatePen(PS_SOLID,1,Color);        
  TradeOutlineColor=Color; 
}
//------------------------------------------------------------
void QTSeqCharterPrice::ResetZeroRange(void)
{
   JTools_SimpleChartTemplate::ResetZeroRange();
   LastMaxPriceResize=0;
   LastMinPriceResize=0;
}
//------------------------------------------------------------
void QTSeqCharterPrice::FindMaxBar(BarIndicatorType *Indicator)
{
   MaxBarHeight=MinBarHeight=0.0;

   if (!Indicator->Array[0]) return;

   if (Indicator->ScaleTo==3)
   {
     MaxBarHeight=Indicator->HardPriceMax;
     MinBarHeight=Indicator->HardPriceMin;
   }
   else
   {
     double TradePrice,BidPrice,AskPrice;

     for (int loop=StartBar;loop<=StopBar;loop++)
     {
       if (loop>0)
       {
         TradePrice=0.0;
         BidPrice=0.0;
         AskPrice=0.0;

         TradePrice=Indicator->Array[0]->Last[loop];

         if ((TradePrice>0.0)&&(TradePrice>MaxBarHeight))
         {
           MaxBarHeight=TradePrice;
           LastMaxPriceResize=loop;
         }
         else if ((TradePrice>0.0)&&((TradePrice<MinBarHeight)||(MinBarHeight==0.0)))
         {
           MinBarHeight=TradePrice;
           LastMinPriceResize=loop;
         }       
         if (Indicator->ScaleTo==1)
         {
           BidPrice=Indicator->Array[0]->BidLast[loop];
           
           if ((BidPrice>0.0)&&(BidPrice>MaxBarHeight))
           {
             MaxBarHeight=BidPrice;
             LastMaxPriceResize=loop;
           }
           else if ((BidPrice>0.0)&&((BidPrice<MinBarHeight)||(MinBarHeight==0.0)))
           {
             MinBarHeight=BidPrice;
             LastMinPriceResize=loop;
           }

           AskPrice=Indicator->Array[0]->AskLast[loop];
           
           if ((AskPrice>0.0)&&(AskPrice>MaxBarHeight))
           {
             MaxBarHeight=AskPrice;
             LastMaxPriceResize=loop;
           }
           else if ((AskPrice>0.0)&&((AskPrice<MinBarHeight)||(MinBarHeight==0.0)))
           {
             MinBarHeight=AskPrice;
             LastMinPriceResize=loop;
           }

           if (Indicator->UseBBOBands)
           {
             if ((Indicator->BBOBidBand[loop]>0.0)&&(Indicator->BBOBidBand[loop]>MaxBarHeight))
             {
               MaxBarHeight=Indicator->BBOBidBand[loop];
               LastMaxPriceResize=loop;
             }
             else if ((Indicator->BBOBidBand[loop]>0.0)&&((Indicator->BBOBidBand[loop]<MinBarHeight)||(MinBarHeight==0.0)))
             {
               MinBarHeight=Indicator->BBOBidBand[loop];
               LastMinPriceResize=loop;
             }
             if ((Indicator->BBOAskBand[loop]>0.0)&&(Indicator->BBOAskBand[loop]>MaxBarHeight))
             {
               MaxBarHeight=Indicator->BBOAskBand[loop];
               LastMaxPriceResize=loop;
             }
             else if ((Indicator->BBOAskBand[loop]>0.0)&&((Indicator->BBOAskBand[loop]<MinBarHeight)||(MinBarHeight==0.0)))
             {
               MinBarHeight=Indicator->BBOAskBand[loop];
               LastMinPriceResize=loop;
             }
           }           
         }
         else if (Indicator->ScaleTo==2)
         {
           for (int loop2=1;loop2<MAXEXCHANGES;loop2++)
           {
             if (Indicator->Array[loop2]->TradeOn)
             {
               TradePrice=0.0;
               TradePrice=Indicator->Array[loop2]->Last[loop];
               
               if ((TradePrice>0.0)&&(BidPrice>MaxBarHeight))
               {
                 MaxBarHeight=TradePrice;
                 LastMaxPriceResize=loop;
               }
               else if ((TradePrice>0.0)&&((TradePrice<MinBarHeight)||(MinBarHeight==0.0)))
               {
                 MinBarHeight=TradePrice;
                 LastMinPriceResize=loop;
               }
             }
             if (Indicator->Array[loop2]->BidOn)
             {
               BidPrice=0.0;
               BidPrice=Indicator->Array[loop2]->BidLast[loop];
               
               if ((BidPrice>0.0)&&(BidPrice>MaxBarHeight))
               {
                 MaxBarHeight=BidPrice;
                 LastMaxPriceResize=loop;
               }
               else if ((BidPrice>0.0)&&((BidPrice<MinBarHeight)||(MinBarHeight==0.0)))
               {
                 MinBarHeight=BidPrice;
                 LastMinPriceResize=loop;
               }
             }
             if (Indicator->Array[loop2]->AskOn)
             {
               AskPrice=0.0;
               AskPrice=Indicator->Array[loop2]->AskLast[loop];
               
               if ((AskPrice>0.0)&&(AskPrice>MaxBarHeight))
               {
                 MaxBarHeight=AskPrice;
                 LastMaxPriceResize=loop;
               }
               else if ((AskPrice>0.0)&&((AskPrice<MinBarHeight)||(MinBarHeight==0.0)))
               {
                 MinBarHeight=AskPrice;
                 LastMinPriceResize=loop;
               }
             }
           }
         }
       }
     }
   }

   double inc_amount=(MaxBarHeight-MinBarHeight)/10.0; 
   if (inc_amount>10.0) inc_amount=ceil(inc_amount);
   else if (inc_amount>5.0)  inc_amount=10.0;
   else if (inc_amount>4.0)  inc_amount=5.0;
   else if (inc_amount>2.5)  inc_amount=2.5;
   else if (inc_amount>2.0)  inc_amount=2.0;
   else if (inc_amount>1.0)  inc_amount=1.0;
   else if (inc_amount>0.50) inc_amount=0.50;
   else if (inc_amount>0.25) inc_amount=0.25; 
   else if (inc_amount>0.10) inc_amount=0.10; 
   else if (inc_amount>0.050) inc_amount=0.050; 
   else if (inc_amount>0.010) inc_amount=0.010; 
   else if (inc_amount>0.005) inc_amount=0.005; 
   else if (inc_amount>0.0025) inc_amount=0.0025; 
   else inc_amount=0.0001;   

   while (((MaxBarHeight-MinBarHeight) / inc_amount) > 10)
   {
      if (inc_amount==0.0001) inc_amount=0.0025;
	  if (inc_amount==0.01) inc_amount=0.05;
      else inc_amount*=2.0; 
   }
   IncAmount=inc_amount;

   if (MaxBarHeight > (int)(MaxBarHeight / inc_amount) * inc_amount)
     MaxBarHeight=(int)(MaxBarHeight / inc_amount) * inc_amount + inc_amount;
   else
     MaxBarHeight=((int)(MaxBarHeight / inc_amount) * inc_amount);

  MinBarHeight=((int)(MinBarHeight / inc_amount) * inc_amount);

  if ((MaxBarHeight-MinBarHeight)!=0.0)
     RangeDiv=(double) StudyHeight / (MaxBarHeight-MinBarHeight);
  else RangeDiv=1.0;	     
}
//------------------------------------------------------------
void QTSeqCharterPrice::DrawStudy(BarIndicatorType *Indicator,
                                  ColorIndicatorType *Colors,
                                  unsigned char UseISO,
                                  int PointExpand,int PointExpand2)
{
   int loop,maxy,y1,y2;
   double CurrXNorm;         
         
   CPen *OldPen=(CPen *)NewDC.SelectObject(&TradeOutlinePen);
   CBrush *OldBrush=(CBrush *)NewDC.SelectObject(&ChartBKBrush);
   NewDC.SelectClipRgn(&ClipRgn);               
   
   // Figure graph min's and max's   
   maxy=StudyY+StudyHeight;          
   CurrXNorm=StudyX;   

   int ERadius=PointExpand2;
   if (ERadius<1) ERadius=1;
   if (ERadius>6) ERadius=6;
   int DoTLine=1;
   int DoBLine=1;
   int DoALine=1;

   double TradePrice,BidPrice,AskPrice;
   double LastTradePrice[MAXEXCHANGES],LastBidPrice[MAXEXCHANGES],LastAskPrice[MAXEXCHANGES];

   memset(LastTradePrice,0,sizeof(double)*MAXEXCHANGES);
   memset(LastBidPrice,0,sizeof(double)*MAXEXCHANGES);
   memset(LastAskPrice,0,sizeof(double)*MAXEXCHANGES);
   
   for (loop=StartBar;loop<=StopBar;loop++)
   {                 
     if (loop>0)
     {          
       // Loop exchanges backwards to Composite and NBBO are always on top
       for (int loop2=MAXEXCHANGES-1;loop2>=0;loop2--)
       {
          if (loop2==0)
          {
            DoBLine=Indicator->DrawCBidLine;
            DoALine=Indicator->DrawCAskLine;
          }
          else
          {
            DoBLine=Indicator->DrawBidLine;
            DoALine=Indicator->DrawAskLine;
          }

          if (Indicator->Array[loop2]->BidOn)
          {  
            BidPrice=Indicator->Array[loop2]->BidLast[loop];
            if (BidPrice==0.0) BidPrice=LastBidPrice[loop2];
                        
            NewDC.SelectObject(&Colors->Colors[loop2]->BidPen);
            
            if (DoBLine)
            {
              if ((BidPrice!=0.0)&&(LastBidPrice[loop2]!=0.0))
              {
                 y1=(int)(maxy-((LastBidPrice[loop2]-MinBarHeight)*RangeDiv));					    
                 y2=(int)(maxy-((BidPrice-MinBarHeight)*RangeDiv));					    
             
                 NewDC.MoveTo(CurrXNorm-DoubleBarSpacing,y1);                 
                 if (Indicator->CircuitStyle) NewDC.LineTo(CurrXNorm,y1);
                 NewDC.LineTo(CurrXNorm,y2);
              }
              LastBidPrice[loop2]=BidPrice;
            }
            else if (Indicator->Array[loop2]->BidLast[loop]!=0.0)
            {
              NewDC.SelectObject(&Colors->Colors[loop2]->BidBrush);
              y2=(int)(maxy-((BidPrice-MinBarHeight)*RangeDiv));					    
              NewDC.Ellipse(CurrXNorm-ERadius,y2-ERadius,CurrXNorm+ERadius,y2+ERadius);
            }
          }

          if (Indicator->Array[loop2]->AskOn)
          {  
            AskPrice=Indicator->Array[loop2]->AskLast[loop];
            if (AskPrice==0.0) AskPrice=LastAskPrice[loop2];
            
            NewDC.SelectObject(&Colors->Colors[loop2]->AskPen);
            if (DoALine)
            {
              if ((AskPrice!=0.0)&&(LastAskPrice[loop2]!=0.0))
              {
                 y1=(int)(maxy-((LastAskPrice[loop2]-MinBarHeight)*RangeDiv));					    
                 y2=(int)(maxy-((AskPrice-MinBarHeight)*RangeDiv));					    
             
                 NewDC.MoveTo(CurrXNorm-DoubleBarSpacing,y1);
                 if (Indicator->CircuitStyle) NewDC.LineTo(CurrXNorm,y1);
                 NewDC.LineTo(CurrXNorm,y2);
              }     
              LastAskPrice[loop2]=AskPrice;
            }
            else if (Indicator->Array[loop2]->AskLast[loop]!=0.0)
            {
              NewDC.SelectObject(&Colors->Colors[loop2]->AskBrush);
              y2=(int)(maxy-((AskPrice-MinBarHeight)*RangeDiv));					    
              NewDC.Ellipse(CurrXNorm-ERadius,y2-ERadius,CurrXNorm+ERadius,y2+ERadius);
            }
          }          
       }
       if (Indicator->UseBBOBands)
       {
          if ((Indicator->BBOBidBand[loop-1]!=0.0)&&(Indicator->BBOBidBand[loop]!=0.0))
          {
            NewDC.SelectObject(&Colors->BBOBidBandPen);
            y1=(int)(maxy-((Indicator->BBOBidBand[loop-1]-MinBarHeight)*RangeDiv));					    
            y2=(int)(maxy-((Indicator->BBOBidBand[loop]-MinBarHeight)*RangeDiv));					    
            
            NewDC.MoveTo(CurrXNorm-DoubleBarSpacing,y1);                 
            if (Indicator->CircuitStyle) NewDC.LineTo(CurrXNorm,y1);
            NewDC.LineTo(CurrXNorm,y2);
          }
          if ((Indicator->BBOAskBand[loop-1]!=0.0)&&(Indicator->BBOAskBand[loop]!=0.0))
          {
            NewDC.SelectObject(&Colors->BBOAskBandPen);
            y1=(int)(maxy-((Indicator->BBOAskBand[loop-1]-MinBarHeight)*RangeDiv));					    
            y2=(int)(maxy-((Indicator->BBOAskBand[loop]-MinBarHeight)*RangeDiv));					    
            
            NewDC.MoveTo(CurrXNorm-DoubleBarSpacing,y1);                 
            if (Indicator->CircuitStyle) NewDC.LineTo(CurrXNorm,y1);
            NewDC.LineTo(CurrXNorm,y2);
          }
       }
     }
     CurrXNorm+=DoubleBarSpacing;  
   }
    
   // DO Trades AFTER all quotes
   CurrXNorm=StudyX;   
   for (loop=StartBar;loop<=StopBar;loop++)
   {                 
     if (loop>0)
     {          
       // Loop exchanges backwards to Composite and NBBO are always on top
       for (int loop2=MAXEXCHANGES-1;loop2>=0;loop2--)
       {
          // If composite trades are on then no need to do the others, composite will overwrite
          if ((loop2>0)&&((Indicator->Array[0]->TradeOn)&&(!Indicator->DrawCTradeLine))) 
          {
            continue;
          }

          if (loop2==0)
            DoTLine=Indicator->DrawCTradeLine;
          else
            DoTLine=Indicator->DrawTradeLine;

          if (Indicator->Array[loop2]->TradeOn)
          {           
            
            NewDC.SelectObject(&Colors->Colors[loop2]->LastBrush);

            TradePrice=Indicator->Array[loop2]->Last[loop];
            if (TradePrice==0.0) TradePrice=LastTradePrice[loop2];
            
            // Only allow lines for composite trades
            if ((DoTLine)&&(loop2==0))
            {             
              if ((TradePrice!=0.0)&&(LastTradePrice[loop2]!=0.0))
              {
                 NewDC.SelectObject(&Colors->Colors[loop2]->LastPen);
                 y1=(int)(maxy-((LastTradePrice[loop2]-MinBarHeight)*RangeDiv));					    
                 y2=(int)(maxy-((TradePrice-MinBarHeight)*RangeDiv));					    
             
                 NewDC.MoveTo(CurrXNorm-DoubleBarSpacing,y1);
                 if (Indicator->CircuitStyle) NewDC.LineTo(CurrXNorm,y1);
                 NewDC.LineTo(CurrXNorm,y2);
              }     
              LastTradePrice[loop2]=TradePrice;
            }
            else if (TradePrice!=0.0)
            {            
              NewDC.SelectObject(&TradeOutlinePen);
              y2=(int)(maxy-((TradePrice-MinBarHeight)*RangeDiv));					    

              if ((UseISO)&&(Indicator->Array[loop2]->TCondition[loop]==95))
              {
                 POINT TriPoints[3];
                 TriPoints[0].x=CurrXNorm;
                 TriPoints[0].y=(int) y2-(3+PointExpand);
                 TriPoints[1].x=CurrXNorm-(3+PointExpand);
                 TriPoints[1].y=(int) y2+(2+PointExpand);
                 TriPoints[2].x=CurrXNorm+(3+PointExpand);
                 TriPoints[2].y=(int) y2+(2+PointExpand);
                 NewDC.Polygon((const POINT *)&TriPoints,3);
              }
              else
              {
                if (PointExpand<4)         
                   NewDC.Rectangle(CurrXNorm-(3+PointExpand),
                        (int) y2-(3+PointExpand),
                        CurrXNorm+(3+PointExpand),
                        (int) y2+(3+PointExpand));             
                 else
                   NewDC.Ellipse(CurrXNorm-(3+PointExpand),
                        (int) y2-(3+PointExpand),
                        CurrXNorm+(3+PointExpand),
                        (int) y2+(3+PointExpand));
              }
            }
          }
       }
     }
     CurrXNorm+=DoubleBarSpacing;  
   }


   if (OldPen) NewDC.SelectObject(OldPen);	
   if (OldBrush) NewDC.SelectObject(OldBrush);	
   NewDC.SelectClipRgn(NULL);
}
//-------------------------------------------------------------
void QTSeqCharterPrice::DrawScale(BarIndicatorType *Indicator,unsigned char TradePriceType,unsigned char ShowGrid)
{
    double inc_amount,now_amount;
    int number1;    
    char str[255];
      
	int CTop       =StudyY;
    CPen *OldPen;   
    CFont *OldFont;
    
    NewDC.SetTextColor(WindowTextColor);
	NewDC.SetBkColor(WindowBKColor);
    OldPen=NewDC.SelectObject(&ChartOutlinePen);
    if (ChartFont) OldFont=NewDC.SelectObject(ChartFont);
	
    int maxy=StudyY+StudyHeight;       
    int ScaleX=ChartingX+ChartingWidth;
   
    if (Indicator->ScaleTo<3)
    {
      if (StudyHeight<75)       inc_amount=IncAmount*4;
      else if (StudyHeight<175) inc_amount=IncAmount*2;
      else                      inc_amount=IncAmount;
    }
    else inc_amount=Indicator->HardPriceScale;

    now_amount=MinBarHeight;
    
    if ((MaxBarHeight!=0.0)&&(inc_amount!=0.0))
    {
        while (now_amount <= MaxBarHeight+(inc_amount))
		{          
		    number1=(int)(maxy-((now_amount-MinBarHeight)*RangeDiv));	
            if (number1>=ChartingY)
            {
              NewDC.SetBkColor(WindowBKColor);
              switch(TradePriceType)
              {
                case 8: sprintf(str,"%0.2f",now_amount);break;
                case 7: sprintf(str,"%0.3f",now_amount);break;
                case 6: sprintf(str,"%0.4f",now_amount);break;
                case 5: sprintf(str,"%0.5f",now_amount);break;
                default:sprintf(str,"%0.2f",now_amount);break;
              }
				            
		      NewDC.TextOut(12 + ScaleX,(int) number1-7,str,(int)strlen(str));	 	 
			  
              NewDC.SelectObject(&ChartOutlinePen);
              NewDC.MoveTo(ScaleX+3,number1);
              NewDC.LineTo(ScaleX+6,number1);

              if (ShowGrid)
              {
                NewDC.SetBkColor(ChartBKColor);
                NewDC.SelectObject(&StudyGridPen);            
                NewDC.MoveTo(ChartingX+1,number1);
                NewDC.LineTo(ScaleX-1,number1);
              }
            }
            now_amount+=inc_amount;
	    }
    }
    
    if (OldPen) NewDC.SelectObject(OldPen);    
    if (OldFont) NewDC.SelectObject(OldFont);
}
