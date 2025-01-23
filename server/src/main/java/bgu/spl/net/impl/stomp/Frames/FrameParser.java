package bgu.spl.net.impl.stomp.Frames;

//import java.lang.reflect.Constructor;
import java.util.Arrays;
//import java.util.HashMap;
import java.util.LinkedList;
//import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.Connections;

public class FrameParser {

    public static Frame Parse(String msg, Connections<String> connections, int connectionId) {
        Queue<String> msgLines = new LinkedList<>(Arrays.asList(msg.split("\\n")));
        //get the command tayp
        String frameCommandType = (String)msgLines.remove();
        //Creating headers
        ConcurrentHashMap<String, String> headers = creatHeaders(msgLines);
        //Creating body
        if (!msgLines.isEmpty() && ((String)msgLines.peek()).equals("")) {
            msgLines.remove();
        }
        String body = creatBody(msgLines);
        //Creating frame by command type
        Frame frame = creatFrame( connectionId, frameCommandType, headers, body, connections);
        return frame;
   }

    private static ConcurrentHashMap<String, String> creatHeaders(Queue<String> msgLines) {
        ConcurrentHashMap<String,String> headers = new ConcurrentHashMap<>();
        while(!msgLines.isEmpty() && !((String)msgLines.peek()).equals("")) {
            String[] lineKeyVal = ((String)msgLines.remove()).split(":");
            headers.put(lineKeyVal[0], lineKeyVal[1]);
        }
        return headers;
    }

   private static String creatBody(Queue<String> msgLinesQueue) {
        StringBuilder body = new StringBuilder();
        while (!msgLinesQueue.isEmpty() && !msgLinesQueue.peek().equals("\u0000")) {
        body.append(msgLinesQueue.remove()).append("\n");
        }
        return body.toString();
    }

    private static Frame creatFrame(int connectionId, String frameCommandType, ConcurrentHashMap<String, String> headers, String body, Connections<String> connections) {
        switch (frameCommandType) {
            case "CONNECT":
                return new ConnectFrame(connectionId, headers, body, connections);
            case "CONNECTED":
                return new ConnectedFrame(connectionId, headers, body, connections);
            case "DISCONNECT":
                return new DisconnectFrame(connectionId, headers, body, connections);
            case "MESSAGE":
                return new MessageFrame(connectionId, headers, body, connections);
            case "RECEIPT":
                return new ReceiptFrame(connectionId, headers, body, connections);
            case "SEND":
                return new SendFrame(connectionId, headers, body, connections);
            case "SUBSCRIBE":
                return new SubscribeFrame(connectionId, headers, body, connections);
            case "UNSUBSCRIBE":
                return new UnsubscribeFrame(connectionId, headers, body, connections);
            case "ERROR":
                return new ErrorFrame(connectionId, headers, body, connections);
            default:
                return null;
        }
    }
    
/* 
    private static Frame creatFrame(int connectionId ,String frameCommandType, ConcurrentHashMap<String, String> headers, String body, Connections<String> connections) {
        Map<String, Class<? extends Frame>> commandToFrameMap = new HashMap<>();
        commandToFrameMap.put("CONNECT", ConnectFrame.class);
        commandToFrameMap.put("CONNECTED", ConnectedFrame.class);
        commandToFrameMap.put("DISCONNECT", DisconnectFrame.class);
        commandToFrameMap.put("MESSAGE", MessageFrame.class);
        commandToFrameMap.put("RECEIPT", ReceiptFrame.class);
        commandToFrameMap.put("SEND", SendFrame.class);
        commandToFrameMap.put("SUBSCRIBE", SubscribeFrame.class);
        commandToFrameMap.put("UNSUBSCRIBE", UnsubscribeFrame.class);
        commandToFrameMap.put("ERROR", ErrorFrame.class);

        Class<? extends Frame> frameClass = commandToFrameMap.get(frameCommandType);
        if (frameClass == null) {
            return null; 
        }
        try {
            Constructor<? extends Frame> constructor = frameClass.getConstructor(int.class,ConcurrentHashMap.class, String.class, Connections.class);
            return constructor.newInstance(connectionId,headers,body ,connections);
        } catch (Exception err) {
            err.printStackTrace();
            return null;
        }
    }*/
}
