#include "TestSupport.h"
#include "NativeDisplayContext.h"
#include "../../Main/wMain.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wUnitCommands.h"
#include "../../Main/wUnitExec.h"
#include "../../Main/wDebris.h"
#include "../../Main/wUICommands.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemMap.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/wUnitAttackExec.h"
#include "../../Main/rpgGlobal.h"
#include "../../Main/scScenarioTracker.h"
#include "../../Main/scFlowChart.h"
#include "../../Main/scFlowChartItems.h"
#include "../../Main/GInit.h"
#include "../../Main/GResource.h"
#include "../../Main/Gfx.h"
#include "../../Main/iMission.h"
#include "../../Main/iMissionInternal.h"
#include "../../Main/iLoading.h"
#include "../../Main/iMain.h"
#include "../../Main/iSaveManager.h"
#include "../../Main/iDesktopWindow.h"
#include "../../Main/PlayerTracker.h"
#include "../../Main/UIBaseCtrls.h"
#include "../../Main/UIInterface.h"
#include "../../Main/aiUnit.h"
#include "../../Main/aiWeapon.h"
#include "../../Main/AILog.h"
#include "../../DBFormat/DataRPG.h"
#include "../../DBFormat/DataFormat.h"
#include "../../DBFormat/DataMap.h"
#include "../../DBFormat/DataObject.h"
#include "../../DBFormat/DataMisc.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace UpstreamTests
{
namespace
{
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

void CursorClipOntoWeapon(const char* databasePath, const char* resourcePath, bool inactiveSlot)
{
    using namespace NWorld;
    ReloadFixture fixture(databasePath, resourcePath);
    auto* actor = fixture.actor;
    fixture.world->GivePlayerTurn(actor->GetTBSPlayer());
    const auto slot = inactiveSlot ? NDb::SLOT_2 : NDb::SLOT_1;
    const auto other = inactiveSlot ? NDb::SLOT_1 : NDb::SLOT_2;
    CObj<NRPG::CWeaponItem> target = fixture.weapon;
    if (fixture.inventory->Get(NDb::SLOT_2))
        fixture.inventory->TakeOff(NDb::SLOT_2);
    if (inactiveSlot) {
        target = new NRPG::CWeaponItem(fixture.weapon->GetDBWeapon());
        Require(fixture.inventory->Equip(slot, target), "fixture cannot equip its off-hand firearm");
    }
    auto* installed = dynamic_cast<NRPG::CClipItem*>(target->GetInnerClip());
    Require(installed && installed->GetItem(), "drop target has no compatible internal magazine");
    const int capacity = installed->GetMaxIncQuantity();
    installed->GetItem()->nQuantity = inactiveSlot ? capacity - 1 : 0;
    const int starting = target->GetAmmoQuantity();
    const int otherAmmo = fixture.weapon->GetAmmoQuantity();
    auto held = fixture.StockClip(2);
    auto stale = fixture.StockClip(2);
    fixture.inventory->Take(held);
    const SItem cursor(actor, SItem::HAND, held);
    actor->GetTBSPlayer()->SetInHandItem(cursor);
    const SItem destination(actor, SItem::SLOT, slot);

    EUnitCommandResult result = UCR_UNAVAILABLE;
    CObj<CCmdLoadWeapon> manual = new CCmdLoadWeapon(target, cursor);
    CObj<CCommandExecute> order = actor->CreateExecutor(manual, &result);
    Require(order && result == UCR_OK, "the normal manual load path rejected the fixture");
    const int reloadCost = order->GetStartAP();
    CObj<CCmdMoveInventoryItem> outdated = new CCmdMoveInventoryItem(SItem(actor, SItem::HAND, stale), destination);
    order = actor->CreateExecutor(outdated, &result);
    Require(!order && result != UCR_OK, "a drop accepted a magazine that was no longer on the cursor");
    if (!inactiveSlot) {
        CObj<CCmdMoveInventoryItem> empty = new CCmdMoveInventoryItem(cursor, SItem(actor, SItem::SLOT, other));
        order = actor->CreateExecutor(empty, &result);
        Require(!order && result != UCR_OK, "a magazine became directly equippable in an empty slot");
    }
    CObj<CCmdMoveInventoryItem> drop = new CCmdMoveInventoryItem(cursor, destination);
    order = actor->CreateExecutor(drop, &result);
    Require(dynamic_cast<CExecLoadWeapon*>(order.GetPtr()) && result == UCR_OK && order->GetStartAP() == reloadCost,
        "dropping a cursor clip onto an equipped firearm did not produce the normal load order");
    Require(target->GetAmmoQuantity() == starting && held->GetIncQuantity() == 2,
        "accepting the drop mutated ammunition before the load label");
    order->TimeLabelReached();
    const int remaining = inactiveSlot ? 1 : 0;
    SItem after;
    Require(target->GetAmmoQuantity() == (inactiveSlot ? capacity : 2) && held->GetIncQuantity() == remaining &&
        stale->GetIncQuantity() == 2 &&
        (inactiveSlot ? actor->GetTBSPlayer()->GetInHandItem(&after) && after.pItem == held
                      : !actor->GetTBSPlayer()->GetInHandItem(&after)),
        "the load label consumed the wrong clip, lost a partial clip, or retained an empty cursor item");
    Require(fixture.inventory->GetActive() == fixture.weapon && (!inactiveSlot || fixture.weapon->GetAmmoQuantity() == otherAmmo),
        "the drop changed the active slot or loaded a different firearm");
}

void ItemHintOnInventoryPlacement(const char* databasePath, const char* resourcePath, int mode)
{
    using namespace NWorld;
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    world->GivePlayerTurn(actor->GetTBSPlayer());
    const bool arena = mode == 4;
    const bool equipped = mode == 1 || mode == 2;
    world->bNetworkArena = arena;
    NDb::CUIHint* hint = nullptr;
    CDBIterator<NDb::CUIHint> hints(*NDatabase::GetTable<NDb::CUIHint>());
    while (hints.MoveNext()) {
        auto* candidate = hints.Get();
        if (candidate->nSequenceID > game->nHintSequenceID &&
            (!hint || candidate->GetRecordID() < hint->GetRecordID()))
            hint = candidate;
    }
    auto* record = NDb::GetWeapon(65);
    Require(record && record->pItem && hint, "fixture lacks authored equipment or a tutorial hint");
    record->pItem->pHint = hint;
    CObj<NRPG::IInventoryItem> item = new NRPG::CWeaponItem(record);
    for (const auto slot : {NDb::SLOT_1, NDb::SLOT_2})
        if (inventory->Get(slot)) inventory->TakeOff(slot);
    CObj<NRPG::IInventoryItem> blocker = new NRPG::CWeaponItem(record);
    Require(inventory->Equip(NDb::SLOT_1, blocker) && inventory->Activate(NDb::SLOT_1),
        "fixture cannot reserve the active firearm slot");
    if (mode == 3)
        Require(inventory->Equip(NDb::SLOT_2, new NRPG::CWeaponItem(record)), "fixture cannot force automatic backpack placement");
    CTPoint<int> cell;
    if (!equipped)
        Require(inventory->FindPlace(item, &cell), "fixture has no backpack space");
    const SItem destination = mode == 2 || mode == 3 ? SItem(actor, SItem::UNIT_ANYPLACE)
        : equipped ? SItem(actor, SItem::SLOT, NDb::SLOT_2) : SItem(actor, SItem::BACKPACK, cell);
    const int cursor = game->nHintSequenceID;
    const size_t known = game->hintsSet.size();
    auto countHints = [&] {
        int count = 0;
        while (CObj<CUICmd> event = world->GetUICommand())
            if (auto* shown = dynamic_cast<CUICmdShowHint*>(event.GetPtr()); shown && shown->pHint == hint)
                ++count;
        return count;
    };
    Require(countHints() == 0, "the item hint is already queued");
    for (int attempt = 0; attempt < 2; ++attempt) {
        const SItem held(actor, SItem::HAND, item);
        actor->GetTBSPlayer()->SetInHandItem(held);
        CObj<CCmdMoveInventoryItem> command = new CCmdMoveInventoryItem(held, destination);
        EUnitCommandResult result = UCR_UNAVAILABLE;
        CObj<CCommandExecute> order = actor->CreateExecutor(command, &result);
        Require(order && result == UCR_OK, "the item transfer could not be issued");
        order->Run();
        if (equipped)
            Require(inventory->TakeOff(NDb::SLOT_2) == item, "the hinted item did not reach the requested slot");
        else {
            Require(std::any_of(inventory->GetItems().begin(), inventory->GetItems().end(),
                [&](const auto& entry) { return entry.pItem == item; }), "the hinted item did not reach the backpack");
            inventory->Take(item);
        }
        Require(countHints() == (!arena && attempt == 0 ? 1 : 0),
            "the item did not produce exactly one campaign hint across repeated transfers");
        Require(game->nHintSequenceID == cursor && game->hintsSet.size() == known + (arena ? 0 : 1) &&
            (arena || game->hintsSet.back() == hint), "the item hint altered book progression or the wrong journal entry");
    }
}

static void InventoryWithoutUniform()
{
    CObj<NRPG::CUnit> owner = new NRPG::CUnit;
    CObj<NRPG::IInventory> inventory = NRPG::CreateInventory(owner);
    CObj<NDb::CRPGUniform> uniform = new NDb::CRPGUniform;
    for (auto& slot: uniform->subTypes)
        for (auto& priority: slot.priorities) priority = NDb::SUBTYPE_NONE;
    uniform->subTypes[1].priorities[1] = NDb::SUBTYPE_KNIFE;
    uniform->subTypes[2].priorities[0] = NDb::SUBTYPE_KNIFE;
    owner->pUniform = uniform;
    Require(inventory->GetPlaceBySubType(NDb::SUBTYPE_KNIFE) == 2,
        "uniform placement control did not preserve preferred-subtype-only matching");
    owner->pUniform = nullptr;
    Require(inventory->GetUniform() == nullptr, "uniform-less fixture unexpectedly retained a uniform");
    Require(inventory->GetPlaceBySubType(NDb::SUBTYPE_KNIFE) == -1,
        "uniform-less NPC inventory invented a belt placement");
    CObj<NRPG::IInventory> ownerless = NRPG::CreateInventory(nullptr);
    Require(ownerless->GetPlaceBySubType(NDb::SUBTYPE_HEAVY) == -1,
        "ownerless inventory invented a belt placement");
    owner->pUniform = uniform;
    Require(inventory->GetPlaceBySubType(NDb::SUBTYPE_KNIFE) == 2,
        "restoring a uniform did not restore its original belt placement");
}

static void InventoryClueCreation(const char* databasePath, const char* resourcePath, bool checkCount = false, bool noSpace = false)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    auto* clue = game->pScenarioTracker->GetClueByName("Find_Cipher_key");
    Require(clue && clue->GetDBClue()->clueType == NDb::CT_ITEM, "fixture has no item clue");
    auto* record = NDb::GetRPGItem(clue->GetDBClue()->nItemID);
    Require(record != nullptr, "fixture item clue has no RPG record");
    // A pure quest item has no ordinary-item successor. Keep fixture changes
    // in the loaded database only; no game.db or resource copy is written.
    record->pSuccessor = nullptr;
    record->sSize = noSpace ? CTPoint<int>(100, 100) : CTPoint<int>(1, 1);
    CDBIterator<NDb::CUnit> units(*NDatabase::GetTable<NDb::CUnit>());
    while (units.MoveNext()) {
        auto* unit = units.Get();
        unit->bClueSlot = false;
        unit->bClueInventorySlot = IsValid(unit->pMonster) && !IsValid(unit->pMonster->pPanzerklein);
    }
    // MapBuild's item 188 is the floor clue-slot marker. This fixture routes
    // every candidate slot through a normal human's backpack instead.
    CDBIterator<NDb::CPlacableObject> objects(*NDatabase::GetTable<NDb::CPlacableObject>());
    while (objects.MoveNext()) {
        auto* object = objects.Get();
        if (IsValid(object->pRPGItem) && object->pRPGItem->GetRecordID() == 188)
            object->pRPGItem = nullptr;
    }
    CObj<NWorld::CWorld> world = new NWorld::CWorld(game);
    CObj<NWorld::CPostWorldCreateInfo> post;
    world->CreateRandom(810, {}, true, std::list<CPtr<NScenario::CScenarioClue>>{clue}, 0, &post, SRandomSeed(123));
    vector<CPtr<NWorld::CUnit>> actors;
    world->GetAllUnits(&actors);
    int found = 0;
    for (auto& actor : actors) {
        auto* inventory = actor->GetRPG()->GetInventoryInfo();
        int carried = 0;
        for (const auto& entry : inventory->GetItems())
            if (dynamic_cast<NRPG::IClueItem*>(entry.pItem.GetPtr()) && entry.pItem->GetDBItem() == record)
                ++carried;
        found += carried;
        if (checkCount)
            Require(dynamic_cast<NWorld::CUnitServer*>(actor.GetPtr())->nClueCount == carried,
                "inventory clue placement did not count the clue on its carrier");
    }
    if (noSpace)
        Require(found == 0, "inventory clue placement inserted a quest item without room in the backpack");
    else
        Require(found == 1, "inventory clue placement lost a quest item without an ordinary successor");
}

static void ReadHistoricalMissingClip(NRPG::CWeaponItem* weapon)
{
    CStructureNetworkContext refs;auto original=BorrowedObjectFields(weapon,&refs);
    CMemoryStream legacy;CObj<NRPG::CClipItem> missing;
    CDBPtr<NDb::CRPGWeapon> record=weapon->GetDBWeapon();auto mode=weapon->GetShootMode();bool working=weapon->IsWorking();
    // Keep all five historical fields: an omitted bool is read as false, not left unchanged.
    {CStructureSaver writer(legacy,CStructureSaver::WRITE,&refs);writer.Add(1,static_cast<NRPG::CInventoryItem*>(weapon));writer.Add(2,&missing);writer.Add(3,&mode);writer.Add(4,&record);writer.Add(5,&working);}legacy.Seek(0);
    {CStructureSaver reader(legacy,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(weapon)->operator&(reader);}
    Require(weapon->GetShootMode()==mode && weapon->IsWorking()==working,"historical full weapon archive changed unrelated saved fields");
}

static void WeaponClipArchive(const char* databasePath,bool missing)
{
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);
    CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());int count=0,states=0,overrides=0,alternate=0,skipped=0;
    while(weapons.MoveNext()){
        auto* record=weapons.Get();bool shootable=false;for(int i=0;i<NDb::SM_MAXVALUE;++i)if(record->shootModes[i])shootable=true;
        if(!record->pItem || !record->pInnerClip || !record->pWeaponType || !shootable){++skipped;continue;}
        ++count;if(record->nInnerClipAmmoQuantity>0 && record->nInnerClipAmmoQuantity!=record->pInnerClip->nQuantity)++overrides;
        for(int state=0;state<3;++state){
            CObj<NRPG::CWeaponItem> source=new NRPG::CWeaponItem(record);auto* clip=source->GetInnerClip();Require(clip && IsValid(clip),"authored weapon constructor did not create its mandatory real clip");
            if(state==1)source->Damage();if(state==2)source->DiscardAmmo();
            // Exercise a valid nondefault ammo color too, without editing any DB record.
            if(state==2 && !missing){CDBIterator<NDb::CRPGAmmo> ammo(*NDatabase::GetTable<NDb::CRPGAmmo>());while(ammo.MoveNext()){auto* a=ammo.Get();if(a->nAmmoGroup!=record->pInnerClip->nAmmoGroup || a->color==clip->GetDBAmmo()->color)continue;CObj<NRPG::IInventoryItem> spare=NRPG::CreateClipItem(record->pInnerClip,a,1);Require(source->Load(dynamic_cast<NRPG::IClipItem*>(spare.GetPtr())),"archive control could not load actual compatible alternate ammunition");++alternate;break;}}
            const int before=clip->GetIncQuantity(),maximum=clip->GetMaxIncQuantity(),ammoID=clip->GetDBAmmo()->GetRecordID();const auto mode=source->GetShootMode();const bool working=source->IsWorking();
            CStructureNetworkContext refs;auto saved=BorrowedObjectFields(source,&refs);CObj<NRPG::CWeaponItem> restored=dynamic_cast<NRPG::CWeaponItem*>(pSSClasses->CreateObject(pSSClasses->GetObjectTypeID(source)));
            Require(restored,"weapon clip archive has no native factory class");{CStructureSaver reader(saved,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);}
            Require(restored->GetInnerClip()==clip && clip->GetIncQuantity()==before && clip->GetMaxIncQuantity()==maximum && clip->GetDBAmmo()->GetRecordID()==ammoID,"valid weapon archive replaced its existing clip/ammo or capacity");
            if(missing)ReadHistoricalMissingClip(restored);
            auto* after=restored->GetInnerClip();
            std::printf("native weapon clip archive: missing=%d weapon=%d state=%d present=%d quantity=%d maximum=%d expectedMaximum=%d before=%d ammo=%d expectedAmmo=%d working=%d expectedWorking=%d override=%d\n",int(missing),record->GetRecordID(),state,int(IsValid(after)),after?after->GetIncQuantity():-1,after?after->GetMaxIncQuantity():-1,maximum,before,after?after->GetDBAmmo()->GetRecordID():-1,ammoID,int(restored->IsWorking()),int(working),record->nInnerClipAmmoQuantity);std::fflush(stdout);
            Require(after && IsValid(after),"historical weapon archive retains a null inner clip after loading");
            Require(after->GetIncQuantity()==(missing?0:before) && after->GetMaxIncQuantity()==maximum && restored->GetDBWeapon()==record && restored->GetDBItem()==record->pItem && restored->GetShootMode()==mode && restored->IsWorking()==working,"weapon clip restoration grants ammunition or changes valid weapon state/capacity");
            if(!missing)Require(after->GetDBAmmo()->GetRecordID()==ammoID,"valid alternate-ammo weapon archive changed ammo type");
            NRPG::SWeaponInfo info;restored->GetInfo(&info);Require(info.nMinRange==record->nMinRange && info.nMaxRange==record->nMaxRange,"restored weapon cannot supply its real tooltip/combat information");++states;
        }
    }
    std::printf("native weapon clip archive summary: missing=%d weapons=%d states=%d overrides=%d alternate=%d skipped=%d\n",int(missing),count,states,overrides,alternate,skipped);std::fflush(stdout);
    Require(count>90 && states==count*3 && overrides>0 && (missing || alternate>0),"weapon clip archive did not cover its authored valid/empty/damaged/override/ammo corpus");
}

