#include "StompConnectionHandler.h"

// Constructor - starts without a connection
StompConnectionHandler::StompConnectionHandler() : connectionHandler(nullptr) {}

// Destructor - ensures the connection is properly closed
StompConnectionHandler::~StompConnectionHandler() {
    if (connectionHandler) {
        connectionHandler->close();
    }
}

// Set a new connection with the provided host and port
void StompConnectionHandler::setConnection(const std::string& host, short port) {
    if (connectionHandler) {
        connectionHandler->close(); // Close existing connection if any
    }

    // Create a new ConnectionHandler instance
    connectionHandler = std::make_unique<ConnectionHandler>(host, port);

    if (!connectionHandler->connect()) {
        throw std::runtime_error("Failed to establish connection with host: " + host);
    }
}

// Forward `connect` call to the current connection handler
bool StompConnectionHandler::connect() {
    if (!connectionHandler) {
        throw std::runtime_error("No connection handler available.");
    }
    return connectionHandler->connect();
}

// Forward `close` call to the current connection handler
void StompConnectionHandler::close() {
    if (connectionHandler) {
        connectionHandler->close();
    }
}

// Forward `sendFrameAscii` call to the current connection handler
bool StompConnectionHandler::sendFrameAscii(const std::string& frame, char delimiter) {
    if (!connectionHandler) {
        throw std::runtime_error("No connection handler available.");
    }
    return connectionHandler->sendFrameAscii(frame, delimiter);
}

// Forward `getFrameAscii` call to the current connection handler
bool StompConnectionHandler::getFrameAscii(std::string& frame, char delimiter) {
    if (!connectionHandler) {
        throw std::runtime_error("No connection handler available.");
    }
    return connectionHandler->getFrameAscii(frame, delimiter);
}
