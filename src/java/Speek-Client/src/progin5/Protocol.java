package progin5;

import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JFileChooser;
import javax.swing.JFrame;

/**
 *
 * @author Ananti
 */
public class Protocol extends Thread {
    //constant service
    static final int SMSG_SERVICE_LOGON     = 1;
    static final int SMSG_SERVICE_LOGOFF    = 2;
    static final int SMSG_MESSAGE           = 3;
    static final int SMSG_FILETRANSFER      = 4;
    static final int SMSG_ADDFRIEND         = 5;
    static final int SMSG_REMFRIEND         = 6;
    static final int SMSG_RETFRIEND         = 7;
    static final int SMSG_STATUS_UPDATE     = 8;
    static final int SMSG_REGISTER          = 9;

    //constant status
    static final int SMSG_STATUS_FAILED     = 0;
    static final int SMSG_STATUS_SUCCESS    = 1;
    static final int SMSG_STATUS_ONLINE     = 2;
    static final int SMSG_STATUS_OFFLINE    = 3;
    static final int SMSG_STATUS_UNDEFINED  = -999;

    private String name;
    private int version;
    private int pkt_len;
    private int service;
    private int status;
    private String data;


    //socket
    public Socket requestSocket;
    public Socket respondSocket;
    public static BufferedOutputStream out;
    public static BufferedInputStream in;
    public FileListener listen;
    public String message;
    public String username;
    public MainMenu mainmenu;
    public Map map = new HashMap();

    @Override
    public void run()
    {
        while(true){
            System.out.println("waiting...");
            try {
                in = new BufferedInputStream(requestSocket.getInputStream());
                receiveMessage();
            } catch (IOException ex) {
                Logger.getLogger(Protocol.class.getName()).log(Level.SEVERE, null, ex);
            }

            if(getService() == Protocol.SMSG_SERVICE_LOGON)
            {
                System.out.println("SMSG_SERVICE_LOGON");
                if(getStatus() == Protocol.SMSG_STATUS_SUCCESS)
                {
                    mainmenu = new MainMenu();
                    mainmenu.setUsername(username);
                    mainmenu.setVisible(true);
                    //membuat listener untuk p2p
                    listen = new FileListener();
                    listen.start();
                    //listen.receiveFile("D:\\Institut Teknologi Bandung\\Kuliah\\Semester VI\\Pemrograman Internet\\progin_tugas_3_received.pdf");
                }
                else{
                    Login.getSingleton().setVisible(true);
                    socketClose();
                }
            }
            else if(getService() == Protocol.SMSG_REGISTER)
            {
                System.out.println("SMSG_REGISTER");
                //if(getStatus() == Protocol.SMSG_STATUS_SUCCESS)
                Login.getSingleton().setVisible(true);
                Register.getSingleton().setVisible(false);
                socketClose();
            }
            else if(getService() == Protocol.SMSG_RETFRIEND)
            {
                System.out.println("SMSG_RETFRIEND");
                String tData = Login.SMSG_PROTOCOL.getData().substring(0, pkt_len - 20);
                String tList[]= tData.split("\30");
                mainmenu.setBuddy(tList);
            }
            else if(getService() == Protocol.SMSG_MESSAGE)
            {
                System.out.println("SMSG_MESSAGE");
                String tData = Login.SMSG_PROTOCOL.getData().substring(0, pkt_len - 20);
                String tList[]= tData.split("\30");

                System.out.println("ini list"+getData());
                System.out.println("ini list"+tList[1]);
                
                String tMSG[] = tList[1].split("\31");
                System.out.println("|"+tMSG[0]+"|"+tMSG[1]+"|"+tMSG[2]);
                String temp = tMSG[0];
                chat(temp);
                ((ChatBox) map.get(tMSG[0])).writeChat(tMSG[0], tMSG[2]);
                System.out.println("pesan berhasil dikirim");
            }
            else if(getService() == Protocol.SMSG_SERVICE_LOGOFF)
            {

            }
            else if(getService() == Protocol.SMSG_FILETRANSFER)
            {
                System.out.println("SMSG_MESSAGE");
                if(getStatus() == Protocol.SMSG_STATUS_SUCCESS)
                {
                    String tData = Login.SMSG_PROTOCOL.getData().substring(0, pkt_len - 20);
                    String tList[]= tData.split("\30");

                    System.out.println("ini list"+getData());
                    System.out.println("ini list"+tList[1]);

                    String tMSG[] = tList[1].split("\31");
                    String temp = tMSG[0];
                    ((ChatBox) map.get(tMSG[0])).writeChat(tMSG[0], "sending file...");
                    FileSender sender = new FileSender(tMSG[1]);
                    try {
                        sender.sendFile(((ChatBox) map.get(tMSG[0])).getPath());
                    } catch (IOException ex) {
                        Logger.getLogger(Protocol.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }
            }
            else if(getService() == Protocol.SMSG_ADDFRIEND)
            {
                if(getStatus() == Protocol.SMSG_STATUS_SUCCESS)
                {
                    mainmenu.updateFriend();
                }
            }
            else if(getService() == Protocol.SMSG_REMFRIEND)
            {

            }
            else if(getService() == Protocol.SMSG_STATUS_UPDATE)
            {
                mainmenu.updateFriend();
            }
        }
    }

    public void chat(String temp)
    {
        if(((ChatBox) map.get(temp)) == null)
        {
            map.put(temp, new ChatBox());
            ((ChatBox) map.get(temp)).setTitle(temp);
            ((ChatBox) map.get(temp)).setName(temp);
            ((ChatBox) map.get(temp)).setVisible(true);
            ((ChatBox) map.get(temp)).setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
            ((ChatBox) map.get(temp)).setLocationRelativeTo(null);
        }
        else{
            ((ChatBox) map.get(temp)).setVisible(true);
        }
        ((ChatBox) map.get(temp)).chatFocus();
    }

    public boolean connect()
    {
        try {
            createSocket("202.51.235.99", 7674);
            return true;
        } catch (IOException ex) {
            Logger.getLogger(Protocol.class.getName()).log(Level.SEVERE, null, ex);
            return false;
        }

    }

    public void send()
    {
        int tStatus = Protocol.SMSG_STATUS_UNDEFINED;
        try{
            out = new BufferedOutputStream(requestSocket.getOutputStream());
            sendMessage();
        }
        catch(IOException ioException){
            ioException.printStackTrace();
        }
    }

    public void createSocket(String IPAddress, int Port) throws IOException{
        requestSocket = new Socket(IPAddress, Port);
        System.out.println("Connected to server");
    }

    public void setUsername(String tInput)
    {
        username = tInput;
    }

    public void sendMessage( ) throws IOException {
        ByteBuffer buf = ByteBuffer.allocate(this.pkt_len);
        buf.put(this.name.getBytes("US-ASCII"));
        buf.putInt(this.version);
        buf.putInt(this.pkt_len);
        buf.putInt(this.service);
        buf.putInt(this.status);
        buf.put(this.data.getBytes("US-ASCII"));
        System.out.println("MP pkt_len " + this.pkt_len);
        System.out.println("buf len " + buf.array().length);
        out.write(buf.array());
        System.out.println("sending packet..!!");
        //printProtocol();
        out.flush();
    }

    public void receiveMessage() throws IOException{
        byte buf[] = new byte[32788];
        System.out.println("receiveMessage, with : " + in.available());
        in.read(buf);
        int k = 0;
        byte tByte[][] = new byte[5][4];
        byte tData[] = new byte[32768];
        for(int i = 0; i < 5; i++)
            for(int j = 0; j < 4; j++){
                tByte[i][j] = buf[k];
                k++;
            }
        setProtocolName(new String(tByte[0]));
        setVersion(byteArrayToInt(tByte[1], 0));
        setPktLen(byteArrayToInt(tByte[2], 0));
        setService(byteArrayToInt(tByte[3], 0));
        setStatus(byteArrayToInt(tByte[4], 0));
        int l = 0;
        
        while(k < tData.length && l < getPktLen()){
            tData[l] = buf[k];
            k++;
            l++;
        }
        setData(new String(tData));
        System.out.println("receive packet..!!");
        //printProtocol();
    }

    //konstruktor
    public Protocol (int pkt_len, int service, int status, String data){
       this.name = "SMSG";
       this.version = 9;
       this.pkt_len = pkt_len;
       this.service = service;
       this.status = status;
       this.data = data;
    }
    public Protocol (){
       this.name = "SMSG";
       this.version = 9;
    }

    //getter setter
    public String getProtocolName() {
        return name;
    }

    public void setProtocolName(String name) {
        this.name = name;
    }

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }

    public int getPktLen() {
        return pkt_len;
    }

    public void setPktLen(int pkt_len) {
        this.pkt_len = pkt_len;
    }

    public int getService() {
        return service;
    }

    public void setService(int service) {
        this.service = service;
    }

    public int getStatus(){
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }

    public String getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
    }

