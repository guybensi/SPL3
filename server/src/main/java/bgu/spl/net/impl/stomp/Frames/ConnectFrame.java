package bgu.spl.net.impl.stomp.Frames;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

import java.io.IOException;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectFrame extends Frame {
   ConnectFrame(int connectionId,ConcurrentHashMap<String, String> headers,String body, Connections<String> connections) {
      super(connectionId,headers, body,connections);
   }

   public void process() {
      boolean toLogin = true;
      try {
         checkAcceptVersion();
         checkHost();
         checkLogin();
      } catch (IOException errowMessage) {
         toLogin = false;
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
      if (toLogin == true) {
         String userName  = (String)headers.get("login"); 
         String passcode = (String)headers.get("passcode");
         connections.login(connectionId,userName , passcode);
         ServerFrameSender.sendConnectedFrame((String)headers.get("accept-version"), connectionId, connections);

         if (headers.containsKey("receipt")){
            String receiptId = (String)headers.get("receipt");
            ServerFrameSender.sendReceiptFrame(connectionId, receiptId,connections);
         }
      }
   }

   public String getNameCommand() {
    return "CONNECT";
    }

    ////// processפונקציות עזר ל
    private void checkAcceptVersion() throws IOException {
        if (!headers.containsKey("accept-version")) {
           throw new IOException("Missing accept-version header:The frame should contain accept-version header");
        } else if (!((String)headers.get("accept-version")).equals(ConnectionsImpl.STOMP_VERSION)) {
           throw new IOException("Invalid accept-version:Accept-version must be 1.2 ");
        }
     }

     private void checkHost() throws IOException {
        if (!headers.containsKey("host")) {
           throw new IOException("Missing host header:CONNECT frame must contain host header");
        } else if (!((String)headers.get("host")).equals(ConnectionsImpl.HOST)) {
           throw new IOException("Invalid host:The host must be equal to:"+ ConnectionsImpl.HOST);
        }
     }

     private void checkLogin() throws IOException {
        if (headers.containsKey("login") && headers.containsKey("passcode")) {
            String userName = (String)headers.get("login");
            String password = (String)headers.get("passcode");
           if (connections.checkPasswordToUser(userName, password)){//
              throw new IOException("Incorrect passcode:The passcode does not match the username" + userName);
           } else if (connections.checkIfUserLogedIn(userName, password)){
              throw new IOException("Multiple login:The user"+ userName+ "already logged in");
           }
        } else {
           throw new IOException("Missing login and password headers:CONNECT frame must contain login and password headers");
        }
     }
}
