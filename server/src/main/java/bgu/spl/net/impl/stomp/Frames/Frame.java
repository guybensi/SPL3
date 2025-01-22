package bgu.spl.net.impl.stomp.Frames;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.Connections;

public abstract  class Frame {
    protected final int connectionId;
    protected final ConcurrentHashMap <String, String> headers; // Key-value pairs for metadata
    protected final String body; // The message content
    protected final Connections<String> connections;
    
    // Constructor
    public Frame(int connectionId, ConcurrentHashMap <String, String> headers, String body, Connections<String> connections) {
        this.connectionId = connectionId;
        this.headers = headers;
        this.body = body;
        this.connections = connections ;
    }

    @Override
    public String toString() {
    String msg = "";
    msg += this.getNameCommand() + "\n";
    for (Map.Entry<String, String> entry : this.headers.entrySet()) {
        msg += entry.getKey() + ":" + entry.getValue() + "\n";
    }
    msg += "\n";
    msg += this.body + "\n";
    return msg;
}
    public abstract String getNameCommand();

    public abstract void process();
}
