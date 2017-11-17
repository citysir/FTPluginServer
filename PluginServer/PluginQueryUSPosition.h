#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct_Trade.h"
#include "TimerWnd.h"
#include "MsgHandler.h"
#include "JsonCpp/json.h"

class CPluginUSTradeServer;

class CPluginQueryUSPosition : public CTimerWndInterface, public CMsgHandlerEventInterface
{
public:
	CPluginQueryUSPosition();
	virtual ~CPluginQueryUSPosition();
	
	void Init(CPluginUSTradeServer* pTradeServer, ITrade_US*  pTradeOp, IFTQuoteData* pQuote);
	void Uninit();	
	void SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock);
	void NotifyOnQueryPosition(Trade_Env enEnv, UINT32 nCookie, INT32 nCount, const Trade_PositionItem* pArrPosition);

	void NotifySocketClosed(SOCKET sock);

protected:
	//CTimerWndInterface 
	virtual void OnTimeEvent(UINT nEventID);

	//CMsgHandlerEventInterface
	virtual void OnMsgEvent(int nEvent,WPARAM wParam,LPARAM lParam);

protected:
	//tomodify 1
	typedef QueryPosition_Req	TradeReqType;
	typedef QueryPosition_Ack	TradeAckType;
	typedef QueryPositionAckItem TradeAckItemType;

	struct	StockDataReq
	{
		SOCKET	sock;
		DWORD	dwReqTick;
		DWORD	dwLocalCookie;
		TradeReqType req;
	};
	typedef std::vector<StockDataReq*>		VT_REQ_TRADE_DATA;	
	
	struct PLRatioCond
	{
		bool bPLRatioMinValid;
		bool bPLRatioMaxValid;

		int nPLRatioMin; //0.001%=1
		int nPLRatioMax;
		PLRatioCond()
		{
			bPLRatioMinValid = false;
			bPLRatioMaxValid = false;

			nPLRatioMin = 0;
			nPLRatioMax = 0;
		}
	};

protected:	
	void HandleTimeoutReq();
	void HandleTradeAck(TradeAckType *pAck, SOCKET	sock);
	void SetTimerHandleTimeout(bool bStartOrStop);
	void ClearAllReqAckData();
	
private: 
	bool DoDeleteReqData(StockDataReq* pReq); 

private:
	void DoClearReqInfo(SOCKET socket);
	void DoGetFilterCode(const std::string& strFilter, std::set<std::wstring>& setCode);
	void DoGetFilterStockType(const std::string& strFilter, std::set<int>& setStockType);
	void DoGetFilterPLRatio(const std::string &strFilter, int &fPLRatioMinMax, bool &bPLRatioMinMaxValid);

	bool IsFitFilter(const TradeAckItemType& AckItem, const PLRatioCond& PLRatioCondition,
		const std::set<int>& setStockType, const std::set<std::wstring>& setCode);

protected:
	CPluginUSTradeServer	*m_pTradeServer;
	ITrade_US				*m_pTradeOp;	
	IFTQuoteData			*m_pQuoteData;
	BOOL					m_bStartTimerHandleTimeout;
	
	CTimerMsgWndEx		m_TimerWnd;
	CMsgHandler			m_MsgHandler;

	VT_REQ_TRADE_DATA	m_vtReqData;
};