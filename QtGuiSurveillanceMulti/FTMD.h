#pragma once
#include "stdafx.h"
class CFTMD : public CThostFtdcMdSpi  
{
public:
	CFTMD();
	virtual ~CFTMD();
	typedef std::map<std::string, CThostFtdcDepthMarketDataField> TYP_QUTOE;
	TYP_QUTOE LastDepth;

	TThostFtdcBrokerIDType qh_BrokerID;
	TThostFtdcAddressType qh_MDAddress;
	TThostFtdcUserIDType qh_UserID;
    TThostFtdcPasswordType qh_Password;


	AccountInfo g_AccountInfo;
	HANDLE hEvent;
	CThostFtdcMdApi *m_pMdApi;

	time_t nowtime;
	struct tm local;
	bool bIsCostZero=false;
	bool bIsMultiZero = false;

    void Init(AccountInfo account_info, CFTTD* pTdHandler);
	void initSubMD();
	
	double GetLastPrice(std::string code);
	double GetVolume(std::string code);
	double GetTClose(std::string code);
	double GetTSettle(std::string code);
	int GetMultiple(std::string code);
	void UnsubscribeMD();
	void AccountLogout();



	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	void OnFrontConnected();
	///登录请求响应
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	void OnFrontDisconnected(int nReason);
	///深度行情通知
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//退订行情通知
	void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//登出通知
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
private:
    CFTTD *g_pTdHandler;
};