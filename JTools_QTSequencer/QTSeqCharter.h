/////////////////////////////////////////////////////////////////////////////
// QTSeqCharter
// Author: Jeffrey Donovan
// Copyright 2012 by Nanex, LLC
// LDM: 08-06-2012
/////////////////////////////////////////////////////////////////////////////
#ifndef __QTSEQCHARTER_H
#define __QTSEQCHARTER_H

#include "..\JT_CommonChart\JTools_SimpleChartTemplate.h"
#include "QTSeqChartTypes.h"

class QTSeqCharter: public JTools_SimpleChartTemplate
{
public:

	QTSeqCharter();
	virtual ~QTSeqCharter();

    void SetGridColor(COLORREF Color);   		        
    void SetTimeTickColor(COLORREF HighColor,COLORREF LowColor);
    void DrawLeftVertLabel(int XOffset,int YOffset,char *Text);
    void DrawTimeLine(BarIndicatorType *Indicator,unsigned char WithText,unsigned char ShowGrid,
                      unsigned char ShowGridTime,
                      unsigned char ShowGridCount,
                      unsigned char ReverseTimeGridOrder);
    void DrawTitle(char *str);
    void DrawProductTitle(char *str);
    void SetStartStopBars(int TStartBar,int TStopBar);

    double DoubleBarSpacing;
    COLORREF TimeTickHighColor,TimeTickLowColor;
    CPen TimeTickHighPen,TimeTickLowPen;
};

/////////////////////////////////////////////////////////////////////////////
#endif 