static void HandContainerCharges(const char* databasePath,const char* resourcePath,bool right,bool legacyClip=false)
{
    Require(std::getenv("S2_USER_DATA_DIR"),"hand charges fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"hand charges started with a live interface");
    NativeDisplayContext display;display.Initialize(databasePath,resourcePath,"Silent Storm native hand charges regression");
    NMainLoop::CreateProfile("UpstreamHandCharges");NMainLoop::SetActiveProfile("UpstreamHandCharges");NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(-1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginMission> enter=new NGame::CICBeginMission(810,-1,vector<string>(),game);enter->Exec();
    CObj<NGame::CMission> mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(mission && mission->GetWorld() && mission->GetDesktop() && mission->GetInterface(),"hand charges did not open the actual tactical interface");
    CObj<NMainLoop::CInterfaceCommand> startup=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(startup.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"hand charges did not isolate its unrelated startup save");
    NWorld::IPlayer::CUnitSet party;mission->GetActivePlayer()->GetPlayer()->GetUnits(&party);Require(party.size()==1,"hand charges lacks its actual single party unit");
    auto* actor=dynamic_cast<NWorld::CUnitServer*>(party.begin()->GetPtr());Require(actor && actor->CanFight(),"hand charges lacks its actual native party server");
    auto* inventory=actor->GetUnitRPG()->GetInventory();auto* gun=dynamic_cast<NRPG::IWeaponItemInfo*>(inventory->Get(NDb::SLOT_1));
    Require(gun && gun->GetInnerClip(),"hand charges lacks its loaded native weapon formatting control");
    mission->GetActivePlayer()->Update(true);mission->Select(actor);Require(mission->CountSelected()==1,"hand charges did not synchronize its real selection");
    auto find=[](auto&& self,NUI::CWindow* window,const char* id)->NUI::CWindow*{if(window->GetWindowID()==id)return window;list<CPtr<NUI::CWindow>> children;window->GetChildrenList(&children);for(auto child:children)if(auto* result=self(self,child,id))return result;return nullptr;};
    auto* left=find(find,mission->GetDesktop(),"slot_left");auto* slot=find(find,mission->GetDesktop(),right?"slot_right":"slot_left");
    Require(left && slot && pSSClasses->GetObjectTypeID(slot)==int(0xB0521155),"hand charges did not locate the real template-built hand slot");
    auto read=[](NUI::CWindow* window,NUI::CText** text,NUI::CWindow** reload,NUI::CWindow** fade){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(window,&refs);CPtr<NUI::CText> caption;CPtr<NUI::CWindow> button,cover;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(5,&cover);reader.Add(6,&caption);reader.Add(7,&button);}*text=caption;*reload=button;*fade=cover;Require(*text && *reload && *fade,"hand charges lost its native caption/button/fade children");};
    STime time=1000;auto draw=[&](){mission->GetInterface()->Draw(time);NGScene::Flip();MarkNewDGFrame();time+=10;};draw();
    NUI::CText* baseline=nullptr;NUI::CWindow* ignoredReload=nullptr;NUI::CWindow* ignoredFade=nullptr;read(left,&baseline,&ignoredReload,&ignoredFade);
    const wstring original=baseline->GetText();const auto last=original.rfind(L'>');Require(last!=wstring::npos && baseline->GetStyle(NUI::STYLE_VISIBLE) && ignoredReload->GetStyle(NUI::STYLE_VISIBLE),"hand charges did not paint its actual weapon counter");
    const wstring prefix=original.substr(0,last+1);Require(prefix.find(L"face=Impact size=36pt")!=wstring::npos,"hand charges lost the fork's existing weapon text style");
    std::printf("native hand charge baseline: side=%d text=%ls DBprefixSame=%d\n",int(right),original.c_str(),int(NUI::GetDBString(18759)==prefix));std::fflush(stdout);
    CObj<NRPG::IInventoryItem> weapon=inventory->TakeOff(NDb::SLOT_1),oldRight;
    if(inventory->Get(NDb::SLOT_2))oldRight=inventory->TakeOff(NDb::SLOT_2);
    const auto target=right?NDb::SLOT_2:NDb::SLOT_1,other=right?NDb::SLOT_1:NDb::SLOT_2;
    NUI::CText* caption=nullptr;NUI::CWindow* reload=nullptr;NUI::CWindow* fade=nullptr;read(slot,&caption,&reload,&fade);
    int checks=0,failures=0;CObj<NRPG::IInventoryItem> previous;
    auto put=[&](NRPG::IInventoryItem* item){if(inventory->Get(target))previous=inventory->TakeOff(target);if(item)Require(inventory->Equip(target,item) && inventory->Activate(target),"hand charges cannot equip/activate its actual item");};
    auto observe=[&](const char* stage,int quantity,int maximum,bool counter,bool reloadButton,bool inactive=false){
        auto* item=inventory->Get(target);auto* container=dynamic_cast<NRPG::IItemContainerInfo*>(item);const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),dose=container?container->GetIncQuantity():-1;
        const int active=inventory->GetActiveSlot();draw();NUI::SPoint extent;caption->GetRealSize(&extent);
        const wstring expected=prefix+std::to_wstring(quantity)+L"/"+std::to_wstring(maximum);const bool textOK=!counter || caption->GetText()==expected;
        const bool good=slot->GetStyle(NUI::STYLE_VISIBLE) && caption->GetStyle(NUI::STYLE_VISIBLE)==counter && reload->GetStyle(NUI::STYLE_VISIBLE)==reloadButton && textOK && fade->GetStyle(NUI::STYLE_VISIBLE)==inactive;
        std::printf("native hand charges: side=%d stage=%s visible=%d reload=%d expectedVisible=%d expectedReload=%d quantity=%d maximum=%d textOK=%d fade=%d inactive=%d slotVisible=%d width=%d height=%d AP=%d dose=%d good=%d\n",int(right),stage,int(caption->GetStyle(NUI::STYLE_VISIBLE)),int(reload->GetStyle(NUI::STYLE_VISIBLE)),int(counter),int(reloadButton),quantity,maximum,int(textOK),int(fade->GetStyle(NUI::STYLE_VISIBLE)),int(inactive),int(slot->GetStyle(NUI::STYLE_VISIBLE)),extent.x,extent.y,ap,dose,int(good));
        std::printf("native hand charge text: side=%d stage=%s text=%ls\n",int(right),stage,caption->GetText().c_str());std::fflush(stdout);
        Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && inventory->Get(target)==item && inventory->GetActiveSlot()==active && (!container || container->GetIncQuantity()==dose),"drawing hand charges spent AP/health/charges or changed inventory/active hand");
        if(counter && good)Require(extent.x>0 && extent.y>0,"visible hand charges produced empty native text layout");++checks;if(!good)++failures;
    };
    if(legacyClip){
        CObj<NRPG::CWeaponItem> restored=new NRPG::CWeaponItem(gun->GetDBWeapon());const int maximum=restored->GetInnerClip()->GetMaxIncQuantity(),oldQuantity=restored->GetInnerClip()->GetIncQuantity();
        ReadHistoricalMissingClip(restored);auto* inner=restored->GetInnerClip();
        std::printf("native legacy hand clip after read: side=%d weapon=%d present=%d quantity=%d maximum=%d expectedMaximum=%d oldQuantity=%d\n",int(right),restored->GetDBWeapon()->GetRecordID(),int(IsValid(inner)),inner?inner->GetIncQuantity():-1,inner?inner->GetMaxIncQuantity():-1,maximum,oldQuantity);std::fflush(stdout);
        Require(inner && IsValid(inner),"historical weapon archive retains a null inner clip after loading");Require(inner->GetIncQuantity()==0 && inner->GetMaxIncQuantity()==maximum,"legacy hand clip restoration granted free ammunition or lost authored capacity");
        NRPG::SWeaponInfo info;restored->GetInfo(&info);put(restored);observe("legacy-empty-weapon",0,maximum,true,true);
        // A real compatible spare is unloaded from the retained original weapon, never fabricated.
        auto* originalWeapon=dynamic_cast<NRPG::CWeaponItem*>(weapon.GetPtr());Require(originalWeapon && originalWeapon->Unload(inventory) && restored->CanReload(inventory),"legacy hand clip has no actual unloaded compatible spare");
        const int ap=actor->GetAP();Require(restored->Reload(inventory),"restored legacy weapon cannot reload through the native inventory API");const int loaded=restored->GetAmmoQuantity();
        Require(loaded>0 && loaded<=maximum && actor->GetAP()==ap,"native low-level reload changed AP or overfilled the restored clip");observe("legacy-reloaded",loaded,maximum,true,true);
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(restored,&refs);CObj<NRPG::CWeaponItem> again=dynamic_cast<NRPG::CWeaponItem*>(pSSClasses->CreateObject(pSSClasses->GetObjectTypeID(restored)));
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(again.GetPtr())->operator&(reader);}Require(again && again->GetInnerClip()==restored->GetInnerClip() && again->GetAmmoQuantity()==loaded,"saving/restoring a repaired legacy weapon reset its loaded clip");
        std::printf("native legacy hand clip summary: side=%d checks=%d failures=%d loaded=%d maximum=%d AP=%d\n",int(right),checks,failures,loaded,maximum,ap);std::fflush(stdout);
        Require(checks==2 && failures==0,"restored legacy weapon cannot draw its actual full hand UI");return;
    }
    auto* ordinary=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(1);auto* repair=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(26);Require(ordinary && repair && ordinary->nQuantity>1 && repair->nQuantity>1,"hand charges lost its authored medical/repair records");
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(ordinary);put(kit);observe("medical-full",ordinary->nQuantity,ordinary->nQuantity,true,false);
    kit->SpendPotion();observe("medical-spent",ordinary->nQuantity-1,ordinary->nQuantity,true,false);
    while(!kit->IsEmpty())kit->SpendPotion();observe("medical-empty",0,ordinary->nQuantity,true,false);
    CObj<NRPG::CFirstAidItem> repairKit=new NRPG::CFirstAidItem(repair);put(repairKit);observe("repair",repair->nQuantity,repair->nQuantity,true,false);
    NDb::CRPGTool* toolRecord=nullptr;CDBIterator<NDb::CRPGTool> tools(*NDatabase::GetTable<NDb::CRPGTool>());while(tools.MoveNext()){auto* record=tools.Get();if(record->pItem && record->nCharges>1 && (!toolRecord || record->GetRecordID()<toolRecord->GetRecordID()))toolRecord=record;}Require(toolRecord,"hand charges lacks a real multi-charge tool");
    CObj<NRPG::CToolItem> tool=new NRPG::CToolItem(toolRecord);put(tool);observe("tool-full",toolRecord->nCharges,toolRecord->nCharges,true,false);tool->SpendOneCharge();observe("tool-spent",toolRecord->nCharges-1,toolRecord->nCharges,true,false);
    NDb::CRPGPicklock* pickRecord=nullptr;CDBIterator<NDb::CRPGPicklock> picks(*NDatabase::GetTable<NDb::CRPGPicklock>());while(picks.MoveNext()){auto* record=picks.Get();if(record->pItem && record->nQuantity>1 && (!pickRecord || record->GetRecordID()<pickRecord->GetRecordID()))pickRecord=record;}Require(pickRecord,"hand charges lacks a real multi-charge picklock");
    CObj<NRPG::CPicklockItem> pick=new NRPG::CPicklockItem(pickRecord);put(pick);observe("picklock-full",pickRecord->nQuantity,pickRecord->nQuantity,true,false);pick->SpendCharge();observe("picklock-spent",pickRecord->nQuantity-1,pickRecord->nQuantity,true,false);
    NDb::CRPGGrenade* grenadeRecord=nullptr;CDBIterator<NDb::CRPGGrenade> grenades(*NDatabase::GetTable<NDb::CRPGGrenade>());while(grenades.MoveNext()){auto* record=grenades.Get();if(record->pItem && (!grenadeRecord || record->GetRecordID()<grenadeRecord->GetRecordID()))grenadeRecord=record;}Require(grenadeRecord,"hand charges lacks a real ordinary grenade control");
    CObj<NRPG::CGrenadeItem> grenade=new NRPG::CGrenadeItem(grenadeRecord);Require(!dynamic_cast<NRPG::IItemContainerInfo*>(grenade.GetPtr()),"ordinary grenade unexpectedly exposes a charge container");put(grenade);observe("grenade",0,0,false,false);
    put(nullptr);observe("empty-hand",0,0,false,false);
    CObj<NRPG::CFirstAidItem> again=new NRPG::CFirstAidItem(ordinary),otherKit=new NRPG::CFirstAidItem(ordinary);put(again);Require(inventory->Equip(other,otherKit) && inventory->Activate(other),"hand charges cannot prepare its actual inactive-hand control");observe("medical-inactive",ordinary->nQuantity,ordinary->nQuantity,true,false,true);
    CObj<NRPG::IInventoryItem> removedOther=inventory->TakeOff(other);put(weapon);observe("weapon",gun->GetInnerClip()->GetIncQuantity(),gun->GetInnerClip()->GetMaxIncQuantity(),true,true);
    put(nullptr);observe("removed",0,0,false,false);
    std::printf("native hand charges summary: side=%d checks=%d failures=%d tool=%d picklock=%d grenade=%d\n",int(right),checks,failures,toolRecord->GetRecordID(),pickRecord->GetRecordID(),grenadeRecord->GetRecordID());std::fflush(stdout);
    Require(checks==13 && failures==0,"actual tactical hand slot hides charge counters for authored medical/tool containers");
}

