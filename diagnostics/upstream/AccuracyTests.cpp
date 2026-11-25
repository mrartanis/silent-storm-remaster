#include "TestSupport.h"
#include "NativeCoverSnapshot.h"
#include "../../Main/aiMap.h"
#include "../../Main/aiPath.h"
#include "../../Main/RPGAttackMech.h"
#include "../../Main/RPGGame.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGToHit.h"
#include "../../Main/wMain.h"
#include "../../Main/wUnitServer.h"
#include "../../DBFormat/DataRPG.h"
#include "../../Main/wUnitAttack.h"
#include "../../Main/wUnitAttackExec.h"
#include "../../Main/aiInventory.h"
#include "../../Main/aiMisc.h"
#include "../../Main/aiUnit.h"
#include "../../Main/aiWeapon.h"
#include <cstring>

namespace UpstreamTests
{
namespace
{
void DistantMeleeApproach(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* network = world->GetPathNetwork();
    const auto origin = actor->GetPosition().GetCP();
    vector<NAI::SPathPlace> nearby;
    network->GetNearPlaces(SSphere(origin, 15.f), &nearby);
    const auto farPlace = std::find_if(nearby.begin(), nearby.end(), [&](auto& place) {
        place.SetPose(NAI::CM_STAND);
        const auto offset = NAI::SPosition(place, network).GetCP() - origin;
        return fabs(offset.z) < 0.1f && fabs(offset) > 8.f && network->IsPassable(place);
    });
    Require(farPlace != nearby.end(), "distant melee fixture needs free ground beyond the attacker search radius");
    const auto destination = NAI::SPosition(*farPlace, network).GetCP();
    vector<NAI::SPathPlace> approaches;
    NWorld::GetMeleeAttackPlaces(actor, origin, &approaches);
    Require(!approaches.empty(), "nearby melee target has no approaches in the native control");
    approaches.clear();
    NWorld::GetMeleeAttackPlaces(actor, destination, &approaches);
    Require(!approaches.empty(), "distant melee target has no approaches because search is centered on the attacker");
    Require(std::all_of(approaches.begin(), approaches.end(), [&](const auto& place) {
        return NWorld::IsWithinHumanReach(NAI::SPosition(place, network).GetCP(), destination, F_MELEE_DISTANCE);
    }), "melee approach search returned a point outside the attacker's reach from the target");
}
NWorld::CUnitServer* EquipAccuracyActor(NWorld::CWorld* world, NDb::CRPGWeapon* weaponRecord = nullptr)
{
    auto* actor = MovementPolicyActor(world);
    auto* mission = actor->GetUnitRPG();
    auto* inventory = mission->GetInventory();
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    auto* record = weaponRecord ? weaponRecord : NDatabase::GetTable<NDb::CRPGWeapon>()->GetRecord(26);
    Require(IsValid(record) && IsValid(record->pInnerClip), "cover precision fixture lacks a loaded native weapon");
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(record);
    Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1) &&
        weapon->SetShootMode(NDb::SM_Snap), "cover precision fixture could not activate the native weapon");
    actor->SetUndrawItem(false);
    actor->animator.SetActiveItem(true);
    actor->animator.SetWeaponAnimation(mission->GetWeaponType());
    actor->animator.SetWeaponName(record->szAnimName.c_str());
    return actor;
}

void FractionalUnitCover(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = EquipAccuracyActor(world);
    auto* mission = actor->GetUnitRPG();
    vector<NRPG::CAttackPortion> attacks;
    Require(mission->CreateAttack(&attacks, false) && !attacks.empty() && attacks.front().nK > 0,
        "cover precision fixture has no native piercing attack");
    const auto position = actor->GetPosition();
    const auto origin = actor->GetAttackOrigin(position);
    const int piercing = attacks.front().nK;
    const bool first = mission->IsFirstTurn();
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    for (const auto& target : units) {
        if (!IsValid(target) || target.GetPtr() == actor || !target->CanFight())
            continue;
        CObj<NRPG::CCoverInfo> cover = world->GetGame()->CalcCovers(origin, attacks.front(), actor, target,
            NAI::HL_ANY, actor->GetMinClearDistance());
        const auto snapshot = CoverSnapshot(cover);
        const auto penetrating = std::count_if(snapshot.hits.begin(), snapshot.hits.end(),
            [](const auto& ray) { return ray.penetrate; });
        if (penetrating == 0)
            continue;
        const float average = (snapshot.piercing / float(penetrating)) / piercing;
        const float precise = (100.f * penetrating / snapshot.hits.size()) * average;
        const float legacy = int(float((100 * penetrating) / snapshot.hits.size()) * average);
        if (precise == legacy)
            continue;
        const auto targetPosition = target->GetPosition();
        const int distance = int(fabs(origin - targetPosition.GetCP()) * FP_INV_GRID_STEP);
        const bool backstab = !target->IsUnitVisible(actor);
        const bool night = world->GetGame()->IsNight();
        auto calculate = [&](float coverage) {
            CObj<NRPG::CUnitToHitCalcer> calcer = new NRPG::CUnitToHitCalcer(actor, position.GetPose(), distance, origin, targetPosition.pos,
                0, 0, coverage, first, night, CVec3(1, 1, 1), NAI::HL_ANY, target, 0, backstab);
            return calcer->GetToHit();
        };
        for (int skill = 10; skill <= 130; ++skill) {
            mission->GetRPGUnit()->Skills(NDb::ST_SHOOTING).SetConst(skill);
            const int expected = calculate(precise);
            const int truncatedCoverResult = calculate(legacy);
            if (expected == truncatedCoverResult)
                continue;
            int actual = 0;
            NRPG::CheckToHit(actor, target, 0, NAI::HL_ANY, {}, cover, first, &actual);
            Require(actual == expected, "native unit shot discarded fractional cover before calculating its hit chance");
            return;
        }
    }
    Require(false, "native map cover fixture has no hit chance sensitive to fractional piercing");
}

