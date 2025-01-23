package bgu.spl.net.srv;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    //our metods
    void addConnection(int connectionId, ConnectionHandler<T> handler);

    boolean checkPasswordToUser(String userName, String Password);
    boolean checkIfUserLogedIn(String userName, String Password);
    void login(int connectionId, String userName, String password);

    void subscribe(int connectionId, int subscriptionId,String channel);    
    ConnectionHandler<T> getCHbyconnectionId(int connectionId);

    void unsubscribe(int connectionId, int subscriptionId);

    boolean isDestinationLegal(String channel, int connectionId);
    int getAndIncMsgIdCounter();
}
