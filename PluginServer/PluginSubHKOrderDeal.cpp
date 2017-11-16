#include "stdafx.h"
#include "PluginSubHKOrderDeal.h"
#include "PluginHKTradeServer.h"
#include "Protocol/ProtoSubHKOrderDeal.h"
#include "IManage_SecurityNum.h"
#include "CM/ca_api.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_CLEAR_SUB_INFO				356	
#define TIMER_ID_REQ_UNLOCK_TIMEROUT		357
//tomodify 2
#define PROTO_ID_QUOTE			PROTO_ID_TDHK_SUB_ORDER_DEAL
typedef CProtoSubHKOrderDeal					CProtoQuote;

//////////////////////////////////////////////////////////////////////////

CPluginSubHKOrderDeal::CPluginSubHKOrderDeal()
{	
	m_pTradeOp = NULL;
	m_pTradeServer = NULL;
	m_pOrderPusher = NULL;
	m_pDealPusher = NULL;
	m_bStartTimerClearSubInfo = false;
	m_bStartTimerReqUnlockTimeout = false;
}

CPluginSubHKOrderDeal::~CPluginSubHKOrderDeal()
{
	Uninit();
}

void CPluginSubHKOrderDeal::Init(CPluginHKTradeServer* pTradeServer, ITrade_HK* pTradeOp, CPluginPushHKOrder* pOrderPusher, CPluginPushHKDeal* pDealPusher)
{
	if ( m_pTradeServer != NULL )
		return;

	if (pTradeServer == NULL || pTradeOp == NULL || 
		pOrderPusher == NULL || pDealPusher == NULL)
	{
		ASSERT(false);
		return;
	}

	m_pTradeServer = pTradeServer;
	m_pTradeOp = pTradeOp;
	m_pOrderPusher = pOrderPusher;
	m_pDealPusher = pDealPusher;

	m_TimerWnd.SetEventInterface(this);
	m_TimerWnd.Create();

	m_MsgHandler.SetEventInterface(this);
	m_MsgHandler.Create();

	SetTimerClearSubInfo(true);
}

void CPluginSubHKOrderDeal::Uninit()
{
	if ( m_pTradeServer != NULL )
	{
		m_pTradeServer = NULL;
		m_pTradeOp = NULL;
		m_pOrderPusher = NULL;
		m_pDealPusher = NULL;

		SetTimerClearSubInfo(false);

		m_TimerWnd.Destroy();
		m_TimerWnd.SetEventInterface(NULL);

		m_MsgHandler.Close();
		m_MsgHandler.SetEventInterface(NULL);

		ClearAllReqAckData();
	}
}

void CPluginSubHKOrderDeal::SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	CHECK_RET(nCmdID == PROTO_ID_QUOTE && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pTradeOp && m_pTradeServer, NORET);

	CProtoQuote proto;
	CProtoQuote::ProtoReqDataType	req;
	proto.SetProtoData_Req(&req);
	if ( !proto.ParseJson_Req(jsnVal) )
	{
		CHECK_OP(false, NORET);
		TradeAckType ack;
		ack.head = req.head;
		ack.head.ddwErrCode = PROTO_ERR_PARAM_ERR;
		CA::Unicode2UTF(L"参数错误！", ack.head.strErrDesc);
		ack.body.nCookie = req.body.nCookie;
		HandleTradeAck(&ack, sock);
		return;
	}

	CHECK_RET(req.head.nProtoID == nCmdID && req.body.nCookie, NORET);

	if (req.body.nEnvType == Trade_Env_Real && !IManage_SecurityNum::IsSafeSocket(sock))
	{
		//
		SetTimerReqUnlockTimeout(true);
		UINT64 nReqTimestamp = time(NULL);
		m_pTradeOp->GetServerTime(nReqTimestamp);
		++m_mapReqQueueSocket[sock];
		m_vtReqQueue.push_back(make_pair(make_pair(sock, req), nReqTimestamp));
		return;
	}

	SetTradeReqData(req, sock);
}

