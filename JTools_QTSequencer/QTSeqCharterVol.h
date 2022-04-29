/////////////////////////////////////////////////////////////////////////////
// QTSeqCharterVol
// Author: Jeffrey Donovan
//
// Copyright 2012 by Nanex, LLC 
//
// LDM: 07-31-2012
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __QTSEQCHARTVOL_H
#define __QTSEQCHARTVOL_H

#include "QTSeqCharter.h"

class QTSeqCharterVol: public QTSeqCharter
{
public:

	QTSeqCharterVol();
	virtual ~QTSeqCharterVol();
				
    void ResetZeroRange(void);
	void FindMaxBar(BarIndicatorType *Indicator);
    void SetTradeOutlineColor(COLORREF Color);
    void SetStudyType(int Type);
    
    void DrawStudy(BarIndicatorType *Indicator,ColorIndicatorType *Colors,ExchangeMapType *Map,unsigned char UseISO,
                   int PointExpand,int PointExpand2);    
    void QTSeqCharterVol::DrawStudy2(BarIndicatorType *Indicator,
                                ColorIndicatorType *Colors,
                                ExchangeMapType *Map,
                                unsigned char UseISO,
                                int PointExpand,int PointExpand2);
    void DrawScale(BarIndicatorType *Indicator,unsigned char ShowGrid);
    void DrawScale2(BarIndicatorType *Indicator,unsigned char ShowGrid);

    int LastMaxSizeResize;
    unsigned char StudyType;

    COLORREF TradeOutlineColor;
    CPen TradeOutlinePen;
};

/////////////////////////////////////////////////////////////////////////////
#endif 

