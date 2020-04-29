//
// Created by luopan on 2020/4/12.
//

#include "XThread.h"
#include "XLog.h"

#include <thread>

const int MAX_WAIT_STOP_TIME = 200;

const int SINGLE_WAIT_STOP_TIME = 1;

const int PAUSE_WAIT_STOP_TIME = 10;

using namespace std;

void XSleep(int mis) {
    chrono::milliseconds du(mis);
    this_thread::sleep_for(du);
}

void XThread::SetPause(bool isP) {
    isPause = isP;
    // 等待PAUSE_WAIT_STOP_TIME * PAUSE_WAIT_STOP_TIME毫秒
    for (int i = 0; i < PAUSE_WAIT_STOP_TIME; i++) {
        if (isPausing == isP) {
            break;
        }
        XSleep(PAUSE_WAIT_STOP_TIME);
    }
}

bool XThread::Start() {
    isExit = false;
    isPause = false;
    thread th(&XThread::ThreadMain, this);
    th.detach();
    return true;
}

void XThread::ThreadMain() {
    isRunning = true;
    XLOGI("ThreadMain come in.");
    Main();
    XLOGI("ThreadMain come out.");
    isRunning = false;
}

void XThread::Stop() {
    isExit = true;
    for (int i = 0; i < MAX_WAIT_STOP_TIME; i++) {
        if (!isRunning) {
            XLOGI("Stop XThread Success");
            return;
        }
        XSleep(SINGLE_WAIT_STOP_TIME);
    }
    XLOGI("Stop XThread time out");
}