//
// Created by luopan on 2020/4/12.
//

#ifndef FFMPEG_XLOG_H
#define FFMPEG_XLOG_H

#include <android/log.h>

class XLog {

};

#define XLOGI(...) __android_log_print(ANDROID_LOG_INFO,"XPlay", __VA_ARGS__);
#define XLOGW(...) __android_log_print(ANDROID_LOG_WARN,"XPlay", __VA_ARGS__);
#define XLOGE(...) __android_log_print(ANDROID_LOG_ERROR,"XPlay", __VA_ARGS__);
#define XLOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"XPlay", __VA_ARGS__);

#endif //FFMPEG_XLOG_H
