#pragma once
#include "Include/FTPluginQuoteInterface.h"

struct StockMktCodeEx
{
	StockMktType eMarketType;
	char strCode[16];
	StockMktCodeEx()
	{
		memset(this, 0, sizeof(*this));
	}
	StockMktCodeEx(StockMktType eVal, const char* pstrCode)
	{
		eMarketType = eVal;
		memset(strCode, 0, sizeof(strCode));
		if (pstrCode)
		{
			strncpy(strCode, pstrCode, _countof(strCode) - 1);
		}
	}
	StockMktCodeEx(StockMktType eVal, const wchar_t* pwstrCode)
	{
		eMarketType = eVal;
		memset(strCode, 0, sizeof(strCode));
		if (pwstrCode)
		{
			strncpy(strCode, CW2A(pwstrCode), _countof(strCode) - 1);
		}
	}
	bool operator < (const StockMktCodeEx& stVal) const
	{
		if (strncmp(strCode, stVal.strCode, _countof(strCode)) == 0)
		{
			return eMarketType < stVal.eMarketType;
		}
		else
		{
			return  strncmp(strCode, stVal.strCode, _countof(strCode)) < 0;
		}
	}
};

class IFTStockUtilImpl;
class IFTStockUtil
{
public:
	IFTStockUtil();
	~IFTStockUtil();

	static void Init(IFTQuoteData* pQuoteData);
	static bool GetStockMktCode(INT64 nStockID, StockMktCodeEx& stMktCode);
	static int GetStockLotsize(INT64 nStockID, bool* pRet);

	static INT64 GetStockHashVal(const char* pstrCode, StockMktType eMktType);
	static INT64 GetStockHashVal(const wchar_t* pwstrCode, StockMktType eMktType);

	//通过前后缀分辩市场
	//注意期货市场无法通过该前后缀分辨
	static StockMktType GetStockMktType(const char* pstrMktSuffix);
	static StockMktType GetStockMktType(const wchar_t* pwstrMktSuffix);

	static void Uninit();

private:
	static IFTStockUtilImpl* m_pImpl;
 
};
