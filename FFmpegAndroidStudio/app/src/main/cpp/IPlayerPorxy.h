//
// Created by luopan on 2020/4/28.
//

#ifndef FFMPEG_IPLAYERPORXY_H
#define FFMPEG_IPLAYERPORXY_H

#include "IPlayer.h"
#include <mutex>

class IPlayerPorxy : public IPlayer {
public:
    static IPlayerPorxy *Get() {
        static IPlayerPorxy px;
        return &px;
    }

    void Init(void *vm = 0);

    virtual bool Open(const char *path);

    virtual bool Seek(double pos);

    virtual void Close();

    virtual bool Start();

    virtual void InitView(void *win);

    virtual void SetPause(bool isP);

    virtual bool IsPause();

    virtual double PlayPos();

protected:
    IPlayerPorxy() {}

    IPlayer *player = 0;
    std::mutex mux;
};


#endif //FFMPEG_IPLAYERPORXY_H
