#pragma once
#include "stdafx.h"
class CFTTD : public CThostFtdcTraderSpi  
{
public:
	std::map<int,OrderStruct> orders;
	std::map<int,OrderStruct> TradeResult;
	std::map<std::string, std::string> Kind;
	CFTTD();
	virtual ~CFTTD();

	bool bWannaLogin;
	bool bIsgetInst;
	bool bWannaDealMsg = false;
	bool bIsTrade = false;
	bool bIsNon = false;
	bool bIsCostZero;
	bool bIsMultiZero;
	HANDLE hEvent,hEventUi;
	std::list<CThostFtdcInstrumentField> Instruments;
	std::list<CThostFtdcInvestorPositionField> CurrentPosition;
	///用于当前账号的成交查询
	std::list<CThostFtdcTradeField> CurrentTrade;
	std::list<int> RequestIdDealed;
    TThostFtdcBrokerIDType qh_BrokerID;
	TThostFtdcAddressType qh_TDAddress;
	TThostFtdcUserIDType qh_UserID;
    TThostFtdcPasswordType qh_Password;
	
	AccountInfo g_AccountInfo;

	time_t nowtime;
	struct tm local;

	void initkind();
	void GetCurrentPosition1();
    void Init(AccountInfo account_info,void * pMdHandler);
    void OnFrontConnected();
	void OnFrontDisconnected(int nReason);
	void GetInstruments();
    void QueryAcct();
	void Run();
	///查询成交汇总
	void GetTradeSummary();
	std::string GetKind(std::string);
	std::string GetTradeCode(std::string code_raw);

	void ConfirmSettleInfo();
	void AccountLogout();

	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


	///请求查询投资者持仓响应
	void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询资金账户响应
	void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///错误应答
	void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///报单录入错误回报
	void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);

	///报单操作错误回报
	void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);

	///成交通知
	void OnRtnTrade(CThostFtdcTradeField *pTrade);

	//账号登出通知
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

private:
	CThostFtdcTraderApi* m_pTdApi;
	void * g_pMdHandler;
};
