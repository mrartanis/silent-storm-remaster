static void TemporaryGroups()
{
    CObj<NWorld::CWorld> world = new NWorld::CWorld;
    CObj<NWorld::CUnitGroup> temporary = world->CreateUnitGroup(42);
    Require(temporary->GetID() == 42, "temporary group lost its requested ID");
    Require(!world->GetUnitGroup(42), "temporary group leaked into the world registry");
    for (int i = 0; i < 100; ++i) {
        CObj<NWorld::CUnitGroup> query = world->CreateUnitGroup(-1);
        Require(!world->GetUnitGroup(query->GetID()), "query claimed a reinforcement ID");
    }
}

static void ScrollInitialization()
{
    for (unsigned char pattern : {0xff, 0x11}) {
        void* memory = ::operator new(sizeof(NUI::CScrollWindowBase));
        std::memset(memory, pattern, sizeof(NUI::CScrollWindowBase));
        CObj<NUI::CScrollWindowBase> scroll =
            new(memory) NUI::CScrollWindowBase(NUI::SWindowInfo());
        Require(scroll->GetValue().x == 0 && scroll->GetValue().y == 0,
            "new paper window inherits stale scroll offsets");
    }
}

class AttackMission: public NGame::CMissionBase
{
    OBJECT_BASIC_METHODS(AttackMission);
public:
    int probes = 0;
    int selected = 0;
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    int CountSelected() override { return selected; }
    void GetSelectedUnits(vector<CPtr<NGame::IUnitTracker>>* result) const override { result->clear(); }
    bool IsRealTime() const override { return false; }
    bool GetTracePosition(CVec3* position) const override { *position = VNULL3; return true; }
    void CanDoCommand(NWorld::CCmd*, bool, NGame::SActionInfo* info) override {
        ++probes;
        *info = NGame::SActionInfo();
    }
};

class AttackState: public NGame::CStateAttack
{
public:
    NWorld::CCmd* TargetCommand() { return GetTargetCmd(); }
    void Attach(NGame::IMission* mission) { CStateBase::Initialize(mission); }
    void RefreshCursor() { UpdateCursor(); }
    void RefreshCaption() { UpdateCursorInfo(); }
};

class AttackCursorUnit: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(AttackCursorUnit);
public:
    EState cursorState = ST_NORMAL_MELEE;
    EState GetState() override { return cursorState; }
};

class AttackCursorMission: public AttackMission
{
    OBJECT_NOCOPY_METHODS(AttackCursorMission);
public:
    vector<CPtr<NGame::IUnitTracker>> selection;
    bool traceAvailable = true;
    void GetSelectedUnits(vector<CPtr<NGame::IUnitTracker>>* result) const override { *result = selection; }
    int CountSelected() override { return static_cast<int>(selection.size()); }
    bool GetTracePosition(CVec3* result) const override { *result = VNULL3; return traceAvailable; }
};

class ToolTargetPlayer: public NWorld::CPlayer
{
    OBJECT_NOCOPY_METHODS(ToolTargetPlayer);
public:
    list<CPtr<CObjectBase>> traps;
    void GetTrappedObjectsList(list<CPtr<CObjectBase>>* result) const override { *result = traps; }
};

class ToolTargetPlayerTracker: public NGame::CPlayerTracker
{
    OBJECT_NOCOPY_METHODS(ToolTargetPlayerTracker);
public:
    CObj<ToolTargetPlayer> player = new ToolTargetPlayer;
    NWorld::IPlayer* GetPlayer() const override { return player; }
};

class ToolTargetMission: public AttackCursorMission
{
    OBJECT_NOCOPY_METHODS(ToolTargetMission);
public:
    CObj<ToolTargetPlayerTracker> player = new ToolTargetPlayerTracker;
    CPtr<CObjectBase> target;
    NGame::EUnitAction requested = NGame::UA_USE;
    NGame::SActionInfo general, targeted;
    int commands = 0, targetQueries = 0;
    ToolTargetMission() { general.SetValid(); targeted.SetValid(); targeted.nMinAP = targeted.nMaxAP = 7; }
    NGame::IPlayerTracker* GetActivePlayer() const override { return player; }
    CObjectBase* GetStateTarget(bool* fromWorld = nullptr) const override {
        if (fromWorld) *fromWorld = true;
        return target;
    }
    void GetActionInfo(NGame::EUnitAction action,NGame::SActionInfo* result) override {
        requested = action;
        *result = action == NGame::UA_USETOOL ? general : NGame::SActionInfo();
    }
    void CanDoCommand(NWorld::CCmd* command,bool,NGame::SActionInfo* result) override {
        auto* untrap = dynamic_cast<NWorld::CCmdUntrapObject*>(command);
        Require(untrap && untrap->pTarget == target.GetPtr(), "tool cursor probed the wrong command or hovered target");
        ++targetQueries; *result = targeted;
    }
    void Command(NWorld::CUnit*,NWorld::CCmd*,bool) override { ++commands; }
};

static void ForcedToolTargetingEntry(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    CObj<ToolTargetMission> mission = new ToolTargetMission;
    CObj<AttackCursorUnit> unit = new AttackCursorUnit;
    unit->cursorState = NWorld::CUnit::ST_NORMAL_TOOL;
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission,unit);
    mission->selection.push_back(tracker.GetPtr());
    CObj<NGame::CStateUntrap> state = new NGame::CStateUntrap(true);
    const bool entered = state->Initialize(mission);
    Require(mission->requested == NGame::UA_USETOOL,
        "forced disarm targeting queried generic Use eligibility instead of UseTool");
    Require(entered && state->GetType() == NGame::IState::FORCED,
        "forced disarm targeting could not begin before a trapped object was hovered");
    Require(state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK) && mission->targetQueries == 0,
        "tool targeting without an object did not begin with a blocked cursor");
    Require(state->OnLButtonUp(0,0) && mission->commands == 0,
        "clicking empty tool-targeting space queued a command");
    CObj<NGame::CStateUntrap> hover = new NGame::CStateUntrap(false);
    Require(!hover->Initialize(mission), "automatic disarm hover mode entered without a known trap");
    mission->general = NGame::SActionInfo();
    CObj<NGame::CStateUntrap> denied = new NGame::CStateUntrap(true);
    Require(!denied->Initialize(mission), "forced tool mode ignored failed general action eligibility");
    mission->general.eResult = NWorld::UCR_PENDING;
    CObj<NGame::CStateUntrap> pending = new NGame::CStateUntrap(true);
    Require(pending->Initialize(mission), "forced tool mode lost the remaster's asynchronous pending eligibility");
    mission->general.SetValid();
    // The general action query owns selection/tool eligibility, as for the other targeting modes.
    unit->cursorState = NWorld::CUnit::ST_NORMAL_RIFLE;
    mission->selection.push_back(tracker.GetPtr());
    CObj<NGame::CStateUntrap> aggregate = new NGame::CStateUntrap(true);
    Require(aggregate->Initialize(mission), "tool targeting overrode the mission's aggregate action eligibility");
    Require(mission->commands == 0, "entering or rejecting tool mode issued gameplay commands");
}

static void ToolTargetingCursorRefresh(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    Require(NDb::GetUICursor(NGame::N_CURSOR_USE_TOOL) && NDb::GetUICursor(NGame::N_CURSOR_BLOCK),
        "tool cursor fixture has no authored tool or blocked cursor");
    CObj<ToolTargetMission> mission = new ToolTargetMission;
    CObj<NWorld::CWindowDoor> trap = new NWorld::CWindowDoor;
    mission->target = trap.GetPtr(); mission->player->player->traps.push_back(trap.GetPtr());
    CObj<NGame::CStateUntrap> state = new NGame::CStateUntrap(true);
    Require(state->Initialize(mission) && mission->targetQueries == 1 &&
        state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_USE_TOOL),
        "tool cursor control did not start with a valid known trap and its real tool cursor");
    const auto caption = [&](int ap) {
        return NUI::GetDBString(20276)+NUI::GetDBString(19808)+std::to_wstring(ap);
    };
    Require(state->GetCursorInfo().wsText == caption(7), "initial native tool cursor lost its action-point caption");
    mission->targeted.bOk = false;
    state->Step();
    Require(mission->targetQueries == 2 && state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK) &&
        state->GetCursorInfo().wsText.empty(),
        "the native disarm cursor retained valid targeting after the hovered action became unavailable");
    mission->targeted.SetValid(); mission->targeted.nMinAP = mission->targeted.nMaxAP = 11;
    state->Step();
    Require(mission->targetQueries == 3 && state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_USE_TOOL) &&
        state->GetCursorInfo().wsText == caption(11), "disarm cursor did not recover with the current target's action-point cost");
    mission->target = nullptr;
    state->Step();
    Require(mission->targetQueries == 3 && state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK) &&
        state->GetCursorInfo().wsText.empty(), "moving off a trap retained its tool cursor or action-point caption");
    mission->target = trap.GetPtr(); mission->player->player->traps.clear();
    state->Step();
    Require(mission->targetQueries == 3 && state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK),
        "tool cursor exposed an unknown trap to its targeting probe");
    CObj<NGame::CStateUntrap> hover = new NGame::CStateUntrap(false);
    Require(!hover->Initialize(mission), "automatic hover state entered for an unknown trap");
    mission->player->player->traps.push_back(trap.GetPtr());
    Require(hover->Initialize(mission) && hover->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_USE_TOOL),
        "automatic disarm hover did not activate for a known valid trap");
    Require(mission->commands == 0, "refreshing a disarm cursor issued a gameplay command");
}

static void SetAttackPreview(AttackState* state, const NGame::SActionInfo& info)
{
    CMemoryStream stream;
    auto value = info;
    auto location = NAI::HL_ANY;
    { CStructureSaver writer(stream, CStructureSaver::WRITE); writer.Add(3, &value); writer.Add(5, &location); }
    stream.Seek(0);
    { CStructureSaver reader(stream, CStructureSaver::READ); static_cast<CObjectBase*>(state)->operator&(reader); }
}

static void MeleeAttackCursor(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    CObj<AttackCursorMission> mission = new AttackCursorMission;
    CObj<AttackCursorUnit> unit = new AttackCursorUnit;
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission, unit);
    mission->selection.push_back(tracker.GetPtr());
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    NGame::SActionInfo info;
    info.SetValid();
    SetAttackPreview(state, info);
    NInput::SEvent event;
    while (NInput::GetEvent(&event)) {}
    for (auto entry : {std::make_pair(NWorld::CUnit::ST_NORMAL_MELEE, 8),
                       std::make_pair(NWorld::CUnit::ST_NORMAL_KNIFE, 9),
                       std::make_pair(NWorld::CUnit::ST_NORMAL_RIFLE, 10)}) {
        Require(NDb::GetUICursor(entry.second) != nullptr, "fixture attack cursor record is absent");
        unit->cursorState = entry.first;
        state->RefreshCursor();
        Require(state->GetCursorInfo().pCursor == NDb::GetUICursor(entry.second),
            "attack cursor did not distinguish fists, knife and firearm");
    }
    info.eResult = NWorld::UCR_OK_RELOAD;
    SetAttackPreview(state, info);
    state->RefreshCursor();
    Require(state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_RELOAD),
        "attack cursor lost replacement-reload priority");
    info.bOk = false;
    info.eResult = NWorld::UCR_UNAVAILABLE;
    SetAttackPreview(state, info);
    state->RefreshCursor();
    Require(state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK),
        "unavailable attack did not retain the blocked cursor");
}

static void AttackCaptionWithoutHitChance(const char* databasePath, bool reload, bool missingTarget = false)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    CObj<AttackCursorMission> mission = new AttackCursorMission;
    mission->traceAvailable = !missingTarget;
    CObj<AttackCursorUnit> unit = new AttackCursorUnit;
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission, unit);
    mission->selection.push_back(tracker.GetPtr());
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    NGame::SActionInfo info;
    info.bValid = info.bAvailable = true;
    info.bOk = info.bEnoughAP = reload || missingTarget;
    info.nMinAP = reload ? 5 : 17;
    info.nMaxAP = reload ? 5 : 21;
    info.eResult = reload ? NWorld::UCR_OK_RELOAD : missingTarget ? NWorld::UCR_OK : NWorld::UCR_TARGET_OUT_OF_RANGE;
    SetAttackPreview(state, info);
    state->RefreshCaption();
    auto caption = state->GetCursorInfo().wsText;
    const auto expected = NUI::GetDBString(info.bEnoughAP ? 20276 : 19807) + NUI::GetDBString(19808)
        + (reload ? L"5" : L"17-21");
    Require(caption == expected, missingTarget
        ? "attack preview displayed a percentage without a valid attack target"
        : reload
        ? "replacement reload preview displayed a shooting percentage instead of only its AP cost"
        : "unavailable attack preview did not preserve its AP cost without a shooting percentage");
}

static void BindInitialization()
{
    for (unsigned char pattern : {0x01, 0x11}) {
        alignas(NInput::SBindCommand) unsigned char storage[sizeof(NInput::SBindCommand)];
        std::memset(storage, pattern, sizeof(storage));
        auto* command = new(storage) NInput::SBindCommand;
        // Inspect the representation instead of reading an indeterminate bool.
        Require(storage[offsetof(NInput::SBindCommand, bActive)] == 0,
            "new input binding inherits an active flag from allocator memory");
        command->~SBindCommand();
    }
    Require(!NInput::CBind("upstream_fresh_binding").IsActive(),
        "new public binding starts active before the first input frame");
}

static void AttackLocationRefresh()
{
    CObj<AttackMission> mission = new AttackMission;
    vector<CPtr<NGame::IUnitTracker>> selection;
    mission->GetSelectedUnits(&selection);
    Require(selection.empty(), "the worldless attack-location fixture unexpectedly selected native units");
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    // Sample an input frame before updating the cursor, as the game does.
    NInput::SEvent event;
    while (NInput::GetEvent(&event)) {}
    for (const char* name : {"hitlocation_head", "hitlocation_body", "hitlocation_larm",
                            "hitlocation_rarm", "hitlocation_lleg", "hitlocation_rleg"})
        Require(!NInput::CBind(name).IsActive(), "fixture retained a targeting key");
    // Restore a state aimed at the head. With no held targeting key, the next
    // cursor update changes it to HL_ANY, just as releasing the targeting key.
    CMemoryStream stream;
    NAI::EHitLocation location = NAI::HL_HEAD;
    { CStructureSaver saver(stream, CStructureSaver::WRITE); saver.Add(5, &location); }
    stream.Seek(0);
    { CStructureSaver saver(stream, CStructureSaver::READ);
      static_cast<CObjectBase*>(state.GetPtr())->operator&(saver); }
    state->RefreshCursor();
    Require(mission->probes == 1, "changed aim location retained stale attack availability");
    state->RefreshCursor();
    Require(mission->probes == 1, "unchanged aim location needlessly refreshed availability");
}

static void FlyingRestoration()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    NAI::SPathPlace flight(10, 10, 200);
    flight.SetFinal(1); // Layer encodes altitude, not a ground-layer index.
    Require(network->IsPassable(flight), "flying pocket restore treated altitude as a ground layer");
    Require(network->GetPassability(flight) == NAI::AIP_YES, "flying passability treated altitude as a ground layer");
    flight.SetIntegral(0);
    Require(network->IsPassable(flight), "flying altitude was treated as a ladder index");
}

template<class T>
static auto SnipingCommand(int, NAI::EHitLocation location)
    -> decltype(new T(nullptr, nullptr, location))
{
    return new T(nullptr, nullptr, location);
}

template<class T>
static auto SnipingCommand(long, NAI::EHitLocation)
    -> decltype(new T(nullptr, nullptr)) { return new T(nullptr, nullptr); }

static void SnipingAim()
{
    for (auto expected : {NAI::HL_HEAD, NAI::HL_LLEG, NAI::HL_ANY}) {
        CObj<NWorld::CExecSnipeAim> command = SnipingCommand<NWorld::CExecSnipeAim>(0, expected);
        CMemoryStream stream;
        { CStructureSaver saver(stream, CStructureSaver::WRITE);
          static_cast<CObjectBase*>(command.GetPtr())->operator&(saver); }
        stream.Seek(0);
        NAI::EHitLocation actual = NAI::HL_BODY;
        { CStructureSaver saver(stream, CStructureSaver::READ); saver.Add(3, &actual); }
        Require(actual == expected, "sniping preparation replaced the requested aim point with the body");
    }
}

class ExplosionDecal: public NGScene::CExplosionDecalGeometry
{
public:
    ExplosionDecal(): CExplosionDecalGeometry(nullptr, {}, VNULL3, 10.f, 0.f) {}
    void Rebuild(NGScene::CObjectInfo::SData* result, const NGScene::CObjectInfo& source) {
        CExplosionDecalGeometry::Recalc(result, source, source.GetPositions());
    }
};

static void GeometryUnusedVertices(bool weighted)
{
    auto vertex=[](const CVec3& position,int marker) {
        NGScene::SVertex result{};result.pos=position;result.tex=CVec2(marker*0.125f,(marker+1)*0.0625f);
        CVec3 normal(marker*0.1f,0,1);Normalize(&normal);
        NGfx::CalcCompactVector(&result.normal,normal);NGfx::CalcCompactVector(&result.texU,CVec3(1,0,0));NGfx::CalcCompactVector(&result.texV,CVec3(0,1,0));return result;
    };
    auto weight=[](int marker) {
        NGScene::SVertexWeight result{};result.fWeights[0]=0.75f;result.fWeights[1]=0.25f;
        result.cBoneIndices[0]=BYTE(marker);result.cBoneIndices[1]=BYTE(marker+1);return result;
    };
    for(int triangleCount:{1,16}) {
        NGScene::CObjectInfo::SData compact;
        compact.verts={vertex(CVec3(10,20,30),1),vertex(CVec3(12,20,30),2),vertex(CVec3(10,22,30),3)};
        if(weighted)compact.weights={weight(1),weight(2),weight(3)};
        compact.geometry.SetTriangles(vector<STriangle>(triangleCount,STriangle(0,1,2)));
        CObj<NGScene::CObjectInfo> reference=new NGScene::CObjectInfo;reference->Assign(compact);SBound expected;reference->CalcBound(&expected);
        Require(reference->GetVertices().size()==3 && reference->GetPositions().size()==3 && reference->GetTrisCount()==triangleCount &&
            fabs2(expected.s.ptCenter-CVec3(11,21,30))<1e-10f && fabs2(expected.ptHalfBox-CVec3(1,1,0))<1e-10f,
            "native compact geometry control lost its actual triangle or bounds");
        // Interior holes fail deterministically before the old implementation
        // can reach the trailing-unused case, whose reorder vector is shorter.
        for(int trailing:{0,3}) {
            NGScene::CObjectInfo::SData data;
            for(int i=0;i<6+trailing;++i) {
                data.verts.push_back(vertex(CVec3(-100-i,-100-i,-100-i),i+4));
                if(weighted)data.weights.push_back(weight(i+4));
            }
            for(int i=0;i<3;++i) {data.verts[i*2+1]=compact.verts[i];if(weighted)data.weights[i*2+1]=compact.weights[i];}
            data.geometry.SetTriangles(vector<STriangle>(triangleCount,STriangle(1,3,5)));
            CObj<NGScene::CObjectInfo> mesh=new NGScene::CObjectInfo;mesh->Assign(data);SBound actual;mesh->CalcBound(&actual);
            std::printf("native unused geometry: weighted=%d triangles=%d trailing=%d input=%u vertices=%u positions=%u boundCenter=%.3f,%.3f,%.3f expected=11,21,30\n",
                int(weighted),triangleCount,trailing,unsigned(data.verts.size()),unsigned(mesh->GetVertices().size()),unsigned(mesh->GetPositions().size()),
                double(actual.s.ptCenter.x),double(actual.s.ptCenter.y),double(actual.s.ptCenter.z));std::fflush(stdout);
            Require(fabs2(actual.s.ptCenter-expected.s.ptCenter)<1e-10f && fabs2(actual.ptHalfBox-expected.ptHalfBox)<1e-10f && std::fabs(actual.s.fRadius-expected.s.fRadius)<1e-6f,
                "unreferenced mesh vertices expanded the actual native render bounds beyond its triangles");
            Require(mesh->GetVertices().size()==3 && mesh->GetPositions().size()==3 && mesh->GetTrisCount()==triangleCount &&
                mesh->GetWeights().size()==reference->GetWeights().size(),"native geometry compaction retained unused vertex/weight slots or changed its triangle count");
            for(int i=0;i<3;++i) {
                Require(mesh->GetPositions()[i]==reference->GetPositions()[i] &&
                    std::memcmp(&mesh->GetVertices()[i],&reference->GetVertices()[i],sizeof(NGScene::SUVInfo))==0,
                    "native geometry compaction changed referenced positions, UVs or packed tangent data");
                if(weighted)Require(std::memcmp(&mesh->GetWeights()[i],&reference->GetWeights()[i],sizeof(NGScene::SRealVertexWeight))==0,
                    "native geometry compaction changed referenced bone indices or weights");
            }
            vector<STriangle> actualTriangles,expectedTriangles;mesh->GetPosTriangles(&actualTriangles);reference->GetPosTriangles(&expectedTriangles);
            Require(actualTriangles.size()==expectedTriangles.size(),"native compaction lost a triangle");
            for(std::size_t i=0;i<actualTriangles.size();++i)Require(actualTriangles[i].i1==expectedTriangles[i].i1 &&
                actualTriangles[i].i2==expectedTriangles[i].i2 && actualTriangles[i].i3==expectedTriangles[i].i3,
                "native compaction changed triangle winding or position references");
        }
    }
}

static void DecalRebuild()
{
    NGScene::CObjectInfo::SData data;
    for (auto position : {CVec3(-1,-1,0), CVec3(1,-1,0), CVec3(0,1,0)}) {
        NGScene::SVertex vertex{};
        vertex.pos = position;
        data.verts.push_back(vertex);
    }
    data.geometry.SetTriangles({STriangle(0,1,2)});
    CObj<NGScene::CObjectInfo> source = new NGScene::CObjectInfo;
    source->AssignFast(data);
    CObj<ExplosionDecal> decal = new ExplosionDecal;
    NGScene::CObjectInfo::SData result;
    decal->Rebuild(&result, *source);
    const auto count = result.verts.size();
    Require(count == 3, "invalid explosion decal test fixture");
    for (int reload = 0; reload < 20; ++reload) {
        decal->Rebuild(&result, *source);
        Require(result.verts.size() == count, "decal rebuild appends another copy of saved vertices");
        Require(result.geometry.indices.size() == 3, "decal triangle count changed on rebuild");
    }
}

template<class T> class EmptyFogTextureNode: public CPtrFuncBase<T>
{
protected:
    bool NeedUpdate() override { return false; }
    void Recalc() override {}
public:
    EmptyFogTextureNode() { this->pValue = nullptr; }
};

static void LightmappedDecalEligibility()
{
    using namespace NGScene;
    CObj<EmptyFogTextureNode<NGfx::CTexture>> texture=new EmptyFogTextureNode<NGfx::CTexture>;
    auto make=[&](int mode) { return CreateMaterial(CVec3(1,1,1),texture,nullptr,0,CVec3(0,0,0),
        nullptr,nullptr,nullptr,0,0,mode); };
    CObj<IMaterial> opaque=make(MA_OPAQUE),overlay=make(MA_OVERLAY),exact=overlay->GetExactDecal();
    Require(opaque->IsSolid() && overlay->GetMaterialInfo().mt==SMaterialInfo::DECAL &&
        exact && exact->GetMaterialInfo().mt==SMaterialInfo::EXACT_DECAL,
        "lightmapped fixture did not create genuine opaque/overlay/exact native materials");
    // The selector and command builder use only lightmap ownership metadata;
    // no pixels from this genuine, unallocated texture object are accessed.
    CObj<NGfx::CTexture> lightmap=new NGfx::CTexture;
    CSceneFragments scene;SRenderGeometryInfo geometry;SBound bound;bound.BoxInit(VNULL3,CVec3(1,1,1));
    const int index=scene.AddGeometry(nullptr,&geometry,bound);CPartFlags parts;parts.Clear();parts.Set(0);
    scene.AddElement(index,parts,opaque,lightmap,nullptr);
    scene.AddElement(index,parts,overlay,lightmap,nullptr);
    scene.AddElement(index,parts,exact,lightmap,nullptr);
    scene.AddElement(index,parts,opaque,nullptr,nullptr);
    Require(scene.GetFragments().size()==5,"lightmapped fixture merged distinct native material/lightmap fragments");
    CRenderCmdList visible;MakeSingleOp(&visible,scene,false,RO_DIR_DEPTH);
    Require(visible.ops.size()==4,"native unfiltered scene lost a source surface");
    {
        CSelectFragments lightmapped(&scene,SLightmappedFilter());
        CRenderCmdList commands;MakeSingleOp(&commands,scene,false,RO_DIR_DEPTH);
        int decals=0;
        for(const auto& command:commands.ops)if(command.pFrag->pMaterial->IsDecal())++decals;
        std::printf("native lightmapped draw list: commands=%u decalSurfaces=%d expectedOnlyOpaqueLightmapped=1\n",
            unsigned(commands.ops.size()),decals);std::fflush(stdout);
        Require(commands.ops.size()==1 && decals==0 && commands.ops.front().pFrag->pMaterial==opaque.GetPtr() &&
            commands.ops.front().pFrag->pLightmap==lightmap.GetPtr(),
            "native lightmap selection submitted overlay/exact decal surfaces into its depth draw list");
    }
    CRenderCmdList restored;MakeSingleOp(&restored,scene,false,RO_DIR_DEPTH);
    Require(restored.ops.size()==4,"native lightmap selection permanently hid decals from later surface passes");
    {
        CSelectFragments ordinary(&scene,SNonLightmappedFilter());
        CRenderCmdList commands;MakeSingleOp(&commands,scene,false,RO_DIR_DEPTH);
        Require(commands.ops.size()==1 && commands.ops.front().pFrag->pMaterial==opaque.GetPtr() &&
            commands.ops.front().pFrag->pLightmap==nullptr,
            "native decal exclusion changed the separate non-lightmapped surface path");
    }
}

