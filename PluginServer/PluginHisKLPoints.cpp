#include "stdafx.h"
#include "PluginHisKLPoints.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoHisKLPoints.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//tomodify 2
#define TIMER_ID_HANDLE_TIMEOUT_REQ		355

#define PROTO_ID_QUOTE		PROTO_ID_QT_GET_HISKL_POINTS

typedef CProtoHisKLPoints	CProtoQuote;


//////////////////////////////////////////////////////////////////////////

CPluginHisKLPoints::CPluginHisKLPoints()
{	
	m_pQuoteData = NULL;
	m_pQuoteServer = NULL;
	m_bStartTimerHandleTimeout = FALSE;
}

CPluginHisKLPoints::~CPluginHisKLPoints()
{
	Uninit();
}

void CPluginHisKLPoints::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData* pQuoteData)
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

	m_TimerWnd.SetEventInterface(this);
	m_TimerWnd.Create();

	m_MsgHandler.SetEventInterface(this);
	m_MsgHandler.Create();
}

void CPluginHisKLPoints::Uninit()
{
	ReleaseAllReqData();

	if ( m_pQuoteServer != NULL )
	{
		m_pQuoteServer = NULL;
		m_pQuoteData = NULL;		

		m_MsgHandler.Close();
		m_MsgHandler.SetEventInterface(NULL);
	}
}

void CPluginHisKLPoints::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	CHECK_RET(nCmdID == PROTO_ID_QUOTE && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pQuoteData && m_pQuoteServer, NORET);

	CProtoQuote proto;
	CProtoQuote::ProtoReqDataType  req;	
	proto.SetProtoData_Req(&req);
	if ( !proto.ParseJson_Req(jsnVal) )
	{
		CHECK_OP(false, NORET);
		return;
	}
	CHECK_RET(req.head.nProtoID == nCmdID, NORET);
	StockDataReq reqError;
	reqError.sock = sock;
	reqError.req = req;
	reqError.dwReqTick = ::GetTickCount();

	std::wstring wstrTimePoint;
	CA::UTF2Unicode(req.body.strTimePoints.c_str(), wstrTimePoint);
	std::vector<CString> vtTime;
	CA::DivStr(wstrTimePoint.c_str(), L",", vtTime);
	if (vtTime.empty())
	{
		ReplyDataReqError(&reqError, PROTO_ERR_UNKNOWN_ERROR, L"无时间点!");
		return;
	}

	if (vtTime.size() > 5)
	{
		ReplyDataReqError(&reqError, PROTO_ERR_UNKNOWN_ERROR, L"时间点过多!");
		return;
	}

	if (req.body.nMaxAckKLItemNum > 0 && req.body.nMaxAckKLItemNum < (int)vtTime.size())
	{
		ReplyDataReqError(&reqError, PROTO_ERR_UNKNOWN_ERROR, L"数据最多返回限制不可小于时间点个数!");
		return;
	}

	std::vector<INT64> vtStockID;
	std::vector<StockMktType> vtMktType;
	std::vector<CString> vtStockCode;
	CString cstrInvalidStockCode;

	int nMaxAckStockNum = 0;
	if (!vtTime.empty() && req.body.nMaxAckKLItemNum > 0)
	{
		nMaxAckStockNum = req.body.nMaxAckKLItemNum / vtTime.size();
	}

	DoGetStockID(req.body.vtStock, vtStockID, nMaxAckStockNum, vtStockCode, vtMktType, cstrInvalidStockCode);
	if (!cstrInvalidStockCode.IsEmpty())
	{
		ReplyDataReqError(&reqError, PROTO_ERR_STOCK_NOT_FIND, CString(L"找不到股票：") + cstrInvalidStockCode);
		return;
	}

	StockDataReq *pReqInfo = new StockDataReq;
	CHECK_RET(pReqInfo, NORET);	
	pReqInfo->sock = sock;
	pReqInfo->req = req;
	pReqInfo->vtStockID = vtStockID;
	pReqInfo->vtStockCode = vtStockCode;
	pReqInfo->vtMktType = vtMktType;
	pReqInfo->vtTime = vtTime;
	pReqInfo->dwReqTick = ::GetTickCount();

	DWORD nCookie = 0;
	
	std::vector<LPCWSTR> vTimeTemp;
	for (auto iter = vtTime.begin(); iter != vtTime.end(); ++iter)
	{
		vTimeTemp.push_back(iter->GetString());
	}

	QueryDataErrCode eErrCode = m_pQuoteServer->QueryBatchHisKLPoints(_vect2Ptr(vtStockID), _vectIntSize(vtStockID),
		_vect2Ptr(vTimeTemp), _vectIntSize(vTimeTemp), req.body.nKLType, req.body.nRehabType, (NoDataMode)req.body.nNodataMode, req.body.nMaxAckKLItemNum, nCookie);

	if (eErrCode == QueryData_Suc && nCookie > 0)
	{
		m_mapReqData[nCookie] = (pReqInfo);
		SetTimerHandleTimeout(true);
	}
	else
	{
		ReplyDataReqError(&reqError, PROTO_ERR_UNKNOWN_ERROR, L"请求失败！");
		return;
	}
}

