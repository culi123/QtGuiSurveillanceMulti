#include "stdafx.h"
using namespace std;
extern string GetExePath();
extern HANDLE hMutex;
extern map<string, Display_GUI>Display;
extern logInfo *pLog;
CFTTD::CFTTD()
{ 
}

CFTTD::~CFTTD()
{
}

void CFTTD::Init(AccountInfo account_info, void * pMdHandler)
{
    memset(qh_BrokerID, 0, sizeof(qh_BrokerID));
    memset(qh_TDAddress, 0, sizeof(qh_TDAddress));
    memset(qh_UserID, 0, sizeof(qh_UserID));
    memset(qh_Password, 0, sizeof(qh_Password));
	strcpy_s(qh_BrokerID, account_info.BrokerID.c_str());
    strcpy_s(qh_UserID, account_info.UserID.c_str());
    strcpy_s(qh_Password, account_info.Password.c_str());

    bIsgetInst = false;
	bWannaLogin = true;
	g_pMdHandler = pMdHandler;
	g_AccountInfo = account_info;
	// 产生一个CThostFtdcTraderApi实例
	m_pTdApi = CThostFtdcTraderApi::CreateFtdcTraderApi();

	// 注册一事件处理的实例
	m_pTdApi->RegisterSpi(this);

    // 订阅公共流
	//        TERT_RESTART:从本交易日开始重传
	//        TERT_RESUME:从上次收到的续传
	//        TERT_QUICK:只传送登录后公共流的内容
	m_pTdApi->SubscribePublicTopic(THOST_TERT_QUICK);

    // 订阅私有流
	//        TERT_RESTART:从本交易日开始重传
	//        TERT_RESUME:从上次收到的续传
	//        TERT_QUICK:只传送登录后私有流的内容
	m_pTdApi->SubscribePrivateTopic(THOST_TERT_RESTART);
	
	// 设置交易托管系统服务的地址，可以注册多个地址备用
	for (list<string>::iterator it = account_info.TdAddress.begin(); it != account_info.TdAddress.end(); it++)
	{
		strcpy_s(qh_TDAddress, (*it).c_str());
		m_pTdApi->RegisterFront(qh_TDAddress);
	}

	// 使客户端开始与后台服务建立连接
	hEvent = CreateEvent(NULL, false, false, NULL);
	hEventUi = CreateEvent(NULL, false, false, NULL);
	m_pTdApi->Init();
	if (WaitForSingleObject(hEvent, 30000) == WAIT_TIMEOUT)
	{
		cout << "***" + g_AccountInfo.AccountName + "***" << "交易前端登陆失败 该线程已退出" << endl;
		nowtime = time(NULL); //获取日历时间
		localtime_s(&local, &nowtime);  //获取当前系统时间
		char filepre_tm[18];
		strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
		Display[g_AccountInfo.AccountName].log.push_back(string(filepre_tm) + string(":Transaction front landed failed !!!"));
	}

	ConfirmSettleInfo();
	initkind();
}

void CFTTD::OnFrontConnected()
{
	nowtime = time(NULL); //获取日历时间
	localtime_s(&local, &nowtime);  //获取当前系统时间
	char filepre_tm[18];
	strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
	Display[g_AccountInfo.AccountName].log.push_back(string(filepre_tm) + string(":Transaction front connected successfully !!!"));
	if (bWannaLogin)
	{
		CThostFtdcReqUserLoginField reqUserLogin;
		memset(&reqUserLogin, 0, sizeof(reqUserLogin));
		strcpy_s(reqUserLogin.BrokerID, qh_BrokerID);//此处2011不要改
		strcpy_s(reqUserLogin.UserID, qh_UserID);//输入自己的帐号
		strcpy_s(reqUserLogin.Password, qh_Password);//输入密码
		int login = m_pTdApi->ReqUserLogin(&reqUserLogin, 1);//登录
	}
}
void CFTTD::OnFrontDisconnected (int nReason)
{
	nowtime = time(NULL); //获取日历时间
	localtime_s(&local, &nowtime);  //获取当前系统时间
	char filepre_tm[18];
	strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
	Display[g_AccountInfo.AccountName].log.push_back(string(filepre_tm) + string(":Transaction front connection is broken !!!"));
	WaitForSingleObject(hMutex, INFINITE);
	ReleaseMutex(hMutex);
}

