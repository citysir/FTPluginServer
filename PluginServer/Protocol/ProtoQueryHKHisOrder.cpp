#include "stdafx.h"
#include "ProtoQueryHKHisOrder.h"
#include "CppJsonConv.h"
#include "../JsonCpp/json_op.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CProtoQueryHKHisOrder::CProtoQueryHKHisOrder()
{
	m_pReqData = NULL;
	m_pAckData = NULL;
}

CProtoQueryHKHisOrder::~CProtoQueryHKHisOrder()
{

}

bool CProtoQueryHKHisOrder::ParseJson_Req(const Json::Value &jsnVal)
{
	CHECK_RET(m_pReqData != NULL, false);

	bool bSuc = true;
	do 
	{
		bSuc &= ParseProtoHead_Req(jsnVal, m_pReqData->head);
		CHECK_OP(bSuc, break);
		bSuc &= ParseProtoBody_Req(jsnVal, *m_pReqData);
		CHECK_OP(bSuc, break);
	} while (0);

	return bSuc;
}

bool CProtoQueryHKHisOrder::ParseJson_Ack(const Json::Value &jsnVal)
{
	CHECK_RET(m_pAckData != NULL, false);

	bool bSuc = true;
	do 
	{
		bSuc &= ParseProtoHead_Ack(jsnVal, m_pAckData->head);
		CHECK_OP(bSuc, break);

		if ( m_pAckData->head.ddwErrCode == PROTO_ERR_NO_ERROR )
		{
			bSuc &= ParseProtoBody_Ack(jsnVal, *m_pAckData);
			CHECK_OP(bSuc, break);
		}
	} while (0);

	return bSuc;
}


bool CProtoQueryHKHisOrder::MakeJson_Req(Json::Value &jsnVal)
{
	CHECK_RET(m_pReqData != NULL, false);

	bool bSuc = true;
	do 
	{
		bSuc &= MakeProtoHead_Req(jsnVal, m_pReqData->head);
		CHECK_OP(bSuc, break);
		bSuc &= MakeProtoBody_Req(jsnVal, *m_pReqData);
		CHECK_OP(bSuc, break);
	} while (0);

	return bSuc;
}

bool CProtoQueryHKHisOrder::MakeJson_Ack(Json::Value &jsnVal)
{
	CHECK_RET(m_pAckData != NULL, false);

	bool bSuc = true;
	do 
	{
		bSuc &= MakeProtoHead_Ack(jsnVal, m_pAckData->head);
		CHECK_OP(bSuc, break);

		if ( m_pAckData->head.ddwErrCode == PROTO_ERR_NO_ERROR )
		{
			bSuc &= MakeProtoBody_Ack(jsnVal, *m_pAckData);
			CHECK_OP(bSuc, break);
		}
	} while (0);

	return bSuc;
}

void CProtoQueryHKHisOrder::SetProtoData_Req(ProtoReqDataType *pData)
{
	m_pReqData = pData;
}

void CProtoQueryHKHisOrder::SetProtoData_Ack(ProtoAckDataType *pData)
{
	m_pAckData = pData;
}

//tomodify 3(����ȸ��ӽṹ�򵥲�Ľṹ��)
bool CProtoQueryHKHisOrder::ParseProtoBody_Req(const Json::Value &jsnVal, ProtoReqDataType &data)
{	
	if ( !warn_if_prop_not_set(jsnVal, KEY_REQ_PARAM) )
		return true;

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Req(vtField, data.body);

	const Json::Value &jsnBody = jsnVal[KEY_REQ_PARAM];
	bool bSuc = CProtoParseBase::ParseProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	return true;
}

//tomodify 4(����ȸ��ӽṹ�򵥲�Ľṹ��)
bool CProtoQueryHKHisOrder::ParseProtoBody_Ack(const Json::Value &jsnVal, ProtoAckDataType &data)
{
	CHECK_RET(warn_if_prop_not_set(jsnVal, KEY_ACK_DATA), false);	
		
	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Ack(vtField, data.body);

	const Json::Value &jsnBody = jsnVal[KEY_ACK_DATA];
	bool bSuc = CProtoParseBase::ParseProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	if ( bSuc )
	{
		bSuc &= ParseHKOrderArr(jsnBody, data.body);
	}

	return bSuc;
}

