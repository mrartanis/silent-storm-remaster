#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wUnitCommands.h"
#include "../../Main/wUnitExec.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGCritical.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemMap.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/wUnitAttackExec.h"
#include "../../Main/rpgPerk.h"
#include "../../Main/rpgGlobal.h"
#include "../../DBFormat/DataRPG.h"
#include "../../MiscDll/LogStream.h"
#include "../../Misc/RandomGen.h"
#include <cstring>

namespace UpstreamTests
{
namespace
{
void DifficultySkillScaling()
{
    CObj<NRPG::CDynamicSkill> reduced = new NRPG::CDynamicSkill(27, 20);
    reduced->ScaleForDifficulty(0.8f);
    Require(int(*reduced) == 21 && reduced->GetMaxValue() == 21 && reduced->GetXPPart() == 7,
        "AI difficulty rounded up a fractional skill cap");

    CObj<NRPG::CDynamicSkill> increased = new NRPG::CDynamicSkill(27, 20);
    increased->ScaleForDifficulty(1.25f);
    Require(int(*increased) == 27 && increased->GetMaxValue() == 33 && increased->GetXPPart() == 7,
        "raising the difficulty cap rounded up or increased the unit's current value");
}

void TakePerkDuringTreatment(const char* databasePath, const char* resourcePath)
{
    using namespace NWorld;
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    world->GivePlayerTurn(actor->GetTBSPlayer());
    auto* mission = actor->GetUnitRPG();
    auto* unit = mission->GetRPGUnit();
    auto* inventory = mission->GetInventory();
    auto* record = NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(1);
    Require(record && record->effect == NDb::FAE_NORMAL, "fixture lacks the ordinary first-aid kit");
    CObj<NRPG::CFirstAidItem> kit = new NRPG::CFirstAidItem(record);
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    Require(inventory->Equip(NDb::SLOT_1, kit) && inventory->Activate(NDb::SLOT_1),
        "fixture could not ready the first-aid kit");
    actor->animator.SetActiveItem(true);
    actor->animator.SetWeaponAnimation(mission->GetWeaponType());
    auto& hp = unit->Skills(NDb::ST_VP);
    hp.SetValue(hp.GetMaxValue() / 2);
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_ANY, NDb::C_VP));
    world->GetAIMap()->Sync();

    auto* perks = unit->GetPerksTree();
    perks->AddPerkPoints(1);
    vector<CPtr<NRPG::CPerk>> available;
    perks->GetAvailablePerks(&available);
    Require(!available.empty(), "fixture has no purchasable perk");
    const int id = available.front()->GetDBPerk()->GetRecordID();
    const int points = perks->GetPerkPoints();
    CObj<CCmdTakePerk> purchase = new CCmdTakePerk(id);
    EUnitCommandResult result = UCR_UNAVAILABLE;
    CObj<CCommandExecute> order = actor->CreateExecutor(purchase, &result);
    Require(order && result == UCR_OK && !unit->HasPerk(id), "idle unit cannot validate its available perk");

    CObj<CExecHeal> treatment = new CExecHeal(actor, actor);
    Require(treatment->CanDoIt(actor->GetPosition()) == UCR_OK, "fixture cannot begin treating its wound");
    treatment->Run();
    treatment->AnimationFinished();
    Require(mission->HasCritical(NDb::C_PATIENT), "fixture did not remain in the treatment state");
    const int ap = actor->GetAP();
    const int doses = kit->GetIncQuantity();
    order = actor->CreateExecutor(purchase, &result);
    Require(order && result == UCR_OK, "active treatment blocked an available perk purchase");
    order->Run();
    Require(unit->HasPerk(id) && perks->GetPerkPoints() == points - 1 && actor->GetAP() == ap &&
        kit->GetIncQuantity() == doses && mission->HasCritical(NDb::C_PATIENT),
        "purchasing a perk did not spend precisely one point or interrupted the treatment");
    order = actor->CreateExecutor(purchase, &result);
    Require(!order && result == UCR_GENERAL_FAILURE, "treatment bypassed ordinary perk eligibility");
}