void CFTTD::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo->ErrorID == 0)
	{
		nowtime = time(NULL); //获取日历时间
		localtime_s(&local, &nowtime);  //获取当前系统时间
		char filepre_tm[18];
		strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
		Display[g_AccountInfo.AccountName].log.push_back(string(filepre_tm) + string(":Transaction front landed successfully!!!"));
		GetInstruments();
	}
	else
	{
		
		nowtime = time(NULL); //获取日历时间
		localtime_s(&local, &nowtime);  //获取当前系统时间
		char filepre_tm[18];
		strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
		pLog->printLog("(%s) (%s)\n 交易前端登陆错误 ErrorID=%d ErrorMsg=%s 当前日期=%s \n", g_AccountInfo.AccountName.c_str(), filepre_tm, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pRspUserLogin->TradingDay);
		Display[g_AccountInfo.AccountName].log.push_back(string(filepre_tm) + string(":Transaction front landing error !!!"));
		printf("***%s*** 交易前端登陆错误 ErrorID=%d ErrorMsg=%s 当前日期=%s\n", g_AccountInfo.AccountName.c_str(), pRspInfo->ErrorID, pRspInfo->ErrorMsg, pRspUserLogin->TradingDay);
	}
}
void CFTTD::GetInstruments()
{
    //获得合约列表
	Instruments.clear();
	CThostFtdcQryInstrumentField qryField;
    memset(&qryField, 0, sizeof(qryField));
	Sleep(1000);
    int resCode = m_pTdApi-> ReqQryInstrument(&qryField, 0);
}

void CFTTD::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if ((pInstrument != NULL)
		&& ((strcmp(pInstrument->ExchangeID, "CFFEX") == 0)
		|| (strcmp(pInstrument->ExchangeID, "CZCE") == 0)
		|| (strcmp(pInstrument->ExchangeID, "SHFE") == 0)
		|| (strcmp(pInstrument->ExchangeID, "DCE") == 0))
		&& ((pInstrument->ProductClass == '1')))
	{
		CThostFtdcInstrumentField InstFld;
		memset(&InstFld, 0, sizeof(InstFld));
		memcpy(&InstFld, pInstrument, sizeof(InstFld));
		CFTTD::Instruments.push_back(InstFld);
		//g_pLog->printLog("名称：%s,交易所代码：%s \n", InstFld.InstrumentID, InstFld.ExchangeID);
	}
	if (bIsLast)
	{
		bIsgetInst = true;
		cout << "***" + g_AccountInfo.AccountName + "***" << "交易前端登陆成功" << endl;
		SetEvent(hEvent);
		
	}
    return;
}
void CFTTD::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pInvestorPosition != NULL)
	{
		//忽略组合合约
		string code = pInvestorPosition->InstrumentID;
		if ((code.find("SPC") == code.npos)&& (code.find("SP") == code.npos))
		{
			CThostFtdcInvestorPositionField position_temp;
			position_temp = *pInvestorPosition;
			CurrentPosition.push_back(position_temp);
		}
	}
	if (bIsLast)
		SetEvent(hEvent);
	return;
}
void CFTTD::QueryAcct()
{
   CThostFtdcQryTradingAccountField qryTradingAccount;
   memset(&qryTradingAccount,0,sizeof(qryTradingAccount));
   strcpy_s(qryTradingAccount.BrokerID, qh_BrokerID);
   strcpy_s(qryTradingAccount.InvestorID, qh_UserID);
   ResetEvent(hEvent);
   Sleep(1000);
   int resCode = m_pTdApi->ReqQryTradingAccount(&qryTradingAccount, 0);
   WaitForSingleObject(hEvent,1000);
   return;
}
void CFTTD::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

	if (pTradingAccount != NULL)
	{
		///每次查询后固定项
		//静态权益，为昨日结算后权益，不会随今日交易或者行情变动而变动
		//静态权益 = 上日结算 - 出金金额 + 入金金额
		Display[g_AccountInfo.AccountName].account.StaticEquity = pTradingAccount->PreBalance- pTradingAccount->Withdraw+ pTradingAccount->Deposit;//静态权益
		//当前占用保证金，仅仅与当前持仓有关，若无交易，则占用保证金不会变动。
		Display[g_AccountInfo.AccountName].account.CurrMargin = pTradingAccount->CurrMargin;  //当前占用保证金

		///动态项，可计算项
		//持仓盈亏
		Display[g_AccountInfo.AccountName].account.CloseProfit = pTradingAccount->CloseProfit;
		Display[g_AccountInfo.AccountName].account.PositionProfit = pTradingAccount->PositionProfit;
		//平仓盈亏
		//动态权益，动态权益=静态权益+持仓盈亏+平仓盈亏-手续费
		Display[g_AccountInfo.AccountName].account.DynamicEquity = pTradingAccount->PreBalance + pTradingAccount->CloseProfit + pTradingAccount->PositionProfit- pTradingAccount->Commission;
		//可用资金，可用资金=动态权益-当前占用保证金
		Display[g_AccountInfo.AccountName].account.AvailableAmount = Display[g_AccountInfo.AccountName].account.DynamicEquity - Display[g_AccountInfo.AccountName].account.CurrMargin;
		//风险度，风险度=占用保证金/动态权益
		Display[g_AccountInfo.AccountName].account.RiskDegree = Display[g_AccountInfo.AccountName].account.CurrMargin / Display[g_AccountInfo.AccountName].account.DynamicEquity;
		
	}
	SetEvent(hEvent);
	return;
}
void CFTTD::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	int index = pInputOrder->RequestID;
	if (find(RequestIdDealed.begin(), RequestIdDealed.end(), index)==RequestIdDealed.end() && bWannaDealMsg)
	{
		int order_sn = int((int(pInputOrder->RequestID / 10)) / 1000);
		OrderStruct order;
		memset(&order, 0, sizeof(order));
		strcpy_s(order.code, pInputOrder->InstrumentID);
		order.volume = pInputOrder->VolumeTotalOriginal;
		order.direction = pInputOrder->Direction;
		strcpy_s(order.kp, pInputOrder->CombOffsetFlag);
		TradeResult.insert({ index,order });
		RequestIdDealed.push_back(index);
		return;
	}
}


