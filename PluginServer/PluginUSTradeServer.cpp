#include "stdafx.h"
#include "PluginUSTradeServer.h"
#include "PluginNetwork.h"
#include "Protocol/ProtoOrderErrorPush.h"
#include "Protocol/ProtoBasicPrice.h"
#include "IManage_SecurityNum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern GUID PLUGIN_GUID;

//////////////////////////////////////////////////////////////////////////

CPluginUSTradeServer::CPluginUSTradeServer()
{
	m_pPluginCore = NULL;
	m_pTradeOp = NULL;	
	m_pNetwork = NULL;
}

CPluginUSTradeServer::~CPluginUSTradeServer()
{
	UninitTradeSvr();
}

void CPluginUSTradeServer::InitTradeSvr(IFTPluginCore* pPluginCore, CPluginNetwork *pNetwork)
{
	if ( m_pPluginCore != NULL )
		return;

	if ( pPluginCore == NULL || pNetwork == NULL )
	{
		ASSERT(false);
		return;
	}

	m_pNetwork = pNetwork;
	m_pPluginCore = pPluginCore;
	pPluginCore->QueryFTInterface(IID_IFTTrade_US, (void**)&m_pTradeOp);
	pPluginCore->QueryFTInterface(IID_IFTDataReport, (void**)&m_pDataReport);

	IFTQuoteData *pQuoteData = NULL;
	pPluginCore->QueryFTInterface(IID_IFTQuoteData, (void**)&pQuoteData);
	CHECK_OP(pQuoteData, NOOP);

	if ( m_pTradeOp == NULL || m_pDataReport == NULL )
	{
		ASSERT(false);		
		m_pTradeOp = NULL;
		m_pDataReport = NULL;
		m_pPluginCore = NULL;
		m_pNetwork = NULL;
		return;
	}	

	m_QueryPos.Init(this, m_pTradeOp, pQuoteData);
	m_QueryUSAcc.Init(this, m_pTradeOp);
	m_QueryUSOrder.Init(this, m_pTradeOp);
	m_PlaceOrder.Init(this, m_pTradeOp, pQuoteData);
	m_ChangeOrder.Init(this, m_pTradeOp);
	m_SetOrderStatus.Init(this, m_pTradeOp);
	m_QueryUSDeal.Init(this, m_pTradeOp);
	m_QueryHisOrder.Init(this, m_pTradeOp);
	m_QueryHisDeal.Init(this, m_pTradeOp);
	m_PushUSOrder.Init(this, m_pTradeOp);
	m_PushUSDeal.Init(this, m_pTradeOp);
	m_SubUSOrderDeal.Init(this, m_pTradeOp, &m_PushUSOrder, &m_PushUSDeal);
	
}

void CPluginUSTradeServer::UninitTradeSvr()
{
	if ( m_pPluginCore != NULL )
	{
		m_QueryPos.Uninit();
		m_QueryUSAcc.Uninit();
		m_QueryUSOrder.Uninit();
		m_PlaceOrder.Uninit();
		m_ChangeOrder.Uninit();
		m_SetOrderStatus.Uninit();
		m_QueryUSDeal.Uninit();
		m_QueryHisOrder.Uninit();
		m_QueryHisDeal.Uninit();
		m_PushUSOrder.Uninit();
		m_PushUSDeal.Uninit();
		m_SubUSOrderDeal.Uninit();

		m_pTradeOp = NULL;
		m_pDataReport = NULL;
		m_pPluginCore = NULL;
		m_pNetwork = NULL;
	}	
}

void CPluginUSTradeServer::SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	m_pDataReport->PLSCmdIDReport(PLUGIN_GUID, nCmdID);

	switch (nCmdID)
	{ 
	case PROTO_ID_TDUS_PLACE_ORDER:
		m_PlaceOrder.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDUS_SET_ORDER_STATUS:
		m_SetOrderStatus.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDUS_CHANGE_ORDER:
		m_ChangeOrder.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDUS_QUERY_ORDER:
		m_QueryUSOrder.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDUS_QUERY_ACC_INFO:
		m_QueryUSAcc.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDUS_QUERY_POSITION:
		m_QueryPos.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDUS_QUERY_DEAL:
		m_QueryUSDeal.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDUS_QUERY_HIS_ORDER:
		m_QueryHisOrder.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDUS_QUERY_HIS_DEAL:
		m_QueryHisDeal.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDUS_SUB_ORDER_DEAL:
		m_SubUSOrderDeal.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	default:
		CHECK_OP(false, NOOP);
		BasicPrice_Ack Ack;
		Ack.head.ddwErrCode = PROTO_ERR_PARAM_ERR;
		Ack.head.nProtoID = nCmdID;
		CA::Unicode2UTF(L"协议号错误！", Ack.head.strErrDesc);
		CProtoBasicPrice proto;	
		proto.SetProtoData_Ack(&Ack);

		Json::Value jsnAck;
		if ( proto.MakeJson_Ack(jsnAck) )
		{
			std::string strOut;
			CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
			m_pNetwork->SendData(sock, strOut.c_str(), (int)strOut.size());
		}
		break;
	}
}

void CPluginUSTradeServer::ReplyTradeReq(int nCmdID, const char *pBuf, int nLen, SOCKET sock)
{
	CHECK_RET(nCmdID && pBuf && nLen && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pNetwork, NORET);
	m_pNetwork->SendData(sock, pBuf, nLen);
}

