/////////////////////////////////////////////////////////////////////////////
// QTSeqCharterLegend
// Author: Jeffrey Donovan
//
// Copyright 2012 by Nanex, LLC 
//
// LDM: 07-31-2012
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __QTSEQCHARTLEG_H
#define __QTSEQCHARTLEG_H

#include "QTSeqCharter.h"

class QTSeqCharterLegend: public QTSeqCharter
{
public:

	QTSeqCharterLegend();
	virtual ~QTSeqCharterLegend();
				
    void ResetZeroRange(void);
	void FindMaxBar(BarIndicatorType *Indicator);
    void SetVolColor(COLORREF Color);
    
    void DrawStudy(BarIndicatorType *Indicator,
                   ColorIndicatorType *Colors,
                   ExchangeMapType *ExchangeMap,
                   unsigned char UseISO,int PointExpand,int PointExpand2);

    int LastMaxSizeResize;
    int LegendStudyHeight;

    unsigned char AnyTradesOn,AnyBidsOn,AnyAsksOn;
};

/////////////////////////////////////////////////////////////////////////////
#endif 

