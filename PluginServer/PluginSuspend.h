#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct_Quote.h"
#include "TimerWnd.h"
#include "MsgHandler.h"
#include "JsonCpp/json.h"
#include "Protocol/ProtoSuspend.h"

class CPluginQuoteServer;

class CPluginSuspend : public CMsgHandlerEventInterface
{
	//tomodify 1
	typedef CProtoSuspend	CProtoQuote;

public:
	CPluginSuspend();
	virtual ~CPluginSuspend();
	
	void Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData);
	void Uninit();	
	void SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock);	

	void NotifySocketClosed(SOCKET sock);

protected:
	//CMsgHandlerEventInterface
	virtual void OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam);

protected:
	typedef CProtoQuote::ProtoAckBodyType		QuoteAckDataBody;
	typedef std::map<INT64, std::pair<string, StockMktType> >	MAP_STOCK_INFO;
	
	struct	StockDataReq
	{
		SOCKET	sock;
		DWORD	dwReqTick;
		DWORD	nReqCookie;
		std::vector<INT64> vtStockID;
		MAP_STOCK_INFO mapStock;
		CProtoQuote::ProtoReqDataType req;
	};
	typedef std::vector<StockDataReq*>	VT_STOCK_DATA_REQ;

protected:
	void ReplyAllRequest();
	void ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data);
	void ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc);	
	void ReplyDataDefError(SOCKET sock, int nErrCode, LPCWSTR pErrDesc);
	void ClearAllReqCache();
	
private:
	void DoClearReqInfo(SOCKET socket);

protected:
	CPluginQuoteServer* m_pQuoteServer;
	IFTQuoteData*		m_pQuoteData;
	
	CMsgHandler			m_MsgHandler;

	VT_STOCK_DATA_REQ	m_vtReqData;
};