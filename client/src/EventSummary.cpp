#include "EventSummary.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include "EventSummary.h"
#include <algorithm>


// מפת הסיכומים לפי ערוצים
std::map<std::string, std::vector<EventSummary>> eventSummaryMap;

EventSummary::EventSummary(const std::string& dateTime, const std::string& eventName, const std::string& description)
    : dateTime(dateTime), eventName(eventName), description(description) {}

// השוואה בין אירועים לצורך מיון
bool EventSummary::operator<(const EventSummary& other) const {
    if (dateTime != other.dateTime) {
        return dateTime < other.dateTime;
    }
    return eventName < other.eventName;
}

// פונקציה להוספת אירוע לערוץ בסיכום
void addToSummary(const std::string& channelName, const std::string& rawDateTime,
                  const std::string& eventName, const std::string& description) {
    // וודא שלערוץ יש מנעול
    ensureChannelMutexExists(channelName);

    // נעילת המנעול של הערוץ המסוים
    std::lock_guard<std::mutex> lock(channelMutexes[channelName]);

    // המרת תאריך לפורמט הנדרש
    std::string formattedDateTime = formatDateTime(rawDateTime);

    // יצירת האירוע
    EventSummary event{formattedDateTime, eventName, description.substr(0, 27)};

    // הוספת האירוע לערוץ המתאים במפה
    eventSummaryMap[channelName].push_back(event);

    // מיון האירועים בערוץ
    std::sort(eventSummaryMap[channelName].begin(), eventSummaryMap[channelName].end());
}

void ensureChannelMutexExists(const std::string& channelName) {
    static std::mutex mutexForMutexes; // מנעול להגנה על map המנעולים
    std::lock_guard<std::mutex> lock(mutexForMutexes);

    if (channelMutexes.find(channelName) == channelMutexes.end()) {
        channelMutexes.emplace(channelName, std::mutex());
    }
}

// פונקציה להמרת תאריך לפורמט הנדרש
std::string formatDateTime(const std::string& rawDateTime) {
    std::istringstream input(rawDateTime);
    std::ostringstream output;
    int year, month, day, hour, minute, second;
    char dash1, dash2, space, colon1, colon2;

    input >> year >> dash1 >> month >> dash2 >> day >> space >> hour >> colon1 >> minute >> colon2 >> second;
    if (input.fail()) {
        throw std::invalid_argument("Invalid date format");
    }

    output << std::setfill('0') << std::setw(2) << day << "/"
           << std::setw(2) << month << "/"
           << year << " "
           << std::setw(2) << hour << ":"
           << std::setw(2) << minute;

    return output.str();
}

// פונקציות גישה
const std::string& EventSummary::getDateTime() const {
    return dateTime;
}

const std::string& EventSummary::getEventName() const {
    return eventName;
}

const std::string& EventSummary::getDescription() const {
    return description;
}
