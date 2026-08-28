#include "WebSocketServer.hpp"

#include <iostream>

namespace edgesense {

WebSocketServer::WebSocketServer(std::uint16_t port)
    : port(port) {
}

WebSocketServer::~WebSocketServer() {
    stop();
}

void WebSocketServer::start() {
    if (running.exchange(true)) {
        return;
    }

    serverThread = std::thread(
        &WebSocketServer::run,
        this
    );
}

void WebSocketServer::stop() {
    if (!running.exchange(false)) {
        return;
    }

    if (server) {
        websocketpp::lib::error_code error;

        server->stop_listening(error);

        if (error) {
            std::cerr
                << "[WebSocket] Stop listening failed: "
                << error.message()
                << '\n';
        }

        std::lock_guard<std::mutex> lock(
            connectionsMutex
        );

        for (const auto& connection : connections) {
            server->close(
                connection,
                websocketpp::close::status::going_away,
                "Server shutting down",
                error
            );
        }

        connections.clear();
    }

    if (serverThread.joinable()) {
        serverThread.join();
    }
}

void WebSocketServer::broadcast(
    const std::string& message
) {
    if (!server) {
        return;
    }

    std::lock_guard<std::mutex> lock(
        connectionsMutex
    );

    for (const auto& connection : connections) {
        websocketpp::lib::error_code error;

        server->send(
            connection,
            message,
            websocketpp::frame::opcode::text,
            error
        );

        if (error) {
            std::cerr
                << "[WebSocket] Send failed: "
                << error.message()
                << '\n';
        }
    }
}

void WebSocketServer::onOpen(
    ConnectionHandle handle
) {
    std::lock_guard<std::mutex> lock(
        connectionsMutex
    );

    connections.insert(handle);

    std::cout
        << "[WebSocket] Client connected. "
        << "Clients: "
        << connections.size()
        << '\n';
}

void WebSocketServer::onClose(
    ConnectionHandle handle
) {
    std::lock_guard<std::mutex> lock(
        connectionsMutex
    );

    connections.erase(handle);

    std::cout
        << "[WebSocket] Client disconnected. "
        << "Clients: "
        << connections.size()
        << '\n';
}

void WebSocketServer::run() {
    server =
        std::make_unique<Server>();

    server->clear_access_channels(
        websocketpp::log::alevel::all
    );

    server->init_asio();

    server->set_open_handler(
        [this](ConnectionHandle handle) {
            onOpen(handle);
        }
    );

    server->set_close_handler(
        [this](ConnectionHandle handle) {
            onClose(handle);
        }
    );

    websocketpp::lib::error_code error;

    server->listen(
        websocketpp::lib::asio::ip::tcp::v4(),
        port,
        error
    );

    if (error) {
        std::cerr
            << "[WebSocket] Listen failed: "
            << error.message()
            << '\n';

        running = false;
        return;
    }

    server->start_accept(error);

    if (error) {
        std::cerr
            << "[WebSocket] Accept failed: "
            << error.message()
            << '\n';

        running = false;
        return;
    }

    std::cout
        << "[WebSocket] Server listening on port "
        << port
        << '\n';

    server->run();

    server.reset();
}

} // namespace edgesense