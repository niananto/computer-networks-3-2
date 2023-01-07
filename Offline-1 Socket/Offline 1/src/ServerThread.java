import java.io.*;
import java.net.Socket;
import java.util.Date;

public class ServerThread extends Thread {
    private final Socket socket;
    private final BufferedReader in;
    private final PrintWriter pr;
    private final BufferedWriter logWriter;
    private static final int CHUNK_SIZE = 32;
    public static int serverThreadCount = 0;
    private static final String ROOT_DIR = "root";
    private final DataOutputStream dataOut;
    private final DataInputStream dataIn;

    public ServerThread(Socket socket) {
        this.socket = socket;
        serverThreadCount++;

        try {
            logWriter = new BufferedWriter(new FileWriter("log.txt", true));
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            pr = new PrintWriter(socket.getOutputStream());
            dataOut = new DataOutputStream(socket.getOutputStream());
            dataIn = new DataInputStream(socket.getInputStream());
        } catch (IOException e) {
            throw new RuntimeException(e);
        }

        System.out.println("Remote port: " + socket.getPort());
        System.out.println("Local port: " + socket.getLocalPort());
    }

    private static String response417() {
        return "HTTP/1.1 417 Expectation Failed\r\n" +
                "Date: " + new Date() + "\r\n" +
                "Content-Type: text/html\r\n" +
                "Content-Length: 0\r\n" +
                "Connection: close\r\n" +
                "\r\n";
    }

    private static String response405() {
        return "HTTP/1.1 405 Method Not Allowed\r\n" +
                "Server: Java HTTP Server : 1.0\r\n" +
                "Date: " + new Date() + "\r\n" +
                "Content-type: text/html\r\n" +
                "Content-Length: 0\r\n" +
                "Connection: close\r\n" +
                "\r\n";
    }

    private static String response404() {
        return "HTTP/1.1 404 Not Found\r\n" +
                "Server: Java HTTP Server : 1.0\r\n" +
                "Date: " + new Date() + "\r\n" +
                "Content-type: text/html\r\n" +
                "Content-Length: 0\r\n" +
                "Connection: close\r\n" +
                "\r\n";
    }

    private static String response400() {
        return "HTTP/1.1 400 Bad Request\r\n" +
                "Server: Java HTTP Server : 1.0\r\n" +
                "Date: " + new Date() + "\r\n" +
                "Content-type: text/html\r\n" +
                "Content-Length: 0\r\n" +
                "Connection: close\r\n" +
                "\r\n";
    }

    private static String response200(String contentType, int contentLength) {
        return "HTTP/1.1 200 OK\r\n" +
                "Server: Java HTTP Server : 1.0\r\n" +
                "Date: " + new Date() + "\r\n" +
                "Content-type: " + contentType + "\r\n" +
                "Content-Length: " + contentLength + "\r\n" +
                "Connection: close\r\n" +
                "\r\n";
    }

    private void sendResponse200(String contentType, int contentLength, String content) throws IOException {
        String res = response200(contentType, contentLength);
        pr.write(res + content);
        pr.flush();
        logWriter.write("Response:\n" + res + "\n\n");
        logWriter.flush();
    }

    private void sendData(File directory) throws IOException {
        FileInputStream fis = new FileInputStream(directory);
        byte[] buffer = new byte[CHUNK_SIZE];
        int bytesRead;
        while ((bytesRead = fis.read(buffer)) != -1) {
            dataOut.write(buffer, 0, bytesRead);
            dataOut.flush();
        }
        fis.close();
    }

