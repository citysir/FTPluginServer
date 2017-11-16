#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct_Trade.h"
#include "JsonCpp/json.h"
#include "TimerWnd.h"

class CPluginHKTradeServer;

class CPluginPushHKDeal : public CTimerWndInterface
{
public:
	CPluginPushHKDeal();
	virtual ~CPluginPushHKDeal();

	void Init(CPluginHKTradeServer* pTradeServer, ITrade_HK* pTradeOp);
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
		PushHKDeal_Req req;
	};

	//tomodify 1
	typedef PushHKDealAckBody					QuoteAckDataBody;
	typedef std::map<std::pair<UINT64, SOCKET>, QuoteAckDataBody>	DEAL_LAST_PHKH;
	typedef std::map<UINT64, int>				ORDER_ENV;

protected:
	CPluginHKTradeServer* m_pTradeServer;
	ITrade_HK* m_pTradeOp;

	CTimerMsgWndEx m_TimerWnd;
	bool m_bStartTimerClearSubInfo;
	DEAL_LAST_PHKH m_mapLastAckBody;//保存上次推送的数据，用于查重
	ORDER_ENV m_mapOrderEnv;
};