//tomodify 5(����ȸ��ӽṹ�򵥲�Ľṹ��)
bool CProtoQueryHKHisOrder::MakeProtoBody_Req(Json::Value &jsnVal, const ProtoReqDataType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnVal, KEY_REQ_PARAM), false);

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Req(vtField, data.body);

	Json::Value &jsnBody = jsnVal[KEY_REQ_PARAM];
	bool bSuc = CProtoParseBase::MakeProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	return bSuc;
}

//tomodify 6(����ȸ��ӽṹ�򵥲�Ľṹ��)
bool CProtoQueryHKHisOrder::MakeProtoBody_Ack(Json::Value &jsnVal, const ProtoAckDataType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnVal, KEY_ACK_DATA), false);	
	
	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Ack(vtField, data.body);

	Json::Value &jsnBody = jsnVal[KEY_ACK_DATA];
	bool bSuc = CProtoParseBase::MakeProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	if ( bSuc )
	{
		bSuc &= MakeHKOrderArr(jsnBody, data.body);
	}

	return bSuc;
}

//tomodify 7
void CProtoQueryHKHisOrder::GetProtoBodyField_Req(VT_PROTO_FIELD &vtField, const ProtoReqBodyType &reqData)
{
	static BOOL arOptional[] = {
		TRUE, FALSE, TRUE,
		TRUE, TRUE, TRUE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_Int32, ProtoFild_StringA,
		ProtoFild_StringA, ProtoFild_StringA, ProtoFild_StringA,
	};
	static LPCSTR arFieldKey[] = {
		"EnvType", "Cookie", "StatusFilterStr",
		"StockCode", "start_date", "end_date",
	};

	ProtoReqBodyType &body = const_cast<ProtoReqBodyType &>(reqData);
	void *arPtr[] = {
		&body.nEnvType, &body.nCookie, &body.strStatusFilter,
		&body.strStockCode, &body.strStartDate, &body.strEndDate,
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	vtField.clear();
	PROTO_FIELD field;
	for ( int n = 0; n < _countof(arOptional); n++ )
	{
		field.bOptional = arOptional[n];
		field.eFieldType = arFieldType[n];
		field.strFieldKey = arFieldKey[n];
		switch (field.eFieldType)
		{
		case ProtoFild_Int32:
			field.pInt32 = (int*)arPtr[n];
			break;
		case ProtoFild_Int64:
			field.pInt64 = (INT64*)arPtr[n];
			break;
		case ProtoFild_StringA:
			field.pStrA = (std::string*)arPtr[n];
			break;
		case ProtoFild_StringW:
			field.pStrW = (std::wstring*)arPtr[n];
			break;
		default:
			CHECK_OP(FALSE, NOOP);
			break;
		}

		vtField.push_back(field);
	}	
}

//tomodify 8
void CProtoQueryHKHisOrder::GetProtoBodyField_Ack(VT_PROTO_FIELD &vtField, const ProtoAckBodyType &ackData)
{
	static BOOL arOptional[] = {
		FALSE, FALSE, 		
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32,	ProtoFild_Int32		
	};
	static LPCSTR arFieldKey[] = {		
		"EnvType",	"Cookie",
	};

	ProtoAckBodyType &body = const_cast<ProtoAckBodyType &>(ackData);
	void *arPtr[] = {
		&body.nEnvType,		&body.nCookie,
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	vtField.clear();
	PROTO_FIELD field;
	for ( int n = 0; n < _countof(arOptional); n++ )
	{
		field.bOptional = arOptional[n];
		field.eFieldType = arFieldType[n];
		field.strFieldKey = arFieldKey[n];
		switch (field.eFieldType)
		{
		case ProtoFild_Int32:
			field.pInt32 = (int*)arPtr[n];
			break;
		case ProtoFild_Int64:
			field.pInt64 = (INT64*)arPtr[n];
			break;
		case ProtoFild_StringA:
			field.pStrA = (std::string*)arPtr[n];
			break;
		case ProtoFild_StringW:
			field.pStrW = (std::wstring*)arPtr[n];
			break;
		default:
			CHECK_OP(FALSE, NOOP);
			break;
		}

		vtField.push_back(field);
	}	
}

bool CProtoQueryHKHisOrder::ParseHKOrderArr(const Json::Value &jsnRetData, ProtoAckBodyType &data)
{
	CHECK_RET(warn_if_prop_not_set(jsnRetData, "HKOrderArr"), false);	

	const Json::Value &jsnHKOrderArr = jsnRetData["HKOrderArr"];
	CHECK_RET(jsnHKOrderArr.isArray(), false);

	bool bSuc = true;
	int nArrSize = jsnHKOrderArr.size();
	for ( int n = 0; n < nArrSize; n++ )
	{		
		QueryHKHisOrderAckItem item;
		VT_PROTO_FIELD vtField;
		GetHKOrderArrField(vtField, item);

		const Json::Value jsnItem = jsnHKOrderArr[n];
		CHECK_OP(!jsnItem.isNull(), continue);
		bSuc = CProtoParseBase::ParseProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
		data.vtHisOrder.push_back(item);
	}

	return bSuc;
}

bool CProtoQueryHKHisOrder::MakeHKOrderArr(Json::Value &jsnRetData, const ProtoAckBodyType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnRetData, "HKOrderArr"), false);	

	Json::Value &jsnHKOrderArr = jsnRetData["HKOrderArr"];	

	bool bSuc = true;
	for ( int n = 0; n < (int)data.vtHisOrder.size(); n++ )
	{
		const QueryHKHisOrderAckItem &item = data.vtHisOrder[n];
		VT_PROTO_FIELD vtField;
		GetHKOrderArrField(vtField, item);

		Json::Value &jsnItem = jsnHKOrderArr[n];
		bSuc = CProtoParseBase::MakeProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
	}

	return bSuc;
}

