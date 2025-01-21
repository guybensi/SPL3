package bgu.spl.net.srv;

import java.util.LinkedList;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;


public class ConnectionsImpl<T> implements Connections<T> {

    private final ConcurrentMap<Integer, ConnectionHandler<T>> activeConnectionHandlers; // Map of connectionId -> ConnectionHandler
    private final ConcurrentMap<String, LinkedList<Integer>> channels; // Map of channel -> Set of connectionIds
    private ConcurrentHashMap<String, UserStomp<T>> users = new ConcurrentHashMap<>(); // Map of users names -> UserStomp

    public ConnectionsImpl() {
        activeConnectionHandlers = new ConcurrentHashMap<>();
        channels = new ConcurrentHashMap<>();
        users = new ConcurrentHashMap<>();
    }
    // Helper method to add a connection
    public void addConnection(int connectionId, ConnectionHandler<T> handler) {
        activeConnectionHandlers.put(connectionId, handler);
    }

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> handler = activeConnectionHandlers.get(connectionId);
        if (handler != null) {
            handler.send(msg);
            return true;
        }
        return false; // Client not found
    }

    @Override
    public void send(String channel, T msg) {
        LinkedList<Integer> subscribers = channels.get(channel);
        if (subscribers != null) {
            for (int connectionId : subscribers) {
                send(connectionId, msg); // Send the message to each subscriber
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        UserStomp<T> userToDisconnect = ((ConnectionHandler<T>)this.activeConnectionHandlers.get(connectionId)).getUser();
        if (userToDisconnect != null) {
            //unsubscribe the user from all channels
            for (LinkedList<Integer> subscribers : channels.values()) {
                subscribers.remove(connectionId);
            }
            //Update the user as disconnected
            userToDisconnect.getChannelSubscriptions().clear();
            userToDisconnect.setIsConnected(false);
            userToDisconnect.setConnectionHandler(null);
            userToDisconnect.setConnectionId(-1);
        }
        activeConnectionHandlers.remove(connectionId);
    }
    //////////////////////////////////////////////////////////////////
    
    //////////for connect frame//////////
    public boolean checkPasswordToUser(String userName, String Password) {
        return !this.users.containsKey(userName) || ((UserStomp<T>)this.users.get(userName)).getPassword().equals(Password);
    }
    public boolean checkIfUserLogedIn(String userName, String Password) {
        return this.users.containsKey(userName) && ((UserStomp<T>)this.users.get(userName)).isConnected();
    }
    public void login(int connectionId, String userName, String password) {
        UserStomp<T> user;
        ConnectionHandler<T> newHandler = (ConnectionHandler<T>)this.activeConnectionHandlers.get(connectionId);
        if (this.users.containsKey(userName)) {
            user = (UserStomp<T>)this.users.get(userName);
            user.setIsConnected(true);
            user.setConnectionId(connectionId);
            user.setConnectionHandler(newHandler);
        } else {
           user = new UserStomp<T> (connectionId, userName, password, (ConnectionHandler<T>)this.activeConnectionHandlers.get(connectionId));
           users.put(userName, user);        
        }
        newHandler.setUser(user);
     }

    //////////for subscribe frame//////////
    public void subscribe(int connectionId, int subscriptionId,String channel ) {
        //add the subscription to the user's subscription list 
        ConnectionHandler <T> currHandler = (ConnectionHandler<T>)activeConnectionHandlers.get(connectionId);
        UserStomp<T> currUser =  (UserStomp<T>)currHandler.getUser() ;
        Map<Integer, String> ChannelsOfUser = currUser.getChannelSubscriptions();
        ChannelsOfUser.put(subscriptionId, channel);
        //add the connectionId to the channel's list
        if (!this.channels.containsKey(channel)) {
            this.channels.put(channel, new LinkedList<Integer>());
        }
        ((LinkedList<Integer>)channels.get(channel)).push(connectionId);
    }
    public ConnectionHandler<T> getCHbyconnectionId(int connectionId) {
        return (ConnectionHandler<T>)this.activeConnectionHandlers.get(connectionId);
     }

    //////////for unsubscribe frame//////////
    public void unsubscribe(int connectionId, int subscriptionId) {
        ConnectionHandler <T> currHandler = (ConnectionHandler<T>)activeConnectionHandlers.get(connectionId);
        UserStomp<T> currUser =  (UserStomp<T>)currHandler.getUser() ;
        Map<Integer, String> ChannelsOfUser = currUser.getChannelSubscriptions();
        String channelToRemove = (String)ChannelsOfUser.get(subscriptionId);
        ///remove the subscription from the channel's list
        ((LinkedList<Integer>)this.channels.get(channelToRemove)).remove(connectionId);
        ///remove the channel from the user's subscription list
        ChannelsOfUser.remove(subscriptionId);
     }

}