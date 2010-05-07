/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package progin5;

/**
 *
 * @author Ginanjar Fahrul M
 */
import java.net.*;
import java.io.*;
import java.util.logging.Level;
import java.util.logging.Logger;

public class FileSender extends Thread {
    private Socket sock;
    public FileSender(String IPReceiver)
    {
        try {
            sock = new Socket(IPReceiver, 7676);
            System.out.println("Connecting...");
        } catch (UnknownHostException ex) {
            Logger.getLogger(FileSender.class.getName()).log(Level.SEVERE, null, ex);
        } catch (IOException ex) {
            Logger.getLogger(FileSender.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    public void sendFile(String path) throws IOException
    {
        File myFile = new File(path);
        System.out.println("ini path " + path);
        byte [] mybytearray = new byte[(int)myFile.length()];
        FileInputStream fis;
        try {
            fis = new FileInputStream(myFile);
            BufferedInputStream bis = new BufferedInputStream(fis);
            bis.read(mybytearray, 0, mybytearray.length);
        } catch (FileNotFoundException ex) {
            Logger.getLogger(FileSender.class.getName()).log(Level.SEVERE, null, ex);
        }
        OutputStream os = sock.getOutputStream();
        System.out.println("Sending...");
        os.write(mybytearray, 0, mybytearray.length);
        os.flush();
        sock.close();
    }

    public static void main (String [] args ) throws IOException {
        FileSender fs = new FileSender("127.0.0.1");
        fs.sendFile("D:\\Institut Teknologi Bandung\\Kuliah\\Semester VI\\Pemrograman Internet\\progin_tugas_3.pdf");
    }
}