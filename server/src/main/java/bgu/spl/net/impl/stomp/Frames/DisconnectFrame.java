package bgu.spl.net.impl.stomp.Frames;

import bgu.spl.net.srv.Connections;
import java.io.IOException;
import java.util.concurrent.ConcurrentHashMap;

public class DisconnectFrame extends Frame{
    DisconnectFrame(int connectionId ,ConcurrentHashMap<String, String> headers,String body, Connections<String> connections) {
        super(connectionId, headers, body, connections);
     }

    public void process() {
        boolean toDisconnect = true;
        try{
            checkReceipt();
        } catch(IOException errowMessage){
            toDisconnect = false;
            //String[] SummaryAndBodyErr = var4.getMessage().split(":", 2);
            //FrameUtil.handleError(this, SummaryAndBodyErr[0], SummaryAndBodyErr[1], this.connections, this.connectionId, (String)this.headers.get("receipt"));
        }
        if (toDisconnect == true) {
            Integer id  = Integer.parseInt((String)this.headers.get("id")); 
            connections.unsubscribe(this.connectionId, id);
            //if (this.headers.containsKey("receipt")) {
                //FrameUtil.sendReceiptFrame((String)this.headers.get("receipt"), this.connections, this.connectionId);
        }

    }

    public String getNameCommand() {
        return "DISCONNECT";
     }
    
}
