class WeaponIconMission: public NGame::CMissionBase
{
    OBJECT_BASIC_METHODS(WeaponIconMission);
public:
    vector<CPtr<NGame::IUnitTracker>> selected;
    CObj<NGame::CStateWait> state = new NGame::CStateWait;
    NGame::EActionIconsSet icons = NGame::AIS_MAIN;
    bool attackAvailable = true;
    bool mixedActionFixture = false;
    bool togglesAvailable = true;
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    void GetSelectedUnits(vector<CPtr<NGame::IUnitTracker>>* units) const override { *units = selected; }
    NGame::IState* GetState() const override { return state; }
    int GetUnitsState() override { return mixedActionFixture ? NGame::N_UNITSTATE_DEFAULT : NGame::N_UNITSTATE_CANNON; }
    NWorld::CUnit::EState GetUnitsWorldState() override { return mixedActionFixture ? NWorld::CUnit::ST_NORMAL_MELEE : NWorld::CUnit::ST_MACHINE_GUN; }
    NGame::EActionIconsSet GetActionIconsSet() const override { return icons; }
    void SetActionIconsSet(NGame::EActionIconsSet value) override { icons = value; }
    void GetActionInfo(NGame::EUnitAction action, NGame::SActionInfo* info) override {
        *info = NGame::SActionInfo();
        info->bValid = info->bOk = true;
        info->bAvailable = mixedActionFixture
            ? togglesAvailable && (action == NGame::UA_HIDE || action == NGame::UA_STRAFE)
            : action != NGame::UA_ATTACK || attackAvailable;
        info->eResult = NWorld::UCR_OK;
    }
};

class IconBarProbe: public NUI::CUnitIconsBar
{
    vector<CMObj<NUI::CWindow>> detachedControls;
public:
    using CUnitIconsBar::CUnitIconsBar;
    void DetachRendering() {
        // Transfer the main owning references; RemoveChild would invalidate
        // a control even if the fixture retained an ordinary CObj reference.
        detachedControls.swap(listChildren);
        for (auto& control : detachedControls) control->SetParent(nullptr);
    }
};

class IconBarFixture
{
public:
    CObj<IconBarProbe> bar;
    CObj<NDb::CUIContainer> layout;
    vector<CObj<NDb::CUIControl>> controls;
    vector<CObj<NUI::CComplexButton>> buttons;
    explicit IconBarFixture(NGame::IMission* mission) {
        bar = new IconBarProbe(
            NUI::SWindowInfo(nullptr, NUI::SPoint(0, 0), NUI::SPoint(400, 80), "mounted-mode-test"), mission);
        layout = new NDb::CUIContainer();
        layout->nWidth = 400; layout->nHeight = 80;
        for (int i = 0; i < 8; ++i) {
            CObj<NDb::CUIControl> control = new NDb::CUIControl();
            control->szID = "icon_" + std::to_string(i + 1);
            control->type = NDb::UI_WINDOW;
            control->nDepth = i; control->bVisible = true;
            control->rect = CTRect<int>(i % 4 * 100, i / 4 * 40, i % 4 * 100 + 100, i / 4 * 40 + 40);
            controls.push_back(control); layout->controls.push_back(control.GetPtr());
        }
        NUI::LoadTemplate(bar, layout);
        for (int i = 0; i < 8; ++i) {
            auto* button = dynamic_cast<NUI::CComplexButton*>(bar->GetChildByID("icon_" + std::to_string(i + 1)));
            Require(button != nullptr, "fixture icon template did not build all action buttons");
            buttons.emplace_back(button);
        }
        bar->DetachRendering();
        CMemoryStream state;
        int worldState = mission->GetUnitsWorldState();
        { CStructureSaver writer(state, CStructureSaver::WRITE); writer.Add(6, &worldState); }
        state.Seek(0);
        { CStructureSaver reader(state, CStructureSaver::READ);
          static_cast<CObjectBase*>(bar.GetPtr())->operator&(reader); }
    }
};

static void MountedModeButton(WeaponIconMission* mission)
{
    IconBarFixture fixture(mission);
    auto& bar = fixture.bar;
    auto& buttons = fixture.buttons;
    bar->Draw(0, nullptr);
    Require(buttons[4]->GetStyle(NUI::STYLE_VISIBLE) && buttons[4]->GetStyle(NUI::STYLE_ENABLED),
        "mounted-gun action bar omitted the fire-mode submenu button");
    Require(buttons[0]->GetStyle(NUI::STYLE_VISIBLE) && buttons[7]->GetStyle(NUI::STYLE_VISIBLE),
        "mounted-gun mode button replaced attack or stop");
    mission->attackAvailable = false;
    bar->Draw(0, nullptr);
    Require(!buttons[4]->GetStyle(NUI::STYLE_VISIBLE), "unavailable mounted attack still exposed fire modes");
    mission->attackAvailable = true;
    bar->Draw(0, nullptr);
    Require(buttons[4]->GetStyle(NUI::STYLE_VISIBLE), "mounted fire-mode button did not return with attack eligibility");
    mission->icons = NGame::AIS_WEAPONMODES;
    bar->Draw(0, nullptr);
    Require(buttons[0]->GetStyle(NUI::STYLE_VISIBLE) && !buttons[0]->IsChecked() &&
            buttons[1]->GetStyle(NUI::STYLE_VISIBLE) && buttons[1]->IsChecked(),
        "actual mounted-gun mode controls did not check the mounted aimed mode");
    mission->icons = NGame::AIS_MAIN;
}


static void MixedActionButton(const char* databasePath, const char* resourcePath, bool hide)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> actors;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) actors.push_back(unit);
        if (actors.size() == 2) break;
    }
    Require(actors.size() == 2, "mixed-button fixture needs two ordinary actors");
    CObj<WeaponIconMission> mission = new WeaponIconMission;
    mission->mixedActionFixture = true;
    vector<CObj<NGame::CUnitTracker>> trackers;
    for (auto* actor : actors) {
        trackers.emplace_back(new NGame::CUnitTracker(mission, actor));
        mission->selected.push_back(trackers.back().GetPtr());
    }
    IconBarFixture fixture(mission);
    auto& button = fixture.buttons[hide ? 7 : 6];
    auto setGroup = [&](bool first, bool second) {
        for (int i = 0; i < 2; ++i)
            if (hide) actors[i]->GetUnitRPG()->SetHiding(i == 0 ? first : second);
            else actors[i]->SetStrafe(i == 0 ? first : second);
        fixture.bar->Draw(0, nullptr);
    };
    setGroup(true, false);
    Require(button->GetStyle(NUI::STYLE_VISIBLE) && button->GetStyle(NUI::STYLE_ENABLED) && !button->IsChecked(),
        hide ? "Hide button disappeared for a selection with mixed hiding states"
             : "Strafe button disappeared for a selection with mixed strafe states");
    setGroup(false, true);
    Require(button->GetStyle(NUI::STYLE_VISIBLE) && !button->IsChecked(), "mixed toggle depends on selection order");
    setGroup(true, true);
    Require(button->GetStyle(NUI::STYLE_VISIBLE) && button->IsChecked(), "all-active selection did not check its toggle");
    setGroup(false, false);
    Require(button->GetStyle(NUI::STYLE_VISIBLE) && !button->IsChecked(), "all-inactive selection did not uncheck its toggle");
    mission->togglesAvailable = false;
    fixture.bar->Draw(0, nullptr);
    Require(!button->GetStyle(NUI::STYLE_VISIBLE), "unavailable group toggle remained visible");
    mission->togglesAvailable = true;
    setGroup(true, false);
    Require(button->GetStyle(NUI::STYLE_VISIBLE), "mixed toggle did not return after becoming available");
}

static NWorld::CCommandExecute* FindNativeReload(NWorld::CCommandExecute* executor)
{
    if (pSSClasses->GetObjectTypeID(executor) == int(0x00122170)) return executor;
    CPtr<NWorld::CCommandExecute> reload;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* object) {
        if (pSSClasses->GetObjectTypeID(object) == int(0x00122170)) {
            reload = dynamic_cast<NWorld::CCommandExecute*>(object); return false;
        }
        return dynamic_cast<NWorld::CCommandExecute*>(object) != nullptr;
    };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(executor)->operator&(writer); }
    Require(IsValid(reload), "native executor queue did not contain its reload action");
    return reload;
}

struct NativeCannonFixture
{
    CObj<NRPG::CGlobalGame> game;
    CObj<NWorld::CWorld> world;
    CPtr<NWorld::CUnitServer> actor;
    CPtr<NWorld::CCannon> cannon;
    CDBPtr<NDb::CAnimation> entryAnimation;
    vector<int> entryClipIDs;
    int personalFlags=0;
};

static NativeCannonFixture MakeNativeCannonFixture(const char* databasePath,const char* resourcePath)
{
    NativeCannonFixture fixture;fixture.world=FixtureWorld(databasePath,resourcePath,true,&fixture.game);
    auto* world=fixture.world.GetPtr();auto* network=world->GetPathNetwork();world->GetTime()->Set(1000);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
    vector<NWorld::CCannon*> cannons;world->GetCannons(&cannons);
    for(auto entry:units) {
        auto* actor=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(!actor || !actor->CanFight() || actor->IsWearingPK() || actor->IsEmptyPK() || actor->GetUnitRPG()->GetCannonItem())continue;
        auto* personal=dynamic_cast<NRPG::CWeaponItem*>(actor->GetUnitRPG()->GetInventory()->GetActive());
        if(!personal)continue;
        const auto original=actor->GetPosition();
        for(auto* cannon:cannons) {
            if(!cannon || cannon->IsBroken() || cannon->IsOccupied() || !cannon->GetSkeletonAnimator())continue;
            auto* weapon=dynamic_cast<NRPG::CWeaponItem*>(cannon->GetItem());
            if(!weapon || weapon->GetDBWeapon()->szAnimName.empty())continue;
            vector<NAI::SPathPlace> places;cannon->GetApproaches(&places,network);
            for(auto place:places) {
                place.SetPose(NAI::CM_STAND);
                if(!network->IsPassable(place) || network->IsLocked(place))continue;
                auto position=original;position.pos.p=place;position.bRun=false;actor->SetPosition(position);
                actor->animator.AlignTime();actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
                actor->animator.SetActiveItem(true);actor->animator.SetWeaponName(personal->GetDBWeapon()->szAnimName.c_str());
                actor->animator.PlaceUnit(position);
                CDBPtr<NDb::CSkeleton> skeleton;CDBPtr<NDb::CSide> side;int flags=0,classFlags=0;
                CMemoryStream fields;CStructureNetworkContext refs;refs.include=[](CObjectBase*){return false;};
                {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);actor->animator.operator&(writer);}
                fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(4,&flags);reader.Add(5,&classFlags);reader.Add(31,&skeleton);reader.Add(40,&side);}
                auto* expected=skeleton?skeleton->GetAnimation(NDb::CAnimation::START_ATTACK,NDb::CAnimation::POSE_STAND,
                    weapon->GetDBWeapon()->szAnimName.c_str(),classFlags,side):nullptr;
                if(!expected || flags==NDb::CAnimation::POSE_STAND)continue;
                CObj<NWorld::CCmdCannon> command=new NWorld::CCmdCannon(cannon);
                CObj<NWorld::CExecCannon> probe=new NWorld::CExecCannon(actor,cannon,true,command);
                if(probe->CanDoIt(position)!=NWorld::UCR_OK)continue;
                fixture.actor=actor;fixture.cannon=cannon;fixture.entryAnimation=expected;fixture.personalFlags=flags;
                for(const auto& clip:skeleton->pAnimations[NDb::CAnimation::START_ATTACK].anims)
                    if(clip->szParams==weapon->GetDBWeapon()->szAnimName)fixture.entryClipIDs.push_back(clip->GetRecordID());
                Require(std::find(fixture.entryClipIDs.begin(),fixture.entryClipIDs.end(),expected->GetRecordID())!=fixture.entryClipIDs.end(),
                    "native cannon positive clip is missing from its authored skeleton entry candidates");
                std::printf("native cannon fixture: persona=%d personalWeapon=%d cannonWeapon=%d name=%s poseFlags=0x%X entryClip=%d\n",
                    actor->GetUnitRPG()->GetRPGPers()->GetRecordID(),personal->GetDBWeapon()->GetRecordID(),weapon->GetDBWeapon()->GetRecordID(),
                    weapon->GetDBWeapon()->szAnimName.c_str(),unsigned(flags),expected->GetRecordID());std::fflush(stdout);
                return fixture;
            }
        }
        actor->SetPosition(original);actor->animator.AlignTime();actor->animator.PlaceUnit(original);
    }
    Require(false,"cannon fixture has no actual armed actor, free authored approach and matching original stance-only entry clip");
    return fixture;
}

static void AICannonUndockPlan(const char* databasePath,const char* resourcePath,int mode)
{
    auto fixture=MakeNativeCannonFixture(databasePath,resourcePath);auto* actor=fixture.actor.GetPtr();auto* cannon=fixture.cannon.GetPtr();auto* world=fixture.world.GetPtr();
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    CObj<NWorld::CCmdCannon> mount=new NWorld::CCmdCannon(cannon);
    CObj<NWorld::CExecCannon> enter=new NWorld::CExecCannon(actor,cannon,true,mount);
    Require(enter->CanDoIt(actor->GetPosition())==NWorld::UCR_OK,"undock planning fixture cannot enter its actual authored cannon");
    enter->Run();world->GetTime()->Set(actor->animator.GetTimeEnd());enter->AnimationFinished();enter=nullptr;
    Require(!world->IsAction() && actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN && cannon->GetCurrentUnit()==actor && actor->animator.GetCannon()==cannon,
        "undock planning fixture did not finish native cannon entry before planning");
    CObj<NAI::IAIUnit> gunner=NAI::CreateAIUnit(actor,true);gunner->Synchronize(false);
    Require(gunner->GetCannon()==cannon && gunner->GetUnitMission()==actor->GetUnitRPG(),"undock planning wrapper lost its live cannon or actual RPG mission");
    const int ap=actor->GetAP(),shadowAP=gunner->GetAP(),vp=actor->GetUnitRPG()->GetTotalVP(),ammo=cannon->GetItem()->GetAmmoQuantity();
    const auto position=actor->GetPosition();const STime animationEnd=actor->animator.GetTimeEnd();
    Require(ap>7 && world->IsTurnBased() && !world->IsRealTime() && !actor->HasCommand(),"undock fixture has no idle affordable native turn-based planning state");
    CObj<NAI::IAILogic> logic=NAI::CreateAIAttackLogic(gunner);logic->StopThinking();const bool needBefore=logic->IsNeedToThink();
    Require(!needBefore,"native combat logic did not snapshot unchanged server AP before undock planning");
    CObj<NAI::CAILog> log=NAI::CreateAILog();CObj<NAI::CAIUndockFromHGAction> action=new NAI::CAIUndockFromHGAction(gunner);
    CObj<NWorld::CUnitServer> tombstone;
    struct Restore{NAI::IAIUnit* unit;NWorld::CUnitServer* actor;~Restore(){SetToHitProbeServer(unit,actor);}}restore{gunner,actor};
    if(mode==2)gunner->SetCannon(nullptr);
    if(mode==3)SetToHitProbeServer(gunner,nullptr);
    if(mode==4){tombstone=new NWorld::CUnitServer;SetToHitProbeServer(gunner,tombstone);tombstone->Invalidate();}
    GlobalGameRandom().SeedForHarness(0x98ca);auto random=GlobalGameRandom();
    action->Do(mode==1?nullptr:log.GetPtr());const int after=actor->GetAP();const bool empty=log->IsEmpty();
    list<CPtr<NWorld::CCommand>> commands;log->GetWorldCommands(&commands);
    SetToHitProbeServer(gunner,actor);const bool needAfter=logic->IsNeedToThink();
    Require(actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN && cannon->GetCurrentUnit()==actor && actor->animator.GetCannon()==cannon
        && actor->animator.GetTimeEnd()==animationEnd && actor->GetPosition()==position && actor->GetUnitRPG()->GetTotalVP()==vp
        && cannon->GetItem()->GetAmmoQuantity()==ammo && gunner->GetAP()==shadowAP && random.Get()==GlobalGameRandom().Get(),
        "planning cannon exit changed physical mount, animation, location, health, ammunition, cached AP or RNG");
    NWorld::EUnitCommandResult result=NWorld::UCR_OK;CObj<NWorld::CCommandExecute> leave;
    if(mode==0) {
        auto* order=commands.size()==1?dynamic_cast<NWorld::CCmdSetCommand*>(commands.front().GetPtr()):nullptr;
        auto* exit=order?dynamic_cast<NWorld::CCmdExitCannon*>(order->GetCmd()):nullptr;
        Require(exit && order->pUnit==actor && exit->pCannon.GetPtr()==static_cast<NWorld::IObject*>(cannon),
            "undock planning lost the exact actor/cannon in its actual exit-log command");
        leave=NWorld::CreateExecutor(actor,exit,&result);Require(leave && result==NWorld::UCR_OK,"native mounted executor rejected the planned exit command");
    }else leave=new NWorld::CExecCannon(actor,cannon,false);
    leave->Run();world->GetTime()->Set(actor->animator.GetTimeEnd());leave->AnimationFinished();leave=nullptr;
    Require(!world->IsAction() && !cannon->IsOccupied() && !actor->GetUnitRPG()->GetCannonItem() && actor->GetState()!=NWorld::CUnit::ST_MACHINE_GUN
        && actor->GetAP()==after && actor->GetUnitRPG()->GetTotalVP()==vp && actor->GetPosition()==position,
        "normal native cannon exit added another AP cost or lost actual detachment/animation cleanup");
    const int expected=ap-(mode==0?1:0);
    std::printf("native cannon undock planning: mode=%d AP=%d after=%d expected=%d shadowAP=%d needBefore=%d needAfter=%d commands=%zu empty=%d exitAP=%d\n",
        mode,ap,after,expected,shadowAP,int(needBefore),int(needAfter),commands.size(),int(empty),actor->GetAP());std::fflush(stdout);
    Require(after==expected && needAfter==(mode==0),"native cannon undock planning omitted original live mission AP spend and failed to re-arm the next combat decision");
    Require(commands.size()==size_t(mode==0) && empty==(mode!=0),"cannon undock planning queued an exit for a missing or ref-invalid owner server");
}

