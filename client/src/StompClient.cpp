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
#include "EventSummary.h" // כולל את מבנה הסיכום ואת הפונקציות הרלוונטיות
#include <event.h>


using namespace std;

class StompClient {
private:
    ConnectionHandler connectionHandler; // ניהול חיבור ו-I/O
    string username;
    bool connected;
    int nextSubscriptionId;
    int nextReceiptId;
    map<string, int> topicToSubscriptionId;
    map<int, string> receiptCallbacks;
    mutex mutex;
    thread readThread;//from the server
    thread keyboardThread;//from the user 
    bool shouldTerminate;
    bool isRunning;
    map<string, vector<EventSummary>> eventSummaryMap;


public:
    StompClient(const string& host, int port)
        : connectionHandler(host, port), username(""), connected(false),
          nextSubscriptionId(0), nextReceiptId(0), shouldTerminate(false),isRunning(false) {}//לבדוק מי קורא לו

    void start() {
        if (isRunning) {
            cerr << "Client is already running, stop it and try again" << endl;
            return;
        }

        if (!connectionHandler.connect()) {
            cerr << "Failed to connect to server." << endl;
            return;
        }

        isRunning = true; // מסמן שהלקוח פעיל
        cout << "Connected to server." << endl;

        // הפעלת תהליכון לקריאת הודעות מהשרת
        readThread = thread([this]() { readLoop(); });

        // הפעלת תהליכון לקלט משתמש
        keyboardThread = thread([this]() { keyboardLoop(); });
    }

    void stop() {
        if (!isRunning) {
            cerr << "Client is not running." << endl;
            return;
        }

        shouldTerminate = true; // מסמן לולאות לעצור
        connectionHandler.close(); // סגירת החיבור

        // סיום תהליכון הקריאה
        if (readThread.joinable()) {
            readThread.join();
        }

        // סיום תהליכון הקלט
        if (keyboardThread.joinable()) {
            keyboardThread.join();
        }

        isRunning = false; // עדכון שהלקוח נעצר
        cout << "Client stopped." << endl;
    }


private:
    void keyboardLoop() {
        while (!shouldTerminate) {
            string line;
            getline(cin, line);

            vector<string> tokens = splitString(line, ' ');
            if (tokens.empty()) continue;

            try {
                if (tokens[0] == "login") {
                    if (tokens.size() != 4) {
                        cout << "Usage: login <host:port> <username> <password>" << endl;
                        continue;
                    }
                    handleLogin(tokens[1], tokens[2], tokens[3]);
                } else if (tokens[0] == "join") {
                    if (tokens.size() != 2) {
                        cout << "Usage: join <topic>" << endl;
                        continue;
                    }
                    handleJoin(tokens[1]);
                } else if (tokens[0] == "exit") {
                    if (tokens.size() != 2) {
                        cout << "Usage: exit <topic>" << endl;
                        continue;
                    }
                    handleExit(tokens[1]);
                } else if (tokens[0] == "report") {
                    if (tokens.size() != 2) {
                        cout << "Usage: report <file>" << endl;
                        continue;
                    }
                    handleReport(tokens[1]);
                } else if (tokens[0] == "logout") {
                    handleLogout();
                }else if (tokens[0] == "summary") {
                    printSummary();
                } else {
                    cout << "Unknown command" << endl;
                }
            } catch (const exception& e) {
                cerr << "Error: " << e.what() << endl;
            }
        }
    }

    void handleLogin(const string& hostPort, const string& username, const string& password) {
        size_t colonPos = hostPort.find(':');
        if (colonPos == string::npos) {
            throw runtime_error("Invalid host:port format");
        }

        string host = hostPort.substr(0, colonPos);
        int port = stoi(hostPort.substr(colonPos + 1));

        Frame frame;
        frame.command = "CONNECT";
        frame.headers["accept-version"] = "1.2";
        frame.headers["host"] = host;
        frame.headers["login"] = username;
        frame.headers["passcode"] = password;

        sendFrame(frame);
        this->username = username;
        connected = true;
        cout << "Login successful." << endl;
    }

    void handleJoin(const string& topic) {
        if (topicToSubscriptionId.find(topic) != topicToSubscriptionId.end()) {
        cerr << "Already subscribed to topic: " << topic << endl;
        return;
        }
        Frame frame;
        frame.command = "SUBSCRIBE";
        frame.headers["destination"] = "/" + topic;
        frame.headers["id"] = to_string(nextSubscriptionId);
        frame.headers["receipt"] = to_string(nextReceiptId);
        // עדכון המפה עם המנוי החדש
        topicToSubscriptionId[topic] = nextSubscriptionId;
        // עדכון הקלטות לקבלות
        receiptCallbacks[nextReceiptId] = "Joined topic: " + topic;
        // שליחת פריים
        sendFrame(frame);
        nextSubscriptionId++;
        nextReceiptId++;
        cout << "Joined topic: " << topic << endl;
    }

