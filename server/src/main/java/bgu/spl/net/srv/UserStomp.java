package bgu.spl.net.srv;

import java.util.concurrent.ConcurrentHashMap;

public class UserStomp<T> {
   private int connectionId;
   private String username;
   private String password;
   private ConnectionHandler<T> connectionHandler;
   private ConcurrentHashMap<Integer, String> ChannelSubscriptions;
   private boolean isConnected;

    //Geters
    public int getConnectionId() {
       return connectionId;
    }

    public String getUsername() {
       return username;
    }

    public String getPassword() {
       return password;
    }

    public ConnectionHandler<T> getConnectionHandler() {
       return connectionHandler;
    }

     public ConcurrentHashMap<Integer, String> getChannelSubscriptions() {
       return ChannelSubscriptions;
    }

    public boolean isConnected() {
       return isConnected;
    }

    //Seters
    public void setIsConnected(boolean ans){
        isConnected = ans;
    }

    public void setConnectionHandler(ConnectionHandler<T> handler){
        connectionHandler = handler;
    }

    public void setConnectionId(int connectionId) {
        this.connectionId = connectionId;
    }
}
