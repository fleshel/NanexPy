// JTools_ShellApp.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CJTools_ShellAppApp:
// See JTools_ShellApp.cpp for the implementation of this class
//

class CJTools_ShellAppApp : public CWinApp
{
public:
	CJTools_ShellAppApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CJTools_ShellAppApp theApp;