    public static int byteArrayToInt(byte[] b, int offset) {
        int value = 0;
        for (int i = 0; i < 4; i++) {
            int shift = (4 - 1 - i) * 8;
            value += (b[i + offset] & 0x000000FF) << shift;
        }
        return value;
    }

    public void printProtocol(){
        System.out.println("{");
        System.out.println("name: " + this.name);
        System.out.println("version: " + this.version);
        System.out.println("pkt_len: " + this.pkt_len);
        System.out.println("service: " + this.service);
        System.out.println("status: " + this.status);
        System.out.println("data: " + this.data);
        System.out.println("}");
    }

    public void socketClose(){
        //4: Closing connection
        try{
               requestSocket.close();
               out.close();
               in.close();
        }
        catch(IOException ioException){
                ioException.printStackTrace();
        }
    }
}
/*
 * SMSG_SERVICE_LOGON
 * Client:
 * \30username\31password
 * Server:
 * Server akan mengembalikan status 1 kalau login sukses, 0 kalau gagal
 *
 * SMSG_SERVICE_LOGOFF
 * Client:
 * \30username
 *
 * SMSG_MESSAGE
 * Client:
 * \30sender\31receiver\31message
 * Server:
 * Teruskan ke client receiver
 *
 * SMSG_RETFRIEND
 * Client:
 * \30username
 * Server:
 * \30friendname1\31status1\30friendname\31status2
 *
 * SMSG_ADDFRIEND
 * Client:
 * \30username\31friendname
 * Server:
 * Status 1 kalau berhasil, 0 kalau gagal
 *
 * SMSG_REMFRIEND
 * Client:
 * \30username\31friendname
 * Server:
 * Status 1 kalau berhasil, 0 kalau gagal
 *
 * SMSG_STATUS_UPDATE
 * Server:
 * \30username\31status
 *
 * SMSG_REGISTER
 * Client:
 * \30username\31status
 * Server:
 * Status 1 kalau berhasil, 0 kalau gagal
 *
 * SMSG_FILETRANSFER
 * Client:
 * \30targetusername
 * Server
 * Status: SMSG_STATUS_SUCCESS atau SMSG_STATUS_FAILED
 * \30ipaddress
 *
 */