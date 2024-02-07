package com.example.fortest;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import java.io.File;
import java.util.Objects;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    private static final int REQUEST_EXTERNAL_STORAGE = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Log.i("ycj", "MainActivity on create");
//        JniHelper.KPL_initialize("mode_123", new StationParameters(), 10, 10);
        JniHelper.SDK_init();
        Log.i("ycj",  getFilesDir().getPath());

        // 如果获取写权限成功，则将配置文件拷贝到App目录。
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M &&
                ContextCompat.checkSelfPermission(this, android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
                        != PackageManager.PERMISSION_GRANTED) {
            Log.i(TAG, "no permission to read");
            // Request the permission
            ActivityCompat.requestPermissions(this,
                    new String[]{android.Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    REQUEST_EXTERNAL_STORAGE);
        } else {
            // Permission already granted, proceed with copying assets
            copyAssets();
        }
        @SuppressLint({"MissingInflatedId", "LocalSuppress"}) Button tcpButton = findViewById(R.id.bt1);
        @SuppressLint({"MissingInflatedId", "LocalSuppress"}) Button ntrip1Button = findViewById(R.id.bt2);
        @SuppressLint({"MissingInflatedId", "LocalSuppress"}) Button ntrip2Button = findViewById(R.id.bt3);
        tcpButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(MainActivity.this, TcpActivity.class);
                startActivity(intent);
            }
        });
        ntrip1Button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(MainActivity.this, NtripActivity.class);
                startActivity(intent);
            }
        });

    }

    private void copyAssets() {
        String path = Objects.requireNonNull(getExternalFilesDir(null)).getPath();
        FileUtils.copyAssetsToStorage(this, "configures", path);
        JniHelper.sendLoadStorage(path);
        new TcpActivity.TcpClientTask().executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        new NtripActivity.NtripConnectTaskSsr(this).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        new NtripActivity.NtripConnectTask(this).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_EXTERNAL_STORAGE) {
            // Check if the permission is granted
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                // Permission granted, proceed with copying assets
                copyAssets();
            } else {
                // Permission denied, handle accordingly
                Log.e(TAG, "Write external storage permission denied");
            }
        }
    }
}