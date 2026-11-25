#include "TestSupport.h"
#include "NativeCoverSnapshot.h"
#include "../../Main/wMain.h"
#include "../../Main/wExplTracker.h"
#include "../../Main/wAckBase.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wUnitCommands.h"
#include "../../Main/wUnitExec.h"
#include "../../Main/wMine.h"
#include "../../Main/wObject.h"
#include "../../Main/A5Script.h"
#include "../../Main/wDebris.h"
#include "../../Main/wOSBase.h"
#include "../../Main/wMisc.h"
#include "../../Main/wTSFlags.h"
#include "../../Main/aiMap.h"
#include "../../Main/aiInterval.h"
#include "../../Main/aiLocker.h"
#include "../../Main/aiPath.h"
#include "../../Main/RPGAttackMech.h"
#include "../../Main/RPGBullet.h"
#include "../../Main/RPGGame.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/GAnimParticles.h"
#include "../../Main/rpgGlobal.h"
#include "../../Main/wUnitAttackExec.h"
#include "../../DBFormat/DataRPG.h"
#include "../../DBFormat/DataAI.h"
#include "../../DBFormat/DataMap.h"
#include "../../DBFormat/DataFormat.h"
#include "../../DBFormat/DataSound.h"
#include "../../DBFormat/DataGeometry.h"
#include "../../DBFormat/DataObject.h"
#include "../../DBFormat/DataPerk.h"
#include "../../Misc/RandomGen.h"
#include "SoundVisitor.h"
#include "NativeGrenadeThrowFixture.h"
#include <set>
#include <cstring>

namespace UpstreamTests
{
namespace
{
class ShotFeedbackAck : public NWorld::CGlobalAck
{
    OBJECT_NOCOPY_METHODS(ShotFeedbackAck);
public:
    int empty = 0;
    int hits = 0;
    void OnLastPieceOfAmmo(NWorld::CUnitServer*) override { ++empty; }
    void OnTargetHit(NWorld::CUnitServer*) override { ++hits; }
    void OnHardTargetHit(NWorld::CUnitServer*) override { ++hits; }
};

class ShotFeedbackWorld : public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(ShotFeedbackWorld);
public:
    CObj<ShotFeedbackAck> feedback;
    int visibility = 0;
    ShotFeedbackWorld() = default;
    explicit ShotFeedbackWorld(NRPG::CGlobalGame* game) : NWorld::CWorld(game), feedback(new ShotFeedbackAck) {}
    NWorld::CGlobalAck* GetGlobalAck() const override { return feedback ? feedback.GetPtr() : NWorld::CWorld::GetGlobalAck(); }
    void UpdateVisible(bool force) override {
        ++visibility;
        NWorld::CWorld::UpdateVisible(force);
    }
};

class CompletionBurstProbe : public NWorld::CExecShootUnit
{
public:
    using CExecShootUnit::CExecShootUnit;
    void RetainSound(NWorld::C3DSound* sound) { longBurstSnd.pLongBurstSnd = sound; }
    bool IsRetainingSound() const { return IsValid(longBurstSnd.pLongBurstSnd); }
    STime GetNextLaunch() const { return tNextBulletGo; }
};

class TileAccuracyProbe : public NWorld::CExecShootTile
{
public:
    TileAccuracyProbe(NWorld::CUnitServer* actor, const CVec3& target) : CExecShootTile(actor, target)
    {
        bOnlyPrepareToShoot = true;
    }
    int GetChance() const { return nToHit; }
};

void CheckShotTurnFlag(NWorld::CWorld* world, NWorld::CUnitServer* actor)
{
    auto* mission = actor->GetUnitRPG();
    const auto position = actor->GetPosition();
    for (int turn = 0; turn < 2 && mission->IsFirstTurn(); ++turn)
        mission->StartNewTurn(position.GetCP());
    Require(!mission->IsFirstTurn() && world->IsFirstTurn(), "shot fixture needs different unit and world turn flags");
    world->GetGame()->SetNight(false);
    vector<NRPG::CAttackPortion> attacks;
    Require(mission->CreateAttack(&attacks, false) && !attacks.empty(), "shot fixture has no native attack for cover");
    auto* path = world->GetPathNetwork();
    vector<NAI::SPathPlace> places;
    path->GetNearPlaces(SSphere(position.GetCP(), 14.f), &places);
    for (const auto& place : places) {
        if (!place.IsIntegral() || place.IsFinal())
            continue;
        const CVec3 target = NAI::SPosition(place, path).GetCP() + CVec3(0, 0, 0.625f);
        if (fabs2(target - position.GetCP()) < 9.f)
            continue;
        CObj<NRPG::CCoverInfo> cover = world->GetGame()->CalcCoversForTile(actor->GetAttackOrigin(position),
            attacks.front(), actor, target, actor->GetMinClearDistance(), 0x8210);
        int first = 0, later = 0;
        NRPG::CheckTileToHit(actor, target, 0, NAI::THL_LOWER, cover, true, &first);
        NRPG::CheckTileToHit(actor, target, 0, NAI::THL_LOWER, cover, false, &later);
        if (first == later)
            continue;
        CObj<TileAccuracyProbe> shot = new TileAccuracyProbe(actor, target);
        shot->Start();
        Require(shot->GetChance() == later, "native shot used the map first-turn penalty for a later-turn shooter");
        return;
    }
    Require(false, "shot fixture has no real tile cover sensitive to the first-turn inputs");
}

void ShootingCompletion(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game, 810,
        [](NRPG::CGlobalGame* campaign) -> NWorld::CWorld* { return new ShotFeedbackWorld(campaign); });
    auto* observed = static_cast<ShotFeedbackWorld*>(world.GetPtr());
    auto* actor = MovementPolicyActor(world);
    auto* mission = actor->GetUnitRPG();
    auto* inventory = mission->GetInventory();
    auto* record = NDatabase::GetTable<NDb::CRPGWeapon>()->GetRecord(26);
    Require(IsValid(record) && IsValid(record->pInnerClip), "shot completion fixture lacks its native single-shot gun");
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(record);
    const bool delayedBurst = mode == 3 || mode == 4;
    if (delayedBurst) {
        Require(record->pAnimWeaponType, "completion fixture lacks native firing animation settings");
        record->shootModes[NDb::SM_LongBurst] = true;
        record->pAnimWeaponType->nBulletDelay = 100;
    }
    if (mode == 4)
        record->nBPM = 725;
    Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1) &&
        weapon->SetShootMode(delayedBurst ? NDb::SM_LongBurst : NDb::SM_Snap),
        "shot completion fixture failed to activate its loaded gun");
    actor->SetUndrawItem(false);
    actor->animator.SetActiveItem(true);
    actor->animator.SetWeaponAnimation(mission->GetWeaponType());
    actor->animator.SetWeaponName(record->szAnimName.c_str());
    if (mode != 5)
        actor->SetScriptToHit(100);
    world->GivePlayerTurn(actor->GetTBSPlayer());
    const auto position = actor->GetPosition();
    mission->StartNewTurn(position.GetCP());
    const int ap = actor->GetAP();
    const int ammo = weapon->GetAmmoQuantity();
    Require(ammo > 1, "shot completion fixture has no ammunition controls");
    if (mode == 5) {
        CheckShotTurnFlag(world, actor);
        return;
    }
    if (mode == 6) {
        record->shootModes[NDb::SM_ShortBurst] = true;
        record->nRoF = 2;
        Require(weapon->SetShootMode(NDb::SM_ShortBurst), "short burst fixture failed to select the native fire mode");
        Require(mission->GetBulletsQuantityInShot() == 1, "short burst preview resolved to zero bullets");
        return;
    }

    if (mode == 0) {
        CObj<NWorld::CCmdShootObject> command;
        for (const auto& candidate : world->GetObjectsForHarness()) {
            auto* door = dynamic_cast<NWorld::CWindowDoor*>(candidate.GetPtr());
            if (!IsValid(door) || !world->GetAIMap()->GetHull(door))
                continue;
            command = new NWorld::CCmdShootObject(door, 0);
            command->bOnlyPrepareToShoot = true;
            NWorld::EUnitCommandResult result;
            CObj<NWorld::CCommandExecute> action = actor->CreateExecutor(command, &result);
            if (action && result == NWorld::UCR_OK)
                break;
            command = nullptr;
        }
        Require(command, "aim-only order fixture has no reachable native door");
        actor->Do(new NWorld::CCmdSetCommand(actor, command));
        actor->Do(new NWorld::CCmdSetCommand(actor, new NWorld::CCmdContinue));
        Require(actor->IsPerformingAction(), "aim-only door order did not start its native executor");
        Require(weapon->GetAmmoQuantity() == ammo && actor->GetAP() == ap,
            "aim-only object command spent AP or ammunition while raising the gun");
        for (int tick = 0; tick < 24 && actor->IsPerformingAction(); ++tick) {
            auto* clock = world->GetTime();
            clock->Set((std::max)(clock->GetValue() + 1, actor->animator.GetTimeEnd()));
            actor->Segment();
        }
        Require(!actor->IsPerformingAction() && weapon->GetAmmoQuantity() == ammo && actor->GetAP() == ap,
            "aim-only object command completed with a real shot");
        return;
    }

    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* target = nullptr;
    CObj<CompletionBurstProbe> shot;
    for (const auto& candidate : units) {
        if (!IsValid(candidate) || !candidate->CanFight() || actor->GetDiplomacyState(candidate) != NDb::DS_ENEMY)
            continue;
        CObj<CompletionBurstProbe> attempt = new CompletionBurstProbe(actor, candidate, NAI::HL_ANY, 0);
        if (attempt->CanDoIt(position) == NWorld::UCR_OK) {
            target = candidate.GetPtr();
            shot = attempt;
            break;
        }
    }
    Require(shot && target && !target->WasInterrupted(actor), "shot feedback fixture lacks an unmarked native target");
    auto* clip = dynamic_cast<NRPG::CClipItem*>(weapon->GetInnerClip());
    Require(clip && clip->GetItem(), "shot completion fixture lacks its native loaded ammunition");
    if (delayedBurst)
        clip->GetItem()->nQuantity = 1;
    if (mode == 1) {
        clip->GetItem()->nQuantity = 1;
        Require(record->fSilencer >= 1.0f, "concealment fixture requires an unsuppressed gun");
        actor->Hide(true, false);
        Require(actor->IsHiding(), "shot completion fixture did not conceal the shooter");
    }
    const int before = weapon->GetAmmoQuantity();
    GlobalGameRandom().SeedForHarness(42);
    shot->Run();
    Require(weapon->GetAmmoQuantity() == before - 1, "real shot did not spend one round before completion");
    const int visibility = observed->visibility;
    world->GetTime()->Set((std::max)(world->GetTime()->GetValue(), actor->animator.GetTimeLabel1()));
    bool keepLabels = shot->TimeLabelReached();
    if (delayedBurst) {
        Require(keepLabels && shot->GetNextLaunch() > world->GetTime()->GetValue(),
            "delayed burst did not separate its first label from the last projectile");
        if (mode == 4) {
            Require(shot->GetNextLaunch() == world->GetTime()->GetValue() + 100 + 82,
                "native burst launch rounded up its fractional 82-millisecond interval");
            return;
        }
        CObj<NDb::CSound> burstSound = new NDb::CSound;
        burstSound->nEndingSamples = 1;
        CObj<NWorld::C3DSound> sound = new NWorld::C3DSound(position.GetCP(), burstSound);
        shot->RetainSound(sound);
        world->GetTime()->Set(shot->GetNextLaunch());
        shot->Segment();
        Require(shot->IsRetainingSound(), "long burst released its ending before the completion label");
        Require(!shot->TimeLabelReached() && !shot->IsRetainingSound(),
            "long burst did not release its ending at the completion label");
        return;
    }
    for (int tick = 0; tick < 12 && keepLabels; ++tick) {
        world->GetTime()->Set(world->GetTime()->GetValue() + 1000);
        shot->Segment();
        keepLabels = shot->TimeLabelReached();
    }
    Require(!keepLabels, "native shot did not finish its bullet schedule");
    shot->AnimationFinished();
    Require(observed->feedback->hits == 1, "native shot control did not produce exactly one successful result");
    Require(observed->feedback->empty == (mode == 1 ? 1 : 0), "real last round did not deliver exactly one empty-weapon acknowledgment");
    Require(target->WasInterrupted(actor), "a successful native shot left its target eligible to interrupt the shooter again");
    Require((observed->visibility > visibility) == (mode == 1) && !actor->IsHiding(),
        "native shot completion must refresh visibility after revealing the shooter");
}

