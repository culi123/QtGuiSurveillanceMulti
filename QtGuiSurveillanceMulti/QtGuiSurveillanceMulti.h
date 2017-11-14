#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QListView>
#include <QtWidgets/QTreeWidget>
#include "qcustomplot.h"
#include "ui_QtGuiSurveillanceMulti.h"
#include "stdafx.h"
using namespace std;
class QtGuiSurveillanceMulti : public QMainWindow
{
	Q_OBJECT

public:
	QtGuiSurveillanceMulti(QWidget *parent = Q_NULLPTR);
	~QtGuiSurveillanceMulti();

	
	//***********************************************共有变量声明********************************************///
	QTabWidget *TabWidget;
	QGridLayout *AccountGridLayout, **P_AccountGridLayout, *MonitorPageGridLayout;
	QWidget *AccountWidget, **P_AccountWidget,*MonitorPageWidget;
	QTableView *AccountCapitalTable, **P_AccountCapitalTable, *AccountSummaryTable,**P_AccountSummaryTable,*MonitorPageStopTable, *MonitorPageSummayTable;
	QCustomPlot *AccountPlot,**P_AccountPlot, *MonitorPageTotalPnLPlot, *MonitorPageKindPnLPlot, *MonitorPagePositionPlot;
	QTabWidget *AccountTab,**P_AccountTab;
	QWidget *AccountHoldingWidget, **P_AccountHoldingWidget, *AccountTradingWidget, **P_AccountTradingWidget,*AccountLogWidget,**P_AccountLogWidget;
	QTableView *AccountHoldingTable, **P_AccountHoldingTable, *AccountTradingTable, **P_AccountTradingTable;
	QListView *AccountLogListview, **P_AccountLogListview;
	QHBoxLayout *AccountTabHorizontalLayout,**P_AccountTabHorizontalLayout;
	//账号信息
	list<string> accounts;
	int n, num_accounts, sizeofplot;
	//时间变量
	double  startTime;
	QTime now_min,start_time;
	//画图所需数据
	std::map<QTime, double>plotdata;
	//汇总统计信息
	Display_Monitor total_monitor;

	 

	//*************************************************函数声明**********************************************///
	///从外部获取账号列表和账号信息
	void GetAccount();

	///UI界面外部整体框架的初始化
	void InitUi();

	///UI界面中监控页整体布局的初始化设计
	void InitUiMonitorPage();

	///UI界面中各个账户页面整体布局的初始化设计
	void InitUiAccountPages();

	///UI界面中框架内部内容的初始化
	void Init();

	///UI界面中监控页框架内部内容的初始化
	void InitMonitorPage();

	///UI界面中各个账户页面框架内部内容的初始化
	void InitAccountPages();

	///实时汇总计算函数
	void CalculateSummary();

	/// 重写QObject类的虚函数 （继承自QObject)
	virtual void timerEvent(QTimerEvent * event);

	///更新监控页内容
	void UpdateMonitorPage();

	///更新监控页总盈亏折线图
	void UpdateMonitorPageTotalPnLPlot();

	///更新监控页大类盈亏柱状图
	void UpdateMonitorPageKindPnLPlot();

	///更新监控页分账户仓位分布柱状图
	void UpdateMonitorPagePositionPlot();

	///更新各个账户内容
	void UpdateAccountPages();


private:
	Ui::QtGuiSurveillanceMultiClass ui;
	QStandardItemModel  *tablemodel_position, *tablemodel_trade, *tablemodel_summary, *tablemodel_account,*MonitorPageStopTableMode,*MonitorPageSummayTableModel;
	QStandardItemModel **p_tablemodel_position, **p_tablemodel_trade, **p_tablemodel_summary, **p_tablemodel_account;
	QStringListModel *listviewmodel,**p_listviewmodel;
	QCPBars *Agriculture, *Black, *BaseMetal, *Energy, *PreciousMetal, *Financial,*regen;
	QSharedPointer<QCPAxisTickerText> textTicker, positiontextTicker;
	int m_timerId; // 每个Timer有一个id
};
