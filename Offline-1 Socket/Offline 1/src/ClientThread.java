import java.io.*;
import java.net.Socket;

public class ClientThread extends Thread {
    private final static int PORT = 5093;
    private final static int CHUNK_SIZE = 16;

    private final Socket socket;
    private final String req;
    private final BufferedReader in;
    private final PrintWriter pr;
//    private final DataInputStream dataIn;
    private final DataOutputStream dataOut;
    public ClientThread(String req) throws IOException {
        this.req = req;
        this.socket = new Socket("localhost", PORT);
        this.in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        this.pr = new PrintWriter(socket.getOutputStream());
//        this.dataIn = new DataInputStream(socket.getInputStream());
        this.dataOut = new DataOutputStream(socket.getOutputStream());
    }

    private void handleFileUpload(File file) {
        try {
            int bytes = 0;
            FileInputStream fis = new FileInputStream(file);
            dataOut.writeLong(file.length());
            dataOut.flush();
            byte[] buffer = new byte[CHUNK_SIZE];
            while ((bytes = fis.read(buffer)) != -1) {
                dataOut.write(buffer, 0, bytes);
                dataOut.flush();
            }
            fis.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void run() {
        try {
            if (!req.startsWith("UPLOAD")) {
                // send the request to server
                pr.println(req);
                pr.flush();
                // read the response from server
                String res = in.readLine();
                System.out.println(res);

            } else {
                String fileName = req.split(" ")[1];
                File file = new File(fileName);
                if (!file.exists()) {
                    System.out.println(file.getPath() + " not found");
                } else {
                    // send the request to server
                    pr.println(req);
                    pr.flush();
                    // read the response from server
                    String res = in.readLine();
                    System.out.println(res);

                    if (res.contains("OK")) {
                        System.out.println("Server is ready to receive file");
                        System.out.println("Uploading file: " + fileName);

                        pr.println("UPLOADING");
                        pr.flush();
                        // upload the file to server
                        handleFileUpload(file);

                        System.out.println("Uploaded file: " + fileName);
                    } else {
                        pr.println("CANCEL");
                        pr.flush();
                        System.out.println("Server rejected the request");
                    }
                }
            }

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                socket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
