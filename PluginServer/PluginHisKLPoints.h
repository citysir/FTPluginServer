#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Protocol/ProtoDataStruct_Quote.h"
#include "TimerWnd.h"
#include "MsgHandler.h"
#include "JsonCpp/json.h"
#include <set>

class CPluginQuoteServer;

class CPluginHisKLPoints : public CTimerWndInterface, public CMsgHandlerEventInterface
{
public:
	CPluginHisKLPoints();
	virtual ~CPluginHisKLPoints();

	void Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData* pQuoteData);
	void Uninit();	
	void SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock);

	void NotifyQueryBatchHisKLPoints(DWORD dwCookie, Quote_StockKLData *arStockKLData, int nNum);
	void NotifySocketClosed(SOCKET sock);

protected:	
	//CTimerWndInterface
	virtual void OnTimeEvent(UINT nEventID);

	//CMsgHandlerEventInterface
	virtual void OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam);

protected:
	//tomodify 1
	struct	StockDataReq
	{
		SOCKET	sock;
		DWORD	dwReqTick;
		std::vector<INT64> vtStockID;
		std::vector<StockMktType> vtMktType;
		std::vector<CString> vtTime;
		std::vector<CString> vtStockCode;
		HisKLPoints_Req req;
	};

	typedef std::map<DWORD, StockDataReq*>	MAP_STOCK_DATA_REQ;
	typedef HisKLPointsAckBody	QuoteAckDataBody;	

protected:	
	void ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data);
	void ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc);

	void ReleaseAllReqData();
	void DoGetFilterField(const std::string& strFilter, std::set<KLDataField>& setField);
	void DoGetStockID(const VT_REQ_STOCK_PARAM &vtStock, std::vector<INT64> &vtStockID, int nMaxStockNun,
		std::vector<CString> &vtStockCode, std::vector<StockMktType> &vtMktType, CString &cstrInvalidStockCode);

	void KLDataToHisKLAckItem(INT64 nStockID, const Quote_StockKLData &stKLData, HisKLPointsAckItem &stAckItem);
	
	void SetTimerHandleTimeout(bool bStartOrStop);
	void HandleTimeoutReq();

private:
	void DoClearReqInfo(SOCKET socket);

protected:
	CPluginQuoteServer* m_pQuoteServer;
	IFTQuoteData*		m_pQuoteData;
	CMsgHandler			m_MsgHandler;
	CTimerMsgWndEx		m_TimerWnd;
	BOOL m_bStartTimerHandleTimeout;
	MAP_STOCK_DATA_REQ	m_mapReqData;
};

