
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
#include "EmergencyEvent.h" // כולל את מבנה הסיכום ואת הפונקציות הרלוונטיות
#include <event.h>
#include <queue>

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
    map<string, map<string, vector<EmergencyEvent>>> eventSummaryMap;
    mutex eventSummaryMapMutex;
    mutex topicToSubscriptionIdMutex;
    mutex receiptCallbacksMutex;
    thread readThread;//from the server
    thread keyboardThread;//from the user 
    bool shouldTerminate;
    bool isRunning;
    queue<Frame> sendQueue; 
    queue<Frame> receiveQueue; 
    mutex receiveMutex;
    mutex sendMutex; // Mutex for thread-safe access to the queue
    condition_variable queueCV; // Condition variable for synchronization
    


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
        topicToSubscriptionId.clear();
        isRunning = false; // עדכון שהלקוח נעצר
        cout << "Client stopped." << endl;
    }


private:
    void keyboardLoop() {
        while (!shouldTerminate) {
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
                }else if (tokens[0] == "summary") {
                    if (tokens.size() != 4) {
                        cout << "Usage: report <file>" << endl;
                        continue;
                    }
                    createSummary(tokens[1],tokens[2],tokens[3]);
                } else {
                    cout << "Unknown command" << endl;
                }
                {
                    std::lock_guard<std::mutex> lock(sendMutex);
                    sendQueue.push(frame);
                }
                queueCV.notify_all(); // לעורר את התרד של השרת

            } catch (const exception& e) {
                cerr << "Error: " << e.what() << endl;
            }
        }
    }

    Frame handleLogin(const std::string& hostPort, const std::string& username, const std::string& password) {
        if (connected) {
            std::cerr << "The client is already logged in, log out before trying again." << std::endl;
            return;
        }

        size_t colonPos = hostPort.find(':');
        if (colonPos == std::string::npos) {
            throw std::runtime_error("Invalid host:port format");
        }

        std::string host = hostPort.substr(0, colonPos);
        int port = stoi(hostPort.substr(colonPos + 1));

        if (!connectionHandler.connect()) {
            std::cerr << "Could not connect to server" << std::endl;
            return;
        }

        // Construct the CONNECT frame
        Frame frame;
        frame.command = "CONNECT";
        frame.headers["accept-version"] = "1.2";
        frame.headers["host"] = "stomp.cs.bgu.ac.il";
        frame.headers["login"] = username;
        frame.headers["passcode"] = password;

        return frame;
    }


    Frame handleJoin(const string& topic) {
        if (!connected){
            cout << "the user is not logged in, can't join: " << topic << endl;
            return;
        }
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
        nextSubscriptionId++;
        nextReceiptId++;
        return frame;
        //-------------------------------------------------
        Frame response;
 
        if (isReceiptValid(response, nextReceiptId -1)){
            cout << "Joined topic: " << topic << endl;        }
        else{
            cout << "the frame recived is not correct!" << endl;
        }
        
    }

    Frame handleExit(const string& topic) {
        if (!connected){
            cout << "the user is not logged in, can't Exit: " << topic << endl;
            return;
        }
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
        topicToSubscriptionId.erase(topic);
        return frame;
        //------------------------
        Frame response;
        if (!connectionHandler.getFrame(response)) {
            cerr << "Failed to receive response from server during Exit" << endl;
            return;
        }
        if (isReceiptValid(response, nextReceiptId -1)){
            cout << "Exited topic: " << topic << endl;
        }
        else{
            cout << "the frame recived is not correct!" << endl;
        }
    }

    Frame handleReport(const std::string& file) {
        if (!connected){
            cout << "the user is not logged in, can't report! "  << endl;
            return;
        }
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
        for (auto& event : parsedData.events) {
            try {
                // הוספת האירוע לערוץ בסיכום
                addToSummary(event, this->username);
                event.setEventOwnerUser (this->username);

                // יצירת פריים ושליחתו לשרת
                Frame frame;
                frame.command = "SEND";
                frame.headers["destination"] = "/" + channelName;

                std::stringstream body;
                body << "user: " << username << "\n";
                body << "city: " << event.get_city() << "\n";
                body << "event name: " << event.get_name() << "\n";
                body << "date time: " << event.get_date_time() << "\n";
                body << "general information: " << "\n";
                for (const auto& info : event.get_general_information()) {
                    body << "\t" << info.first << ": " << info.second << "\n";
                }       
                body << "description: " << event.get_description() << "\n";
                frame.body = body.str();
                std::cout << "Event reported: " << event.get_name() << " to channel: " << channelName << std::endl;
                std::cout << "Report summary updated from file: " << file << std::endl;
                return frame;

            } catch (const std::exception& e) {
                std::cerr << "Error processing event: " << e.what() << std::endl;
            }
        }

    }

    Frame handleLogout() {
        if (!connected) {
            cerr << "Error: Not connected to the server." << endl;
            return;
        }
        Frame frame;
        frame.command = "DISCONNECT";
        frame.headers["receipt"] = to_string(nextReceiptId);
        receiptCallbacks[nextReceiptId++] = "logout: " + username;
        return frame;
        //------------------
        Frame response;
        if (!connectionHandler.getFrame(response)) {
            cerr << "Failed to receive response from server during Exit" << endl;
            return;
        }
        if (isReceiptValid(response, nextReceiptId -1)){
            stop();
            cout << "Logged out successfully." << endl; 
            //-----------מזה לחכות לפקודות נוספות--------
        }
        else{
            cout << "the frame recived is not correct, cant log out" << endl;
        }
    }

    void sendFrame(const Frame& frame) {
        string frameStr = frame.toString();
        if (!connectionHandler.sendFrameAscii(frameStr, '\0')) {
            connectionHandler.close();
            cerr << "Failed to send frame: " << frame.command << endl;
        }
    }
    void createSummary(const string& channel_name, const string& user, const string& file) {
        if (!connected) {
            cerr << "Error: Not connected to the server." << endl;
            return;
        }
        lock_guard<std::mutex> lock(eventSummaryMapMutex); // מנעול לוודא גישה בטוחה למפה
        if (eventSummaryMap.find(channel_name) == eventSummaryMap.end()) {
            cerr << "Error: Channel " << channel_name << " not found." << endl;
            return;
        }

        if (eventSummaryMap[channel_name].find(user) == eventSummaryMap[channel_name].end()) {
            cerr << "Error: User " << user << " not found in channel " << channel_name << "." << endl;
            return;
        }
        ofstream outFile(file, ios::trunc); // פתיחת קובץ חדש (trunc = למחוק תוכן קודם אם קיים)
        if (!outFile.is_open()) {
            cerr << "Error: Failed to create or open file: " << file << endl;
            return;
        }
        // סטטיסטיקות
        int i = 1;
        int active = 0;
        int forcesArrival = 0;
        const vector<EmergencyEvent>& userEvents = eventSummaryMap[channel_name][user];
        
        // צבירת תוכן האירועים במשתנה צדדי
        stringstream eventDetails;

        for (auto& event : userEvents) {
            eventDetails << "Report_" << i << "\n";
            eventDetails << "\tcity: " << event.get_city() << "\n";
            eventDetails << "\tdate time: " << event.getFormatedDateTime() << "\n";
            eventDetails << "\tevent name: " << event.get_name() << "\n";
            string description = event.get_description();
            if (description.length() > 30) {
                description = description.substr(0, 27) + "...";
            }
            eventDetails << "\tsummary: " << description << "\n";
            eventDetails << endl;
            if (event.getActive()) { active++; }
            if (event.getForcesArrival()) { forcesArrival++; }
            i++;
        }
        // כתיבת סטטיסטיקות בתחילת הקובץ
        outFile << "Channel " << channel_name << "\n";
        outFile << "Stats:\n";
        outFile << "Total: " << i - 1 << "\n";
        outFile << "active: " << active << "\n";
        outFile << "forces arrival at scene: " << forcesArrival << "\n";
        outFile << "\n"; 
        outFile << "Event Reports: " << "\n"; 
        // הוספת פרטי האירועים לסוף הקובץ
        outFile << eventDetails.str();
        outFile.close(); // סגירת הקובץ
        cout << "Summary written to file: " << file << endl;
    }
    bool isReceiptValid(const Frame& frame, int receiptId) {
        // בדיקה אם הפקודה בפריים היא "RECEIPT"
        if (frame.command != "RECEIPT") {
            return false;
        }

        // בדיקה אם ה-header "receipt-id" תואם ל-ID שציפינו לו
        auto it = frame.headers.find("receipt-id");
        if (it != frame.headers.end() && it->second == std::to_string(receiptId)) {
            return true;
        }

        return false;
    }


    void readLoop() {
        while (!shouldTerminate) {
            Frame frameToSend;
            {
                std::unique_lock<std::mutex> lock(sendMutex);
                queueCV.wait(lock, [this]() { return !sendQueue.empty() || !readQueue.empty() || shouldTerminate; });

                if (shouldTerminate) break;

                if (!sendQueue.empty()) {
                    frameToSend = sendQueue.front();
                    sendQueue.pop();
                }
            }
            // אם יש הודעה לשליחה, שולחים אותה וממתינים לתשובה
            if (!frameToSend.command.empty()) {
                sendFrame(frameToSend);

                // קריאת תשובה מהשרת
                Frame response;
                if (connectionHandler.getFrame(response)) {
                    handleFrame(response); // טיפול בתגובה שהתקבלה
                } else {
                    std::cerr << "Failed to receive response from server." << std::endl;
                }
            }
            Frame receivedFrame;
            {
                std::lock_guard<std::mutex> lock(receiveMutex);
                while (!receiveQueue.empty()) {
                    receivedFrame = receiveQueue.front();
                    receiveQueue.pop();
                    handleFrame(receivedFrame);
                }
            }
            
        }
    }

    void handleFrame(const Frame& response) {
        if (response.command == "CONNECTED") {
            connected = true;
            this->username = username;
            std::cout << "Login successful" << std::endl;
        } else if (response.command == "RECEIPT") {
            int receiptId = stoi(response.headers.at("receipt-id"));
            if (receiptCallbacks.find(receiptId) != receiptCallbacks.end()) {
                cout << receiptCallbacks[receiptId] << endl;
                receiptCallbacks.erase(receiptId);
            }
        } else if (response.command == "MESSAGE") {
            cout << "Message: " << response.body << endl;
        } else if (response.command == "ERROR") {
            std::string errorMessage = response.headers["message"];
            if (errorMessage == "User already logged in") {
                std::cerr << "User already logged in." << std::endl;
            } else if (errorMessage == "Wrong password") {
                std::cerr << "Wrong password." << std::endl;
            } else {
                std::cerr << "Login failed: " << errorMessage << std::endl;
            }
            connectionHandler.close();
        } else {
            std::cerr << "Unexpected Frame recived " << response.command << std::endl;
            connectionHandler.close();
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