void CPluginHisKLPoints::NotifyQueryBatchHisKLPoints(DWORD dwCookie, Quote_StockKLData *arStockKLData, int nNum)
{
	if (m_mapReqData.count(dwCookie) == 0)
	{
		return;
	}

	CHECK_RET(m_pQuoteData && m_pQuoteServer, NORET);

	StockDataReq *pReqData = m_mapReqData[dwCookie];
	CHECK_RET(pReqData, NORET);

	CProtoQuote::ProtoReqBodyType &reqBody = pReqData->req.body;
	std::vector<CString> &vtTime = pReqData->vtTime;

	int nStockNum = min(pReqData->vtStockID.size(), pReqData->vtMktType.size());
	int nTimePointNum = vtTime.size();

	int nCanGetStockNum = nStockNum;
	BOOL bHasNext = 0;
	if (nTimePointNum != 0)
	{
		nCanGetStockNum = nNum / nTimePointNum;
	}
	nCanGetStockNum = min(nStockNum, nCanGetStockNum);
	bHasNext = nCanGetStockNum < nStockNum;	

	CProtoQuote::ProtoAckBodyType ackBody;
	ackBody.nCookie = reqBody.nCookie;
	ackBody.nRehabType = reqBody.nRehabType;
	ackBody.nKLType = reqBody.nKLType;
	ackBody.nMaxAckKLItemNum = reqBody.nMaxAckKLItemNum;
	ackBody.nNodataMode = reqBody.nNodataMode;
	ackBody.vtStock = reqBody.vtStock;
	ackBody.strTimePoints = reqBody.strTimePoints;
	ackBody.strNeedKLData = reqBody.strNeedKLData;
	ackBody.nHasNext = bHasNext;

	for (int i = 0; i < nCanGetStockNum; ++i)
	{
		INT64 nStockID = pReqData->vtStockID[i];
		StockMktType eMktType = pReqData->vtMktType[i];

		HISKL stHisKL;
		stHisKL.nStockMarket = (int)eMktType;
		CA::Unicode2UTF(pReqData->vtStockCode[i], stHisKL.strStockCode);

		for (int j = 0; j < nTimePointNum; ++j)
		{
			int nDataIndex = i * nTimePointNum + j;
			CHECK_OP(nDataIndex < nNum, continue);
			Quote_StockKLData &stKLData = arStockKLData[nDataIndex];

			HisKLPointsAckItem stAckItem;
			KLDataToHisKLAckItem(nStockID, stKLData, stAckItem);
			stAckItem.wstrTimePoint = vtTime[j].GetString();
			stHisKL.vtKL.push_back(stAckItem);
		}
		ackBody.vtHisKL.push_back(std::move(stHisKL));
	}
	//
	ReplyStockDataReq(pReqData, ackBody);

	m_mapReqData.erase(dwCookie);
	SAFE_DELETE(pReqData);
}