void NativeHitArithmetic(const char* databasePath, const char* resourcePath, int test)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    if (test == 2) {
        auto rain = NWorld::IWorld::WEATHER_RAIN;
        CMemoryStream fields;
        { CStructureSaver writer(fields, CStructureSaver::WRITE); writer.Add(2, &rain); }
        fields.Seek(0);
        CStructureSaver reader(fields, CStructureSaver::READ);
        static_cast<NWorld::CWeatherTracker&>(*world).operator&(reader);
    }
    auto* actor = EquipAccuracyActor(world);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    const auto target = std::find_if(units.begin(), units.end(), [&](const auto& unit) {
        return IsValid(unit) && unit.GetPtr() != actor && unit->CanFight();
    });
    Require(target != units.end(), "accuracy fixture has no living target");
    const auto position = actor->GetPosition();
    const auto origin = actor->GetAttackOrigin(position);
    const auto destination = (*target)->GetPosition();
    const int distance = test == 2 ? 6 : 21;
    CObj<NRPG::CTileToHitCalcer> tile = new NRPG::CTileToHitCalcer(actor, position.GetPose(), distance,
        origin, 0, 67.29f, true, true, VNULL3, destination.GetCenter(), 0);
    CObj<NRPG::CUnitToHitCalcer> unit = new NRPG::CUnitToHitCalcer(actor, position.GetPose(), distance,
        origin, destination.pos, 0, 0, 67.29f, true, true, VNULL3, NAI::HL_ANY, *target, 0, false);
    auto* mission = actor->GetUnitRPG();
    auto* calcer = test == 0 ? static_cast<NRPG::CToHitCalcer*>(tile.GetPtr()) : unit.GetPtr();
    if (test == 2)
        mission->GetToHitConstants()->fICModifier = 3.17f;
    bool observed = false;
    bool observedCover = false;
    bool observedUnit = false;
    const int maxSkill = test == 2 ? 160 : 100;
    for (int skill = 25; skill <= maxSkill; skill += 3) {
        mission->GetRPGUnit()->Skills(NDb::ST_SHOOTING).SetConst(skill);
        if (test == 2)
            (*target)->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_IC).SetConst(skill / 2);
        for (int step = 1; step < 30; ++step) {
            const float light = step / 30.f;
            calcer->ptIllumination = CVec3(light, light, light);
            const int hit = calcer->GetToHit();
            if (test < 2) {
                const float raw = calcer->fToHit;
                if (raw > 2.f && raw < 99.f && raw - int(raw) >= 0.5f) {
                    Require(hit == int(raw), "native firearm chance rounded up instead of truncating");
                    return;
                }
                continue;
            }
            const float add = unit->GetAllAdd();
            const float firstRound = unit->GetFRMult();
            const float illumination = unit->GetLight();
            const float attackMultiplier = std::max(0.f, (add * firstRound) * illumination);
            if (attackMultiplier != std::max(0.f, (add * illumination) * firstRound)) {
                observed = true;
                Require(unit->GetAllMult() == attackMultiplier, "native shot applied light before first-round penalty");
            }
            float coverPerk = 1.f;
            mission->HasPerk(0x48, &coverPerk);
            const float scope = unit->sWeaponInfo.fScopeFactor;
            const float range = (scope + 1.f) * 100.f / (distance + scope);
            const float area = unit->GetTArea();
            unit->fHitCover = (3700.f + step * 29.f) / 47.f;
            const float coverage = ((100.f - range) * ((unit->fHitCover * 0.01f * coverPerk) * area) + range) * 0.01f;
            const float byDivision = ((100.f - range) * ((unit->fHitCover / 100.f * coverPerk) * area) + range) / 100.f;
            if (coverage != byDivision) {
                observedCover = true;
                Require(unit->CToHitCalcer::GetCA() == coverage, "native cover conversion lost original float operation order");
            }
            const float defense = unit->GetTAuraEvasionAdd() +
                (*target)->GetUnitRPG()->GetIC() / mission->GetToHitConstants()->fICModifier;
            const float combined = unit->GetCA() * unit->GetAllMult() + unit->GetCarefulShootPerk();
            const float aura = unit->GetAuraToHitAdd();
            const float weather = unit->GetWeatherPenalty();
            const float expected = std::clamp(float(double(combined) + aura - weather - defense), 2.f, 100.f);
            const float previous = std::clamp(combined + aura - defense - weather, 2.f, 100.f);
            if (expected != previous) {
                observedUnit = true;
                unit->GetToHit();
                Require(unit->fToHit == expected, "native unit shot applied defense before weather");
            }
        }
    }
    Require(test == 2 && observed, "native accuracy fixture did not distinguish truncation or first-round order");
    Require(observedCover, "native accuracy fixture did not distinguish original cover multiplication");
    Require(observedUnit, "native accuracy fixture did not distinguish original unit defense and weather order");
}

