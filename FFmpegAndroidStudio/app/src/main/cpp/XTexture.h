//
// Created by luopan on 2020/4/14.
//

#ifndef FFMPEG_XTEXTURE_H
#define FFMPEG_XTEXTURE_H

enum XTextureType {
    XTEXTURE_YUV420P = 0,  // Y 4  u 1 v 1
    XTEXTURE_NV12 = 25,    // Y4   uv1
    XTEXTURE_NV21 = 26     // Y4   vu1

};

class XTexture {
public:
    static XTexture *Create();

    /**
     * 初始化Texture---纹理
     *
     * @param win 窗口
     * @return 是否初始化成功
     */
    virtual bool Init(void *win, XTextureType type = XTEXTURE_YUV420P) = 0;

    /**
     * 绘制数据
     *
     * @param data
     * @param width
     * @param height
     */
    virtual void Draw(unsigned char *data[], int width, int height) = 0;

    virtual void Drop() = 0;

    virtual ~XTexture() {};
protected:
    XTexture() {};
};


#endif //FFMPEG_XTEXTURE_H