void MeleeZeroCover(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    CDBIterator<NDb::CRPGMeleeWeapon> weapons(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
    NDb::CRPGMeleeWeapon* record = nullptr;
    while (weapons.MoveNext()) {
        if (!weapons.Get()->bThrowing && weapons.Get()->pItem && weapons.Get()->pWeaponType) {
            record = weapons.Get();
            break;
        }
    }
    Require(record, "zero-cover fixture has no native melee item");
    CObj<NRPG::CMeleeWeaponItem> blade = new NRPG::CMeleeWeaponItem(record);
    blade->nQuantity = 1;
    Require(inventory->Equip(NDb::SLOT_1, blade) && inventory->Activate(NDb::SLOT_1) &&
        NRPG::GetToHitType(actor) == NRPG::TH_MELEE, "zero-cover fixture could not equip the test blade");
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    const auto entry = std::find_if(units.begin(), units.end(), [actor](const auto& candidate) {
        return IsValid(candidate) && candidate.GetPtr() != actor && candidate->CanFight();
    });
    Require(entry != units.end(), "zero-cover fixture lacks a living unit target");
    auto* target = entry->GetPtr();
    const auto center = actor->GetPosition().GetCP();
    const vector<int> visibleParts = {NAI::HL_BODY};
    auto chance = [&](int cover) {
        return NRPG::GetToHit(actor, actor->GetPose(), 1, center, target->GetPosition().pos,
            NAI::HL_BODY, 0, target, visibleParts, cover, false, CVec3(1, 1, 1), false, 0, false);
    };
    Require(chance(100) > 0 && chance(1) > 0, "positive cover controls did not reach the melee calculation");
    Require(chance(0) == 0 && chance(-1) == 0, "blocked native unit melee still had a positive hit chance");
}

void BossWeaponState(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    NDb::CRPGWeapon* record = nullptr;
    CDBIterator<NDb::CRPGWeapon> rows(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while (rows.MoveNext()) {
        if (rows.Get()->pItem && rows.Get()->pWeaponType && rows.Get()->pInnerClip &&
            !rows.Get()->bBazookaLogic && !rows.Get()->nPanzerkleinType) {
            record = rows.Get();
            break;
        }
    }
    Require(record, "boss weapon fixture lacks a native firearm");
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(record);
    Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1),
        "boss weapon fixture could not equip the native firearm");
    auto check = [&](NDb::EWeaponType type, NWorld::CUnit::EState expected) {
        record->eWeaponType = type;
        Require(actor->GetUnitRPG()->GetWeaponType() == type, "native firearm did not present its animation type");
        Require(actor->GetState() == expected, "native unit classified the active weapon as the wrong state");
    };
    check(NDb::WT_RIFLE, NWorld::CUnit::ST_NORMAL_RIFLE);
    check(NDb::WT_PK_PLAZMAGUN, NWorld::CUnit::ST_NORMAL_HAND_MACHINE_GUN);
    check(NDb::WT_BOSS_PLAZMAGUN, NWorld::CUnit::ST_NORMAL_HAND_MACHINE_GUN);
    check(NDb::WT_TERROR_SHOOTER, NWorld::CUnit::ST_NORMAL_HAND_MACHINE_GUN);
    check(NDb::WT_TERROR_SPECIAL_GUN, NWorld::CUnit::ST_NORMAL_DEFAULT);
}

void SniperTargetPolicy(const char* databasePath, const char* resourcePath, int mode)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    const bool launcher = mode == 2;
    NDb::CRPGWeapon* record = nullptr;
    CDBIterator<NDb::CRPGWeapon> rows(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while (rows.MoveNext()) {
        auto* candidate = rows.Get();
        if (candidate->bBazookaLogic == launcher && candidate->pItem && candidate->pInnerClip &&
            candidate->pWeaponType && !candidate->nPanzerkleinType && (launcher || candidate->shootModes[NDb::SM_Snipe])) {
            record = candidate;
            break;
        }
    }
    Require(record, "sniper validation fixture lacks a native weapon");
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(record);
    if (launcher)
        record->shootModes[NDb::SM_Snipe] = true;
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1) &&
        weapon->SetShootMode(NDb::SM_Snipe) && weapon->HasAmmo() && !actor->IsSniping(),
        "sniper validation fixture could not prepare its loaded weapon");
    actor->animator.SetActiveItem(true);
    actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    actor->SetUndrawItem(false);

    CObj<NWorld::CCmdShootObject> command = new NWorld::CCmdShootObject(nullptr, 0);
    CObj<NWorld::CWindowDoor> stale;
    if (mode == 0) {
        for (const auto& item : world->GetObjectsForHarness()) {
            auto* door = dynamic_cast<NWorld::CWindowDoor*>(item.GetPtr());
            if (IsValid(door) && world->GetAIMap()->GetHull(door)) {
                command->pTarget = door;
                break;
            }
        }
        Require(IsValid(command->pTarget), "sniper validation fixture lacks a native non-unit target");
    } else if (mode == 1) {
        stale = new NWorld::CWindowDoor;
        command->pTarget = stale.GetPtr();
        stale->Invalidate();
        Require(command->pTarget.GetPtr() && !IsValid(command->pTarget), "native target did not become stale");
    }
    NWorld::EUnitCommandResult result = NWorld::UCR_OK;
    CObj<NWorld::CCommandExecute> executor = NWorld::CreateExecutor(actor, command, &result);
    std::printf("native sniper target case=%d result=%d executor=%d\n", mode, int(result), bool(executor));
    if (launcher) {
        Require(result == NWorld::UCR_OK && dynamic_cast<NWorld::CExecLaunchRocket*>(executor.GetPtr()),
            "launcher validation incorrectly routed to rifle sniping");
    } else {
        Require(!executor && result == (mode == 0 ? NWorld::UCR_UNAVAILABLE : NWorld::UCR_GENERAL_FAILURE),
            "sniper validation accepted a non-unit or stale target as an availability query");
    }
}