static void ExhaustedHintPickups(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    Require(!units.empty() && !game->bNoMoreHints && game->hintsSet.empty(),
        "hint exhaustion fixture did not start with an unconsumed authored sequence");
    int candidates = 0;
    CDBIterator<NDb::CUIHint> records(*NDatabase::GetTable<NDb::CUIHint>());
    while (records.MoveNext()) if (records.Get()->nSequenceID > game->nHintSequenceID) ++candidates;
    Require(candidates >= 2, "hint exhaustion fixture needs at least two remaining authored hints");
    CObj<NRPG::IInventoryItem> first = NRPG::CreateHintItem(), second = NRPG::CreateHintItem();
    Require(first && second && dynamic_cast<NRPG::IHintItem*>(first.GetPtr()) &&
        dynamic_cast<NRPG::IHintItem*>(second.GetPtr()), "hint exhaustion fixture did not create genuine hint items");
    CObj<NRPG::IInventoryItem> ordinary = NRPG::CreateDummyItem(first->GetDBItem());
    Require(ordinary && !dynamic_cast<NRPG::IHintItem*>(ordinary.GetPtr()) &&
        ordinary->GetDBItem() == first->GetDBItem(), "hint exhaustion marker control differs from the authored book record");
    const auto point = units.front()->GetPosition().GetCP();
    CPtr<NWorld::CDFrozenItem> bodyA = world->AddFrozenItem(world->GetAIMap(),point,QNULL,first);
    CPtr<NWorld::CDFrozenItem> bodyB = world->AddFrozenItem(world->GetAIMap(),point+CVec3(1,0,0),QNULL,second);
    CPtr<NWorld::CDFrozenItem> otherBody = world->AddFrozenItem(world->GetAIMap(),point+CVec3(0,1,0),QNULL,ordinary);
    Require(bodyA && bodyB && otherBody && world->GetFrozenItem(first) == bodyA.GetPtr() &&
        world->GetFrozenItem(second) == bodyB.GetPtr() && world->GetFrozenItem(ordinary) == otherBody.GetPtr(),
        "hint exhaustion fixture could not place its actual frozen pickups");
    while (CObj<NWorld::CUICmd> pending = world->GetUICommand()) {}
    world->AddNextUIHint(true);
    Require(!game->bNoMoreHints && game->hintsSet.size() == 1 &&
        world->GetFrozenItem(first) == bodyA.GetPtr() && world->GetFrozenItem(second) == bodyB.GetPtr() &&
        world->GetFrozenItem(ordinary) == otherBody.GetPtr(), "non-exhausted hints removed their books or ordinary control");
    for (int step=0;step<candidates && !game->bNoMoreHints;++step) world->AddNextUIHint(true);
    Require(game->bNoMoreHints, "actual authored hint sequence did not exhaust within its record count");
    std::printf("native hint sequence exhausted: hints=%zu remaining-books=%d ordinary-retained=%d\n",
        game->hintsSet.size(),int(bool(world->GetFrozenItem(first)))+int(bool(world->GetFrozenItem(second))),
        int(world->GetFrozenItem(ordinary) == otherBody.GetPtr()));
    Require(!world->GetFrozenItem(first) && !world->GetFrozenItem(second),
        "exhausting the authored hint sequence left obsolete hint pickups in the world");
    Require(world->GetFrozenItem(ordinary) == otherBody.GetPtr() && IsValid(otherBody),
        "hint exhaustion removed an ordinary item sharing the same authored book record");
    const auto shown = game->hintsSet.size(); const auto cursor = game->nHintSequenceID;
    world->AddNextUIHint(true);
    Require(game->bNoMoreHints && game->hintsSet.size() == shown && game->nHintSequenceID == cursor &&
        world->GetFrozenItem(ordinary) == otherBody.GetPtr(), "repeated exhausted hint cleanup changed progression or ordinary items");
    Require(!world->IsAction(), "hint exhaustion started a gameplay action");
}

