#include "stdafx.h"
using namespace std;
extern map<string, Display_GUI>Display;
extern logInfo *pLog;
CFTMD::CFTMD()
{

}

CFTMD::~CFTMD()
{
	
}

void CFTMD::Init(AccountInfo account_info, CFTTD* pTdHandler)
{
	// 产生一个CThostFtdcMdApi实例
	memset(qh_BrokerID, 0, sizeof(qh_BrokerID));
	memset(qh_MDAddress, 0, sizeof(qh_MDAddress));
	memset(qh_UserID, 0, sizeof(qh_UserID));
	memset(qh_Password, 0, sizeof(qh_Password));
	strcpy_s(qh_BrokerID, account_info.BrokerID.c_str());
	strcpy_s(qh_UserID, account_info.UserID.c_str());
	strcpy_s(qh_Password, account_info.Password.c_str());
	g_pTdHandler = pTdHandler;
	g_AccountInfo = account_info;
	m_pMdApi = CThostFtdcMdApi::CreateFtdcMdApi();
	m_pMdApi->RegisterSpi(this);
	for (list<string>::iterator it = account_info.MdAddress.begin(); it != account_info.MdAddress.end(); it++)
	{
		strcpy_s(qh_MDAddress, (*it).c_str());
		m_pMdApi->RegisterFront(qh_MDAddress);
	}
	hEvent = CreateEvent(NULL, false, false, NULL);
	m_pMdApi->Init();
	if (WaitForSingleObject(hEvent, 30000) == WAIT_TIMEOUT)
	{
		printf("***%s*** 行情前端登录错误 该线程已退出\n", g_pTdHandler->g_AccountInfo.AccountName.c_str());
		nowtime = time(NULL); //获取日历时间
		localtime_s(&local, &nowtime);  //获取当前系统时间
		char filepre_tm[18];
		strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
		Display[g_AccountInfo.AccountName].log.push_back(string(filepre_tm)+string(":Market front landed failed !!!"));
	}

		
	
}

void CFTMD::OnFrontConnected()
{
	nowtime = time(NULL); //获取日历时间
	localtime_s(&local, &nowtime);  //获取当前系统时间
	char filepre_tm[18];
	strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
	Display[g_AccountInfo.AccountName].log.push_back(string(filepre_tm) + string(":Market front connected successfully!!!"));
	CThostFtdcReqUserLoginField reqUserLogin;
	memset(&reqUserLogin,0,sizeof(reqUserLogin));
	strcpy_s(reqUserLogin.BrokerID, qh_BrokerID);
	strcpy_s(reqUserLogin.UserID, qh_UserID);
	strcpy_s(reqUserLogin.Password, qh_Password);
	int login = m_pMdApi->ReqUserLogin(&reqUserLogin,1);
}
void CFTMD::OnFrontDisconnected(int nReason)
{
	printf("***%s*** 行情连接断开 原因代码:%d\n", g_pTdHandler->g_AccountInfo.AccountName.c_str(), nReason);
	nowtime = time(NULL); //获取日历时间
	localtime_s(&local, &nowtime);  //获取当前系统时间
	char filepre_tm[18];
	strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
	Display[g_AccountInfo.AccountName].log.push_back(string(filepre_tm) + string(":The market connection is broken !!!"));
}
void CFTMD::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo->ErrorID == 0)
	{
		initSubMD();
		nowtime = time(NULL); //获取日历时间
		localtime_s(&local, &nowtime);  //获取当前系统时间
		char filepre_tm[18];
		strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
		Display[g_AccountInfo.AccountName].log.push_back(string(filepre_tm) + string(":Market front landed successfully!!!"));
		
	}
	else
	{
		nowtime = time(NULL); //获取日历时间
		localtime_s(&local, &nowtime);  //获取当前系统时间
		char filepre_tm[18];
		strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
		Display[g_AccountInfo.AccountName].log.push_back(string(filepre_tm) + string(":Market front landing error!!!"));
		printf("***%s*** 行情前端登陆错误 ErrorID=%d ErrorMsg=%s 当前日期=%s\n", g_pTdHandler->g_AccountInfo.AccountName.c_str(), pRspInfo->ErrorID, pRspInfo->ErrorMsg, pRspUserLogin->TradingDay);
	}
}

