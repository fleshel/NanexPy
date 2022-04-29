/////////////////////////////////////////////////////
// JTNX_NxFeedHandler.cpp                          //
// Modified specifically for the                   //
// JTools_QTSequencer app                          //
/////////////////////////////////////////////////////
// Author: Jeffrey Donovan                         //
// Copyright (C) 2012 Nanex, LLC.                  //
// All Rights Reserved.                            //
/////////////////////////////////////////////////////
//	If you are a Subscriber to NxCore, you may	   //
//  copy, modify, and distribute the contents	   //
//	of this file without royalty, provided that    //
//	you assume any and all responsibilty, and	   //
//	forever release Jeffrey Donovan and Eric       //
//  Scott Hunsader from	any damage claims, real,   //
//  perceived, intentional, or imagined.	       //
/////////////////////////////////////////////////////
// LDM: 09-18-13                                   //
/////////////////////////////////////////////////////

#include "StdAfx.h"
#include "process.h"
#include "math.h"
#include "..\JT_Common\NxCoreLoadLib.h"         // DLL load functions and dll references
#include "JTNX_NxFeedHandler.h"     // Defines and forwards


extern "C" {

// Delayed feed settings
typedef struct
{
  unsigned int MinutesDelayed;	// How many minutes to delay feed?
  unsigned short SleepDelay;    // Sleep delay for timer pause 
  unsigned char IsDelayed;      // Is the feed to be delayed?    
}JTNXDelayedSettings;

// NxCore parms
typedef struct
{ 
  char TapeFileName[512];         // Tape to run ("" for realtime)
  char DLLFileName[512];          // Dll to use
  unsigned int AppCommand;		  // App command 
  unsigned int ExcludeFlags;      // Exclude flags for ProcessTape  
  unsigned short SlaveHistDelay;  // How much delay to use in RT Sim mode
  unsigned char SlaveHistMode;    // Hist mode...Full, RT SIM, Step
  unsigned char StopFromUser;     // Special command from calling app to stop
  unsigned char StepRelease;      // Step release flag  
  unsigned char NxDateResetLog;   // Reset log filename to NxTape date
  int NxClockGUIInterval;         // GUI Interval to display NxCore time
  JTNXDelayedSettings Delayed;    // Delayed Feed settings
}JTNXUserParms;

// Global structure used to deal with specific NxCore needs
typedef struct 
{       
  JTNXUserParms UserParms;        // User parms and settings
  JTLogStruct *AppLog;            // The app log
    
  int ErrorCode;                  // Error code (if any)
  __int64 MessageCount;           // Message Counter
  unsigned int  APIVersion;       // API Version
  unsigned char IsOSI;            // OSI or traditional OPRA tape?
  unsigned char IsNxCoreRunning;  // Is NxCore running flag  
  char RunStatus;                 // Nxcore Running Status
      
  // App GUI Callbacks
  int (*JTNX_TimeDisplayCallBack)(const NxCoreSystem*,const NxCoreMessage*,unsigned char);
  int (*JTNX_EndOfTapeCallBack)(void);
  int (*JTNX_StatusDisplayCallBack)(char *str);
  int (*JTNX_ErrorCallBack)(int);

  char UserSymbol[32];          // User's symbol to watch
  unsigned char UserExg;        // Listed exg of symbol (if specified)
  NxString *CurrSymbol;         // Current Symbol being listned to
  unsigned char TriggerLookup;  // Did user just make a new request?
  unsigned int AppCommand;		// Specific for app commands (user stop, etc) 
  unsigned int SpinComplete;	// Is the symbol spin complete?	
  unsigned char FirstTime;

  unsigned long StartTime,StopTime;

  BarIndicatorType *pChartData;
  int MAXBARS;
  
  int (*JTNX_MessageCallBack)(int,const NxCoreSystem*,const NxCoreMessage*);

} JTNX_AppUserDataType;


// Forwards
//---------
UINT JTNX_Sys_StartNxCore(void* pVoid);
int __stdcall OnNxCoreCallback(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg);


// Allocate user data
//-----------------------------------------------------------
void *JTNX_Interface_AllocateGlobalUserData(void)
{  
  JTNX_AppUserDataType *Data=(JTNX_AppUserDataType *)calloc(1,sizeof(JTNX_AppUserDataType));
  if (Data) 
  {
    Data->RunStatus=-1;
    Data->FirstTime=1;
    Data->UserParms.NxClockGUIInterval=NxCLOCK_SECOND;
  }
  
  return((void*)Data);
}

// Free user data
//-----------------------------------------------------------
int JTNX_Interface_FreeGlobalUserData(void *pUserData)
{
  JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
  if (pAppUserData) 
  {
	free(pAppUserData);
  }
  return 0;
}

void JTNX_Interface_SetMaxBars(void *pUserData,int Max)
{
  JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
  pAppUserData->MAXBARS=Max;
}

//------------------------------------------------------------
void JTNX_Interface_SetChartDataPtr(void *pUserData,BarIndicatorType *Data)
{
  JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
  pAppUserData->pChartData=Data;
}

//------------------------------------------------------------
void JTNX_Interface_SetUserSymbol(void *pUserData,char *Symbol,unsigned char Exg)
{  
  JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
  strcpy(pAppUserData->UserSymbol,Symbol);
  pAppUserData->UserExg=Exg;

  if (pAppUserData->CurrSymbol)
  {
    pAppUserData->CurrSymbol->UserData1=0;
    pAppUserData->CurrSymbol=NULL;
  }
  pAppUserData->TriggerLookup=1;
}
//------------------------------------------------------------
int JTNX_Interface_SetMessageCallback(void *pUserData,int (*CallbackFunction)(int,const NxCoreSystem*,const NxCoreMessage*))
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
	// Set the callback function and start up the Systems spin thread		
	pAppUserData->JTNX_MessageCallBack=CallbackFunction;	   
	return 0;
}


