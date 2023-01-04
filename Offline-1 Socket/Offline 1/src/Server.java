import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Date;

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
        }
        
    }
    
}
