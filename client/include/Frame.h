#ifndef FRAME_H
#define FRAME_H

#include <string>
#include <map>
using namespace std;

class Frame {
public:
    string command;                     // Command of the frame (e.g., CONNECT, SUBSCRIBE, etc.)
    map<string, string> headers; // Headers as key-value pairs
    string body;                        // Body of the frame

    // Parses a raw STOMP frame string into a Frame object
    static Frame parse(const string& rawFrame);

    // Converts a Frame object into a raw STOMP frame string
    string toString() const;
};

#endif // FRAME_H