static void AIMountedRoute(const char* databasePath,const char* resourcePath,int mode)
{
    auto fixture=MakeNativeCannonFixture(databasePath,resourcePath);auto* actor=fixture.actor.GetPtr();auto* cannon=fixture.cannon.GetPtr();auto* world=fixture.world.GetPtr();
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    CObj<NWorld::CCmdCannon> mount=new NWorld::CCmdCannon(cannon);CObj<NWorld::CExecCannon> enter=new NWorld::CExecCannon(actor,cannon,true,mount);
    Require(enter->CanDoIt(actor->GetPosition())==NWorld::UCR_OK,"mounted-route fixture cannot enter its actual authored gun");
    enter->Run();world->GetTime()->Set(actor->animator.GetTimeEnd());enter->AnimationFinished();enter=nullptr;
    auto leaveNormally=[&]() {
        CObj<NWorld::CExecCannon> leave=new NWorld::CExecCannon(actor,cannon,false);leave->Run();world->GetTime()->Set(actor->animator.GetTimeEnd());leave->AnimationFinished();leave=nullptr;
        Require(!world->IsAction() && !cannon->IsOccupied() && !actor->GetUnitRPG()->GetCannonItem(),"mounted-route fixture did not finish normal native cannon exit");
    };
    Require(!world->IsAction() && actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN && cannon->GetCurrentUnit()==actor,"mounted-route fixture did not finish its real entry");
    if(mode==5)leaveNormally();
    CObj<NAI::IAIUnit> unit=NAI::CreateAIUnit(actor,true);unit->Synchronize(false);
    const auto initial=actor->GetPosition();auto destination=initial;destination.pos.p.SetDirection((initial.pos.p.GetDirection()+2)&7);
    CObj<NAI::CTaskCommand> step;CObj<NWorld::CCmd> queued;
    if(mode==1) {
        vector<NAI::SPathPlace> places;NAI::GetNearestPlaces(actor,initial.pos.p,4,NAI::RUN,&places);bool found=false;
        for(auto point:places) {
            if(point.GetPose()!=NAI::CM_STAND || (point.GetX()==initial.pos.p.GetX() && point.GetY()==initial.pos.p.GetY() && point.GetLayer()==initial.pos.p.GetLayer()))continue;
            CObj<NAI::CPath> path=NWorld::FindPath(world->GetPathNetwork(),actor,initial.pos.p,vector<NAI::SPathPlace>{point},actor);
            if(path && path->points.size()>1){destination=initial;destination.pos.p=point;found=true;break;}
        }
        Require(found,"mounted-route fixture has no real reachable nearby movement destination");step=new NAI::CTaskCommandGoto(destination.pos);
    }else if(mode==2)step=new NAI::CTaskCommandChangeWishPose(NAI::WALK);
    else if(mode==7)step=new NAI::CTaskCommandWait(3);
    else step=new NAI::CTaskCommandChangeDirection(destination.pos.p.GetDirection());
    // Pending native command-list inputs also occur when a route is restored
    // after the unit mounted a gun. They use the existing public queue.
    if(mode==3)queued=new NWorld::CCmdStrafe(true);
    if(mode==4)queued=new NWorld::CCmdHide();
    if(mode==6)queued=new NWorld::CCmdShootMode(NDb::EShootMode(4));
    if(queued)step->Commands.push_back(queued.GetPtr());
    CObj<NAI::CAIRouteLogic> route=NAI::CreateAIRouteLogic(unit,vector<CPtr<NAI::CTaskCommand>>{step.GetPtr()},false);
    Require(route,"mounted-route native factory rejected its actual unit and task");
    if(mode==8) {
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(route,&refs);CObj<NAI::CAIRouteLogic> restored=new NAI::CAIRouteLogic;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);}
        route=restored;unit->SetCannon(nullptr);
        Require(actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN && !unit->GetCannon(),"restored-route control did not retain live mount independently of transient AI cannon cache");
    }
    const int ap=actor->GetAP(),vp=actor->GetUnitRPG()->GetTotalVP(),ammo=cannon->GetItem()->GetAmmoQuantity();
    const STime animationEnd=actor->animator.GetTimeEnd();auto random=GlobalGameRandom();
    CObj<NWorld::CCommand> first=route->GetCommand();
    if(mode==7) {
        std::printf("native mounted route wait: mode=7 first=%d queued=%d mounted=%d AP=%d\n",int(IsValid(first)),int(route->HasCommands()),int(actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN),actor->GetAP());std::fflush(stdout);
        Require(!first && !route->HasCommands() && !route->IsRouteComplete() && actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN && actor->GetAP()==ap,"native mounted wait unexpectedly queued an exit or completed its route");leaveNormally();return;
    }
    auto* order=dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());Require(order && order->pUnit==actor,"native route did not preserve its actual command owner");
    const bool needsExit=mode<5 || mode==8;auto* exit=dynamic_cast<NWorld::CCmdExitCannon*>(order->GetCmd());const int firstResult=int(actor->CanDo(order->GetCmd()));
    std::printf("native mounted route prefix: mode=%d mounted=%d firstExit=%d expectedExit=%d firstResult=%d queued=%d AP=%d cache=%d\n",mode,int(actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN),int(exit!=nullptr),int(needsExit),firstResult,int(route->HasCommands()),ap,int(IsValid(unit->GetCannon())));std::fflush(stdout);
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==vp && actor->GetPosition()==initial && actor->animator.GetTimeEnd()==animationEnd && cannon->GetItem()->GetAmmoQuantity()==ammo && random.Get()==GlobalGameRandom().Get(),"generating a native route altered physical state, AP, animation, health, ammunition or RNG");
    if(needsExit && !exit){leaveNormally();Require(false,"mounted native route dispatches a state-rejected command without first queuing normal cannon exit");}
    Require(bool(exit)==needsExit && firstResult==int(NWorld::UCR_OK),"native route added an unnecessary exit or produced an unavailable leading command");
    if(mode==6) {
        Require(order->GetCmd()==queued.GetPtr() && !route->HasCommands() && actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN,"mounted shoot-mode control was replaced by exit");leaveNormally();return;
    }
    CObj<NWorld::CCommand> original=first;int resumes=0;
    if(needsExit) {
        Require(route->HasCommands() && exit->pCannon.GetPtr()==static_cast<NWorld::IObject*>(cannon),"normal mounted state did not bind the route exit to the actual cannon or retain its payload");
        actor->Do(first);actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));
        Require(actor->IsPerformingAction(),"route exit did not start the actual living dismount animation");
        Require(!route->GetCommand() && route->HasCommands(),"native route consumed its original command during dismount animation");
        for(int edge=0;edge<8 && actor->IsPerformingAction();++edge){world->GetTime()->Set(actor->animator.GetTimeEnd());actor->Segment();}
        Require(!actor->IsPerformingAction() && actor->GetState()!=NWorld::CUnit::ST_MACHINE_GUN && !cannon->IsOccupied() && !actor->GetUnitRPG()->GetCannonItem() && actor->GetAP()==ap,"route dismount failed to finish native detachment or added an AP charge");
        original=route->GetCommand();
        while(resumes<4) {
            auto* resume=dynamic_cast<NWorld::CCmdSetCommand*>(original.GetPtr());if(!resume || !dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()))break;
            actor->Do(original);world->GetTime()->Set(actor->animator.GetTimeEnd());actor->Segment();++resumes;original=route->GetCommand();
        }
    }
    auto* payload=dynamic_cast<NWorld::CCmdSetCommand*>(original.GetPtr());Require(payload && payload->pUnit==actor && !dynamic_cast<NWorld::CCmdExitCannon*>(payload->GetCmd()),"native route lost or repeated its original command after normal dismount");
    if(mode==1 && dynamic_cast<NWorld::CCmdStrafe*>(payload->GetCmd())) {
        // Native Goto for a WALK actor emits Strafe then Path. Preserve and
        // execute that existing prelude, rather than changing the real pose.
        Require(actor->CanDo(payload->GetCmd())==NWorld::UCR_OK,"normal Goto strafe prelude remained blocked after dismount");
        actor->Do(original);actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));
        for(int edge=0;edge<8 && actor->IsPerformingAction();++edge){world->GetTime()->Set(actor->animator.GetTimeEnd());actor->Segment();}
        actor->Segment();original=route->GetCommand();
        while(resumes<4) {
            auto* resume=dynamic_cast<NWorld::CCmdSetCommand*>(original.GetPtr());if(!resume || !dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()))break;
            actor->Do(original);world->GetTime()->Set(actor->animator.GetTimeEnd());actor->Segment();++resumes;original=route->GetCommand();
        }
        payload=dynamic_cast<NWorld::CCmdSetCommand*>(original.GetPtr());
        Require(payload && payload->pUnit==actor && actor->GetAP()==ap && actor->GetState()!=NWorld::CUnit::ST_MACHINE_GUN,
            "native Goto prelude changed AP or lost the queued Path after dismount");
        std::printf("native mounted route Goto prelude: mode=1 AP=%d mounted=0\n",actor->GetAP());std::fflush(stdout);
    }
    const bool correct=mode==1?dynamic_cast<NWorld::CCmdPath*>(payload->GetCmd())!=nullptr:mode==2?dynamic_cast<NWorld::CCmdWishPose*>(payload->GetCmd())!=nullptr:queued?payload->GetCmd()==queued.GetPtr():dynamic_cast<NWorld::CCmdLook*>(payload->GetCmd())!=nullptr;
    Require(correct && !route->HasCommands(),"native route changed its original payload kind/reference or added commands");
    const int afterExit=actor->GetAP(),payloadResult=int(actor->CanDo(payload->GetCmd()));
    if(mode==0 || mode==1 || mode==5 || mode==8) {
        Require(payloadResult==int(NWorld::UCR_OK),"native dismount did not make its genuine movement/turn request executable");
        actor->Do(original);actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));
        for(int edge=0;edge<32 && actor->IsPerformingAction();++edge){world->GetTime()->Set(actor->animator.GetTimeEnd());actor->Segment();}
        Require(!actor->IsPerformingAction() && actor->GetAP()<=afterExit,"native route movement/turn did not finish normally");
        const auto actual=actor->GetPosition().pos.p;
        if(mode==1)Require(actual.GetX()==destination.pos.p.GetX() && actual.GetY()==destination.pos.p.GetY() && actual.GetLayer()==destination.pos.p.GetLayer(),"native route failed to reach its genuine nearby destination after dismount");
        else Require(actual.GetDirection()==destination.pos.p.GetDirection(),"native route failed to turn to its requested direction after dismount");
    }
    if(actor->HasCommand())actor->Do(new NWorld::CCmdCancel(actor));
    Require(!world->IsAction() && !actor->HasCommand() && !actor->IsPerformingAction() && actor->GetUnitRPG()->GetTotalVP()==vp && cannon->GetItem()->GetAmmoQuantity()==ammo,"native mounted route retained an action or changed health/ammunition");
    std::printf("native mounted route result: mode=%d afterExitAP=%d finalAP=%d resumes=%d payloadResult=%d mounted=%d\n",mode,afterExit,actor->GetAP(),resumes,payloadResult,int(actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN));std::fflush(stdout);
}

static vector<NAnimation::CAnimation*> NativeCannonAnimationClips(NAnimation::CSkeletonAnimator* animator)
{
    vector<NAnimation::CAnimation*> result;vector<CObjectBase*> pending{animator},visited;
    while(!pending.empty()) {
        auto* object=pending.back();pending.pop_back();
        if(std::find(visited.begin(),visited.end(),object)!=visited.end())continue;visited.push_back(object);
        if(auto* clip=dynamic_cast<NAnimation::CAnimation*>(object))result.push_back(clip);
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(object,&refs);
        for(const auto& ref:refs.refs)if(dynamic_cast<NAnimation::CAnimator*>(ref.second.GetPtr()))pending.push_back(ref.second.GetPtr());
    }
    return result;
}

static void MovementRotationQueueWire(const char* databasePath,const char* resourcePath,int mode)
{
    auto fixture=MakeNativeMovementFixture(databasePath,resourcePath);auto* actor=fixture.actor.GetPtr();auto* world=fixture.world.GetPtr();
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));world->GetTime()->Set(1000);actor->animator.AlignTime();
    auto samePlace=[](NAI::SPathPlace a,NAI::SPathPlace b){a.SetMoving(0);b.SetMoving(0);return a==b;};
    auto expected=actor->GetPosition().pos.p;expected.SetDirection((expected.GetDirection()+2)&7);expected.SetMoving(0);
    CObj<NAI::CPath> path=NWorld::FindPath(world->GetPathNetwork(),actor,actor->GetPosition().pos.p,vector<NAI::SPathPlace>{expected},nullptr,true,NAI::PF_USE_POSEDIR);
    Require(path && path->points.size()>=2 && samePlace(path->points.back(),expected),"rotation wire fixture could not find its native same-cell turn");
    CObj<NWorld::CCommandExecute> move=NWorld::CreateSimpleMoveExecutor(actor,path,NAI::PF_USE_POSEDIR);
    Require(move,"rotation wire fixture could not create its native mover");
    if(mode) {
        auto rotating=[](CObjectBase* command){return command && pSSClasses->GetObjectTypeID(command)==int(0x02731143);};
        move->Run();
        for(int edge=0;edge<16 && !rotating(ReadMovementCommands(move).current.GetPtr()) && move->GetState()==NWorld::CCommandExecute::RUNNING;++edge) {
            world->GetTime()->Set(actor->animator.GetTimeEnd());move->AnimationFinished();
        }
        Require(rotating(ReadMovementCommands(move).current),"rotation wire fixture did not reach its native active turn");
        expected=actor->GetPosition().pos.p;expected.SetMoving(0);
        if(mode==1)move->Cancel();
        else {
            expected.SetPose(NAI::CM_CROUCH);
            path=NWorld::FindPath(world->GetPathNetwork(),actor,actor->GetPosition().pos.p,vector<NAI::SPathPlace>{expected},nullptr,true,NAI::PF_USE_POSEDIR);
            Require(path && path->points.size()==2 && samePlace(path->points.back(),expected),"rotation wire fixture lacks its real crouch reroute");
            dynamic_cast<NWorld::IExecMove*>(move.GetPtr())->SetNewPath(path,NAI::PF_USE_POSEDIR);
        }
    }
    CMemoryStream file;
    {CStructureSaver writer(file,CStructureSaver::WRITE);writer.Add(1,&fixture.game);writer.Add(2,&fixture.world);writer.Add(3,&fixture.actor);writer.Add(4,&move);}
    file.Seek(0);NativeMovementFixture restored;CObj<NWorld::CCommandExecute> executor;
    {CStructureSaver reader(file,CStructureSaver::READ);reader.Add(1,&restored.game);reader.Add(2,&restored.world);reader.Add(3,&restored.actor);reader.Add(4,&executor);}
    Require(restored.game && restored.world && restored.actor && executor && executor->GetUnitServer()==restored.actor.GetPtr(),"rotation wire snapshot lost its native graph or unit identity");
    restored.world->RestoreRuntimeCaches(restored.game);auto commands=ReadMovementCommands(executor);unsigned rotations=0,invalidMoveEnders=0;
    for(auto& command:commands.queued) {
        const int type=pSSClasses->GetObjectTypeID(command);
        if(type==int(0x72312170)) {
            auto* unitCommand=dynamic_cast<NWorld::CCmdUnit*>(command.GetPtr());Require(unitCommand && unitCommand->pUnit==restored.actor.GetPtr(),"saved rotation ender lost its owning unit reference");++rotations;
        }
        if(type==int(0x12541120)) {
            CStructureNetworkContext refs;auto fields=BorrowedObjectFields(command,&refs);CObj<CObjectBase> ended;
            {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&ended);}
            if(!ended)++invalidMoveEnders;
        }
    }
    std::printf("native saved rotation queue: mode=%d EndRotate=%u nullEndMove=%u bytes=%u\n",mode,rotations,invalidMoveEnders,unsigned(file.GetSize()));std::fflush(stdout);
    Require(rotations==1 && invalidMoveEnders==0,"fresh saved rotation queue violates the original separate EndRotate/non-null EndMove wire contract");
    if(!mode)executor->Run();
    for(int edge=0;edge<32 && executor->GetState()==NWorld::CCommandExecute::RUNNING;++edge) {
        restored.world->GetTime()->Set(restored.actor->animator.GetTimeEnd());executor->AnimationFinished();
    }
    Require(executor->GetState()==(mode==1?NWorld::CCommandExecute::FAILED:NWorld::CCommandExecute::FINISHED) && samePlace(restored.actor->GetPosition().pos.p,expected),
        "restored native rotation queue changed its cancellation policy or failed its requested turn/pose");
    dynamic_cast<NWorld::IExecMove*>(executor.GetPtr())->FullCancel();executor=nullptr;
    dynamic_cast<NWorld::IExecMove*>(move.GetPtr())->FullCancel();move=nullptr;
    Require(!restored.world->IsAction() && !world->IsAction(),"rotation wire fixture retained its original/restored action");
}

