package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
      if (args.length != 2) {
         System.out.println("Please enter two arguments: port and the type of server you want - tpc or reactor)");
         System.exit(1);
      }
      int port = Integer.parseInt(args[0]);
      String serverType = args[1];
      if (serverType.equals("tpc")) {
        Server.threadPerClient(port, StompProtocolImpl::new, StompEncoderDecoder::new).serve();
      } else if (serverType.equals("reactor")) {
        Server.reactor(Runtime.getRuntime().availableProcessors(), port, StompProtocolImpl::new, StompEncoderDecoder::new).serve();
      } else {
         System.out.println("The second argument must be tpc or reactor");
         System.exit(1);
      }
   }
}