void CPluginSubHKOrderDeal::SetTradeReqData(const TradeReqType &req, SOCKET sock)
{
	StockDataReq *pReq = new StockDataReq;
	CHECK_RET(pReq, NORET);
	pReq->sock = sock;
	pReq->dwReqTick = ::GetTickCount();
	pReq->req = req;
	pReq->dwLocalCookie = req.body.nCookie;//不经过牛牛代码，用请求cookie代替即可标识对应req

	m_vtReqData.push_back(pReq);

	const CProtoQuote::ProtoReqBodyType &body = req.body;
	bool bSubOrder = (body.nSubOrder != 0);
	bool bSubDeal = (body.nSubDeal != 0);
	bool bFirstPush = (body.nFirstPush != 0);
	Trade_Env eEnv = (Trade_Env)body.nEnvType;
	std::string strSubOrderSuc;
	std::string strSubDealSuc;

	if (body.strOrderID.empty())//全部订阅
	{
		if (bSubOrder)
		{
			m_mapSubAllOrderSocket[sock] = body.nEnvType;
		}

		if (bSubDeal)
		{
			m_mapSubAllDealSocket[sock] = body.nEnvType;
		}

		int nOrderIDCount = 0;
		if (bFirstPush && m_pTradeOp->GetOrderIDList(eEnv, NULL, nOrderIDCount))
		{
			UINT64* pOrderIDList = new UINT64[nOrderIDCount];
			if (m_pTradeOp->GetOrderIDList(eEnv, pOrderIDList, nOrderIDCount))
			{
				for (int i = 0; i < nOrderIDCount; ++i)
				{
					if (bSubOrder)
					{
						PushOrder(sock, eEnv, pOrderIDList[i]);
					}

					if (bSubDeal)
					{
						PushDeal(sock, eEnv, pOrderIDList[i]);
					}
				}
			}
		}

		//
		strSubOrderSuc = bSubOrder ? "1" : "0";
		strSubDealSuc = bSubDeal ? "1" : "0";
		NotifyOnSubHKOrder(eEnv, body.nCookie, body.strOrderID, strSubOrderSuc, strSubDealSuc);
	}
	else
	{
		std::vector<UINT64> vOrderID;
		ParseOrderIDStr(body.strOrderID, vOrderID);
		int nSucCount = 0;
		for (auto iter = vOrderID.begin(); iter != vOrderID.end(); ++iter)
		{
			UINT64 nOrderID = *iter;
			bool bRet = SubOrderDeal(sock, eEnv, nOrderID, bSubOrder, bSubDeal, bFirstPush);
			if (bRet)
			{
				++nSucCount;
			}

			strSubOrderSuc += (bRet && bSubOrder) ? "1" : "0";
			strSubDealSuc += (bRet && bSubDeal) ? "1" : "0";
			if (iter + 1 != vOrderID.end())
			{
				strSubOrderSuc += ",";
				strSubDealSuc += ",";
			}
		}

		if (nSucCount > 0)
		{
			NotifyOnSubHKOrder(eEnv, body.nCookie, body.strOrderID, strSubOrderSuc, strSubDealSuc);
		}
		else
		{
			//都不成功的情况下才返回错误包
			TradeAckType ack;
			ack.head = req.head;
			ack.head.ddwErrCode = PROTO_ERR_ORDER_NOT_FIND;
			CA::Unicode2UTF(L"未知订单！", ack.head.strErrDesc);

			memset(&ack.body, 0, sizeof(ack.body));
			ack.body.nEnvType = body.nEnvType;
			ack.body.nCookie = body.nCookie;
			ack.body.strOrderID = body.strOrderID;
			ack.body.strSubOrderSuc = strSubOrderSuc;
			ack.body.strSubDealSuc = strSubDealSuc;
			HandleTradeAck(&ack, sock);

			DoDeleteReqData(pReq);
			return;
		}
	}
}

bool CPluginSubHKOrderDeal::DoDeleteReqData(StockDataReq* pReq)
{
	VT_REQ_TRADE_DATA::iterator it = m_vtReqData.begin();
	while (it != m_vtReqData.end())
	{
		if (*it  == pReq)
		{ 
			SAFE_DELETE(pReq);
			it = m_vtReqData.erase(it);
			return true; 
		}
		++it;
	}
	return false;
}

