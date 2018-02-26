#include "stdafx.h"
#include "PluginSuspend.h"
#include "PluginQuoteServer.h"
#include "Include/FTPluginQuoteDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern GUID		PLUGIN_GUID;
#define EVENT_ID_ACK_REQUEST	368

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_QT_GET_SUSPEND

//////////////////////////////////////////////////////////////////////////

CPluginSuspend::CPluginSuspend()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
}

CPluginSuspend::~CPluginSuspend()
{
	Uninit();
}

void CPluginSuspend::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData)
{
	if ( m_pQuoteServer != NULL )
		return;

	if (pQuoteServer == NULL || pQuoteData == NULL)
	{
		ASSERT(false);
		return;
	}

	m_pQuoteServer = pQuoteServer;
	m_pQuoteData = pQuoteData;

	m_MsgHandler.SetEventInterface(this);
	m_MsgHandler.Create();
}

void CPluginSuspend::Uninit()
{
	if ( m_pQuoteServer != NULL )
	{
		m_pQuoteServer = NULL;
		m_pQuoteData = NULL;

		m_MsgHandler.Close();
		m_MsgHandler.SetEventInterface(NULL);

		ClearAllReqCache();
	}
}

void CPluginSuspend::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	CHECK_RET(nCmdID == PROTO_ID_QUOTE && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pQuoteData && m_pQuoteServer, ReplyDataDefError(sock, PROTO_ERR_UNKNOWN_ERROR, L"内部状态错误"));
	
	CProtoQuote proto;
	CProtoQuote::ProtoReqDataType	req;
	proto.SetProtoData_Req(&req);
	if ( !proto.ParseJson_Req(jsnVal) || req.head.nProtoID != nCmdID)
	{
		CHECK_OP(false, NORET);
		req.head.nProtoID = nCmdID;

		StockDataReq req_info;		
		req_info.sock = sock;
		req_info.req = req;
		req_info.dwReqTick = ::GetTickCount();
		ReplyDataReqError(&req_info, PROTO_ERR_PARAM_ERR, L"参数错误！");
		return;
	}
	
	int nStockNum = (int)req.body.vtStock.size();
	std::vector<INT64> vtReqStockID;
	MAP_STOCK_INFO mapStock;
	for (int n = 0; n < nStockNum; n++)
	{
		std::string strStockCode = req.body.vtStock[n].strStockCode;
		StockMktType eMktType = (StockMktType)req.body.vtStock[n].nStockMarket;
		INT64 nStockID = IFTStockUtil::GetStockHashVal(strStockCode.c_str(), eMktType);
		if (0 == nStockID)
		{
			CHECK_OP(false, NOOP);
			StockDataReq req_info;			
			req_info.sock = sock;
			req_info.req = req;
			req_info.dwReqTick = ::GetTickCount();
			ReplyDataReqError(&req_info, PROTO_ERR_STOCK_NOT_FIND, L"找不到股票！");
			return;
		}
		vtReqStockID.push_back(nStockID);
		mapStock[nStockID] = std::make_pair(strStockCode, eMktType);
	}

	StockDataReq *pReqInfo = new StockDataReq;
	CHECK_RET(pReqInfo, NORET);	
	pReqInfo->sock = sock;
	pReqInfo->req = req;
	pReqInfo->dwReqTick = ::GetTickCount();
	pReqInfo->vtStockID = vtReqStockID;
	pReqInfo->mapStock = mapStock;
	m_vtReqData.push_back(pReqInfo);

	m_MsgHandler.RaiseEvent(EVENT_ID_ACK_REQUEST, 0, 0);
}

void CPluginSuspend::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginSuspend::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
	if (EVENT_ID_ACK_REQUEST == nEvent)
	{
		ReplyAllRequest();
	}
}

