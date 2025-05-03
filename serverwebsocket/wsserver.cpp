#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>
#include <winsock2.h>
#include <atomic>
#include <fstream>
#include <chrono>
#include <iomanip>

using json = nlohmann::json;

// Map of symbol to clients
std::unordered_map<std::string, std::vector<ix::WebSocket*>> symbolClients;
std::unordered_map<std::string, std::atomic<bool>> activeFetchers;
std::mutex symbolClientsMutex;
std::atomic<int> totalConnectedClients = 0;

// Log file for client actions
std::ofstream logFile("server_log.txt", std::ios::app);

// Function to log messages with timestamps
void logMessage(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::localtime(&now_time_t);

    std::lock_guard<std::mutex> lock(symbolClientsMutex);
    logFile << "[" << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << "] " << message << std::endl;
    std::cout << "[" << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << "] " << message << std::endl;
}

// Broadcast symbol data to subscribed clients
void broadcastToClients(const std::string& symbol, const std::string& message) {
    std::lock_guard<std::mutex> lock(symbolClientsMutex);
    if (symbolClients.count(symbol)) {
        for (auto* client : symbolClients[symbol]) {
            client->send(message);
        }
    }
}

// Fetch from Deribit
void fetchFromDeribit(const std::string& symbol) {
    ix::WebSocket ws;
    ws.setUrl("wss://test.deribit.com/ws/api/v2");

    ws.setOnMessageCallback([symbol, &ws](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            json sub_msg = {
                {"jsonrpc", "2.0"},
                {"id", 1},
                {"method", "public/subscribe"},
                {"params", {
                    {"channels", {
                        "book." + symbol + "_USDC-PERPETUAL.none.10.100ms"
                    }}
                }}
            };
            ws.send(sub_msg.dump());
        } else if (msg->type == ix::WebSocketMessageType::Message) {
            try {
                auto parsed = json::parse(msg->str);
                if (parsed.contains("params") && parsed["params"].contains("data")) {
                    broadcastToClients(symbol, parsed["params"]["data"].dump());
                }
            } catch (...) {
                std::cerr << "Error parsing data from Deribit.\n";
            }
        }
    });

    ws.start();

    // Keep alive until no subscribers
    while (activeFetchers[symbol]) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    ws.stop();
}

// WebSocket Server
void startWebSocketServer() {
    ix::WebSocketServer server(8080);

    server.setOnClientMessageCallback(
        [](std::shared_ptr<ix::ConnectionState> connState, ix::WebSocket& webSocket,
           const std::unique_ptr<ix::WebSocketMessage>& msg) {
            if (msg->type == ix::WebSocketMessageType::Open) {
                std::cout << "Client connected: " << connState->getId() << "\n";
                logMessage("Client connected: " + connState->getId() + " from IP: " + connState->getRemoteIp());
                logMessage("Total connected clients: " + std::to_string(++totalConnectedClients));
            } else if (msg->type == ix::WebSocketMessageType::Close) {
                std::cout << "Client disconnected: " << connState->getId() << "\n";
                logMessage("Client disconnected: " + connState->getId() + " from IP: " + connState->getRemoteIp());
                logMessage("Total connected clients: " + std::to_string(--totalConnectedClients));
                std::lock_guard<std::mutex> lock(symbolClientsMutex);
                for (auto& [symbol, clients] : symbolClients) {
                    clients.erase(std::remove(clients.begin(), clients.end(), &webSocket), clients.end());
                    if (clients.empty()) {
                        activeFetchers[symbol] = false;
                    }
                }
            } else if (msg->type == ix::WebSocketMessageType::Message) {
                try {
                    auto data = json::parse(msg->str);
                    std::string type = data.value("type", "");
                    std::string symbol = data.value("symbol", "");

                    if (type == "subscribe" && !symbol.empty()) {
                        {
                            std::lock_guard<std::mutex> lock(symbolClientsMutex);
                            symbolClients[symbol].push_back(&webSocket);
                        }

                        if (!activeFetchers[symbol]) {
                            activeFetchers[symbol] = true;
                            std::thread(fetchFromDeribit, symbol).detach();
                        }

                        std::cout << "Client subscribed to: " << symbol << "\n";
                        logMessage("Client subscribed to: " + symbol);
                    } else if (type == "unsubscribe" && !symbol.empty()) {
                        bool stopFetcher = false;
                        {
                            std::lock_guard<std::mutex> lock(symbolClientsMutex);
                            auto& clients = symbolClients[symbol];
                            clients.erase(std::remove(clients.begin(), clients.end(), &webSocket), clients.end());
                            stopFetcher = clients.empty();
                        }

                        if (stopFetcher) {
                            activeFetchers[symbol] = false;
                        }

                        std::cout << "Client unsubscribed from: " << symbol << "\n";
                        logMessage("Client unsubscribed from: " + symbol);
                    } else if (type == "disconnect") {
                        std::cout << "Client sent disconnect message: " << connState->getId() << "\n";
                        logMessage("Client sent disconnect message: " + connState->getId());
                        std::lock_guard<std::mutex> lock(symbolClientsMutex);
                        for (auto& [symbol, clients] : symbolClients) {
                            clients.erase(std::remove(clients.begin(), clients.end(), &webSocket), clients.end());
                            if (clients.empty()) {
                                activeFetchers[symbol] = false;
                            }
                        }
                    }

                } catch (...) {
                    std::cerr << "Invalid client message.\n";
                    logMessage("Invalid client message from: " + connState->getId());
                }
            }
        });

    server.listen();
    server.start();

    std::cout << "WebSocket server running on port 8080...\n";
    logMessage("WebSocket server running on port 8080...");

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Entry
int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        logMessage("WSAStartup failed");
        return 1;
    }

    startWebSocketServer();

    WSACleanup();
    return 0;
}