static void AuthoredHintPlacement(const char* databasePath,const char* resourcePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    auto* book = NDb::GetRPGItem(438);
    Require(book && IsValid(book->pModel), "hint map fixture has no authored book record/model");
    auto* variant = NDb::GetTemplVariant(810);
    Require(variant && IsValid(variant->pTemplate), "hint map fixture has no authored root template");
    // Add a marker in the loaded database using initialized authored placement
    // fields and the real book record. Nothing is written back to game assets.
    CDBIterator<NDb::CFinalElement> placements(*NDatabase::GetTable<NDb::CFinalElement>());
    Require(placements.MoveNext(), "hint map fixture has no authored placement fields");
    CObj<NDb::CFinalElement> element = new NDb::CFinalElement;
    CStructureNetworkContext references; auto fields = BorrowedObjectFields(placements.Get(),&references);
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); element->operator&(reader); }
    CObj<NDb::CPlacableObject> marker = new NDb::CPlacableObject; marker->pRPGItem = book;
    element->pObject = marker; element->pVariant = variant;
    element->ptPos = CVec2(variant->pTemplate->nWidth*0.5f,variant->pTemplate->nHeight*0.5f);
    element->nFloor = 0; element->fDZ = 0.15f; element->fRotation = 37.f; element->ptScale = CVec3(1,1,1);
    element->szName = "UpstreamHintPlacement"; element->pChest = nullptr; element->pGrenade = nullptr;
    element->bIsLocked = element->bArmed = false; element->fPower = element->fRadius = 0;
    variant->pFinalElements.push_back(element.GetPtr());
    const int replaced = 1;
    auto create = [&](NRPG::CGlobalGame* game,bool arena) {
        CObj<NWorld::CWorld> world = new NWorld::CWorld(game);
        world->bNetworkArena = arena;
        CObj<NWorld::CPostWorldCreateInfo> post;
        world->CreateRandom(810,{},true,{},0,&post,SRandomSeed(123));
        Require(post && world->GetPathNetwork(), "hint map fixture failed native world creation");
        return world;
    };
    auto count = [](NWorld::CWorld* world) {
        vector<CPtr<NRPG::IInventoryItem>> items; world->GetGroundItemsForDiagnostics(&items);
        std::pair<int,int> counts{0,0};
        for (const auto& item: items) if (item && item->GetDBItem()->GetRecordID() == 438) {
            ++counts.first;
            if (dynamic_cast<NRPG::IHintItem*>(item.GetPtr())) ++counts.second;
        }
        return counts;
    };
    CObj<NRPG::CGlobalGame> arenaGame = NRPG::CreateGlobalGame(1);
    { auto arena = create(arenaGame,true); const auto counts = count(arena);
      Require(counts.second == 0 && !arenaGame->bNoMoreHints,
          "campaign hint pickups leaked into our multiplayer arena"); }
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    CObj<NWorld::CWorld> world = create(game,false);
    const auto counts = count(world);
    std::printf("native hint markers: converted=%d map-book-records=%d interactive-hints=%d\n",
        replaced,counts.first,counts.second);
    Require(counts.first > 0, "hint map fixture did not reach its authored marker placements");
    Require(counts.second == counts.first,
        "authored hint placements became inert ordinary items instead of interactive hint pickups");
    const auto initialCursor = game->nHintSequenceID;
    Require(!game->bNoMoreHints && game->hintsSet.empty(), "placing hint books advanced their sequence");
    int hints = 0; CDBIterator<NDb::CUIHint> records(*NDatabase::GetTable<NDb::CUIHint>());
    while (records.MoveNext()) ++hints;
    for (int step=0;step<=hints && !game->bNoMoreHints;++step) world->AddNextUIHint(true);
    Require(game->bNoMoreHints && game->nHintSequenceID > initialCursor && count(world).first == 0,
        "actual hint exhaustion did not clean up the generated pickups");
    world = nullptr;
    const auto cursor = game->nHintSequenceID; const auto shown = game->hintsSet.size();
    world = create(game,false);
    Require(count(world).first == 0 && game->bNoMoreHints && game->nHintSequenceID == cursor &&
        game->hintsSet.size() == shown, "an exhausted campaign respawned books or changed hint progression");
}

