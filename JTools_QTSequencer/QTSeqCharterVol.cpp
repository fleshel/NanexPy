/////////////////////////////////////////////////////////////////////////////
// QTSeqCharterVol
// Author: Jeffrey Donovan
// Copyright 2012 by Nanex, LLC 
// LDM: 07-31-2012
/////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "QTSeqCharterVol.h"
#include "math.h"

//------------------------------------------------------------
QTSeqCharterVol::QTSeqCharterVol()
{
    LastMaxSizeResize=0;        
    StudyType=0;
}
//------------------------------------------------------------
QTSeqCharterVol::~QTSeqCharterVol()
{
    TradeOutlinePen.DeleteObject();
}
//------------------------------------------------------------
void QTSeqCharterVol::ResetZeroRange(void)
{
   JTools_SimpleChartTemplate::ResetZeroRange();
   LastMaxSizeResize=0;
}
//------------------------------------------------------------
void QTSeqCharterVol::SetStudyType(int Type)
{
  StudyType=Type;
}
//------------------------------------------------------------
void QTSeqCharterVol::SetTradeOutlineColor(COLORREF Color)
{
   TradeOutlineColor=Color;
   TradeOutlinePen.DeleteObject();
   TradeOutlinePen.CreatePen(PS_SOLID,1,Color);      
}
//------------------------------------------------------------
void QTSeqCharterVol::FindMaxBar(BarIndicatorType *Indicator)
{
   MaxBarHeight=0.0;
   MinBarHeight=0.0;

   if (!Indicator->Array[0]) return;

   if (Indicator->VolScaleTo==1)
   {
     MaxBarHeight=Indicator->HardVolMax;
   }
   else
   {
      for (int loop=StartBar;loop<=StopBar;loop++)
      {
        if (loop>=0)
        {
          for (int loop2=0;loop2<MAXEXCHANGES;loop2++)
          {
            // NOTE: YES, we WANT >= so that LastMaxSizeResize is set to the
            // last possible bar.
            if ((!StudyType)&&(Indicator->Array[loop2]->TradeOn)&&(Indicator->Array[loop2]->Volume[loop]>=MaxBarHeight))
            {
              MaxBarHeight=Indicator->Array[loop2]->Volume[loop];
              LastMaxSizeResize=loop;
            }
            if ((Indicator->Array[loop2]->BidOn)&&(Indicator->Array[loop2]->BidVolume[loop]>=MaxBarHeight)) 
            {
              MaxBarHeight=Indicator->Array[loop2]->BidVolume[loop];
              LastMaxSizeResize=loop;
            }
            if ((Indicator->Array[loop2]->AskOn)&&(Indicator->Array[loop2]->AskVolume[loop]>=MaxBarHeight))
            {
              MaxBarHeight=Indicator->Array[loop2]->AskVolume[loop];
              LastMaxSizeResize=loop;
            }
          }
        }
     }     
   }

   if (StudyType==1) MaxBarHeight*=2.0;

   double inc_amount=1;
   if      (MaxBarHeight < 10 )        inc_amount=1;
   else if (MaxBarHeight < 50 )        inc_amount=5;
   else if (MaxBarHeight < 100)        inc_amount=10;
   else if (MaxBarHeight < 250)        inc_amount=25;
   else if (MaxBarHeight < 500)        inc_amount=50;
   else if (MaxBarHeight < 1000)       inc_amount=100;
   else if (MaxBarHeight < 2500)       inc_amount=250;
   else if (MaxBarHeight < 5000)       inc_amount=500;
   else if (MaxBarHeight < 10000)      inc_amount=1000;
   else if (MaxBarHeight < 25000)      inc_amount=2500;
   else if (MaxBarHeight < 50000)      inc_amount=5000;
   else if (MaxBarHeight < 100000)     inc_amount=10000;
   else if (MaxBarHeight < 250000)     inc_amount=25000;
   else if (MaxBarHeight < 500000)     inc_amount=50000;
   else if (MaxBarHeight < 1000000)    inc_amount=100000;
   else if (MaxBarHeight < 2500000)    inc_amount=250000;
   else if (MaxBarHeight < 5000000)    inc_amount=500000;
   else if (MaxBarHeight < 10000000)   inc_amount=1000000;
   else if (MaxBarHeight < 25000000)   inc_amount=2500000;
   else if (MaxBarHeight < 50000000)   inc_amount=5000000;
   else if (MaxBarHeight < 100000000)  inc_amount=10000000;
   else if (MaxBarHeight < 250000000)  inc_amount=25000000;
   else if (MaxBarHeight < 500000000)  inc_amount=50000000;
   else if (MaxBarHeight < 1000000000) inc_amount=100000000;
   else if (MaxBarHeight < 2500000000) inc_amount=250000000;
   else if (MaxBarHeight < 5000000000) inc_amount=500000000;
   else if (MaxBarHeight < 10000000000) inc_amount=1000000000;
   else if (MaxBarHeight < 25000000000) inc_amount=2500000000;
   else if (MaxBarHeight < 50000000000) inc_amount=5000000000;
   else if (MaxBarHeight < 100000000000) inc_amount=10000000000;
   else if (MaxBarHeight < 250000000000) inc_amount=25000000000;
   else if (MaxBarHeight < 500000000000) inc_amount=50000000000;

   while ((MaxBarHeight/inc_amount)>5)
       inc_amount*=2;

   if (Indicator->VolScaleTo==0)
     MaxBarHeight=int(MaxBarHeight / inc_amount) * inc_amount + inc_amount;
   IncAmount=inc_amount;

   if (MaxBarHeight!=0.0)
       RangeDiv=(double) StudyHeight / (MaxBarHeight);
   else RangeDiv=1.0;	      
}