void SelectedCriticalTreatmentLog(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    auto* persona = NDb::GetPers(103);
    Require(IsValid(persona), "treatment log fixture lacks an authored patient");
    CObj<NRPG::IUnitMission> patient = NRPG::CreateUnit(persona);
    CObj<NRPG::IUnitMission> control = NRPG::CreateUnit(persona);
    NRPG::SCritical injury(NDb::CL_HEAD, NDb::C_BLEEDING, -1, 2.f, 3);
    NRPG::SCritical separate(NDb::CL_LEGS, NDb::C_BLEEDING, -1, 4.f, 5);
    patient->ApplyCritical(injury);
    patient->ApplyCritical(separate);
    control->ApplyCritical(injury);

    class Capture : public ILogNotify {
        OBJECT_NOCOPY_METHODS(Capture);
    public:
        vector<wstring> cured;
        void OnAddConsoleLine(const SConsoleLine& line) override {
            if (line.eType == STREAM_RPG && line.szText.find(L"Cured:") != wstring::npos)
                cured.push_back(line.szText);
        }
    };
    CObj<Capture> log = new Capture;
    SetLogNotify(log);
    struct RestoreNotify { ~RestoreNotify() { SetLogNotify(nullptr); } } restore;
    Require(control->HealCriticals(3, true) == 3 && log->cured.size() == 1,
        "the ordinary critical treatment control did not log its cure");
    const auto expected = log->cured.front();
    log->cured.clear();
    const auto choice = patient->TryHealCritical(3, true);
    Require(choice.eCritical == NDb::C_BLEEDING && choice.eCl == NDb::CL_HEAD && choice.nDC == 3,
        "treatment did not select the intended head injury");
    Require(patient->HealCritical(choice) == 3 && patient->TryHealCritical(5, true).eCl == NDb::CL_LEGS,
        "treatment changed the result or removed the separate leg injury");
    Require(log->cured.size() == 1 && log->cured.front() == expected,
        "targeted treatment did not log the cured injury with its location and difficulty");
    Require(patient->HealCritical(choice) == 0 && log->cured.size() == 1,
        "targeted treatment logged a cure when the requested injury was absent");
}

void PanzerkleinRegenerationBleeding(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    auto* persona = NDb::GetPers(103);
    Require(IsValid(persona), "PK regeneration fixture lacks an authored pilot");
    CObj<NRPG::IUnitMission> mission = NRPG::CreateUnit(persona);
    auto& hp = mission->GetRPGUnit()->Skills(NDb::ST_VP);
    hp.SetValue(hp.GetMaxValue() - 10);

    CObj<NDb::CPanzerklein> suit = new NDb::CPanzerklein;
    suit->fRegenerationValue = 4.f;
    suit->bSingleSlot = false;
    CObj<NRPG::CDynamicSkill> shell = new NRPG::CDynamicSkill(100, 100);
    mission->SetPanzerklein(suit, shell, nullptr);
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD, NDb::C_BLEEDING, -1, 2.f, NRPG::N_MAX_DC));
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_LEGS, NDb::C_BLEEDING, -1, 2.f, NRPG::N_MAX_DC + 1));
    const auto first = mission->TryHealCritical(NRPG::N_MAX_DC, false);
    Require(first.eCl == NDb::CL_HEAD && first.nDC == NRPG::N_MAX_DC,
        "fixture did not retain bleeding at the normal difficulty limit");

    int bleeding = -1;
    mission->DoRegenerations(nullptr, &bleeding);
    Require(mission->GetHealedVP() == 4 && int(*shell) == 100 && bleeding > 0,
        "regeneration did not calculate bleeding and heal the pilot as before");
    Require(mission->TryHealCritical(NRPG::N_MAX_DC, false).eCritical == NDb::C_NONE,
        "regeneration did not cure bleeding at the normal difficulty limit");
    const auto remaining = mission->TryHealCritical(NRPG::N_MAX_DC + 1, false);
    Require(remaining.eCritical == NDb::C_BLEEDING && remaining.eCl == NDb::CL_LEGS && remaining.nDC == NRPG::N_MAX_DC + 1,
        "regeneration bypassed the critical difficulty limit for bleeding");
}

static void LookActionPointCost(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    world->GivePlayerTurn(actor->GetTBSPlayer());
    const auto original = actor->GetPosition();
    const int ap = actor->GetAP();
    const int rotatePrice = actor->GetActionAP(NRPG::AC_ROTATE);
    Require(rotatePrice > 0 && ap >= rotatePrice, "look test requires a priced turn and sufficient AP");
    auto turned = original.pos;
    turned.p.SetDirection((turned.p.GetDirection() + 2) & 7);
    int start = -1, total = -1;
    CObj<NWorld::CCmdLook> change = new NWorld::CCmdLook(turned);
    Require(actor->CanDo(change, &start, &total) == NWorld::UCR_OK && total >= rotatePrice,
        "a real change of facing must have a rotation cost");

    CObj<NWorld::CCmdLook> same = new NWorld::CCmdLook(original.pos);
    Require(actor->CanDo(same, &start, &total) == NWorld::UCR_OK,
        "the fixture did not prepare a valid unchanged-facing look order");
    Require(total == 0, "an unchanged-facing look order charges AP");
    Require(actor->GetAP() == ap && actor->GetPosition() == original, "querying look cost changed the actor");
}

