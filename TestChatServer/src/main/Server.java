package main;

import java.io.BufferedReader;  
import java.io.IOException;  
import java.io.InputStream;  
import java.io.InputStreamReader;  
import java.io.OutputStream;  
import java.io.PrintWriter;  
import java.net.ServerSocket;  
import java.net.Socket;  

public class Server implements Runnable{
    int max=10;      //最大开启线程数  
    int i=0;         //回复数字  
    int temp;  
    ServerSocket serverSocket;  
    Socket socket[];  
      
    public Server(){  
          
        try {  
            serverSocket=new ServerSocket(6667);    //在5648端口进行侦听  
        } catch (IOException e) {  
            // TODO Auto-generated catch block  
            e.printStackTrace();  
            System.out.println("can't initate ServerSocket!");  
            return;  
        }  
          
        socket=new Socket[max];  
          
        System.out.println("waiting for connect");  
        try {  
            while((socket[i]=serverSocket.accept())!=null){  
                temp=i;  
                i++;  
                new Thread(this).start();           //每侦听到一个客户端的连接，就会开启一个工作线程  
                  
            }  
        } catch (IOException e) {  
            // TODO Auto-generated catch block  
            e.printStackTrace();  
        }  
    }  
    public static void main(String[] args) {  
        new Server();  
  
    }  
  
    @Override  
    public void run() {  
        Socket sk=socket[temp];  
        System.out.println("accept:"+sk.getInetAddress().getHostAddress());  
        InputStream is=null;  
        OutputStream os=null;  
        BufferedReader br=null;  
        PrintWriter pw=null;  
        try {  
            is=sk.getInputStream();  
            br=new BufferedReader(new InputStreamReader(is));  
        } catch (IOException e) {  
            // TODO Auto-generated catch block  
            e.printStackTrace();  
            try {  
                sk.close();  
            } catch (IOException e1) {  
                // TODO Auto-generated catch block  
                e1.printStackTrace();  
            }  
            return;  
        }  
        String str;  
        try {  
            //int m=0;  
            System.out.println("begin read");
            while((!sk.isClosed()) && (str=br.readLine())!=null){  
                System.out.println(str);  
                for(int i = 0; i < socket.length; i ++){
                	if((socket[i] != null) && !socket[i].isClosed()){
	                	os=socket[i].getOutputStream();  
		                pw=new PrintWriter(os);  
		                pw.println(socket[i].getInetAddress().getHostAddress() + ":" + str + "\r\n");  
		                pw.flush();  
                	}
                }  
            }  
        } catch (IOException e) {  
            // TODO Auto-generated catch block  
            e.printStackTrace();  
        }  
          
    }  
}