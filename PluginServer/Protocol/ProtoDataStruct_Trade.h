#pragma once
#include <vector>
#include <set>
#include <map>
#include "ProtoDataStruct.h"

//////////////////////////////////////////////////////////////////////////
//���͸۹ɶ���������Ϣ, PROTO_ID_TDHK_PUSH_ORDER_ERROR
struct	OrderErrorPushHKReqBody
{
};

struct OrderErrorPushHKAckBody
{	
	int nEnvType;
	INT64 nOrderID;
	int   nOrderErrNotifyHK;
	int	  nOrderErrCode;
	std::string  strOrderErrDesc;
	
	OrderErrorPushHKAckBody()
	{
		nEnvType = 0;
		nOrderID = 0;
		nOrderErrNotifyHK = 0;
		nOrderErrCode = 0;
	}
};

struct	OrderErrorPushHK_Req
{
	ProtoHead				head;
	OrderErrorPushHKReqBody	body;
};

struct	OrderErrorPushHK_Ack
{
	ProtoHead				head;
	OrderErrorPushHKAckBody	body;
};


//////////////////////////////////////////////////////////////////////////
//�¶��� PROTO_ID_TDHK_PLACE_ORDER 
struct	PlaceOrderReqBody
{
	int nEnvType;
	int nCookie;
	int nOrderDir;
	int nOrderType;
	int nPrice;
	INT64 nQty;
	std::string strCode;

	PlaceOrderReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
		nOrderDir = 0;
		nOrderType = 0;
		nPrice = 0;
		nQty = 0;
	}
};

struct PlaceOrderAckBody
{	
	int nEnvType;
	int nCookie;
	INT64 nLocalID;
	int nSvrResult;	
	INT64	nSvrOrderID;

	int nOrderType; //��ͬ�г���ȡֵ��Ӧ�����ö�ٶ��� Trade_OrderType_HK �� Trade_OrderType_US
	int enSide; //Trade_OrderSide
	int nStatus; //ȡֵ��Ӧ�����ö�ٶ���Trade_OrderStatus
	std::wstring strStockCode;
	std::wstring strStockName;
	INT64 nPrice;
	INT64 nQty;
	INT64 nDealtQty; //�ɽ�����
	int nDealtAvgPrice;

	INT64 nSubmitedTime; //�������յ��Ķ����ύʱ��
	INT64 nUpdatedTime; //���������µ�ʱ��

	int   nErrCode; //�����룬��֧�ָ۹�

	PlaceOrderAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
		nLocalID = 0;
		nSvrResult = 0;
		nSvrOrderID = 0;

		nOrderType = 0;
		enSide = 0;
		nStatus = 0;
		nPrice = 0;
		nQty = 0;
		nDealtQty = 0;
		nDealtAvgPrice = 0;
		nSubmitedTime = 0;
		nUpdatedTime = 0;
		nErrCode = 0;
	}
};

struct	PlaceOrder_Req
{
	ProtoHead			head;
	PlaceOrderReqBody	body;
};

struct	PlaceOrder_Ack
{
	ProtoHead				head;
	PlaceOrderAckBody		body;
};


//////////////////////////////////////////////////////////////////////////
//���ö���״̬ PROTO_ID_TDHK_SET_ORDER_STATUS
struct	SetOrderStatusReqBody
{
	int		nEnvType;
	int		nCookie;
	int		nSetOrderStatus;
	INT64	nSvrOrderID;
	INT64	nLocalOrderID;

	SetOrderStatusReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
		nSetOrderStatus = 0;
		nSvrOrderID = 0;
		nLocalOrderID = 0;
	}

};

struct SetOrderStatusAckBody
{	
	int		nEnvType;
	int		nCookie;
	INT64	nSvrOrderID;
	INT64	nLocalOrderID;
	int		nSvrResult;	

	SetOrderStatusAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
		nSvrOrderID = 0;
		nLocalOrderID = 0;
		nSvrResult = 0;
	}
};

struct	SetOrderStatus_Req
{
	ProtoHead				head;
	SetOrderStatusReqBody	body;
};

struct	SetOrderStatus_Ack
{
	ProtoHead				head;
	SetOrderStatusAckBody	body;
};


//////////////////////////////////////////////////////////////////////////
//��������
struct UnlockTradeReqBody
{
	int			nCookie;
	std::string strPasswd;

	UnlockTradeReqBody()
	{
		nCookie = 0;
	}
};

