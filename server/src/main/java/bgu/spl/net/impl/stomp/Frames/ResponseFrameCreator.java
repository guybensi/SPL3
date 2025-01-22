package bgu.spl.net.impl.stomp.Frames;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;
import java.io.IOException;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class ResponseFrameCreator {
   public static final String STOMP_VERSION = "1.2";
   public static final Object HOST = "stomp.cs.bgu.ac.il";


////////ConnectedFrame//////
public static void sendConnectedFrame(String stompVersion, int connectionId, Connections<String> connections) {
    ConcurrentHashMap<String, String> headers = new ConcurrentHashMap<>();
    headers.put("version", stompVersion);
    ConnectedFrame connectedFrameToSend = new ConnectedFrame(connectionId ,headers ,"",connections);
    connections.send(connectionId, connectedFrameToSend.toString());
}

///////////MessageFrame//////
/// 
///////////ReceiptFrame//////
/// 
///////////ErrorFrame////////
}
