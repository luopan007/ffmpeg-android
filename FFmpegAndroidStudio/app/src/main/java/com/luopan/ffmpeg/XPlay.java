package com.luopan.ffmpeg;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.View;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/** 视频播放呈现类 */
public class XPlay extends GLSurfaceView
        implements SurfaceHolder.Callback, GLSurfaceView.Renderer, View.OnClickListener {
    private static final String TAG = "XPlay";

    public XPlay(Context context) {
        // Java new Go this
        super(context);
    }

    public XPlay(Context context, AttributeSet attrs) {
        // XML Go This
        super(context, attrs);
        setRenderer(this);
        setOnClickListener(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // 初始化opengl egl 显示
        InitView(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {}

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {}

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {}

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {}

    @Override
    public void onDrawFrame(GL10 gl) {}

    @Override
    public void onClick(View v) {
        PlayOrPause();
    }
    /**
     * 初始化窗口；将Java层Surface传递给Native层
     *
     * @param surface 窗口
     */
    public native void InitView(Object surface);

    /** 暂停播放 */
    public native void PlayOrPause();
}
