//
// Created by luopan on 2020/4/12.
//

#include "IDemux.h"
#include "XLog.h"

void IDemux::Main() {
    while (!isExit) {
        XData data = Read();
//        XLOGI("data size %d", data.size);
    }
}