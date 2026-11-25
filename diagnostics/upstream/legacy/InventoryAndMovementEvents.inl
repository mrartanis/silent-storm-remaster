class EmptyCorpseNetwork: public NAI::CPathNetwork
{
    OBJECT_BASIC_METHODS(EmptyCorpseNetwork);
public:
    int queries = 0;
    void GetNearPlaces(const SSphere&, vector<NAI::SPathPlace>* places, bool) override {
        Require(++queries <= 16, "corpse snap exceeded two bounded navigation searches");
        places->clear();
    }
};

static void FailedCorpseSnap(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) {
            actor = unit; break;
        }
    }
    Require(actor != nullptr, "corpse-snap fixture contains no ordinary actor");
    CObj<EmptyCorpseNetwork> network = new EmptyCorpseNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    NAI::SUnitPosition logical = actor->GetPosition();
    logical.pos = NAI::SPosition(NAI::SPathPlace(3, 3, 0), network);
    logical.pos.p.SetPose(NAI::CM_STAND);
    network->GetLayer(0)->tiles[3][3].nFloor = 0;
    actor->GetUnitRPG()->GetRPGUnit()->Kill();
    actor->SetState(new NWorld::CUnitStateDeath(actor));
    actor->SetTemporaryPosition(logical);
    Require(!actor->CanFight() && actor->GetPosition().IsValid(), "corpse fixture did not retain a valid logical tile");
    // Empty physical-search results are transient: retain the prior valid tile
    // and retry on the next segment rather than caching the failed body snap.
    actor->CDumbUnitServer::Segment();
    Require(actor->GetPosition() == logical, "failed corpse snap replaced the last valid logical position");
    Require(network->queries == 8 && actor->GetCorpseHLs()->size() == 6,
        "corpse segment did not refresh hit locations after its bounded search");
    actor->CDumbUnitServer::Segment();
    Require(actor->GetPosition() == logical && network->queries == 16,
        "failed corpse snap was cached instead of being retried on the next segment");
}


static void MovementCriticalProbe(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK()) { actor = unit; break; }
    }
    Require(actor != nullptr, "movement-critical fixture has no human actor");
    actor->SetState(new NWorld::CUnitStateNormal(actor));
    CObj<NWorld::CCmdPath> command = new NWorld::CCmdPath(actor->GetPosition().pos);
    const auto wishPose = actor->GetWishPose();
    Require(actor->CanDo(command) == NWorld::UCR_OK, "movement-critical fixture has no baseline path");
    for (auto injury : {NDb::C_PATIENT, NDb::C_MOTIONLESS}) {
        actor->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_LEGS, injury, 3, 2.0f));
        Require(actor->GetUnitRPG()->HasCritical(injury), "fixture did not apply the movement-banning critical");
        const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
        int startAP = 0;
        Require(actor->CanDo(command, &startAP) == NWorld::UCR_CRITICALS_BAN,
            "normal movement eligibility did not reject the actual injury");
        Require(actor->CanDo(command) == NWorld::UCR_CRITICALS_BAN,
            "fast movement preview discarded the existing critical-injury rejection");
        Require(actor->GetWishPose() == wishPose && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
            "movement eligibility probes changed stance or action points");
        Require(actor->GetUnitRPG()->RemoveCritical(injury), "fixture injury could not be removed");
        Require(actor->CanDo(command) == NWorld::UCR_OK, "movement preview stayed blocked after the injury was removed");
    }
    Require(actor->GetWishPose() == wishPose, "movement preview changed the requested stance");
}


class RetargetMoveProbe: public NWorld::CCommandExecute, public NWorld::IExecMove
{
    OBJECT_BASIC_METHODS(RetargetMoveProbe);
    bool started = false;
    NAI::EFindPathParams params = NAI::PF_DEFAULT;
    CObj<NAI::CPath> path;
public:
    int updates = 0;
    NAI::SPathPlace destination;
    RetargetMoveProbe() {}
    RetargetMoveProbe(NWorld::CUnitServer* unit, NWorld::CCmdPath* command): CCommandExecute(unit),
        params(command->eParams), destination(command->ptDst.p) {}
    void Run() override { started = true; }
    bool IsExecuting() override { return started; }
    void GetSearchFromPosition(NAI::SPathPlace* result) override { *result = pUS->GetPosition().pos.p; }
    void SetNewPath(NAI::CPath* value, NAI::EFindPathParams flags, NWorld::ENeedActiveItem) override {
        Require(value && !value->points.empty(), "retarget fixture received an empty accepted path");
        path = value; params = flags; destination = value->points.back(); ++updates;
    }
    void GetDesiredPlace(NAI::SPathPlace* result, NAI::EFindPathParams* flags, NWorld::ENeedActiveItem* active) override {
        *result = destination; *flags = params; *active = NWorld::ITEM_NO_MATTER;
    }
    void GetPathPoints(std::list<SPathPoint>* result) override { result->clear(); }
    void FullCancel() override { started = false; Failed(); }
    NAI::CPath* GetCurrentPath() const override { return path; }
};

class RetargetStateProbe: public NWorld::CUnitStateNormal
{
    OBJECT_BASIC_METHODS(RetargetStateProbe);
public:
    int creations = 0;
    CPtr<NWorld::CCmdPath> restoredCommand;
    CObj<RetargetMoveProbe> move;
    explicit RetargetStateProbe(NWorld::CUnitServer* unit = nullptr): CUnitStateNormal(unit) {}
    NWorld::CCommandExecute* CreateExecutor(NWorld::CCmd* command, NWorld::EUnitCommandResult* result) override {
        auto* route = dynamic_cast<NWorld::CCmdPath*>(command);
        if (!route) return CUnitStateNormal::CreateExecutor(command, result);
        restoredCommand = route; ++creations;
        move = new RetargetMoveProbe(pUS, route);
        *result = NWorld::UCR_OK;
        return move;
    }
};

static void LatestRouteCommand(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK()) { actor = unit; break; }
    }
    Require(actor != nullptr, "retarget fixture has no human actor");
    const auto original = actor->GetPosition();
    vector<NAI::SPathPlace> nearby;
    world->GetPathNetwork()->GetNearPlaces(SSphere(original.GetCP(), 5.f), &nearby);
    vector<CObj<NWorld::CCmdPath>> commands;
    for (auto place : nearby) {
        place.SetPose(NAI::CM_STAND); place.SetDirection(original.GetDir());
        auto destination = original.pos; destination.p = place;
        if (fabs2(destination.GetCP() - original.GetCP()) < 1.f) continue;
        if (!commands.empty() && commands.front()->ptDst.p.GetX() == place.GetX() &&
                commands.front()->ptDst.p.GetY() == place.GetY()) continue;
        CObj<NWorld::CCmdPath> command = new NWorld::CCmdPath(destination);
        if (actor->CanDo(command) != NWorld::UCR_OK) continue;
        commands.push_back(command);
        if (commands.size() == 2) break;
    }
    Require(commands.size() == 2, "retarget fixture needs two different reachable destinations");
    CObj<RetargetStateProbe> state = new RetargetStateProbe(actor);
    actor->SetState(state);
    const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    actor->Do(new NWorld::CCmdSetCommand(actor, commands[0]));
    actor->Do(new NWorld::CCmdSetCommand(actor, new NWorld::CCmdContinue));
    Require(state->creations == 1 && actor->IsPerformingAction(), "retarget fixture did not start its original route");
    CObj<RetargetMoveProbe> previous = state->move;
    actor->Do(new NWorld::CCmdSetCommand(actor, commands[1]));
    Require(previous->updates == 1, "retarget fixture did not accept the replacement path through real pathfinding");
    Require(previous->destination.GetX() == commands[1]->ptDst.p.GetX() &&
            previous->destination.GetY() == commands[1]->ptDst.p.GetY(), "accepted live path has the wrong replacement endpoint");
    // The executor is controlled, but command dispatch, pathfinding, the actual
    // interruption event and RefreshExecutor all run through production code.
    actor->OnTBSEvent(NWorld::TBS_RECALC_COMMAND);
    actor->Do(new NWorld::CCmdSetCommand(actor, new NWorld::CCmdContinue));
    Require(state->creations == 2 && state->restoredCommand == commands[1].GetPtr(),
        "movement interruption restored the old route after a new path was accepted");
    actor->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_LEGS, NDb::C_PATIENT, 3));
    actor->Do(new NWorld::CCmdSetCommand(actor, commands[0]));
    Require(state->move->updates == 0, "injury-blocked route replaced the accepted live path");
    actor->GetUnitRPG()->RemoveCritical(NDb::C_PATIENT);
    actor->OnTBSEvent(NWorld::TBS_RECALC_COMMAND);
    actor->Do(new NWorld::CCmdSetCommand(actor, new NWorld::CCmdContinue));
    Require(state->creations == 3 && state->restoredCommand == commands[1].GetPtr(),
        "rejected retarget replaced the route retained for interruption recovery");
    Require(actor->GetPosition() == original && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "retarget bookkeeping moved the actor or changed action points");
}


template<class> struct NativeUIDrawArguments;
template<class R,class C,class Layout,class Position,class Clip,class... Tail>
struct NativeUIDrawArguments<R(C::*)(Layout,Position,Clip,Tail...)>
{
    using Point=std::decay_t<Position>;
    using Rect=std::decay_t<Clip>;
};
using NativeUIDrawPoint=NativeUIDrawArguments<decltype(&NGScene::I2DGameView::CreateDynamicClearRects)>::Point;
using NativeUIDrawRect=NativeUIDrawArguments<decltype(&NGScene::I2DGameView::CreateDynamicClearRects)>::Rect;
template<class P> static NativeUIDrawPoint NativeUIDrawPosition(const P& p){return NativeUIDrawPoint(p.x,p.y);}
template<class R> static NativeUIDrawRect NativeUIDrawWindow(const R& r){return NativeUIDrawRect(r.x1,r.y1,r.x2,r.y2);}

class DialogueLayoutView: public NGScene::I2DGameView
{
    OBJECT_BASIC_METHODS(DialogueLayoutView);
    CObj<NGScene::CTextLocaleInfo> locale;
    CVec2 viewport = CVec2(1024, 768);
public:
    int queries = 0;
    int maxQueries = 100000;
    DialogueLayoutView(): locale(new NGScene::CTextLocaleInfo) { locale->Setup(viewport); }
    void CreateDynamicRects(CFuncBase<NGScene::SText>*, const NativeUIDrawPoint&, const NativeUIDrawRect&) override {}
    void CreateDynamicRects(NDb::CTexture*, const CRectLayout&, const NativeUIDrawPoint&, const NativeUIDrawRect&) override {}
    void CreateDynamicRects(CPtrFuncBase<NGfx::CTexture>*, const CRectLayout&, const NativeUIDrawPoint&, const NativeUIDrawRect&) override {}
    CFuncBase<NGScene::SText>* CreateText(CFuncBase<wstring>*, CFuncBase<CTPoint<int>>*, bool) override { return nullptr; }
    void CreateDynamicClearRects(const CRectLayout&, const NativeUIDrawPoint&, const NativeUIDrawRect&, float) override {}
    NGScene::CTextLocaleInfo* GetLocaleInfo() const override { return locale; }
    const CVec2& GetViewportSize() override {
        Require(++queries <= maxQueries, "dialogue pagination made no progress with markup-only content");
        return viewport;
    }
    void StartNewFrame() override {}
    void Flush() override {}
};

static void OwnedMenuSoundPump(const char* databasePath,const char* resourcePath)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS")!=nullptr,"menu sound fixture requires headless native audio");
    NFMSound::SStartInfo info;info.nMaxChannels=4;
    Require(NFMSound::Init(info),"menu sound fixture could not initialize native audio");
    struct Cleanup { ~Cleanup() { ClearHoldQueue(); NFMSound::Done(); } } cleanup;
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
    Require(sounds.MoveNext(),"menu sound fixture has no native sound record");
    auto* record=sounds.Get();record->bLoop=false;record->nStartSamples=record->nEndingSamples=0;
    auto wave=SilentPCMFixture(44100);
    for(std::size_t i=44;i<wave.size();i+=2) { wave[i]=0xa0;wave[i+1]=0x0f; }
    CObj<NFMSound::CSample2D> sample=NFMSound::LoadSample2D(wave.data(),int(wave.size()));
    Require(IsValid(sample),"menu sound fixture could not decode its real PCM sample");
    CObj<NSound::CFileSample2D> loader=NSound::share2DSamples.Get(record->GetRecordID());
    (loader.GetPtr()->*CachedChapterSampleAccess::ValueMember())=sample.GetPtr();
    struct ReachedRender {};
    class FrameBoundaryView: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(FrameBoundaryView);
        void StartNewFrame() override { throw ReachedRender(); }
    };
    CObj<NGScene::I2DGameView> view=new FrameBoundaryView;
    auto attachView=[&](NUI::CInterface* root,bool expectedOwner) {
        CStructureNetworkContext references;
        auto fields=BorrowedObjectFields(root,&references);
        CObj<NGScene::I2DGameView> original=root->GetView();
        Require(references.ids.count(original.GetPtr())!=0,"render view was not captured in native interface fields");
        references.refs[references.ids.at(original.GetPtr())]=view.GetPtr();
        { CStructureSaver reader(fields,CStructureSaver::READ,&references);static_cast<CObjectBase*>(root)->operator&(reader); }
        Require(root->GetView()==view.GetPtr(),"menu sound fixture could not attach its render-boundary observer");
        CStructureNetworkContext verify;auto snapshot=BorrowedObjectFields(root,&verify);
        bool owner=!expectedOwner;CPtr<NSound::ISoundScene> sound;
        { CStructureSaver reader(snapshot,CStructureSaver::READ,&verify);reader.Add(12,&sound);reader.Add(23,&owner); }
        Require(owner==expectedOwner && sound.GetPtr()==root->GetSound(),
            "render-boundary observer changed native interface sound ownership");
    };
    auto draw=[&](NUI::CInterface* root,STime time) {
        MarkNewDGFrame();bool reached=false;
        try { root->Draw(time); } catch(const ReachedRender&) { reached=true; }
        Require(reached,"native interface frame did not reach the render boundary");
    };
    CObj<CCTime> clock=new CCTime(0);
    CObj<NSound::ISoundScene> shared=NSound::CreateSoundScene(nullptr,nullptr,clock);
    CObj<NUI::ICursor> cursor=NUI::ICursor::Create(false);
    CObj<NUI::CInterface> overlay=new NUI::CInterface(cursor,shared);attachView(overlay,false);
    CObj<NSound::ISound2D> control=shared->Add2DSound(record);
    Require(!control->IsPlaying(),"shared scene started a queued sample before its own pump");
    draw(overlay,100);
    Require(!control->IsPlaying(),"interface pumped a shared mission sound scene twice");
    CTransformStack transform;transform.Init();shared->Draw(&transform);
    Require(control->IsPlaying(),"explicit shared-scene native audio positive control did not start playback");
    vector<float> pcm(512*2);
    Require(S2FmodRenderForTest(pcm.data(),512)!=0 &&
        std::any_of(pcm.begin(),pcm.end(),[](float value) { return std::abs(value)>0.01f; }),
        "shared-scene positive control did not render actual non-silent PCM");
    control=nullptr;shared->Draw(&transform);
    std::puts("native menu sound control: shared scene remains externally pumped; actual PCM playback passes");std::fflush(stdout);
    CObj<NUI::CInterface> menu=new NUI::CInterface(cursor);attachView(menu,true);
    Require(menu->GetSound()!=shared.GetPtr(),"menu did not create its own independent sound scene");
    CObj<NSound::ISound2D> click=menu->GetSound()->Add2DSound(record);
    Require(IsValid(click) && !click->IsPlaying(),"menu sample was not queued before the frame");
    draw(menu,200);
    std::printf("native owned menu sound after Draw: playing=%d\n",int(click->IsPlaying()));std::fflush(stdout);
    Require(click->IsPlaying(),"owned menu frame never pumped its queued native click sound");
    Require(S2FmodRenderForTest(pcm.data(),512)!=0 &&
        std::any_of(pcm.begin(),pcm.end(),[](float value) { return std::abs(value)>0.01f; }),
        "owned menu click did not reach the actual PCM mixer");
    auto activeChannel=[&]() {
        int channel=-1;
        for(int i=0;i<info.nMaxChannels;++i) if(FSOUND_IsPlaying(i)) {
            Require(channel==-1,"menu frame duplicated a queued native channel");channel=i;
        }
        return channel;
    };
    const int channel=activeChannel();
    Require(channel>=0,"playing menu sound has no actual native channel");
    const auto position=FSOUND_GetCurrentPosition(channel);
    Require(position>0,"native menu PCM rendering did not advance its channel");
    draw(menu,300);
    Require(click->IsPlaying() && activeChannel()==channel && FSOUND_GetCurrentPosition(channel)==position,
        "next owned menu frame restarted or duplicated a live native click");
    vector<float> finish(44100*2);
    Require(S2FmodRenderForTest(finish.data(),44100)!=0 && !click->IsPlaying(),
        "menu click did not naturally complete its real PCM sample");
    draw(menu,400);
    Require(!click->IsPlaying() && activeChannel()==-1,"owned menu frame restarted a finished click");
#else
    Require(false,"menu sound fixture requires native SFX");
#endif
}