//ErrRSP&Rtn/////////////////////////////////////////////////////////////////////
void CFTTD::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	return;
}
void CFTTD::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	return;
}
void CFTTD::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	return;
}
//Rtn/////////////////////////////////////////////////////////////////////

void CFTTD::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	CurrentTrade.clear();
	CThostFtdcTradeField trade_tmpe;
	trade_tmpe = *pTrade;
	CurrentTrade.push_back(trade_tmpe);
	GetTradeSummary();
	//判断是否需要有成交，如果有则更新持仓。
	bIsTrade = true;
	
}
void CFTTD::GetTradeSummary()
{
	//list<tradesummary2> summary_trade;

	for (list<CThostFtdcTradeField>::iterator it = CurrentTrade.begin(); it != CurrentTrade.end(); it++)
	{
	
		//循环交易明细，对每一条交易进行判断
		string Code = string((*it).InstrumentID);
		string  OffsetFlag;
		if ((*it).OffsetFlag == THOST_FTDC_OF_Open)
			OffsetFlag = "Open";
		else if ((*it).OffsetFlag == THOST_FTDC_OF_Close)
			OffsetFlag = "Close";
		else if ((*it).OffsetFlag == THOST_FTDC_OF_ForceClose)
			OffsetFlag = "ForceClose";
		else if ((*it).OffsetFlag == THOST_FTDC_OF_CloseToday)
			OffsetFlag = "CloseToday";
		else if ((*it).OffsetFlag == THOST_FTDC_OF_CloseYesterday)
			OffsetFlag = "CloseYesterday";
		string Direction;
		if ((*it).Direction == THOST_FTDC_D_Buy)
			Direction = "Buy";
		else if ((*it).Direction == THOST_FTDC_D_Sell)
			Direction = "Sell";
		int num = (*it).Volume;
		double price = (*it).Price;
		bool isin = false;
		for (list<tradesummary2>::iterator its = Display[g_AccountInfo.AccountName].transaction.begin(); its != Display[g_AccountInfo.AccountName].transaction.end(); its++)
		{
			//循环交易汇总
			if (
				(its->code == Code)
				&& (its->Direction == Direction)
				&& (its->OffsetFlag == OffsetFlag)
				)
			{
				//如果当前交易明细在交易汇总中出现，则按照加权平均的方式修改该合约交易的成交汇总
				its->price = (double(its->num)*its->price + double(num)*price) / (double(num + its->num));
				its->num = its->num + num;//数量相加
				isin = true;
			}

		}
		if (!isin)
		{
			//当前交易明细不在任何一条交易汇总中出现，则插入该条交易明细，作为新的一条交易汇总。
			tradesummary2 ss;
			ss.code = Code;
			ss.Direction = Direction;
			ss.num = num;
			ss.OffsetFlag = OffsetFlag;
			ss.price = price;
			
			Display[g_AccountInfo.AccountName].transaction.push_back(ss);
		}


	}

	

}


