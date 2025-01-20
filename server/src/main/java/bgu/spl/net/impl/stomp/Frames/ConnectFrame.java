package bgu.spl.net.impl.stomp.Frames;

import bgu.spl.net.srv.Connections;
import java.io.IOException;
import java.util.Map;

public class ConnectFrame extends Frame {
   ConnectFrame(int connectionId,Map<String, String> headers,String body, Connections<String> connections) {
      super(connectionId,headers, body,connections);
   }
   public void process() {

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
           if (connections.isLegalLoginInfo(userName, password)){
              throw new IOException("Password does not match UserName:User " + (String)this.headers.get("login") + "'s password is diffrent than what you inserted");
           } else if (connections.isUserLogedIn(userName, password)){
              throw new IOException("User already logged in:User " + (String)this.headers.get("login") + "is logged in somewhere else");
           }
        } else {
           throw new IOException("CONNECT frame must contain login and password headers");
        }
     }

   
}
