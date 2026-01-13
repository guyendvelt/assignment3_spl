#include "../include/StompProtocol.h"
#include "../include/event.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

StompProtocol::StompProtocol() :
username(""),
subscriptionIdCounter(0),
receiptIdCounter(0),
activeSubscriptions(),
receiptActions(),
gameEvents(),
shouldTerminate(false){}

string StompProtocol::processInput(string commandLine) {
    stringstream commandLineStream(commandLine);
    string command;
    //get command from input line
    commandLineStream >> command;
    if(command == "join"){
        string game_name;
        commandLineStream >> game_name;
        return handleJoin(game_name);
    }else if(command == "exit"){
        string game_name;
        commandLineStream >> game_name;
        return handleExit(game_name);

    }else if(command == "report"){
        string filePath;
        commandLineStream >> filePath;
        return handleReport(filePath);

    }else if(command == "summary"){
        string gameName, username, filePath;
        commandLineStream >> gameName >> username >> filePath;
        handleSummary(gameName, username, filePath);
        return "";


    }else if(command == "login"){
       cout << "client already logged in" << endl;

    }else if(command == "logout"){
        handleLogout();

    }else {
        cout << "Illegal command" << endl;
        return "";
    }
}

string StompProtocol::handleJoin(string gameName){
    subscriptionIdCounter++;
    receiptIdCounter++;
    activeSubscriptions[gameName] = subscriptionIdCounter;
    receiptActions[receiptIdCounter] = "Joined channel " + gameName;
    string response = "SUBSCRIBE\n"
                      "destination: " + gameName + "\n" +
                      "id: " + to_string(subscriptionIdCounter) + "\n" +
                      "receipt: " + to_string(receiptIdCounter) + "\n\n";
    return response;
}

string StompProtocol::handleExit(string gameName){
if (activeSubscriptions.count(gameName) == 0){
    cout << "You are not subscribed to " << gameName << endl;
    return "";
}
receiptIdCounter++;
int subscriptionId = activeSubscriptions[gameName];
activeSubscriptions.erase(gameName);
receiptActions[receiptIdCounter] = "Exited channel " + gameName;
string response = "UNSUBSCRIBE\n"
                  "id: " + to_string(subscriptionId) + "\n" +
                  "receipt: " + to_string(receiptIdCounter) + "\n\n";
return response;
}

string StompProtocol::handleReport(string filePath){
    names_and_events data;
    try {
        data = parseEventsFile(filePath);
    } catch(exception& e) {
        cout << "Error parsing file" << endl;
        return "";
    }
    string frames = "";
    string gameName = data.team_a_name + "_" + data.team_b_name;

    if (activeSubscriptions.count(gameName) == 0){
        cout << "Error: You must join the channel " << gameName << " before reporting." << endl;
        return "";
    }

    for (const Event& event : data.events){
        string body = "user: " + username + "\n" +
                      "team a: " + data.team_a_name + "\n" +
                      "team b: " + data.team_b_name + "\n" +
                      "event name: " + event.get_name() + "\n" +
                      "time: " + to_string(event.get_time()) + "\n" +
                      "general game updates:\n";
        for (auto const& [key, val] : event.get_game_updates()){
            body += key + ":" + val + "\n";
        }

        body += "team a updates:\n";
        for (auto const& [key, val] : event.get_team_a_updates()) {
            body += key + ":" + val + "\n";
        }
        
        body += "team b updates:\n";
        for (auto const& [key, val] : event.get_team_b_updates()) {
            body += key + ":" + val + "\n";
        }

        body += "description:\n" + event.get_discription() + "\n";

        string frame = "SEND\n"
                       "destination:/" + gameName + "\n"
                       "\n" + 
                       body + "\0";
        
        frames += frame;
        gameEvents[gameName][username].push_back(event);

    }
        return frames;
    
}

void StompProtocol::handleSummary(string gameName, string userName, string filePath){
    if(gameEvents.count(gameName) == 0 || gameEvents[gameName].count(userName) == 0){
        cout << "No events found for user " << userName << " in game " << gameName << endl;
        return;
    }
    std::ofstream outFile(filePath);
    if(!outFile){
        cout << "Error creating file: " << filePath << endl;
        return;
    }
    
    const vector<Event>& events = gameEvents[gameName][userName];
    string team_a_name = events[0].get_team_a_name();
    string team_b_name = events[0].get_team_b_name();
    map<string,string> generalStats;
    map<string,string> team_a_stats;
    map<string,string> team_b_stats;
    //update the statistics
    for(auto& const event : events){
        for (auto const& [key, val] : event.get_game_updates()) {
            generalStats[key] = val;
        }
        for (auto const& [key, val] : event.get_team_a_updates()) {
            team_a_stats[key] = val;
        }
        for (auto const& [key, val] : event.get_team_b_updates()) {
            team_b_stats[key] = val;
        }
    }

    outFile << team_a_name << " vs " << team_b_name << "\n";
    outFile << "Game stats:\n";
    outFile << "General stats:\n";
    for(auto& const [key, val] : generalStats){
        outFile << key << ": " << val + "\n";
    }
    outFile << team_a_name << " stats:\n";
    for(auto& const [key, val] : team_a_stats){
        outFile << key << ": " << val + "\n";
    }
    outFile << team_b_name << " stats:\n";
    for(auto& const [key, val] : generalStats){
        outFile << key << ": " << val + "\n";
    }
    
    outFile << "Game event reports:\n";
    for(const Event& event : events){
        outFile << event.get_time() << " - " << event.get_name() << ":\n\n";
        outFile  << event.get_discription() << "\n\n";
    }

    outFile.close();
    cout << "Summary file created at " << filePath << endl;

}

    string StompProtocol::handleLogout(){
        receiptIdCounter++;
        receiptActions[receiptIdCounter] = "Disconnecting...";
        string response = "DISCONNECT\n"
                          "receipt:" +to_string(receiptIdCounter) + "\n\n";
        return response;
    }






