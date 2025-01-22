#ifndef EMERGENCYEVENT_H
#define EMERGENCYEVENT_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>
#include "event.h"
using namespace std;

class EmergencyEvent : public Event {
private:
    string formatDateTime;
    bool active;
    bool forcesArrival;

public:
    EmergencyEvent(const Event& e);
    bool operator<(const EmergencyEvent& other) const;
    bool isFieldTrue(const string& fieldName) const;

    // פונקציות גישה
    const string& getFormatedDateTime() const;
    const bool getActive() const;
    const bool getForcesArrival() const;
};

// מפת סיכומים
extern map<string, map<string, vector<EmergencyEvent>>> eventSummaryMap;
extern map<string, shared_ptr<mutex>> channelMutexes;

// פונקציות נלוות
void addToSummary(const Event& e, const string& username);
void ensureChannelMutexExists(const string& channelName);
string formatToDateTime(const string& rawDateTime);

#endif // EMERGENCYEVENT_H