void CPluginSubHKOrderDeal::NotifyOnSubHKOrder(Trade_Env enEnv, UINT32 nCookie, std::string strOrderID,
	std::string strSubOrderSuc, std::string strSubDealSuc)
{
	CHECK_RET(nCookie, NORET);
	CHECK_RET(m_pTradeOp && m_pTradeServer, NORET);

	VT_REQ_TRADE_DATA::iterator itReq = m_vtReqData.begin();
	StockDataReq *pFindReq = NULL;
	for ( ; itReq != m_vtReqData.end(); ++itReq )
	{
		StockDataReq *pReq = *itReq;
		CHECK_OP(pReq, continue);
		if ( pReq->dwLocalCookie == nCookie )
		{
			pFindReq = pReq;
			break;
		}
	}
	if (!pFindReq)
		return;

	TradeAckType ack;
	ack.head = pFindReq->req.head;
	ack.head.ddwErrCode = PROTO_ERR_NO_ERROR;

	//tomodify 4
	ack.body.nEnvType = enEnv;
	ack.body.nCookie = pFindReq->req.body.nCookie;
	ack.body.strOrderID = strOrderID;
	ack.body.strSubOrderSuc = strSubOrderSuc;
	ack.body.strSubDealSuc = strSubDealSuc;
	HandleTradeAck(&ack, pFindReq->sock);

	m_vtReqData.erase(itReq);
	delete pFindReq;
}

void CPluginSubHKOrderDeal::NotifySocketClosed(SOCKET sock)
{
	DoClearStocketInfo(sock);
}

void CPluginSubHKOrderDeal::ClearSubOrderInfo(UINT64 nOrderID)
{
	SET_ORDER_SOCKET& setOrder = m_mapSubOrder;
	auto itOrder = setOrder.begin();
	while (itOrder != setOrder.end())
	{
		if (itOrder->first == nOrderID)
		{
			itOrder = setOrder.erase(itOrder);
		}
		else
		{
			++itOrder;
		}
	}

	if (!IsSubDeal(nOrderID))
	{
		ClearOrderEnvInfo(nOrderID);
	}
}

void CPluginSubHKOrderDeal::ClearSubDealInfo(UINT64 nOrderID)
{
	SET_DEAL_SOCKET& setDeal = m_mapSubDeal;
	auto itDeal = setDeal.begin();
	while (itDeal != setDeal.end())
	{
		if (itDeal->first == nOrderID)
		{
			itDeal = setDeal.erase(itDeal);
		}
		else
		{
			++itDeal;
		}
	}

	if (!IsSubOrder(nOrderID))
	{
		ClearOrderEnvInfo(nOrderID);
	}
}

void CPluginSubHKOrderDeal::GetSubOrderSocket(Trade_Env enEnv, UINT64 nOrderID, std::vector<SOCKET> &vtSock)
{
	vtSock.clear();
	if (!m_mapSubAllOrderSocket.empty())
	{
		for (auto iter = m_mapSubAllOrderSocket.begin();
			iter != m_mapSubAllOrderSocket.end(); ++iter)
		{
			if ((int)enEnv == iter->second)
			{
				vtSock.push_back(iter->first);
			}			
		}
	}
	
	SET_ORDER_SOCKET& setOrder = m_mapSubOrder;
	auto itOrder = setOrder.begin();
	while (itOrder != setOrder.end())
	{
		if (itOrder->first == nOrderID)
		{
			if (std::find(vtSock.begin(), vtSock.end(), itOrder->second) == vtSock.end())
			{
				vtSock.push_back(itOrder->second);
			}
		}
		++itOrder;
	}		
}

void CPluginSubHKOrderDeal::GetSubDealSocket(Trade_Env enEnv, UINT64 nOrderID, std::vector<SOCKET> &vtSock)
{
	vtSock.clear();
	if (!m_mapSubAllDealSocket.empty())
	{
		for (auto iter = m_mapSubAllDealSocket.begin();
			iter != m_mapSubAllDealSocket.end(); ++iter)
		{
			if ((int)enEnv == iter->second)
			{
				vtSock.push_back(iter->first);
			}
		}
	}
	
	SET_DEAL_SOCKET& setDeal = m_mapSubDeal;
	auto itDeal = setDeal.begin();
	while (itDeal != setDeal.end())
	{
		if (itDeal->first == nOrderID)
		{
			if (std::find(vtSock.begin(), vtSock.end(), itDeal->second) == vtSock.end())
			{
				vtSock.push_back(itDeal->second);
			}
		}
		++itDeal;
	}	
}

