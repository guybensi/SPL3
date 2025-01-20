#ifndef EMERGENCYEVENT_H
#define EMERGENCYEVENT_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "event.h"


class EmergencyEvent : public Event {

private:

    std::string formatDateTime;    // תאריך ושעה
    bool active;
    bool forcesArrival;

public:
    // בנאי
    EmergencyEvent(const Event& e);

    // השוואה בין אירועים לצורך מיון
    bool operator<(const EmergencyEvent& other) const;

    bool isFieldTrue(const std::string& fieldName) const;

    // פונקציות גישה
    const std::string& getFormatedDateTime() const;
     
    const bool getActive();
    const bool getForcesArrival();
    
};

// מפת הסיכומים לפי ערוצים
extern std::map<std::string, std::vector<EmergencyEvent>> eventSummaryMap;
std::map<std::string, std::mutex> channelMutexes;


// פונקציה להוספת אירוע לערוץ בסיכום
void addToSummary(Event e);

// פונקציה להמרת תאריך לפורמט הנדרש
std::string formatDateTime(const std::string& rawDateTime);



#endif // EMERGENCYEVENT_H
