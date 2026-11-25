#pragma once

#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/rpgGlobal.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/wUnitAttackExec.h"
#include "../../DBFormat/DataRPG.h"
#include "../../DBFormat/DataFormat.h"

namespace UpstreamTests
{
struct NativeGrenadeThrowFixture
{
    CObj<NRPG::CGlobalGame> game;
    CObj<NWorld::CWorld> world;
    NWorld::CUnitServer* actor = nullptr;
    NDb::CRPGGrenade* regular = nullptr;
    NDb::CRPGEngGrenade* special = nullptr;
    CObj<NRPG::CGrenadeItem> grenade;
    CObj<NRPG::IInventoryItem> previous;
    CObj<NWorld::CExecThrowGrenade> executor;
    NativeGrenadeThrowFixture(const char* databasePath,const char* resourcePath,bool engineer,bool human = false,bool aiOnly = false)
    {
        world = FixtureWorld(databasePath,resourcePath,human,&game);
        vector<CPtr<NWorld::CUnit>> units;
        world->GetAllUnits(&units);
        for (auto& entry: units) {
            auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
            if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK() &&
                (!human || !unit->IsAIUnit()) && (!aiOnly || unit->IsAIUnit())) { actor = unit; break; }
        }
        Require(actor != nullptr, "restored-grenade fixture lacks an ordinary native actor");
        if (engineer) {
            CDBIterator<NDb::CRPGEngGrenade> rows(*NDatabase::GetTable<NDb::CRPGEngGrenade>());
            while (rows.MoveNext()) if (IsValid(rows.Get()->pItem) && IsValid(rows.Get()->pItem->pModel) &&
                rows.Get()->pItem->nWeight > 0 && (rows.Get()->nRequiredPerkID <= 0 ||
                actor->GetUnitRPG()->HasPerk(rows.Get()->nRequiredPerkID))) { special = rows.Get(); break; }
        } else {
            CDBIterator<NDb::CRPGGrenade> rows(*NDatabase::GetTable<NDb::CRPGGrenade>());
            while (rows.MoveNext()) if (IsValid(rows.Get()->pItem) && IsValid(rows.Get()->pItem->pModel) &&
                rows.Get()->pItem->nWeight > 0) { regular = rows.Get(); break; }
        }
        Require(regular || special, "restored-grenade fixture lacks a modelled authored grenade of positive weight");
        if (special) {
            auto& engineering = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_ENGINEERING);
            if (int(engineering) < special->nSkillReq) engineering.SetConst(special->nSkillReq);
        }
        grenade = engineer ? new NRPG::CGrenadeItem(special) : new NRPG::CGrenadeItem(regular);
        grenade->nQuantity = 1;
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        while (!inventory->GetItems().empty()) inventory->Take(inventory->GetItems().front().pItem);
        previous = inventory->Get(NDb::SLOT_1) ? inventory->TakeOff(NDb::SLOT_1) : nullptr;
        Require(inventory->Equip(NDb::SLOT_1,grenade) && inventory->Activate(NDb::SLOT_1),
            "restored-grenade fixture could not equip its authored grenade");
        actor->animator.SetActiveItem(true);
        actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
        for (float radius: {0.5f,1.f,2.f,3.f,5.f}) {
            for (const auto& offset: {CVec3(radius,0,0.1f),CVec3(-radius,0,0.1f),CVec3(0,radius,0.1f),CVec3(0,-radius,0.1f)}) {
                CObj<NWorld::CExecThrowGrenade> candidate = new NWorld::CExecThrowGrenade(actor,actor->GetPosition().GetCP()+offset);
                const auto result = candidate->CanDoIt(actor->GetPosition());
                if (result == NWorld::UCR_OK && actor->CanSpendAP(candidate->GetStartAP())) {
                    executor = candidate; break;
                }
            }
            if (executor) break;
        }
        Require(executor != nullptr, "restored-grenade fixture has no nearby valid native throw with sufficient AP");
    }
};

}
