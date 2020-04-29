//
// Created by luopan on 2020/4/12.
//

#include "IObserver.h"

void IObserver::AddObserver(IObserver *observer) {
    if (!observer) {
        return;
    }
    mux.lock();
    obss.push_back(observer);
    mux.unlock();
}

void IObserver::Notify(XData data) {
    mux.lock();
    for (int i = 0; i < obss.size(); i++) {
        obss[i]->Update(data);
    }
    mux.unlock();
}