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
    return stringFrame.str();
}

Frame Frame::parse(const string& msg){
    stringstream msgStream(msg);
    string line, command;
    if(!getline(msgStream, command)){
        return Frame("ERROR");
    }
    if(!command.empty() && command.back() == '\r'){
        command.pop_back();
    }

    //create new frame
    Frame frame(command);
    //parse headers
    while(getline(msgStream,line) && !line.empty()){
        if(!line.empty() && line.back() == '\r'){
            line.pop_back();
        }
        size_t colon = line.find(":");
        if(colon != string::npos){
            string key = line.substr(0, colon);
            string val = line.substr(colon+1);
            frame.addHeader(key,val);
        }
    }
     // parse body
     string body;
     char c;
    while(msgStream.get(c)){
        body += c;
    }
     frame.setBody(body);
     return frame;
}
    
