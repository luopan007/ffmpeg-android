//
// Created by luopan on 2020/4/12.
//

#include "IObserver.h"

void IObserver::AddObserver(IObserver *observer) {
    if (!observer) {
        return;
    }
    lock.lock();
    obss.push_back(observer);
    lock.unlock();
}

void IObserver::Notify(XData data) {
    lock.lock();
    for (int i = 0; i < obss.size(); i++) {
        obss[i]->Update(data);
    }
    lock.unlock();
}