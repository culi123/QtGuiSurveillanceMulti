#ifndef LOGINFO_H
#define LOGINFO_H
#pragma once
#include "stdafx.h"
#define BUFFSIZE 8192
class logInfo
{
public:
	logInfo(void);
	~logInfo(void);

	//日志文件
	FILE* m_pfLogFile;
	char m_logFileName[1000];
	char m_cInfo[BUFFSIZE];
	char systime[20];
	int g_logType = 3; //0:不输出; 1:输出到文件; 2:输出到屏幕; 3:输出到文件和屏幕; 

	int  SetLogPath(const char *pLogPath);
	int  WriteLogInfo(const char *pInfo);
	void flushLog(void);
	void GetSysTime(void);
	void printLog(const char *format, ...);
	void setLogType(int logType);
};
#endif // LOGINFO_H
