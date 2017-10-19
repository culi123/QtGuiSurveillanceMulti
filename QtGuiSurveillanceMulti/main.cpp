#include "QtGuiSurveillanceMulti.h"
#include <QtWidgets/QApplication>
#include"stdafx.h"
using namespace std;
//定义全局变量
HANDLE hMutex = CreateMutex(NULL, false, NULL);
map<string, Display_GUI>Display;//需要输出到GUI所保存的数据


logInfo *pLog;


string GetExePath()
{
	char path[MAX_PATH];
	wchar_t path_w[MAX_PATH];
	GetModuleFileName(NULL, path_w, sizeof(path_w));
	(wcsrchr(path_w, _T('\\')))[1] = 0;
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, path_w, -1, path, MAX_PATH, NULL, NULL);
	string path_str = path;
	return path_str;
}
AccountInfo GetAccountInfo(string account_name)
{
	string path = GetExePath();
	string fn_plus = path + account_name + "/账号信息.txt";
	ifstream in_file(fn_plus);
	string s;
	AccountInfo account_info;
	while (!in_file.is_open())
	{
		cout << "读取***" + account_name + "***账号信息文件失败 请确保该文件存在并且文件名正确" << endl;
		Sleep(10000);
		in_file.open(fn_plus);
	}
	int item_count = 0;
	bool bIsGetTdAddress = false;
	bool bIsGetMdAddress = false;
	while (getline(in_file, s))
	{
		stringstream line(s);
		string name;
		line >> name;
		string info;
		line >> info;
		if (strcmp(name.c_str(), "交易前端") == 0)
		{
			account_info.TdAddress.push_back(info);
			bIsGetTdAddress = true;
		}
		else if (strcmp(name.c_str(), "行情前端") == 0)
		{
			account_info.MdAddress.push_back(info);
			bIsGetMdAddress = true;
		}
		else if (strcmp(name.c_str(), "经纪商代码") == 0)
		{
			account_info.BrokerID = info;
			item_count++;
		}
		else if (strcmp(name.c_str(), "账号") == 0)
		{
			account_info.UserID = info;
			item_count++;
		}
		else if (strcmp(name.c_str(), "密码") == 0)
		{
			account_info.Password = info;
			item_count++;
		}
		else if (strcmp(name.c_str(), "order文档名头部") == 0)
		{
			account_info.PositionFileHead = info;
			item_count++;
		}
		else if (strcmp(name.c_str(), "order文档名尾部") == 0)
		{
			account_info.PositionFileTail = info;
			item_count++;
		}
		else if (strcmp(name.c_str(), "账号名称") == 0)
		{
			account_info.AccountName = info;
			item_count++;
		}
		else
		{
			;
		}
	}
	if (bIsGetTdAddress && bIsGetMdAddress)
	{
		item_count++;
		item_count++;
	}
	if (item_count < 8)
	{
		cout << "***" + account_name + "*** 账号信息文件格式错误 该线程已退出" << endl;
		ExitThread(0);
	}
	return account_info;
}
DWORD WINAPI TradeAccount(LPVOID lpParameter)
{
	string account_name = (*(string *)lpParameter);
	AccountInfo account_info = GetAccountInfo(account_name);
	CFTTD * pTdHandler = new CFTTD();
	CFTMD * pMdHandler = new CFTMD();

	pTdHandler->Init(account_info, pMdHandler);
	pMdHandler->Init(account_info, pTdHandler);

	pTdHandler->QueryAcct();
	pTdHandler->GetCurrentPosition1();
	pTdHandler->Run();
	return 0;
}
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QtGuiSurveillanceMulti w;
	w.show();
	

	//获取账号列表
	list<string> accounts;
	string path = GetExePath();
	string accounts_file_fn = path + "账号列表.txt";
	ifstream in_file(accounts_file_fn);
	if (in_file.good() && in_file.is_open())
	{
		string s;
		while (getline(in_file, s))
		{
			stringstream line(s);
			string account_s;
			line >> account_s;
			accounts.push_back(account_s);
			Display_GUI a;
			Display.insert({ account_s,a });
		}
	}
	else
	{
		cout << "获取账号列表失败" << endl;
		exit(-1);
	}
	//log
	logInfo log;
	pLog = &log;
	string log_path = path + "\\DebugLogs";
	pLog->SetLogPath(log_path.c_str());
	list<HANDLE> hThreads;
	for (list<string>::iterator it = accounts.begin(); it != accounts.end(); it++)
	{
		HANDLE hThread = CreateThread(NULL, 0, TradeAccount, &(*it), 0, NULL);
		hThreads.push_back(hThread);
	}

	return a.exec();
}
