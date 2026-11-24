#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGCritical.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/aiActions.h"
#include "../../Main/aiMisc.h"
#include "../../Main/aiUnit.h"
#include "../../Main/aiInventory.h"
#include "../../Main/aiUnitState.h"
#include "../../Main/aiWeapon.h"
#include "../../DBFormat/DataRPG.h"
#include <cstring>

namespace UpstreamTests
{
namespace
{
void SelfHealingWithBandagedHealth(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    const auto candidate = std::find_if(units.begin(), units.end(), [](const auto& unit) {
        return IsValid(unit) && unit->IsAIUnit() && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK();
    });
    Require(candidate != units.end(), "the healing fixture has no live AI character");
    auto* actor = candidate->GetPtr();
    auto* mission = actor->GetUnitRPG();
    auto* rpg = mission->GetRPGUnit();
    auto* inventory = mission->GetInventory();
    auto* record = NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(1);
    Require(record && record->nAPToUse == 0 && record->nRequiedSkill == 0 && record->nRequiredPerkID == 0,
        "the healing fixture needs a usable first-aid kit");
    CObj<NRPG::IInventoryItem> previous = inventory->TakeOff(NDb::SLOT_1);
    CObj<NRPG::CFirstAidItem> kit = new NRPG::CFirstAidItem(record);
    Require(inventory->Equip(NDb::SLOT_1, kit) && inventory->Activate(NDb::SLOT_1) && !kit->IsEmpty(),
        "could not equip the first-aid kit");

    auto& health = rpg->Skills(NDb::ST_VP);
    health.SetValue(health.GetCurrentMaxValue() / 4);
    rpg->nHealedVP = 0;
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_ANY, NDb::C_VP));
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD, NDb::C_WEAPONSKILL_REDUCTION, -1, 2.f, 1));
    auto* ai = NAI::GetAIUnit(actor);
    Require(ai && ai->IsUnderAIControl(), "the wounded unit has no AI controller");
    ai->Synchronize(false);
    Require(!ai->GetAIUnitState()->pEnemy, "the AI is already responding to an enemy");
    const float threshold = ai->GetMaxHP() * 0.3f;
    Require(ai->GetHP() > 0 && ai->GetHP() <= threshold, "the raw health is outside the healing threshold");
    const NAI::SPlaceWithAP place(ai->GetUnitPosition(), ai->GetAP());
    CObj<NAI::CAIHealAction> action = new NAI::CAIHealAction(ai);
    NAI::CAIHealAction::SInfo before, after;
    action->GetInfoInner(place, &before);
    Require(before.bCanDo && IsValid(before.pFirstAid) && before.pFirstAid->GetItem() == kit,
        "the AI cannot use its equipped kit before bandaging");

    rpg->nHealedVP = ai->GetMaxHP() / 4;
    Require(mission->GetTotalVP() > threshold && ai->GetHP() <= threshold,
        "the bandaged and raw values must straddle the healing threshold");
    action->GetInfoInner(place, &after);
    Require(!after.bCanDo && after.pFirstAid == before.pFirstAid,
        "the AI still spends first aid when bandaged health exceeds its threshold");
}
}

bool RunAITests(const char* mode, int argc, char** argv)
{
    if (std::strcmp(mode, "ai-self-healing-bandaged-health") != 0 || argc != 4)
        return false;
    SelfHealingWithBandagedHealth(argv[2], argv[3]);
    return true;
}
}
