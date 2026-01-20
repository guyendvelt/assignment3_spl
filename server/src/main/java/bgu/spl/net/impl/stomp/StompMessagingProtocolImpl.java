package bgu.spl.net.impl.stomp;
import java.util.Map;

import javax.xml.crypto.Data;

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
                        handleSend(request);
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
                    sendError("Illegal command", "command " + command + "is not supported in STOMP", request);
                }
                    
               
        }
            
         

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private void handleSend(Frame request){
        String topic = request.getHeader("destination");
        if(request.getHeader("filePath") != null){
            String filePath = request.getHeader("filePath");
            Database.getInstance().trackFileUpload(username, filePath, topic);
        }
        
        if(topic != null){
            boolean isRegistered = SubscriptionManager.getInstance().isRegistered(this.connectionId);
            if(!isRegistered){
                sendError("NOT SUBSCRIBED","You must be subsribed to " + topic + " to send messages", request);
            } else {
                 SubscriptionManager.getInstance().broadcast(request, connections);
                 handleReceipt(request);
            }
             
        } else {
            sendError("TOPIC ERROR","topic should not be null", request);
        }

        
    }

private void handleConnect(Frame request) {
    
    String login = request.getHeader("login");
    String passcode = request.getHeader("passcode");
    String acceptVersion = request.getHeader("accept-version");
    String host = request.getHeader("host");

    if (acceptVersion == null || host == null || login == null || passcode == null) {
        sendError("MALFORMED_FRAME", "Missing mandatory headers for CONNECT", request);
       
    } else {
            LoginStatus status = Database.getInstance().login(connectionId, login, passcode);
            switch (status) {
                case WRONG_PASSWORD:
                    sendError("Login failed", "wrong password", request);
                    break;
                case ALREADY_LOGGED_IN:
                    sendError("Login failed", "user already logged in", request);
                    break;
                case CLIENT_ALREADY_CONNECTED:
                    sendError("Login failed", "client already connected", request);
                    break;
                default:
                    isLoggedIn = true;
                    username = login;
                    Frame response = new Frame("CONNECTED");
                    response.addHeader("version", "1.2");
                    System.out.println("connecting to user " + username);
                    connections.send(connectionId, response.toString());
                    
                    break;
            }

    }

}

    private void handleSubscribe(Frame request){
        String topic = request.getHeader("destination");
        String subId = request.getHeader("id");
        System.out.println(topic + " " + subId);
        if(topic != null && subId != null){
           SubscriptionManager.getInstance().subscribe(topic, connectionId, subId);
            handleReceipt(request);
        } else {
            sendError("SUBSCRIBE ERROR", "topic and subId should not be null", request);
        }
        
    }

    private void handleUnSubscribe(Frame request){
        String subId = request.getHeader("id");
        if(subId != null) {
            SubscriptionManager.getInstance().unsubscribe(request.getHeader("id"), connectionId);
            handleReceipt(request);
        } else {
            sendError("UNSUBSCRIBE ERROR", "subID should not be null", request);
        }
        
    }

    private void handleDisconnect(Frame request){
        System.out.println("Received DISCONNECTED request from user " + username);
        handleReceipt(request);
        SubscriptionManager.getInstance().clearConnection(connectionId);
        shouldTerminate = true;
        connections.disconnect(connectionId);
        Database.getInstance().logout(connectionId);
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
        Database.getInstance().logout(connectionId);
    }

    private void handleReceipt(Frame request) {
        String receiptId = request.getHeader("receipt");
        if (receiptId != null) {
            Frame response = new Frame("RECEIPT");
            response.addHeader("receipt-id", receiptId);
            connections.send(connectionId, response.toString());
        }
}

}


