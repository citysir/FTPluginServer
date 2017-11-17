#include "stdafx.h"
#include "PluginPushUSOrder.h"
#include "PluginUSTradeServer.h"
#include "Protocol/ProtoPushUSOrder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_TDUS_PUSH_ORDER
typedef CProtoPushUSOrder	CProtoQuote;


//////////////////////////////////////////////////////////////////////////

CPluginPushUSOrder::CPluginPushUSOrder()
{	
	m_pTradeOp = NULL;
	m_pTradeServer = NULL;
}

CPluginPushUSOrder::~CPluginPushUSOrder()
{
	Uninit();
}

void CPluginPushUSOrder::Init(CPluginUSTradeServer* pTradeServer, ITrade_US*  pTradeOp)
{
	if (m_pTradeServer != NULL)
		return;

	if (pTradeServer == NULL || pTradeOp == NULL)
	{
		ASSERT(false);
		return;
	}

	m_pTradeServer = pTradeServer;
	m_pTradeOp = pTradeOp;
}

void CPluginPushUSOrder::Uninit()
{
	if (m_pTradeServer != NULL)
	{
		m_pTradeServer = NULL;
		m_pTradeOp = NULL;
	}
}

void CPluginPushUSOrder::PushOrderData(const Trade_OrderItem& orderItem, int nEnv, SOCKET sock)
{
	CHECK_RET(m_pTradeServer, NORET);

	QuoteAckDataBody ackbody;
	ackbody.nEnvType = nEnv;
	ackbody.USOrderItem.nOrderID = orderItem.nOrderID;
	ackbody.USOrderItem.nOrderType = orderItem.nType;
	ackbody.USOrderItem.enSide = orderItem.enSide;
	ackbody.USOrderItem.nStatus = orderItem.nStatus;
	ackbody.USOrderItem.nPrice = orderItem.nPrice;
	ackbody.USOrderItem.nQty = orderItem.nQty;
	ackbody.USOrderItem.nDealtQty = orderItem.nDealtQty;
	ackbody.USOrderItem.nDealtAvgPrice = int(round(orderItem.fDealtAvgPrice * 1000));
	ackbody.USOrderItem.nSubmitedTime = orderItem.nSubmitedTime;
	ackbody.USOrderItem.nUpdatedTime = orderItem.nUpdatedTime;
	ackbody.USOrderItem.strStockCode = orderItem.szCode;
	ackbody.USOrderItem.strStockName = orderItem.szName;

	std::pair<UINT64, SOCKET> Order = std::make_pair(orderItem.nOrderID, sock);
	if (m_mapLastAckBody.count(Order) != 0)
	{
		QuoteAckDataBody &LastPushData = m_mapLastAckBody[Order];

		if (LastPushData.Equal(ackbody))
		{
			return;
		}

		if (IsUSOrderFinalStatus(orderItem.nStatus))
		{
			m_mapLastAckBody.erase(Order);
		}
		else
		{
			LastPushData = ackbody;
		}
	}
	else
	{
		m_mapLastAckBody[Order] = ackbody;
	}

	CProtoQuote::ProtoAckDataType ack;
	ack.head.nProtoID = PROTO_ID_QUOTE;
	ack.head.ddwErrCode = 0;
	ack.body = ackbody;

	CProtoQuote proto;
	proto.SetProtoData_Ack(&ack);

	Json::Value jsnAck;
	if (proto.MakeJson_Ack(jsnAck))
	{
		std::string strOut;
		CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
		m_pTradeServer->ReplyTradeReq(PROTO_ID_QUOTE, strOut.c_str(), (int)strOut.size(), sock);
	}
	else
	{
		CHECK_OP(false, NOOP);
	}
}

void CPluginPushUSOrder::NotifySocketClosed(SOCKET sock)
{
	for (auto iter = m_mapLastAckBody.begin(); iter != m_mapLastAckBody.end();)
	{
		if (iter->first.second == sock)
		{
			iter = m_mapLastAckBody.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}
