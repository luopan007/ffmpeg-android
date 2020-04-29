//
// Created by luopan on 2020/4/14.
//

#include <android/native_window.h>
#include <EGL/egl.h>
#include <mutex>
#include "XEGL.h"
#include "XLog.h"

const int BIT_WIDTH = 8;

class CXEGL : public XEGL {
public:
    /**
     * 记录display成员值----EGL_NO_DISPLAY可以作为显示设备是否有效判断
     */
    EGLDisplay display = EGL_NO_DISPLAY;

    /**
     * 记录surface成员值----EGL_NO_SURFACE可以作为显示窗口是否有效判断
     */
    EGLSurface surface = EGL_NO_SURFACE;

    /**
     * 记录context成员值----EGL_NO_CONTEXT可以作为上下文是否有效判断
     */
    EGLContext context = EGL_NO_CONTEXT;
    std::mutex mux;

    virtual void Draw() {
        mux.lock();
        if (display == EGL_NO_DISPLAY || surface == EGL_NO_SURFACE) {
            mux.unlock();
            return;
        }
        eglSwapBuffers(display, surface);
        mux.unlock();
    }

    virtual void Close() {
        mux.lock();
        if (display == EGL_NO_DISPLAY) {
            mux.unlock();
            return;
        }
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (surface != EGL_NO_SURFACE) {
            eglDestroySurface(display, surface);
        }
        if (context != EGL_NO_CONTEXT) {
            eglDestroyContext(display, context);
        }

        eglTerminate(display);

        display = EGL_NO_DISPLAY;
        surface = EGL_NO_SURFACE;
        context = EGL_NO_CONTEXT;
        mux.unlock();
    }

    virtual bool Init(void *win) {
        ANativeWindow *nwin = (ANativeWindow *) win;
        Close();

        //初始化EGL
        mux.lock();
        // 1. 获取EGLDisplay对象---显示设备
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            mux.unlock();
            XLOGW("eglGetDisplay failed");
            return false;
        }
        XLOGI("eglGetDisplay success!");

        // 2. 初始化Display
        if (EGL_TRUE != eglInitialize(display, 0, 0)) {
            mux.unlock();
            XLOGW("eglInitialize failed");
            return false;
        }

        // 3. 获取配置
        EGLint configSpec[] = {
                EGL_RED_SIZE, BIT_WIDTH,
                EGL_GREEN_SIZE, BIT_WIDTH,
                EGL_BLUE_SIZE, BIT_WIDTH,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
        };
        EGLConfig config = 0;
        EGLint configNum = 0;
        if (EGL_TRUE != eglChooseConfig(display, configSpec, &config, 1, &configNum)) {
            mux.unlock();
            XLOGW("eglChooseConfig failed");
            return false;
        }
        XLOGI("eglChooseConfig success!");

        // 4. 创建surface----实际上是关联显示设备和数据窗口
        surface = eglCreateWindowSurface(display, config, nwin, NULL);

        // 5. 创建上下文----EGL_NONE表示数组结束
        const EGLint ctxAttr[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
        };
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);
        if (context == EGL_NO_CONTEXT) {
            mux.unlock();
            XLOGW("eglCreateContext failed");
            return false;
        }
        XLOGI("eglCreateContext success!");

        // 6. 打开上下文---关联显示设备和显示窗口
        if (EGL_TRUE != eglMakeCurrent(display, surface, surface, context)) {
            mux.unlock();
            XLOGW("eglMakeCurrent failed");
            return false;
        }
        
        XLOGI("eglMakeCurrent success!");
        mux.unlock();
        return true;
    }
};

XEGL *XEGL::Get() {
    static CXEGL egl;
    return &egl;
}