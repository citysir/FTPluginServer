#include "stdafx.h"
#include "PluginQueryHKOrder.h"
#include "PluginHKTradeServer.h"
#include "Protocol/ProtoQueryHKOrder.h"
#include "IManage_SecurityNum.h"
#include "CM/ca_api.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_HANDLE_TIMEOUT_REQ	355
#define EVENT_ID_ACK_REQUEST		368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_TDHK_QUERY_ORDER
typedef CProtoQueryHKOrder	CProtoQuote;

//////////////////////////////////////////////////////////////////////////

CPluginQueryHKOrder::CPluginQueryHKOrder()
{	
	m_pTradeOp = NULL;
	m_pTradeServer = NULL;
	m_bStartTimerHandleTimeout = FALSE;
}

CPluginQueryHKOrder::~CPluginQueryHKOrder()
{
	Uninit();
}

void CPluginQueryHKOrder::Init(CPluginHKTradeServer* pTradeServer, ITrade_HK*  pTradeOp)
{
	if ( m_pTradeServer != NULL )
		return;

	if ( pTradeServer == NULL || pTradeOp == NULL )
	{
		ASSERT(false);
		return;
	}

	m_pTradeServer = pTradeServer;
	m_pTradeOp = pTradeOp;
	m_TimerWnd.SetEventInterface(this);
	m_TimerWnd.Create();

	m_MsgHandler.SetEventInterface(this);
	m_MsgHandler.Create();
}

void CPluginQueryHKOrder::Uninit()
{
	if ( m_pTradeServer != NULL )
	{
		m_pTradeServer = NULL;
		m_pTradeOp = NULL;

		m_TimerWnd.Destroy();
		m_TimerWnd.SetEventInterface(NULL);

		m_MsgHandler.Close();
		m_MsgHandler.SetEventInterface(NULL);

		ClearAllReqAckData();
	}
}

void CPluginQueryHKOrder::SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
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

	if (req.body.nEnvType == Trade_Env_Real && !IManage_SecurityNum::IsSafeSocket(sock))
	{
		CHECK_OP(false, NORET);
		TradeAckType ack;
		ack.head = req.head;
		ack.head.ddwErrCode = PROTO_ERR_UNKNOWN_ERROR;
		CA::Unicode2UTF(L"请重新解锁！", ack.head.strErrDesc);
		ack.body.nCookie = req.body.nCookie;
		HandleTradeAck(&ack, sock);
		return;
	}

	CHECK_RET(req.head.nProtoID == nCmdID && req.body.nCookie, NORET);

	StockDataReq *pReq = new StockDataReq;
	CHECK_RET(pReq, NORET);
	pReq->sock = sock;
	pReq->dwReqTick = ::GetTickCount();
	pReq->req = req;	
	
	m_vtReqData.push_back(pReq);

	//tomodify 3
	QueryHKOrderReqBody &body = req.body;	
	wstring strStartTime, strEndTime;
	CA::UTF2Unicode(body.strStartTime.c_str(), strStartTime);
	CA::UTF2Unicode(body.strEndTime.c_str(), strEndTime);
	bool bRet = m_pTradeOp->QueryOrderList((Trade_Env)body.nEnvType, (UINT32*)&pReq->dwLocalCookie, strStartTime.c_str(), strEndTime.c_str());

	if ( !bRet )
	{
		TradeAckType ack;
		ack.head = req.head;
		ack.head.ddwErrCode = PROTO_ERR_UNKNOWN_ERROR;
		CA::Unicode2UTF(L"发送失败", ack.head.strErrDesc);

		memset(&ack.body, 0, sizeof(ack.body));
		ack.body.nEnvType = body.nEnvType;
		ack.body.nCookie = body.nCookie;		
		HandleTradeAck(&ack, sock);

		DoDeleteReqData(pReq);

		return ;
	}
	SetTimerHandleTimeout(true);
}

bool CPluginQueryHKOrder::DoDeleteReqData(StockDataReq* pReq)
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

void CPluginQueryHKOrder::NotifyOnQueryHKOrder(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_OrderItem* pArrOrder)
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
// 	if ( nErrCode )
// 	{
// 		WCHAR szErr[256] = L"";
// 		if ( m_pTradeOp->GetErrDescV2(nErrCode, szErr) )
// 			CA::Unicode2UTF(szErr, ack.head.strErrDesc);
// 	}

	//tomodify 4
	ack.body.nEnvType = enEnv;
	ack.body.nCookie = pFindReq->req.body.nCookie;

	std::set<int> setStatus;
	std::set<std::wstring> setCode;
	INT64 nFilterOrderID = pFindReq->req.body.nOrderID;
	DoGetFilterStatus(pFindReq->req.body.strStatusFilter, setStatus);
	DoGetFilterCode(pFindReq->req.body.strStockCode, setCode);

	if ( nCount > 0 && pArrOrder )
	{
		for ( int n = 0; n < nCount; n++ )
		{			
			const Trade_OrderItem &order = pArrOrder[n];
			QueryHKOrderAckItem item;
			item.nLocalID = order.nLocalID;
			item.nOrderID = order.nOrderID;
			item.nOrderType = order.nType;
			item.enSide = order.enSide;
			item.nStatus = order.nStatus;
			item.strStockCode = order.szCode;
			item.strStockName = order.szName;
			item.nPrice = order.nPrice;
			item.nQty = order.nQty;
			item.nDealtQty = order.nDealtQty;
			item.nDealtAvgPrice = int(round(order.fDealtAvgPrice * 1000));
			item.nSubmitedTime = order.nSubmitedTime;
			item.nUpdatedTime = order.nUpdatedTime;
			item.nErrCode = order.nErrCode;

			if (IsFitFilter(item, nFilterOrderID, setStatus, setCode))
			{
				ack.body.vtOrder.push_back(item);
			}
		}
	}	 
	
	HandleTradeAck(&ack, pFindReq->sock);

	m_vtReqData.erase(itReq);
	delete pFindReq;
}

