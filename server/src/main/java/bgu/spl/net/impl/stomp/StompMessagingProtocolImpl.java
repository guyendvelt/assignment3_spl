package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String>, MessagingProtocol<String> {
    int connectionId;
    boolean shouldTerminate = false;
    Connections<String> connections;

    @Override
    public void start(int connectionId, Connections<String> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
    }

    @Override
    public void process(String message) {
        String lines[] = message.split("\n");
        String command = lines[0];
        Map<String, String> headersMap = new HashMap<>(); 
        String body = "";
        int i = 0;
        while(i < lines.length && !lines[i].isEmpty()){
            String[] lineParts = lines[i].split(":");
            headersMap.put(lineParts[0], lineParts[1]);
            i++;
        }

            i++;
          
          switch (command) {
            case "SEND":
                 while(i<lines.length){
                    body = body + " " + lines[i];
                    i++;
                    handleSend(headersMap, body);
                    break;
                }
                
            case "CONNECT":
                handleConnect(headersMap);
                break;
            case "SUBSCRIBE":
                handleSubscribe(headersMap);
                break;
            case "UNSUBSCRIBE":
                handleUnSubscribe(headersMap);
                break;
            case "DISCONNECT":
                handleDisconnect(headersMap);
                break;
          }

        }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private void handleSend(Map<String, String> headers, String body){
    }

    private void handleConnect(Map<String, String> headers){

    }

    private void handleSubscribe(Map<String, String> headers){

    }

    private void handleUnSubscribe(Map<String, String> headers){

    }

    private void handleDisconnect(Map<String, String> headers){

    }


}