void CPluginSubHKOrderDeal::NotifyUnLockTrade(SOCKET sock, Trade_SvrResult enSvrRet)
{
	if (m_mapReqQueueSocket.count(sock) == 0)
	{
		//别的连接解锁回调，不处理
		return;
	}

	//
	if (enSvrRet == Trade_SvrResult_Succeed)
	{
		//该连接解锁成功，将该连接上面的请求全部回包
		m_mapReqQueueSocket.erase(sock);
	}
	else
	{
		//解锁失败，继续等待，可能还有下次解锁能够成功，直到超时
		return;
	}

	VT_REQ_QUEUE vtHandle;
	VT_REQ_QUEUE vtLast;

	for (auto iter = m_vtReqQueue.begin(); iter != m_vtReqQueue.end(); ++iter)
	{
		if (sock == iter->first.first)
		{
			vtHandle.push_back(*iter);
		}
		else
		{
			vtLast.push_back(*iter);
		}
	}
	//处理该连接上面的请求，其他请求保留继续等待
	std::swap(vtLast, m_vtReqQueue);
	if (m_vtReqQueue.empty())
	{
		SetTimerReqUnlockTimeout(false);
	}

	if (enSvrRet == Trade_SvrResult_Succeed)
	{
		for (auto iter = vtHandle.begin(); iter != vtHandle.end(); ++iter)
		{
			SOCKET &sock = iter->first.first;
			TradeReqType &req = iter->first.second;

			//
			SetTradeReqData(req, sock);
		}
	}
}

void CPluginSubHKOrderDeal::OnTimeEvent(UINT nEventID)
{
	if(TIMER_ID_CLEAR_SUB_INFO == nEventID)
	{
		HandleTimerClearSubInfo();
	}
	else if(TIMER_ID_REQ_UNLOCK_TIMEROUT == nEventID)
	{
		HandleReqUnlockTimeout();
	}
}

void CPluginSubHKOrderDeal::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{

}

void CPluginSubHKOrderDeal::HandleTradeAck(TradeAckType *pAck, SOCKET sock)
{
	CHECK_RET(pAck && pAck->body.nCookie && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pTradeServer, NORET);

	CProtoQuote proto;
	proto.SetProtoData_Ack(pAck);

	Json::Value jsnValue;
	bool bRet = proto.MakeJson_Ack(jsnValue);
	CHECK_RET(bRet, NORET);
	
	std::string strBuf;
	CProtoParseBase::ConvJson2String(jsnValue, strBuf, true);
	m_pTradeServer->ReplyTradeReq(PROTO_ID_QUOTE, strBuf.c_str(), (int)strBuf.size(), sock);
}
void CPluginSubHKOrderDeal::HandleTimerClearSubInfo()
{
	CHECK_RET(m_pTradeOp, NORET);
	std::vector<UINT64> vtEndStatusOrder;
	MAP_ORDER_ENVTYPE::iterator iter = m_mapEnvType.begin();
	UINT64 nSrvTimeStamp = time(NULL);
	m_pTradeOp->GetServerTime(nSrvTimeStamp);
	for (; iter != m_mapEnvType.end(); ++iter)
	{
		Trade_OrderItem orderItem = {};
		bool bRet = m_pTradeOp->GetOrderItem((Trade_Env)iter->second, iter->first, orderItem);
		if (!bRet || IsHKOrderFinalStatus(orderItem.nStatus))
		{
			if (nSrvTimeStamp - orderItem.nUpdatedTime >= 60)
			{
				vtEndStatusOrder.push_back(iter->first);
			}
		}
	}

	for (std::vector<UINT64>::iterator iter = vtEndStatusOrder.begin(); iter != vtEndStatusOrder.end(); ++iter)
	{
		ClearSubOrderInfo(*iter);
		ClearSubDealInfo(*iter);
	}
}

void CPluginSubHKOrderDeal::SetTimerClearSubInfo(bool bStartOrStop)
{
	if (m_bStartTimerClearSubInfo)
	{
		if (!bStartOrStop)
		{
			m_TimerWnd.StopTimer(TIMER_ID_CLEAR_SUB_INFO);
			m_bStartTimerClearSubInfo = false;
		}
	}
	else
	{
		if (bStartOrStop)
		{
			m_TimerWnd.StartTimer(30, TIMER_ID_CLEAR_SUB_INFO);
			m_bStartTimerClearSubInfo = true;
		}
	}
}

