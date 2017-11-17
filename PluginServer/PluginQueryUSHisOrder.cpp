#include "stdafx.h"
#include "PluginQueryUSHisOrder.h"
#include "PluginUSTradeServer.h"
#include "Protocol/ProtoQueryUSHisOrder.h"
#include "IManage_SecurityNum.h"
#include "CM/ca_api.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_HANDLE_TIMEOUT_REQ	355
#define EVENT_ID_ACK_REQUEST		368

//tomodify 2
#define PROTO_ID_QUOTE			PROTO_ID_TDUS_QUERY_HIS_ORDER
typedef CProtoQueryUSHisOrder	CProtoQuote;

//////////////////////////////////////////////////////////////////////////
CPluginQueryUSHisOrder::CPluginQueryUSHisOrder()
{	
	m_pTradeOp = NULL;
	m_pTradeServer = NULL;
	m_bStartTimerHandleTimeout = FALSE;
}

CPluginQueryUSHisOrder::~CPluginQueryUSHisOrder()
{
	Uninit();
}

void CPluginQueryUSHisOrder::Init(CPluginUSTradeServer* pTradeServer, ITrade_US*  pTradeOp)
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

void CPluginQueryUSHisOrder::Uninit()
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

void CPluginQueryUSHisOrder::SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
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
		CA::Unicode2UTF(L"��������", ack.head.strErrDesc);
		ack.body.nCookie = req.body.nCookie;
		HandleTradeAck(&ack, sock);
		return;
	}

	if (!IManage_SecurityNum::IsSafeSocket(sock))
	{
		CHECK_OP(false, NORET);
		TradeAckType ack;
		ack.head = req.head;
		ack.head.ddwErrCode = PROTO_ERR_UNKNOWN_ERROR;
		CA::Unicode2UTF(L"�����½�����", ack.head.strErrDesc);
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
	QueryUSHisOrderReqBody &body = req.body;	
	wstring strStartDate, strEndDate;
	CA::UTF2Unicode(body.strStartDate.c_str(), strStartDate);
	CA::UTF2Unicode(body.strEndDate.c_str(), strEndDate);
	int nReqResult = 0;
	bool bRet = m_pTradeOp->QueryHisOrderList((UINT32*)&pReq->dwLocalCookie, strStartDate.c_str(), strEndDate.c_str(), &nReqResult);

	if ( !bRet )
	{
		TradeAckType ack;
		ack.head = req.head;
		if (nReqResult != 0)
		{
			ack.head.ddwErrCode = UtilPlugin::ConvertErrCode((QueryDataErrCode)nReqResult);
			ack.head.strErrDesc = UtilPlugin::GetErrStrByCode((QueryDataErrCode)nReqResult);
		}
		else
		{
			ack.head.ddwErrCode = PROTO_ERR_UNKNOWN_ERROR;
			CA::Unicode2UTF(L"����ʧ��", ack.head.strErrDesc);
		}

		memset(&ack.body, 0, sizeof(ack.body));
		ack.body.nEnvType = body.nEnvType;
		ack.body.nCookie = body.nCookie;		
		HandleTradeAck(&ack, sock);

		DoDeleteReqData(pReq);

		return;
	}
	SetTimerHandleTimeout(true);
}

bool CPluginQueryUSHisOrder::DoDeleteReqData(StockDataReq* pReq)
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

void CPluginQueryUSHisOrder::NotifyOnQueryUSHisOrder(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_OrderItem* pArrOrder)
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

	std::set<int> setStatus;
	std::set<std::wstring> setCode;
	DoGetFilterStatus(pFindReq->req.body.strStatusFilter, setStatus);
	DoGetFilterCode(pFindReq->req.body.strStockCode, setCode);

	if ( nCount > 0 && pArrOrder )
	{
		for ( int n = 0; n < nCount; n++ )
		{			
			const Trade_OrderItem &order = pArrOrder[n];
			QueryUSHisOrderAckItem item;
			item.nOrderID = order.nOrderID;
			item.nOrderType = order.nType;
			item.enSide = order.enSide;
			item.nStatus = order.nStatus;
			item.strStockCode = order.szCode;
			item.strStockName = order.szName;
			item.nPrice = order.nPrice;
			item.nQty = order.nQty;
			item.nDealtQty = order.nDealtQty;
			item.nSubmitedTime = order.nSubmitedTime;
			item.nUpdatedTime = order.nUpdatedTime;

			if (IsFitFilter(item, setStatus, setCode))
			{
				ack.body.vtHisOrder.push_back(item);
			}
		}
	}	 
	
	HandleTradeAck(&ack, pFindReq->sock);

	m_vtReqData.erase(itReq);
	delete pFindReq;
}

void CPluginQueryUSHisOrder::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginQueryUSHisOrder::OnTimeEvent(UINT nEventID)
{
	if ( TIMER_ID_HANDLE_TIMEOUT_REQ == nEventID )
	{
		HandleTimeoutReq();
	}
}

void CPluginQueryUSHisOrder::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{		
	}	
}

void CPluginQueryUSHisOrder::HandleTimeoutReq()
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
			CA::Unicode2UTF(L"Э�鳬ʱ", ack.head.strErrDesc);

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

void CPluginQueryUSHisOrder::HandleTradeAck(TradeAckType *pAck, SOCKET sock)
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

void CPluginQueryUSHisOrder::SetTimerHandleTimeout(bool bStartOrStop)
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

void CPluginQueryUSHisOrder::ClearAllReqAckData()
{
	VT_REQ_TRADE_DATA::iterator it_req = m_vtReqData.begin();
	for ( ; it_req != m_vtReqData.end(); )
	{
		StockDataReq *pReq = *it_req;
		delete pReq;
	}

	m_vtReqData.clear();
}

void CPluginQueryUSHisOrder::DoClearReqInfo(SOCKET socket)
{
	VT_REQ_TRADE_DATA& vtReq = m_vtReqData;

	//���socket��Ӧ��������Ϣ
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

void CPluginQueryUSHisOrder::DoGetFilterStatus(const std::string& strFilter, std::set<int>& setStatus)
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

void CPluginQueryUSHisOrder::DoGetFilterCode(const std::string& strFilter, std::set<std::wstring>& setCode)
{
	setCode.clear();
	if (strFilter.empty())
	{
		return;
	}
	setCode.insert(L"");//��ֹ����û������,��ӿղ�Ӱ��ɸѡ���

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

bool CPluginQueryUSHisOrder::IsFitFilter(const TradeAckItemType& AckItem,
	const std::set<int>& vtStatus, const std::set<std::wstring>& vtCode)
{
	//����������&&��ϵ
	if (!vtStatus.empty() && vtStatus.count(AckItem.nStatus) == 0)
	{
		return false;
	}

	if (!vtCode.empty() && vtCode.count(AckItem.strStockCode) == 0)
	{
		return false;
	}

	return true;
}
