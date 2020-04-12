//
// Created by luopan on 2020/4/12.
//

#include "IDemux.h"
#include "XLog.h"

void IDemux::Main() {
    while (!isExit) {
        XData data = Read();
        if (data.size > 0) {
            Notify(data);
        }
    }
}