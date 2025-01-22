package bgu.spl.net.impl.stomp.Frames;

import bgu.spl.net.srv.ConnectionHandler;
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
            //Send Error Frame
            String[] headerAndBodyErr = errowMessage.getMessage().split(":", 2);
            ServerFrameSender.sendErrorFrame(connectionId, this, (String)headers.get("receipt"), headerAndBodyErr[0], headerAndBodyErr[1],connections);
            //Hendle Error
            ConnectionHandler<String> handler = connections.getCHbyconnectionId(connectionId);
            connections.disconnect(connectionId);
            try {
                handler.close();
            } catch (IOException errException) {
                errException.printStackTrace();
            }
        }
        if (toSend == true) { 
            String destination = (String)this.headers.get("destination");
            String msgID = String.valueOf(connections.getAndIncMsgIdCounter());
            String subscriptionId = findSubscriptionId(destination);
            ServerFrameSender.sendMessageFrame(connectionId, msgID, destination, subscriptionId, body, connections);
            
            if (headers.containsKey("receipt")){
                String receiptId = (String)headers.get("receipt");
                ServerFrameSender.sendReceiptFrame(connectionId, receiptId,connections);
            }
        }
    }

    public String getNameCommand() {
        return "SEND";
     }

    ////// processפונקציות עזר ל
    private void checkDestination() throws IOException {
        if (!this.headers.containsKey("destination")) {
            throw new IOException("Missing destination header:SEND should contain destination header");
        } else if (!this.connections.isDestinationLegal(((String)this.headers.get("destination")).substring(1), this.connectionId)) {
            throw new IOException("Illegal destination:Channel doesn't exist or you're not subsribe to it");
         }
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
