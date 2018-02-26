#pragma once
#include "ProtoDataStruct_Quote.h"
#include "ProtoParseBase.h"

class CProtoSuspend : public CProtoParseBase
{
public:
	//tomodify 1
	typedef Suspend_Req		ProtoReqDataType;
	typedef Suspend_Ack		ProtoAckDataType;

	//tomodify 2
	typedef SuspendReqBody	ProtoReqBodyType;
	typedef SuspendAckBody	ProtoAckBodyType;


public:
	CProtoSuspend();
	virtual ~CProtoSuspend();

	virtual bool ParseJson_Req(const Json::Value &jsnVal);
	virtual bool ParseJson_Ack(const Json::Value &jsnVal);
	virtual bool MakeJson_Req(Json::Value &jsnVal);
	virtual bool MakeJson_Ack(Json::Value &jsnVal);

	void	SetProtoData_Req(ProtoReqDataType *pData);
	void	SetProtoData_Ack(ProtoAckDataType *pData);

private:	
	bool ParseProtoBody_Req(const Json::Value &jsnVal, ProtoReqDataType &data);
	bool MakeProtoBody_Req(Json::Value &jsnVal, const ProtoReqDataType &data); 
	bool MakeProtoBody_Ack(Json::Value &jsnVal, const ProtoAckDataType &data);

	void GetProtoBodyField_Req(VT_PROTO_FIELD &vtField, const ProtoReqBodyType &reqData);
	void GetProtoBodyField_Ack(VT_PROTO_FIELD &vtField, const ProtoAckBodyType &ackData);

private:
 	bool MakeStockSuspendArr(Json::Value &jsnRetData, const ProtoAckBodyType &data);
	bool MakeSuspendArr(Json::Value &jsnRetData, const SuspendAckItem &data);

	void GetSuspendArrField(VT_PROTO_FIELD &vtField, const SuspendAckItem &ackItem);
	void GetSuspendItemField(VT_PROTO_FIELD &vtField, const SuspendInfoItem &ackItem);

private:
	bool ParseStockArr_Req(const Json::Value &jsnRetData, ProtoReqBodyType &data);
	bool MakeStockArr_Req(Json::Value &jsnRetData, const ProtoReqBodyType &data);

	bool ParseStockArr_Ack(const Json::Value &jsnRetData, ProtoAckBodyType &data);
	bool MakeStockArr_Ack(Json::Value &jsnRetData, const ProtoAckBodyType &data);
	void GetStockArrField(VT_PROTO_FIELD &vtField, const ReqParamStockItem &ackItem);

private:
	ProtoReqDataType	*m_pReqData;
	ProtoAckDataType	*m_pAckData;
};