static void InventoryTransferValidation(const char* databasePath, const char* resourcePath, int mode)
{
    using namespace NWorld;
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    CObj<NDb::CRPGItem> record = new NDb::CRPGItem;
    record->sSize = CTPoint<int>(1, 1);
    CObj<NRPG::IInventoryItem> item = NRPG::CreateDummyItem(record);
    CTPoint<int> place;
    Require(inventory->FindPlace(item, &place), "transfer fixture has no backpack space");
    inventory->Place(place, item);
    const auto position = actor->GetPosition();
    const SItem source(actor, SItem::BACKPACK, place, item);
    const SItem target(nullptr, SItem::STORAGE, nullptr, actor->GetTBSPlayer());
    CObj<CCmdMoveInventoryItem> command = new CCmdMoveInventoryItem(source, target);
    CObj<CExecMoveInventoryItem> executor = new CExecMoveInventoryItem(actor, command);
    Require(executor->CanDoIt(position) == UCR_OK, "ordinary backpack-to-store transfer is unavailable");
    const auto rejected = [&](CExecMoveInventoryItem* transfer) {
        const int ap = actor->GetAP();
        const auto count = actor->GetTBSPlayer()->GetStoreItems()->size();
        transfer->Run();
        Require(transfer->GetState() == CCommandExecute::FAILED && actor->GetAP() == ap &&
            actor->GetTBSPlayer()->GetStoreItems()->size() == count,
            "a rejected transfer still ran or changed AP or store contents");
    };

    if (mode == 0) {
        inventory->Take(item);
        const auto& backpack = inventory->GetItems();
        Require(std::none_of(backpack.begin(), backpack.end(), [&](const auto& entry) { return entry.pItem == item; }),
            "failed to remove the item between queuing and validation");
        Require(executor->CanDoIt(position) == UCR_INVALID_COMMAND,
            "a stale command accepted an item no longer in the source backpack");
        rejected(executor);
        CObj<CCmdMoveInventoryItem> ownerless = new CCmdMoveInventoryItem(
            SItem(nullptr, SItem::BACKPACK, place, item), target);
        CObj<CExecMoveInventoryItem> ownerlessExec = new CExecMoveInventoryItem(actor, ownerless);
        Require(ownerlessExec->CanDoIt(position) == UCR_INVALID_COMMAND,
            "a backpack transfer accepted a missing source unit");
    } else if (mode == 1) {
        CObj<CCmdMoveInventoryItem> ownerless = new CCmdMoveInventoryItem(
            source, SItem(nullptr, SItem::STORAGE));
        CObj<CExecMoveInventoryItem> ownerlessExec = new CExecMoveInventoryItem(actor, ownerless);
        Require(ownerlessExec->CanDoIt(position) == UCR_INVALID_COMMAND,
            "a transfer accepted storage without an owner");
        rejected(ownerlessExec);
    } else {
        CObj<CObjectBase> foreign = item->Lock(actor->GetTBSPlayer());
        Require(foreign && item->IsLocked(actor), "failed to establish a foreign item reservation");
        Require(executor->CanDoIt(position) == UCR_GENERAL_FAILURE,
            "a transfer disregarded another actor's existing reservation");
        rejected(executor);
        Require(item->IsLocked(actor), "validating the transfer cleared a foreign reservation");
        foreign = nullptr;
        Require(executor->CanDoIt(position) == UCR_OK && !item->IsLocked(actor->GetTBSPlayer()),
            "an unreserved transfer query took ownership of the item");
        command->Lock(actor);
        Require(executor->CanDoIt(position) == UCR_OK && item->IsLocked(actor->GetTBSPlayer()),
            "the issuing actor's own reservation blocked validation or was lost");
        command->Unlock();
    }
}