static void OversizedTooltipPosition(const char* databasePath,const char* resourcePath)
{
    S2Platform::SetErrorDialogs(false);
    struct Cleanup { ~Cleanup() { S2Platform::Done(); } } cleanup;
    Require(S2Platform::Init("native oversized tooltip",1024,768,true),
        "tooltip layout fixture could not initialize its dummy display");
    CFileStream database;database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    const auto oldShow=NGlobal::GetVar("ui_showtooltips"),oldDelay=NGlobal::GetVar("ui_tooltipdelay");
    struct Options{NGlobal::CValue show,delay;~Options(){NGlobal::SetVar("ui_showtooltips",show);NGlobal::SetVar("ui_tooltipdelay",delay);}} options{oldShow,oldDelay};
    NGlobal::SetVar("ui_showtooltips",NGlobal::CValue(1.f));NGlobal::SetVar("ui_tooltipdelay",NGlobal::CValue(0.f));
    struct FrameBoundary{};
    class TooltipLayoutView: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(TooltipLayoutView);
        CObj<NGScene::CCVec2> screen=new NGScene::CCVec2(CVec2(1024,768));
    public:
        int images=0,texts=0;
        void StartNewFrame() override { throw FrameBoundary(); }
        CFuncBase<NGScene::SText>* CreateText(CFuncBase<wstring>* text,CFuncBase<CTPoint<int>>* size,bool tags) override {
            return NGScene::CreateTextFormater(GetLocaleInfo(),screen,text,size,tags);
        }
        void CreateDynamicRects(CFuncBase<NGScene::SText>* node,const NativeUIDrawPoint&,const NativeUIDrawRect&) override {
            CDGPtr<CFuncBase<NGScene::SText>> text=node;text.Refresh();
            Require(!text->GetValue().rectLayouts.empty(),"tooltip renderer lost its packaged font glyphs");++texts;
        }
        void CreateDynamicRects(NDb::CTexture*,const CRectLayout&,const NativeUIDrawPoint&,const NativeUIDrawRect&) override { ++images; }
        void CreateDynamicRects(CPtrFuncBase<NGfx::CTexture>*,const CRectLayout& layout,const NativeUIDrawPoint&,const NativeUIDrawRect&) override {
            if (!layout.rects.empty()) ++texts;
        }
    };
    CObj<TooltipLayoutView> view=new TooltipLayoutView;
    CObj<NUI::ICursor> cursor=NUI::ICursor::Create(false);
    CObj<CCTime> clock=new CCTime(0);CObj<NSound::ISoundScene> sound=NSound::CreateSoundScene(nullptr,nullptr,clock);
    CObj<NUI::CInterface> root=new NUI::CInterface(cursor,sound);
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(root,&refs);auto* original=root->GetView();Require(refs.ids.count(original)!=0,"oversized protocol has no saved native view");refs.refs[refs.ids.at(original)]=view.GetPtr();CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(root.GetPtr())->operator&(reader);}
    Require(root->GetView()==view.GetPtr() && root->GetSound()==sound.GetPtr(),"oversized protocol changed its native view/shared sound");
    CObj<NUI::CToolTip> tip=new NUI::CToolTip(NUI::SWindowInfo(root,NUI::SPoint(0,0),NUI::SPoint(0,0),
        "tooltip",NUI::STYLE_VISIBLE|NUI::STYLE_ENABLED));
    const int width=S2UI::Width(),height=S2UI::Height();
    CObj<NUI::CWindow> owner=new NUI::CWindow(NUI::SWindowInfo(root,NUI::SPoint(0,height-1),NUI::SPoint(1,1),"oversized-owner",NUI::STYLE_VISIBLE|NUI::STYLE_ENABLED));
    owner->SetToolTip(tip);root->SetToolTipOwner(owner);STime frameTime=100;
    auto drawTooltip=[&](){for(int pass=0;pass<2;++pass){MarkNewDGFrame();bool reached=false;try{root->Draw(frameTime++);}catch(const FrameBoundary&){reached=true;}Require(reached,"oversized actual interface did not place before frame");}MarkNewDGFrame();tip->Draw(frameTime,view);};
    const wstring format=L"<font face=Courier size=16pt><left>";
    auto* font=root->GetView()->GetLocaleInfo()->GetFont(NGScene::SFont(16,"Courier"));
    Require(IsValid(font) && font->IsVector(),"tooltip fixture did not use our packaged high-resolution Courier font");
    tip->SetText(format+L"Short tooltip");tip->SetPosition(NUI::SPoint(width-1,height-1));
    drawTooltip();
    Require(view->texts>0,"ordinary tooltip control did not submit native packaged-font glyphs");
    Require(tip->GetSize().x>0 && tip->GetSize().y>0 &&
        tip->GetPosition().x>=0 && tip->GetPosition().y>=0 &&
        tip->GetPosition().x+tip->GetSize().x<=width && tip->GetPosition().y+tip->GetSize().y<=height,
        "ordinary native tooltip edge-placement control failed");
    std::printf("native tooltip control: viewport=%dx%d tip=%dx%d position=%d,%d\n",width,height,
        tip->GetSize().x,tip->GetSize().y,tip->GetPosition().x,tip->GetPosition().y);std::fflush(stdout);
    const auto ordinarySize=tip->GetSize(),ordinaryPosition=tip->GetPosition();
    wstring oversized=format+L"HEADER";
    for(int row=0;row<100;++row)oversized+=L"\nNative packaged-font tooltip line";
    tip->SetText(oversized);tip->SetPosition(NUI::SPoint(width-1,height-1));drawTooltip();
    std::printf("native oversized tooltip: tip=%dx%d position=%d,%d glyph submissions=%d\n",
        tip->GetSize().x,tip->GetSize().y,tip->GetPosition().x,tip->GetPosition().y,view->texts);std::fflush(stdout);
    Require(tip->GetSize().y>height,
        "oversized tooltip fixture did not measure content taller than its viewport");
    Require(tip->GetPosition().x>=0 && tip->GetPosition().y>=0,
        "oversized native tooltip moves its header above the viewport");
    const auto measuredSize=tip->GetSize(),position=tip->GetPosition();
    drawTooltip();
    Require(tip->GetSize()==measuredSize && tip->GetPosition()==position,
        "oversized tooltip drifts during repeated draw");
    tip->SetText(format+L"Short tooltip");tip->SetPosition(NUI::SPoint(width-1,height-1));drawTooltip();
    Require(tip->GetSize()==ordinarySize && tip->GetPosition()==ordinaryPosition,
        "clamping an oversized tooltip changed ordinary font metrics or edge placement");
}

static void MedalCursorVisible(const char* databasePath,const char* resourcePath)
{
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "medal cursor fixture started with a live interface or command");
    CFileStream database; database.OpenRead(databasePath); NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    struct Cleanup { bool native = S2Platform::NativeCursorEnabled();
        ~Cleanup() { NMainLoop::DoneInterface(); S2Platform::UseNativeCursor(native); } } cleanup;
    S2Platform::UseNativeCursor(false); // Exercise the same cursor's existing software fallback.
    NDb::CSide* side = nullptr; NDb::CMedal* medal = nullptr;
    CDBIterator<NDb::CSide> sides(*NDatabase::GetTable<NDb::CSide>());
    while (sides.MoveNext()) if (IsValid(sides.Get()->pMedalPaperContainer)) { side = sides.Get(); break; }
    CDBIterator<NDb::CMedal> medals(*NDatabase::GetTable<NDb::CMedal>());
    while (medals.MoveNext()) if (IsValid(medals.Get()->pName)) { medal = medals.Get(); break; }
    Require(side && medal,"medal cursor fixture has no authored medal and side paper");
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(2,2);
    for (int y=0; y<2; ++y) for (int x=0; x<2; ++x) pixels[y][x] = NGfx::SPixel8888(12,23,34,255);
    texture->Set(pixels);
    CObj<NGame::CICShowMedal> show = new NGame::CICShowMedal(side,L"Cursor fixture",medal,texture); show->Exec();
    CObj<NMainLoop::IInterfaceBase> modal = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(modal) && NMainLoop::GetInterfaceStackDepth() == 1,"native medal command did not open its real dialog");
    CStructureNetworkContext refs; auto fields = BorrowedObjectFields(modal,&refs);
    CPtr<NUI::ICursor> cursor; CPtr<NUI::CInterface> interface;
    { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(5,&cursor); reader.Add(6,&interface); }
    Require(IsValid(cursor) && IsValid(interface) && IsValid(interface->GetDefaultCursorInfo().pCursor),
        "native medal dialog did not retain its cursor and authored arrow");
    class CursorPaintView: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(CursorPaintView);
    public:
        int images = 0;
        void CreateDynamicRects(NDb::CTexture*,const CRectLayout&,const NativeUIDrawPoint&,const NativeUIDrawRect&) override { ++images; }
        void CreateDynamicRects(CPtrFuncBase<NGfx::CTexture>*,const CRectLayout&,const NativeUIDrawPoint&,const NativeUIDrawRect&) override { ++images; }
    };
    CObj<CursorPaintView> view = new CursorPaintView;
    const auto info = interface->GetDefaultCursorInfo();
    CObj<NUI::ICursor> control = NUI::ICursor::Create(true);
    control->SetCursor(info); control->SetPos(CVec2(60,60)); control->Draw(1000,view); control->Draw(2000,view);
    Require(view->images > 0,"native visible-cursor control submitted no arrow image");
    const int expected = view->images; view->images = 0;
    cursor->SetCursor(info); cursor->SetPos(CVec2(60,60)); cursor->Draw(1000,view); cursor->Draw(2000,view);
    std::printf("native medal cursor image submissions=%d visible control=%d\n",view->images,expected);
    Require(view->images == expected,"native medal dialog hides its cursor instead of submitting its arrow image");
    NInput::SEvent event; while (NInput::GetEvent(&event)) {}
    NInput::PostEvent("cancel");
    Require(NInput::GetEvent(&event) && modal->ProcessEvent(event),"native medal dialog did not consume its close command");
    CObj<NMainLoop::CInterfaceCommand> close = NMainLoop::TakeInterfaceCommandForHarness();
    Require(dynamic_cast<NMainLoop::CICExitModal*>(close.GetPtr()) != nullptr,"native medal close did not use its ordinary modal command");
    close->Exec();
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"native medal dialog did not close cleanly");
}

static void ChapterMedalScreenshot(const char* databasePath,const char* resourcePath)
{
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "chapter medal screenshot fixture started with a live interface or command");
    NativeDisplayContext display;
    display.Initialize(databasePath,resourcePath,"Silent Storm chapter medal screenshot regression");
    struct GammaRestore {
        NGlobal::CValue value = NGlobal::GetVar("gfx_gamma",1);
        ~GammaRestore() { NGlobal::SetVar("gfx_gamma",value); NGfx::SetGamma(true); }
    } gammaRestore;
    NGfx::SetGamma(true);
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"chapter medal fixture has no authored map");
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game); enter->Exec();
    CObj<NGame::CMissionBase> mission = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* chapter = IsValid(mission) ? mission->GetInterface()->GetChildByID("chaptermapUI") : nullptr;
    Require(IsValid(chapter),"chapter medal fixture did not open its actual native chapter UI");
    list<CPtr<NScenario::CScenarioClue>> clues; game->pScenarioTracker->GetAvailableClues(&clues);
    for (auto clue : clues) clue->SetJustFound(false);
    auto* merc = game->players.front()->mercs.front().GetPtr();
    Require(merc->medalInfos.size() >= 2 && IsValid(game->players.front()->pSide->pMedalPaperContainer),
        "chapter medal fixture needs two authored medals and their paper");
    // Prepare notification state only; scoring and award thresholds are not exercised.
    for (auto& medal : merc->medalInfos) medal.bJustFound = false;
    struct ReachedRender {};
    class StopBeforeRender: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(StopBeforeRender);
        const CVec2& GetViewportSize() override { throw ReachedRender(); }
    };
    CObj<StopBeforeRender> stop = new StopBeforeRender;
    auto drain = [&](STime time) {
        bool reached = false;
        try { chapter->Draw(time,stop); } catch (const ReachedRender&) { reached = true; }
        Require(reached,"native chapter medal drain did not reach its drawing boundary");
    };
    for (float gamma : {1.f,2.f}) {
        NGlobal::SetVar("gfx_gamma",gamma);
        CMemoryStream reset; int frame = 0;
        { CStructureSaver writer(reset,CStructureSaver::WRITE); writer.Add(18,&frame); }
        reset.Seek(0); { CStructureSaver reader(reset,CStructureSaver::READ); static_cast<CObjectBase*>(chapter)->operator&(reader); }
        for (int i=0; i<2; ++i) { merc->medalInfos[i].bIsGained = true; merc->medalInfos[i].bJustFound = true; }
        NGScene::ClearScreen(CVec3(0.125f,0.25f,0.5f));
        mission->GetInterface()->Draw(1000); NGScene::Flip(); MarkNewDGFrame();
        Require(!NMainLoop::HaveInterfaceCommand(),"native chapter queued its medals before its second painted frame");
        CArray2D<NGfx::SPixel8888> original,corrected;
        NGfx::MakeScreenShot(&original,false); NGfx::MakeScreenShot(&corrected,true);
        Require(original.GetXSize() == 320 && original.GetYSize() == 240,"chapter screenshot did not use its native drawable");
        int distinct = 0, gammaChanged = 0;
        const auto first = original[0][0];
        for (int y=0; y<original.GetYSize(); ++y) for (int x=0; x<original.GetXSize(); ++x) {
            const auto a = original[y][x], b = corrected[y][x];
            if (a.r != first.r || a.g != first.g || a.b != first.b) ++distinct;
            if (a.r != b.r || a.g != b.g || a.b != b.b) ++gammaChanged;
        }
        Require(distinct > 100,"the actual painted chapter framebuffer contains no useful image");
        Require(gamma == 1.f ? gammaChanged == 0 : gammaChanged > 100,
            "native gamma control did not distinguish raw scene pixels from presentation correction");
        drain(1000);
        for (int i=0; i<2; ++i) {
            CObj<NMainLoop::CInterfaceCommand> award = NMainLoop::TakeInterfaceCommandForHarness();
            Require(dynamic_cast<NGame::CICShowMedal*>(award.GetPtr()) != nullptr,"native chapter did not queue exactly its medal commands");
            award->Exec();
            auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
            CStructureNetworkContext refs; auto fields = BorrowedObjectFields(modal,&refs);
            CPtr<NUI::CScreenShot> background;
            { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(8,&background); }
            std::printf("native chapter medal screenshot: gamma=%.1f award=%d texture=%d distinct=%d gammaChanged=%d\n",
                gamma,i,int(IsValid(background) && IsValid(background->GetTexture())),distinct,gammaChanged);
            Require(IsValid(background) && IsValid(background->GetTexture()),
                "native chapter medal dialog has no frozen chapter backdrop");
            CArray2D<NGfx::SPixel8888> captured; background->GetTexture()->Get(&captured);
            Require(captured.GetXSize() == original.GetXSize() && captured.GetYSize() == original.GetYSize(),
                "native medal backdrop lost the chapter framebuffer size");
            for (int y=0; y<original.GetYSize(); ++y) for (int x=0; x<original.GetXSize(); ++x) {
                const auto a = original[y][x], b = captured[y][x];
                Require(a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a,
                    "native medal backdrop recaptured an award overlay or applied presentation gamma twice");
            }
            // Paint the first modal before inspecting the second queued snapshot.
            CPtr<NUI::CInterface> root;
            fields.Seek(0);
            { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(6,&root); }
            Require(IsValid(root),"native medal dialog has no drawable root");
            NGScene::ClearScreen(CVec3(0,0,0)); root->Draw(1100); NGScene::Flip(); MarkNewDGFrame();
            CArray2D<NGfx::SPixel8888> overlay; NGfx::MakeScreenShot(&overlay,false);
            Require(overlay.GetXSize() == original.GetXSize() && overlay.GetYSize() == original.GetYSize(),
                "native medal overlay lost its drawable size");
            int changed = 0;
            for (int y=0; y<original.GetYSize(); ++y) for (int x=0; x<original.GetXSize(); ++x) {
                const auto a = original[y][x], b = overlay[y][x];
                if (a.r != b.r || a.g != b.g || a.b != b.b) ++changed;
            }
            Require(changed > 100,"painting the native award modal did not cover its original chapter framebuffer");
            CObj<NMainLoop::CICExitModal> close = new NMainLoop::CICExitModal; close->Exec();
            Require(NMainLoop::GetCurrentInterfaceForHarness() == mission.GetPtr(),"native medal close did not restore its chapter");
        }
        Require(!NMainLoop::HaveInterfaceCommand() && !merc->medalInfos[0].bJustFound && !merc->medalInfos[1].bJustFound,
            "native chapter did not consume its two notification bits exactly once");
        drain(1000);
        Require(!NMainLoop::HaveInterfaceCommand(),"native chapter repeated its medal notifications on the third frame");
    }
}

