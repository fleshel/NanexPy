/////////////////////////////////////////////////////////////////////////////
// QTSeqCharterPrice
// Author: Jeffrey Donovan
//
// Copyright 2012 by Nanex, LLC
//
// LDM: 08-06-2012
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __QTSEQCHARTPRICE_H
#define __QTSEQCHARTPRICE_H

#include "QTSeqCharter.h"

class QTSeqCharterPrice: public QTSeqCharter
{
public:

	// Constructor/destructor
	QTSeqCharterPrice();
	virtual ~QTSeqCharterPrice();
				
    void ResetZeroRange(void);
    void SetTradeOutlineColor(COLORREF Color);
    void FindMaxBar(BarIndicatorType *Indicator);	
    
    void DrawStudy(BarIndicatorType *Indicator,ColorIndicatorType *Colors,unsigned char UseISO,
                                  int PointExpand,int PointExpand2);
    void DrawScale(BarIndicatorType *Indicator,unsigned char TradePriceType,unsigned char ShowGrid);
    
    int LastMaxPriceResize;
    int LastMinPriceResize;   
    
    COLORREF TradeOutlineColor;
    CPen TradeOutlinePen;
    
};

/////////////////////////////////////////////////////////////////////////////
#endif

