# 基于CTP和Qt的可视化期货监控系统
![Alt text](https://github.com/culi123/QtGuiSurveillanceMulti/blob/master/Screenshots/GUI%E5%8A%A8%E5%9B%BE.gif)
## 主要功能:
国内期货多账户的盈亏分类汇总以及实时监控。
## 界面介绍：
主监控界面
![Alt text](https://github.com/culi123/QtGuiSurveillanceMulti/blob/master/Screenshots/%E4%B8%BB%E7%9B%91%E6%8E%A7%E9%A1%B5.jpg)
分账户监控界面
界面1
![Alt text](https://github.com/culi123/QtGuiSurveillanceMulti/blob/master/Screenshots/%E8%B4%A6%E6%88%B71.jpg)
界面2
![Alt text](https://github.com/culi123/QtGuiSurveillanceMulti/blob/master/Screenshots/%E8%B4%A6%E6%88%B72.jpg)

## 原理说明：
  * 信息获取：通过CTP的API接口获取期货账户持仓信息，以及实时的价格数据。
  * UI：利用Qt完成界面设计。其中动、静态态图片通过QCustomPlot - v2.0实现。
## 使用说明：
  * 信息配置：通过外部文件，配置账户数量以及账户信息。
  * 点击开启。
