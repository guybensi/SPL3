#include "StompProtocol.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>

using namespace std;


StompProtocol::StompProtocol(ConnectionHandler* connectionHandler)
    : CH(connectionHandler), username(""), connected(false), nextSubscriptionId(0), nextReceiptId(0),
      receiptDisconnect(-1), topicToSubscriptionId(), gotReceipt(), gotReceiptMutex(), 
      receiptCallbacks(), receiptCallbacksMutex(), eventSummaryMap(), eventSummaryMapMutex(),
      readThread(), keyboardThread(), shouldTerminate(false), isRunning(false) {

}
StompProtocol::StompProtocol(const StompProtocol& SP){}

StompProtocol& StompProtocol::operator=(const StompProtocol&){
    return *this;
}

StompProtocol::~StompProtocol() {
    if (isRunning) {
        stop();   
    }
    delete CH;
}
void StompProtocol::start() {

    if (isRunning) {
        cerr << "Client is already running, stop it and try again." << endl;
        return;
    }

    isRunning = true;


    // הפעלת לולאות הקלט והפלט
    keyboardThread = thread([this]() { keyboardLoop(); });
    readThread = thread([this]() { readLoop(); });

    // המתנה לסיום הלולאות
    keyboardThread.join();
    readThread.join();
}


void StompProtocol::stop() {
    if (!isRunning) {
        cerr << "Client is not running." << endl;
        return;
    }

    shouldTerminate = true;
    CH->close();

    if (readThread.joinable()) {
        readThread.join();
    }

    if (keyboardThread.joinable()) {
        keyboardThread.join();
    }

    topicToSubscriptionId.clear();
    isRunning = false;
    cout << "Logged out successfully." << endl;
    cout << "Client stopped." << endl;
}



void StompProtocol::keyboardLoop() {
    while (!shouldTerminate) {
        cout << "started keyboardLoop" << endl;
        string line;
        getline(cin, line);
        Frame frame;
        vector<string> tokens = splitString(line, ' ');

        if (tokens.empty()) continue;

        try {
            if (tokens[0] == "login") {
                if (tokens.size() != 4) {
                    cout << "Usage: login <host:port> <username> <password>" << endl;
                    continue;
                }
                frame = handleLogin(tokens[1], tokens[2], tokens[3]);
            } else if (tokens[0] == "join") {
                if (tokens.size() != 2) {
                    cout << "Usage: join <topic>" << endl;
                    continue;
                }
                frame = handleJoin(tokens[1]);
            } else if (tokens[0] == "exit") {
                if (tokens.size() != 2) {
                    cout << "Usage: exit <topic>" << endl;
                    continue;
                }
                frame = handleExit(tokens[1]);
            } else if (tokens[0] == "report") {
                if (tokens.size() != 2) {
                    cout << "Usage: report <file>" << endl;
                    continue;
                }
                frame = handleReport(tokens[1]);
            } else if (tokens[0] == "logout") {
                frame = handleLogout();
            } else if (tokens[0] == "summary") {
                if (tokens.size() != 4) {
                    cout << "Usage: summary <channel_name> <user> <file>" << endl;
                    continue;
                }
                createSummary(tokens[1], tokens[2], tokens[3]);
            } else {
                cout << "Unknown command" << endl;
            }

            if (!frame.command.empty()) {
                sendFrame(frame);
            }

        } catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
        }
    }
}

Frame StompProtocol::handleLogin(const string& hostPort, const string& username, const string& password) {
    if (connected) {
        cerr << "The client is already logged in, log out before trying again." << endl;
        return {};
    }

    size_t colonPos = hostPort.find(':');
    if (colonPos == string::npos) {
        throw runtime_error("Invalid host:port format");
    }

    string host = hostPort.substr(0, colonPos);
    short port = static_cast<short>(stoi(hostPort.substr(colonPos + 1)));

    if (host.empty() ||  username.empty() || password.empty()) {//|| port.empty()
        std::cerr << "Missing one or more arguments. Expected <host:port> <username> <password>." << std::endl;
        return {};
    }

    CH = new ConnectionHandler(host, port);
    if(!CH->connect()){
        std::cerr << "Coulden't connect to server...." << std::endl;
        return {};
    }

    Frame frame;
    frame.command = "CONNECT";
    frame.headers["accept-version"] = "1.2";
    frame.headers["host"] = "stomp.cs.bgu.ac.il";
    frame.headers["login"] = username;
    frame.headers["passcode"] = password;

    this->username = username;
    return frame;
}


