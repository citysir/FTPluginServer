#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Protocol/ProtoDataStruct_Quote.h"
#include "TimerWnd.h"
#include "MsgHandler.h"
#include "JsonCpp/json.h"
#include <set>

class CPluginQuoteServer;

class CPluginHistoryKL : public CMsgHandlerEventInterface
{
public:
	CPluginHistoryKL();
	virtual ~CPluginHistoryKL();

	void Init(CPluginQuoteServer* pQuoteServer, IFTQuoteData*  pQuoteData);
	void Uninit();	
	void SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock);
	void NotifyQuoteDataUpdate(int nCmdID, INT64 nStockID);

	void NotifySocketClosed(SOCKET sock);

protected:	
	//CMsgHandlerEventInterface
	virtual void OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam);

protected:
	//tomodify 1
	struct	StockDataReq
	{
		SOCKET	sock;
		DWORD	dwReqTick;
		INT64	nStockID;
		HistoryKL_Req req;
	};
	typedef std::vector<StockDataReq*>	VT_STOCK_DATA_REQ;
	typedef HistoryKLAckBody	QuoteAckDataBody;	

protected:	
	void ReplyAllRequest();
	void ReplyStockDataReq(StockDataReq *pReq, const QuoteAckDataBody &data);
	void ReplyDataReqError(StockDataReq *pReq, int nErrCode, LPCWSTR pErrDesc);

	void ReleaseAllReqData();
	void DoGetFilterField(const std::string& strFilter, std::set<KLDataField>& setField);

private:
	void DoClearReqInfo(SOCKET socket);

protected:
	CPluginQuoteServer* m_pQuoteServer;
	IFTQuoteData*		m_pQuoteData;
	CMsgHandler			m_MsgHandler;

	VT_STOCK_DATA_REQ	m_vtReqData;
};