//------------------------------------------------------------
void QTSeqCharterVol::DrawStudy(BarIndicatorType *Indicator,
                                ColorIndicatorType *Colors,
                                ExchangeMapType *Map,
                                unsigned char UseISO,
                                int PointExpand,int PointExpand2)
{
   int loop,maxy,y1,y2;
   double CurrX;
   CPen *OldPen;
   CBrush *OldBrush;

   if (!StopBar) return;
   
   NewDC.SelectClipRgn(&ClipRgn);          
   OldPen=NewDC.SelectObject(&TradeOutlinePen);
   OldBrush=NewDC.SelectObject(&WindowBKBrush);

   int ERadius=PointExpand2;
   if (ERadius<1) ERadius=1;
   if (ERadius>6) ERadius=6;
   
   // Figure graph min's and max's     
   maxy=StudyY+StudyHeight;       
      
   CurrX=StudyX;
   int VolWidth=DoubleBarSpacing;
   if (DoubleBarSpacing>2) --VolWidth;
   
   y2=StudyY+StudyHeight;
   
   for (loop=StartBar;loop<=StopBar;loop++)
   {              
     if (loop>=0)
     {
       for (int loop2=MAXEXCHANGES-1;loop2>=0;loop2--)
       {
         if ((Indicator->Array[loop2]->BidOn)&&(Indicator->Array[loop2]->BidVolume[loop]>0))
         {   
           y1=(int)(maxy-(Indicator->Array[loop2]->BidVolume[loop]*RangeDiv));        
           if (((loop2==0)&&(Indicator->DrawCBidLine))||((loop2>0)&&(Indicator->DrawBidLine)))
           {             
             if (DoubleBarSpacing>1)   
               NewDC.FillSolidRect(CurrX,y1,VolWidth,y2-y1+1,Map[loop2].BidColor);
             else
             {
               NewDC.SelectObject(&Colors->Colors[loop2]->BidPen);
               NewDC.MoveTo(CurrX,y1);  
               NewDC.LineTo(CurrX,y2); 
             }
           }
           else
           {
             NewDC.SelectObject(&Colors->Colors[loop2]->BidPen);
             NewDC.SelectObject(&Colors->Colors[loop2]->BidBrush);
             NewDC.Ellipse(CurrX-ERadius,y1-ERadius,CurrX+ERadius,y1+ERadius);
           }
         }
         if ((Indicator->Array[loop2]->AskOn)&&(Indicator->Array[loop2]->AskVolume[loop]>0))
         {   
           y1=(int)(maxy-(Indicator->Array[loop2]->AskVolume[loop]*RangeDiv));        
           if (((loop2==0)&&(Indicator->DrawCAskLine))||((loop2>0)&&(Indicator->DrawAskLine)))
           {
             if (DoubleBarSpacing>1)   
               NewDC.FillSolidRect(CurrX,y1,VolWidth,y2-y1+1,Map[loop2].AskColor);
             else
             {
               NewDC.SelectObject(&Colors->Colors[loop2]->AskPen);
               NewDC.MoveTo(CurrX,y1);  
               NewDC.LineTo(CurrX,y2); 
             }
           }
           else
           {
             NewDC.SelectObject(&Colors->Colors[loop2]->AskPen);
             NewDC.SelectObject(&Colors->Colors[loop2]->AskBrush);
             NewDC.Ellipse(CurrX-ERadius,y1-ERadius,CurrX+ERadius,y1+ERadius);
           }
         }
       }			                	
     }
  	 CurrX+=DoubleBarSpacing;
   }
   
   // Do Trades AFTER all quotes
   CurrX=StudyX;
   for (loop=StartBar;loop<=StopBar;loop++)
   {              
     if (loop>=0)
     {
       for (int loop2=MAXEXCHANGES-1;loop2>=0;loop2--)
       {
         if ((Indicator->Array[loop2]->TradeOn)&&(Indicator->Array[loop2]->Volume[loop]>0))
         {   
           y1=(int)(maxy-(Indicator->Array[loop2]->Volume[loop]*RangeDiv));        
           NewDC.SelectObject(&TradeOutlinePen);
           NewDC.SelectObject(&Colors->Colors[loop2]->LastBrush);

           if ((UseISO)&&(Indicator->Array[loop2]->TCondition[loop]==95))
           {
              POINT TriPoints[3];
              TriPoints[0].x=CurrX;
              TriPoints[0].y=(int) y1-(3+PointExpand);
              TriPoints[1].x=CurrX-(3+PointExpand);
              TriPoints[1].y=(int) y1+(2+PointExpand);
              TriPoints[2].x=CurrX+(3+PointExpand);
              TriPoints[2].y=(int) y1+(2+PointExpand);
              NewDC.Polygon((const POINT *)&TriPoints,3);
           }
           else
           {
             if (PointExpand<4)         
                NewDC.Rectangle(CurrX-(3+PointExpand),
                     (int) y1-(3+PointExpand),
                     CurrX+(3+PointExpand),
                     (int) y1+(3+PointExpand));             
              else
                NewDC.Ellipse(CurrX-(3+PointExpand),
                     (int) y1-(3+PointExpand),
                     CurrX+(3+PointExpand),
                     (int) y1+(3+PointExpand));
           }
         }         
       }			                	
     }
  	 CurrX+=DoubleBarSpacing;
   }

   if (OldPen) NewDC.SelectObject(OldPen);  
   if (OldBrush) NewDC.SelectObject(OldBrush);  
   NewDC.SelectClipRgn(NULL);
}