static void MovementRerouteContinuation(const char* databasePath,const char* resourcePath,bool strafe)
{
    auto fixture=MakeNativeMovementFixture(databasePath,resourcePath);auto* actor=fixture.actor.GetPtr();auto* world=fixture.world.GetPtr();auto* network=world->GetPathNetwork();
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));world->GetTime()->Set(1000);
    auto facing=actor->GetPosition();facing.pos.p.SetDirection((facing.pos.p.GetDirection()+(strafe?4:2))&7);actor->SetPosition(facing);
    actor->animator.AlignTime();actor->animator.PlaceUnit(facing);actor->SetStrafe(strafe);
    fixture.path=NWorld::FindPath(network,actor,facing.pos.p,vector<NAI::SPathPlace>{fixture.path->points.back()},nullptr,true,NAI::PF_DEFAULT,strafe);
    Require(fixture.path && fixture.path->points.size()>=5 && fixture.path->bStrafePath==strafe,"continuation fixture lacks its actual native walking/strafe route");
    CObj<NWorld::CCommandExecute> move=NWorld::CreateSimpleMoveExecutor(actor,fixture.path,NAI::PF_DEFAULT);
    Require(move,"continuation fixture could not create its native mover");
    const int wanted=strafe?int(0x02731142):int(0x02731143);
    auto matches=[&](CObjectBase* command){return command && pSSClasses->GetObjectTypeID(command)==wanted;};
    move->Run();
    for(int edge=0;edge<32 && !matches(ReadMovementCommands(move).current.GetPtr()) && move->GetState()==NWorld::CCommandExecute::RUNNING;++edge) {
        world->GetTime()->Set(actor->animator.GetTimeEnd());move->AnimationFinished();
    }
    Require(matches(ReadMovementCommands(move).current),"continuation fixture did not reach its actual interrupted rotation/strafe command");
    auto* moving=dynamic_cast<NWorld::IExecMove*>(move.GetPtr());NAI::SPathPlace search;moving->GetSearchFromPosition(&search);search.SetMoving(0);
    auto samePlace=[](NAI::SPathPlace a,NAI::SPathPlace b){a.SetMoving(0);b.SetMoving(0);return a==b;};
    vector<NAI::SPathPlace> destinations;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),5.f),&destinations);CObj<NAI::CPath> route;
    for(auto destination:destinations) {
        destination.SetPose(NAI::CM_STAND);destination.SetDirection(search.GetDirection());
        auto target=actor->GetPosition().pos;target.p=destination;
        if(fabs2(target.GetCP()-actor->GetPosition().GetCP())<4.f || !network->IsPassable(destination) || network->IsLocked(destination))continue;
        auto candidate=CObj<NAI::CPath>(NWorld::FindPath(network,actor,search,vector<NAI::SPathPlace>{destination},nullptr,true,NAI::PF_DEFAULT,strafe));
        if(candidate && candidate->points.size()>=3 && samePlace(candidate->points.front(),search) && candidate->bStrafePath==strafe &&
           (candidate->points[1].GetX()!=search.GetX() || candidate->points[1].GetY()!=search.GetY()) && candidate->points[1].GetPose()==search.GetPose()) {route=candidate;break;}
    }
    Require(route && !move->IsWaitingForPath(),"continuation fixture could not find its unblocked native route beginning with a movement");
    const auto position=actor->GetPosition();const int ap=actor->GetAP();moving->SetNewPath(route,NAI::PF_DEFAULT);
    Require(actor->GetAP()==ap && actor->GetPosition()==position,"preparing a continuation committed its step or AP too early");
    auto* skeleton=actor->animator.GetSkeletonAnimator();const auto before=NativeCannonAnimationClips(skeleton);
    vector<CObj<NAnimation::CAnimation>> retained;for(auto* clip:before)retained.push_back(clip);
    const STime start=actor->animator.GetTimeEnd();world->GetTime()->Set(start);move->AnimationFinished();
    bool trajectory=false;const auto after=NativeCannonAnimationClips(skeleton);
    for(auto* clip:after)if(std::find(before.begin(),before.end(),clip)==before.end() && clip->GetDistance()>0.01f && clip->GetTime()>0)trajectory=true;
    std::printf("native movement continuation: strafe=%d AP=%d->%d duration=%u newTrajectory=%d position=%u,%u->%u,%u\n",int(strafe),ap,actor->GetAP(),
        unsigned(actor->animator.GetTimeEnd()-start),int(trajectory),position.pos.p.GetX(),position.pos.p.GetY(),actor->GetPosition().pos.p.GetX(),actor->GetPosition().pos.p.GetY());std::fflush(stdout);
    Require(trajectory,"rerouting to another movement inserted a stand instead of continuing the native trajectory");
    Require(actor->GetAP()<ap && samePlace(actor->GetPosition().pos.p,strafe?search:route->points[1]),"native continuation did not commit exactly its next logical step");
    for(int edge=0;edge<64 && move->GetState()==NWorld::CCommandExecute::RUNNING;++edge) {
        world->GetTime()->Set(actor->animator.GetTimeEnd());move->AnimationFinished();
    }
    Require(move->GetState()==NWorld::CCommandExecute::FINISHED && samePlace(actor->GetPosition().pos.p,route->points.back()),"native continuation did not reach its real path destination");
    moving->FullCancel();move=nullptr;Require(!world->IsAction(),"continuation fixture retained its native world action");
}

static void MovementRerouteFinalStep(const char* databasePath,const char* resourcePath)
{
    auto fixture=MakeNativeMovementFixture(databasePath,resourcePath);auto* actor=fixture.actor.GetPtr();auto* world=fixture.world.GetPtr();
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));world->GetTime()->Set(1000);
    actor->animator.AlignTime();actor->animator.PlaceUnit(actor->GetPosition());
    CObj<NWorld::CCommandExecute> move=NWorld::CreateSimpleMoveExecutor(actor,fixture.path,NAI::PF_DEFAULT);
    Require(move,"reroute final-step fixture could not create its native mover");
    auto walking=[](CObjectBase* command){return command && pSSClasses->GetObjectTypeID(command)==int(0x02731142);};
    move->Run();
    for(int edge=0;edge<32 && !walking(ReadMovementCommands(move).current.GetPtr()) && move->GetState()==NWorld::CCommandExecute::RUNNING;++edge) {
        world->GetTime()->Set(actor->animator.GetTimeEnd());move->AnimationFinished();
    }
    auto commands=ReadMovementCommands(move);Require(walking(commands.current),"reroute final-step fixture did not reach an actual pending walk");
    auto* moving=dynamic_cast<NWorld::IExecMove*>(move.GetPtr());NAI::SPathPlace search;moving->GetSearchFromPosition(&search);
    Require(!(search==actor->GetPosition().pos.p) && !move->IsWaitingForPath(),"reroute final-step fixture lacks a pending unblocked step");
    auto samePlace=[](NAI::SPathPlace a,NAI::SPathPlace b){a.SetMoving(0);b.SetMoving(0);return a==b;};
    search.SetMoving(0);auto target=search;target.SetPose(NAI::CM_CROUCH);
    CObj<NAI::CPath> path=NWorld::FindPath(world->GetPathNetwork(),actor,search,vector<NAI::SPathPlace>{target},nullptr,true,NAI::PF_USE_POSEDIR);
    std::printf("native reroute path: search=%08x target=%08x points=%u first=%08x last=%08x\n",unsigned(search.GetData()),unsigned(target.GetData()),path?unsigned(path->points.size()):0,
        path && !path->points.empty()?unsigned(path->points.front().GetData()):0,path && !path->points.empty()?unsigned(path->points.back().GetData()):0);std::fflush(stdout);
    Require(path && path->points.size()>=2 && samePlace(path->points.front(),search) && samePlace(path->points.back(),target),
        "reroute final-step fixture could not find a native same-cell crouch path from the pending destination");
    moving->SetNewPath(path,NAI::PF_USE_POSEDIR);
    auto replaced=ReadMovementCommands(move);
    Require(!replaced.queued.empty() && pSSClasses->GetObjectTypeID(replaced.queued.front())==int(0x12541120),
        "reroute final-step fixture lacks the actual movement ender before its pose change");
    auto* skeleton=actor->animator.GetSkeletonAnimator();const auto before=NativeCannonAnimationClips(skeleton);
    vector<CObj<NAnimation::CAnimation>> retained;for(auto* clip:before)retained.push_back(clip);
    const auto position=actor->GetPosition();const int ap=actor->GetAP();const STime start=actor->animator.GetTimeEnd();
    world->GetTime()->Set(start);move->AnimationFinished();
    const auto after=NativeCannonAnimationClips(skeleton);bool hasFinalStep=false;
    for(auto* clip:after)if(std::find(before.begin(),before.end(),clip)==before.end()) {
        std::printf("native reroute final clip: id=%d distance=%.6f time=%u\n",clip->GetRecordID(),double(clip->GetDistance()),unsigned(clip->GetTime()));
        if(clip->GetDistance()>0.01f && clip->GetTime()>0)hasFinalStep=true;
    }
    std::printf("native reroute final step: before=%u,%u pending=%u,%u actual=%u,%u duration=%u AP=%d->%d trajectory=%d\n",
        position.pos.p.GetX(),position.pos.p.GetY(),search.GetX(),search.GetY(),actor->GetPosition().pos.p.GetX(),actor->GetPosition().pos.p.GetY(),
        unsigned(actor->animator.GetTimeEnd()-start),ap,actor->GetAP(),int(hasFinalStep));std::fflush(stdout);
    Require(samePlace(actor->GetPosition().pos.p,search) && actor->GetAP()<ap,"reroute did not commit its real pending step through DoGameMove");
    Require(hasFinalStep,"rerouting a walking step to a pose change lost its final native movement trajectory");
    for(int edge=0;edge<32 && move->GetState()==NWorld::CCommandExecute::RUNNING;++edge) {
        world->GetTime()->Set(actor->animator.GetTimeEnd());move->AnimationFinished();
    }
    Require(move->GetState()==NWorld::CCommandExecute::FINISHED && samePlace(actor->GetPosition().pos.p,target),
        "rerouted walking step did not finish its actual native crouch transition");
    moving->FullCancel();move=nullptr;Require(!world->IsAction(),"reroute final-step fixture retained its native world action");
}

static void CannonEntryAnimation(const char* databasePath,const char* resourcePath)
{
    auto fixture=MakeNativeCannonFixture(databasePath,resourcePath);auto* actor=fixture.actor.GetPtr();auto* cannon=fixture.cannon.GetPtr();
    auto* animator=actor->animator.GetSkeletonAnimator();const STime start=actor->animator.GetTimeEnd();
    CObj<NAnimation::CAnimation> control=animator->CreateAnimation(fixture.entryAnimation,start);
    Require(control && control->GetTime()>0,"native authored cannon entry clip did not load with a real nonzero duration");
    const auto before=NativeCannonAnimationClips(animator);
    Require(std::none_of(before.begin(),before.end(),[&](NAnimation::CAnimation* clip){return clip->GetRecordID()==fixture.entryAnimation->GetRecordID();}),
        "cannon entry fixture already had its expected clip in the actual skeleton graph");
    CObj<NWorld::CCmdCannon> command=new NWorld::CCmdCannon(cannon);
    CObj<NWorld::CExecCannon> executor=new NWorld::CExecCannon(actor,cannon,true,command);
    Require(executor->CanDoIt(actor->GetPosition())==NWorld::UCR_OK,"native actor cannot enter its actual unoccupied cannon");
    executor->Run();
    const STime end=actor->animator.GetTimeEnd();const auto after=NativeCannonAnimationClips(animator);
    NAnimation::CAnimation* played=nullptr;
    for(auto* clip:after)
        if(std::find(before.begin(),before.end(),clip)==before.end() &&
           std::find(fixture.entryClipIDs.begin(),fixture.entryClipIDs.end(),clip->GetRecordID())!=fixture.entryClipIDs.end()) {played=clip;break;}
    std::printf("native cannon entry: controlClip=%d actualClip=%d duration=%u controlDuration=%u allowedEntryClips=%u\n",fixture.entryAnimation->GetRecordID(),
        played?played->GetRecordID():0,unsigned(end-start),unsigned(control->GetTime()),unsigned(fixture.entryClipIDs.size()));std::fflush(stdout);
    Require(played && played->GetTime()>0 && end-start==played->GetTime(),"native cannon approach lost its authored entry clip by filtering on the personal weapon instead of stance-only flags");
    CMemoryStream fields;CStructureNetworkContext refs;refs.include=[](CObjectBase*){return false;};int flags=0;
    {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);actor->animator.operator&(writer);}
    fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(4,&flags);}
    Require(flags==NDb::CAnimation::POSE_STAND,"native cannon approach retained personal weapon flags instead of the original stance-only flags");
    fixture.world->GetTime()->Set(start+played->GetTime()/2);CVec3 hip;
    Require(actor->animator.GetHipPos(&hip) && std::isfinite(hip.x) && std::isfinite(hip.y) && std::isfinite(hip.z),
        "native cannon approach did not produce a finite real skeleton pose during its entry clip");
    fixture.world->GetTime()->Set(end);executor->AnimationFinished();executor=nullptr;
    CObj<NWorld::CExecCannon> leave=new NWorld::CExecCannon(actor,cannon,false);leave->Run();leave->AnimationFinished();leave=nullptr;
    Require(!fixture.world->IsAction() && !cannon->IsOccupied() && !actor->GetUnitRPG()->GetCannonItem(),
        "native cannon animation fixture did not release its actual mounted state and action");
}

static void SaveNativeCannonGraph(CDataStream& snapshot,NativeCannonFixture& fixture,CObj<NWorld::CExecCannon>& executor,int paidAP,int cost)
{
    CObj<NWorld::CUnitServer> actor=fixture.actor.GetPtr();CObj<NWorld::CCannon> cannon=fixture.cannon.GetPtr();
    CStructureSaver writer(snapshot,CStructureSaver::WRITE);writer.Add(1,&fixture.game);writer.Add(2,&fixture.world);
    writer.Add(3,&actor);writer.Add(4,&cannon);writer.Add(5,&executor);writer.Add(6,&paidAP);writer.Add(7,&cost);
}

static void LoadNativeCannonGraph(CDataStream& snapshot,NativeCannonFixture* fixture,CObj<NWorld::CExecCannon>* executor,int* paidAP,int* cost)
{
    CObj<NWorld::CUnitServer> actor;CObj<NWorld::CCannon> cannon;
    {CStructureSaver reader(snapshot,CStructureSaver::READ);reader.Add(1,&fixture->game);reader.Add(2,&fixture->world);
     reader.Add(3,&actor);reader.Add(4,&cannon);reader.Add(5,executor);reader.Add(6,paidAP);reader.Add(7,cost);}
    Require(fixture->game && fixture->world && actor && cannon && *executor,"cannon snapshot lost part of its complete native game/world/actor/cannon/executor graph");
    fixture->actor=actor.GetPtr();fixture->cannon=cannon.GetPtr();fixture->world->RestoreRuntimeCaches(fixture->game);
    Require((*executor)->GetUnitServer()==actor.GetPtr(),"cannon snapshot lost native executor/actor reference identity");
}

static void FinishNativeCannonFixture(NativeCannonFixture& fixture,CObj<NWorld::CExecCannon>& executor)
{
    auto* actor=fixture.actor.GetPtr();auto* cannon=fixture.cannon.GetPtr();
    executor=nullptr;
    CObj<NWorld::CExecCannon> leave=new NWorld::CExecCannon(actor,cannon,false);leave->Run();leave->AnimationFinished();leave=nullptr;
    Require(!fixture.world->IsAction() && !cannon->IsOccupied() && !actor->GetUnitRPG()->GetCannonItem(),
        "native cannon fixture did not release its completed entry and exit actions");
}

static void WriteLegacyCannonEntryFixture(const char* databasePath,const char* resourcePath,const char* fixturePath)
{
    auto fixture=MakeNativeCannonFixture(databasePath,resourcePath);auto* actor=fixture.actor.GetPtr();auto* cannon=fixture.cannon.GetPtr();
    CObj<NWorld::CCmdCannon> command=new NWorld::CCmdCannon(cannon);
    CObj<NWorld::CExecCannon> executor=new NWorld::CExecCannon(actor,cannon,true,command);
    const int beforeAP=actor->GetAP(),cost=executor->GetStartAP();Require(cost>0 && beforeAP>=cost,"legacy cannon fixture has no affordable paid entry control");
    executor->Run();const int paidAP=actor->GetAP();
    Require(paidAP==beforeAP-cost && actor->GetUnitRPG()->GetCannonItem()==cannon->GetItem() && cannon->GetCurrentUnit()==actor &&
        actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN && executor->GetState()==NWorld::CCommandExecute::RUNNING && executor->IsExecuting(),
        "legacy cannon generator requires the actual historical already-paid and already-mounted running entry implementation");
    CFileStream snapshot;snapshot.OpenWrite(fixturePath);SaveNativeCannonGraph(snapshot,fixture,executor,paidAP,cost);
    std::printf("native legacy cannon snapshot written: bytes=%d AP=%d->%d cost=%d mounted=1 entryRunning=1\n",snapshot.GetSize(),beforeAP,paidAP,cost);std::fflush(stdout);
    executor->AnimationFinished();FinishNativeCannonFixture(fixture,executor);
}

