#include "stdafx.h"
#include "ProtoSuspend.h"
#include "CppJsonConv.h"
#include "../JsonCpp/json_op.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CProtoSuspend::CProtoSuspend()
{
	m_pReqData = NULL;
	m_pAckData = NULL;
}

CProtoSuspend::~CProtoSuspend()
{

}

bool CProtoSuspend::ParseJson_Req(const Json::Value &jsnVal)
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

bool CProtoSuspend::ParseJson_Ack(const Json::Value &jsnVal)
{
	return false;
}

bool CProtoSuspend::MakeJson_Req(Json::Value &jsnVal)
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

bool CProtoSuspend::MakeJson_Ack(Json::Value &jsnVal)
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

void CProtoSuspend::SetProtoData_Req(ProtoReqDataType *pData)
{
	m_pReqData = pData;
}

void CProtoSuspend::SetProtoData_Ack(ProtoAckDataType *pData)
{
	m_pAckData = pData;
}

//tomodify 3(数组等复杂结构或单层的结构体)
bool CProtoSuspend::ParseProtoBody_Req(const Json::Value &jsnVal, ProtoReqDataType &data)
{	
	if ( !warn_if_prop_not_set(jsnVal, KEY_REQ_PARAM) )
		return true;

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Req(vtField, data.body);

	const Json::Value &jsnBody = jsnVal[KEY_REQ_PARAM];
	bool bSuc = CProtoParseBase::ParseProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);
	
	if (bSuc)
	{
		bSuc &= ParseStockArr_Req(jsnBody, data.body);
	}
	return true;
}

//tomodify 5(数组等复杂结构或单层的结构体)
bool CProtoSuspend::MakeProtoBody_Req(Json::Value &jsnVal, const ProtoReqDataType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnVal, KEY_REQ_PARAM), false);

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Req(vtField, data.body);

	Json::Value &jsnBody = jsnVal[KEY_REQ_PARAM];
	bool bSuc = CProtoParseBase::MakeProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	if (bSuc)
	{
		bSuc &= MakeStockArr_Req(jsnBody, data.body);
	}
	return bSuc;
}

//tomodify 6(数组等复杂结构或单层的结构体)
bool CProtoSuspend::MakeProtoBody_Ack(Json::Value &jsnVal, const ProtoAckDataType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnVal, KEY_ACK_DATA), false);	

	VT_PROTO_FIELD vtField;
	GetProtoBodyField_Ack(vtField, data.body);

	Json::Value &jsnBody = jsnVal[KEY_ACK_DATA];
	bool bSuc = CProtoParseBase::MakeProtoFields(jsnBody, vtField);
	CHECK_OP(bSuc, NOOP);

	if (bSuc)
	{
		bSuc &= MakeStockArr_Ack(jsnBody, data.body);
	}

	if ( bSuc )
	{
		bSuc &= MakeStockSuspendArr(jsnBody, data.body);
	}

	return bSuc;
}