void CFTTD::GetCurrentPosition1()
{
	CThostFtdcQryInvestorPositionField QryPosFd;
	memset(&QryPosFd, 0, sizeof(QryPosFd));
	CThostFtdcQryInvestorPositionField * pQryPos = &QryPosFd;
	strcpy_s(pQryPos->BrokerID, qh_BrokerID);
	strcpy_s(pQryPos->InvestorID, qh_UserID);
	strcpy_s(pQryPos->InstrumentID, "");
	CurrentPosition.clear();
	ResetEvent(hEvent);
	ResetEvent(hEventUi);
	Display[g_AccountInfo.AccountName].hEventUi = false;
	Sleep(1000);
	m_pTdApi->ReqQryInvestorPosition(pQryPos, 0);
	bIsTrade = false;
	if (WaitForSingleObject(hEvent, 10000) == WAIT_OBJECT_0)
	{
		map<string, int> current_position;
		Display[g_AccountInfo.AccountName].position1.erase(Display[g_AccountInfo.AccountName].position1.begin(), Display[g_AccountInfo.AccountName].position1.end());
		for (list<CThostFtdcInvestorPositionField>::iterator it = CurrentPosition.begin(); it != CurrentPosition.end(); it++)
		{
			string code = (*it).InstrumentID;
			string direction;
			int num_direction;
			if (it->PosiDirection == THOST_FTDC_PD_Long)
			{
				direction = "多";
				num_direction = 1;
			}
			else
			{
				direction = "空";
				num_direction = -1;
			}
			Display_Position_Key key;
			key.ID = code;
			key.Direction = direction;
			if (Display[g_AccountInfo.AccountName].position1.find(key) != Display[g_AccountInfo.AccountName].position1.end())
			{
				//汇总的持仓信息中已存在该合约的信息，则合并计算
				int Multiple = ((CFTMD*)g_pMdHandler)->GetMultiple(key.ID);
				double LastPrice = ((CFTMD*)g_pMdHandler)->GetLastPrice(key.ID);
				double T1Close = ((CFTMD*)g_pMdHandler)->GetTClose(key.ID);
				double T1Settle = ((CFTMD*)g_pMdHandler)->GetTSettle(key.ID);
				string kind = GetKind(key.ID);
				if (it->TodayPosition != 0)
				{
					//该合约有今仓的情况下，
					double costprice = (it->PositionCost - (it->Position - it->TodayPosition)*it->PreSettlementPrice*Multiple)/ Multiple / (it->TodayPosition);
					Display[g_AccountInfo.AccountName].position1[key].CostClose = (Display[g_AccountInfo.AccountName].position1[key].CostClose*Display[g_AccountInfo.AccountName].position1[key].NumPosition + costprice*(it->TodayPosition)) / (Display[g_AccountInfo.AccountName].position1[key].NumPosition + it->TodayPosition);
					Display[g_AccountInfo.AccountName].position1[key].CostSettle = (Display[g_AccountInfo.AccountName].position1[key].CostSettle*Display[g_AccountInfo.AccountName].position1[key].NumPosition + costprice*(it->TodayPosition)) / (Display[g_AccountInfo.AccountName].position1[key].NumPosition + it->TodayPosition);
				}
				else
				{
					if (abs(Display[g_AccountInfo.AccountName].position1[key].NumPosition) + it->Position == 0)
					{
						Display[g_AccountInfo.AccountName].position1[key].CostClose = 0;
						Display[g_AccountInfo.AccountName].position1[key].CostSettle = 0;
						//输出Debug日志
						nowtime = time(NULL); //获取日历时间
						localtime_s(&local, &nowtime);  //获取当前系统时间
						char filepre_tm[18];
						strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
						pLog->printLog("(%s) (%s)\n %s 持仓成本为0，;\n", g_AccountInfo.AccountName.c_str(), filepre_tm, key.ID.c_str());

					}
					else
					{
						Display[g_AccountInfo.AccountName].position1[key].CostClose = (Display[g_AccountInfo.AccountName].position1[key].CostClose*abs(Display[g_AccountInfo.AccountName].position1[key].NumPosition) + T1Close*(it->Position )) / (abs(Display[g_AccountInfo.AccountName].position1[key].NumPosition) + it->Position );
						Display[g_AccountInfo.AccountName].position1[key].CostSettle = (Display[g_AccountInfo.AccountName].position1[key].CostSettle*abs(Display[g_AccountInfo.AccountName].position1[key].NumPosition) + T1Settle*(it->Position )) / (abs(Display[g_AccountInfo.AccountName].position1[key].NumPosition) + it->Position );
					}
				}
				
				Display[g_AccountInfo.AccountName].position1[key].NumPosition += it->Position*num_direction;
				Display[g_AccountInfo.AccountName].position1[key].YesNumPosition += (it->Position- it->TodayPosition)*num_direction;
				Display[g_AccountInfo.AccountName].position1[key].TodayNumPosition += (it->TodayPosition)*num_direction;
				
				Display[g_AccountInfo.AccountName].position1[key].KIND = kind;
				Display[g_AccountInfo.AccountName].position1[key].LastPrice = LastPrice;
				Display[g_AccountInfo.AccountName].position1[key].T1CLose = T1Close;
				Display[g_AccountInfo.AccountName].position1[key].T1Settle = T1Settle;
				Display[g_AccountInfo.AccountName].position1[key].Multiplie = Multiple;

				Display[g_AccountInfo.AccountName].position1[key].Position = abs(Display[g_AccountInfo.AccountName].position1[key].NumPosition*Display[g_AccountInfo.AccountName].position1[key].T1CLose*Display[g_AccountInfo.AccountName].position1[key].Multiplie);
				Display[g_AccountInfo.AccountName].position1[key].NetPosition = Display[g_AccountInfo.AccountName].position1[key].NumPosition*Display[g_AccountInfo.AccountName].position1[key].T1CLose*Display[g_AccountInfo.AccountName].position1[key].Multiplie;

				Display[g_AccountInfo.AccountName].position1[key].PctClose = Display[g_AccountInfo.AccountName].position1[key].LastPrice / Display[g_AccountInfo.AccountName].position1[key].T1CLose - 1;
				Display[g_AccountInfo.AccountName].position1[key].PctSettle = Display[g_AccountInfo.AccountName].position1[key].LastPrice / Display[g_AccountInfo.AccountName].position1[key].T1Settle - 1;
				if (Display[g_AccountInfo.AccountName].position1[key].CostClose < 1)
					Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose = 0;
				else
					Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose = (Display[g_AccountInfo.AccountName].position1[key].LastPrice - Display[g_AccountInfo.AccountName].position1[key].CostClose)*Display[g_AccountInfo.AccountName].position1[key].Multiplie*Display[g_AccountInfo.AccountName].position1[key].NumPosition;

				if (Display[g_AccountInfo.AccountName].position1[key].CostSettle < 1)
					Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle = 0;
				else
					Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle = (Display[g_AccountInfo.AccountName].position1[key].LastPrice - Display[g_AccountInfo.AccountName].position1[key].CostSettle)*Display[g_AccountInfo.AccountName].position1[key].Multiplie*Display[g_AccountInfo.AccountName].position1[key].NumPosition;
				
				Display[g_AccountInfo.AccountName].position1[key].TradingPnlClose += it->CloseProfit;
				Display[g_AccountInfo.AccountName].position1[key].TradingPnlSettle += it->CloseProfit;

				Display[g_AccountInfo.AccountName].position1[key].PnlClose = Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose + Display[g_AccountInfo.AccountName].position1[key].TradingPnlClose;
				Display[g_AccountInfo.AccountName].position1[key].PnlSettle = Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle + Display[g_AccountInfo.AccountName].position1[key].TradingPnlSettle;



			}
			else
			{
				//汇总的持仓信息中尚未存在该合约的信息，则插入
				int Multiple = ((CFTMD*)g_pMdHandler)->GetMultiple(key.ID);
				double LastPrice = ((CFTMD*)g_pMdHandler)->GetLastPrice(key.ID);
				double T1Close = ((CFTMD*)g_pMdHandler)->GetTClose(key.ID);
				double T1Settle = ((CFTMD*)g_pMdHandler)->GetTSettle(key.ID);
				string kind = GetKind(key.ID);
				if (it->TodayPosition != 0)
				{
					//该合约有今仓的情况下，
					double costprice = (it->PositionCost - (it->Position - it->TodayPosition)*it->PreSettlementPrice*Multiple) / Multiple / (it->TodayPosition);
					Display[g_AccountInfo.AccountName].position1[key].CostClose = (it->TodayPosition*costprice+(it->Position - it->TodayPosition)*T1Close)/it->Position;
					Display[g_AccountInfo.AccountName].position1[key].CostSettle = (it->TodayPosition*costprice + (it->Position - it->TodayPosition)*T1Settle) / it->Position;
				}
				else
				{
					Display[g_AccountInfo.AccountName].position1[key].CostClose = T1Close;
					Display[g_AccountInfo.AccountName].position1[key].CostSettle = T1Settle;
				
				}
				Display[g_AccountInfo.AccountName].position1[key].NumPosition = it->Position*num_direction;
				Display[g_AccountInfo.AccountName].position1[key].YesNumPosition = (it->Position - it->TodayPosition)*num_direction;
				Display[g_AccountInfo.AccountName].position1[key].TodayNumPosition = (it->TodayPosition)*num_direction;
				
				Display[g_AccountInfo.AccountName].position1[key].KIND = kind;
				Display[g_AccountInfo.AccountName].position1[key].LastPrice = LastPrice;
				Display[g_AccountInfo.AccountName].position1[key].T1CLose = T1Close;
				Display[g_AccountInfo.AccountName].position1[key].T1Settle = T1Settle;
				Display[g_AccountInfo.AccountName].position1[key].Multiplie = Multiple;
				Display[g_AccountInfo.AccountName].position1[key].Position = abs(Display[g_AccountInfo.AccountName].position1[key].NumPosition*Display[g_AccountInfo.AccountName].position1[key].T1CLose*Display[g_AccountInfo.AccountName].position1[key].Multiplie);

				Display[g_AccountInfo.AccountName].position1[key].NetPosition = Display[g_AccountInfo.AccountName].position1[key].NumPosition*Display[g_AccountInfo.AccountName].position1[key].T1CLose*Display[g_AccountInfo.AccountName].position1[key].Multiplie;
				Display[g_AccountInfo.AccountName].position1[key].PctClose = Display[g_AccountInfo.AccountName].position1[key].LastPrice / Display[g_AccountInfo.AccountName].position1[key].T1CLose - 1;
				Display[g_AccountInfo.AccountName].position1[key].PctSettle = Display[g_AccountInfo.AccountName].position1[key].LastPrice / Display[g_AccountInfo.AccountName].position1[key].T1Settle - 1;
				
				if (Display[g_AccountInfo.AccountName].position1[key].CostClose < 1)
				{
					Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose = 0;
					//输出Debug日志
					nowtime = time(NULL); //获取日历时间
					localtime_s(&local, &nowtime);  //获取当前系统时间
					char filepre_tm[18];
					strftime(filepre_tm, 18, "%Y%m%d_%H%M%S", &local);
					pLog->printLog("(%s) (%s)\n %s的持仓成本为T-1Close，该值为%f，;\n", g_AccountInfo.AccountName.c_str(), filepre_tm, key.ID.c_str(), T1Close);
				}
				else
					Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose = (Display[g_AccountInfo.AccountName].position1[key].LastPrice - Display[g_AccountInfo.AccountName].position1[key].CostClose)*Display[g_AccountInfo.AccountName].position1[key].Multiplie*Display[g_AccountInfo.AccountName].position1[key].NumPosition;
				
				if (Display[g_AccountInfo.AccountName].position1[key].CostSettle <1)
					Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle = 0;
				else
					Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle = (Display[g_AccountInfo.AccountName].position1[key].LastPrice - Display[g_AccountInfo.AccountName].position1[key].CostSettle)*Display[g_AccountInfo.AccountName].position1[key].Multiplie*Display[g_AccountInfo.AccountName].position1[key].NumPosition;
				
				Display[g_AccountInfo.AccountName].position1[key].TradingPnlClose = it->CloseProfit;
				Display[g_AccountInfo.AccountName].position1[key].TradingPnlSettle = it->CloseProfit;

				Display[g_AccountInfo.AccountName].position1[key].PnlClose = Display[g_AccountInfo.AccountName].position1[key].HoldingPnlClose + Display[g_AccountInfo.AccountName].position1[key].TradingPnlClose;
				Display[g_AccountInfo.AccountName].position1[key].PnlSettle = Display[g_AccountInfo.AccountName].position1[key].HoldingPnlSettle + Display[g_AccountInfo.AccountName].position1[key].TradingPnlSettle;
			}	
		}
	}

	SetEvent(hEventUi);
	Display[g_AccountInfo.AccountName].hEventUi = true;
	//刷新持仓信息之后这些变量重置为false
	bIsTrade = false;
	bIsCostZero = false;
	((CFTMD*)g_pMdHandler)->bIsCostZero = false;
	((CFTMD*)g_pMdHandler)->bIsCostZero = false;
}

