#include "TestSupport.h"
#include "NativeDisplayContext.h"
#include "../../Main/wMain.h"
#include "../../Main/rpgGlobal.h"
#include "../../Main/RPGItem.h"
#include "../../Main/iMission.h"
#include "../../Main/iGameStates.h"
#include "../../Main/iCommonUI.h"
#include "../../Main/PlayerTracker.h"
#include "../../Main/iMain.h"
#include "../../Main/iSideMenu.h"
#include "../../Main/iHeroMenu.h"
#include "../../Main/iOptionsMenu.h"
#include "../../Main/ScreenShot.h"
#include "../../Main/UIInterface.h"
#include "../../Main/UICommCtrls.h"
#include "../../Main/UIBaseCtrls.h"
#include "../../Main/GfxBuffers.h"
#include "../../DBFormat/DataInterface.h"
#include "../../DBFormat/DataRPG.h"
#include "../../Input/Bind.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace UpstreamTests
{
namespace
{
void TemplateControlTieOrder()
{
    CObj<NDb::CUIContainer> menu = new NDb::CUIContainer;
    menu->nWidth = 320;
    menu->nHeight = 200;
    vector<CObj<NDb::CUIControl>> authored;
    const int depths[] = {3, 7, 3, 7};
    for (int index = 0; index < 4; ++index) {
        CObj<NDb::CUIControl> control = new NDb::CUIControl;
        control->type = NDb::UI_WINDOW;
        control->szID = "choice" + std::to_string(index);
        control->rect = CTRect<int>(0, 0, 100, 30);
        control->nDepth = depths[index];
        control->bVisible = true;
        control->bDefault = control->bTopmost = control->bBottommost = control->bTransparent = false;
        control->toolTipAnchorType = NDb::UIA_NONE;
        control->sToolTipAnchor = CTPoint<int>(0, 0);
        menu->controls.push_back(control.GetPtr());
        authored.push_back(control);
    }
    CObj<NUI::CWindow> window = new NUI::CWindow(NUI::SWindowInfo(nullptr,
        NUI::SPoint(0, 0), NUI::SPoint(320, 200), "root", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE));
    NUI::LoadTemplate(window, menu);
    list<CPtr<NUI::CWindow>> children;
    window->GetChildrenList(&children);
    Require(children.size() == 4, "native template did not create all overlapping controls");
    const int expected[] = {3, 1, 2, 0};
    size_t index = 0;
    for (const auto& child : children) {
        const auto& id = authored[expected[index++]]->szID;
        Require(child->GetWindowID() == id, "overlapping native controls did not take retail depth and equal-depth order");
    }
}

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

static void SideMenuSelectedStyle(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR"),"side selected-style fixture requires isolated user data");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm side selection regression");
    const auto normal=NUI::GetDBString(11129),hover=NUI::GetDBString(11130),selected=NUI::GetDBString(17338);
    Require(!normal.empty() && !hover.empty() && !selected.empty() && normal!=selected,"side style fixture lacks distinct authored state styles");
    for(const auto& entry:{std::pair<int,wstring>{11129,normal},{11130,hover},{17338,selected}}) {
        string ascii;for(auto ch:entry.second)ascii+=ch>=32 && ch<127?char(ch):'?';
        std::printf("native side style control: id=%d markup=%s\n",entry.first,ascii.c_str());
    }
    const auto fontPrefix=normal.substr(0,normal.find(L" outlinesize="));
    Require(fontPrefix==hover.substr(0,hover.find(L" outlinesize=")) && fontPrefix==selected.substr(0,selected.find(L" outlinesize=")),
        "side style fixture did not retain the same authored font face and size in all states");
    CObj<NGame::CICSideMenu> open=new NGame::CICSideMenu;open->Exec();
    auto* side=NMainLoop::GetCurrentInterfaceForHarness();
    Require(side && NMainLoop::StepApp(true,false,false),"side style fixture failed to initialize its actual native menu/frame");
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(side,&refs);CPtr<NUI::CWindow> page;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&page);}
    Require(page,"side style fixture lost its native page");
    struct Button {int tag;const char* id;int caption;};
    for(const auto entry:{Button{9,"easy",17329},Button{11,"normal",17330},Button{10,"hard",17331},Button{7,"axis",11131},Button{8,"allies",11132}}) {
        CStructureNetworkContext pageRefs;auto pageFields=BorrowedObjectFields(page,&pageRefs);CPtr<NUI::CButton> button;
        {CStructureSaver reader(pageFields,CStructureSaver::READ,&pageRefs);reader.Add(entry.tag,&button);}
        Require(button && button->GetWindowID()==entry.id,"side style fixture did not resolve the actual bottom-panel script button");
        auto text=[&](int state) {
            auto* window=button->GetState(state);auto* label=window?dynamic_cast<NUI::CText*>(window->GetChildByID("text")):nullptr;
            Require(label,"side style fixture lost an actual dynamic state label");return label->GetText();
        };
        const auto caption=NUI::GetDBString(entry.caption);
        Require(text(0)==normal+caption && text(1)==hover+caption,"side selected-style fixture changed the normal or hover caption");
        Require(page->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY,string(entry.id))) && NMainLoop::StepApp(true,false,false),
            "side style fixture failed to select its actual button and draw a native frame");
        Require(button->GetActiveState()==3,"side style fixture did not reach the actual selected button state");
        std::printf("native side selection: button=%s selectedState=%d selectedMarkupMatches=%d disabledEmpty=%d\n",entry.id,button->GetActiveState(),
            int(text(3)==selected+L"[ "+caption+L" ]"),int(text(2).empty()));std::fflush(stdout);
        Require(text(3)==selected+L"[ "+caption+L" ]","selected side-menu caption uses the normal color style instead of the authored selected style");
        Require(text(2).empty(),"side-menu script button retained a caption for its empty disabled state");
    }
    Require(NMainLoop::GetCurrentInterfaceForHarness()==side && !NMainLoop::HaveInterfaceCommand(),"selecting side-menu captions changed the native interface");
    NMainLoop::DoneInterface();
}