//tomodify 7
void CProtoSuspend::GetProtoBodyField_Req(VT_PROTO_FIELD &vtField, const ProtoReqBodyType &reqData)
{
	static BOOL arOptional[] = {
		TRUE, FALSE, FALSE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_StringA, ProtoFild_StringA,
	};
	static LPCSTR arFieldKey[] = {
		"Cookie", "start_date", "end_date",  
	};

	const void *arPtr[] = {
		&reqData.nCookie, &reqData.strStartDate, &reqData.strEndDate,
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
void CProtoSuspend::GetProtoBodyField_Ack(VT_PROTO_FIELD &vtField, const ProtoAckBodyType &ackData)
{
	static BOOL arOptional[] = {
		FALSE, FALSE, FALSE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_StringA, ProtoFild_StringA,
	};
	static LPCSTR arFieldKey[] = {
		"Cookie", "start_date", "end_date",
	};

	const void *arPtr[] = {
		&ackData.nCookie, &ackData.strStartDate, &ackData.strEndDate,
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

bool CProtoSuspend::MakeStockSuspendArr(Json::Value &jsnRetData, const ProtoAckBodyType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnRetData, "StockSuspendArr"), false);	

	Json::Value &jsnSuspendArr = jsnRetData["StockSuspendArr"];	

	bool bSuc = true;
	for (int n = 0; n < (int)data.vtStockSupend.size(); n++)
	{
		const SuspendAckItem &item = data.vtStockSupend[n];
		VT_PROTO_FIELD vtField;
		GetSuspendArrField(vtField, item);

		Json::Value &jsnItem = jsnSuspendArr[n];
		bSuc = CProtoParseBase::MakeProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);

		bSuc &= MakeSuspendArr(jsnItem, item);
		CHECK_OP(bSuc, break);
	}

	return bSuc;
}

bool CProtoSuspend::MakeSuspendArr(Json::Value &jsnRetData, const SuspendAckItem &data)
{
	CHECK_RET(warn_if_prop_exists(jsnRetData, "SuspendArr"), false);

	Json::Value &jsnSuspendArr = jsnRetData["SuspendArr"];

	bool bSuc = true;
	for (int n = 0; n < (int)data.vtSupend.size(); n++)
	{
		const SuspendInfoItem &item = data.vtSupend[n];
		VT_PROTO_FIELD vtField;
		GetSuspendItemField(vtField, item);

		Json::Value &jsnItem = jsnSuspendArr[n];
		bSuc = CProtoParseBase::MakeProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
	}

	return bSuc;
}

void CProtoSuspend::GetSuspendArrField(VT_PROTO_FIELD &vtField, const SuspendAckItem &data)
{
	static BOOL arOptional[] = {
		FALSE, FALSE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_StringA,
	};
	static LPCSTR arFieldKey[] = {
		"Market", "StockCode",
	};

	const void *arPtr[] = {
		&data.nStockMarket, &data.strStockCode,
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	vtField.clear();
	PROTO_FIELD field;
	for (int n = 0; n < _countof(arOptional); n++)
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

void CProtoSuspend::GetSuspendItemField(VT_PROTO_FIELD &vtField, const SuspendInfoItem &ackItem)
{
	static BOOL arOptional[] = {
		FALSE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_StringA,
	};
	static LPCSTR arFieldKey[] = {
		"SuspendTime", 
	};

	const void *arPtr[] = {
		&ackItem.strSuspendTime,
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

bool CProtoSuspend::ParseStockArr_Req(const Json::Value &jsnRetData, ProtoReqBodyType &data)
{
	CHECK_RET(warn_if_prop_not_set(jsnRetData, "StockArr"), false);
	const Json::Value &jsnStockArr = jsnRetData["StockArr"];
	CHECK_RET(jsnStockArr.isArray(), false);
	bool bSuc = true;
	for (int n = 0; n < (int)jsnStockArr.size(); n++)
	{
		ReqParamStockItem item;
		VT_PROTO_FIELD vtField;
		GetStockArrField(vtField, item);

		const Json::Value jsnItem = jsnStockArr[n];
		CHECK_OP(!jsnItem.isNull(), continue);
		bSuc = CProtoParseBase::ParseProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
		data.vtStock.push_back(item);
	}

	return bSuc;
}

bool CProtoSuspend::MakeStockArr_Req(Json::Value &jsnRetData, const ProtoReqBodyType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnRetData, "StockArr"), false);

	Json::Value &jsnSuspendArr = jsnRetData["StockArr"];

	bool bSuc = true;
	for (int n = 0; n < (int)data.vtStock.size(); n++)
	{
		const ReqParamStockItem &item = data.vtStock[n];
		VT_PROTO_FIELD vtField;
		GetStockArrField(vtField, item);

		Json::Value &jsnItem = jsnSuspendArr[n];
		bSuc = CProtoParseBase::MakeProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
	}

	return bSuc;
}

bool CProtoSuspend::ParseStockArr_Ack(const Json::Value &jsnRetData, ProtoAckBodyType &data)
{
	CHECK_RET(warn_if_prop_not_set(jsnRetData, "StockArr"), false);
	const Json::Value &jsnStockArr = jsnRetData["StockArr"];
	CHECK_RET(jsnStockArr.isArray(), false);
	bool bSuc = true;
	for (int n = 0; n < (int)jsnStockArr.size(); n++)
	{
		ReqParamStockItem item;
		VT_PROTO_FIELD vtField;
		GetStockArrField(vtField, item);

		const Json::Value jsnItem = jsnStockArr[n];
		CHECK_OP(!jsnItem.isNull(), continue);
		bSuc = CProtoParseBase::ParseProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
		data.vtStock.push_back(item);
	}

	return bSuc;
}

bool CProtoSuspend::MakeStockArr_Ack(Json::Value &jsnRetData, const ProtoAckBodyType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnRetData, "StockArr"), false);

	Json::Value &jsnSuspendArr = jsnRetData["StockArr"];

	bool bSuc = true;
	for (int n = 0; n < (int)data.vtStock.size(); n++)
	{
		const ReqParamStockItem &item = data.vtStock[n];
		VT_PROTO_FIELD vtField;
		GetStockArrField(vtField, item);

		Json::Value &jsnItem = jsnSuspendArr[n];
		bSuc = CProtoParseBase::MakeProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
	}

	return bSuc;
}

void CProtoSuspend::GetStockArrField(VT_PROTO_FIELD &vtField, const ReqParamStockItem &ackItem)
{
	static BOOL arOptional[] = {
		FALSE, FALSE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_StringA,
	};
	static LPCSTR arFieldKey[] = {
		"Market", "StockCode"
	};

	const void *arPtr[] = {
		&ackItem.nStockMarket, &ackItem.strStockCode,
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);

	vtField.clear();
	PROTO_FIELD field;
	for (int n = 0; n < _countof(arOptional); n++)
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
