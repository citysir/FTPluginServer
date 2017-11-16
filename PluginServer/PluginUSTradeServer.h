#pragma once
#include "Include/FTPluginCore.h"
#include "Include/FTPluginQuoteInterface.h"
#include "Include/FTPluginTradeInterface.h"
#include "Protocol/ProtoDataStruct.h"
#include "JsonCpp/json.h"
#include "PluginPlaceOrder_US.h"
#include "PluginChangeOrder_US.h"
#include "PluginSetOrderStatus_US.h"
#include "PluginQueryUSOrder.h"
#include "PluginQueryUSAccInfo.h"
#include "PluginQueryUSPosition.h"
#include "PluginQueryUSDeal.h"
#include "PluginQueryUSHisOrder.h"
#include "PluginQueryUSHisDeal.h"
#include "PluginSubUSOrderDeal.h"
#include "PluginPushUSOrder.h"
#include "PluginPushUSDeal.h"

class CPluginNetwork;

class CPluginUSTradeServer: 	
	public ITradeCallBack_US
{
public:
	CPluginUSTradeServer();
	virtual ~CPluginUSTradeServer();
	
	void InitTradeSvr(IFTPluginCore* pPluginCore, CPluginNetwork *pNetwork);
	void UninitTradeSvr();	
	void SetTradeReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock);
	void ReplyTradeReq(int nCmdID, const char *pBuf, int nLen, SOCKET sock);	

	void CloseSocket(SOCKET sock);

protected:
	//ITradeCallBack_US
	virtual void OnPlaceOrder(UINT32 nCookie, Trade_SvrResult enSvrRet, UINT64 nLocalID, INT64 nErrHash);
	virtual void OnOrderUpdate(const Trade_OrderItem& orderItem);

	virtual void OnCancelOrder(UINT32 nCookie, Trade_SvrResult enSvrRet, UINT64 nOrderID, INT64 nErrHash);
	virtual void OnChangeOrder(UINT32 nCookie, Trade_SvrResult enSvrRet, UINT64 nOrderID, INT64 nErrHash);

	//virtual void OnOrderErrNotify(Trade_Env enEnv, UINT64 nOrderID, Trade_OrderErrNotify_HK enErrNotify, UINT16 nErrCode);
	virtual void OnUnlockTrade(UINT32 nCookie, Trade_SvrResult enSvrRet, UINT64 nErrCode);
	virtual void OnQueryOrderList(UINT32 nCookie, INT32 nCount, const Trade_OrderItem* pArrOrder);
	virtual void OnQueryDealList(UINT32 nCookie, INT32 nCount, const Trade_DealItem* pArrOrder);
	virtual void OnQueryAccInfo(UINT32 nCookie, const Trade_AccInfo& accInfo, int nResult);
	virtual void OnQueryPositionList(UINT32 nCookie, INT32 nCount, const Trade_PositionItem* pArrPosition);
	virtual void OnQueryHisOrderList(UINT32 nCookie, INT32 nCount, const Trade_OrderItem* pArrOrder);
	virtual void OnQueryHisDealList(UINT32 nCookie, INT32 nCount, const Trade_DealItem* pArrDeal);
	virtual void OnDealUpdate(const Trade_DealItem& dealItem);

protected:
	IFTPluginCore		*m_pPluginCore;
	ITrade_US			*m_pTradeOp;	
	IFTDataReport		*m_pDataReport;
	CPluginNetwork		*m_pNetwork;

	CPluginPlaceOrder_US	m_PlaceOrder;
	CPluginChangeOrder_US  m_ChangeOrder;
	CPluginSetOrderStatus_US m_SetOrderStatus;

	CPluginQueryUSOrder		m_QueryUSOrder;
	CPluginQueryUSAccInfo	m_QueryUSAcc;
	CPluginQueryUSPosition	m_QueryPos;

	CPluginQueryUSDeal		m_QueryUSDeal;

	CPluginQueryUSHisOrder	m_QueryHisOrder;
	CPluginQueryUSHisDeal	m_QueryHisDeal;

	CPluginPushUSOrder		m_PushUSOrder;
	CPluginPushUSDeal		m_PushUSDeal;
	CPluginSubUSOrderDeal	m_SubUSOrderDeal;
};