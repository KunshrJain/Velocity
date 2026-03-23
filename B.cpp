#include "cppzmq-master/zmq.hpp"
#include "common.hpp"
#include <iostream>

int main(int argc, char** argv) {
    uint32_t uid = (argc > 1) ? std::stoi(argv[1]) : 1;
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::req);
    socket.connect("tcp://localhost:5555");

    while (true) {
        Order o;
        o.userId = uid;
        int s;
        std::cout << "\n(0:BUY, 1:SELL) | Price | Qty: ";
        std::cin >> s >> o.price >> o.qty;
        o.side = static_cast<Side>(s);

        socket.send(zmq::buffer(&o, sizeof(Order)), zmq::send_flags::none);

        zmq::message_t reply;
        socket.recv(reply, zmq::recv_flags::none);
        ExchangeResponse* res = static_cast<ExchangeResponse*>(reply.data());

        std::cout << "--- STATUS ---" << std::endl;
        std::cout << "Current Valuation: " << res->lastTradePrice << std::endl;
        std::cout << "Unfilled Quantity: " << res->remainingQty << std::endl;
    }
}