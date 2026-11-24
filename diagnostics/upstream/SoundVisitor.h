#pragma once

#include "TestSupport.h"
#include "../../Main/wInterfaceVisitors.h"

namespace UpstreamTests
{
struct ObjectStageAudio: NWorld::ISoundVisitor
{
    vector<NDb::CSound*> sounds;
    vector<NDb::CSoundEffect*> effects;
    vector<STime> soundTimes,effectTimes;
    vector<CObj<CFuncBase<CVec3>>> positions;
    void Add3DSound(STime time,NDb::CSound* sound,CFuncBase<CVec3>* position) override { sounds.push_back(sound);soundTimes.push_back(time);positions.push_back(position); }
    void AddEffect(STime time,NDb::CSoundEffect* effect,CFuncBase<CVec3>* position,const vector<int>&) override { effects.push_back(effect);effectTimes.push_back(time);positions.push_back(position); }
    void Add2DSound(STime,NDb::CSound*) override { throw std::runtime_error("object stage emitted a 2D sound"); }
};
}
