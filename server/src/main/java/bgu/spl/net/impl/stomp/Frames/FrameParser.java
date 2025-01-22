package bgu.spl.net.impl.stomp.Frames;

import java.lang.reflect.Constructor;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.Connections;

public class FrameParser {

    public static Frame Parse(String msg, Connections<String> connections, int connectionId) {
        Queue<String> msgLines = new LinkedList(Arrays.asList(msg.split("\\n")));
        //get the command tayp
        String frameCommandType = (String)msgLines.remove();
        //Creating headers
        Map<String, String> headers = creatHeaders(msgLines);
        //Creating body
        if (!msgLines.isEmpty() && ((String)msgLines.peek()).equals("")) {
            msgLines.remove();
        }
        String body = creatBody(msgLines);
        //Creating frame by command type
        Frame frame = creatFrame(frameCommandType, headers, body, connections, connectionId);
        return frame;
   }

    private static Map<String, String> creatHeaders(Queue<String> msgLines) {
        ConcurrentHashMap headers = new ConcurrentHashMap();
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

    private static Frame creatFrame(String frameCommandType, Map<String, String> headers, String body, Connections<String> connections, int connectionId) {
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
            Constructor<? extends Frame> constructor = frameClass.getConstructor(String.class, Map.class, Connections.class, int.class);
            return constructor.newInstance(body, headers, connections, connectionId);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }







}
