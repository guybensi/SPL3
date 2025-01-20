package bgu.spl.net.srv;

import java.util.LinkedList;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CopyOnWriteArraySet;


public class ConnectionsImpl<T> implements Connections<T> {

    private final ConcurrentMap<Integer, ConnectionHandler<T>> activeConnectionHandlers; // Map of connectionId -> ConnectionHandler
    private final ConcurrentMap<String, CopyOnWriteArraySet<Integer>> channels; // Map of channel -> Set of connectionIds
    private ConcurrentHashMap<String, UserStomp<T>> users = new ConcurrentHashMap(); // Map of users names -> UserStomp

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
        CopyOnWriteArraySet<Integer> subscribers = channels.get(channel);
        if (subscribers != null) {
            for (int connectionId : subscribers) {
                send(connectionId, msg); // Send the message to each subscriber
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        UserStomp<T> userToDisconnect = ((ConnectionHandler)this.activeConnectionHandlers.get(connectionId)).getUser();
        if (userToDisconnect != null) {
            //unsubscribe the user from all channels
            for (CopyOnWriteArraySet<Integer> subscribers : channels.values()) {
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
    
    //for connect frame
    public boolean checkPasswordToUser(String userName, String Password) {
        return !this.users.containsKey(userName) || ((UserStomp)this.users.get(userName)).getPassword().equals(Password);
    }
    public boolean checkIfUserLogedIn(String userName, String Password) {
        return this.users.containsKey(userName) && ((UserStomp)this.users.get(userName)).isConnected();
    }
    public void login(int connectionId, String userName, String password) {
        UserStomp user;
        ConnectionHandler<T> newHandler = (ConnectionHandler)this.activeConnectionHandlers.get(connectionId);
        if (this.users.containsKey(userName)) {
            user = (UserStomp)this.users.get(userName);
            user.setIsConnected(true);
            user.setConnectionId(connectionId);
            user.setConnectionHandler(newHandler);
        } else {
           user = new UserStomp (connectionId, userName, password, (ConnectionHandler)this.connectionHandlers.get(connectionId));
           users.put(userName, user);        
        }
        newHandler.setUser(user);
     }

    //for subscribe frame
    public void subscribe(String channel, int connectionId) {
        channels.putIfAbsent(channel, new CopyOnWriteArraySet<>());
        channels.get(channel).add(connectionId);
    }

    //for unsubscribe frame
    public void unsubscribe(String channel, int connectionId) {
        CopyOnWriteArraySet<Integer> subscribers = channels.get(channel);
        if (subscribers != null) {
            subscribers.remove(connectionId);
        }
    }
}