void CFTMD::initSubMD()
{
    //根据合约列表订阅行情
	int n_count = g_pTdHandler->Instruments.size();
	char ** codes = new char*[n_count];
	TThostFtdcInstrumentIDType* InstrumentIDs = new TThostFtdcInstrumentIDType[n_count];
	int i = 0;
	for (list<CThostFtdcInstrumentField>::iterator it = g_pTdHandler->Instruments.begin(); it!=g_pTdHandler->Instruments.end(); it++)
	{
		strcpy_s(InstrumentIDs[i], it->InstrumentID);
		codes[i] = InstrumentIDs[i];
		i++;
	}
	m_pMdApi->SubscribeMarketData(codes, n_count);

	delete[] codes;
	delete[] InstrumentIDs;
}

void CFTMD::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast)
		SetEvent(hEvent);
}
void CFTMD::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) 
{
	CThostFtdcDepthMarketDataField *pMD;
	LastDepth[pDepthMarketData->InstrumentID] = *pDepthMarketData;
	pMD = &LastDepth[pDepthMarketData->InstrumentID];
	pMD->LastPrice = (pMD->LastPrice > 10000000.0) ? 0 : pMD->LastPrice;                          ///最新价
	pMD->OpenPrice = (pMD->OpenPrice > 10000000.0) ? pMD->LastPrice : pMD->OpenPrice;             ///今开盘
	pMD->HighestPrice = (pMD->HighestPrice > 10000000.0) ? pMD->LastPrice : pMD->HighestPrice;    ///最高价
	pMD->LowestPrice = (pMD->LowestPrice > 10000000.0) ? pMD->LastPrice : pMD->LowestPrice;       ///最低价
	pMD->BidPrice1 = (pMD->BidPrice1 > 10000000.0) ? pMD->LastPrice : pMD->BidPrice1;             ///申买价一
	pMD->AskPrice1 = (pMD->AskPrice1 > 10000000.0) ? pMD->LastPrice : pMD->AskPrice1;             ///申卖价一
	pMD->AveragePrice = (pMD->AveragePrice > 10000000.0) ? pMD->LastPrice : pMD->AveragePrice;    ///当日均
	string code ;
	string direction;
	Display_Position_Key key;
	code = string(pDepthMarketData->InstrumentID);

	
	char *p = new char[g_AccountInfo.AccountName.length() + 1]();
	strcpy(p, g_AccountInfo.AccountName.c_str());
	if (Display[g_AccountInfo.AccountName].hEventUi)
	{

		//等待持仓信息刷新结束后，再更新数据，否则刷新持仓信息数据时，会清空Display.position1，导致空map引用，导致报错。
		direction = "空";
		key.ID = code;
		key.Direction = direction;
		if (Display[g_AccountInfo.AccountName].position1.find(key) != Display[g_AccountInfo.AccountName].position1.end())
		{
			Display[g_AccountInfo.AccountName].position1[key].LastPrice = pMD->LastPrice;
			Display[g_AccountInfo.AccountName].position1[key].T1CLose = pMD->PreClosePrice;
			Display[g_AccountInfo.AccountName].position1[key].T1Settle = pMD->PreSettlementPrice;
			Display[g_AccountInfo.AccountName].position1[key].PctClose = Display[g_AccountInfo.AccountName].position1[key].LastPrice / Display[g_AccountInfo.AccountName].position1[key].T1CLose - 1;
			Display[g_AccountInfo.AccountName].position1[key].PctSettle = Display[g_AccountInfo.AccountName].position1[key].LastPrice / Display[g_AccountInfo.AccountName].position1[key].T1Settle - 1;
			if (Display[g_AccountInfo.AccountName].position1[key].CostClose < 1)
			{
				Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose = 0;
				nowtime = time(NULL); //获取日历时间
				localtime_s(&local, &nowtime);  //获取当前系统时间
				char filepre_tm[18];
				strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
				if (local.tm_hour <= 15 && local.tm_hour >= 9)
					bIsCostZero = true;				
				pLog->printLog("(%s) (%s)\n %s的CostClose为%f\n", g_AccountInfo.AccountName.c_str(), filepre_tm, key.ID.c_str(), Display[g_AccountInfo.AccountName].position1[key].CostClose);
			}
			else
				Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose = (Display[g_AccountInfo.AccountName].position1[key].LastPrice - Display[g_AccountInfo.AccountName].position1[key].CostClose)*Display[g_AccountInfo.AccountName].position1[key].Multiplie*Display[g_AccountInfo.AccountName].position1[key].NumPosition;
			if (Display[g_AccountInfo.AccountName].position1[key].CostSettle < 1)
			{
				Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle = 0;
				nowtime = time(NULL); //获取日历时间
				localtime_s(&local, &nowtime);  //获取当前系统时间
				if (local.tm_hour <= 15 && local.tm_hour >= 9)
					bIsCostZero = true;
			}
			else
				Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle = (Display[g_AccountInfo.AccountName].position1[key].LastPrice - Display[g_AccountInfo.AccountName].position1[key].CostSettle)*Display[g_AccountInfo.AccountName].position1[key].Multiplie*Display[g_AccountInfo.AccountName].position1[key].NumPosition;

			Display[g_AccountInfo.AccountName].position1[key].Position = abs(Display[g_AccountInfo.AccountName].position1[key].NumPosition*Display[g_AccountInfo.AccountName].position1[key].T1CLose*Display[g_AccountInfo.AccountName].position1[key].Multiplie);
			Display[g_AccountInfo.AccountName].position1[key].NetPosition = Display[g_AccountInfo.AccountName].position1[key].NumPosition*Display[g_AccountInfo.AccountName].position1[key].T1CLose*Display[g_AccountInfo.AccountName].position1[key].Multiplie;

			Display[g_AccountInfo.AccountName].position1[key].PnlClose = Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose + Display[g_AccountInfo.AccountName].position1[key].TradingPnlClose;
			Display[g_AccountInfo.AccountName].position1[key].PnlSettle = Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle + Display[g_AccountInfo.AccountName].position1[key].TradingPnlSettle;

		}
		direction = "多";
		key.ID = code;
		key.Direction = direction;
		if (Display[g_AccountInfo.AccountName].position1.find(key) != Display[g_AccountInfo.AccountName].position1.end())
		{
			Display[g_AccountInfo.AccountName].position1[key].LastPrice = pMD->LastPrice;
			Display[g_AccountInfo.AccountName].position1[key].T1CLose = pMD->PreClosePrice;
			Display[g_AccountInfo.AccountName].position1[key].T1Settle = pMD->PreSettlementPrice;
			Display[g_AccountInfo.AccountName].position1[key].PctClose = Display[g_AccountInfo.AccountName].position1[key].LastPrice / Display[g_AccountInfo.AccountName].position1[key].T1CLose - 1;
			Display[g_AccountInfo.AccountName].position1[key].PctSettle = Display[g_AccountInfo.AccountName].position1[key].LastPrice / Display[g_AccountInfo.AccountName].position1[key].T1Settle - 1;

			if (Display[g_AccountInfo.AccountName].position1[key].CostClose < 1)
			{
				Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose = 0;
				nowtime = time(NULL); //获取日历时间
				localtime_s(&local, &nowtime);  //获取当前系统时间
				if (local.tm_hour <= 15 && local.tm_hour >= 9)
					bIsCostZero = true;

			}
			else
				Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose = (Display[g_AccountInfo.AccountName].position1[key].LastPrice - Display[g_AccountInfo.AccountName].position1[key].CostClose)*Display[g_AccountInfo.AccountName].position1[key].Multiplie*Display[g_AccountInfo.AccountName].position1[key].NumPosition;
			if (Display[g_AccountInfo.AccountName].position1[key].CostSettle < 1)
			{
				Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle = 0;
				nowtime = time(NULL); //获取日历时间
				localtime_s(&local, &nowtime);  //获取当前系统时间
				if (local.tm_hour <= 15 && local.tm_hour >= 9)
					bIsCostZero = true;
			}
			else
				Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle = (Display[g_AccountInfo.AccountName].position1[key].LastPrice - Display[g_AccountInfo.AccountName].position1[key].CostSettle)*Display[g_AccountInfo.AccountName].position1[key].Multiplie*Display[g_AccountInfo.AccountName].position1[key].NumPosition;

			Display[g_AccountInfo.AccountName].position1[key].Position = abs(Display[g_AccountInfo.AccountName].position1[key].NumPosition*Display[g_AccountInfo.AccountName].position1[key].T1CLose*Display[g_AccountInfo.AccountName].position1[key].Multiplie);
			Display[g_AccountInfo.AccountName].position1[key].NetPosition = Display[g_AccountInfo.AccountName].position1[key].NumPosition*Display[g_AccountInfo.AccountName].position1[key].T1CLose*Display[g_AccountInfo.AccountName].position1[key].Multiplie;

			Display[g_AccountInfo.AccountName].position1[key].PnlClose = Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose + Display[g_AccountInfo.AccountName].position1[key].TradingPnlClose;
			Display[g_AccountInfo.AccountName].position1[key].PnlSettle = Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle + Display[g_AccountInfo.AccountName].position1[key].TradingPnlSettle;
		}
	}
	delete p;
}




