package bgu.spl.net.impl.stomp.Frames;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;
import java.io.IOException;
import java.util.concurrent.ConcurrentHashMap;

public class UnsubscribeFrame extends Frame{
    UnsubscribeFrame(int connectionId ,ConcurrentHashMap<String, String> headers,String body, Connections<String> connections) {
        super(connectionId, headers, body, connections);
     }

    public void process() {
        boolean toUnsubscribe = true;
        try{
            checkId();
        } catch(IOException errowMessage){
            toUnsubscribe = false;
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
        if (toUnsubscribe == true) {
            Integer id  = Integer.parseInt((String)headers.get("id")); 
            connections.unsubscribe(connectionId, id);

            if (headers.containsKey("receipt")){
                String receiptId = (String)headers.get("receipt");
                ServerFrameSender.sendReceiptFrame(connectionId, receiptId,connections);
            }
        }
    }

    public String getNameCommand() {
        return "UNSUBSCRIBE";
     }
    
      ////// processפונקציות עזר ל
      private void checkId() throws IOException {
        if (!headers.containsKey("id")) {
           throw new IOException("Missing id header:SUBSCRIBE should contain id header");
        }
     }
}
