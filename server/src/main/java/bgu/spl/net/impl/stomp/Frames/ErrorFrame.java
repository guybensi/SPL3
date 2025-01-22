package bgu.spl.net.impl.stomp.Frames;

import bgu.spl.net.srv.Connections;
import java.util.concurrent.ConcurrentHashMap;

public class ErrorFrame extends Frame{
    ErrorFrame(int connectionId ,ConcurrentHashMap<String, String> headers,String body, Connections<String> connections) {
        super(connectionId, headers, body, connections);
     }

    public void process() {
    }

    public String getNameCommand() {
        return "ERROR";
     }
}
