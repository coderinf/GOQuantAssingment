#include <iostream>
#include <nlohmann/json.hpp>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXNetSystem.h> // Include for initNetSystem and uninitNetSystem
#include <thread>
#include <chrono>

using json = nlohmann::json;

int main()
{
    // Display a welcome message
    std::cout << "Welcome to the Deribit Trading System!" << std::endl;

    // Ask for client_id and client_secret
    std::string client_id, client_secret;
    std::cout << "Enter your client ID: ";
    std::cin >> client_id;
    std::cout << "Enter your client secret: ";
    std::cin >> client_secret;

    // Initialize Winsock
    ix::initNetSystem();

    ix::WebSocket webSocket;

    // Set the WebSocket URL
    webSocket.setUrl("wss://test.deribit.com/ws/api/v2");

    // Flag to ensure only one order is sent
    bool orderSent = false;

    // Variables to track latency
    auto start_time = std::chrono::high_resolution_clock::now();
    auto auth_start_time = std::chrono::high_resolution_clock::now();
    auto order_start_time = std::chrono::high_resolution_clock::now();

    // Set the callback to handle WebSocket events
    webSocket.setOnMessageCallback([&webSocket, client_id, client_secret, &orderSent, &start_time, &auth_start_time, &order_start_time](const ix::WebSocketMessagePtr& msg)
    {
        if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::cout << "Connection established!" << std::endl;

            // Start measuring authentication latency
            auth_start_time = std::chrono::high_resolution_clock::now();

            // Authenticate with the API
            json auth_msg = {
                {"jsonrpc", "2.0"},
                {"id", 9929},
                {"method", "public/auth"},
                {"params", {
                    {"grant_type", "client_credentials"},
                    {"client_id", client_id},
                    {"client_secret", client_secret}
                }}
            };

            // Send the authentication message
            webSocket.send(auth_msg.dump());
        }
        else if (msg->type == ix::WebSocketMessageType::Message)
        {
            auto current_time = std::chrono::high_resolution_clock::now();

            // Print the received message
            std::cout << "Received: " << msg->str << std::endl;

            // Check if authentication was successful
            if (msg->str.find("\"result\"") != std::string::npos && !orderSent)
            {
                // Calculate and log authentication latency
                auto auth_latency = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - auth_start_time).count();
                std::cout << "Authentication Latency: " << auth_latency << " ms" << std::endl;

                std::cout << "Authentication successful!" << std::endl;

                // Start measuring order placement latency
                order_start_time = std::chrono::high_resolution_clock::now();

                // Prepare the order message
                json order_msg = {
                    {"jsonrpc", "2.0"},
                    {"id", 5275},
                    {"method", "private/buy"},
                    {"params", {
                        {"instrument_name", "ETH-PERPETUAL"},
                        {"amount", 40},
                        {"type", "market"},
                        {"label", "market0000234"}
                    }}
                };

                // Send the order message
                webSocket.send(order_msg.dump());
                orderSent = true; // Set the flag to true to prevent further orders
            }
            else if (orderSent)
            {
                // Calculate and log order placement latency
                auto order_latency = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - order_start_time).count();
                std::cout << "Order Placement Latency: " << order_latency << " ms" << std::endl;

                // Calculate and log end-to-end latency
                auto end_to_end_latency = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
                std::cout << "End-to-End Latency: " << end_to_end_latency << " ms" << std::endl;

                // Stop the WebSocket connection after receiving the order response
                webSocket.stop();
            }
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
    start_time = std::chrono::high_resolution_clock::now();
    webSocket.start();

    // Keep the WebSocket connection alive for 30 seconds to receive responses
    std::this_thread::sleep_for(std::chrono::seconds(30));

    // Stop the WebSocket connection
    webSocket.stop();

    // Clean up Winsock
    ix::uninitNetSystem();

    return 0;
}