static void LegacyCannonEntryCompletion(const char* databasePath,const char* resourcePath,const char* fixturePath)
{
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    NativeCannonFixture fixture;CObj<NWorld::CExecCannon> executor;int paidAP=-1,cost=-1;
    CFileStream snapshot;snapshot.OpenRead(fixturePath);LoadNativeCannonGraph(snapshot,&fixture,&executor,&paidAP,&cost);
    auto* actor=fixture.actor.GetPtr();auto* cannon=fixture.cannon.GetPtr();
    Require(cost>0 && paidAP>=0 && actor->GetAP()==paidAP && actor->GetUnitRPG()->GetCannonItem()==cannon->GetItem() &&
        cannon->GetCurrentUnit()==actor && actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN &&
        executor->GetState()==NWorld::CCommandExecute::RUNNING && executor->IsExecuting(),
        "legacy cannon snapshot did not restore its real already-paid mounted running phase");
    fixture.world->GetTime()->Set(actor->animator.GetTimeEnd());executor->AnimationFinished();
    std::printf("native legacy cannon completion: AP=%d->%d historicalCost=%d mounted=%d finished=%d\n",paidAP,actor->GetAP(),cost,
        int(cannon->GetCurrentUnit()==actor),int(executor->GetState()==NWorld::CCommandExecute::FINISHED));std::fflush(stdout);
    Require(actor->GetAP()==paidAP && actor->GetUnitRPG()->GetCannonItem()==cannon->GetItem() && cannon->GetCurrentUnit()==actor &&
        actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN && executor->GetState()==NWorld::CCommandExecute::FINISHED,
        "completing an actual historical cannon save charged its already-paid approach again or lost its mounted state");
    FinishNativeCannonFixture(fixture,executor);
}

static void CannonEntryCompletion(const char* databasePath,const char* resourcePath)
{
    auto fixture=MakeNativeCannonFixture(databasePath,resourcePath);auto* actor=fixture.actor.GetPtr();auto* cannon=fixture.cannon.GetPtr();
    CObj<NWorld::CCmdCannon> command=new NWorld::CCmdCannon(cannon);
    CObj<NWorld::CExecCannon> executor=new NWorld::CExecCannon(actor,cannon,true,command);
    const int beforeAP=actor->GetAP(),cost=executor->GetStartAP();const auto state=actor->GetState();const auto position=actor->GetPosition();
    Require(cost>0 && beforeAP>=cost && !actor->GetUnitRPG()->GetCannonItem() && !cannon->IsOccupied() && state!=NWorld::CUnit::ST_MACHINE_GUN,
        "cannon completion fixture lacks an affordable initially unmounted native actor");
    const STime start=actor->animator.GetTimeEnd();executor->Run();const STime end=actor->animator.GetTimeEnd();
    std::printf("native cannon approach phase: AP=%d->%d cost=%d mountedItem=%d occupied=%d state=%d->%d duration=%u\n",beforeAP,actor->GetAP(),cost,
        int(actor->GetUnitRPG()->GetCannonItem()!=nullptr),int(cannon->IsOccupied()),int(state),int(actor->GetState()),unsigned(end-start));std::fflush(stdout);
    Require(end>start && executor->GetState()==NWorld::CCommandExecute::RUNNING && executor->IsExecuting(),
        "cannon completion fixture did not start its actual nonzero approach clip and native action");
    Require(actor->GetAP()==beforeAP && !actor->GetUnitRPG()->GetCannonItem() && !cannon->IsOccupied() && actor->GetState()==state && actor->GetPosition()==position,
        "native cannon entry committed AP, weapon, occupancy or mounted state before its approach animation finished");
    fixture.world->GetTime()->Set(start+(end-start)/2);
    CMemoryStream snapshot;SaveNativeCannonGraph(snapshot,fixture,executor,beforeAP,cost);snapshot.Seek(0);
    NativeCannonFixture restored;CObj<NWorld::CExecCannon> restoredExecutor;int restoredAP=-1,restoredCost=-1;
    LoadNativeCannonGraph(snapshot,&restored,&restoredExecutor,&restoredAP,&restoredCost);
    Require(restored.world.GetPtr()!=fixture.world.GetPtr() && restored.actor.GetPtr()!=actor && restored.cannon.GetPtr()!=cannon &&
        restoredAP==beforeAP && restoredCost==cost && restored.actor->GetAP()==beforeAP && !restored.actor->GetUnitRPG()->GetCannonItem() &&
        !restored.cannon->IsOccupied() && restored.actor->GetState()==state && restoredExecutor->IsExecuting(),
        "loading an independent in-flight cannon approach committed its uncompleted action or lost its native running phase");
    auto finish=[&](NativeCannonFixture& current,CObj<NWorld::CExecCannon>& action) {
        auto* unit=current.actor.GetPtr();auto* gun=current.cannon.GetPtr();current.world->GetTime()->Set(unit->animator.GetTimeEnd());
        action->AnimationFinished();
        std::printf("native cannon finished: restored=%d AP=%d->%d cost=%d mounted=%d state=%d\n",int(unit!=actor),beforeAP,unit->GetAP(),cost,
            int(gun->GetCurrentUnit()==unit),int(unit->GetState()));std::fflush(stdout);
        Require(action->GetState()==NWorld::CCommandExecute::FINISHED && unit->GetAP()==beforeAP-cost &&
            unit->GetUnitRPG()->GetCannonItem()==gun->GetItem() && gun->GetCurrentUnit()==unit && unit->GetState()==NWorld::CUnit::ST_MACHINE_GUN,
            "native cannon completion did not commit its original cost, effective weapon, occupant and state exactly at animation completion");
        FinishNativeCannonFixture(current,action);
    };
    std::printf("native in-flight cannon full graph restored: bytes=%d\n",snapshot.GetSize());finish(restored,restoredExecutor);finish(fixture,executor);
}

// Complete actual queue callbacks at their native animation deadlines; no animator
// weapon/active flags are injected after the authored fixture is constructed.
static void CompleteCannonPoseQueue(NWorld::CCommandExecute* executor,NativeCannonFixture& fixture)
{
    for(int step=0;step<32 && executor->GetState()==NWorld::CCommandExecute::RUNNING;++step) {
        fixture.world->GetTime()->Set(fixture.actor->animator.GetTimeLabel1());executor->TimeLabelReached();
        fixture.world->GetTime()->Set(fixture.actor->animator.GetTimeEnd());executor->AnimationFinished();
    }
    Require(executor->GetState()==NWorld::CCommandExecute::FINISHED,"personal cannon pose fixture failed to complete its actual native queue");
}

static void NativeMeleeAttackerIgnore(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=2,"unknown native melee attacker ignore mode");auto fixture=MakeNativeCannonFixture(databasePath,resourcePath);auto* world=fixture.world.GetPtr();auto* actor=fixture.actor.GetPtr();auto* cannon=fixture.cannon.GetPtr();
    if(mode!=0){world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCmdCannon> request=new NWorld::CCmdCannon(cannon);CObj<NWorld::CCommandExecute> entry=NWorld::CreateExecutor(actor,request,&result);Require(entry && result==NWorld::UCR_OK,"melee ignore fixture cannot prepare actual mount");entry->Run();CompleteCannonPoseQueue(entry,fixture);entry=nullptr;request=nullptr;Require(actor->animator.GetCannon()==cannon && cannon->GetCurrentUnit()==actor && actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN && !world->IsAction(),"melee ignore fixture did not finish actual native mount");}
    auto* mission=actor->GetUnitRPG();auto* rpg=mission->GetRPGUnit();rpg->Skills(NDb::ST_VP).SetConst(500);rpg->nHealedVP=0;rpg->Skills(NDb::ST_IC).SetConst(0);rpg->Skills(NDb::ST_IC).SetProgress(0);Require(mission->GetTotalVP()==500 && mission->GetIC()==0 && actor->CanFight(),"melee ignore fixture lacks healthy no-evasion native actor");auto* map=world->GetAIMap();map->Sync();CVec3 center;Require(map->GetUnitHLPos(&center,map->GetHull(actor),NAI::HL_BODY),"melee ignore fixture lacks actual actor body locator");if(mode==2){SBound bound;Require(map->GetObjectBound(&bound,cannon),"cannon ignore control lacks actual native bounds");center=bound.s.ptCenter;}CRay ray;int actorPieces=0,cannonPieces=0;NDb::CRPGArmor* armor=nullptr;bool found=false;
    const CVec3 directions[]={CVec3(1,0,0),CVec3(0,1,0),CVec3(-1,0,0),CVec3(0,-1,0),CVec3(0,0,1),CVec3(0,0,-1)};
    for(auto direction:directions){ray.ptDir=direction;ray.ptOrigin=center-direction*2.f;vector<NAI::SInterval> intervals;map->Trace(ray,&intervals,NWorld::TS_FRAGMENTED);actorPieces=cannonPieces=0;armor=nullptr;for(const auto& interval:intervals)if(interval.enter.fT>0){if(interval.pSrc->pUserData==actor){++actorPieces;if(!armor)armor=interval.pSrc->pArmor;}if(interval.pSrc->pUserData==cannon){++cannonPieces;if(mode==2 && !armor)armor=interval.pSrc->pArmor;}}if((mode==2?cannonPieces>0:actorPieces>0) && armor && armor->pMaterial){found=true;break;}}
    Require(found,"melee ignore fixture lacks required actual actor/cannon hull intersections");int damageType=-1;CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());while(types.MoveNext())if(types.Get()->armors[armor->pMaterial->nDR]>0 && (damageType<0 || types.Get()->GetRecordID()<damageType))damageType=types.Get()->GetRecordID();Require(damageType>=0,"melee ignore fixture lacks native flesh damage type");NRPG::CAttackPortion portion(100000,damageType,0.f,2,2,0,0,mission,mission,1.f,-1);const int base=portion.CalcStructDmg(world,armor,0);Require(base>0 && base<6,"melee ignore fixture lacks small native literal damage");vector<NRPG::CAttackPortion> portions(1,portion);CObj<NWorld::CExecMeleeTile> exec=new NWorld::CExecMeleeTile(actor,center);
    for(;;){CObj<NWorld::CHitLocator> event=world->GetHitEvent();if(!event)break;}const int ap=actor->GetAP(),cannonStage=cannon->GetDestroyStage(),cannonHP=cannon->GetHP();const auto position=actor->GetPosition();CStructureNetworkContext cannonRefs;auto cannonBefore=BorrowedObjectFields(cannon,&cannonRefs);GlobalGameRandom().SeedForHarness(655);exec->PerformAttack(portions,ray,1.f);int actorHits=0;for(;;){CObj<NWorld::CHitLocator> event=world->GetHitEvent();if(!event)break;if(event->pUnit.GetPtr()==actor)++actorHits;}
    std::printf("native melee attacker ignore mode=%d mounted=%d base=%d actorPieces=%d cannonPieces=%d selfDrop=%d selfHits=%d cannonStage=%d expectedStage=%d\n",mode,int(actor->animator.GetCannon()!=nullptr),base,actorPieces,cannonPieces,500-mission->GetTotalVP(),actorHits,cannon->GetDestroyStage(),cannonStage);std::fflush(stdout);Require(mission->GetTotalVP()==500 && actorHits==0,"native melee ray struck its own mounted attacker");Require(actor->GetAP()==ap && actor->GetPosition()==position && actor->CanFight(),"melee ignore changed native actor AP/position/life");CStructureNetworkContext afterRefs;auto cannonAfter=BorrowedObjectFields(cannon,&afterRefs);Require(cannon->GetDestroyStage()==cannonStage && cannon->GetHP()==cannonHP && cannonBefore.GetSize()==cannonAfter.GetSize() && !std::memcmp(cannonBefore.GetBuffer(),cannonAfter.GetBuffer(),cannonBefore.GetSize()),"melee ignore changed existing cannon alias protection");if(mode!=0)Require(actor->animator.GetCannon()==cannon && cannon->GetCurrentUnit()==actor,"melee ignore damaged actual mount identity");
}

