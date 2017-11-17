#include "stdafx.h"
#include "PluginQueryUSPosition.h"
#include "PluginUSTradeServer.h"
#include "Protocol/ProtoQueryUSPosition.h"
#include "IManage_SecurityNum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_HANDLE_TIMEOUT_REQ	355
#define EVENT_ID_ACK_REQUEST		368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_TDUS_QUERY_POSITION
typedef CProtoQueryUSPosition	CProtoQuote;

//////////////////////////////////////////////////////////////////////////

CPluginQueryUSPosition::CPluginQueryUSPosition()
{	
	m_pTradeOp = NULL;
	m_pTradeServer = NULL;
	m_bStartTimerHandleTimeout = FALSE;
}

CPluginQueryUSPosition::~CPluginQueryUSPosition()
{
	Uninit();
}

void CPluginQueryUSPosition::Init(CPluginUSTradeServer* pTradeServer, ITrade_US*  pTradeOp, IFTQuoteData *pQuote)
{
	if ( m_pTradeServer != NULL )
		return;

	if (pTradeServer == NULL || pTradeOp == NULL)
	{
		ASSERT(false);
		return;
	}

	m_pTradeServer = pTradeServer;
	m_pTradeOp = pTradeOp;
	m_pQuoteData = pQuote;

	m_TimerWnd.SetEventInterface(this);
	m_TimerWnd.Create();

	m_MsgHandler.SetEventInterface(this);
	m_MsgHandler.Create();
}

void CPluginQueryUSPosition::Uninit()
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

void CPluginQueryUSPosition::SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
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

	if (!IManage_SecurityNum::IsSafeSocket(sock))
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
	QueryPositionReqBody &body = req.body;	
	bool bRet = m_pTradeOp->QueryPositionList((UINT32*)&pReq->dwLocalCookie);

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

bool CPluginQueryUSPosition::DoDeleteReqData(StockDataReq* pReq)
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

void CPluginQueryUSPosition::NotifyOnQueryPosition(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_PositionItem* pArrPosition)
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

	std::set<int> setStockType;
	std::set<std::wstring> setCode;
	DoGetFilterStockType(pFindReq->req.body.strStockType, setStockType);
	DoGetFilterCode(pFindReq->req.body.strStockCode, setCode);
	PLRatioCond PLRatioCondition;
	DoGetFilterPLRatio(pFindReq->req.body.strPLRatioMin, PLRatioCondition.nPLRatioMin, PLRatioCondition.bPLRatioMinValid);
	DoGetFilterPLRatio(pFindReq->req.body.strPLRatioMax, PLRatioCondition.nPLRatioMax, PLRatioCondition.bPLRatioMaxValid);
	if ( nCount > 0 && pArrPosition )
	{
		for ( int n = 0; n < nCount; n++ )
		{			
			const Trade_PositionItem &pos = pArrPosition[n];
			QueryPositionAckItem item;
			item.strStockCode = pos.szCode;
			item.strStockName = pos.szName;
			item.nQty = pos.nQty;
			item.nCanSellQty = pos.nCanSellQty;
			item.nNominalPrice = pos.nNominalPrice;
			item.nMarketVal = pos.nMarketVal;
			item.nCostPrice = int(round(pos.fCostPrice * 1000));
			item.nCostPriceValid = pos.bCostPriceValid;
			item.nPLVal = pos.nPLVal;
			item.nPLValValid = pos.bPLValValid;
			item.nPLRatio = int(round(pos.fPLRatio * 100000));
			item.nPLRatioValid = pos.bPLRatioValid;
			
			item.nToday_PLVal = pos.nToday_PLVal;
			item.nToday_BuyQty = pos.nToday_BuyQty;
			item.nToday_BuyVal = pos.nToday_BuyVal;
			item.nToday_SellQty = pos.nToday_SellQty;
			item.nToday_SellVal = pos.nToday_SellVal;
			if (IsFitFilter(item, PLRatioCondition, setStockType, setCode))
			{
				ack.body.vtPosition.push_back(item);
			}
		}
	}	 
	
	HandleTradeAck(&ack, pFindReq->sock);

	m_vtReqData.erase(itReq);
	delete pFindReq;
}