void InventoryTransferInactivePose(const char* databasePath, const char* resourcePath)
{
    using namespace NWorld;
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    CObj<CUnitServer> recipient = world->AddUnitInGame(actor->GetPosition().pos.p,
        NRPG::CreateUnit(NDb::GetPers(103)), actor->GetTBSPlayer());
    Require(recipient && recipient->CanFight() && recipient->GetTBSPlayer() == actor->GetTBSPlayer(),
        "inactive transfer fixture could not deploy its recipient");
    world->GivePlayerTurn(actor->GetTBSPlayer());
    auto* sourceInventory = actor->GetUnitRPG()->GetInventory();
    auto* targetInventory = recipient->GetUnitRPG()->GetInventory();
    CObj<NDb::CRPGItem> record = new NDb::CRPGItem;
    record->sSize = CTPoint<int>(1, 1);
    CObj<NRPG::IInventoryItem> item = NRPG::CreateDummyItem(record);
    CTPoint<int> sourceCell, recipientCell;
    Require(sourceInventory->FindPlace(item, &sourceCell) && sourceInventory->Place(sourceCell, item) &&
        targetInventory->FindPlace(item, &recipientCell), "inactive transfer fixture lacks backpack space");
    const SItem source(actor, SItem::BACKPACK, sourceCell, item);
    const auto original = actor->GetPosition();
    CObj<CCmdMoveInventoryItem> transfer = new CCmdMoveInventoryItem(
        source, SItem(recipient, SItem::BACKPACK, recipientCell, nullptr));
    CObj<CCmdMoveInventoryItem> rearrange = new CCmdMoveInventoryItem(
        source, SItem(actor, SItem::BACKPACK, sourceCell, nullptr));
    EUnitCommandResult result = UCR_UNAVAILABLE;
    CObj<CCommandExecute> order = actor->CreateExecutor(transfer, &result);
    Require(order && result == UCR_OK, "standing unit could not pass an item to its nearby ally");
    order = actor->CreateExecutor(rearrange, &result);
    Require(order && result == UCR_OK, "standing unit could not access its own backpack");

    auto inactive = original;
    inactive.pos.p.SetPose(NAI::CM_INACTIVE);
    actor->SetPosition(inactive);
    Require(actor->GetPosition().pos.p.GetPose() == NAI::CM_INACTIVE, "fixture did not enter the inactive pose");
    order = actor->CreateExecutor(rearrange, &result);
    Require(!order && result == UCR_UNAVAILABLE, "inactive pose allowed the unit's own backpack command");
    order = actor->CreateExecutor(transfer, &result);
    Require(order && result == UCR_OK, "inactive pose rejected a valid transfer between allies");
}

void ForcedActionReplacement(const char* databasePath, const char* resourcePath, bool stale)
{
    using namespace NWorld;
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    world->GivePlayerTurn(actor->GetTBSPlayer());
    const auto previousPose = actor->GetWishPose();
    const auto desiredPose = previousPose == NAI::CROUCH ? NAI::WALK : NAI::CROUCH;
    const auto submit = [&] {
        actor->Do(new CCmdSetCommand(actor, new CCmdWishPose(desiredPose)));
        if (actor->HasCommand())
            actor->Do(new CCmdSetCommand(actor, new CCmdContinue));
    };
    const auto current = [&] {
        CStructureNetworkContext refs;
        auto fields = BorrowedObjectFields(actor, &refs);
        CObj<CCommandExecute> executor;
        bool forced = false;
        CStructureSaver reader(fields, CStructureSaver::READ, &refs);
        reader.Add(7, &executor);
        reader.Add(12, &forced);
        return std::make_pair(executor, forced);
    };
    Require(!current().first && !current().second, "fixture starts with an existing command");
    if (stale) {
        bool forced = true;
        CMemoryStream fields;
        { CStructureSaver writer(fields, CStructureSaver::WRITE); writer.Add(12, &forced); }
        fields.Seek(0);
        { CStructureSaver reader(fields, CStructureSaver::READ); static_cast<CObjectBase*>(actor)->operator&(reader); }
        Require(!current().first && current().second, "legacy state fixture did not restore the orphaned latch");
    } else {
        Require(IsValid(NDb::GetAnimation(4437)), "fixture lacks the authored animation");
        CObj<CExecPlayAnimation> animation = new CExecPlayAnimation(actor, 4437, false);
        actor->RunCriticalExecutor(animation);
        Require(current().first == animation && current().second && actor->IsPerformingAction(),
            "the forced animation could not start");
        submit();
        Require(current().first == animation && current().second && actor->GetWishPose() == previousPose,
            "an ordinary command replaced the running forced action");
        world->GetTime()->Set(actor->animator.GetTimeEnd());
        actor->Segment();
    }
    submit();
    Require(actor->GetWishPose() == desiredPose && !current().second,
        "the forced-action flag blocked or outlived the next ordinary command");
}

void FallDamageMinimum(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    auto* mission = actor->GetUnitRPG();
    auto expectedRandom = GlobalGameRandom();
    int expected = 0;
    for (int n = 0; n < 3; ++n)
        expected += expectedRandom.Get(0, 25);
    Require(mission->GetFallDamage(4.8f) == Max(2, expected), "the ordinary fall changed its three damage rolls");
    auto actualRandom = GlobalGameRandom();
    Require(actualRandom.Get() == expectedRandom.Get(), "the ordinary fall changed the shared RNG consumption");

    const int health = mission->GetTotalVP();
    actor->FallFromHigh(2.8f);
    Require(mission->GetTotalVP() == health, "a nonqualifying fall inflicted damage");
    expectedRandom = GlobalGameRandom();
    Require(mission->GetFallDamage(2.81f) == 2, "a qualifying fall did not impose the minimum damage");
    actualRandom = GlobalGameRandom();
    Require(actualRandom.Get() == expectedRandom.Get(), "a zero-range damage roll advanced the RNG");
    actor->FallFromHigh(2.81f);
    Require(mission->GetTotalVP() < health, "the minimum fall did not reach normal unit damage");
}

