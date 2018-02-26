#pragma once
#include "Protocol/ProtoDataStruct_Quote.h"
#include "Include/FTPluginTradeInterface.h"
#include "JsonCpp/json.h"

class CPluginQuoteServer;
class CPluginSwitchUser
{
public:
	CPluginSwitchUser();
	virtual ~CPluginSwitchUser();
	
	void Init(CPluginQuoteServer* pQuoteServer, IFTQuoteOperation *pQuoteOp);
	void Uninit();	
	void SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock);

protected:
	typedef SwitchUser_Req	TradeReqType;
	typedef SwitchUser_Ack	TradeAckType;

	void HandleQuoteAck(TradeAckType *pAck, SOCKET	sock);

private:
	CPluginQuoteServer* m_pQuoteServer;
	IFTQuoteOperation *m_pQuoteOp;
};