#include <iostream>
#include <nlohmann/json.hpp>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXNetSystem.h>
#include <chrono>

using json = nlohmann::json;

int main()
{
    // Initialize the WebSocket system
    ix::initNetSystem();

    ix::WebSocket webSocket;

    // Set the WebSocket URL
    webSocket.setUrl("wss://test.deribit.com/ws/api/v2");

    // Variable to track latency
    auto start_time = std::chrono::high_resolution_clock::now();

    // Set the callback to handle WebSocket events
    webSocket.setOnMessageCallback([&webSocket, &start_time](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::cout << "Connection established!" << std::endl;

            // Prepare the market data request
            json market_data_msg = {
                {"jsonrpc", "2.0"},
                {"id", 8106},
                {"method", "public/ticker"},
                {"params", {
                    {"instrument_name", "BTC-PERPETUAL"}
                }}
            };

            // Start measuring latency
            start_time = std::chrono::high_resolution_clock::now();

            // Send the market data request
            webSocket.send(market_data_msg.dump());
        }
        else if (msg->type == ix::WebSocketMessageType::Message)
        {
            // Calculate and log the latency
            auto end_time = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

            std::cout << "Market data received. Latency: " << latency << " ms" << std::endl;

            // Print the received message
            std::cout << "Response: " << msg->str << std::endl;

            // Stop the WebSocket connection
            webSocket.stop();
        }
        else if (msg->type == ix::WebSocketMessageType::Close)
        {
            std::cout << "Connection closed! Reason: " << msg->closeInfo.reason << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
        }
    });

    // Start the WebSocket connection
    webSocket.start();

    // Keep the WebSocket connection alive for 10 seconds to receive responses
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // Stop the WebSocket connection
    webSocket.stop();

    // Clean up the WebSocket system
    ix::uninitNetSystem();

    return 0;
}