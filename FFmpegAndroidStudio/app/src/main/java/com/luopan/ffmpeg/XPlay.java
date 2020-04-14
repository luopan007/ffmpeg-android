package com.luopan.ffmpeg;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;

/** 视频播放呈现类 */
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
        // 1、打开视频，显示RGB
        open("/sdcard/1080.mp4", getHolder().getSurface());
        InitView(getHolder().getSurface());
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        new Thread(this).start(); // SurfaceView创建成功
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {}

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {}

    /**
     * 初始化窗口；将Java层Surface传递给Native层
     *
     * @param surface 窗口
     */
    public native void InitView(Object surface);

    /**
     * 打开视频，显示RGB
     *
     * @param url 视频地址
     * @param surface 显示对象
     */
    public native void open(String url, Object surface);
}