// a7b644e9 DiscardAmmo cleared the owned clip; its serialized tag2 can legitimately be null.
static void MedalPanelCaption(const char* databasePath,const char* resourcePath,bool camera = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"medal panel fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "medal panel fixture started with a live interface or command");
    NativeDisplayContext display;
    display.Initialize(databasePath,resourcePath,"Silent Storm medal panel caption regression");
    NMainLoop::CreateProfile("UpstreamMedalPanelCaption"); NMainLoop::SetActiveProfile("UpstreamMedalPanelCaption");
    NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(-1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginMission> enter = new NGame::CICBeginMission(810,-1,vector<string>(),game); enter->Exec();
    CObj<NGame::CMission> mission = dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(mission) && IsValid(mission->GetWorld()),"medal panel fixture did not open its actual tactical mission");
    CObj<NMainLoop::CInterfaceCommand> restart = NMainLoop::TakeInterfaceCommandForHarness();
    Require(dynamic_cast<NMainLoop::CICSaveFile*>(restart.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),
        "medal panel fixture did not isolate its unrelated startup snapshot");
    // The isolated fixture never writes a restart save; only the live panel is tested.
    NWorld::IPlayer::CUnitSet party; mission->GetActivePlayer()->GetPlayer()->GetUnits(&party);
    Require(party.size() == 1,"medal panel fixture needs its single actual roster unit");
    auto* actor = dynamic_cast<NWorld::CUnit*>(party.begin()->GetPtr());
    Require(IsValid(actor),"medal panel fixture has no live world unit");
    auto* merc = actor->GetRPG()->GetRPGUnit();
    Require(!merc->medalInfos.empty(),"medal panel fixture has no authored medal records");
    for (auto& medal : merc->medalInfos) medal.bIsGained = false;
    int awardIndex = 0;
    if (camera) {
        awardIndex = -1;
        const auto& medals = merc->pPers->pSide->medals;
        for (int i=0; i<medals.size() && i<merc->medalInfos.size(); ++i)
            if (IsValid(medals[i]->pModel) && IsValid(medals[i]->pModel->pModel)) { awardIndex = i; break; }
        Require(awardIndex >= 0,"medal camera fixture has no authored award model");
    }
    merc->medalInfos[awardIndex].bIsGained = true; // Prepare an existing award, without scoring it.
    vector<CDBPtr<NDb::CMedal>> gained; merc->GetGainedMedals(&gained);
    Require(gained.size() == 1 && IsValid(gained.front()->pName),"medal panel fixture did not expose its actual award");
    const auto name = NUI::GetDBString(gained.front()->pName);
    const auto normal = NUI::GetDBString(0x1d85), highlighted = NUI::GetDBString(0x1d86);
    Require(!name.empty() && !normal.empty() && !highlighted.empty() && normal.find(L"<") != wstring::npos,
        "medal panel fixture has no authored name and formatting prefixes");
    mission->GetActivePlayer()->Update(true);
    mission->Select(actor);
    Require(mission->CountSelected() == 1,"medal panel fixture did not synchronize its native selected tracker");
    auto* panel = dynamic_cast<NUI::CMedalsPanel*>(mission->GetDesktop()->GetChildByID("medalspanel"));
    Require(IsValid(panel),"actual tactical UI did not construct its medal panel");
    // Loading may already have painted this selection before the fixture award.
    // Reset only the existing cached selection, so Draw regenerates through its native path.
    CMemoryStream selection; CPtr<NWorld::CUnit> previous;
    { CStructureSaver writer(selection,CStructureSaver::WRITE); writer.Add(3,&previous); }
    selection.Seek(0); { CStructureSaver reader(selection,CStructureSaver::READ); static_cast<CObjectBase*>(panel)->operator&(reader); }
    panel->SetStyle(NUI::STYLE_VISIBLE,true);
    struct ReachedRender {};
    class StopBeforeRender: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(StopBeforeRender);
        const CVec2& GetViewportSize() override { throw ReachedRender(); }
    };
    CObj<StopBeforeRender> stop = new StopBeforeRender;
    bool reached = false;
    try { panel->Draw(1000,stop); } catch (const ReachedRender&) { reached = true; }
    Require(reached,"native medal panel did not generate its award before the drawing boundary");
    CStructureNetworkContext refs; auto fields = BorrowedObjectFields(panel,&refs);
    CPtr<NUI::CListView> list;
    { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(8,&list); }
    auto* row = IsValid(list) ? dynamic_cast<NUI::CButton*>(list->GetItem(0)) : nullptr;
    Require(IsValid(row),"native medal panel did not construct its actual award row");
    for (int state : {0,1,255}) {
        auto* window = row->GetState(state);
        auto* text = IsValid(window) ? dynamic_cast<NUI::CText*>(window->GetChildByID("iml-text")) : nullptr;
        Require(IsValid(text) && text->GetText().find(name) != wstring::npos,"native medal caption lost its authored name");
        const auto& prefix = state == 0 ? normal : highlighted;
        std::printf("native medal caption: state=%d prefix-first=%d name-first=%d prefix-chars=%zu name-chars=%zu\n",
            state,int(text->GetText() == prefix+name),int(text->GetText() == name+prefix),prefix.size(),name.size());
        Require(text->GetText() == prefix+name,"native medal caption appends its formatting after the name");
        NUI::SPoint size; text->GetRealSize(&size);
        Require(size.x > 0 && size.y > 0,"native medal caption has empty text layout");
    }
    if (camera) {
        fields.Seek(0); CPtr<NUI::CModel> preview;
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(9,&preview); }
        Require(IsValid(preview) && IsValid(preview->GetModel()),"native medal panel did not assign its actual preview model");
        CMemoryStream base; CStructureNetworkContext modelRefs;
        { CStructureSaver writer(base,CStructureSaver::WRITE,&modelRefs); preview->NUI::CModel::operator&(writer); }
        base.Seek(0); CPtr<NUI::CModelDraw> model;
        { CStructureSaver reader(base,CStructureSaver::READ,&modelRefs); reader.Add(2,&model); }
        Require(IsValid(model),"native medal preview did not retain its real renderer");
        auto modelFields = BorrowedObjectFields(model,&modelRefs); SHMatrix actual;
        { CStructureSaver reader(modelFields,CStructureSaver::READ,&modelRefs); reader.Add(6,&actual); }
        auto matrix = [](const NDb::SCameraParams& parameters) {
            CVec3 forward; (CQuat(parameters.fYaw,V3_AXIS_Z)*CQuat(parameters.fPitch,V3_AXIS_X)).GetYAxis(&forward);
            SHMatrix result; MakeMatrix(&result,parameters.fPitch,parameters.fYaw,parameters.fRoll,
                parameters.vAnchor-forward*parameters.fDistance); return result;
        };
        const auto& parameters = gained.front()->pModel->sCameras[NDb::CAMERA_RELOADBUTTON];
        const auto expected = matrix(parameters), normal = matrix(gained.front()->pModel->sCameras[NDb::CAMERA_NORMAL]);
        auto distance = [](const SHMatrix& a,const SHMatrix& b) {
            const float left[] = {a._11,a._12,a._13,a._14,a._21,a._22,a._23,a._24,a._31,a._32,a._33,a._34,a._41,a._42,a._43,a._44};
            const float right[] = {b._11,b._12,b._13,b._14,b._21,b._22,b._23,b._24,b._31,b._32,b._33,b._34,b._41,b._42,b._43,b._44};
            float result = 0;
            for (int i=0; i<16; ++i) result = Max(result,float(fabs(left[i]-right[i])));
            return result;
        };
        const float authoredDifference = distance(expected,normal);
        Require(authoredDifference > 0.01f,"medal camera fixture cannot distinguish its two authored camera slots");
        std::printf("native medal preview camera: award=%d item=%d authored-slot-difference=%.6f reload-error=%.6f normal-error=%.6f\n",
            gained.front()->GetRecordID(),gained.front()->pModel->GetRecordID(),authoredDifference,distance(actual,expected),distance(actual,normal));
        Require(distance(actual,expected) < 0.0001f,"native medal preview uses the normal inventory camera instead of its authored award camera");
    }
    // Existing remaster snapshots serialize these raw bits. Do not reinterpret
    // an old inventory panel as medals when importing unrelated UI fixes.
    auto loadPanelFields = [&](int bits,bool firstMission) {
        CMemoryStream saved;
        { CStructureSaver writer(saved,CStructureSaver::WRITE); writer.Add(22,&bits); writer.Add(34,&firstMission); }
        saved.Seek(0); { CStructureSaver reader(saved,CStructureSaver::READ); static_cast<CObjectBase*>(mission.GetPtr())->operator&(reader); }
    };
    loadPanelFields(0x4,true);
    Require(mission->GetPanelState(NGame::PANEL_INVENTORY) == 0x4 &&
        !mission->GetPanelState(NGame::PANEL_MEDALS|NGame::PANEL_BIOGRAPHY),
        "existing remaster inventory snapshot bits were reinterpreted as another panel");
    mission->SetPanelState(NGame::PANEL_MEDALS|NGame::PANEL_BIOGRAPHY,true);
    Require(mission->GetPanelState(NGame::PANEL_INVENTORY) == 0x4 &&
        !mission->GetPanelState(NGame::PANEL_MEDALS|NGame::PANEL_BIOGRAPHY),
        "native first-mission restriction blocked inventory or allowed a restricted character tab");
    loadPanelFields(0x10,false);
    Require(mission->GetPanelState(NGame::PANEL_MEDALS) == 0x10 &&
        !mission->GetPanelState(NGame::PANEL_INVENTORY|NGame::PANEL_BIOGRAPHY),
        "existing remaster medal snapshot bits were reinterpreted as another panel");
    loadPanelFields(0x20,false);
    Require(mission->GetPanelState(NGame::PANEL_BIOGRAPHY) == 0x20 &&
        !mission->GetPanelState(NGame::PANEL_INVENTORY|NGame::PANEL_MEDALS),
        "existing remaster biography snapshot bits were reinterpreted as another panel");
    std::puts("native medal panel compatibility: saved bits 4/16/32 and named first-mission restrictions preserved");
    Require(!NMainLoop::HaveInterfaceCommand(),"measuring native medal captions queued an unrelated game command");
}

static void CheckChapterClueHeading(NGame::CMissionBase* mission,NUI::CWindow* chapter)
{
    auto* game = mission->GetRPGGame(); auto* tracker = game->pScenarioTracker.GetPtr();
    auto* map = chapter->GetChildByID("view");
    list<CPtr<NUI::CWindow>> children; map->GetChildrenList(&children);
    std::printf("chapter heading fixture map=%d scenario=%d available=%d children=%zu\n",
        mission->GetChapterMap()->GetRecordID(),tracker->GetScenarioID(),int(tracker->IsScenarioAvailable()),children.size());
    CPtr<NUI::CWindow> sector; CPtr<NScenario::CScenarioZone> zone; CPtr<NScenario::CScenarioClue> clue;
    for (auto child : children) {
        if (pSSClasses->GetObjectTypeID(child) != int(0xB2606153)) continue;
        CStructureNetworkContext refs; auto fields = BorrowedObjectFields(child,&refs);
        CPtr<NScenario::CScenarioZone> candidate;
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(3,&candidate); }
        std::printf("chapter zone=%d clues=%zu\n",IsValid(candidate) ? candidate->GetDBZone()->GetRecordID() : -1,
            IsValid(candidate) ? candidate->GetClues().size() : 0);
        if (!IsValid(candidate)) continue;
        for (auto entry : candidate->GetClues()) {
            if (!entry->IsCompound() && entry->IsPlaced() && !entry->IsDestroyed() &&
                !entry->GetParentZones().empty() && entry->GetParentZones().front() == candidate &&
                !NUI::GetDBString(entry->GetDBClue()->pDescription).empty()) {
                sector = child; zone = candidate; clue = entry; break;
            }
        }
        if (IsValid(clue)) break;
    }
    Require(IsValid(clue),"chapter clue-heading fixture found no native map sector with a placed clue");
    tracker->CheatOpenZone(zone); tracker->CheatTakeClue(clue,true);
    list<CPtr<NScenario::CScenarioClue>> found; tracker->GetAvailableClues(&found);
    Require(std::find(found.begin(),found.end(),clue) != found.end(),"native chapter fixture did not acquire its actual clue");
    for (auto entry : found) entry->SetJustFound(false);
    const auto clueText = NUI::GetDBString(clue->GetDBClue()->pDescription);
    auto* header = NDb::GetString(0x4f35);
    Require(IsValid(header) && !header->szStr.empty(),"native chapter clue heading has no authored localized record");
    std::printf("native chapter heading record: chars=%zu font-tag=%d count-placeholder=%d\n",header->szStr.size(),
        int(header->szStr.find(L"<font") != wstring::npos),int(header->szStr.find(L"%d") != wstring::npos));
    struct Restore { NDb::CString* record; wstring original; ~Restore() { record->szStr = original; } } restore{header,header->szStr};
    struct ReachedRender {};
    class StopBeforeRender: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(StopBeforeRender);
        const CVec2& GetViewportSize() override { throw ReachedRender(); }
    };
    CObj<StopBeforeRender> view = new StopBeforeRender;
    CStructureNetworkContext chapterRefs; auto chapterFields = BorrowedObjectFields(chapter,&chapterRefs);
    CVec2 position; { CStructureSaver reader(chapterFields,CStructureSaver::READ,&chapterRefs); reader.Add(8,&position); }
    CMemoryStream settled; STime time = 1000; int specialFrame = 3;
    { CStructureSaver writer(settled,CStructureSaver::WRITE); writer.Add(8,&position); writer.Add(9,&position); writer.Add(10,&time); writer.Add(18,&specialFrame); }
    settled.Seek(0); { CStructureSaver reader(settled,CStructureSaver::READ); static_cast<CObjectBase*>(chapter)->operator&(reader); }
    auto draw = [&]() {
        bool reached = false;
        try { chapter->Draw(time,view); } catch (const ReachedRender&) { reached = true; }
        Require(reached && !NMainLoop::HaveInterfaceCommand(),"chapter heading fixture did not reach its drawing boundary cleanly");
    };
    draw(); // Native UpdateSector establishes scenario visibility before the hover pass.
    NUI::SPoint origin; NUI::SRect clip;
    Require(sector->ClientToScreen(&origin,&clip),"native clue sector has no visible map rectangle");
    NUI::SPoint cursor((clip.x1+clip.x2)/2,(clip.y1+clip.y2)/2);
    CMemoryStream cursorFields;
    { CStructureSaver writer(cursorFields,CStructureSaver::WRITE); writer.Add(5,&cursor); }
    cursorFields.Seek(0);
    { CStructureSaver reader(cursorFields,CStructureSaver::READ); static_cast<CObjectBase*>(mission->GetInterface())->operator&(reader); }
    const char* panelID = cursor.x > S2UI::MapOffsetX()+512 ? "text_left" : "text_right";
    auto* panel = chapter->GetChildByID(panelID);
    auto* text = panel ? dynamic_cast<NUI::CText*>(panel->GetChildByID("text")) : nullptr;
    Require(IsValid(text),"native chapter map did not build its description text");
    for (const auto& localized : {restore.original,restore.original+L"\u0423\u043B\u0438\u043A\u0438 regression"}) {
        header->szStr = localized; draw();
        Require(text->GetText().find(clueText) != wstring::npos,"hovering the actual chapter sector did not display its acquired clue");
        std::printf("native chapter clue text: localized heading=%d hardcoded English=%d\n",
            int(text->GetText().find(localized) != wstring::npos),int(text->GetText().find(L"Clues found:") != wstring::npos));
        Require(text->GetText().find(localized) != wstring::npos,"native chapter clue description ignored its localized heading record");
    }
}