void RealTimeTurnReset(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    auto* mission = actor->GetUnitRPG();
    actor->OnTBSEvent(NWorld::TBS_START_NEW_TURN);
    actor->OnTBSEvent(NWorld::TBS_START_NEW_TURN);
    Require(!mission->IsFirstTurn(), "fixture did not advance the unit turn counter");
    const int ap = actor->GetAP();
    mission->SpendAP(1);
    Require(actor->GetAP() == ap - 1, "fixture could not exercise the usual AP reset");
    actor->OnTBSEvent(NWorld::TBS_START_REAL_TIME);
    Require(actor->GetAP() == ap, "entering real time failed to reset ordinary AP accounting");
    Require(mission->IsFirstTurn(), "entering real time kept the previous encounter's turn counter");
    actor->OnTBSEvent(NWorld::TBS_START_NEW_TURN);
    Require(mission->IsFirstTurn(), "the new encounter did not get its first turn");
    actor->OnTBSEvent(NWorld::TBS_START_NEW_TURN);
    Require(!mission->IsFirstTurn(), "the new encounter did not advance the normal turn counter");
}

static void WishPoseStrafe(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    world->GivePlayerTurn(actor->GetTBSPlayer());
    const auto position = actor->GetPosition();
    for (auto pose : {NAI::WALK, NAI::CROUCH, NAI::RUN, NAI::CRAWL}) {
        actor->SetStrafe(true);
        CObj<NWorld::CCmdWishPose> cmd = new NWorld::CCmdWishPose(pose);
        NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
        CObj<NWorld::CCommandExecute> order = actor->CreateExecutor(cmd, &result);
        Require(order && result == NWorld::UCR_OK, "the ordinary wish-pose command was unavailable");
        order->Run();
        auto expectedPosition = position;
        expectedPosition.bRun = pose == NAI::RUN;
        Require(actor->GetWishPose() == pose && actor->GetPosition() == expectedPosition,
            "a wish-pose order failed to update the desired movement mode");
        Require(actor->IsStrafing() == (pose == NAI::WALK || pose == NAI::CROUCH),
            "the wish-pose executor applied the wrong strafe policy");
    }
}

static void HideWithEmptyRosterSlot(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    actor->EnableHide();
    CObj<NWorld::CCmdHide> hide = new NWorld::CCmdHide;
    NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
    CObj<NWorld::CCommandExecute> ordinary = NWorld::CreateExecutor(actor, hide, &result);
    Require(ordinary && result == NWorld::UCR_OK, "the ordinary hide order was ineligible before adding the empty slot");
    vector<CPtr<NWorld::CPlayer>> players;
    world->GetPlayersList(&players);
    for (auto opponent : players) {
        if (!IsValid(opponent) || opponent == actor->GetTBSPlayer()) continue;
        opponent->AddUnit(nullptr);
        CObj<NWorld::CCommandExecute> withEmpty = NWorld::CreateExecutor(actor, hide, &result);
        opponent->RemoveUnit(nullptr);
        Require(withEmpty && result == NWorld::UCR_OK, "an empty opposing roster slot blocked the hide order");
        return;
    }
    Require(false, "hide fixture lacks an opposing roster");
}

static void NormalStateCommandLifetime(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    const auto initial = actor->GetWishPose();
    const auto wanted = initial == NAI::CROUCH ? NAI::WALK : NAI::CROUCH;
    const auto position = actor->GetPosition();
    const int ap = actor->GetAP();
    NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
    CObj<NWorld::CCommandExecute> executor = actor->CreateExecutor(new NWorld::CCmdWishPose(wanted), &result);
    Require(executor && result == NWorld::UCR_OK, "normal-state validation released a newly issued native command");
    Require(actor->GetWishPose() == initial && actor->GetAP() == ap && actor->GetPosition() == position,
        "creating a pose executor changed unit state");
    executor->Run();
    Require(actor->GetWishPose() == wanted, "the retained command lost its requested pose");
}

