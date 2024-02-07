package com.example.fortest;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import androidx.appcompat.app.AppCompatActivity;
import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import android.os.Bundle;
import android.util.Base64;
import android.util.Log;
import androidx.appcompat.app.AppCompatActivity;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.Arrays;

public class NtripActivity extends AppCompatActivity {

    private static final String TAG = "NtripActivity";

    Context c = this;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ntrip);
        @SuppressLint({"MissingInflatedId", "LocalSuppress"}) Button ntripButton = findViewById(R.id.btnConnectNtrip);
        ntripButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // 在 onCreate 方法中执行异步任务
                Log.i(TAG, "建立ntrip");
                new NtripConnectTask(c).execute();
            }
        });
    }

    public static class NtripConnectTask extends AsyncTask<Void, Void, Void> {
        private Context context = null;
        private static final String NTRIP_SERVER_IP = "119.96.165.202";
        private static final int NTRIP_SERVER_PORT = 8600;
        private Socket socket;
        private  boolean isRunning = false; // 控制循环的标志
        private static final String MOUNTPOINT = "TEST";
        private static final String USERNAME = "test";
        private static final String PASSWORD = "test";

        NtripConnectTask(Context context){
            this.context = context;
        }
        @Override
        protected Void doInBackground(Void... voids) {
            isRunning = true;
            try {
                // 创建Socket连接
                socket = new Socket(NTRIP_SERVER_IP, NTRIP_SERVER_PORT);
                // 获取输入输出流
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                InputStream inputStream = socket.getInputStream();

                // 发送连接请求，包括挂载点和账号密码
                sendConnectRequest(out, MOUNTPOINT, USERNAME, PASSWORD);

                // 循环读取服务器发送的数据
                while (isRunning) {
                    String response = in.readLine();
                    if (response == null) {
                        // 服务器关闭连接或发生其他错误
                        break;
                    }
                    // 处理从服务器获取的数据，例如更新UI或执行其他操作
                    Log.d(TAG, "Received Obs data from Ntrip server: " + response);
                    handleReceivedData(inputStream);
                }
                socket.close();
            } catch (IOException e) {
                Log.e(TAG, "Error: " + e.getMessage());
            }
            return null;
        }

        private void handleReceivedData(InputStream inputStream) throws IOException {
            byte[] buffer = new byte[1024];
            int bytesRead = inputStream.read(buffer);
            byte[] newArray = Arrays.copyOfRange(buffer, 0, bytesRead+1);
            FileUtils.writeByteArrayToFile(context,"obs.rtcm",newArray);
            if (bytesRead == -1) {
                // 服务器关闭连接或发生其他错误
                return;
            }
            // 处理从服务器获取的数据，例如更新UI或执行其他操作
            byte[] receivedBytes = new byte[bytesRead];
            System.arraycopy(buffer, 0, receivedBytes, 0, bytesRead);
            for (byte b : receivedBytes) {
//                Log.i(TAG, " byte " + String.format("%02X", b));
                System.out.print(String.format("%02X ", b));
            }
            System.out.println();
            JniHelper.sendObsData(receivedBytes);
        }

        private void sendConnectRequest(PrintWriter out, String mountpoint, String username, String password) {
            StringBuilder requestBuilder = new StringBuilder();
            requestBuilder.append("GET /").append(mountpoint).append(" HTTP/1.0\r\n");
            requestBuilder.append("User-Agent: NTRIP NTRIPClient/1.0\r\n");
            // 添加账号密码信息
            if (!username.isEmpty()) {
                String authHeader = "Authorization: Basic " + Base64.encodeToString((username + ":" + password).getBytes(), Base64.NO_WRAP);
                requestBuilder.append(authHeader).append("\r\n");
            }
            requestBuilder.append("\r\n");

            // 发送请求
            out.println(requestBuilder.toString());
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            isRunning = false; // 结束循环
        }
    }
    public static class NtripConnectTaskSsr extends AsyncTask<Void, Void, Void> {
        //域名: ssr.kplgnss.com (推荐使用), IP: 103.143.19.54
        //端口: 8060
        //源节点: SSRKPL0CLK
        private static final String NTRIP_SERVER_IP = "119.96.237.211";
        private static final int NTRIP_SERVER_PORT = 8005;
        private Socket socket;
        private  boolean isRunning = false; // 控制循环的标志
        // "test:test@119.96.223.176:8007/SSR_COM_BAK"
        private static final String MOUNTPOINT = "SSR_COM";
        private static final String USERNAME = "test";
        private static final String PASSWORD = "test";

        private static Context context;
        NtripConnectTaskSsr(Context context){
            this.context = context;
        }
        @Override
        protected Void doInBackground(Void... voids) {
            isRunning = true;
            try {
                // 创建Socket连接
                socket = new Socket(NTRIP_SERVER_IP, NTRIP_SERVER_PORT);
                // 获取输入输出流
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                InputStream inputStream = socket.getInputStream();

                // 发送连接请求，包括挂载点和账号密码
                sendConnectRequest(out, MOUNTPOINT, USERNAME, PASSWORD);

                // 循环读取服务器发送的数据
                while (isRunning) {
                    String response = in.readLine();
                    if (response == null) {
                        // 服务器关闭连接或发生其他错误
                        break;
                    }
                    // 处理从服务器获取的数据，例如更新UI或执行其他操作
                    Log.d(TAG, "Received Ssr data from Ntrip server: " + response);
                    handleReceivedData(inputStream);
                }
                // 关闭连接
                socket.close();
            } catch (IOException e) {
                Log.e(TAG, "Error: " + e.getMessage());
            }
            return null;
        }

        private void handleReceivedData(InputStream inputStream) throws IOException {
            byte[] buffer = new byte[1024];
            int bytesRead = inputStream.read(buffer);
            if (bytesRead == -1) {
                // 服务器关闭连接或发生其他错误
                return;
            }

            // 处理从服务器获取的数据，例如更新UI或执行其他操作
            byte[] receivedBytes = new byte[bytesRead];
            System.arraycopy(buffer, 0, receivedBytes, 0, bytesRead);
            for (byte b : receivedBytes) {
//                Log.i(TAG, " byte " + String.format("%02X", b));
                System.out.print(String.format("%02X ", b));
            }
            System.out.println();
            JniHelper.sendSsrData(receivedBytes);
            String logstr = JniHelper.SDKRetrieve("SSR-ORBCLK",  0);
            for (String line : logstr.split("\n")){
                Log.i("ssrlog", line);
            }
            logstr = JniHelper.SDKRetrieve("SSR-IFPB",  0);
            for (String line : logstr.split("\n")){
                Log.i("ssrlog", line);
            }
            logstr = JniHelper.SDKRetrieve("SSR-CODEBIAS",  0);
            for (String line : logstr.split("\n")){
                Log.i("ssrlog", line);
            }
            logstr = JniHelper.SDKRetrieve("SSR-ALL",  0);
            for (String line : logstr.split("\n")){
                Log.i("ssrlog", line);
            }

        }
        private void sendConnectRequest(PrintWriter out, String mountpoint, String username, String password) {
            StringBuilder requestBuilder = new StringBuilder();
            requestBuilder.append("GET /").append(mountpoint).append(" HTTP/1.0\r\n");
            requestBuilder.append("User-Agent: NTRIP NTRIPClient/1.0\r\n");

            // 添加账号密码信息
            if (!username.isEmpty()) {
                String authHeader = "Authorization: Basic " + Base64.encodeToString((username + ":" + password).getBytes(), Base64.NO_WRAP);
                requestBuilder.append(authHeader).append("\r\n");
            }
            requestBuilder.append("\r\n");

            // 发送请求
            out.println(requestBuilder.toString());
        }
        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            isRunning = false; // 结束循环
        }
    }
}