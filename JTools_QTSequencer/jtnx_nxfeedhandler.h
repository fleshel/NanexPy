#ifndef JWX_NXFEEDHANDLER_H
#define JWX_NXFEEDHANDLER_H

#include "..\JT_Common\JT_SimpleLog.h"  // Logging function
#include "QTSeqChartTypes.h"

#define NXCORE_NOERROR 0
#define NXCORE_DLLLOADERROR 1
#define NXCORE_MEMALLOCATIONERROR 2

#define NXCORE_STARTCORE 1
#define NXCORE_STOPCORE	2
#define NXCORE_RESETCORE 10

#define NXCORE_REALTIME 1
#define NXCORE_TAPE 2

#define TIMEMODE_FULLTIME 1
#define TIMEMODE_SIMTIME 2
#define TIMEMODE_STEPTIME 3

extern "C"
{
void *JTNX_Interface_AllocateGlobalUserData(void);
int JTNX_Interface_FreeGlobalUserData(void *pUserData);

__int64 JTNX_Interface_GetMsgCount(void *pUserData);
__int64 JTNX_Interface_GetRestMsgCount(void *pUserData);

int JTNX_Interface_SetAppLog(void *pUserData,JTLogStruct *pLog);								 
int JTNX_Interface_SetLogResetNxDate(void *pUserData,unsigned char ResetOnNXDate);
int JTNX_Interface_SetNxClockGUIInterval(void *pUserData,unsigned char NxClockGUIInterval);
int JTNX_Interface_SetTimeDisplayCallback(void *pUserData,int (*CallbackFunction)(const NxCoreSystem*,const NxCoreMessage*,unsigned char));
int JTNX_Interface_SetStatusDisplayCallback(void *pUserData,int (*CallbackFunction)(char *str));
int JTNX_Interface_SetEndOfTapeCallback(void *pUserData,int (*CallbackFunction)(void));
int JTNX_Interface_SetErrorCallback(void *pUserData,int (*CallbackFunction)(int ErrorCode));

int JTNX_Interface_Start(void *pUserData,char *TapeFileName,char *DllFileName,unsigned char IsThreaded);
int JTNX_Interface_Stop(void *pUserData);
int JTNX_Interface_SetHistMode(void *pUserData,unsigned char Mode,int RTDelay);
int JTNX_Interface_PingRelease(void *pUserData);
int JTNX_Interface_TriggerStop(void *pUserData);

int JTNX_Interface_SetExcludeFlags(void *pUserData,
					               unsigned char Quotes,
								   unsigned char L2Quotes,
								   unsigned char OPRA,
								   unsigned char CRC);
int JTNX_Interface_SetFeedDelay(void *pData,
								 unsigned char IsDelayed,
								 unsigned int MinDelayed,
								 unsigned short SleepDelay);

void JTNX_Interface_SetUserSymbol(void *pUserData,char *Symbol,unsigned char Exg);
int JTNX_Interface_SetMessageCallback(void *pUserData,int (*CallbackFunction)(int,const NxCoreSystem*,const NxCoreMessage*));
void JTNX_Interface_SetChartDataPtr(void *pUserData,BarIndicatorType *Data);
void JTNX_Interface_SetMaxBars(void *pUserData,int Max);

}

#endif