static void CarriedClueTransfer(const char* databasePath, const char* resourcePath, bool automatic)
{
    using namespace NWorld;
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    CObj<NRPG::IInventoryItem> previous1 = inventory->TakeOff(NDb::SLOT_1);
    CObj<NRPG::IInventoryItem> previous2 = inventory->TakeOff(NDb::SLOT_2);
    CObj<NDb::CRPGItem> record = new NDb::CRPGItem;
    record->sSize = CTPoint<int>(1, 1);
    CObj<NRPG::IInventoryItem> clue = NRPG::CreateClueItem(record);
    CTPoint<int> place;
    Require(inventory->FindPlace(clue, &place) && inventory->Place(place, clue),
        "could not place a carried clue into the source backpack");
    Require(inventory->CanEquip(NDb::SLOT_1, clue),
        "the fixture does not reach the world transfer's clue policy");
    const SItem source(actor, SItem::BACKPACK, place, clue);
    const SItem target = automatic ? SItem(actor, SItem::UNIT_ANYPLACE) : SItem(actor, SItem::SLOT, NDb::SLOT_1);
    CObj<CCmdMoveInventoryItem> command = new CCmdMoveInventoryItem(source, target);
    CObj<CExecMoveInventoryItem> executor = new CExecMoveInventoryItem(actor, command);
    const auto result = executor->CanDoIt(actor->GetPosition());
    if (!automatic) {
        Require(result == UCR_GENERAL_FAILURE, "a carried clue can be moved into an equipped slot");
        return;
    }
    Require(result == UCR_OK, "automatic transfer rejected a carried clue despite backpack space");
    executor->Run();
    const auto& backpack = inventory->GetItems();
    const auto copies = std::count_if(backpack.begin(), backpack.end(),
        [&](const auto& entry) { return entry.pItem == clue; });
    Require(copies == 1 && !inventory->Get(NDb::SLOT_1) && !inventory->Get(NDb::SLOT_2),
        "automatic transfer equipped a carried clue instead of storing it in the backpack");
}

static void HolsteredEquipment(const char* databasePath, const char* resourcePath, bool transfer)
{
    using namespace NWorld;
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    auto* record = NDb::GetWeapon(65);
    Require(record && record->pItem && record->pItem->bPlaceInHand && inventory->Activate(NDb::SLOT_1),
        "holster fixture needs a visible authored weapon and active first slot");
    CObj<NRPG::IInventoryItem> previous = inventory->TakeOff(NDb::SLOT_1);
    CObj<NRPG::IInventoryItem> weapon = new NRPG::CWeaponItem(record);
    actor->animator.SetActiveItem(false);
    actor->animator.SetWeaponAnimation(NDb::WT_DEFAULT);
    actor->SetWalkWithoutWeapon(true);
    actor->SetUndrawItem(transfer);
    Require(actor->animator.CanActivateItem(), "the fixture cannot play the draw animation");

    CObj<CCommandExecute> executor;
    if (transfer) {
        CTPoint<int> backpack;
        Require(inventory->FindPlace(weapon, &backpack) && inventory->Place(backpack, weapon),
            "the fixture could not place its weapon into the backpack");
        const SItem source(actor, SItem::BACKPACK, backpack, weapon);
        const SItem target(actor, SItem::SLOT, NDb::SLOT_1);
        CObj<CCmdMoveInventoryItem> command = new CCmdMoveInventoryItem(source, target);
        auto* move = new CExecMoveInventoryItem(actor, command);
        executor = move;
        Require(move->CanDoIt(actor->GetPosition()) == UCR_OK, "the real equip transfer is unavailable");
    } else {
        Require(inventory->Equip(NDb::SLOT_1, weapon), "the fixture could not equip its weapon");
        CObj<CCmdSetActiveItem> command = new CCmdSetActiveItem(NDb::SLOT_1);
        EUnitCommandResult result = UCR_UNAVAILABLE;
        executor = actor->CreateExecutor(command, &result);
        Require(executor && result == UCR_OK, "the real draw command is unavailable");
    }
    executor->Run();
    Require(inventory->GetActive() == weapon && executor->IsExecuting() && actor->animator.IsActiveItem(),
        "the real draw sequence did not start with the expected weapon");
    if (transfer)
        Require(!actor->IsWalkingWithoutWeapon(), "equipping a weapon retained the scripted holster policy");
    Require(actor->GetUndrawItem(), "the new weapon became visible before the draw label");
    executor->TimeLabelReached();
    Require(!actor->GetUndrawItem(), "the draw label did not reveal the weapon");
    executor->AnimationFinished();
}

struct ReloadFixture
{
    CObj<NWorld::CWorld> world;
    NWorld::CUnitServer* actor;
    NRPG::IInventory* inventory;
    CObj<NRPG::CWeaponItem> weapon;
    NRPG::CClipItem* installed;

