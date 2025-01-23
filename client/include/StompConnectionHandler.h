#pragma once

#include "ConnectionHandler.h"
#include <memory>
#include <string>

class StompConnectionHandler {
private:
    std::unique_ptr<ConnectionHandler> connectionHandler;

public:
    // Default constructor
    StompConnectionHandler();

    // Destructor
    virtual ~StompConnectionHandler();

    // Set connection with a new host and port
    void setConnection(const std::string& host, short port);

    // Forward methods to the current connection handler
    bool connect();
    void close();
    bool sendFrameAscii(const std::string& frame, char delimiter);
    bool getFrameAscii(std::string& frame, char delimiter);
};