struct UnlockTradeAckBody
{
	int	nCookie;
	int nSvrResult;	
	std::string strSecNum;

	UnlockTradeAckBody()
	{
		nCookie = 0;
		nSvrResult = 0;
	}
};

struct UnlockTrade_Req
{
	ProtoHead				head;
	UnlockTradeReqBody		body;
};

struct UnlockTrade_Ack
{
	ProtoHead				head;
	UnlockTradeAckBody		body;
};


//////////////////////////////////////////////////////////////////////////
//�۹ɸĵ� PROTO_ID_TDHK_CHANGE_ORDER
struct	ChangeOrderReqBody
{
	int		nEnvType;
	int		nCookie;
	INT64	nSvrOrderID;
	INT64	nLocalOrderID;
	int		nPrice;
	INT64	nQty;

	ChangeOrderReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
		nSvrOrderID = 0;
		nLocalOrderID = 0;
		nPrice = 0;
		nQty = 0;
	}
};

struct ChangeOrderAckBody
{	
	int		nEnvType;
	int		nCookie;
	INT64	nSvrOrderID;
	INT64	nLocalOrderID;
	int		nSvrResult;	

	ChangeOrderAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
		nSvrOrderID = 0;
		nLocalOrderID = 0;
		nSvrResult = 0;
	}
};

struct	ChangeOrder_Req
{
	ProtoHead			head;
	ChangeOrderReqBody	body;
};

struct	ChangeOrder_Ack
{
	ProtoHead			head;
	ChangeOrderAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//��ȡ�û��۹��ʻ���Ϣ
struct	QueryHKAccInfoReqBody
{
	int		nEnvType;
	int		nCookie;	

	QueryHKAccInfoReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryHKAccInfoAckBody
{	
	int		nEnvType;
	int		nCookie;

	//������ Trade_AccInfo ͬ��
	INT64 nPower; //������
	INT64 nZcjz; //�ʲ���ֵ
	INT64 nZqsz; //֤ȯ��ֵ
	INT64 nXjjy; //�ֽ����
	INT64 nKqxj; //��ȡ�ֽ�
	INT64 nDjzj; //�����ʽ�
	INT64 nZsje; //׷�ս��

	INT64 nZgjde; //��߽����
	INT64 nYyjde; //�����Ŵ���
	INT64 nGpbzj; //��Ʊ��֤��

	QueryHKAccInfoAckBody()
	{
		nEnvType = 0;
		nCookie = 0;

		nPower = 0;
		nZcjz = 0;
		nZqsz = 0;
		nXjjy = 0;
		nKqxj = 0;
		nDjzj = 0;
		nZsje = 0;

		nZgjde = 0;
		nYyjde = 0;
		nGpbzj = 0;
	}
};

struct	QueryHKAccInfo_Req
{
	ProtoHead				head;
	QueryHKAccInfoReqBody	body;
};

struct	QueryHKAccInfo_Ack
{
	ProtoHead				head;
	QueryHKAccInfoAckBody	body;
};



//////////////////////////////////////////////////////////////////////////
//��ȡ�û������ʻ���Ϣ
struct	QueryUSAccInfoReqBody
{
	int		nEnvType;
	int		nCookie;	

	QueryUSAccInfoReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryUSAccInfoAckBody
{	
	int		nEnvType;
	int		nCookie;

	//������ Trade_AccInfo ͬ��
	INT64 nPower; //������
	INT64 nZcjz; //�ʲ���ֵ
	INT64 nZqsz; //֤ȯ��ֵ
	INT64 nXjjy; //�ֽ����
	INT64 nKqxj; //��ȡ�ֽ�
	INT64 nDjzj; //�����ʽ�
	INT64 nZsje; //׷�ս��

	INT64 nZgjde; //��߽����
	INT64 nYyjde; //�����Ŵ���
	INT64 nGpbzj; //��Ʊ��֤��

	QueryUSAccInfoAckBody()
	{
		nEnvType = 0;
		nCookie = 0;

		nPower = 0;
		nZcjz = 0;
		nZqsz = 0;
		nXjjy = 0;
		nKqxj = 0;
		nDjzj = 0;
		nZsje = 0;

		nZgjde = 0;
		nYyjde = 0;
		nGpbzj = 0;
	}
};

struct	QueryUSAccInfo_Req
{
	ProtoHead				head;
	QueryUSAccInfoReqBody	body;
};

struct	QueryUSAccInfo_Ack
{
	ProtoHead				head;
	QueryUSAccInfoAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//��ѯ���и۹ɶ���
struct	QueryHKOrderReqBody
{
	int		nEnvType;
	int		nCookie;
	INT64	nOrderID;
	std::string strStatusFilter; //״̬�����ַ����� ��","�ŷָ�����"0,1,2"
	std::string strStockCode;
	std::string strStartTime;
	std::string strEndTime;

	QueryHKOrderReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
		nOrderID = 0;
	}
};

//�� Trade_OrderItem ͬ��
struct QueryHKOrderAckItem
{
	INT64 nLocalID; //�ͻ��˲����Ķ���ID���Ƕ���������ID�����ڹ���
	INT64 nOrderID; //�����ţ������������Ķ���������ID

	int nOrderType; //��ͬ�г���ȡֵ��Ӧ�����ö�ٶ��� Trade_OrderType_HK �� Trade_OrderType_US
	int/*Trade_OrderSide*/ enSide;
	int nStatus; //ȡֵ��Ӧ�����ö�ٶ���Trade_OrderStatus
	std::wstring strStockCode;
	std::wstring strStockName;	
	INT64 nPrice;
	INT64 nQty;
	INT64 nDealtQty; //�ɽ�����
	int nDealtAvgPrice; 

	INT64 nSubmitedTime; //�������յ��Ķ����ύʱ��
	INT64 nUpdatedTime; //���������µ�ʱ��

	int   nErrCode; //�����룬��֧�ָ۹�

	QueryHKOrderAckItem()
	{
		nLocalID = 0;
		nOrderID = 0;

		nOrderType = 0;
		nStatus = 0;
		nPrice = 0;
		nQty = 0;
		nDealtQty = 0;
		nDealtAvgPrice = 0;

		nSubmitedTime = 0;
		nUpdatedTime = 0;

		nErrCode = 0;
	}
};

typedef std::vector<QueryHKOrderAckItem>	VT_HK_ORDER;

struct QueryHKOrderAckBody
{	
	int		nEnvType;
	int		nCookie;
	VT_HK_ORDER vtOrder;

	QueryHKOrderAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct	QueryHKOrder_Req
{
	ProtoHead			head;
	QueryHKOrderReqBody	body;
};

struct	QueryHKOrder_Ack
{
	ProtoHead			head;
	QueryHKOrderAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//��ѯ�������ɶ���
struct	QueryUSOrderReqBody
{
	int		nEnvType;
	int		nCookie;
	INT64	nOrderID;
	std::string strStatusFilter; //״̬�����ַ����� ��","�ŷָ�����"0,1,2"
	std::string strStockCode;
	std::string strStartTime;
	std::string strEndTime;

	QueryUSOrderReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
		nOrderID = 0;
	}
};

//�� Trade_OrderItem ͬ��_
struct QueryUSOrderAckItem
{
	INT64 nLocalID; //�ͻ��˲����Ķ���ID���Ƕ���������ID�����ڹ���
	INT64 nOrderID; //�����ţ������������Ķ���������ID

	int nOrderType; //��ͬ�г���ȡֵ��Ӧ�����ö�ٶ��� Trade_OrderType_US �� Trade_OrderType_US
	int enSide; //Trade_OrderSide
	int nStatus; //ȡֵ��Ӧ�����ö�ٶ���Trade_OrderStatus
	std::wstring strStockCode;
	std::wstring strStockName;	
	INT64 nPrice;
	INT64 nQty;
	INT64 nDealtQty; //�ɽ�����
	int   nDealtAvgPrice; 

	INT64 nSubmitedTime; //�������յ��Ķ����ύʱ��
	INT64 nUpdatedTime; //���������µ�ʱ��

	int   nErrCode; 

	QueryUSOrderAckItem()
	{
		nLocalID = 0;
		nOrderID = 0;

		nOrderType = 0;
		nStatus = 0;
		nPrice = 0;
		nQty = 0;
		nDealtQty = 0;
		nDealtAvgPrice = 0;

		nSubmitedTime = 0;
		nUpdatedTime = 0;

		nErrCode = 0;
	}
};

typedef std::vector<QueryUSOrderAckItem>	VT_US_ORDER;

struct QueryUSOrderAckBody
{	
	int		nEnvType;
	int		nCookie;
	VT_US_ORDER vtOrder;

	QueryUSOrderAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct	QueryUSOrder_Req
{
	ProtoHead			head;
	QueryUSOrderReqBody	body;
};

struct	QueryUSOrder_Ack
{
	ProtoHead			head;
	QueryUSOrderAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//��ѯ�ֲ��б�
struct	QueryPositionReqBody
{
	int		nEnvType;
	int		nCookie;
	std::string strStockType;
	std::string strStockCode;
	std::string strPLRatioMin;
	std::string strPLRatioMax;

	QueryPositionReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

//�� Trade_PositionItem ͬ��_
struct QueryPositionAckItem
{
	std::wstring strStockCode;
	std::wstring strStockName;	

	INT64 nQty; //��������
	INT64 nCanSellQty; //��������
	INT64 nNominalPrice; //�м�
	INT64 nMarketVal; //��ֵ

	int  nCostPrice; //�ɱ���
	int  nCostPriceValid; //�ɱ����Ƿ���Ч
	INT64 nPLVal; //ӯ�����
	int  nPLValValid; //ӯ������Ƿ���Ч
	int nPLRatio; //ӯ������
	int nPLRatioValid; //ӯ�������Ƿ���Ч

	INT64 nToday_PLVal; //����ӯ�����
	INT64 nToday_BuyQty; //��������ɽ���
	INT64 nToday_BuyVal; //��������ɽ���
	INT64 nToday_SellQty; //���������ɽ���
	INT64 nToday_SellVal; //���������ɽ���

	QueryPositionAckItem()
	{
		nQty = 0;
		nCanSellQty = 0;
		nNominalPrice = 0;
		nMarketVal = 0;

		nCostPrice = 0;
		nCostPriceValid = 0;
		nPLVal = 0;
		nPLValValid = 0;
		nPLRatio = 0;
		nPLRatioValid = 0;

		nToday_PLVal = 0;
		nToday_BuyQty = 0;
		nToday_BuyVal = 0;
		nToday_SellQty = 0;
		nToday_SellVal = 0;
	}
};

typedef std::vector<QueryPositionAckItem>	VT_Position;

struct QueryPositionAckBody
{	
	int		nEnvType;
	int		nCookie;
	VT_Position  vtPosition;

	QueryPositionAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryPosition_Req
{
	ProtoHead				head;
	QueryPositionReqBody	body;
};

struct QueryPosition_Ack
{
	ProtoHead				head;
	QueryPositionAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//��ѯ���и۹ɳɽ���¼
struct QueryHKDealReqBody
{
	int		nEnvType;
	int		nCookie;

	QueryHKDealReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

//�� Trade_DealItem ͬ��
struct QueryHKDealAckItem
{
	//�ر����ѣ�������������������������������������������������������������
	//����API�м۸񡢽�����������Ϊ�����ͣ�����ԭʼ����û�б��Ŵ��������ͣ����Ǹ���ֵ��1000������С��λ��0.001Ԫ

	INT64 nOrderID; //�����ţ������������Ķ���������ID
	INT64 nDealID; //�ɽ���

	int enSide; //����

	std::wstring strStockCode;
	std::wstring strStockName;	
	INT64 nPrice; //�ɽ��۸�
	INT64 nQty; //�ɽ�����

	INT64 nTime;	//�ɽ�ʱ��

	int	nContraBrokerID;
	std::wstring strContraBrokerName;

	QueryHKDealAckItem()
	{
		nOrderID = 0;
		nDealID = 0;

		enSide = 0;
		nPrice = 0;
		nQty = 0;
		nTime = 0;

		nContraBrokerID = 0;
	}
};

typedef std::vector<QueryHKDealAckItem>	VT_HK_Deal;

struct QueryHKDealAckBody
{	
	int		nEnvType;
	int		nCookie;
	VT_HK_Deal vtDeal;

	QueryHKDealAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryHKDeal_Req
{
	ProtoHead			head;
	QueryHKDealReqBody	body;
};

struct QueryHKDeal_Ack
{
	ProtoHead			head;
	QueryHKDealAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//��ѯ�������ɳɽ���¼
struct QueryUSDealReqBody
{
	int		nEnvType;
	int		nCookie;

	QueryUSDealReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

//�� Trade_DealItem ͬ��
struct QueryUSDealAckItem
{
	//�ر����ѣ�������������������������������������������������������������
	//����API�м۸񡢽�����������Ϊ�����ͣ�����ԭʼ����û�б��Ŵ��������ͣ����Ǹ���ֵ��1000������С��λ��0.001Ԫ

