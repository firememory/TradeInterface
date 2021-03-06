﻿#ifndef _TRADEAPIH_
#define _TRADEAPIH_

#include <string>
#include <map>
#include <list>
#include <queue>
#include <vector>
#include <exception>
#include <stdexcept>
#include <memory>
#include <assert.h>
#include <cmath>

//#ifdef TRADEGATEWAY_API_EXPORTS
//#define TRADE_API __declspec(dllexport)
//#else
//#define TRADE_API __declspec(dllimport)
//#endif

namespace TradeAPI {
    enum MessageType {
        MTUnknown,
        MTInfo,
        MTWarning,
        MTError,
        MTCommand,
    };

    typedef std::map<std::string, std::string> DICT;
    //账户信息
    struct AccountInfo {
        std::string currency;            //币种  RMB,USD,HKD
        double fundBalance;             //资金余额
        double equity;                  //浮动权益
        double fundAvaForTrade;         //资金交易可用
        double positionsMarketValue;    //参考持仓市值
        double fundAvaForTransout;      //资金可转出
        DICT extra; //其他字段

        AccountInfo(){
            fundBalance = NAN;
            equity =  NAN;
            fundAvaForTrade =  NAN;
            positionsMarketValue =  NAN;
            fundAvaForTransout =  NAN;
        }
    };

    struct Instrument {
        std::string id;         //唯一ID
        std::string symbol;     //代码
        std::string localSymbol;//本地名称
        std::string currency;   //币种  RMB,USD,HKD
        std::string secType;    //STK,ETF,IND,BOND,FUT,CASH,CMDTY,FOP,OPT
        std::string exchange;   //SSE, SZSE, SHFE, DCE, CZCE, CFFEX, HKEX,ECBOT,SMART
        std::string strike;     //执行价
        std::string right;      //P=PUT or C=CALL
        std::string multiplier; //乘数
        std::string expiry;     //到期日201103

        void generate_id(void) {
            if (secType == "OPT") {
                id = symbol + "." + secType + "." + exchange + "." + currency + "." + expiry + "." + right + "." +
                     strike;
            }
            else if (secType == "FUT") {
                id = symbol + "." + secType + "." + exchange + "." + currency + "." + expiry;
            }
            else {
                id = symbol + "." + secType + "." + exchange + "." + currency;
            }
        }
    };

    struct Fee {
        std::string name;     //费用名称
        double volumeFee;     //按交易量的费用
        double turnoverFee;  //按交易金额的费用
        double minFee;       //每笔成交的最小费用

        double get_fee(long volume, double turnover) {
            assert(volume > 0);
            assert(turnover > 0);
            double ret = volume * volumeFee + turnover * turnoverFee;
            if (ret < minFee)
                ret = minFee;
            return ret;
        }

        Fee(){
            volumeFee = NAN;
            turnoverFee = NAN;
            minFee = NAN;
        }
    };

    typedef std::vector<Fee> FeesSeq;

    struct InstrumentDetails {
        Instrument summary;
        FeesSeq fees;
        std::string execStyle;  //USA,EUR
        std::string underlying; //标的代码
        double minTick;         //最小报价单位元
        double upStopPrice;     //涨停价
        double downStopPrice;   //跌停价
        long minOrderUnit;      //最小委托单位
        long maxOrderSize;      //最大委托上限
        std::string contractMonth;
        //合约月份
        std::string expiration; //到期日
        std::string marketName;
        std::string longName;
        std::string timeZoneId; //时间 UCT EST
        std::string tradingHours;
        //xxxxxx:xxxxxx;xxxxxx:xxxxxx
        DICT extra;  //其他细节信息

        InstrumentDetails(){
            minTick = NAN;
            upStopPrice = NAN;
            downStopPrice = NAN;
            minOrderUnit = 0;
            maxOrderSize = 0;
        }
    };
    typedef std::shared_ptr<InstrumentDetails> InstrumentDetailsPtr;
    typedef std::map<std::string, InstrumentDetailsPtr> InstrumentDetailsDict;

