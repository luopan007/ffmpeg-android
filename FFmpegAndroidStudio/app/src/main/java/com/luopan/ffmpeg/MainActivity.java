package com.luopan.ffmpeg;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.SeekBar;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

public class MainActivity extends AppCompatActivity
        implements Runnable, SeekBar.OnSeekBarChangeListener {
    private static final String TAG = "XPlay";

    private static final int REQUEST_EXTERNAL_STORAGE = 1;

    private static String[] PERMISSIONS_STORAGE = {
        Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE,
    };

    static {
        System.loadLibrary("native-lib");
        Log.i(TAG, "loadLibrary success");
    }

    private SeekBar seek;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(TAG, "onCreate IN");
        // 去掉标题栏
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
        // 全屏播放
        getWindow()
                .setFlags(
                        WindowManager.LayoutParams.FLAG_FULLSCREEN,
                        WindowManager.LayoutParams.FLAG_FULLSCREEN);
        // 横屏播放
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        setContentView(R.layout.activity_main);
        verifyStoragePermissions(this);

        Button bt = findViewById(R.id.open_button);
        seek = findViewById(R.id.aplayseek);
        seek.setMax(1000);
        seek.setOnSeekBarChangeListener(this);

        bt.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        Log.i(TAG, "open button click!");
                        // 打开选择路径窗口
                        Intent intent = new Intent();
                        intent.setClass(MainActivity.this, OpenUrl.class);
                        startActivity(intent);
                    }
                });

        // 启动播放进度线程
        Thread th = new Thread(this);
        th.start();
        Log.i(TAG, "onCreate thread name:" + Thread.currentThread().getName());
    }

    @Override
    public void run() {
        Log.i(TAG, "run thread name:" + Thread.currentThread().getName());
        for (; ; ) {
            seek.setProgress((int) (GetPlayPosition() * 1000));
            try {
                Thread.sleep(40);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {}

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {}

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        SeekTo((double) seekBar.getProgress() / (double) seekBar.getMax());
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "onResume IN");
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.i(TAG, "onPause IN");
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        Log.i(TAG, "onRestart IN");
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.i(TAG, "onStart IN");
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.i(TAG, "onStop IN");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "onDestroy IN");
    }

    /**
     * Android 6.0以后，需要使用动态申请权限
     *
     * @param activity 当前Activity的对象
     */
    private void verifyStoragePermissions(Activity activity) {
        int permission =
                ActivityCompat.checkSelfPermission(
                        activity, Manifest.permission.ACCESS_FINE_LOCATION);
        if (permission != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    activity, PERMISSIONS_STORAGE, REQUEST_EXTERNAL_STORAGE);
        }
    }

    public native double GetPlayPosition();

    public native void SeekTo(double position);
}