	INT64 nOrderID; //�����ţ������������Ķ���������ID
	INT64 nDealID; //�ɽ���

	int enSide; //����

	std::wstring strStockCode;
	std::wstring strStockName;	

	INT64 nPrice; //�ɽ��۸�
	INT64 nQty; //�ɽ�����

	INT64 nTime;	//�ɽ�ʱ��

	int	nContraBrokerID;
	std::wstring strContraBrokerName;
	QueryUSDealAckItem()
	{
		nOrderID = 0;
		nDealID = 0;

		enSide = 0;

		nPrice = 0;
		nQty = 0;

		nTime = 0;

		nContraBrokerID = 0;
	}
};

typedef std::vector<QueryUSDealAckItem>	VT_US_Deal;

struct QueryUSDealAckBody
{	
	int		nEnvType;
	int		nCookie;
	VT_US_Deal vtDeal;

	QueryUSDealAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryUSDeal_Req
{
	ProtoHead			head;
	QueryUSDealReqBody	body;
};

struct QueryUSDeal_Ack
{
	ProtoHead			head;
	QueryUSDealAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//���ĸ۹ɹɶ������ɽ�����
struct SubHKOrderDealReqBody
{
	int		nEnvType;
	int		nCookie;
	std::string strOrderID;
	int		nSubOrder;
	int		nSubDeal;
	int		nFirstPush;
	SubHKOrderDealReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
		nSubOrder = 0;
		nSubDeal = 0;
	}
};

struct SubHKOrderDealAckBody
{
	int		nEnvType;
	int		nCookie;
	std::string strOrderID;
	std::string strSubOrderSuc;
	std::string strSubDealSuc;
	SubHKOrderDealAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct SubHKOrderDeal_Req
{
	ProtoHead				head;
	SubHKOrderDealReqBody	body;
};

struct SubHKOrderDeal_Ack
{
	ProtoHead				head;
	SubHKOrderDealAckBody	body;
};

struct PushHKOrderReqBody
{
};

typedef QueryHKOrderAckItem	PushHKOrderAckItem;

struct PushHKOrderAckBody
{
	int	nEnvType;
	PushHKOrderAckItem HKOrderItem;

	PushHKOrderAckBody()
	{
		nEnvType = 0;
	}

	bool Equal(const PushHKOrderAckBody& AckBody)
	{
		bool bEqual = (nEnvType == AckBody.nEnvType);
		bEqual &= (HKOrderItem.nOrderID == AckBody.HKOrderItem.nOrderID);
		bEqual &= (HKOrderItem.nOrderType == AckBody.HKOrderItem.nOrderType);
		bEqual &= (HKOrderItem.nStatus == AckBody.HKOrderItem.nStatus);
		bEqual &= (HKOrderItem.nPrice == AckBody.HKOrderItem.nPrice);
		bEqual &= (HKOrderItem.nQty == AckBody.HKOrderItem.nQty);
		bEqual &= (HKOrderItem.nDealtQty == AckBody.HKOrderItem.nDealtQty);
		bEqual &= (HKOrderItem.nDealtAvgPrice == AckBody.HKOrderItem.nDealtAvgPrice);
		bEqual &= (HKOrderItem.nSubmitedTime == AckBody.HKOrderItem.nSubmitedTime);
		bEqual &= (HKOrderItem.nUpdatedTime == AckBody.HKOrderItem.nUpdatedTime);
		bEqual &= (HKOrderItem.nErrCode == AckBody.HKOrderItem.nErrCode);
		bEqual &= (HKOrderItem.strStockCode == AckBody.HKOrderItem.strStockCode);
		bEqual &= (HKOrderItem.strStockName == AckBody.HKOrderItem.strStockName);
		return bEqual;
	}
};

struct PushHKOrder_Req
{
	ProtoHead			head;
	PushHKOrderReqBody	body;
};

struct PushHKOrder_Ack
{
	ProtoHead			head;
	PushHKOrderAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//�������ɶ������ɽ�����
struct SubUSOrderDealReqBody
{
	int		nEnvType;
	int		nCookie;
	std::string strOrderID;
	int		nSubOrder;
	int		nSubDeal;
	int		nFirstPush;
	SubUSOrderDealReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
		nSubOrder = 0;
		nSubDeal = 0;
	}
};

struct SubUSOrderDealAckBody
{
	int		nEnvType;
	int		nCookie;
	std::string strOrderID;
	std::string strSubOrderSuc;
	std::string strSubDealSuc;

	SubUSOrderDealAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct SubUSOrderDeal_Req
{
	ProtoHead				head;
	SubUSOrderDealReqBody	body;
};

struct SubUSOrderDeal_Ack
{
	ProtoHead				head;
	SubUSOrderDealAckBody	body;
};

struct PushUSOrderReqBody
{
	PushUSOrderReqBody()
	{
	}
};

typedef QueryUSOrderAckItem	PushUSOrderAckItem;

struct PushUSOrderAckBody
{
	int	nEnvType;
	PushUSOrderAckItem USOrderItem;

	PushUSOrderAckBody()
	{
		nEnvType = 0;
	}

	bool Equal(const PushUSOrderAckBody& AckBody)
	{
		bool bEqual = (nEnvType == AckBody.nEnvType);
		bEqual &= (USOrderItem.nOrderID == AckBody.USOrderItem.nOrderID);
		bEqual &= (USOrderItem.nOrderType == AckBody.USOrderItem.nOrderType);
		bEqual &= (USOrderItem.nStatus == AckBody.USOrderItem.nStatus);
		bEqual &= (USOrderItem.nPrice == AckBody.USOrderItem.nPrice);
		bEqual &= (USOrderItem.nQty == AckBody.USOrderItem.nQty);
		bEqual &= (USOrderItem.nDealtQty == AckBody.USOrderItem.nDealtQty);
		bEqual &= (USOrderItem.nDealtAvgPrice == AckBody.USOrderItem.nDealtAvgPrice);
		bEqual &= (USOrderItem.nSubmitedTime == AckBody.USOrderItem.nSubmitedTime);
		bEqual &= (USOrderItem.nUpdatedTime == AckBody.USOrderItem.nUpdatedTime);
		bEqual &= (USOrderItem.strStockCode == AckBody.USOrderItem.strStockCode);
		bEqual &= (USOrderItem.strStockName == AckBody.USOrderItem.strStockName);
		return bEqual;
	}
};

struct PushUSOrder_Req
{
	ProtoHead			head;
	PushUSOrderReqBody	body;
};

struct PushUSOrder_Ack
{
	ProtoHead			head;
	PushUSOrderAckBody	body;
};


//////////////////////////////////////////////////////////////////////////
//��ѯ��ʷ�۹ɶ���
struct QueryHKHisOrderReqBody
{
	int		nEnvType;
	int		nCookie;
	std::string strStatusFilter; //״̬�����ַ����� ��","�ŷָ�����"0,1,2"
	std::string strStockCode;
	std::string strStartDate;
	std::string strEndDate;

	QueryHKHisOrderReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryHKHisOrderAckItem
{
	INT64 nOrderID; //�����ţ������������Ķ���������ID
	int nOrderType; //��ͬ�г���ȡֵ��Ӧ�����ö�ٶ��� Trade_OrderType_HK �� Trade_OrderType_US
	int enSide;/*Trade_OrderSide*/
	int nStatus; //ȡֵ��Ӧ�����ö�ٶ���Trade_OrderStatus
	std::wstring strStockCode;
	std::wstring strStockName;
	INT64 nPrice;
	INT64 nQty;
	INT64 nDealtQty; //�ɽ�����
	INT64 nSubmitedTime; //�������յ��Ķ����ύʱ��
	INT64 nUpdatedTime; //���������µ�ʱ��
	int   nErrCode; //�����룬��֧�ָ۹�

	QueryHKHisOrderAckItem()
	{
		nOrderID = 0;
		nOrderType = 0;
		nStatus = 0;
		nPrice = 0;
		nQty = 0;
		nDealtQty = 0;
		nSubmitedTime = 0;
		nUpdatedTime = 0;
		nErrCode = 0;
	}
};

typedef std::vector<QueryHKHisOrderAckItem>	VT_HK_HIS_ORDER;

struct QueryHKHisOrderAckBody
{
	int		nEnvType;
	int		nCookie;
	VT_HK_HIS_ORDER vtHisOrder;//ע����ʷ����û�гɽ������ֶ�

	QueryHKHisOrderAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryHKHisOrder_Req
{
	ProtoHead				head;
	QueryHKHisOrderReqBody	body;
};

struct QueryHKHisOrder_Ack
{
	ProtoHead				head;
	QueryHKHisOrderAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//��ѯ��ʷ���ɶ���
struct QueryUSHisOrderReqBody
{
	int		nEnvType;
	int		nCookie;
	std::string strStatusFilter; //״̬�����ַ����� ��","�ŷָ�����"0,1,2"
	std::string strStockCode;
	std::string strStartDate;
	std::string strEndDate;

	QueryUSHisOrderReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryUSHisOrderAckItem
{
	INT64 nOrderID; //�����ţ������������Ķ���������ID
	int nOrderType; //��ͬ�г���ȡֵ��Ӧ�����ö�ٶ��� Trade_OrderType_US �� Trade_OrderType_US
	int enSide; //Trade_OrderSide
	int nStatus; //ȡֵ��Ӧ�����ö�ٶ���Trade_OrderStatus
	std::wstring strStockCode;
	std::wstring strStockName;
	INT64 nPrice;
	INT64 nQty;
	INT64 nDealtQty; //�ɽ�����
	int   nDealtAvgPrice;
	INT64 nSubmitedTime; //�������յ��Ķ����ύʱ��
	INT64 nUpdatedTime; //���������µ�ʱ��

	QueryUSHisOrderAckItem()
	{
		nOrderID = 0;
		nOrderType = 0;
		nStatus = 0;
		nPrice = 0;
		nQty = 0;
		nDealtQty = 0;
		nSubmitedTime = 0;
		nUpdatedTime = 0;
	}
};

typedef std::vector<QueryUSHisOrderAckItem>	VT_US_HIS_ORDER;

struct QueryUSHisOrderAckBody
{
	int		nEnvType;
	int		nCookie;
	VT_US_HIS_ORDER vtHisOrder;//ע����ʷ����û�гɽ������ֶ�

	QueryUSHisOrderAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryUSHisOrder_Req
{
	ProtoHead				head;
	QueryUSHisOrderReqBody	body;
};

struct QueryUSHisOrder_Ack
{
	ProtoHead				head;
	QueryUSHisOrderAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//�۹ɹɳɽ�����
struct PushHKDealReqBody
{
};

struct PushHKDealAckBody
{
	int	nEnvType;
	INT64 nOrderID; //�����ţ������������Ķ���������ID
	INT64 nDealID; //�ɽ���

	int enSide; //����
	std::wstring strStockCode;
	std::wstring strStockName;

	INT64 nPrice; //�ɽ��۸�
	INT64 nQty; //�ɽ�����
	INT64 nTime;	//�ɽ�ʱ��

	int	nContraBrokerID;
	std::wstring strContraBrokerName;

	PushHKDealAckBody()
	{
		nEnvType = 0;
		nOrderID = 0;
		nDealID = 0;
		enSide = 0;
		nPrice = 0;
		nQty = 0;
		nTime = 0;
		nContraBrokerID = 0;
	}
	bool Equal(const PushHKDealAckBody &AckBody)
	{
		bool bEqual = (nEnvType == AckBody.nEnvType);
		bEqual &= (nOrderID == AckBody.nOrderID);
		bEqual &= (nDealID == AckBody.nDealID);
		bEqual &= (enSide == AckBody.enSide);
		bEqual &= (strStockCode == AckBody.strStockCode);
		bEqual &= (strStockName == AckBody.strStockName);
		bEqual &= (nPrice == AckBody.nPrice);
		bEqual &= (nQty == AckBody.nQty);
		bEqual &= (nTime == AckBody.nTime);
		bEqual &= (nContraBrokerID == AckBody.nContraBrokerID);
		bEqual &= (strContraBrokerName == AckBody.strContraBrokerName);
		return bEqual;
	}
};

struct PushHKDeal_Req
{
	ProtoHead			head;
	PushHKDealReqBody	body;
};

struct PushHKDeal_Ack
{
	ProtoHead			head;
	PushHKDealAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
//���ɳɽ�����
struct PushUSDealReqBody
{
};

struct PushUSDealAckBody
{
	int	nEnvType;
	INT64 nOrderID; //�����ţ������������Ķ���������ID
	INT64 nDealID; //�ɽ���

	int enSide; //����

	std::wstring strStockCode;
	std::wstring strStockName;

	INT64 nPrice; //�ɽ��۸�
	INT64 nQty; //�ɽ�����

