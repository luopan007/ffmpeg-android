//
// Created by luopan on 2020/4/14.
//

#include "XTexture.h"
#include "XLog.h"
#include "XEGL.h"
#include "XShader.h"

class CXTexture : public XTexture {
public:

    XShader sh;

    virtual bool Init(void *win) {
        if (!win) {
            XLOGW("XTexture Init failed.");
            return false;
        }
        if (!XEGL::Get()->Init(win)) {
            XLOGI("XEGL Init failed.");
            return false;
        };
        sh.Init();
        return true;
    }

    virtual void Draw(unsigned char *data[], int width, int height) {
        sh.GetTexture(0, width, height, data[0]);          // Y
        sh.GetTexture(1, width / 2, height / 2, data[1]);  // U
        sh.GetTexture(2, width / 2, height / 2, data[2]);  // V
        sh.Draw();
        XEGL::Get()->Draw();
    }
};

XTexture *XTexture::Create() {
    return new CXTexture();
}
