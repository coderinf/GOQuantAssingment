#include <iostream>
#include <nlohmann/json.hpp> // Include the JSON library
#include "actions.h"         // Include the header file for actions
#include <winsock2.h>        // Include Winsock2 for WSAStartup and WSACleanup

using json = nlohmann::json; // Declare the alias for JSON

void displaySpotMenu(ix::WebSocket& webSocket)
{
    while (true)
    {
        std::cout << "\nSpot Market Menu:\n";
        std::cout << "1. Place Spot Order\n";
        std::cout << "2. Cancel Spot Order\n";
        std::cout << "3. Edit Spot Order\n";
        std::cout << "4. Get Spot Orderbook\n";
        std::cout << "5. Get Spot Positions\n";
        std::cout << "6. Exit to Main Menu\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            placeSpotOrder(webSocket);
            break;
        case 2:
            cancelSpotOrder(webSocket);
            break;
        case 3:
            ModifySpotOrder(webSocket); // Spot-specific orderbook retrieval
            break;
        case 4:
            getSpotOrderbook(webSocket); // Spot-specific orderbook retrieval
            break;
        case 5:
            getSpotPositions(webSocket); // Spot-specific orderbook retrieval
            break;        
        case 6:
            return; // Exit to main menu
        default:
            std::cout << "Invalid choice. Please try again.\n";
        }
    }
}

void displayFuturesMenu(ix::WebSocket& webSocket)
{
    while (true)
    {
        std::cout << "\nFutures Market Menu:\n";
        std::cout << "1. Place Futures Order\n";
        std::cout << "2. Cancel Futures Order\n";
        std::cout << "3. Modify Futures Order\n";
        std::cout << "4. Get Futures Orders\n";
        std::cout << "5. View Futures Positions\n";
        std::cout << "6. Exit to Main Menu\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            placeFuturesOrder(webSocket);
            break;
        case 2:
            cancelFuturesOrder(webSocket);
            break;
        case 3:
            modifyFuturesOrder(webSocket); // Futures-specific order modification
            break;
        case 4:
            getFuturesOrders(webSocket);    
        case 5:
            viewFuturesPositions(webSocket); // Futures-specific position viewing
            break;
        case 6:
            return; // Exit to main menu
        default:
            std::cout << "Invalid choice. Please try again.\n";
        }
    }
}

void displayOptionsMenu(ix::WebSocket& webSocket)
{
    while (true)
    {
        std::cout << "\nOptions Market Menu:\n";
        std::cout << "1. Place Options Order\n";
        std::cout << "2. Cancel Options Order\n";
        std::cout << "3. View Options Positions\n";
        std::cout << "4. Exit to Main Menu\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            placeOptionsOrder(webSocket);
            break;
        case 2:
            cancelOptionsOrder(webSocket);
            break;
        case 3:
            getOptionsOrders(webSocket); // Options-specific position viewing
            break;    
        case 4:
            viewOptionsPositions(webSocket); // Options-specific position viewing
            break;    
        case 5:
            return; // Exit to main menu
        default:
            std::cout << "Invalid choice. Please try again.\n";
        }
    }
}

void displayMainMenu(ix::WebSocket& webSocket)
{
    while (true)
    {
        std::cout << "\nSelect Market Type:\n";
        std::cout << "1. Spot\n";
        std::cout << "2. Futures\n";
        std::cout << "3. Options\n";
        std::cout << "4. Exit\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            displaySpotMenu(webSocket);
            break;
        case 2:
            displayFuturesMenu(webSocket);
            break;
        case 3:
            displayOptionsMenu(webSocket);
            break;
        case 4:
            std::cout << "Exiting...\n";
            return; // Exit the program
        default:
            std::cout << "Invalid choice. Please try again.\n";
        }
    }
}

int main()
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed. Exiting...\n";
        return 1;
    }

    // Display a welcome message
    std::cout << "Welcome to the Deribit Trading System!" << std::endl;

    // Hardcoded API credentials
    std::string client_id = "Om1upWjs";
    std::string client_secret = "VSSM9PkV4nELHUghyejnvgWJX_A9k6_AdzlpommToQU";

    ix::WebSocket webSocket;

    // Set the WebSocket URL
    webSocket.setUrl("wss://test.deribit.com/ws/api/v2");

    // Authenticate the API credentials
    if (!authenticate(webSocket, client_id, client_secret))
    {
        std::cerr << "Authentication failed. Exiting...\n";
        WSACleanup(); // Clean up Winsock
        return 1;
    }

    // Display the main menu for further actions
    displayMainMenu(webSocket);

    // Clean up Winsock
    WSACleanup();

    return 0;
}