// Set feed delay parameters
//-----------------------------------------------------------
int JTNX_Interface_SetFeedDelay(void *pUserData,
								 unsigned char IsDelayed,
								 unsigned int MinDelayed,
								 unsigned short SleepDelay)
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
	pAppUserData->UserParms.Delayed.IsDelayed=IsDelayed;
	pAppUserData->UserParms.Delayed.MinutesDelayed=MinDelayed;
    pAppUserData->UserParms.Delayed.SleepDelay=SleepDelay;
    return 0;
}

//-----------------------------------------------------------
int JTNX_Interface_SetAppLog(void *pUserData,JTLogStruct *pLog)								 
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
    pAppUserData->AppLog=pLog;
    return 0;
}
// Set mode to run in when using historical tapes (full, rt-sim, etc)
//------------------------------------------------------------
int JTNX_Interface_SetLogResetNxDate(void *pUserData,unsigned char ResetOnNXDate)
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
    pAppUserData->UserParms.NxDateResetLog=ResetOnNXDate;
	return 0;
}
// Set GUI interval to display NxTime
//------------------------------------------------------------
int JTNX_Interface_SetNxClockGUIInterval(void *pUserData,unsigned char NxClockGUIInterval)
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
    pAppUserData->UserParms.NxClockGUIInterval=NxClockGUIInterval;
	return 0;
}

// Get current message count
//-----------------------------------------------------------
__int64 JTNX_Interface_GetMsgCount(void *pUserData)
{	
	return(((JTNX_AppUserDataType*)pUserData)->MessageCount);	
}

// Get and reset the current message count
//-----------------------------------------------------------
__int64 JTNX_Interface_GetRestMsgCount(void *pUserData)
{
    __int64 num=((JTNX_AppUserDataType*)pUserData)->MessageCount;
	((JTNX_AppUserDataType*)pUserData)->MessageCount=0;	
    return(num);	
}

// Set callback for system status messages
//-----------------------------------------------------------
int JTNX_Interface_SetStatusDisplayCallback(void *pUserData,int (*CallbackFunction)(char *str))
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
	pAppUserData->JTNX_StatusDisplayCallBack=CallbackFunction;	   
	return 0;
}

// Set callback for system status messages
//-----------------------------------------------------------
int JTNX_Interface_SetErrorCallback(void *pUserData,int (*CallbackFunction)(int ErrorCode))
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
	pAppUserData->JTNX_ErrorCallBack=CallbackFunction;	   
	return 0;
}

// Set callback for NxCore time messages
//-----------------------------------------------------------
int JTNX_Interface_SetTimeDisplayCallback(void *pUserData,int (*CallbackFunction)(const NxCoreSystem*,const NxCoreMessage*,unsigned char))
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
	pAppUserData->JTNX_TimeDisplayCallBack=CallbackFunction;	   
	return 0;
}


// Set callback for end of tape notification
//-----------------------------------------------------------
int JTNX_Interface_SetEndOfTapeCallback(void *pUserData,int (*CallbackFunction)(void))
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
	pAppUserData->JTNX_EndOfTapeCallBack=CallbackFunction;	   
	return 0;
}

// Set mode to run in when using historical tapes (full, rt-sim, etc)
//------------------------------------------------------------
int JTNX_Interface_SetHistMode(void *pUserData,
					           unsigned char Mode,int RTDelay)
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
    pAppUserData->UserParms.SlaveHistMode=Mode;
	pAppUserData->UserParms.SlaveHistDelay=RTDelay;
	pAppUserData->UserParms.StepRelease=1;
	return 0;
}

// Set flags to exclude items in NxCore processing
//------------------------------------------------------------
int JTNX_Interface_SetExcludeFlags(void *pUserData,
					               unsigned char Quotes,unsigned char L2Quotes,unsigned char OPRA,unsigned char CRC)
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
	pAppUserData->UserParms.ExcludeFlags=0;
	if (Quotes)   pAppUserData->UserParms.ExcludeFlags|=NxCF_EXCLUDE_QUOTES;
	if (L2Quotes) pAppUserData->UserParms.ExcludeFlags|=NxCF_EXCLUDE_QUOTES2;
	if (OPRA)     pAppUserData->UserParms.ExcludeFlags|=NxCF_EXCLUDE_OPRA;
    if (CRC)     pAppUserData->UserParms.ExcludeFlags|=NxCF_EXCLUDE_CRC_CHECK;
	return 0;
}

// Start NxCore processing 
//------------------------------------------------------------
int JTNX_Interface_Start(void *pUserData,
				         char *TapeFileName,
						 char *DLLFilename,
                         unsigned char IsThreaded)
{	
	DWORD idThread;
    HANDLE hndThread;
    

	JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;

	pAppUserData->ErrorCode                =0;
	pAppUserData->IsNxCoreRunning          =0;
	pAppUserData->UserParms.StopFromUser   =0;	
    pAppUserData->UserParms.AppCommand     = NXCORE_STARTCORE;  
    pAppUserData->UserParms.TapeFileName[0]=0;
	pAppUserData->UserParms.DLLFileName[0]=0;
	if (TapeFileName)
	    strcpy(pAppUserData->UserParms.TapeFileName,TapeFileName);   	
	if (DLLFilename)
	    strcpy(pAppUserData->UserParms.DLLFileName,DLLFilename);   	
	
    if (pAppUserData->AppLog)
      JTLog_WriteToFile(pAppUserData->AppLog,"Starting NxCore");
   
    // If sys is multi-threaded start ProcessTape as new thread...
    if (IsThreaded)
    {
      // Create a block of memory to pass to the new thread we are creating. 
      // This function returns before the thread starts -- 
      // You can't pass variables from the stack because they go out of scope before the thread ThreadNxCoreAPI starts.  
	  hndThread = ::CreateThread(0,0,(LPTHREAD_START_ROUTINE)JTNX_Sys_StartNxCore,(void *)pAppUserData,0,&idThread);
      if (pAppUserData->AppLog)
	     JTLog_WriteToFile(pAppUserData->AppLog,"Exiting Start Function");
    }
    // Else just make call to start ProcessTape 
    else
    {
      // Start Non-Threaded.  This function will not return until ProcessTape() exits
      JTNX_Sys_StartNxCore((void *)pAppUserData);
    }
		
	return(pAppUserData->ErrorCode);
}

