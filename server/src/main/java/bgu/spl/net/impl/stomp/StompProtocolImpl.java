package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;
import java.util.Map;
import java.util.HashMap;

public class StompProtocolImpl<T> implements StompMessagingProtocol<T> {

    private int connectionId;
    private Connections<T> connections =  new ConnectionsImpl<>(); ////////////////cheak!!!
    private boolean shouldTerminate = false;
    private final Map<String, String> subscriptionIdToChannel = new HashMap<>();

    @Override
    public void start(int connectionId, Connections<T> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
    }

    @Override
    public void process(T message) {
        String frame = (String) message;
        String[] lines = frame.split("\n");
        String command = lines[0];

        switch (command) {
            case "CONNECT":
                handleConnect(frame);
                break;
            case "SEND":
                handleSend(frame);
                break;
            case "SUBSCRIBE":
                handleSubscribe(frame);
                break;
            case "UNSUBSCRIBE":
                handleUnsubscribe(frame);
                break;
            case "DISCONNECT":
                handleDisconnect(frame);
                break;
            default:
                sendError("Unknown command: " + command);
                break;
        }
    }

    private void handleConnect(String frame) {
        String login = extractHeader(frame, "login");
        String passcode = extractHeader(frame, "passcode");
    
        if (isValidUser(login, passcode)) {
            LoggedIn = true;
            connections.send(connectionId, (T) "CONNECTED\nversion:1.2\n\n\u0000");
            System.out.println("Client " + connectionId + " connected successfully as user: " + login);
        } else {
            sendError("Invalid login or passcode");
            System.out.println("Client " + connectionId + " failed to connect with login: " + login);
        }
    }

    private void handleSend(String frame) {
        // שליפת ה-destination והגוף מתוך המסגרת
        String destination = extractHeader(frame, "destination");
        String body = extractBody(frame);
    
        if (destination == null || body == null) {
            sendError("Missing destination or body in SEND frame");
            return;
        }
    
        // בדיקה אם הלקוח רשום לערוץ
        boolean isSubscribed = subscriptionIdToChannel.containsValue(destination);
        if (!isSubscribed) {
            sendError("Client not subscribed to destination: " + destination);
            System.out.println("Client " + connectionId + " attempted to send to unsubscribed destination: " + destination);
            return;
        }
    
        // שליחת ההודעה לכל המנויים של הערוץ
        String messageFrame = "MESSAGE\ndestination:" + destination + "\n\n" + body + "\u0000";
        connections.send(destination, (T) messageFrame);
        System.out.println("Message sent to destination: " + destination + " by client: " + connectionId);
    
        // שליחת RECEIPT אם נדרש
        String receipt = extractHeader(frame, "receipt");
        if (receipt != null) {
            sendReceipt(receipt);
        }
    }

    private void handleSubscribe(String frame) {
        String destination = extractHeader(frame, "destination");
        String subscriptionId = extractHeader(frame, "id");

        if (destination == null || subscriptionId == null) {
            sendError("Missing destination or subscription id in SUBSCRIBE frame");
            return;
        }

        subscriptionIdToChannel.put(subscriptionId, destination);
        connections.subscribe(destination, connectionId);

        System.out.println("Subscribed to destination: " + destination + " with subscription id: " + subscriptionId);

        String receipt = extractHeader(frame, "receipt");
        if (receipt != null) {
            sendReceipt(receipt);
        }
    }

    private void handleUnsubscribe(String frame) {
        String subscriptionId = extractHeader(frame, "id");

        if (subscriptionId == null) {
            sendError("Missing subscription id in UNSUBSCRIBE frame");
            return;
        }

        String destination = subscriptionIdToChannel.remove(subscriptionId);

        if (destination == null) {
            sendError("No channel found for subscription id: " + subscriptionId);
            return;
        }

        connections.unsubscribe(destination, connectionId);
        System.out.println("Unsubscribed from subscription id: " + subscriptionId + " (channel: " + destination + ")");

        String receipt = extractHeader(frame, "receipt");
        if (receipt != null) {
            sendReceipt(receipt);
        }
    }

    private void handleDisconnect(String frame) {
        subscriptionIdToChannel.clear();
        connections.disconnect(connectionId);

        System.out.println("Client " + connectionId + " disconnected.");

        String receipt = extractHeader(frame, "receipt");
        if (receipt != null) {
            sendReceipt(receipt);
        }

        shouldTerminate = true;
    }


    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }
}