struct ReloadLogFixture
{
    CObj<NAI::CAIFireArmsWeapon> weapon;
    CObj<NAI::CAIFireArmsWeaponClip> current, next;
};

static CStructureNetworkContext ReloadReferences(const ReloadLogFixture& fixture)
{
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[fixture.weapon.GetPtr()] = 1; references.refs[1] = fixture.weapon.GetPtr();
    references.ids[fixture.current.GetPtr()] = 2; references.refs[2] = fixture.current.GetPtr();
    references.ids[fixture.next.GetPtr()] = 3; references.refs[3] = fixture.next.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    return references;
}

static ReloadLogFixture MakeReloadLogFixture()
{
    ReloadLogFixture fixture;
    fixture.weapon = new NAI::CAIFireArmsWeapon;
    fixture.current = new NAI::CAIFireArmsWeaponClip; fixture.current->SetAmmoCount(3);
    fixture.next = new NAI::CAIFireArmsWeaponClip; fixture.next->SetAmmoCount(12);
    struct FirearmClipsWire {
        CObj<NAI::CAIFireArmsWeaponClip> current;
        vector<CObj<NAI::CAIFireArmsWeaponClip>> spare;
        int operator&(CStructureSaver& saver) { saver.Add(3, &current); saver.Add(4, &spare); return 0; }
    } base;
    base.current = fixture.current.GetPtr(); base.spare.push_back(fixture.next.GetPtr());
    auto references = ReloadReferences(fixture);
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); writer.Add(2, &base); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(fixture.weapon.GetPtr())->operator&(reader); }
    Require(fixture.weapon->GetCurrentClip() == fixture.current.GetPtr()
            && fixture.weapon->GetNextClip() == fixture.next.GetPtr(), "native AI reload clip fixture failed to restore");
    return fixture;
}

