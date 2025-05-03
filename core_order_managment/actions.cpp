#ifndef ACTIONS_H
#define ACTIONS_H
#include <iostream>
#include <nlohmann/json.hpp>
#include <ixwebsocket/IXWebSocket.h>
#include <thread>
#include <chrono>

using json = nlohmann::json;

// Function to authenticate the user
bool authenticate(ix::WebSocket& webSocket, const std::string& client_id, const std::string& client_secret)
{
    bool isAuthenticated = false;
    bool responseReceived = false;

    webSocket.setOnMessageCallback([&webSocket, &isAuthenticated, &responseReceived, client_id, client_secret](const ix::WebSocketMessagePtr& msg)
    {
        if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::cout << "Connection established!" << std::endl;

            // Prepare the authentication message
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
            std::cout << "Received: " << msg->str << std::endl;

            // Check if authentication was successful
            if (msg->str.find("\"result\"") != std::string::npos)
            {
                std::cout << "Authentication successful!" << std::endl;
                isAuthenticated = true;
            }
            else
            {
                std::cerr << "Authentication failed!" << std::endl;
            }
            responseReceived = true;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
            responseReceived = true;
        }
    });

    // Start the WebSocket connection
    webSocket.start();

    // Wait for the authentication response
    while (!responseReceived)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return isAuthenticated;
}

// Spot Market Actions
void placeSpotOrder(ix::WebSocket& webSocket)
{
    std::string instrument_name, order_type;
    int amount;
    double price = 0.0;

    std::cout << "Enter the instrument name (e.g., ETH-PERPETUAL): ";
    std::cin >> instrument_name;

    std::cout << "Enter the amount: ";
    std::cin >> amount;

    std::cout << "Enter the order type (market/limit): ";
    std::cin >> order_type;

    if (order_type == "limit")
    {
        std::cout << "Enter the price: ";
        std::cin >> price;
    }

    json order_msg = {
        {"jsonrpc", "2.0"},
        {"id", 5275},
        {"method", "private/buy"},
        {"params", {
            {"instrument_name", instrument_name},
            {"amount", amount},
            {"type", order_type}
        }}
    };

    if (order_type == "limit")
    {
        order_msg["params"]["price"] = price;
    }

    webSocket.send(order_msg.dump());
    std::cout << "Spot order placed for instrument: " << instrument_name << " with amount: " << amount << " as a " << order_type << " order." << std::endl;
}

void cancelSpotOrder(ix::WebSocket& webSocket)
{
    std::string order_id;
    std::cout << "Enter the Spot order ID to cancel: ";
    std::cin >> order_id;

    json cancel_msg = {
        {"jsonrpc", "2.0"},
        {"id", 4214},
        {"method", "private/cancel"},
        {"params", {
            {"order_id", order_id}
        }}
    };

    webSocket.send(cancel_msg.dump());
    std::cout << "Cancel request sent for Spot order ID: " << order_id << std::endl;
}

void ModifySpotOrder(ix::WebSocket& webSocket)
{
    std::string order_id;
    int amount;
    double price;
    std::string advanced;

    // Get user input for order modification
    std::cout << "Enter the Order ID to modify: ";
    std::cin >> order_id;

    std::cout << "Enter the new amount: ";
    std::cin >> amount;

    std::cout << "Enter the new price: ";
    std::cin >> price;
     // Prepare the JSON-RPC request
     json modify_msg = {
        {"jsonrpc", "2.0"},
        {"id", 3725},
        {"method", "private/edit"},
        {"params", {
            {"order_id", order_id},
            {"amount", amount},
            {"price", price}
        }}
    };

    // Send the request
    webSocket.send(modify_msg.dump());

    // Set up a callback to handle the response
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            // Print the response
            std::cout << "Response: " << msg->str << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
        }
    });
}

