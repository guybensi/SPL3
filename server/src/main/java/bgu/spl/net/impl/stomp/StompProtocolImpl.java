package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;
import java.util.Map;
import java.util.HashMap;

public class StompProtocolImpl<T> implements StompMessagingProtocol<T> {

    private int connectionId;
    private Connections<T> connections =  new ConnectionsImpl<>(); ////////////////cheak!!!
    private boolean shouldTerminate = false;

    @Override
    public void start(int connectionId, Connections<T> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
    }

    @Override
    public void process(T message) {
        String frame = (String) message;
        String[] lines = frame.split("\n");
        String command = lines[0];

    }
    
    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }
}
