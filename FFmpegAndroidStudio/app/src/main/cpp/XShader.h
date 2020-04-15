//
// Created by luopan on 2020/4/14.
//

#ifndef FFMPEG_XSHADER_H
#define FFMPEG_XSHADER_H


class XShader {
public:
    /**
     * 初始化Shader
     *
     * @return 是否初始化成功
     */
    virtual bool Init();

    /**
     * 获取材质并映射到内存
     *
     * @param index
     * @param width
     * @param height
     * @param buf
     */
    virtual void GetTexture(unsigned int index, int width, int height, unsigned char *buf);

    virtual void Draw();

protected:
    /**
     * 顶点着色器 --- vertex Shader
     */
    unsigned int vsh = 0;
    /**
     * 片元着色器 --- fragment Shader
     */
    unsigned int fsh = 0;

    /**
     * 渲染程序 --- Shader program
     */
    unsigned int program = 0;
    unsigned int texts[100] = {0};
};


#endif //FFMPEG_XSHADER_H