// Stop NxCore processing 
//------------------------------------------------------------
int JTNX_Interface_Stop(void *pUserData)
{	
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
    
    if (pAppUserData->AppLog)
	  JTLog_WriteToFile(pAppUserData->AppLog,"NxCore Disconnect");	

	if (pAppUserData->IsNxCoreRunning)
	{
	    pAppUserData->UserParms.StopFromUser=1;	
	    pAppUserData->UserParms.AppCommand  = NXCORE_STOPCORE;	
	    while (pAppUserData->IsNxCoreRunning)	Sleep(50);
	}

	UnloadNxCore();
    return 0;
}

// Trigger a stop
//------------------------------------------------------------
int JTNX_Interface_TriggerStop(void* pUserData)
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
	pAppUserData->UserParms.StopFromUser=1;	
	pAppUserData->UserParms.AppCommand  = NXCORE_STOPCORE;
	return 0;
}

// Ping step mode to next interval
//------------------------------------------------------------
int JTNX_Interface_PingRelease(void *pUserData)
{
    JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType*)pUserData;
    pAppUserData->UserParms.StepRelease=1;
	return 0;
}

//------------------------------------------------------------
// Load and start NxCore
//------------------------------------------------------------
UINT JTNX_Sys_StartNxCore(void* pVoid)
{  	
	// Grab user data and set error code to none
	JTNX_AppUserDataType *pAppUserData=(JTNX_AppUserDataType *)pVoid;
	pAppUserData->ErrorCode=NXCORE_NOERROR;
	
	// Load NxCore DLL
	if (!LoadNxCore(pAppUserData->UserParms.DLLFileName))
	{
      if (pAppUserData->AppLog)
      {
	    char str[255];	  
	    JTLog_WriteToFile(pAppUserData->AppLog,"ERROR! Could Not Load NxCore DLL!");
	    sprintf(str,"Filename=%s",pAppUserData->UserParms.DLLFileName);
	    JTLog_WriteToFile(pAppUserData->AppLog,str);
      }
	  
	  pAppUserData->IsNxCoreRunning=0;
	  pAppUserData->ErrorCode=NXCORE_DLLLOADERROR;
	  if (pAppUserData->JTNX_ErrorCallBack) 
	  {	      
	      pAppUserData->JTNX_ErrorCallBack(NXCORE_DLLLOADERROR);
	  }
	  return 1;
	}
	
    if (pAppUserData->AppLog)
	  JTLog_WriteToFile(pAppUserData->AppLog,"NxCore DLL Loaded");
	
	// Reset Globals
	pAppUserData->UserParms.StopFromUser=0;
	pAppUserData->IsNxCoreRunning=1;		 			 		        
    pAppUserData->MessageCount=0;
    pAppUserData->RunStatus=-1;
    pAppUserData->SpinComplete=0;
    pAppUserData->FirstTime=1;

    for (int loop=0;loop<MAXEXCHANGES;loop++)
    {
      if (pAppUserData->pChartData->Array[loop]) 
      {        
        if (pAppUserData->pChartData->Array[loop]->Last) free(pAppUserData->pChartData->Array[loop]->Last);
        if (pAppUserData->pChartData->Array[loop]->BidLast) free(pAppUserData->pChartData->Array[loop]->BidLast);  
        if (pAppUserData->pChartData->Array[loop]->AskLast) free(pAppUserData->pChartData->Array[loop]->AskLast);
        if (pAppUserData->pChartData->Array[loop]->TCondition) free(pAppUserData->pChartData->Array[loop]->TCondition);
        if (pAppUserData->pChartData->Array[loop]->Volume) free(pAppUserData->pChartData->Array[loop]->Volume);
        if (pAppUserData->pChartData->Array[loop]->BidVolume) free(pAppUserData->pChartData->Array[loop]->BidVolume);
        if (pAppUserData->pChartData->Array[loop]->AskVolume) free(pAppUserData->pChartData->Array[loop]->AskVolume);

        pAppUserData->pChartData->Array[loop]->Last=NULL;
        pAppUserData->pChartData->Array[loop]->BidLast=NULL;
        pAppUserData->pChartData->Array[loop]->AskLast=NULL;
        pAppUserData->pChartData->Array[loop]->TCondition=NULL;
        pAppUserData->pChartData->Array[loop]->Volume=NULL;
        pAppUserData->pChartData->Array[loop]->BidVolume=NULL;
        pAppUserData->pChartData->Array[loop]->AskVolume=NULL;

        free(pAppUserData->pChartData->Array[loop]);
      }
      pAppUserData->pChartData->Array[loop]=NULL;      
    }
    if (pAppUserData->pChartData->BBOBidBand) free(pAppUserData->pChartData->BBOBidBand);  pAppUserData->pChartData->BBOBidBand=NULL;
    if (pAppUserData->pChartData->BBOAskBand) free(pAppUserData->pChartData->BBOAskBand);  pAppUserData->pChartData->BBOAskBand=NULL;    
    if (pAppUserData->pChartData->Hour) free(pAppUserData->pChartData->Hour);pAppUserData->pChartData->Hour=NULL;   
    if (pAppUserData->pChartData->Min) free(pAppUserData->pChartData->Min);  pAppUserData->pChartData->Min=NULL;   
    if (pAppUserData->pChartData->Sec) free(pAppUserData->pChartData->Sec);  pAppUserData->pChartData->Sec=NULL;   
    if (pAppUserData->pChartData->MSec) free(pAppUserData->pChartData->MSec);  pAppUserData->pChartData->MSec=NULL;   
        	
    if (pAppUserData->AppLog)
	  JTLog_WriteToFile(pAppUserData->AppLog,"NxCore Start Process Tape");
    
    // Start Process tape	
    if (pAppUserData->UserParms.TapeFileName[0]==0)		    	    
        pAppUserData->ErrorCode=pfNxCoreProcessTape("",0,
                                                    pAppUserData->UserParms.ExcludeFlags,
                                                    (int)pAppUserData,OnNxCoreCallback);	    
    else
        pAppUserData->ErrorCode=pfNxCoreProcessTape(pAppUserData->UserParms.TapeFileName,0,                                                   
                                                    pAppUserData->UserParms.ExcludeFlags,
                                                    (int)pAppUserData,OnNxCoreCallback);        
    
    // No longer running
	pAppUserData->IsNxCoreRunning=0;       
    pAppUserData->CurrSymbol=NULL;
    pAppUserData->TriggerLookup=0;
    pAppUserData->UserExg=0;
    pAppUserData->UserSymbol[0]=0;

	// If there was an error trying to start Process Tape then....
	if (pAppUserData->ErrorCode<0)
	{
      if (pAppUserData->AppLog)
      {
	    char str[255];	  	  
	    sprintf(str,"Process Tape Error! Error=%d",pAppUserData->ErrorCode);
	    JTLog_WriteToFile(pAppUserData->AppLog,str);
      }	    	  
	  if (pAppUserData->JTNX_ErrorCallBack) 
	      pAppUserData->JTNX_ErrorCallBack(pAppUserData->ErrorCode);

	  return 1;
	}
	
    if (pAppUserData->AppLog)
	  JTLog_WriteToFile(pAppUserData->AppLog,"NxCore Process Tape End");
    
	// If end of tape callback set, call it
	if (pAppUserData->JTNX_EndOfTapeCallBack)
	{
      if (pAppUserData->AppLog)
	    JTLog_WriteToFile(pAppUserData->AppLog,"Hail EndTape CallBack");
	  pAppUserData->JTNX_EndOfTapeCallBack();
	}

    if (pAppUserData->AppLog)
	  JTLog_WriteToFile(pAppUserData->AppLog,"Exit ProcessTape Start Thread");

	if (pAppUserData->JTNX_StatusDisplayCallBack)
	    pAppUserData->JTNX_StatusDisplayCallBack("NxCore is Stopped");
	
	return 0;
}