static void CannonPersonalPose(const char* databasePath,const char* resourcePath,int mode)
{
    auto fixture=MakeNativeCannonFixture(databasePath,resourcePath);auto* actor=fixture.actor.GetPtr();auto* cannon=fixture.cannon.GetPtr();
    auto* rpg=actor->GetUnitRPG();auto* inventory=rpg->GetInventory();
    auto* personal=dynamic_cast<NRPG::CWeaponItem*>(inventory->GetActive());auto* mounted=dynamic_cast<NRPG::CWeaponItem*>(cannon->GetItem());
    Require(personal && mounted,"personal cannon pose fixture lacks actual personal/mounted weapons");
    fixture.world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    // The base actor carries the same weapon type as this cannon. Equip a real
    // authored pistol in the other hand and switch through the normal executor.
    CObj<NRPG::CWeaponItem> selected;CDBIterator<NDb::CRPGWeapon> records(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while(records.MoveNext()) {auto* record=records.Get();if(!record->pItem || !record->pItem->bPlaceInHand || !record->pItem->pModel || !record->pAnimWeaponType)continue;
        CObj<NRPG::CWeaponItem> candidate=new NRPG::CWeaponItem(record);if(candidate->GetWeaponType()==NDb::WT_PISTOL && candidate->GetWeaponType()!=mounted->GetWeaponType() && (!selected || record->GetRecordID()<selected->GetDBWeapon()->GetRecordID()))selected=candidate;}
    Require(selected,"personal cannon pose fixture lacks a distinct authored pistol");const auto slot=inventory->GetActiveSlot()==NDb::SLOT_1?NDb::SLOT_2:NDb::SLOT_1;
    CObj<NRPG::IInventoryItem> previous=inventory->Get(slot)?inventory->TakeOff(slot):nullptr;Require(inventory->Equip(slot,selected),"personal cannon pose fixture cannot equip its actual distinct pistol");
    NWorld::EUnitCommandResult switchResult=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCmdSetActiveItem> switchRequest=new NWorld::CCmdSetActiveItem(slot);
    CObj<NWorld::CCommandExecute> switchItem=NWorld::CreateExecutor(actor,switchRequest,&switchResult);Require(switchItem && switchResult==NWorld::UCR_OK,"personal cannon pose fixture cannot switch its real active hand");
    switchItem->Run();CompleteCannonPoseQueue(switchItem,fixture);switchItem=nullptr;personal=selected.GetPtr();
    Require(inventory->GetActive()==personal && actor->animator.IsActiveItem() && !fixture.world->IsAction(),"personal cannon pose fixture did not finish its actual personal weapon switch");
    const auto personalType=personal->GetWeaponType(),mountedType=mounted->GetWeaponType();const int personalAmmo=personal->GetAmmoQuantity();
    NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCmdCannon> request=new NWorld::CCmdCannon(cannon);
    CObj<NWorld::CCommandExecute> entry=NWorld::CreateExecutor(actor,request,&result);
    Require(entry && result==NWorld::UCR_OK,"personal cannon pose fixture cannot create the real entry preparation queue");
    entry->Run();CompleteCannonPoseQueue(entry,fixture);entry=nullptr;
    Require(rpg->GetCannonItem()==mounted && cannon->GetCurrentUnit()==actor && actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN && !actor->animator.IsActiveItem() && !fixture.world->IsAction(),
        "personal cannon pose fixture did not holster its personal weapon and finish real native entry");
    if(mode!=1) {
        while(mounted->GetAmmoQuantity()>0)Require(mounted->Unload(inventory),"personal cannon pose fixture cannot unload its actual cannon ammunition to the native inventory");
        Require(mounted->CanReload(inventory),"personal cannon pose fixture lacks actual compatible cannon ammunition");
        CObj<NWorld::CCmdReload> request=new NWorld::CCmdReload;
        CObj<NWorld::CCommandExecute> reload=NWorld::CreateExecutor(actor,request,&result);
        Require(reload && result==NWorld::UCR_OK,"personal cannon pose fixture cannot create its actual mounted reload queue");
        CObj<NWorld::CCommandExecute> actual=FindNativeReload(reload);reload->Run();
        if(mode==4) {
            for(int step=0;step<32 && !actual->IsExecuting();++step) {
                fixture.world->GetTime()->Set(actor->animator.GetTimeLabel1());reload->TimeLabelReached();
                fixture.world->GetTime()->Set(actor->animator.GetTimeEnd());reload->AnimationFinished();
            }
            Require(actual->IsExecuting() && mounted->GetAmmoQuantity()==0,"cannon reload cancellation control missed the native pre-transfer phase");
            // CExecReload inherits the original no-op Cancel. Dropping its owning
            // queue releases the skippable action without transferring ammunition.
            reload->Cancel();Require(mounted->GetAmmoQuantity()==0,"cancelled native reload unexpectedly transferred ammunition");
        }else CompleteCannonPoseQueue(reload,fixture);
        actual=nullptr;reload=nullptr;
        Require(!fixture.world->IsAction() && actor->animator.IsActiveItem() && personal->GetAmmoQuantity()==personalAmmo && (mode==4?mounted->GetAmmoQuantity()==0:mounted->GetAmmoQuantity()>0),
            "native reload/activation lost its completed mount, transferred personal ammunition or failed the intended reload/cancel outcome");
    }
    CObj<NWorld::CExecCannon> exit=new NWorld::CExecCannon(actor,cannon,false);
    if(mode==5) {
        const int paidAP=actor->GetAP();CMemoryStream snapshot;SaveNativeCannonGraph(snapshot,fixture,exit,paidAP,0);snapshot.Seek(0);
        int restoredAP=-1,cost=-1;LoadNativeCannonGraph(snapshot,&fixture,&exit,&restoredAP,&cost);
        actor=fixture.actor.GetPtr();cannon=fixture.cannon.GetPtr();rpg=actor->GetUnitRPG();inventory=rpg->GetInventory();
        personal=dynamic_cast<NRPG::CWeaponItem*>(inventory->GetActive());mounted=dynamic_cast<NRPG::CWeaponItem*>(cannon->GetItem());
        Require(actor->GetAP()==paidAP && restoredAP==paidAP && cost==0 && rpg->GetCannonItem()==mounted && cannon->GetCurrentUnit()==actor && actor->animator.GetCannon()==cannon,
            "personal cannon pose roundtrip lost its actual mounted graph, AP or weapon identities");
    }
    for(int hit=0;hit<(mode==2?1:mode==3?2:0);++hit) {
        CRay ray;ray.ptOrigin=actor->GetPosition().GetCP();ray.ptDir=CVec3(0,1,0);NRPG::CAttackPortion attack(0,0,0.f,0,0,0);attack.MakeClickOfDeath(ray);
        Require(rpg->ProcessAttack(fixture.world.GetPtr(),NAI::HL_BODY,&attack,rpg->GetRPGArmor()).nDmg>=0,"cannon pose native damage control failed");
    }
    struct State {NDb::EWeaponType type=NDb::WT_DEFAULT;int flags=0,classFlags=0;CDBPtr<NDb::CSkeleton> skeleton;CDBPtr<NDb::CSide> side;};
    auto read=[&]() {State state;CMemoryStream fields;CStructureNetworkContext refs;refs.include=[](CObjectBase*){return false;};
        {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);actor->animator.operator&(writer);}fields.Seek(0);
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(4,&state.flags);reader.Add(5,&state.classFlags);reader.Add(29,&state.type);reader.Add(31,&state.skeleton);reader.Add(40,&state.side);}return state;};
    const auto before=read();const auto position=actor->GetPosition();const int ap=actor->GetAP(),hp=rpg->GetTotalVP(),ammo=mounted->GetAmmoQuantity();
    const bool dead=rpg->IsDead(),unconscious=rpg->IsUnconscious(),downed=dead || unconscious;const STime beforeEnd=actor->animator.GetTimeEnd();
    Require(dead==(mode==3) && unconscious==(mode==2) && (mode==1 || before.type==mountedType),"real mounted reload did not naturally populate its mounted animation type or expected RPG damage result");
    const auto priorClips=NativeCannonAnimationClips(actor->animator.GetSkeletonAnimator());auto random=GlobalGameRandom();
    Require(exit->CanDoIt(position)==NWorld::UCR_OK,"cannon pose native queued exit became unavailable");exit->Run();const auto after=read();
    const auto clips=NativeCannonAnimationClips(actor->animator.GetSkeletonAnimator());
    const int expectedFlags=NDb::CAnimation::POSE_STAND|NDb::WeaponTypeToAnimFlags(personalType,actor->animator.IsActiveItem(),false);
    auto* expected=after.skeleton?after.skeleton->GetAnimation(NDb::CAnimation::POSE,expectedFlags,nullptr,after.classFlags,after.side):nullptr;
    bool personalPose=false;for(auto* clip:clips)if(expected && clip->GetRecordID()==expected->GetRecordID() && std::find(priorClips.begin(),priorClips.end(),clip)==priorClips.end())personalPose=true;
    auto afterRandom=GlobalGameRandom();
    std::printf("native cannon personal pose: mode=%d personalType=%d mountedType=%d beforeType=%d afterType=%d beforeFlags=0x%X afterFlags=0x%X expectedFlags=0x%X expectedClip=%d personalPose=%d active=%d dead=%d unconscious=%d AP=%d->%d HP=%d->%d ammo=%d->%d delay=%u\n",mode,int(personalType),int(mountedType),int(before.type),int(after.type),unsigned(before.flags),unsigned(after.flags),unsigned(expectedFlags),expected?expected->GetRecordID():0,int(personalPose),int(actor->animator.IsActiveItem()),int(dead),int(unconscious),ap,actor->GetAP(),hp,rpg->GetTotalVP(),ammo,mounted->GetAmmoQuantity(),unsigned(actor->animator.GetTimeEnd()-beforeEnd));std::fflush(stdout);
    Require(!rpg->GetCannonItem() && !cannon->IsOccupied() && actor->GetState()!=NWorld::CUnit::ST_MACHINE_GUN && actor->GetAP()==ap && rpg->GetTotalVP()==hp && actor->GetPosition()==position && personal->GetAmmoQuantity()==personalAmmo && mounted->GetAmmoQuantity()==ammo && rpg->IsDead()==dead && rpg->IsUnconscious()==unconscious && random.Get()==afterRandom.Get(),
        "cannon pose correction changed physical detachment, AP/HP/position, equipment ammunition, consciousness or game RNG");
    if(downed)Require(after.type==before.type && after.flags==before.flags && actor->animator.GetTimeEnd()==beforeEnd && actor->animator.GetCannon()==cannon,"downed cannon exit changed its mounted animation context");
    else Require((mode==1 || after.type==personalType) && after.flags==expectedFlags && personalPose && !actor->animator.GetCannon(),"native cannon exit retained the mounted weapon type or selected the wrong personal standing animation");
    exit->AnimationFinished();exit=nullptr;Require(!fixture.world->IsAction(),"native cannon pose exit leaked its finished world action");
    if(downed)actor->SyncConscious();
}

static void CannonDismountClip(const char* databasePath,const char* resourcePath,int mode)
{
    auto fixture=MakeNativeCannonFixture(databasePath,resourcePath);auto* actor=fixture.actor.GetPtr();auto* cannon=fixture.cannon.GetPtr();
    fixture.world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCmdCannon> request=new NWorld::CCmdCannon(cannon);
    CObj<NWorld::CCommandExecute> entry=NWorld::CreateExecutor(actor,request,&result);Require(entry && result==NWorld::UCR_OK,"dismount clip fixture cannot prepare its real authored cannon entry");
    entry->Run();CompleteCannonPoseQueue(entry,fixture);entry=nullptr;request=nullptr;
    Require(actor->GetUnitRPG()->GetCannonItem()==cannon->GetItem() && cannon->GetCurrentUnit()==actor && actor->animator.GetCannon()==cannon && !fixture.world->IsAction(),"dismount clip fixture did not finish real native entry");
    CObj<NWorld::CExecCannon> exit=new NWorld::CExecCannon(actor,cannon,false);
    if(mode==3) {const int ap=actor->GetAP();CMemoryStream snapshot;SaveNativeCannonGraph(snapshot,fixture,exit,ap,0);snapshot.Seek(0);int savedAP=-1,cost=-1;
        LoadNativeCannonGraph(snapshot,&fixture,&exit,&savedAP,&cost);actor=fixture.actor.GetPtr();cannon=fixture.cannon.GetPtr();Require(actor->GetAP()==ap && savedAP==ap && cost==0 && actor->animator.GetCannon()==cannon,"dismount clip graph roundtrip lost mount/AP identity");}
    auto* rpg=actor->GetUnitRPG();for(int hit=0;hit<(mode==1?1:mode==2?2:0);++hit) {CRay ray;ray.ptOrigin=actor->GetPosition().GetCP();ray.ptDir=CVec3(0,1,0);NRPG::CAttackPortion attack(0,0,0.f,0,0,0);attack.MakeClickOfDeath(ray);Require(rpg->ProcessAttack(fixture.world.GetPtr(),NAI::HL_BODY,&attack,rpg->GetRPGArmor()).nDmg>=0,"dismount clip native incapacitation failed");}
    const bool downed=mode==1 || mode==2;Require(rpg->IsUnconscious()==(mode==1) && rpg->IsDead()==(mode==2),"dismount clip fixture has the wrong actual RPG damage result");
    CMemoryStream fields;CStructureNetworkContext refs;refs.include=[](CObjectBase*){return false;};CDBPtr<NDb::CSkeleton> skeleton;CDBPtr<NDb::CSide> side;string weapon;
    {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);actor->animator.operator&(writer);}fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(6,&weapon);reader.Add(31,&skeleton);reader.Add(40,&side);}
    Require(skeleton && !weapon.empty(),"dismount clip fixture lost its actual body skeleton or mounted animation name");
    vector<int> allowed;for(const auto& clip:skeleton->pAnimations[NDb::CAnimation::DEACTIVATE].anims)
        if(clip->szParams==weapon && (!clip->pSide || clip->pSide==side) && !(clip->nClassSexFlags&NDb::CAnimation::BOSS) && clip->fRndWeight>0)allowed.push_back(clip->GetRecordID());
    auto* expected=skeleton->GetAnimation(NDb::CAnimation::DEACTIVATE,0,weapon.c_str(),0,side);const STime start=actor->animator.GetTimeEnd();
    auto* body=actor->animator.GetSkeletonAnimator();CObj<NAnimation::CAnimation> control=body->CreateAnimation(expected,start);
    Require(control && control->GetTime()>0 && !allowed.empty(),"actual authored cannon has no real timed dismount clip for its body/name/side");
    const auto before=NativeCannonAnimationClips(body);const int ap=actor->GetAP(),hp=rpg->GetTotalVP(),ammo=cannon->GetItem()->GetAmmoQuantity();auto* personal=dynamic_cast<NRPG::CWeaponItem*>(rpg->GetInventory()->GetActive());Require(personal,"dismount clip fixture lacks its actual personal firearm");const int personalAmmo=personal->GetAmmoQuantity();const auto position=actor->GetPosition();auto random=GlobalGameRandom();
    Require(exit->CanDoIt(position)==NWorld::UCR_OK,"native mounted actor cannot run its queued dismount");exit->Run();const auto after=NativeCannonAnimationClips(body);
    NAnimation::CAnimation* played=nullptr;int count=0;STime poseStart=0;
    for(auto* clip:after)if(std::find(before.begin(),before.end(),clip)==before.end()) {
        if(std::find(allowed.begin(),allowed.end(),clip->GetRecordID())!=allowed.end()){played=clip;++count;}
        auto* record=NDb::GetAnimation(clip->GetRecordID());if(record && record->nType==NDb::CAnimation::POSE){CStructureNetworkContext clipRefs;auto clipFields=BorrowedObjectFields(clip,&clipRefs);CStructureSaver reader(clipFields,CStructureSaver::READ,&clipRefs);reader.Add(7,&poseStart);}
    }
    STime actualStart=0;bool placement=false,frameFinite=false;
    if(played) {CStructureNetworkContext clipRefs;auto clipFields=BorrowedObjectFields(played,&clipRefs);CPtr<NAnimation::CAnimator> placementOwner;
        {CStructureSaver reader(clipFields,CStructureSaver::READ,&clipRefs);reader.Add(3,&placementOwner);reader.Add(7,&actualStart);}auto* stand=dynamic_cast<NAnimation::CStandAnimator*>(placementOwner.GetPtr());Require(stand,"native dismount clip lost its real cannon-position standing owner");
        CStructureNetworkContext standRefs;auto standFields=BorrowedObjectFields(stand,&standRefs);CVec3 point;float angle=0;{CStructureSaver reader(standFields,CStructureSaver::READ,&standRefs);reader.Add(2,&point);reader.Add(3,&angle);}
        auto expectedPoint=cannon->GetPosition();expectedPoint.z=0;placement=fabs(point-expectedPoint)<.0001f && fabs(angle-cannon->GetDirection())<.0001f;
        fixture.world->GetTime()->Set(start+played->GetTime()/2);CVec3 hip;frameFinite=actor->animator.GetHipPos(&hip) && std::isfinite(hip.x) && std::isfinite(hip.y) && std::isfinite(hip.z);}
    auto afterRandom=GlobalGameRandom();std::printf("native cannon dismount clip: mode=%d downed=%d weapon=%s control=%d controlLength=%u played=%d count=%d length=%u start=%u actualStart=%u poseStart=%u end=%u placement=%d finiteFrame=%d AP=%d->%d HP=%d->%d ammo=%d->%d\n",mode,int(downed),weapon.c_str(),expected?expected->GetRecordID():0,unsigned(control->GetTime()),played?played->GetRecordID():0,count,played?unsigned(played->GetTime()):0,unsigned(start),unsigned(actualStart),unsigned(poseStart),unsigned(actor->animator.GetTimeEnd()),int(placement),int(frameFinite),ap,actor->GetAP(),hp,rpg->GetTotalVP(),ammo,cannon->GetItem()->GetAmmoQuantity());std::fflush(stdout);
    Require(!rpg->GetCannonItem() && !cannon->IsOccupied() && actor->GetState()!=NWorld::CUnit::ST_MACHINE_GUN && actor->GetAP()==ap && rpg->GetTotalVP()==hp && actor->GetPosition()==position && rpg->GetInventory()->GetActive()==personal && (personal?personal->GetAmmoQuantity():0)==personalAmmo && cannon->GetItem()->GetAmmoQuantity()==ammo && random.Get()==afterRandom.Get(),"native dismount clip changed logical exit, unit/item identity, AP/HP/position, ammo or game RNG");
    if(downed)Require(!played && count==0 && actor->animator.GetTimeEnd()==start && actor->animator.GetCannon()==cannon,"downed cannon exit inserted a live dismount clip or lost its death-animation context");
    else Require(played && count==1 && played->GetTime()>0 && actualStart==start && poseStart==start+played->GetTime() && actor->animator.GetTimeEnd()>poseStart && placement && frameFinite && !actor->animator.GetCannon(),"native cannon exit omitted or misplaced the authored dismount clip before its standing pose");
    fixture.world->GetTime()->Set(actor->animator.GetTimeEnd());exit->AnimationFinished();exit=nullptr;Require(!fixture.world->IsAction(),"native dismount clip leaked its finished world action");if(downed)actor->SyncConscious();
}

static void LadderDoorPath(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game,4517);auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());Require(network,"ladder door path lacks its actual map network");
    NAI::SPathPlace bottom,rung;bool found=false;int ladderLayer=-1,ladderIndex=-1;
    for(int layer=0;layer<network->GetNumLayers() && !found;++layer){auto* nodes=network->GetLayer(layer);for(int index=0;index<int(nodes->ladders.size());++index){const auto& ladder=nodes->ladders[index];auto landing=ladder.placeOnBottom;landing.SetPose(NAI::CM_STAND);
        if(ladder.pointPassable.size()<5 || !landing.IsIntegral() || landing.IsFinal() || !network->IsPassable(landing) || network->IsLocked(landing,false) || network->GetFlippersAt(landing).empty())continue;
        NAI::SPathPlace point(index,0,layer,(4+2*int(ladder.eDir))&7,NAI::CM_STAND,0);point.SetIntegral(0);if(network->GetPassability(point)!=NAI::AIP_YES)continue;auto above=point;above.SetY(1);if(network->GetPassability(above)!=NAI::AIP_YES)continue;
        auto aligned=landing;aligned.SetDirection(point.GetDirection());if(!network->IsPassable(aligned))continue;bottom=landing;rung=point;found=true;ladderLayer=layer;ladderIndex=index;break;}}
    Require(found,"ladder door path lacks an actual free authored ladder bottom overlapping a real door footprint");
    auto from=bottom,to=rung;if(mode==1)from.SetDirection(rung.GetDirection());if(mode==2){from=rung;to=rung;to.SetY(1);}if(mode==3){to=bottom;to.SetDirection(rung.GetDirection());}
    const auto transition=NAI::GetTransitionType(network,from,to);Require(transition==(mode==2?NAI::TT_LADDER_MOVE:mode==3?NAI::TT_TURN:NAI::TT_LADDER_DOWN),"ladder door path lost its actual native transition type");
    Require(network->GetPassability(from)==NAI::AIP_YES && network->GetPassability(to)==NAI::AIP_YES && !network->IsLocked(from) && !network->IsLocked(to),"ladder door path does not have real free valid endpoints");
    const auto sourceDoors=network->GetFlippersAt(from),targetDoors=network->GetFlippersAt(to);Require((mode==2?sourceDoors.empty():!sourceDoors.empty()) && (mode==3?!targetDoors.empty():targetDoors.empty()),"ladder door path does not distinguish actual tile door footprints from native rung coordinates");
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);vector<std::pair<int,int>> stats;vector<NAI::SUnitPosition> positions;for(auto unit:units){stats.push_back({unit->GetAP(),unit->GetUnitRPG()->GetTotalVP()});positions.push_back(unit->GetPosition());}const bool action=world->IsAction();auto random=GlobalGameRandom();
    CObj<NAI::CPath> path=new NAI::CPath;path->pNet=network;Require(path->AddNewPoint(from),"ladder door path cannot add its actual starting point");const bool added=path->AddNewPoint(to);auto expectedEnd=to;if(mode==3)expectedEnd.SetMoving(1);bool endpoint=added && path->points.back()==expectedEnd;
    const int expectedPoints=mode==3 || (mode==0 && from.GetDirection()!=to.GetDirection())?3:2;
    auto afterRandom=GlobalGameRandom();bool unchanged=world->IsAction()==action && random.Get()==afterRandom.Get();int index=0;for(auto unit:units){unchanged&=stats[index].first==unit->GetAP() && stats[index].second==unit->GetUnitRPG()->GetTotalVP() && positions[index]==unit->GetPosition();++index;}
    std::printf("native ladder door path: mode=%d layer=%d ladder=%d from=%08X to=%08X transition=%d sourceDoors=%u targetDoors=%u fromPass=%d toPass=%d added=%d points=%u expectedPoints=%d actions=%u endpoint=%d units=%u unchanged=%d\n",mode,ladderLayer,ladderIndex,unsigned(from.GetBits()),unsigned(to.GetBits()),int(transition),unsigned(sourceDoors.size()),unsigned(targetDoors.size()),int(network->GetPassability(from)),int(network->GetPassability(to)),int(added),unsigned(path->points.size()),expectedPoints,unsigned(path->actions.size()),int(endpoint),unsigned(stats.size()),int(unchanged));std::fflush(stdout);
    Require(unchanged,"native ladder path query changed actual unit AP/HP/position, world actions or game RNG");
    Require(added && endpoint && path->points.size()==size_t(expectedPoints) && path->actions.empty() && NAI::IsValidPath(*path),"from-door candidates interpreted native ladder coordinates as a tile and rejected a valid authored transition");
}

