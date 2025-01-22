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
map<string, map<string, vector<EmergencyEvent>>> eventSummaryMap;

EmergencyEvent::EmergencyEvent(const Event& e) : Event(e), formatDateTime(""), active(false), forcesArrival (false) {
    this->formatDateTime = formatToDateTime(to_string(e.get_date_time()));
    this->active = isFieldTrue ("active");
    this->forcesArrival = isFieldTrue ("forces_arrival_at_scene");
}


// השוואה בין אירועים לצורך מיון
bool EmergencyEvent::operator<(const EmergencyEvent& other) const {
    if (this->get_date_time() != other.get_date_time()) {
        return this->get_date_time() < other.get_date_time();
    }
    return this->get_name() < other.get_name();
}

// פונקציה להוספת אירוע לערוץ בסיכום
void addToSummary(const Event& e, const string& username) {
    // וודא שלערוץ יש מנעול
    ensureChannelMutexExists(e.get_channel_name());

    // נעילת המנעול של הערוץ המסוים
    lock_guard<mutex> lock(channelMutexes[e.get_channel_name()]);

    // יצירת האירוע
    EmergencyEvent eventSummary(e);

    // הוספת האירוע לערוץ ולמשתמש המתאים
    eventSummaryMap[e.get_channel_name()][username].push_back(eventSummary);

    // מיון האירועים של המשתמש בתוך הערוץ
    sort(eventSummaryMap[e.get_channel_name()][username].begin(),
         eventSummaryMap[e.get_channel_name()][username].end());
}


void ensureChannelMutexExists(const string& channelName) {
    static mutex mutexForMutexes; // מנעול להגנה על map המנעולים
    lock_guard<mutex> lock(mutexForMutexes);

    if (channelMutexes.find(channelName) == channelMutexes.end()) {
        channelMutexes.emplace(channelName, mutex());
    }
}

// פונקציה להמרת תאריך לפורמט הנדרש
string formatToDateTime(const string& rawDateTime) {
    istringstream input(rawDateTime);
    ostringstream output;
    int year, month, day, hour, minute, second;
    char dash1, dash2, space, colon1, colon2;

    input >> year >> dash1 >> month >> dash2 >> day >> space >> hour >> colon1 >> minute >> colon2 >> second;
    if (input.fail()) {
        cerr << "Invalid date format" << endl;
        return "-1";
    }

    output << setfill('0') << setw(2) << day << "/"
           << setw(2) << month << "/"
           << year << " "
           << setw(2) << hour << ":"
           << setw(2) << minute;

    return output.str();
}

bool EmergencyEvent::isFieldTrue(const string& fieldName) const {
    // אחזור על המידע הכללי
    const auto& generalInfo = get_general_information();
    auto it = generalInfo.find(fieldName);

    // בדיקה אם השדה קיים
    if (it != generalInfo.end()) {
        // הפיכת הערך ל-lowercase
        string value = it->second;
        transform(value.begin(), value.end(), value.begin(), ::tolower);

        // החזרה אם הערך הוא "true"
        return value == "true";
    }

    // אם השדה לא נמצא
    cout << fieldName << " not found!" << endl;
    return false;
}



// פונקציות גישה
const string& EmergencyEvent::getFormatedDateTime() const {
    return this->formatDateTime; 
}

const bool EmergencyEvent:: getActive() const {return this->active;}
const bool EmergencyEvent:: getForcesArrival() const {return this->forcesArrival;}