static void FogSurfaceEligibility()
{
    using namespace NGScene;
    CObj<EmptyFogTextureNode<NGfx::CTexture>> texture = new EmptyFogTextureNode<NGfx::CTexture>;
    CObj<EmptyFogTextureNode<NGfx::CCubeTexture>> sky = new EmptyFogTextureNode<NGfx::CCubeTexture>;
    auto make = [&](int mode) {
        return CreateMaterial(CVec3(1,1,1), texture, nullptr, 0, CVec3(0,0,0),
            nullptr, nullptr, nullptr, 0, 0, mode);
    };
    struct Case { const char* name; CObj<IMaterial> material; bool fog; };
    CObj<IMaterial> decal = make(MA_OVERLAY);
    std::vector<Case> cases = {
        {"opaque",make(MA_OPAQUE),true},
        {"alpha-tested solid",make(MA_ALPHA_TEST),true},
        {"self-illuminated solid",make(MA_SELF_ILLUM),true},
        {"decal overlay",decal,false},
        {"exact decal",decal->GetExactDecal(),false},
        {"transparent",make(MA_TRANSPARENT),false},
        {"occluder",CreateOccluderMaterial(),false},
        {"alien fallback",CreateAlienMaterial(sky),false},
        {"explosion decal",CreateExplosionDecal(texture),false}
    };
    Require(decal->IsDecal() && !decal->IsSolid() && cases[4].material->IsDecal(),
        "fog fixture did not construct genuine native decal materials");
    for(auto path : {RP_FASTEST,RP_GF3_CL}) for(auto& item : cases) {
        CSceneFragments fragments;
        SRenderGeometryInfo geometry;SBound bounds;bounds.BoxInit(CVec3(0,0,0),CVec3(1,1,1));
        const int index = fragments.AddGeometry(nullptr,&geometry,bounds);
        CPartFlags flags;flags.Clear();flags.Set(0);
        fragments.AddElement(index,flags,item.material,nullptr,nullptr);
        Require(fragments.GetFragments().size()==2 && fragments.HasSelectedFragments(),
            "fog fixture did not retain its native scene fragment");
        SFogParams fog{};
        CRenderCmdList materialOnly;
        AddFinalOps(&materialOnly,fragments,path,RO_NOP,0.f);
        Require(materialOnly.ops.size()==(std::string(item.name)=="alien fallback" ? 1u : 0u),
            "unexpected native material-only operations in fog fixture");
        for(auto operation : {RO_FOG_STATIC,RO_FOG_DYNAMIC}) {
            CRenderCmdList commands;AddFinalOps(&commands,fragments,path,operation,&fog);
            int count=0,other=0;
            for(const auto& command : commands.ops) {
                Require(command.pFrag==fragments.GetFragments()[1],
                    "final fog/material operation lost its genuine scene fragment");
                if(command.op==operation) {
                    ++count;
                    Require(command.nPass==50 && command.nStencilBlendMode==(ABM_SMART|DPM_EQUAL) &&
                        command.nDestRegister==0 && command.p1.pFog==&fog,
                        "fog surface operation changed pass, depth/blend policy or parameters");
                } else {
                    Require(other<int(materialOnly.ops.size()),"fog changed native material operation count");
                    const auto& control=materialOnly.ops[other++];
                    Require(command.op==control.op && command.nPass==control.nPass &&
                        command.nStencilBlendMode==control.nStencilBlendMode &&
                        command.nDestRegister==control.nDestRegister &&
                        command.p1.pVec4==control.p1.pVec4,
                        "fog changed the existing native alien fallback operation");
                }
            }
            std::printf("native fog material=%s path=%d operation=%d fog=%d expected=%d materialOps=%d\n",
                item.name,int(path),int(operation),count,int(item.fog),other);std::fflush(stdout);
            Require(count==int(item.fog),"fog pass exposes non-solid/decal geometry");
            Require(other==int(materialOnly.ops.size()),"fog omitted the existing material operations");
            {
                CSelectFragments filtered(&fragments,[](const SRenderFragmentInfo*) { return true; });
                CRenderCmdList hidden;AddFinalOps(&hidden,fragments,path,operation,&fog);
                Require(hidden.IsEmpty(),"final fog pass ignored scene-fragment filtering");
            }
        }
    }
}

static void HeadRenderTopology(const char* databasePath,const char* resourcePath)
{
    CFileStream database;database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    NLSHead::SHeadResourceData data;NLSHead::LoadHeadResourceData(11,&data);
    Require(!data.streams.empty() && data.tris.size()>2 && !data.UVs.empty(),
        "head topology fixture has no authored head11 resource");
    std::vector<WORD> expectedIndices,expectedPolys{0};
    int start=0;
    for(int end:data.tris) {
        Require(end>=start+3 && end<=int(data.indices.size()),"invalid authored smoothing range");
        for(int i=0;i<3;++i)expectedIndices.push_back(data.indices[start+i]);
        expectedPolys.push_back(WORD(expectedIndices.size()));start=end;
    }
    auto makeHolder=[&](bool persisted) {
        CObj<NLSHead::CFaceGenMeshHolder> holder=new NLSHead::CFaceGenMeshHolder;
        auto& raw=holder->GetInfo();
        raw.nVertices=data.nVertices;raw.copys=data.copys;raw.UVs=data.UVs;
        raw.indices=data.indices;raw.tris=data.tris;
        raw.animatorStreams.resize(data.streams.size());
        for(int i=0;i<int(data.streams.size());++i)
            raw.animatorStreams[i].Write(data.streams[i].GetBuffer(),data.streams[i].GetSize());
        if(persisted) {
            raw.trueIndices=expectedIndices;raw.trueTris=expectedPolys;
            // A valid alternate triangle ordering, with identical geometry:
            // saved render topology must survive independently of smoothing ranges.
            std::rotate(raw.trueIndices.begin(),raw.trueIndices.begin()+3,raw.trueIndices.end());
        }
        CMemoryStream bytes;
        { CStructureSaver saver(bytes,CStructureSaver::WRITE);saver.Add(1,&holder); }
        holder=nullptr;bytes.SetRMode();bytes.Seek(0);
        { CStructureSaver saver(bytes,CStructureSaver::READ);saver.Add(1,&holder); }
        Require(IsValid(holder) && holder->GetInfo().indices==data.indices &&
            holder->GetInfo().tris==data.tris && holder->GetInfo().UVs==data.UVs,
            "head holder save/load changed source geometry");
        if(persisted) {
            auto permutation=expectedIndices;
            std::rotate(permutation.begin(),permutation.begin()+3,permutation.end());
            Require(holder->GetInfo().trueIndices==permutation && holder->GetInfo().trueTris==expectedPolys,
                "existing saved render-topology tags did not round-trip");
        } else Require(holder->GetInfo().trueIndices.empty() && holder->GetInfo().trueTris.empty(),
            "legacy head fixture unexpectedly has populated render-topology tags");
        return holder;
    };
    CObj<NLSHead::CHeadMeshLoader> baseLoader=new NLSHead::CHeadMeshLoader;
    baseLoader->SetKey(11);
    auto* cachedBase=baseLoader->GetValue();
    Require(cachedBase && cachedBase->trueIndices==expectedIndices && cachedBase->trueTris==expectedPolys,
        "native head resource loader did not cache the original render topology");
    CObj<CCTime> clock=new CCTime(0);
    auto render=[&](NLSHead::CFaceGenMeshHolder* holder) {
        CDGPtr<NLSHead::CHeadAnimator> animator=new NLSHead::CHeadAnimator(clock,holder);
        MarkNewDGFrame();animator.Refresh();
        CObj<NGScene::CObjectInfo> output=animator->GetValue();
        Require(IsValid(output) && output->GetPositions().size()==data.UVs.size(),
            "our native facial engine did not produce the authored head mesh");
        for(const auto& position:output->GetPositions())
            Require(std::isfinite(position.x) && std::isfinite(position.y) && std::isfinite(position.z),
                "our native facial engine produced a nonfinite neutral vertex");
        return output;
    };
    auto legacyHolder=makeHolder(false);auto legacy=render(legacyHolder);
    Require(legacy->GetGeometry().indices==expectedIndices && legacy->GetGeometry().polys==expectedPolys,
        "legacy saved head lost its derived render triangles");
    std::printf("native head11 legacy empty-tag save/load control: vertices=%zu triangles=%zu\n",
        legacy->GetPositions().size(),expectedIndices.size()/3);std::fflush(stdout);
    auto storedHolder=makeHolder(true);auto stored=render(storedHolder);
    const auto& saved=storedHolder->GetInfo();
    Require(stored->GetPositions()==legacy->GetPositions(),
        "reading stored head topology changed our native neutral facial positions");
    std::printf("native head11 stored topology tags survive save/load; renderer honors them=%d\n",
        int(stored->GetGeometry().indices==saved.trueIndices));std::fflush(stdout);
    Require(stored->GetGeometry().indices==saved.trueIndices && stored->GetGeometry().polys==saved.trueTris,
        "head renderer ignores the persisted render triangle ordering");
    NDb::CComplexHead* transformable=nullptr;
    CDBIterator<NDb::CComplexHead> rows(*NDatabase::GetTable<NDb::CComplexHead>());
    while(rows.MoveNext()) {
        auto* candidate=rows.Get();
        if(IsValid(candidate) && IsValid(candidate->pHead) && candidate->pHead->isTransformable &&
           IsValid(candidate->pHead->pTransformableTextures)) { transformable=candidate;break; }
    }
    Require(transformable!=nullptr,"head topology fixture has no authored FaceGen template");
    CObj<NLSHead::CHeadTransformInfo> transform=new NLSHead::CHeadTransformInfo(transformable,clock);
    transform->SetPlayIdle(false);
    CObj<NLSHead::CHeadInfo> committed=transform->CreateHeadInfo();
    auto* baked=IsValid(committed) ? dynamic_cast<NLSHead::CFaceGenMeshHolder*>(committed->GetMesh()) : nullptr;
    Require(baked && committed->IsStaticHead() && baked->GetInfo().animatorStreams.size()==1,
        "our native FaceGen producer did not commit a real baked head");
    CObj<NLSHead::CHeadMeshLoader> templateLoader=new NLSHead::CHeadMeshLoader;
    templateLoader->SetKey(transformable->pHead->GetRecordID());
    const auto* base=templateLoader->GetValue();
    const auto committedIndices=baked->GetInfo().trueIndices,committedPolys=baked->GetInfo().trueTris;
    Require(base && !committedIndices.empty() && committedIndices==base->trueIndices &&
        committedPolys==base->trueTris,"our FaceGen commit omitted its base render topology");
    CMemoryStream bytes;
    { CStructureSaver saver(bytes,CStructureSaver::WRITE);saver.Add(1,&committed); }
    committed=nullptr;bytes.SetRMode();bytes.Seek(0);
    { CStructureSaver saver(bytes,CStructureSaver::READ);saver.Add(1,&committed); }
    baked=IsValid(committed) ? dynamic_cast<NLSHead::CFaceGenMeshHolder*>(committed->GetMesh()) : nullptr;
    Require(baked && baked->GetInfo().trueIndices==committedIndices &&
        baked->GetInfo().trueTris==committedPolys,
        "our FaceGen commit lost render topology after head-info save/load");
    CDGPtr<NLSHead::CHeadAnimator> live=new NLSHead::CHeadAnimator(clock,baked);
    MarkNewDGFrame();live.Refresh();
    Require(live->GetValue() && live->GetValue()->GetGeometry().indices==committedIndices &&
        live->GetValue()->GetGeometry().polys==committedPolys,
        "our native facial engine could not render the round-tripped FaceGen commit");
    std::printf("native FaceGen producer template=%d: single baked stream, populated topology and head-info save/load rendered\n",
        transformable->GetRecordID());
}

class MeshNode: public CPtrFuncBase<NGScene::CObjectInfo>
{
    OBJECT_BASIC_METHODS(MeshNode);
protected:
    bool NeedUpdate() override { return false; }
    void Recalc() override {}
public:
    explicit MeshNode(NGScene::CObjectInfo* mesh = nullptr) { pValue = mesh; }
};

class GeometryPart: public NGScene::IPart
{
    OBJECT_BASIC_METHODS(GeometryPart);
public:
    NGScene::ETransformType transform = NGScene::TT_NONE;
    GeometryPart() {}
    explicit GeometryPart(MeshNode* mesh): IPart(mesh, nullptr, false) {}
    NGScene::ETransformType GetTransformType() const override { return transform; }
};

class BoundsProbe: public NGScene::CVBCombiner
{
    OBJECT_NOCOPY_METHODS(BoundsProbe);
public:
    explicit BoundsProbe(CFuncBase<std::vector<CPtr<NGScene::IPart>>>* parts = nullptr):
        CVBCombiner(parts, NGScene::LT_NONE, NGScene::CT_DYNAMIC, nullptr) {}
    void Calculate() { XFormPosition(); }
};

static void NegativeMeshBounds()
{
    for (float shift : {0.f, 20.f, 4.f}) {
        NGScene::CObjectInfo::SData data;
        for (auto position : {CVec3(-4,-6,-8), CVec3(-2,-5,-7), CVec3(-3,-4,-9)}) {
            NGScene::SVertex vertex{};
            vertex.pos = position + CVec3(shift, shift, shift);
            data.verts.push_back(vertex);
        }
        data.geometry.SetTriangles({STriangle(0,1,2)});
        CObj<NGScene::CObjectInfo> mesh = new NGScene::CObjectInfo;
        mesh->AssignFast(data);
        CObj<MeshNode> node = new MeshNode(mesh);
        CObj<GeometryPart> part = new GeometryPart(node);
        CObj<NGScene::CAutomaticCombiner> parts = new NGScene::CAutomaticCombiner;
        parts->AddPart(part);
        CObj<BoundsProbe> combiner = new BoundsProbe(parts);
        combiner->Calculate();
        Require(part->vBVMax == CVec3(shift-2, shift-4, shift-7),
            "negative-coordinate mesh bounds incorrectly include the origin");
        Require(part->vBVMin == CVec3(shift-4, shift-6, shift-9), "mesh bounds lost the minimum vertex");
    }
}

class ProjectionRecorder: public NGScene::CPerPolyDecal
{
    OBJECT_BASIC_METHODS(ProjectionRecorder);
protected:
    void Recalc(NGScene::CObjectInfo::SData*, const NGScene::CObjectInfo&,
                const std::vector<CVec3>& positions) override { captured = positions; }
public:
    std::vector<CVec3> captured;
    ProjectionRecorder() {}
    ProjectionRecorder(GeometryPart* part, const std::vector<CVec3>& saved): CPerPolyDecal(part, saved) {}
    void Rebuild() { CPerPolyDecal::Recalc(); }
};

static void DecalSurfaceOffset(bool impact)
{
    NGScene::CObjectInfo::SData data;
    for(auto point:{CVec3(-0.3f,-0.3f,0),CVec3(0.3f,-0.3f,0),CVec3(0,0.3f,0)}) {
        NGScene::SVertex vertex{};vertex.pos=point;
        NGfx::CalcCompactVector(&vertex.normal,CVec3(0,0,1));
        NGfx::CalcCompactVector(&vertex.texU,CVec3(1,0,0));
        NGfx::CalcCompactVector(&vertex.texV,CVec3(0,1,0));data.verts.push_back(vertex);
    }
    data.geometry.SetTriangles({STriangle(0,1,2)});
    CObj<NGScene::CObjectInfo> source=new NGScene::CObjectInfo;source->Assign(data);
    CObj<MeshNode> node=new MeshNode(source);
    auto project=[&](int shift) {
        CDGPtr<NGScene::CDecalGeometry> decal=new NGScene::CDecalGeometry(node,SDiscretePos(nullptr,VNULL3,0),
            VNULL3,CVec3(0,0,1),CVec2(2,2),0.f,CVec2(0.5f,0.5f));
        decal->nShift=shift;MarkNewDGFrame();decal.Refresh();auto* mesh=decal->GetValue();
        Require(mesh && mesh->GetTrisCount()==1 && mesh->GetPositions().size()==3,
            "native decal clipping control did not produce the source triangle");
        SBound bound;mesh->CalcBound(&bound);
        Require(std::fabs(bound.s.ptCenter.x)<1e-6f && std::fabs(bound.s.ptCenter.y)<1e-6f &&
            std::fabs(bound.ptHalfBox.x-0.3f)<1e-6f && std::fabs(bound.ptHalfBox.y-0.3f)<1e-6f && bound.ptHalfBox.z<1e-6f,
            "native decal clipping changed the source triangle footprint");
        for(const auto& point:mesh->GetPositions())Require(std::fabs(point.z-bound.s.ptCenter.z)<1e-6f,
            "native decal clipping did not produce a planar projected surface");
        return bound.s.ptCenter.z;
    };
    const float authored=project(0);
    if(impact) {
        const float bullet=project(1);
        std::printf("native projected decal: authoredZ=%.6f impactZ=%.6f expectedSeparateSurfaces=1\n",double(authored),double(bullet));std::fflush(stdout);
        Require(bullet>authored+1e-5f && std::fabs(bullet-2*authored)<1e-6f,
            "projected bullet impact remained coplanar with authored surface spots");
        // Real serializable producer and the constructor path used by opaque
        // scene impacts: tag11 must retain its layer through native save/load.
        vector<CVec3> points;for(const auto& vertex:data.verts)points.push_back(vertex.pos);
        CObj<NGScene::CMemObjectInfo> memory=new NGScene::CMemObjectInfo({STriangle(0,1,2)},points,vector<CVec3>(3,CVec3(0,0,1)));
        CObj<NGScene::CDecalGeometry> stored=new NGScene::CDecalGeometry(memory,SDiscretePos(nullptr,VNULL3,0),
            VNULL3,CVec3(0,0,1),CVec2(2,2),0.f,CVec2(0.5f,0.5f),-0.01f,1e10f,1);
        Require(stored->nShift==1,"native opaque-impact constructor lost its requested layer");
        CMemoryStream bytes;{CStructureSaver saver(bytes,CStructureSaver::WRITE);saver.Add(1,&stored);}
        stored=nullptr;bytes.SetRMode();bytes.Seek(0);
        {CStructureSaver saver(bytes,CStructureSaver::READ);saver.Add(1,&stored);}
        Require(stored && stored->nShift==1,"native decal layer tag11 did not survive save/load");
        CDGPtr<NGScene::CDecalGeometry> loaded=stored.GetPtr();MarkNewDGFrame();loaded.Refresh();auto* loadedMesh=loaded->GetValue();
        Require(loadedMesh && loadedMesh->GetTrisCount()==1,"round-tripped native impact did not regenerate its clipped mesh");
        SBound loadedBound;loadedMesh->CalcBound(&loadedBound);
        Require(std::fabs(loadedBound.s.ptCenter.z-bullet)<1e-6f,
            "round-tripped native impact lost its separate projected surface");
        std::printf("native impact constructor/save-load tag11=1: regenerated centerZ=%.6f\n",double(loadedBound.s.ptCenter.z));
    } else {
        std::printf("native projected decal: authoredZ=%.6f originalZ=0.008000\n",double(authored));std::fflush(stdout);
        Require(std::fabs(authored-0.008f)<1e-6f,
            "projected authored decal uses half the original surface separation");
    }
}

class MemoryProjectionPart: public NGScene::IPart
{
    OBJECT_BASIC_METHODS(MemoryProjectionPart);
    SFBTransform pose;
public:
    MemoryProjectionPart() { Identity(&pose.forward);Identity(&pose.backward); }
    explicit MemoryProjectionPart(NGScene::CMemObjectInfo* node): IPart(node,nullptr,false) {
        Identity(&pose.forward);Identity(&pose.backward);
    }
    NGScene::ETransformType GetTransformType() const override { return NGScene::TT_SIMPLE; }
    const SFBTransform& GetSimplePos() override { return pose; }
};

static void MemoryMeshDecalProjection()
{
    // Identical points and surfaces; a regenerated mesh enumerates its two
    // triangles in the opposite order. The saved projection pose stays valid.
    vector<CVec3> points={CVec3(-0.3f,-0.3f,0),CVec3(0.3f,-0.3f,0),CVec3(0,0.3f,0),
        CVec3(9.7f,-0.3f,0),CVec3(10.3f,-0.3f,0),CVec3(10,0.3f,0)};
    vector<CVec3> normals(points.size(),CVec3(0,0,1));
    vector<CVec3> savedPose;
    for(bool reversed:{false,true}) {
        vector<STriangle> triangles={STriangle(0,1,2),STriangle(3,4,5)};
        if(reversed)std::reverse(triangles.begin(),triangles.end());
        CDGPtr<NGScene::CMemObjectInfo> source=new NGScene::CMemObjectInfo(triangles,points,normals);
        CObj<MemoryProjectionPart> part=new MemoryProjectionPart(source.Get());
        MarkNewDGFrame();source.Refresh();
        Require(source->GetValue() && source->GetValue()->GetTrisCount()==2,
            "native memory mesh fixture did not generate both source surfaces");
        if(!reversed)NGScene::TransformPart(part,&savedPose,nullptr);
        Require(savedPose.size()==6 && source->GetValue()->GetPositions().size()==6,
            "native memory mesh fixture lost its fixed point count");
        CDGPtr<NGScene::CExplosionDecalGeometry> decal=new NGScene::CExplosionDecalGeometry(part,savedPose,VNULL3,2.f,0.f);
        decal.Refresh();auto* mesh=decal->GetValue();
        Require(mesh && mesh->GetTrisCount()==1,"native projection did not retain exactly the near source triangle");
        SBound bound;mesh->CalcBound(&bound);
        std::printf("native memory decal: reversed=%d sourcePositions=%u decalTriangles=%d center=%.3f,%.3f,%.3f expectedNearOrigin=1\n",
            int(reversed),unsigned(source->GetValue()->GetPositions().size()),mesh->GetTrisCount(),
            double(bound.s.ptCenter.x),double(bound.s.ptCenter.y),double(bound.s.ptCenter.z));std::fflush(stdout);
        Require(std::fabs(bound.s.ptCenter.x)<0.01f && std::fabs(bound.s.ptCenter.y)<0.01f && bound.ptHalfBox.x<0.31f,
            "regenerating a memory mesh moved its saved explosion decal onto the distant triangle");
    }
    // The shared no-opt API still merges UV-seam positions and establishes
    // native position references; AssignFast deliberately has other semantics.
    NGScene::CObjectInfo::SData seam;
    for(auto point:{CVec3(0,0,0),CVec3(1,0,0),CVec3(1,1,0),CVec3(0,1,0),CVec3(0,0,0)}) {
        NGScene::SVertex vertex{};vertex.pos=point;seam.verts.push_back(vertex);
    }
    seam.verts.back().tex=CVec2(1,1);
    seam.geometry.indices={0,1,2,3,4,2,3};seam.geometry.polys={0,4,7};
    CObj<NGScene::CObjectInfo> mesh=new NGScene::CObjectInfo;mesh->Assign(seam,false);
    Require(mesh->GetVertices().size()==5 && mesh->GetPositions().size()==4 && mesh->GetTrisCount()==3 &&
        mesh->GetGeometry().indices==seam.geometry.indices && mesh->GetGeometry().polys==seam.geometry.polys &&
        mesh->GetPositionIndices()[0]==mesh->GetPositionIndices()[4],
        "no-opt generated geometry lost its polygons, UV seam or merged position references");
    vector<STriangle> references;mesh->GetVxPositionTriangles(&references);
    Require(references.size()==3 && references.back().i1==0,
        "no-opt generated geometry failed to establish the first-vertex reference for a UV seam");
    std::printf("native no-opt control: vertices5/positions4/triangles3, quad and UV seam references retained\n");
}

static void DecalProjection()
{
    NGScene::CObjectInfo::SData data;
    for (auto position : {CVec3(1,2,0), CVec3(3,4,0), CVec3(5,6,0)}) {
        NGScene::SVertex vertex{}; vertex.pos = position; data.verts.push_back(vertex);
    }
    data.geometry.SetTriangles({STriangle(0,1,2)});
    CObj<NGScene::CObjectInfo> mesh = new NGScene::CObjectInfo;
    mesh->AssignFast(data);
    CObj<MeshNode> node = new MeshNode(mesh);
    CObj<GeometryPart> part = new GeometryPart(node);
    auto saved = mesh->GetPositions();
    std::reverse(saved.begin(), saved.end());
    CObj<ProjectionRecorder> terrain = new ProjectionRecorder(part, saved);
    terrain->Rebuild();
    Require(terrain->captured == mesh->GetPositions(), "terrain decal reused a stale saved vertex order");
    part->transform = NGScene::TT_SIMPLE;
    CObj<ProjectionRecorder> transformed = new ProjectionRecorder(part, saved);
    transformed->Rebuild();
    Require(transformed->captured == saved, "transformed decal lost its original projection pose");
}


struct ItemPhysicsSnapshot
{
    vector<SMassSphere> spheres;
    CVec3 pos,lastPos,momentum,angular,posVelocity;
    CDBPtr<NDb::CDBPhysParams> parameters;
};

static ItemPhysicsSnapshot ReadItemPhysics(NAnimation::CASphereSet* item)
{
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(item,&refs);
    ItemPhysicsSnapshot result;
    CStructureSaver reader(fields,CStructureSaver::READ,&refs);
    reader.Add(2,&result.spheres);reader.Add(8,&result.pos);reader.Add(10,&result.momentum);
    reader.Add(11,&result.angular);reader.Add(12,&result.lastPos);reader.Add(18,&result.parameters);
    reader.Add(23,&result.posVelocity);
    return result;
}

