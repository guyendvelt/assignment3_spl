#include <stdlib.h>
#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"
#include <thread>
#include <iostream>
#include <vector>
#include <sstream>
#include "../include/Frame.h"

using namespace std;
	ConnectionHandler* connectionHandler = nullptr;
	StompProtocol* protocol = nullptr;;
	thread* listenerThread = nullptr;
	bool isLoggedIn = false;

void socketListener(){
	while(isLoggedIn){
		string answer;
		if(!connectionHandler->getFrameAscii(answer, '\0')){
			cout << "[DEBUG] Disconnected: ConnectionHandler returned false (Socket closed?)" << endl;
			cout << "Disconnected. Exiting...\n" << endl;
			isLoggedIn = false;
		} else {
			bool shouldKeepRunning = protocol->processServerResponse(answer);
			if(!shouldKeepRunning){
				cout << "[DEBUG] Disconnected: Protocol decided to stop (Logout/Error)" << endl;
				isLoggedIn = false;
				connectionHandler->close();
			}
		}

	}
}

int main(int argc, char *argv[]) {
	// TODO: implement the STOMP client
	while(true){
		const short bufsize = 1024;
		char buf[bufsize];
		cin.getline(buf, bufsize);
		if (cin.fail() || cin.eof()) {
            break; 
        }
		string line(buf);
		stringstream lineStream(line);
		string command;
		vector<string> args;
		while( lineStream >> command){
			args.push_back(command);
		}
		if(args.empty()){
			continue;
		}
		command = args[0];
		if(command == "login"){
			if(connectionHandler != nullptr){
				cout << "User is already logged in" << endl;
				continue;
			}
			if(args.size() < 4){
				cout << "wrong login usage, please enter: login {host:port} {username} {password}" << endl;
			}
			string hostPort = args[1];
			string username = args[2];
			string password = args[3];
			//parse host and port
			size_t colon = hostPort.find(":");
			if(colon == string::npos){
				cout << "invalid host:port format" << endl;
				continue;
			}
			string host = hostPort.substr(0,colon);
			short port = stoi(hostPort.substr(colon+1));
			connectionHandler = new ConnectionHandler(host,port);
			if(!connectionHandler->connect()){
				cout << "could not connect to server" << host << ":" << port << endl;
				delete connectionHandler;
				connectionHandler = nullptr;
				continue;
			}
			protocol = new StompProtocol();
			protocol->setUsername(username);
			Frame connectFrame("CONNECT");
			connectFrame.addHeader("accept-version", "1.2");
			connectFrame.addHeader("host", "stomp.cs.bgu.ac.il");
			connectFrame.addHeader("login", username);
			connectFrame.addHeader("passcode", password);
			if (!connectionHandler->sendFrameAscii(connectFrame.toString(), '\0')){ 
                cerr << "Failed to send CONNECT frame" << endl;
				delete connectionHandler;
				delete protocol;
				connectionHandler = nullptr;
				protocol = nullptr;
				continue;
            }
			isLoggedIn = true;
			listenerThread = new thread(socketListener);
		}
		else {
			if(!isLoggedIn){
				cout << "please login first" << endl;
				continue;
			}
			string frame = protocol->processInput(line);
			if(!frame.empty()){
				if(!connectionHandler->sendFrameAscii(frame, '\0')){
					cout << "Error sending frame to server." << endl;
					isLoggedIn = false;
				}
			}

		}
		
	}
	if(!isLoggedIn && listenerThread != nullptr){
			if(listenerThread->joinable()){
				listenerThread->join();
			}
			delete listenerThread;
			listenerThread = nullptr;
		}
		if(connectionHandler!=nullptr){
			delete connectionHandler;
			connectionHandler = nullptr;
		}
		if(protocol != nullptr){
			delete protocol;
			protocol = nullptr;
		}
	return 0;
}