void CPluginUSTradeServer::CloseSocket(SOCKET sock)
{
	m_PlaceOrder.NotifySocketClosed(sock);
	m_ChangeOrder.NotifySocketClosed(sock);
	m_SetOrderStatus.NotifySocketClosed(sock);

	m_QueryUSOrder.NotifySocketClosed(sock);
	m_QueryUSAcc.NotifySocketClosed(sock);
	m_QueryPos.NotifySocketClosed(sock);

	m_QueryUSDeal.NotifySocketClosed(sock);

	m_QueryHisOrder.NotifySocketClosed(sock);
	m_QueryHisDeal.NotifySocketClosed(sock);

	m_PushUSOrder.NotifySocketClosed(sock);
	m_PushUSDeal.NotifySocketClosed(sock);
	m_SubUSOrderDeal.NotifySocketClosed(sock);

}

void CPluginUSTradeServer::OnUnlockTrade(UINT32 nCookie, Trade_SvrResult enSvrRet, UINT64 nErrCode)
{
	SOCKET sock;
	if (!IManage_SecurityNum::GetSocketByCookie(nCookie, sock))
	{
		return;//不是通过脚本的解锁
	}

	if (enSvrRet == Trade_SvrResult_Failed)
	{
		IManage_SecurityNum::DeleteCookieSocket(nCookie);
	}
	else
	{
		IManage_SecurityNum::AddSafeSocket(nCookie);
	}

	m_SubUSOrderDeal.NotifyUnLockTrade(sock, enSvrRet);
}

void CPluginUSTradeServer::OnPlaceOrder(UINT32 nCookie, Trade_SvrResult enSvrRet, UINT64 nLocalID, INT64 nErrHash)
{
	m_PlaceOrder.NotifyOnPlaceOrder(Trade_Env_Real, nCookie, enSvrRet, nLocalID, nErrHash);
}

void CPluginUSTradeServer::OnOrderUpdate(const Trade_OrderItem& orderItem)
{ 
	std::vector<SOCKET> vtSock;
	const UINT64 &nOrderID = orderItem.nOrderID;
	m_SubUSOrderDeal.GetSubOrderSocket(Trade_Env_Real, nOrderID, vtSock);
	for (auto iter = vtSock.begin(); iter != vtSock.end(); ++iter)
	{
		m_PushUSOrder.PushOrderData(orderItem, Trade_Env_Real, *iter);
	}

	if (IsUSOrderFinalStatus(orderItem.nStatus))
	{
		m_SubUSOrderDeal.ClearSubOrderInfo(nOrderID);
	}
}

void CPluginUSTradeServer::OnCancelOrder(UINT32 nCookie, Trade_SvrResult enSvrRet, UINT64 nOrderID, INT64 nErrHash)
{
	m_SetOrderStatus.NotifyOnSetOrderStatus(Trade_Env_Real, nCookie, enSvrRet, nOrderID, nErrHash);
}

void CPluginUSTradeServer::OnChangeOrder(UINT32 nCookie, Trade_SvrResult enSvrRet, UINT64 nOrderID, INT64 nErrHash)
{
	m_ChangeOrder.NotifyOnChangeOrder(Trade_Env_Real, nCookie, enSvrRet, nOrderID, nErrHash);
}
 
void CPluginUSTradeServer::OnQueryOrderList(UINT32 nCookie, INT32 nCount, const Trade_OrderItem* pArrOrder)
{
	m_QueryUSOrder.NotifyOnQueryUSOrder(nCookie, nCount, pArrOrder);
}

void CPluginUSTradeServer::OnQueryDealList(UINT32 nCookie, INT32 nCount, const Trade_DealItem* pArrOrder)
{
	m_QueryUSDeal.NotifyOnQueryUSDeal(nCookie, nCount, pArrOrder);
}

void CPluginUSTradeServer::OnQueryAccInfo(UINT32 nCookie, const Trade_AccInfo& accInfo, int nResult)
{
	m_QueryUSAcc.NotifyOnQueryUSAccInfo(Trade_Env_Real, nCookie, accInfo, nResult);
}

void CPluginUSTradeServer::OnQueryPositionList( UINT32 nCookie, INT32 nCount, const Trade_PositionItem* pArrPosition )
{
	m_QueryPos.NotifyOnQueryPosition(Trade_Env_Real, nCookie, nCount, pArrPosition);
}

void CPluginUSTradeServer::OnQueryHisOrderList(UINT32 nCookie, INT32 nCount, const Trade_OrderItem* pArrOrder)
{
	m_QueryHisOrder.NotifyOnQueryUSHisOrder(Trade_Env_Real, nCookie, nCount, pArrOrder);
}

void CPluginUSTradeServer::OnQueryHisDealList(UINT32 nCookie, INT32 nCount, const Trade_DealItem* pArrDeal)
{
	m_QueryHisDeal.NotifyOnQueryUSHisDeal(Trade_Env_Real, nCookie, nCount, pArrDeal);
}

void CPluginUSTradeServer::OnDealUpdate(const Trade_DealItem& dealItem)
{
	std::vector<SOCKET> vtSock;
	const UINT64 &nOrderID = dealItem.nOrderID;
	m_SubUSOrderDeal.GetSubDealSocket(Trade_Env_Real, nOrderID, vtSock);
	for (auto iter = vtSock.begin(); iter != vtSock.end(); ++iter)
	{
		m_PushUSDeal.PushDealData(dealItem, Trade_Env_Real, *iter);
	}
}