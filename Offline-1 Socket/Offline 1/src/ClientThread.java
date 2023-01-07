import java.io.*;
import java.net.Socket;

public class ClientThread extends Thread {
    private final static int PORT = 5093;
    private final static int CHUNK_SIZE = 32;

    private final Socket socket;
    private final String req;
    private final BufferedReader in;
    private final PrintWriter pr;
    private final DataOutputStream dataOut;
    public ClientThread(String req) throws IOException {
        this.req = req;
        this.socket = new Socket("localhost", PORT);
        this.in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        this.pr = new PrintWriter(socket.getOutputStream());
        this.dataOut = new DataOutputStream(socket.getOutputStream());
    }

    private void handleFileUpload(File file) {
        try {
            int bytes = 0;
            FileInputStream fis = new FileInputStream(file);
            dataOut.writeLong(file.length());
            dataOut.flush();
            long sent = 0;
            byte[] buffer = new byte[CHUNK_SIZE];
            while ((bytes = fis.read(buffer)) != -1) {
                dataOut.write(buffer, 0, bytes);
                dataOut.flush();

                // show progress bar here
                sent += bytes;
                System.out.print("\r" + (sent * 100 / file.length()) + "%\t");
                for (int i=0; i<sent*20/file.length(); i++) {
                    System.out.print("=");
                }

                // sleep for 1 ns to make it feel like it's on real internet
                // comment if necessary
//                Thread.sleep(0, 1);
            }
            System.out.println();
            fis.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void sendMessage(String msg) {
        pr.println(msg);
        pr.flush();
        System.out.println(msg);
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
                return;
            }

            String fileName = req.split(" ")[1];
            File file = new File(fileName);
            if (!file.exists()) {
                sendMessage(file.getPath() + " not found");
                return;
            }
            if (file.isDirectory()) {
                sendMessage(file.getPath() + " is a directory");
                return;
            }
            if (!file.canRead()) {
                sendMessage(file.getPath() + " cannot be read");
                return;
            }

            // get file extension
            String ext = "";
            int i = fileName.lastIndexOf('.');
            if (i > 0) {
                ext = fileName.substring(i+1);
            }
            // Only text and image files (.txt, .jpg, .png., .mp4 etc.) are allowed to upload
            if (!ext.matches("txt|jpg|png|mp4|mkv|avi|flv|mov|gif|jpeg|svg|csv|html|htm|log")) {
                sendMessage("Only text and image files (.txt, .jpg, .png., .mp4 etc.) are allowed to upload");
                return;
            }

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
