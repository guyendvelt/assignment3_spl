#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/event.h"
#include <string>
#include <vector>
#include <map>

using std::string;
using std::map;
using std::vector;

class StompProtocol
{
private:
    string username;
    int subscriptionIdCounter;
    int receiptIdCounter;
    map<string,int> activeSubscriptions;
    map<int, string> receiptActions;
    map<string, map<string, vector<Event>>> gameEvents;
    bool shouldTerminate;

public:
    StompProtocol();
    string processInput(string command);
    bool processServerResponse(string serverResponse);
    void setUsername(string user) {
        username = user;
    }
    string getUsername(){
        return username;
    }
private:
    string handleJoin(string gameName);
    string handleExit(string gameName);
    string handleReport(string filePath);
    string handleLogout();
    void handleSummary(string gameName, string user, string filePath);
    void parseFrame(string& frame, map<string, string> headers, string& body);
    };