void CPluginSuspend::ReplyAllRequest()
{
	CHECK_RET(m_pQuoteData && m_pQuoteServer, NORET);

	VT_STOCK_DATA_REQ vtReqData;
	vtReqData.swap(m_vtReqData);

	//tomodify 3
	VT_STOCK_DATA_REQ::iterator itReq = vtReqData.begin();
	for (; itReq != vtReqData.end(); ++itReq)
	{
		StockDataReq *pReqData = *itReq;
		CHECK_OP(pReqData, continue);
		CProtoQuote::ProtoReqBodyType &reqBody = pReqData->req.body;

		LPCWSTR pszDateFrom = NULL, pszDateTo = NULL;
		std::wstring strDateFrom, strDateTo;
		if (!reqBody.strStartDate.empty())
		{
			CA::UTF2Unicode(reqBody.strStartDate.c_str(), strDateFrom);
			pszDateFrom = strDateFrom.c_str();
		}
		if (!reqBody.strEndDate.empty())
		{
			CA::UTF2Unicode(reqBody.strEndDate.c_str(), strDateTo);
			pszDateTo = strDateTo.c_str();
		}

		QuoteAckDataBody stAckBody;
		stAckBody.nCookie = reqBody.nCookie;
		stAckBody.vtStock = reqBody.vtStock;
		stAckBody.strStartDate = reqBody.strStartDate;
		stAckBody.strEndDate = reqBody.strEndDate;

		MAP_STOCK_INFO &mapStock = pReqData->mapStock;
		std::vector<INT64> &vtStockID = pReqData->vtStockID;
		for (auto itStock = vtStockID.begin(); itStock != vtStockID.end(); ++itStock)
		{
			INT64 nStockID = *itStock;
			INT64 *parSuspendDateList = NULL;
			int nDateNum = 0;

			const std::pair<string, StockMktType> &pairStock = mapStock[nStockID];
			std::string strStockCode = pairStock.first;
			StockMktType eMktType = pairStock.second;

			SuspendAckItem stAckItem;
			stAckItem.strStockCode = strStockCode;
			stAckItem.nStockMarket = (int)eMktType;

			bool bRet = m_pQuoteData->GetSuspendDateList(nStockID, pszDateFrom, pszDateTo, parSuspendDateList, nDateNum);
			if (bRet && parSuspendDateList && nDateNum > 0)
			{
				for (int i = 0; i < nDateNum; ++i)
				{
					INT64 nTimestamp = parSuspendDateList[i];
					SuspendInfoItem stSuspendItem;
					stSuspendItem.nSuspendTimestamp = nTimestamp;
					if (m_pQuoteData)
					{
						wchar_t szDate[64] = {};
						m_pQuoteData->TimeStampToStrDate(nStockID, (DWORD)nTimestamp, szDate);
						std::string strDate;
						CA::Unicode2UTF(szDate, strDate);
						stSuspendItem.strSuspendTime = strDate;
					}
					stAckItem.vtSupend.push_back(stSuspendItem);
				}
			}
			stAckBody.vtStockSupend.push_back(std::move(stAckItem));
		}
		ReplyStockDataReq(pReqData, stAckBody);
	}
}

void CPluginSuspend::ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data)
{
	CHECK_RET(pReq && m_pQuoteServer, NORET);

	CProtoQuote::ProtoAckDataType ack;
	ack.head = pReq->req.head;
	ack.head.ddwErrCode = 0;

	ack.body = data;
	
	CProtoQuote proto;	
	proto.SetProtoData_Ack(&ack);

	Json::Value jsnAck;
	if (proto.MakeJson_Ack(jsnAck))
	{
		std::string strOut;
		CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
		m_pQuoteServer->ReplyQuoteReq(pReq->req.head.nProtoID, strOut.c_str(), (int)strOut.size(), pReq->sock);
	}
	else
	{
		CHECK_OP(false, NOOP);
	}
}

void CPluginSuspend::ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc)
{
	CHECK_RET(pReq && m_pQuoteServer, NORET);

	CProtoQuote::ProtoAckDataType ack;
	ack.head = pReq->req.head;
	ack.head.ddwErrCode = nErrCode;

	if (pErrDesc)
	{
		CA::Unicode2UTF(pErrDesc, ack.head.strErrDesc);		 
	}

	CProtoQuote proto;	
	proto.SetProtoData_Ack(&ack);

	Json::Value jsnAck;
	if (proto.MakeJson_Ack(jsnAck))
	{
		std::string strOut;
		CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
		m_pQuoteServer->ReplyQuoteReq(pReq->req.head.nProtoID, strOut.c_str(), (int)strOut.size(), pReq->sock);
	}
	else
	{
		CHECK_OP(false, NOOP);
	}
}

void CPluginSuspend::ReplyDataDefError(SOCKET sock, int nErrCode, LPCWSTR pErrDesc)
{
	CHECK_RET(m_pQuoteServer, NORET);

	CProtoQuote::ProtoAckDataType ack;
	ack.head.nProtoID = PROTO_ID_QUOTE;	
	ack.head.ddwErrCode = nErrCode;

	if (pErrDesc)
	{
		CA::Unicode2UTF(pErrDesc, ack.head.strErrDesc);		 
	}

	CProtoQuote proto;	
	proto.SetProtoData_Ack(&ack);

	Json::Value jsnAck;
	if (proto.MakeJson_Ack(jsnAck))
	{
		std::string strOut;
		CProtoParseBase::ConvJson2String(jsnAck, strOut, true);
		m_pQuoteServer->ReplyQuoteReq(PROTO_ID_QUOTE, strOut.c_str(), (int)strOut.size(), sock);
	}
	else
	{
		CHECK_OP(false, NOOP);
	}
}

void CPluginSuspend::ClearAllReqCache()
{
	VT_STOCK_DATA_REQ::iterator it = m_vtReqData.begin();
	for (; it != m_vtReqData.end(); ++it)
	{
		StockDataReq *pReqData = *it;
		delete pReqData;
	}
	m_vtReqData.clear();
}

void CPluginSuspend::DoClearReqInfo(SOCKET socket)
{
	VT_STOCK_DATA_REQ& vtReq = m_vtReqData;

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
