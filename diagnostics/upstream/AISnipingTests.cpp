#include "NativeSnipeFixture.h"
#include "../../Main/DG.H"
#include "../../Main/RPGAttackMech.h"
#include "../../Main/RPGGame.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/RPGToHit.h"
#include "../../Main/aiActions.h"
#include "../../Main/aiCombatLog.h"
#include "../../Main/aiGrid.h"
#include "../../Main/aiInventory.h"
#include "../../Main/aiMap.h"
#include "../../Main/aiMisc.h"
#include "../../Main/aiRouteMisc.h"
#include "../../Main/aiUnit.h"
#include "../../Main/aiUnitState.h"
#include "../../Main/aiWeapon.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wUnitStates.h"
#include "../../DBFormat/DataMap.h"
#include <cstdio>
#include <cstring>

namespace UpstreamTests
{
namespace
{
static std::pair<int,int> NativeSnipeEscapeVisibility(NAI::IAIUnit* actor,NAI::IAIUnit* target,int cost)
{
    auto* world=actor->GetUnitServer()->GetWorld();auto* game=world->GetGame();auto* network=world->GetPathNetwork();
    const auto pose=actor->GetUnitPosition();const float range=game->GetUnitSightDistance(actor->GetRPGUnit()),fov=actor->GetRPGUnit()->GetSightFOV();
    vector<NAI::SPathPlace> places;NAI::GetNearestPlaces(target->GetUnitServer(),target->GetPosition().p,cost,NAI::RUN,&places);
    int blocked=0;for(const auto& point:places)if(!game->CheckPositionVisibility(pose,NAI::SPosition(point,network),range,fov))++blocked;
    return {int(places.size()),blocked};
}
static bool PlaceNativeVisibleEscapeFixture(NAI::IAIUnit* actor,NAI::IAIUnit* target,
    NAI::CAIFireArmsWeapon* weapon,NAI::CAIBeginSnipeAction* action)
{
    auto* server=actor->GetUnitServer();auto* victim=target->GetUnitServer();auto* world=server->GetWorld();
    auto* network=world->GetPathNetwork();auto* game=world->GetGame();
    const auto original=server->GetPosition(),targetOriginal=victim->GetPosition();
    vector<NAI::SPathPlace> anchors;NAI::GetNearestPlaces(victim,targetOriginal.pos.p,target->GetMaxAP(),NAI::RUN,&anchors);
    const float range=game->GetUnitSightDistance(actor->GetRPGUnit()),fov=actor->GetRPGUnit()->GetSightFOV();
    auto place=[&](NWorld::CUnitServer* unit,const NAI::SUnitPosition& position) {
        unit->SetPosition(position);unit->animator.AlignTime();unit->animator.PlaceUnit(position);
        MarkNewDGFrame();world->GetAIMap()->Sync();
    };
    for(int facing=0;facing<8;++facing) {
        auto turned=original;turned.pos.p.SetDirection(facing);place(server,turned);
        for(auto anchor:anchors) {
            if(!network->IsPassable(anchor) || network->IsLocked(anchor))continue;
            NAI::SPosition position(anchor,network);
            if(fabs(position.GetCP()-original.GetCP())*1.6f<15.f
                || !game->CheckPositionVisibility(server->GetPosition(),position,range,fov))continue;
            vector<NAI::SPathPlace> four;NAI::GetNearestPlaces(victim,anchor,4,NAI::RUN,&four);
            if(four.empty())continue;
            bool visible=true;for(const auto& point:four)if(!game->CheckPositionVisibility(server->GetPosition(),NAI::SPosition(point,network),range,fov)){visible=false;break;}
            if(!visible)continue;
            auto moved=targetOriginal;moved.pos.p=anchor;place(victim,moved);
            NAI::CAIBeginSnipeAction::SInfo info;action->GetInfoInner(NAI::SPlaceWithAP(server->GetPosition(),actor->GetAP()),&info);
            const auto actualFour=NativeSnipeEscapeVisibility(actor,target,4),full=NativeSnipeEscapeVisibility(actor,target,target->GetMaxAP());
            if(info.hitLocation==NAI::HL_HEAD && info.pWeapon==weapon && actualFour.first>0 && actualFour.second==0 && full.second>0
                && actor->GetToHitWithWeapon(target,actor->GetUnitPosition(),NAI::HL_HEAD,weapon)>0) {
                std::printf("native escape placement: direction=%d distance=%.2f four=%d blocked4=%d full=%d blockedFull=%d\n",
                    facing,fabs(server->GetPosition().GetCP()-victim->GetPosition().GetCP()),actualFour.first,actualFour.second,full.first,full.second);std::fflush(stdout);
                return true;
            }
            place(victim,targetOriginal);
        }
    }
    place(victim,targetOriginal);place(server,original);return false;
}
// Audit an upstream assignment against the fork's actual reachable poses.
// This is an equivalence check, not a manufactured RED for unchanged behavior.
static void NativeSnipeFacingParity(NAI::IAIUnit* actor,NAI::IAIUnit* target,bool canSnipe)
{
    auto* world=actor->GetUnitServer()->GetWorld();auto* network=world->GetPathNetwork();auto* game=world->GetGame();
    const auto source=actor->GetUnitPosition(),destination=target->GetUnitPosition();
    const int ap=actor->GetAP(),targetAP=target->GetAP(),hp=actor->GetRPGUnit()->Skills(NDb::ST_VP),targetHP=target->GetRPGUnit()->Skills(NDb::ST_VP);
    auto rng=GlobalGameRandom();const float range=game->GetUnitSightDistance(actor->GetRPGUnit()),fov=actor->GetRPGUnit()->GetSightFOV();
    const auto closest=network->GetClosestDir(source.pos.p,destination.pos.p);
    int total=0,poseMask=0,changed=0,alternatives=0,visibility=0,hidden=0,fourCount=0;
    for(int budget:{4,target->GetMaxAP()}) {
        vector<NAI::SPathPlace> places;NAI::GetNearestPlaces(target->GetUnitServer(),destination.pos.p,budget,NAI::RUN,&places);
        Require(!places.empty(),"facing audit lost its nonempty actual native escape flood");
        if(budget==4)fourCount=int(places.size());
        for(auto point:places) {
            const auto pose=point.GetPose();Require(pose==NAI::CM_CROUCH || pose==NAI::CM_STAND,"native escape flood gained a direction-sensitive or inactive pose");
            poseMask|=1<<pose;NAI::SPosition baseline(point,network);vector<CVec3> cubes;NRPG::GetOccupiedCubes(&cubes,baseline);
            Require(!cubes.empty(),"facing audit has no actual occupied target cubes");
            const bool seen=budget==4?game->CheckPositionVisibility(source,baseline,range,fov):false;
            if(budget==4 && !seen)++hidden;
            if(point.GetDirection()!=closest)++changed;
            for(int direction=0;direction<8;++direction) {
                if(direction!=point.GetDirection())++alternatives;
                auto candidate=baseline;candidate.p.SetDirection(direction);vector<CVec3> actual;NRPG::GetOccupiedCubes(&actual,candidate);
                Require(candidate.GetCP()==baseline.GetCP() && actual==cubes,"target escape facing changed native coordinates or occupied cubes");
                if(budget==4) {
                    Require(game->CheckPositionVisibility(source,candidate,range,fov)==seen,"target escape facing changed the native snipe visibility decision");++visibility;
                }
                ++total;
            }
        }
    }
    std::printf("native snipe facing parity: can=%d four=%d hidden=%d directions=8 poseMask=%d comparisons=%d visibility=%d alternateDirections=%d originalDirectionChanges=%d\n",
        int(canSnipe),fourCount,hidden,poseMask,total,visibility,alternatives,changed);std::fflush(stdout);
    Require(total>8 && alternatives>0 && visibility==fourCount*8 && canSnipe==(hidden==0),"native facing audit did not exercise real changed direction bits and the expected positive/blocked decision");
    Require(actor->GetUnitPosition()==source && target->GetUnitPosition()==destination && actor->GetAP()==ap && target->GetAP()==targetAP
        && int(actor->GetRPGUnit()->Skills(NDb::ST_VP))==hp && int(target->GetRPGUnit()->Skills(NDb::ST_VP))==targetHP
        && rng.Get()==GlobalGameRandom().Get(),"native facing equivalence queries changed position, health, AP or RNG");
}
static void AISnipeSelectedWeapon(const char* databasePath, const char* resourcePath, int mode, int lifecycle = 0)
{
    CObj<NRPG::CGlobalGame> campaign;auto world=SnipeFixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NDb::CRPGWeapon* rifle=nullptr;CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while(weapons.MoveNext()) {
        auto* record=weapons.Get();
        if(record->eWeaponType==NDb::WT_RIFLE && record->shootModes[NDb::SM_Snipe] && record->shootModes[NDb::SM_Snap]
            && !record->bBazookaLogic && record->pItem && record->pInnerClip && record->pInnerClip->nQuantity>0){rifle=record;break;}
    }
    Require(rifle,"selected-snipe fixture has no authored loaded sniper rifle");
    CObj<NAI::IAIUnit> actor,target;CPtr<NAI::CAIFireArmsWeapon> selected;NAI::CAIBeginSnipeAction::SInfo baseline;
    int selectedHead=0;float distance=0;
    const bool visibleEscape=mode==3 || mode==5 || mode==7;
    for(const auto& server:units) {
        if(!IsValid(server) || !server->CanFight() || server->HasCommand() || server->IsSniping() || !server->IsAIUnit()
            || server->IsEmptyPK() || server->IsWearingPK() || server->animator.GetCannon())continue;
        auto* shooter=NAI::GetAIUnit(server);if(!IsValid(shooter))continue;
        CObj<NRPG::CWeaponItem> item=new NRPG::CWeaponItem(rifle);auto* equipment=server->GetUnitRPG()->GetInventory();
        if(!equipment->Equip(NDb::SLOT_1,item) || !equipment->Activate(NDb::SLOT_1) || !item->SetShootMode(NDb::SM_Snap))continue;
        shooter->Synchronize(false);auto* inventory=shooter->GetAIInventory();auto* choice=inventory->GetBestWeaponForSnipe();
        if(!IsValid(choice) || !IsValid(choice->GetCurrentClip()) || choice->GetCurrentClip()->IsEmpty())continue;
        auto* state=shooter->GetAIUnitState();if(!state)continue;
        CPtr<NAI::IAIUnit> oldEnemy=state->pEnemy,oldPossible=state->pPossibleEnemy;
        struct RestoreState{NAI::SAIUnitState* state;CPtr<NAI::IAIUnit> enemy,possible;~RestoreState(){state->pEnemy=enemy;state->pPossibleEnemy=possible;}}restore{state,oldEnemy,oldPossible};
        CObj<NAI::CAIBeginSnipeAction> action=new NAI::CAIBeginSnipeAction(shooter);
        for(const auto& other:units) {
            if(!IsValid(other) || other.GetPtr()==server.GetPtr() || !other->CanFight() || other->IsEmptyPK() || other->IsWearingPK()
                || server->GetDiplomacyState(other)!=NDb::DS_ENEMY)continue;
            auto* victim=NAI::GetAIUnit(other);if(!IsValid(victim))continue;
            float range=fabs(server->GetPosition().GetCP()-other->GetPosition().GetCP());
            if(range*1.6f<15.f)continue;
            state->pEnemy=victim;state->pPossibleEnemy=nullptr;
            if(visibleEscape && !PlaceNativeVisibleEscapeFixture(shooter,victim,choice,action))continue;
            range=fabs(server->GetPosition().GetCP()-other->GetPosition().GetCP());
            const auto pose=shooter->GetUnitPosition();
            const int cover=shooter->GetCoverForFixedUnit(pose,other,choice->GetItem(),NAI::HL_HEAD);
            CObj<NRPG::CAIUnitToHitCalcer> reference=new NRPG::CAIUnitToHitCalcer(shooter,pose,victim,cover,NAI::HL_HEAD,0,choice->GetItem(),0);
            const int expectedHead=reference->GetToHit();NAI::CAIBeginSnipeAction::SInfo info;
            action->GetInfoInner(NAI::SPlaceWithAP(server->GetPosition(),shooter->GetAP()),&info);
            std::printf("native snipe calibration: distance=%.2f selectedHead=%d location=%d can=%d\n",range,expectedHead,int(info.hitLocation),int(info.bCanDo));std::fflush(stdout);
            if(mode>=3 && expectedHead>0 && info.hitLocation==NAI::HL_HEAD && info.pWeapon==choice) {
                const auto four=NativeSnipeEscapeVisibility(shooter,victim,4),full=NativeSnipeEscapeVisibility(shooter,victim,victim->GetMaxAP());
                std::printf("native escape calibration: mode=%d distance=%.2f four=%d blocked4=%d full=%d blockedFull=%d maxAP=%d\n",
                    mode,range,four.first,four.second,full.first,full.second,victim->GetMaxAP());std::fflush(stdout);
                if(four.first==0 || (visibleEscape?four.second!=0 || full.second==0:four.second==0))continue;
            }
            if(expectedHead>0 && info.hitLocation==NAI::HL_HEAD && info.pWeapon==choice) {
                actor=shooter;target=victim;selected=choice;baseline=info;selectedHead=expectedHead;distance=range;break;
            }
        }
        if(actor)break;
    }
    Require(actor && target && selected && selectedHead>0 && baseline.hitLocation==NAI::HL_HEAD,
        "selected-snipe fixture has no native distant enemy with independently positive selected-rifle head prediction and reached head-choice stage");
    auto* inventory=actor->GetAIInventory();auto* state=actor->GetAIUnitState();CObj<NAI::IAIInventoryItem> current=inventory->GetCurrentItem();
    struct Restore{NAI::CAIInventory* inventory;CObj<NAI::IAIInventoryItem> current;NAI::SAIUnitState* state;CPtr<NAI::IAIUnit> enemy,possible;
        vector<std::pair<CPtr<NAI::CAIFireArmsWeaponClip>,int>> clips;
        ~Restore(){for(auto& entry:clips)entry.first->SetAmmoCount(entry.second);inventory->SetCurrentItem(current);state->pEnemy=enemy;state->pPossibleEnemy=possible;}
    }restore{inventory,current,state,state->pEnemy,state->pPossibleEnemy,{}};
    state->pEnemy=target;state->pPossibleEnemy=nullptr;
    if(mode==0 || mode==2)inventory->SetCurrentItem(nullptr);
    if(mode==2) {
        while(auto* weapon=inventory->GetBestWeaponForSnipe()) {
            auto* clip=weapon->GetCurrentClip();Require(clip && clip->GetAmmoCount()>0 && restore.clips.size()<128,"snipe ammo control did not advance through native planning clips");
            restore.clips.push_back({clip,clip->GetAmmoCount()});clip->SetAmmoCount(0);
        }
    } else Require(inventory->GetBestWeaponForSnipe()==selected.GetPtr(),"empty planning hand removed the actual selected sniper rifle");
    auto* server=actor->GetUnitServer();auto* victim=target->GetUnitServer();const auto position=server->GetPosition(),targetPosition=victim->GetPosition();
    auto* equipped=server->GetUnitRPG()->GetInventory()->GetActive();const int ap=server->GetAP(),hp=server->GetUnitRPG()->GetTotalVP(),targetHP=victim->GetUnitRPG()->GetTotalVP();
    const int ammo=selected->GetItem()->GetAmmoQuantity(),planningAmmo=selected->GetCurrentClip()->GetAmmoCount();const bool command=server->HasCommand();
    auto* planningHand=inventory->GetCurrentItem();GlobalGameRandom().SeedForHarness(0x79da);auto rng=GlobalGameRandom();
    CObj<NAI::CAIBeginSnipeAction> action=new NAI::CAIBeginSnipeAction(actor);NAI::CAIBeginSnipeAction::SInfo info;
    action->GetInfoInner(NAI::SPlaceWithAP(position,actor->GetAP()),&info);
    std::printf("native selected snipe: mode=%d emptyHand=%d noAmmo=%d distance=%.2f selectedHead=%d location=%d baselineLocation=%d can=%d baselineCan=%d selected=%d planningAmmo=%d\n",
        mode,int(mode==0 || mode==2),int(mode==2),distance,selectedHead,int(info.hitLocation),int(baseline.hitLocation),int(info.bCanDo),int(baseline.bCanDo),int(info.pWeapon==selected),planningAmmo);std::fflush(stdout);
    Require(server->GetPosition()==position && victim->GetPosition()==targetPosition && server->GetAP()==ap && server->GetUnitRPG()->GetTotalVP()==hp
        && victim->GetUnitRPG()->GetTotalVP()==targetHP && server->GetUnitRPG()->GetInventory()->GetActive()==equipped
        && selected->GetItem()->GetAmmoQuantity()==ammo && selected->GetCurrentClip()->GetAmmoCount()==planningAmmo
        && inventory->GetCurrentItem()==planningHand && server->HasCommand()==command && rng.Get()==GlobalGameRandom().Get(),
        "snipe prediction changed native equipment/ammo/AP/HP/positions/commands, planning hand/clip or RNG");
    if (mode == 7)
    {
        Require(info.bCanDo, "snipe fixture lacks a visible target from the current firing position");
        auto* net = world->GetPathNetwork();
        auto* game = world->GetGame();
        vector<NRPG::CAttackPortion> attacks;
        selected->GetItem()->CreateNewAttackPortion(&attacks, false);
        Require(!attacks.empty(), "snipe fixture lacks a real attack");
        vector<NAI::SPathPlace> nearby;
        net->GetNearPlaces(SSphere(position.GetCP(), 7.f), &nearby);
        bool foundBlockedPose = false;
        for (auto p : nearby)
        {
            if (!p.IsIntegral() || p.IsFinal() || p.GetLayer() != position.pos.p.GetLayer())
                continue;
            p.SetPose(NAI::CM_CROUCH);
            p.SetDirection(net->GetClosestDir(p, targetPosition.pos.p));
            if (net->GetPassability(p) != NAI::AIP_YES || fabs2(NAI::SPosition(p, net).GetCP() - position.GetCP()) < 1.f)
                continue;
            auto proposed = position;
            proposed.pos.p = p;
            proposed.bRun = false;
            const float cover = game->GetCoverForAIUnitPrecise(server->GetAttackOrigin(proposed, false),
                server, victim, attacks.front(), NAI::HL_ANY);
            if (cover > 0)
                continue;
            Require(actor->GetToHitWithWeapon(target, proposed, NAI::HL_ANY, selected) == 0,
                "AI shot calculation ignored the fully blocked proposed position");
            NAI::CAIBeginSnipeAction::SInfo blocked;
            action->GetInfoInner(NAI::SPlaceWithAP(proposed, actor->GetAP()), &blocked);
            Require(!blocked.bCanDo && server->GetPosition() == position,
                "AI chose the blocked firing position by testing its current position");
            foundBlockedPose = true;
            break;
        }
        Require(foundBlockedPose, "snipe fixture lacks a nearby crouching position behind real cover");

        world->GivePlayerTurn(server->GetTBSPlayer());
        server->GetUnitRPG()->StartNewTurn(position.GetCP());
        // Refresh the planner's AP without replacing the inventory used by the fixture.
        actor->SetAP(server->GetAP(), actor->GetMaxAP());
        state->pEnemy = target;
        state->pPossibleEnemy = nullptr;
        if (lifecycle == 1) {
            const int before = server->GetAP();
            CObj<NAI::CAILog> log = NAI::CreateAILog();
            action->Do(log);
            Require(!log->IsEmpty() && !server->IsSniping(), "native AI planning did not issue an unexecuted aim order");
            Require(server->GetAP() == before - 1, "the native AI aim plan did not spend one live AP");
            return;
        }
        CDynamicCast<NRPG::CWeaponItem> liveGun(server->GetUnitRPG()->GetInventory()->GetActive());
        Require(liveGun && liveGun->SetShootMode(NDb::SM_Snipe), "unable to arm the real sniper rifle");
        const auto location = game->GetCompositeToHit(server, victim, NAI::HL_HEAD, false) > 0 ?
            NAI::HL_HEAD : NAI::HL_ANY;
        CObj<NWorld::CUnitStateSniping> aiming = new NWorld::CUnitStateSniping(server, victim, 12, location);
        server->SetState(aiming);
        CObj<NAI::CAISnipeShotAction> shot = new NAI::CAISnipeShotAction(actor);
        CObj<NAI::CAICollectSnipeAPAction> collect = new NAI::CAICollectSnipeAPAction(actor);
        CObj<NAI::CAICancelSnipeAction> cancel = new NAI::CAICancelSnipeAction(actor);
        const NAI::SPlaceWithAP here(position, actor->GetAP());
        if (lifecycle == 2) {
            server->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_SNIPE).SetConst(17);
            actor->SetAP(2, actor->GetMaxAP());
            NAI::CAICollectSnipeAPAction::SInfo bank;
            collect->GetInfoInner(here, &bank);
            const int expected = (std::min)(server->GetAP(), 17);
            Require(expected > 2 && expected != aiming->GetBaseSnipeAP() && bank.bCanDo && bank.nAP == expected,
                "the native snipe plan used simulated AP or the starting skill in place of live AP and skill");
            return;
        }
        if (lifecycle == 3) {
            const auto current = actor->GetUnitPosition();
            auto otherPose = current;
            otherPose.pos.p.SetPose(current.pos.p.GetPose() == NAI::CM_CROUCH ? NAI::CM_STAND : NAI::CM_CROUCH);
            const NAI::SPlaceWithAP exact(current, 3);
            const NAI::SPlaceWithAP cheaper(otherPose, 30);
            const NAI::CAIAction* actions[] = {collect.GetPtr(), shot.GetPtr(), cancel.GetPtr()};
            for (const auto* active : actions)
                Require(active->ComparePlaces(exact, cheaper) && !active->ComparePlaces(cheaper, exact),
                    "active sniping ranked a new stance above its current stance");
            return;
        }
        NAI::CAISnipeShotAction::SInfo shotInfo;
        NAI::CAICollectSnipeAPAction::SInfo collectInfo;
        NAI::CAICancelSnipeAction::SInfo cancelInfo;
        shot->GetInfoInner(here, &shotInfo);
        collect->GetInfoInner(here, &collectInfo);
        cancel->GetInfoInner(here, &cancelInfo);
        std::printf("stored snipe: location=%d chance=%d AP=%d shot=%d collect=%d cancel=%d\n",
            int(location), game->GetCompositeToHit(server, victim, location, false), actor->GetAP(),
            int(shotInfo.bCanDo), int(collectInfo.bCanDo), int(cancelInfo.bCanDo));
        Require(shotInfo.bCanDo && collectInfo.bCanDo && !cancelInfo.bCanDo,
            "the sniper could not continue aiming at a live, hittable target");
        if (lifecycle == 4) {
            CPtr<NAI::IAIUnit> replacement;
            for (const auto& other : units) {
                if (!IsValid(other) || other.GetPtr() == server || other.GetPtr() == victim || !other->CanFight() ||
                    server->GetDiplomacyState(other) != NDb::DS_ENEMY ||
                    fabs(other->GetPosition().GetCP() - position.GetCP()) * 1.6f < 15.f)
                    continue;
                replacement = NAI::GetAIUnit(other);
                if (IsValid(replacement))
                    break;
            }
            Require(IsValid(replacement), "native retarget fixture lacks a second distant enemy");
            state->pEnemy = replacement;
            cancel->GetInfoInner(here, &cancelInfo);
            Require(cancelInfo.bCanDo && server->GetSnipingState() == aiming.GetPtr() && victim->CanFight(),
                "the AI retained its old aim after selecting another eligible enemy");
            return;
        }
        victim->Die();
        Require(victim->IsDead() && server->IsSniping(), "the stored target did not die while aim remained active");
        shot->GetInfoInner(here, &shotInfo);
        collect->GetInfoInner(here, &collectInfo);
        cancel->GetInfoInner(here, &cancelInfo);
        Require(!shotInfo.bCanDo && !collectInfo.bCanDo && cancelInfo.bCanDo,
            "the sniper kept planning shots or saving AP for a dead target");
        return;
    }
    if(mode>=3) {
        const auto four=NativeSnipeEscapeVisibility(actor,target,4),full=NativeSnipeEscapeVisibility(actor,target,target->GetMaxAP());
        std::printf("native snipe escape: mode=%d four=%d blocked4=%d full=%d blockedFull=%d maxAP=%d can=%d expected=%d location=%d\n",
            mode,four.first,four.second,full.first,full.second,target->GetMaxAP(),int(info.bCanDo),int(four.second==0),int(info.hitLocation));std::fflush(stdout);
        Require(four.first>0 && info.pWeapon==selected && info.hitLocation==NAI::HL_HEAD,"escape fixture did not reach its native visibility decision with a positive selected rifle");
        Require(visibleEscape?four.second==0 && full.second>0:four.second>0,"escape fixture did not separate four-AP visibility from a genuinely blocked control");
        Require(info.bCanDo==(four.second==0),"AI snipe eligibility scanned beyond the original four-AP escape budget");
        if(mode>=5)NativeSnipeFacingParity(actor,target,info.bCanDo);
        return;
    }
    if(mode==2)Require(!info.bCanDo && !IsValid(info.pWeapon) && info.hitLocation==NAI::HL_ANY,"snipe selection ignored empty native planning magazines");
    else Require(info.pWeapon==selected && info.hitLocation==NAI::HL_HEAD && info.bCanDo==baseline.bCanDo && info.pTarget==baseline.pTarget,
        "AI snipe prediction ignored the selected loaded rifle when its current planning hand was empty");
}
}

bool RunAISnipingTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "ai-snipe-retarget") && argc == 4) { AISnipeSelectedWeapon(argv[2], argv[3], 7, 4); return true; }
    if (!std::strcmp(mode, "ai-snipe-live-ap") && argc == 5) { AISnipeSelectedWeapon(argv[2], argv[3], 7, std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "ai-snipe-proposed-pose-and-target") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-snipe-selected-empty-hand") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-snipe-selected-current-hand") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-snipe-selected-no-ammo") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-snipe-escape-four-visible") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-snipe-escape-four-blocked") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-snipe-facing-parity-visible") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-snipe-facing-parity-blocked") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],6); return true; }
    return false;
}
}