static CObj<NAI::CAILogReloadWeapon> RestoreRetailReloadLog(const ReloadLogFixture& fixture)
{
    CObj<NAI::CAILogReloadWeapon> restored = new NAI::CAILogReloadWeapon;
    auto references = ReloadReferences(fixture);
    CPtr<NAI::CAIFireArmsWeapon> weapon = fixture.weapon.GetPtr();
    CPtr<NAI::CAIFireArmsWeaponClip> next = fixture.next.GetPtr();
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); writer.Add(3, &weapon); writer.Add(4, &next); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
    return restored;
}

static void AIReloadLogRecord(int check)
{
    auto fixture = MakeReloadLogFixture();
    if (check == 0) {
        CObj<NAI::CAILogReloadWeapon> created = new NAI::CAILogReloadWeapon(nullptr, fixture.weapon);
        auto references = ReloadReferences(fixture);
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
          static_cast<CObjectBase*>(created.GetPtr())->operator&(writer); }
        saved.Seek(0);
        CPtr<NAI::CAIFireArmsWeaponClip> next, obsolete;
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(4, &next); reader.Add(5, &obsolete); }
        Require(next == fixture.next.GetPtr(), "AI reload record did not write the next clip in retail save tag 4");
        Require(obsolete == nullptr, "AI reload record retained its obsolete clip save tag 5");
        auto restored = RestoreRetailReloadLog(fixture);
        CPtr<NAI::CAIFireArmsWeaponClip> expected = fixture.next.GetPtr();
        fixture.next = nullptr;
        restored->Commit();
        Require(IsValid(expected) && fixture.weapon->GetCurrentClip() == expected.GetPtr()
                && expected->GetAmmoCount() == 12 && fixture.weapon->GetNextClip() == nullptr,
            "restored retail reload record failed to transfer spare-clip ownership before removing it");
    } else if (check == 1) {
        auto restored = RestoreRetailReloadLog(fixture);
        restored->RollBack();
        Require(fixture.weapon->GetCurrentClip() == fixture.current.GetPtr()
                && fixture.current->GetAmmoCount() == 3 && fixture.next->GetAmmoCount() == 12,
            "restored AI reload rollback changed the loaded magazine without committing");
        restored->Commit();
        Require(fixture.weapon->GetCurrentClip() == fixture.next.GetPtr(), "AI reload commit did not install the next clip");
    } else {
        CObj<NAI::CAILogReloadWeapon> created = new NAI::CAILogReloadWeapon(nullptr, fixture.weapon);
        CPtr<NAI::CAIFireArmsWeaponClip> old = fixture.current.GetPtr(), next = fixture.next.GetPtr();
        fixture.current = nullptr; fixture.next = nullptr;
        created->Commit();
        Require(!IsValid(old), "committed AI reload log retained ownership of the obsolete magazine");
        Require(IsValid(next) && fixture.weapon->GetCurrentClip() == next.GetPtr() && next->GetAmmoCount() == 12,
            "AI reload log lost the loaded spare or changed its ammunition during ownership transfer");
    }
}
}