void CFTTD::Run()
{

	//当出现交易时，用于刷新持仓信息和账户资金信息
	while (1)
	{
		Sleep(1000);
		nowtime = time(NULL); //获取日历时间
		localtime_s(&local, &nowtime);  //获取当前系统时间
		char filepre_tm[18];
		strftime(filepre_tm, 18, "%H:%M:%S", &local);

		bIsNon = Display[g_AccountInfo.AccountName].bIsEmpty;
		bIsCostZero = ((CFTMD*)g_pMdHandler)->bIsCostZero;
		bIsMultiZero = ((CFTMD*)g_pMdHandler)->bIsMultiZero;
		if (bIsTrade || bIsNon ||bIsCostZero || bIsMultiZero || (strcmp(filepre_tm, "09:29:00") == 0))
		{
			pLog->printLog("(%s) (%s)\n 程序触发获取持仓，bIsTrade=%d;bIsNon=%d;bIsCostZero=%d;bIsMultiZero=%d;\n", g_AccountInfo.AccountName.c_str(), filepre_tm, bIsTrade, bIsNon, bIsCostZero, bIsMultiZero);
			//判断是否需要有成交，如果有则更新持仓。
			//断线重连后，Display结构体中会出现持仓信息map变为空的情况，如果为空则重新查询持仓信息。
			//白天开盘以后，仍有金融期货成本价为零的情况，则重新查询持仓信息。
			//每天九点半刷新是为了金融期货在夜盘无法刷新的情况，
			//九点半刷新会出现Multi为0 的情况，此时需要重新刷新
			char *p =  new char[g_AccountInfo.AccountName.length() + 1];
			strcpy(p, g_AccountInfo.AccountName.c_str());
			GetCurrentPosition1();
			QueryAcct();
			

			delete p;
		}
			
	}
}
string CFTTD::GetKind(string code_raw)
{
	std::transform(code_raw.begin(), code_raw.end(), code_raw.begin(), ::toupper);
	size_t index = code_raw.find_last_not_of("0123456789");
	string head_u = code_raw.substr(0, index + 1);
	string head_l = head_u;
	std::transform(head_l.begin(), head_l.end(), head_l.begin(), ::tolower);
	return Kind[head_l];

}
string CFTTD::GetTradeCode(string code_raw)
{
	std::transform(code_raw.begin(), code_raw.end(),code_raw.begin(), ::toupper);
	size_t index = code_raw.find_last_not_of("0123456789");
	string head_u = code_raw.substr(0, index + 1);
	string head_l = head_u;
	std::transform(head_l.begin(), head_l.end(), head_l.begin(), ::tolower);
	string tail_4;
	if ((code_raw.substr(index + 1)).length() == 4)
		tail_4 = code_raw.substr(index + 1);
	else
		tail_4 = '1' + code_raw.substr(index + 1);
	string tail_3 = tail_4.substr(1);

	list<string> InstrumentCodes;
	for (list<CThostFtdcInstrumentField>::iterator it = Instruments.begin(); it != Instruments.end(); it++)
	{
		string code = it->InstrumentID;
		InstrumentCodes.push_back(code);
	}
	if (std::find(InstrumentCodes.begin(), InstrumentCodes.end(), head_u + tail_3) != InstrumentCodes.end())
		return head_u + tail_3;
	else if (std::find(InstrumentCodes.begin(), InstrumentCodes.end(), head_u + tail_4) != InstrumentCodes.end())
		return head_u + tail_4;
	else if (std::find(InstrumentCodes.begin(), InstrumentCodes.end(), head_l + tail_3) != InstrumentCodes.end())
		return head_l + tail_3;
	else if (std::find(InstrumentCodes.begin(), InstrumentCodes.end(), head_l + tail_4) != InstrumentCodes.end())
		return head_l + tail_4;
	else
	{
		printf("****%s*** CFTTD::GetTradeCode 找不到合约%s 该线程已退出\n",g_AccountInfo.AccountName.c_str(),code_raw.c_str());
		ExitThread(0);
	}
}

