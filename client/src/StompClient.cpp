#include "StompProtocol.h"
#include <iostream>
#include <string>
#include <cstdlib> // לשימוש ב-std::stoi עם בדיקה בסיסית

using namespace std;

int main(int argc, char* argv[]) {
    
    // בדיקה אם מספר הפרמטרים תקין
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <host> <port>" << endl;
        return 1;
    }

    string host = argv[1];
    string portStr = argv[2];

    // בדיקה אם הפורט הוא מספר תקין
    int port;
    try {
        port = stoi(portStr);
        if (port <= 0 || port > 65535) { // פורט חייב להיות בתחום התקין
            throw invalid_argument("Invalid port range");
        }
    } catch (const exception& e) {
        cerr << "Error: Invalid port number. Please provide a valid port." << endl;
        return 1;
    }
    cout << "Connecting to " << host << " on port " << port << "..." << endl;


    try {
        // יצירת אובייקט StompProtocol והפעלתו
        StompProtocol stompProtocol(host, port);
        stompProtocol.start();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
