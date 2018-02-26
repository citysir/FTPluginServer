#include "stdafx.h"
#include "PluginHKTradeServer.h"
#include "PluginNetwork.h"
#include "Protocol/ProtoOrderErrorPush.h"
#include "Protocol/ProtoBasicPrice.h"
#include "IManage_SecurityNum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern GUID PLUGIN_GUID;

//////////////////////////////////////////////////////////////////////////

CPluginHKTradeServer::CPluginHKTradeServer()
{
	m_pPluginCore = NULL;
	m_pTradeOp = NULL;	
	m_pNetwork = NULL;
	//IManage_SecurityNum::Init();
}

CPluginHKTradeServer::~CPluginHKTradeServer()
{
	UninitTradeSvr();
}

void CPluginHKTradeServer::InitTradeSvr(IFTPluginCore* pPluginCore, CPluginNetwork *pNetwork)
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
	pPluginCore->QueryFTInterface(IID_IFTTrade_HK, (void**)&m_pTradeOp);
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

	m_PlaceOrder.Init(this, m_pTradeOp, pQuoteData);
	m_ChangeOrder.Init(this, m_pTradeOp);
	m_SetOrderStatus.Init(this, m_pTradeOp);
	m_UnlockTrade.Init(this, m_pTradeOp);
	m_QueryAccInfo.Init(this, m_pTradeOp);
	m_QueryHKOrder.Init(this, m_pTradeOp);
	m_QueryHKPos.Init(this, m_pTradeOp, pQuoteData);
	m_QueryHKDeal.Init(this, m_pTradeOp);
	m_QueryHKHisOrder.Init(this, m_pTradeOp);
	m_QueryHKHisDeal.Init(this, m_pTradeOp);
	m_PushHKOrder.Init(this, m_pTradeOp);
	m_PushHKDeal.Init(this, m_pTradeOp);
	m_SubHKOrderDeal.Init(this, m_pTradeOp, &m_PushHKOrder, &m_PushHKDeal);
	
}

void CPluginHKTradeServer::UninitTradeSvr()
{
	if ( m_pPluginCore != NULL )
	{
		m_QueryHKPos.Uninit();
		m_QueryHKOrder.Uninit();
		m_QueryAccInfo.Uninit();
		m_UnlockTrade.Uninit();
		m_PlaceOrder.Uninit();
		m_ChangeOrder.Uninit();
		m_SetOrderStatus.Uninit();
		m_QueryHKDeal.Uninit();
		m_QueryHKHisOrder.Uninit();
		m_QueryHKHisDeal.Uninit();
		m_PushHKOrder.Uninit();
		m_PushHKDeal.Uninit();
		m_SubHKOrderDeal.Uninit();		

		m_pTradeOp = NULL;
		m_pDataReport = NULL;
		m_pPluginCore = NULL;
		m_pNetwork = NULL;
	}	
}

void CPluginHKTradeServer::SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	m_pDataReport->PLSCmdIDReport(PLUGIN_GUID, nCmdID);
	switch (nCmdID)
	{
	case PROTO_ID_TDHK_UNLOCK_TRADE:
		m_UnlockTrade.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_QUERY_ACC_INFO:
		m_QueryAccInfo.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_QUERY_ORDER:
		m_QueryHKOrder.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_QUERY_POSITION:
		m_QueryHKPos.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_PLACE_ORDER:
		m_PlaceOrder.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_SET_ORDER_STATUS:
		m_SetOrderStatus.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_CHANGE_ORDER:
		m_ChangeOrder.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_QUERY_DEAL:
		m_QueryHKDeal.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_QUERY_HIS_ORDER:
		m_QueryHKHisOrder.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_QUERY_HIS_DEAL:
		m_QueryHKHisDeal.SetTradeReqData(nCmdID, jsnVal, sock);
		break;

	case PROTO_ID_TDHK_SUB_ORDER_DEAL:
		m_SubHKOrderDeal.SetTradeReqData(nCmdID, jsnVal, sock);
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

void CPluginHKTradeServer::ReplyTradeReq(int nCmdID, const char *pBuf, int nLen, SOCKET sock)
{
	CHECK_RET(nCmdID && pBuf && nLen && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pNetwork, NORET);
	m_pNetwork->SendData(sock, pBuf, nLen);
}

void CPluginHKTradeServer::CloseSocket(SOCKET sock)
{
	m_PlaceOrder.NotifySocketClosed(sock);
	m_ChangeOrder.NotifySocketClosed(sock);
	m_SetOrderStatus.NotifySocketClosed(sock);

	m_UnlockTrade.NotifySocketClosed(sock);
	m_QueryAccInfo.NotifySocketClosed(sock);
	m_QueryHKOrder.NotifySocketClosed(sock);

	m_QueryHKPos.NotifySocketClosed(sock);
	m_QueryHKDeal.NotifySocketClosed(sock);

	m_QueryHKHisOrder.NotifySocketClosed(sock);
	m_QueryHKHisDeal.NotifySocketClosed(sock);

	m_PushHKDeal.NotifySocketClosed(sock);
	m_PushHKOrder.NotifySocketClosed(sock);
	m_SubHKOrderDeal.NotifySocketClosed(sock);

}

void CPluginHKTradeServer::OnUnlockTrade(UINT32 nCookie, Trade_SvrResult enSvrRet, UINT64 nErrCode)
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

	m_UnlockTrade.NotifyOnUnlockTrade(nCookie, enSvrRet, nErrCode);
	m_SubHKOrderDeal.NotifyUnLockTrade(sock, enSvrRet);
}

