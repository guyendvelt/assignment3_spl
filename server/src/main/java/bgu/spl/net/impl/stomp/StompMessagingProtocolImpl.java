package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

import javax.swing.DefaultBoundedRangeModel;

import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.data.Database;
import bgu.spl.net.impl.data.LoginStatus;
import bgu.spl.net.srv.Connections;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String>{
    int connectionId;
    boolean shouldTerminate = false;
    Connections<String> connections;
    boolean isLoggedIn = false;
    String username;

    @Override
    public void start(int connectionId, Connections<String> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
    }

    @Override
    public void process(String message) {

            Frame request = Frame.parse(message);
            String command = request.getCommand();
                 switch (command) {
                    case "SEND":
                        return handleSend(request);
                        break;
                        
                    case "CONNECT":
                        handleConnect(request);
                        break;
                    case "SUBSCRIBE":
                        handleSubscribe(request);
                        break;
                    case "UNSUBSCRIBE":
                        handleUnSubscribe(request);
                        break;
                    case "DISCONNECT":
                        handleDisconnect(request);
                        break;
                    default:
                    sendError("Illegal command", "command " + command + "is not supported in STOMP");
                }
                    
               
        }
            }
         

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private void handleSend(Frame request){
        String topic = request.getHeader("destination");
        if(topic != null){
            SubscriptionManager.getInstance().broadcast(request, connections);
            handleReceipt(request);  
        } else {
            sendError("TOPIC ERROR","topic should not be null");
        }

        
    }

private void handleConnect(Map<String, String> headers) {
    String login = headers.get("login");
    String passcode = headers.get("passcode");
    String acceptVersion = headers.get("accept-version");
    String host = headers.get("host");

    if (acceptVersion == null || host == null || login == null || passcode == null) {
        sendError("MALFORMED_FRAME", "Missing mandatory headers for CONNECT", headers);
       
    } else {
            LoginStatus status = Database.getInstance().login(connectionId, login, passcode);
           
            switch (status) {
                case WRONG_PASSWORD:
                    sendError("Login failed", "wrong password", headers);
                    break;
                case ALREADY_LOGGED_IN:
                    sendError("Login failed", "user already logged in", headers);
                    break;
                case CLIENT_ALREADY_CONNECTED:
                    sendError("Login failed", "client already connected", headers);
                    break;
                default:
                    isLoggedIn = true;
                    username = login;
                     String response = "CONNECTED\n" +
                          "version:1.2\n" +
                          "\n" + 
                          "\u0000";
                    connections.send(connectionId, response);
                    break;
            }

    }

}

    private void handleSubscribe(Map<String, String> headers){
        String topic = headers.get("destination");
        String subId = headers.get("subscription");
        if(topic != null && subId != null){
            SubscriptionManager.getInstance().subscribe(topic, connectionId, subId);
            handleReceipt(headers);
        } else {
            sendError("SUBSCRIBE ERROR", "topic and subId should not be null", headers);
        }
        
    }

    private void handleUnSubscribe(Map<String, String> headers){
        String subId = headers.get("subscription");
        if(subId != null) {
            SubscriptionManager.getInstance().unsubscribe(headers.get("subscription"), connectionId);
            handleReceipt(headers);
        } else {
            sendError("UNSUBSCRIBE ERROR", "subID should not be null", headers);
        }
        
    }

    private void handleDisconnect(Map<String, String> headers){
        handleReceipt(headers);
        SubscriptionManager.getInstance().clearConnection(connectionId);
        shouldTerminate = true;
        connections.disconnect(connectionId);
    }


    private void sendError(String message, String description, Frame request){
        String response = "ERROR\n";
        Map<String,String> headers = request.getHeaders();
        if (headers != null && headers.containsKey("receipt")) {
        response += "receipt-id:" + headers.get("receipt") + "\n";
    }
        response +=   "message: " + message + "\n\n" +
                      "The message:\n" +
                      "-----\n" +
                      description + "\n" +
                      "-----\n" +
                      "\u0000"; 
        connections.send(connectionId, response);
        SubscriptionManager.getInstance().clearConnection(connectionId);
        shouldTerminate = true;
        connections.disconnect(connectionId);
    }

    private void handleReceipt(Frame request) {
        String receiptId = request.getHeader("receipt");
        if (receiptId != null) {
            String response = "RECEIPT\n" +
                                "receipt-id:" + receiptId + "\n\n" +
                                "\u0000";
                                
            connections.send(connectionId, response);
        }
}

}


