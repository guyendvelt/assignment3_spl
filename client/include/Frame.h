#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

class Frame{
private:
    string command;
    map<string,string> headers;
    string body;

public:
    Frame(string command);
    Frame();
    
    string getCommand();
    void addHeader(const string& key, const string& val);
    string getHeader(const string& key);
    map<string,string> getHeaders();
    string getBody();
    void setBody(const string& body);
    string toString();
    static Frame parse(const string& msg);
    











}