void CPluginQueryUSPosition::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginQueryUSPosition::OnTimeEvent(UINT nEventID)
{
	if ( TIMER_ID_HANDLE_TIMEOUT_REQ == nEventID )
	{
		HandleTimeoutReq();
	}
}

void CPluginQueryUSPosition::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if ( EVENT_ID_ACK_REQUEST == nEvent )
	{		
	}	
}

void CPluginQueryUSPosition::HandleTimeoutReq()
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

void CPluginQueryUSPosition::HandleTradeAck(TradeAckType *pAck, SOCKET sock)
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

void CPluginQueryUSPosition::SetTimerHandleTimeout(bool bStartOrStop)
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

void CPluginQueryUSPosition::ClearAllReqAckData()
{
	VT_REQ_TRADE_DATA::iterator it_req = m_vtReqData.begin();
	for ( ; it_req != m_vtReqData.end(); )
	{
		StockDataReq *pReq = *it_req;
		delete pReq;
	}

	m_vtReqData.clear();
}

void CPluginQueryUSPosition::DoClearReqInfo(SOCKET socket)
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

void CPluginQueryUSPosition::DoGetFilterCode(const std::string& strFilter, std::set<std::wstring>& setCode)
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

void CPluginQueryUSPosition::DoGetFilterStockType(const std::string& strFilter, std::set<int>& setStockType)
{
	setStockType.clear();
	CString strDiv = _T(",");
	std::vector<CString> arFilterStr;
	CA::DivStr(CString(strFilter.c_str()), strDiv, arFilterStr);
	for (UINT i = 0; i < arFilterStr.size(); i++)
	{
		int nTmp = _ttoi(arFilterStr[i]);

		setStockType.insert(nTmp);
	}
}

void CPluginQueryUSPosition::DoGetFilterPLRatio(const std::string &strFilter, int &fPLRatioMinMax, bool &bPLRatioMinMaxValid)
{
	bPLRatioMinMaxValid = !strFilter.empty();
	if (bPLRatioMinMaxValid)
	{
		fPLRatioMinMax = atoi(strFilter.c_str());
	}
}

bool CPluginQueryUSPosition::IsFitFilter(const TradeAckItemType& AckItem, const PLRatioCond& PLRatioCondition,
	const std::set<int>& setStockType, const std::set<std::wstring>& setCode)
{
	if (PLRatioCondition.bPLRatioMinValid || PLRatioCondition.bPLRatioMaxValid)
	{
		if (AckItem.nPLRatioValid == 0)
		{
			return false;
		}

		if (PLRatioCondition.bPLRatioMinValid)
		{
			if (AckItem.nPLRatio < PLRatioCondition.nPLRatioMin)
			{
				return false;
			}
		}
		
		if (PLRatioCondition.bPLRatioMaxValid)
		{
			if (AckItem.nPLRatio > PLRatioCondition.nPLRatioMax)
			{
				return false;
			}
		}
	}

	if (!setCode.empty() && setCode.count(AckItem.strStockCode) == 0)
	{
		return false;
	}

	if (!setStockType.empty() && setStockType.count(PluginSecurity_All) == 0 && m_pQuoteData)
	{
		INT64 nStockID = m_pQuoteData->GetStockHashVal(AckItem.strStockCode.c_str(), StockMkt_US);
		StockMktType eMkt = StockMkt_None;
		wchar_t szStockCode[16] = {};
		wchar_t szStockName[128] = {};
		int nSecurityType = 0;
		if (m_pQuoteData->GetStockInfoByHashVal(nStockID, eMkt, szStockCode, szStockName,
			NULL, &nSecurityType, NULL, NULL))
		{
			if (setStockType.count(nSecurityType) == 0)
			{
				return false;
			}
		}
	}

	return true;
}
