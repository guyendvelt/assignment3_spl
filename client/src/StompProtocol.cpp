#include "../include/StompProtocol.h"
#include "../include/event.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "../include/Frame.h"

using namespace std;

StompProtocol::StompProtocol() :
username(""),
subscriptionIdCounter(0),
receiptIdCounter(0),
activeSubscriptions(),
receiptActions(),
gameEvents(),
isLoggedIn(false){}

bool StompProtocol::processServerResponse(string response){
    Frame frame = Frame::parse(response);
    string command = frame.getCommand();
    if(command == "CONNECTED"){
        cout << "Login successful" << endl;
         isLoggedIn = true;
         return true;
    } else if(command == "MESSAGE"){
        string gameName = frame.getHeader("destination");
        string body = frame.getBody();
        Event event(body);
        string userName = getUserName(body);
        gameEvents[gameName][userName].push_back(event);
        cout << "Received message from " << frame.getHeader("destination") << ":\n" << body << endl;
        return true;
    } else if(command == "RECEIPT"){
        string receiptAction = receiptActions.at(stoi(frame.getHeader("receipt-id")));
        cout << receiptAction << endl;
        return true;
    } else if(command == "ERROR"){
        string msg = frame.getHeader("message");
        string body = frame.getBody();
        cout << "ERROR message: " << msg << "\n" << "ERROR description: " << body << endl;
        isLoggedIn = false;
        return false;
    } else {
        cout << "Warning: Received unknown command: " << command << endl;
        return true;
    }
    

}

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
       return "";

    }else if(command == "logout"){
        return handleLogout();

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
    Frame response("SUBSCRIBE");
    response.addHeader("destination", gameName);
    response.addHeader("id", to_string(subscriptionIdCounter));
    response.addHeader("receipt", to_string(receiptIdCounter));
    return response.toString();
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
Frame response("UNSUBSCRIBE");
response.addHeader("id", to_string(subscriptionId));
response.addHeader("receipt", to_string(receiptIdCounter));
return response.toString();
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
        for (const auto& pair : event.get_game_updates()){
            body += pair.first + ":" + pair.second + "\n";
        }

        body += "team a updates:\n";
        for (const auto& pair : event.get_team_a_updates()) {
            body += pair.first + ":" + pair.second + "\n";
        }
        
        body += "team b updates:\n";
        for (const auto& pair : event.get_team_b_updates()) {
            body += pair.first + ":" + pair.second + "\n";
        }

        body += "description:\n" + event.get_discription() + "\n";
        Frame response("SEND");
        response.addHeader("destination", gameName);
        response.setBody(body);

        frames += response.toString();
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
    for(const auto& event : events){
        for (const auto& pair : event.get_game_updates()) {
            generalStats[pair.first] = pair.second;
        }
        for (const auto& pair : event.get_team_a_updates()) {
            team_a_stats[pair.first] = pair.second;
        }
        for (const auto& pair : event.get_team_b_updates()) {
            team_b_stats[pair.first] = pair.second;
        }
    }

    outFile << team_a_name << " vs " << team_b_name << "\n";
    outFile << "Game stats:\n";
    outFile << "General stats:\n";
    for(const auto& pair : generalStats){
        outFile << pair.first << ": " << pair.second + "\n";
    }
    outFile << team_a_name << " stats:\n";
    for(const auto& pair : team_a_stats){
        outFile << pair.first << ": " << pair.second + "\n";
    }
    outFile << team_b_name << " stats:\n";
    for(const auto& pair : team_b_stats){
        outFile << pair.first << ": " << pair.second + "\n";
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
        Frame response("DISCONNECT");
        response.addHeader("receipt", to_string(receiptIdCounter));
        return response.toString();
    }

    string StompProtocol::getUserName(const string& body){
        stringstream bodyStream(body);
        string line;
        if(getline(bodyStream, line)){
            if(line.find("user:") == 0){
                return line.substr(6);
            }
        }
        return "";
    }