void CPluginHisKLPoints::NotifySocketClosed(SOCKET sock)
{
	DoClearReqInfo(sock);
}

void CPluginHisKLPoints::OnTimeEvent(UINT nEventID)
{
	if (TIMER_ID_HANDLE_TIMEOUT_REQ == nEventID)
	{
		HandleTimeoutReq();
	}
}

void CPluginHisKLPoints::OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam)
{
}

void CPluginHisKLPoints::KLDataToHisKLAckItem(INT64 nStockID, const Quote_StockKLData &stKLData, HisKLPointsAckItem &stAckItem)
{
	wchar_t szTime[64] = {};
	if (m_pQuoteData)
	{
		m_pQuoteData->TimeStampToStr(nStockID, stKLData.dwTime, szTime);
	}
	stAckItem.nDataValid = stKLData.eDataVaild;
	stAckItem.wstrTime = szTime;
	stAckItem.nOpenPrice = stKLData.nOpenPrice;
	stAckItem.nClosePrice = stKLData.nClosePrice;
	stAckItem.nHighestPrice = stKLData.nHighestPrice;
	stAckItem.nLowestPrice = stKLData.nLowestPrice;
	stAckItem.nPERatio = stKLData.nPERatio;
	stAckItem.nTurnoverRate = stKLData.nTurnoverRate;
	stAckItem.ddwTDVol = stKLData.ddwTDVol;
	stAckItem.ddwTDVal = stKLData.ddwTDVal;
	stAckItem.nRaiseRate = stKLData.nRaiseRate;
}

void CPluginHisKLPoints::SetTimerHandleTimeout(bool bStartOrStop)
{
	if (m_bStartTimerHandleTimeout)
	{
		if (!bStartOrStop)
		{
			m_TimerWnd.StopTimer(TIMER_ID_HANDLE_TIMEOUT_REQ);
			m_bStartTimerHandleTimeout = FALSE;
		}
	}
	else
	{
		if (bStartOrStop)
		{
			m_TimerWnd.StartMillionTimer(1000, TIMER_ID_HANDLE_TIMEOUT_REQ);
			m_bStartTimerHandleTimeout = TRUE;
		}
	}
}

void CPluginHisKLPoints::HandleTimeoutReq()
{
	if (m_mapReqData.empty())
	{
		SetTimerHandleTimeout(false);
		return;
	}

	DWORD dwTickNow = ::GetTickCount();
	MAP_STOCK_DATA_REQ &vtReq = m_mapReqData;

	for (auto it_req = m_mapReqData.begin(); it_req != m_mapReqData.end();)
	{
		StockDataReq *pReq = it_req->second;
		if (pReq == NULL)
		{
			CHECK_OP(false, NOOP);
			it_req = vtReq.erase(it_req);
			continue;
		}
		if (int(dwTickNow - pReq->dwReqTick) > REQ_TIMEOUT_MILLISECOND)
		{
			ReplyDataReqError(pReq, PROTO_ERR_SERVER_TIMEROUT, L"请求超时！");
			it_req = vtReq.erase(it_req);
			delete pReq;
		}
		else
		{
			++it_req;
		}
	}

	if (m_mapReqData.empty())
	{
		SetTimerHandleTimeout(false);
		return;
	}
}