void CPluginQueryHKOrder::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginQueryHKOrder::OnTimeEvent(UINT nEventID)
{
	if ( TIMER_ID_HANDLE_TIMEOUT_REQ == nEventID )
	{
		HandleTimeoutReq();
	}
}

void CPluginQueryHKOrder::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{		
	}	
}

void CPluginQueryHKOrder::HandleTimeoutReq()
{
	if ( m_vtReqData.empty() )
	{
		SetTimerHandleTimeout(false);
		return;
	}

	DWORD dwTickNow = ::GetTickCount();	
	VT_REQ_TRADE_DATA::iterator it_req = m_vtReqData.begin();
	for ( ; it_req != m_vtReqData.end(); )
	{
		StockDataReq *pReq = *it_req;	
		if ( pReq == NULL )
		{
			CHECK_OP(false, NOOP);
			++it_req;
			continue;
		}		

		if ( int(dwTickNow - pReq->dwReqTick) > 8000 )
		{
			TradeAckType ack;
			ack.head = pReq->req.head;
			ack.head.ddwErrCode= PROTO_ERR_SERVER_TIMEROUT;
			CA::Unicode2UTF(L"协议超时", ack.head.strErrDesc);

			//tomodify 5
			memset(&ack.body, 0, sizeof(ack.body));
			ack.body.nEnvType = pReq->req.body.nEnvType;
			ack.body.nCookie = pReq->req.body.nCookie;			
			
			HandleTradeAck(&ack, pReq->sock);
			
			it_req = m_vtReqData.erase(it_req);
			delete pReq;
		}
		else
		{
			++it_req;
		}
	}

	if ( m_vtReqData.empty() )
	{
		SetTimerHandleTimeout(false);
		return;
	}
}

void CPluginQueryHKOrder::HandleTradeAck(TradeAckType *pAck, SOCKET sock)
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

void CPluginQueryHKOrder::SetTimerHandleTimeout(bool bStartOrStop)
{
	if ( m_bStartTimerHandleTimeout )
	{
		if ( !bStartOrStop )
		{			
			m_TimerWnd.StopTimer(TIMER_ID_HANDLE_TIMEOUT_REQ);
			m_bStartTimerHandleTimeout = FALSE;
		}
	}
	else
	{
		if ( bStartOrStop )
		{
			m_TimerWnd.StartMillionTimer(500, TIMER_ID_HANDLE_TIMEOUT_REQ);
			m_bStartTimerHandleTimeout = TRUE;
		}
	}
}

void CPluginQueryHKOrder::ClearAllReqAckData()
{
	VT_REQ_TRADE_DATA::iterator it_req = m_vtReqData.begin();
	for ( ; it_req != m_vtReqData.end(); )
	{
		StockDataReq *pReq = *it_req;
		delete pReq;
	}

	m_vtReqData.clear();
}

void CPluginQueryHKOrder::DoClearReqInfo(SOCKET socket)
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
}

void CPluginQueryHKOrder::DoGetFilterStatus(const std::string& strFilter, std::set<int>& setStatus)
{
	setStatus.clear();
	CString strDiv = _T(",");
	std::vector<CString> arFilterStr;
	CA::DivStr(CString(strFilter.c_str()), strDiv, arFilterStr);
	for (UINT i = 0; i < arFilterStr.size(); i++)
	{
		int nTmp = _ttoi(arFilterStr[i]);

		setStatus.insert(nTmp);
	}
}

void CPluginQueryHKOrder::DoGetFilterCode(const std::string& strFilter, std::set<std::wstring>& setCode)
{
	setCode.clear();
	if (strFilter.empty())
	{
		return;
	}
	setCode.insert(L"");//防止当作没有限制,添加空不影响筛选结果

	CString strDiv = _T(",");
	std::vector<CString> vtFilterStr;
	std::wstring wstrFilter;
	CA::UTF2Unicode(strFilter.c_str(), wstrFilter);
	CA::DivStr(wstrFilter.c_str(), strDiv, vtFilterStr);
	for (auto iter = vtFilterStr.begin(); iter != vtFilterStr.end(); ++iter)
	{
		iter->TrimLeft();
		iter->TrimRight();

		std::wstring wstrCode = iter->GetString();
		setCode.insert(wstrCode);
	}
}
 
bool CPluginQueryHKOrder::IsFitFilter(const TradeAckItemType& AckItem, INT64 nOrderID,
	const std::set<int>& vtStatus, const std::set<std::wstring>& setCode)
{
	//&&关系
	if (nOrderID != 0 && AckItem.nOrderID != nOrderID)
	{
		return false;
	}

	if (!vtStatus.empty() && vtStatus.count(AckItem.nStatus) == 0)
	{
		return false;
	}

	if (!setCode.empty() && setCode.count(AckItem.strStockCode) == 0)
	{
		return false;
	}

	return true;
}