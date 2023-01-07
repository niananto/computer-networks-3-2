import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;

public class Server {
    private static final int PORT = 5093;
    
    public static void main(String[] args) throws IOException {
        
        ServerSocket serverConnect = new ServerSocket(PORT);

        while(true)
        {
            System.out.println("Server started.\nListening for connections on port : " + PORT + " ...\n");
            Socket s = serverConnect.accept();
            System.out.println("Connection established");

            Thread serverThread = new ServerThread(s);
            serverThread.start();

//            System.out.println("Threads running: " + Thread.activeCount());
//            System.out.println("Server thread running: " + ServerThread.serverThreadCount);
        }
        
    }
    
}