struct ItemFloorTrack
{
    CPtr<NWorld::CDItem> item;CPtr<NAnimation::CASphereSet> animator;
    int operator&(CStructureSaver& saver) { saver.Add(2,&item);saver.Add(3,&animator);return 0; }
};
struct ItemFloorController
{
    list<ItemFloorTrack> items;
    int operator&(CStructureSaver& saver) { saver.Add(2,&items);return 0; }
};

static void AuthoredItemInitialFloorContact(NWorld::CWorld* world)
{
    auto* book=NDb::GetRPGItem(438);
    Require(book && IsValid(book->pModel),"authored waking item has no native book model");
    CObj<NRPG::IInventoryItem> inventory=NRPG::CreateClueItem(book);
    CObj<NWorld::CDFrozenItem> frozen=world->AddFrozenItem(world->GetAIMap(),CVec3(2,2,0),QNULL,inventory,false,0);
    Require(frozen && frozen->GetModel().pModel,"native frozen book construction failed");
    vector<SMassSphere> authored;CVec3 massCenter;
    NAI::GetSpheres(frozen->GetModel().pModel,&authored,&massCenter);
    Require(!authored.empty(),"native book has no authored physics spheres");
    float originalBottom=1.e10f;
    for(const auto& sphere:authored) originalBottom=Min(originalBottom,sphere.ptCenter.z-Max(.05f,sphere.fRadius));
    std::printf("authored waking book438 sphere-count=%zu initial-lowest-surface=%.9g\n",authored.size(),originalBottom);std::fflush(stdout);
    Require(originalBottom<-.001f,"authored waking book fixture does not reproduce an initial floor overlap");
    GlobalGameRandom().SeedForHarness(0x6e41);auto reference=GlobalGameRandom();
    for(int axis=0;axis<3;++axis)reference.GetFloat(-2.f,2.f);
    world->ActivateDebris(frozen,world->GetAIMap(),world->GetTime());
    auto actual=GlobalGameRandom();Require(actual.Get()==reference.Get(),"native waking book changed its three ISAAC initialization draws");
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(world,&refs);ItemFloorController controller;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(5,&controller);}
    Require(controller.items.size()==1 && controller.items.front().animator && controller.items.front().item,
        "native activation did not replace the frozen book with one physics item");
    auto* animator=controller.items.front().animator.GetPtr();auto initial=ReadItemPhysics(animator);
    Require(controller.items.front().item->GetInvItem()==inventory.GetPtr(),"native activation replaced the authored inventory item");
    float bottom=1.e10f;
    for(const auto& sphere:initial.spheres)bottom=Min(bottom,initial.pos.z+sphere.ptCenter.z-sphere.fRadius);
    std::printf("authored waking book438 native-center-z=%.9g lowest-surface=%.9g physics=%s\n",
        initial.pos.z,bottom,initial.parameters?initial.parameters->szName.c_str():"missing");std::fflush(stdout);
    Require(bottom>=-.0001f,"authored frozen book wakes with its collision spheres still embedded in the native floor");
    Require(fabs(initial.lastPos-initial.pos)<.00001f,"authored waking book left its interpolation origin embedded");
    Require(initial.parameters && initial.parameters->szName=="ItemThrowOut","authored waking book changed its original physics parameters");
}

static void ItemInitialFloorContact(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=8,"unknown item floor-contact case");
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(1);
    CObj<NWorld::CWorld> world=new NWorld::CWorld(game);world->CreateDefault();
    Require(world->GetAIMap()!=nullptr,"native item physics has no map");
    if(mode==8) { AuthoredItemInitialFloorContact(world);return; }
    const bool noFloor=mode==7;
    const bool centerOrigin=mode==4 || mode==6;
    const auto physicsCase=mode==4 || mode==5 ? NAnimation::PH_GRENADE_ATTACK :
        mode==1 ? NAnimation::PH_FALL_FROM_BODY : NAnimation::PH_THROW_OUT;
    const CVec3 origin(noFloor?-100.f:2.f,noFloor?-100.f:2.f,mode==2?.2f:mode==3?1.f:0.f);
    vector<SMassSphere> spheres{SMassSphere(CVec3(-.15f,0,0),.2f,1),SMassSphere(CVec3(.15f,0,0),.2f,1)};
    vector<SSphere> probes;vector<CVec3> sweeps;
    for(const auto& sphere:spheres) {
        probes.push_back(SSphere(origin+sphere.ptCenter+CVec3(0,0,sphere.fRadius),sphere.fRadius));
        sweeps.push_back(CVec3(0,0,-2*sphere.fRadius));
    }
    vector<NAI::SCollisionPoint> contacts;
    NAI::PhysCollideInfo(world->GetAIMap(),probes,sweeps,&contacts,NWorld::TS_ITEM_BLOCKER);
    Require(contacts.size()==spheres.size(),"native floor collider changed its dense sphere/result correspondence");
    for(const auto& contact:contacts) {
        if(noFloor || mode==3) Require(contact.fDist==NAI::FP_NO_COLLISION,"floor-free control unexpectedly swept into geometry");
        else {
            Require(contact.fDist!=NAI::FP_NO_COLLISION && contact.plane.n.z>.99f,"native flat terrain did not supply its upward contact");
            const float expected=mode==2?.2f:0.f;
            Require(std::abs(contact.fDist-expected)<.0001f,"native sweep contact distance is not measured in world units");
            std::printf("item floor mode=%d sweep-distance=%.9g normal-z=%.9g\n",mode,contact.fDist,contact.plane.n.z);
        }
    }
    CObj<CCTime> clock=new CCTime(1000);
    CObj<NAnimation::CASphereSet> item=new NAnimation::CASphereSet(spheres,VNULL3,CVec3(1,1,1),nullptr,-2);
    item->pMap=world->GetAIMap();item->pTime=clock;
    GlobalGameRandom().SeedForHarness(0x6e41);auto expectedRandom=GlobalGameRandom();
    CVec3 angular;
    angular.x=expectedRandom.GetFloat(-2.f,2.f)/3.f;
    angular.y=expectedRandom.GetFloat(-2.f,2.f)/3.f;
    angular.z=expectedRandom.GetFloat(-2.f,2.f)/3.f;
    const CVec3 velocity(.1f,-.2f,0);
    item->Init(1000,origin,QNULL,velocity,centerOrigin,physicsCase,nullptr);
    auto actualRandom=GlobalGameRandom();
    Require(actualRandom.Get()==expectedRandom.Get(),"initial floor correction changed the original three ISAAC draws");
    auto initial=ReadItemPhysics(item);
    Require(initial.parameters && initial.parameters->szName==(physicsCase==NAnimation::PH_GRENADE_ATTACK?"GrenadeAttack":mode==1?"ItemFallFromBody":"ItemThrowOut"),
        "initial floor correction changed the authored physics case");
    Require(fabs(initial.momentum-velocity*2.f)<.00001f && fabs(initial.posVelocity-velocity)<.00001f &&
        fabs(initial.angular-angular)<.00001f,"initial floor correction changed native momentum or initial velocity");
    Require(!item->DidCollide() && !item->HasStopped() && item->GetFloor()==-2,"initial placement changed collision/rest/floor bookkeeping");
    Require(initial.spheres.size()==2,"native item fixture unexpectedly expanded its touching sphere chain");
    const bool lift=mode==0 || mode==1;
    const float expectedZ=origin.z+(lift?.2f:0.f);
    std::printf("item floor mode=%d initial-origin-z=%.9g native-center-z=%.9g prior-z=%.9g expected-z=%.9g dt=%u\n",
        mode,origin.z,initial.pos.z,initial.lastPos.z,expectedZ,initial.parameters->DELTA_T_DWORD);
    item->Calc(1100);auto advanced=ReadItemPhysics(item);
    std::printf("item floor mode=%d after100ms-center-z=%.9g collided=%d\n",mode,advanced.pos.z,item->DidCollide());
    std::printf("item trajectory mode=%d pos=%.9g,%.9g,%.9g momentum=%.9g,%.9g,%.9g angular=%.9g,%.9g,%.9g prior=%.9g,%.9g,%.9g velocity=%.9g,%.9g,%.9g\n",
        mode,advanced.pos.x,advanced.pos.y,advanced.pos.z,advanced.momentum.x,advanced.momentum.y,advanced.momentum.z,
        advanced.angular.x,advanced.angular.y,advanced.angular.z,advanced.lastPos.x,advanced.lastPos.y,advanced.lastPos.z,
        advanced.posVelocity.x,advanced.posVelocity.y,advanced.posVelocity.z);std::fflush(stdout);
    Require(std::abs(initial.pos.z-expectedZ)<.0001f,
        "item physics starts with its mass spheres embedded in the native terrain floor");
    Require(fabs(initial.lastPos-initial.pos)<.00001f,"initial floor correction left the interpolation origin below the floor");
    Require(std::abs(initial.pos.x-origin.x)<.00001f && std::abs(initial.pos.y-origin.y)<.00001f,
        "initial floor correction displaced the item horizontally");
}

class HiddenSegmentBudgetProbe:public CObjectBase
{
    OBJECT_NOCOPY_METHODS(HiddenSegmentBudgetProbe);
    NWorld::CWorld* owner;
    NGlobal::CEventRegister<HiddenSegmentBudgetProbe,NWorld::CEventOnSegment> registration;
public:
    int segments=0,delayMilliseconds=0;
    explicit HiddenSegmentBudgetProbe(NWorld::CWorld* world=nullptr):owner(world),registration(this,&HiddenSegmentBudgetProbe::OnSegment){}
    void OnSegment(const NWorld::CEventOnSegment&) {
        if(NWorld::pCurrentWorld!=owner)return;
        ++segments;
        if(delayMilliseconds)std::this_thread::sleep_for(std::chrono::milliseconds(delayMilliseconds));
    }
};

static void HiddenSkipBudget(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown hidden skip budget mode");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CPlayer>> players;world->GetPlayersList(&players);
    Require(!players.empty() && !world->IsSequence(),"hidden skip fixture has no owned turn candidate");
    auto* owner=players.front().GetPtr();
    struct RestoreContext {NWorld::CWorld* previous=NWorld::pCurrentWorld;~RestoreContext(){NWorld::pCurrentWorld=previous;}} restore;
    NWorld::pCurrentWorld=world;
    world->GivePlayerTurn(owner);
    CObj<NWorld::CActionCounter> action=world->GetSkippableCounter();
    CObj<NWorld::CPlayer> observer=new NWorld::CPlayer(L"Unseen observer",game,game->players.front(),999);
    Require(world->GetTBSCurrentPlayer()==owner && world->CanSkip(observer),"hidden skip fixture did not establish an unseen skippable action");
    CObj<HiddenSegmentBudgetProbe> probe=new HiddenSegmentBudgetProbe(world);probe->delayMilliseconds=mode==0?50:0;
    const STime before=world->GetTime()->GetValue(),beforeAim=world->GetAimTime()->GetValue();
    if(mode==3) {
        world->bNetworkArena=true;world->UpdateWorld(beforeAim+1000,observer);
        world->bNetworkArena=false;world->bNetworkReplica=true;world->UpdateWorld(beforeAim+1000,observer);
        Require(probe->segments==0 && world->GetTime()->GetValue()==before && world->GetAimTime()->GetValue()==beforeAim,
            "single-player skip timing entered the fork's LAN host or replica scheduler");
        std::printf("hidden skip mode=3 networkHost/replica=untouched segments=0\n");return;
    }
    NWorld::IPlayer* viewer=mode==1?owner:mode==2?nullptr:observer.GetPtr();
    world->UpdateWorld(beforeAim,viewer);
    std::printf("hidden skip mode=%d firstSegments=%d worldDelta=%d aimDelta=%d delay=%dms\n",mode,probe->segments,int(world->GetTime()->GetValue()-before),int(world->GetAimTime()->GetValue()-beforeAim),probe->delayMilliseconds);std::fflush(stdout);
    Require(probe->segments==1,"world continued hidden fast-forward after a segment exceeded the30ms work budget");
    Require(world->GetTime()->GetValue()-before==50 && world->GetAimTime()->GetValue()-beforeAim==50,
        "hidden skip timing changed the ordinary50ms segment clock");
    world->UpdateWorld(world->GetAimTime()->GetValue(),viewer);
    Require(probe->segments==2 && world->GetTime()->GetValue()-before==100 && world->GetAimTime()->GetValue()-beforeAim==100,
        "hidden skip work budget did not reset on the next world update");
}

static CPtr<NWorld::CUnitState> ReadCorpseCancelUnitState(NWorld::CUnitServer* unit)
{
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(unit,&refs);CPtr<NWorld::CUnitState> state;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(10,&state);}
    return state;
}

static void CorpseCancelState(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown corpse cancellation phase");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> ordinary;
    for(auto entry:units) {
        auto* unit=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK() && !unit->animator.GetCannon())ordinary.push_back(unit);
        if(ordinary.size()==2)break;
    }
    Require(ordinary.size()==2,"corpse cancellation needs two actual ordinary actors");
    auto* actor=ordinary[0];auto* body=ordinary[1];
    body->GetUnitRPG()->GetRPGUnit()->Kill();body->SetState(new NWorld::CUnitStateDeath(body));
    Require(body->IsDead() && actor->CanFight() && !actor->HasCommand(),"corpse cancellation native actor/death preparation failed");
    const bool take=mode<2,afterHandoff=(mode%2)!=0;
    CObj<NWorld::CExecCorpse> setup;
    if(!take) {
        setup=new NWorld::CExecCorpse(actor,body,true);setup->Run();setup->TimeLabelReached();
        Require(actor->GetState()==NWorld::CUnit::ST_CARRY_CORPSE && actor->GetCorpse()==body &&
            body->GetCorpseCarrier()==actor,"drop cancellation did not prepare native reciprocal carrying");
    }
    CObj<NWorld::CExecCorpse> command=new NWorld::CExecCorpse(actor,body,take);
    command->Run();if(afterHandoff)command->TimeLabelReached();
    const bool beforeCarrying=take?afterHandoff:!afterHandoff;
    Require((actor->GetState()==NWorld::CUnit::ST_CARRY_CORPSE)==beforeCarrying,
        "corpse cancellation did not reach the expected native label phase");
    CPtr<NWorld::CUnitState> previous=ReadCorpseCancelUnitState(actor);
    const auto* identity=previous.GetPtr();Require(identity!=nullptr,"native corpse cancellation has no state to observe");
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();
    command->Cancel();auto current=ReadCorpseCancelUnitState(actor);
    const bool carryingState=actor->GetState()==NWorld::CUnit::ST_CARRY_CORPSE;
    const bool reciprocal=actor->GetCorpse()==body && body->GetCorpseCarrier()==actor;
    const bool preserved=current.GetPtr()==identity;
    std::printf("native corpse cancel mode=%d take=%d after-label=%d old-carry=%d now-carry=%d reciprocal=%d same-state=%d AP=%d->%d\n",
        mode,int(take),int(afterHandoff),int(beforeCarrying),int(carryingState),int(reciprocal),int(preserved),ap,actor->GetAP());std::fflush(stdout);
    Require(command->GetState()==NWorld::CCommandExecute::FINISHED,"corpse cancellation did not finish its native executor");
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && body->IsDead(),
        "corpse cancellation changed AP, health, world position or death state");
    Require(carryingState==!take && actor->IsCarryingCorpse()==!take,
        "corpse cancellation did not restore the intended carrying mode");
    Require(take ? !body->GetCorpseCarrier() : reciprocal,
        "cancelling a corpse drop detached the body while the actor still carries it");
    Require(preserved==!afterHandoff,"corpse cancellation replaced an unchanged native unit state");
}

struct NativeAuthoredEffectLight {CDBPtr<NDb::CLightInstance> instance;CObj<NGScene::CAnimLightInfo> info;};

static vector<NativeAuthoredEffectLight> NativeAuthoredEffectLights()
{
    vector<NativeAuthoredEffectLight> lights;
    CDBIterator<NDb::CLightInstance> candidates(*NDatabase::GetTable<NDb::CLightInstance>());
    while(candidates.MoveNext() && lights.size()<2) {
        auto* instance=candidates.Get();if(!IsValid(instance->pLight))continue;
        CObj<NGScene::CFileRequest> request=new NGScene::CFileRequest("Lights",instance->pLight->GetRecordID());request->Read();
        if(request->GetStream()->GetSize()==0)continue;
        CObj<NGScene::CAnimLightInfo> info=new NGScene::CAnimLightInfo;info->fFrameRate=info->fTStart=info->fTEnd=0;
        try {CStructureSaver reader(*request->GetStream(),CStructureSaver::READ);reader.Add(1,&info->fFrameRate);reader.Add(2,&info->fTStart);reader.Add(3,&info->fTEnd);reader.Add(4,&info->pos);reader.Add(5,&info->color);reader.Add(6,&info->radius);}catch(...){continue;}
        if(!(info->fFrameRate>0 && info->fTStart>=0 && info->fTEnd>info->fTStart+0.05f && info->fTEnd<600) || info->pos.keys.empty() || info->color.keys.empty() || info->radius.keys.empty())continue;
        // Keep real resource curves/placement/scale; control only the instance timeline.
        instance->fSpeed=1;instance->fOffset=0;instance->fEndCycle=0;instance->nCycleCount=1;
        lights.push_back({instance,info});
    }
    Require(lights.size()==2,"ambient fixture lacks two actual authored light resources");
    return lights;
}

class NativeAmbientEffectMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(NativeAmbientEffectMission);
    CObj<NUI::CDesktopWindow> desktop;
public:
    void Attach(NWorld::CWorld* world,NRPG::CGlobalGame* game,NGScene::IGameView* view,NRender::IRenderGame* renderer) {
        pWorld=world;pGlobalGame=game;pScene=view;pRender=renderer;desktop=new NUI::CDesktopWindow;
    }
    NUI::CDesktopWindow* GetDesktop() const override {return desktop;}
    void Drain() {ExecWorldCommands();}
};

static void ScriptAmbientEffect(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=9,"unknown ambient-effect case");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm native scripted ambient-effect regression");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}
    world->SetTimeOfDay(NWorld::TOD_DAY);
    CObj<NGScene::IGameView> view=NGScene::CreateNewView();
    CObj<NRender::IRenderGame> renderer=NRender::CreateRenderGame(world,view,nullptr);
    CObj<NativeAmbientEffectMission> mission=new NativeAmbientEffectMission;mission->Attach(world,game,view,renderer);
    Require(view && renderer && world->GetOwnScript(),"ambient fixture did not create native world/Lua/view/renderer");
    auto* scene=view->GetGScene();Require(scene!=nullptr,"ambient fixture has no real graphics scene");
    CStructureNetworkContext references;auto fields=BorrowedObjectFields(scene,&references);
    CPtr<CFuncBase<CVec3>> mean,animated;
    {CStructureSaver reader(fields,CStructureSaver::READ,&references);reader.Add(18,&mean);reader.Add(33,&animated);}
    Require(IsValid(mean) && IsValid(animated),"ambient fixture lost the existing native animated-ambient graph");
    const CVec3 top(0.17f,0.31f,0.53f),bottom(0.07f,0.13f,0.23f);
    view->SetAmbient(bottom,top);
    auto binding=[&](){CStructureNetworkContext refs;auto saved=BorrowedObjectFields(animated,&refs);CPtr<NGScene::CLightAnimator> result;
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(3,&result);}return result;};
    auto geometry=[&](){CStructureNetworkContext refs;auto saved=BorrowedObjectFields(scene,&refs);list<CPtr<CObjectBase>> lights,particles;
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(7,&lights);reader.Add(10,&particles);}
        return std::make_pair(std::count_if(lights.begin(),lights.end(),[](const CPtr<CObjectBase>& p){return IsValid(p);}),
            std::count_if(particles.begin(),particles.end(),[](const CPtr<CObjectBase>& p){return IsValid(p);}));};
    const auto originalGeometry=geometry();
    auto lights=NativeAuthoredEffectLights();
    CDBIterator<NDb::CEffect> effects(*NDatabase::GetTable<NDb::CEffect>());
    Require(effects.MoveNext(),"ambient fixture has no registered effect");CDBPtr<NDb::CEffect> effect=effects.Get();
    Require(effects.MoveNext(),"ambient fixture has no replacement effect");CDBPtr<NDb::CEffect> replacement=effects.Get();
    for(auto record:{effect.GetPtr(),replacement.GetPtr()}){record->instances.clear();record->lights.clear();record->flags.clear();}
    effect->lights.push_back(lights[0].instance.GetPtr());replacement->lights.push_back(lights[1].instance.GetPtr());
    NDb::SVariantFlags allowed;allowed.flags=world->GetCreateFlags();effect->flags.push_back(allowed);replacement->flags.push_back(allowed);
    Require(!allowed.flags.empty() && IsSuitableVariant(world->GetCreateFlags(),effect->flags),"ambient fixture's normal variant does not match its actual world flags");
    CDBIterator<NDb::CTEffect> templates(*NDatabase::GetTable<NDb::CTEffect>());
    Require(templates.MoveNext(),"ambient fixture has no registered effect template");CDBPtr<NDb::CTEffect> pool=templates.Get();
    pool->variants.clear();pool->variants.push_back(effect.GetPtr());pool->roulette=CRoulette();pool->roulette.AddSector(1);
    auto* clock=renderer->GetTime();const STime begin=1375;clock->Set(begin);
    CDGPtr<CFuncBase<CVec3>> meanEdge=mean.GetPtr();
    auto sample=[&](){MarkNewDGFrame();meanEdge.Refresh();return meanEdge->GetValue();};
    Require(fabs2(sample()-(top+bottom)*0.5f)<1e-10f,"ambient fixture changed its native unanimated mean");
    CObj<NGScene::CLightLoader> seedLoader;
    CObj<NGScene::CLightAnimator> seed;
    const bool seeded=(mode>=3 && mode<=6)||mode==9;
    if(seeded) {
        // Existing saved animation state, using a real native animator/resource.
        seedLoader=new NGScene::CLightLoader;seedLoader->SetKey(lights[0].instance->pLight->GetRecordID());
        seed=new NGScene::CLightAnimator(lights[0].instance,begin,lights[0].instance->fScale);seed->pTime=clock;seed->pInfo=seedLoader;
        SFBTransform placement;MakeMatrix(&placement.forward,lights[0].instance->position,lights[0].instance->rotation,CVec3(lights[0].instance->fScale,lights[0].instance->fScale,lights[0].instance->fScale));placement.backward.HomogeneousInverse(placement.forward);seed->pPlacement=new NGScene::CCFBTransform(placement);
        CStructureNetworkContext refs;auto seedFields=BorrowedObjectFields(animated,&refs);refs.ids[seed.GetPtr()]=1;refs.refs[1]=seed.GetPtr();CPtr<NGScene::CLightAnimator> edge=seed.GetPtr();CMemoryStream saved;
        {CStructureSaver writer(saved,CStructureSaver::WRITE,&refs);writer.Add(3,&edge);}saved.Seek(0);
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);animated->operator&(reader);}animated->NetworkFieldsChanged();
    }
    auto expected=[&](std::size_t which,STime start){CVec3 color;lights[which].info->color.GetValue((clock->GetValue()-start)*lights[which].info->fFrameRate/1000.f,&color);return (color+bottom)*0.5f;};
    auto observeAnimation=[&](std::size_t which,STime start){
        const STime tick=start+STime((lights[which].info->fTStart+(lights[which].info->fTEnd-lights[which].info->fTStart)*0.25f)*1000.f);clock->Set(tick);
        CVec3 value;const auto deadline=SDL_GetTicks()+10000;
        do {value=sample();auto edge=binding();if(edge){CDGPtr<NGScene::CLightAnimator> refresh=edge.GetPtr();refresh.Refresh();if(refresh->GetValue()->bActive)break;}SDL_Delay(1);}while(SDL_GetTicks()<deadline);
        Require(fabs2(value-expected(which,start))<1e-7f,"real authored light curve did not reach the native ambient mean");
    };
    if(seeded){Require(binding().GetPtr()==seed.GetPtr(),"ambient fixture did not restore its native seed edge");observeAnimation(0,begin);}
    if(mode==1) {
        effect->lights.push_back(lights[1].instance.GetPtr());
        CDBIterator<NDb::CParticleInstance> particles(*NDatabase::GetTable<NDb::CParticleInstance>());NDb::CParticleInstance* particle=nullptr;
        while(particles.MoveNext())if(IsValid(particles.Get()->pParticle)){particle=particles.Get();break;}
        Require(particle!=nullptr,"mixed ambient fixture has no native particle instance");effect->instances.push_back(particle);
    }
    if(mode==4)effect->lights.clear();
    if(mode==5 || mode==9){lights[0].instance->pLight=nullptr;if(mode==5)effect->lights.push_back(lights[1].instance.GetPtr());}
    if(mode==6) {
        effect->flags[0].flags.clear();effect->flags[0].flags.push_back(NWorld::TOD_NIGHT);
        Require(!IsSuitableVariant(world->GetCreateFlags(),effect->flags),"night-only fixture variant unexpectedly matched the day world");
        SRand random(SRandomSeed(123));Require(pool->GetEffect(&random,world->GetCreateFlags())==nullptr,"native variant filtering did not reject the fixture's night-only effect");
    }
    auto run=[&](int id){const string lua="SetAmbientEffect("+std::to_string(id)+")";
        Require(world->GetOwnScript()->DoString(lua.c_str())==0,"ambient fixture could not execute its actual Lua producer");world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);mission->Drain();
        CObj<NWorld::CUICmd> extra=world->GetUICommand();Require(!IsValid(extra),"ambient dispatch left an unexpected world command");};
    CDGPtr<CFuncBase<STime>> clockRead=clock;clockRead.Refresh();
    const STime requestTime=clockRead->GetValue();run(mode==3 || mode==8 ? -1 : pool->GetRecordID());
    auto current=binding();auto shape=geometry();
    std::printf("native script ambient mode=%d first=%d light=%d start=%u binding=%d extraLights=%lld extraParticles=%lld seeded=%d\n",mode,lights[0].instance->GetRecordID(),lights[0].instance->pLight?lights[0].instance->pLight->GetRecordID():0,unsigned(requestTime),int(IsValid(current)),static_cast<long long>(shape.first-originalGeometry.first),static_cast<long long>(shape.second-originalGeometry.second),int(seeded));std::fflush(stdout);
    const bool clears=mode==3 || mode==4 || mode==6 || mode==8;
    const bool retains=mode==5 || mode==9;
    Require(clears ? !IsValid(current) : retains ? current.GetPtr()==seed.GetPtr() : IsValid(current),"SetAmbientEffect did not bind/clear/retain the original first-light ambient edge");
    Require(shape==originalGeometry,"ambient-only command spawned ordinary point lights or effect particles");
    if(clears) {
        // Original null-edge removal does not invalidate an already cached color.
        // The next normal top-light update recomputes the unanimated fallback.
        view->SetAmbient(bottom,top);Require(fabs2(sample()-(top+bottom)*0.5f)<1e-10f,"cleared ambient effect did not use the normal refreshed top-light fallback");
    } else if(retains) observeAnimation(0,begin);
    else {
        CStructureNetworkContext refs;auto saved=BorrowedObjectFields(current,&refs);CDBPtr<NDb::CLightInstance> instance;STime start=0;float scale=0;CDGPtr<CFuncBase<STime>> time;
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(2,&scale);reader.Add(3,&start);reader.Add(4,&instance);reader.Add(5,&time);}
        Require(instance.GetPtr()==lights[0].instance.GetPtr() && start==requestTime && time.GetPtr()==clock && scale==lights[0].instance->fScale,"ambient animator lost first-instance/renderer-clock/start-time/scale identity");
        observeAnimation(0,requestTime);
        if(mode==2) {
            CObj<NGScene::CLightAnimator> old=current.GetPtr();pool->variants[0]=replacement.GetPtr();const STime nextStart=clock->GetValue()+205;clock->Set(nextStart);run(pool->GetRecordID());
            auto next=binding();Require(IsValid(next) && next.GetPtr()!=old.GetPtr(),"ambient replacement retained the old light animator");observeAnimation(1,nextStart);
        }
        if(mode==7) {clock->Set(requestTime+STime(std::ceil(lights[0].info->fTEnd*1000))+20);Require(fabs2(sample()-(top+bottom)*0.5f)<1e-10f,"naturally ended ambient animation did not restore its original top-light fallback");}
    }
    Require(geometry()==originalGeometry,"ambient replacement or clear leaked ordinary effect geometry");
    Require(!NMainLoop::HaveInterfaceCommand(),"ambient command unexpectedly changed the game interface");
}



