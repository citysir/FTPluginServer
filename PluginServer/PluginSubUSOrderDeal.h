#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct_Trade.h"
#include "TimerWnd.h"
#include "MsgHandler.h"
#include "JsonCpp/json.h"

class CPluginUSTradeServer;
class CPluginPushUSOrder;
class CPluginPushUSDeal;

class CPluginSubUSOrderDeal : public CTimerWndInterface, public CMsgHandlerEventInterface
{
	//tomodify 1
	typedef SubUSOrderDeal_Req	TradeReqType;
	typedef SubUSOrderDeal_Ack	TradeAckType;

public:
	CPluginSubUSOrderDeal();
	virtual ~CPluginSubUSOrderDeal();
	
	void Init(CPluginUSTradeServer* pTradeServer, ITrade_US* pTradeOp, 
		CPluginPushUSOrder* pOrderPusher, CPluginPushUSDeal* pDealPusher);
	void Uninit();	
	void SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock);
	void SetTradeReqData(const TradeReqType &req, SOCKET sock);
	void NotifyOnSubUSOrder(Trade_Env enEnv, UINT32 nCookie, std::string strOrderID,
		std::string strSubOrderSuc, std::string strSubDealSuc);

	void NotifySocketClosed(SOCKET sock);

	void GetSubOrderSocket(Trade_Env enEnv, UINT64 nOrderID, std::vector<SOCKET> &vtSock);
	void GetSubDealSocket(Trade_Env enEnv, UINT64 nOrderID, std::vector<SOCKET> &vtSock);

	//到了订单最终状态，清理对应订阅信息
	void ClearSubOrderInfo(UINT64 nOrderID);
	void ClearSubDealInfo(UINT64 nOrderID);

	void NotifyUnLockTrade(SOCKET sock, Trade_SvrResult enSvrRet);

protected:
	//CTimerWndInterface 
	virtual void OnTimeEvent(UINT nEventID);

	//CMsgHandlerEventInterface
	virtual void OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam);

protected:

	struct	StockDataReq
	{
		SOCKET	sock;
		DWORD	dwReqTick;
		DWORD	dwLocalCookie;
		TradeReqType req;
	};
	
	typedef std::vector<StockDataReq*>				VT_REQ_TRADE_DATA;	
	typedef std::set<std::pair<UINT64, SOCKET> >	SET_ORDER_SOCKET;
	typedef	std::set<std::pair<UINT64, SOCKET> >	SET_DEAL_SOCKET;
	typedef std::map<UINT64, int>					MAP_ORDER_ENVTYPE;
	typedef std::map<SOCKET, int>					MAP_ORDER_SUBALL;
	typedef std::map<SOCKET, int>					MAP_DEAL_SUBALL;
	typedef std::pair<std::pair<SOCKET, TradeReqType>, UINT32> REQ_INFO;
	typedef std::vector<REQ_INFO>                    VT_REQ_QUEUE;
	typedef std::map<SOCKET, int>                    MAP_REQ_QUEUE_SOCKET;

protected:	
	void HandleTradeAck(TradeAckType *pAck, SOCKET sock);

	void HandleTimerClearSubInfo();
	void SetTimerClearSubInfo(bool bStartOrStop);

	void HandleReqUnlockTimeout();
	void SetTimerReqUnlockTimeout(bool bStartOrStop);

	void ClearAllReqAckData();
	
private: 
	bool DoDeleteReqData(StockDataReq* pReq); 

private:
	void DoClearStocketInfo(SOCKET socket);
	bool IsSubOrder(UINT64 nOrderID);
	bool IsSubDeal(UINT64 nOrderID);
	void ClearOrderEnvInfo(UINT64 nOrderID);
	void ParseOrderIDStr(std::string strOrderID, std::vector<UINT64> &vOrderID);
	bool SubOrderDeal(SOCKET socket, Trade_Env enEnv, UINT64 nOrderID, bool bSubOrder, bool bSubDeal, bool bFirstPush);
	void PushOrder(SOCKET sock, Trade_Env enEnv, UINT64 nOrderID);
	void PushDeal(SOCKET sock, Trade_Env enEnv, UINT64 nOrderID);

protected:
	CPluginUSTradeServer	*m_pTradeServer;
	ITrade_US				*m_pTradeOp;
	CPluginPushUSOrder		*m_pOrderPusher;
	CPluginPushUSDeal		*m_pDealPusher;
	bool					m_bStartTimerClearSubInfo;
	bool                    m_bStartTimerReqUnlockTimeout;

	CTimerMsgWndEx		m_TimerWnd;
	CMsgHandler			m_MsgHandler;

	VT_REQ_TRADE_DATA	m_vtReqData;
	SET_ORDER_SOCKET	m_mapSubOrder;
	SET_DEAL_SOCKET		m_mapSubDeal;
	MAP_ORDER_ENVTYPE	m_mapEnvType;

	MAP_ORDER_SUBALL	m_mapSubAllOrderSocket;
	MAP_DEAL_SUBALL		m_mapSubAllDealSocket;
	VT_REQ_QUEUE        m_vtReqQueue;
	MAP_REQ_QUEUE_SOCKET m_mapReqQueueSocket;
};