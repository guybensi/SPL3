package bgu.spl.net.impl.stomp.Frames;

import bgu.spl.net.srv.ConnectionHandler;
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
        if (toDisconnect == true) {
            String receiptId = (String)headers.get("receipt");
            ServerFrameSender.sendReceiptFrame(connectionId, receiptId,connections);
            connections.disconnect(this.connectionId);
        }
    }

    public String getNameCommand() {
        return "DISCONNECT";
     }

    ////// processפונקציות עזר ל
    private void checkReceipt() throws IOException {
        if (!this.headers.containsKey("receipt")) {
            throw new IOException("DISCONNECT should contain receipt header");
        }
    }
    
}
