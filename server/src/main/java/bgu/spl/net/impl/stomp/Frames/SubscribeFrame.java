package bgu.spl.net.impl.stomp.Frames;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.UserStomp;
import java.io.IOException;
import java.util.concurrent.ConcurrentHashMap;

public class SubscribeFrame extends Frame{
    SubscribeFrame(int connectionId ,ConcurrentHashMap<String, String> headers,String body, Connections<String> connections) {
        super(connectionId, headers, body, connections);
     }

    public void process() {
        boolean toSubscribe = true;
        try{
            checkHeaders();
            checkIfAlreadySub();
        } catch(IOException errowMessage){
            toSubscribe = false;
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
        if (toSubscribe== true) {
            Integer id  = Integer.parseInt((String)headers.get("id")); 
            String destination = (String)headers.get("destination");
            connections.subscribe(connectionId, id ,destination);

            if (headers.containsKey("receipt")){
                String receiptId = (String)headers.get("receipt");
                ServerFrameSender.sendReceiptFrame(connectionId, receiptId,connections);
             }
        }
    }

    public String getNameCommand() {
        return "SUBSCRIBE";
     }

     ////// processפונקציות עזר ל
     private void checkHeaders() throws IOException {
        if (!headers.containsKey("id") || !headers.containsKey("destination")) {
           throw new IOException("Missing id and destination headers:SUBSCRIBE should contain id and destination headers");
        }
     }
  
     private void checkIfAlreadySub() throws IOException {
        UserStomp<?> user = connections.getCHbyconnectionId(connectionId).getUser();
        Integer id = Integer.parseInt((String)headers.get("id"));
        Boolean isAlreadySub = user.getChannelSubscriptions().containsKey(id);
        if (isAlreadySub) {
            throw new IOException("Multiple Subscription :The user already subscribed with id " + id);
        }
    }
    
}
