#include "stdafx.h"
#include "PluginPushUSDeal.h"
#include "PluginUSTradeServer.h"
#include "Protocol/ProtoPushUSDeal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_TDUS_PUSH_DEAL
typedef CProtoPushUSDeal	CProtoQuote;

#define TIMER_ID_CLEAR_LAST_PUSH_DATA		356    

//////////////////////////////////////////////////////////////////////////

CPluginPushUSDeal::CPluginPushUSDeal()
{	
	m_pTradeOp = NULL;
	m_pTradeServer = NULL;
	m_bStartTimerClearSubInfo = false;
}

CPluginPushUSDeal::~CPluginPushUSDeal()
{
	Uninit();
}

void CPluginPushUSDeal::Init(CPluginUSTradeServer* pTradeServer, ITrade_US*  pTradeOp)
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

	m_TimerWnd.SetEventInterface(this);
	m_TimerWnd.Create();

	SetTimerClearLashPushData(true);
}

void CPluginPushUSDeal::Uninit()
{
	if (m_pTradeServer != NULL)
	{
		m_pTradeServer = NULL;
		m_pTradeOp = NULL;

		SetTimerClearLashPushData(false);

		m_TimerWnd.Destroy();
		m_TimerWnd.SetEventInterface(NULL);
	}
}

void CPluginPushUSDeal::PushDealData(const Trade_DealItem& dealItem, int nEnv, SOCKET sock)
{
	CHECK_RET(m_pTradeServer, NORET);

	QuoteAckDataBody ackbody;
 	ackbody.nEnvType = nEnv;
	ackbody.nOrderID = dealItem.nOrderID;
	ackbody.nDealID = dealItem.nDealID;
	ackbody.enSide = dealItem.enSide;
	ackbody.strStockCode = std::wstring(dealItem.szCode);
	ackbody.strStockName = std::wstring(dealItem.szName);
	ackbody.nPrice = dealItem.nPrice;
	ackbody.nQty = dealItem.nQty;
	ackbody.nTime = dealItem.nTime;

	std::pair<UINT64, SOCKET> Deal = std::make_pair(dealItem.nOrderID, sock);
	if (m_mapLastAckBody.count(Deal) != 0)
	{
		QuoteAckDataBody &LastPushData = m_mapLastAckBody[Deal];

		if (LastPushData.Equal(ackbody))
		{
			return;
		}
	}
	m_mapLastAckBody[Deal] = ackbody;
	m_mapOrderEnv[dealItem.nOrderID] = nEnv;

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

void CPluginPushUSDeal::NotifySocketClosed(SOCKET sock)
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

void CPluginPushUSDeal::OnTimeEvent(UINT nEventID)
{
	if (TIMER_ID_CLEAR_LAST_PUSH_DATA == nEventID)
	{
		HandleTimerClearLashPushData();
	}
}

void CPluginPushUSDeal::SetTimerClearLashPushData(bool bStartOrStop)
{
	if (m_bStartTimerClearSubInfo)
	{
		if (!bStartOrStop)
		{
			m_TimerWnd.StopTimer(TIMER_ID_CLEAR_LAST_PUSH_DATA);
			m_bStartTimerClearSubInfo = false;
		}
	}
	else
	{
		if (bStartOrStop)
		{
			m_TimerWnd.StartTimer(30, TIMER_ID_CLEAR_LAST_PUSH_DATA);
			m_bStartTimerClearSubInfo = true;
		}
	}
}

void CPluginPushUSDeal::HandleTimerClearLashPushData()
{
	DEAL_LAST_PUSH::iterator iter = m_mapLastAckBody.begin();
	UINT64 nSrvTimeStamp = 0;
	m_pTradeOp->GetServerTime(nSrvTimeStamp);

	for (; iter != m_mapLastAckBody.end();)
	{
		Trade_OrderItem orderItem = {};
		UINT64 nOrderID = iter->first.first;
		bool bRet = m_pTradeOp->GetOrderItem(nOrderID, orderItem);
		if (!bRet || IsUSOrderFinalStatus(orderItem.nStatus))
		{
			if (nSrvTimeStamp - orderItem.nUpdatedTime >= 60)
			{
				iter = m_mapLastAckBody.erase(iter);
				m_mapOrderEnv.erase(nOrderID);
				continue;
			}
		}
		++iter;
	}
}
