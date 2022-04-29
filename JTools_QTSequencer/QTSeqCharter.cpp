/////////////////////////////////////////////////////////////////////////////
// QTSeqCharter
// Author: Jeffrey Donovan
//
// Copyright 2012 by Nanex, LLC 
//
// LDM: 08-06-2012
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "QTSeqCharter.h"

extern void ChartCallback(char *str);

//------------------------------------------------------------
QTSeqCharter::QTSeqCharter()
{
  DoubleBarSpacing=1;
  TimeTickHighColor=RGB(255,255,255);
  TimeTickLowColor=RGB(155,155,155);
  TimeTickHighPen.CreatePen(PS_SOLID,1,TimeTickHighColor);
  TimeTickLowPen.CreatePen(PS_SOLID,1,TimeTickLowColor);
}
//------------------------------------------------------------
QTSeqCharter::~QTSeqCharter()
{
  TimeTickHighPen.DeleteObject();
  TimeTickLowPen.DeleteObject();
}
//-------------------------------------------------------------
void QTSeqCharter::SetGridColor(COLORREF Color)
{
  ChartGridColor=Color;
}
//-------------------------------------------------------------
void QTSeqCharter::SetTimeTickColor(COLORREF HighColor,COLORREF LowColor)
{
  TimeTickHighPen.DeleteObject();
  TimeTickLowPen.DeleteObject();
  TimeTickHighColor=HighColor;
  TimeTickLowColor=LowColor;
  TimeTickHighPen.CreatePen(PS_SOLID,1,TimeTickHighColor);
  TimeTickLowPen.CreatePen(PS_SOLID,1,TimeTickLowColor);
}
//-------------------------------------------------------------
 void QTSeqCharter::DrawTimeLine(BarIndicatorType *Indicator,unsigned char WithText,unsigned char ShowGrid,
                                 unsigned char ShowGridTime,
                                 unsigned char ShowGridCount,
                                 unsigned char ReverseTimeGridOrder)
{   
   if (StopBar==0) return;

   int Top;      
   char str[32]; 

   CPen *OldPen=NewDC.SelectObject(&TimeTickHighPen);
   NewDC.SetBkColor(WindowBKColor);
   NewDC.SetTextColor(WindowTextColor);
 
   Top   =ChartingY+ChartingHeight+1;                
   double loop1=StudyX;
   int Divisor=30;
   int Gy1,Gy2;

   CFont *OldFont;
   if (ChartFont) OldFont=NewDC.SelectObject(ChartFont);

   Gy1=ChartingY+1;
   Gy2=ChartingY+ChartingHeight-1;

   int loop,DateY,CountY;      

   int numbars=StopBar-StartBar-1;  
   if (numbars<=50) numbars=5;
   else if (numbars<=110) numbars=10;
   else if (numbars<=275) numbars=25;
   else if (numbars<=550) numbars=50;
   else if (numbars<=1200) numbars=100;
   else if (numbars<=2600) numbars=250;
   else if (numbars<=5100) numbars=500;
   else if (numbars<=12000) numbars=1000;
   else if (numbars<=27000) numbars=2500;
   else if (numbars<=51000) numbars=5000;
   else if (numbars<=510000) numbars=10000;

   if (ShowGridTime) CountY=4+ChartingY+ChartingHeight;	          	 	 
   else              CountY=1+ChartingY+ChartingHeight;

   if (ShowGridCount) DateY=CountY+12;
   else               DateY=CountY;

   SetTextAlign(NewDC,TA_CENTER);

   int CurrBarCount=0;
   if (ShowGridCount)
   {
     loop1=StudyX;

     if (StartBar>1)
     {
       sprintf_s(str,25,"%d",(int)StartBar-1);
       NewDC.TextOut((int)(loop1),CountY,str,(int)strlen(str));     
     }
     else
     {
       sprintf_s(str,25,"0");       
       NewDC.TextOut((int)(loop1),CountY,str,(int)strlen(str));     
     }

     for(loop=StartBar;loop<=StopBar;loop++) 
     {     
       ++CurrBarCount;           
       if (CurrBarCount>=numbars)
       {
         sprintf_s(str,25,"%d",(int)loop+1);
         NewDC.TextOut((int)(loop1),CountY,str,(int)strlen(str));     
         CurrBarCount=0;

         if (ShowGrid)
         {
            NewDC.SetBkColor(ChartBKColor);
            NewDC.SelectObject(&StudyGridPen);
            NewDC.MoveTo(loop1,Gy1);
            NewDC.LineTo(loop1,Gy2);
            NewDC.SetBkColor(WindowBKColor);
         }
       }
       loop1+=DoubleBarSpacing;       
     }
  }

  if (ShowGridTime)
  {
    int TickBegin=0,TickEnd=0;
    int CTop=ChartingY+ChartingHeight;
    int y2=CTop+2;

    int Region=0;
    int NMin=0,NSec=0,NMSec=0;
    for(loop=StartBar+1;loop<=StopBar;loop++)            
    {
       if ((Indicator->Min[loop]!=Indicator->Min[loop-1])&&(Indicator->Hour[loop]!=0)&&(Indicator->Hour[loop-1]!=0)) ++NMin;
       if ((Indicator->Sec[loop]!=Indicator->Sec[loop-1])&&(Indicator->Hour[loop]!=0)&&(Indicator->Hour[loop-1]!=0)) ++NSec;
       if ((Indicator->Sec[loop]!=Indicator->Sec[loop-1])&&(Indicator->Hour[loop]!=0)&&(Indicator->Hour[loop-1]!=0)) ++NMSec;
    }
    if (NMin>=4) Region=1;
    else if (NSec>=4) Region=2;
    else if (NMSec>=2) Region=3;

    
    switch(Region)
    {
    case 0:
       SetTextAlign(NewDC,TA_LEFT);    
       loop1=StudyX;   
       sprintf_s(str,25,"<%02d:%02d:%02d",Indicator->Hour[StartBar],Indicator->Min[StartBar],Indicator->Sec[StartBar]);

       NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));            
       
       loop1=StudyX + ((StopBar-StartBar+1)*DoubleBarSpacing) - 33;   
       sprintf_s(str,25,"%02d:%02d:%02d>",Indicator->Hour[StopBar],Indicator->Min[StopBar],Indicator->Sec[StopBar]);

       NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));                   
       SetTextAlign(NewDC,TA_CENTER);    
       break;

    case 1:
      if (!ReverseTimeGridOrder)
      {    
        loop1=StudyX;   
        int LastSecBar=StartBar;
        for(loop=StartBar;loop<=StopBar;loop++) 
        {   
         if ((loop>0)&&(Indicator->Min[loop]!=Indicator->Min[loop-1]))
         {          
           if (TickBegin==0) TickBegin=loop1;
           TickEnd=loop1;

           int Dist=(loop-LastSecBar)*DoubleBarSpacing;
           if (((Dist>20)&&(LastSecBar==StartBar)) ||
               (Dist>40))
           {                       
             sprintf_s(str,25,"%02d:%02d",Indicator->Hour[loop],Indicator->Min[loop]);
             NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));            
             
             NewDC.SelectObject(&TimeTickHighPen);
             LastSecBar=loop;
             NewDC.MoveTo((int) loop1-3,(int) y2-1);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);

             LastSecBar=loop;
           }
           else
           {
             NewDC.SelectObject(&TimeTickLowPen);
             NewDC.MoveTo((int) loop1-3,(int) y2-1);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);
           }
         }
         loop1+=DoubleBarSpacing;
        }
      }
      else
      {
        loop1=StudyX + ((StopBar-StartBar+1)*DoubleBarSpacing);   
        int LastSecBar=StopBar;
        for(loop=StopBar;loop>StartBar;loop--) 
        {        
         if ((loop>0)&&(Indicator->Min[loop-1]!=Indicator->Min[loop]))
         {          
           if (TickEnd==0) TickEnd=loop1;
           TickBegin=loop1;

           int Dist=(LastSecBar-loop)*DoubleBarSpacing;
           if (((Dist>20)&&(LastSecBar==StopBar)) ||
              (Dist>40))
           {
             sprintf_s(str,25,"%02d:%02d",Indicator->Hour[loop],Indicator->Min[loop]);
             NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));            
             
             NewDC.SelectObject(&TimeTickHighPen);
             NewDC.MoveTo((int) loop1-3,(int) y2-2);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);
             
             LastSecBar=loop;
           }
           else
           {
             NewDC.SelectObject(&TimeTickLowPen);
             NewDC.MoveTo((int) loop1-3,(int) y2-1);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);
           }
         }
         loop1-=DoubleBarSpacing;
        }
       }     
       break;

    case 2:
       if (!ReverseTimeGridOrder)
      {    
        loop1=StudyX;   
        int LastSecBar=StartBar;
        for(loop=StartBar;loop<=StopBar;loop++) 
        {   
         if (Indicator->Sec[loop]!=Indicator->Sec[loop-1])
         {          
           if (TickBegin==0) TickBegin=loop1;
           TickEnd=loop1;

           int Dist=(loop-LastSecBar)*DoubleBarSpacing;
           if (((Dist>25)&&(LastSecBar==StartBar)) ||
               (Dist>50))
           {          
             sprintf_s(str,25,"%02d:%02d:%02d",Indicator->Hour[loop],Indicator->Min[loop],Indicator->Sec[loop]);
             NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));            
             
             NewDC.SelectObject(&TimeTickHighPen);
             LastSecBar=loop;
             NewDC.MoveTo((int) loop1-3,(int) y2-1);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);

             LastSecBar=loop;
           }
           else
           {

             NewDC.SelectObject(&TimeTickLowPen);
             NewDC.MoveTo((int) loop1-3,(int) y2-1);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);
           }
         }
         loop1+=DoubleBarSpacing;
        }
      }
      else
      {
        loop1=StudyX + ((StopBar-StartBar+1)*DoubleBarSpacing);   
        int LastSecBar=StopBar;
        for(loop=StopBar;loop>StartBar;loop--) 
        {        
         if (Indicator->Sec[loop-1]!=Indicator->Sec[loop])
         {          
           if (TickEnd==0) TickEnd=loop1;
           TickBegin=loop1;

           int Dist=(LastSecBar-loop)*DoubleBarSpacing;
           if (((Dist>25)&&(LastSecBar==StopBar)) ||
              (Dist>50))
           {
             sprintf_s(str,25,"%02d:%02d:%02d",Indicator->Hour[loop],Indicator->Min[loop],Indicator->Sec[loop]);
             NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));            
             
             NewDC.SelectObject(&TimeTickHighPen);
             NewDC.MoveTo((int) loop1-3,(int) y2-2);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);
             
             LastSecBar=loop;
           }
           else
           {
             NewDC.SelectObject(&TimeTickLowPen);
             NewDC.MoveTo((int) loop1-3,(int) y2-1);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);
           }
         }
         loop1-=DoubleBarSpacing;
        }
      }
      break;

    case 3:
      if (!ReverseTimeGridOrder)
      {    
        loop1=StudyX;   
        int LastSecBar=StartBar;
        
        for(loop=StartBar;loop<=StopBar;loop++) 
        {   
         if ((Indicator->Sec[loop]!=Indicator->Sec[loop-1]))
         {          
           if (TickBegin==0) TickBegin=loop1;
           TickEnd=loop1;

           int Dist=(loop-LastSecBar)*DoubleBarSpacing;
           if (((Dist>35)&&(LastSecBar==StartBar)) ||
               ((Dist>75)&&((Indicator->Sec[loop]>Indicator->Sec[LastSecBar])||(Indicator->Sec[loop]>Indicator->Sec[LastSecBar]))))
           {          
             sprintf_s(str,25,"%02d:%02d:%02d.%03d",Indicator->Hour[loop],Indicator->Min[loop],Indicator->Sec[loop],Indicator->MSec[loop]);
             NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));            
             
             NewDC.SelectObject(&TimeTickHighPen);
             LastSecBar=loop;
             NewDC.MoveTo((int) loop1-3,(int) y2-1);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);

             LastSecBar=loop;
           }
           else
           {

             NewDC.SelectObject(&TimeTickLowPen);
             NewDC.MoveTo((int) loop1-3,(int) y2-1);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);
           }
         }
         loop1+=DoubleBarSpacing;
        }
      }
      else
      {
        loop1=StudyX + ((StopBar-StartBar+1)*DoubleBarSpacing);   
        int LastSecBar=StopBar;
        for(loop=StopBar;loop>StartBar;loop--) 
        {        
          if ((Indicator->Sec[loop-1]!=Indicator->Sec[loop]))
         {          
           if (TickEnd==0) TickEnd=loop1;
           TickBegin=loop1;

           int Dist=(LastSecBar-loop)*DoubleBarSpacing;
           if (((Dist>35)&&(LastSecBar==StopBar)) ||
               ((Dist>75)&&((Indicator->Sec[LastSecBar]>Indicator->Sec[loop])||(Indicator->Sec[LastSecBar]>Indicator->Sec[loop]))))
           {
             sprintf_s(str,25,"%02d:%02d:%02d.%03d",Indicator->Hour[loop],Indicator->Min[loop],Indicator->Sec[loop],Indicator->MSec[loop]);
             NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));            
             
             NewDC.SelectObject(&TimeTickHighPen);
             NewDC.MoveTo((int) loop1-3,(int) y2-2);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);
             
             LastSecBar=loop;
           }
           else
           {
             NewDC.SelectObject(&TimeTickLowPen);
             NewDC.MoveTo((int) loop1-3,(int) y2-1);
             NewDC.LineTo((int) loop1,(int) y2+3);
             NewDC.LineTo((int) loop1+3,(int) y2-2);
           }
         }
         loop1-=DoubleBarSpacing;
        }
      }
      break;
    }
    
    SetTextAlign(NewDC,TA_LEFT);    

    switch(Region)
    {
    case 1:
       if (TickBegin>75)
       {
         loop1=StudyX;   
         sprintf_s(str,25,"<%02d:%02d",Indicator->Hour[StartBar],Indicator->Min[StartBar]);
         NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));                     
       }
       if (TickEnd< (StudyX + ((StopBar-StartBar+1)*DoubleBarSpacing))-22)
       {
         loop1=StudyX + ((StopBar-StartBar+1)*DoubleBarSpacing)-22;   
         sprintf_s(str,25,"%02d:%02d>",Indicator->Hour[StopBar],Indicator->Min[StopBar]);
         NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));                     
       }
       break;

    case 2:
       if (TickBegin>100)
       {
         loop1=StudyX;   
         sprintf_s(str,25,"<%02d:%02d:%02d",Indicator->Hour[StartBar],Indicator->Min[StartBar],Indicator->Sec[StartBar]);
         NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));                     
       }
       if (TickEnd< (StudyX + ((StopBar-StartBar+1)*DoubleBarSpacing))-100)
       {
         loop1=StudyX + ((StopBar-StartBar+1)*DoubleBarSpacing)-33;   
         sprintf_s(str,25,"%02d:%02d:%02d>",Indicator->Hour[StopBar],Indicator->Min[StopBar],Indicator->Sec[StopBar]);
         NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));                     
       }
      break;

    case 3:
      if (TickBegin>150)
      {
        loop1=StudyX;   
        sprintf_s(str,25,"<%02d:%02d:%02d.%03d",Indicator->Hour[StartBar],Indicator->Min[StartBar],Indicator->Sec[StartBar],Indicator->MSec[StartBar]);
        NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));                    
      }
      if (TickEnd< (StudyX + ((StopBar-StartBar+1)*DoubleBarSpacing))-150)
      {
        loop1=StudyX + ((StopBar-StartBar+1)*DoubleBarSpacing)-55;   
        sprintf_s(str,25,"%02d:%02d:%02d.%03d>",Indicator->Hour[StopBar],Indicator->Min[StopBar],Indicator->Sec[StopBar],Indicator->MSec[StopBar]);
        NewDC.TextOut((int)(loop1),DateY,str,(int)strlen(str));                    
      }
      break;
    }  
  }

   SetTextAlign(NewDC,TA_LEFT);    
   NewDC.SelectObject(OldPen);
   if (OldFont) NewDC.SelectObject(OldFont);
   
}
//-------------------------------------------------------------
void QTSeqCharter::DrawLeftVertLabel(int XOffset,int YOffset,char *Text)
{
    CFont *OldFont;
    if (ChartFont) OldFont=NewDC.SelectObject(ChartFont);
    NewDC.SetTextColor(WindowTextColor);
	NewDC.SetBkColor(WindowBKColor);

    SetTextAlign(NewDC,TA_CENTER);

    int len=(int)strlen(Text);
    int y=ChartingY;
    for (int i=0;i<len;i++)
    {
      NewDC.TextOut(XOffset,y+YOffset,&Text[i],1);	 	 
      y+=FontHeight;
    }

    SetTextAlign(NewDC,TA_LEFT);    
    if (OldFont) NewDC.SelectObject(OldFont);
}

