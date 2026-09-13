#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace edgesense {

class WebSocketServer {
public:
    explicit WebSocketServer(std::uint16_t port);

    ~WebSocketServer();

    void start();

    void stop();

    void broadcast(const std::string& message);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace edgesense