void DisarmCallback(const char* databasePath, const char* resourcePath, bool floorMine, bool succeeds)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    auto* rpg = actor->GetUnitRPG()->GetRPGUnit();
    auto* inventory = rpg->GetInventory();
    rpg->Skills(NDb::ST_ENGINEERING).SetConst(100);
    CObj<NRPG::CToolItem> tool;
    CDBIterator<NDb::CRPGTool> tools(*NDatabase::GetTable<NDb::CRPGTool>());
    while (tools.MoveNext()) {
        auto* record = tools.Get();
        if (!record->pItem || !record->bCanUseForMineCleaning)
            continue;
        CObj<NRPG::CToolItem> candidate = new NRPG::CToolItem(record);
        if (candidate->CanBeUsed(rpg) && candidate->GetIncQuantity() > 0) {
            tool = candidate;
            break;
        }
    }
    Require(tool, "native disarm fixture lacks a usable mine-clearing tool");
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    Require(inventory->Equip(NDb::SLOT_1, tool) && inventory->Activate(NDb::SLOT_1),
        "native disarm fixture could not equip the mine-clearing tool");

    const int difficulty = 20;
    CObj<NWorld::CMine> mine;
    CPtr<NWorld::CWindowDoor> door;
    CObj<NWorld::CCommandExecute> executor;
    const auto position = actor->GetPosition();
    if (floorMine) {
        NDb::CRPGMine* record = nullptr;
        CDBIterator<NDb::CRPGMine> rows(*NDatabase::GetTable<NDb::CRPGMine>());
        while (rows.MoveNext()) {
            if (rows.Get()->pItem && rows.Get()->pItem->pModel && rows.Get()->pExplosion) {
                record = rows.Get();
                break;
            }
        }
        Require(record, "native disarm fixture lacks an authored floor mine");
        const CVec3 center = position.GetCP() + CVec3(0.6f, 0.f, 0.f);
        mine = new NWorld::CMine(world, center, record, difficulty, position.pos.GetFloor(), actor, 0);
        actor->AddToVisibleTraps(mine);
        executor = new NWorld::CExecDisarmMine(actor, mine);
    } else {
        NDb::CRPGGrenade* record = nullptr;
        CDBIterator<NDb::CRPGGrenade> rows(*NDatabase::GetTable<NDb::CRPGGrenade>());
        while (rows.MoveNext()) {
            if (rows.Get()->pItem && rows.Get()->pItem->pModel) {
                record = rows.Get();
                break;
            }
        }
        Require(record, "native disarm fixture lacks an authored trap grenade");
        for (const auto& object : world->GetObjectsForHarness()) {
            auto* candidate = dynamic_cast<NWorld::CWindowDoor*>(object.GetPtr());
            if (candidate && !candidate->IsBroken() && !candidate->IsMineSet() && world->GetAIMap()->GetHull(candidate)) {
                door = candidate;
                break;
            }
        }
        Require(IsValid(door) && door->SetTrap(record, difficulty), "native disarm fixture could not arm its door");
        actor->AddToVisibleTraps(door);
        executor = new NWorld::CExecDisarmTrap(actor, door);
    }
    world->UpdateVisible(true);
    const auto availability = floorMine ? static_cast<NWorld::CExecDisarmMine*>(executor.GetPtr())->CanDoIt(position) :
        static_cast<NWorld::CExecDisarmTrap*>(executor.GetPtr())->CanDoIt(position);
    Require(availability == NWorld::UCR_OK, "native disarm command is blocked before execution");

    auto* script = world->GetOwnScript();
    Require(IsValid(script) && script->DoString(
        "TestDisarmCalls = 0; function OnMineDiffusion(obj, result) "
        "TestDisarmCalls = TestDisarmCalls + 1; TestDisarmResult = result end") == 0,
        "native disarm fixture could not install its mission callback");
    world->ExecuteOwnScript();
    auto readMedals = [&] {
        struct MedalBook {
            vector<NRPG::CMedalsGainer::SMedalInfo> entries;
            int operator&(CStructureSaver& fields) { fields.Add(2, &entries); return 0; }
        } medals;
        CStructureNetworkContext refs;
        auto fields = BorrowedObjectFields(rpg, &refs);
        CStructureSaver reader(fields, CStructureSaver::READ, &refs);
        reader.Add(20, &medals);
        return medals.entries;
    };
    const auto before = readMedals();
    const auto activeMedal = std::find_if(before.begin(), before.end(), [](const auto& medal) {
        return medal.bIsCollectingPoints && !medal.bIsGained && !medal.bWillBeGiven && IsValid(medal.pMedal) &&
            !medal.pMedal->bIsRussianOnly && medal.pMedal->nPointsToStart > 24 && medal.nProbability == 0;
    });
    Require(activeMedal != before.end(), "native disarm fixture lacks a medal collecting subthreshold points");
    const auto medalIndex = activeMedal - before.begin();

    unsigned seed = 0;
    for (unsigned candidate = 1; candidate < 10000; ++candidate) {
        auto random = GlobalGameRandom();
        random.SeedForHarness(candidate);
        if (random.Get(0, 100) == (succeeds ? 0u : 99u)) {
            seed = candidate;
            break;
        }
    }
    Require(seed != 0, "native disarm fixture could not seed the boundary roll");
    const int charges = tool->GetIncQuantity();
    GlobalGameRandom().SeedForHarness(seed);
    executor->TimeLabelReached();
    world->ExecuteOwnScript();
    const auto after = readMedals();
    const int top = script->GetTop();
    const auto calls = script->GetGlobal("TestDisarmCalls");
    const auto result = script->GetGlobal("TestDisarmResult");
    const bool called = calls.IsNumber() && calls.GetNumber() == 1;
    const float delivered = result.IsNumber() ? result.GetNumber() : -1.f;
    script->SetTop(top);
    Require(called, "native disarm executor did not deliver exactly one mission callback");
    Require(tool->GetIncQuantity() == charges - 1, "native disarm changed how the tool charge is consumed");
    if (!floorMine)
        Require(!door->IsMineSet(), "door trap remained armed after native disarm");
    Require(after.size() == before.size(), "native disarm unexpectedly changed the medal book");
    const float points = after[medalIndex].fPoints - before[medalIndex].fPoints;
    std::printf("native disarm mine=%d success=%d callback=%.0f medal=%g\n", floorMine, succeeds, delivered, points);
    const float expectedPoints = succeeds && !floorMine ? 24.f : 0.f;
    Require(delivered == float(succeeds) && std::fabs(points - expectedPoints) < 1e-4f,
        "native disarm delivered the wrong result or trap medal credit");
}

void CoverRayRowOrder(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    const CVec3 source = actor->GetAttackOrigin(actor->GetPosition()) + CVec3(0.f, 0.f, 50.f);
    const CVec3 target = source + CVec3(8.f, 0.f, 0.f);
    NRPG::CAttackPortion attack(100000, 0, 0.f, 5, 5, 0);
    CObj<NRPG::CCoverInfo> cover = world->GetGame()->CalcCoversForTile(source, attack, actor, target, 0.f);
    Require(cover, "tile cover solver returned no native ray grid");
    const auto saved = CoverSnapshot(cover);
    Require(saved.source == source && saved.hits.size() == 16 && saved.loose.size() == 384 && saved.obstacles.empty(),
        "native open tile cover fixture did not partition the complete 20-by-20 ray grid");
    NAI::CFastRenderer projection;
    const float halfView = 1.25f * 30.f / (8.f - 0.625f);
    projection.InitProjective(source, source + CVec3(30.f, 0.f, 0.f), CVec2(halfView, halfView), 10);
    size_t hit = 0, loose = 0;
    for (int y = 19; y >= 0; --y)
        for (int x = 0; x < 20; ++x) {
            CVec3 expected;
            projection.GetDir(&expected, float(x), float(y));
            const auto& ray = x >= 8 && x < 12 && y >= 8 && y < 12 ? saved.hits[hit++] : saved.loose[loose++];
            Require(fabs2(ray.direction - expected) < 1e-10f,
                "native cover rays did not traverse descending rows from left to right");
        }
}

CObj<NRPG::CCoverInfo> RestoreCoverRays(vector<CoverParitySnapshot::Ray> hits, vector<CoverParitySnapshot::Ray> loose)
{
    CObj<CObjectBase> savedCover = pSSClasses->CreateObject(0x02841161);
    CObj<NRPG::CCoverInfo> cover = CastToUserObject(savedCover.GetPtr(), static_cast<NRPG::CCoverInfo*>(nullptr));
    Require(cover, "retail cover save class is unavailable");
    CMemoryStream fields;
    CVec3 origin(1.f, 2.f, 3.f);
    {
        CStructureSaver writer(fields, CStructureSaver::WRITE);
        writer.Add(2, &hits);
        writer.Add(3, &origin);
        writer.Add(4, &loose);
    }
    fields.Seek(0);
    { CStructureSaver reader(fields, CStructureSaver::READ); savedCover->operator&(reader); }
    return cover;
}

void EqualMissRayOrder()
{
    using Ray = CoverParitySnapshot::Ray;
    vector<Ray> authored = {
        {CVec3(1.f, 0.f, 0.f), false, 0.4f},
        {CVec3(0.f, 1.f, 0.f), false, 0.7f},
        {CVec3(-1.f, 0.f, 0.f), false, 0.4f},
        {CVec3(0.f, -1.f, 0.f), false, 0.7f},
    };
    auto cover = RestoreCoverRays({}, authored);
    CRay ray;
    bool missed = false;
    Require(NRPG::PeekRay(cover, &ray, 0.5f, &missed) && missed && ray.ptOrigin == CVec3(1.f, 2.f, 3.f),
        "native miss selection did not use the restored cover");
    const auto after = CoverSnapshot(cover);
    const int order[] = {2, 0, 3, 1};
    Require(after.loose.size() == authored.size(), "native miss selection lost authored rays");
    for (size_t index = 0; index < after.loose.size(); ++index)
        Require(after.loose[index].direction == authored[order[index]].direction &&
            after.loose[index].deviation == authored[order[index]].deviation,
            "equal miss rays did not take the second retail merge chain first");
}

void BurstRayFollowsPrevious(bool miss)
{
    const CVec3 preceding(0.f, 1.f, 0.f);
    const CVec3 orthogonal(1.f, 0.f, 0.f);
    const vector<CoverParitySnapshot::Ray> rays = {{preceding, true, 0.f}, {orthogonal, true, 0.9f}};
    auto cover = miss ? RestoreCoverRays({}, rays) : RestoreCoverRays(rays, {});
    NRPG::SAttackRayInfo shot;
    const float score = miss ? 0.f : 2.f;
    shot.vDir = preceding;
    shot.nBullet = 0;
    Require(NRPG::PrepareAttackRay(nullptr, cover, &shot, score) && shot.vDir == orthogonal,
        "first burst shot did not use the ordinary deviation weights");
    shot.nBullet = 1;
    shot.vDir = preceding;
    Require(NRPG::PrepareAttackRay(nullptr, cover, &shot, score) && shot.vDir == preceding &&
        shot.bTargetIsHit == !miss, "next burst shot ignored the preceding bullet direction");
    shot.vDir = VNULL3;
    Require(NRPG::PrepareAttackRay(nullptr, cover, &shot, score) && shot.vDir == orthogonal,
        "a restored burst without a previous direction could not use ordinary deviation weights");
}

void BurstRayWeightOperationOrder(bool miss)
{
    const float diagonal = 0.70710677f;
    const CVec3 previous(0.57735026f, 0.57735026f, 0.57735026f);
    const CVec3 zero(0.f, diagonal, -diagonal);
    const CVec3 cancelled(diagonal, -diagonal, 1e-8f);
    const vector<CoverParitySnapshot::Ray> rays = {{zero, true, 0.2f}, {cancelled, true, 0.8f}};
    auto cover = miss ? RestoreCoverRays({}, rays) : RestoreCoverRays(rays, {});
    CRay ray;
    ray.ptDir = previous;
    bool missed = false;
    // Both retail dot products cancel to zero. The alternate addition order leaves
    // a positive residue in the second ray and changes the roulette result.
    Require(NRPG::PeekRay(cover, &ray, miss ? 0.f : 2.f, &missed, true) && ray.ptDir == zero,
        "burst ray weight accumulated x and y before z and changed the selected ray");
}