    ReloadFixture(const char* databasePath, const char* resourcePath)
        : world(FixtureWorld(databasePath, resourcePath, true)), actor(MovementPolicyActor(world)),
          inventory(actor->GetUnitRPG()->GetInventory())
    {
        auto* record = NDb::GetWeapon(65);
        Require(record && record->pInnerClip, "the reload fixture requires an authored firearm and magazine");
        CObj<NRPG::IInventoryItem> previous = inventory->TakeOff(NDb::SLOT_1);
        weapon = new NRPG::CWeaponItem(record);
        Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1),
            "the reload fixture could not equip the firearm");
        installed = dynamic_cast<NRPG::CClipItem*>(weapon->GetInnerClip());
        Require(installed && installed->GetItem() && installed->GetDBAmmo() && installed->GetMaxIncQuantity() >= 5,
            "the firearm lacks an initialized magazine with enough capacity");
    }

    CObj<NRPG::CClipItem> StockClip(int rounds)
    {
        CObj<NRPG::IInventoryItem> item = NRPG::CreateClipItem(installed->GetDBClip(), installed->GetDBAmmo(), rounds);
        CObj<NRPG::CClipItem> clip = dynamic_cast<NRPG::CClipItem*>(item.GetPtr());
        CTPoint<int> cell;
        Require(clip && clip->GetIncQuantity() == rounds && inventory->FindPlace(clip, &cell) &&
            inventory->Place(cell, clip), "the reload fixture could not stock the matching ammunition");
        return clip;
    }
};

void MatchingAmmoTransfer(const char* databasePath, const char* resourcePath, bool topUp)
{
    using namespace NWorld;
    ReloadFixture fixture(databasePath, resourcePath);
    auto* inventory = fixture.inventory;
    const int capacity = fixture.installed->GetMaxIncQuantity();
    fixture.installed->GetItem()->nQuantity = topUp ? capacity - 2 : 0;
    auto supply = fixture.StockClip(topUp ? 4 : 2);
    if (!topUp) supply->nQuantity = 1;
    const auto count = inventory->GetItems().size();
    const SItem source(fixture.actor, SItem::BACKPACK, supply);
    CObj<CExecLoadWeapon> executor = new CExecLoadWeapon(fixture.actor, fixture.weapon, source);
    Require(executor->CanDoIt() == UCR_OK, "the actual reload executor rejected matching ammunition");
    executor->TimeLabelReached();
    Require(fixture.weapon->GetAmmoQuantity() == (topUp ? capacity : 2),
        "matching ammunition replaced the installed rounds instead of topping up");
    const auto& backpack = inventory->GetItems();
    const bool kept = std::any_of(backpack.begin(), backpack.end(),
        [&](const auto& entry) { return entry.pItem == supply; });
    Require(kept == topUp && backpack.size() == (topUp ? count : count - 1) &&
        supply->GetIncQuantity() == (topUp ? 2 : 0),
        "the reload lost a partial magazine or left a spent magazine in the backpack");
}

void QueuedReloadMagazine(const char* databasePath, const char* resourcePath, bool removed)
{
    using namespace NWorld;
    ReloadFixture fixture(databasePath, resourcePath);
    auto* inventory = fixture.inventory;
    while (!inventory->GetItems().empty())
        inventory->Take(inventory->GetItems().front().pItem);
    const int capacity = fixture.installed->GetMaxIncQuantity();
    fixture.installed->GetItem()->nQuantity = capacity - 2;
    auto selected = fixture.StockClip(2);
    auto alternative = fixture.StockClip(2);
    NRPG::SFindClipResult found;
    Require(fixture.weapon->FindProperClip(inventory, &found, true) && found.pItem == selected,
        "the reload fixture failed to select its first available magazine");
    CObj<CExecReload> command = new CExecReload(fixture.actor, -1);
    Require(command->CanDoIt() == UCR_OK, "the selected magazine is unavailable before queueing");

    inventory->Take(selected);
    if (!removed) {
        CTPoint<int> place;
        Require(inventory->FindPlace(selected, &place) && inventory->Place(place, selected),
            "the reload fixture could not move the selected magazine");
    }
    Require(fixture.weapon->FindProperClip(inventory, &found, true) && found.pItem == alternative,
        "a new magazine lookup must prefer the other magazine after rearranging");
    if (removed)
        Require(command->CanDoIt() == UCR_NO_EQUIPMENT, "a queued reload accepted a removed magazine");
    else
        Require(command->CanDoIt() == UCR_OK, "a queued reload rejected a magazine moved within its own backpack");
    command->TimeLabelReached();
    Require(fixture.weapon->GetAmmoQuantity() == (removed ? capacity - 2 : capacity) &&
        selected->GetIncQuantity() == (removed ? 2 : 0) && alternative->GetIncQuantity() == 2,
        "a queued reload replaced its selected magazine or consumed detached ammunition");
}