//------------------------------------------------------------//
//  NxCore Message Handlers                                   //
//------------------------------------------------------------//

// ExtractTapeVersion  - Extract Tape Version Number
//----------------------------------------------------------
void ExtractTapeVersion(char *TapeString,
					    char *VersionString,
					    int *Major,int *Minor,int *Sub)
{
  if ((!TapeString)||(!VersionString)) return;

  char *pt=&TapeString[12]; // Version num starts at char 13 in TapeString
  char *pv=VersionString;
  char extracted[3];  
  char *pe=extracted;
  unsigned char vpos=0;
  
  // Extract version string and numeric components   
  while ((*pt)&&(*pt!='\n'))
  {
	// '.' seperates the major/minor/sub version nums
	if (*pt=='.')
	{
	  // Extract Major and Minor versions
	  *pe=0;	  
	  if (vpos==0) *Major=atoi(extracted);			    
	  else	       *Minor=atoi(extracted);	
	  				 	
	  ++vpos;
	  *pv++ = *pt++;	  

	  pe=extracted;
	}
	else	
	  *pe++ = *pv++ = *pt++;		
  } 
  // Set version string end to null
  *pv=0;

  // Extract final Sub version
  *pe=0;
  *Sub=atoi(extracted);
}

// Handle NxCore STATUS messages
//------------------------------------------------------------
int processNxCoreStatus(const NxCoreSystem* pNxCoreSys, const NxCoreMessage* pNxCoreMsg)
{
    char str[256];

	// Grab pointer to user data
    JTNX_AppUserDataType* pAppUserData = (JTNX_AppUserDataType*) pNxCoreSys->UserData;

	//------------------------------//
	// NxCore System Clock Messages //
	//------------------------------//

    // If there is a timer callback (to display NxCore time), and
    // the interval of this message equals the GUI interval, and we
    // are not in step-mode, show the time.
    if ((pAppUserData->JTNX_TimeDisplayCallBack)&&
        (pNxCoreSys->ClockUpdateInterval>=pAppUserData->UserParms.NxClockGUIInterval)&&
        (pAppUserData->UserParms.SlaveHistMode!=TIMEMODE_STEPTIME))
        
    {
        pAppUserData->JTNX_TimeDisplayCallBack(pNxCoreSys,pNxCoreMsg,0);	
    }

	// If the NxCore clock just ticked by a full minute...
	if (pNxCoreSys->ClockUpdateInterval >= NxCLOCK_MINUTE )
	{		
	}

	// If the NxCore clock just ticked by a full second...
	if ((pNxCoreSys->ClockUpdateInterval >= NxCLOCK_SECOND ) && (!pAppUserData->UserParms.StopFromUser))
	{		    	    
		// Is the feed delayed?
		if (pAppUserData->UserParms.Delayed.IsDelayed)		
	    {		   		  	      		  
		  // While diff is less than delay
		  long Diff=0;
		  while (Diff<pAppUserData->UserParms.Delayed.MinutesDelayed)
		  {	
			  // Get the system time
              struct tm *ST;
              time_t SysDT; 
			  time(&SysDT);		   
              ST=localtime(&SysDT);
              int CSec=ST->tm_hour*3600+ST->tm_min*60+ST->tm_sec;		      
              Diff=(CSec - (pNxCoreSys->nxTime.MsOfDay/1000))/60;

			  // If diff is less than delay sleep a bit	
			  if (Diff<pAppUserData->UserParms.Delayed.MinutesDelayed)
			     Sleep(pAppUserData->UserParms.Delayed.SleepDelay);			  
		  }		  
	    }	
		
        if ((pAppUserData->UserParms.SlaveHistMode==TIMEMODE_STEPTIME)&&(pAppUserData->JTNX_TimeDisplayCallBack))
			 pAppUserData->JTNX_TimeDisplayCallBack(pNxCoreSys,pNxCoreMsg,1);	
	}

	// Is this a millisecond tick?
	if ((pAppUserData->UserParms.SlaveHistMode!=TIMEMODE_FULLTIME)&&(pNxCoreSys->ClockUpdateInterval >= NxCLOCK_CLOCK)) 
	{        
		// If we are in the RT-Sim mode, sleep one cycle and continue on
	    if (pAppUserData->UserParms.SlaveHistMode==TIMEMODE_SIMTIME)
            Sleep(pAppUserData->UserParms.SlaveHistDelay);

		// Else if we are in the step mode...
	    else if (pAppUserData->UserParms.SlaveHistMode==TIMEMODE_STEPTIME)
	    {
		    // Set step release flag to 0, format and print time
		   pAppUserData->UserParms.StepRelease=0;
		   if (pAppUserData->JTNX_TimeDisplayCallBack)
		       pAppUserData->JTNX_TimeDisplayCallBack(pNxCoreSys,pNxCoreMsg,1);	
		   

		   // Sleep while the use has not pressed the "NEXT" step button or the slave is not trying to exit...
		   while ((!pAppUserData->UserParms.StepRelease)&&(pAppUserData->UserParms.AppCommand  != NXCORE_STOPCORE))
		   {
			    Sleep(pAppUserData->UserParms.SlaveHistDelay);
		   }

		   // Format the new time upon exit. As the mode may have changed check it for correct format
		   if ((pAppUserData->UserParms.SlaveHistMode!=TIMEMODE_STEPTIME)&&(pAppUserData->JTNX_TimeDisplayCallBack))
           {
				 pAppUserData->JTNX_TimeDisplayCallBack(pNxCoreSys,pNxCoreMsg,0);	
           }
		   else
			   if (pAppUserData->JTNX_TimeDisplayCallBack)
				 pAppUserData->JTNX_TimeDisplayCallBack(pNxCoreSys,pNxCoreMsg,0);	
		}
	}

	//-------------------------------//
	// NxCore System Status Messages //
	//-------------------------------//
    switch( pNxCoreSys->Status ) 
    {
	  // Capture a SYMBOL SPIN Message
	  case NxCORESTATUS_SYMBOLSPIN:
		   switch (pNxCoreSys->StatusData)
		   {
			   // If initial symbol spin is complete
			   case NxCSSYMBOLSPIN_COMPLETE:
                    if (pAppUserData->AppLog)
				      JTLog_WriteToFileNX(pAppUserData->AppLog,
                                          (NxDate *)&pNxCoreSys->nxDate,
			                              (NxTime *)&pNxCoreSys->nxTime,
									      "Symbol Spin Complete Message.");        		   
                    pAppUserData->SpinComplete=1;
				    break;
		   }
	  	   break;

	  case NxCORESTATUS_COMPLETE:
           if (pAppUserData->AppLog)
		     JTLog_WriteToFileNX(pAppUserData->AppLog,
                                 (NxDate *)&pNxCoreSys->nxDate,
			                     (NxTime *)&pNxCoreSys->nxTime,
							     "NxCore Complete Message.");        		   
		   break;

      case NxCORESTATUS_INITIALIZING:
    	   pAppUserData->APIVersion = pfNxCoreAPIVersion();

           if (pAppUserData->AppLog)
           {
             sprintf(str,"NxCore Initialize Message. NxCoreAPI.dll Ver: %ld.%ld.%ld",
                      NxCORE_VER_MAJOR(pAppUserData->APIVersion),
                      NxCORE_VER_MINOR(pAppUserData->APIVersion),
                      NxCORE_VER_BUILD(pAppUserData->APIVersion));

		     JTLog_WriteToFileNX(pAppUserData->AppLog,
                                 (NxDate *)&pNxCoreSys->nxDate,
			                     (NxTime *)&pNxCoreSys->nxTime,
				  			     str);		
           }
		   
		   if (pAppUserData->JTNX_StatusDisplayCallBack)
		   	   pAppUserData->JTNX_StatusDisplayCallBack("NxCore has Started");

		   break;

      // Some common tasks may need to be done for both of these messages....
      case NxCORESTATUS_LOADED_STATE:
	  case NxCORESTATUS_TAPEOPEN:		   
           char VString[13];
           int Major,Minor,Sub;

           // If set for logging on TapeDate, this is the first time we can set it
           // IE this is the first message where NxDate becomes available 
           // (LOADED_STATE for state tapes and TAPEOPEN for regular tapes, realtime.
           if ((pAppUserData->AppLog)&&(pAppUserData->UserParms.NxDateResetLog))
           {             
             sprintf(str,"Switching to NxDate Log: %s_%02d%02d%04d.log",pAppUserData->AppLog->LogPrefix,pNxCoreSys->nxDate.Month,pNxCoreSys->nxDate.Day,pNxCoreSys->nxDate.Year);
             JTLog_WriteToFileNX(pAppUserData->AppLog,
                               (NxDate *)&pNxCoreSys->nxDate,
			                   (NxTime *)&pNxCoreSys->nxTime,
							    str);            
       
             JTLog_ResetFileName_Nx(pAppUserData->AppLog,(NxDate *)&pNxCoreSys->nxDate);
           }
           
           // Extract tape version from TapeDisplay string
           ExtractTapeVersion((char *)pNxCoreSys->TapeDisplay,VString,&Major,&Minor,&Sub);
       
           // If tape version is greater than 1.20.x then this is an OSI tape
           if ((Major>1)||((Major==1)&&(Minor>20))) pAppUserData->IsOSI=1;
		   else pAppUserData->IsOSI=0;

           // Now the differences - NxCORESTATUS_LOADED_STATE
           if ( pNxCoreSys->Status== NxCORESTATUS_LOADED_STATE)
           {
             // Get the API version.
             pAppUserData->APIVersion = pfNxCoreAPIVersion();             

             if (pAppUserData->AppLog)
             {             		   
		        sprintf(str,"NxCore Has Been Loaded From A State. Tape Ver: %s",VString);	
		        JTLog_WriteToFileNX(pAppUserData->AppLog,
                                 (NxDate *)&pNxCoreSys->nxDate,
			                     (NxTime *)&pNxCoreSys->nxTime,
								  str);		

		        sprintf(str,"NxCoreAPI.dll Ver: %ld.%ld.%ld",
                    NxCORE_VER_MAJOR(pAppUserData->APIVersion),
                    NxCORE_VER_MINOR(pAppUserData->APIVersion),
                    NxCORE_VER_BUILD(pAppUserData->APIVersion));


		         JTLog_WriteToFileNX(pAppUserData->AppLog,
                                   (NxDate *)&pNxCoreSys->nxDate,
			                       (NxTime *)&pNxCoreSys->nxTime,
								   str);		
             }
             pAppUserData->SpinComplete=1;
           }
           // Or NxCORESTATUS_TAPEOPEN
           else if (pAppUserData->AppLog)
           {
		     sprintf(str,"NxCore TapeOpen Message. Tape Ver: %s",VString);
		     JTLog_WriteToFileNX(pAppUserData->AppLog,
                               (NxDate *)&pNxCoreSys->nxDate,
			                   (NxTime *)&pNxCoreSys->nxTime,
							    str);
           }

           if ((pAppUserData->JTNX_StatusDisplayCallBack)||(pAppUserData->AppLog))
           {
             sprintf(str,"Processing %02d/%02d/%d",pNxCoreSys->nxDate.Month,pNxCoreSys->nxDate.Day,pNxCoreSys->nxDate.Year);	                 
             if (pAppUserData->JTNX_StatusDisplayCallBack)
		        pAppUserData->JTNX_StatusDisplayCallBack(str);

             if (pAppUserData->AppLog)
                JTLog_WriteToFileNX(pAppUserData->AppLog,
                                 (NxDate *)&pNxCoreSys->nxDate,
			                     (NxTime *)&pNxCoreSys->nxTime,str);		
           }
		   
           break;

	  case NxCORESTATUS_SYNCHRONIZING:
           if (pAppUserData->AppLog)
           {
		     JTLog_WriteToFileNX(pAppUserData->AppLog,
                                 (NxDate *)&pNxCoreSys->nxDate,
			                     (NxTime *)&pNxCoreSys->nxTime,
							     "NxCore Synchronizing Message.");
           }
		   break;

	  case NxCORESTATUS_WAITFORCOREACCESS:
           if (pAppUserData->AppLog)
           {
	         sprintf(str,"NxCore WaitForCoreAccess Message, Status=%s  Time=%02d:%02d:%02d",
			             pNxCoreSys->StatusDisplay,
   			             pNxCoreSys->nxTime.Hour,
					     pNxCoreSys->nxTime.Minute,
					     pNxCoreSys->nxTime.Second);
	         JTLog_WriteToFileNX(pAppUserData->AppLog,(NxDate *)&pNxCoreSys->nxDate,(NxTime *)&pNxCoreSys->nxTime,str);		   	
           }
		   break;

	  case NxCORESTATUS_RESTARTING_TAPE:
           if (pAppUserData->AppLog)
           {
		     JTLog_WriteToFileNX(pAppUserData->AppLog,
                                 (NxDate *)&pNxCoreSys->nxDate,
			                     (NxTime *)&pNxCoreSys->nxTime,
							     "NxCore Restart Tape Message.");
           }
		   break;

      case NxCORESTATUS_ERROR:
           pAppUserData->ErrorCode=-1;
           if (pAppUserData->AppLog)
           {
		     sprintf(str,"NxCore Error Message, Status=%s  Time=%02d:%02d:%02d",
			             pNxCoreSys->StatusDisplay,
   			             pNxCoreSys->nxTime.Hour,
					     pNxCoreSys->nxTime.Minute,
					     pNxCoreSys->nxTime.Second);
	
		   
		     JTLog_WriteToFileNX(pAppUserData->AppLog,
                               (NxDate *)&pNxCoreSys->nxDate,
			                   (NxTime *)&pNxCoreSys->nxTime,
							   str);		   
           }
		   break;
 
      case NxCORESTATUS_RUNNING:		           
           if (pNxCoreSys->StatusData!=pAppUserData->RunStatus)
           {
             pAppUserData->RunStatus=pNxCoreSys->StatusData;
             if (pAppUserData->AppLog)
             {
               switch(pAppUserData->RunStatus)
               {
                 case NxCSRUNMODE_SRC_DISK_FILE:        strcpy(str,"Processing from a tape on disk.");break;
                 case NxCSRUNMODE_SRC_ACCESS_FILE:      strcpy(str,"Processing NxCoreAccess's file, dll has not yet reached memory buffers.");break;               
                 case NxCSRUNMODE_SRC_ACCESS_MB_OLDEST: strcpy(str,"Processing oldest of NxCoreAccess's memory buffers.");break;
                 case NxCSRUNMODE_SRC_ACCESS_MB_LOWER:  strcpy(str,"Processing older half of NxCoreAccess's memory buffers.");break;
                 case NxCSRUNMODE_SRC_ACCESS_MB_UPPER:  strcpy(str,"Processing most recent half of NxCoreAccess's memory buffers.");break;
                 case NxCSRUNMODE_SRC_ACCESS_MB_SECOND: strcpy(str,"Processing 2nd most recent of NxCoreAccess's memory buffers.");break;
                 case NxCSRUNMODE_SRC_ACCESS_MB_CURRENT:strcpy(str,"Processing most recent of NxCoreAccess's memory buffers.");break;
                 default:sprintf(str,"Run Status=%d",pAppUserData->RunStatus);break;
               }
               JTLog_WriteToFileNX(pAppUserData->AppLog,
                                   (NxDate *)&pNxCoreSys->nxDate,
			                       (NxTime *)&pNxCoreSys->nxTime,
								    str);		   
             }
           }
		   break;

    }

	return NxCALLBACKRETURN_CONTINUE;
}

