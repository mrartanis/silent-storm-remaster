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
#include <cstring>

namespace UpstreamTests
{
namespace
{
void FractionalUnitCover(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* mission = actor->GetUnitRPG();
    auto* inventory = mission->GetInventory();
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    auto* record = NDatabase::GetTable<NDb::CRPGWeapon>()->GetRecord(26);
    Require(IsValid(record) && IsValid(record->pInnerClip), "cover precision fixture lacks a loaded native weapon");
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(record);
    Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1) &&
        weapon->SetShootMode(NDb::SM_Snap), "cover precision fixture could not activate the native weapon");
    actor->SetUndrawItem(false);
    actor->animator.SetActiveItem(true);
    actor->animator.SetWeaponAnimation(mission->GetWeaponType());
    actor->animator.SetWeaponName(record->szAnimName.c_str());
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
}

bool RunAccuracyTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "fractional-unit-cover") && argc == 4) {
        FractionalUnitCover(argv[2], argv[3]);
        return true;
    }
    return false;
}
}