struct AnimationReseatFixture
{
    CObj<NRPG::CGlobalGame> game;CObj<NWorld::CWorld> world;CObj<NWorld::CUnitServer> actor;CObj<NAI::CPath> path;
};

static std::pair<CObj<NWorld::CCommandExecute>,CObj<NWorld::CCmd>> AnimationReseatOrder(NWorld::CUnitServer* actor)
{
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(actor,&refs);CObj<NWorld::CCommandExecute> executor;CObj<NWorld::CCmd> command;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(7,&executor);reader.Add(9,&command);}return {executor,command};
}

static CPtr<NWorld::CCommandExecute> AnimationReseatMover(NWorld::CCommandExecute* executor)
{
    if(executor && pSSClasses->GetObjectTypeID(executor)==int(0x03112170))return executor;CPtr<NWorld::CCommandExecute> mover;CStructureNetworkContext refs;
    refs.include=[&](CObjectBase* object){if(pSSClasses->GetObjectTypeID(object)==int(0x03112170)){mover=dynamic_cast<NWorld::CCommandExecute*>(object);return false;}return dynamic_cast<NWorld::CCommandExecute*>(object)!=nullptr;};
    CMemoryStream fields;{CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);static_cast<CObjectBase*>(executor)->operator&(writer);}return mover;
}

// Restore a real, unstarted CExecMove and its order through native actor save
// fields. This isolates interruption from the unrelated high-level path solver.
// All other actor fields are retained; animation flags come from real execution.
static void RestoreAnimationReseatExecutor(NWorld::CUnitServer* actor,NWorld::CCommandExecute* executor,NWorld::CCmd* command)
{
    CStructureNetworkContext refs;auto saved=BorrowedObjectFields(actor,&refs);
    for(CObjectBase* object:{static_cast<CObjectBase*>(executor),static_cast<CObjectBase*>(command)}){const int id=refs.nextID++;refs.ids[object]=id;refs.refs[id]=object;}
    CObj<NWorld::CCommandExecute> owned=executor;CObj<NWorld::CCmd> order=command;CMemoryStream replacement;
    {CStructureSaver writer(replacement,CStructureSaver::WRITE,&refs);writer.Add(7,&owned);writer.Add(9,&order);}
    auto original=OuterSaveChunks(saved),patch=OuterSaveChunks(replacement);Require(original.count(1) && patch.count(1),"native actor interruption setup lost its root chunk");
    CMemoryStream originalRoot,patchRoot;originalRoot.Write(original[1].data(),unsigned(original[1].size()));patchRoot.Write(patch[1].data(),unsigned(patch[1].size()));auto fields=OuterSaveChunks(originalRoot),changes=OuterSaveChunks(patchRoot);
    Require(changes.size()==2 && changes.count(7) && changes.count(9) && fields.count(7) && fields.count(9),"native actor interruption setup did not identify exactly its existing executor/order save tags");
    for(const auto& change:changes)fields[change.first]=change.second;
    auto encode=[](CMemoryStream& output,const std::map<unsigned char,vector<unsigned char>>& chunks){for(const auto& chunk:chunks){std::uint8_t length[4]={};std::size_t count=0;Require(S2FileIO::EncodeStructureLength(unsigned(chunk.second.size()),length,&count),"native actor interruption setup length encoding failed");output.Write(&chunk.first,1);output.Write(length,unsigned(count));if(!chunk.second.empty())output.Write(chunk.second.data(),unsigned(chunk.second.size()));}};
    CMemoryStream root;encode(root,fields);original[1].assign(static_cast<const unsigned char*>(root.GetBuffer()),static_cast<const unsigned char*>(root.GetBuffer())+root.GetSize());CMemoryStream loaded;encode(loaded,original);loaded.Seek(0);
    {CStructureSaver reader(loaded,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(actor)->operator&(reader);}
    Require(AnimationReseatOrder(actor).first==executor && AnimationReseatOrder(actor).second==command && executor->GetUnitServer()==actor,"native saved interruption setup lost real actor/executor/order reference identities");
}

static AnimationReseatFixture MakeAnimationReseatFixture(const char* databasePath,const char* resourcePath,int kind)
{
    AnimationReseatFixture fixture;fixture.world=FixtureWorld(databasePath,resourcePath,true,&fixture.game,kind==0?4517:810);auto* world=fixture.world.GetPtr();auto* network=world->GetPathNetwork();
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);for(auto entry:units){auto* actor=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());if(actor && !actor->IsAIUnit() && actor->CanFight() && !actor->IsWearingPK() && !actor->IsEmptyPK() && !actor->HasCommand()){fixture.actor=actor;break;}}
    Require(fixture.actor,"animation reseat fixture has no real ordinary idle party actor");auto* actor=fixture.actor.GetPtr();
    const auto original=actor->GetPosition();auto selected=original;int ladderCount=0;
    if(kind==0) {
        auto* nodes=dynamic_cast<NAI::CPathNetwork*>(network);Require(nodes,"animation reseat fixture has no actual nodes network");
        for(int layer=0;layer<network->GetNumLayers() && !fixture.path;++layer) {auto* nodeLayer=nodes->GetLayer(layer);ladderCount+=int(nodeLayer->ladders.size());
            for(int index=0;index<int(nodeLayer->ladders.size());++index) {const auto& ladder=nodeLayer->ladders[index];auto bottom=ladder.placeOnBottom;bottom.SetPose(NAI::CM_STAND);
                if(ladder.pointPassable.size()<2 || !bottom.IsIntegral() || bottom.IsFinal() || !network->IsPassable(bottom) || network->IsLocked(bottom,false))continue;
                NAI::SPathPlace rung(index,0,layer,(4+2*int(ladder.eDir))&7,NAI::CM_STAND,0);rung.SetIntegral(0);if(network->GetPassability(rung)!=NAI::AIP_YES)continue;
                CObj<NAI::CPath> path=new NAI::CPath;path->pNet=network;const bool first=path->AddNewPoint(bottom),second=first && path->AddNewPoint(rung),valid=second && NAI::IsValidPath(*path);
                std::printf("native ladder edge detail: add=%d/%d valid=%d poseLocked=%d actor=%08X bottom=%08X rung=%08X transition=%d points=%u\n",int(first),int(second),int(valid),int(actor->CannotFreelyChangePoses()),unsigned(actor->GetPosition().pos.p.GetBits()),unsigned(bottom.GetBits()),unsigned(rung.GetBits()),int(NAI::GetTransitionType(network,bottom,rung)),unsigned(path->points.size()));
                for(const auto& point:path->points)std::printf("native ladder path point: bits=%08X passability=%d locked=%d doors=%u\n",unsigned(point.GetBits()),int(network->GetPassability(point)),int(network->IsLocked(point)),unsigned(nodes->GetFlippersAt(point).size()));std::fflush(stdout);
                Require(valid,"native authored ladder edge did not build a valid actual movement path");
                std::printf("native animation ladder route candidate: layer=%d index=%d bottom=%08X rung=%08X path=%d points=%u freeze=%d jobs=%d\n",layer,index,unsigned(bottom.GetBits()),unsigned(rung.GetBits()),int(path!=nullptr),path?unsigned(path->points.size()):0,int(nodeLayer->pGroup->bFreeze),int(nodeLayer->pGroup->bHasRecolourJob));std::fflush(stdout);
                if(path && path->points.size()>=2 && std::any_of(path->points.begin(),path->points.end(),[](const NAI::SPathPlace& point){return !point.IsIntegral();})){fixture.path=path;selected.pos=NAI::SPosition(bottom,network);break;}
            }
        }
        std::printf("native animation reseat ladder search: authoredLadders=%d found=%d\n",ladderCount,int(fixture.path!=nullptr));std::fflush(stdout);
        Require(fixture.path,"animation reseat fixture lacks a real intact authored ladder and affordable native route");
    }else if(kind==1) {
        vector<NAI::SPathPlace> starts;network->GetNearPlaces(SSphere(original.GetCP(),8.f),&starts);starts.insert(starts.begin(),original.pos.p);auto* nodes=dynamic_cast<NAI::CPathNetwork*>(network);Require(nodes,"walking control lacks actual nodes");int clearLines=0,validLines=0;
        for(auto start:starts){start.SetPose(NAI::CM_STAND);if(!start.IsIntegral() || !network->IsPassable(start) || network->IsLocked(start) && network->GetWhoLocksThisPlace(start)!=actor)continue;
            for(int direction=0;direction<8 && !fixture.path;++direction){const int dx=NAI::nMoveShift[direction][0],dy=NAI::nMoveShift[direction][1],steps=3;const int x=int(start.GetX())+dx*steps,y=int(start.GetY())+dy*steps;auto* layer=nodes->GetLayer(start.GetLayer());if(x<0 || y<0 || x>=layer->tiles.GetXSize() || y>=layer->tiles.GetYSize())continue;
                bool clear=true;for(int k=1;k<=steps;++k){NAI::SPathPlace cell(int(start.GetX())+k*dx,int(start.GetY())+k*dy,start.GetLayer(),direction,NAI::CM_STAND,0);if(!network->IsPassable(cell) || network->IsLocked(cell))clear=false;}if(!clear)continue;++clearLines;
                start.SetDirection(direction);auto facing=original;facing.pos=NAI::SPosition(start,network);NAI::SPathPlace destination(x,y,start.GetLayer(),direction,NAI::CM_STAND,0);CObj<NAI::CPath> path=new NAI::CPath;path->pNet=network;bool valid=path->AddNewPoint(start);for(int k=1;k<=steps && valid;++k){NAI::SPathPlace cell(int(start.GetX())+k*dx,int(start.GetY())+k*dy,start.GetLayer(),direction,NAI::CM_STAND,0);valid=path->AddNewPoint(cell);}if(!valid || !NAI::IsValidPath(*path)){if(clearLines<=3){std::printf("native straight edge detail: origin=%08X destination=%08X add=%d points=%u poseLocked=%d\n",unsigned(start.GetBits()),unsigned(destination.GetBits()),int(valid),unsigned(path->points.size()),int(actor->CannotFreelyChangePoses()));std::fflush(stdout);}continue;}++validLines;
                if(!path || path->points.size()<3 || !path->actions.empty())continue;bool straight=true;for(const auto& point:path->points)if(!point.IsIntegral() || point.GetLayer()!=start.GetLayer() || (point.GetX()!=start.GetX() || point.GetY()!=start.GetY()) && network->GetClosestDir(start,point)!=direction)straight=false;
                if(straight){fixture.path=path;selected=facing;std::printf("native straight animation route: layer=%u origin=%u,%u destination=%d,%d direction=%d points=%u\n",unsigned(start.GetLayer()),unsigned(start.GetX()),unsigned(start.GetY()),x,y,direction,unsigned(path->points.size()));std::fflush(stdout);}}
            if(fixture.path)break;}
        std::printf("native straight route selection: candidates=%u clear=%d valid=%d found=%d\n",unsigned(starts.size()),clearLines,validLines,int(fixture.path!=nullptr));std::fflush(stdout);
        Require(fixture.path,"animation reseat fixture lacks an actual clear native straight walking route");
    }
    if(kind==2){
        vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(original.GetCP(),8.f),&places);
        for(auto from:places){from.SetPose(NAI::CM_STAND);from.SetDirection(original.GetDir());auto to=from;to.SetDirection((from.GetDirection()+2)&7);
            if(!from.IsIntegral() || from.IsFinal() || !network->IsPassable(from) || !network->IsPassable(to) || network->IsLocked(from) || network->IsLocked(to))continue;
            CObj<NAI::CPath> path=new NAI::CPath;path->pNet=network;if(path->AddNewPoint(from) && path->AddNewPoint(to) && NAI::IsValidPath(*path)){fixture.path=path;selected.pos=NAI::SPosition(from,network);break;}}
        Require(fixture.path,"native turn control has no actual free standing rotation footprint");
    }
    // Preview the actual route before turn/sequence notifications can freeze
    // lazy grid jobs; move the real actor only after all needed spots exist.
    actor->SetPosition(selected);actor->animator.AlignTime();actor->animator.PlaceUnit(selected);
    CObj<NWorld::CCmdPath> command=new NWorld::CCmdPath(NAI::SPosition(fixture.path->points.back(),network));
    world->StartSequence();Require(world->IsSequence(),"animation reseat fixture failed to enter the native ownerless scripted sequence");world->GetTime()->Set(1000);actor->animator.AlignTime();actor->animator.PlaceUnit(actor->GetPosition());
    CObj<NWorld::CCommandExecute> executor=NWorld::CreateSimpleMoveExecutor(actor,fixture.path,NAI::PF_USE_POSEDIR);Require(executor,"native interruption setup could not create its actual mover");RestoreAnimationReseatExecutor(actor,executor,command);
    const auto routeStart=actor->GetPosition();actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));
    {auto order=AnimationReseatOrder(actor);std::printf("native animation route start: kind=%d points=%u exec=%08X state=%d active=%d pending=%d TB=%d AP=%d from=%u,%u to=%u,%u end=%u now=%u\n",kind,fixture.path?unsigned(fixture.path->points.size()):0,order.first?unsigned(pSSClasses->GetObjectTypeID(order.first)):0,order.first?int(order.first->GetState()):-1,int(actor->IsPerformingAction()),int(order.second!=nullptr),int(world->IsTurnBased()),actor->GetAP(),routeStart.pos.p.GetX(),routeStart.pos.p.GetY(),actor->GetPosition().pos.p.GetX(),actor->GetPosition().pos.p.GetY(),unsigned(actor->animator.GetTimeEnd()),unsigned(world->GetTime()->GetValue()));std::fflush(stdout);}
    const int expected=kind==0?0x12352171:kind==1?0x02731142:0x02731143;bool found=false;int turns=0;
    for(int edge=0;edge<96;++edge){auto order=AnimationReseatOrder(actor);if(!order.first || !actor->IsPerformingAction())break;auto mover=AnimationReseatMover(order.first);Require(mover,"animation reseat real route lost its native mover");auto current=ReadMovementCommands(mover).current;
        const int type=current?pSSClasses->GetObjectTypeID(current):0;if(edge<10){std::printf("native animation route edge: edge=%d mover=%08X current=%08X queued=%u state=%d end=%u now=%u\n",edge,unsigned(pSSClasses->GetObjectTypeID(mover)),unsigned(type),unsigned(ReadMovementCommands(mover).queued.size()),int(order.first->GetState()),unsigned(actor->animator.GetTimeEnd()),unsigned(world->GetTime()->GetValue()));std::fflush(stdout);}if(type==0x02731143)++turns;if(type==expected){found=true;break;}world->GetTime()->Set(actor->animator.GetTimeEnd());actor->Segment();}
    std::printf("native animation reseat path setup: kind=%d expected=%08X found=%d turns=%d marker=%d AP=%d\n",kind,unsigned(expected),int(found),turns,int(actor->animator.bStandIfRecalcCommand),actor->GetAP());std::fflush(stdout);
    Require(found && (kind!=1 || turns==0),"animation reseat fixture did not reach its actual ladder/walk/turn command without a fabricated animator flag");return fixture;
}

static void SaveAnimationReseatGraph(CDataStream& stream,AnimationReseatFixture& fixture,const char* producerID="runtime-animation-reseat")
{
    auto executor=AnimationReseatOrder(fixture.actor).first;string producer=producerID;CStructureSaver writer(stream,CStructureSaver::WRITE);
    writer.Add(1,&fixture.game);writer.Add(2,&fixture.world);writer.Add(3,&fixture.actor);writer.Add(4,&fixture.path);writer.Add(5,&executor);writer.Add(6,&producer);
}