void CPluginHisKLPoints::ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data)
{
	CHECK_RET(pReq && m_pQuoteServer, NORET);

	CProtoQuote::ProtoAckDataType ack;
	ack.head = pReq->req.head;
	ack.head.ddwErrCode = 0;
	ack.body = data;

	std::set<KLDataField> setField;
	DoGetFilterField(pReq->req.body.strNeedKLData, setField);

	//tomodify 4
	CProtoQuote proto;	
	proto.SetProtoData_Ack(&ack);
	proto.SetHisKLPointsArrFieldFilter(setField);

	Json::Value jsnAck;
	if ( proto.MakeJson_Ack(jsnAck) )//
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

void CPluginHisKLPoints::ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc)
{
	CHECK_RET(pReq && m_pQuoteServer, NORET);

	CProtoQuote::ProtoAckDataType ack;
	ack.head = pReq->req.head;
	ack.head.ddwErrCode = nErrCode;

	if ( pErrDesc )
	{
		CA::Unicode2UTF(pErrDesc, ack.head.strErrDesc);		 
	}

	CProtoQuote proto;	
	proto.SetProtoData_Ack(&ack);

	Json::Value jsnAck;
	if ( proto.MakeJson_Ack(jsnAck) )
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

void CPluginHisKLPoints::ReleaseAllReqData()
{
	MAP_STOCK_DATA_REQ::iterator it = m_mapReqData.begin();
	for ( ; it != m_mapReqData.end(); ++it )
	{
		StockDataReq *pReqData = it->second;
		delete pReqData;
	}
	m_mapReqData.clear();
}

void CPluginHisKLPoints::DoGetFilterField(const std::string& strFilter, std::set<KLDataField>& setField)
{
	setField.clear();
	CString strDiv = _T(",");
	std::vector<CString> arFilterStr;
	CA::DivStr(CString(strFilter.c_str()), strDiv, arFilterStr);
	for (UINT i = 0; i < arFilterStr.size(); i++)
	{
		int nTmp = _ttoi(arFilterStr[i]);

		setField.insert((KLDataField)nTmp);
	}
}

void CPluginHisKLPoints::DoGetStockID(const VT_REQ_STOCK_PARAM &vtStock, std::vector<INT64> &vtStockID, int nMaxStockNun,
	std::vector<CString> &vtStockCode, std::vector<StockMktType> &vtMktType, CString &cstrInvalidStockCode)
{
	vtStockID.clear();
	vtMktType.clear();
	cstrInvalidStockCode = L"";

	CString strDiv = _T(",");

	std::vector<StockMktType> vtMktTypeTemp;
	std::vector<std::wstring> vcstrStockCodeTemp;

	int nGetStockNum = vtStock.size();
	if (nMaxStockNun > 0)
	{
		nGetStockNum = min(nMaxStockNun + 1, nGetStockNum);
	}

	for (int i = 0; i < nGetStockNum; i++)
	{
		vtMktTypeTemp.push_back((StockMktType)vtStock[i].nStockMarket);

		std::wstring wstrStockCode;
		CA::UTF2Unicode(vtStock[i].strStockCode.c_str(), wstrStockCode);
		vcstrStockCodeTemp.push_back(wstrStockCode);
	}

	int nStockNum = min(vcstrStockCodeTemp.size(), vtMktTypeTemp.size());
	for (int i = 0; i < nStockNum; ++i)
	{
		INT64 nStockID = IFTStockUtil::GetStockHashVal(vcstrStockCodeTemp[i].c_str(), vtMktTypeTemp[i]);

		if (nStockID != 0)
		{
			vtStockID.push_back(nStockID);
			vtMktType.push_back(vtMktTypeTemp[i]);
			vtStockCode.push_back(vcstrStockCodeTemp[i].c_str());
		}
		else
		{
			cstrInvalidStockCode += vcstrStockCodeTemp[i].c_str();
			cstrInvalidStockCode += L",";
		}
	}
}

void CPluginHisKLPoints::DoClearReqInfo(SOCKET socket)
{
	MAP_STOCK_DATA_REQ& mapReq = m_mapReqData;

	//清掉socket对应的请求信息
	auto itReq = mapReq.begin();
	while (itReq != mapReq.end())
	{
		StockDataReq* pReq = itReq->second;
		if (pReq && pReq->sock == socket)
		{
			delete pReq;
			itReq = mapReq.erase(itReq);
		}
		else
		{
			++itReq;
		}
	}
}