//------------------------------------------------------------
void QTSeqCharter::SetStartStopBars(int TStartBar,int TStopBar)
{
	StartBar=TStartBar;
    StopBar=TStopBar;
    DoubleBarSpacing=(double) StudyWidth/ (double) (StopBar-StartBar+1);  
}

//-------------------------------------------------------------
void QTSeqCharter::DrawTitle(char *str)
{
   CFont *OldFont;
   if (ChartFont) OldFont=NewDC.SelectObject(ChartFont);
   NewDC.SelectClipRgn(&TitleClipRgn);               

   NewDC.SetTextColor(ChartTextColor);
   NewDC.SetBkColor(WindowBKColor);    
   NewDC.TextOut(ChartingX,ChartingY-FontHeight-4,str,(int)strlen(str));	 	 
        
   if (OldFont) NewDC.SelectObject(OldFont);
   NewDC.SelectClipRgn(NULL);               
}
//-------------------------------------------------------------
void QTSeqCharter::DrawProductTitle(char *str)
{
   CFont *OldFont;
   if (ChartFont) OldFont=NewDC.SelectObject(ChartFont);
   NewDC.SelectClipRgn(&TitleClipRgn);               

   int X=ChartingX+ChartingWidth-((strlen(str)+3)*FontWidth);

   NewDC.SetTextColor(ChartTextColor);
   NewDC.SetBkColor(WindowBKColor);    
   NewDC.TextOut(X,ChartingY-FontHeight-4,str,(int)strlen(str));	 	 
        
   if (OldFont) NewDC.SelectObject(OldFont);
   NewDC.SelectClipRgn(NULL);               
}
