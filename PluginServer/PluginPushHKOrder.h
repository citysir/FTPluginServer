#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct_Trade.h"
#include "JsonCpp/json.h"

class CPluginHKTradeServer;

class CPluginPushHKOrder
{
public:
	CPluginPushHKOrder();
	virtual ~CPluginPushHKOrder();
	
	void Init(CPluginHKTradeServer* pTradeServer, ITrade_HK* pTradeOp);
	void Uninit();	
	void PushOrderData(const Trade_OrderItem& orderItem, int nEnv, SOCKET sock);

	void NotifySocketClosed(SOCKET sock);

protected:
	struct	StockDataReq
	{
		SOCKET	sock;	
		PushHKOrder_Req req;
	};

	//tomodify 1
	typedef PushHKOrderAckBody	QuoteAckDataBody;
	typedef std::map<std::pair<UINT64, SOCKET>, QuoteAckDataBody>	ORDER_LAST_PUSH;

protected:
	CPluginHKTradeServer* m_pTradeServer;
	ITrade_HK* m_pTradeOp;

	ORDER_LAST_PUSH m_mapLastAckBody;//保存上次推送的数据，用于查重
};