static void SideMenuTooltips(const char* databasePath,const char* resourcePath,int mode)
{
    struct Button {int tag;const char* id;int caption,tooltip;};
    const Button cases[]={{7,"axis",11131,19254},{8,"allies",11132,19253},{6,"next",16820,19249},
        {9,"easy",17329,19246},{11,"normal",17330,19247},{10,"hard",17331,19248},{5,"cancel",11174,19250}};
    Require(mode>=0 && mode<7,"unknown side-menu tooltip mode");
    Require(std::getenv("S2_USER_DATA_DIR"),"side-menu tooltip fixture requires isolated user data");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm side menu tooltip regression");
    const auto entry=cases[mode];const auto expected=NUI::GetDBString(entry.tooltip);
    Require(!expected.empty(),"native side-menu tooltip fixture has no authored localized text");
    CObj<NGame::CICSideMenu> open=new NGame::CICSideMenu;open->Exec();
    auto* side=NMainLoop::GetCurrentInterfaceForHarness();
    Require(side && NMainLoop::StepApp(true,false,false),"side-menu tooltip fixture did not initialize its actual menu and frame");
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(side,&refs);CPtr<NUI::CWindow> page;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&page);}
    Require(page,"side-menu tooltip fixture has no native page");
    CStructureNetworkContext pageRefs;auto pageFields=BorrowedObjectFields(page,&pageRefs);CPtr<NUI::CButton> button;
    {CStructureSaver reader(pageFields,CStructureSaver::READ,&pageRefs);reader.Add(entry.tag,&button);}
    Require(button && button->GetWindowID()==entry.id,"side-menu tooltip fixture resolved a different button");
    for(int state:{0,1}) {
        auto* window=button->GetState(state);auto* text=window?dynamic_cast<NUI::CText*>(window->GetChildByID("text")):nullptr;
        Require(text && text->GetText()==NUI::GetDBString(state==0?11129:11130)+NUI::GetDBString(entry.caption),
            "tooltip attachment changed the actual normal/hover caption or its font markup");
    }
    auto* tooltip=button->GetToolTip();
    std::printf("native side tooltip button=%s expectedTextID=%d attached=%d chars=%zu\n",entry.id,entry.tooltip,int(tooltip!=nullptr),expected.size());std::fflush(stdout);
    Require(tooltip,"native side-menu button has no tooltip attached");
    CStructureNetworkContext tooltipRefs;auto tooltipFields=BorrowedObjectFields(tooltip,&tooltipRefs);CPtr<NUI::CText> text;
    {CStructureSaver reader(tooltipFields,CStructureSaver::READ,&tooltipRefs);reader.Add(2,&text);}
    Require(text && text->GetText()==expected,"native side-menu tooltip contains a different localized description");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==side && !NMainLoop::HaveInterfaceCommand(),
        "reading side-menu tooltips changed selection or queued a menu transition");
    NMainLoop::DoneInterface();
}

