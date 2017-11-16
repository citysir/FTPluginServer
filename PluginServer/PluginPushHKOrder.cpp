#include "stdafx.h"
#include "PluginPushHKOrder.h"
#include "PluginHKTradeServer.h"
#include "Protocol/ProtoPushHKOrder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_TDHK_PUSH_ORDER
typedef CProtoPushHKOrder	CProtoQuote;


//////////////////////////////////////////////////////////////////////////

CPluginPushHKOrder::CPluginPushHKOrder()
{	
	m_pTradeOp = NULL;
	m_pTradeServer = NULL;
}

CPluginPushHKOrder::~CPluginPushHKOrder()
{
	Uninit();
}

void CPluginPushHKOrder::Init(CPluginHKTradeServer* pTradeServer, ITrade_HK*  pTradeOp)
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

void CPluginPushHKOrder::Uninit()
{
	if (m_pTradeServer != NULL)
	{
		m_pTradeServer = NULL;
		m_pTradeOp = NULL;
	}
}

void CPluginPushHKOrder::PushOrderData(const Trade_OrderItem& orderItem, int nEnv, SOCKET sock)
{
	CHECK_RET(m_pTradeServer, NORET);

	QuoteAckDataBody ackbody;
	ackbody.nEnvType = nEnv;
	ackbody.HKOrderItem.nOrderID = orderItem.nOrderID;
	ackbody.HKOrderItem.nOrderType = orderItem.nType;
	ackbody.HKOrderItem.enSide = orderItem.enSide;
	ackbody.HKOrderItem.nStatus = orderItem.nStatus;
	ackbody.HKOrderItem.nPrice = orderItem.nPrice;
	ackbody.HKOrderItem.nQty = orderItem.nQty;
	ackbody.HKOrderItem.nDealtQty = orderItem.nDealtQty;
	ackbody.HKOrderItem.nDealtAvgPrice = int(round(orderItem.fDealtAvgPrice * 1000));
	ackbody.HKOrderItem.nSubmitedTime = orderItem.nSubmitedTime;
	ackbody.HKOrderItem.nUpdatedTime = orderItem.nUpdatedTime;
	ackbody.HKOrderItem.nErrCode = orderItem.nErrCode;
	ackbody.HKOrderItem.strStockCode = orderItem.szCode;
	ackbody.HKOrderItem.strStockName = orderItem.szName;
	
	std::pair<UINT64, SOCKET> Order = std::make_pair(orderItem.nOrderID, sock);
	if (m_mapLastAckBody.count(Order) != 0)
	{
		QuoteAckDataBody &LastPushData = m_mapLastAckBody[Order];

		if (LastPushData.Equal(ackbody))
		{
			return;
		}

		if (IsHKOrderFinalStatus(orderItem.nStatus))
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

void CPluginPushHKOrder::NotifySocketClosed(SOCKET sock)
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