void UnitKnifeIllumination(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    const auto target = std::find_if(units.begin(), units.end(), [&](const auto& unit) {
        return IsValid(unit) && unit != actor && unit->CanFight() && !unit->IsEmptyPK() &&
            world->GetDiplomacyState(unit->GetPlayer(), actor->GetPlayer()) == NDb::DS_ENEMY;
    });
    Require(target != units.end() && actor->GetScriptToHit() < 0,
        "knife illumination fixture lacks its ordinary shooter and hostile target");
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    NDb::CRPGMeleeWeapon* record = nullptr;
    CDBIterator<NDb::CRPGMeleeWeapon> blades(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
    while (blades.MoveNext()) {
        if (blades.Get()->bThrowing && IsValid(blades.Get()->pItem)) {
            record = blades.Get();
            break;
        }
    }
    Require(record, "knife illumination fixture lacks an authored throwing blade");
    CObj<NRPG::CMeleeWeaponItem> knife = new NRPG::CMeleeWeaponItem(record);
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    Require(inventory->Equip(NDb::SLOT_1, knife) && inventory->Activate(NDb::SLOT_1) &&
        NRPG::GetToHitType(actor) == NRPG::TH_THROWING, "native accuracy does not recognize the equipped blade");
    const auto start = actor->GetPosition();
    const auto origin = actor->GetAttackOrigin(start);
    const auto enemy = (*target)->GetPosition().pos;
    const CVec3 bright(1.f, 1.f, 1.f);
    const CVec3 dark(0.f, 0.f, 0.f);
    bool tileReactedToLight = false;
    for (int distance : {3, 6, 9}) {
        const auto tile = [&](const CVec3& light) {
            return NRPG::GetTileToHit(actor, start.GetPose(), distance, origin, enemy.GetCP(),
                NAI::THL_LOWER, 0, 100, false, light);
        };
        const auto unit = [&](const CVec3& light) {
            return NRPG::GetToHit(actor, start.GetPose(), distance, origin, enemy, NAI::HL_ANY, 0,
                target->GetPtr(), {}, 100, false, light);
        };
        tileReactedToLight |= tile(bright) > tile(dark);
        Require(unit(dark) == unit(bright), "a unit-target knife preview inherited the tile illumination factor");
    }
    Require(tileReactedToLight, "knife illumination fixture has no unsaturated tile accuracy control");
}

static void FrozenItemArmorResistance(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    auto* book = NDb::GetRPGItem(438);
    Require(book && IsValid(book->pModel), "test requires the authored book model");
    CObj<NRPG::IInventoryItem> item = NRPG::CreateClueItem(book);
    CObj<NWorld::CDFrozenItem> body =
        world->AddFrozenItem(world->GetAIMap(), CVec3(2, 2, 0), QNULL, item, false, 0);
    Require(IsValid(body) && body->GetModel().pModel, "the dropped item has no world body");
    auto* armor = body->GetModel().pModel->pRPGArmor.GetPtr();
    Require(IsValid(armor) && IsValid(armor->pMaterial), "the dropped item has no authored armor material");
    struct RestoreResistance {
        NDb::CRPGMaterial* material;
        int value;
        int threshold;
        ~RestoreResistance() { material->nDR = value; material->nThreshold = threshold; }
    } restore{armor->pMaterial, armor->pMaterial->nDR, armor->pMaterial->nThreshold};

    NRPG::CAttackPortion hit(100000, 0, 0.f, 5, 5, 0);
    armor->pMaterial->nThreshold = 0;
    Require(hit.CalcStructDmg(world, armor, 0) == 5, "test hit does not deal its fixed amount of damage");
    armor->pMaterial->nDR = 0;
    Require(body->ProcessAttack(world, 0, &hit, CVec3(1, 0, 0), armor) == 0,
        "the dropped item took damage from its zero-resistance material");
    armor->pMaterial->nDR = 1;
    Require(body->ProcessAttack(world, 0, &hit, CVec3(1, 0, 0), armor) == 5,
        "a positive-resistance material stopped applying damage");
}

NDb::CRPGGrenade* FindModelledGrenade()
{
    NDb::CRPGGrenade* authored = nullptr;
    CDBIterator<NDb::CRPGGrenade> grenades(*NDatabase::GetTable<NDb::CRPGGrenade>());
    while (grenades.MoveNext()) {
        auto* candidate = grenades.Get();
        if (IsValid(candidate->pItem) && IsValid(candidate->pItem->pModel) &&
            (!authored || candidate->GetRecordID() < authored->GetRecordID()))
            authored = candidate;
    }
    Require(authored, "the fixture needs a modelled grenade");
    return authored;
}

list<NWorld::CExplosionMaster::SStartInfo> PendingExplosions(NWorld::CWorld* world)
{
    auto* master = world->GetExplosionMasterForHarness();
    Require(IsValid(master), "projectile fixture lacks its native blast scheduler");
    list<NWorld::CExplosionMaster::SStartInfo> pending;
    CStructureNetworkContext refs;
    auto fields = BorrowedObjectFields(master, &refs);
    CStructureSaver reader(fields, CStructureSaver::READ, &refs);
    reader.Add(5, &pending);
    return pending;
}

void ExplosionWitnessedKill(const char* databasePath, const char* resourcePath, bool visible)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* thrower = MovementPolicyActor(world);
    auto* player = thrower->GetTBSPlayer();
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    auto receiver = std::find_if(units.begin(), units.end(), [thrower](const auto& unit) {
        return IsValid(unit) && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() &&
            thrower->GetDiplomacyState(unit) == NDb::DS_ENEMY;
    });
    Require(receiver != units.end(), "explosion fixture lacks a native enemy receiver");
    auto* target = receiver->GetPtr();
    Require(IsValid(player), "explosion fixture lacks the thrower's native player");
    target->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP).SetValue(5);

    CStructureNetworkContext playerRefs;
    playerRefs.externalReferences = true;
    playerRefs.ids[target] = 1;
    playerRefs.refs[1] = target;
    playerRefs.nextID = 100;
    playerRefs.include = [](CObjectBase*) { return false; };
    list<CPtr<NWorld::CUnitServer>> witnesses;
    if (visible)
        witnesses.push_back(target);
    CMemoryStream state;
    { CStructureSaver writer(state, CStructureSaver::WRITE, &playerRefs); writer.Add(3, &witnesses); }
    state.Seek(0);
    { CStructureSaver reader(state, CStructureSaver::READ, &playerRefs);
      player->CPlayerBaseVision<NWorld::CUnitServer>::operator&(reader); }
    list<CPtr<NWorld::CUnit>> seen;
    player->GetVisible(&seen);
    Require((std::find(seen.begin(), seen.end(), static_cast<NWorld::CUnit*>(target)) != seen.end()) == visible,
        "native player vision fixture did not set the intended witness state");

    CObj<NDb::CRPGGrenade> grenade = new NDb::CRPGGrenade;
    grenade->nWaveNumber = 1;
    grenade->fWaveRadius = 1.f;
    grenade->fWaveDmgMin = grenade->fWaveDmgMax = 50.f;
    grenade->nFragmentNumber = 0;
    grenade->nFragmentAPA = 10000;
    grenade->nCriticalProbability = grenade->nCriticalDifficulty = 0;
    grenade->fStructureDamageCoeff = 0.f;
    grenade->fDecalRadius = 0.f;
    world->GetAIMap()->Sync();
    CObj<NWorld::CExplosionSpace> space = new NWorld::CExplosionSpace(world->GetAIMap());
    CObj<NWorld::CVoxelExplTracker> explosion = new NWorld::CVoxelExplTracker(space, target->GetPosition().GetCenter(),
        nullptr, grenade, thrower, NWorld::SPerkMineModifiers(), world);
    bool stillExpanding = true;
    for (int step = 0; step < 256 && stillExpanding; ++step) {
        NWorld::ResetBreakExplCalcs();
        stillExpanding = explosion->MakeSingleStep();
    }
    Require(!stillExpanding, "native grenade wave did not settle");
    explosion->MakeDamage();
    Require(IsValid(target) && target->IsDead() && explosion->IsFinished(),
        "native explosion wave did not kill its target and finish");
    int acknowledged = -1;
    CStructureNetworkContext blastRefs;
    auto fields = BorrowedObjectFields(explosion, &blastRefs);
    { CStructureSaver reader(fields, CStructureSaver::READ, &blastRefs); reader.Add(6, &acknowledged); }
    Require(acknowledged == int(visible), "native explosion counted a kill outside the thrower's field of knowledge");
}

