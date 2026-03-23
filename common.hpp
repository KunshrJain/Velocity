#pragma once
#include <stdint.h>

enum Side { BUY = 0, SELL = 1 };

struct Order {
    uint32_t userId;
    Side side;
    int64_t price;
    uint32_t qty;
};

struct ExchangeResponse {
    bool accepted;
    int64_t lastTradePrice;
    uint32_t remainingQty;
};