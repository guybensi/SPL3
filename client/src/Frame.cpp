#include "Frame.h"
#include <sstream>
#include <stdexcept>

// שימוש ב-using namespace std כדי לפשט את הקוד
using namespace std;

Frame::Frame() : command(""), headers(), body("") {}

Frame Frame::parse(const string& rawFrame) {
    Frame frame;
    istringstream stream(rawFrame);

    // Parse command
    if (!getline(stream, frame.command) || frame.command.empty()) {
        throw runtime_error("Invalid frame: missing command");
    }

    // Parse headers
    string line;
    while (getline(stream, line) && !line.empty()) {
        size_t colonPos = line.find(':');
        if (colonPos == string::npos) {
            throw runtime_error("Invalid frame: malformed header");
        }
        string key = line.substr(0, colonPos);
        string value = line.substr(colonPos + 1);
        frame.headers[key] = value;
    }

    // Parse body
    ostringstream bodyStream;
    while (getline(stream, line)) {
        bodyStream << line << '\n';
    }
    frame.body = bodyStream.str();

    return frame;
}

string Frame::toString() const {
    ostringstream ss;

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