    enum OrderStatus {
        //以下是委托未确认状态
        OSNew,           //委托最初状态，还未发出该委托
        OSPendingNew,    //委托已发
        OSWorking,       //委托在成交中(已经有成交了,第一次有成交就更新为该状态）
        OSNeedCancel,    //委托请求撤单，还未发生该请求
        OSPendingCancel, //委托撤单请求已发

        //以下是确定状态
        OSFilled,        //全部成交了，委托量==成交量
        OSCanceled,      //委托量==成交量+撤销量，撤销量>0
        OSStopped,       //委托已停止，委托量==成交量+撤销量，撤销量>0
        OSRejected,      //委托已拒绝，成交量=0

        //未决状态
        OSUnknown,
    };

    enum ExecutionReportType {
        RTCanceled, //撤单回报
        RTRejected, //拒绝回报
        RTStopped,  //停止回报,日终停止
        RTTrade,     //成交回报
        RTUnknown,
    };

    struct ExecutionReport {
        std::string ordId;
        std::string execId;       //成交编号
        ExecutionReportType type;
        std::string ordRejReason;
        double lastQty;
        double lastPx;
        int date;
        int time;
        ExecutionReport(){
            type =RTUnknown;
            lastQty = 0;
            lastPx = NAN;
            date = 0;
            time = 0;
        }
    };

    typedef std::shared_ptr<ExecutionReport> ExecutionReportPtr;
    typedef std::list<ExecutionReportPtr> ExecutionReportSeq;

    //查询委托返回的结果
    struct OrderReport {
        OrderStatus ordStatus;
        double totalQty;            //成交量
        double avgPric;             //成交均价
        double leavesQty;             //剩余量
        ExecutionReportSeq exec;
        DICT extra;
        OrderReport(){
            ordStatus = OSNew;
            totalQty = NAN;
            avgPric= NAN;
            leavesQty =NAN;
        }
    };

    struct Order {
        long id;             //本地ID
        std::string account;
        Instrument inst;

        // Buy:买,Sell:卖,
        // Subscribe:申购,Redeem:赎回,
        // LendBuy:融资买入,BorrowSell:融券卖出
        // SellRepayment:卖券还款,BuyGiveBack:买券还券,GiveBack:现券还券,Repayment:直接还款
        std::string side;
        std::string posEfct;  // Open,Close, CloseToday
        /*
            市价委托类型
            Market0 以买一的价格买入，是对方买入。
            Market1 以卖一的价格买入，是本方买入。
            Market3 以你要买入的股票数量看看买到卖一、二、三共有多少股一起吃入，一次成交，是即时买入。
            Market5 以卖五的价格全部吃进，是五档买入。
            Market  以涨停的价格大批买入，是全额买入。
         */
        std::string type;     // Limit限定价格 ,Market:市价委托
        double lmtPrice;
        double ordQty;
        std::string ordId; //接口返回的编号
        int createDate;
        int createTime;
        std::string text;      //备注
        int endDate;
        int endTime;
        OrderReport report;
        DICT extra;

        Order(){
            id = 0;
            lmtPrice = NAN;
            ordQty = NAN;
            createDate = 0;
            createTime = 0;
            endDate = 0;
            endTime = 0;
        }
    };

    typedef std::shared_ptr<Order> OrderPtr;
    typedef std::list<OrderPtr> OrderSeq;
    typedef std::map<std::string, OrderPtr> OrderDict;
    typedef std::queue<OrderPtr> OrderQueue;

    enum PositionDirection {
        PDUnknown,
        PDLong, //多头
        PDShort //空头
    };

    struct PositionInfo {
        std::string account;
        Instrument inst;

        //NetDelta(DLT):净增加的量,Credit Event Adjustment(CEA):信用交易量
        std::string positionType;
        PositionDirection direction;
        long balance;           //已交收余额
        long holdPosition;      //总计持有量
        long prePosition;       //昨日及以前生成的仓
        long todayPosition;     //今日生成的仓=可以用于申购或者用于赎回的量
        long canClosePosition;  //可平仓的量，或者可以卖出的量
        double avgPrice;        //持仓均价
        double margin;          //保证金占用
        double mktPrice;        //当前价
        DICT extra;