void NativeAIShotInput(const char* databasePath, const char* resourcePath, bool launcher)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    NDb::CRPGWeapon* record = nullptr;
    if (launcher) {
        CDBIterator<NDb::CRPGWeapon> rows(*NDatabase::GetTable<NDb::CRPGWeapon>());
        while (!record && rows.MoveNext()) {
            auto* candidate = rows.Get();
            if (candidate->bBazookaLogic && !candidate->nPanzerkleinType && IsValid(candidate->pItem) &&
                IsValid(candidate->pInnerClip) && candidate->shootModes[NDb::SM_Snap])
                record = candidate;
        }
        Require(IsValid(record), "AI shot fixture needs a loaded handheld launcher");
    }
    auto* actor = EquipAccuracyActor(world, record);
    auto* shooter = NAI::GetAIUnit(actor);
    Require(IsValid(shooter), "AI shot fixture lacks the native shooter wrapper");
    shooter->Synchronize(false);
    auto* item = dynamic_cast<NRPG::CWeaponItem*>(actor->GetUnitRPG()->GetInventory()->GetActive());
    auto* current = shooter->GetAIInventory()->GetCurrentItem();
    Require(IsValid(item) && IsValid(current) && current->GetInventoryItem() == item &&
        (dynamic_cast<NAI::CAIRocketLauncherWeapon*>(current) != nullptr) == launcher,
        "AI shot fixture did not recognize its selected native weapon type");
    const auto position = shooter->GetUnitPosition();
    const auto muzzle = actor->GetAttackOrigin(position, false);
    Require(fabs(muzzle - position.GetEyePosition()) > 0.01f, "AI shot fixture needs distinct eye and muzzle positions");
    actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_SHOOTING).SetConst(100);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    for (const auto& unit : units) {
        if (!IsValid(unit) || unit.GetPtr() == actor || !unit->CanFight())
            continue;
        auto* target = NAI::GetAIUnit(unit);
        if (!IsValid(target))
            continue;
        target->Synchronize(false);
        const float cover = shooter->GetCoverForFixedUnit(position, unit, item, NAI::HL_ANY);
        if (!(cover > 0))
            continue;
        CObj<NRPG::CAIUnitToHitCalcer> expected = new NRPG::CAIUnitToHitCalcer(shooter, position,
            target, cover, NAI::HL_ANY, 0, item, 0);
        if (launcher) {
            const int chance = shooter->GetToHitWithWeapon(target, position, NAI::HL_ANY,
                dynamic_cast<NAI::CAIRocketLauncherWeapon*>(current));
            if (chance <= 0)
                continue;
            Require(shooter->GetToHit(target, position, NAI::HL_ANY) == chance,
                "AI hit prediction discarded the current rocket-launcher weapon");
        } else {
            Require(fabs(expected->ptAttacker - muzzle) < 0.0001f,
                "AI shot accuracy used the unit's eyes rather than its weapon muzzle");
        }
        return;
    }
    Require(false, "AI shot fixture found no native target with usable cover");
}