double CFTMD::GetLastPrice(string order)
{
	double price = 0.001;
	if (LastDepth.size() == 0)
	{
		ExitThread(0);
	}
	else
		price = LastDepth[order].LastPrice;
	//对于未定义到的合约，设定价格为0.001，小于目前所有合约的最小变动单位，报单不会有效。
	if (price > 10000 * 1000)
		price = 0.001;
	else if (price < -10000 * 1000)
		price = 0.001;
	return price;
}
int CFTMD::GetMultiple(string code)
{
	for (list<CThostFtdcInstrumentField>::iterator it = g_pTdHandler->Instruments.begin(); it != g_pTdHandler->Instruments.end(); it++)
	{
		if (strcmp(it->InstrumentID, code.c_str())==0)
		{
			int multi = it->VolumeMultiple;
			return multi;
		}
	}
	bIsMultiZero = true;

	nowtime = time(NULL); //获取日历时间
	localtime_s(&local, &nowtime);  //获取当前系统时间
	char filepre_tm[18];
	strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
	pLog->printLog("(%s) (%s)\n %s获取Multi失败，此时list<CThostFtdcInstrumentField>长度为%d \n", g_AccountInfo.AccountName.c_str(), filepre_tm, code.c_str(), g_pTdHandler->Instruments.size());
	return 0;
}
double CFTMD::GetTClose(string code)
{
	if (LastDepth.size() == 0)
	{
		ExitThread(0);
	}
	return LastDepth[code].PreClosePrice;
}
double CFTMD::GetTSettle(string code)
{
	if (LastDepth.size() == 0)
	{
		ExitThread(0);
	}
	return LastDepth[code].PreSettlementPrice;
}

