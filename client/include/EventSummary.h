#ifndef EVENTSUMMARY_H
#define EVENTSUMMARY_H

#include <string>
#include <vector>
#include <map>
#include <mutex>


class EventSummary {
private:
    std::string dateTime;    // תאריך ושעה
    std::string eventName;   // שם האירוע
    std::string description; // תיאור (27 תווים בלבד)

public:
    // בנאי
    EventSummary(const std::string& dateTime, const std::string& eventName, const std::string& description);

    // השוואה בין אירועים לצורך מיון
    bool operator<(const EventSummary& other) const;

    // פונקציות גישה
    const std::string& getDateTime() const;
    const std::string& getEventName() const;
    const std::string& getDescription() const;
};

// מפת הסיכומים לפי ערוצים
extern std::map<std::string, std::vector<EventSummary>> eventSummaryMap;
std::map<std::string, std::mutex> channelMutexes;


// פונקציה להוספת אירוע לערוץ בסיכום
void addToSummary(const std::string& channelName, const std::string& rawDateTime, 
                  const std::string& eventName, const std::string& description);

// פונקציה להמרת תאריך לפורמט הנדרש
std::string formatDateTime(const std::string& rawDateTime);

#endif // EVENTSUMMARY_H
