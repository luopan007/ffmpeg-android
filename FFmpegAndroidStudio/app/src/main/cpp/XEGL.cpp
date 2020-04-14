//
// Created by luopan on 2020/4/14.
//

#include <android/native_window.h>
#include <EGL/egl.h>
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

    virtual bool Init(void *win) {
        // 1. 获取EGLDisplay对象---显示设备
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            XLOGW("eglGetDisplay failed");
            return false;
        }

        // 2. 初始化Display
        if (EGL_TRUE != eglInitialize(display, 0, 0)) {
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
            XLOGW("eglChooseConfig failed");
            return false;
        }

        // 4. 创建surface----实际上是关联显示设备和数据窗口
        ANativeWindow *nativeWindow = static_cast<ANativeWindow *>(win);
        surface = eglCreateWindowSurface(display, config, nativeWindow, NULL);

        // 5. 创建上下文----EGL_NONE表示数组结束
        const EGLint ctxAttr[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
        };
        EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);
        if (context == EGL_NO_CONTEXT) {
            XLOGW("eglCreateContext failed");
            return false;
        }

        // 6. 打开上下文---关联显示设备和显示窗口
        if (EGL_TRUE != eglMakeCurrent(display, surface, surface, context)) {
            XLOGW("eglMakeCurrent failed");
            return false;
        }
        XLOGI("XEGL Init success.");
        return true;
    }
};

XEGL *XEGL::Get() {
    static CXEGL egl;
    return &egl;
}