void CPluginHKTradeServer::OnQueryOrderList(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_OrderItem* pArrOrder)
{
	m_QueryHKOrder.NotifyOnQueryHKOrder(enEnv, nCookie, nCount, pArrOrder);
}

void CPluginHKTradeServer::OnQueryDealList(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_DealItem* pArrOrder)
{
	m_QueryHKDeal.NotifyOnQueryHKDeal(enEnv, nCookie, nCount, pArrOrder);
}

void CPluginHKTradeServer::OnQueryPositionList(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_PositionItem* pArrPosition)
{
	m_QueryHKPos.NotifyOnQueryPosition(enEnv, nCookie, nCount, pArrPosition);
}

void CPluginHKTradeServer::OnQueryHisOrderList(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_OrderItem* pArrOrder)
{
	m_QueryHKHisOrder.NotifyOnQueryHKHisOrder(enEnv, nCookie, nCount, pArrOrder);
}

void CPluginHKTradeServer::OnQueryHisDealList(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_DealItem* pArrDeal)
{
	m_QueryHKHisDeal.NotifyOnQueryHKHisDeal(enEnv, nCookie, nCount, pArrDeal);
}

void CPluginHKTradeServer::OnQueryAccInfo(Trade_Env enEnv, UINT32 nCookie, const Trade_AccInfo& accInfo, int nResult)
{
	m_QueryAccInfo.NotifyOnQueryHKAccInfo(enEnv, nCookie, accInfo, nResult);
}

void CPluginHKTradeServer::OnPlaceOrder(Trade_Env enEnv, UINT nCookie, Trade_SvrResult enSvrRet, UINT64 nLocalID, UINT16 nErrCode)
{
	m_PlaceOrder.NotifyOnPlaceOrder(enEnv, nCookie, enSvrRet, nLocalID, nErrCode);
}

void CPluginHKTradeServer::OnOrderUpdate(Trade_Env enEnv, const Trade_OrderItem& orderItem)
{ 
	std::vector<SOCKET> vtSock;
	const UINT64 &nOrderID = orderItem.nOrderID;
	m_SubHKOrderDeal.GetSubOrderSocket(enEnv, nOrderID, vtSock);
	for (auto iter = vtSock.begin(); iter != vtSock.end(); ++iter)
	{
		m_PushHKOrder.PushOrderData(orderItem, (int)enEnv, *iter);
	}

	if (IsHKOrderFinalStatus(orderItem.nStatus))
 	{
		m_SubHKOrderDeal.ClearSubOrderInfo(nOrderID);
 	}
}

void CPluginHKTradeServer::OnSetOrderStatus(Trade_Env enEnv, UINT nCookie, Trade_SvrResult enSvrRet, UINT64 nOrderID, UINT16 nErrCode)
{
	m_SetOrderStatus.NotifyOnSetOrderStatus(enEnv, nCookie, enSvrRet, nOrderID, nErrCode);
}

void CPluginHKTradeServer::OnChangeOrder(Trade_Env enEnv, UINT nCookie, Trade_SvrResult enSvrRet, UINT64 nOrderID, UINT16 nErrCode)
{
	m_ChangeOrder.NotifyOnChangeOrder(enEnv, nCookie, enSvrRet, nOrderID, nErrCode);
}

void CPluginHKTradeServer::OnOrderErrNotify(Trade_Env enEnv, UINT64 nOrderID, Trade_OrderErrNotify_HK enErrNotify, UINT16 nErrCode)
{
	 
}

void CPluginHKTradeServer::OnDealUpdate(Trade_Env enEnv, const Trade_DealItem& dealItem)
{
	std::vector<SOCKET> vtSock;
	const UINT64 &nOrderID = dealItem.nOrderID;
	m_SubHKOrderDeal.GetSubDealSocket(enEnv, nOrderID, vtSock);
	for (auto iter = vtSock.begin(); iter != vtSock.end(); ++iter)
	{
		m_PushHKDeal.PushDealData(dealItem, (int)enEnv, *iter);
	}
}