    private void handleGetRequest(String input) throws Exception {

        String contentType = "";
        String reqPath = input.split(" ")[1];
        reqPath = reqPath.replaceAll("%20", " ");

        if (reqPath.equals("/")) {
            contentType = "text/html";
            String content = "<html><body><ul>\n"
                    + "\t<li><a href=\"root\">root</a></li>\n"
                    + "\t<li><a href=\"uploaded\">uploaded</a></li>\n"
                    + "</ul></body></html>";

            sendResponse200(contentType, content.length(), content);
        }

        reqPath = reqPath.replaceFirst("/", "");
        File directory = new File(reqPath);
        if (!directory.exists()) {
            pr.write(response404());
            pr.flush();
            logWriter.write("Response:\n" + response404() + "\n\n");
            logWriter.flush();
            System.out.println("404 Not Found - " + reqPath);
            return;
        }

        File[] files = directory.listFiles();
        StringBuilder sb = new StringBuilder();

        if (files != null) {
            // requested path is a folder with some other files/folders in it
            contentType = "text/html";
            sb.append("<html><body><ul>\n");

            for (File file : files) {

                if (file.isDirectory()) {
                    sb.append("\t<li><a href=\"/").append(file.getPath()).append("\">");
                    sb.append("<b><i>").append(file.getName()).append("</i></b>");

                } else {
                    String ext = "";
                    if (file.getName().contains(".")) {
                        ext = file.getName().split("\\.")[1];
                    }

                    // If path is a text or image file, then show the content of that file in a new html page
                    if (ext.equals("jpg") || ext.equals("png") || ext.equals("jpeg") || ext.equals("txt")) {

                        sb.append("\t<li><a href=\"/").append(file.getPath()).append("\" target=\"blank\">");
                        sb.append(file.getName());

                    } else {
                        // For all other formats enforce downloading
                        sb.append("\t<li><a href=\"/").append(file.getPath()).append("\">");
                        sb.append(file.getName());
                    }
                }
                sb.append("</a></li>\n");
            }
            sb.append("</ul></body></html>");
            sendResponse200(contentType, sb.toString().getBytes().length, sb.toString());
            return;

        }

        // requested path is a file
        String ext = "";
        if (directory.getName().contains(".")) {
            ext = directory.getName().split("\\.")[1];
        }

        // If path is a text or image file show that
        // For all other formats enforce downloading
        if (ext.equalsIgnoreCase("txt")) {
            contentType = "text/plain";

        } else if (ext.equalsIgnoreCase("jpg") || ext.equalsIgnoreCase("png") || ext.equalsIgnoreCase("jpeg")) {
            contentType = "image/" + ext;

        } else {
            contentType = "application/octet-stream";

        }

        sendResponse200(contentType, (int) directory.length(), "");
        // Read the file and send it to the client
        sendData(directory);
    }

    private void handleUploadRequest(String input) throws IOException {
        String[] tokens = input.split(" ");
        if (tokens.length < 2) {
            String res = response400();
            pr.write(res);
            pr.flush();
            logWriter.write("Response:\n" + res + "\n\n");
            logWriter.flush();
            return;
        }

        // send response
        sendResponse200("text/html", 0, "");

        String[] t = tokens[1].split("/");
        String fileName = t[t.length - 1];
//        fileName = fileName.replaceAll("%20", " ");


        String reqMsg = in.readLine();
        System.out.println("Request: " + reqMsg);
        logWriter.write("Request:\n" + reqMsg + "\n\n");
        logWriter.flush();

        if (!reqMsg.contains("CANCEL")) {

            FileOutputStream fos = new FileOutputStream("uploaded/" + fileName);
            byte[] buffer = new byte[CHUNK_SIZE];
            int bytes;

            long fileSize = dataIn.readLong();
            long received = 0;
            while (fileSize > 0 && (bytes = dataIn.read(buffer, 0, (int) Math.min(buffer.length, fileSize))) != -1) {
                fos.write(buffer, 0, bytes);
                fos.flush();

                // show progress bar here
//                received += bytes;
//                System.out.print("\r" + (received * 100 / fileSize) + "%\t");
//                for (int i=0; i<received*20/fileSize; i++) {
//                    System.out.print("=");
//                }
            }
            System.out.println();
            fos.close();
            System.out.println("Upload complete: " + fileName);

        } else {
            System.out.println("Upload failed: " + fileName);

            String res = response417();
            pr.write(res);
            pr.flush();
            logWriter.write("Response:\n" + res + "\n\n");
            logWriter.flush();
        }
    }

    @Override
    public void run() {
        try {
            String input = in.readLine();

            if (input == null) return;
            if (input.length() == 0) return;
            if (input.contains("/favicon.ico")) {
                pr.write(response404());
                pr.flush();
                return;
            }

            System.out.println("Request: " + input);
            logWriter.write("Request:\n" + input + "\n\n");
            logWriter.flush();

            if (!input.startsWith("GET") && !input.startsWith("UPLOAD")) {
                pr.write(response405());
                pr.flush();
                logWriter.write("Response:\n" + response405() + "\n\n");
                logWriter.flush();
                return;
            }
            if (input.startsWith("GET")) {
                handleGetRequest(input);
                return;
            }
            if (input.startsWith("UPLOAD")) {
                handleUploadRequest(input);
                return;
            }

        } catch (Exception e) {
            e.printStackTrace();

        } finally { // finally block gets executed even for a return

            try {
                // should I close the socket here?
                socket.close();
                logWriter.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

            // looks like a perfect place to decrement the counter
            serverThreadCount--;
        }
    }
}
