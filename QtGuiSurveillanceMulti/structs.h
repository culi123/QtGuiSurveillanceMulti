#pragma once
#include "stdafx.h"
struct OrderStruct
{
	TThostFtdcInstrumentIDType code;
	TThostFtdcVolumeType volume;
	TThostFtdcDirectionType direction;
	TThostFtdcCombOffsetFlagType kp;
};
struct AccountInfo
{
	std::list<std::string>  TdAddress;
	std::list<std::string>  MdAddress;
	std::string  UserID;
	std::string  Password;
	std::string  BrokerID;
	std::string  PositionFileHead;
	std::string  PositionFileTail;
	std::string  AccountName;
};
struct  Display_Position_Key
{
	std::string ID;
	std::string Direction;
	//由于需要作为map的键，所以需要重载operator 函数
	friend bool operator < (const struct Display_Position_Key &k1, const struct Display_Position_Key &k2) 
	{
		if (k1.ID < k2.ID)        //类型按升序排序
			return true;
		else if (k1.ID == k2.ID)  //如果类型相同，按比例尺升序排序
			return k1.Direction < k2.Direction;
       return false;
	};

};
struct Display_Position
{
	std::string KIND;
	int NumPosition;
	int TodayNumPosition;
	int YesNumPosition;
	double LastPrice;
	double T1CLose;
	double T1Settle;
	int Multiplie;
	int Position;
	double NetPosition;
	double CostClose;
	double CostSettle;
	double PnlSettle;
	double PnlClose;
	double HoldingPnlClose;
	double HoldingPnlSettle;
	double TradingPnlClose;
	double TradingPnlSettle;
	double PctSettle;
	double PctClose;
};
struct Display_Summary
{

	double Position;
	double NetPosition;
	double PnlSettle;
	double PnlClose;
	double HoldingPnlClose;
	double HoldingPnlSettle;
	double TradingPnlClose;
	double TradingPnlSettle;
	double ReturnOnPosition;

};
struct tradesummary2
{
	std::string code;
	std::string OffsetFlag;
	std::string Direction;
	double price;
	int num;

};
struct Display_Account
{
	double StaticEquity;
	double DynamicEquity;
	double CloseProfit;
	double PositionProfit;
	double CurrMargin;
	double AvailableAmount;
	double RiskDegree;

};
struct Display_Monitor
{

	double Position;
	double NetPosition;
	double CommondityPosition;
	double FinancialPosition;
	double PnlSettle;
	double PnlClose;
	double ReturnOnPosition;
	double PnlFinancial;
	double PnlAgriculture;
	double PnlBlack;
	double PnlEnergy;
	double PnlBaseMetal;
	double PnlPreciousMetal;
	std::map<QTime, double>plotdata;
};
struct Display_GUI
{
	std::map<Display_Position_Key, Display_Position>position1;	//持仓信息    <<品种ID,品种多空>，持仓数据>
	std::map<std::string, Display_Position>position;			//持仓信息    <品种ID，持仓数据>
	std::map<std::string, Display_Summary>summary;				//汇总统计信息  <大类ID,大类汇总数据>
	std::list<tradesummary2>transaction;						//交易信息
	Display_Account account;
	std::list<std::string> log;									//CTP运行日志		
	std::map<QTime, double>plotdata;
	bool hEventUi=false;	
	bool bIsEmpty=false;												//判断汇总页是不是全是0
};