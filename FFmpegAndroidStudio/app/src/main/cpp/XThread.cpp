//
// Created by luopan on 2020/4/12.
//

#include "XThread.h"
#include "XLog.h"

#include <thread>

const int MAX_WAIT_STOP_TIME = 200;

const int SINGLE_WAIT_STOP_TIME = 1;

using namespace std;

void XSleep(int mis) {
    chrono::milliseconds du(mis);
    this_thread::sleep_for(du);
}

void XThread::Start() {
    isExit = false;
    thread th(&XThread::ThreadMain, this);
    th.detach();
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

void XThread::Main() {
}