void AIFirearmBudget(const char* databasePath, const char* resourcePath, bool partialBurst)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = EquipAccuracyActor(world);
    auto* shooter = NAI::GetAIUnit(actor);
    auto* mission = actor->GetUnitRPG();
    auto* inventory = mission->GetInventory();
    auto* held = dynamic_cast<NRPG::CWeaponItem*>(inventory->GetActive());
    Require(IsValid(held) && IsValid(shooter), "AI budget fixture needs a real active firearm and owner");
    NDb::CRPGWeapon* candidate = nullptr;
    CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while (!candidate && weapons.MoveNext()) {
        auto* weapon = weapons.Get();
        if (weapon != held->GetDBWeapon() && !weapon->bBazookaLogic && !weapon->nPanzerkleinType &&
            IsValid(weapon->pItem) && IsValid(weapon->pInnerClip) && IsValid(weapon->pWeaponType) &&
            weapon->pWeaponType != held->GetDBWeapon()->pWeaponType && weapon->shootModes[NDb::SM_Snap] &&
            (!partialBurst || (weapon->nRoF >= 18 && weapon->shootModes[NDb::SM_ShortBurst])))
            candidate = weapon;
    }
    Require(IsValid(candidate), "AI budget fixture needs a loaded candidate with a distinct preparation record");
    candidate->pWeaponType->nPrepareCost = 7;
    held->GetDBWeapon()->pWeaponType->nPrepareCost = 2;
    CObj<NRPG::CWeaponItem> item = new NRPG::CWeaponItem(candidate);
    Require(item->SetShootMode(partialBurst ? NDb::SM_ShortBurst : NDb::SM_Snap) && item->GetAmmoQuantity() >= 3,
        "AI budget fixture did not load the required fire mode and rounds");
    CObj<NAI::CAIFireArmsWeapon> weapon = new NAI::CAIFireArmsWeapon(shooter, item);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    const auto target = std::find_if(units.begin(), units.end(), [&](const auto& unit) {
        return IsValid(unit) && unit != actor && unit->CanFight() && IsValid(NAI::GetAIUnit(unit));
    });
    Require(target != units.end(), "AI budget fixture has no native target wrapper");
    auto* targetAI = NAI::GetAIUnit(*target);
    const auto position = actor->GetPosition();
    const int heldPrep = mission->GetActionAP(position.GetPose(), NRPG::AC_PREPARE);
    CObj<NRPG::IInventoryItem> saved = inventory->TakeOff(NDb::SLOT_1);
    Require(inventory->Equip(NDb::SLOT_1, item), "AI budget fixture could not equip its reference candidate");
    const int candidatePrep = mission->GetActionAP(position.GetPose(), NRPG::AC_PREPARE);
    Require(IsValid(inventory->TakeOff(NDb::SLOT_1)) && inventory->Equip(NDb::SLOT_1, saved),
        "AI budget fixture could not restore the original active item");
    const int first = weapon->GetShotAP() + candidatePrep;
    const int additional = weapon->GetBurstAP();
    Require(candidatePrep > heldPrep && first > 0 && additional > 0,
        "AI budget fixture does not distinguish held and candidate preparation costs");
    if (!partialBurst) {
        int ap, rounds, damage;
        bool reload;
        weapon->GetShotParameters(position, targetAI, 90.f, first - 1, &ap, &rounds, &damage, &reload);
        Require(ap == 0 && rounds == 0 && !reload, "AI accepted a candidate shot below its actual preparation budget");
        weapon->GetShotParameters(position, targetAI, 90.f, first, &ap, &rounds, &damage, &reload);
        Require(ap == first && rounds == 1 && !reload, "AI priced a candidate shot from the weapon currently held");
    } else {
        int ap, rounds, damage;
        bool reload;
        weapon->GetShotParameters(position, targetAI, 90.f, first + additional, &ap, &rounds, &damage, &reload);
        Require(ap == first + additional && rounds == 2 && !reload,
            "AI short burst exceeded its budget instead of firing the two affordable rounds");
    }
    Require(inventory->GetActive() == held, "AI budget evaluation equipped the candidate prematurely");
}


bool RunAccuracyTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "ai-firearm-budget") && argc == 5) {
        AIFirearmBudget(argv[2], argv[3], std::atoi(argv[4]) != 0);
        return true;
    }
    if (!std::strcmp(mode, "distant-melee-approach") && argc == 4) {
        DistantMeleeApproach(argv[2], argv[3]);
        return true;
    }
    if (!std::strcmp(mode, "fractional-unit-cover") && argc == 4) {
        FractionalUnitCover(argv[2], argv[3]);
        return true;
    }
    if (!std::strcmp(mode, "native-hit-arithmetic") && argc == 5) {
        NativeHitArithmetic(argv[2], argv[3], std::atoi(argv[4]));
        return true;
    }
    if (!std::strcmp(mode, "native-ai-shot-input") && argc == 5) {
        NativeAIShotInput(argv[2], argv[3], std::atoi(argv[4]) != 0);
        return true;
    }
    return false;
}
}