static AnimationReseatFixture LoadAnimationReseatGraph(CDataStream& stream,const char* expectedProducer="runtime-animation-reseat")
{
    AnimationReseatFixture fixture;CObj<NWorld::CCommandExecute> executor;string producer;{CStructureSaver reader(stream,CStructureSaver::READ);reader.Add(1,&fixture.game);reader.Add(2,&fixture.world);reader.Add(3,&fixture.actor);reader.Add(4,&fixture.path);reader.Add(5,&executor);reader.Add(6,&producer);}
    Require(fixture.game && fixture.world && fixture.actor && fixture.path && executor && producer==expectedProducer && AnimationReseatOrder(fixture.actor).first==executor,"animation reseat save lost the actual world/actor/path/executor graph or producer");fixture.world->RestoreRuntimeCaches(fixture.game);return fixture;
}

static void WriteLegacyAnimationWalk(const char* databasePath,const char* resourcePath,const char* output)
{
    auto fixture=MakeAnimationReseatFixture(databasePath,resourcePath,1);Require(!fixture.actor->animator.bStandIfRecalcCommand,"historical animation walk fixture must be generated by the actual old straight-walk implementation");
    CFileStream stream;stream.OpenWrite(output);SaveAnimationReseatGraph(stream,fixture,"8a5e6cdff2402cc9e6a40cd9d12807df617b21dd");std::printf("native legacy animation walk: bytes=%d producer=8a5e6cdf marker=0 actualWalking=1\n",stream.GetSize());std::fflush(stdout);
    auto order=AnimationReseatOrder(fixture.actor);auto mover=AnimationReseatMover(order.first);dynamic_cast<NWorld::IExecMove*>(mover.GetPtr())->FullCancel();fixture.actor->Do(new NWorld::CCmdCancel(fixture.actor));
}

static void AnimationReseat(const char* databasePath,const char* resourcePath,int mode,const char* legacyPath=nullptr)
{
    const bool bodyPolicy=mode>=8 && mode<14,legacy=mode>=14,stop=mode%2!=0,ladder=mode<4,moveMode=!bodyPolicy;
    AnimationReseatFixture fixture;NativeCannonFixture cannonFixture;
    if(bodyPolicy){cannonFixture=MakeNativeCannonFixture(databasePath,resourcePath);fixture.game=cannonFixture.game;fixture.world=cannonFixture.world;fixture.actor=cannonFixture.actor.GetPtr();auto* actor=fixture.actor.GetPtr();auto position=actor->GetPosition(),turned=position;turned.pos.p.SetDirection((position.pos.p.GetDirection()+1)&7);
        actor->animator.Rotate(position,turned,true);actor->animator.EndRotate(turned);actor->SetPosition(turned);
        if(mode==10 || mode==11){CRay ray;ray.ptOrigin=turned.GetEyePosition();ray.ptDir=CVec3(cos(turned.GetDirection()),sin(turned.GetDirection()),0);actor->animator.Snipe(turned,ray);Require(actor->animator.IsAiming(),"native body aim control did not enter its real aiming graph");}
        if(mode==12 || mode==13){NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCmdCannon> request=new NWorld::CCmdCannon(cannonFixture.cannon);CObj<NWorld::CCommandExecute> enter=NWorld::CreateExecutor(actor,request,&result);Require(enter && result==NWorld::UCR_OK,"body policy control cannot enter its actual cannon");enter->Run();CompleteCannonPoseQueue(enter,cannonFixture);enter=nullptr;Require(actor->animator.GetCannon()==cannonFixture.cannon,"body policy control did not finish native mount");}
    }else if(legacy){Require(legacyPath,"legacy animation walk requires its actual historical save");CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);CFileStream snapshot;snapshot.OpenRead(legacyPath);fixture=LoadAnimationReseatGraph(snapshot,"8a5e6cdff2402cc9e6a40cd9d12807df617b21dd");Require(!fixture.actor->animator.bStandIfRecalcCommand,"actual historical straight-walk save no longer contains the original false marker");}
    else{fixture=MakeAnimationReseatFixture(databasePath,resourcePath,ladder?0:mode<6?1:2);if(mode==2 || mode==3){CMemoryStream snapshot;SaveAnimationReseatGraph(snapshot,fixture);snapshot.Seek(0);fixture=LoadAnimationReseatGraph(snapshot);}}
    auto* actor=fixture.actor.GetPtr();auto* world=fixture.world.GetPtr();auto before=AnimationReseatOrder(actor);CObj<NWorld::CCommandExecute> mover;if(moveMode)mover=AnimationReseatMover(before.first).GetPtr();
    if(moveMode)Require(before.first && mover && before.first->IsExecuting(),"native animation reseat event lacks its actual installed live movement executor");
    auto* body=actor->animator.GetSkeletonAnimator();const auto clips=NativeCannonAnimationClips(body);vector<CObj<NAnimation::CAnimation>> witnesses;for(auto* clip:clips)witnesses.push_back(clip);
    const STime end=actor->animator.GetTimeEnd();const bool marker=actor->animator.bStandIfRecalcCommand,aimed=actor->animator.IsAiming();auto* cannon=actor->animator.GetCannon();
    const auto position=actor->GetPosition();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();auto* item=actor->GetUnitRPG()->GetInventory()->GetActive();auto* weapon=dynamic_cast<NRPG::CWeaponItem*>(item);const int ammo=weapon?weapon->GetAmmoQuantity():0;auto random=GlobalGameRandom();
    actor->OnTBSEvent(stop?NWorld::TBS_STOP_MOVE_AND_CANCEL_ACTION:NWorld::TBS_RECALC_COMMAND);auto after=AnimationReseatOrder(actor);const auto current=NativeCannonAnimationClips(body);int added=0;for(auto* clip:current)if(std::find(clips.begin(),clips.end(),clip)==clips.end())++added;
    auto afterRandom=GlobalGameRandom();std::printf("native animation reseat: mode=%d ladder=%d legacy=%d stop=%d beforeMarker=%d afterMarker=%d newClips=%d beforeEnd=%u afterEnd=%u retainedExec=%d pendingCommand=%d aimed=%d->%d cannon=%d->%d AP=%d->%d HP=%d->%d ammo=%d->%d\n",mode,int(ladder),int(legacy),int(stop),int(marker),int(actor->animator.bStandIfRecalcCommand),added,unsigned(end),unsigned(actor->animator.GetTimeEnd()),int(after.first==before.first),int(after.second!=nullptr),int(aimed),int(actor->animator.IsAiming()),int(cannon!=nullptr),int(actor->animator.GetCannon()!=nullptr),ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP(),ammo,weapon?weapon->GetAmmoQuantity():0);std::fflush(stdout);
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && actor->GetUnitRPG()->GetInventory()->GetActive()==item && (weapon?weapon->GetAmmoQuantity():0)==ammo && random.Get()==afterRandom.Get(),"native interruption animation policy changed AP/HP/position, equipment/ammunition or game RNG");
    if(moveMode){const bool pending=stop && actor->IsAIUnit() ? !after.second : after.second==before.second;Require((stop?after.first==before.first:!after.first) && pending,"animation reseat changed safe stop ownership, human pending route or AI cleanup policy");}
    if(ladder || bodyPolicy)Require(added==0 && actor->animator.GetTimeEnd()==end && actor->animator.IsAiming()==aimed && actor->animator.GetCannon()==cannon,"command interruption reseated a native ladder/settled/aiming/mounted animation that must retain its own clip");
    else Require(added>0 && !actor->animator.IsAiming(),"command interruption did not reseat a real walking/turning actor, including an actual old-fork save");
    if(moveMode){dynamic_cast<NWorld::IExecMove*>(mover.GetPtr())->FullCancel();actor->Do(new NWorld::CCmdCancel(actor));mover=nullptr;before.first=nullptr;after.first=nullptr;}
    if(cannon){CObj<NWorld::CExecCannon> exit=new NWorld::CExecCannon(actor,cannon,false);exit->Run();world->GetTime()->Set(actor->animator.GetTimeEnd());exit->AnimationFinished();exit=nullptr;}
    Require(!world->IsAction(),"native animation reseat fixture retained a finished movement/cannon action");
}

static void CannonExitIncapacitated(const char* databasePath,const char* resourcePath,int mode)
{
    auto fixture=MakeNativeCannonFixture(databasePath,resourcePath);auto* actor=fixture.actor.GetPtr();auto* cannon=fixture.cannon.GetPtr();
    CObj<NWorld::CCmdCannon> command=new NWorld::CCmdCannon(cannon);
    CObj<NWorld::CExecCannon> enter=new NWorld::CExecCannon(actor,cannon,true,command);
    Require(enter->CanDoIt(actor->GetPosition())==NWorld::UCR_OK,"cannon exit fixture cannot mount its actual cannon");
    enter->Run();fixture.world->GetTime()->Set(actor->animator.GetTimeEnd());enter->AnimationFinished();enter=nullptr;
    Require(actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN && cannon->GetCurrentUnit()==actor && actor->animator.GetCannon()==cannon,
        "cannon exit fixture did not finish its actual native mounted entry");
    CObj<NWorld::CExecCannon> exit=new NWorld::CExecCannon(actor,cannon,false);
    // Exercise the real RPG-to-world seam: the damage result is committed
    // before CDumbUnitServer::SyncConscious consumes it, while exit is pending.
    for(int hit=0;hit<mode;++hit) {
        CRay ray;ray.ptOrigin=actor->GetPosition().GetCP();ray.ptDir=CVec3(0,1,0);
        NRPG::CAttackPortion attack(0,0,0.f,0,0,0);attack.MakeClickOfDeath(ray);
        const int damage=actor->GetUnitRPG()->ProcessAttack(fixture.world.GetPtr(),NAI::HL_BODY,&attack,actor->GetUnitRPG()->GetRPGArmor()).nDmg;
        Require(damage>=0,"native RPG cannon incapacitation control did not penetrate its actual actor armor");
    }
    const bool dead=actor->GetUnitRPG()->IsDead(),unconscious=actor->GetUnitRPG()->IsUnconscious();
    std::printf("native cannon exit input: mode=%d RPGdead=%d RPGunconscious=%d worldMounted=%d\n",mode,int(dead),int(unconscious),int(actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN));std::fflush(stdout);
    Require(dead==(mode==2) && unconscious==(mode==1) && actor->GetState()==NWorld::CUnit::ST_MACHINE_GUN,
        "cannon exit fixture did not reach its real unsynchronized RPG damage result with mounted world state");
    auto* animator=actor->animator.GetSkeletonAnimator();CStructureNetworkContext refs;auto fields=BorrowedObjectFields(animator,&refs);
    CObj<NAnimation::CASequence> sequence;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&sequence);}
    Require(sequence,"cannon exit fixture lacks its actual body animation sequence");
    const STime now=fixture.world->GetTime()->GetValue(),beforeEnd=actor->animator.GetTimeEnd();
    CObj<NAnimation::CAnimator> scheduled=sequence->GetAnimator(now);Require(scheduled,"cannon exit fixture lacks its active mounted animation");
    const int ap=actor->GetAP(),vp=actor->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();
    Require(exit->CanDoIt(position)==NWorld::UCR_OK,"native queued exit rejected its actual already-mounted cannon");
    exit->Run();const bool sameAnimation=sequence->GetAnimator(now)==scheduled.GetPtr();const STime afterEnd=actor->animator.GetTimeEnd();
    std::printf("native cannon exit: mode=%d sameAnimation=%d extraDelay=%u retainedAnimatorCannon=%d AP=%d->%d\n",mode,int(sameAnimation),
        unsigned(afterEnd-beforeEnd),int(actor->animator.GetCannon()==cannon),ap,actor->GetAP());std::fflush(stdout);
    Require(!actor->GetUnitRPG()->GetCannonItem() && !cannon->IsOccupied() && actor->GetState()!=NWorld::CUnit::ST_MACHINE_GUN &&
        actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==vp && actor->GetPosition()==position &&
        actor->GetUnitRPG()->IsDead()==dead && actor->GetUnitRPG()->IsUnconscious()==unconscious,
        "native cannon exit failed logical detachment or changed its real RPG damage state, AP, VP or position");
    if(mode==0) Require(!sameAnimation && afterEnd>beforeEnd && !actor->animator.GetCannon(),"a living cannon exit lost its normal standing transition");
    else Require(sameAnimation && afterEnd==beforeEnd && actor->animator.GetCannon()==cannon,
        "incapacitated cannon exit inserted a living stand transition and lost the mounted context needed by native death animation");
    exit->AnimationFinished();exit=nullptr;
    Require(!fixture.world->IsAction(),"native cannon exit retained its finished action before consciousness synchronization");
    if(mode) {
        actor->SyncConscious();
        auto* particle=dynamic_cast<NAnimation::CParticleSkeleton*>(sequence->GetAnimator(now));
        Require(particle && particle->tActive==particle->tMax && !actor->animator.GetCannon() &&
            (mode==2?actor->IsDead():actor->IsUnconscious()),
            "native consciousness synchronization did not hand the mounted downed actor directly to its actual clipless ragdoll");
        CStructureNetworkContext particleRefs;auto particleFields=BorrowedObjectFields(particle,&particleRefs);CPtr<NWorld::CUnitServer> owner;
        {CStructureSaver reader(particleFields,CStructureSaver::READ,&particleRefs);reader.Add(11,&owner);}
        Require(owner==actor,"native cannon ragdoll lost its real unit owner");
        std::printf("native cannon downed sync: mode=%d particleOwner=1 active=%u max=%u\n",mode,unsigned(particle->tActive),unsigned(particle->tMax));
    }
}

static void MountedReloadWeapon(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    CPtr<NRPG::CWeaponItem> personal;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!server || !server->CanFight() || server->IsEmptyPK() || server->IsWearingPK()) continue;
        auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(server->GetUnitRPG()->GetInventory()->GetActive());
        if (!firearm || firearm->GetAmmoQuantity() <= 0) continue;
        firearm->DiscardAmmo();
        if (firearm->CanReload(server->GetUnitRPG()->GetInventory())) { actor = server; personal = firearm; break; }
    }
    Require(actor && IsValid(personal), "mounted-reload fixture has no firearm with spare ammunition");
    CObj<NRPG::CWeaponItem> mounted = new NRPG::CWeaponItem(personal->GetDBWeapon());
    mounted->DiscardAmmo();
    Require(mounted->CanReload(actor->GetUnitRPG()->GetInventory()), "mounted-reload fixture has no compatible spare for its cannon");
    actor->GetUnitRPG()->SetCannonItem(mounted);
    Require(actor->GetUnitRPG()->GetWeaponItem() == mounted.GetPtr(), "mounted-reload fixture did not install its effective weapon");
    const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    auto selectedWeapon = [&](int slot) {
        CObj<NWorld::CCmdReload> command = new NWorld::CCmdReload(slot);
        NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
        CObj<NWorld::CCommandExecute> queue = NWorld::CreateExecutor(actor, command, &result);
        Require(IsValid(queue) && result == NWorld::UCR_OK, "mounted-reload fixture could not create a reload executor");
        auto* reload = FindNativeReload(queue);
        CStructureNetworkContext references;
        references.externalReferences = true;
        references.ids[actor] = 1; references.refs[1] = actor;
        references.ids[personal.GetPtr()] = 2; references.refs[2] = personal.GetPtr();
        references.ids[mounted.GetPtr()] = 3; references.refs[3] = mounted.GetPtr();
        references.nextID = 100; references.include = [](CObjectBase*) { return false; };
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
          static_cast<CObjectBase*>(reload)->operator&(writer); }
        saved.Seek(0);
        struct LoadState {
            CPtr<NRPG::IWeaponItemInfo> weapon;
            int operator&(CStructureSaver& saver) { saver.Add(3, &weapon); return 0; }
        } load;
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(1, &load); }
        return load.weapon;
    };
    Require(selectedWeapon(-1) == mounted.GetPtr(), "default reload selected the personal firearm instead of the effective mounted weapon");
    Require(selectedWeapon(actor->GetUnitRPG()->GetInventory()->GetActiveSlot()) == personal.GetPtr(),
        "explicit inventory-slot reload stopped selecting the requested personal item");
    actor->GetUnitRPG()->SetCannonItem(nullptr);
    Require(selectedWeapon(-1) == personal.GetPtr(), "leaving the cannon did not restore personal weapon reload selection");
    Require(personal->GetAmmoQuantity() == 0 && mounted->GetAmmoQuantity() == 0
            && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "constructing reload executors spent ammunition or action points");
}