static void RejectedHitLabelText(const char* databasePath,const char* resourcePath,int colourKind=-1)
{
    struct FontModeRestore { NGlobal::CValue previous;~FontModeRestore(){NGlobal::SetVar("ui_vector_fonts",previous);} } mode{NGlobal::GetVar("ui_vector_fonts",1)};
    struct PlatformCleanup { bool started=false;~PlatformCleanup(){if(started)S2Platform::Done();} } platform;
    if(colourKind>=0) {
        S2Platform::SetErrorDialogs(false);
        Require(S2Platform::Init("native hit glyph colours",1024,768,true),"hit colour fixture could not initialize SDL");platform.started=true;
    }
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    auto* rejected = NDb::GetString(21060);
    Require(IsValid(rejected) && !rejected->szStr.empty(), "hit-label fixture has no authored localized rejected-hit text");
    class HitLayoutReached: public std::exception {};
    class HitView: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(HitView);
    public:
        bool layout=false;
        vector<DWORD> colours;
        int vectorGlyphs=0,bitmapGlyphs=0;
        const CVec2& GetViewportSize() override { if(!layout)throw HitLayoutReached();return DialogueLayoutView::GetViewportSize(); }
        void CreateDynamicRects(CPtrFuncBase<NGfx::CTexture>* texture,const CRectLayout& glyphs,const NativeUIDrawPoint&,const NativeUIDrawRect&) override {
            for(const auto& glyph:glyphs.rects) {
                colours.push_back(glyph.sColor.color);
                if(NGScene::IsVectorTexture(texture))++vectorGlyphs;else ++bitmapGlyphs;
            }
        }
    };
    CObj<HitView> view = new HitView;view->layout=colourKind>=0;
    bool correctColours=true;
    for (bool pk: {false,true}) for (int value: {-1,0,42}) for(bool vectorFonts:{false,true}) {
        if(colourKind<0 && !vectorFonts)continue;
        if(colourKind>=0 && int(pk)!=colourKind)continue;
        if(colourKind>=0)NGlobal::SetVar("ui_vector_fonts",vectorFonts?1:0);
        CObj<EnemyIconMission> mission = new EnemyIconMission;
        mission->view = new NGScene::GameViewFixture;
        CObj<MissionIconProbe> desktop = new MissionIconProbe;
        mission->Attach(world,desktop);
        CObj<NUI::CWindow> client = new NUI::CWindow(NUI::SWindowInfo(nullptr,
            NUI::SPoint(0,0),NUI::SPoint(1024,768),"hit-view",NUI::STYLE_ENABLED|NUI::STYLE_VISIBLE));
        CObj<NUI::CText> pause = new NUI::CText;
        CStructureNetworkContext refs;
        refs.externalReferences = true; refs.nextID = 100; refs.include = [](CObjectBase*) { return false; };
        for (CObjectBase* object: {static_cast<CObjectBase*>(mission.GetPtr()),static_cast<CObjectBase*>(client.GetPtr()),static_cast<CObjectBase*>(pause.GetPtr())}) {
            const auto id = refs.nextID++; refs.ids[object] = id; refs.refs[id] = object;
        }
        struct ClientWire {
            CObj<NUI::CWindow> client;
            int operator&(CStructureSaver& saver) { saver.Add(2,&client); return 0; }
        } base;
        base.client = client;
        CPtr<NGame::IMission> missionRef = mission.GetPtr();
        CMemoryStream saved;
        { CStructureSaver writer(saved,CStructureSaver::WRITE,&refs); writer.Add(1,&base); writer.Add(2,&missionRef); writer.Add(27,&pause); }
        saved.Seek(0);
        { CStructureSaver reader(saved,CStructureSaver::READ,&refs); static_cast<CObjectBase*>(desktop.GetPtr())->operator&(reader); }
        world->AddHitLocator(new NWorld::CHitLocator(value,pk,CVec3(0,0,0.5f),nullptr));
        desktop->RefreshHits(0);
        auto* label = dynamic_cast<NUI::CText*>(client->GetChildByID("hit"));
        Require(label && pSSClasses->GetObjectTypeID(label) == int(0xB0241949),
            "native HUD did not create its actual hit-text tracker from a world hit event");
        for (STime time: {STime(0),STime(1000)}) {
            view->colours.clear();view->vectorGlyphs=view->bitmapGlyphs=0;
            bool reached = false;
            try { label->Draw(time,view); } catch (const HitLayoutReached&) { reached = true; }
            Require(reached || colourKind>=0, "native hit text did not reach layout after updating its position and source");
            const auto& text = label->GetText();
            const auto prefixEnd = text.find(L'>');
            Require(prefixEnd != wstring::npos && text.substr(0,7) == L"<color=",
                "native hit text lost its existing colour markup");
            const wstring expected = value < 0 ? rejected->szStr : std::to_wstring(value);
            Require(text.substr(prefixEnd+1) == expected,
                "a rejected native HUD hit displayed the negative damage sentinel instead of its localized label");
            const wstring expectedMarkup = pk ? (time == 0 ? L"<color=ff00bff3>" : L"<color=7f005f79>") :
                (time == 0 ? L"<color=ffff0000>" : L"<color=7f7f0000>");
            if(colourKind<0)Require(text.substr(0,prefixEnd+1) == expectedMarkup,
                "native rejected hit text changed its verified original colour or fade rules");
            else {
                const DWORD expectedColour=pk?(time==0?0xff00bff3u:0x7f005f79u):(time==0?0xffff0000u:0x7f7f0000u);
                Require(!view->colours.empty(),"native hit label did not submit actual glyph quads");
                Require((vectorFonts?view->vectorGlyphs:view->bitmapGlyphs)==int(view->colours.size()) && (vectorFonts?view->bitmapGlyphs:view->vectorGlyphs)==0,
                    "native hit colour fixture did not use the requested real bitmap/vector font texture");
                std::printf("native hit glyph colour: pk=%d vector=%d value=%d time=%u glyphs=%zu actual=%08x expected=%08x\n",int(pk),int(vectorFonts),value,unsigned(time),view->colours.size(),unsigned(view->colours.front()),unsigned(expectedColour));std::fflush(stdout);
                for(DWORD colour:view->colours)correctColours=correctColours && colour==expectedColour;
            }
        }
        const auto lastText = label->GetText();
        label->Draw(2001,view);
        Require(label->GetText() == lastText, "an expired native hit label regenerated or changed its text");
    }
    Require(correctColours,"native hit glyphs use the wrong armor colour or fail to fade RGB with alpha");
}

static void LocalizedLogStyle(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=2,"unknown localized log mode");
    S2Platform::SetErrorDialogs(false);
    Require(S2Platform::Init("native localized log",2048,1536,true),"log fixture could not initialize dummy SDL");
    struct Cleanup {~Cleanup(){S2Platform::Done();}} cleanup;
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    S2Platform::UpdateDisplay(mode==2?200:100);MarkNewDGFrame();
    const float scale=S2UI::Scale();
    const wstring ownFont=L"<font face=Impact size=20pt>";
    const wstring authored=NUI::GetDBString(20952);
    Require(authored.find(L"0xFFEAC78B")!=wstring::npos && authored.find(L"outlinecolor=0xFF513E2B")!=wstring::npos,
        "log fixture lacks the actual localized authored colour and outline");
    CObj<NUI::CLogPanel> panel=new NUI::CLogPanel(NUI::SWindowInfo(nullptr,NUI::SPoint(0,0),NUI::SPoint(1024,768),"log",NUI::STYLE_VISIBLE|NUI::STYLE_ENABLED),STREAM_GAME);
    const int added=mode==1?12:1;
    for(int i=0;i<added;++i)panel->AddLine(L"Native log "+std::to_wstring(i)+L" \u0416");
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(panel,&refs);
    vector<NUI::STextLine> lines;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(6,&lines);}
    Require(lines.size()==10 && lines[0].bUsed,"log changed its original ten-slot ring");
    int current=lines[0].nNext,count=0;std::set<int> visited;
    class LogStyleView:public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(LogStyleView);
    public:
        vector<DWORD> colours;
        vector<CRectLayout::SRect> glyphs;
        vector<CPtrFuncBase<NGfx::CTexture>*> textures;
        void CreateDynamicRects(CPtrFuncBase<NGfx::CTexture>* texture,const CRectLayout& layout,const NativeUIDrawPoint&,const NativeUIDrawRect&) override {
            for(const auto& quad:layout.rects){colours.push_back(quad.sColor.color);textures.push_back(texture);glyphs.push_back(quad);}
        }
    };
    CObj<LogStyleView> view=new LogStyleView;
    auto* existingFont=view->GetLocaleInfo()->GetFont(NGScene::SFont(Float2Int(20*scale),"Impact"));Require(IsValid(existingFont),"log fixture has no existing packaged font");
    bool correctStyle=true;
    while(current!=0) {
        Require(current>0 && current<lines.size() && visited.insert(current).second,"log ring is not a valid finite linked list");
        auto& line=lines[current];Require(line.bUsed && line.sTime==0 && IsValid(line.pText),"new log line lost its text or initial display age");
        const auto payload=L"Native log "+std::to_wstring(added-1-count)+L" \u0416";
        Require(line.pText->GetText().find(payload)!=wstring::npos,"log eviction lost newest-first event text");
        view->colours.clear();view->textures.clear();view->glyphs.clear();
        line.pText->Draw(nullptr,1000,view);
        Require(!view->textures.empty(),"combat log's actual CML renderer produced no glyph quads");
        Require(std::all_of(view->textures.begin(),view->textures.end(),[&](auto* texture){return texture==existingFont->GetTexture();}),
            "localized log style changed the remaster's existing font family, point size or texture");
        const bool foreground=std::find(view->colours.begin(),view->colours.end(),DWORD(0xFFEAC78B))!=view->colours.end();
        const bool outline=std::find(view->colours.begin(),view->colours.end(),DWORD(0xFF513E2B))!=view->colours.end();
        vector<CRectLayout::SRect> foregroundGlyphs;
        for(const auto& quad:view->glyphs)if(quad.sColor.color!=0xFF513E2B)foregroundGlyphs.push_back(quad);
        view->colours.clear();view->textures.clear();view->glyphs.clear();
        CObj<NUI::CTextDraw> reference=new NUI::CTextDraw(NUI::SPoint(0,0),NUI::SPoint(-1,-1),ownFont+payload);
        reference->Draw(nullptr,1000,view);
        Require(foregroundGlyphs.size()==view->glyphs.size(),"localized log changed the original font's glyph count");
        for(int i=0;i<foregroundGlyphs.size();++i)Require(std::fabs(foregroundGlyphs[i].fSizeX-view->glyphs[i].fSizeX)<0.001f &&
            std::fabs(foregroundGlyphs[i].fSizeY-view->glyphs[i].fSizeY)<0.001f,
            "localized log changed the original20pt glyph dimensions");
        correctStyle=correctStyle && foreground && outline;
        ++count;current=line.nNext;
    }
    Require(count==Min(added,9),"log ring has the wrong number of retained events");
    std::printf("localized log mode=%d scale=%.1f lines=%d font=Impact20 sameTexture=1 vector=%d authoredStyle=%d\n",mode,scale,count,int(existingFont->IsVector()),int(correctStyle));std::fflush(stdout);
    Require(correctStyle,"combat log omitted localized colour/outline while retaining the existing font");
}

static void ScaledEditTextLayout()
{
    S2Platform::SetErrorDialogs(false);
    struct Cleanup { ~Cleanup() { S2Platform::Done(); } } cleanup;
    Require(S2Platform::Init("scaled native edit text",2048,1536,true), "edit layout fixture could not initialize dummy SDL");
    class EditLayoutView: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(EditLayoutView);
        CObj<NGScene::CCVec2> screen = new NGScene::CCVec2(CVec2(2048,1536));
    public:
        CDGPtr<CFuncBase<CTPoint<int>>> dimensions;
        NGScene::SText rendered;
        vector<CPtrFuncBase<NGfx::CTexture>*> textures;
        CFuncBase<NGScene::SText>* CreateText(CFuncBase<wstring>* text,CFuncBase<CTPoint<int>>* size,bool tags) override {
            dimensions = size;
            return NGScene::CreateTextFormater(GetLocaleInfo(),screen,text,size,tags);
        }
        void CreateDynamicRects(CFuncBase<NGScene::SText>* node,const NativeUIDrawPoint&,const NativeUIDrawRect&) override {
            CDGPtr<CFuncBase<NGScene::SText>> text = node;
            text.Refresh(); rendered = text->GetValue();
            for (const auto& layout: rendered.rectLayouts) textures.push_back(layout.pFontInfo->GetTexture());
        }
        void CreateDynamicRects(CPtrFuncBase<NGfx::CTexture>* texture,const CRectLayout& layout,
            const NativeUIDrawPoint&,const NativeUIDrawRect&) override {
            if (!layout.rects.empty()) textures.push_back(texture);
        }
    };
    CObj<EditLayoutView> view = new EditLayoutView;
    CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false);
    CObj<NUI::CInterface> interface = new NUI::CInterface(cursor);
    NUI::CEdit* edit = new NUI::CEdit(NUI::SWindowInfo(interface,NUI::SPoint(5,5),NUI::SPoint(48,100),"edit",
        NUI::STYLE_VISIBLE | NUI::STYLE_ENABLED));
    const wchar_t* formats[] = {L"",L"<font face=Courier size=16pt><left>",L"<font face=Times size=18pt><left>",L"<font face=Arial size=20pt><left>"};
    const char* families[] = {"System","Courier","Times","Arial"};
    const int points[] = {22,16,18,20}; // Preserve the existing formatter's 22pt unformatted default.
    const wstring caption = L"a b c \u0416 \u042f \u0451 d e f g h i j k";
    for (int family = 0; family < 4; ++family) for (float scale: {1.f,1.5f,2.f}) {
        S2Platform::UpdateDisplay(scale*100); MarkNewDGFrame();
        Require(std::fabs(S2Platform::Display().uiScale-scale) < 0.001f, "edit font fixture could not establish its actual UI scale");
        edit->SetTextFormat(formats[family]); edit->SetText(caption); edit->SetCursorPosition(2);
        view->textures.clear();
        auto* font = view->GetLocaleInfo()->GetFont(NGScene::SFont(Float2Int(points[family]*scale),families[family]));
        Require(IsValid(font) && font->IsVector(), "the test application is missing its packaged high-resolution fonts");
        edit->Draw(1000,view);
        Require(!view->textures.empty() && std::all_of(view->textures.begin(),view->textures.end(),
            [&](auto* texture) { return texture == font->GetTexture(); }),
            "the edit renderer changed the remaster's original high-resolution font or its default point size");
        Require(view->dimensions.GetPtr() && !view->rendered.charsSet.empty() && !view->rendered.rectLayouts.empty(),
            "edit font fixture did not evaluate the real native formatter and packaged glyphs");
        view->dimensions.Refresh();
        const CTPoint<int> expectedSize(Float2Int(48*scale),Float2Int(100*scale));
        Require(view->dimensions->GetValue() == expectedSize,
            "the native edit renderer wrapped packaged screen-sized glyphs at the unscaled logical control width");
        for (const auto& layout: view->rendered.rectLayouts)
            Require(layout.pFontInfo.GetPtr() == font, "edit layout changed the packaged font family or its original point size");
        CObj<NGScene::CCWString> source = new NGScene::CCWString(wstring(formats[family])+caption);
        CObj<NGScene::CCTPoint> size = new NGScene::CCTPoint(expectedSize);
        CObj<NGScene::CCVec2> screen = new NGScene::CCVec2(CVec2(2048,1536));
        CDGPtr<CFuncBase<NGScene::SText>> reference = NGScene::CreateTextFormater(view->GetLocaleInfo(),screen,source,size,true);
        reference.Refresh();
        const auto& expected = reference->GetValue();
        Require(view->rendered.sSize == expected.sSize && view->rendered.charsSet.size() == expected.charsSet.size(),
            "edit text wraps differently from the unchanged native font renderer at the correct pixel width");
        for (std::size_t i = 0; i < expected.charsSet.size(); ++i) {
            const auto& actualRect = view->rendered.charsSet[i].sRect;
            const auto& expectedRect = expected.charsSet[i].sRect;
            Require(view->rendered.charsSet[i].nIndex == expected.charsSet[i].nIndex &&
                actualRect.x1 == expectedRect.x1 && actualRect.y1 == expectedRect.y1 &&
                actualRect.x2 == expectedRect.x2 && actualRect.y2 == expectedRect.y2,
                "edit glyph geometry differs from the remaster's original font renderer");
        }
        Require(edit->GetText() == caption && edit->GetCursorPosition() == 2, "font layout modified the editable source or cursor position");
        std::printf("FONT-COMPAT: %s %dpt scale=%.1f vector=1 glyphs=%zu width=%d height=%d\n",
            families[family],points[family],scale,expected.charsSet.size(),expectedSize.x,expected.sSize.y);
    }
}

static void CheckChapterMarkerToolTipTransitions(NUI::CWindow* marker,NUI::CText* text)
{
    CObj<DialogueLayoutView> view = new DialogueLayoutView;
    const int modes[] = {0,1,2}; // Existing native Move, Normal and Zone marker states.
    const int hints[] = {11123,11124,11126};
    for (int i = 0; i < 3; ++i) {
        int from = modes[i] == 0 ? 1 : 0, target = modes[i];
        STime morph = 1000, flash = 1000;
        CMemoryStream state;
        {
            CStructureSaver writer(state,CStructureSaver::WRITE);
            writer.Add(2,&from); writer.Add(3,&target); writer.Add(4,&morph); writer.Add(5,&flash);
        }
        state.Seek(0);
        { CStructureSaver reader(state,CStructureSaver::READ); static_cast<CObjectBase*>(marker)->operator&(reader); }
        marker->Draw(1001,view);
        Require(!NUI::GetDBString(hints[i]).empty() && text->GetText() == NUI::GetDBString(hints[i]),
            "the chapter marker retained its previous hint when its real draw transitioned to another action");
        // Finish this transition and ensure the hint survives steady-state drawing.
        marker->Draw(10000,view);
        Require(text->GetText() == NUI::GetDBString(hints[i]), "the chapter action hint disappeared after its transition");
    }
}

static void CheckChapterExitMarkerHint(NGame::CMissionBase* mission,NUI::CWindow* chapter,NUI::CWindow* marker,NUI::CText* text)
{
    CDGPtr<CPtrFuncBase<CChapterInfo>> info = mission->GetChapterInfo(); info.Refresh();
    CVec2 position;
    bool found = false;
    for (const auto& sector: info->GetValue()->sectorsSet) {
        if (sector.eType != EXITZONE || sector.pointsSet.empty()) continue;
        position = sector.pointsSet.front()+CVec2(10,10); found = true; break;
    }
    Require(found, "chapter exit-hint fixture has no authored exit sector");
    CMemoryStream state;
    STime last = 1000;
    int specialFrame = 3;
    {
        CStructureSaver writer(state,CStructureSaver::WRITE);
        writer.Add(8,&position); writer.Add(9,&position); writer.Add(10,&last); writer.Add(18,&specialFrame);
    }
    state.Seek(0);
    { CStructureSaver reader(state,CStructureSaver::READ); static_cast<CObjectBase*>(chapter)->operator&(reader); }
    struct ReachedChapterRender {};
    class ChapterSelectionView: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(ChapterSelectionView);
    public:
        const CVec2& GetViewportSize() override { throw ReachedChapterRender(); }
    };
    CObj<ChapterSelectionView> selectionView = new ChapterSelectionView;
    CObj<DialogueLayoutView> view = new DialogueLayoutView;
    marker->Draw(1000,view); // A real preceding frame establishes the marker's transition clock.
    bool reachedRender = false;
    try { chapter->Draw(1001,selectionView); }
    catch (const ReachedChapterRender&) { reachedRender = true; }
    Require(reachedRender, "chapter exit-hint fixture never reached its rendering boundary");
    Require(!NMainLoop::HaveInterfaceCommand(), "chapter exit-hint selection unexpectedly queued a campaign command");
    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(marker,&references);
    int target = -1;
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(3,&target); }
    Require(target == 2 || target == 3, "chapter exit-hint fixture did not place the party inside an action sector");
    Require(target == 3, "the native chapter map selected a mission-entry marker for its campaign exit sector");
    marker->Draw(1001,view);
    Require(!NUI::GetDBString(20253).empty() && text->GetText() == NUI::GetDBString(20253),
        "the chapter exit marker displayed the mission-entry hint instead of its authored exit hint");
}

