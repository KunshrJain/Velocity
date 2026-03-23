#include "cppzmq-master/zmq.hpp"
#include "common.hpp"
#include <iostream>
#include <map>
#include <algorithm>

class Matcher {
public:
    std::map<int64_t, uint32_t, std::greater<int64_t>> bids; 
    std::map<int64_t, uint32_t> asks;                        
    int64_t lastPrice = 0;

    ExchangeResponse process(Order& o) {
        if (o.side == BUY) {
            auto it = asks.begin();
            while (it != asks.end() && o.qty > 0) {
                if (o.price >= it->first) {
                    uint32_t fill = std::min(o.qty, it->second);
                    o.qty -= fill;
                    it->second -= fill;
                    lastPrice = it->first;
                    std::cout << "[MATCH] Price: " << lastPrice << " Qty: " << fill << std::endl;
                    if (it->second == 0) it = asks.erase(it);
                    else ++it;
                } else break;
            }
            if (o.qty > 0) bids[o.price] += o.qty;
        } else {
            auto it = bids.begin();
            while (it != bids.end() && o.qty > 0) {
                if (o.price <= it->first) {
                    uint32_t fill = std::min(o.qty, it->second);
                    o.qty -= fill;
                    it->second -= fill;
                    lastPrice = it->first;
                    std::cout << "[MATCH] Price: " << lastPrice << " Qty: " << fill << std::endl;
                    if (it->second == 0) it = bids.erase(it);
                    else ++it;
                } else break;
            }
            if (o.qty > 0) asks[o.price] += o.qty;
        }

        int64_t valuation = lastPrice;
        if (!bids.empty() && !asks.empty()) {
            valuation = (bids.begin()->first + asks.begin()->first) / 2;
        }

        return {true, valuation, o.qty};
    }
};

int main() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.bind("tcp://*:5555");

    Matcher engine;
    std::cout << "[SYSTEM] Matching Engine Live..." << std::endl;

    while (true) {
        zmq::message_t request;
        auto res = socket.recv(request, zmq::recv_flags::none);

        if (request.size() == sizeof(Order)) {
            Order* o = static_cast<Order*>(request.data());
            ExchangeResponse response = engine.process(*o);
            socket.send(zmq::buffer(&response, sizeof(ExchangeResponse)), zmq::send_flags::none);
        }
    }
    return 0;
}