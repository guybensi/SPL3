package bgu.spl.net.impl.stomp.Frames;

import bgu.spl.net.srv.Connections;
import java.util.concurrent.ConcurrentHashMap;

public class ServerFrameSender {

////////ConnectedFrame//////
public static void sendConnectedFrame(String stompVersion, int connectionId, Connections<String> connections) {
    //create headers
    ConcurrentHashMap<String, String> headers = new ConcurrentHashMap<>();
    headers.put("version", stompVersion);
    //create frame
    ConnectedFrame connectedFrameToSend = new ConnectedFrame(connectionId ,headers ,"",connections);
    //send frame
    connections.send(connectionId, connectedFrameToSend.toString());
}

///////////MessageFrame//////
public static void sendMessageFrame(int connectionId,String messageId,String channel,String subscriptionId, String body,Connections<String> connections) {
    //create headers
    ConcurrentHashMap<String, String> headers = new ConcurrentHashMap<>();
    headers.put("subscription", subscriptionId);
    headers.put("message-id", messageId);
    headers.put("destination", channel);
    //create frame
    MessageFrame messageFrameToSend = new MessageFrame(connectionId, headers, body ,connections);
    //send frame
    connections.send(channel, messageFrameToSend.toString());
 }

///////////ReceiptFrame//////
public static void sendReceiptFrame(int connectionId, String receiptId, Connections<String> connections) {
    //create headers
    ConcurrentHashMap<String, String> headers = new ConcurrentHashMap<>();
    headers.put("receipt-id", receiptId);
    //create frame
    MessageFrame receiptFrameToSend = new MessageFrame(connectionId, headers, "" ,connections);
    //send frame
    connections.send(connectionId, receiptFrameToSend.toString());
 }
///////////ErrorFrame////////
public static void sendErrorFrame(int connectionId, Frame frameCausedErr,String receiptId, String errHeader, String errExplain, Connections<String> connections) {
    //create headers
    ConcurrentHashMap<String, String> headers = new ConcurrentHashMap<>();
    if (receiptId != null) {
        headers.put("receipt-id", receiptId);
     }
     headers.put("message", errHeader);
    //create body
    String body = createErrBody(frameCausedErr,errExplain);
    //create frame
    ErrorFrame errorFrameToSend = new ErrorFrame(connectionId, headers, body ,connections);
    //send frame
    connections.send(connectionId, errorFrameToSend.toString());
 }

 private static String createErrBody(Frame frameCausedErr, String errExplain) {
    String errBody = "";
    errBody = errBody + "The message that caused the error: \\n";
    errBody = errBody + "----- \\n";
    errBody = errBody + frameCausedErr.toString();
    errBody = errBody + "----- \\n";
    errBody = errBody + "The error: \\n";
    errBody = errBody + errExplain;
    return errBody;
 }
}