class DialogueMissionProbe: public NGame::CMissionBase
{
    OBJECT_BASIC_METHODS(DialogueMissionProbe);
public:
    int stageEvents = 0;
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    void DoEvent(NWorld::CCommand* command) override { CObj<NWorld::CCommand> hold = command; ++stageEvents; }
};

class DialoguePagesProbe: public NUI::CMissionDlgUI
{
    OBJECT_NOCOPY_METHODS(DialoguePagesProbe);
public:
    using CMissionDlgUI::CMissionDlgUI;
    using CMissionDlgUI::UpdatePhrases;
    using CMissionDlgUI::SetStage;
};

static void MarkupLeadingWhitespace(const char* databasePath,const char* resourcePath)
{
    auto world=FixtureWorld(databasePath,resourcePath,true);
    CObj<DialogueLayoutView> view=new DialogueLayoutView;
    auto* font=view->GetLocaleInfo()->GetFont(NGScene::SFont(32,"Courier"));
    Require(IsValid(font) && font->IsVector(),"leading-space fixture did not use our packaged Courier font");
    for(int iteration=0;iteration<48;++iteration) {
        CObj<NUI::IML> previous=NUI::CreateML();
        previous->SetText(L"<font face=Courier size=32pt><left>Alpha<br>Beta<br>Gamma<br>Delta",0);
        previous->Generate(view,240);previous=nullptr;
        CObj<NUI::IML> text=NUI::CreateML();text->SetText(L"<font face=Courier size=32pt><left>  Alpha",0);text->Generate(view,240);
        list<NativeUIDrawRect> bounds;text->Render(&bounds, NativeUIDrawPoint(0,0), NativeUIDrawRect(0,0,0,0));
        Require(bounds.size()==7,"native leading-space fixture lost its character bounds");
        const auto size=text->GetSize();
        for(const auto& bound:bounds) {
            if(bound.x1<0 || bound.y1<0 || bound.y2>size.y || bound.x2>240) {
                std::printf("native ignored-space bounds: iteration=%d rect=%.9g,%.9g..%.9g,%.9g layout=%d,%d\n",
                    iteration,double(bound.x1),double(bound.y1),double(bound.x2),double(bound.y2),size.x,size.y);std::fflush(stdout);
                Require(false,"native markup retained undefined coordinates for a leading space omitted from reflow");
            }
        }
    }
}

static void MarkupConvenienceSymbols(const char* databasePath,const char* resourcePath)
{
    auto world=FixtureWorld(databasePath,resourcePath,true);
    CObj<DialogueLayoutView> view=new DialogueLayoutView;
    for(const auto& content:{wstring(L"Ctrl+UP"),wstring(L"\u00d7" L"2"),wstring(L"(+3/-2)"),wstring(L"1\u22122\u21923")}) {
        CObj<NUI::IML> text=NUI::CreateML();
        text->SetText(L"<font face=Courier size=24pt><color=red><left>"+content,0);
        text->Generate(view,512);
        list<NativeUIDrawRect> bounds;
        text->Render(&bounds,NativeUIDrawPoint(0,0),NativeUIDrawRect(0,0,0,0));
        if(bounds.size()!=content.size())std::printf("symbol text first=U+%04x expected=%zu actual=%zu\n",unsigned(content[0]),content.size(),bounds.size());
        Require(bounds.size()==content.size(),"markup dropped a shortcut, speed or comparison symbol");
    }
}

static void OversizedDialogueLine(const char* databasePath, const char* resourcePath, int scenario = 0, float formattedScale = 1)
{
    const bool markupOnly = scenario == 1;
    const bool formattedPages = scenario == 3;
    const bool normalPages = scenario >= 2;
    struct PlatformCleanup { bool active; ~PlatformCleanup(){if(active)S2Platform::Done();} } platform{formattedPages};
    if(formattedPages) {
        S2Platform::SetErrorDialogs(false);
        Require(S2Platform::Init("formatted native dialogue",2048,1536,true),"formatted dialogue fixture could not initialize its dummy display");
    }
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* format = NDb::GetString(11209);
    Require(format != nullptr, "dialogue formatting fixture has no prefix record");
    struct RestoreFormat { NDb::CString* record; wstring original; ~RestoreFormat(){record->szStr=original;} } restoreFormat{format,format->szStr};
    if(formattedPages) {
        format->szStr=L"<font face=Courier size=32pt><left>";
        S2Platform::UpdateDisplay(Float2Int(formattedScale*100));MarkNewDGFrame();
        Require(std::fabs(S2Platform::Display().uiScale-formattedScale)<.001f,"formatted dialogue fixture could not set its native UI scale");
    }

    vector<CPtr<NWorld::CUnit>> actors;
    world->GetAllUnits(&actors);
    Require(!actors.empty(), "dialogue fixture has no speaker");
    CObj<DialogueMissionProbe> mission = new DialogueMissionProbe;
    vector<CObj<NDb::CString>> texts;
    vector<CObj<NDb::CDBAckInfo>> records;
    vector<CObj<NWorld::CAckEvent>> owners;
    vector<CPtr<NWorld::CAckEvent>> phrases;
    wstring first = normalPages
        ? L"alpha beta gamma delta epsilon zeta eta theta iota kappa lambda mu nu xi omicron pi rho sigma tau upsilon phi chi psi omega"
        : L"Oversized first phrase";
    if (markupOnly) {
        int textureID = 0;
        CDBIterator<NDb::CUITexture> textures(*NDatabase::GetTable<NDb::CUITexture>());
        while (textures.MoveNext()) {
            auto* record = textures.Get();
            for (int mode = 0; mode < NDb::UIM_MAX; ++mode)
                if (IsValid(record->pTextures[mode]) && record->pTextures[mode]->nWidth > 0 && record->pTextures[mode]->nHeight > 0)
                    textureID = record->GetRecordID();
            if (textureID) break;
        }
        Require(textureID != 0, "markup dialogue fixture has no usable image");
        const auto image = L"<image id=" + std::to_wstring(textureID) + L" width=32 height=8>";
        first = image + L"<br>" + image;
    }
    for (const wstring& value : {first, wstring(L"Following second phrase")}) {
        CObj<NDb::CString> text = new NDb::CString; text->szStr = value; texts.push_back(text);
        CObj<NDb::CDBAckInfo> info = new NDb::CDBAckInfo;
        info->pText = text; info->voices.resize(6); records.push_back(info);
        owners.emplace_back(new NWorld::CAckEvent(1, actors.front(), info));
        phrases.push_back(owners.back().GetPtr());
    }
    CObj<DialoguePagesProbe> dialog = new DialoguePagesProbe(
        NUI::SWindowInfo(nullptr, NUI::SPoint(0, 0), NUI::SPoint(500, 100), "dialogue-test"),
        mission, nullptr, "diagnostic", vector<CObj<NWorld::CUnit>>{}, phrases);
    CObj<NUI::CText> text = new NUI::CText(
        NUI::SWindowInfo(nullptr, NUI::SPoint(0, 0), NUI::SPoint(formattedPages ? 240 : normalPages ? 120 : 500, formattedPages ? 90 : normalPages ? 50 : markupOnly ? 10 : 1), "dialogue-text"));
    CObj<NUI::CHoverButton> back = new NUI::CHoverButton, next = new NUI::CHoverButton, exit = new NUI::CHoverButton;
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[text.GetPtr()] = 1; references.refs[1] = text.GetPtr();
    references.ids[back.GetPtr()] = 2; references.refs[2] = back.GetPtr();
    references.ids[next.GetPtr()] = 3; references.refs[3] = next.GetPtr();
    references.ids[exit.GetPtr()] = 4; references.refs[4] = exit.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CMemoryStream controls;
    { CStructureSaver writer(controls, CStructureSaver::WRITE, &references);
      writer.Add(14, &text); writer.Add(15, &back); writer.Add(16, &next); writer.Add(17, &exit); }
    controls.Seek(0);
    { CStructureSaver reader(controls, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(dialog.GetPtr())->operator&(reader); }
    CObj<DialogueLayoutView> view = new DialogueLayoutView;
    CObj<NUI::IML> layout = NUI::CreateML();
    if(formattedPages) {
        auto* font=view->GetLocaleInfo()->GetFont(NGScene::SFont(Float2Int(32*formattedScale),"Courier"));
        Require(IsValid(font) && font->IsVector(),"dialogue formatting fixture did not use our packaged high-resolution Courier font");
    }
    layout->SetText((formattedPages?format->szStr:L"")+texts.front()->szStr, 0);
    layout->Generate(view, S2UI::ToPixel(text->GetSize().x));
    Require(layout->GetSize().y > text->GetSize().y, "dialogue fixture did not overflow its text box");
    if (markupOnly) {
        list<NativeUIDrawRect> bounds;
        layout->Render(&bounds, NativeUIDrawPoint(0,0), NativeUIDrawRect(0,0,0,0));
        Require(bounds.size() == 2, "markup dialogue fixture did not lay out both image bounds");
        for (const auto& bound : bounds) Require(bound.y2 <= 10, "markup fixture did not expose fitting bounds with overflowing line spacing");
        view->queries = 0; view->maxQueries = 512;
    }
    dialog->UpdatePhrases(view);
    if (normalPages) {
        auto plainText = [](const wstring& input) {
            wstring output;
            for (size_t i = 0; i < input.size(); ++i) {
                if (input[i] == L'<') {
                    const auto end = input.find(L'>', i);
                    if (end == wstring::npos) break;
                    i = end;
                } else output += input[i];
            }
            return output;
        };
        wstring recovered;
        int pages = 0, previousPhrase = -1, previousOffset = -1;
        for (; pages < 32; ) {
            dialog->SetStage(pages);
            int page, phrase, offset, phase;
            dialog->GetDisplayProgressForDiagnostics(&page, &phrase, &offset, &phase);
            Require(page == pages && phrase >= 0 && phrase <= 1 && offset >= 0,
                "normal dialogue page lost its logical progress anchor");
            if (phrase == previousPhrase) Require(offset > previousOffset, "normal dialogue continuation did not advance its source offset");
            else Require(phrase == previousPhrase + 1 && offset == 0, "new dialogue phrase did not restart its source offset");
            if(formattedPages) {
                layout->SetText(text->GetText(),0);layout->Generate(view,S2UI::ToPixel(text->GetSize().x));
                list<NativeUIDrawRect> glyphs;layout->Render(&glyphs, NativeUIDrawPoint(0,0), NativeUIDrawRect(0,0,0,0));
                float bottom=0;for(const auto& glyph:glyphs)bottom=Max(bottom,float(glyph.y2));
                const float glyphBottom=S2UI::FromPixel(bottom);
                std::printf("native formatted dialogue: page=%d phrase=%d offset=%d layoutHeight=%.1f glyphBottom=%.1f available=%d\n",
                    page,phrase,offset,S2UI::FromPixel(layout->GetSize().y),glyphBottom,text->GetSize().y);std::fflush(stdout);
                Require(glyphBottom<=text->GetSize().y,"dialogue continuation was paginated without its packaged-font formatting and its glyphs overflow when displayed");
            }
            previousPhrase = phrase; previousOffset = offset;
            recovered += plainText(text->GetText());
            ++pages;
            if (!next->GetStyle(NUI::STYLE_VISIBLE)) break;
        }
        Require(pages > 2 && pages < 32 && previousPhrase == 1,
            "normal dialogue pagination did not reach its final phrase through multiple pages");
        Require(recovered == texts[0]->szStr + texts[1]->szStr,
            "normal dialogue pagination lost or duplicated visible text");
        Require(mission->stageEvents == pages && exit->GetStyle(NUI::STYLE_VISIBLE),
            "normal dialogue pagination did not finish with one stage event per page");
        return;
    }

    dialog->SetStage(0);
    Require(text->GetText().find(texts[0]->szStr) != wstring::npos,
        "oversized dialogue line aborted subtitle pagination");
    dialog->SetStage(1);
    Require(text->GetText().find(texts[1]->szStr) != wstring::npos && mission->stageEvents == 2,
        "oversized dialogue line discarded later phrases");
    int page, phrase, offset, phase;
    dialog->GetDisplayProgressForDiagnostics(&page, &phrase, &offset, &phase);
    Require(page == 1 && phrase == 1 && offset == 0, "oversized dialogue fallback lost logical save progress");
    Require(back->GetStyle(NUI::STYLE_VISIBLE) && exit->GetStyle(NUI::STYLE_VISIBLE) && !next->GetStyle(NUI::STYLE_VISIBLE),
        "dialogue fallback did not preserve final-page navigation");
}


class CancellingMoveActor: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(CancellingMoveActor);
public:
    using CUnitServer::CUnitServer;
    mutable bool armed = false;
    mutable int steps = 0, callbacks = 0;
    mutable NAI::SPathPlace observed, cancelledAt;
    int cancelOnStep = 1;
    bool gracefulCancel = false;
    CPtr<NWorld::CCommandExecute> mover;
    CObj<NWorld::CCommandExecute>* releasingOwner = nullptr;
    bool CanFight() const override {
        if (armed) {
            const auto current = CUnitServer::GetPosition().pos.p;
            if (current.GetX() != observed.GetX() || current.GetY() != observed.GetY() || current.GetLayer() != observed.GetLayer()) {
                observed = current;
                if (++steps == cancelOnStep) {
                    armed = false; ++callbacks; cancelledAt = current;
                    auto* moving = dynamic_cast<NWorld::IExecMove*>(releasingOwner ? releasingOwner->GetPtr() : mover.GetPtr());
                    Require(moving != nullptr, "movement-callback fixture lost its mover");
                    if (gracefulCancel) (releasingOwner ? releasingOwner->GetPtr() : mover.GetPtr())->Cancel();
                    else moving->FullCancel();
                    if (releasingOwner) *releasingOwner = nullptr;
                }
            }
        }
        return CUnitServer::CanFight();
    }
};
REGISTER_SAVELOAD_CLASS(0x7f550106, CancellingMoveActor);

struct NativeMovementFixture
{
    CObj<NRPG::CGlobalGame> game;
    CObj<NWorld::CWorld> world;
    CObj<NRPG::IUnitMission> rpg;
    CObj<CancellingMoveActor> actor;
    CObj<NAI::CPath> path;
};

static NativeMovementFixture MakeNativeMovementFixture(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* source = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK()) { source = unit; break; }
    }
    Require(source != nullptr, "movement-callback fixture has no ordinary actor");
    auto* network = world->GetPathNetwork();
    vector<NAI::SPathPlace> places;
    network->GetNearPlaces(SSphere(source->GetPosition().GetCP(), 8.f), &places);
    auto position = source->GetPosition();
    bool found = false;
    for (auto place : places) {
        place.SetPose(NAI::CM_STAND); place.SetDirection(0);
        if (network->IsPassable(place) && !network->IsLocked(place, true)) {
            position.pos.p = place; found = true; break;
        }
    }
    Require(found, "movement-callback fixture has no free spawn tile");
    CObj<NRPG::IUnitMission> rpg = NRPG::CreateUnit(source->GetUnitRPG()->GetRPGUnit()->GetPers());
    CObj<CancellingMoveActor> actor = new CancellingMoveActor(world, rpg, rpg->GetModel(),
        dynamic_cast<NWorld::CPlayer*>(source->GetPlayer()), position);
    world->AddUnit(actor);
    actor->SetPosition(position);
    CObj<NAI::CPath> path;
    network->GetNearPlaces(SSphere(position.GetCP(), 5.f), &places);
    for (auto destination : places) {
        destination.SetPose(NAI::CM_STAND); destination.SetDirection(0);
        NAI::SPosition target(destination, network);
        if (fabs2(target.GetCP() - position.GetCP()) < 9.f) continue;
        path = NWorld::FindPath(network, actor, position.pos.p, vector<NAI::SPathPlace>{destination}, nullptr, true);
        if (path && path->points.size() >= 5) break;
        path = nullptr;
    }
    Require(path && path->points.size() >= 5, "movement-callback fixture needs a multi-step reachable path");
    return {game, world, rpg, actor, path};
}

