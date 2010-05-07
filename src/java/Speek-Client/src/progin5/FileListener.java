/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package progin5;

import java.net.*;
import java.io.*;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JFileChooser;

/**
 *
 * @author Ginanjar Fahrul M
 */

public class FileListener extends Thread{
    private ServerSocket servsock;
    public FileListener()
    {
        try {
            servsock = new ServerSocket(7676);
        } catch (IOException ex) {
            Logger.getLogger(FileListener.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    @Override
    public void run()
    {
        int filesize = 6022386; // filesize temporary hardcoded
        int bytesRead;


        while (true) {
            System.out.println("Waiting...");
            byte [] mybytearray = new byte [filesize];
            long start = System.currentTimeMillis();
            int current = 0;
            Socket sock;
            InputStream is;
            BufferedOutputStream bos;
            try {
                sock = servsock.accept();
                System.out.println("Accepted connection : " + sock);
                is = sock.getInputStream();
                String path = "";
                JFileChooser browseFile = new JFileChooser();
                int returnval = browseFile.showSaveDialog(null);
                if (returnval == JFileChooser.APPROVE_OPTION) {
                    path = browseFile.getSelectedFile().getAbsolutePath();
                    FileOutputStream fos = new FileOutputStream(path);
                    bos = new BufferedOutputStream(fos);
                    bytesRead = is.read(mybytearray, 0, mybytearray.length);
                    current = bytesRead;
                    do{
                        bytesRead =
                            is.read(mybytearray, current, (mybytearray.length - current));
                        if(bytesRead >= 0) current += bytesRead;
                    } while (bytesRead != -1);
                    bos.write(mybytearray, 0, current);
                    bos.flush();
                    bos.close();
                }
                sock.close();
                long end = System.currentTimeMillis();
                System.out.println(end-start);
            } catch (IOException ex) {
                Logger.getLogger(FileListener.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    public static void main (String [] args ) throws IOException {
        FileListener fl = new FileListener();
    }
}