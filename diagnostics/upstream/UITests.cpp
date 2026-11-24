#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/rpgGlobal.h"
#include "../../Main/RPGItem.h"
#include "../../Main/iMission.h"
#include "../../Main/iGameStates.h"
#include "../../Main/iCommonUI.h"
#include "../../Main/PlayerTracker.h"
#include "../../DBFormat/DataRPG.h"
#include <cstring>

namespace UpstreamTests
{
namespace
{
class WaitMission : public NGame::CMissionBase
{
    OBJECT_NOCOPY_METHODS(WaitMission);
public:
    bool ready = false;
    int resets = 0;
    CPtr<NGame::IPlayerTracker> player;

    bool IsReady() const override { return ready; }
    bool IsSequence() const override { return false; }
    bool IsRealTime() const override { return true; }
    NGame::IPlayerTracker* GetActivePlayer() const override { return player; }
    NUI::CDesktopWindow* GetDesktop() const override { return nullptr; }
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    void ResetState() override { ++resets; }
};

class DragPlayerTracker : public NGame::CPlayerTracker
{
    OBJECT_NOCOPY_METHODS(DragPlayerTracker);
public:
    CPtr<NWorld::IPlayer> tracked;
    NWorld::IPlayer* GetPlayer() const override { return tracked; }
};

class DragSlot : public NUI::CSlot
{
    OBJECT_NOCOPY_METHODS(DragSlot);
public:
    using NUI::CSlot::GetDragItem;
    void Take(int, int) override {}
    void Place(int, int, const NWorld::SItem&) override {}
    bool CanPlace(int, int, const NWorld::SItem&, int*) override { return false; }
    void GetItemsList(vector<SItem>*) override {}
    CObjectBase* GetTarget() override { return nullptr; }
};

void InventoryDragDuringAction()
{
    CObj<WaitMission> mission = new WaitMission;
    CObj<DragPlayerTracker> tracker = new DragPlayerTracker;
    CObj<NRPG::CGlobalPlayer> global = new NRPG::CGlobalPlayer;
    CObj<NWorld::CPlayer> player = new NWorld::CPlayer(L"Drag fixture", nullptr, global, 0);
    tracker->tracked = player;
    mission->player = tracker;
    CObj<NDb::CRPGItem> record = new NDb::CRPGItem;
    CObj<NRPG::IInventoryItem> item = NRPG::CreateDummyItem(record);
    player->SetInHandItem(NWorld::SItem(nullptr, NWorld::SItem::HAND, item));
    CObj<DragSlot> slot = new DragSlot;
    CStructureNetworkContext refs;
    refs.externalReferences = true;
    refs.include = [](CObjectBase*) { return false; };
    refs.ids[mission.GetPtr()] = 1;
    refs.refs[1] = mission.GetPtr();
    CPtr<NGame::IMission> missionReference = mission.GetPtr();
    CMemoryStream fields;
    { CStructureSaver writer(fields, CStructureSaver::WRITE, &refs); writer.Add(2, &missionReference); }
    fields.Seek(0);
    { CStructureSaver reader(fields, CStructureSaver::READ, &refs); slot->NUI::CSlot::operator&(reader); }
    NWorld::SItem visible;
    mission->ready = true;
    Require(slot->GetDragItem(&visible) && visible.pItem == item,
        "the ready inventory lost its dragged item");
    visible = NWorld::SItem();
    mission->ready = false;
    Require(!slot->GetDragItem(&visible) && !visible.pItem,
        "the busy inventory still published a drag target");
    mission->ready = true;
    Require(slot->GetDragItem(&visible) && visible.pItem == item,
        "resuming the inventory lost the held item");
}

void InventoryWaitEndsWhenReady()
{
    CObj<WaitMission> mission = new WaitMission;
    CObj<NGame::IState> wait = new NGame::CStateWait;
    Require(wait->Initialize(mission), "the wait state could not be installed");
    wait->Step();
    Require(mission->resets == 0, "the waiting cursor dismissed itself while an action was active");
    mission->ready = true;
    wait->Step();
    Require(mission->resets == 1 && wait->GetType() == NGame::IState::TEMPORARY,
        "the waiting cursor did not leave its temporary state when the mission became ready");
}
}

bool RunUITests(const char* mode)
{
    if (!std::strcmp(mode, "inventory-drag-during-action")) { InventoryDragDuringAction(); return true; }
    if (!std::strcmp(mode, "inventory-wait-ends-when-ready")) { InventoryWaitEndsWhenReady(); return true; }
    return false;
}
}