static void ScriptImmediateAmbient(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=4,"unknown immediate ambient case");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm native immediate ambient regression");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}
    world->SetTimeOfDay(NWorld::TOD_DAY);const auto dayFlags=world->GetCreateFlags();
    world->SetTimeOfDay(NWorld::TOD_NIGHT);const auto nightFlags=world->GetCreateFlags();world->SetTimeOfDay(NWorld::TOD_DAY);
    CObj<NGScene::IGameView> view=NGScene::CreateNewView();
    CObj<NRender::IRenderGame> renderer=NRender::CreateRenderGame(world,view,nullptr);
    CObj<NativeAmbientEffectMission> mission=new NativeAmbientEffectMission;mission->Attach(world,game,view,renderer);
    Require(view && renderer && world->GetOwnScript(),"immediate ambient fixture lacks native world/Lua/view/renderer");
    vector<CDBPtr<NDb::CAmbientLight>> records;
    CDBIterator<NDb::CAmbientLight> lights(*NDatabase::GetTable<NDb::CAmbientLight>());
    while(lights.MoveNext() && records.size()<3)records.push_back(lights.Get());
    Require(records.size()==3,"immediate ambient fixture needs three registered light records");
    for(int i=0;i<3;++i){auto* light=records[i].GetPtr();const float k=0.1f*(i+1);
        light->vAmbientColor=CVec3(k,k+0.01f,k+0.02f);light->vGroundAmbientColor=CVec3(k+0.03f,k+0.04f,k+0.05f);
        light->vLightColor=CVec3(k+0.06f,k+0.07f,k+0.08f);light->vGlossColor=CVec3(k,k,k);
        light->vFogColor=CVec3(k+0.09f,k+0.10f,k+0.11f);light->vVapourColor=CVec3(k,k,k);
        light->vShadowColor=CVec3(k,k,k);light->vBackColor=CVec3(k+0.12f,k+0.13f,k+0.14f);
        light->fPitch=30+i;light->fYaw=45+i;light->fFogDistance=100+10*i;light->fVapourHeight=2+i;
        light->fVapourDensity=0.1f;light->fVapourNoiseParam=1;light->fVapourSpeed=1;light->fVapourSwitchTime=1;
        light->fFogStartDistance=10+i;light->fVapourStartHeight=1;light->fBlurStrength=0;
        light->pSky=0;light->pGF2Light=0;light->bInGameUse=(i==0);light->flags.clear();
        if(i>0){NDb::SVariantFlags flags;flags.flags=i==1?dayFlags:nightFlags;light->flags.push_back(flags);}
    }
    vector<CDBPtr<NDb::CTAmbientLight>> pools;CDBIterator<NDb::CTAmbientLight> templates(*NDatabase::GetTable<NDb::CTAmbientLight>());
    while(templates.MoveNext()){auto* pool=templates.Get();pool->variants.clear();pool->variants.push_back(records[0].GetPtr());pool->roulette=CRoulette();pool->roulette.AddSector(1);pools.push_back(pool);}
    Require(pools.size()>=3,"immediate ambient fixture needs three registered light templates");
    CStructureNetworkContext worldRefs;auto worldFields=BorrowedObjectFields(world,&worldRefs);CDBPtr<NDb::CTAmbientLight> defaultPool;
    {CStructureSaver reader(worldFields,CStructureSaver::READ,&worldRefs);reader.Add(18,&defaultPool);}
    Require(IsValid(defaultPool),"authored fixture world has no actual default light template");
    auto* zonePool=defaultPool.GetPtr();zonePool->variants.clear();zonePool->variants.push_back(records[1].GetPtr());zonePool->variants.push_back(records[2].GetPtr());
    zonePool->roulette=CRoulette();zonePool->roulette.AddSector(1);zonePool->roulette.AddSector(1);
    NDb::CTAmbientLight* explicitPool=nullptr;for(const auto& pool:pools)if(pool.GetPtr()!=zonePool){explicitPool=pool;break;}
    Require(explicitPool!=nullptr,"immediate ambient fixture lacks a separate explicit light template");
    CStructureNetworkContext sceneRefs;auto sceneFields=BorrowedObjectFields(view->GetGScene(),&sceneRefs);CPtr<CFuncBase<CVec3>> mean;
    {CStructureSaver reader(sceneFields,CStructureSaver::READ,&sceneRefs);reader.Add(18,&mean);}Require(IsValid(mean),"immediate ambient fixture lacks native scene ambient graph");
    auto verify=[&](int expected){auto* actual=view->GetPrevLight();auto* wanted=records[expected].GetPtr();
        CDGPtr<CFuncBase<CVec3>> sample=mean.GetPtr();sample.Refresh();const CVec3 color=sample->GetValue();
        std::printf("native immediate ambient mode=%d tod=%d expectedRecord=%d actualAmbient=%.3f expectedAmbient=%.3f mean=%.3f flags=%zu\n",mode,int(world->GetTimeOfDay()),wanted->GetRecordID(),actual?actual->vAmbientColor.x:-1.0f,wanted->vAmbientColor.x,color.x,world->GetCreateFlags().size());std::fflush(stdout);
        Require(actual && fabs2(actual->vAmbientColor-wanted->vAmbientColor)<1e-10f && fabs2(actual->vGroundAmbientColor-wanted->vGroundAmbientColor)<1e-10f,"immediate ambient command selected a global/debug light instead of the current zone light");
        Require(fabs2(actual->vFogColor-wanted->vFogColor)<1e-10f && actual->fFogDistance==wanted->fFogDistance && fabs2(actual->vBackColor-wanted->vBackColor)<1e-10f,"immediate ambient lost zone fog or clear-color values");
        Require(fabs2(color-(wanted->vAmbientColor+wanted->vGroundAmbientColor)*0.5f)<1e-10f,"immediate ambient did not update the actual scene ambient graph");};
    auto timeOfDay=[&](NWorld::ETimeOfDay tod){const string lua="NativeAmbientTOD = SetTimeOfDay("+std::to_string(int(tod))+")";
        auto* script=world->GetOwnScript();Require(script->DoString(lua.c_str())==0,"could not execute actual SetTimeOfDay producer");world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        CObj<NWorld::CUICmd> command=world->GetUICommand();auto* ambient=dynamic_cast<NWorld::CUICmdSetAmbient*>(command.GetPtr());
        Require(ambient && ambient->bImmediate && !IsValid(ambient->pLight) && world->GetTimeOfDay()==tod,"SetTimeOfDay did not queue its native immediate null-template command");
        const int top=script->GetTop();auto returned=script->GetGlobal("NativeAmbientTOD");const bool correctReturn=returned.IsNumber() && returned.GetNumber()==int(tod);script->SetTop(top);Require(correctReturn,"SetTimeOfDay changed its original Lua return");
        world->AddUICommand(command);mission->Drain();Require(!world->GetUICommand(),"immediate ambient dispatch left a queued world command");};
    CObj<NWorld::CWorld> initializedWorld=world;
    if(mode==1){world=new NWorld::CWorld;mission->Attach(world,game,view,renderer);Require(world->GetDefaultLight()==nullptr,"bare control world unexpectedly has a default light");world->AddUICommand(new NWorld::CUICmdSetAmbient(nullptr,true));mission->Drain();verify(0);}
    else if(mode==0){timeOfDay(NWorld::TOD_NIGHT);verify(2);timeOfDay(NWorld::TOD_DAY);verify(1);}
    else if(mode==3){const string lua="SetupAmbientLight("+std::to_string(explicitPool->GetRecordID())+")";Require(world->GetOwnScript()->DoString(lua.c_str())==0,"could not execute actual SetupAmbientLight producer");world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);mission->Drain();verify(0);}
    else {world->AddUICommand(new NWorld::CUICmdSetAmbient(mode==2?explicitPool:nullptr,mode==2));mission->Drain();verify(mode==2?1:0);}
    Require(!NMainLoop::HaveInterfaceCommand(),"ambient command changed the active interface");
}


static void ScriptAmbientVariants(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=5,"unknown scripted ambient variant case");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm native scripted ambient variants");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}
    world->SetTimeOfDay(NWorld::TOD_DAY);const auto dayFlags=world->GetCreateFlags();
    world->SetTimeOfDay(NWorld::TOD_NIGHT);const auto nightFlags=world->GetCreateFlags();world->SetTimeOfDay(NWorld::TOD_DAY);
    CObj<NGScene::IGameView> view=NGScene::CreateNewView();CObj<NRender::IRenderGame> renderer=NRender::CreateRenderGame(world,view,nullptr);
    CObj<NativeAmbientEffectMission> mission=new NativeAmbientEffectMission;mission->Attach(world,game,view,renderer);
    vector<CDBPtr<NDb::CAmbientLight>> records;CDBIterator<NDb::CAmbientLight> lights(*NDatabase::GetTable<NDb::CAmbientLight>());
    while(lights.MoveNext() && records.size()<3)records.push_back(lights.Get());Require(records.size()==3,"ambient variant fixture lacks registered light records");
    for(int i=0;i<3;++i){auto* light=records[i].GetPtr();const float k=0.1f*(i+1);
        light->vAmbientColor=CVec3(k,k+0.01f,k+0.02f);light->vGroundAmbientColor=CVec3(k+0.03f,k+0.04f,k+0.05f);
        light->vLightColor=CVec3(k+0.06f,k+0.07f,k+0.08f);light->vGlossColor=CVec3(k,k,k);
        light->vFogColor=CVec3(k+0.09f,k+0.10f,k+0.11f);light->vVapourColor=CVec3(k,k,k);
        light->vShadowColor=CVec3(k,k,k);light->vBackColor=CVec3(k+0.12f,k+0.13f,k+0.14f);
        light->fPitch=30+i;light->fYaw=45+i;light->fFogDistance=100+10*i;light->fVapourHeight=2+i;
        light->fVapourDensity=0.1f;light->fVapourNoiseParam=1;light->fVapourSpeed=1;light->fVapourSwitchTime=1;
        light->fFogStartDistance=10+i;light->fVapourStartHeight=1;light->fBlurStrength=0;
        light->pSky=0;light->pGF2Light=0;light->bInGameUse=(i==0);light->flags.clear();
        if(i<2){NDb::SVariantFlags flags;flags.flags=i==0?dayFlags:nightFlags;light->flags.push_back(flags);}
    }
    CStructureNetworkContext worldRefs;auto worldFields=BorrowedObjectFields(world,&worldRefs);CDBPtr<NDb::CTAmbientLight> defaultPool;
    {CStructureSaver reader(worldFields,CStructureSaver::READ,&worldRefs);reader.Add(18,&defaultPool);}Require(IsValid(defaultPool),"ambient variant fixture lacks authored world default");
    vector<CDBPtr<NDb::CTAmbientLight>> otherPools;CDBIterator<NDb::CTAmbientLight> templates(*NDatabase::GetTable<NDb::CTAmbientLight>());
    while(templates.MoveNext())if(templates.Get()!=defaultPool.GetPtr())otherPools.push_back(templates.Get());Require(otherPools.size()>=2,"ambient variant fixture lacks explicit/fallback templates");
    auto setPool=[](NDb::CTAmbientLight* pool,NDb::CAmbientLight* light){pool->variants.clear();pool->variants.push_back(light);pool->roulette=CRoulette();pool->roulette.AddSector(3);};
    auto* pool=otherPools[0].GetPtr();auto* fallbackPool=otherPools[1].GetPtr();setPool(defaultPool,records[2]);setPool(pool,records[mode==0?1:0]);
    if(mode==1){setPool(fallbackPool,records[1]);records[0]->pGF2Light=fallbackPool;}
    if(mode==2 || mode==5){pool->variants.push_back(records[1].GetPtr());pool->roulette.AddSector(7);}
    if(mode==3)records[0]->flags.clear();
    if(mode==5)records[1]->flags[0].flags=dayFlags;
    CPtr<NDb::CAmbientLightReal> baseline=world->GetDefaultLight();Require(IsValid(baseline),"ambient variant fixture could not create baseline light");view->SetAmbient(baseline);
    CStructureNetworkContext sceneRefs;auto sceneFields=BorrowedObjectFields(view->GetGScene(),&sceneRefs);CPtr<CFuncBase<CVec3>> mean;
    {CStructureSaver reader(sceneFields,CStructureSaver::READ,&sceneRefs);reader.Add(18,&mean);}Require(IsValid(mean),"ambient variant fixture has no actual ambient graph");
    GlobalGameRandom().SeedForHarness(0x73da);SRand reference;vector<int> sequence;
    if(mode==5){for(int i=0;i<16;++i)sequence.push_back(reference.GetFloat(0,10)<3?0:1);Require(std::count(sequence.begin(),sequence.end(),0)>0 && std::count(sequence.begin(),sequence.end(),1)>0,"ambient stream control seed does not distinguish repeated initialization");}
    const int count=mode==5?16:mode==2?3:1;
    for(int step=0;step<count;++step){
        if(mode==2)world->SetTimeOfDay(step==1?NWorld::TOD_NIGHT:NWorld::TOD_DAY);
        const int expected=mode==0 || mode==4?2:mode==2?(step==1?1:0):mode==5?sequence[step]:0;
        const string lua="NativeAmbientApplied = 0; SetupAmbientLight("+std::to_string(mode==4?-1:pool->GetRecordID())+"); NativeAmbientApplied = 1";
        auto* script=world->GetOwnScript();Require(script->DoString(lua.c_str())==0,"ambient variants could not execute actual Lua producer");world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        const int top=script->GetTop();auto value=script->GetGlobal("NativeAmbientApplied");const bool executed=value.IsNumber() && value.GetNumber()==1;script->SetTop(top);Require(executed,"ambient variant Lua chunk did not reach its command producer");
        CObj<NWorld::CUICmd> command=world->GetUICommand();auto* ambient=dynamic_cast<NWorld::CUICmdSetAmbient*>(command.GetPtr());
        Require(mode==4?!IsValid(command):(ambient && !ambient->bImmediate && ambient->pLight.GetPtr()==pool),"SetupAmbientLight changed its valid/invalid native command contract");
        if(command)world->AddUICommand(command);auto expectedGameRandom=GlobalGameRandom();mission->Drain();auto actualGameRandom=GlobalGameRandom();Require(actualGameRandom.Get()==expectedGameRandom.Get(),"ambient variant dispatch consumed gameplay ISAAC RNG");
        auto* actual=view->GetPrevLight();CDGPtr<CFuncBase<CVec3>> graph=mean.GetPtr();graph.Refresh();const CVec3 color=graph->GetValue();auto* wanted=records[expected].GetPtr();
        std::printf("native scripted ambient mode=%d step=%d tod=%d command=%d retained=%d alternate=%d actual=%.3f expected=%.3f mean=%.3f\n",mode,step,int(world->GetTimeOfDay()),int(IsValid(command)),int(actual==baseline.GetPtr()),int(actual && IsValid(actual->pGF2Light)),actual?actual->vAmbientColor.x:-1.f,wanted->vAmbientColor.x,color.x);std::fflush(stdout);
        if(mode==0 || mode==4)Require(actual==baseline.GetPtr(),"absent or invalid ambient variant replaced the current light");
        Require(actual && !IsValid(actual->pGF2Light),"world flags did not suppress an incompatible alternate light");
        Require(fabs2(actual->vAmbientColor-wanted->vAmbientColor)<1e-10f && actual->fFogDistance==wanted->fFogDistance,"ambient command ignored world flags or restarted its random stream");
        Require(fabs2(color-(wanted->vAmbientColor+wanted->vGroundAmbientColor)*0.5f)<1e-10f,"scripted ambient selection did not reach the native scene graph");
        Require(!world->GetUICommand() && !NMainLoop::HaveInterfaceCommand(),"ambient variants left an unexpected command or interface transition");
    }
}