//------------------------------------------------------------
void QTSeqCharterVol::DrawStudy2(BarIndicatorType *Indicator,
                                ColorIndicatorType *Colors,
                                ExchangeMapType *Map,
                                unsigned char UseISO,
                                int PointExpand,int PointExpand2)
{
   int loop,maxy,y1,y2;
   double CurrX;
   CPen *OldPen;
   CBrush *OldBrush;

   if (!StopBar) return;
   
   NewDC.SelectClipRgn(&ClipRgn);          
   OldPen=NewDC.SelectObject(&TradeOutlinePen);
   OldBrush=NewDC.SelectObject(&WindowBKBrush);

   int ERadius=PointExpand2;
   if (ERadius<1) ERadius=1;
   if (ERadius>6) ERadius=6;

   int centerline=StudyY+(StudyHeight/2);
   
   // Figure graph min's and max's     
   maxy=StudyY+StudyHeight;       
      
   CurrX=StudyX;
   int VolWidth=DoubleBarSpacing;
   if (DoubleBarSpacing>2) --VolWidth;   
   y2=StudyY+StudyHeight;

   
   for (loop=StartBar;loop<=StopBar;loop++)
   {              
     if (loop>=0)
     {
       for (int loop2=MAXEXCHANGES-1;loop2>=0;loop2--)
       {
         if ((Indicator->Array[loop2]->BidOn)&&(Indicator->Array[loop2]->BidVolume[loop]>0))
         {   
           y1=(int)(centerline+(Indicator->Array[loop2]->BidVolume[loop]*RangeDiv));        
           if (((loop2==0)&&(Indicator->DrawCBidLine))||((loop2>0)&&(Indicator->DrawBidLine)))
           {             
             if (DoubleBarSpacing>1)   
               NewDC.FillSolidRect(CurrX,y1,VolWidth,centerline,Map[loop2].BidColor);
             else
             {
               NewDC.SelectObject(&Colors->Colors[loop2]->BidPen);
               NewDC.MoveTo(CurrX,y1);  
               NewDC.LineTo(CurrX,centerline); 
             }
           }
           else
           {
             NewDC.SelectObject(&Colors->Colors[loop2]->BidPen);
             NewDC.SelectObject(&Colors->Colors[loop2]->BidBrush);
             NewDC.Ellipse(CurrX-ERadius,y1-ERadius,CurrX+ERadius,y1+ERadius);
           }
         }
         if ((Indicator->Array[loop2]->AskOn)&&(Indicator->Array[loop2]->AskVolume[loop]>0))
         {   
           y1=(int)(centerline-(Indicator->Array[loop2]->AskVolume[loop]*RangeDiv));        
           if (((loop2==0)&&(Indicator->DrawCAskLine))||((loop2>0)&&(Indicator->DrawAskLine)))
           {
             if (DoubleBarSpacing>1)   
               NewDC.FillSolidRect(CurrX,y1,VolWidth,centerline,Map[loop2].AskColor);
             else
             {
               NewDC.SelectObject(&Colors->Colors[loop2]->AskPen);
               NewDC.MoveTo(CurrX,y1);  
               NewDC.LineTo(CurrX,centerline); 
             }
           }
           else
           {
             NewDC.SelectObject(&Colors->Colors[loop2]->AskPen);
             NewDC.SelectObject(&Colors->Colors[loop2]->AskBrush);
             NewDC.Ellipse(CurrX-ERadius,y1-ERadius,CurrX+ERadius,y1+ERadius);
           }
         }
       }			                	
     }
  	 CurrX+=DoubleBarSpacing;
   }
   
   
   if (OldPen) NewDC.SelectObject(OldPen);  
   if (OldBrush) NewDC.SelectObject(OldBrush);  
   NewDC.SelectClipRgn(NULL);
}

