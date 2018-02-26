#include "stdafx.h"
#include "ProtoHisKLPoints.h"
#include "CppJsonConv.h"
#include "../JsonCpp/json_op.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CProtoHisKLPoints::CProtoHisKLPoints()
{
	m_pReqData = NULL;
	m_pAckData = NULL;
}

CProtoHisKLPoints::~CProtoHisKLPoints()
{

}

bool CProtoHisKLPoints::ParseJson_Req(const Json::Value &jsnVal)
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

bool CProtoHisKLPoints::ParseJson_Ack(const Json::Value &jsnVal)
{
	return false;
}

bool CProtoHisKLPoints::MakeJson_Req(Json::Value &jsnVal)
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

bool CProtoHisKLPoints::MakeJson_Ack(Json::Value &jsnVal)
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

void CProtoHisKLPoints::SetProtoData_Req(ProtoReqDataType *pData)
{
	m_pReqData = pData;
}

void CProtoHisKLPoints::SetProtoData_Ack(ProtoAckDataType *pData)
{
	m_pAckData = pData;
}

void CProtoHisKLPoints::SetHisKLPointsArrFieldFilter(std::set<KLDataField> setFilter)
{
	m_setFieldFilter = setFilter;
}

//tomodify 3(数组等复杂结构或单层的结构体)
bool CProtoHisKLPoints::ParseProtoBody_Req(const Json::Value &jsnVal, ProtoReqDataType &data)
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
bool CProtoHisKLPoints::MakeProtoBody_Req(Json::Value &jsnVal, const ProtoReqDataType &data)
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
bool CProtoHisKLPoints::MakeProtoBody_Ack(Json::Value &jsnVal, const ProtoAckDataType &data)
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
		bSuc &= MakeHisKLPointsArr(jsnBody, data.body);
	}

	return bSuc;
}