        PositionInfo(){
            balance = 0;
            holdPosition = 0;
            prePosition = 0;
            todayPosition = 0;
            canClosePosition = 0;
            avgPrice =NAN;
            margin = NAN;
            mktPrice = NAN;
        }
    };

    typedef std::shared_ptr<PositionInfo> PositionInfoPtr;
    typedef std::list<PositionInfoPtr> PositionInfoSeq;
    typedef std::map<std::string, PositionInfoPtr> PositionDict;

    enum ResumeType {
        RTRestart, //从本交易日开始重传
        RTResume,  //从指定的消息ID续传
        RTQuick    //只传送请求后的内容
    };

    //API Error,此接口API触发的错误，比如API参数输入不准确
    class api_issue_error : public std::logic_error {    // base of all logic-error exceptions
    public:
        typedef std::logic_error _Mybase;

        explicit api_issue_error(const std::string &_Message)
                : _Mybase(_Message.c_str()) {
        }

        explicit api_issue_error(const char *_Message)
                : _Mybase(_Message) {
        }
    };

    enum EventType {
        ETMessage = 0x00000001,
        ETOrder = 0x00000010,
        ETAccount = 0x00000100,
        ETPosition = 0x00001000
    };

    // callback
    class EventReceiver {
    public:
        //Session信息反馈
        virtual void onMessage(MessageType type, const std::string &msg) = 0;

        //委托的执行报告，reqExecRep后可能会收到消息，cancelRequest，cancelAllRequest撤销该请求
        virtual void onOrderExecution(const long id, const Order &order, const ExecutionReportSeq &report) = 0;

        //调用newOrderSingle,cancelOrderSingle,newOrders,cancelOrders后会导致委托状态变化
        //或者因为成交、拒绝等导致委托状态变化
        virtual void onOrderStatus(const Order &order) = 0;

        //订阅了资产变动后subscribeAssetChange
        virtual void onFundAccountChanged(const AccountInfo &info) = 0;

        //持仓发生变动时触发
        virtual void onPositionChanged(const PositionInfo &info) = 0;
    };

    typedef std::shared_ptr<EventReceiver> EventReceiverPtr;

    class ITradeSession {
    public:
        //启动会话
        virtual void start(
                const std::string &sessionName,
                const std::string &account,
                const std::string &accPassword,
                EventReceiverPtr &receiver) = 0;

        //停止会话
        virtual void stop(void) = 0;

        //*pretrade
        //查询金融工具
        //type:STK,ETF,IND,BOND,FUT,CASH,CMDTY,FOP,OPT
        //market:SSE, SZSE, SHFE, DCE, CZCE, CFFEX, HKEX,ECBOT,SMART
        virtual InstrumentDetailsDict qryInstruments(const std::string &mkt, const std::string &secType) = 0;

        //订阅变动通知
        // events 是 EventType的组合
        virtual void subscribeEvents(const long events, ResumeType rt) = 0;

        virtual void cancelEvents(const long events) = 0;

        //*trading
        //seqId为续传的id，只有当ResumeType == Resume时生效
        virtual void reqExecReport(const ResumeType type, const std::string &seqId) = 0;

        virtual void cancelExeReport(void) = 0;

        //单个委托，返回委托
        virtual void newOrderSingle(OrderPtr &ord) = 0;

        //撤销单个委托
        virtual void cancelOrderSingle(const OrderPtr &ord) = 0;

        //查未成交委托
        virtual OrderSeq qryWorkingOrders(void) = 0;

        //*posttrade
        //查持仓信息,输入MktUnknown查全部市场
        virtual PositionInfoSeq qryPositions(const std::string &mkt) = 0;

        virtual ~ITradeSession() { };
    };

    extern "C" ITradeSession *create(void);

    extern "C" void destroy(ITradeSession *);
};

#endif //_TRADEINTERFACEH_