void ExplosionModifierOwnership(const char* databasePath, const char* resourcePath, int mode)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* unit = actor->GetUnitRPG()->GetRPGUnit();
    auto* perk = NDb::GetDBPerk(0x30);
    Require(perk && !perk->params.empty() && perk->params.front() != 1.f,
        "explosive perk fixture lacks its authored structure modifier");
    RestoreSinglePerk(unit, perk->GetRecordID());

    const auto before = PendingExplosions(world).size();
    NWorld::SPerkMineModifiers placed;
    placed.fStructureDmgModifier = 0.5f;
    placed.fAEDmgModifier = 1.25f;
    placed.bAlwaysHumanCritical = true;
    auto* regular = FindModelledGrenade();
    NDb::CRPGEngGrenade* engineer = nullptr;
    CDBIterator<NDb::CRPGEngGrenade> engineers(*NDatabase::GetTable<NDb::CRPGEngGrenade>());
    if (engineers.MoveNext())
        engineer = engineers.Get();
    Require(regular && engineer, "explosive perk fixture lacks authored blast records");
    const auto origin = actor->GetPosition().GetCP() + CVec3(0.6f, 0.f, 1.f);
    if (mode == 0) {
        NDb::CRPGMine* record = nullptr;
        CDBIterator<NDb::CRPGMine> mines(*NDatabase::GetTable<NDb::CRPGMine>());
        while (mines.MoveNext()) {
            if (mines.Get()->pItem && mines.Get()->pItem->pModel && mines.Get()->pExplosion) {
                record = mines.Get();
                break;
            }
        }
        Require(record, "explosive perk fixture lacks an authored mine");
        CObj<NWorld::CMine> mine = new NWorld::CMine(world, origin, record, 20,
            actor->GetPosition().pos.GetFloor(), actor, 0);
        mine->SetPerkModifiers(placed);
        mine->GoBoom();
    } else if (mode == 1) {
        world->AddGrenadeExplosion(origin, engineer, 57, actor, nullptr, &placed);
    } else if (mode == 2) {
        world->AddGrenadeExplosion(origin, regular, actor);
    } else {
        SRand random(SRandomSeed(303));
        CObj<NDb::CModel> model = regular->pItem->pModel->CreateModel(&random);
        const auto release = world->GetTime()->GetValue();
        const bool timed = mode == 3;
        regular->nMaxDelay = 1;
        CObj<NWorld::IDynamicObject> grenade = dynamic_cast<NWorld::IDynamicObject*>(world->ThrowGrenade(
            origin, CVec3(0, 0, -3), release, 1.f, model, timed ? regular : nullptr, actor,
            timed ? nullptr : engineer));
        Require(grenade, "native world could not launch the perk-bearing grenade");
        bool finished = false;
        for (int elapsed = 0; elapsed <= 4000 && !finished; elapsed += 100) {
            world->GetTime()->Set(release + elapsed);
            MarkNewDGFrame();
            finished = grenade->Segment();
        }
        Require(finished, "native grenade never reached its detonation condition");
    }
    const auto pending = PendingExplosions(world);
    Require(pending.size() == before + 1 && pending.back().pThrower.GetPtr() == actor,
        "native blast queue lost the detonation or its attribution");
    const auto& modifiers = pending.back().modifiers;
    const float expectedStructure = mode <= 1 ? placed.fStructureDmgModifier :
        mode == 2 ? 1.f : perk->params.front();
    std::printf("native blast source=%d structure=%g area=%g critical=%d\n",
        mode, modifiers.fStructureDmgModifier, modifiers.fAEDmgModifier, modifiers.bAlwaysHumanCritical);
    Require(modifiers.fStructureDmgModifier == expectedStructure &&
        modifiers.fAEDmgModifier == (mode <= 1 ? placed.fAEDmgModifier : 1.f) &&
        modifiers.bAlwaysHumanCritical == (mode <= 1),
        "native blast queue replaced the caller's stored or default explosive modifiers");
    if (mode == 1)
        Require(pending.back().pEngGrenade.GetPtr() == engineer && pending.back().nEngSkill == 57,
            "native engineer blast changed its record or engineering skill");
}

auto CurrentProjectileObjects(NWorld::CWorld* world)
{
    std::set<NWorld::IDynamicObject*> existing;
    for (const auto& object : *world->GetMiscObjects())
        existing.insert(object.GetPtr());
    return existing;
}

void MeleeSwingSound(const char* databasePath, const char* resourcePath, int mode)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    CObj<NWorld::CUnitServer> shell;
    if (mode == 2) {
        CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
        NDb::CRPGPers* suit = nullptr;
        while (people.MoveNext()) {
            auto* candidate = people.Get();
            if (candidate->pPanzerklein && candidate->pModel && !candidate->pPanzerklein->bHasNoHead &&
                candidate->pPanzerklein->nMaxVP > 0) {
                suit = candidate;
                break;
            }
        }
        Require(suit, "melee sound fixture has no armored suit");
        auto* path = world->GetPathNetwork();
        vector<NAI::SPathPlace> places;
        path->GetNearPlaces(SSphere(actor->GetPosition().GetCP(), 20.f), &places);
        for (auto place : places) {
            place.SetPose(NAI::CM_STAND);
            if (!path->IsPassable(place) || path->IsLocked(place) || !NAI::IsBigLockerPlacePassable(path, place))
                continue;
            shell = world->AddUnitInGame(place, NRPG::CreateUnit(suit), nullptr);
            if (shell && shell->IsEmptyPK() && shell->WearAsPK(true)) {
                actor->SetPosition(shell->GetPosition());
                actor->FlipPanzerklein(shell, false, false);
                break;
            }
        }
        Require(actor->GetWearingDBPK() == suit->pPanzerklein.GetPtr(), "melee sound fixture could not equip the armored suit");
    }

    auto* noise = NDb::GetAISound(19);
    auto* audibleSound = NDb::GetSound(mode == 2 ? 0x4059 : 0x3285);
    Require(noise && audibleSound, "melee sound fixture is missing authored sounds");
    noise->fRadius = 4000.f;
    noise->bTileTypeIndependent = true;
    NDb::SAISound descriptor = {noise, 0, 1.f};
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* listener = nullptr;
    for (const auto& candidate : units) {
        if (!IsValid(candidate) || candidate.GetPtr() == actor || !candidate->CanFight())
            continue;
        candidate->SetAudible(actor, false);
        if (candidate->CanHearSound(actor->GetPosition().GetCP(), descriptor, actor)) {
            listener = candidate.GetPtr();
            break;
        }
    }
    Require(listener && !listener->IsAudible(actor), "melee sound fixture has no fresh native listener");

    const auto existing = CurrentProjectileObjects(world);
    const CVec3 from = actor->GetPosition().GetEyePosition();
    CRay ray;
    ray.ptOrigin = from;
    ray.ptDir = V3_AXIS_Z;
    CObj<NWorld::CExecMeleeTile> action = new NWorld::CExecMeleeTile(actor, from + V3_AXIS_Z);
    vector<NRPG::CAttackPortion> portions;
    if (mode != 0)
        portions.emplace_back(0, 0, 0.f, 0, 0, 0);
    action->PerformAttack(portions, ray);
    Require(listener->IsAudible(actor), "native melee swing produced no AI hearing event");

    ObjectStageAudio audio;
    for (const auto& object : *world->GetMiscObjects()) {
        if (existing.count(object.GetPtr()))
            continue;
        if (auto* sound = dynamic_cast<NWorld::C3DSound*>(object.GetPtr()))
            sound->Visit(&audio);
    }
    Require(std::count(audio.sounds.begin(), audio.sounds.end(), audibleSound) == 1,
        "native melee swing did not submit exactly one sound for the active armor");
}

void AccidentalShotPayload(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* mission = actor->GetUnitRPG();
    auto* inventory = mission->GetInventory();
    auto* record = NDatabase::GetTable<NDb::CRPGWeapon>()->GetRecord(26);
    Require(IsValid(record) && IsValid(record->pInnerClip), "accidental shot fixture has no loaded firearm");
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(record);
    Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1) &&
        weapon->SetShootMode(NDb::SM_Snap), "accidental shot fixture could not activate the firearm");
    actor->SetUndrawItem(false);
    actor->animator.SetActiveItem(true);
    actor->animator.SetWeaponAnimation(mission->GetWeaponType());
    actor->animator.SetWeaponName(record->szAnimName.c_str());
    actor->animator.PlaceUnit(actor->GetPosition());

    CDBIterator<NDb::CRPGGrenade> grenades(*NDatabase::GetTable<NDb::CRPGGrenade>());
    Require(grenades.MoveNext(), "accidental shot fixture has no authored grenade payload");
    NDb::CRPGGrenade* payload = grenades.Get();
    auto* clip = weapon->GetInnerClip();
    Require(clip && clip->GetDBAmmo() && weapon->GetAmmoQuantity() > 1,
        "accidental shot fixture has no loaded ammunition");
    clip->GetDBAmmo()->pExplosiveBullet = payload;
    record->nShotEffectType = 2;

    CObj<NWorld::CCommandExecute> shot = NWorld::CreateAccidentalShot(actor);
    Require(shot, "accidental shot could not find the real gun barrel");
    const auto before = CurrentProjectileObjects(world);
    const int ammo = weapon->GetAmmoQuantity();
    shot->Run();
    Require(weapon->GetAmmoQuantity() == ammo - 1, "the native critical shot did not consume its round");

    NWorld::IDynamicObject* carrier = nullptr;
    for (const auto& object : *world->GetMiscObjects()) {
        if (!before.count(object.GetPtr()) && pSSClasses->GetObjectTypeID(object.GetPtr()) == int(0xB18B1160)) {
            Require(!carrier, "the accidental single shot spawned multiple carriers");
            carrier = object;
        }
    }
    Require(carrier, "the native critical shot did not spawn a bullet carrier");
    CStructureNetworkContext refs;
    auto fields = BorrowedObjectFields(carrier, &refs);
    CStructureSaver reader(fields, CStructureSaver::READ, &refs);
    vector<NRPG::STrailPoint> trail;
    CPtr<NWorld::CUnitServer> shooter;
    CPtr<NDb::CRPGGrenade> loadedPayload;
    int effect = -1;
    reader.Add(11, &trail);
    reader.Add(13, &shooter);
    reader.Add(14, &loadedPayload);
    reader.Add(15, &effect);
    Require(!trail.empty() && !shooter && !trail.front().sAttack.pAttacker,
        "the accidental bullet was credited as an intentional attack");
    Require(loadedPayload == payload && effect == record->nShotEffectType,
        "the accidental bullet lost its loaded explosive payload or weapon impact selector");
}

NWorld::IDynamicObject* NewFlyingGrenade(NWorld::CWorld* world,
    const std::set<NWorld::IDynamicObject*>& existing)
{
    NWorld::IDynamicObject* launched = nullptr;
    for (const auto& object : *world->GetMiscObjects()) {
        if (existing.count(object.GetPtr()) || pSSClasses->GetObjectTypeID(object.GetPtr()) != int(0x118B1160))
            continue;
        Require(!launched, "native release created more than one flying grenade");
        launched = object;
    }
    Require(launched, "native release did not create a flying grenade");
    return launched;
}

CQuat InitialGrenadeRotation(NWorld::IDynamicObject* grenade, CVec3* center)
{
    CStructureNetworkContext refs;
    auto fields = BorrowedObjectFields(grenade, &refs);
    CObj<NAnimation::CAnimator> animator;
    { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(10, &animator); }
    Require(dynamic_cast<NAnimation::CASphereSet*>(animator.GetPtr()), "flying grenade lacks its native collision body");
    CStructureNetworkContext physicsRefs;
    auto physicsFields = BorrowedObjectFields(animator, &physicsRefs);
    CStructureSaver reader(physicsFields, CStructureSaver::READ, &physicsRefs);
    CQuat rotation;
    reader.Add(12, center);
    reader.Add(13, &rotation);
    return rotation;
}

