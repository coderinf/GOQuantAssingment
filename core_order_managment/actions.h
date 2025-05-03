#ifndef ACTIONS_H
#define ACTIONS_H

#include <iostream>
#include <nlohmann/json.hpp>
#include <ixwebsocket/IXWebSocket.h>

using json = nlohmann::json;

// Function to authenticate the user
bool authenticate(ix::WebSocket& webSocket, const std::string& client_id, const std::string& client_secret);

// Spot Market Actions
void placeSpotOrder(ix::WebSocket& webSocket);
void cancelSpotOrder(ix::WebSocket& webSocket);
void ModifySpotOrder(ix::WebSocket& webSocket);
void getSpotOrderbook(ix::WebSocket& webSocket);
void getSpotPositions(ix::WebSocket& webSocket);

// Futures Market Actions
void placeFuturesOrder(ix::WebSocket& webSocket);
void cancelFuturesOrder(ix::WebSocket& webSocket);
void modifyFuturesOrder(ix::WebSocket& webSocket);
void getFuturesOrders(ix::WebSocket& webSocket);
void viewFuturesPositions(ix::WebSocket& webSocket);

// Options Market Actions
void placeOptionsOrder(ix::WebSocket& webSocket);
void cancelOptionsOrder(ix::WebSocket& webSocket);
void getOptionsOrders(ix::WebSocket& webSocket);
void viewOptionsPositions(ix::WebSocket& webSocket);

#endif // ACTIONS_H