#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct_Trade.h"
#include "JsonCpp/json.h"
#include "TimerWnd.h"

class CPluginUSTradeServer;

class CPluginPushUSDeal : public CTimerWndInterface
{
public:
	CPluginPushUSDeal();
	virtual ~CPluginPushUSDeal();
	
	void Init(CPluginUSTradeServer* pTradeServer, ITrade_US* pTradeOp);
	void Uninit();	
	void PushDealData(const Trade_DealItem& dealItem, int nEnv, SOCKET sock);

	void NotifySocketClosed(SOCKET sock);

protected:
	//CTimerWndInterface 
	virtual void OnTimeEvent(UINT nEventID);

protected:
	void SetTimerClearLashPushData(bool bStartOrStop);
	void HandleTimerClearLashPushData();

protected:
	struct	StockDataReq
	{
		SOCKET	sock;	
		PushUSDeal_Req req;
	};

	//tomodify 1
	typedef PushUSDealAckBody					QuoteAckDataBody;
	typedef std::map<UINT64, int>				ORDER_ENV;
	typedef std::map<std::pair<UINT64, SOCKET>, QuoteAckDataBody>	DEAL_LAST_PUSH;

protected:
	CPluginUSTradeServer* m_pTradeServer;
	ITrade_US* m_pTradeOp;

	CTimerMsgWndEx m_TimerWnd;
	bool m_bStartTimerClearSubInfo;
	DEAL_LAST_PUSH m_mapLastAckBody;//保存上次推送的数据，用于查重
	ORDER_ENV m_mapOrderEnv;
};