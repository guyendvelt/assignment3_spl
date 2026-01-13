package bgu.spl.net.srv;

import java.sql.Connection;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

public class ConnectionImpl<T> implements Connections<T> {
    private final ConcurrentHashMap<Integer, ConnectionHandler<T>> connectionsMap;
    private final AtomicInteger idCounter;
    private static final ConnectionImpl<?> INSTANCE = new ConnectionImpl<>();


    private ConnectionImpl(){
        connectionsMap = new ConcurrentHashMap<>();
        idCounter = new AtomicInteger(0);
    }

    public static <T> ConnectionImpl<T> getInstance(){
        return (ConnectionImpl<T>) INSTANCE;
    }

    public void connect(int connectionId, ConnectionHandler<T> handler) {
        connectionsMap.put(connectionId, handler);
    }

    @Override
    public boolean send(int connectionId, T msg) {
       ConnectionHandler<T> handler = connectionsMap.get(connectionId);
       if(handler != null){
            handler.send(msg);
            return true;
       } else {
        return false;
       }
    }

    @Override
    public void send(String channel, T msg) {
       //we do not implement this function because it will never be used
    }

    @Override
    public void disconnect(int connectionId) {
        connectionsMap.remove(connectionId);
    }

    public int getUniqueId() {
        return idCounter.getAndIncrement();
    }


}