//------------------------------------------------------------
void processNxCoreSymbolSpin(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{
  // Put symbol spin handler here
  JTNX_AppUserDataType* pAppUserData = (JTNX_AppUserDataType*) pNxCoreSys->UserData;

  // No pinks, no Canadian issues
  if ((pNxCoreMsg->coreHeader.ListedExg==62)&&(pAppUserData->UserExg!=62)) return;
  if ((pNxCoreMsg->coreHeader.ListedExg==19)&&(pAppUserData->UserExg!=19)) return;
  if ((pNxCoreMsg->coreHeader.ListedExg==18)&&(pAppUserData->UserExg!=18)) return;

  if (
     (strcmp(pNxCoreMsg->coreHeader.pnxStringSymbol->String,pAppUserData->UserSymbol)==0)&&
     ((pAppUserData->UserExg==0)||(pAppUserData->UserExg==pNxCoreMsg->coreHeader.ListedExg))
     )
  {
              
    pNxCoreMsg->coreHeader.pnxStringSymbol->UserData1=1;
    
    pAppUserData->CurrSymbol=pNxCoreMsg->coreHeader.pnxStringSymbol;
    if (pAppUserData->JTNX_MessageCallBack)
    {
      pAppUserData->JTNX_MessageCallBack(204,pNxCoreSys,pNxCoreMsg);
      pAppUserData->JTNX_MessageCallBack(203,pNxCoreSys,pNxCoreMsg);    
    }
    if (pAppUserData->JTNX_MessageCallBack)
      pAppUserData->JTNX_MessageCallBack(201,pNxCoreSys,pNxCoreMsg);
  }
  else
  {
    pNxCoreMsg->coreHeader.pnxStringSymbol->UserData1=0;
  }
}


//------------------------------------------------------------
int __stdcall NxCore_SymbolSpin(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{
  processNxCoreSymbolSpin(pNxCoreSys,pNxCoreMsg);  
  return NxCALLBACKRETURN_CONTINUE;
}
//------------------------------------------------------------
void processNxCoreSymbolChange(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{
  // Put symbol change handler here
}
//------------------------------------------------------------
void processNxCoreCategory(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{
}
//------------------------------------------------------------
void processNxCoreTrade(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{
  // Put trade message handler here
}
//------------------------------------------------------------
void processNxCoreExgQuote(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{
  // Put exg quote message handler here  
}
//------------------------------------------------------------
void processNxCoreMMQuote(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{
  // Put mm quote message handler here
}


//------------------------------------------------------------
// The Main NxCore Callback
//------------------------------------------------------------
int __stdcall OnNxCoreCallback(const NxCoreSystem* pNxCoreSys,const NxCoreMessage* pNxCoreMsg)
{    
    // Grab a pointer to the user data
    JTNX_AppUserDataType* pAppUserData = (JTNX_AppUserDataType*) pNxCoreSys->UserData;

    // If first time in the callback allocate all the data we will need.  We do this here
    // because there is a good deal of data, and allocating it before ProcessTape might prevent
    // a state file from starting properly, so allocate it here to be sure.
    if (pAppUserData->FirstTime)
    {
      pAppUserData->FirstTime=0;
      for (int loop=0;loop<MAXEXCHANGES;loop++)
      {
        pAppUserData->pChartData->Array[loop]=(QTRecordType *)calloc(1,sizeof(QTRecordType));    
        if (pAppUserData->pChartData->Array[loop])
        {
          pAppUserData->pChartData->Array[loop]->Last=(double *)calloc(pAppUserData->MAXBARS,sizeof(double));    
          pAppUserData->pChartData->Array[loop]->BidLast=(double *)calloc(pAppUserData->MAXBARS,sizeof(double));    
          pAppUserData->pChartData->Array[loop]->AskLast=(double *)calloc(pAppUserData->MAXBARS,sizeof(double));    
          pAppUserData->pChartData->Array[loop]->TCondition=(unsigned char *)calloc(pAppUserData->MAXBARS,sizeof(unsigned char));    
          pAppUserData->pChartData->Array[loop]->Volume=(unsigned int *)calloc(pAppUserData->MAXBARS,sizeof(unsigned int));    
          pAppUserData->pChartData->Array[loop]->BidVolume=(unsigned int *)calloc(pAppUserData->MAXBARS,sizeof(unsigned int));    
          pAppUserData->pChartData->Array[loop]->AskVolume=(unsigned int *)calloc(pAppUserData->MAXBARS,sizeof(unsigned int));    
        }
      }

      pAppUserData->pChartData->BBOBidBand=(double *)calloc(pAppUserData->MAXBARS,sizeof(double));    
      pAppUserData->pChartData->BBOAskBand=(double *)calloc(pAppUserData->MAXBARS,sizeof(double));    

      pAppUserData->pChartData->Hour=(unsigned char *)calloc(pAppUserData->MAXBARS,sizeof(unsigned char));    
      pAppUserData->pChartData->Min=(unsigned char *)calloc(pAppUserData->MAXBARS,sizeof(unsigned char));    
      pAppUserData->pChartData->Sec=(unsigned char *)calloc(pAppUserData->MAXBARS,sizeof(unsigned char));    
      pAppUserData->pChartData->MSec=(unsigned short *)calloc(pAppUserData->MAXBARS,sizeof(unsigned short));    
      pAppUserData->pChartData->MSTOD=(unsigned long *)calloc(pAppUserData->MAXBARS,sizeof(unsigned long));    

    } // If FirstTime

    // If the user has triggered a lookup and the spin is complere....
    if ((pAppUserData->TriggerLookup)&&(pAppUserData->SpinComplete))
    {
      pAppUserData->TriggerLookup=0;


      for (int loop=0;loop<MAXEXCHANGES;loop++)
      {
         if (pAppUserData->pChartData->Array[loop])
         {
            pAppUserData->pChartData->Array[loop]->TradeOn=pAppUserData->pChartData->Array[loop]->BidOn=pAppUserData->pChartData->Array[loop]->AskOn=0;
            pAppUserData->pChartData->Array[loop]->UToggleTradeOff=pAppUserData->pChartData->Array[loop]->UToggleBidOff=pAppUserData->pChartData->Array[loop]->UToggleAskOff=0;    
            pAppUserData->pChartData->Array[loop]->TotalTradeCount=pAppUserData->pChartData->Array[loop]->TotalQuoteCount=0;

            memset(pAppUserData->pChartData->Array[loop]->Last,0,sizeof(double)*pAppUserData->MAXBARS);
            memset(pAppUserData->pChartData->Array[loop]->BidLast,0,sizeof(double)*pAppUserData->MAXBARS);
            memset(pAppUserData->pChartData->Array[loop]->AskLast,0,sizeof(double)*pAppUserData->MAXBARS);
            memset(pAppUserData->pChartData->Array[loop]->TCondition,0,sizeof(unsigned char)*pAppUserData->MAXBARS);
            memset(pAppUserData->pChartData->Array[loop]->Volume,0,sizeof(unsigned int)*pAppUserData->MAXBARS); 
            memset(pAppUserData->pChartData->Array[loop]->BidVolume,0,sizeof(unsigned int)*pAppUserData->MAXBARS);
            memset(pAppUserData->pChartData->Array[loop]->AskVolume,0,sizeof(unsigned int)*pAppUserData->MAXBARS);
         }
      }
      memset(pAppUserData->pChartData->BBOBidBand,0,pAppUserData->MAXBARS*sizeof(double));    
      memset(pAppUserData->pChartData->BBOAskBand,0,pAppUserData->MAXBARS*sizeof(double));    

      pfNxCoreSpinSymbols((unsigned short)-1,0,200,NxCore_SymbolSpin,0);			
      if (pAppUserData->JTNX_MessageCallBack)
      {
        if (!pAppUserData->CurrSymbol)
          pAppUserData->JTNX_MessageCallBack(200,pNxCoreSys,pNxCoreMsg);
        else
          pAppUserData->JTNX_MessageCallBack(201,pNxCoreSys,pNxCoreMsg);
      }
    }
    
	// Inc the message counter
	++pAppUserData->MessageCount;

	// Process the message
	switch( pNxCoreMsg->MessageType ) 
    {
	      case NxMSG_STATUS:         {
			                         int Status=processNxCoreStatus(pNxCoreSys,pNxCoreMsg);
		                             if (Status==NxCALLBACKRETURN_STOP)
			   					     {								 							   	         
							   	         pAppUserData->IsNxCoreRunning=0;		
                                         return NxCALLBACKRETURN_STOP;		    
								     }								     

									 }
								     break;
	      
          case NxMSG_SYMBOLSPIN:     processNxCoreSymbolSpin(pNxCoreSys,pNxCoreMsg);break;
          case NxMSG_CATEGORY:   	 processNxCoreCategory(pNxCoreSys,pNxCoreMsg);break;
		  case NxMSG_TRADE:          if ((pNxCoreMsg->coreHeader.pnxStringSymbol->UserData1)&&(pAppUserData->JTNX_MessageCallBack))
                                     {
                                         pAppUserData->JTNX_MessageCallBack(NxMSG_TRADE,pNxCoreSys,pNxCoreMsg);                                       
                                     }
                                     break;
          case NxMSG_EXGQUOTE:       if ((pNxCoreMsg->coreHeader.pnxStringSymbol->UserData1)&&(pAppUserData->JTNX_MessageCallBack))
                                     {
                                         pAppUserData->JTNX_MessageCallBack(NxMSG_EXGQUOTE,pNxCoreSys,pNxCoreMsg);                                       
                                     }
                                     break;

    }
	    
	// Process any commands from GUI
	if( pAppUserData->UserParms.AppCommand ) 
	{
	    int AppCommand = pAppUserData->UserParms.AppCommand;
	    pAppUserData->UserParms.AppCommand = 0;
	    switch( AppCommand ) 
		{		
	        case NXCORE_STOPCORE:				
				 pAppUserData->IsNxCoreRunning=0;				                  
				 return NxCALLBACKRETURN_STOP;		         
				 break;

		}
	}
  
    return NxCALLBACKRETURN_CONTINUE;
}



} // extern "c"
