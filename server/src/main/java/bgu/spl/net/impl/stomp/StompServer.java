package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Reactor;
import bgu.spl.net.srv.ReactorServer;
import bgu.spl.net.srv.Server;
import bgu.spl.net.srv.TPCServer;

public class StompServer {

    public static void main(String[] args) {
        if(args.length < 2){
            System.out.println("should get exactly 2 args : <port> <tpc/reactor>");
            System.exit(1);
        }
        int port = Integer.parseInt(args[0]);
        String serverType = args[1];

        Server<String> server;

        if(serverType.equals("tpc")){
            server = new TPCServer<>(port, 
                () -> new StompMessagingProtocolImpl(), 
                () -> new StompEncDec()
                );
            server.serve();
        } else if(serverType.equals("reactor")){
           final int NUM_THREADS = 100;
           server =  new Reactor<>(NUM_THREADS,
             port,
              () -> new StompMessagingProtocolImpl(),
               () -> new StompEncDec());
               server.serve();
        }
            
                
            
    }
}
