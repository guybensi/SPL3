package bgu.spl.net.impl.stomp.Frames;

import bgu.spl.net.srv.Connections;
import java.io.IOException;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectFrame extends Frame {
   ConnectFrame(int connectionId,ConcurrentHashMap<String, String> headers,String body, Connections<String> connections) {
      super(connectionId,headers, body,connections);
   }

   public void process() {
    boolean toLogin = true;
    try {
       this.checkAcceptVersion();
       this.checkHost();
       this.checkLogin();
    } catch (IOException errowMessage) {
       toLogin = false;
       ///String[] SummaryAndBodyErr = var4.getMessage().split(":", 2);
       //FrameUtil.handleError(this, SummaryAndBodyErr[0], SummaryAndBodyErr[1], this.connections, this.connectionId, (String)this.headers.get("receipt"));
    }
    if (toLogin == true) {
        String userName  = (String)this.headers.get("login"); 
        String passcode = (String)this.headers.get("passcode");
        connections.login(this.connectionId,userName , passcode);
        //FrameUtil.sendConnectedFrame((String)this.headers.get("accept-version"), this.connectionId, this.connections);
        //if (this.headers.containsKey("receipt")) {
            //FrameUtil.sendReceiptFrame((String)this.headers.get("receipt"), this.connections, this.connectionId);
        //}
   }

   public String getNameCommand() {
    return "CONNECT";
    }

    ////// processפונקציות עזר ל
    private void checkAcceptVersion() throws IOException {
        if (!this.headers.containsKey("accept-version")) {
           throw new IOException("the frame should contain accept-version header");
        } else if (!((String)this.headers.get("accept-version")).equals(FrameUtil.STOMP_VERSION)) {
           throw new IOException("accept-version must be 1.2 ");
        }
     }

     private void checkHost() throws IOException {
        if (!this.headers.containsKey("host")) {
           throw new IOException("the frame should contain host header");
        } else if (!((String)this.headers.get("host")).equals(FrameUtil.HOST)) {
           throw new IOException("the host must be equal to:"+ FrameUtil.HOST);
        }
     }

     private void checkLogin() throws IOException {
        if (this.headers.containsKey("login") && this.headers.containsKey("passcode")) {
            String userName = (String)this.headers.get("login");
            String password = (String)this.headers.get("passcode");
           if (connections.checkPasswordToUser(userName, password)){//
              throw new IOException("The password does not match the username" + userName);
           } else if (connections.checkIfUserLogedIn(userName, password)){
              throw new IOException("the user"+ userName+ "already logged in");
           }
        } else {
           throw new IOException("CONNECT frame must contain login and password headers");
        }
     }
}
