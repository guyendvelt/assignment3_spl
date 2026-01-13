#include <stdlib.h>
#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"
#include <thread>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

StompProtocol::StompProtocol() :
username(""),
subscriptionIdCounter(0),
receiptIdCounter(0),
activeSubscriptions(),
receiptActions(),
gameEvents(),
shouldTerminate(false){}

string StompProtocol

void socketListener(ConnectionHandler* handler, StompProtocol* protocol, bool* shouldTerminate){
	while(!(*shouldTerminate)){
		string answer;
		if(!handler->getFrameAscii(answer, '\0')){
			cout << "Disconnected. Exiting...\n" << endl;
			*shouldTerminate = true;
		} else {
			bool keepRunning = protocol->processServerResponse(answer);
			if(!keepRunning){
				*shouldTerminate = true;
				handler->close();
			}
		}

	}
}

int main(int argc, char *argv[]) {
	// TODO: implement the STOMP client
	while(true){
		cout << "Please login (login {host:port} {username} {password})" << endl;
		string line;
        getline(cin, line);

		stringstream stringStream(line);
        string command;
        stringStream >> command;

		if(command == "login"){
			string hostPort, username, password;
			stringStream >> hostPort >> username >> password;

			string host = hostPort.substr(0, hostPort.find(':'));
			short port = stoi(hostPort.substr(hostPort.find(':') + 1));
			ConnectionHandler handler(host, port);
			if(!handler.connect()){
				cerr << "Cannot connect to " << host << ":" << port << endl;
				continue;
			}

			string connectFrame = "CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:" + username + "\npasscode:" + password + "\n\n\0";
			if (!handler.sendFrameAscii(connectFrame.substr(0, connectFrame.length()-1), '\0')) { 
                cerr << "Failed to send CONNECT frame" << endl;
                continue;
            }
			string answer;
			if(!handler.getFrameAscii(answer, '\0')){
				cerr << "Could not hear back from server" << endl;
                 continue;
			}
			if(answer.find("CONNECTED") != string::npos){
				cout << "Login succesful" << endl;
			}

			StompProtocol protocol;
			protocol.setUsername(username);
			bool shouldTerminate = false;
			thread listenerThread(socketListener, &handler, &protocol, &shouldTerminate);
			while(!shouldTerminate){
				string input;
				getline(cin, input);
				if(shouldTerminate){
					break;
				}
				string frameToSend = protocol.processInput(input);
				if(!frameToSend.empty()){
					if(!handler.sendFrameAscii(frameToSend, '\0')){
						cout << "Error sending frame. Disconnecting..." << endl;
                            shouldTerminate = true;
					}
				}
			}

			if(listenerThread.joinable()){
				listenerThread.join();
				return 0;
			} else {
				cout << "Login failed. Response: " << answer << endl;
			}
		


		}
	}
	return 0;
}