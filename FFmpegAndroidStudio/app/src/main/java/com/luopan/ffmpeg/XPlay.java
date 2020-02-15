package com.luopan.ffmpeg;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;

/**
 * 视频播放呈现类
 */
public class XPlay extends GLSurfaceView implements Runnable, Callback {
    private static final String TAG = "XPlay";

    public XPlay(Context context) {
        super(context);
    }

    public XPlay(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void run() {
        // 1、打开视频
        open("/sdcard/DCIM/test.mp4", getHolder().getSurface());
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        new Thread(this).start(); // SurfaceView创建成功
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
    }

    /**
     * 打开视频
     *
     * @param url     视频地址
     * @param surface 显示对象
     */
    public native void open(String url, Object surface);

}