static void ForcedMoveItemEligibility(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;
    for(auto& entry:units) {
        auto* unit=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK() &&
           unit->animator.CanActivateItem() && unit->GetUnitRPG()->GetInventory()->GetActive()) {
            actor=unit;break;
        }
    }
    Require(actor!=nullptr,"forced-move pose fixture has no ordinary armed actor");
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    const auto original=actor->GetPosition();
    const int ap=actor->GetAP();
    CObj<NRPG::IInventoryItem> item=actor->GetUnitRPG()->GetInventory()->GetActive();
    const bool active=actor->animator.IsActiveItem();
    const bool command=actor->HasCommand();
    const bool action=actor->IsPerformingAction();
    actor->animator.AlignTime();actor->animator.ForcedMove(original);
    Require(actor->animator.CanActivateItem(),"ordinary forced-move control disabled item activation");
    std::puts("native forced-move control: ordinary armed human remains eligible");std::fflush(stdout);
    for(auto pose:{NAI::CM_STAND,NAI::CM_CROUCH,NAI::CM_LAY}) {
        auto destination=original;destination.pos.p.SetPose(pose);
        actor->animator.AlignTime();
        const STime before=actor->animator.GetTimeEnd();
        actor->animator.Climb(original,destination,false);
        Require(actor->animator.GetTimeEnd()>before && !actor->animator.CanActivateItem(),
            "native jump-start control did not schedule its clip and enter inactive pose");
        std::printf("native jump-start control pose=%d: clip scheduled, activation blocked\n",int(pose));std::fflush(stdout);
        actor->animator.ForcedMove(destination);
        std::printf("native forced-move pose=%d: can activate=%d\n",int(pose),int(actor->animator.CanActivateItem()));
        std::fflush(stdout);
        Require(actor->animator.CanActivateItem(),"forced relocation retains stale inactive pose and blocks item activation");
        Require(actor->GetPosition()==original && actor->GetAP()==ap &&
            actor->GetUnitRPG()->GetInventory()->GetActive()==item.GetPtr() &&
            actor->animator.IsActiveItem()==active && actor->HasCommand()==command &&
            actor->IsPerformingAction()==action,
            "animator forced-move eligibility altered gameplay AP, inventory, hand state or world order");
    }
}

static void InterruptedMovementRoute(const char* databasePath,const char* resourcePath,bool restore = false)
{
    for(bool ai: {true,false}) {
        CObj<NRPG::CGlobalGame> game;
        auto world=FixtureWorld(databasePath,resourcePath,true,&game);
        vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
        NWorld::CUnitServer* actor=nullptr;
        CObj<NAI::CPath> route;
        for(auto entry: units) {
            auto* unit=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
            if(!unit || unit->IsAIUnit()!=ai || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK() || !unit->GetUnitRPG()->CanMove())continue;
            world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(unit->GetPlayer()));
            vector<NAI::SPathPlace> places;
            world->GetPathNetwork()->GetNearPlaces(SSphere(unit->GetPosition().GetCP(),6.f),&places);
            for(auto destination: places) {
                destination.SetPose(NAI::CM_STAND);destination.SetDirection(unit->GetPosition().GetDir());
                auto target=unit->GetPosition().pos;target.p=destination;
                if(!world->GetPathNetwork()->IsPassable(destination) || world->GetPathNetwork()->IsLocked(destination))continue;
                const float distance2=fabs2(target.GetCP()-unit->GetPosition().GetCP());
                if(distance2<9.f || distance2>16.f)continue;
                CObj<NWorld::CCmdPath> command=new NWorld::CCmdPath(target);
                if(unit->CanDo(command)!=NWorld::UCR_OK)continue;
                route=NWorld::FindPath(world->GetPathNetwork(),unit,unit->GetPosition().pos.p,vector<NAI::SPathPlace>{destination},nullptr,true);
                if(route && route->points.size()>=5 && route->points.size()<=7 && route->actions.empty() && std::all_of(route->points.begin(),route->points.end(),[&](const NAI::SPathPlace& p) { return p.IsIntegral() && p.GetLayer()==unit->GetPosition().pos.p.GetLayer() && p.GetPose()==NAI::CM_STAND; })) {actor=unit;break;}
                route=nullptr;
            }
            if(actor)break;
        }
        Require(actor && route,"interrupted-route fixture has no native reachable multi-step actor path");
        const auto initial=actor->GetPosition();
        auto target=initial.pos;target.p=route->points.back();
        CObj<NWorld::CCmdPath> command=new NWorld::CCmdPath(target);
        auto readOrder = [&]() {
            CStructureNetworkContext refs;auto fields=BorrowedObjectFields(actor,&refs);
            CObj<NWorld::CCommandExecute> executor;CObj<NWorld::CCmd> current;
            {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(7,&executor);reader.Add(9,&current);}
            return std::make_pair(executor,current);
        };
        auto sameCell = [](const NAI::SPathPlace& a,const NAI::SPathPlace& b) {
            return a.GetX()==b.GetX() && a.GetY()==b.GetY() && a.GetLayer()==b.GetLayer();
        };
        actor->Do(new NWorld::CCmdSetCommand(actor,command));
        auto prepared=readOrder();
        CObj<NAI::CPath> preparedPath=actor->GetCurrentPath();
        Require(prepared.first && prepared.second==command.GetPtr() && preparedPath,
            "interrupted-route fixture did not prepare its real native command and path");
        if(ai) {
            // Exercise the actual FAILED retirement branch, without inventing a
            // mock executor or depending on the AI's next tactical decision.
            prepared.first->Cancel();
            const int ap=actor->GetAP();
            actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));
            auto failed=readOrder();
            Require(prepared.first->GetState()==NWorld::CCommandExecute::FAILED && !failed.first && !failed.second &&
                !actor->HasCommand() && !actor->IsPerformingAction(),"failed AI movement retained a stale order instead of allowing its commander to decide again");
            for(int tick=0;tick<4;++tick)actor->Segment();
            Require(actor->GetAP()==ap && actor->GetPosition()==initial && !actor->HasCommand(),
                "failed AI cleanup restarted its abandoned movement or spent additional AP");
            std::puts("native interrupted route: AI native failed executor retired and command cleared");
            continue;
        }
        Require(actor->CanDo(new NWorld::CCmdContinue)==NWorld::UCR_OK,"native prepared movement cannot be explicitly continued");
        actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));
        for(int edge=0;edge<32 && sameCell(actor->GetPosition().pos.p,initial.pos.p) && actor->IsPerformingAction();++edge) {
            world->GetTime()->Set(actor->animator.GetTimeEnd());actor->Segment();
        }
        Require(!sameCell(actor->GetPosition().pos.p,initial.pos.p) && !sameCell(actor->GetPosition().pos.p,target.p) &&
            actor->IsPerformingAction() && actor->HasCommand(),"interrupted-route fixture did not start an actual intermediate native movement step");
        auto running=readOrder();
        Require(running.first && running.first->GetState()==NWorld::CCommandExecute::RUNNING,
            "interrupted-route fixture has no running native executor");
        const auto cancellingAt=actor->GetPosition();const int beforeCancelAP=actor->GetAP(),vp=actor->GetUnitRPG()->GetTotalVP();
        actor->OnTBSEvent(NWorld::TBS_CANCEL_ACTION);
        for(int edge=0;edge<32 && readOrder().first;++edge) {
            world->GetTime()->Set(actor->animator.GetTimeEnd());actor->Segment();
        }
        auto cancelled=readOrder();
        Require(!cancelled.first && running.first->GetState()==NWorld::CCommandExecute::FAILED && !actor->IsPerformingAction(),
            "interrupted-route fixture did not retire its failed native movement executor");
        const int ap=actor->GetAP();
        const auto stopped=actor->GetPosition();
        std::printf("native cancellation boundary: AP=%d->%d VP=%d->%d position=%u,%u->%u,%u\n",beforeCancelAP,ap,vp,actor->GetUnitRPG()->GetTotalVP(),cancellingAt.pos.p.GetX(),cancellingAt.pos.p.GetY(),stopped.pos.p.GetX(),stopped.pos.p.GetY());
        Require(actor->GetUnitRPG()->GetTotalVP()==vp && !sameCell(stopped.pos.p,target.p),
            "native cancellation did not stop movement before the requested destination");
        std::printf("native interrupted route: ai=%d failed=1 command=%d target=%u,%u\n",int(ai),int(actor->HasCommand()),target.p.GetX(),target.p.GetY());
        Require(cancelled.second==command.GetPtr() && actor->HasCommand(),"cancelled human movement discarded the destination needed for its pending route");
        CObj<NWorld::CWorld> retainedSourceWorld;CObj<NRPG::CGlobalGame> retainedSourceGame;
        if(restore) {
            CMemoryStream snapshot;CObj<NWorld::CUnitServer> selected=actor;
            {CStructureSaver writer(snapshot,CStructureSaver::WRITE);writer.Add(1,&game);writer.Add(2,&world);writer.Add(3,&selected);}
            snapshot.Seek(0);
            CObj<NRPG::CGlobalGame> loadedGame;CObj<NWorld::CWorld> loadedWorld;CObj<NWorld::CUnitServer> loadedActor;
            {CStructureSaver reader(snapshot,CStructureSaver::READ);reader.Add(1,&loadedGame);reader.Add(2,&loadedWorld);reader.Add(3,&loadedActor);}
            Require(loadedGame && loadedWorld && loadedActor && loadedActor.GetPtr()!=actor && loadedWorld.GetPtr()!=world.GetPtr(),
                "pending-route snapshot did not restore an independent complete native world and actor graph");
            retainedSourceWorld=world;retainedSourceGame=game;
            world=loadedWorld;game=loadedGame;actor=loadedActor;
            world->RestoreRuntimeCaches(game);
            auto restored=readOrder();
            auto* loadedCommand=dynamic_cast<NWorld::CCmdPath*>(restored.second.GetPtr());
            Require(!restored.first && loadedCommand && loadedCommand->ptDst.p==target.p && actor->HasCommand() && !actor->IsAIUnit() &&
                !actor->IsPerformingAction() && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==vp && sameCell(actor->GetPosition().pos.p,stopped.pos.p),
                "loading the pending route lost its target, changed player ownership or started/spent a native action");
            command=loadedCommand;
            std::printf("native pending-route complete world graph restored: bytes=%d command=1 idle=1 AP=%d\n",snapshot.GetSize(),actor->GetAP());
        }
        CObj<NAI::CPath> pending=actor->GetCurrentPath();
        Require(pending && !pending->points.empty() && sameCell(pending->points.back(),target.p) && !actor->IsPerformingAction(),
            "retained human movement did not rebuild an idle native pending path to its destination");
        for(int edge=0;edge<4;++edge) {world->GetTime()->Set(actor->animator.GetTimeEnd()+1);actor->Segment();}
        Require(actor->GetAP()==ap && sameCell(actor->GetPosition().pos.p,stopped.pos.p) && !actor->IsPerformingAction(),
            "querying or displaying a pending native route restarted movement without Continue");
        Require(actor->CanDo(new NWorld::CCmdContinue)==NWorld::UCR_OK,"cancelled human route cannot be continued through native command validation");
        actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));
        Require(actor->IsPerformingAction(),"native Continue did not resume a preserved human movement command");
        int continues=1;
        for(int edge=0;edge<128;++edge) {
            auto current=readOrder();
            if(!current.first) {
                if(!actor->HasCommand())break;
                Require(continues<8 && current.second==command.GetPtr() && actor->CanDo(new NWorld::CCmdContinue)==NWorld::UCR_OK,
                    "another native movement interruption lost its order or could not be explicitly continued");
                actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));++continues;
            } else {world->GetTime()->Set(actor->animator.GetTimeEnd());actor->Segment();}
        }
        std::printf("native resumed route explicit Continue requests=%d\n",continues);
        auto completed=readOrder();
        std::printf("native resumed route: at=%u,%u target=%u,%u AP=%d->%d command=%d executing=%d executorState=%d\n",actor->GetPosition().pos.p.GetX(),actor->GetPosition().pos.p.GetY(),target.p.GetX(),target.p.GetY(),ap,actor->GetAP(),int(actor->HasCommand()),int(actor->IsPerformingAction()),completed.first ? int(completed.first->GetState()) : -1);std::fflush(stdout);
        Require(sameCell(actor->GetPosition().pos.p,target.p) && !actor->HasCommand() && !actor->IsPerformingAction(),
            "continued human movement did not finish at its destination and consume the successful order");
        const int finalAP=actor->GetAP();const auto finished=actor->GetPosition();
        actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdPath(stopped.pos)));
        CObj<NAI::CPath> returnPath=actor->GetCurrentPath();
        Require(actor->HasCommand() && returnPath,"explicit-cancel control did not prepare a new native route");
        actor->Do(new NWorld::CCmdCancel(actor));
        Require(!actor->HasCommand() && !actor->GetCurrentPath() && !actor->IsPerformingAction() &&
            actor->GetAP()==finalAP && actor->GetPosition()==finished,
            "an explicit player Cancel no longer discards a prepared order without executing it");
    }
}

static void MovementFullCancelAction(const char* databasePath,const char* resourcePath)
{
    auto fixture = MakeNativeMovementFixture(databasePath,resourcePath);
    Require(!fixture.world->IsAction(),"full-cancel fixture did not begin with an idle world");
    CObj<NWorld::CActionCounter> unrelated = fixture.world->GetActiveCounter();
    Require(fixture.world->IsAction(),"full-cancel fixture failed to retain its unrelated native action");
    CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(fixture.actor,fixture.path,NAI::PF_DEFAULT);
    Require(IsValid(move),"full-cancel fixture could not create its real movement executor");
    move->Run();
    Require(move->GetState() == NWorld::CCommandExecute::RUNNING && move->IsExecuting(),
        "full-cancel control did not start a live native movement action");
    const int ap = fixture.actor->GetAP(), vp = fixture.actor->GetUnitRPG()->GetTotalVP();
    const auto position = fixture.actor->GetPosition();
    auto* mover = dynamic_cast<NWorld::IExecMove*>(move.GetPtr());
    Require(mover,"full-cancel fixture lost its native movement interface");
    mover->FullCancel();
    Require(move->GetState() == NWorld::CCommandExecute::FINISHED,
        "full cancellation did not retain the native finished-state contract");
    std::printf("native full cancellation: retainedMover=1 finished=1 executing=%d unrelatedAction=%d\n",
        move->IsExecuting(),fixture.world->IsAction());
    Require(!move->IsExecuting(),"a fully cancelled retained mover kept its native action token");
    Require(fixture.world->IsAction(),"cancelling a mover released another live world's action");
    unrelated = nullptr;
    Require(!fixture.world->IsAction() && IsValid(move),
        "a retained fully cancelled mover kept the world busy after the unrelated action ended");
    mover->FullCancel();
    Require(move->GetState() == NWorld::CCommandExecute::FINISHED && !move->IsExecuting() && !fixture.world->IsAction(),
        "repeated full cancellation restarted or retained a world action");
    Require(fixture.actor->GetAP() == ap && fixture.actor->GetUnitRPG()->GetTotalVP() == vp && fixture.actor->GetPosition() == position,
        "releasing a cancelled movement action spent additional AP, changed health or moved the actor");
}

static void MovementCallbackCancellation(const char* databasePath, const char* resourcePath, int cancelOnStep, bool releaseOwner = false)
{
    auto fixture = MakeNativeMovementFixture(databasePath, resourcePath);
    auto world = fixture.world;
    auto actor = fixture.actor;
    auto path = fixture.path;
    CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(actor, path, NAI::PF_DEFAULT);
    Require(move != nullptr, "movement-callback fixture could not create a real move executor");
    if (releaseOwner) actor->releasingOwner = &move;
    else actor->mover = move;
    actor->observed = actor->GetPosition().pos.p;
    actor->cancelOnStep = cancelOnStep;
    actor->armed = true;
    move->Run();
    for (int animation = 0; actor->callbacks == 0 && animation < 32 && move->GetState() == NWorld::CCommandExecute::RUNNING; ++animation)
        move->AnimationFinished();
    Require(actor->callbacks == 1 && actor->steps == cancelOnStep,
        "movement callback did not cancel at the requested physical step");
    Require((releaseOwner ? !move : move->GetState() != NWorld::CCommandExecute::RUNNING) &&
            actor->GetPosition().pos.p == actor->cancelledAt,
        "cancelled movement continued to a later queued tile or retained its released owner");
    actor->releasingOwner = nullptr;
    actor->mover = nullptr;
    move = nullptr;
    Require(!world->IsAction(), "cancelled movement left an action counter running");
}

struct MovementWaitWire
{
    int timer = -1;
    int operator&(CStructureSaver& saver) { saver.Add(4, &timer); return 0; }
};

static int MovementWaitTimer(NWorld::CCommandExecute* move)
{
    CStructureNetworkContext graph;
    graph.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(move)->operator&(writer); }
    saved.Seek(0);
    MovementWaitWire state;
    { CStructureSaver reader(saved, CStructureSaver::READ, &graph); reader.Add(1, &state); }
    return state.timer;
}