void CPluginSubHKOrderDeal::HandleReqUnlockTimeout()
{
	CHECK_RET(m_pTradeOp, NORET);
	UINT64 nSrvTimeStamp = time(NULL);
	m_pTradeOp->GetServerTime(nSrvTimeStamp);

	for (auto iter = m_vtReqQueue.begin(); iter != m_vtReqQueue.end();)
	{
		//请求超过3秒（用时间戳判断，而且500ms检查一次，稍有误差）
		if (nSrvTimeStamp - iter->second >= 3)
		{
			SOCKET &sock = iter->first.first;
			TradeReqType &req = iter->first.second;

			TradeAckType ack;
			ack.head = req.head;
			ack.head.ddwErrCode = PROTO_ERR_UNKNOWN_ERROR;
			CA::Unicode2UTF(L"请重新解锁！", ack.head.strErrDesc);
			ack.body.nCookie = req.body.nCookie;
			ack.body.nEnvType = req.body.nEnvType;
			HandleTradeAck(&ack, sock);

			iter = m_vtReqQueue.erase(iter);

			if (m_mapReqQueueSocket.count(sock) != 0)
			{
				--m_mapReqQueueSocket[sock];
				if (m_mapReqQueueSocket[sock] <= 0)
				{
					m_mapReqQueueSocket.erase(sock);
				}
			}
		}
		else
		{
			++iter;
		}		
	}

	if (m_vtReqQueue.empty())
	{
		SetTimerReqUnlockTimeout(false);
	}
}

void CPluginSubHKOrderDeal::SetTimerReqUnlockTimeout(bool bStartOrStop)
{
	if (m_bStartTimerReqUnlockTimeout)
	{
		if (!bStartOrStop)
		{
			m_TimerWnd.StopTimer(TIMER_ID_REQ_UNLOCK_TIMEROUT);
			m_bStartTimerReqUnlockTimeout = false;
		}
	}
	else
	{
		if (bStartOrStop)
		{
			m_TimerWnd.StartMillionTimer(500, TIMER_ID_REQ_UNLOCK_TIMEROUT);
			m_bStartTimerReqUnlockTimeout = true;
		}
	}
}

void CPluginSubHKOrderDeal::ClearAllReqAckData()
{
	VT_REQ_TRADE_DATA::iterator it_req = m_vtReqData.begin();
	for ( ; it_req != m_vtReqData.end(); )
	{
		StockDataReq *pReq = *it_req;
		delete pReq;
	}

	m_vtReqData.clear();
}

void CPluginSubHKOrderDeal::DoClearStocketInfo(SOCKET socket)
{
	VT_REQ_TRADE_DATA& vtReq = m_vtReqData;

	//清掉socket对应的请求信息
	auto itReq = vtReq.begin();
	while (itReq != vtReq.end())
	{
		if (*itReq && (*itReq)->sock == socket)
		{
			delete *itReq;
			itReq = vtReq.erase(itReq);
		}
		else
		{
			++itReq;
		}
	}

	//清理记录信息
	SET_ORDER_SOCKET& setOrder = m_mapSubOrder;
	auto itOrder = setOrder.begin();
	while (itOrder != setOrder.end())
	{
		if (itOrder->second == socket)
		{
			itOrder = setOrder.erase(itOrder);
		}
		else
		{
			++itOrder;
		}
	}

	SET_DEAL_SOCKET& setDeal = m_mapSubDeal;
	auto itDeal = setDeal.begin();
	while (itDeal != setDeal.end())
	{
		if (itDeal->second == socket)
		{
			itDeal = setDeal.erase(itDeal);
		}
		else
		{
			++itDeal;
		}
	}

	m_mapSubAllOrderSocket.erase(socket);
	m_mapSubAllDealSocket.erase(socket);
}