//-------------------------------------------------------------
void QTSeqCharterVol::DrawScale(BarIndicatorType *Indicator,unsigned char ShowGrid)
{
    int inc_amount,now_amount,number1;    
    char str[255];

    if (LastMaxSizeResize==0) return;
      
	int CTop       =StudyY;
    CPen *OldPen;   
    CFont *OldFont;
    
    NewDC.SetTextColor(WindowTextColor);
	NewDC.SetBkColor(WindowBKColor);

    if (ChartFont) OldFont=NewDC.SelectObject(ChartFont);
    OldPen=NewDC.SelectObject(&ChartOutlinePen);
	
    int maxy=StudyY+StudyHeight;       
    int ScaleX=ChartingX+ChartingWidth;
   
    if (Indicator->VolScaleTo==1)
      inc_amount=Indicator->HardVolScale;
    else
    {
      if (StudyHeight<75) inc_amount=(int)IncAmount*2;
      else                inc_amount=(int)IncAmount;
    }

    now_amount=(int)inc_amount;

    if ((MaxBarHeight!=0)&&(inc_amount!=0))
    {
        while (now_amount <= MaxBarHeight+(IncAmount))///errormargin))//+inc_amount) 
		{          
		    number1=(int)(maxy-((now_amount-MinBarHeight)*RangeDiv));	
            if (number1>=ChartingY)
            {
              NewDC.SetBkColor(WindowBKColor);
              sprintf(str,"%d",(int)now_amount);              	            
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

//-------------------------------------------------------------
void QTSeqCharterVol::DrawScale2(BarIndicatorType *Indicator,unsigned char ShowGrid)
{
    int inc_amount,now_amount,number1,number2;    
    char str[255];

    if (LastMaxSizeResize==0) return;
      
	int CTop       =StudyY;
    CPen *OldPen;   
    CFont *OldFont;
    
    NewDC.SetTextColor(WindowTextColor);
	NewDC.SetBkColor(WindowBKColor);

    if (ChartFont) OldFont=NewDC.SelectObject(ChartFont);
    OldPen=NewDC.SelectObject(&ChartOutlinePen);
	
    int maxy=StudyY+StudyHeight;       
    int ScaleX=ChartingX+ChartingWidth;
   
    if (Indicator->VolScaleTo==1)
      inc_amount=Indicator->HardVolScale;
    else
    {
      if (StudyHeight<75) inc_amount=(int)IncAmount*2;
      else                inc_amount=(int)IncAmount;
    }
    int centerline=StudyY+(StudyHeight/2);

    now_amount=(int)inc_amount;

    if ((MaxBarHeight!=0)&&(inc_amount!=0))
    {
        while (now_amount <= MaxBarHeight+(IncAmount))
		{          
		    number1=(int)(centerline-((now_amount-MinBarHeight)*RangeDiv));	
            number2=(int)(centerline+((now_amount-MinBarHeight)*RangeDiv));	
            if (number1>=ChartingY)
            {
              NewDC.SetBkColor(WindowBKColor);
              sprintf(str,"%d",(int)now_amount);              	            
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
            if (number2<ChartingY+ChartingHeight)
            {
              NewDC.SetBkColor(WindowBKColor);
              sprintf(str,"%d",(int)now_amount);              	            
		      NewDC.TextOut(12 + ScaleX,(int) number2-7,str,(int)strlen(str));	 	 
			  
              NewDC.SelectObject(&ChartOutlinePen);
              NewDC.MoveTo(ScaleX+3,number1);
              NewDC.LineTo(ScaleX+6,number1);

              if (ShowGrid)
              {
                NewDC.SetBkColor(ChartBKColor);
                NewDC.SelectObject(&StudyGridPen);            
                NewDC.MoveTo(ChartingX+1,number2);
                NewDC.LineTo(ScaleX-1,number2);
              }
            }
            now_amount+=inc_amount;
	    }
    }

    NewDC.SetBkColor(WindowBKColor);
    sprintf(str,"0");              	            
    NewDC.TextOut(12 + ScaleX,(int) centerline-7,str,(int)strlen(str));	 	 
			  
    NewDC.SelectObject(&ChartOutlinePen);
    NewDC.MoveTo(ScaleX+3,centerline);
    NewDC.LineTo(ScaleX+6,centerline);

    if (ShowGrid)
    {
      NewDC.SetBkColor(ChartBKColor);
      NewDC.SelectObject(&StudyGridPen);            
      NewDC.MoveTo(ChartingX+1,centerline);
      NewDC.LineTo(ScaleX-1,centerline);
    }

    if (OldPen) NewDC.SelectObject(OldPen);    
    if (OldFont) NewDC.SelectObject(OldFont);
}

