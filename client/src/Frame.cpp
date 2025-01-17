#include "Frame.h"
#include <sstream>
#include <stdexcept>

// מחלקת Frame - מימוש פונקציות מחלקה

Frame Frame::parse(const std::string& rawFrame) {
    Frame frame;
    std::istringstream stream(rawFrame);

    // Parse command
    if (!std::getline(stream, frame.command) || frame.command.empty()) {
        throw std::runtime_error("Invalid frame: missing command");
    }

    // Parse headers
    std::string line;
    while (std::getline(stream, line) && !line.empty()) {
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            throw std::runtime_error("Invalid frame: malformed header");
        }
        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);
        frame.headers[key] = value;
    }

    // Parse body
    std::ostringstream bodyStream;
    while (std::getline(stream, line)) {
        bodyStream << line << '\n';
    }
    frame.body = bodyStream.str();

    return frame;
}

std::string Frame::toString() const {
    std::ostringstream ss;

    // Append command
    ss << command << "\n";

    // Append headers
    for (const auto& header : headers) {
        ss << header.first << ":" << header.second << "\n";
    }

    // Append empty line and body
    ss << "\n";
    if (!body.empty()) {
        ss << body;
    }

    // Append null terminator
    ss << '\0';

    return ss.str();
}
