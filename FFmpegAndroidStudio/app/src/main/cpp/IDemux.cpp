//
// Created by luopan on 2020/4/12.
//

#include "IDemux.h"
#include "XLog.h"

const static int DEMUX_SLEEP_TIME = 2;

void IDemux::Main() {
    while (!isExit) {
        if (IsPause()) {
            XSleep(DEMUX_SLEEP_TIME);
            continue;
        }
        XData data = Read();
        if (data.size > 0) {
            Notify(data);
        } else {
            XSleep(DEMUX_SLEEP_TIME);
        }
    }
}