static void SideMenuDifficultyMapping(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR"),"side difficulty fixture requires isolated user data");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm side difficulty regression");
    for(int id:{1,2,3}) {
        auto* record=NDb::GetDBDifficulty(id);
        Require(record && record->GetRecordID()==id,"side difficulty fixture lacks its authored difficulty record");
        std::printf("native side difficulty control: id=%d name=%s AP=%.3f VP=%.3f enemyDamage=%.3f ourDamage=%.3f\n",
            id,record->szUserName.c_str(),record->fAPCoeff,record->fVPCoeff,record->fEnemyDamageMult,record->fOurDamageMult);
    }
    struct Choice { const char* button;int expected; };
    for(const char* faction:{"axis","allies"}) for(const auto choice:{Choice{"normal",2},Choice{"hard",3},Choice{"easy",1},Choice{nullptr,1}}) {
        CObj<NGame::CICSideMenu> open=new NGame::CICSideMenu;open->Exec();
        auto* side=NMainLoop::GetCurrentInterfaceForHarness();
        Require(side && NMainLoop::StepApp(true,false,false) && NMainLoop::GetCurrentInterfaceForHarness()==side && !NMainLoop::HaveInterfaceCommand(),
            "side difficulty fixture failed to open the actual native menu/frame");
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(side,&refs);CPtr<NUI::CWindow> page;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&page);}
        Require(page && page->GetWindowID()=="mainmenuUI","side difficulty fixture lost its actual authored menu page");
        auto next=[&]() {
            NInput::SEvent event;while(NInput::GetEvent(&event)) {}
            NInput::PostEvent("next");
            Require(NInput::GetEvent(&event) && side->ProcessEvent(event),"side difficulty fixture did not dispatch the native next binding");
        };
        next();Require(!NMainLoop::HaveInterfaceCommand(),"side menu advanced without a faction selection");
        Require(page->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY,string(faction))),"native side menu rejected its faction notification");
        if(choice.button) Require(page->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY,string(choice.button))),"native side menu rejected its difficulty notification");
        next();
        CObj<NMainLoop::CInterfaceCommand> transition=NMainLoop::TakeInterfaceCommandForHarness();
        Require(dynamic_cast<NGame::CICHeroMenu*>(transition.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),
            "native side menu did not queue exactly its real hero-menu command");
        transition->Exec();
        auto* hero=NMainLoop::GetCurrentInterfaceForHarness();
        Require(hero && hero!=side && NMainLoop::StepApp(true,false,false) && NMainLoop::GetCurrentInterfaceForHarness()==hero,
            "side difficulty fixture failed to initialize the real hero menu/frame");
        CStructureNetworkContext heroRefs;auto heroFields=BorrowedObjectFields(hero,&heroRefs);
        CDBPtr<NDb::CSide> selectedSide;CDBPtr<NDb::CDBDifficulty> selectedDifficulty;
        {CStructureSaver reader(heroFields,CStructureSaver::READ,&heroRefs);reader.Add(2,&selectedSide);reader.Add(3,&selectedDifficulty);}
        Require(selectedSide==NDb::GetDBSide(!std::strcmp(faction,"axis")?1:2),"difficulty choice changed the selected faction");
        std::printf("native side difficulty: faction=%s button=%s actual=%d expected=%d\n",faction,choice.button?choice.button:"default",
            IsValid(selectedDifficulty)?selectedDifficulty->GetRecordID():-1,choice.expected);std::fflush(stdout);
        Require(selectedDifficulty==NDb::GetDBDifficulty(choice.expected),"side menu passed the enum ordinal instead of the original difficulty record to the real hero menu");
        CObj<NMainLoop::CICExitModal> back=new NMainLoop::CICExitModal;back->Exec();
        Require(NMainLoop::GetCurrentInterfaceForHarness()==side,"hero-menu return did not restore the same native side menu");
        NMainLoop::DoneInterface();Require(!NMainLoop::HaveInterfaceCommand(),"side difficulty fixture left a queued interface transition");
    }
}

