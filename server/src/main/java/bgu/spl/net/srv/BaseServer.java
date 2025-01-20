package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Supplier;

public abstract class BaseServer<T> implements Server<T> {

    private final int port;
    private final Supplier<MessagingProtocol<T>> protocolFactory;
    private final Supplier<MessageEncoderDecoder<T>> encdecFactory;
    private ServerSocket sock;
    private final Connections<T> connections; // Connections instance
    private final AtomicInteger connectionIdCounter; // Unique ID generator for connections

    public BaseServer(
            int port,
            Supplier<MessagingProtocol<T>> protocolFactory,
            Supplier<MessageEncoderDecoder<T>> encdecFactory) {

        this.port = port;
        this.protocolFactory = protocolFactory;
        this.encdecFactory = encdecFactory;
        this.sock = null;
        this.connections = new ConnectionsImpl<T>(); // Initialize Connections
        this.connectionIdCounter = new AtomicInteger(0); // Initialize connection ID generator
    }

    @Override
    public void serve() {

        try (ServerSocket serverSock = new ServerSocket(port)) {
            System.out.println("Server started");

            this.sock = serverSock; // just to be able to close

            while (!Thread.currentThread().isInterrupted()) {

                Socket clientSock = serverSock.accept();

                int connectionId = connectionIdCounter.getAndIncrement(); // Generate unique connection ID
                BlockingConnectionHandler<T> handler = new BlockingConnectionHandler<>(
                        clientSock,
                        encdecFactory.get(),
                        protocolFactory.get());

                // Add the new connection to the Connections instance
                connections.addConnection(connectionId, handler);
                // =---------------- cliant protocol-------------------
                // handler.getProtocol().start(connectionId, connections);

                // Start the connection handler
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