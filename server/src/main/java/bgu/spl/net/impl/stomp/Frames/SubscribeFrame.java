package bgu.spl.net.impl.stomp.Frames;
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
            //String[] SummaryAndBodyErr = var4.getMessage().split(":", 2);
            //FrameUtil.handleError(this, SummaryAndBodyErr[0], SummaryAndBodyErr[1], this.connections, this.connectionId, (String)this.headers.get("receipt"));
        }
        if (toSubscribe== true) {
            Integer id  = Integer.parseInt((String)this.headers.get("id")); 
            String destination = (String)this.headers.get("destination");
            connections.subscribe(this.connectionId, id ,destination);
            //if (this.headers.containsKey("receipt")) {
              //  FrameUtil.sendReceiptFrame((String)this.headers.get("receipt"), this.connections, this.connectionId);
             //}
        }
    }

    public String getNameCommand() {
        return "SUBSCRIBE";
     }

     ////// processפונקציות עזר ל
     private void checkHeaders() throws IOException {
        if (!this.headers.containsKey("id") || !this.headers.containsKey("destination")) {
           throw new IOException("SUBSCRIBE should contain id and destination headers");
        }
     }
  
     private void checkIfAlreadySub() throws IOException {
        UserStomp<?> user = connections.getCHbyconnectionId(connectionId).getUser();
        Integer id = Integer.parseInt((String)headers.get("id"));
        Boolean isAlreadySub = user.getChannelSubscriptions().containsKey(id);
        if (isAlreadySub) {
            throw new IOException("the user already subscribed with id " + id);
        }
    }
    
}
