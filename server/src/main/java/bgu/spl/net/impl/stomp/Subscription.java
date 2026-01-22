package bgu.spl.net.impl.stomp;

import java.util.Objects;

public class Subscription {
    private final String topic;
    private final int connectionId;
    private final String subscriptionId;

    public Subscription(String topic, int connectionId, String subscriptionId){
        this.topic = topic;
        this.connectionId = connectionId;
        this.subscriptionId = subscriptionId;
    }

    public String getTopic(){
        return topic;
    }
    public int getConnectionId(){
        return connectionId;
    }
    public String getSubscriptionId(){
        return subscriptionId;
    }
    
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Subscription that = (Subscription) o;
        return connectionId == that.connectionId &&
                Objects.equals(topic, that.topic) &&
                Objects.equals(subscriptionId, that.subscriptionId);
    }

    @Override
    public int hashCode() {
        return Objects.hash(topic, connectionId, subscriptionId);
    }

}

