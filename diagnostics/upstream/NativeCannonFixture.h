#pragma once

#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wUnitCommands.h"
#include "../../Main/rpgGlobal.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/wObject.h"
#include "../../Main/wUnitAttackExec.h"
#include "../../DBFormat/DataAnimation.h"
#include "../../DBFormat/DataFormat.h"
#include "../../DBFormat/DataRPG.h"

namespace UpstreamTests
{
struct NativeCannonFixture
{
    CObj<NRPG::CGlobalGame> game;
    CObj<NWorld::CWorld> world;
    CPtr<NWorld::CUnitServer> actor;
    CPtr<NWorld::CCannon> cannon;
    CDBPtr<NDb::CAnimation> entryAnimation;
    vector<int> entryClipIDs;
    int personalFlags=0;
};

inline NativeCannonFixture MakeNativeCannonFixture(const char* databasePath,const char* resourcePath)
{
    NativeCannonFixture fixture;fixture.world=FixtureWorld(databasePath,resourcePath,true,&fixture.game);
    auto* world=fixture.world.GetPtr();auto* network=world->GetPathNetwork();world->GetTime()->Set(1000);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
    vector<NWorld::CCannon*> cannons;world->GetCannons(&cannons);
    for(auto entry:units) {
        auto* actor=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(!actor || !actor->CanFight() || actor->IsWearingPK() || actor->IsEmptyPK() || actor->GetUnitRPG()->GetCannonItem())continue;
        auto* personal=dynamic_cast<NRPG::CWeaponItem*>(actor->GetUnitRPG()->GetInventory()->GetActive());
        if(!personal)continue;
        const auto original=actor->GetPosition();
        for(auto* cannon:cannons) {
            if(!cannon || cannon->IsBroken() || cannon->IsOccupied() || !cannon->GetSkeletonAnimator())continue;
            auto* weapon=dynamic_cast<NRPG::CWeaponItem*>(cannon->GetItem());
            if(!weapon || weapon->GetDBWeapon()->szAnimName.empty())continue;
            vector<NAI::SPathPlace> places;cannon->GetApproaches(&places,network);
            for(auto place:places) {
                place.SetPose(NAI::CM_STAND);
                if(!network->IsPassable(place) || network->IsLocked(place))continue;
                auto position=original;position.pos.p=place;position.bRun=false;actor->SetPosition(position);
                actor->animator.AlignTime();actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
                actor->animator.SetActiveItem(true);actor->animator.SetWeaponName(personal->GetDBWeapon()->szAnimName.c_str());
                actor->animator.PlaceUnit(position);
                CDBPtr<NDb::CSkeleton> skeleton;CDBPtr<NDb::CSide> side;int flags=0,classFlags=0;
                CMemoryStream fields;CStructureNetworkContext refs;refs.include=[](CObjectBase*){return false;};
                {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);actor->animator.operator&(writer);}
                fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(4,&flags);reader.Add(5,&classFlags);reader.Add(31,&skeleton);reader.Add(40,&side);}
                auto* expected=skeleton?skeleton->GetAnimation(NDb::CAnimation::START_ATTACK,NDb::CAnimation::POSE_STAND,
                    weapon->GetDBWeapon()->szAnimName.c_str(),classFlags,side):nullptr;
                if(!expected || flags==NDb::CAnimation::POSE_STAND)continue;
                CObj<NWorld::CCmdCannon> command=new NWorld::CCmdCannon(cannon);
                CObj<NWorld::CExecCannon> probe=new NWorld::CExecCannon(actor,cannon,true,command);
                if(probe->CanDoIt(position)!=NWorld::UCR_OK)continue;
                fixture.actor=actor;fixture.cannon=cannon;fixture.entryAnimation=expected;fixture.personalFlags=flags;
                for(const auto& clip:skeleton->pAnimations[NDb::CAnimation::START_ATTACK].anims)
                    if(clip->szParams==weapon->GetDBWeapon()->szAnimName)fixture.entryClipIDs.push_back(clip->GetRecordID());
                Require(std::find(fixture.entryClipIDs.begin(),fixture.entryClipIDs.end(),expected->GetRecordID())!=fixture.entryClipIDs.end(),
                    "native cannon positive clip is missing from its authored skeleton entry candidates");
                std::printf("native cannon fixture: persona=%d personalWeapon=%d cannonWeapon=%d name=%s poseFlags=0x%X entryClip=%d\n",
                    actor->GetUnitRPG()->GetRPGPers()->GetRecordID(),personal->GetDBWeapon()->GetRecordID(),weapon->GetDBWeapon()->GetRecordID(),
                    weapon->GetDBWeapon()->szAnimName.c_str(),unsigned(flags),expected->GetRecordID());std::fflush(stdout);
                return fixture;
            }
        }
        actor->SetPosition(original);actor->animator.AlignTime();actor->animator.PlaceUnit(original);
    }
    Require(false,"cannon fixture has no actual armed actor, free authored approach and matching original stance-only entry clip");
    return fixture;
}

}
