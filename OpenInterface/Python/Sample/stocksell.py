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
    实例: 股票卖出函数
"""
from OpenInterface.Python.openft.open_quant_context import *

from math import floor


def simple_sell(quote_ctx, trade_ctx, stock_code, trade_price, volume, trade_env):
    """简单卖出函数"""
    lot_size = 0
    while True:
        if lot_size == 0:
            ret, data = quote_ctx.get_market_snapshot([stock_code])
            lot_size = data.iloc[0]['lot_size'] if ret == 0 else 0
            if ret != RET_OK:
                print("can't get lot size, retrying")
                continue
            elif lot_size <= 0:
                raise Exception('lot size error {}:{}'.format(lot_size, stock_code))
        qty = floor(volume/lot_size)*lot_size
        ret, data = trade_ctx.place_order(price=trade_price, qty=qty, strcode=stock_code,
                                          orderside=1, envtype=trade_env)
        if ret != RET_OK:
            print('下单失败{}'.format(data))
            return None
        else:
            print('下单成功')
            return data


def smart_sell(quote_ctx, trade_ctx, stock_code, volume, trade_env):
    """智能卖出函数"""
    lot_size = 0
    while True:
        if lot_size == 0:
            ret, data = quote_ctx.get_market_snapshot([stock_code])
            lot_size = data.iloc[0]['lot_size'] if ret == 0 else 0
            if ret != RET_OK:
                print("can't get lot size, retrying")
                continue
            elif lot_size <= 0:
                raise Exception('lot size error {}:{}'.format(lot_size, stock_code))
        qty = floor(volume/lot_size)*lot_size
        ret, data = quote_ctx.get_order_book(stock_code)
        if ret != RET_OK:
            print("can't get orderbook, retrying")
            continue
        price = data['Bid'][0][0]
        print('bid price is {}'.format(price))
        ret, data = trade_ctx.place_order(price=price, qty=qty, strcode=stock_code, orderside=1, envtype=trade_env)
        if ret != RET_OK:
            print('下单失败{}'.format(data))
            return None
        else:
            return data