void getSpotOrderbook(ix::WebSocket& webSocket)
{
    std::cout << "Fetching Spot open orders..." << std::endl;

    // Prepare the JSON-RPC request
    json open_orders_msg = {
        {"jsonrpc", "2.0"},
        {"id", 1234},
        {"method", "private/get_open_orders"},
        {"params", {
            {"kind", "spot"}, // Specify the market type as "spot"
            {"type", "all"}   // Fetch all open orders
        }}
    };

    // Send the request
    webSocket.send(open_orders_msg.dump());

    // Set up a callback to handle the response
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            // Print the response
            std::cout << "Response: " << msg->str << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
        }
    });
}

void getSpotPositions(ix::WebSocket& webSocket)
{   
    std::cout << "Fetching Spot positions for all currencies..." << std::endl;

    // Prepare the JSON-RPC request
    json positions_msg = {
        {"jsonrpc", "2.0"},
        {"id", 1237},
        {"method", "private/get_positions"},
        {"params", {
            {"currency", "any"},   // Fetch positions for all currencies
            {"kind", "spot"}    // Specify the market type as "spot"
        }}
    };

    // Send the request
    webSocket.send(positions_msg.dump());

    // Set up a callback to handle the response
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            // Print the response
            std::cout << "Response: " << msg->str << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
        }
    });
}

// Futures Market Actions
void placeFuturesOrder(ix::WebSocket& webSocket)
{
    std::string instrument_name, order_type;
    int amount;
    double price = 0.0;

    std::cout << "Enter the Futures instrument name (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument_name;

    std::cout << "Enter the amount: ";
    std::cin >> amount;

    std::cout << "Enter the order type (market/limit): ";
    std::cin >> order_type;

    if (order_type == "limit")
    {
        std::cout << "Enter the price: ";
        std::cin >> price;
    }

    json order_msg = {
        {"jsonrpc", "2.0"},
        {"id", 5275},
        {"method", "private/buy"},
        {"params", {
            {"instrument_name", instrument_name},
            {"amount", amount},
            {"type", order_type}
        }}
    };

    if (order_type == "limit")
    {
        order_msg["params"]["price"] = price;
    }

    webSocket.send(order_msg.dump());
    std::cout << "Futures order placed for instrument: " << instrument_name << " with amount: " << amount << " as a " << order_type << " order." << std::endl;
}

void cancelFuturesOrder(ix::WebSocket& webSocket)
{
    std::string order_id;
    std::cout << "Enter the Futures order ID to cancel: ";
    std::cin >> order_id;

    json cancel_msg = {
        {"jsonrpc", "2.0"},
        {"id", 4214},
        {"method", "private/cancel"},
        {"params", {
            {"order_id", order_id}
        }}
    };

    webSocket.send(cancel_msg.dump());
    std::cout << "Cancel request sent for Futures order ID: " << order_id << std::endl;
}

void  modifyFuturesOrder(ix::WebSocket& webSocket)
{
    std::string order_id;
    int amount;
    double price;
    std::string advanced;

    // Get user input for order modification
    std::cout << "Enter the Order ID to modify: ";
    std::cin >> order_id;

    std::cout << "Enter the new amount: ";
    std::cin >> amount;

    std::cout << "Enter the new price: ";
    std::cin >> price;
     // Prepare the JSON-RPC request
     json modify_msg = {
        {"jsonrpc", "2.0"},
        {"id", 3725},
        {"method", "private/edit"},
        {"params", {
            {"order_id", order_id},
            {"amount", amount},
            {"price", price}
        }}
    };

    // Send the request
    webSocket.send(modify_msg.dump());

    // Set up a callback to handle the response
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            // Print the response
            std::cout << "Response: " << msg->str << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
        }
    });
}

void getFuturesOrders(ix::WebSocket& webSocket)
{
    std::cout << "Fetching Futures open orders..." << std::endl;

    // Prepare the JSON-RPC request
    json open_orders_msg = {
        {"jsonrpc", "2.0"},
        {"id", 1235},
        {"method", "private/get_open_orders"},
        {"params", {
            {"kind", "future"}, // Specify the market type as "future"
            {"type", "all"}     // Fetch all open orders
        }}
    };

    // Send the request
    webSocket.send(open_orders_msg.dump());

    // Set up a callback to handle the response
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            // Print the response
            std::cout << "Response: " << msg->str << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
        }
    });
}



