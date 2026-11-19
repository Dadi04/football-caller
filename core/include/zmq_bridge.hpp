#ifndef ZMQ_BRIDGE_HPP
#define ZMQ_BRIDGE_HPP

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <zmq.hpp>
#include <nlohmann/json.hpp>

class ZmqBridge {
public:
    using MessageCallback = std::function<void(const nlohmann::json&)>;

    ZmqBridge(const std::string& push_addr, const std::string& pull_addr);
    ~ZmqBridge();

    ZmqBridge(const ZmqBridge&) = delete;
    ZmqBridge& operator=(const ZmqBridge&) = delete;

    void start_listening(MessageCallback callback);
    void stop();

    bool send_command(const nlohmann::json& cmd);
private:
    zmq::context_t ctx_;
    zmq::socket_t push_socket_;
    zmq::socket_t pull_socket_;

    std::atomic<bool> running_{false};
    std::thread listener_thread_;
    MessageCallback callback_;

    void listen_loop();
}

#endif