static void MovementFirstStepWaitTimer(const char* databasePath, const char* resourcePath)
{
    auto fixture = MakeNativeMovementFixture(databasePath, resourcePath);
    CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(fixture.actor, fixture.path, NAI::PF_DEFAULT);
    Require(IsValid(move), "first-step wait fixture could not create its native mover");
    MovementWaitWire previous; previous.timer = 123;
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE); writer.Add(1, &previous); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ);
      static_cast<CObjectBase*>(move.GetPtr())->operator&(reader); }
    Require(MovementWaitTimer(move) == 123, "first-step wait fixture did not restore its previous wait timer");
    const auto initial = fixture.actor->GetPosition().pos.p;
    auto stepped = [&]() {
        const auto current = fixture.actor->GetPosition().pos.p;
        return current.GetX() != initial.GetX() || current.GetY() != initial.GetY() || current.GetLayer() != initial.GetLayer();
    };
    move->Run();
    for (int frame = 0; frame < 32 && !stepped() && move->GetState() == NWorld::CCommandExecute::RUNNING; ++frame)
        move->AnimationFinished();
    const bool moved = stepped();
    const int timer = MovementWaitTimer(move);
    dynamic_cast<NWorld::IExecMove*>(move.GetPtr())->FullCancel();
    move = nullptr;
    Require(moved, "first-step wait fixture did not begin a physical native move");
    Require(timer == 0, "starting a new physical movement step retained the previous wait timer");
    Require(!fixture.world->IsAction(), "first-step wait fixture retained a cancelled movement action");
}

struct MovementCommandSnapshot
{
    list<CObj<CObjectBase>> queued;
    CObj<CObjectBase> current;
};

static MovementCommandSnapshot ReadMovementCommands(NWorld::CCommandExecute* move)
{
    vector<CObjectBase*> objects;
    CStructureNetworkContext gather;
    gather.include = [&](CObjectBase* object) {
        if (std::find(objects.begin(), objects.end(), object) == objects.end()) objects.push_back(object);
        return false;
    };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &gather);
      static_cast<CObjectBase*>(move)->operator&(writer); }
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.include = [](CObjectBase*) { return false; };
    unsigned id = 1;
    for (auto* object: objects) { references.ids[object] = id; references.refs[id++] = object; }
    references.nextID = id + 100;
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
      static_cast<CObjectBase*>(move)->operator&(writer); }
    saved.Seek(0);
    MovementCommandSnapshot snapshot;
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      reader.Add(2, &snapshot.queued); reader.Add(3, &snapshot.current); }
    return snapshot;
}

// Build the original wire schema from a native CCmdCancel's identical CCmdUnit
// payload. Only the selected command descriptor/body is changed; the complete
// game/world graph and every reference ID remain native CStructureSaver output.
static void MovementEndRotateWire(const char* databasePath,const char* resourcePath,bool legacy)
{
    auto fixture=MakeNativeMovementFixture(databasePath,resourcePath);
    CObj<CObjectBase> command=new NWorld::CCmdCancel(fixture.actor);
    CObj<NWorld::CCommandExecute> mover=NWorld::CreateSimpleMoveExecutor(fixture.actor,fixture.path,NAI::PF_DEFAULT);
    Require(mover!=nullptr,"end-rotate fixture could not create its native mover");
    CMemoryStream saved;
    {CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(1,&fixture.game);writer.Add(2,&fixture.world);
     writer.Add(3,&fixture.actor);writer.Add(4,&command);writer.Add(5,&mover);}
    vector<unsigned char> bytes(saved.GetSize());saved.Seek(0);saved.Read(bytes.data(),unsigned(bytes.size()));
    auto chunk=[](vector<unsigned char>& output,unsigned char id,const unsigned char* data,std::size_t size) {
        std::uint8_t length[4];std::size_t count=0;
        Require(size<=UINT32_MAX && S2FileIO::EncodeStructureLength(std::uint32_t(size),length,&count),"end-rotate fixture length encoding failed");
        output.push_back(id);output.insert(output.end(),length,length+count);output.insert(output.end(),data,data+size);
    };
    S2FileIO::StructureChunk table{},bodies{};
    for(std::size_t at=0;at<bytes.size();) {
        S2FileIO::StructureChunk field;
        Require(S2FileIO::DecodeStructureChunkAt(bytes.data(),bytes.size(),at,&field),"end-rotate fixture invalid native top-level chunk");
        Require(field.id!=3,"end-rotate fixture unexpectedly received packed output");
        if(field.id==0)table=field;if(field.id==2)bodies=field;
        at=std::size_t(field.payloadOffset)+field.length;
    }
    std::vector<S2FileIO::StructureObjectRecord> records;
    Require(table.length && bodies.length && S2FileIO::DecodeStructureObjectTable(bytes.data()+table.payloadOffset,table.length,&records),
        "end-rotate fixture cannot decode its native object table");
    unsigned wireID=0;int replaced=0;
    for(std::size_t i=0;i<records.size();++i)if(records[i].typeId==0x004c1140) {
        wireID=records[i].wireId;records[i].typeId=legacy?0x12541120:0x72312170;++replaced;
        Require(S2FileIO::EncodeStructureObjectRecord(records[i],bytes.data()+table.payloadOffset+i*9,9),"end-rotate descriptor encoding failed");
    }
    Require(replaced==1 && wireID,"end-rotate fixture did not identify exactly one native source command");
    vector<unsigned char> rewrittenBodies;
    for(std::size_t at=0;at<bodies.length;) {
        const auto* data=bytes.data()+bodies.payloadOffset;S2FileIO::StructureChunk record;
        Require(S2FileIO::DecodeStructureChunkAt(data,bodies.length,at,&record) && record.id==1,"end-rotate object record is invalid");
        const auto* object=data+record.payloadOffset;unsigned id=0;vector<unsigned char> replacement;
        for(std::size_t child=0;child<record.length;) {
            S2FileIO::StructureChunk field;
            Require(S2FileIO::DecodeStructureChunkAt(object,record.length,child,&field),"end-rotate object field is invalid");
            if(field.id==0)Require(S2FileIO::DecodeStructureScalar(object+field.payloadOffset,field.length,&id),"end-rotate object ID is invalid");
            child=std::size_t(field.payloadOffset)+field.length;
        }
        if(id==wireID) {
            for(std::size_t child=0;child<record.length;) {
                S2FileIO::StructureChunk field;S2FileIO::DecodeStructureChunkAt(object,record.length,child,&field);
                if(field.id==1) {
                    vector<unsigned char> base;chunk(base,1,object+field.payloadOffset,field.length);
                    // Original CCmdEndRotate has tag 1 (CCmdUnit) only. The old
                    // fork's rotation ender also writes tag 2: a null move ID.
                    if(legacy) {const unsigned char nullID[4]={0,0,0,0};chunk(base,2,nullID,4);}
                    chunk(replacement,1,base.data(),base.size());
                } else chunk(replacement,field.id,object+field.payloadOffset,field.length);
                child=std::size_t(field.payloadOffset)+field.length;
            }
            chunk(rewrittenBodies,record.id,replacement.data(),replacement.size());
        } else chunk(rewrittenBodies,record.id,object,record.length);
        at=std::size_t(record.payloadOffset)+record.length;
    }
    vector<unsigned char> rewritten;
    for(std::size_t at=0;at<bytes.size();) {
        S2FileIO::StructureChunk field;S2FileIO::DecodeStructureChunkAt(bytes.data(),bytes.size(),at,&field);
        if(field.id==2)chunk(rewritten,2,rewrittenBodies.data(),rewrittenBodies.size());
        else chunk(rewritten,field.id,bytes.data()+field.payloadOffset,field.length);
        at=std::size_t(field.payloadOffset)+field.length;
    }
    CMemoryStream wire;wire.Write(rewritten.data(),unsigned(rewritten.size()));wire.Seek(0);
    NativeMovementFixture restored;CObj<CObjectBase> loaded;CObj<NWorld::CCommandExecute> executor;
    std::printf("native end-rotate wire: legacy=%d type=%08x bytes=%u\n",int(legacy),legacy?0x12541120:0x72312170,unsigned(rewritten.size()));std::fflush(stdout);
    {CStructureSaver reader(wire,CStructureSaver::READ);reader.Add(1,&restored.game);reader.Add(2,&restored.world);
     reader.Add(3,&restored.actor);reader.Add(4,&loaded);reader.Add(5,&executor);}
    auto* unitCommand=dynamic_cast<NWorld::CCmdUnit*>(loaded.GetPtr());
    Require(restored.game && restored.world && restored.actor && executor && unitCommand && unitCommand->pUnit==restored.actor.GetPtr(),
        "original end-rotate command did not load with its native unit reference");
    restored.world->RestoreRuntimeCaches(restored.game);
    Require(executor->GetUnitServer()==restored.actor.GetPtr(),"end-rotate restored mover lost its owning actor");
    CStructureNetworkContext refs;refs.externalReferences=true;refs.include=[](CObjectBase*){return false;};refs.ids[loaded]=1;refs.refs[1]=loaded;refs.nextID=100;
    list<CObj<CObjectBase>> queue;queue.push_back(loaded);CObj<CObjectBase> current;
    CMemoryStream fields;
    // Replacing a queue must retain its successful completion policy: missing
    // primitive saver fields are zeroed on READ (RUNNING is not FINISHED).
    auto result=NWorld::CCommandExecute::FINISHED;
    {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);writer.Add(2,&queue);writer.Add(3,&current);writer.Add(5,&result);}
    fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(executor.GetPtr())->operator&(reader);}
    auto* actor=restored.actor.GetPtr();const auto position=actor->GetPosition();const int ap=actor->GetAP();
    actor->animator.AlignTime();const STime start=actor->animator.GetTimeEnd();executor->Run();const STime end=actor->animator.GetTimeEnd();
    std::printf("native end-rotate execution: legacy=%d duration=%u AP=%d->%d\n",int(legacy),unsigned(end-start),ap,actor->GetAP());std::fflush(stdout);
    Require(end>start,"loaded end-rotate command did not execute its native stand animation");
    executor->AnimationFinished();
    std::printf("native end-rotate completion: state=%d expected=%d AP=%d/%d samePosition=%d queue=%u current=%d\n",int(executor->GetState()),int(NWorld::CCommandExecute::FINISHED),actor->GetAP(),ap,int(actor->GetPosition()==position),unsigned(ReadMovementCommands(executor).queued.size()),int(ReadMovementCommands(executor).current!=nullptr));std::fflush(stdout);
    Require(executor->GetState()==NWorld::CCommandExecute::FINISHED && actor->GetAP()==ap && actor->GetPosition()==position,
        "end-rotate completion changed AP/location or did not finish");
    dynamic_cast<NWorld::IExecMove*>(executor.GetPtr())->FullCancel();executor=nullptr;
    Require(!restored.world->IsAction(),"retiring the end-rotate executor retained its native world action");
}

struct MovementTravelPosition
{
    NAI::SUnitPosition position;
    int operator&(CStructureSaver& saver) { saver.Add(2,&position);return 0; }
};

static void MovementBlockedWalkingTrajectory(const char* databasePath,const char* resourcePath,bool rerouteWaiting = false)
{
    auto fixture=MakeNativeMovementFixture(databasePath,resourcePath);
    auto* actor=fixture.actor.GetPtr();auto* world=fixture.world.GetPtr();auto* network=world->GetPathNetwork();
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    auto facing=actor->GetPosition();facing.pos.p.SetDirection((facing.pos.p.GetDirection()+4)&7);
    world->GetTime()->Set(1000);actor->animator.AlignTime();
    actor->SetPosition(facing);actor->animator.PlaceUnit(facing);
    fixture.path=NWorld::FindPath(network,actor,facing.pos.p,vector<NAI::SPathPlace>{fixture.path->points.back()},nullptr,true);
    Require(fixture.path && fixture.path->points.size()>=5,"blocked-walking fixture could not find its real route from the opposite facing");
    CObj<NWorld::CCommandExecute> move=NWorld::CreateSimpleMoveExecutor(actor,fixture.path,NAI::PF_DEFAULT);
    Require(move,"blocked-walking fixture could not create its native movement executor");
    const auto initial=actor->GetPosition();
    auto moving=[](CObjectBase* command) {return command && pSSClasses->GetObjectTypeID(command)==int(0x02731142);};
    move->Run();
    for(int edge=0;edge<32 && !moving(ReadMovementCommands(move).current.GetPtr()) && move->GetState()==NWorld::CCommandExecute::RUNNING;++edge) {
        world->GetTime()->Set(actor->animator.GetTimeEnd());move->AnimationFinished();
    }
    auto before=ReadMovementCommands(move);
    std::printf("native walking gate setup: moving=%d priorTurnFlag=%d position=%u,%u->%u,%u\n",int(moving(before.current.GetPtr())),
        int(actor->animator.bStandIfRecalcCommand),initial.pos.p.GetX(),initial.pos.p.GetY(),actor->GetPosition().pos.p.GetX(),actor->GetPosition().pos.p.GetY());std::fflush(stdout);
    Require(moving(before.current.GetPtr()) && actor->animator.bStandIfRecalcCommand && !(actor->GetPosition().pos.p==initial.pos.p),
        "blocked-walking fixture did not reach a real walking command after a native rotation");
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(before.current,&refs);MovementTravelPosition travel;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,&travel);}
    Require(travel.position.pos.IsValid() && !(travel.position.pos.p==actor->GetPosition().pos.p),
        "blocked-walking fixture did not read its actual pending native travel destination");
    // A real running mover has a nonwaiting conflicts remover, so a transient
    // occupancy waits rather than rerouting around an immobile fixture object.
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* blocker=nullptr;
    for(auto entry:units) {
        auto* unit=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(!unit || unit==actor || unit->IsAIUnit() || !unit->CanFight() || unit->IsWearingPK())continue;
        vector<NAI::SPathPlace> destinations;network->GetNearPlaces(SSphere(unit->GetPosition().GetCP(),5.f),&destinations);
        for(auto destination:destinations) {
            destination.SetPose(NAI::CM_STAND);destination.SetDirection(unit->GetPosition().GetDir());
            auto target=unit->GetPosition().pos;target.p=destination;
            if(fabs2(target.GetCP()-unit->GetPosition().GetCP())<4.f || !network->IsPassable(destination) || network->IsLocked(destination))continue;
            CObj<NWorld::CCmdPath> command=new NWorld::CCmdPath(target);
            if(unit->CanDo(command)!=NWorld::UCR_OK)continue;
            unit->Do(new NWorld::CCmdSetCommand(unit,command));
            unit->Do(new NWorld::CCmdSetCommand(unit,new NWorld::CCmdContinue));
            if(unit->GetPathConflictsRemover() && unit->IsPerformingAction()) {blocker=unit;break;}
            unit->Do(new NWorld::CCmdCancel(unit));
        }
        if(blocker)break;
    }
    Require(blocker,"blocked-walking fixture lacks a real prepared nonwaiting blocker");
    const auto blockerOriginal=blocker->GetPosition();const auto position=actor->GetPosition();
    // Native forced placement can occupy a future cell after a route was made.
    // Release the old two-cell reservation through the ordinary placement API.
    actor->SetPosition(position);blocker->SetPosition(travel.position);
    Require(actor->CanDoGameMove(travel.position)==NWorld::CMR_LOCKED && network->GetWhoLocksThisPlace(travel.position.pos.p)==blocker,
        "blocked-walking fixture did not produce native live-unit occupancy at the pending destination");
    const int ap=actor->GetAP(),vp=actor->GetUnitRPG()->GetTotalVP();const STime end=actor->animator.GetTimeEnd();
    world->GetTime()->Set(end);
    CStructureNetworkContext animationRefs;auto animationFields=BorrowedObjectFields(actor->animator.GetSkeletonAnimator(),&animationRefs);
    CObj<NAnimation::CASequence> sequence;
    {CStructureSaver reader(animationFields,CStructureSaver::READ,&animationRefs);reader.Add(2,&sequence);}
    Require(sequence,"blocked-walking fixture lacks its actual native skeleton animation sequence");
    CObj<NAnimation::CAnimator> scheduled=sequence->GetAnimator(end);
    Require(scheduled,"blocked-walking fixture lacks its scheduled native animation at the blocked boundary");
    move->AnimationFinished();
    auto after=ReadMovementCommands(move);NAI::SUnitPosition waiting;
    const bool parked=move->IsWaitingForPath(&waiting);const STime blockedEnd=actor->animator.GetTimeEnd();
    std::printf("native blocked walking: waiting=%d sameCommand=%d animationEnd=%d->%d AP=%d->%d\n",int(parked),
        int(after.current==before.current),int(end),int(blockedEnd),ap,actor->GetAP());std::fflush(stdout);
    Require(parked && after.current==before.current && waiting==travel.position && actor->GetPosition()==position &&
        actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==vp,
        "native walking blockage did not preserve its pending command, logical position and AP/VP");
    const bool sameTrajectory=sequence->GetAnimator(end)==scheduled.GetPtr();
    std::printf("native blocked walking animation sequence: sameScheduledAnimator=%d extraDelay=%u\n",int(sameTrajectory),unsigned(blockedEnd-end));std::fflush(stdout);
    Require(blockedEnd==end && sameTrajectory,"blocked native walking command was reseated through EndRotate instead of preserving its scheduled trajectory");
    if(rerouteWaiting) {
        auto* moving=dynamic_cast<NWorld::IExecMove*>(move.GetPtr());NAI::SPathPlace search;moving->GetSearchFromPosition(&search);
        auto samePlace=[](NAI::SPathPlace a,NAI::SPathPlace b){a.SetMoving(0);b.SetMoving(0);return a==b;};
        Require(samePlace(search,position.pos.p),"waiting reroute did not search from its actual parked position");
        search.SetMoving(0);auto target=search;target.SetPose(NAI::CM_CROUCH);
        CObj<NAI::CPath> path=NWorld::FindPath(network,actor,search,vector<NAI::SPathPlace>{target},nullptr,true,NAI::PF_USE_POSEDIR);
        Require(path && path->points.size()==2 && samePlace(path->points.front(),search) && samePlace(path->points.back(),target),
            "waiting reroute could not find its real same-cell native crouch path");
        const int beforeAP=actor->GetAP(),cost=actor->GetActionAP(NRPG::AC_POSE_CROUCH);
        moving->SetNewPath(path,NAI::PF_USE_POSEDIR);
        Require(!move->IsWaitingForPath() && samePlace(actor->GetPosition().pos.p,position.pos.p) && actor->GetAP()==beforeAP,
            "waiting reroute did not reseat without spending AP or moving into its blocker");
        const STime reseated=actor->animator.GetTimeEnd();world->GetTime()->Set(reseated);move->AnimationFinished();
        std::printf("native waiting reroute: first callback pose=%d expected=%d AP=%d->%d cost=%d extraEndTime=%u\n",
            int(actor->GetPosition().pos.p.GetPose()),int(target.GetPose()),beforeAP,actor->GetAP(),cost,unsigned(actor->animator.GetTimeEnd()-reseated));std::fflush(stdout);
        Require(actor->GetPosition().pos.p.GetPose()==target.GetPose() && actor->GetAP()==beforeAP-cost,
            "waiting reroute inserted an obsolete movement ender before the newly requested pose transition");
        for(int edge=0;edge<32 && move->GetState()==NWorld::CCommandExecute::RUNNING;++edge) {
            world->GetTime()->Set(actor->animator.GetTimeEnd());move->AnimationFinished();
        }
        Require(move->GetState()==NWorld::CCommandExecute::FINISHED && samePlace(actor->GetPosition().pos.p,target) && actor->GetAP()==beforeAP-cost &&
            blocker->GetPosition()==travel.position,"waiting reroute changed its tile/blocker, repeated its pose cost or failed to complete");
    }
    CStructureNetworkContext blockerRefs;auto blockerFields=BorrowedObjectFields(blocker,&blockerRefs);
    CObj<NWorld::CCommandExecute> blockerMove;
    {CStructureSaver reader(blockerFields,CStructureSaver::READ,&blockerRefs);reader.Add(7,&blockerMove);}
    Require(blockerMove && !blockerMove->IsWaitingForPath(),"native blocker stopped being a running nonwaiting mover");
    dynamic_cast<NWorld::IExecMove*>(blockerMove.GetPtr())->FullCancel();
    blocker->SetPosition(blockerOriginal);blocker->Do(new NWorld::CCmdCancel(blocker));
    dynamic_cast<NWorld::IExecMove*>(move.GetPtr())->FullCancel();move=nullptr;
    Require(!world->IsAction(),"blocked-walking fixture retained its cancelled native action");
}