void GrenadeReleaseOrientation(const char* databasePath, const char* resourcePath, bool engineer)
{
    NativeGrenadeThrowFixture fixture(databasePath, resourcePath, engineer);
    auto* world = fixture.world.GetPtr();
    auto* executor = fixture.executor.GetPtr();
    executor->Run();
    Require(executor->IsExecuting(), "the orientation fixture could not begin its native throw");
    const auto existing = CurrentProjectileObjects(world);
    GlobalGameRandom().SeedForHarness(207);
    executor->TimeLabelReached();
    auto* grenade = NewFlyingGrenade(world, existing);

    NWorld::SGrenadeParams trajectory;
    CStructureNetworkContext refs;
    auto fields = BorrowedObjectFields(executor, &refs);
    { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(3, &trajectory); }
    CVec3 forward = trajectory.vel;
    CVec3 right(forward.y, -forward.x, 0.f);
    Require(fabs2(right) > 0.01f, "native throw fixture lacks a nonvertical release direction");
    Normalize(&forward);
    Normalize(&right);
    CVec3 center;
    const auto rotation = InitialGrenadeRotation(grenade, &center);
    Require(center == trajectory.ptStart, "grenade rotation fixture changed the prepared mass center");
    Require(fabs2(rotation.Rotate(V3_AXIS_Z) - forward) < 1e-5f &&
        fabs2(rotation.Rotate(V3_AXIS_X) - right) < 1e-5f,
        "the native collision body ignored the grenade release direction");
}

void FlyingBossGrenadeOrigin(const char* databasePath, const char* resourcePath)
{
    NativeGrenadeThrowFixture fixture(databasePath, resourcePath, false, true);
    auto* actor = fixture.actor;
    auto* network = fixture.world->GetPathNetwork();
    NDb::CRPGPers* boss = nullptr;
    CDBIterator<NDb::CRPGPers> records(*NDatabase::GetTable<NDb::CRPGPers>());
    while (!boss && records.MoveNext()) {
        auto* candidate = records.Get();
        if (IsValid(candidate->pPanzerklein) && (candidate->szUserName == "Boss" ||
            (IsValid(candidate->pName) && candidate->pName->szStr == L"Boss")))
            boss = candidate;
    }
    Require(IsValid(boss), "grenade-origin fixture needs the authored Boss shell");
    vector<NAI::SPathPlace> nearby;
    network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(), 20.f), &nearby);
    const auto free = std::find_if(nearby.begin(), nearby.end(), [&](auto& point) {
        point.SetPose(NAI::CM_STAND);
        return network->IsPassable(point) && !network->IsLocked(point) && NAI::IsBigLockerPlacePassable(network, point);
    });
    Require(free != nearby.end(), "grenade-origin fixture needs a free Boss-sized footprint");
    auto* shell = fixture.world->AddUnitInGame(*free, NRPG::CreateUnit(boss), nullptr);
    Require(IsValid(shell) && shell->IsEmptyPK(), "grenade-origin fixture did not spawn the native Boss shell");
    actor->FlipPanzerklein(shell, false, false);
    Require(actor->IsFlyingBoss() && actor->GetUnitRPG()->GetInventory()->GetActive() == fixture.grenade,
        "grenade-origin fixture did not keep its active grenade while wearing the Boss shell");

    const auto position = actor->GetPosition();
    CObj<NWorld::CExecThrowGrenade> executor;
    for (const auto& offset : {CVec3(4.f, 0, 0), CVec3(-4.f, 0, 0), CVec3(0, 4.f, 0), CVec3(0, -4.f, 0)}) {
        CObj<NWorld::CExecThrowGrenade> trial = new NWorld::CExecThrowGrenade(actor, position.GetCP() + offset);
        if (trial->CanDoIt(position) == NWorld::UCR_OK && actor->CanSpendAP(trial->GetStartAP())) {
            executor = trial;
            break;
        }
    }
    Require(IsValid(executor), "the Boss has no unblocked native grenade throw in the fixture");
    executor->Run();
    Require(executor->IsExecuting(), "the Boss did not start its valid native grenade throw");
    NWorld::SGrenadeParams trajectory;
    CStructureNetworkContext refs;
    auto fields = BorrowedObjectFields(executor, &refs);
    { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(3, &trajectory); }
    const float angle = position.GetDirection();
    const auto expected = position.GetCenter() + CVec3(-NWorld::F_GRENADE_CHECK_SIDE * cos(angle),
        -NWorld::F_GRENADE_CHECK_SIDE * sin(angle), 0.5f);
    Require(fabs2(trajectory.ptStart - expected) < 1e-6f && trajectory.nSide == 0,
        "Boss grenade launch used the standard left-hand origin instead of the raised rear origin");
}

CObj<NRPG::IClipItem> FixtureRocketAmmo()
{
    CDBIterator<NDb::CRPGAmmo> ammo(*NDatabase::GetTable<NDb::CRPGAmmo>());
    while (ammo.MoveNext()) {
        if (!IsValid(ammo.Get()->pExplosiveBullet))
            continue;
        CDBIterator<NDb::CRPGClip> clips(*NDatabase::GetTable<NDb::CRPGClip>());
        while (clips.MoveNext()) {
            if (IsValid(clips.Get()->pItem) && clips.Get()->nAmmoGroup == ammo.Get()->nAmmoGroup)
                return dynamic_cast<NRPG::IClipItem*>(NRPG::CreateClipItem(clips.Get(), ammo.Get(), 1));
        }
    }
    throw std::runtime_error("the projectile fixture needs native rocket ammunition");
}

class RocketLaunchProbe : public NWorld::CExecLaunchRocket
{
public:
    using CExecLaunchRocket::CExecLaunchRocket;
    const NRPG::SAttackRayInfo& GetLaunchRay() const { return rayInfo; }
};

void RocketLaunchSolve(const char* databasePath, const char* resourcePath, bool trailOnly)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* mission = actor->GetUnitRPG();
    auto* inventory = mission->GetInventory();
    CObj<NRPG::CWeaponItem> weapon;
    CDBIterator<NDb::CRPGWeapon> rows(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while (rows.MoveNext()) {
        auto* candidate = rows.Get();
        if (!candidate->bBazookaLogic || candidate->nPanzerkleinType || !candidate->pItem ||
            !candidate->pTrailEffect || !candidate->pInnerClip || !candidate->pAnimWeaponType || candidate->fTrailSpeed <= 0)
            continue;
        CObj<NRPG::CWeaponItem> loaded = new NRPG::CWeaponItem(candidate);
        if (loaded->GetAmmoQuantity() > 0) {
            weapon = loaded;
            break;
        }
    }
    Require(weapon, "rocket solve fixture has no loaded native launcher");
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1),
        "rocket solve fixture could not activate the native launcher");
    auto* record = weapon->GetDBWeapon();
    constexpr STime delay = 100;
    record->pAnimWeaponType->nBulletDelay = delay;
    actor->SetUndrawItem(false);
    actor->SetScriptToHit(100);
    actor->animator.SetActiveItem(true);
    actor->animator.SetWeaponAnimation(mission->GetWeaponType());
    actor->animator.SetWeaponName(record->szAnimName.c_str());
    world->GivePlayerTurn(actor->GetTBSPlayer());
    mission->StartNewTurn(actor->GetPosition().GetCP());
    const auto from = actor->GetPosition();
    const CVec3 origin = actor->GetAttackOrigin(from);
    const float clearance = actor->GetMinClearDistance();
    Require(clearance > 0.f, "rocket solve fixture cannot distinguish a second muzzle offset");

    vector<NRPG::CAttackPortion> attack;
    Require(mission->CreateAttack(&attack, false) && !attack.empty(), "rocket solve fixture has no native attack portion");
    attack.front().nK = 1;
    auto* path = world->GetPathNetwork();
    vector<NAI::SPathPlace> places;
    path->GetNearPlaces(SSphere(from.GetCP(), 16.f), &places);
    CObj<RocketLaunchProbe> shot;
    for (const auto& place : places) {
        if (!place.IsIntegral() || place.IsFinal())
            continue;
        const CVec3 point = NAI::SPosition(place, path).GetCP() + CVec3(0, 0, 0.5f);
        if (fabs2(point - from.GetCP()) < 25.f)
            continue;
        CObj<RocketLaunchProbe> candidate = new RocketLaunchProbe(actor, point);
        if (candidate->CanDoIt(from) != NWorld::UCR_OK)
            continue;
        CObj<NRPG::CCoverInfo> coverage = world->GetGame()->CalcCoversForTile(origin, attack.front(), actor, point,
            clearance, 0x8210);
        if (!CoverSnapshot(coverage).hits.empty()) {
            shot = candidate;
            break;
        }
    }
    Require(shot, "rocket solve fixture has no reachable native tile with a hit ray");
    shot->Run();
    auto* time = world->GetTime();
    const STime label = actor->animator.GetTimeLabel1();
    const STime arm = (std::max)(time->GetValue() + 1, label);
    time->Set(arm);
    Require(shot->TimeLabelReached(), "normal rocket animation failed to arm its delayed launch");
    const auto existing = CurrentProjectileObjects(world);
    const int ammo = weapon->GetAmmoQuantity();
    time->Set(arm + delay - 1);
    shot->Segment();
    Require(weapon->GetAmmoQuantity() == ammo && world->GetMiscObjects()->size() == existing.size(),
        "rocket fired before its native delayed tick");
    const STime launch = arm + delay + 25;
    time->Set(launch);
    shot->Segment();
    NWorld::IDynamicObject* carrier = nullptr;
    for (const auto& object : *world->GetMiscObjects())
        if (!existing.count(object.GetPtr()) && pSSClasses->GetObjectTypeID(object.GetPtr()) == int(0x73062152))
            carrier = object;
    Require(carrier && weapon->GetAmmoQuantity() == ammo - 1, "native delayed rocket did not launch and spend exactly one round");
    const auto& selected = shot->GetLaunchRay();
    if (trailOnly) {
        Require(selected.bTargetIsHit && !selected.trailPoints.empty() && selected.pUS == actor,
            "native rocket hit did not retain the chosen cover trail");
        return;
    }

    CStructureNetworkContext refs;
    auto fields = BorrowedObjectFields(carrier, &refs);
    CStructureSaver reader(fields, CStructureSaver::READ, &refs);
    CVec3 muzzle, velocity;
    STime finish = 0;
    reader.Add(2, &finish);
    reader.Add(10, &muzzle);
    reader.Add(11, &velocity);
    const STime expectedDuration = Float2Int((record->nMaxRange * FP_GRID_STEP) / fabs(velocity) * 1000.f);
    Require(finish == launch + expectedDuration, "rocket travel ended relative to the animation label rather than the launch tick");
    Require(fabs2(muzzle - origin) < 1e-6f && fabs2(selected.vOrigin - origin) < 1e-6f,
        "native rocket applied its muzzle clearance after selecting the launch origin");
}

