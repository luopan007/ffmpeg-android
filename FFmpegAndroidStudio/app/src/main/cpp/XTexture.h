//
// Created by luopan on 2020/4/14.
//

#ifndef FFMPEG_XTEXTURE_H
#define FFMPEG_XTEXTURE_H


class XTexture {
public:
    static XTexture *Create();

    /**
     * 初始化Texture---纹理
     *
     * @param win 窗口
     * @return 是否初始化成功
     */
    virtual bool Init(void *win) = 0;

    /**
     * 绘制数据
     *
     * @param data
     * @param width
     * @param height
     */
    virtual void Draw(unsigned char *data[], int width, int height) = 0;
};


#endif //FFMPEG_XTEXTURE_H