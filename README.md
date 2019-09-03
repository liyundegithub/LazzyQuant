# LazzyQuant 期货/期权量化交易系统

## 主要功能:
1. 接收市场行情数据, 生成K线数据并保存至数据库  
2. 根据交易策略, 寻找获利机会并做出开仓/平仓, 止损/止盈等决策  
3. 根据决策自动执行报, 撤单操作  
4. 支持上期CTP交易接口  
5. 支持多种数据源用于盘后复盘  
6. 支持美式/欧式期权盘中实时定价 (基于二叉树模型)  
7. 支持各种套利策略  
8. 支持预埋单, 组合单  
9. 支持兼容MQL5语法的策略设计  

## 开发与测试环境:
&emsp;&emsp;Visual Studio 2015/2017 (Windows)  
&emsp;&emsp;GCC 5.3/8.2 (Linux)  
&emsp;&emsp;Qt 5.12.4 (最低要求5.10)  
&emsp;&emsp;Boost 1.68  
&emsp;&emsp;D-Bus 1.10.28  
&emsp;&emsp;MySQL 5.7.26  

#### 部分开发文档存放在LazzyDocument仓库中  