static void InactiveRandomEncounter(const char* databasePath,const char* resourcePath,int wide,int mode)
{
    Require(wide>=0 && wide<=1 && mode>=0 && mode<=22,"unknown inactive random encounter case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"random encounter fixture began with an interface or command");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm inactive random encounter regression");
    if(wide){NGfx::SRenderTargetsInfo targets;Require(NGfx::SetMode(NGfx::SVideoMode(640,360,32,NGfx::WINDOWED),targets),"random encounter fixture could not select its wide viewport");}
    Require(!wide || S2UI::MapOffsetX()!=0,"wide encounter fixture did not activate centered map coordinates");
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} clock{NMainLoop::SetTimeForHarness(1000)};
    const bool samplerCase=mode>=11,radiusCase=mode>=19;
    const int policyIndex=radiusCase?7:(samplerCase?mode-11:0);
    const int radii[4]={31,32,256,257};
    const float radius=radiusCase?float(radii[mode-19]):64.f;
    const int probabilities[8]={0,0,50,50,50,99,99,100};
    const int wantedRolls[8]={0,1,49,50,51,98,99,99};
    const bool expectedSpawns[8]={true,false,true,true,false,true,true,true};
    const bool expectedSpawn=radiusCase?(mode==20 || mode==21):(samplerCase && expectedSpawns[policyIndex]);
    const CVec2 candidate(300+radius,300);
    struct InfoRestore {CObj<CChapterInfo> info;vector<SChapterSector> original;~InfoRestore(){if(info)info->sectorsSet.swap(original);}} info;
    CDBPtr<NDb::CChapterMap> map;SChapterSector random;
    CDBIterator<NDb::CChapterMap> chapters(*NDatabase::GetTable<NDb::CChapterMap>());
    while(chapters.MoveNext()){
        if(!IsValid(chapters.Get()->pBackground))continue;
        CDGPtr<CPtrFuncBase<CChapterInfo>> loader=shareChapterInfo.Get(chapters.Get()->GetRecordID());loader.Refresh();
        auto* value=loader->GetValue();if(!value)continue;
        for(const auto& sector:value->sectorsSet)if(sector.eType==RANDOM && !sector.pointsSet.empty()){
            map=chapters.Get();random=sector;info.info=value;break;}
        if(map)break;
    }
    Require(map && info.info,"random encounter fixture lacks an actual authored chapter/random sector");
    if(samplerCase){random.nProbability=probabilities[policyIndex];
        random.pointsSet={CVec2(candidate.x-1,candidate.y-1),CVec2(candidate.x+1,candidate.y-1),CVec2(candidate.x+1,candidate.y+1),CVec2(candidate.x-1,candidate.y+1)};}
    info.original=info.info->sectorsSet;info.info->sectorsSet=vector<SChapterSector>{random};
    CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    struct ScriptRestore {CPtr<NDb::CChapterMap> map;CPtr<NDb::CScript> script;~ScriptRestore(){if(map)map->pScript=script;}} script{map.GetPtr(),map->pScript};map->pScript=quiet.GetPtr();
    auto* campaign=NDb::GetGlobalMap(3);Require(campaign && campaign->pScenario,"random encounter fixture lacks its authored campaign");
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(campaign->pScenario->GetRecordID());game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    list<CPtr<NScenario::CScenarioClue>> clues;game->pScenarioTracker->GetAvailableClues(&clues);for(auto clue:clues)if(clue)clue->SetJustFound(false);
    for(auto merc:game->players.front()->mercs){vector<CDBPtr<NDb::CMedal>> medals;merc->GetJustFoundMedals(&medals);}
    CObj<NGame::CICBeginChapter> enter=new NGame::CICBeginChapter(map->GetRecordID(),game);enter->Exec();
    CObj<NGame::CMissionBase> chapter=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* ui=chapter?dynamic_cast<NUI::CChapterMapUI*>(chapter->GetInterface()->GetChildByID("chaptermapUI")):nullptr;
    Require(ui && chapter->GetWorld(),"random encounter fixture did not open its native chapter");
    auto sectorWindow=[](NUI::CChapterMapUI* mapUI){
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(mapUI,&refs);vector<CPtr<CObjectBase>> sectors;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(17,&sectors);}
        Require(sectors.size()==1 && pSSClasses->GetObjectTypeID(sectors.front())==int(0xb2606154),"chapter fixture lost its one actual CRandomSector");
        auto* window=dynamic_cast<NUI::CWindow*>(sectors.front().GetPtr());Require(window,"native random sector is not a window");return window;};
    auto* marker=sectorWindow(ui);marker->SetPosition(NUI::SPoint(mode>=4?200:0,mode>=4?200:0));marker->SetStyle(NUI::STYLE_VISIBLE,true);
    NUI::SPoint screen;NUI::SRect rectangle;Require(marker->ClientToScreen(&screen,&rectangle),"random encounter fixture has no native marker rectangle");
    const bool active=mode==2 || mode==3 || mode==6 || mode==7,overlap=mode==0 || mode==1 || mode==2 || mode==4 || mode==6 || mode==7;
    CVec2 team(float((rectangle.x1+rectangle.x2)/2-S2UI::MapOffsetX()),float((rectangle.y1+rectangle.y2)/2-S2UI::MapOffsetY()));
    if(!overlap)team+=CVec2(100,100);if(samplerCase)team=CVec2(300,300);game->vChapterPos=team;
    if(mode==8 || mode==9)game->nCurrentTemplateID=random.nTemplate;
    if(mode==10)game->nCurrentTemplateID=random.nTemplate+1;
    const int uiType=pSSClasses->GetObjectTypeID(ui);
    // Build a complete ordinary save graph, alter fixture state in its bytes,
    // then load fresh objects. Never partially deserialize into a live object.
    CMemoryStream saved;{CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(2,&chapter);writer.Add(3,&game);}
    vector<unsigned char> bytes(saved.GetSize());saved.Seek(0);saved.Read(bytes.data(),unsigned(bytes.size()));
    auto field=[](unsigned char* data,size_t size,unsigned id){S2FileIO::StructureChunk result{};int found=0;
        for(size_t at=0;at<size;){S2FileIO::StructureChunk item;Require(S2FileIO::DecodeStructureChunkAt(data,size,at,&item),"invalid native random encounter save field");
            if(item.id==id){result=item;++found;}at=size_t(item.payloadOffset)+item.length;}
        Require(found==1,"native random encounter save field is missing or duplicated");return result;};
    auto table=field(bytes.data(),bytes.size(),0),bodies=field(bytes.data(),bytes.size(),2);
    std::vector<S2FileIO::StructureObjectRecord> records;std::vector<S2FileIO::StructureObjectBody> objects;
    Require(S2FileIO::DecodeStructureObjectTable(bytes.data()+table.payloadOffset,table.length,&records) &&
        S2FileIO::IndexStructureObjectBodies(bytes.data()+bodies.payloadOffset,bodies.length,&objects),"invalid native random encounter object graph");
    auto body=[&](unsigned type){unsigned id=0;int found=0;for(auto record:records)if(record.typeId==type){id=record.wireId;++found;}
        Require(found==1 && id,"random encounter fixture type is absent or duplicated");
        for(auto object:objects)if(object.wireId==id)return std::make_pair(bytes.data()+bodies.payloadOffset+object.bodyOffset,size_t(object.bodyLength));
        Require(false,"native random encounter object body is missing");return std::make_pair(static_cast<unsigned char*>(nullptr),size_t(0));};
    auto mapBody=body(unsigned(uiType));
    for(unsigned id:{8u,9u}){auto point=field(mapBody.first,mapBody.second,id);Require(point.length==sizeof(team),"chapter position save shape changed");std::memcpy(mapBody.first+point.payloadOffset,&team,sizeof(team));}
    auto randomBody=body(0xb2606154u);auto base=field(randomBody.first,randomBody.second,1);auto visible=field(randomBody.first+base.payloadOffset,base.length,3);
    Require(S2FileIO::EncodeStructureScalar(active,randomBody.first+base.payloadOffset+visible.payloadOffset,visible.length),"could not encode logical encounter activity");
    const STime deadline=(mode==7 || mode==8 || mode==10 || samplerCase)?1000:100000,morph=1100;const float coefficient=mode==0?0.f:1.f;
    for(auto entry:{std::make_pair(3u,deadline),std::make_pair(6u,morph)}){auto item=field(randomBody.first,randomBody.second,entry.first);
        Require(S2FileIO::EncodeStructureScalar(entry.second,randomBody.first+item.payloadOffset,item.length),"could not encode native encounter fixture clock");}
    if(samplerCase){const CTRect<float> fixed(candidate.x,candidate.y,candidate.x,candidate.y);auto zone=field(randomBody.first,randomBody.second,4);
        Require(zone.length==sizeof(fixed),"native random sampler bounds save shape changed");std::memcpy(randomBody.first+zone.payloadOffset,&fixed,sizeof(fixed));}
    auto fade=field(randomBody.first,randomBody.second,5);Require(S2FileIO::EncodeStructureScalar(coefficient,randomBody.first+fade.payloadOffset,fade.length),"could not encode native encounter fade");
    CMemoryStream wire;wire.Write(bytes.data(),unsigned(bytes.size()));wire.Seek(0);
    CObj<NGame::CMissionBase> restored;CObj<NRPG::CGlobalGame> restoredGame;
    {CStructureSaver reader(wire,CStructureSaver::READ);reader.Add(2,&restored);reader.Add(3,&restoredGame);}
    class RestoreChapter:public NMainLoop::CInterfaceCommand {OBJECT_NOCOPY_METHODS(RestoreChapter);
    public:CObj<NGame::CMissionBase> chapter;void Exec() override {chapter->OnSnapshotRestored();SetInterface(chapter);}};
    CObj<RestoreChapter> load=new RestoreChapter;load->chapter=restored;load->Exec();chapter=restored;game=restoredGame;
    ui=dynamic_cast<NUI::CChapterMapUI*>(chapter->GetInterface()->GetChildByID("chaptermapUI"));Require(ui && chapter->GetRPGGame()==game.GetPtr(),"random encounter save lost its actual UI/campaign alias");
    marker=sectorWindow(ui);const bool hit=marker->HitTest(int(team.x)+S2UI::MapOffsetX(),int(team.y)+S2UI::MapOffsetY());
    Require(hit==overlap && marker->GetStyle(NUI::STYLE_VISIBLE),"native random encounter fixture did not preserve overlap/draw visibility");
    auto state=[&](){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(marker,&refs);STime actualDeadline=0;bool logical=false;
        struct Sector {bool* logical;int operator&(CStructureSaver& f){f.Add(3,logical);return 0;}} sector{&logical};
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,&sector);reader.Add(3,&actualDeadline);}return std::make_pair(logical,actualDeadline);};
    Require(state()==std::make_pair(active,deadline),"native random encounter fixture lost logical state/deadline");
    if(samplerCase){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(marker,&refs);CTRect<float> zone;SChapterSector actual;
        struct Base {SChapterSector* sector;int operator&(CStructureSaver& f){f.Add(2,sector);return 0;}} base{&actual};
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,&base);reader.Add(4,&zone);}
        Require(actual.nProbability==probabilities[policyIndex] && actual.eType==RANDOM && IsPointInPolygon(actual.pointsSet,candidate) &&
            zone.x1==candidate.x && zone.x2==candidate.x && zone.y1==candidate.y && zone.y2==candidate.y && fabs(team-candidate)==radius,
            "native chance fixture lost its actual probability/polygon/constant legal sampler input");}
    unsigned chosenSeed=0;STime expectedPolicyDeadline=0;int referenceRoll=-1;
    if(samplerCase){for(;chosenSeed<100000;++chosenSeed){std::int32_t seed=std::int32_t(chosenSeed);S2Random::Next(&seed,4);S2Random::Next(&seed,0xffff);
            if(S2Random::Next(&seed,100)==wantedRolls[policyIndex])break;}
        Require(chosenSeed<100000,"native chance fixture could not choose a deterministic roll");
        // Existing harness API also seeds lazily constructed SRand instances.
        // Its constructor consumes Get(4), then the real update draws timer/roll.
        GlobalGameRandom().SeedForHarness(chosenSeed);SRand reference;
        expectedPolicyDeadline=STime(1100+10000*reference.GetFloat(.5f,1.f));referenceRoll=reference.Get(100);
        Require(referenceRoll==wantedRolls[policyIndex],"native chance fixture seed does not match its reference stream");}
    const float xp=game->players.front()->mercs.front()->fXP;const int currentTemplate=game->nCurrentTemplateID;
    std::printf("native random encounter wide=%d mode=%d stage=before chapter=%d template=%d active=%d visible=1 hit=%d offset=%d,%d team=%.1f,%.1f deadline=%u\n",wide,mode,map->GetRecordID(),random.nTemplate,int(active),int(hit),S2UI::MapOffsetX(),S2UI::MapOffsetY(),team.x,team.y,unsigned(deadline));std::fflush(stdout);
    NMainLoop::SetTimeForHarness(1100);Require(NMainLoop::StepApp(true,false,false),"native chapter encounter frame exited");
    CObj<NMainLoop::CInterfaceCommand> command=NMainLoop::TakeInterfaceCommandForHarness();
    const bool queued=command.GetPtr()!=nullptr;
    std::printf("native random encounter wide=%d mode=%d stage=after queued=%d expected=%d logical=%d deadline=%u\n",wide,mode,int(queued),int(active && overlap),int(state().first),unsigned(state().second));std::fflush(stdout);
    Require(queued==(active && overlap),"inactive random encounter marker queued a phantom mission entry");
    Require(!queued || dynamic_cast<NMainLoop::CICContainer*>(command.GetPtr()),"active encounter did not queue its native autosave/mission container");
    if(mode==7)Require(state().second==deadline,"entering an active encounter continued its spawn timer after queueing the mission");
    if(mode==6 || mode==7)Require(!state().first,"entering an active encounter left its marker active for the queued autosave");
    const int expectedTemplate=mode==8?0:currentTemplate;
    if(mode>=8){std::printf("native random encounter wide=%d mode=%d stage=template before=%d after=%d expected=%d deadline=%u\n",wide,mode,currentTemplate,game->nCurrentTemplateID,expectedTemplate,unsigned(state().second));std::fflush(stdout);
        Require(game->nCurrentTemplateID==expectedTemplate,"finished random encounter template was not cleared on its due update");}
    if(samplerCase){
        std::printf("native random encounter wide=%d mode=%d stage=chance percent=%d referenceRoll=%d seed=%u active=%d expected=%d deadline=%u referenceDeadline=%u\n",wide,mode,probabilities[policyIndex],referenceRoll,chosenSeed,int(state().first),int(expectedSpawn),unsigned(state().second),unsigned(expectedPolicyDeadline));std::fflush(stdout);
        Require(state().second==expectedPolicyDeadline,"native random sampler did not align with its reference timer draw");
        if(radiusCase){std::printf("native random encounter wide=%d mode=%d stage=radius radius=%.1f active=%d expected=%d\n",wide,mode,radius,int(state().first),int(expectedSpawn));std::fflush(stdout);
            Require(state().first==expectedSpawn,"native random encounter distance violates the original radius bounds");}
        else Require(state().first==expectedSpawn,"native random encounter chance comparison reverses the authored probability");
        if(expectedSpawn){NUI::SPoint point;NUI::SRect bounds;Require(marker->ClientToScreen(&point,&bounds),"spawned native encounter marker has no window rectangle");
            Require((bounds.x1+bounds.x2)/2-S2UI::MapOffsetX()==int(candidate.x) && (bounds.y1+bounds.y2)/2-S2UI::MapOffsetY()==int(candidate.y),"spawned encounter lost the own centered map coordinate adapter");}}
    const bool validDeadline=samplerCase?state().second==expectedPolicyDeadline:(mode==8 || mode==10)?state().second>1100:state().second==deadline;
    Require(!NMainLoop::HaveInterfaceCommand() && NMainLoop::GetCurrentInterfaceForHarness()==chapter.GetPtr() && validDeadline && game->nCurrentTemplateID==expectedTemplate && game->players.front()->mercs.front()->fXP==xp,
        "encounter activity gate changed its native queue, timer, campaign or XP");
}


static void GlobalMapDesktop(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=9,"unknown global map desktop case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"global desktop fixture began with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm global map desktop regression");
    struct OptionRestore {NGlobal::CValue globe,hints;~OptionRestore(){NGlobal::SetVar("cheat_global_xcom",globe);NGlobal::SetVar("ui_showhints",hints);}}
        options{NGlobal::GetVar("cheat_global_xcom",0),NGlobal::GetVar("ui_showhints",1)};
    const bool globe=mode==1 || mode==7;
    NGlobal::SetVar("cheat_global_xcom",1);if(!globe)NGlobal::SetVar("cheat_global_xcom",0);
    NGlobal::SetVar("ui_showhints",0);
    CDBPtr<NDb::CGlobalMap> map=NDb::GetGlobalMap(3);Require(map && map->pScenario,"global desktop fixture lacks Axis campaign");
    struct MapRestore {CPtr<NDb::CGlobalMap> map;CPtr<NDb::CScript> script;CPtr<NDb::CDBScenarioZone> base;
        ~MapRestore(){map->pScript=script;map->pBaseZone=base;}} restore{map.GetPtr(),map->pScript,map->pBaseZone};
    map->pScript=nullptr;
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(map->pScenario->GetRecordID());
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));game->bGlobalMapSet=true;game->nGlobalMapID=3;
    auto* recommended=game->pScenarioTracker->GetRecommendedZone(game->players.front());
    auto* base=game->pScenarioTracker->GetZoneByDBZone(map->pBaseZone);
    Require(recommended && base && recommended!=base,"global desktop fixture lost distinct actual base/recommendation");
    if(mode==3)map->pBaseZone=recommended->GetDBZone();
    const float xp=game->players.front()->mercs.front()->fXP;
    CObj<NGame::CICContinueGlobal> open=new NGame::CICContinueGlobal(game);open->Exec();
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(owner && owner->GetRPGGame()==game && owner->GetWorld() && owner->GetInterface(),"global desktop fixture failed actual campaign entry");
    auto* ui=owner->GetInterface()->GetChildByID("globalmapUI");Require(ui,"global desktop fixture lacks actual global UI");
    auto print=[&](const char* stage){std::printf("native global desktop mode=%d stage=%s globe=%d desktop=%d client=%d world=1 xp=%.3f stack=%d\n",mode,stage,int(dynamic_cast<NUI::CXComMapUI*>(ui)!=nullptr),int(owner->GetDesktop()==ui),int(owner->GetDesktop() && owner->GetDesktop()->GetClientWindow()==ui->GetChildByID("view")),game->players.front()->mercs.front()->fXP,NMainLoop::GetInterfaceStackDepth());std::fflush(stdout);};
    if(mode>=6 && mode<=7 || mode==9){
        list<CObj<NMainLoop::IInterfaceBase>> original;original.push_back(owner.GetPtr());CMemoryStream saved;
        {CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(2,&original);}
        vector<unsigned char> bytes(saved.GetSize());saved.Seek(0);saved.Read(bytes.data(),unsigned(bytes.size()));
        if(mode==9){
            // Rebuild only the two known historical own-save fields: flat CWindow
            // UI base and empty mission desktop list. Retain the complete native
            // object table, actual UI children, campaign and logical world graph.
            using Bytes=vector<unsigned char>;
            auto fields=[](const Bytes& data){vector<std::pair<unsigned,Bytes>> result;
                for(size_t at=0;at<data.size();){S2FileIO::StructureChunk c;Require(S2FileIO::DecodeStructureChunkAt(data.data(),data.size(),at,&c),"invalid native global desktop chunk");
                    result.push_back({c.id,Bytes(data.begin()+c.payloadOffset,data.begin()+c.payloadOffset+c.length)});at=size_t(c.payloadOffset)+c.length;}return result;};
            auto pack=[](const vector<std::pair<unsigned,Bytes>>& parts){Bytes result;for(const auto& p:parts){unsigned char length[4];size_t n=0;
                Require(S2FileIO::EncodeStructureLength(unsigned(p.second.size()),length,&n),"cannot encode legacy desktop chunk");
                result.push_back(static_cast<unsigned char>(p.first));result.insert(result.end(),length,length+n);result.insert(result.end(),p.second.begin(),p.second.end());}return result;};
            auto top=fields(bytes);vector<S2FileIO::StructureObjectRecord> records;
            for(const auto& part:top)if(part.first==0)Require(S2FileIO::DecodeStructureObjectTable(part.second.data(),part.second.size(),&records),"invalid desktop object table");
            unsigned missionID=0,uiID=0;for(auto record:records){if(record.typeId==0xb1808180u)missionID=record.wireId;if(record.typeId==0xb0912170u)uiID=record.wireId;}
            Require(missionID && uiID,"legacy desktop fixture lacks actual global map/UI IDs");int rewritten=0;
            for(auto& part:top)if(part.first==2){auto objects=fields(part.second);
                for(auto& object:objects){auto entry=fields(object.second);unsigned id=0;
                    for(const auto& item:entry)if(item.first==0)Require(S2FileIO::DecodeStructureScalar(item.second.data(),item.second.size(),&id),"invalid native desktop object ID");
                    if(id!=missionID && id!=uiID)continue;
                    for(auto& item:entry)if(item.first==1){auto root=fields(item.second);
                        for(auto& field:root)if(field.first==1){auto inner=fields(field.second);
                            if(id==uiID){for(const auto& nested:inner)if(nested.first==1){field.second=nested.second;break;}++rewritten;}
                            else{for(auto& nested:inner)if(nested.first==23){nested.second.clear();++rewritten;}field.second=pack(inner);}}
                        item.second=pack(root);}object.second=pack(entry);}part.second=pack(objects);}
            Require(rewritten==2,"legacy desktop fixture did not preserve both historical field forms");bytes=pack(top);
        }
        CMemoryStream snapshot;snapshot.Write(bytes.data(),unsigned(bytes.size()));snapshot.Seek(0);
        list<CObj<NMainLoop::IInterfaceBase>> restored;{CStructureSaver reader(snapshot,CStructureSaver::READ);reader.Add(2,&restored);}
        Require(restored.size()==1,"global map native save lost stack");
        owner=dynamic_cast<NGame::CMissionBase*>(restored.front().GetPtr());Require(owner && owner.GetPtr()!=original.front().GetPtr(),"global save did not create fresh map");
        owner->OnSnapshotRestored();owner->OnSnapshotRestored();game=owner->GetRPGGame();ui=owner->GetInterface()->GetChildByID("globalmapUI");
        Require(ui && ui->GetInterface()==owner->GetInterface() && ui->GetWindowID()=="globalmapUI" && ui->GetChildByID("view"),"native legacy/current UI lost window fields or client subtree");
        CStructureNetworkContext refs;auto state=BorrowedObjectFields(owner,&refs);struct MissionFields {list<CPtr<NUI::CDesktopWindow>> desktops;int operator&(CStructureSaver& f){f.Add(23,&desktops);return 0;}} mission;
        {CStructureSaver reader(state,CStructureSaver::READ,&refs);reader.Add(1,&mission);}
        print(mode==9?"legacy-restored":"restored");
        Require(mission.desktops.size()==1 && mission.desktops.back().GetPtr()==ui,"native restored global map did not repair/retain exactly one desktop");
        Require(owner->GetDesktop() && owner->GetDesktop()->GetClientWindow()==ui->GetChildByID("view"),"native restored global map lost actual desktop client ownership");
    }else if(mode==2 || mode==3 || mode==4 || mode==5){
        CStructureNetworkContext refs;auto state=BorrowedObjectFields(ui,&refs);CPtr<NUI::CFlashButton> baseButton;vector<CPtr<NUI::CWindow>> sectors;
        {CStructureSaver reader(state,CStructureSaver::READ,&refs);reader.Add(8,&baseButton);reader.Add(9,&sectors);}
        Require(baseButton && !sectors.empty(),"normal map lacks authored base button and sectors");
        auto sector=sectors.front();struct SectorState {SGlobalSector sector;bool selected=false;int operator&(CStructureSaver& f){f.Add(2,&sector);f.Add(3,&selected);return 0;}} before,after;
        auto readSector=[&](SectorState* out){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(sector,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,out);};readSector(&before);
        NUI::SPoint cursor(-1000,-1000);if(mode==4){bool found=false;for(int y=0;y<768 && !found;++y)for(int x=0;x<1024;++x)if(IsPointInPolygon(before.sector.pointsSet,CVec2(x,y))){cursor=NUI::SPoint(x+S2UI::MapOffsetX(),y+S2UI::MapOffsetY());found=true;break;}Require(found,"actual global sector has no native polygon interior");}
        // Supply a cursor input only. The actual interface Draw must perform the
        // native hover/flash work without a preceding Update/Step dispatch.
        CMemoryStream input;{CStructureSaver writer(input,CStructureSaver::WRITE);writer.Add(5,&cursor);}input.Seek(0);
        {CStructureSaver reader(input,CStructureSaver::READ);static_cast<CObjectBase*>(owner->GetInterface())->operator&(reader);}
        Require(owner->GetInterface()->GetCursorPos()==cursor,"native cursor input did not round trip");
        owner->GetInterface()->Draw(1000);NGScene::Flip();readSector(&after);
        std::printf("native global desktop mode=%d stage=draw selectedBefore=%d selectedAfter=%d flash=%d expectedFlash=%d cursor=%d,%d sectors=%zu\n",mode,int(before.selected),int(after.selected),int(baseButton->GetFlashMode()),int(mode==3),cursor.x,cursor.y,sectors.size());std::fflush(stdout);
        if(mode==4 || mode==5)Require(after.selected==(mode==4),"actual global Draw did not refresh native polygon hover without Update");
        else Require(baseButton->GetFlashMode()==(mode==3),"actual global Draw did not refresh authored base recommendation flash");
    }else{
        print("entry");Require(globe?dynamic_cast<NUI::CXComMapUI*>(ui)!=nullptr:dynamic_cast<NUI::CGlobalMapUI*>(ui)!=nullptr,"native global entry ignored globe/default selection");
        if(mode!=8)Require(owner->GetDesktop()==ui && owner->GetDesktop()->GetClientWindow()==ui->GetChildByID("view"),"native global map did not own its actual desktop/client");
    }
    Require(game->players.front()->mercs.front()->fXP==xp && NMainLoop::GetInterfaceStackDepth()==1 && !NMainLoop::HaveInterfaceCommand(),"global desktop changed campaign XP or queued unrelated UI");
    print("complete");
}

