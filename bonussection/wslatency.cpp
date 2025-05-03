#include <iostream>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>
#include <winsock2.h> // Include Winsock2 for WSAStartup and WSACleanup
#include <chrono>     // For latency measurement

using json = nlohmann::json;

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed. Exiting...\n";
        return 1;
    }

    ix::WebSocket webSocket;

    // Deribit testnet endpoint
    webSocket.setUrl("wss://test.deribit.com/ws/api/v2");

    // Variable to track latency
    auto start_time = std::chrono::high_resolution_clock::now();

    // Capture `webSocket` by reference in the lambda
    webSocket.setOnMessageCallback([&webSocket, &start_time](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            // Calculate and log the propagation delay
            auto end_time = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

            std::cout << "Message received. Propagation delay: " << latency << " ms" << std::endl;

            // Parse the message to extract order book updates
            try {
                auto json_msg = json::parse(msg->str);
                if (json_msg.contains("params") && json_msg["params"].contains("data")) {
                    std::cout << "Order book update: " << json_msg["params"]["data"] << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing message: " << e.what() << std::endl;
            }
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            std::cout << "WebSocket connection opened!" << std::endl;

            // Send subscription message
            json subscription_msg = {
                {"jsonrpc", "2.0"},
                {"id", 42},
                {"method", "public/subscribe"},
                {"params", {
                    {"channels", {"book.ETH_USDC-PERPETUAL.none.10.100ms"}}
                }}
            };

            // Start measuring propagation delay
            start_time = std::chrono::high_resolution_clock::now();

            webSocket.send(subscription_msg.dump());
            std::cout << "Sent subscription message: " << subscription_msg.dump() << std::endl;
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
        }
    });

    webSocket.start();

    // Keep the connection alive indefinitely
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Clean up Winsock (this will never be reached in this example)
    WSACleanup();

    return 0;
}