bool RunInventoryTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "inventory-transfer-validation") && argc == 5) { InventoryTransferValidation(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "inventory-transfer-inactive-pose") && argc == 4) { InventoryTransferInactivePose(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "carried-clue-slot") && argc == 4) { CarriedClueTransfer(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "carried-clue-auto-placement") && argc == 4) { CarriedClueTransfer(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "holstered-equipment-draw") && argc == 4) { HolsteredEquipment(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "holstered-equipment-transfer") && argc == 4) { HolsteredEquipment(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "matching-ammo-top-up") && argc == 4) { MatchingAmmoTransfer(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "spent-magazine-cleanup") && argc == 4) { MatchingAmmoTransfer(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "queued-reload-magazine") && argc == 4) { QueuedReloadMagazine(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "queued-reload-removed-magazine") && argc == 4) { QueuedReloadMagazine(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "inventory-cursor-commands") && argc == 4) { InventoryCursorCommands(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "cursor-clip-onto-weapon") && argc == 5) { CursorClipOntoWeapon(argv[2], argv[3], std::atoi(argv[4]) != 0); return true; }
    if (!std::strcmp(mode, "item-hint-on-inventory-placement") && argc == 5) { ItemHintOnInventoryPlacement(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "inventory-without-uniform")) { InventoryWithoutUniform(); return true; }
    if (!std::strcmp(mode, "weapon-clip-archive-valid") && argc==3) { WeaponClipArchive(argv[2],false); return true; }
    if (!std::strcmp(mode, "weapon-clip-archive-legacy") && argc==3) { WeaponClipArchive(argv[2],true); return true; }
    if (!std::strcmp(mode, "legacy-hand-clip-left") && argc==4) { HandContainerCharges(argv[2],argv[3],false,true); return true; }
    if (!std::strcmp(mode, "legacy-hand-clip-right") && argc==4) { HandContainerCharges(argv[2],argv[3],true,true); return true; }
    if (!std::strcmp(mode, "hand-charges-left") && argc==4) { HandContainerCharges(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "hand-charges-right") && argc==4) { HandContainerCharges(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "ai-reload-log-wire")) { AIReloadLogRecord(0); return true; }
    if (!std::strcmp(mode, "ai-reload-log-rollback")) { AIReloadLogRecord(1); return true; }
    if (!std::strcmp(mode, "ai-reload-log-ownership")) { AIReloadLogRecord(2); return true; }
    if (!std::strcmp(mode, "inventory-clue-creation") && argc == 4) { InventoryClueCreation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "inventory-clue-count") && argc == 4) { InventoryClueCreation(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "inventory-clue-capacity") && argc == 4) { InventoryClueCreation(argv[2], argv[3], true, true); return true; }
    if (!std::strcmp(mode, "exhausted-hint-pickups") && argc == 4) { ExhaustedHintPickups(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "authored-hint-placement") && argc == 4) { AuthoredHintPlacement(argv[2], argv[3]); return true; }
    return false;
}
}
