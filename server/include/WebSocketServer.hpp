#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace edgesense {

class WebSocketServer {
public:
    explicit WebSocketServer(std::uint16_t port);

    ~WebSocketServer();

    void start();

    void stop();

    void broadcast(const std::string& message);

private:
    using Server =
        websocketpp::server<websocketpp::config::asio>;

    using ConnectionHandle =
        websocketpp::connection_hdl;

    std::uint16_t port;

    std::atomic<bool> running{false};

    std::thread serverThread;

    std::unique_ptr<Server> server;

    std::set<
        ConnectionHandle,
        std::owner_less<ConnectionHandle>
    > connections;

    std::mutex connectionsMutex;

    void run();

    void onOpen(ConnectionHandle handle);

    void onClose(ConnectionHandle handle);
};

} // namespace edgesense