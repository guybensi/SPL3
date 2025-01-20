#ifndef FRAME_H
#define FRAME_H

#include <string>
#include <map>

class Frame {
public:
    std::string command;                     // Command of the frame (e.g., CONNECT, SUBSCRIBE, etc.)
    std::map<std::string, std::string> headers; // Headers as key-value pairs
    std::string body;                        // Body of the frame

    // Parses a raw STOMP frame string into a Frame object
    static Frame parse(const std::string& rawFrame);

    // Converts a Frame object into a raw STOMP frame string
    std::string toString() const;
};

#endif // FRAME_H