void CProtoQueryHKHisOrder::GetHKOrderArrField(VT_PROTO_FIELD &vtField, const QueryHKHisOrderAckItem &ackItem)
{
	static BOOL arOptional[] = {
		FALSE, FALSE,
		FALSE, FALSE, FALSE,
		FALSE, FALSE, FALSE,
		FALSE, FALSE,
		FALSE, FALSE, 
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int64, ProtoFild_Int32, 
		ProtoFild_Int32, ProtoFild_Int32, ProtoFild_StringW, 
		ProtoFild_StringW, ProtoFild_Int64, ProtoFild_Int64, 
		ProtoFild_Int64, ProtoFild_Int64, 
		ProtoFild_Int64, ProtoFild_Int32,
	};
	static LPCSTR arFieldKey[] = {
		"OrderID",		"OrderType", 
		"OrderSide",	"Status",		"StockCode", 
		"StockName",	"Price",		"Qty", 
		"DealtQty",		"SubmitedTime", 
		"UpdatedTime",  "ErrCode",
	};

	QueryHKHisOrderAckItem &item = const_cast<QueryHKHisOrderAckItem &>(ackItem);
	void *arPtr[] = {
		&item.nOrderID,		&item.nOrderType,
		&item.enSide,		&item.nStatus,		&item.strStockCode,
		&item.strStockName, &item.nPrice,		&item.nQty,			
		&item.nDealtQty,	&item.nSubmitedTime,
		&item.nUpdatedTime,	&item.nErrCode,
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	vtField.clear();
	PROTO_FIELD field;
	for ( int n = 0; n < _countof(arOptional); n++ )
	{
		field.bOptional = arOptional[n];
		field.eFieldType = arFieldType[n];
		field.strFieldKey = arFieldKey[n];
		switch (field.eFieldType)
		{
		case ProtoFild_Int32:
			field.pInt32 = (int*)arPtr[n];
			break;
		case ProtoFild_Int64:
			field.pInt64 = (INT64*)arPtr[n];
			break;
		case ProtoFild_StringA:
			field.pStrA = (std::string*)arPtr[n];
			break;
		case ProtoFild_StringW:
			field.pStrW = (std::wstring*)arPtr[n];
			break;
		default:
			CHECK_OP(FALSE, NOOP);
			break;
		}

		vtField.push_back(field);
	}	
}