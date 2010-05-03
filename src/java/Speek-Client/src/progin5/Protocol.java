package progin5;

import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;

/**
 *
 * @author Ananti
 */
public class Protocol {
    //constant service
    static final int SMSG_SERVICE_LOGON     = 1;
    static final int SMSG_SERVICE_LOGOFF    = 2;
    static final int SMSG_MESSAGE           = 3;
    static final int SMSG_FILETRANSFER      = 4;
    static final int SMSG_ADDFRIEND         = 5;
    static final int SMSG_REMFRIEND         = 6;
    static final int SMSG_RETFRIEND         = 7;
    static final int SMSG_STATUSUPDATE      = 8;
    static final int SMSG_REGISTER          = 9;

    //constant status
    static final int SMSG_STATUS_FAILED     = 0;
    static final int SMSG_STATUS_SUCCESS    = 1;
    static final int SMSG_STATUS_ONLINE     = 2;
    static final int SMSG_STATUS_OFFLINE    = 3;

    private String name;
    private int version;
    private int pkt_len;
    private int service;
    private int status;
    private String data;


    //socket
    public Socket requestSocket;
    public ObjectOutputStream out;
    public ObjectInputStream in;
    public String message;

    void run(Protocol MyProtocol)
	{
		try{
			//1. creating a socket to connect to the server
			requestSocket = new Socket( "202.51.235.99", 7674);
			System.out.println("Connected to server");
			//2. get Input and Output streams
			//out = new ObjectOutputStream(requestSocket.getOutputStream());
			//in = new ObjectInputStream(requestSocket.getInputStream());
			//3: Communicating with the server
			
				//try{
					//message = (String)in.readObject();
					//System.out.println("server>" + message);
                                        System.out.println("coba " + MyProtocol.name);
					sendMessage(MyProtocol);
					//message = "bye";
					//sendMessage(MyProtocol);
				//}
				//catch(ClassNotFoundException classNot){
				//	System.err.println("data received in unknown format");
				//}
			
		}
		catch(UnknownHostException unknownHost){
			System.err.println("You are trying to connect to an unknown host!");
		}
		catch(IOException ioException){
			ioException.printStackTrace();
		}
		finally{
			//4: Closing connection
			try{
				//in.close();
				//out.close();
				requestSocket.close();
			}
			catch(IOException ioException){
				ioException.printStackTrace();
			}
		}
	}

    public void sendMessage(Protocol MyProtocol) throws IOException {
        out = new ObjectOutputStream(requestSocket.getOutputStream());
        ByteBuffer buf = ByteBuffer.allocate(MyProtocol.pkt_len);
        buf.put(MyProtocol.name.getBytes("US-ASCII"));
        System.out.println("s=" + buf.array().length);
        buf.putInt(MyProtocol.version);
        buf.putInt(MyProtocol.pkt_len);
        buf.putInt(MyProtocol.service);
        buf.putInt(MyProtocol.status);
        buf.put(MyProtocol.data.getBytes("US-ASCII"));
        System.out.println("MP pkt_len " + MyProtocol.pkt_len);
        System.out.println("buf len " + buf.array().length);
        out.write(buf.array());
        System.out.println("buf=[" + buf.array() + "]");
        out.close();
    }


    //konstruktor
    public Protocol (String name, int version, int pkt_len, int service, int status, String data){
       this.name = name;
       this.version = version;
       this.pkt_len = pkt_len;
       this.service = service;
       this.status = status;
       this.data = data;
    }

    //getter setter
    public String getName() {
        return name;
    }

    public void setName(String name) {
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
}
