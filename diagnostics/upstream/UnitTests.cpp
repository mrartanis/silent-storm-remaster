#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wUnitCommands.h"
#include "../../Main/wUnitExec.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemMap.h"
#include "../../Main/wUnitAttackExec.h"
#include "../../Main/rpgGlobal.h"
#include "../../DBFormat/DataRPG.h"
#include <cstring>

namespace UpstreamTests
{
namespace
{
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
}

bool RunUnitTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "look-action-point-cost") && argc == 4) { LookActionPointCost(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "inventory-transfer-validation") && argc == 5) { InventoryTransferValidation(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "wish-pose-strafe") && argc == 4) { WishPoseStrafe(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "hide-empty-roster-slot") && argc == 4) { HideWithEmptyRosterSlot(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "normal-state-command-lifetime") && argc == 4) { NormalStateCommandLifetime(argv[2], argv[3]); return true; }
    return false;
}
}