void viewFuturesPositions(ix::WebSocket& webSocket)
{   
    std::cout << "Fetching Future positions for all currencies..." << std::endl;

    // Prepare the JSON-RPC request
    json positions_msg = {
        {"jsonrpc", "2.0"},
        {"id", 1237},
        {"method", "private/get_positions"},
        {"params", {
            {"currency", "any"},   // Fetch positions for all currencies
            {"kind", "future"}    // Specify the market type as "spot"
        }}
    };

    // Send the request
    webSocket.send(positions_msg.dump());

    // Set up a callback to handle the response
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            // Print the response
            std::cout << "Response: " << msg->str << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
        }
    });
}

// Options Market Actions
void placeOptionsOrder(ix::WebSocket& webSocket)
{
    std::string instrument_name, order_type;
    int amount;
    double price = 0.0;

    std::cout << "Enter the Options instrument name (e.g., ETH-25DEC23-1500-C): ";
    std::cin >> instrument_name;

    std::cout << "Enter the amount: ";
    std::cin >> amount;

    std::cout << "Enter the order type (market/limit): ";
    std::cin >> order_type;

    if (order_type == "limit")
    {
        std::cout << "Enter the price: ";
        std::cin >> price;
    }

    json order_msg = {
        {"jsonrpc", "2.0"},
        {"id", 5275},
        {"method", "private/buy"},
        {"params", {
            {"instrument_name", instrument_name},
            {"amount", amount},
            {"type", order_type}
        }}
    };

    if (order_type == "limit")
    {
        order_msg["params"]["price"] = price;
    }

    webSocket.send(order_msg.dump());
    std::cout << "Options order placed for instrument: " << instrument_name << " with amount: " << amount << " as a " << order_type << " order." << std::endl;
}

void cancelOptionsOrder(ix::WebSocket& webSocket)
{
    std::string order_id;
    std::cout << "Enter the Options order ID to cancel: ";
    std::cin >> order_id;

    json cancel_msg = {
        {"jsonrpc", "2.0"},
        {"id", 4214},
        {"method", "private/cancel"},
        {"params", {
            {"order_id", order_id}
        }}
    };

    webSocket.send(cancel_msg.dump());
    std::cout << "Cancel request sent for Options order ID: " << order_id << std::endl;
}

void getOptionsOrders(ix::WebSocket& webSocket)
{
    std::cout << "Fetching Options open orders..." << std::endl;

    // Prepare the JSON-RPC request
    json open_orders_msg = {
        {"jsonrpc", "2.0"},
        {"id", 1236},
        {"method", "private/get_open_orders"},
        {"params", {
            {"kind", "option"}, // Specify the market type as "option"
            {"type", "all"}     // Fetch all open orders
        }}
    };

    // Send the request
    webSocket.send(open_orders_msg.dump());

    // Set up a callback to handle the response
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            // Print the response
            std::cout << "Response: " << msg->str << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
        }
    });
}


void viewOptionsPositions(ix::WebSocket& webSocket)
{   
    std::cout << "Fetching Options positions for all currencies..." << std::endl;

    // Prepare the JSON-RPC request
    json positions_msg = {
        {"jsonrpc", "2.0"},
        {"id", 1237},
        {"method", "private/get_positions"},
        {"params", {
            {"currency", "any"},   // Fetch positions for all currencies
            {"kind", "option"}    // Specify the market type as "spot"
        }}
    };

    // Send the request
    webSocket.send(positions_msg.dump());

    // Set up a callback to handle the response
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            // Print the response
            std::cout << "Response: " << msg->str << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cerr << "Error: " << msg->errorInfo.reason << std::endl;
        }
    });
}

#endif // ACTIONS_H