class CollisionUnhideProbe: public CObjectBase
{
    OBJECT_NOCOPY_METHODS(CollisionUnhideProbe);
    NGlobal::CEventRegister<CollisionUnhideProbe,NWorld::CEventOnUnitUnhide> registration;
public:
    CPtr<NWorld::CDumbUnitServer> target;
    int events=0;
    CollisionUnhideProbe():registration(this,&CollisionUnhideProbe::OnUnhide) {}
    void OnUnhide(const NWorld::CEventOnUnitUnhide& event) {if(event.pWho==target)++events;}
};

static void MovementCollisionReveal(const char* databasePath,const char* resourcePath,bool sameOwner,bool hidden)
{
    auto fixture=MakeNativeMovementFixture(databasePath,resourcePath);
    auto* actor=fixture.actor.GetPtr();auto* world=fixture.world.GetPtr();auto* network=world->GetPathNetwork();
    world->GetTime()->Set(1000);actor->animator.AlignTime();actor->animator.PlaceUnit(actor->GetPosition());
    CObj<NWorld::CCommandExecute> move=NWorld::CreateSimpleMoveExecutor(actor,fixture.path,NAI::PF_DEFAULT);
    Require(move,"collision reveal fixture could not create its actual native mover");
    auto walking=[](CObjectBase* command){return command && pSSClasses->GetObjectTypeID(command)==int(0x02731142);};
    move->Run();
    for(int edge=0;edge<32 && !walking(ReadMovementCommands(move).current.GetPtr()) && move->GetState()==NWorld::CCommandExecute::RUNNING;++edge) {
        world->GetTime()->Set(actor->animator.GetTimeEnd());move->AnimationFinished();
    }
    auto pending=ReadMovementCommands(move);Require(walking(pending.current.GetPtr()),"collision reveal fixture did not reach a native pending walking step");
    CStructureNetworkContext travelRefs;auto travelFields=BorrowedObjectFields(pending.current,&travelRefs);MovementTravelPosition travel;
    {CStructureSaver reader(travelFields,CStructureSaver::READ,&travelRefs);reader.Add(1,&travel);}
    Require(travel.position.IsValid() && !(travel.position.pos.p==actor->GetPosition().pos.p),"collision reveal fixture lacks its actual future travel cell");
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* blocker=nullptr;
    for(auto entry:units) {
        auto* unit=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(unit && unit!=actor && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() &&
           (unit->GetPlayer()==actor->GetPlayer())==sameOwner) {blocker=unit;break;}
    }
    Require(blocker,"collision reveal fixture lacks an actual blocker with the required player ownership");
    CObj<CollisionUnhideProbe> probe=new CollisionUnhideProbe;probe->target=blocker;
    auto delayVisibility=[&](bool delay) {
        CMemoryStream fields;
        {CStructureSaver writer(fields,CStructureSaver::WRITE);writer.Add(53,&delay);}
        fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ);static_cast<CObjectBase*>(world)->operator&(reader);}
    };
    // Restore the real serialized coalescing flag used during CWorld::Segment.
    // The normal refresh must defer; the collision must request a forced sweep.
    delayVisibility(true);
    actor->SetPosition(actor->GetPosition());blocker->SetPosition(travel.position);
    blocker->EnableHide();blocker->Hide(hidden,false);
    Require(blocker->IsHiding()==hidden && probe->events==0,"collision reveal fixture failed to establish its native initial hide state without an event");
    auto readNight=[&]() {
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(world->GetGame(),&refs);bool night=false;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(5,&night);}return night;
    };
    const bool expectedNight=world->GetTimeOfDay()==NWorld::TOD_NIGHT;
    world->GetGame()->SetNight(!expectedNight);world->UpdateVisible(false);
    Require(readNight()!=expectedNight,"native ordinary visibility refresh did not defer under its restored coalescing flag");
    Require(actor->CanDoGameMove(travel.position)==NWorld::CMR_LOCKED && network->GetWhoLocksThisPlace(travel.position.pos.p)==blocker,
        "collision reveal fixture did not produce actual native live-unit occupancy");
    world->GetTime()->Set(actor->animator.GetTimeEnd());move->AnimationFinished();
    const bool revealed=hidden && !sameOwner;
    std::printf("native collision reveal: sameOwner=%d initiallyHidden=%d hidden=%d unhideEvents=%d forcedVisibility=%d\n",int(sameOwner),int(hidden),
        int(blocker->IsHiding()),probe->events,int(readNight()==expectedNight));std::fflush(stdout);
    Require(blocker->IsHiding()==(hidden && !revealed),"native movement collision did not reveal the other player's hidden blocker");
    Require(probe->events==(revealed?1:0),"native movement collision emitted the wrong number of actual unit-unhide events");
    Require((readNight()==expectedNight)==revealed,"native movement collision did not force visibility immediately, or refreshed an unrelated control");
    dynamic_cast<NWorld::IExecMove*>(move.GetPtr())->FullCancel();move=nullptr;
    delayVisibility(false);
    Require(!world->IsAction(),"collision reveal fixture retained its cancelled native movement action");
}

static void MovementTerminalCompletion(const char* databasePath, const char* resourcePath, bool cancelled)
{
    auto fixture = MakeNativeMovementFixture(databasePath, resourcePath);
    fixture.path->points.resize(2); // Retail completion latch is in the first-step/last-ender branch.
    CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(fixture.actor, fixture.path, NAI::PF_DEFAULT);
    Require(IsValid(move), "terminal-movement fixture could not create its native mover");
    if (cancelled) {
        fixture.actor->mover = move;
        fixture.actor->observed = fixture.actor->GetPosition().pos.p;
        fixture.actor->gracefulCancel = true;
        fixture.actor->cancelOnStep = 1;
        fixture.actor->armed = true;
    }
    move->Run();
    if (cancelled) Require(fixture.actor->callbacks == 1,
        "single-step terminal fixture did not cancel through the physical first-step callback");
    for (int frame = 0; frame < 64; ++frame) {
        auto snapshot = ReadMovementCommands(move);
        if (IsValid(snapshot.current) && snapshot.queued.empty()
                && pSSClasses->GetObjectTypeID(snapshot.current.GetPtr()) == int(0x12541120)) {
            const auto state = move->GetState();
            fixture.actor->mover = nullptr;
            move = nullptr;
            Require(state == (cancelled ? NWorld::CCommandExecute::FAILED : NWorld::CCommandExecute::FINISHED),
                "single-step end-move command did not latch the mover completion result");
            Require(!fixture.world->IsAction(), "released terminal movement retained its action counter");
            return;
        }
        Require(move->GetState() == NWorld::CCommandExecute::RUNNING,
            "terminal-movement fixture completed before exposing its final end command");
        move->AnimationFinished();
    }
    throw std::runtime_error("terminal-movement fixture never reached the last end command");
}

static void MovementActionCamera(const char* databasePath, const char* resourcePath)
{
    auto fixture = MakeNativeMovementFixture(databasePath, resourcePath);
    while (auto* pending = fixture.world->GetUICommand()) { CObj<NWorld::CUICmd> release = pending; }
    CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(fixture.actor, fixture.path, NAI::PF_DEFAULT);
    Require(IsValid(move), "movement camera fixture could not create its native mover");
    const auto initial = fixture.actor->GetPosition().pos.p;
    auto stepped = [&]() {
        const auto current = fixture.actor->GetPosition().pos.p;
        return current.GetX() != initial.GetX() || current.GetY() != initial.GetY() || current.GetLayer() != initial.GetLayer();
    };
    move->Run();
    for (int frame = 0; frame < 32 && !stepped() && move->GetState() == NWorld::CCommandExecute::RUNNING; ++frame)
        move->AnimationFinished();
    Require(stepped(), "movement camera fixture did not begin its physical move");
    auto snapshot = ReadMovementCommands(move);
    Require(IsValid(snapshot.current) && !snapshot.queued.empty()
            && pSSClasses->GetObjectTypeID(snapshot.current.GetPtr()) == int(0x02731142),
        "movement camera fixture did not prepare its next moving step");
    int cameras = 0;
    while (auto* pending = fixture.world->GetUICommand()) {
        CObj<NWorld::CUICmd> command = pending;
        auto* camera = dynamic_cast<NWorld::CUICmdUnitCamera*>(command.GetPtr());
        if (!camera || camera->pUnit != fixture.actor.GetPtr() || camera->GetPriority() != NWorld::PR_UNIT_ACTION) continue;
        ++cameras;
        Require(!camera->bUseSloMo && camera->fSloMoIncrProbability == 1.f && camera->pUnitTarget == nullptr,
            "movement camera notification changed its original framing parameters");
    }
    dynamic_cast<NWorld::IExecMove*>(move.GetPtr())->FullCancel(); move = nullptr;
    Require(cameras > 0, "native movement did not notify the automatic camera when preparing its next step");
    Require(!fixture.world->IsAction(), "movement camera fixture retained a cancelled action");
}

static void HideCooldown(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight()) { actor = unit; break; }
    }
    Require(actor != nullptr, "hide fixture has no human actor");
    CObj<NWorld::CCmdHide> command = new NWorld::CCmdHide;
    NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
    actor->EnableHide();
    CObj<NWorld::CCommandExecute> initial = NWorld::CreateExecutor(actor, command, &result);
    Require(initial && result == NWorld::UCR_OK, "fixture actor cannot hide before the cooldown");
    const int beforeAP = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    actor->Hide(false, false);
    CObj<NWorld::CCommandExecute> blocked = NWorld::CreateExecutor(actor, command, &result);
    Require(!blocked && result == NWorld::UCR_GENERAL_FAILURE,
        "Hide order was accepted during the existing hide cooldown");
    actor->EnableHide();
    CObj<NWorld::CCommandExecute> resumed = NWorld::CreateExecutor(actor, command, &result);
    Require(resumed && result == NWorld::UCR_OK, "Hide order did not resume after cooldown rearming");
    Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == beforeAP,
        "hide eligibility probes changed the unit's action points");
}

static void ExplicitHideState(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) { actor = unit; break; }
    }
    Require(actor != nullptr, "explicit Hide fixture has no ordinary human actor");
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    Require(!world->IsRealTime() && !world->IsSequence(), "explicit Hide AP fixture did not enter its native player turn");
    auto command = [](bool explicitState,bool state) {
        CObj<NWorld::CCmdHide> result = new NWorld::CCmdHide;
        CMemoryStream fields;
        { CStructureSaver writer(fields,CStructureSaver::WRITE);
          writer.Add(2,static_cast<NWorld::CCmd*>(result.GetPtr()));
          if (explicitState) { writer.Add(3,&state); writer.Add(4,&explicitState); } }
        fields.Seek(0);
        { CStructureSaver reader(fields,CStructureSaver::READ); static_cast<CObjectBase*>(result.GetPtr())->operator&(reader); }
        return result;
    };
    auto execute = [&](bool initial,bool explicitState,bool requested,bool restore) {
        actor->EnableHide(); actor->Hide(initial,false); actor->EnableHide();
        Require(actor->IsHiding() == initial, "explicit Hide fixture did not establish its native initial state");
        const auto position = actor->GetPosition();
        const int beforeAP = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
        auto request = command(explicitState,requested);
        NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
        CObj<NWorld::CCommandExecute> executor = NWorld::CreateExecutor(actor,request,&result);
        Require(executor && result == NWorld::UCR_OK, "explicit Hide fixture could not create its native executor");
        const bool expected = explicitState ? requested : !initial;
        const int cost = !initial && expected ? actor->GetActionAP(NRPG::AC_HIDE) : 0;
        Require(executor->GetStartAP() == cost && executor->GetActionAP() == cost,
            "explicit Hide charged AP for an unhide or an unchanged requested state");
        if (restore) {
            CStructureNetworkContext references;
            auto fields = BorrowedObjectFields(executor,&references);
            CMemoryStream legacyFields;
            if (!explicitState) {
                { CStructureSaver writer(legacyFields,CStructureSaver::WRITE,&references);
                  writer.Add(2,executor.GetPtr()); }
                legacyFields.Seek(0);
            }
            // This one-shot executor has no saveload class ID. Recreate through
            // its real factory and restore fields with borrowed live references.
            auto defaultRequest = command(false,true);
            CObj<NWorld::CCommandExecute> restored = NWorld::CreateExecutor(actor,defaultRequest,&result);
            Require(IsValid(restored), "Hide executor could not be recreated through its native factory");
            { CStructureSaver reader(explicitState ? fields : legacyFields,CStructureSaver::READ,&references);
              restored->operator&(reader); }
            executor = restored;
            Require(executor->GetStartAP() == cost && executor->GetActionAP() == cost,
                "restoring Hide lost its actor, requested state or cost");
        }
        Require(actor->IsHiding() == initial && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == beforeAP,
            "creating/restoring Hide changed live hiding or AP before execution");
        executor->Run();
        std::printf("native Hide explicit=%d initial=%d requested=%d restored=%d actual=%d\n",
            int(explicitState),int(initial),int(requested),int(restore),int(actor->IsHiding()));
        Require(actor->IsHiding() == expected, "native Hide toggled instead of applying the explicit requested state");
        Require(executor->GetState() == NWorld::CCommandExecute::FINISHED,
            "native Hide did not finish its one-shot action");
        Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == beforeAP-cost,
            "Hide spent a different number of turn-based AP than its unchanged native action cost");
        Require(actor->GetPosition().pos.p == position.pos.p && actor->GetPosition().pos.pNet == position.pos.pNet,
            "native Hide changed the actor's location");
        executor = nullptr;
        Require(!world->IsAction(), "finished Hide retained an action counter");
    };
    // Exercise both the new native constructor and registered command graph
    // round trip; the matrix below separately probes legacy fields and executor fields.
    for (bool requested: {false,true}) {
        CObj<NWorld::CCmdHide> original = new NWorld::CCmdHide(requested), restored;
        Require(original->bExplicitState && original->bState == requested,
            "explicit Hide constructor did not retain its requested state");
        CMemoryStream wire;
        { CStructureSaver writer(wire,CStructureSaver::WRITE); writer.Add(1,&original); }
        wire.Seek(0);
        { CStructureSaver reader(wire,CStructureSaver::READ); reader.Add(1,&restored); }
        Require(restored && restored.GetPtr() != original.GetPtr() &&
            restored->bExplicitState && restored->bState == requested,
            "registered Hide command graph lost its explicit state");
    }
    // The no-argument legacy command must retain its live toggle semantics.
    execute(false,false,true,false); execute(true,false,true,true);
    for (bool restore: {false,true})
        for (bool requested: {false,true})
            for (bool initial: {false,true}) execute(initial,true,requested,restore);
}
