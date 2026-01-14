package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

public class Frame {
    private String command;
    private Map<String,String> headers = new HashMap<String,String>();
    private String body = "";

public Frame(String command){
    this.command = command;
}
public String getCommand(){
    return this.command;
}
public void addHeader(String key, String val){
    headers.put(key,val);
}
public String getHeader(String key){
    return headers.get(key);
}

public Map<String,String> getHeaders(){
    return this.headers;
}

public String getBody(){
    return this.body;
}
public void setBody(String body){
    this.body = body;
}

public static Frame parse(String msg){
    String lines[] = msg.split("\n");
    String command = lines[0];
    Frame frame = new Frame(command);
    int i = 1;
    while(i < lines.length && !lines[i].isEmpty()){
        String[] lineParts = lines[i].split(":");
        frame.addHeader(lineParts[0], lineParts[1]);
        i++;
    }
    StringBuilder buildBody = new StringBuilder();
    i++;
    while(i<lines.length){
        buildBody.append(lines[i]);
        i++;
    }
    frame.setBody(buildBody.toString());
    return frame;
}
@Override
public String toString(){
    StringBuilder sb = new StringBuilder();
    sb.append(command).append("\n");
    if (headers != null) {
        for (Map.Entry<String, String> entry : headers.entrySet()) {
            sb.append(entry.getKey())
              .append(":")
              .append(entry.getValue())
              .append("\n");
        }
    }
    sb.append("\n");
    sb.append('\u0000');
    return sb.toString();

}


    
}