void RocketImpactPoint(const char* databasePath, const char* resourcePath, bool expires)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    SRand random(SRandomSeed(607));
    CObj<NDb::CModel> model = FindModelledGrenade()->pItem->pModel->CreateModel(&random);
    auto ammo = FixtureRocketAmmo();
    Require(IsValid(model) && IsValid(model->pGeometry) && IsValid(ammo), "rocket fixture lacks its model or payload");
    const auto origin = actor->GetPosition().GetCP() + CVec3(0.6f, 0.f, expires ? 50.f : 4.f);
    const CVec3 direction = expires ? V3_AXIS_Y : -V3_AXIS_Z;
    CRay ray;
    ray.ptOrigin = origin;
    ray.ptDir = direction;
    vector<NAI::SInterval> obstacles;
    world->GetAIMap()->Sync();
    world->GetAIMap()->Trace(ray, &obstacles, NWorld::TS_FRAGMENTED);
    const auto collision = std::find_if(obstacles.begin(), obstacles.end(), [&](const auto& obstacle) {
        return obstacle.enter.fT > 0.f && obstacle.pSrc->pUserData.GetPtr() != actor;
    });
    Require(expires ? collision == obstacles.end() || collision->enter.fT > 3.f : collision != obstacles.end(),
        "rocket fixture lacks the expected clear path or collision surface");
    const float distance = expires ? 2.f : collision->enter.fT + 1.f;
    const auto release = world->GetTime()->GetValue();
    NRPG::CAttackPortion attack(100000, 0, 0.f, 5, 5, 0);
    CObj<NWorld::IDynamicObject> rocket = dynamic_cast<NWorld::IDynamicObject*>(world->LaunchRocket(
        origin, direction * 10.f, release, distance, model, attack, ammo, actor, nullptr));
    Require(rocket && !rocket->Segment(), "rocket fixture did not begin a live flight");
    const auto existing = CurrentProjectileObjects(world);
    const auto before = PendingExplosions(world).size();
    world->GetTime()->Set(release + STime(std::ceil(distance * 100)) + (expires ? 20 : -25));
    MarkNewDGFrame();
    Require(rocket->Segment(), "native rocket did not retire on impact or expiry");
    const auto pending = PendingExplosions(world);
    if (expires) {
        Require(pending.size() == before, "a rocket expired in free flight but queued an immediate blast");
        CVec3 center;
        const auto orientation = InitialGrenadeRotation(NewFlyingGrenade(world, existing), &center);
        Require(center == origin && fabs2(orientation.Rotate(V3_AXIS_X) - direction) < 1e-5f,
            "the expired rocket lost its orientation on the continuing collision body");
    } else {
        Require(pending.size() == before + 1 && pending.back().pThrower.GetPtr() == actor,
            "native rocket impact did not queue one attributed blast");
        const CVec3 expected = origin + direction * (std::max)(0.f, collision->enter.fT - 0.05f) + CVec3(0, 0, 0.005f);
        Require(fabs2(pending.back().vCenter - expected) < 1e-6f,
            "native rocket explosion did not use the collision position");
    }
}

static void FlyingGrenadeLifetime(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    SRand random;
    CObj<NDb::CModel> model = FindModelledGrenade()->pItem->pModel->CreateModel(&random);
    Require(IsValid(model) && IsValid(model->pGeometry), "grenade lifetime fixture needs valid projectile geometry");
    CObj<NDb::CRPGGrenade> testGrenade = new NDb::CRPGGrenade;
    testGrenade->nMaxDelay = 20;
    const auto release = world->GetTime()->GetValue();
    const size_t beforeLaunch = world->GetMiscObjects()->size();
    CObj<NWorld::IDynamicObject> grenade = dynamic_cast<NWorld::IDynamicObject*>(
        world->ThrowGrenade(actor->GetPosition().GetEyePosition(), CVec3(1, 0, 1), release,
            0.f, model, testGrenade, actor));
    Require(grenade && world->GetMiscObjects()->size() == beforeLaunch + 1,
        "the real world did not create a flying grenade");
    world->GetTime()->Set(release + 15000);
    MarkNewDGFrame();
    Require(!grenade->Segment(), "a flying grenade expired before the lifetime boundary");
    world->GetTime()->Set(release + 15001);
    MarkNewDGFrame();
    Require(grenade->Segment(), "the flying grenade's expiry still depends on its fuse");
    Require(world->GetMiscObjects()->size() == beforeLaunch + 1,
        "the expired grenade emitted a new world object before its fuse");
}

static void BloodyDeathSoundPosition(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    CObj<NWorld::CUnitServer> actor = MovementPolicyActor(world);
    auto* persona = actor->GetUnitRPG()->GetRPGUnit()->GetAckHolder();
    if (!persona) persona = actor->GetUnitRPG()->GetRPGPers();
    const auto* deathSound = persona->pSoundDeath.GetPtr();
    Require(deathSound && !deathSound->variants.empty() && NDb::GetTEffect(822),
        "bloody-death fixture requires the authored sound and effect");
    const auto position = actor->GetPosition();
    const auto eye = position.GetEyePosition();
    Require(fabs2(eye - position.GetCenter()) > 0.01f, "fixture needs distinct eye and center positions");
    std::set<NWorld::IDynamicObject*> existing;
    for (const auto& object : *world->GetMiscObjects()) existing.insert(object.GetPtr());

    actor->SetBloodyDeath(true);
    actor->GetUnitRPG()->GetRPGUnit()->Kill();
    actor->SyncConscious();

    ObjectStageAudio audio;
    for (const auto& object : *world->GetMiscObjects())
        if (!existing.count(object.GetPtr()))
            if (auto* sound = dynamic_cast<NWorld::C3DSound*>(object.GetPtr())) sound->Visit(&audio);
    const auto matches = [deathSound](auto* sound) {
        return sound && std::any_of(deathSound->variants.begin(), deathSound->variants.end(),
            [sound](const auto& variant) { return IsValid(variant) && variant->pSound == sound; });
    };
    Require(std::count_if(audio.sounds.begin(), audio.sounds.end(), matches) == 1,
        "bloody death did not emit exactly one detached persona death sound");
    const auto index = std::find_if(audio.sounds.begin(), audio.sounds.end(), matches) - audio.sounds.begin();
    CDGPtr<CFuncBase<CVec3>> soundPosition(audio.positions[index].GetPtr());
    soundPosition.Refresh();
    Require(fabs2(soundPosition->GetValue() - eye) < 0.00001f,
        "bloody death emitted its detached sound from the unit center");
}

static void ExplosionVoxelBudget(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    const auto center = actor->GetPosition().GetEyePosition();
    CObj<NWorld::CExplosionSpace> space = NWorld::CreateExplosionSpace(world->GetAIMap());
    CObj<NDb::CRPGGrenade> grenade = new NDb::CRPGGrenade;
    grenade->nWaveNumber = 1;
    int budgets[2] = {};
    for (int radius = 1; radius <= 2; ++radius) {
        grenade->fWaveRadius = radius * NWorld::F_VOXEL_SIZE / FP_GRID_STEP;
        CObj<NWorld::CVoxelExpl> wave = new NWorld::CVoxelExpl(
            world, center, nullptr, 0, grenade, actor, nullptr, space);
        CStructureNetworkContext refs;
        auto fields = BorrowedObjectFields(wave, &refs);
        CStructureSaver reader(fields, CStructureSaver::READ, &refs);
        reader.Add(9, &budgets[radius - 1]);
    }
    Require(budgets[0] == 4, "single-voxel-radius control changed its budget");
    Require(budgets[1] == 33, "a blast rounded its fractional voxel budget upward");
}

void InvalidProjectileResources(const char* databasePath, const char* resourcePath, int mode)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    const auto start = actor->GetPosition().GetEyePosition();
    const auto when = world->GetTime()->GetValue();
    const CVec3 velocity(1, 0, 1);
    auto* grenade = FindModelledGrenade();
    SRand random;
    CObj<NDb::CModel> model = grenade->pItem->pModel->CreateModel(&random);
    CObj<NDb::CModel> noGeometry = new NDb::CModel;
    Require(IsValid(model) && IsValid(model->pGeometry) && !noGeometry->pGeometry,
        "the projectile fixture did not supply contrasting geometries");
    const auto before = world->GetMiscObjects()->size();
    if (mode == 0) {
        Require(world->ThrowGrenade(start, velocity, when, 0.f, model, grenade, actor) &&
            world->GetMiscObjects()->size() == before + 1, "the fixture cannot launch a valid grenade");
        Require(!world->ThrowGrenade(start, velocity, when, 0.f, noGeometry, grenade, actor),
            "the world accepted a grenade without geometry");
    } else {
        auto rocket = FixtureRocketAmmo();
        Require(rocket && rocket->GetIncQuantity() == 1, "the fixture needs a live rocket ammunition record");
        NRPG::CAttackPortion hit(100000, 0, 0.f, 5, 5, 0);
        Require(world->LaunchRocket(start, velocity, when, 3.f, model, hit, rocket, actor, nullptr) &&
            world->GetMiscObjects()->size() == before + 1, "the fixture cannot launch a valid rocket");
        Require(!world->LaunchRocket(start, velocity, when, 3.f, mode == 1 ? noGeometry.GetPtr() : model.GetPtr(),
            hit, mode == 2 ? nullptr : rocket, actor, nullptr), "the world accepted a rocket with missing resources");
    }
    Require(world->GetMiscObjects()->size() == before + 1, "the rejected launch changed the world projectile list");
}