static void OptionsPenCursor(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR"),"options cursor fixture requires isolated user data");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm options cursor regression");
    auto* pen=NDb::GetUICursor(1);auto* arrow=NDb::GetUICursor(2);
    Require(pen && arrow && pen!=arrow && IsValid(pen->pUITexture) && IsValid(arrow->pUITexture) && pen->pUITexture!=arrow->pUITexture,
        "options cursor fixture lacks distinct authored pen and arrow textures");
    CObj<NGScene::CScreenshotTexture> texture=new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels;pixels.SetSizes(1,1);pixels[0][0]=NGfx::SPixel8888(11,22,33,255);texture->Set(pixels);
    for(auto screen:{NGame::OS_VIDEO,NGame::OS_AUDIO,NGame::OS_GAMEPLAY,NGame::OS_CONTROLS,NGame::OS_PROFILE}) {
        CObj<NGame::CICOptions> open=new NGame::CICOptions(screen,texture);open->Exec();
        auto* active=NMainLoop::GetCurrentInterfaceForHarness();
        Require(active && NMainLoop::StepApp(true,false,false) && NMainLoop::GetCurrentInterfaceForHarness()==active,
            "options cursor fixture did not initialize its actual native page/frame");
        CStructureNetworkContext refs;auto saved=BorrowedObjectFields(active,&refs);
        CPtr<NUI::CWindow> page;CPtr<NUI::CInterface> ui;
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(6,&page);reader.Add(4,&ui);}
        Require(page && ui && page->GetInterface()==ui.GetPtr(),"options cursor fixture lost its actual options page or interface");
        Require(ui->GetDefaultCursorInfo().pCursor==arrow,"options cursor control changed the normal menu arrow");
        // Supply only the mouse-message boundary; execute the real authored page's
        // handler and observe what it submits to the native interface.
        ui->SetCursorInfo(NUI::SCursorInfo(arrow,L"previous child caption"));
        page->ProcessMessage(NUI::SEvent(NUI::EVENT_MOUSEMOVE,-100,-100));
        CStructureNetworkContext cursorRefs;auto cursorFields=BorrowedObjectFields(ui,&cursorRefs);NUI::SCursorInfo pending;
        {CStructureSaver reader(cursorFields,CStructureSaver::READ,&cursorRefs);reader.Add(6,&pending);}
        std::printf("native options cursor: page=%d record=%d default=%d\n",int(screen),pending.pCursor ? pending.pCursor->GetRecordID() : -1,
            ui->GetDefaultCursorInfo().pCursor->GetRecordID());std::fflush(stdout);
        Require(pending.pCursor==pen && pending.wsText.empty(),"options page submitted the normal arrow instead of its authored pen cursor");
        Require(ui->GetDefaultCursorInfo().pCursor==arrow,"options pen selection changed the normal cursor outside options");
        NMainLoop::DoneInterface();
        Require(!NMainLoop::HaveInterfaceCommand(),"options cursor fixture left a queued interface transition");
    }
}
}

bool RunUITests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "template-control-tie-order") && argc == 2) { TemplateControlTieOrder(); return true; }
    if (!std::strcmp(mode, "inventory-drag-during-action")) { InventoryDragDuringAction(); return true; }
    if (!std::strcmp(mode, "inventory-wait-ends-when-ready")) { InventoryWaitEndsWhenReady(); return true; }
    if (!std::strcmp(mode, "side-menu-tooltips")) { Require(argc==5,"side-menu tooltips require db/resources/mode"); SideMenuTooltips(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "side-menu-selected-style") && argc==4) { SideMenuSelectedStyle(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "side-menu-difficulty-mapping") && argc==4) { SideMenuDifficultyMapping(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "options-pen-cursor") && argc==4) { OptionsPenCursor(argv[2],argv[3]); return true; }
    return false;
}
}
