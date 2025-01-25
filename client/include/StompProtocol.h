#ifndef STOMPCLIENT_H
#define STOMPCLIENT_H

#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include "ConnectionHandler.h"
#include "Frame.h"
#include "EmergencyEvent.h"
using namespace std;

class StompProtocol {
private:
    ConnectionHandler* CH;
    string username;
    bool connected;
    int nextSubscriptionId;
    int nextReceiptId;
    int receiptDisconnect;
    map<string, int> topicToSubscriptionId;
    map<int, bool> gotReceipt;
    mutex gotReceiptMutex;
    map<int, std::string> receiptCallbacks;
    mutex receiptCallbacksMutex;
    map<std::string, std::map<string, vector<EmergencyEvent>>> eventSummaryMap;
    mutex eventSummaryMapMutex;
    thread readThread;
    thread keyboardThread;
    bool shouldTerminate;
    bool isRunning;

public:
    StompProtocol(); // בנאי ברירת מחדל
    StompProtocol(ConnectionHandler* connectionHandler); // **בנאי מותאם אישית**
    //rule of 3
    StompProtocol(const StompProtocol& SP);
    StompProtocol& operator=(const StompProtocol&);
    ~StompProtocol();
    void start();
    void stop();
    

private:
    void keyboardLoop();
    Frame handleLogin(const string& hostPort, const string& username, const string& password);
    Frame handleJoin(const string& topic);
    Frame handleExit(const string& topic);
    Frame handleReport(const string& file);
    Frame handleLogout();
    void sendFrame(const Frame& frame);
    void createSummary(const string& channel_name, const string& user, const string& file);
    bool isReceiptValid(const Frame& frame, int receiptId);
    void readLoop();
    void handleFrame(const Frame& response);
    vector<string> splitString(const string& str, char delimiter);
};

#endif // STOMPCLIENT_H
