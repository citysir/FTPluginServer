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
验证接口：获取某个市场的全部快照数据
"""
from OpenInterface.Python.openft.open_quant_context import *
import time


def loop_get_mkt_snapshot(api_svr_ip, api_svr_port, market):
    """
    验证接口：获取某个市场的全部快照数据 get_mkt_snapshot
    :param api_svr_ip: (string)ip
    :param api_svr_port: (int)port
    :param market: market type
    :return:
    """
    # 创建行情api
    quote_ctx = OpenQuoteContext(host=api_svr_ip, port=api_svr_port)
    stock_type = ['STOCK', 'IDX', 'ETF', 'WARRANT', 'BOND']

    while True:
        stock_codes = []
        # 枚举所有的股票类型，获取股票codes
        for sub_type in stock_type:
            ret_code, ret_data = quote_ctx.get_stock_basicinfo(market, sub_type)
            if ret_code == 0:
                for ix, row in ret_data.iterrows():
                    stock_codes.append(row['code'])

        if len(stock_codes) == 0:
            quote_ctx.close()
            print("Error market:'{}' can not get stock info".format(market))
            return

        # 按频率限制获取股票快照: 每5秒200支股票
        for i in range(1, len(stock_codes), 200):
            print("from {}, total {}".format(i, len(stock_codes)))
            ret_code, ret_data = quote_ctx.get_market_snapshot(stock_codes[i:i+200])
            if ret_code != 0:
                print(ret_code, ret_data)
            time.sleep(5)
        time.sleep(10)

if __name__ == "__main__":
    API_SVR_IP = '127.0.0.1'
    API_SVR_PORT = 11111
    MARKET = 'US'  # 'US'/'SH'/'SZ'
    loop_get_mkt_snapshot(API_SVR_IP, API_SVR_PORT, MARKET)
