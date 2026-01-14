#include "../include/Frame.h"
#include <sstream>
#include <vector>

using namespace std;

Frame::Frame(string command) : command(command), headers(), body("") {};
Frame::Frame() : command(), headers(), body("") {};

string Frame::getCommand(){
    return command;
}
void Frame::addHeader(const string& key, const string& val){
    headers[key] = val;
}

string Frame::getHeader(const string&key){
    return headers.at(key);
}
map<string,string> Frame::getHeaders(){
    return headers;
}

string Frame::getBody(){
    return body;
}

void Frame::setBody(const string& body){
    this->body = body;
}

string Frame::toString(){
    stringstream stringFrame;
    stringFrame << command << "\n";
    for(const auto& pair : headers){
        stringFrame << pair.first << ":" << pair.second << "\n";
    }
    stringFrame << "\n";
    if(!body.empty()){
        stringFrame << body;
    }
    // stringFrame << "\0";
    return stringFrame.str();
}

Frame Frame::parse(const string& msg){
    stringstream msgStream(msg);
    string line, command;
    if(!getline(msgStream, command)){
        return Frame("ERROR");
    }
    //create new frame
    Frame frame(command);
    //parse headers
    while(getline(msgStream,line) && !line.empty()){
        int colon = line.find(":");
        if(colon != string::npos){
            string key = line.substr(0, colon);
            string val = line.substr(colon+1);
            frame.addHeader(key,val);
        }
    }
     // parse body
     string body;
     getline(msgStream, body, '\0');
     frame.setBody(body);
     return frame;
}
    