static void ReloadActionWire(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    CPtr<NRPG::CWeaponItem> weapon;
    for (auto& entry : units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!server || !server->CanFight() || server->IsEmptyPK() || server->IsWearingPK()) continue;
        auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(server->GetUnitRPG()->GetInventory()->GetActive());
        if (!firearm || firearm->GetAmmoQuantity() <= 0) continue;
        firearm->DiscardAmmo();
        if (firearm->CanReload(server->GetUnitRPG()->GetInventory())
                && server->CanSpendAP(server->GetActionAP(NRPG::AC_RELOAD))) { actor = server; weapon = firearm; break; }
    }
    Require(actor && IsValid(weapon), "reload-wire fixture needs a firearm user with spare ammunition and AP");
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[actor] = 1; references.refs[1] = actor;
    references.ids[weapon.GetPtr()] = 2; references.refs[2] = weapon.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    auto registerItem = [&](NRPG::IInventoryItem* item) {
        if (item && !references.ids.count(item)) {
            const auto id = references.nextID++;
            references.ids[item] = id; references.refs[id] = item;
        }
    };
    auto* sourceInventory = actor->GetUnitRPG()->GetInventory();
    for (int i = 0; i < NDb::N_SLOTS; ++i) registerItem(sourceInventory->Get(NDb::ESlot(i)));
    for (const auto& item : sourceInventory->GetItems()) registerItem(item.pItem);
    auto writeReload = [&](int slot) {
        CObj<NWorld::CExecReload> action = new NWorld::CExecReload(actor, slot);
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
          static_cast<CObjectBase*>(action.GetPtr())->operator&(writer); }
        saved.Seek(0);
        return saved;
    };
    auto saved = writeReload(-1);
    int slot = -7;
    { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(2, &slot); }
    Require(slot == -1, "reload action wrote a weapon reference in the retail slot-number field");
    struct CommandState {
        CPtr<NWorld::CUnitServer> actor;
        int operator&(CStructureSaver& saver) { saver.Add(4, &actor); return 0; }
    };
    struct LoadState {
        CommandState command;
        NWorld::SItem clip;
        CPtr<NRPG::IWeaponItemInfo> weapon;
        int operator&(CStructureSaver& saver) {
            saver.Add(1, &command); saver.Add(2, &clip); saver.Add(3, &weapon); return 0;
        }
    } load;
    bool sameColor = false;
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      reader.Add(1, &load); reader.Add(3, &sameColor); }
    Require(load.command.actor == actor && load.weapon == weapon.GetPtr(),
        "reload action omitted its nested load-weapon/command ownership state");
    auto* spare = dynamic_cast<NRPG::CClipItem*>(load.clip.pItem.GetPtr());
    auto* inner = dynamic_cast<NRPG::CClipItem*>(weapon->GetInnerClip());
    Require(load.clip.pUnit == actor, "reload action lost its magazine's unit owner");
    Require(spare && inner, "reload-wire fixture could not resolve its external magazine references");
    Require(load.clip.sPosition == CTPoint<int>(0, 0)
            && (load.clip.eType == NWorld::SItem::SLOT || load.clip.eType == NWorld::SItem::BACKPACK)
            && inner->IsCompatible(spare, sameColor),
        "reload action did not save its selected native spare magazine and inventory source");
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    if (load.clip.eType == NWorld::SItem::SLOT)
        Require(inventory->Get(NDb::ESlot(load.clip.nSlot)) == spare, "reload saved the wrong spare slot");
    else {
        const auto& items = inventory->GetItems();
        Require(std::any_of(items.begin(), items.end(), [&](const auto& item) { return item.pItem == spare; }),
            "reload saved a spare that is absent from its backpack");
    }
    auto explicitSaved = writeReload(inventory->GetActiveSlot());
    { CStructureSaver reader(explicitSaved, CStructureSaver::READ, &references); reader.Add(2, &slot); }
    Require(slot == inventory->GetActiveSlot(), "explicit reload lost its slot number on save");
    const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    const int rounds = spare->GetQuantity();
    CObj<NWorld::CExecReload> restored = new NWorld::CExecReload;
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
    Require(restored->CanDoIt() == NWorld::UCR_OK && weapon->GetAmmoQuantity() == 0
            && spare->GetQuantity() == rounds && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "restoring reload mutated ammunition/AP or lost execution eligibility");
    const int cost = actor->GetActionAP(NRPG::AC_RELOAD);
    Require(restored->GetStartAP() == cost && restored->GetActionAP() == cost,
        "restoring reload changed the native action cost");
    restored->Run(); restored->TimeLabelReached(); restored->AnimationFinished();
    Require(weapon->GetAmmoQuantity() > 0 && restored->GetState() == NWorld::CCommandExecute::FINISHED,
        "saved reload could not finish its actual ammunition-loading action");
    restored = nullptr;
    Require(!world->IsAction(), "restored reload retained an action counter after completion");
}

class SharedShortcutMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(SharedShortcutMission);
public:
    CObj<NGame::IState> selectedState;
    void Prepare() {
        pCursor = NUI::ICursor::Create(false);
        pInterface = new NUI::CInterface(pCursor);
        desktopWindowsList.push_back(new NUI::CDesktopWindow(NUI::SWindowInfo(
            pInterface,NUI::SPoint(0,0),NUI::SPoint(100,100),"shortcut-probe")));
    }
    bool IsReady() const override { return false; }
    bool IsSequence() const override { return true; }
    bool IsPlayerTurn() const override { return true; }
    bool IsRealTime() const override { return true; }
    bool CommandState(NGame::IState* state) override { selectedState = state; return true; }
    void CacheActions(const vector<NGame::SActionInfo>& actions) {
        auto copy = actions;
        CMemoryStream fields;
        { CStructureSaver writer(fields,CStructureSaver::WRITE); writer.Add(16,&copy); }
        fields.Seek(0);
        { CStructureSaver reader(fields,CStructureSaver::READ); CMission::operator&(reader); }
    }
};

static NInput::SEvent SharedActionEvent(std::initializer_list<const char*> commands)
{
    NInput::SEvent event, part;
    while (NInput::GetEvent(&part)) {}
    for (const char* command: commands) {
        NInput::PostEvent(command);
        Require(NInput::GetEvent(&part) && !part.commands.empty(),
            "shared-key fixture could not obtain its native binding command");
        event.mMessage = part.mMessage;
        event.commands.insert(event.commands.end(),part.commands.begin(),part.commands.end());
    }
    return event;
}

static void UnavailableAttackShortcut()
{
    CObj<SharedShortcutMission> mission = new SharedShortcutMission;
    mission->Prepare();
    vector<NGame::SActionInfo> actions(NGame::UA_MAXVALUE);
    for (auto& action: actions) action.bValid = true;
    actions[NGame::UA_MINE].bAvailable = true;
    actions[NGame::UA_ATTACK].bAvailable = true;
    // Mode entry remains available even when a target or AP is still needed.
    Require(!actions[NGame::UA_ATTACK].bOk, "shortcut control unexpectedly permits immediate execution");
    mission->CacheActions(actions);
    auto event = SharedActionEvent({"attack","setmine"});
    mission->ProcessEvent(event);
    Require(dynamic_cast<NGame::CStateAttack*>(mission->selectedState.GetPtr()),
        "an available attack with no immediate execution target lost its shared-key priority");
    actions[NGame::UA_ATTACK].bAvailable = false;
    mission->CacheActions(actions);
    mission->selectedState = nullptr;
    mission->ProcessEvent(event);
    Require(dynamic_cast<NGame::CStateSetMine*>(mission->selectedState.GetPtr()),
        "an unavailable attack swallowed the shared shortcut instead of falling through to set-mine");
    Require(!NMainLoop::HaveInterfaceCommand(), "selecting an action mode unexpectedly queued a game command");
}

static void SharedActionShortcutPriority()
{
    CObj<SharedShortcutMission> mission = new SharedShortcutMission;
    mission->Prepare();
    vector<NGame::SActionInfo> actions(NGame::UA_MAXVALUE);
    for (auto& action: actions) action.bValid = true;
    const NGame::EUnitAction priority[] = {NGame::UA_ATTACK,NGame::UA_HEAL,NGame::UA_USETOOL,
        NGame::UA_DROPCORPSE,NGame::UA_MINE};
    for (auto action: priority) actions[action].bAvailable = true;
    auto event = SharedActionEvent({"attack","firstaid","usetool","dropcorpse","setmine"});
    auto send = [&]() {
        mission->CacheActions(actions);
        mission->selectedState = nullptr;
        mission->ProcessEvent(event);
    };
    send();
    Require(dynamic_cast<NGame::CStateAttack*>(mission->selectedState.GetPtr()),
        "shared action key lost its available attack priority");
    actions[NGame::UA_ATTACK].bAvailable = false;
    send();
    Require(dynamic_cast<NGame::CStateFirstAid*>(mission->selectedState.GetPtr()),
        "shared action key selected set-mine ahead of available first aid");
    actions[NGame::UA_HEAL].bAvailable = false;
    send();
    Require(dynamic_cast<NGame::CStateUntrap*>(mission->selectedState.GetPtr()) &&
        mission->selectedState->GetType() == NGame::IState::FORCED,
        "shared action key failed to choose the available forced use-tool mode");
    actions[NGame::UA_USETOOL].bAvailable = false;
    send();
    Require(dynamic_cast<NGame::CStateDropCorpse*>(mission->selectedState.GetPtr()),
        "shared action key failed to choose available drop-corpse after unavailable aid and tools");
    actions[NGame::UA_DROPCORPSE].bAvailable = false;
    send();
    Require(dynamic_cast<NGame::CStateSetMine*>(mission->selectedState.GetPtr()),
        "shared action key failed to reach the available set-mine fallback");
    for (const char* command: {"firstaid","usetool","dropcorpse"}) {
        mission->selectedState = nullptr;
        mission->ProcessEvent(SharedActionEvent({command}));
        Require(!IsValid(mission->selectedState),
            "an individually unavailable aid, tool or body shortcut still selected its mode");
    }
    Require(!NMainLoop::HaveInterfaceCommand(), "shared-key mode selection queued a game command");
}

class ShortcutVisibleActor: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(ShortcutVisibleActor);
public:
    // The resource-free routing fixture has no RPG. Native group tests below
    // use actual squad members and their real hiding state.
    bool IsHiding() const override { return false; }
};

class HideShortcutMission: public SharedShortcutMission
{
    OBJECT_NOCOPY_METHODS(HideShortcutMission);
public:
    vector<CPtr<NGame::IUnitTracker>> selection;
    vector<NGame::EUnitAction> queries;
    vector<CObj<NWorld::CCmd>> commands;
    vector<CPtr<NWorld::CUnit>> recipients;
    void GetSelectedUnits(vector<CPtr<NGame::IUnitTracker>>* result) const override { *result = selection; }
    void GetActionInfo(NGame::EUnitAction action,NGame::SActionInfo* info) override {
        queries.push_back(action);
        CMission::GetActionInfo(action,info);
    }
    void Command(NWorld::CUnit* unit,NWorld::CCmd* command,bool instantly = true) override {
        Require(instantly, "Hide shortcut lost its immediate-start request");
        recipients.push_back(unit); commands.emplace_back(command);
    }
};

static void HideShortcutEligibility()
{
    CObj<HideShortcutMission> mission = new HideShortcutMission;
    mission->Prepare();
    CObj<NWorld::CUnitServer> actor = new ShortcutVisibleActor;
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission,actor);
    mission->selection.push_back(tracker.GetPtr());
    const auto event = SharedActionEvent({"hide"});
    auto send = [&](bool hideAllowed,bool strafeAllowed) {
        vector<NGame::SActionInfo> actions(NGame::UA_MAXVALUE);
        for (auto& action: actions) { action.bValid = true; action.eResult = NWorld::UCR_UNAVAILABLE; }
        for (auto action: {NGame::UA_HIDE,NGame::UA_STRAFE}) {
            const bool allowed = action == NGame::UA_HIDE ? hideAllowed : strafeAllowed;
            actions[action].bAvailable = actions[action].bOk = allowed;
            actions[action].eResult = allowed ? NWorld::UCR_OK : NWorld::UCR_UNAVAILABLE;
        }
        mission->CacheActions(actions);
        mission->queries.clear(); mission->commands.clear(); mission->recipients.clear();
        mission->ProcessEvent(event);
        std::printf("Hide hotkey: hide=%d strafe=%d commands=%zu queried=%d\n",
            int(hideAllowed),int(strafeAllowed),mission->commands.size(),
            mission->queries.empty() ? -1 : int(mission->queries.back()));
        Require(mission->commands.size() == size_t(hideAllowed),
            hideAllowed ? "available Hide shortcut was blocked by unavailable Strafe"
                        : "unavailable Hide shortcut issued a command because Strafe was available");
        if (hideAllowed) {
            Require(dynamic_cast<NWorld::CCmdHide*>(mission->commands.front().GetPtr()) &&
                mission->recipients.front() == actor.GetPtr(),
                "Hide shortcut changed its command or selected recipient");
        }
    };
    // Establish both original agreeing-action controls before the divergent case.
    send(false,false); send(true,true);
    send(true,false); send(false,true);
    Require(mission->queries.size() == 1 && mission->queries.front() == NGame::UA_HIDE,
        "Hide shortcut did not ask its own cached action availability");
    mission->selection.clear();
    mission->commands.clear(); mission->recipients.clear();
    mission->ProcessEvent(event);
    Require(mission->commands.empty(), "Hide shortcut issued a command without a selection");
    Require(!NMainLoop::HaveInterfaceCommand(), "Hide shortcut queued an unrelated interface command");
}

class GroupHideMission: public HideShortcutMission
{
    OBJECT_NOCOPY_METHODS(GroupHideMission);
public:
    CObj<NWorld::CCmd> preview;
    bool allowed = true;
    void Refresh() { UpdateActionsInfo(); preview = nullptr; }
    void CanDoCommand(NWorld::CCmd* command,bool noTarget,NGame::SActionInfo* info) override {
        Require(noTarget, "Hide action preview unexpectedly requested a target");
        preview = command;
        *info = NGame::SActionInfo();
        if (allowed) info->SetValid();
        else { info->bValid = true; info->eResult = NWorld::UCR_UNAVAILABLE; }
    }
};

static void GroupHideShortcut(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* first = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) { first = unit; break; }
    }
    Require(first != nullptr, "group Hide fixture has no ordinary human actor");
    CObj<NWorld::CUnitServer> second;
    auto* network = world->GetPathNetwork(); vector<NAI::SPathPlace> places;
    network->GetNearPlaces(SSphere(first->GetPosition().GetCP(),10.f),&places);
    for (auto place: places) {
        place.SetPose(NAI::CM_STAND); place.SetDirection(0);
        if (!network->IsPassable(place) || network->IsLocked(place,true)) continue;
        second = world->AddUnitInGame(place,NRPG::CreateUnit(first->GetUnitRPG()->GetRPGUnit()->GetPers()),
            dynamic_cast<NWorld::CPlayer*>(first->GetPlayer()));
        if (second) break;
    }
    Require(second && second->CanFight() && second->GetPlayer() == first->GetPlayer(),
        "group Hide fixture could not deploy a second native squad member");
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(first->GetPlayer()));
    Require(!world->IsRealTime() && !world->IsSequence(), "group Hide fixture did not enter its actual player turn");
    CObj<GroupHideMission> mission = new GroupHideMission; mission->Prepare();
    mission->CacheActions(vector<NGame::SActionInfo>(NGame::UA_MAXVALUE));
    vector<CObj<NGame::CUnitTracker>> trackers{
        new NGame::CUnitTracker(mission,first), new NGame::CUnitTracker(mission,second)};
    NWorld::CUnitServer* actors[] = {first,second.GetPtr()};
    for (bool reverse: {false,true})
        for (bool a: {false,true})
            for (bool b: {false,true}) {
                bool initial[] = {a,b};
                const bool target = !(a && b);
                int ap[2];
                for (int i=0;i<2;++i) {
                    actors[i]->EnableHide(); actors[i]->Hide(initial[i],false); actors[i]->EnableHide();
                    ap[i] = actors[i]->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
                }
                mission->selection = {trackers[reverse ? 1 : 0].GetPtr(),trackers[reverse ? 0 : 1].GetPtr()};
                mission->commands.clear(); mission->recipients.clear(); mission->Refresh();
                mission->ProcessEvent(SharedActionEvent({"hide"}));
                std::printf("group Hide initial=%d/%d reverse=%d requests=%zu target=%d\n",
                    int(a),int(b),int(reverse),mission->commands.size(),int(target));
                Require(mission->commands.size() == 2, "Hide shortcut did not issue a command for every selected squad member");
                auto* preview = dynamic_cast<NWorld::CCmdHide*>(mission->preview.GetPtr());
                Require(preview && preview->bExplicitState && preview->bState == target,
                    "group Hide availability preview used a different state than its actual requests");
                for (int i=0;i<2;++i) {
                    int index = reverse ? 1-i : i;
                    auto* command = dynamic_cast<NWorld::CCmdHide*>(mission->commands[i].GetPtr());
                    Require(command && command->bExplicitState && command->bState == target &&
                        mission->recipients[i] == actors[index],
                        "group Hide did not use one common explicit state in selection order");
                    Require(actors[index]->IsHiding() == initial[index] &&
                        actors[index]->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap[index],
                        "group Hide input changed a member before command consumption");
                    NWorld::EUnitCommandResult result;
                    CObj<NWorld::CCommandExecute> executor = NWorld::CreateExecutor(actors[index],command,&result);
                    Require(executor && result == NWorld::UCR_OK, "group Hide native execution was rejected");
                    const int cost = !initial[index] && target ? actors[index]->GetActionAP(NRPG::AC_HIDE) : 0;
                    executor->Run();
                    Require(actors[index]->IsHiding() == target &&
                        actors[index]->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap[index]-cost,
                        "mixed squad did not converge to one hiding state at the original AP cost");
                }
                Require(!world->IsAction(), "group Hide retained an action counter");
            }
    mission->commands.clear(); mission->recipients.clear(); mission->allowed = false; mission->Refresh();
    mission->ProcessEvent(SharedActionEvent({"hide"}));
    Require(mission->commands.empty(), "unavailable group Hide issued squad commands");
    mission->selection.clear(); mission->allowed = true; mission->Refresh();
    mission->ProcessEvent(SharedActionEvent({"hide"}));
    Require(mission->commands.empty() && !NMainLoop::HaveInterfaceCommand(),
        "empty group Hide issued a unit or interface command");
}
