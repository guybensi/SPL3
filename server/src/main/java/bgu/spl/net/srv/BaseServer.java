package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.function.Supplier;

public abstract class BaseServer<T> implements Server<T> {

    private final int port;
    private final Supplier<MessagingProtocol<T>> protocolFactory;
    private final Supplier<MessageEncoderDecoder<T>> encdecFactory;
    private ServerSocket sock;
    private final Connections<T> connections; ///Our update
    private int connectionIdCounter = 0; ///Our update

    public BaseServer(
            int port,
            Supplier<MessagingProtocol<T>> protocolFactory,
            Supplier<MessageEncoderDecoder<T>> encdecFactory) {

        this.port = port;
        this.protocolFactory = protocolFactory;
        this.encdecFactory = encdecFactory;
        this.sock = null;
        this.connections = new ConnectionsImpl<T>(); ///Our update
    }

    @Override
    public void serve() {

        try (ServerSocket serverSock = new ServerSocket(port)) {
            System.out.println("Server started");
            this.sock = serverSock;
            while (!Thread.currentThread().isInterrupted()) {
                Socket clientSock = serverSock.accept();
                BlockingConnectionHandler<T> handler = new BlockingConnectionHandler<>(clientSock, encdecFactory.get(),protocolFactory.get());
                handler.getProtocol().start(connectionIdCounter, connections);///Our update
                connections.addConnection(connectionIdCounter, handler);///Our update
                connectionIdCounter++;///Our update
                execute(handler);
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        }
        System.out.println("server closed!!!");
    }

    @Override
    public void close() throws IOException {
        if (sock != null)
            sock.close();
    }

    protected abstract void execute(BlockingConnectionHandler<T> handler);

}