static void GlobalMapScriptLifecycle(const char* databasePath,const char* resourcePath,int context,int mode)
{
    Require(context>=0 && context<=1 && mode>=0 && mode<=6,"unknown global-map script lifecycle case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"global-map script fixture began with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm global map script lifecycle");
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} clock{NMainLoop::SetTimeForHarness(1000)};
    // Select the authored Axis map explicitly; database iteration order differs by platform.
    CDBPtr<NDb::CGlobalMap> map=NDb::GetGlobalMap(3);
    Require(IsValid(map),"global-map script fixture has no authored map/scenario/script record");
    const string authored=map->pScript->strCode;const int authoredScript=map->pScript->GetRecordID();
    Require(authored.find("gmapgfirstenter")!=string::npos && authored.find("ShowHint( 47 )")!=string::npos,"global-map fixture no longer selects its actual known first-country script");
    auto* authoredHint=NDb::GetUIHint(47);Require(authoredHint && authoredHint->pString,"global-map script lacks its actual authored hint47");
    std::printf("native global-map hint47 reward=%d\n",authoredHint->nSequenceID);
    std::printf("native global-map script context=%d mode=%d map=%d authoredScript=%d bytes=%zu\n",context,mode,map->GetRecordID(),authoredScript,authored.size());
    if(context==0 && mode==2)std::printf("native global-map authored script begin\n%s\nnative global-map authored script end\n",authored.c_str());std::fflush(stdout);
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(map->pScenario->GetRecordID());game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    game->bGlobalMapSet=true;game->nGlobalMapID=map->GetRecordID();if(mode==3)game->SetGlobalVar("gmapgfirstenter","1");
    list<CPtr<NScenario::CScenarioClue>> clues;game->pScenarioTracker->GetAvailableClues(&clues);for(auto clue:clues)if(IsValid(clue))clue->SetJustFound(false);
    for(auto merc:game->players.front()->mercs){vector<CDBPtr<NDb::CMedal>> medals;merc->GetJustFoundMedals(&medals);}
    struct OptionRestore {NGlobal::CValue previous;~OptionRestore(){NGlobal::SetVar("ui_showhints",previous);}} hints{NGlobal::GetVar("ui_showhints",1)};NGlobal::SetVar("ui_showhints",mode>=3?1:0);
    CObj<NDb::CScript> probe=new NDb::CScript;probe->strCode=(mode==1 || mode>=5)?
        "GlobalMapRuns = (GlobalMapRuns or 0) + 1; function OnEnterZone() GlobalMapEnters = (GlobalMapEnters or 0) + 1; Sleep(1); GlobalMapWake = 1 end":
        "GlobalMapRuns = (GlobalMapRuns or 0) + 1; "+authored;
    struct ScriptRestore {CPtr<NDb::CGlobalMap> map;CPtr<NDb::CScript> script;CPtr<NDb::CChapterMap> chapter;CPtr<NDb::CScript> chapterScript;
        ~ScriptRestore(){if(IsValid(map))map->pScript=script;if(IsValid(chapter))chapter->pScript=chapterScript;}} restore{map.GetPtr(),map->pScript,nullptr,nullptr};
    map->pScript=mode==0?nullptr:probe.GetPtr();
    struct SectorRestore {CObj<CChapterInfo> info;vector<SChapterSector> original;~SectorRestore(){if(IsValid(info))info->sectorsSet.swap(original);}} sectors;
    CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    CObj<NGame::CMissionBase> underlying;
    if(context==1){
        CDBIterator<NDb::CChapterMap> chapters(*NDatabase::GetTable<NDb::CChapterMap>());while(chapters.MoveNext())if(IsValid(chapters.Get()->pBackground)){restore.chapter=chapters.Get();break;}
        Require(IsValid(restore.chapter),"global overview fixture lacks an actual underlying chapter");restore.chapterScript=restore.chapter->pScript;restore.chapter->pScript=quiet.GetPtr();
        CDGPtr<CPtrFuncBase<CChapterInfo>> info=shareChapterInfo.Get(restore.chapter->GetRecordID());info.Refresh();sectors.info=info->GetValue();Require(sectors.info,"global overview fixture has no native chapter info");
        sectors.original=sectors.info->sectorsSet;auto& live=sectors.info->sectorsSet;live.erase(std::remove_if(live.begin(),live.end(),[](const SChapterSector& s){return s.eType==RANDOM;}),live.end());
        CObj<NGame::CICBeginChapter> open=new NGame::CICBeginChapter(restore.chapter->GetRecordID(),game);open->Exec();underlying=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
        Require(underlying && underlying->GetWorld(),"global overview fixture did not open its real chapter");
        CObj<NGame::CICShowGlobal> show=new NGame::CICShowGlobal(game);show->Exec();
    }else {CObj<NGame::CICContinueGlobal> open=new NGame::CICContinueGlobal(game);open->Exec();}
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(owner && owner.GetPtr()!=underlying.GetPtr() && pSSClasses->GetObjectTypeID(owner)==int(0xb1808180) && owner->GetRPGGame()==game.GetPtr() && owner->IsGlobalMapShowMode()==(context==1),"native global-map entry lost its real UI/game/show mode");
    Require(owner->GetInterface() && owner->GetInterface()->GetChildByID("globalmapUI") && owner->GetDesktop()==owner->GetInterface()->GetChildByID("globalmapUI") && owner->GetSoundScene()==owner->GetInterface()->GetSound(),"global-map fixture lost its actual desktop/shared-audio contract");
    const bool chapterFlag=game->bChapterMapSet;const float initialXP=game->players.front()->mercs.front()->fXP;
    STime time=1050;auto frame=[&](bool expectOwner=true){time+=50;NMainLoop::SetTimeForHarness(time);Require(NMainLoop::StepApp(true,false,false),"global-map native frame exited");if(expectOwner)Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr(),"global-map native frame opened an unrelated interface");};
    auto* world=dynamic_cast<NWorld::CWorld*>(owner->GetWorld());
    auto number=[&](const char* name,int expected){if(!world || !world->GetOwnScript())return false;auto* script=world->GetOwnScript();const int top=script->GetTop();auto value=script->GetGlobal(name);const bool result=value.IsNumber() && value.GetNumber()==expected;script->SetTop(top);return result;};
    std::printf("native global-map script context=%d mode=%d stage=context world=%d active=%d render=%d stack=%d\n",context,mode,int(world!=nullptr),int(owner->GetActivePlayer()!=nullptr),int(owner->GetRenderGame()!=nullptr),NMainLoop::GetInterfaceStackDepth());std::fflush(stdout);
    if(mode>=5){
        if(mode==6){
            Require(world && world->GetOwnScript(),"native global map has no logical world/player/renderer to execute its map script");
            for(int i=0;i<32 && !number("GlobalMapWake",1);++i)frame();
            Require(number("GlobalMapRuns",1) && number("GlobalMapEnters",1) && number("GlobalMapWake",1),"global-map snapshot source never completed its real probe");
        }
        // Use the native save graph, not a network replica: network snapshots
        // deliberately omit AI commanders and UI-only state needed for local resume.
        list<CObj<NMainLoop::IInterfaceBase>> original;if(context==1)original.push_back(underlying.GetPtr());original.push_back(owner.GetPtr());
        CMemoryStream saved;
        {CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(2,&original);}
        vector<unsigned char> bytes(saved.GetSize());saved.Seek(0);saved.Read(bytes.data(),unsigned(bytes.size()));
        if(mode==5){
            // Clear only the global map's serialized logical-context references.
            // Keep every tag and the complete UI/campaign graph; never partially READ
            // into a live engine object and never retain an old executable/build.
            S2FileIO::StructureChunk table{},bodies{};
            for(size_t at=0;at<bytes.size();){S2FileIO::StructureChunk field;
                Require(S2FileIO::DecodeStructureChunkAt(bytes.data(),bytes.size(),at,&field),"invalid native global-map save chunk");
                Require(field.id!=3,"global-map fixture unexpectedly received compressed chunks");
                if(field.id==0)table=field;if(field.id==2)bodies=field;at=size_t(field.payloadOffset)+field.length;}
            std::vector<S2FileIO::StructureObjectRecord> records;
            std::vector<S2FileIO::StructureObjectBody> objects;
            Require(S2FileIO::DecodeStructureObjectTable(bytes.data()+table.payloadOffset,table.length,&records) &&
                S2FileIO::IndexStructureObjectBodies(bytes.data()+bodies.payloadOffset,bodies.length,&objects),"invalid global-map native object graph");
            unsigned id=0;int maps=0,cleared=0;
            for(auto record:records)if(record.typeId==0xb1808180u){id=record.wireId;++maps;}
            Require(maps==1 && id,"legacy-style save did not identify exactly one global map");
            for(auto object:objects)if(object.wireId==id){
                auto* root=bytes.data()+bodies.payloadOffset+object.bodyOffset;
                for(size_t at=0;at<object.bodyLength;){S2FileIO::StructureChunk outer;
                    Require(S2FileIO::DecodeStructureChunkAt(root,object.bodyLength,at,&outer),"invalid global-map native root field");
                    if(outer.id==1){auto* base=root+outer.payloadOffset;
                        for(size_t offset=0;offset<outer.length;){S2FileIO::StructureChunk field;
                            Require(S2FileIO::DecodeStructureChunkAt(base,outer.length,offset,&field),"invalid global-map native base field");
                            if(field.id==3 || field.id==5 || field.id==10 || field.id==11 || field.id==18){
                                Require(field.length==4,"logical context ref is not a native wire ID");std::memset(base+field.payloadOffset,0,4);++cleared;}
                            if(field.id==12){auto* players=base+field.payloadOffset;
                                for(size_t entry=0;entry<field.length;){S2FileIO::StructureChunk player;
                                    Require(S2FileIO::DecodeStructureChunkAt(players,field.length,entry,&player) && player.id==1 && player.length==4,"invalid native player tracker vector");
                                    std::memset(players+player.payloadOffset,0,4);entry=size_t(player.payloadOffset)+player.length;}}
                            offset=size_t(field.payloadOffset)+field.length;}}
                    at=size_t(outer.payloadOffset)+outer.length;}}
            Require(cleared==5,"legacy-style save did not clear all five logical context refs");
        }
        CMemoryStream snapshot;snapshot.Write(bytes.data(),unsigned(bytes.size()));snapshot.Seek(0);
        list<CObj<NMainLoop::IInterfaceBase>> restored;
        {CStructureSaver reader(snapshot,CStructureSaver::READ);reader.Add(2,&restored);}
        Require(restored.size()==size_t(context+1),"native global-map snapshot lost its actual interface stack");
        CObj<NGame::CMissionBase> restoredMap=dynamic_cast<NGame::CMissionBase*>(restored.back().GetPtr());
        Require(restoredMap && restoredMap.GetPtr()!=owner.GetPtr() && pSSClasses->GetObjectTypeID(restoredMap)==int(0xb1808180),"snapshot did not construct an actual fresh global-map interface");
        CObj<NRPG::CGlobalGame> restoredGame=restoredMap->GetRPGGame();Require(restoredGame && restoredGame.GetPtr()!=game.GetPtr(),"native global-map snapshot lost its new campaign graph");
        Require(mode==5?restoredMap->GetWorld()==nullptr:restoredMap->GetWorld()!=nullptr,"legacy/current snapshot did not preserve its intended context presence");
        class RestoreStack:public NMainLoop::CInterfaceCommand {OBJECT_NOCOPY_METHODS(RestoreStack);
        public:list<CObj<NMainLoop::IInterfaceBase>> stack;
            void Exec() override {for(auto entry:stack)entry->OnSnapshotRestored();auto i=stack.begin();SetInterface(*i++);for(;i!=stack.end();++i)PushInterface(*i);}};
        CObj<RestoreStack> load=new RestoreStack;load->stack=restored;load->Exec();
        owner=restoredMap;game=restoredGame;underlying=context==1?dynamic_cast<NGame::CMissionBase*>(restored.front().GetPtr()):nullptr;
        world=dynamic_cast<NWorld::CWorld*>(owner->GetWorld());
        Require(owner->GetInterface()->GetChildByID("globalmapUI") && owner->IsGlobalMapShowMode()==(context==1) && owner->GetSoundScene()==owner->GetInterface()->GetSound(),"restored map lost 2D UI/show-mode/shared-audio identity");
        if(context==1)Require(underlying && underlying->GetRPGGame()==game.GetPtr(),"overview snapshot split the chapter/global-map campaign alias");
        std::printf("native global-map script context=%d mode=%d stage=snapshot legacy=%d world=%d bytes=%u\n",context,mode,int(mode==5),int(world!=nullptr),unsigned(snapshot.GetSize()));std::fflush(stdout);
    }
    if(mode==0){for(int i=0;i<3;++i)frame();Require(!number("GlobalMapRuns",1) && !NMainLoop::HaveInterfaceCommand(),"null global-map script unexpectedly ran a callback");}
    else {
        Require(world && world->GetOwnScript() && owner->GetActivePlayer() && owner->GetRenderGame() && owner->GetScene(),"native global map has no logical world/player/renderer to execute its map script");
        Require(world->GetGlobalGame()==game.GetPtr() && owner->GetActivePlayer()->GetGlobalPlayer()==game->players.front().GetPtr(),"global-map logical context lost actual campaign/player identity");
        if(mode==4){
            for(int i=0;i<32 && !NMainLoop::HaveInterfaceCommand();++i)frame();
            Require(number("GlobalMapRuns",1) && NMainLoop::HaveInterfaceCommand() && game->GetGlobalVar("gmapgfirstenter").empty(),"first country script did not queue its authored hint before setting its visit flag");
            frame(false);auto* modal=NMainLoop::GetCurrentInterfaceForHarness();Require(modal && pSSClasses->GetObjectTypeID(modal)==int(0x1423130) && NMainLoop::GetInterfaceStackDepth()==context+2,"global first-visit script did not open its actual hint modal");
            CStructureNetworkContext refs;auto saved=BorrowedObjectFields(modal,&refs);CPtr<NGame::IMission> origin;int event=-1;
            {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(2,&origin);reader.Add(3,&event);}
            Require(origin.GetPtr()==owner.GetPtr() && event>=0 && world->GetOwnScript()->IsUIActionIDPresent(event),"global first-visit hint lost actual map origin/native Lua wait");
            const auto held=world->GetAimTime()->GetValue();frame(false);Require(world->GetAimTime()->GetValue()==held && game->GetGlobalVar("gmapgfirstenter").empty(),"covered global-map world advanced or marked a still-waiting hint visited");
            const float awarded=game->players.front()->mercs.front()->fXP;Require(authoredHint->nSequenceID>0?awarded>initialXP:awarded==initialXP,"global hint changed its actual reward/no-reward contract");
            NInput::SEvent input;while(NInput::GetEvent(&input)){}NInput::PostEvent("cancel");Require(NInput::GetEvent(&input) && modal->ProcessEvent(input),"global first-visit hint did not accept native cancel");frame();
            for(int i=0;i<32 && game->GetGlobalVar("gmapgfirstenter")!="1";++i)frame();
            Require(!world->GetOwnScript()->IsUIActionIDPresent(event) && game->GetGlobalVar("gmapgfirstenter")=="1" && game->players.front()->mercs.front()->fXP==awarded,"native hint close did not release its Lua wait/visit flag or awarded XP twice");
            std::printf("native global-map script context=%d mode=4 stage=hint closed=1 wait=0 flag=1 xpBefore=%.3f xpAfter=%.3f reward=%d\n",context,initialXP,awarded,authoredHint->nSequenceID);
        }else{
            for(int i=0;i<32 && (!number("GlobalMapRuns",1) || ((mode==1 || mode>=5) && !number("GlobalMapWake",1)) || (mode==2 && game->GetGlobalVar("gmapgfirstenter")!="1"));++i)frame();
            Require(number("GlobalMapRuns",1),"native global-map postinit never executed its registered map script");
            if(mode==1 || mode>=5)Require(number("GlobalMapEnters",1) && number("GlobalMapWake",1),"native global-map world did not enter its script once and resume Sleep");
        }
        if(mode>=2 && mode<=4){Require(game->GetGlobalVar("gmapgfirstenter")=="1","actual first-country script never latched its persistent visited flag");
            Require(game->hintsSet.size()==(mode==3?0u:1u) && (mode==3 || game->hintsSet.front().GetPtr()==authoredHint),"global map lost authored hint one-shot registration or replayed an already-visited script");}
        for(int i=0;i<3;++i)frame();Require(number("GlobalMapRuns",1) && ((mode!=1 && mode<5) || number("GlobalMapEnters",1)),"global-map frame pump reran map postinit or enter callback");
    }
    Require(game->bChapterMapSet==chapterFlag && (mode==4 || game->players.front()->mercs.front()->fXP==initialXP) && NMainLoop::GetInterfaceStackDepth()==(context==1?2:1) && !NMainLoop::HaveInterfaceCommand(),"global-map lifecycle changed return destination, XP or native stack");
    std::printf("native global-map script context=%d mode=%d stage=complete runs=%d enter=%d wake=%d return=1 xp=1 stack=%d flag=%s hints=%zu\n",context,mode,int(number("GlobalMapRuns",1)),int(number("GlobalMapEnters",1)),int(number("GlobalMapWake",1)),NMainLoop::GetInterfaceStackDepth(),game->GetGlobalVar("gmapgfirstenter").c_str(),game->hintsSet.size());std::fflush(stdout);
}

static void WorldAIFlash(const char* databasePath,const char* resourcePath,int context,int mode)
{
    Require(context>=0 && context<=1 && mode>=0 && mode<=4,"unknown shared-world AI flash case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"world AI flash fixture started with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm shared world AI flash regression");
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} frameClock{NMainLoop::SetTimeForHarness(1000)};
    CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    struct RestoreScripts {vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> variants;CPtr<NDb::CChapterMap> chapter;CPtr<NDb::CScript> entry;
        ~RestoreScripts(){for(auto& p:variants)if(IsValid(p.first))p.first->pScript=p.second;if(IsValid(chapter))chapter->pScript=entry;}} restore;
    struct SectorRestore {CObj<CChapterInfo> info;vector<SChapterSector> original;
        ~SectorRestore(){if(IsValid(info))info->sectorsSet.swap(original);}} sectors;
    CObj<NRPG::CGlobalGame> game;
    if(context==0){
        CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());while(maps.MoveNext())if(IsValid(maps.Get()->pBackground)){restore.chapter=maps.Get();break;}
        Require(IsValid(restore.chapter),"world AI flash fixture has no authored chapter");restore.entry=restore.chapter->pScript;restore.chapter->pScript=quiet.GetPtr();
        game=NRPG::CreateGlobalGame(1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        list<CPtr<NScenario::CScenarioClue>> clues;game->pScenarioTracker->GetAvailableClues(&clues);for(auto clue:clues)if(IsValid(clue))clue->SetJustFound(false);
        for(auto merc:game->players.front()->mercs){vector<CDBPtr<NDb::CMedal>> medals;merc->GetJustFoundMedals(&medals);}
        CDGPtr<CPtrFuncBase<CChapterInfo>> info=shareChapterInfo.Get(restore.chapter->GetRecordID());info.Refresh();
        sectors.info=info->GetValue();Require(IsValid(sectors.info),"clue fixture has no cached native map info");
        sectors.original=sectors.info->sectorsSet;auto& isolated=sectors.info->sectorsSet;
        isolated.erase(std::remove_if(isolated.begin(),isolated.end(),[](const SChapterSector& sector){return sector.eType==RANDOM;}),isolated.end());
        Require(!isolated.empty(),"AI flash isolation removed every authored zone and exit");
        CObj<NGame::CICBeginChapter> enter=new NGame::CICBeginChapter(restore.chapter->GetRecordID(),game);enter->Exec();
    }else{
        CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
        while(variants.MoveNext())if(IsValid(variants.Get()->pTemplate) && variants.Get()->pTemplate->GetRecordID()==2425){restore.variants.push_back({variants.Get(),variants.Get()->pScript});variants.Get()->pScript=quiet.GetPtr();}
        Require(!restore.variants.empty(),"world AI flash fixture has no authored menu variants");CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    }
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* world=owner?dynamic_cast<NWorld::CWorld*>(owner->GetWorld()):nullptr;auto* view=owner?owner->GetScene():nullptr;auto* renderer=owner?owner->GetRenderGame():nullptr;
    Require(world && view && renderer && world->GetOwnScript(),"world AI flash fixture lacks actual chapter/menu world, view, renderer or Lua");
    Require(context==0?owner->GetDesktop()!=nullptr:owner->GetDesktop()==nullptr,"world AI flash fixture lost its real chapter/null-menu desktop distinction");
    // Materialize authored static world effects before taking geometry baselines.
    NMainLoop::SetTimeForHarness(1050);Require(NMainLoop::StepApp(true,false,false),"world AI flash warm-up frame exited");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"world AI flash warm-up opened an unrelated interface");
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}




    Require(context==0?owner->GetActivePlayer()!=nullptr:owner->GetActivePlayer()==nullptr,"world AI flash fixture lost native active/null-player distinction");
    if(context==1){
        Require(mode==0 || mode==4,"native unit-less menu supports quiet/null-unit command controls");
        vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);Require(units.empty(),"quiet native backdrop unexpectedly contains combat units");
        if(mode==4)world->AddUICommand(new NWorld::CUICmdAIUnitWillMove(nullptr));
        NMainLoop::SetTimeForHarness(1100);Require(NMainLoop::StepApp(true,false,false) && NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr(),"native menu AI control lost its actual owner");
        struct Flash {CPtr<CObjectBase> node;STime end=0;int operator&(CStructureSaver& f){f.Add(2,&node);f.Add(3,&end);return 0;}};
        struct Units {list<Flash> flashes;int operator&(CStructureSaver& f){f.Add(9,&flashes);return 0;}} value;
        CStructureNetworkContext refs;auto saved=BorrowedObjectFields(renderer,&refs);{CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(7,&value);}
        Require(value.flashes.empty() && !world->GetUICommand() && !NMainLoop::HaveInterfaceCommand(),"unit-less/null-active-player menu flashed or retained a native command");
        std::printf("native world AI flash context=1 mode=%d stage=complete units=0 activePlayer=0 flashes=0 queue=0\n",mode);return;
    }
    struct OptionRestore {NGlobal::CValue value;~OptionRestore(){NGlobal::SetVar("game_selectionmode",value);}} option{NGlobal::GetVar("game_selectionmode",0)};
    NGlobal::SetVar("game_selectionmode",1);owner->SetCheatVisibility(true);owner->PauseGame(true);
    vector<CPtr<NWorld::CPlayer>> players;world->GetPlayersList(&players);Require(!players.empty(),"world AI flash fixture lacks its real observer player");
    auto* observer=context==0?dynamic_cast<NWorld::CPlayer*>(owner->GetActivePlayer()->GetPlayer()):players.front().GetPtr();
    Require(observer && observer->GetScenarioPlayerID()==0,"world AI flash fixture lacks its native scenario-zero observer");
    auto* scriptGame=world->GetOwnScript()->GetGlobalGame();Require(scriptGame,"world AI flash fixture lacks actual world global game");
    CObj<NRPG::CGlobalPlayer> emptyParty=NRPG::CreateGlobalPlayer();
    auto* party=dynamic_cast<NWorld::CPlayer*>(world->AddPlayer(L"Native flash target",emptyParty,new NWorld::CCommander));
    Require(party && party!=observer,"world AI flash fixture failed to register its real separate party");party->SetCommander(new NAI::CAICommander(world,party));
    // Native worlds also use held neutral (-1) owners outside the turn ring
    // (e.g. deployed bodies); a unit itself must still have a valid owner.
    CObj<NWorld::CPlayer> neutral=new NWorld::CPlayer(L"Native neutral flash owner",scriptGame,nullptr,-1);neutral->SetCommander(new NWorld::CCommander);
    NWorld::CPlayer* unitOwner=mode==2?neutral.GetPtr():mode==3?observer:party;
    const NDb::EDiplomacyState relation=mode==2?NDb::DS_NEUTRAL:mode==3?NDb::DS_ALLY:NDb::DS_ENEMY;
    Require(world->GetDiplomacyState(observer,unitOwner)==relation,"world AI flash fixture lost its actual native diplomacy");
    auto* network=world->GetPathNetwork();Require(network,"world AI flash fixture has no native navigation network");
    vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(CVec3(3,3,0),50.f),&places);NAI::SPathPlace position;bool found=false;
    for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetMoving(0);if(network->IsPassable(p) && !network->IsLocked(p)){position=p;found=true;break;}}
    Require(found,"world AI flash fixture found no real free native standing tile");
    NAI::SUnitPosition placement;placement.bRun=false;placement.pos.SetNetwork(network);placement.pos.p=position;
    CObj<NRPG::IUnitMission> rpg=NRPG::CreateUnit(NDb::GetPers(54));Require(rpg && rpg->GetModel(),"world AI flash fixture has no authored body");
    CObj<NWorld::CUnitServer> target=new NWorld::CUnitServer(world,rpg,rpg->GetModel(),unitOwner,placement);world->AddUnit(target);
    Require(target && target->CanFight() && (unitOwner!=party || target->IsAIUnit()),"world AI flash fixture failed to create a real living native AI unit");
    // Match the existing tactical flash fixture: hold simulation ahead of the
    // short renderer-clock interval, so no unrelated AI commands can run.
    world->GetTime()->Set(1000000);world->GetAimTime()->Set(1000000);
    auto frame=[&](STime tick){NMainLoop::SetTimeForHarness(tick);Require(NMainLoop::StepApp(true,false,false) && NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr(),"world AI flash fixture lost its actual owner frame");};
    for(int step=0;step<4;++step)frame(1100+step*50);
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}
    auto parts=[&](CObjectBase* object){CStructureNetworkContext refs;auto saved=BorrowedObjectFields(object,&refs);vector<CPtr<CObjectBase>> result;
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(1,&result);}return result;};
    auto targetOf=[&](CObjectBase* binder){CStructureNetworkContext refs;auto saved=BorrowedObjectFields(binder,&refs);CPtr<CObjectBase> result;
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(2,&result);}return result;};
    CObj<CObjectBase> selected=renderer->Select(target,CVec4(.25f,0,0,1));Require(selected,"native renderer did not accept its real visible AI unit selection");
    CPtr<CObjectBase> selectedNode;{CStructureNetworkContext refs;auto saved=BorrowedObjectFields(selected,&refs);CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(3,&selectedNode);}
    Require(selectedNode,"native selection has no actual scene node");auto selectedParts=parts(selectedNode);Require(!selectedParts.empty(),"native visible-model selection has no bound mesh parts");
    vector<CPtr<CObjectBase>> geometry;for(auto binder:selectedParts){auto actual=targetOf(binder);Require(actual,"native selection has a null bound geometry target");geometry.push_back(actual);}
    struct Flash {CPtr<CObjectBase> node;STime end=0;int operator&(CStructureSaver& f){f.Add(2,&node);f.Add(3,&end);return 0;}};
    struct Units {list<Flash> flashes;int operator&(CStructureSaver& f){f.Add(9,&flashes);return 0;}};
    auto flashes=[&](){CStructureNetworkContext refs;auto saved=BorrowedObjectFields(renderer,&refs);Units value;
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(7,&value);}return value.flashes;};
    Require(flashes().empty(),"quiet world AI flash fixture already had a transient filter");
    const auto before=target->GetPosition();const int ap=target->GetAP(),hp=target->GetUnitRPG()->GetTotalVP();
    const CVec4 wanted=mode==0?CVec4(.11f,.12f,.13f,0):relation==NDb::DS_ENEMY?CVec4(78.f/255,20.f/255,0,0):CVec4(0,77.f/255,78.f/255,0);
    if(mode==0)renderer->FlashUnit(target,wanted); // Independent real-renderer observability control.
    else {world->AddUICommand(new NWorld::CUICmdAIUnitWillMove(mode==4?nullptr:target.GetPtr()));frame(1350);}
    const bool show=mode==0 || (context==0 && mode!=4);auto live=flashes();
    std::printf("native world AI flash context=%d mode=%d stage=dispatch relation=%d geometry=%zu flashes=%zu expected=%d\n",context,mode,int(relation),geometry.size(),live.size(),int(show));std::fflush(stdout);
    Require(live.size()==(show?1u:0u),"chapter lost its AI-unit flash command or a null-menu/unit control flashed");
    if(show){Require(live.front().node,"native AI flash has no scene filter");auto filters=parts(live.front().node);Require(filters.size()==geometry.size(),"AI flash did not bind all actual selected model geometry");
        STime start=0;CVec4 color;CPtr<CFuncBase<STime>> clock;
        for(auto binder:filters){Require(std::find(geometry.begin(),geometry.end(),targetOf(binder))!=geometry.end(),"AI flash targeted a different unit's geometry");
            CPtr<CObjectBase> post;{CStructureNetworkContext refs;auto saved=BorrowedObjectFields(binder,&refs);CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(3,&post);}Require(post,"AI flash binder lacks a real postprocessor");
            CPtr<CObjectBase> function;{CStructureNetworkContext refs;auto saved=BorrowedObjectFields(post,&refs);CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(2,&function);}Require(function,"AI flash postprocessor lacks a real color function");
            {CStructureNetworkContext refs;auto saved=BorrowedObjectFields(function,&refs);CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(2,&clock);reader.Add(3,&start);reader.Add(4,&color);}
            Require(clock.GetPtr()==renderer->GetTime() && live.front().end==start+1500 && std::abs(color.x-wanted.x)<1e-6f && std::abs(color.y-wanted.y)<1e-6f && std::abs(color.z-wanted.z)<1e-6f && color.w==wanted.w,"AI flash lost its native diplomacy color, renderer clock or1500ms lifetime");}
        std::printf("native world AI flash context=%d mode=%d stage=filter parts=%zu color=%.6f,%.6f,%.6f,%.6f duration=%u clock=1\n",context,mode,filters.size(),color.x,color.y,color.z,color.w,unsigned(live.front().end-start));}
    CObj<CObjectBase> retained=renderer->Select(target,CVec4(.25f,0,0,1));
    Require(retained.GetPtr()==selected.GetPtr(),"AI flash replaced the ordinary native unit selection");
    Require(target->GetPosition()==before && target->GetAP()==ap && target->GetUnitRPG()->GetTotalVP()==hp && !world->GetUICommand() && !NMainLoop::HaveInterfaceCommand(),"world AI flash changed combat state/position or left a queued command");
    std::printf("native world AI flash context=%d mode=%d stage=complete selection=1 state=1 queue=0\n",context,mode);std::fflush(stdout);
}