bool CPluginSubHKOrderDeal::IsSubOrder(UINT64 nOrderID)
{
	SET_ORDER_SOCKET& setOrder = m_mapSubOrder;
	auto itOrder = setOrder.begin();
	while (itOrder != setOrder.end())
	{
		if (itOrder->first == nOrderID)
		{
			return true;
		}

		++itOrder;
	}
	return false;
}

bool CPluginSubHKOrderDeal::IsSubDeal(UINT64 nOrderID)
{
	SET_DEAL_SOCKET& setDeal = m_mapSubDeal;
	auto itDeal = setDeal.begin();
	while (itDeal != setDeal.end())
	{
		if (itDeal->first == nOrderID)
		{
			return true;
		}

		++itDeal;
	}
	return false;
}

void CPluginSubHKOrderDeal::ClearOrderEnvInfo(UINT64 nOrderID)
{
	MAP_ORDER_ENVTYPE& mapEnv = m_mapEnvType;
	auto itEnv = mapEnv.begin();
	while (itEnv != mapEnv.end())
	{
		if (itEnv->first == nOrderID)
		{

			itEnv = mapEnv.erase(itEnv);
		}
		else
		{
			++itEnv;
		}
	}
}

void CPluginSubHKOrderDeal::ParseOrderIDStr(std::string strOrderID, std::vector<UINT64> &vOrderID)
{
	vOrderID.clear();
	CString strDiv = _T(",");
	std::vector<CString> arOrderIDStr;
	CA::DivStr(CString(strOrderID.c_str()), strDiv, arOrderIDStr);
	for (UINT i = 0; i < arOrderIDStr.size(); i++)
	{
		vOrderID.push_back(_ttoi64(arOrderIDStr[i]));
	}
}

bool CPluginSubHKOrderDeal::SubOrderDeal(SOCKET sock, Trade_Env enEnv, UINT64 nOrderID, bool bSubOrder, bool bSubDeal, bool bFirstPush)
{	
	Trade_OrderStatus eOrderStatus = Trade_OrderStatus(0);
	bool bRet = m_pTradeOp->GetOrderStatus(enEnv, nOrderID, eOrderStatus);

	if (!bRet)
	{
		return false;
	}

	if (bSubOrder || bSubDeal)
	{
		m_mapEnvType[nOrderID] = enEnv;
	}

	std::pair<UINT64, SOCKET> SubOrder = make_pair(nOrderID, sock);
	if (bSubOrder && m_mapSubOrder.count(SubOrder) == 0)
	{
		m_mapSubOrder.insert(SubOrder);
		if (bFirstPush)
		{
			PushOrder(sock, enEnv, nOrderID);
		}
	}

	if (bSubDeal && m_mapSubDeal.count(SubOrder) == 0)
	{
		m_mapSubDeal.insert(SubOrder);
		if (bFirstPush)
		{
			PushDeal(sock, enEnv, nOrderID);
		}
	}

	return bRet;
}

void CPluginSubHKOrderDeal::PushOrder(SOCKET sock, Trade_Env enEnv, UINT64 nOrderID)
{
	CHECK_RET(m_pTradeOp && m_pOrderPusher, NORET);

	//订阅首推订单状态
	Trade_OrderItem orderItem = {};
	bool bGetOrderItm = m_pTradeOp->GetOrderItem(enEnv, nOrderID, orderItem);
	if (m_pOrderPusher && bGetOrderItm)
	{
		m_pOrderPusher->PushOrderData(orderItem, enEnv, sock);
	}
}

void CPluginSubHKOrderDeal::PushDeal(SOCKET sock, Trade_Env enEnv, UINT64 nOrderID)
{
	CHECK_RET(m_pTradeOp && m_pOrderPusher, NORET);

	//订阅首推该订单之前成交记录
	int nCount = 0;
	m_pTradeOp->GetDealItemsByOrderID(enEnv, nOrderID, NULL, nCount);
	if (nCount > 0)
	{
		Trade_DealItem* pDealItems = new Trade_DealItem[nCount];
		CHECK_RET(pDealItems, NORET);
		if (m_pTradeOp->GetDealItemsByOrderID(enEnv, nOrderID, &pDealItems, nCount))
		{
			for (int i = 0; i < nCount; ++i)
			{
				m_pDealPusher->PushDealData(pDealItems[i], enEnv, sock);
			}
		}
		SAFE_DELETE_ARR(pDealItems);
	}
}