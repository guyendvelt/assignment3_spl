#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/event.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>

using std::string;
using std::map;
using std::vector;


class StompProtocol
{
private:
    string username;
    int subscriptionIdCounter;
    int receiptIdCounter;
    // channel -> subscriptions id
    map<string,int> activeSubscriptions;
    map<int, string> receiptActions;
    //game -> <user -> events>
    map<string, map<string, vector<Event>>> gameEvents;
    bool isLoggedIn;
    std::mutex mtx; 



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
    string getUserName(const string& body);
    std::string formatMap(const std::map<std::string, std::string>& updates);
    vector<Event> eventsSorting(vector<Event> events);
    };
