package com.example.fortest;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.BufferedReader;
import java.util.Objects;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

public class FileUtils {
    private static final String TAG = "FileUtils";
    public static void writeToFile(Context context, String fileName, String data) {
        try {
            FileOutputStream fos = context.openFileOutput(fileName, Context.MODE_PRIVATE);
            OutputStreamWriter osw = new OutputStreamWriter(fos);
            osw.write(data);
            osw.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    public static void writeByteArrayToFile(Context context, String fileName, byte[] data) {
        try {
            String dirname = Objects.requireNonNull(context.getExternalFilesDir(null)).getPath();
//            FileOutputStream fos = context.openFileOutput(dirname+File.separator+fileName,  Context.MODE_APPEND);
            FileOutputStream fos = new FileOutputStream(dirname+File.separator+fileName,  true);
            fos.write(data);
            fos.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    public static void copyAssetsToStorage(Context context, String assetsPath, String storagePath) {
        Log.i(TAG, "ycj copyAssetsToStorage");
        AssetManager assetManager = context.getAssets();

        try {
            String[] files = assetManager.list(assetsPath);

            if (files != null) {
                for (String filename : files) {
                    Log.i(TAG, " filename = " + filename);
                    String assetPath = assetsPath + File.separator + filename;
                    String storageFilePath = storagePath + File.separator ;
                    Log.i(TAG, "assetPath = " + assetPath + " storageFilePath" + storageFilePath);
                    if (assetManager.list(assetPath).length == 0) {
                        // It's a file
                        copyAssetFile(context, assetPath, storageFilePath,filename);
                    } else {
                        // It's a directory
                        File directory = new File(storageFilePath+filename);
                        if (!directory.exists()) {
                            Log.i(TAG, "Creating directory: " + directory.getPath());
                            if (!directory.mkdirs()) {
                                Log.e(TAG, "Failed to create directory: " + directory.getPath());
                                return;
                            }
                        }
                        copyAssetsToStorage(context, assetPath, storageFilePath+filename);
                    }
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "Error copying assets", e);
        }
    }

    private static void copyAssetFile(Context context, String assetFilePath, String storageFilePath, String fname) {
        Pattern p_fname = Pattern.compile("[^\"]+\\.json$");
        if (p_fname.matcher(fname).matches()){
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(context.getAssets().open(assetFilePath)));
                 OutputStreamWriter writer = new OutputStreamWriter(new FileOutputStream(storageFilePath+fname))) {
                String buffer ;
                Pattern p_jsonfname = Pattern.compile(".+([^\"]+\\.json).+");
                while ((buffer = reader.readLine()) != null) {
                    if (p_jsonfname.matcher(buffer).matches()){
                        buffer = Pattern.compile("([^\"]+\\.json)").matcher(buffer).replaceAll(storageFilePath+"$1");
                        writer.write(buffer + "\n");
                    }else{
                        writer.write(buffer+"\n");
                    }
                }
                Log.d(TAG, "Copied " + assetFilePath + " to " + storageFilePath);
            } catch (IOException e) {
                Log.e(TAG, "Error copying asset file: " + assetFilePath, e);
            }
        }else{
            try (InputStream in = context.getAssets().open(assetFilePath);
                 OutputStream out = new FileOutputStream(storageFilePath+fname)) {
                byte[] buffer = new byte[1024];
                int read;
                while ((read = in.read(buffer)) != -1) {
                    out.write(buffer, 0, read);
                }
                Log.d(TAG, "Copied " + assetFilePath + " to " + storageFilePath);
            } catch (IOException e) {
                Log.e(TAG, "Error copying asset file: " + assetFilePath, e);
            }
        }
    }
}