    void handleExit(const string& topic) {
        if (topicToSubscriptionId.find(topic) == topicToSubscriptionId.end()) {
            cerr << "Not subscribed to topic: " << topic << endl;
            return;
        }

        int subscriptionId = topicToSubscriptionId[topic];
        Frame frame;
        frame.command = "UNSUBSCRIBE";
        frame.headers["id"] = to_string(subscriptionId);
        frame.headers["receipt"] = to_string(nextReceiptId);

        receiptCallbacks[nextReceiptId++] = "Exited topic: " + topic;
        sendFrame(frame);
        topicToSubscriptionId.erase(topic);
        cout << "Exited topic: " << topic << endl;
    }

    void handleReport(const std::string& file) {
        // פריסת הקובץ באמצעות הפונקציה `parseEventsFile`
        names_and_events parsedData;
        try {
            parsedData = parseEventsFile(file);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing events file: " << e.what() << std::endl;
            return;
        }

        // שמירת שם הערוץ מתוך הנתונים
        std::string channelName = parsedData.channel_name;
        if (channelName.empty()) {
            std::cerr << "Error: Channel name is missing in the file." << std::endl;
            return;
        }

        // עיבוד ושליחת כל אירוע
        for (const auto& event : parsedData.events) {
            try {
                // הוספת האירוע לערוץ בסיכום
                addToSummary(channelName, std::to_string(event.get_date_time()), 
                            event.get_name(), event.get_description());

                // יצירת פריים ושליחתו לשרת
                Frame frame;
                frame.command = "SEND";
                frame.headers["destination"] = "/" + channelName;

                std::stringstream body;
                body << "user: " << username << "\n";
                body << "event name: " << event.get_name() << "\n";
                body << "date time: " << event.get_date_time() << "\n";
                body << "description: " << event.get_description().substr(0, 27) << "\n";

                for (const auto& info : event.get_general_information()) {
                    body << info.first << ": " << info.second << "\n";
                }

                frame.body = body.str();
                sendFrame(frame);

                std::cout << "Event reported: " << event.get_name() << " to channel: " << channelName << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error processing event: " << e.what() << std::endl;
            }
        }

        std::cout << "Report summary updated from file: " << file << std::endl;
    }

    void handleLogout() {
        Frame frame;
        frame.command = "DISCONNECT";
        frame.headers["receipt"] = "disconnect";

        sendFrame(frame);
        stop();
        cout << "Logged out successfully." << endl;
    }

    void sendFrame(const Frame& frame) {
        string frameStr = frame.toString();
        if (!connectionHandler.sendFrameAscii(frameStr, '\0')) {
            cerr << "Failed to send frame: " << frame.command << endl;
        }
    }
    void printSummary() {
        lock_guard<std::mutex> lock(mutex); // ודא שמשתמשים ב-mutex נכון
        cout << "Event Summary:\n";

        // מעבר על כל ערוץ במפה
        for (const auto& [channelName, events] : eventSummaryMap) {
            cout << "Channel: " << channelName << "\n"; // הצגת שם הערוץ

            // מעבר על כל האירועים בערוץ
            for (const auto& event : events) {
                cout << "  " << event.getDateTime() << " | " 
                    << event.getEventName() << " | " 
                    << event.getDescription() << "\n";
            }
            cout << endl; // ריווח בין ערוצים
        }
    }




    void readLoop() {
        while (!shouldTerminate) {
            string message;
            if (!connectionHandler.getFrameAscii(message, '\0')) {
                cerr << "Connection closed by server." << endl;
                break;
            }

            Frame frame = Frame::parse(message);
            handleFrame(frame);
        }
    }

    void handleFrame(const Frame& frame) {
        if (frame.command == "CONNECTED") {
            cout << "Server response: CONNECTED" << endl;
        } else if (frame.command == "RECEIPT") {
            int receiptId = stoi(frame.headers.at("receipt-id"));
            if (receiptCallbacks.find(receiptId) != receiptCallbacks.end()) {
                cout << receiptCallbacks[receiptId] << endl;
                receiptCallbacks.erase(receiptId);
            }
        } else if (frame.command == "MESSAGE") {
            cout << "Message: " << frame.body << endl;
        } else if (frame.command == "ERROR") {
            cerr << "Error from server: " << frame.headers["message"] << endl;
        } else {
            cerr << "Unknown frame received: " << frame.command << endl;
        }
    }

    vector<string> splitString(const string& str, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(str);
        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <host> <port>" << endl;
        return 1;
    }

    string host = argv[1];
    int port = stoi(argv[2]);

    StompClient client(host, port);
    client.start();

    return 0;
}
