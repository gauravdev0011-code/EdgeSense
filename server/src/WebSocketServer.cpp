#include "WebSocketServer.hpp"

#define ASIO_STANDALONE

#include <atomic>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace edgesense {

struct WebSocketServer::Impl {
    using Server = websocketpp::server<websocketpp::config::asio>;
    using ConnectionHandle = websocketpp::connection_hdl;

    explicit Impl(std::uint16_t port)
        : port(port),
          server(std::make_unique<Server>()) {
    }

    ~Impl() {
        stop();
    }

    void start() {
        if (running.exchange(true)) {
            return;
        }

        serverThread = std::thread([this]() { run(); });
    }

    void stop() {
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

            {
                std::lock_guard<std::mutex> lock(connectionsMutex);

                for (const auto& handle : connections) {
                    server->close(
                        handle,
                        websocketpp::close::status::going_away,
                        "Server shutting down",
                        error
                    );
                }

                connections.clear();
            }

            server->stop();
        }

        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

    void broadcast(const std::string& message) {
        if (!server) {
            return;
        }

        std::lock_guard<std::mutex> lock(connectionsMutex);

        for (const auto& handle : connections) {
            websocketpp::lib::error_code error;

            server->send(
                handle,
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

        std::cout
            << "[WebSocket] Broadcast: "
            << message
            << '\n';
    }

private:
    void run() {
        try {
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
            server->listen(port, error);

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
                    << "[WebSocket] Start accept failed: "
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
        }
        catch (const std::exception& error) {
            std::cerr
                << "[WebSocket] Server exception: "
                << error.what()
                << '\n';
            running = false;
        }
    }

    void onOpen(ConnectionHandle handle) {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        connections.insert(handle);
        std::cout << "[WebSocket] Client connected\n";
    }

    void onClose(ConnectionHandle handle) {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        connections.erase(handle);
        std::cout << "[WebSocket] Client disconnected\n";
    }

    std::uint16_t port;
    std::atomic<bool> running{false};
    std::thread serverThread;
    std::unique_ptr<Server> server;
    std::set<ConnectionHandle, std::owner_less<ConnectionHandle>> connections;
    std::mutex connectionsMutex;
};

WebSocketServer::WebSocketServer(std::uint16_t port)
    : impl(std::make_unique<Impl>(port)) {
}

WebSocketServer::~WebSocketServer() = default;

void WebSocketServer::start() {
    impl->start();
}

void WebSocketServer::stop() {
    impl->stop();
}

void WebSocketServer::broadcast(const std::string& message) {
    impl->broadcast(message);
}

} // namespace edgesense
