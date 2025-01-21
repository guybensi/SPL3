package bgu.spl.net.impl.stomp.Frames;

import bgu.spl.net.srv.Connections;
import java.io.IOException;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class SendFrame extends Frame{
    SendFrame(int connectionId ,ConcurrentHashMap<String, String> headers,String body, Connections<String> connections) {
        super(connectionId, headers, body, connections);
     }

    public void process() {
        boolean toSend = true;
        try{
            checkDestination();
        } catch(IOException errowMessage){
            toSend = false;
            //String[] SummaryAndBodyErr = var4.getMessage().split(":", 2);
            //FrameUtil.handleError(this, SummaryAndBodyErr[0], SummaryAndBodyErr[1], this.connections, this.connectionId, (String)this.headers.get("receipt"));
        }
        if (toSend == true) { 
            String destination = (String)this.headers.get("destination");
            MessageFrame message = creatMessageFrame();
            connections.send(destination ,message.toString());
            //if (this.headers.containsKey("receipt")) {
                //FrameUtil.sendReceiptFrame((String)this.headers.get("receipt"), this.connections, this.connectionId);
             //}
        }
    }

    public String getNameCommand() {
        return "SEND";
     }

    ////// processפונקציות עזר ל
    private void checkDestination() throws IOException {
        if (!this.headers.containsKey("destination")) {
            throw new IOException("SEND should contain destination header");
        } else if (!this.connections.isDestinationLegal(((String)this.headers.get("destination")).substring(1), this.connectionId)) {
            throw new IOException("channel doesn't exist or you're not subsribe to it");
         }
    }
    private MessageFrame creatMessageFrame(){
        String destination = (String)this.headers.get("destination");
        String subscriptionId = findSubscriptionId(destination);
        ConcurrentHashMap<String, String> HeadersForMessageFrame = new ConcurrentHashMap<>();
        HeadersForMessageFrame.put("subscription", subscriptionId);
        HeadersForMessageFrame.put("message-id", String.valueOf(connections.getAndIncMsgIdCounter()));
        HeadersForMessageFrame.put("destination", destination);
        return new MessageFrame(connectionId ,HeadersForMessageFrame, body, connections);
    }

    private String findSubscriptionId(String destination) {
    Map<Integer, String> ChannelsOfUser = this.connections.getCHbyconnectionId(this.connectionId).getUser().getChannelSubscriptions();
    for (Map.Entry<Integer, String> entry : ChannelsOfUser.entrySet()) {
        if (destination.equals(entry.getValue())) {
            return String.valueOf(entry.getKey());
        }
    }
    return null;
}

    
}