static void WorldScriptClue(const char* databasePath,const char* resourcePath,int context,int mode)
{
    Require(context>=0 && context<=1 && mode>=0 && mode<=2,"unknown shared-world clue case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"world clue fixture started with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm shared world clue regression");
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} frameClock{NMainLoop::SetTimeForHarness(1000)};
    CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    struct RestoreScripts {vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> variants;CPtr<NDb::CChapterMap> chapter;CPtr<NDb::CScript> entry;
        ~RestoreScripts(){for(auto& p:variants)if(IsValid(p.first))p.first->pScript=p.second;if(IsValid(chapter))chapter->pScript=entry;}} restore;
    struct SectorRestore {CObj<CChapterInfo> info;vector<SChapterSector> original;
        ~SectorRestore(){if(IsValid(info))info->sectorsSet.swap(original);}} sectors;
    CObj<NRPG::CGlobalGame> game;
    if(context==0){
        CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());while(maps.MoveNext())if(IsValid(maps.Get()->pBackground)){restore.chapter=maps.Get();break;}
        Require(IsValid(restore.chapter),"world clue fixture has no authored chapter");restore.entry=restore.chapter->pScript;restore.chapter->pScript=quiet.GetPtr();
        game=NRPG::CreateGlobalGame(1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        list<CPtr<NScenario::CScenarioClue>> clues;game->pScenarioTracker->GetAvailableClues(&clues);for(auto clue:clues)if(IsValid(clue))clue->SetJustFound(false);
        for(auto merc:game->players.front()->mercs){vector<CDBPtr<NDb::CMedal>> medals;merc->GetJustFoundMedals(&medals);}
        CDGPtr<CPtrFuncBase<CChapterInfo>> info=shareChapterInfo.Get(restore.chapter->GetRecordID());info.Refresh();
        sectors.info=info->GetValue();Require(IsValid(sectors.info),"clue fixture has no cached native map info");
        sectors.original=sectors.info->sectorsSet;auto& isolated=sectors.info->sectorsSet;
        isolated.erase(std::remove_if(isolated.begin(),isolated.end(),[](const SChapterSector& sector){return sector.eType==RANDOM;}),isolated.end());
        Require(!isolated.empty(),"clue isolation removed every authored zone and exit");
        CObj<NGame::CICBeginChapter> enter=new NGame::CICBeginChapter(restore.chapter->GetRecordID(),game);enter->Exec();
    }else{
        CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
        while(variants.MoveNext())if(IsValid(variants.Get()->pTemplate) && variants.Get()->pTemplate->GetRecordID()==2425){restore.variants.push_back({variants.Get(),variants.Get()->pScript});variants.Get()->pScript=quiet.GetPtr();}
        Require(!restore.variants.empty(),"world clue fixture has no authored menu variants");CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    }
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* world=owner?dynamic_cast<NWorld::CWorld*>(owner->GetWorld()):nullptr;auto* view=owner?owner->GetScene():nullptr;auto* renderer=owner?owner->GetRenderGame():nullptr;
    Require(world && view && renderer && world->GetOwnScript(),"world clue fixture lacks actual chapter/menu world, view, renderer or Lua");
    Require(context==0?owner->GetDesktop()!=nullptr:owner->GetDesktop()==nullptr,"world clue fixture lost its real chapter/null-menu desktop distinction");
    // Materialize authored static world effects before taking geometry baselines.
    NMainLoop::SetTimeForHarness(1050);Require(NMainLoop::StepApp(true,false,false),"world clue warm-up frame exited");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"world clue warm-up opened an unrelated interface");
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}



    auto* script=world->GetOwnScript();auto* scriptGame=script->GetGlobalGame();auto* tracker=script->GetScenarioTracker();
    Require(scriptGame && tracker && tracker==scriptGame->pScenarioTracker.GetPtr(),"world clue fixture lost its actual script game/tracker identity");
    Require(context==0?owner->GetRPGGame()==scriptGame:owner->GetRPGGame()==nullptr,"world clue fixture lost its chapter/campaignless-menu distinction");
    Require(tracker->IsScenarioAvailable()==(context==0),"world clue fixture lost its actual scenario/no-scenario distinction");
    auto* clue=tracker->GetClueByName("Find_Cipher_key");
    NDb::CString* description=nullptr;
    if(context==0){Require(clue,"chapter clue fixture lacks its authored cipher-key clue");tracker->CheatTakeClue(clue,true);
        description=tracker->GetClueDescriptionFromObjective(clue);Require(description && !NUI::GetDBString(description).empty(),"chapter clue fixture did not finish its real capture objective");
        list<CPtr<NScenario::CScenarioClue>> available;tracker->GetAvailableClues(&available);for(auto current:available)if(IsValid(current))current->SetJustFound(false);
    }else Require(!clue,"native scenario-less menu unexpectedly exposed a campaign clue");
    const bool found=clue && tracker->IsClueFound(clue);const bool justFound=clue && clue->IsJustFound();
    auto xp=[](NRPG::CGlobalGame* game){float value=0;for(auto player:game->players)for(auto merc:player->mercs)if(IsValid(merc))value+=merc->fXP;return value;};
    const float initialXP=xp(scriptGame);
    auto number=[&](const char* name,int expected){const int top=script->GetTop();auto value=script->GetGlobal(name);const bool result=value.IsNumber() && value.GetNumber()==expected;script->SetTop(top);return result;};
    auto nil=[&](const char* name){const int top=script->GetTop();const bool result=script->GetGlobal(name).IsNil();script->SetTop(top);return result;};
    auto frame=[&](STime tick){if(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand())world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        NMainLoop::SetTimeForHarness(tick);Require(NMainLoop::StepApp(true,false,false),"world clue native frame exited");};
    const string lua=mode==0?"WorldClueProduced = 1":string("WorldClueResult = ClueShow('")+(mode==1?"__absent_world_clue__":"Find_Cipher_key")+"'); WorldClueProduced = 1";
    Require(script->DoString(lua.c_str())==0,"world clue fixture could not execute actual Lua ClueShow");world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
    Require(number("WorldClueProduced",1) && nil("WorldClueResult"),"native ClueShow did not preserve its no-return-value Lua API");
    frame(1100);const bool show=context==0 && mode==2;
    std::printf("native world clue context=%d mode=%d stage=dispatch scenario=%d queued=%d expected=%d found=%d xp=%.3f\n",context,mode,int(tracker->IsScenarioAvailable()),int(NMainLoop::HaveInterfaceCommand()),int(show),int(found),xp(scriptGame));std::fflush(stdout);
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && NMainLoop::HaveInterfaceCommand()==show,"chapter discarded its real Lua ClueShow command or a control opened a modal");
    if(show){frame(1150);auto* modal=NMainLoop::GetCurrentInterfaceForHarness();
        Require(modal && pSSClasses->GetObjectTypeID(modal)==int(0xb1001180) && NMainLoop::GetInterfaceStackDepth()==2,"world clue did not open the actual native clue modal");
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(modal,&refs);int event=-2;CPtr<NGame::IMission> origin;CPtr<NRPG::CGlobalGame> modalGame;CPtr<NScenario::CScenarioClue> modalClue;CPtr<NUI::CWindow> ui;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&event);reader.Add(3,&origin);reader.Add(4,&modalGame);reader.Add(6,&modalClue);reader.Add(9,&ui);}
        Require(event>=0 && !script->IsUIActionIDPresent(event) && origin.GetPtr()==owner.GetPtr() && modalGame.GetPtr()==scriptGame && modalClue.GetPtr()==clue && ui,"native clue modal lost its unregistered native event, origin, game or selected clue");
        CPtr<NUI::CWindow> viewWindow;{auto saved=BorrowedObjectFields(ui,&refs);CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(5,&viewWindow);}
        Require(viewWindow,"native clue UI did not construct its real description view");
        CPtr<NUI::CText> text;{auto saved=BorrowedObjectFields(viewWindow,&refs);CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(5,&text);}
        Require(text,"native clue description has no actual text control");
        wstring expected;const auto authored=NUI::GetDBString(description);for(size_t i=0;i<authored.size();++i){const auto c=authored[i];if(c==L'\r' || c==L'\n'){expected+=L"<br>";if(c==L'\r' && i+1<authored.size() && authored[i+1]==L'\n')++i;}else if(c==133)expected+=L"...";else expected+=c;}
        Require(text->GetText()==NUI::GetDBString(0x4f22)+expected,"native clue modal displayed a different objective description or lost its existing markup");
        NUI::SPoint size;text->GetRealSize(&size);Require(size.x>0 && size.y>0,"native clue description has empty text layout");
        const STime held=world->GetAimTime()->GetValue();frame(1200);Require(world->GetAimTime()->GetValue()==held,"covered clue owner advanced its world");
        NInput::SEvent input;while(NInput::GetEvent(&input)){}NInput::PostEvent("cancel");Require(NInput::GetEvent(&input) && modal->ProcessEvent(input),"native clue modal rejected its close event");frame(1250);
        Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && NMainLoop::GetInterfaceStackDepth()==1,"native clue close did not restore its actual chapter owner");
        std::printf("native world clue context=0 mode=2 stage=modal event=%d noWait=1 origin=1 clue=1 game=1 text=1 owner=1 stack=1\n",event);
    }
    frame(1300);frame(1350);
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && NMainLoop::GetInterfaceStackDepth()==1 && !NMainLoop::HaveInterfaceCommand() && !world->GetUICommand(),"world clue left an unrelated modal or pending command");
    Require(xp(scriptGame)==initialXP && (!clue || (tracker->IsClueFound(clue)==found && clue->IsJustFound()==justFound)),"displaying a clue changed its scenario state or experience");
    std::printf("native world clue context=%d mode=%d stage=complete modal=%d apiNil=1 state=1 xp=1\n",context,mode,int(show));std::fflush(stdout);
}

static void WorldScriptVideo(const char* databasePath,const char* resourcePath,int context,int mode)
{
    Require(context>=0 && context<=1 && mode>=0 && mode<=4,"unknown shared-world video sequence case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"world video sequence fixture started with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm shared world video sequence regression");
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} frameClock{NMainLoop::SetTimeForHarness(1000)};
    CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    struct RestoreScripts {vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> variants;CPtr<NDb::CChapterMap> chapter;CPtr<NDb::CScript> entry;
        ~RestoreScripts(){for(auto& p:variants)if(IsValid(p.first))p.first->pScript=p.second;if(IsValid(chapter))chapter->pScript=entry;}} restore;
    struct SectorRestore {CObj<CChapterInfo> info;vector<SChapterSector> original;
        ~SectorRestore(){if(IsValid(info))info->sectorsSet.swap(original);}} sectors;
    CObj<NRPG::CGlobalGame> game;
    if(context==0){
        CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());while(maps.MoveNext())if(IsValid(maps.Get()->pBackground)){restore.chapter=maps.Get();break;}
        Require(IsValid(restore.chapter),"world video sequence fixture has no authored chapter");restore.entry=restore.chapter->pScript;restore.chapter->pScript=quiet.GetPtr();
        game=NRPG::CreateGlobalGame(1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        list<CPtr<NScenario::CScenarioClue>> clues;game->pScenarioTracker->GetAvailableClues(&clues);for(auto clue:clues)if(IsValid(clue))clue->SetJustFound(false);
        for(auto merc:game->players.front()->mercs){vector<CDBPtr<NDb::CMedal>> medals;merc->GetJustFoundMedals(&medals);}
        CDGPtr<CPtrFuncBase<CChapterInfo>> info=shareChapterInfo.Get(restore.chapter->GetRecordID());info.Refresh();
        sectors.info=info->GetValue();Require(IsValid(sectors.info),"video sequence fixture has no cached native map info");
        sectors.original=sectors.info->sectorsSet;auto& isolated=sectors.info->sectorsSet;
        isolated.erase(std::remove_if(isolated.begin(),isolated.end(),[](const SChapterSector& sector){return sector.eType==RANDOM;}),isolated.end());
        Require(!isolated.empty(),"video isolation removed every authored zone and exit");
        CObj<NGame::CICBeginChapter> enter=new NGame::CICBeginChapter(restore.chapter->GetRecordID(),game);enter->Exec();
    }else{
        CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
        while(variants.MoveNext())if(IsValid(variants.Get()->pTemplate) && variants.Get()->pTemplate->GetRecordID()==2425){restore.variants.push_back({variants.Get(),variants.Get()->pScript});variants.Get()->pScript=quiet.GetPtr();}
        Require(!restore.variants.empty(),"world video sequence fixture has no authored menu variants");CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    }
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* world=owner?dynamic_cast<NWorld::CWorld*>(owner->GetWorld()):nullptr;auto* view=owner?owner->GetScene():nullptr;auto* renderer=owner?owner->GetRenderGame():nullptr;
    Require(world && view && renderer && world->GetOwnScript(),"world video sequence fixture lacks actual chapter/menu world, view, renderer or Lua");
    Require(context==0?owner->GetDesktop()!=nullptr:owner->GetDesktop()==nullptr,"world video sequence fixture lost its real chapter/null-menu desktop distinction");
    // Materialize authored static world effects before taking geometry baselines.
    NMainLoop::SetTimeForHarness(1050);Require(NMainLoop::StepApp(true,false,false),"world video sequence warm-up frame exited");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"world video sequence warm-up opened an unrelated interface");
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}


    Require(std::getenv("S2_USER_DATA_DIR"),"world video fixture requires isolated user data");
    Require(context==0?owner->GetRPGGame()!=nullptr:owner->GetRPGGame()==nullptr,"world video fixture lost its campaign/null-menu distinction");
    struct SequenceFile {
        std::filesystem::path path;
        ~SequenceFile(){std::error_code error;std::filesystem::remove(path,error);}
    } sequence{std::filesystem::path(std::getenv("S2_USER_DATA_DIR"))/"native world video.seq"};
    std::filesystem::create_directories(sequence.path.parent_path());
    {std::ofstream file(sequence.path,std::ios::binary|std::ios::trunc);
        file<<"exec setvar native_world_video_first = 10\nexec setvar native_world_video_stage = 10\nexec setvar native_world_video_stage = 20\n";
        if(mode>=3)file<<"exec mainmenu\n";
        Require(file.good(),"world video fixture could not write its temporary native sequence");}
    struct StageRestore {NGlobal::CValue previous,first;~StageRestore(){NGlobal::SetVar("native_world_video_stage",previous);NGlobal::SetVar("native_world_video_first",first);}} stage{NGlobal::GetVar("native_world_video_stage",0),NGlobal::GetVar("native_world_video_first",0)};
    NGlobal::SetVar("native_world_video_stage",0);NGlobal::SetVar("native_world_video_first",0);
    auto* script=world->GetOwnScript();
    auto number=[&](const char* name,int expected){const int top=script->GetTop();auto value=script->GetGlobal(name);const bool matches=value.IsNumber() && value.GetNumber()==expected;script->SetTop(top);return matches;};
    auto absent=[&](const char* name){const int top=script->GetTop();const bool result=script->GetGlobal(name).IsNil();script->SetTop(top);return result;};
    auto frame=[&](STime tick){if(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand())world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        NMainLoop::SetTimeForHarness(tick);Require(NMainLoop::StepApp(true,false,false),"world video native frame exited");};
    if(mode==0){for(int step=0;step<3;++step)frame(1100+step*50);
        Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand() && NMainLoop::GetInterfaceStackDepth()==1 && NGlobal::GetVar("native_world_video_stage").GetFloat()==0 && NGlobal::GetVar("native_world_video_first").GetFloat()==0 && absent("WorldVideoID"),
            "quiet native video control created an interface, wait or sequence side effect");
        std::printf("native world video context=%d mode=0 stage=control stack=1 marker=0\n",context);return;}
    const string lua="WorldVideoID = PlayVideo('"+sequence.path.generic_string()+"'); WorldVideoProduced = 1; while IsUIActionIDPresent(WorldVideoID) do Sleep(1) end; WorldVideoResumed = 1";
    Require(script->DoString(lua.c_str())==0,"world video fixture could not start its real Lua producer");
    world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
    Require(number("WorldVideoProduced",1),"native Lua PlayVideo producer did not run");
    const int top=script->GetTop();auto value=script->GetGlobal("WorldVideoID");Require(value.IsNumber(),"native PlayVideo returned no wait ID");const int eventID=int(value.GetNumber());script->SetTop(top);
    Require(script->IsUIActionIDPresent(eventID) && absent("WorldVideoResumed"),"native video wait completed before its consumer");
    frame(1100);
    std::printf("native world video context=%d mode=%d stage=dispatch queued=%d wait=%d marker=%.0f\n",context,mode,int(NMainLoop::HaveInterfaceCommand()),int(script->IsUIActionIDPresent(eventID)),NGlobal::GetVar("native_world_video_stage").GetFloat());std::fflush(stdout);
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && NMainLoop::HaveInterfaceCommand(),"non-tactical world discarded its PlayVideo command");
    frame(1150);auto* modal=NMainLoop::GetCurrentInterfaceForHarness();
    Require(modal && pSSClasses->GetObjectTypeID(modal)==int(0xb3320180) && NMainLoop::GetInterfaceStackDepth()==2,"world video did not push the actual native sequence interface");
    // Initialize runs line0, then StepApp immediately Steps the new interface (line1).
    Require(NGlobal::GetVar("native_world_video_first").GetFloat()==10 && NGlobal::GetVar("native_world_video_stage").GetFloat()==10 && script->IsUIActionIDPresent(eventID) && absent("WorldVideoResumed"),"native sequence did not run its first line while retaining its Lua wait");
    const STime coveredTime=world->GetAimTime()->GetValue();
    if(mode%2==0){NInput::SEvent input;while(NInput::GetEvent(&input)){}NInput::PostEvent("cancel");
        Require(NInput::GetEvent(&input) && modal->ProcessEvent(input),"native sequence did not accept its cancel input");
        Require(NGlobal::GetVar("native_world_video_stage").GetFloat()==20,"native sequence cancellation lost its remaining exec command");}
    else {frame(1200);Require(NGlobal::GetVar("native_world_video_stage").GetFloat()==20,"native sequence Step did not run its second line");}
    Require(world->GetAimTime()->GetValue()==coveredTime && script->IsUIActionIDPresent(eventID) && absent("WorldVideoResumed"),"covered sequence advanced its owner world or released its Lua wait early");

    frame(1250);
    Require(NMainLoop::GetCurrentInterfaceForHarness()==modal && NMainLoop::HaveInterfaceCommand(),"native sequence did not queue its original terminal action");
    Require(world->GetAimTime()->GetValue()==coveredTime && NGlobal::GetVar("native_world_video_stage").GetFloat()==20 && NGlobal::GetVar("native_world_video_first").GetFloat()==10,"sequence terminal action lost its exec effects or advanced its covered owner");
    if(mode>=3){
        frame(1300);auto* next=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
        Require(next && next!=owner.GetPtr() && next->GetWorld() && next->GetDesktop()==nullptr && next->GetActivePlayer()==nullptr && next->GetRPGGame()==nullptr && NMainLoop::GetInterfaceStackDepth()==1,
            "authored sequence mainmenu command did not install the real fresh menu");
        Require(script->IsUIActionIDPresent(eventID) && absent("WorldVideoResumed") && world->GetAimTime()->GetValue()==coveredTime,
            "explicit sequence transition unexpectedly resumed its abandoned Lua world");
        std::printf("native world video context=%d mode=%d stage=complete terminal=mainmenu stack=1 marker=20 first=10 wait=%d\n",context,mode,int(script->IsUIActionIDPresent(eventID)));
    }else{
        // Original EOF queues Command(0): an application-exit sentinel, not modal pop.
        NMainLoop::SetTimeForHarness(1300);Require(!NMainLoop::StepApp(true,false,false),"native EOF did not preserve its original application-exit sentinel");
        Require(NMainLoop::GetCurrentInterfaceForHarness()==modal && NMainLoop::GetInterfaceStackDepth()==2 && !NMainLoop::HaveInterfaceCommand(),"EOF quit sentinel changed the interface stack or remained queued");
        std::printf("native world video context=%d mode=%d stage=complete terminal=quit stack=2 marker=20 first=10 wait=%d\n",context,mode,int(script->IsUIActionIDPresent(eventID)));
    }
    std::fflush(stdout);
}