Frame StompProtocol::handleJoin(const string& topic) {
    if (!connected) {
        cerr << "User is not logged in, can't join: " << topic << endl;
        return {};
    }
    if (topicToSubscriptionId.find(topic) != topicToSubscriptionId.end()) {
        cerr << "Already subscribed to topic: " << topic << endl;
        return {};
    }

    Frame frame;
    frame.command = "SUBSCRIBE";
    frame.headers["destination"] = "/" + topic;
    frame.headers["id"] = to_string(nextSubscriptionId);
    frame.headers["receipt"] = to_string(nextReceiptId);

    topicToSubscriptionId[topic] = nextSubscriptionId;

    lock_guard<mutex> gotReceiptLock(gotReceiptMutex); // שם מנעול שונה
    gotReceipt[nextReceiptId] = false;

    lock_guard<mutex> receiptCallbacksLock(receiptCallbacksMutex); // שם מנעול שונה
    receiptCallbacks[nextReceiptId] = "Joined topic: " + topic;

    ++nextSubscriptionId;
    ++nextReceiptId;
    return frame;
}


Frame StompProtocol::handleExit(const string& topic) {
    if (!connected) {
        cerr << "User is not logged in, can't exit: " << topic << endl;
        return {};
    }
    if (topicToSubscriptionId.find(topic) == topicToSubscriptionId.end()) {
        cerr << "Not subscribed to topic: " << topic << endl;
        return {};
    }

    int subscriptionId = topicToSubscriptionId[topic];
    Frame frame;
    frame.command = "UNSUBSCRIBE";
    frame.headers["id"] = to_string(subscriptionId);
    frame.headers["receipt"] = to_string(nextReceiptId);

    lock_guard<mutex> gotReceiptLock(gotReceiptMutex); // שם מנעול שונה
    gotReceipt[nextReceiptId] = false;

    lock_guard<mutex> receiptCallbacksLock(receiptCallbacksMutex); // שם מנעול שונה
    receiptCallbacks[nextReceiptId] = "Exited topic: " + topic;
    topicToSubscriptionId.erase(topic);

    ++nextReceiptId;
    return frame;
}


Frame StompProtocol::handleReport(const string& file) {
    if (!connected) {
        cerr << "User is not logged in, can't report." << endl;
        return {};
    }

    // Implement file parsing and reporting logic here
    return {};
}

Frame StompProtocol::handleLogout() {
    if (!connected) {
        cerr << "User is not connected to the server." << endl;
        return {};
    }

    Frame frame;
    frame.command = "DISCONNECT";
    frame.headers["receipt"] = to_string(nextReceiptId);

    receiptDisconnect = nextReceiptId;

    // שינוי שמות המנעולים למנוע קונפליקט
    lock_guard<mutex> gotReceiptLock(gotReceiptMutex);
    gotReceipt[nextReceiptId] = false;

    lock_guard<mutex> receiptCallbacksLock(receiptCallbacksMutex);
    receiptCallbacks[nextReceiptId] = "Logged out.";

    ++nextReceiptId;
    return frame;
}


void StompProtocol::sendFrame(const Frame& frame) {
    string frameStr = frame.toString();
    if (!CH->sendFrameAscii(frameStr, '\0')) {
        CH->close();
        cerr << "Failed to send frame: " << frame.command << endl;
    }
}

void StompProtocol::createSummary(const string& channel_name, const string& user, const string& file) {
    lock_guard<mutex> lock(eventSummaryMapMutex);

    if (eventSummaryMap.find(channel_name) == eventSummaryMap.end()) {
        cerr << "Channel not found: " << channel_name << endl;
        return;
    }

    if (eventSummaryMap[channel_name].find(user) == eventSummaryMap[channel_name].end()) {
        cerr << "User not found in channel: " << user << endl;
        return;
    }

    ofstream outFile(file, ios::trunc);
    if (!outFile.is_open()) {
        cerr << "Failed to open file: " << file << endl;
        return;
    }

    // Write summary to file (custom logic needed here)
    outFile.close();
}

bool StompProtocol::isReceiptValid(const Frame& frame, int receiptId) {
    auto it = frame.headers.find("receipt-id");
    return frame.command == "RECEIPT" && it != frame.headers.end() && it->second == to_string(receiptId);
}

void StompProtocol::readLoop() {
    
    while (!shouldTerminate) {
        cout << "started readLoop"  << endl;
        Frame response;
        if (CH->getFrame(response)) {
            handleFrame(response);
        } else {
            cerr << "Failed to receive response from server." << endl;
        }
    }
}

void StompProtocol::handleFrame(const Frame& response) {
    if (response.command == "CONNECTED") {
        connected = true;
        cout << "Login successful." << endl;
    } else if (response.command == "RECEIPT") {
        int receiptId = stoi(response.headers.at("receipt-id"));
        
        lock_guard<mutex> gotReceiptLock(gotReceiptMutex); // שם מנעול ייחודי
        if (gotReceipt[receiptId]) {
            lock_guard<mutex> receiptCallbacksLock(receiptCallbacksMutex); // שם מנעול ייחודי
            cout << receiptCallbacks[receiptId] << endl;
            gotReceipt.erase(receiptId);
            receiptCallbacks.erase(receiptId);
        }
        if (receiptId == receiptDisconnect) {
            stop();
        }
    } else if (response.command == "ERROR") {
        cerr << "Error frame received: " << response.body << endl;
    } else {
        cerr << "Unexpected frame received: " << response.command << endl;
    }
}


vector<string> StompProtocol::splitString(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}