	INT64 nTime;	//�ɽ�ʱ��

	PushUSDealAckBody()
	{
		nEnvType = 0;
		nOrderID = 0;
		nDealID = 0;
		enSide = 0;
		nPrice = 0;
		nQty = 0;
		nTime = 0;
	}

	bool Equal(const PushUSDealAckBody &AckBody)
	{
		bool bEqual = (nEnvType == AckBody.nEnvType);
		bEqual &= (nOrderID == AckBody.nOrderID);
		bEqual &= (nDealID == AckBody.nDealID);
		bEqual &= (enSide == AckBody.enSide);
		bEqual &= (strStockCode == AckBody.strStockCode);
		bEqual &= (strStockName == AckBody.strStockName);
		bEqual &= (nPrice == AckBody.nPrice);
		bEqual &= (nQty == AckBody.nQty);
		bEqual &= (nTime == AckBody.nTime);
		return bEqual;
	}
};

struct PushUSDeal_Req
{
	ProtoHead			head;
	PushUSDealReqBody	body;
};

struct PushUSDeal_Ack
{
	ProtoHead			head;
	PushUSDealAckBody	body;
};

//////////////////////////////////////////////////////////////////////////
struct QueryHKHisDealReqBody
{
	int nEnvType;
	int nCookie;
	std::string strStockCode;
	std::string strStartDate;
	std::string strEndDate;
	QueryHKHisDealReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryHKHisDealAckItem
{
	INT64 nOrderID;
	INT64 nDealID;

	int enSide;
	std::wstring strStockCode;
	std::wstring strStockName;
	INT64 nPrice;
	INT64 nQty;

	INT64 nTime;

	int nContraBrokerID;
	std::wstring strContraBrokerName;

	QueryHKHisDealAckItem()
	{
		nOrderID = 0;
		nDealID = 0;

		enSide = 0;
		nPrice = 0;
		nQty = 0;
		nTime = 0;

		nContraBrokerID = 0;
	}
};

typedef std::vector<QueryHKHisDealAckItem>    VT_HK_HisDeal;

struct QueryHKHisDealAckBody
{
	int nEnvType;
	int nCookie;
	VT_HK_HisDeal vtHisDeal;

	QueryHKHisDealAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryHKHisDeal_Req
{
	ProtoHead				head;
	QueryHKHisDealReqBody   body;
};

struct QueryHKHisDeal_Ack
{
	ProtoHead				head;
	QueryHKHisDealAckBody   body;
};

//////////////////////////////////////////////////////////////////////////
struct QueryUSHisDealReqBody
{
	int nEnvType;
	int nCookie;
	std::string strStockCode;
	std::string strStartDate;
	std::string strEndDate;
	QueryUSHisDealReqBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryUSHisDealAckItem
{
	INT64 nOrderID; 
	INT64 nDealID; 

	int enSide; 

	std::wstring strStockCode;
	std::wstring strStockName;

	INT64 nPrice; 
	INT64 nQty; 

	INT64 nTime;

	QueryUSHisDealAckItem()
	{
		nOrderID = 0;
		nDealID = 0;

		enSide = 0;

		nPrice = 0;
		nQty = 0;

		nTime = 0;
	}
};

typedef std::vector<QueryUSHisDealAckItem>    VT_US_HisDeal;

struct QueryUSHisDealAckBody
{
	int        nEnvType;
	int        nCookie;
	VT_US_HisDeal vtHisDeal;

	QueryUSHisDealAckBody()
	{
		nEnvType = 0;
		nCookie = 0;
	}
};

struct QueryUSHisDeal_Req
{
	ProtoHead				head;
	QueryUSHisDealReqBody   body;
};

struct QueryUSHisDeal_Ack
{
	ProtoHead				head;
	QueryUSHisDealAckBody   body;
};

///////////////////////////////////////////////////////////////////////////
//��֤
struct CheckSecNumReqBody
{
	int	nCookie;

	CheckSecNumReqBody()
	{
		nCookie = 0;
	}
};

struct CheckSecNumAckBody
{
	int	nCookie;
	int nSvrResult;	

	CheckSecNumAckBody()
	{
		nCookie = 0;
		nSvrResult = 0;
	}
};

struct CheckSecNum_Req
{
	ProtoHead				head;
	CheckSecNumReqBody		body;
};

struct CheckSecNum_Ack
{
	ProtoHead				head;
	CheckSecNumAckBody		body;
};