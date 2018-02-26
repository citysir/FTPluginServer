#include "stdafx.h"
#include "PluginSwitchUser.h"
#include "PluginQuoteServer.h"
#include "Protocol/ProtoSwitchUser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//tomodify 2
#define PROTO_ID_QUOTE		PROTO_ID_QT_SWITCH_USER
typedef CProtoSwitchUser	CProtoQuote;

//////////////////////////////////////////////////////////////////////////

CPluginSwitchUser::CPluginSwitchUser()
{	
	m_pQuoteServer = NULL;
}

CPluginSwitchUser::~CPluginSwitchUser()
{
	Uninit();
}

void CPluginSwitchUser::Init(CPluginQuoteServer* pQuoteServer, IFTQuoteOperation *pQuoteOp)
{
	if (m_pQuoteServer != NULL)
		return;

	CHECK_RET(pQuoteServer && pQuoteOp, NORET);

	m_pQuoteServer = pQuoteServer;
	m_pQuoteOp = pQuoteOp;
}

void CPluginSwitchUser::Uninit()
{
	m_pQuoteServer = NULL;
}

void CPluginSwitchUser::SetQuoteReqData(int nCmdID, const Json::Value &jsnVal, SOCKET sock)
{
	CHECK_RET(nCmdID == PROTO_ID_QUOTE && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pQuoteOp, NORET);

	CProtoQuote proto;
	CProtoQuote::ProtoReqDataType	req;
	proto.SetProtoData_Req(&req);
	if ( !proto.ParseJson_Req(jsnVal) )
	{
		CHECK_OP(false, NORET);
		TradeAckType ack;
		ack.head = req.head;
		ack.head.ddwErrCode = PROTO_ERR_PARAM_ERR;
		CA::Unicode2UTF(L"²ÎÊý´íÎó£¡", ack.head.strErrDesc);
		ack.body.nCookie = req.body.nCookie;
		HandleQuoteAck(&ack, sock);
		return;
	}

	CHECK_RET(req.head.nProtoID == nCmdID && req.body.nCookie, NORET);

	m_pQuoteOp->SwitchUser(req.body.nUserID, req.body.strPasswordMD5.c_str());
}

void CPluginSwitchUser::HandleQuoteAck(TradeAckType *pAck, SOCKET sock)
{
	CHECK_RET(pAck && pAck->body.nCookie && sock != INVALID_SOCKET, NORET);
	CHECK_RET(m_pQuoteServer, NORET);

	CProtoQuote proto;
	proto.SetProtoData_Ack(pAck);

	Json::Value jsnValue;
	bool bRet = proto.MakeJson_Ack(jsnValue);
	CHECK_RET(bRet, NORET);

	std::string strBuf;
	CProtoParseBase::ConvJson2String(jsnValue, strBuf, true);
	m_pQuoteServer->ReplyQuoteReq(PROTO_ID_QUOTE, strBuf.c_str(), (int)strBuf.size(), sock);
}