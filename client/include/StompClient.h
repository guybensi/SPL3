#ifndef STOMPCLIENT_H
#define STOMPCLIENT_H

#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <sstream>
#include <vector>
#include <fstream>
#include <stdexcept>
#include "ConnectionHandler.h"
#include "Frame.h"
#include "EmergencyEvent.h"
#include <event.h>
#include <queue>

using namespace std;

class StompClient {
private:
    ConnectionHandler connectionHandler;
    string username;
    bool connected;
    int nextSubscriptionId;
    int nextReceiptId;
    int receiptDisconnect;
    map<string, int> topicToSubscriptionId;
    map<int, bool> gotReceipt;
    map<int, string> receiptCallbacks;
    map<string, map<string, vector<EmergencyEvent>>> eventSummaryMap;
    mutex eventSummaryMapMutex;
    thread readThread;
    thread keyboardThread;
    bool shouldTerminate;
    bool isRunning;

public:
    StompClient(const string& host, int port);
    void start();
    void stop();

private:
    void keyboardLoop();
    Frame handleLogin(const std::string& hostPort, const std::string& username, const std::string& password);
    Frame handleJoin(const string& topic);
    Frame handleExit(const string& topic);
    Frame handleReport(const std::string& file);
    Frame handleLogout();
    void sendFrame(const Frame& frame);
    void createSummary(const string& channel_name, const string& user, const string& file);
    bool isReceiptValid(const Frame& frame, int receiptId);
    void readLoop();
    void handleFrame(const Frame& response);
    vector<string> splitString(const string& str, char delimiter);
};

#endif // STOMPCLIENT_H
