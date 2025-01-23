#include "StompProtocol.h"
#include <iostream>
#include <string>

using namespace std;

int main() {
    try {
        // יצירת אובייקט StompProtocol
        StompProtocol stompProtocol("", 0);

        // הפעלת התוכנית
        stompProtocol.start();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