static void WorldScriptCameraLocator(const char* databasePath,const char* resourcePath,int context,int mode)
{
    Require(context>=0 && context<=1 && mode>=0 && mode<=7,"unknown shared-world camera locator case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"world camera locator fixture started with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm shared world camera locator regression");
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} frameClock{NMainLoop::SetTimeForHarness(1000)};
    CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    struct RestoreScripts {vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> variants;CPtr<NDb::CChapterMap> chapter;CPtr<NDb::CScript> entry;
        ~RestoreScripts(){for(auto& p:variants)if(IsValid(p.first))p.first->pScript=p.second;if(IsValid(chapter))chapter->pScript=entry;}} restore;
    struct SectorRestore {CObj<CChapterInfo> info;vector<SChapterSector> original;
        ~SectorRestore(){if(IsValid(info))info->sectorsSet.swap(original);}} sectors;
    CObj<NRPG::CGlobalGame> game;
    if(context==0){
        CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());while(maps.MoveNext())if(IsValid(maps.Get()->pBackground)){restore.chapter=maps.Get();break;}
        Require(IsValid(restore.chapter),"world camera locator fixture has no authored chapter");restore.entry=restore.chapter->pScript;restore.chapter->pScript=quiet.GetPtr();
        game=NRPG::CreateGlobalGame(1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        list<CPtr<NScenario::CScenarioClue>> clues;game->pScenarioTracker->GetAvailableClues(&clues);for(auto clue:clues)if(IsValid(clue))clue->SetJustFound(false);
        for(auto merc:game->players.front()->mercs){vector<CDBPtr<NDb::CMedal>> medals;merc->GetJustFoundMedals(&medals);}
        CDGPtr<CPtrFuncBase<CChapterInfo>> info=shareChapterInfo.Get(restore.chapter->GetRecordID());info.Refresh();
        sectors.info=info->GetValue();Require(IsValid(sectors.info),"camera locator fixture has no cached native map info");
        sectors.original=sectors.info->sectorsSet;auto& isolated=sectors.info->sectorsSet;
        isolated.erase(std::remove_if(isolated.begin(),isolated.end(),[](const SChapterSector& sector){return sector.eType==RANDOM;}),isolated.end());
        Require(!isolated.empty(),"floor isolation removed every authored zone and exit");
        CObj<NGame::CICBeginChapter> enter=new NGame::CICBeginChapter(restore.chapter->GetRecordID(),game);enter->Exec();
    }else{
        CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
        while(variants.MoveNext())if(IsValid(variants.Get()->pTemplate) && variants.Get()->pTemplate->GetRecordID()==2425){restore.variants.push_back({variants.Get(),variants.Get()->pScript});variants.Get()->pScript=quiet.GetPtr();}
        Require(!restore.variants.empty(),"world camera locator fixture has no authored menu variants");CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    }
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* world=owner?dynamic_cast<NWorld::CWorld*>(owner->GetWorld()):nullptr;auto* view=owner?owner->GetScene():nullptr;auto* renderer=owner?owner->GetRenderGame():nullptr;
    Require(world && view && renderer && world->GetOwnScript(),"world camera locator fixture lacks actual chapter/menu world, view, renderer or Lua");
    Require(context==0?owner->GetDesktop()!=nullptr:owner->GetDesktop()==nullptr,"world camera locator fixture lost its real chapter/null-menu desktop distinction");
    // Materialize authored static world effects before taking geometry baselines.
    NMainLoop::SetTimeForHarness(1050);Require(NMainLoop::StepApp(true,false,false),"world camera locator warm-up frame exited");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"world camera locator warm-up opened an unrelated interface");
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}

    struct OptionRestore {NGlobal::CValue follow,hints;~OptionRestore(){NGlobal::SetVar("ui_followcamera",follow);NGlobal::SetVar("ui_showhints",hints);}}
        options{NGlobal::GetVar("ui_followcamera",1),NGlobal::GetVar("ui_showhints",1)};
    NGlobal::SetVar("ui_followcamera",mode==5?0:1);NGlobal::SetVar("ui_showhints",1);
    Require(owner->GetFollowCameraState()==(mode!=5),"camera locator fixture lost its native follow option binding");
    Require(context==0?owner->GetActivePlayer()!=nullptr:owner->GetActivePlayer()==nullptr,"camera locator fixture lost its player/null-player distinction");
    Require(!world->IsBase(),"camera locator fixture unexpectedly selected a base world");
    if(context==0){auto* ui=dynamic_cast<NUI::CChapterMapUI*>(owner->GetInterface()->GetChildByID("chaptermapUI"));
        Require(ui && ui->GetTravelForDiagnostics().find(" type=1 template=")==string::npos,"camera locator fixture retained a random encounter");}
    struct BaseState {CObj<ICamera> camera;CObj<NGame::CUICmdLocatorExec> exec;
        int operator&(CStructureSaver& f){f.Add(18,&camera);f.Add(21,&exec);return 0;}};
    auto state=[&](){BaseState value;struct RenderState {BaseState* value;int operator&(CStructureSaver& f){f.Add(1,value);return 0;}} render{&value};
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(owner,&refs);
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);if(context==0)reader.Add(1,&value);else reader.Add(1,&render);}return value;};
    auto initialState=state();Require(initialState.camera && !IsValid(initialState.exec),"quiet camera locator fixture has no cinematic camera or has an executor");
    CObj<ICamera> cinematic=initialState.camera;CObj<ICamera> player=context==0?owner->GetActivePlayer()->GetCamera():nullptr;
    CObj<ICamera> destination=context==0 && mode==5?player.GetPtr():cinematic.GetPtr();
    auto pose=[](ICamera* camera){ICamera::SCameraPos value;camera->GetPlacement(&value);return value;};
    const auto liveStart=pose(owner->GetCamera()),cinematicStart=pose(cinematic),playerStart=player?pose(player):liveStart;
    Require(std::isfinite(liveStart.ptAnchor.x) && liveStart.fRod>0,"camera locator fixture has an invalid live starting pose");
    struct CameraRestore {CDBPtr<NDb::CDBCamera> record;ICamera::SCameraPos original;
        ~CameraRestore(){if(IsValid(record)){record->vAnchor=original.ptAnchor;record->fDistance=original.fRod;record->fPitch=original.fPitch;record->fYaw=original.fYaw;record->fRoll=original.fRoll;record->fFOV=original.fFOV;}}};
    vector<CDBPtr<NDb::CDBCamera>> records;CDBIterator<NDb::CDBCamera> cameras(*NDatabase::GetTable<NDb::CDBCamera>());
    while(cameras.MoveNext() && records.size()<2)records.push_back(cameras.Get());Require(records.size()==2,"camera locator fixture lacks real DB camera records");
    auto dbPose=[](NDb::CDBCamera* camera){return ICamera::SCameraPos(camera->vAnchor,camera->fDistance,camera->fPitch,camera->fYaw,camera->fRoll,camera->fFOV);};
    CameraRestore restore0{records[0],dbPose(records[0])},restore1{records[1],dbPose(records[1])};
    ICamera::SCameraPos targets[2]={liveStart,liveStart};
    for(int i=0;i<2;++i){targets[i].ptAnchor+=CVec3(0.5f*(i+1),0.25f*(i+1),0);
        auto* record=records[i].GetPtr();record->vAnchor=targets[i].ptAnchor;record->fDistance=targets[i].fRod;record->fPitch=targets[i].fPitch;
        record->fYaw=targets[i].fYaw;record->fRoll=targets[i].fRoll;record->fFOV=targets[i].fFOV;}
    auto* script=world->GetOwnScript();
    auto number=[&](const char* name,int wanted){const int top=script->GetTop();auto value=script->GetGlobal(name);const bool yes=value.IsNumber() && value.GetNumber()==wanted;script->SetTop(top);return yes;};
    CDBPtr<NDb::CUIHint> hint;
    if(mode==4){CDBIterator<NDb::CUIHint> hints(*NDatabase::GetTable<NDb::CUIHint>());while(hints.MoveNext())if(IsValid(hints.Get()->pString) && hints.Get()->nSequenceID==0){hint=hints.Get();break;}
        Require(IsValid(hint) && script->GetGlobalGame(),"camera locator modal fixture lacks a zero-reward native hint");script->GetGlobalGame()->hintsSet.clear();}
    // A modal gap can advance the chapter game counter by its existing150ms cap on return.
    // Use a one-second move so both native clock domains still expose interpolation.
    const int duration=mode==1 || mode==5?0:mode==4?1000:200;
    const string move=mode==3?"CameraSequence({"+std::to_string(records[0]->GetRecordID())+","+std::to_string(records[1]->GetRecordID())+"},200)":
        "CameraMove(GetCamera("+std::to_string(records[0]->GetRecordID())+"),"+std::to_string(duration)+")";
    const string code=mode==0?"LocatorProduced = 1":
        "LocatorID = "+move+"; LocatorProduced = 1; "+(mode==4?"LocatorHint = ShowHint("+std::to_string(hint->GetRecordID())+"); ":"")+
        "while IsUIActionIDPresent(LocatorID) do Sleep(1) end; LocatorResumed = 1";
    Require(script->DoString(code.c_str())==0,"camera locator fixture could not queue its real Lua producer");
    world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);Require(number("LocatorProduced",1),"camera locator Lua producer never ran");
    int eventID=-1;
    if(mode!=0){const int top=script->GetTop();auto id=script->GetGlobal("LocatorID");Require(id.IsNumber(),"native camera producer returned no wait ID");eventID=int(id.GetNumber());script->SetTop(top);
        Require(script->IsUIActionIDPresent(eventID) && !number("LocatorResumed",1),"camera locator wait completed before its native consumer");}
    STime usedClock=0;
    auto frame=[&](STime tick,bool active=true){if(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand())world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        if(context==1)usedClock=world->GetAimTime()->GetValue();NMainLoop::SetTimeForHarness(tick);Require(NMainLoop::StepApp(active,false,false),"camera locator native frame exited");
        if(context==0)usedClock=owner->GetGameTime();};
    auto sameAnchor=[](const CVec3& a,const CVec3& b){return fabs2(a-b)<1e-6f;};
    frame(1100);Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr(),"camera locator changed owner before its native modal");
    if(mode==0){Require(!IsValid(state().exec) && sameAnchor(pose(cinematic).ptAnchor,cinematicStart.ptAnchor),"camera locator control created an executor or moved the cinematic camera");
        std::printf("native world camera locator context=%d mode=0 stage=control\n",context);return;}
    auto first=state();
    std::printf("native world camera locator context=%d mode=%d stage=dispatch exec=%d wait=%d clock=%u\n",context,mode,int(IsValid(first.exec)),int(script->IsUIActionIDPresent(eventID)),unsigned(usedClock));std::fflush(stdout);
    if(duration==0)Require(!IsValid(first.exec) && sameAnchor(pose(destination).ptAnchor,targets[0].ptAnchor),"non-tactical world discarded its camera locator command");
    else Require(IsValid(first.exec),"non-tactical world discarded its camera locator command");
    Require(mode==4?NMainLoop::HaveInterfaceCommand():!NMainLoop::HaveInterfaceCommand(),"camera locator fixture queued an unexpected interface command");
    STime begin=0;bool sawMiddle=false,sawSecond=false;
    auto inspect=[&](){auto current=state();const auto actual=pose(destination);
        if(IsValid(current.exec)){Require(pSSClasses->GetObjectTypeID(current.exec.GetPtr())==int(0x50412162) && current.exec->GetPriority()==6,"camera locator lost its real move executor/priority");
            CStructureNetworkContext refs;auto fields=BorrowedObjectFields(current.exec,&refs);STime start=0,transition=0;int segment=-1;CPtr<NGame::IMission> origin;
            {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&origin);reader.Add(4,&start);reader.Add(5,&transition);reader.Add(8,&segment);}
            Require(origin.GetPtr()==owner.GetPtr() && start!=0 && transition==STime(duration),"camera locator lost its native owner, start time or per-waypoint duration");
            if(!begin)begin=start;Require(start==begin && usedClock>=start,"camera locator start clock changed during native execution");
            const STime elapsed=usedClock-start;
            if(mode!=3 && elapsed>0 && elapsed<STime(duration)){const float fraction=float(elapsed)/float(duration);
                Require(sameAnchor(actual.ptAnchor,liveStart.ptAnchor+(targets[0].ptAnchor-liveStart.ptAnchor)*fraction),"camera locator did not interpolate against its native owner clock");sawMiddle=true;}
            if(mode==3 && segment==1)sawSecond=true;
            Require(owner->GetCamera()==destination.GetPtr(),"active camera locator selected the wrong native camera");
            std::printf("native world camera locator context=%d mode=%d stage=move clock=%u begin=%u segment=%d x=%.6f\n",context,mode,unsigned(usedClock),unsigned(start),segment,actual.ptAnchor.x);
        }return IsValid(current.exec);};
    if(duration!=0)inspect();
    if(mode>=6){
        // Do not retain a diagnostic strong reference to the outgoing executor.
        // Its lifetime during replacement must be owned only by the real slot.
        frame(1150);Require(inspect() && script->IsUIActionIDPresent(eventID),"replacement fixture lost its running first camera move");
        const auto outgoing=pose(destination);
        Require(!sameAnchor(outgoing.ptAnchor,liveStart.ptAnchor) && !sameAnchor(outgoing.ptAnchor,targets[0].ptAnchor),
            "replacement fixture did not reach an intermediate first-move pose");
        const auto wantedStart=context==0?pose(player):outgoing;
        first.exec=nullptr;
        const int replacementDuration=mode==6?0:200;
        const string replacement="ReplacementID = CameraMove(GetCamera("+std::to_string(records[1]->GetRecordID())+"),"+
            std::to_string(replacementDuration)+"); ReplacementProduced = 1; while IsUIActionIDPresent(ReplacementID) do Sleep(1) end; ReplacementResumed = 1";
        Require(script->DoString(replacement.c_str())==0,"replacement fixture could not queue its second real Lua producer");
        world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        Require(number("ReplacementProduced",1),"replacement Lua producer never ran beside the first waiting coroutine");
        const int top=script->GetTop();auto id=script->GetGlobal("ReplacementID");Require(id.IsNumber(),"replacement camera returned no native wait ID");
        const int replacementID=int(id.GetNumber());script->SetTop(top);
        Require(replacementID!=eventID && script->IsUIActionIDPresent(eventID) && script->IsUIActionIDPresent(replacementID),
            "replacement fixture lost its two distinct pending native waits before dispatch");
        frame(1200);Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"replacement camera changed its native owner");
        {auto replacementState=state();
            if(replacementDuration==0)Require(!IsValid(replacementState.exec) && sameAnchor(pose(destination).ptAnchor,targets[1].ptAnchor),
                "immediate replacement camera did not land its own target");
            else {Require(IsValid(replacementState.exec) && replacementState.exec->GetPriority()==6,"timed replacement did not create its real equal-priority executor");
                CStructureNetworkContext refs;auto fields=BorrowedObjectFields(replacementState.exec,&refs);
                ICamera::SCameraPos sampled;STime start=0,transition=0;CPtr<NGame::IMission> origin;
                {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&origin);reader.Add(4,&start);reader.Add(5,&transition);reader.Add(6,&sampled);}
                std::printf("native camera replacement context=%d mode=%d stage=start sampled=%.6f wanted=%.6f outgoing=%.6f clock=%u begin=%u\n",
                    context,mode,sampled.ptAnchor.x,wantedStart.ptAnchor.x,outgoing.ptAnchor.x,unsigned(usedClock),unsigned(start));std::fflush(stdout);
                Require(origin.GetPtr()==owner.GetPtr() && start==usedClock && transition==200,"replacement executor lost its native owner or fresh clock");
                Require(sameAnchor(sampled.ptAnchor,wantedStart.ptAnchor) && sameAnchor(pose(destination).ptAnchor,wantedStart.ptAnchor),
                    "replacement camera sampled the outgoing locator instead of the native selected camera");}}
        for(int step=0;step<12 && (IsValid(state().exec) || script->IsUIActionIDPresent(eventID) || script->IsUIActionIDPresent(replacementID) ||
            !number("LocatorResumed",1) || !number("ReplacementResumed",1));++step)frame(1250+step*50);
        std::printf("native camera replacement context=%d mode=%d stage=complete oldWait=%d newWait=%d oldResumed=%d newResumed=%d exec=%d x=%.6f\n",
            context,mode,int(script->IsUIActionIDPresent(eventID)),int(script->IsUIActionIDPresent(replacementID)),int(number("LocatorResumed",1)),
            int(number("ReplacementResumed",1)),int(IsValid(state().exec)),pose(destination).ptAnchor.x);std::fflush(stdout);
        Require(!script->IsUIActionIDPresent(replacementID) && number("ReplacementResumed",1) && !IsValid(state().exec) && sameAnchor(pose(destination).ptAnchor,targets[1].ptAnchor),
            "replacement camera never completed its own native move and wait");
        Require(!script->IsUIActionIDPresent(eventID) && number("LocatorResumed",1),"replaced camera move kept its original Lua wait alive");
        if(context==0)Require(owner->GetCamera()==player.GetPtr() && sameAnchor(pose(player).ptAnchor,playerStart.ptAnchor),"replacement changed the chapter player camera ownership or pose");
        Require(!world->GetUICommand(),"replacement camera command pump left a queued command");return;
    }
    STime nextTick=1150;
    if(mode==4){const auto held=pose(destination);auto heldExec=first.exec;frame(1150);auto* modal=NMainLoop::GetCurrentInterfaceForHarness();
        Require(modal && pSSClasses->GetObjectTypeID(modal)==int(0x1423130) && NMainLoop::GetInterfaceStackDepth()==2,"camera locator did not open its real hint modal");
        const STime covered=world->GetAimTime()->GetValue();frame(1300);frame(1500);
        Require(world->GetAimTime()->GetValue()==covered && state().exec.GetPtr()==heldExec.GetPtr() && sameAnchor(pose(destination).ptAnchor,held.ptAnchor) && script->IsUIActionIDPresent(eventID),"covered camera locator advanced or released its native wait");
        NInput::SEvent input;while(NInput::GetEvent(&input)){}NInput::PostEvent("cancel");Require(NInput::GetEvent(&input) && modal->ProcessEvent(input),"camera locator hint did not accept cancel");frame(1550,false);
        Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && NMainLoop::GetInterfaceStackDepth()==1,"camera locator modal did not restore its native owner");
        std::printf("native world camera locator context=%d mode=4 stage=modal-paused clock=%u gameAfterClose=%u worldAfterClose=%u\n",context,unsigned(covered),unsigned(owner->GetGameTime()),unsigned(world->GetAimTime()->GetValue()));nextTick=1600;}
    for(int step=0;step<32 && (IsValid(state().exec) || script->IsUIActionIDPresent(eventID) || !number("LocatorResumed",1));++step){frame(nextTick);nextTick+=50;
        Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"camera locator was covered by an unrelated interface");if(duration!=0)inspect();}
    const auto final=pose(destination);const auto& wanted=targets[mode==3?1:0];
    Require(!IsValid(state().exec) && !script->IsUIActionIDPresent(eventID) && number("LocatorResumed",1) && sameAnchor(final.ptAnchor,wanted.ptAnchor),"camera locator never landed or released its native Lua wait");
    if(mode==2 || mode==4)Require(sawMiddle,"timed camera locator skipped every intermediate frame");
    if(mode==3)Require(sawSecond,"camera sequence never entered its second native waypoint segment");
    if(context==0){Require(owner->GetCamera()==player.GetPtr(),"finished chapter locator did not return selection to its actual player camera");
        if(mode==5)Require(sameAnchor(pose(cinematic).ptAnchor,cinematicStart.ptAnchor),"disabled-follow script move changed the unrelated cinematic camera");
        else Require(sameAnchor(pose(player).ptAnchor,playerStart.ptAnchor),"followed script move changed the chapter player camera");}
    std::printf("native world camera locator context=%d mode=%d stage=complete released=1 middle=%d second=%d x=%.6f\n",context,mode,int(sawMiddle),int(sawSecond),final.ptAnchor.x);
    Require(!world->GetUICommand(),"camera locator command pump left a queued command");
}

static void WorldScriptFloor(const char* databasePath,const char* resourcePath,int context,int mode)
{
    Require(context>=0 && context<=1 && mode>=0 && mode<=4,"unknown shared-world floor case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"world floor fixture started with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm shared world floor regression");
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} frameClock{NMainLoop::SetTimeForHarness(1000)};
    CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    struct RestoreScripts {vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> variants;CPtr<NDb::CChapterMap> chapter;CPtr<NDb::CScript> entry;
        ~RestoreScripts(){for(auto& p:variants)if(IsValid(p.first))p.first->pScript=p.second;if(IsValid(chapter))chapter->pScript=entry;}} restore;
    struct SectorRestore {CObj<CChapterInfo> info;vector<SChapterSector> original;
        ~SectorRestore(){if(IsValid(info))info->sectorsSet.swap(original);}} sectors;
    CObj<NRPG::CGlobalGame> game;
    if(context==0){
        CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());while(maps.MoveNext())if(IsValid(maps.Get()->pBackground)){restore.chapter=maps.Get();break;}
        Require(IsValid(restore.chapter),"world floor fixture has no authored chapter");restore.entry=restore.chapter->pScript;restore.chapter->pScript=quiet.GetPtr();
        game=NRPG::CreateGlobalGame(1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        list<CPtr<NScenario::CScenarioClue>> clues;game->pScenarioTracker->GetAvailableClues(&clues);for(auto clue:clues)if(IsValid(clue))clue->SetJustFound(false);
        for(auto merc:game->players.front()->mercs){vector<CDBPtr<NDb::CMedal>> medals;merc->GetJustFoundMedals(&medals);}
        CDGPtr<CPtrFuncBase<CChapterInfo>> info=shareChapterInfo.Get(restore.chapter->GetRecordID());info.Refresh();
        sectors.info=info->GetValue();Require(IsValid(sectors.info),"floor fixture has no cached native map info");
        sectors.original=sectors.info->sectorsSet;auto& isolated=sectors.info->sectorsSet;
        isolated.erase(std::remove_if(isolated.begin(),isolated.end(),[](const SChapterSector& sector){return sector.eType==RANDOM;}),isolated.end());
        Require(!isolated.empty(),"floor isolation removed every authored zone and exit");
        CObj<NGame::CICBeginChapter> enter=new NGame::CICBeginChapter(restore.chapter->GetRecordID(),game);enter->Exec();
    }else{
        CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
        while(variants.MoveNext())if(IsValid(variants.Get()->pTemplate) && variants.Get()->pTemplate->GetRecordID()==2425){restore.variants.push_back({variants.Get(),variants.Get()->pScript});variants.Get()->pScript=quiet.GetPtr();}
        Require(!restore.variants.empty(),"world floor fixture has no authored menu variants");CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    }
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* world=owner?dynamic_cast<NWorld::CWorld*>(owner->GetWorld()):nullptr;auto* view=owner?owner->GetScene():nullptr;auto* renderer=owner?owner->GetRenderGame():nullptr;
    Require(world && view && renderer && world->GetOwnScript(),"world floor fixture lacks actual chapter/menu world, view, renderer or Lua");
    Require(context==0?owner->GetDesktop()!=nullptr:owner->GetDesktop()==nullptr,"world floor fixture lost its real chapter/null-menu desktop distinction");
    // Materialize authored static world effects before taking geometry baselines.
    NMainLoop::SetTimeForHarness(1050);Require(NMainLoop::StepApp(true,false,false),"world floor warm-up frame exited");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"world floor warm-up opened an unrelated interface");
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}

    auto* selected=owner->GetCamera();
    Require(selected && (context==0?owner->GetActivePlayer()!=nullptr:owner->GetActivePlayer()==nullptr),
        "world floor fixture lost its player/null-player distinction");
    if(context==0){auto* ui=dynamic_cast<NUI::CChapterMapUI*>(owner->GetInterface()->GetChildByID("chaptermapUI"));
        Require(ui && ui->GetTravelForDiagnostics().find(" type=1 template=")==string::npos,"world floor fixture retained a random encounter");}
    view->SetCutFloorRange(-1,2);view->SetCutFloor(0);
    Require(view->GetCutFloor()==0,"world floor fixture could not establish its native rendered baseline");
    vector<string> commands;
    if(mode==0)commands={"WorldFloorControl = 1"};
    else if(mode==1)commands={"Floor(1)"};
    else if(mode==2)commands={"Floor(-99)"};
    else if(mode==3)commands={"Floor(99)"};
    else commands={"SetTutorialMode(1); CameraLock(); Floor(1)","CameraLock(nil); Floor(1)"};
    auto* script=world->GetOwnScript();
    for(int round=0;round<commands.size();++round){
        Require(script->DoString((commands[round]+"; WorldFloorProduced = "+std::to_string(round+1)).c_str())==0,
            "world floor fixture could not execute its real Lua producer");
        world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        const int top=script->GetTop();auto produced=script->GetGlobal("WorldFloorProduced");
        const bool ran=produced.IsNumber() && produced.GetNumber()==round+1;script->SetTop(top);Require(ran,"world floor Lua producer never ran");
        NMainLoop::SetTimeForHarness(1100+round*50);Require(NMainLoop::StepApp(true,false,false),"world floor native interface frame exited");
        Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"world floor fixture opened an unrelated interface");
        const int wanted=mode==0 || (mode==4 && round==0)?0:mode==2?-1:mode==3?2:1;
        int minimum=-99,maximum=-99;view->GetCutFloorRange(&minimum,&maximum);
        std::printf("native world floor context=%d mode=%d round=%d actual=%d wanted=%d min=%d max=%d selected=%d\n",
            context,mode,round,view->GetCutFloor(),wanted,minimum,maximum,int(owner->GetCamera()==selected));std::fflush(stdout);
        Require(view->GetCutFloor()==wanted,"non-tactical world discarded its rendered floor command");
        Require(minimum==-1 && maximum==2 && owner->GetCamera()==selected,"world floor command replaced native range or camera ownership");
        Require(!world->GetUICommand(),"world floor command pump left a queued command");
    }
}

static void WorldScriptCameraLock(const char* databasePath,const char* resourcePath,int context,int mode)
{
    Require(context>=0 && context<=1 && mode>=0 && mode<=3,"unknown shared-world camera lock case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"world camera lock fixture started with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm shared world camera lock regression");
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} frameClock{NMainLoop::SetTimeForHarness(1000)};
    CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    struct RestoreScripts {vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> variants;CPtr<NDb::CChapterMap> chapter;CPtr<NDb::CScript> entry;
        ~RestoreScripts(){for(auto& p:variants)if(IsValid(p.first))p.first->pScript=p.second;if(IsValid(chapter))chapter->pScript=entry;}} restore;
    struct SectorRestore {CObj<CChapterInfo> info;vector<SChapterSector> original;
        ~SectorRestore(){if(IsValid(info))info->sectorsSet.swap(original);}} sectors;
    CObj<NRPG::CGlobalGame> game;
    if(context==0){
        CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());while(maps.MoveNext())if(IsValid(maps.Get()->pBackground)){restore.chapter=maps.Get();break;}
        Require(IsValid(restore.chapter),"world camera lock fixture has no authored chapter");restore.entry=restore.chapter->pScript;restore.chapter->pScript=quiet.GetPtr();
        game=NRPG::CreateGlobalGame(1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        list<CPtr<NScenario::CScenarioClue>> clues;game->pScenarioTracker->GetAvailableClues(&clues);for(auto clue:clues)if(IsValid(clue))clue->SetJustFound(false);
        for(auto merc:game->players.front()->mercs){vector<CDBPtr<NDb::CMedal>> medals;merc->GetJustFoundMedals(&medals);}
        CDGPtr<CPtrFuncBase<CChapterInfo>> info=shareChapterInfo.Get(restore.chapter->GetRecordID());info.Refresh();
        sectors.info=info->GetValue();Require(IsValid(sectors.info),"camera lock fixture has no cached native map info");
        sectors.original=sectors.info->sectorsSet;auto& isolated=sectors.info->sectorsSet;
        isolated.erase(std::remove_if(isolated.begin(),isolated.end(),[](const SChapterSector& sector){return sector.eType==RANDOM;}),isolated.end());
        Require(!isolated.empty(),"camera lock isolation removed every authored zone and exit");
        CObj<NGame::CICBeginChapter> enter=new NGame::CICBeginChapter(restore.chapter->GetRecordID(),game);enter->Exec();
    }else{
        CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
        while(variants.MoveNext())if(IsValid(variants.Get()->pTemplate) && variants.Get()->pTemplate->GetRecordID()==2425){restore.variants.push_back({variants.Get(),variants.Get()->pScript});variants.Get()->pScript=quiet.GetPtr();}
        Require(!restore.variants.empty(),"world camera lock fixture has no authored menu variants");CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    }
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* world=owner?dynamic_cast<NWorld::CWorld*>(owner->GetWorld()):nullptr;auto* view=owner?owner->GetScene():nullptr;auto* renderer=owner?owner->GetRenderGame():nullptr;
    Require(world && view && renderer && world->GetOwnScript(),"world camera lock fixture lacks actual chapter/menu world, view, renderer or Lua");
    Require(context==0?owner->GetDesktop()!=nullptr:owner->GetDesktop()==nullptr,"world camera lock fixture lost its real chapter/null-menu desktop distinction");
    // Materialize authored static world effects before taking geometry baselines.
    NMainLoop::SetTimeForHarness(1050);Require(NMainLoop::StepApp(true,false,false),"world camera lock warm-up frame exited");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"world camera lock warm-up opened an unrelated interface");
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}

    auto* selected=owner->GetCamera();auto* tracker=owner->GetActivePlayer();
    auto* playerCamera=tracker?tracker->GetCamera():nullptr;
    Require(selected && (context==0?tracker!=nullptr:tracker==nullptr),"camera lock fixture lost its player/null-player distinction");
    if(context==0){auto* ui=dynamic_cast<NUI::CChapterMapUI*>(owner->GetInterface()->GetChildByID("chaptermapUI"));
        Require(ui && ui->GetTravelForDiagnostics().find(" type=1 template=")==string::npos,"camera lock fixture retained a random encounter");}
    struct LockState {int scroll=-1000,frozen=-1000;
        int operator&(CStructureSaver& f){f.Add(2,&scroll);f.Add(14,&frozen);return 0;}};
    auto state=[](ICamera* camera){LockState value;CStructureNetworkContext refs;auto fields=BorrowedObjectFields(camera,&refs);
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,&value);}return value;};
    const auto initial=state(selected);Require(initial.frozen==0,"quiet camera lock fixture started frozen");
    const auto playerInitial=playerCamera?state(playerCamera):LockState{};
    selected->SetCutFloorRange(-1,2);view->SetCutFloorRange(-1,2);
    const bool tutorial=mode==2,alias=selected==playerCamera;
    vector<string> commands;
    if(mode==0)commands={"SetTutorialMode(nil)"};
    else if(mode==3)commands={"SetTutorialMode(nil); CameraLock(nil)","CameraLock()","CameraLock()","CameraLock(nil)"};
    else commands={tutorial?"SetTutorialMode(1); CameraLock()":"SetTutorialMode(nil); CameraLock()","CameraLock()","CameraLock(nil)","CameraLock(nil)"};
    int net=0;auto* script=world->GetOwnScript();
    for(int round=0;round<commands.size();++round){
        if(mode!=0)net+=mode==3?(round==0 || round==3?-1:1):(round<2?1:-1);
        Require(script->DoString((commands[round]+"; WorldLockProduced = "+std::to_string(round+1)).c_str())==0,
            "camera lock fixture could not execute its real Lua producer");
        world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        const int top=script->GetTop();auto produced=script->GetGlobal("WorldLockProduced");
        const bool ran=produced.IsNumber() && produced.GetNumber()==round+1;script->SetTop(top);Require(ran,"camera lock Lua producer never ran");
        NMainLoop::SetTimeForHarness(1100+round*50);Require(NMainLoop::StepApp(true,false,false),"camera lock native interface frame exited");
        Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"camera lock fixture opened an unrelated interface");
        Require(owner->GetCamera()==selected,"camera lock changed selected-camera ownership");
        const auto actual=state(selected);const auto player=playerCamera?state(playerCamera):LockState{};
        const int wanted=initial.frozen+net*(tutorial && alias?2:1);
        const int wantedScroll=initial.scroll+net*(tutorial && alias?2:1);
        const int wantedPlayer=playerCamera?playerInitial.frozen+net*(alias?(tutorial?2:1):(tutorial?1:0)):-1000;
        std::printf("native world camera lock context=%d mode=%d round=%d net=%d tutorial=%d alias=%d frozen=%d scroll=%d wanted=%d player=%d wantedPlayer=%d\n",
            context,mode,round,net,int(tutorial),int(alias),actual.frozen,actual.scroll,wanted,player.frozen,wantedPlayer);std::fflush(stdout);
        Require(actual.frozen==wanted && actual.scroll==wantedScroll && player.frozen==wantedPlayer,
            "non-tactical world discarded its camera lock command or tutorial tracker freeze");
        const int cameraBefore=selected->GetCutFloor(),cameraTarget=cameraBefore==0?1:0;
        const int sceneBefore=view->GetCutFloor(),sceneTarget=sceneBefore==0?1:0;
        selected->SetCutFloor(cameraTarget);view->SetCutFloor(sceneTarget);
        Require(selected->GetCutFloor()==(wanted>0?cameraBefore:cameraTarget),"camera freeze did not gate the actual selected camera floor");
        Require(view->GetCutFloor()==(net>0?sceneBefore:sceneTarget),"camera lock lost the remaster scene floor-lock mirror");
        std::printf("native world camera lock floors context=%d mode=%d round=%d cameraBlocked=%d sceneBlocked=%d\n",
            context,mode,round,int(selected->GetCutFloor()==cameraBefore),int(view->GetCutFloor()==sceneBefore));
        Require(!world->GetUICommand(),"camera lock command pump left a queued command");
    }
}
