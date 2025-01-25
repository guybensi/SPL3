#include "StompProtocol.h"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    cout << "Welcome to the STOMP client. Please type 'login <host:port> <username> <password>' to connect." << endl;
    try {
        ConnectionHandler* connectionHandler = nullptr;
        StompProtocol StompProtocol(connectionHandler);
        // הפעלת התוכנית
        StompProtocol.start();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