//tomodify 7
void CProtoHisKLPoints::GetProtoBodyField_Req(VT_PROTO_FIELD &vtField, const ProtoReqBodyType &reqData)
{
	static BOOL arOptional[] = {
		TRUE, FALSE, FALSE, 
		FALSE, FALSE, 
		FALSE, FALSE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_Int32, ProtoFild_Int32,
		ProtoFild_Int32, ProtoFild_Int32,
		ProtoFild_StringA, ProtoFild_StringA,
	};
	static LPCSTR arFieldKey[] = {
		"Cookie", "RehabType",  "KLType",  
		"MaxKLNum", "NoDataMode",
		"TimePoints", "NeedKLData",
	};

	ProtoReqBodyType &body = const_cast<ProtoReqBodyType &>(reqData);
	void *arPtr[] = {
		&body.nCookie, &body.nRehabType, &body.nKLType,
		&body.nMaxAckKLItemNum, &body.nNodataMode, 
		&body.strTimePoints, &body.strNeedKLData,
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
void CProtoHisKLPoints::GetProtoBodyField_Ack(VT_PROTO_FIELD &vtField, const ProtoAckBodyType &ackData)
{
	static BOOL arOptional[] = {
		FALSE, FALSE, 
		FALSE, FALSE,
		FALSE, FALSE,
		FALSE, FALSE,
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_Int32, ProtoFild_Int32, 
		ProtoFild_Int32, ProtoFild_Int32,
		ProtoFild_Int32, ProtoFild_Int32,
		ProtoFild_StringA, ProtoFild_StringA,
	};
	static LPCSTR arFieldKey[] = {		
		"Cookie", "HasNext",
		"RehabType", "KLType",
		"MaxKLNum", "NoDataMode",
		"TimePoints", "NeedKLData",
	};

	const void *arPtr[] = {
		&ackData.nCookie, &ackData.nHasNext,
		&ackData.nRehabType, &ackData.nKLType,
		&ackData.nMaxAckKLItemNum, &ackData.nNodataMode,
		&ackData.strTimePoints, &ackData.strNeedKLData,
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

bool CProtoHisKLPoints::MakeHisKLPointsArr(Json::Value &jsnRetData, const ProtoAckBodyType &data)
{
	CHECK_RET(warn_if_prop_exists(jsnRetData, "StockHistoryKLArr"), false);	

	Json::Value &jsnHisKLPointsArr = jsnRetData["StockHistoryKLArr"];	

	bool bSuc = true;
	for (int n = 0; n < (int)data.vtHisKL.size(); n++)
	{
		const HISKL &item = data.vtHisKL[n];
		VT_PROTO_FIELD vtField;
		GetHisKLPointsArrField(vtField, item);

		Json::Value &jsnItem = jsnHisKLPointsArr[n];
		bSuc = CProtoParseBase::MakeProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);

		bSuc &= MakeHisKLItem(jsnItem, item);
		CHECK_OP(bSuc, break);
	}

	return bSuc;
}

bool CProtoHisKLPoints::MakeHisKLItem(Json::Value &jsnRetData, const HISKL &data)
{
	CHECK_RET(warn_if_prop_exists(jsnRetData, "HistoryKLArr"), false);

	Json::Value &jsnHisKLPointsArr = jsnRetData["HistoryKLArr"];

	bool bSuc = true;
	for (int n = 0; n < (int)data.vtKL.size(); n++)
	{
		const HisKLPointsAckItem &item = data.vtKL[n];
		VT_PROTO_FIELD vtField;
		GetHisKLItemField(vtField, item);

		Json::Value &jsnItem = jsnHisKLPointsArr[n];
		bSuc = CProtoParseBase::MakeProtoFields(jsnItem, vtField);
		CHECK_OP(bSuc, break);
	}

	return bSuc;
}

void CProtoHisKLPoints::GetHisKLPointsArrField(VT_PROTO_FIELD &vtField, const HISKL &data)
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

void CProtoHisKLPoints::GetHisKLItemField(VT_PROTO_FIELD &vtField, const HisKLPointsAckItem &ackItem)
{
	static BOOL arOptional[] = {
		FALSE, FALSE, FALSE, FALSE,
		FALSE, FALSE, FALSE,
		FALSE, FALSE, FALSE,
		FALSE, FALSE
	};
	static EProtoFildType arFieldType[] = {
		ProtoFild_StringW, ProtoFild_StringW, ProtoFild_Int32, ProtoFild_Int64,
		ProtoFild_Int64, ProtoFild_Int64, ProtoFild_Int64,
		ProtoFild_Int32, ProtoFild_Int32, ProtoFild_Int64,
		ProtoFild_Int64, ProtoFild_Int32,
	};
	static LPCSTR arFieldKey[] = {
		"TimePoint", "Time", "DataValid", "Open",
		"Close", "High", "Low",
		"PERatio", "TurnoverRate", "Volume",
		"Turnover", "ChangeRate", 
	};

	static KLDataField arDataFidle[] = {
		KLDataField_NoFilter, KLDataField_TimeStr, KLDataField_NoFilter, KLDataField_Open,
		KLDataField_Close, KLDataField_Highest, KLDataField_Lowest,
		KLDataField_PERate, KLDataField_TurnoverRate, KLDataField_TDVol,
		KLDataField_TDVal, KLDataField_RaiseRate,
	};

	const void *arPtr[] = {
		&ackItem.wstrTimePoint, &ackItem.wstrTime, &ackItem.nDataValid, &ackItem.nOpenPrice,
		&ackItem.nClosePrice, &ackItem.nHighestPrice, &ackItem.nLowestPrice,
		&ackItem.nPERatio, &ackItem.nTurnoverRate, &ackItem.ddwTDVol,
		&ackItem.ddwTDVal, &ackItem.nRaiseRate,
	};

	CHECK_OP(_countof(arOptional) == _countof(arFieldType), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arFieldKey), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arPtr), NOOP);
	CHECK_OP(_countof(arOptional) == _countof(arDataFidle), NOOP);

	vtField.clear();
	PROTO_FIELD field;
	for ( int n = 0; n < _countof(arOptional); n++ )
	{
		KLDataField eDataField = arDataFidle[n];
		if (eDataField != KLDataField_NoFilter &&
			!m_setFieldFilter.empty() &&
			m_setFieldFilter.count(eDataField) == 0)
		{
			continue;
		}

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


bool CProtoHisKLPoints::ParseStockArr_Req(const Json::Value &jsnRetData, ProtoReqBodyType &data)
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

bool CProtoHisKLPoints::MakeStockArr_Req(Json::Value &jsnRetData, const ProtoReqBodyType &data)
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

bool CProtoHisKLPoints::ParseStockArr_Ack(const Json::Value &jsnRetData, ProtoAckBodyType &data)
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

bool CProtoHisKLPoints::MakeStockArr_Ack(Json::Value &jsnRetData, const ProtoAckBodyType &data)
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

void CProtoHisKLPoints::GetStockArrField(VT_PROTO_FIELD &vtField, const ReqParamStockItem &ackItem)
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
