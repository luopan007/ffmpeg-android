package com.luopan.ffmpeg;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import androidx.appcompat.app.AppCompatActivity;

public class OpenUrl extends AppCompatActivity {
    private Button btfile;
    private Button btrtmp;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_open_url);
        btfile = findViewById(R.id.playvideo);
        btfile.setOnClickListener(
                (View view) -> {
                    EditText t = findViewById(R.id.fileurl);
                    Open(t.getText().toString());
                    finish();
                });
        btrtmp = findViewById(R.id.playrtmp);
        btrtmp.setOnClickListener(
                (View view) -> {
                    EditText t = findViewById(R.id.rtmpurl);
                    Open(t.getText().toString());
                    finish();
                });
    }

    /**
     * 打开播放传递地址
     *
     * @param url
     */
    public native void Open(String url);
}