void StuckKnifeFloor(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, false, nullptr, 4517);
    auto* map = world->GetAIMap();
    map->Sync();
    int floor = 0;
    float distance = 0.f;
    CRay shot;
    for (const auto& object : world->GetObjectsForHarness()) {
        SBound bounds;
        if (!map->GetObjectBound(&bounds, object)) continue;
        for (int heading = 0; heading < 16 && !floor; ++heading) {
            const float angle = heading * FP_2PI / 16;
            CRay ray;
            ray.ptDir = CVec3(std::cos(angle), std::sin(angle), 0);
            ray.ptOrigin = bounds.s.ptCenter - ray.ptDir * (bounds.s.fRadius + 0.4f);
            vector<NAI::SInterval> hits;
            map->Trace(ray, &hits, NWorld::TS_FRAGMENTED | NWorld::TS_ITEM_BLOCKER);
            const NAI::SInterval* first = nullptr;
            for (const auto& hit : hits)
                if (hit.enter.fT > 0.f && (!first || hit.enter.fT < first->enter.fT))
                    first = &hit;
            if (!first || first->pSrc->pUserData != object.GetPtr() || !first->pSrc->nFloor ||
                (first->pSrc->nTSFlags & (NWorld::TS_STATE_OPEN | NWorld::TS_STATE_CLOSED)) ||
                !first->pSrc->pArmor || !first->pSrc->pArmor->pMaterial)
                continue;
            const int resistance = first->pSrc->pArmor->pMaterial->nDR;
            if (resistance != 1 && resistance != 2) continue;
            floor = first->pSrc->nFloor;
            distance = first->enter.fT;
            shot = ray;
        }
        if (floor) break;
    }
    Require(floor != 0, "the authored world has no clear knife ray to a non-ground wood surface");
    NDb::CRPGMeleeWeapon* record = nullptr;
    CDBIterator<NDb::CRPGMeleeWeapon> blades(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
    while (blades.MoveNext()) {
        if (blades.Get()->bThrowing && IsValid(blades.Get()->pItem) && IsValid(blades.Get()->pItem->pModel)) {
            record = blades.Get();
            break;
        }
    }
    Require(record, "the fixture has no authored throwing knife");
    SRand random;
    CObj<NDb::CModel> model = record->pItem->pModel->CreateModel(&random);
    CObj<NRPG::CMeleeWeaponItem> item = new NRPG::CMeleeWeaponItem(record);
    item->nQuantity = 1;
    NRPG::CAttackPortion hit(100000, 0, 0.f, 0, 0, 0);
    NRPG::SAttackRayInfo ray(hit, shot.ptOrigin, shot.ptDir, false, 0.f, distance + 1.f, nullptr);
    const auto start = world->GetTime()->GetValue();
    CObj<NWorld::IDynamicObject> knife = dynamic_cast<NWorld::IDynamicObject*>(
        world->ThrowKnife(ray, 10.f, start, distance + 1.f, model, item));
    Require(knife && !knife->Segment() && !world->GetFrozenItem(item),
        "the knife fixture hit before the projectile advanced");
    world->GetTime()->Set(start + STime(std::ceil((distance + 0.05f) * 100)));
    MarkNewDGFrame();
    Require(knife->Segment(), "the knife did not retire on the wood surface");
    const auto* frozen = world->GetFrozenItem(item);
    Require(frozen && frozen->GetFloor() == floor, "the stuck knife lost the floor of the surface it hit");
}

void ShotRayWithoutCoverCandidates(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* map = world->GetAIMap();
    map->Sync();
    const CVec3 source = actor->GetAttackOrigin(actor->GetPosition());
    const CVec3 target = source - CVec3(0.3f, 0.2f, 10.f);
    NRPG::CAttackPortion attack(100000, 0, 0.f, 5, 5, 0);
    const int mask = NWorld::TS_COVER | NWorld::TS_ITEM_BLOCKER | NWorld::TS_FRAGMENTED;

    CObj<NRPG::CCoverInfo> unrestricted = world->GetGame()->CalcCoversForTile(source, attack, actor, target, 0.f, mask);
    Require(NRPG::CanShoot(unrestricted), "fixture has no usable rays without the muzzle clearance gate");
    NRPG::SAttackRayInfo ray(attack, VNULL3, VNULL3, false, 0.f, 30.f, nullptr);
    Require(NRPG::PrepareAttackRay(map, unrestricted, &ray, 1.f) && fabs2(ray.vDir - V3_AXIS_Z) > 1.f,
        "control shot did not select a real trajectory toward the target");

    CObj<NRPG::CCoverInfo> blocked = world->GetGame()->CalcCoversForTile(source, attack, actor, target, 4.f, mask);
    Require(!NRPG::CanShoot(blocked), "nearby terrain did not block the entire muzzle cone");
    ray.vOrigin = VNULL3;
    Require(!NRPG::PrepareAttackRay(map, blocked, &ray, 1.f) && !ray.bTargetIsHit && ray.trailPoints.empty(),
        "a blocked muzzle yielded a selected ray, a hit or a stale hit trail");
    Require(ray.vOrigin == source && ray.vDir == V3_AXIS_Z,
        "a blocked muzzle did not return the finite upward miss from the attack origin");
}

void RangedHitRollRange(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    const auto shooter = std::find_if(units.begin(), units.end(), [](const auto& unit) {
        if (!unit || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK()) return false;
        auto* weapon = dynamic_cast<NRPG::IWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        return weapon && weapon->GetAmmoQuantity() > 0 && weapon->SetShootMode(NDb::SM_Snap);
    });
    Require(shooter != units.end(), "hit roll fixture lacks an equipped firearm");
    auto* actor = shooter->GetPtr();
    auto* mission = actor->GetUnitRPG();
    vector<NRPG::CAttackPortion> attack;
    Require(mission->CreateAttack(&attack, false) && !attack.empty(), "fixture could not quote a native shot");
    const auto position = actor->GetPosition();
    const auto source = actor->GetAttackOrigin(position);
    CObj<NRPG::CCoverInfo> cover;
    CVec3 target;
    const bool firstTurn = world->IsFirstTurn();
    int expectedChance = 0;
    for (float distance : {2.f, 4.f, 8.f, 12.f}) {
        target = position.GetCP() + CVec3(distance, 0.f, 0.1f);
        cover = world->GetGame()->CalcCoversForTile(source, attack.front(), actor, target, actor->GetMinClearDistance());
        if (!NRPG::CanShoot(cover)) continue;
        NRPG::CheckTileToHit(actor, target, 0, NAI::THL_LOWER, cover, firstTurn, &expectedChance);
        if (expectedChance > 0 && expectedChance < 100) break;
    }
    Require(expectedChance > 0 && expectedChance < 100, "fixture has no shot with a nondegenerate hit chance");

    GlobalGameRandom().SeedForHarness(0x5201);
    int previous = -10;
    for (int sample = 0; sample < 400; ++sample) {
        int chance = -1;
        const float result = NRPG::CheckTileToHit(actor, target, 0, NAI::THL_LOWER, cover, firstTurn, &chance);
        Require(chance == expectedChance && std::isfinite(result) && result > 0.f,
            "the native hit check divided by a zero roll");
        const int roll = int(std::lround(chance / result));
        Require(roll >= 1 && roll <= 99 && std::abs(roll - previous) >= 6,
            "hit roll left the original range or repeated a nearby value");
        previous = roll;
    }
}
}

bool RunProjectileTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "flying-boss-grenade-origin") && argc == 4) { FlyingBossGrenadeOrigin(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "melee-zero-cover") && argc == 4) { MeleeZeroCover(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "melee-swing-sound") && argc == 5) { MeleeSwingSound(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "accidental-shot-payload") && argc == 4) { AccidentalShotPayload(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "shooting-completion") && argc == 5) { ShootingCompletion(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "explosion-witnessed-kill") && argc == 5) { ExplosionWitnessedKill(argv[2], argv[3], std::atoi(argv[4]) != 0); return true; }
    if (!std::strcmp(mode, "boss-weapon-state") && argc == 4) { BossWeaponState(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "sniper-target-policy") && argc == 5) { SniperTargetPolicy(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "disarm-callback") && argc == 6) { DisarmCallback(argv[2], argv[3], std::atoi(argv[4]) != 0, std::atoi(argv[5]) != 0); return true; }
    if (!std::strcmp(mode, "explosion-modifier-ownership") && argc == 5) { ExplosionModifierOwnership(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "grenade-release-orientation") && argc == 5) { GrenadeReleaseOrientation(argv[2], argv[3], std::atoi(argv[4]) != 0); return true; }
    if (!std::strcmp(mode, "rocket-launch-solve") && argc == 5) { RocketLaunchSolve(argv[2], argv[3], std::atoi(argv[4]) != 0); return true; }
    if (!std::strcmp(mode, "rocket-impact-point") && argc == 5) { RocketImpactPoint(argv[2], argv[3], std::atoi(argv[4]) != 0); return true; }
    if (!std::strcmp(mode, "cover-ray-row-order") && argc == 4) { CoverRayRowOrder(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "equal-miss-ray-order") && argc == 2) { EqualMissRayOrder(); return true; }
    if (!std::strcmp(mode, "burst-ray-previous") && argc == 3) { BurstRayFollowsPrevious(std::atoi(argv[2]) != 0); return true; }
    if (!std::strcmp(mode, "burst-ray-weight-order") && argc == 3) { BurstRayWeightOperationOrder(std::atoi(argv[2]) != 0); return true; }
    if (!std::strcmp(mode, "unit-knife-illumination") && argc == 4) { UnitKnifeIllumination(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "frozen-item-armor-resistance") && argc == 4) { FrozenItemArmorResistance(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "flying-grenade-lifetime") && argc == 4) { FlyingGrenadeLifetime(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "bloody-death-sound-position") && argc == 4) { BloodyDeathSoundPosition(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "explosion-voxel-budget") && argc == 4) { ExplosionVoxelBudget(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "invalid-projectile-resources") && argc == 5) { InvalidProjectileResources(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "stuck-knife-floor") && argc == 4) { StuckKnifeFloor(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "shot-ray-without-cover-candidates") && argc == 4) { ShotRayWithoutCoverCandidates(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ranged-hit-roll-range") && argc == 4) { RangedHitRollRange(argv[2], argv[3]); return true; }
    return false;
}
}
