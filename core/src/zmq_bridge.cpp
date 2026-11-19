#include "zmq_bridge.hpp"
#include <iostream>

ZmqBridge::ZmqBridge(const std::string& push_addr, const std::string& pull_addr)
    : ctx_(1), push_socket_(ctx_, zmq::socket_type::push), pull_socket_(ctx_, zmq::socket_type::pull) {
    push_socket_.connect(push_addr);
    pull_socket_.set(zmq::socketopt::rcvtimeo, 200);
    pull_socket_.bind(pull_addr);
}

ZmqBridge::~ZmqBridge() {
    stop();
}

void ZmqBridge::start_listening(MessageCallback callback) {
    if (running_) {
        return;
    }
    running_ = true;
    callback_ = std::move(callback);
    listener_thread_ = std::thread(&ZmqBridge::listen_loop, this);
}

void ZmqBridge::stop() {
    if (!running_) {
        running_ = false;
        if (listener_thread_.joinable()) {
            listener_thread_.join();
        }
    }
}

bool ZmqBridge::send_command(const nlohmann::json& cmd) {
    try {
        std::string msg = cmd.dump();
        zmq::message_t msg(msg.data(), msg.size());
        auto res = push_socket_.send(msg, zmq::send_flags::none);
        return res.has_value();
    } catch (const zmq::error_t& e) {
        std::cerr << "ZMQ send error: " << e.what() << std::endl;
        return false;
    }
}

void ZmqBridge::listen_loop() {
    while (running_) {
        zmq::message_t msg;
        try {
            auto res = pull_socket_.recv(msg, zmq::recv_flags::none);
            if (res.has_value()) {
                std::string text(static_cast<char*>(msg.data()), msg.size());
                auto parsed = nlohmann::json::parse(text, nullptr, false);
                if (!parsed.is_discarded() && callback_) {
                    callback_(parsed);
                }
            }
        } catch (const zmq::error_t& e) {
            if (running_) {
                std::cerr << "ZMQ receive error: " << e.what() << std::endl;
            }
        }
    }
}