double CFTMD::GetVolume(string code)
{
	if (LastDepth.size() == 0)
	{
		ExitThread(0);
	}
	return LastDepth[code].Volume;
}

void CFTMD::AccountLogout()
{
	UnsubscribeMD();
	CThostFtdcUserLogoutField UserLogoutField;
	memset(&UserLogoutField, 0, sizeof(UserLogoutField));
	CThostFtdcUserLogoutField * pUserLogoutField = &UserLogoutField;
	strcpy_s(pUserLogoutField->BrokerID, g_pTdHandler->g_AccountInfo.BrokerID.c_str());
	strcpy_s(pUserLogoutField->UserID, g_pTdHandler->g_AccountInfo.UserID.c_str());
	ResetEvent(hEvent);
	m_pMdApi->ReqUserLogout(pUserLogoutField, 1);
	if (WaitForSingleObject(hEvent, 10000) == WAIT_TIMEOUT)
	{

		printf("***%s*** 行情前端登出失败 该线程已退出\n", g_pTdHandler->g_AccountInfo.AccountName.c_str());
		ExitThread(0);
	}
	else
	{

		printf("***%s*** 行情前端登出\n", g_pTdHandler->g_AccountInfo.AccountName.c_str());
	}

}

void CFTMD::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo->ErrorID == 0)
		SetEvent(hEvent);
}

void CFTMD::UnsubscribeMD()
{
	//取消所订阅的行情列表
	int n_count = g_pTdHandler->Instruments.size();
	char ** codes = new char*[n_count];
	TThostFtdcInstrumentIDType* InstrumentIDs = new TThostFtdcInstrumentIDType[n_count];
	int i = 0;
	for (list<CThostFtdcInstrumentField>::iterator it = g_pTdHandler->Instruments.begin(); it != g_pTdHandler->Instruments.end(); it++)
	{
		strcpy_s(InstrumentIDs[i], it->InstrumentID);
		codes[i] = InstrumentIDs[i];
		i++;
	}
	ResetEvent(hEvent);
	m_pMdApi->UnSubscribeMarketData(codes, n_count);
	if (WaitForSingleObject(hEvent, 10000) == WAIT_TIMEOUT)
	{

		printf("***%s*** 退订行情失败 该线程已退出\n", g_pTdHandler->g_AccountInfo.AccountName.c_str());
		ExitThread(0);
	}
	else
	{

		printf("***%s*** 退订行情成功\n", g_pTdHandler->g_AccountInfo.AccountName.c_str());
	}
	delete[] codes;
	delete[] InstrumentIDs;
}

void CFTMD::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast)
		SetEvent(hEvent);
}