void CFTTD::initkind()
{	
	Kind["a"] = "Agriculture";
	Kind["cf"] = "Agriculture";
	Kind["y"] = "Agriculture";
	Kind["oi"] = "Agriculture";
	Kind["m"] = "Agriculture";
	Kind["rm"] = "Agriculture";
	Kind["p"] = "Agriculture";
	Kind["c"] = "Agriculture";
	Kind["sr"] = "Agriculture";
	Kind["cs"] = "Agriculture";
	Kind["jd"] = "Agriculture";
	Kind["rb"] = "Black";
	Kind["hc"] = "Black";
	Kind["i"] = "Black";
	Kind["j"] = "Black";
	Kind["jm"] = "Black";
	Kind["fg"] = "Energy";
	Kind["ma"] = "Energy";
	Kind["ta"] = "Energy";
	Kind["v"] = "Energy";
	Kind["pp"] = "Energy";
	Kind["ru"] = "Energy";
	Kind["l"] = "Energy";
	Kind["bu"] = "Energy";
	Kind["zc"] = "Energy";
	Kind["ic"] = "Financial";
	Kind["ih"] = "Financial";
	Kind["if"] = "Financial";
	Kind["t"] = "Financial";
	Kind["tf"] = "Financial";
	Kind["pb"] = "BaseMetal";
	Kind["al"] = "BaseMetal";
	Kind["ni"] = "BaseMetal";
	Kind["zn"] = "BaseMetal";
	Kind["sn"] = "BaseMetal";
	Kind["cu"] = "BaseMetal";
	Kind["au"] = "PreciousMetal";
	Kind["ag"] = "PreciousMetal";
}
void CFTTD::ConfirmSettleInfo()
{
	CThostFtdcSettlementInfoConfirmField SettleInfoConfirm;
	memset(&SettleInfoConfirm, 0, sizeof(SettleInfoConfirm));
	CThostFtdcSettlementInfoConfirmField * pSettleInfoConfirm = &SettleInfoConfirm;
	strcpy_s(pSettleInfoConfirm->BrokerID, qh_BrokerID);
	strcpy_s(pSettleInfoConfirm->InvestorID, qh_UserID);
	time_t now;
	time(&now);
	struct tm now_tm;
	struct tm *today = &now_tm;
	localtime_s(today,&now);
	strftime(SettleInfoConfirm.ConfirmDate, 9, "%Y%m%d", today);
	strftime(SettleInfoConfirm.ConfirmTime, 9, "%H%M%S", today);
	ResetEvent(hEvent);
	m_pTdApi->ReqSettlementInfoConfirm(pSettleInfoConfirm, 1);
	if (WaitForSingleObject(hEvent, 10000) == WAIT_OBJECT_0)
	{
		printf("***%s*** 确认结算结果完毕\n",g_AccountInfo.AccountName.c_str());
	}
	else
	{
		printf("***%s*** 确认结算结果失败 该线程已退出\n",g_AccountInfo.AccountName.c_str());
		ExitThread(0);
	}
}

void CFTTD::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	SetEvent(hEvent);
}
void CFTTD::AccountLogout()
{
	CThostFtdcUserLogoutField UserLogoutField;
	memset(&UserLogoutField, 0, sizeof(UserLogoutField));
	CThostFtdcUserLogoutField * pUserLogoutField = &UserLogoutField;
	strcpy_s(pUserLogoutField->BrokerID, g_AccountInfo.BrokerID.c_str());
	strcpy_s(pUserLogoutField->UserID, g_AccountInfo.UserID.c_str());
	ResetEvent(hEvent);
	bWannaLogin = false;
	m_pTdApi->ReqUserLogout(pUserLogoutField, 1);
	if (WaitForSingleObject(hEvent, 10000) == WAIT_TIMEOUT)
	{
		
		printf("***%s*** 交易前端登出失败 该线程已退出\n", g_AccountInfo.AccountName.c_str());
		ExitThread(0);
	}
	else
	{

		printf("***%s*** 交易前端登出\n", g_AccountInfo.AccountName.c_str());
	}
}

void CFTTD::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo->ErrorID == 0)
		SetEvent(hEvent);
}