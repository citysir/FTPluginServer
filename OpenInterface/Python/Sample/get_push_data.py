# -*- coding: utf-8 -*-
#
# Copyright 2017 Futu Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Examples for use the python functions: get push data
"""
from OpenInterface.Python.openft.open_quant_context import *


class StockQuoteTest(StockQuoteHandlerBase):
    """
    获得报价推送数据
    """
    def on_recv_rsp(self, rsp_str):
        """数据响应回调函数"""
        ret_code, content = super(StockQuoteTest, self).on_recv_rsp(rsp_str)
        if ret_code != RET_OK:
            print("StockQuoteTest: error, msg: %s" % content)
            return RET_ERROR, content
        print("StockQuoteTest\n ", content)
        return RET_OK, content


class OrderBookTest(OrderBookHandlerBase):
    """
    获得摆盘推送数据
    """
    def on_recv_rsp(self, rsp_str):
        """数据响应回调函数"""
        ret_code, content = super(OrderBookTest, self).on_recv_rsp(rsp_str)
        if ret_code != RET_OK:
            print("OrderBookTest: error, msg: %s" % content)
            return RET_ERROR, content
        print("OrderBookTest\n", content)
        return RET_OK, content


class CurKlineTest(CurKlineHandlerBase):
    """
    获取K线推送数据
    """
    def on_recv_rsp(self, rsp_str):
        """数据响应回调函数"""
        ret_code, content = super(CurKlineTest, self).on_recv_rsp(rsp_str)
        if ret_code != RET_OK:
            print("CurKlineTest: error, msg: %s" % content)
            return RET_ERROR, content
        print("CurKlineTest\n", content)
        return RET_OK, content


class TickerTest(TickerHandlerBase):
    """
    获取逐笔推送数据
    """
    def on_recv_rsp(self, rsp_str):
        """数据响应回调函数"""
        ret_code, content = super(TickerTest, self).on_recv_rsp(rsp_str)
        if ret_code != RET_OK:
            print("TickerTest: error, msg: %s" % content)
            return RET_ERROR, content
        print("TickerTest\n", content)
        return RET_OK, content


class RTDataTest(RTDataHandlerBase):
    """
    获取分时推送数据
    """
    def on_recv_rsp(self, rsp_str):
        """数据响应回调函数"""
        ret_code, content = super(RTDataTest, self).on_recv_rsp(rsp_str)
        if ret_code != RET_OK:
            print("RTDataTest: error, msg: %s" % content)
            return RET_ERROR, content
        print("RTDataTest\n")
        print(content)
        return RET_OK, content


class BrokerTest(BrokerHandlerBase):
    """
    获取经纪队列推送数据
    """
    def on_recv_rsp(self, rsp_str):
        """数据响应回调函数"""
        ret_code, content = super(BrokerTest, self).on_recv_rsp(rsp_str)
        if ret_code != RET_OK:
            print("BrokerTest: error, msg: %s " % content)
            return RET_ERROR, content
        print("BrokerTest\n", content[0])
        print("BrokerTest\n", content[1])
        return RET_OK, content


if __name__ == "__main__":

    quote_context = OpenQuoteContext(host='127.0.0.1', port=11111)

    # 获取推送数据
    quote_context.subscribe('HK.00700', "QUOTE", push=True)
    quote_context.set_handler(StockQuoteTest())

    quote_context.subscribe('HK.00700', "K_DAY", push=True)
    quote_context.set_handler(CurKlineTest())

    quote_context.subscribe('HK.00700', "ORDER_BOOK", push=True)
    quote_context.set_handler(OrderBookTest())

    quote_context.subscribe('HK.00700', "TICKER", push=True)
    quote_context.set_handler(TickerTest())

    quote_context.subscribe('HK.00700', "RT_DATA", push=True)
    quote_context.set_handler(RTDataTest())

    quote_context.subscribe('HK.00700', "BROKER", push=True)
    quote_context.set_handler(BrokerTest())
    quote_context.start()