void InventoryCursorCommands(const char* databasePath, const char* resourcePath)
{
    using namespace NWorld;
    ReloadFixture fixture(databasePath, resourcePath);
    auto* actor = fixture.actor;
    fixture.world->GivePlayerTurn(actor->GetTBSPlayer());
    fixture.installed->GetItem()->nQuantity = 0;
    auto held = fixture.StockClip(2);
    fixture.StockClip(3);
    const SItem hand(actor, SItem::HAND, held);
    const auto pose = actor->GetPosition();
    CObj<CCmdLook> look = new CCmdLook(pose.pos);
    EUnitCommandResult result = UCR_UNAVAILABLE;
    CObj<CCommandExecute> order = actor->CreateExecutor(look, &result);
    Require(order && result == UCR_OK, "normally valid look command did not pass its control query");

    fixture.inventory->Take(held);
    actor->GetTBSPlayer()->SetInHandItem(hand);
    CObj<CCmdLoadWeapon> load = new CCmdLoadWeapon(fixture.weapon, hand);
    CObj<CCmdReload> reload = new CCmdReload;
    order = actor->CreateExecutor(load, &result);
    Require(order && result == UCR_OK, "the occupied cursor made its own ammunition unavailable to load");
    order = actor->CreateExecutor(reload, &result);
    Require(order && result == UCR_OK, "the occupied cursor blocked reloading from the backpack");
    order = actor->CreateExecutor(look, &result);
    Require(!order && result == UCR_UNAVAILABLE, "a unit accepted an unrelated order while an item was on the cursor");

    CTPoint<int> cell;
    Require(fixture.inventory->FindPlace(held, &cell), "fixture has no slot to place the held magazine");
    CObj<CCmdMoveInventoryItem> place = new CCmdMoveInventoryItem(hand, SItem(actor, SItem::BACKPACK, cell, nullptr));
    order = actor->CreateExecutor(place, &result);
    Require(order && result == UCR_OK, "the held item could not be returned to the backpack");
    order->Run();
    SItem after;
    Require(!actor->GetTBSPlayer()->GetInHandItem(&after), "placing the item did not clear the cursor");
    order = actor->CreateExecutor(look, &result);
    Require(order && result == UCR_OK, "the unit ignored a valid order after releasing its cursor item");
}
}

bool RunUnitTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "difficulty-skill-scaling") && argc == 2) { DifficultySkillScaling(); return true; }
    if (!std::strcmp(mode, "take-perk-during-treatment") && argc == 4) { TakePerkDuringTreatment(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "selected-critical-treatment-log") && argc == 3) { SelectedCriticalTreatmentLog(argv[2]); return true; }
    if (!std::strcmp(mode, "panzerklein-regeneration-bleeding") && argc == 3) { PanzerkleinRegenerationBleeding(argv[2]); return true; }
    if (!std::strcmp(mode, "look-action-point-cost") && argc == 4) { LookActionPointCost(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "inventory-transfer-validation") && argc == 5) { InventoryTransferValidation(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "inventory-transfer-inactive-pose") && argc == 4) { InventoryTransferInactivePose(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "forced-action-replacement") && argc == 5) { ForcedActionReplacement(argv[2], argv[3], std::atoi(argv[4]) != 0); return true; }
    if (!std::strcmp(mode, "fall-damage-minimum") && argc == 4) { FallDamageMinimum(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "realtime-turn-reset") && argc == 4) { RealTimeTurnReset(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "wish-pose-strafe") && argc == 4) { WishPoseStrafe(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "hide-empty-roster-slot") && argc == 4) { HideWithEmptyRosterSlot(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "normal-state-command-lifetime") && argc == 4) { NormalStateCommandLifetime(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "carried-clue-slot") && argc == 4) { CarriedClueTransfer(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "carried-clue-auto-placement") && argc == 4) { CarriedClueTransfer(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "holstered-equipment-draw") && argc == 4) { HolsteredEquipment(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "holstered-equipment-transfer") && argc == 4) { HolsteredEquipment(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "matching-ammo-top-up") && argc == 4) { MatchingAmmoTransfer(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "spent-magazine-cleanup") && argc == 4) { MatchingAmmoTransfer(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "queued-reload-magazine") && argc == 4) { QueuedReloadMagazine(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "queued-reload-removed-magazine") && argc == 4) { QueuedReloadMagazine(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "inventory-cursor-commands") && argc == 4) { InventoryCursorCommands(argv[2], argv[3]); return true; }
    return false;
}
}
