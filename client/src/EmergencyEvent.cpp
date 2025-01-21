#include "EmergencyEvent.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <algorithm>
#include <iostream>
#include <string>

using namespace std;



// מפת הסיכומים לפי ערוצים
map<string, vector<EmergencyEvent>> eventSummaryMap;

EmergencyEvent::EmergencyEvent(const Event& e) : Event(e) {
    this->formatDateTime = formatToDateTime(to_string(e.get_date_time()));
    //------------לוודא שמאוית נכון---------------
    this->active = isFieldTrue ("active");
    this->forcesArrival = isFieldTrue ("forces arrival at scene");
}


// השוואה בין אירועים לצורך מיון
bool EmergencyEvent::operator<(const EmergencyEvent& other) const {
    if (this->get_date_time() != other.get_date_time()) {
        return this->get_date_time() < other.get_date_time();
    }
    return this->get_name() < other.get_name();
}

// פונקציה להוספת אירוע לערוץ בסיכום
void addToSummary(const Event& e) {
    // וודא שלערוץ יש מנעול
    ensureChannelMutexExists(e.get_channel_name());

    // נעילת המנעול של הערוץ המסוים
    std::lock_guard<std::mutex> lock(channelMutexes[e.get_channel_name()]); 

    // יצירת האירוע
    EmergencyEvent eventSummary(e);

    // הוספת האירוע לערוץ המתאים במפה
    eventSummaryMap[e.get_channel_name()].push_back(eventSummary);

    // מיון האירועים בערוץ
    std::sort(eventSummaryMap[eventSummary.get_channel_name()].begin(), eventSummaryMap[eventSummary.get_channel_name()].end());
}

void ensureChannelMutexExists(const std::string& channelName) {
    static std::mutex mutexForMutexes; // מנעול להגנה על map המנעולים
    std::lock_guard<std::mutex> lock(mutexForMutexes);

    if (channelMutexes.find(channelName) == channelMutexes.end()) {
        channelMutexes.emplace(channelName, std::mutex());
    }
}

// פונקציה להמרת תאריך לפורמט הנדרש
string formatToDateTime(const std::string& rawDateTime) {
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

bool EmergencyEvent::isFieldTrue(const std::string& fieldName) const {
    // אחזור על המידע הכללי
    const auto& generalInfo = get_general_information();
    auto it = generalInfo.find(fieldName);

    // בדיקה אם השדה קיים
    if (it != generalInfo.end()) {
        // הפיכת הערך ל-lowercase
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        // החזרה אם הערך הוא "true"
        return value == "true";
    }

    // אם השדה לא נמצא
    return false;
}



// פונקציות גישה
const std::string& EmergencyEvent::getFormatedDateTime() const {
    return this->formatDateTime; 
}

const bool EmergencyEvent:: getActive() const {return this->active;}
const bool EmergencyEvent:: getForcesArrival() const {return this->forcesArrival;}


