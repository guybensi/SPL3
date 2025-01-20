package bgu.spl.net.impl.stomp.Frames;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.Connections;

public abstract  class Frame {
    protected final int connectionId;
    protected final ConcurrentHashMap<String, String> headers; // Key-value pairs for metadata
    protected final String body; // The message content
    protected final Connections<String> connections;
    

    // Constructor
    public Frame(int connectionId, Map <String, String> headers, String body, Connections<String> connections) {
        this.connectionId = connectionId;
        this.headers = new ConcurrentHashMap(headers);
        this.body = body;
        this.connections = connections ;
    }

    @Override
    public String toString() {
        return null;
         ////להשלים
    }
    public abstract String getNameCommand();

    public abstract void process();
}
