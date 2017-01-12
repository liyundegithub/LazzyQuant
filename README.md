# 量化策略交易系统

## 模块功能:
1. 接收行情数据接收模块发送的行情数据, 生成各个不同时间尺度的K线数据  
2. 储存收集到的K线数据, 必要时在程序启动时导入以前收集的K线数据  
3. 在有行情数据到来时运行相关合约的量化策略, 做出开/平仓, 止损/止盈等决策  
4. 将量化策略做出的决策总结后发送合约目标仓位到执行模块  

## 配置文件示例:
文件名: quant_trader.ini  
<pre><code>
[HistoryPath]
ktexport=F:/KT/KTExportData     # 飞狐交易师历史数据导出目录
collector=E:/collected          # K线数据收集保存目录(程序启动时导入此目录中的数据)

[Collector]                     # 需要收集的合约K线 (1分钟线是默认被收集的)
cu1703=MIN5 | MIN15 | MIN60     # cu1703的1分钟线, 5分钟线, 15分钟线, 60分钟线
i1705=MIN5 | MIN15 | MIN60      # i1705的1分钟线, 5分钟线, 15分钟线, 60分钟线
CF705=MIN1                      # CF705的1分钟线
ag1706=MIN5 | MIN15             # ag1706的1分钟线, 5分钟线, 15分钟线
</code></pre>

文件名: trade_strategy.ini  
<pre><code>
[1]                             # 策略ID
strategy=DblMaPsarStrategy      # 策略名称 (与代码中的策略类名相同)
instrument=cu1703               # 合约代码
timeframe=MIN15                 # 时间框架 (枚举)
param1=3                        # 第一个参数 (int型)
param2=16                       # 第二个参数 (int型)
param3=MODE_EMA                 # 第三个参数 (枚举, 平均线类型)
param4=PRICE_TYPICAL            # 第四个参数 (枚举, 价格类型)
param5=0.02                     # 第五个参数 (double型)
param6=0.18                     # 第六个参数 (double型)
                                # 最多九个参数

[2]
Strategy=DblMaPsarStrategy
Instrument=i1705
timeframe=MIN5
param1=3
param2=16
param3=MODE_SMA
param4=PRICE_CLOSE
param5=0.02
param6=0.18
</code></pre>
