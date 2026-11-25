class HotseatVisibilityWorld: public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(HotseatVisibilityWorld);
public:
    using NWorld::CWorld::RegisterPlayer;
    using NWorld::CWorld::UnregisterPlayer;
};

static void HotseatOpponentVisibility()
{
    CObj<HotseatVisibilityWorld> world = new HotseatVisibilityWorld;
    CObj<NRPG::CGlobalDiplomacy> diplomacy = NRPG::CreateGlobalDiplomacy();
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.ids[diplomacy.GetPtr()] = 1; references.refs[1] = diplomacy.GetPtr();
    references.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved,CStructureSaver::WRITE,&references); writer.Add(40,&diplomacy); }
    saved.Seek(0);
    { CStructureSaver reader(saved,CStructureSaver::READ,&references); static_cast<CObjectBase*>(world.GetPtr())->operator&(reader); }
    Require(world->GetDiplomacy() == diplomacy.GetPtr(), "hotseat vision fixture did not bind its native diplomacy table");
    auto player = [&](const wchar_t* name,int scenario) {
        CObj<NWorld::CPlayer> result = new NWorld::CPlayer(name,nullptr,nullptr,scenario);
        result->SetCommander(new NAI::CSequenceCommander(world));
        world->RegisterPlayer(result);
        Require(!NAI::IsAIPlayer(result.GetPtr()), "hotseat vision fixture unexpectedly created an AI-controlled player");
        return result;
    };
    auto first = player(L"first hotseat player",0);
    auto second = player(L"second hotseat player",0);
    Require(world->GetDiplomacyState(static_cast<NWorld::IPlayer*>(first.GetPtr()),
        static_cast<NWorld::IPlayer*>(second.GetPtr())) == NDb::DS_ENEMY &&
        diplomacy->GetDiplomacyState(0,0) == NDb::DS_ALLY,
        "hotseat vision control did not reproduce distinct opponents sharing scenario ID zero");
    CObj<NWorld::CUnitServer> firstContact = new NWorld::CUnitServer;
    CObj<NWorld::CUnitServer> secondContact = new NWorld::CUnitServer;
    CObj<NWorld::CWindowDoor> object = new NWorld::CWindowDoor, trap = new NWorld::CWindowDoor;
    auto setVision = [&](NWorld::CPlayer* owner,NWorld::CUnitServer* contact,bool extras) {
        CStructureNetworkContext graph;
        graph.externalReferences = true; graph.nextID = 100;
        vector<CObjectBase*> objects{contact,object.GetPtr(),trap.GetPtr()};
        for (std::size_t i = 0; i < objects.size(); ++i) {
            graph.ids[objects[i]] = unsigned(i+1); graph.refs[unsigned(i+1)] = objects[i];
        }
        graph.include = [](CObjectBase*) { return false; };
        list<CPtr<NWorld::CUnitServer>> visible{contact};
        list<CPtr<CObjectBase>> visibleObjects, traps;
        if (extras) { visibleObjects.push_back(object.GetPtr()); traps.push_back(trap.GetPtr()); }
        CMemoryStream state;
        { CStructureSaver writer(state,CStructureSaver::WRITE,&graph);
          writer.Add(3,&visible); writer.Add(4,&visibleObjects); writer.Add(6,&traps); }
        state.Seek(0);
        { CStructureSaver reader(state,CStructureSaver::READ,&graph);
          static_cast<NWorld::CPlayerBaseVision<NWorld::CUnitServer>*>(owner)->operator&(reader); }
    };
    setVision(first,firstContact,true); setVision(second,secondContact,false);
    Require(first->CanSee(firstContact) && !second->CanSee(firstContact) && second->CanSee(secondContact),
        "hotseat vision fixture did not restore distinct native contact sets");
    world->MergeFriendlyPlayersVisibleSets();
    Require(!second->CanSee(firstContact) && !first->CanSee(secondContact) &&
        !second->CanSeeObject(object) && !second->CanSeeTrap(trap),
        "opposing hotseat players shared contacts, objects or traps merely because both used scenario ID zero");
    Require(first->CanSee(firstContact) && second->CanSee(secondContact) &&
        first->CanSeeObject(object) && first->CanSeeTrap(trap), "preventing opponent vision sharing erased existing sightings");
    world->UnregisterPlayer(second);
    auto ally = player(L"distinct allied side",1);
    const vector<CPtr<NRPG::IUnitMission>> units;
    diplomacy->SetDiplomacyState(0,units,1,NDb::DS_ALLY);
    diplomacy->SetDiplomacyState(1,units,0,NDb::DS_ALLY);
    Require(world->GetDiplomacyState(static_cast<NWorld::IPlayer*>(first.GetPtr()),
        static_cast<NWorld::IPlayer*>(ally.GetPtr())) == NDb::DS_ALLY,
        "allied-visibility control did not establish an actual native alliance");
    setVision(ally,secondContact,false);
    world->MergeFriendlyPlayersVisibleSets();
    Require(first->CanSee(secondContact) && ally->CanSee(firstContact) &&
        ally->CanSeeObject(object) && ally->CanSeeTrap(trap),
        "the opponent-visibility correction also prevented actual allies from sharing sightings");
}

struct MissionSoundClockCaptured {};

class MissionSoundClockRenderer: public NRender::IRenderGame
{
    OBJECT_NOCOPY_METHODS(MissionSoundClockRenderer);
public:
    STime time = 0;
    bool advance = false;
    int calls = 0;
    CObjectBase* Select(CObjectBase*,const CVec4&,bool) override { throw std::runtime_error("unexpected sound-clock selection"); }
    void FlashUnit(CObjectBase*,const CVec4&) override { throw std::runtime_error("unexpected sound-clock flash"); }
    CCTime* GetTime() override { throw std::runtime_error("unexpected sound-clock render timer query"); }
    NLSHead::CHeadsController* GetHeadController() const override { throw std::runtime_error("unexpected sound-clock head query"); }
    void UpdateViewWorld(bool,STime,NWorld::IPlayer*,bool) override { throw std::runtime_error("unexpected sound-clock world update"); }
    void FastUpdate(STime) override { throw std::runtime_error("unexpected sound-clock fast update"); }
    void ResetTiming() override { throw std::runtime_error("unexpected sound-clock reset"); }
    void UpdateSound(bool shouldAdvance,CTransformStack* transform,STime current) override {
        Require(transform != nullptr, "native mission did not supply its camera transform to sound");
        ++calls; time = current; advance = shouldAdvance;
        // Observe the real caller, then stop before any GPU rendering or presentation.
        throw MissionSoundClockCaptured();
    }
};

class MissionSoundClockMission: public NGame::CMissionBase
{
    OBJECT_NOCOPY_METHODS(MissionSoundClockMission);
protected:
    const STime GetTime() override { return 10000; }
public:
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    void Attach(NRender::IRenderGame* renderer,NGScene::IGameView* scene) {
        pRender = renderer; pScene = scene; sTimeCounter.SetCurrent(10000);
    }
    void SetSkippedTime(STime skipped) { nDeltaTime = skipped; }
    void DrawSound(ICamera* camera,bool advance) { RenderFrame(NGame::N_RENDERMODE_3D|8,advance,camera,false); }
};

static void MissionSoundGameClock()
{
    CObj<MissionSoundClockMission> mission = new MissionSoundClockMission;
    CObj<MissionSoundClockRenderer> renderer = new MissionSoundClockRenderer;
    CObj<NGScene::GameViewFixture> scene = new NGScene::GameViewFixture;
    CObj<ICamera> camera = ::CreateCamera(CAMERA_PC);
    Require(IsValid(camera), "sound-clock fixture could not create a real native camera");
    mission->Attach(renderer,scene);
    auto draw = [&](bool advance) {
        bool reached = false;
        try { mission->DrawSound(camera,advance); }
        catch (const MissionSoundClockCaptured&) { reached = true; }
        Require(reached, "native mission frame did not reach its sound update before rendering");
    };
    draw(true);
    Require(renderer->calls == 1 && renderer->advance && renderer->time == 10000,
        "ordinary native mission frame did not pass its baseline sound clock");
    mission->SetSkippedTime(5000);
    Require(mission->GetGameTime() == 15000, "sound-clock fixture did not retain the native skipped-game-time offset");
    draw(true);
    Require(renderer->calls == 2 && renderer->advance && renderer->time == 15000,
        "the native mission passed raw wall time to sound and omitted the fast-forwarded game interval");
    draw(false);
    Require(renderer->calls == 3 && !renderer->advance && renderer->time == 15000,
        "using the game clock lost the native sound pause flag or skipped-time offset");
}

namespace NSound { extern CBasicShare<int,CFileSample3D> share3DSamples; extern CBasicShare<int,CFileSample2D> share2DSamples; }

class CachedEffectSampleAccess: public NSound::CFileSample3D
{
public:
    static auto ValueMember() { return &CachedEffectSampleAccess::pValue; }
};

class EffectSoundSource: public NWorld::IVisObj
{
    OBJECT_NOCOPY_METHODS(EffectSoundSource);
public:
    CPtr<NDb::CSoundEffect> effect;
    CObj<NGScene::CCVec3> position = new NGScene::CCVec3(CVec3(0,0,0));
    STime start;
    EffectSoundSource(): start(0) {}
    EffectSoundSource(NDb::CSoundEffect* record,STime time): effect(record),start(time) {}
    void Visit(NWorld::ISoundVisitor* visitor) override { visitor->AddEffect(start,effect,position,vector<int>()); }
};

static void LegacySoundEffectFields(NDb::CSoundInstance* record,CFuncBase<STime>* time,CFuncBase<CVec3>* position)
{
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.ids[time] = 1; references.refs[1] = time;
    references.ids[position] = 2; references.refs[2] = position;
    references.include = [](CObjectBase*) { return false; };
    STime begin = 12000, last = 11000;
    CDBPtr<NDb::CSoundInstance> data = record;
    CDGPtr<CFuncBase<STime>> clock = time;
    CDGPtr<CFuncBase<CVec3>> placement = position;
    CObj<NFMSound::CSound3D> channel;
    vector<int> flags{21,34};
    CMemoryStream legacy;
    { CStructureSaver writer(legacy,CStructureSaver::WRITE,&references);
      writer.Add(2,&begin); writer.Add(3,&last); writer.Add(4,&data); writer.Add(5,&clock);
      writer.Add(6,&placement); writer.Add(7,&channel); writer.Add(8,&flags); }
    CObj<NSound::CSoundInstance> restored = new NSound::CSoundInstance;
    legacy.Seek(0);
    { CStructureSaver reader(legacy,CStructureSaver::READ,&references);
      static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
    CStructureNetworkContext roundReferences;
    auto saved = BorrowedObjectFields(restored,&roundReferences);
    STime savedBegin = 0, savedLast = 0;
    CDBPtr<NDb::CSoundInstance> savedRecord;
    CDGPtr<CFuncBase<STime>> savedTime;
    CDGPtr<CFuncBase<CVec3>> savedPosition;
    vector<int> savedFlags;
    { CStructureSaver reader(saved,CStructureSaver::READ,&roundReferences);
      reader.Add(2,&savedBegin); reader.Add(3,&savedLast); reader.Add(4,&savedRecord);
      reader.Add(5,&savedTime); reader.Add(6,&savedPosition); reader.Add(8,&savedFlags); }
    Require(savedBegin == begin && savedLast == last && savedRecord.GetPtr() == record &&
        savedTime.Get() == time && savedPosition.Get() == position && savedFlags == flags,
        "sound-effect ageing changed the existing saved field numbers or their original meanings");
}

static void DelayedSoundEffectAge(const char* databasePath,bool infiniteLoop = false)
{
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr, "sound-effect fixture requires headless native audio");
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
    Require(sounds.MoveNext(), "sound-effect fixture has no actual sample record");
    auto* soundRecord = sounds.Get(); soundRecord->bLoop = false;
    NFMSound::SStartInfo info; info.nMaxChannels = 4;
    Require(NFMSound::Init(info), "sound-effect fixture could not initialize audio");
    struct AudioCleanup { ~AudioCleanup() { ClearHoldQueue(); NFMSound::Done(); } } cleanup;
    const auto wave = SilentPCMFixture();
    CObj<NFMSound::CSample3D> sample = NFMSound::LoadSample3D(wave.data(),static_cast<int>(wave.size()),1.f,100.f,0);
    Require(IsValid(sample), "sound-effect fixture could not decode its real PCM sample");
    CObj<NSound::CFileSample3D> loader = NSound::share3DSamples.Get(soundRecord->GetRecordID());
    (loader.GetPtr()->*CachedEffectSampleAccess::ValueMember()) = sample.GetPtr();
    CDGPtr<CPtrFuncBase<NFMSound::CSample3D>> cached = loader.GetPtr();
    cached.Refresh();
    Require(cached->GetValue() == sample.GetPtr(), "sound-effect fixture did not seed the actual native sample cache");
    CObj<NDb::CSoundVariant> variant = new NDb::CSoundVariant;
    variant->pSound = soundRecord;
    CObj<NDb::CTSound> pool = new NDb::CTSound;
    pool->variants.push_back(variant.GetPtr()); pool->roulette.AddSector(1);
    CDBIterator<NDb::CSoundInstance> instances(*NDatabase::GetTable<NDb::CSoundInstance>());
    Require(instances.MoveNext(), "sound-effect fixture has no native instance record for save compatibility");
    auto* instance = instances.Get();
    instance->pSound = pool; instance->nStartTime = 200; instance->nCycleCount = 1;
    instance->bFadeIn = instance->bFadeOut = false; instance->nFadeSamples = 64;
    instance->eSoundType = NDb::ST_PERMANENT; instance->fSoundAvgInterval = 1.f; instance->nVolume = 128;
    CObj<NDb::CSoundEffect> effect = new NDb::CSoundEffect;
    effect->instances.push_back(instance);
    CObj<CSetSyncSrc<NWorld::IVisObj>> source = new CSetSyncSrc<NWorld::IVisObj>;
    CTimeCounter clock; clock.SetCurrent(10000);
    CObj<NSound::ISoundScene> scene = NSound::CreateSoundScene(nullptr,nullptr,clock.GetTime());
    CObj<NRender::IRenderSound> mixer = dynamic_cast<NRender::IRenderSound*>(pSSClasses->CreateObject(int(0x03081142)));
    Require(IsValid(mixer), "sound-effect fixture has no registered native render mixer");
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    const vector<CObjectBase*> objects{source.GetPtr(),scene.GetPtr(),clock.GetTime()};
    for (std::size_t i = 0; i < objects.size(); ++i) {
        references.ids[objects[i]] = unsigned(i+1); references.refs[unsigned(i+1)] = objects[i];
    }
    references.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved,CStructureSaver::WRITE,&references);
      writer.Add(1,&source); writer.Add(2,&scene); writer.Add(3,&clock); }
    saved.Seek(0);
    { CStructureSaver reader(saved,CStructureSaver::READ,&references); static_cast<CObjectBase*>(mixer.GetPtr())->operator&(reader); }
    CObj<ICamera> camera = ::CreateCamera(CAMERA_PC);
    CTransformStack transform; camera->GetTransform(&transform,CVec2(1024,768));
    auto tick = [&](STime time) { MarkNewDGFrame(); mixer->Update(true,&transform,9000 + time); };
    auto playingChannel = [&]() {
        int channel = -1;
        // Native handles increase across sequential starts; maxChannels only caps live voices.
        // This isolated fixture performs fewer than 16 starts, with handles beginning at one.
        for (int i = 0; i < 32; ++i) if (FSOUND_IsPlaying(i)) {
            Require(channel == -1, "sound-effect fixture unexpectedly has multiple playing channels"); channel = i;
        }
        return channel;
    };
    if (infiniteLoop) {
        instance->nStartTime = 0; instance->nCycleCount = -1;
        CObj<EffectSoundSource> loop = new EffectSoundSource(effect,10000);
        source->Set(vector<NWorld::IVisObj*>{loop.GetPtr()}); tick(1000);
        const int loopChannel = playingChannel();
        Require(loopChannel >= 0, "infinite-effect fixture did not start an actual native channel");
        Require(FSOUND_GetLoopMode(loopChannel) == FSOUND_LOOP_NORMAL,
            "an effect with infinite cycle count played once because its sample record was not itself looped");
#if defined(S2_UPSTREAM_NATIVE_SFX)
        vector<float> output(4096*2);
        Require(S2FmodRenderForTest(output.data(),4096) != 0, "infinite-effect fixture could not render real native PCM frames");
        Require(FSOUND_IsPlaying(loopChannel) && FSOUND_GetCurrentPosition(loopChannel) > 0 &&
            FSOUND_GetCurrentPosition(loopChannel) < 1000,
            "an infinitely repeated effect stopped instead of wrapping after more than four actual sample lengths");
#endif
        source->Set(vector<NWorld::IVisObj*>()); tick(1000);
        Require(playingChannel() == -1, "removing an infinite effect did not release its actual native channel");
        instance->nCycleCount = 1;
        CObj<EffectSoundSource> once = new EffectSoundSource(effect,10000);
        source->Set(vector<NWorld::IVisObj*>{once.GetPtr()}); tick(1000);
        const int onceChannel = playingChannel();
        Require(onceChannel >= 0 && FSOUND_GetLoopMode(onceChannel) == FSOUND_LOOP_OFF,
            "correcting infinite effects also looped an explicitly single-cycle effect");
#if defined(S2_UPSTREAM_NATIVE_SFX)
        Require(S2FmodRenderForTest(output.data(),4096) != 0, "single-cycle control could not render its native PCM frames");
        Require(!FSOUND_IsPlaying(onceChannel), "an explicitly single-cycle effect continued after its actual sample ended");
#endif
        source->Set(vector<NWorld::IVisObj*>()); tick(1000);
        Require(playingChannel() == -1, "infinite-effect fixture retained a native playback channel");
        return;
    }
    CObj<EffectSoundSource> expired = new EffectSoundSource(effect,9500);
    LegacySoundEffectFields(instance,clock.GetTime(),expired->position);
    source->Set(vector<NWorld::IVisObj*>{expired.GetPtr()});
    tick(1000);
    Require(clock.GetTime()->GetValue() == 10000 && playingChannel() == -1,
        "sound-effect initial-delay control unexpectedly played or moved its baseline clock");
    tick(1300);
    Require(clock.GetTime()->GetValue() == 10300, "sound-effect fixture did not advance its real mixer clock");
    Require(playingChannel() == -1,
        "an expired world sound effect restarted after visibility was restored instead of seeking past its sample end");
    source->Set(vector<NWorld::IVisObj*>()); tick(1300);
    instance->nStartTime = 0;
    CObj<EffectSoundSource> tail = new EffectSoundSource(effect,10240);
    source->Set(vector<NWorld::IVisObj*>{tail.GetPtr()}); tick(1300);
    const int tailChannel = playingChannel();
    Require(tailChannel >= 0 && FSOUND_GetCurrentPosition(tailChannel) == 441,
        "a still-audible late effect did not retain its ten-millisecond sample offset");
    source->Set(vector<NWorld::IVisObj*>()); tick(1300);
    Require(playingChannel() == -1, "removing an effect source retained its actual native channel");
    {
        // Save a not-yet-started runtime node so only the new seek state, not a live channel,
        // must survive the native object-field round trip.
        CObj<NSound::CSoundInstance> node = new NSound::CSoundInstance(instance,10,clock.GetTime(),tail->position,vector<int>());
        CStructureNetworkContext graph;
        auto snapshot = BorrowedObjectFields(node,&graph);
        CObj<NSound::CSoundInstance> restored = new NSound::CSoundInstance;
        { CStructureSaver reader(snapshot,CStructureSaver::READ,&graph);
          static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
        CDGPtr<CFuncBase<bool>> refreshed = restored.GetPtr();
        MarkNewDGFrame(); refreshed.Refresh();
        const int restoredChannel = playingChannel();
        Require(restoredChannel >= 0 && FSOUND_GetCurrentPosition(restoredChannel) == 441,
            "restoring a late sound-effect node discarded its saved sample seek offset");
    }
    Require(playingChannel() == -1, "releasing the restored effect node retained its native channel");
    instance->nStartTime = 200;
    CObj<EffectSoundSource> future = new EffectSoundSource(effect,10300);
    source->Set(vector<NWorld::IVisObj*>{future.GetPtr()}); tick(1300);
    Require(playingChannel() == -1, "a fresh effect ignored its authored start delay");
    tick(1499);
    Require(playingChannel() == -1, "a delayed effect started before its scheduled native clock value");
    tick(1500);
    const int freshChannel = playingChannel();
    Require(freshChannel >= 0 && FSOUND_GetCurrentPosition(freshChannel) == 0,
        "an ordinary delayed effect failed to begin at its authored time and sample start");
    source->Set(vector<NWorld::IVisObj*>()); tick(1500);
    Require(playingChannel() == -1, "sound-effect fixture retained a playback channel after source removal");
}

static void SoundMixerClockCatchUp()
{
    CObj<NWorld::CWorldSyncSrc> source = new NWorld::CWorldSyncSrc;
    CObj<CCTime> sceneTime = new CCTime(0);
    CObj<NSound::ISoundScene> scene = NSound::CreateSoundScene(nullptr,nullptr,sceneTime);
    CObj<NRender::IRenderSound> mixer = dynamic_cast<NRender::IRenderSound*>(pSSClasses->CreateObject(int(0x03081142)));
    Require(IsValid(mixer), "sound-clock fixture has no registered native render mixer");
    CTimeCounter clock;
    clock.SetCurrent(5000);
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.include = [](CObjectBase*) { return false; };
    const vector<CObjectBase*> objects{source.GetPtr(),scene.GetPtr(),clock.GetTime()};
    for (std::size_t i = 0; i < objects.size(); ++i) {
        references.ids[objects[i]] = unsigned(i+1); references.refs[unsigned(i+1)] = objects[i];
    }
    references.nextID = 100;
    CMemoryStream saved;
    { CStructureSaver writer(saved,CStructureSaver::WRITE,&references);
      writer.Add(1,&source); writer.Add(2,&scene); writer.Add(3,&clock); }
    saved.Seek(0);
    { CStructureSaver reader(saved,CStructureSaver::READ,&references); static_cast<CObjectBase*>(mixer.GetPtr())->operator&(reader); }
    CTransformStack transform; transform.Init();
    mixer->Update(true,&transform,5000);
    Require(clock.GetTime()->GetValue() == 5000, "the first mixer update changed its matching restored epoch");
    mixer->Update(true,&transform,5100);
    Require(clock.GetTime()->GetValue() == 5100, "ordinary mixer-frame control did not advance its actual shared clock");
    mixer->Update(true,&transform,10100);
    Require(clock.GetTime()->GetValue() == 10100,
        "the native sound mixer lost most of a skipped interval by clamping its clock advance to one 150ms step");
    mixer->Update(false,&transform,10100);
    Require(clock.GetTime()->GetValue() == 10100, "a paused mixer advanced while consuming its clock baseline");
    mixer->Update(true,&transform,10400);
    Require(clock.GetTime()->GetValue() == 10400, "resuming the mixer replayed paused time or lost the resumed interval");
    mixer->ResetTiming();
    mixer->Update(true,&transform,10400);
    Require(clock.GetTime()->GetValue() == 10400, "resetting mixer timing caused a spurious catch-up jump");
}

static void FirstRenderSoundClock()
{
    CObj<NWorld::CWorld> world = new NWorld::CWorld(nullptr);
    CObj<NGScene::GameViewFixture> view = new NGScene::GameViewFixture;
    CObj<NSound::ISoundScene> sound = NSound::CreateSoundScene(nullptr, nullptr, world->GetAimTime());
    CObj<NRender::IRenderGame> renderer = NRender::CreateRenderGame(world, view, sound);

    CPtr<NRender::IRenderSound> worldMixer, unitMixer;
    CStructureNetworkContext renderRefs;
    auto renderFields = BorrowedObjectFields(renderer, &renderRefs);
    {
        CStructureSaver reader(renderFields, CStructureSaver::READ, &renderRefs);
        reader.Add(15, &worldMixer);
        reader.Add(16, &unitMixer);
    }
    Require(IsValid(worldMixer) && IsValid(unitMixer), "the renderer did not construct its two real mixers");
    auto getClock = [](NRender::IRenderSound* mixer) {
        CTimeCounter clock;
        CStructureNetworkContext refs;
        auto fields = BorrowedObjectFields(mixer, &refs);
        CStructureSaver reader(fields, CStructureSaver::READ, &refs);
        reader.Add(3, &clock);
        return CObj<CCTime>(clock.GetTime());
    };
    auto worldClock = getClock(worldMixer);
    auto unitClock = getClock(unitMixer);
    Require(worldClock->GetValue() == 0 && unitClock->GetValue() == 0,
        "the render mixers did not start on the world's initial epoch");

    renderer->FastUpdate(1000);
    for (STime time = 1100; time <= 6000; time += 100)
        renderer->FastUpdate(time);
    const auto afterSkip = renderer->GetTime()->GetValue();
    Require(afterSkip == 5000, "the simulation did not advance before its first sound frame");
    unitClock->Set(9000);
    CTransformStack transform;
    transform.Init();
    renderer->UpdateSound(true, &transform, 6000);
    Require(worldClock->GetValue() == afterSkip && unitClock->GetValue() == afterSkip,
        "the first rendered sound frame lost simulated time or retained a stale saved clock");
    renderer->UpdateSound(false, &transform, 7000);
    Require(worldClock->GetValue() == afterSkip && unitClock->GetValue() == afterSkip,
        "a paused sound frame used elapsed wall time");
    world->bNetworkArena = true;
    world->GetAimTime()->Set(9050);
    renderer->UpdateSound(true, &transform, 10000);
    Require(worldClock->GetValue() == 9050 && unitClock->GetValue() == 9050,
        "LAN sound did not follow the aim clock used by world rendering");
}




static void WriteLegacyChapterFixture(const char* databasePath,const char* resourcePath,const char* fixturePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"legacy chapter fixture has no authored map");
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game);
    enter->Exec();
    CObj<NGame::CMissionBase> chapter = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(chapter) && !chapter->GetWorld() && !chapter->GetRenderGame() && !chapter->GetScene(),
        "legacy fixture writer must run with the historical pre-world chapter implementation");
    auto* ui = dynamic_cast<NUI::CChapterMapUI*>(chapter->GetInterface()->GetChildByID("chaptermapUI"));
    Require(ui && !chapter->GetDesktop() && !chapter->GetActivePlayer(),"historical chapter graph has an unexpected shape");
    ui->SetTarget(CVec2(113,227));
    CFileStream snapshot; snapshot.OpenWrite(fixturePath);
    { CStructureSaver writer(snapshot,CStructureSaver::WRITE); writer.Add(2,&game); writer.Add(3,&chapter); }
    NMainLoop::DoneInterface();
}

static void LegacyChapterWorldMigration(const char* databasePath,const char* resourcePath,const char* fixturePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness(),"legacy chapter test started with an active interface");
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CObj<NRPG::CGlobalGame> game;
    CObj<NGame::CMissionBase> chapter;
    CFileStream snapshot; snapshot.OpenRead(fixturePath);
    { CStructureSaver reader(snapshot,CStructureSaver::READ); reader.Add(2,&game); reader.Add(3,&chapter); }
    Require(IsValid(game) && IsValid(chapter) && pSSClasses->GetObjectTypeID(chapter) == int(0xB2301230) &&
        !chapter->GetWorld() && !chapter->GetRenderGame() && !chapter->GetScene(),
        "legacy fixture did not restore the actual historical null-world chapter graph");
    auto* ui = dynamic_cast<NUI::CChapterMapUI*>(chapter->GetInterface()->GetChildByID("chaptermapUI"));
    Require(ui && !chapter->GetDesktop() && !chapter->GetActivePlayer(),"legacy chapter fixture lost its original UI graph");
    auto* interface = chapter->GetInterface(); auto* cursor = chapter->GetCursor();
    const string travel = ui->GetTravelForDiagnostics();
    const int chapterID = game->nChapterMapID, difficulty = game->nCurrentChapterDifficulty;
    const auto mercs = game->players.front()->mercs;
    chapter->OnSnapshotRestored();
    Require(IsValid(chapter->GetWorld()) && IsValid(chapter->GetRenderGame()) && IsValid(chapter->GetScene()),
        "loading a historical chapter save left its native world/render graph missing");
    auto check = [&](NGame::CMissionBase* restored,NRPG::CGlobalGame* global) {
        auto* world = restored->GetWorld(); auto* player = restored->GetActivePlayer();
        auto* mapUI = dynamic_cast<NUI::CChapterMapUI*>(restored->GetInterface()->GetChildByID("chaptermapUI"));
        Require(IsValid(world) && world->GetGlobalGame() == global && IsValid(world->GetAIMap()) && IsValid(world->GetPathNetwork()) &&
            IsValid(player) && player->GetGlobalPlayer() == global->players.front() && IsValid(player->GetCommander()) &&
            IsValid(player->GetPlayer()) && IsValid(player->GetCamera()) && IsValid(restored->GetEnemyTurnCamera()),
            "legacy chapter migration did not rebuild its complete native world/player/camera links");
        Require(mapUI && restored->GetDesktop() == mapUI && mapUI->GetTravelForDiagnostics() == travel &&
            restored->GetInterface()->GetSound() == restored->GetSoundScene(),
            "legacy chapter migration discarded saved travel/UI state or failed to share its new sound scene");
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(restored->GetInterface(),&references);
        bool ownsSound = true;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(23,&ownsSound); }
        Require(!ownsSound,"migrated chapter UI still pumps its old independently clocked sound scene");
        Require(global->bChapterMapSet && global->nChapterMapID == chapterID && global->nCurrentChapterDifficulty == difficulty &&
            global->players.size() == 1 && global->players.front()->mercs.size() == mercs.size(),
            "legacy chapter migration changed campaign destination, difficulty or party size");
    };
    check(chapter,game);
    Require(chapter->GetInterface() == interface && chapter->GetCursor() == cursor && chapter->GetDesktop() == ui &&
        game->players.front()->mercs == mercs,"legacy migration replaced the original UI, cursor or RPG party objects");
    auto* world = chapter->GetWorld();
    const STime initial = world->GetAimTime()->GetValue();
    Require(initial == 0,"legacy migration ran a tactical warm-up");
    world->UpdateWorld(initial+100,nullptr);
    const STime savedTime = world->GetAimTime()->GetValue();
    Require(savedTime > initial,"migrated historical chapter world cannot advance native simulation");
    chapter->OnSnapshotRestored();
    Require(chapter->GetWorld() == world && world->GetAimTime()->GetValue() == savedTime,
        "a second restoration hook recreated the migrated world or reset its simulation");
    CMemoryStream current;
    { CStructureSaver writer(current,CStructureSaver::WRITE); writer.Add(2,&game); writer.Add(3,&chapter); }
    current.Seek(0);
    CObj<NRPG::CGlobalGame> loadedGame; CObj<NGame::CMissionBase> loaded;
    { CStructureSaver reader(current,CStructureSaver::READ); reader.Add(2,&loadedGame); reader.Add(3,&loaded); }
    auto* savedWorld = loaded->GetWorld(); auto* savedRender = loaded->GetRenderGame();
    Require(savedWorld && savedWorld != world && savedWorld->GetAimTime()->GetValue() == savedTime,
        "a migrated chapter did not save a complete independent world graph");
    loaded->OnSnapshotRestored(); loaded->OnSnapshotRestored();
    check(loaded,loadedGame);
    Require(loaded->GetWorld() == savedWorld && loaded->GetRenderGame() == savedRender && savedWorld->GetAimTime()->GetValue() == savedTime,
        "restoring an already complete chapter recreated its world/render or reset the saved clock");
    savedWorld->UpdateWorld(savedTime+50,nullptr);
    Require(savedWorld->GetAimTime()->GetValue() > savedTime,"a complete migrated chapter failed to resume after its next snapshot load");
    // Real focus and rendering clocks must now be safe for both generations.
    chapter->OnLostFocus(); chapter->OnGetFocus(); loaded->OnLostFocus(); loaded->OnGetFocus();
}

class ChapterZoneDrainMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(ChapterZoneDrainMission);
    CPtr<NUI::CDesktopWindow> desktop;
public:
    void Attach(NWorld::CWorld* world,NRPG::CGlobalGame* game,NUI::CDesktopWindow* top) {
        pWorld = world; pGlobalGame = game; desktop = top;
    }
    NUI::CDesktopWindow* GetDesktop() const override { return desktop; }
    void Drain() { ExecWorldCommands(); }
};

class BeginMissionCommandAccess: public NGame::CICBeginMission
{
public:
    static auto Zone() { return &BeginMissionCommandAccess::pZone; }
    static auto Game() { return &BeginMissionCommandAccess::pGlobalGame; }
    static auto Template() { return &BeginMissionCommandAccess::nTemplateID; }
    static auto EmulatesChapter() { return &BeginMissionCommandAccess::bEmulateChapter; }
    static auto Image() { return &BeginMissionCommandAccess::pPWLImage; }
};

class CachedChapterSampleAccess: public NSound::CFileSample2D
{
public:
    static auto ValueMember() { return &CachedChapterSampleAccess::pValue; }
};

class NativeBackdropInterface: public NGame::CRenderBaseInterface
{
    OBJECT_NOCOPY_METHODS(NativeBackdropInterface);
public:
    void Step() override {
        NGame::CRenderBaseInterface::Step();
        if (CanRender()) RenderFrame(GetTime(),GetCamera());
    }
};

static void BackdropScriptAudio(const char* databasePath,const char* resourcePath,bool spatial)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr,"backdrop sound fixture requires headless native audio");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"backdrop sound fixture started with an interface or pending command");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm backdrop sound regression");
    Require(NFMSound::Init(NFMSound::SStartInfo()),"backdrop sound fixture could not initialize native audio");
    cleanup.audio = true; NFMSound::SetMusicMasterVolume(0);
    CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
    Require(sounds.MoveNext(),"backdrop sound fixture has no native sound record");
    auto* record = sounds.Get(); record->bLoop = true; record->nStartSamples = record->nEndingSamples = 0;
    const auto wave = SilentPCMFixture(44100);
    CObj<NFMSound::CSample2D> sample2D = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
    CObj<NFMSound::CSample3D> sample3D = NFMSound::LoadSample3D(wave.data(),static_cast<int>(wave.size()),1.f,100.f,0);
    Require(IsValid(sample2D) && IsValid(sample3D),"backdrop sound fixture could not decode its native PCM samples");
    CObj<NSound::CFileSample2D> loader2D = NSound::share2DSamples.Get(record->GetRecordID());
    CObj<NSound::CFileSample3D> loader3D = NSound::share3DSamples.Get(record->GetRecordID());
    (loader2D.GetPtr()->*CachedChapterSampleAccess::ValueMember()) = sample2D.GetPtr();
    (loader3D.GetPtr()->*CachedEffectSampleAccess::ValueMember()) = sample3D.GetPtr();
    CObj<NDb::CScript> entry = new NDb::CScript;
    const string play = spatial ? "Play3DSound("+std::to_string(record->GetRecordID())+",GetWaypointPos('__native_backdrop_audio__'))" :
        "PlaySound("+std::to_string(record->GetRecordID())+")";
    entry->strCode = "BackdropStarts = (BackdropStarts or 0) + 1; BackdropEnters = 0; "
        "function OnEnterZone() BackdropEnters = BackdropEnters + 1 end; BackdropHandle = "+play+"; BackdropRequested = 1";
    struct VariantRestore {
        vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> records;
        ~VariantRestore() { for (auto& pair: records) if (IsValid(pair.first)) pair.first->pScript = pair.second; }
    } restore;
    CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
    while (variants.MoveNext()) {
        auto* variant = variants.Get();
        if (IsValid(variant->pTemplate) && variant->pTemplate->GetRecordID() == 2425) {
            std::printf("backdrop template2425 variant=%d originalScript=%d\n",variant->GetRecordID(),IsValid(variant->pScript) ? variant->pScript->GetRecordID() : 0);
            restore.records.push_back({variant,variant->pScript}); variant->pScript = entry;
        }
    }
    Require(!restore.records.empty(),"backdrop sound fixture has no authored main-menu map variant");
    CObj<NativeBackdropInterface> backdrop = new NativeBackdropInterface;
    backdrop->Initialize(2425);
    auto* placement = NDb::GetDBCamera(26);
    Require(placement != nullptr,"backdrop sound fixture has no authored menu camera");
    backdrop->GetCamera()->SetPlacement(ICamera::SCameraPos(placement->vAnchor,placement->fDistance,placement->fPitch,placement->fYaw,placement->fRoll,placement->fFOV));
    backdrop->GetCamera()->SetScreenRect(CTRect<float>(0.f,0.f,1.f,1.f));
    class InstallBackdrop: public NMainLoop::CInterfaceCommand {
        OBJECT_NOCOPY_METHODS(InstallBackdrop);
    public:
        CPtr<NMainLoop::IInterfaceBase> owner;
        void Exec() override { SetInterface(owner); }
    };
    CObj<InstallBackdrop> install = new InstallBackdrop; install->owner = backdrop; install->Exec();
    auto* world = dynamic_cast<NWorld::CWorld*>(backdrop->GetWorld());
    auto* script = world ? world->GetOwnScript() : nullptr;
    Require(IsValid(script),"backdrop sound fixture has no actual initialized world script");
    auto number = [&](const char* name,int expected) {
        const int top = script->GetTop(); auto value = script->GetGlobal(name);
        const bool matches = value.IsNumber() && value.GetNumber() == expected; script->SetTop(top); return matches;
    };
    Require(number("BackdropRequested",1) && number("BackdropStarts",1),
        "the existing native backdrop initializer never ran its map-variant script exactly once");
    const int top = script->GetTop(); auto handle = script->GetGlobal("BackdropHandle");
    auto* command = handle.IsUserData() ? dynamic_cast<NWorld::CUICmdPlaySound*>(handle.GetUserData()) : nullptr;
    Require(command != nullptr,"the backdrop variant script did not create its native Lua sound command");
    script->SetTop(top);
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(1000)};
    auto frame = [&](STime time,bool active) {
        NMainLoop::SetTimeForHarness(time);
        Require(NMainLoop::StepApp(active,false,false),"native backdrop frame requested application exit");
        Require(NMainLoop::GetCurrentInterfaceForHarness() == backdrop.GetPtr() && !NMainLoop::HaveInterfaceCommand(),
            "backdrop sound fixture was covered by an unrelated interface");
    };
    frame(1000,false);
    Require(!IsValid(command->pChannel),"inactive backdrop frames consumed a queued script sound");
    frame(1050,true);
    Require(IsValid(command->pChannel),"the active native backdrop ignored its initialized map script's sound command");
    auto channelExcept = [&](int except) {
        int found = -1;
        for (int i = 0; i < 128; ++i) if (i != except && FSOUND_IsPlaying(i)) {
            Require(found == -1,"backdrop sound fixture has an unexpected extra native channel"); found = i;
        }
        return found;
    };
    const int played = channelExcept(-1);
    Require(played >= 0 && FSOUND_GetCurrentPosition(played) == 0,"backdrop dispatch did not start its real native PCM channel at offset zero");
    vector<float> output(512*2);
    auto render = [&]() { Require(S2FmodRenderForTest(output.data(),512) != 0,"backdrop sound fixture could not render native PCM"); };
    render();
    Require(FSOUND_GetCurrentPosition(played) > 0,"backdrop sound never advanced native PCM playback");
    CObj<NFMSound::CSound2D> independent = NFMSound::PlaySound(sample2D);
    const int control = channelExcept(played);
    Require(IsValid(independent) && control >= 0,"backdrop sound fixture could not start its independent control");
    Require(script->DoString("StopSound(BackdropHandle); BackdropStopped = 1") == 0,"backdrop sound fixture could not queue its native Lua stop");
    for (int tick = 0; tick < 8; ++tick) frame(1100+tick*50,true);
    render();
    Require(number("BackdropStopped",1) && !FSOUND_IsPlaying(played) && FSOUND_IsPlaying(control),
        "the native backdrop ignored Lua StopSound or interrupted an unrelated channel");
    Require(script->DoString(("BackdropHandle = "+play+"; BackdropRestarted = 1").c_str()) == 0,"backdrop sound fixture could not queue its native Lua restart");
    for (int tick = 0; tick < 8; ++tick) frame(1500+tick*50,true);
    const int restarted = channelExcept(control);
    Require(number("BackdropRestarted",1) && restarted >= 0 && number("BackdropStarts",1) && number("BackdropEnters",1),
        "the native backdrop failed to restart audio or repeated initialization/OnEnterZone");
    render();
    Require(FSOUND_GetCurrentPosition(restarted) > 0,"restarted backdrop sound did not advance native PCM");
    NMainLoop::DoneInterface(); backdrop = nullptr; ClearHoldQueue(); render();
    Require(!FSOUND_IsPlaying(restarted) && FSOUND_IsPlaying(control),"backdrop teardown leaked its sound or stopped the independent control");
#else
    Require(false,"backdrop sound test requires the native SFX backend");
#endif
}

static void InGameSaveButton(const char* databasePath,const char* resourcePath,bool shortcut = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"in-game save button fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"in-game save button fixture started with an active interface");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm in-game save permission regression");
    NMainLoop::CreateProfile("UpstreamInGameSave"); NMainLoop::SetActiveProfile("UpstreamInGameSave");
    CObj<NRPG::CGlobalPlayer> player = NRPG::CreateGlobalPlayer(vector<int>{54});
    player->pSide = NDb::GetDBSide(1);
    Require(IsValid(player->pSide),"in-game save button fixture has no authored player side");
    NInput::CBind saveBind("savegame"), loadBind("loadgame");
    auto click = [&](NUI::CWindow* control) {
        NUI::SPoint position; NUI::SRect rect;
        Require(IsValid(control) && control->ClientToScreen(&position,&rect) && rect.x2 > rect.x1 && rect.y2 > rect.y1,
            "in-game menu button has no native hit-test rectangle");
        const int x = (rect.x1+rect.x2)/2, y = (rect.y1+rect.y2)/2;
        Require(control->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONDOWN,x,y)) && control->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONUP,x,y)),
            "in-game menu fixture could not click its native button");
    };
    for (bool allowed : {false,true}) {
        CObj<NGame::CICInGameMenu> open = new NGame::CICInGameMenu(player,false,allowed); open->Exec();
        auto* active = NMainLoop::GetCurrentInterfaceForHarness();
        Require(IsValid(active) && pSSClasses->GetObjectTypeID(active) == int(0xB0905130) && NMainLoop::StepApp(true,false,false),
            "in-game save button fixture did not open and draw its actual native menu");
        CStructureNetworkContext references; auto fields = BorrowedObjectFields(active,&references);
        bool menuPermission = !allowed; CPtr<NUI::CWindow> page;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(2,&menuPermission); reader.Add(9,&page); }
        Require(menuPermission == allowed && IsValid(page),"in-game menu lost its incoming save permission");
        auto* save = dynamic_cast<NUI::CButton*>(page->GetChildByID("savegame"));
        auto* load = dynamic_cast<NUI::CButton*>(page->GetChildByID("loadgame"));
        auto* restart = page->GetChildByID("restart");
        Require(IsValid(save) && IsValid(load) && IsValid(restart) && load->GetStyle(NUI::STYLE_ENABLED) && !restart->GetStyle(NUI::STYLE_ENABLED),
            "in-game save button fixture has no independent Load/Restart controls");
        Require(save->GetStyle(NUI::STYLE_ENABLED) == allowed,"the in-game Save button ignored its explicit save permission");
        CStructureNetworkContext pageReferences; auto pageFields = BorrowedObjectFields(page,&pageReferences);
        bool pagePermission = !allowed, pageRestart = true; CPtr<NRPG::CGlobalPlayer> savedPlayer; CPtr<NUI::CWindow> savedButton;
        { CStructureSaver reader(pageFields,CStructureSaver::READ,&pageReferences);
          reader.Add(2,&pagePermission); reader.Add(3,&pageRestart); reader.Add(4,&savedPlayer); reader.Add(8,&savedButton); }
        Require(pagePermission == allowed && !pageRestart && savedPlayer.GetPtr() == player.GetPtr() && savedButton.GetPtr() == save,
            "in-game menu changed its existing saved permission or button fields");
        NInput::SEvent event; while (NInput::GetEvent(&event)) {}
        if (shortcut) {
            NInput::PostEvent("savegame");
            Require(NInput::GetEvent(&event) && saveBind.ProcessEvent(event),"in-game save shortcut fixture did not receive its actual bound command");
        } else if (!allowed) {
            click(save);
            Require(!NInput::GetEvent(&event) && !NMainLoop::HaveInterfaceCommand(),"a disabled in-game Save button still emitted a save action");
            click(load);
            Require(NInput::GetEvent(&event) && loadBind.ProcessEvent(event),"disabling in-game Save also disabled the real Load button");
        } else {
            click(save);
            Require(NInput::GetEvent(&event) && saveBind.ProcessEvent(event),"an explicitly permitted in-game Save button did not emit its action");
        }
        Require(active->ProcessEvent(event) && NMainLoop::HaveInterfaceCommand(),"the in-game menu did not dispatch its actual save/load button action");
        CObj<NMainLoop::CInterfaceCommand> command = NMainLoop::TakeInterfaceCommandForHarness();
        Require(dynamic_cast<NGame::CICSaveLoadMenu*>(command.GetPtr()) != nullptr,"the in-game button queued an unrelated command");
        command->Exec();
        CStructureNetworkContext dialogReferences; auto dialog = BorrowedObjectFields(NMainLoop::GetCurrentInterfaceForHarness(),&dialogReferences);
        bool dialogPermission = !allowed; int mode = -1; CPtr<NUI::CWindow> dialogPage;
        { CStructureSaver reader(dialog,CStructureSaver::READ,&dialogReferences); reader.Add(2,&dialogPermission); reader.Add(3,&mode); reader.Add(6,&dialogPage); }
        Require(IsValid(dialogPage),"in-game save/load command did not create its native dialog page");
        CStructureNetworkContext viewReferences; auto viewFields = BorrowedObjectFields(dialogPage,&viewReferences);
        CPtr<NUI::CWindow> view;
        { CStructureSaver reader(viewFields,CStructureSaver::READ,&viewReferences); reader.Add(4,&view); }
        Require(IsValid(view),"a forbidden in-game Save shortcut opened an empty native save/load page");
        Require(dialogPermission == allowed && mode == (allowed ? NGame::SAVE : NGame::LOAD),"the in-game button lost its save/load destination or permission");
        Require(pSSClasses->GetObjectTypeID(view) == int(allowed ? 0xB1014135 : 0xB1014134),"the in-game command created the wrong native save/load view");
        NMainLoop::DoneInterface();
    }
    std::printf("actual in-game Save permission false/true, shortcut=%d, native save/load views and saved UI fields verified\n",int(shortcut));
}

static void LuaUnusedStackInitialization()
{
    alignas(TObject) unsigned char storage[sizeof(TObject)];
    std::memset(storage,0xa5,sizeof(storage));
    TObject* fresh = new(storage) TObject;
    Require(fresh->GetType() == LUA_TNIL,"a native Lua value created over dirty storage did not start as nil");
    fresh->~TObject();
    CObj<CLuaThread> thread = new CLuaThread("unused stack initialization audit");
    thread->stack.resize(thread->stack.size()+37);
    for (const auto& value : thread->stack) Require(value.GetType() == LUA_TNIL,"new or expanded native Lua stack slots have invalid tags");
    thread->top = 2; thread->stack[0].SetN(19); thread->stack[1].SetN(23);
    // A popped slot can retain an already collected string. The existing
    // thread saver must clear spare slots before serializing the whole vector.
    thread->stack[5].SetS(reinterpret_cast<TString*>(std::uintptr_t(1)));
    CMemoryStream wire;
    { CStructureSaver writer(wire,CStructureSaver::WRITE); writer.Add(2,&thread); }
    Require(thread->stack[5].GetType() == LUA_TNIL,"saving a Lua thread dereferenced or retained a stale spare stack slot");
    wire.Seek(0); CObj<CLuaThread> restored;
    { CStructureSaver reader(wire,CStructureSaver::READ); reader.Add(2,&restored); }
    Require(IsValid(restored) && restored.GetPtr() != thread.GetPtr() && restored->top == 2 && restored->stack.size() == thread->stack.size() &&
        restored->stack[0].GetType() == LUA_TNUMBER && restored->stack[0].GetN() == 19 && restored->stack[1].GetType() == LUA_TNUMBER && restored->stack[1].GetN() == 23,
        "native Lua thread serialization lost its live numeric slots");
    for (std::size_t i = restored->top; i < restored->stack.size(); ++i)
        Require(restored->stack[i].GetType() == LUA_TNIL,"native Lua thread serialization restored an invalid spare slot");
    std::printf("existing native Lua nil constructor and %zu serialized spare slots verified\n",restored->stack.size()-restored->top);
}


// Exercise the real interface commands and native scene/music state machine.
// Only the database audio payloads are replaced with deterministic PCM fixtures.
struct NativeMenuMusicState
{
    CDGPtr<CFuncBase<STime>> clock;
    CPtr<CObjectBase> music;
    CDBPtr<NDb::CTMusic> ambient,combat;
    NDb::EMusicType current=NDb::MT_PRECOMBAT,next=NDb::MT_PRECOMBAT;
    __int64 start=-1,stop=-1;
    int random=0;
};

static NativeMenuMusicState MenuMusicState(NSound::ISoundScene* scene)
{
    Require(scene!=nullptr,"menu music observer has no real sound scene");
    CStructureNetworkContext references;auto fields=BorrowedObjectFields(scene,&references);
    NativeMenuMusicState state;
    { CStructureSaver reader(fields,CStructureSaver::READ,&references);
      reader.Add(2,&state.start);reader.Add(4,&state.clock);reader.Add(5,&state.music);
      reader.Add(6,&state.ambient);reader.Add(7,&state.combat);reader.Add(8,&state.current);
      reader.Add(9,&state.next);reader.Add(10,&state.stop);reader.Add(11,&state.random); }
    return state;
}

static NDb::CMusic* MenuMusicRecord(const NativeMenuMusicState& state)
{
    if(!IsValid(state.music))return nullptr;
    CStructureNetworkContext references;auto fields=BorrowedObjectFields(state.music,&references);
    CDBPtr<NDb::CMusic> record;
    {CStructureSaver reader(fields,CStructureSaver::READ,&references);reader.Add(2,&record);}
    return record;
}

static void MenuMusicPools(const char* databasePath,const char* resourcePath,int origin)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS") && std::getenv("S2_USER_DATA_DIR"),"menu music fixture needs isolated headless native audio");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"menu music fixture started with an existing interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm contextual menu music regression");
    Require(NFMSound::Init(NFMSound::SStartInfo()),"menu music fixture could not initialize native audio");cleanup.audio=true;
    TemporaryAudioFile ambientFile("menu-pool-ambient.wav"),combatFile("menu-pool-combat.wav");
    CDBIterator<NDb::CMusic> records(*NDatabase::GetTable<NDb::CMusic>());
    Require(records.MoveNext(),"menu music fixture has no ambient record");CDBPtr<NDb::CMusic> ambient=records.Get();
    Require(records.MoveNext(),"menu music fixture has no distinct combat record");CDBPtr<NDb::CMusic> combat=records.Get();
    ambient->szFileName=ambientFile.path.string();combat->szFileName=combatFile.path.string();
    for(auto record:{ambient.GetPtr(),combat.GetPtr()}) {
        record->nFadeIn=0;record->nFadeOut=0;record->nPlayTime=60000;record->nRndPlayTime=0;
        record->nSilence=0;record->nRndSilence=0;
    }
    CDBIterator<NDb::CTMusic> pools(*NDatabase::GetTable<NDb::CTMusic>());
    Require(pools.MoveNext(),"menu music fixture has no registered ambient pool");CDBPtr<NDb::CTMusic> ambientPool=pools.Get();
    Require(pools.MoveNext(),"menu music fixture has no registered combat pool");CDBPtr<NDb::CTMusic> combatPool=pools.Get();
    ambientPool->variants.clear();ambientPool->variants.push_back(ambient.GetPtr());ambientPool->roulette=CRoulette();ambientPool->roulette.AddSector(1);
    combatPool->variants.clear();combatPool->variants.push_back(combat.GetPtr());combatPool->roulette=CRoulette();combatPool->roulette.AddSector(1);
    CObj<NRPG::CGlobalGame> game;
    if(origin==0) {CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();}
    else if(origin==1) {
        game=NRPG::CreateGlobalGame(-1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        CObj<NGame::CICBeginMission> open=new NGame::CICBeginMission(810,-1,vector<string>(),game);open->Exec();
        CObj<NMainLoop::CInterfaceCommand> snapshot=NMainLoop::TakeInterfaceCommandForHarness();
        Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"menu music fixture lost its mission-start snapshot command");
        // This test covers menu audio, not disk saves; consume the unrelated command.
    } else if(origin==2) {
        CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());NDb::CChapterMap* map=nullptr;
        while(maps.MoveNext())if(IsValid(maps.Get()->pBackground)){map=maps.Get();break;}
        Require(map!=nullptr,"menu music fixture has no authored chapter");
        game=NRPG::CreateGlobalGame(1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        CObj<NGame::CICBeginChapter> open=new NGame::CICBeginChapter(map->GetRecordID(),game);open->Exec();
    } else if(origin==3) {
        CDBIterator<NDb::CGlobalMap> maps(*NDatabase::GetTable<NDb::CGlobalMap>());NDb::CGlobalMap* map=nullptr;
        while(maps.MoveNext())if(IsValid(maps.Get()->pScenario)){map=maps.Get();break;}
        Require(map!=nullptr,"menu music fixture has no authored global map");
        game=NRPG::CreateGlobalGame(map->pScenario->GetRecordID());game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        game->bGlobalMapSet=true;game->nGlobalMapID=map->GetRecordID();
        CObj<NGame::CICContinueGlobal> open=new NGame::CICContinueGlobal(game);open->Exec();
    } else {
        Require(origin==4,"unknown menu music fixture origin");
        CObj<NGame::CICOptions> open=new NGame::CICOptions;open->Exec();
    }
    CObj<NGame::CMissionBase> mission=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    CObj<NSound::ISoundScene> source=mission?mission->GetSoundScene():nullptr;
    NativeMenuMusicState before;
    CObj<NFMSound::CStream> sourceStream;
    if(origin!=4) {
        Require(IsValid(mission) && IsValid(source) && !NMainLoop::HaveInterfaceCommand(),"menu music fixture did not create its actual origin/scene");
        CMemoryStream fields;
        {CStructureSaver writer(fields,CStructureSaver::WRITE);writer.Add(6,&ambientPool);writer.Add(7,&combatPool);}
        fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ);static_cast<CObjectBase*>(source.GetPtr())->operator&(reader);}
        source->SetMusic(NDb::MT_COMBAT);
        before=MenuMusicState(source);
        Require(before.ambient.GetPtr()==ambientPool.GetPtr() && before.combat.GetPtr()==combatPool.GetPtr() &&
            before.current==NDb::MT_COMBAT && before.next==NDb::MT_COMBAT && MenuMusicRecord(before)==combat.GetPtr(),
            "source positive control did not start actual combat music with registered pools");
        sourceStream=NFMSound::PlayStream(combat->szFileName.c_str(),true,0,true,0);
        Require(IsValid(sourceStream) && NFMSound::IsPlaying(sourceStream),"source combat music never reached the native stream backend");
    }
    auto event=[&](const char* name) {
        NInput::SEvent input;while(NInput::GetEvent(&input)){}
        NInput::PostEvent(name);
        Require(NInput::GetEvent(&input),"menu music fixture did not receive its posted native binding");
        // Options queues its two commands and returns false; inspect the queue itself.
        NMainLoop::GetCurrentInterfaceForHarness()->ProcessEvent(input);
        int count=0;
        while(NMainLoop::HaveInterfaceCommand()) {
            Require(++count<=2,"menu binding queued extra commands");
            CObj<NMainLoop::CInterfaceCommand> command=NMainLoop::TakeInterfaceCommandForHarness();command->Exec();
        }
        Require(count>0,"menu binding did not queue its actual interface command");
    };
    int errors=0,observed=0;
    CObj<NUI::CInterface> previousUI;
    auto inspect=[&](bool options,const char* label) {
        auto* active=NMainLoop::GetCurrentInterfaceForHarness();
        Require(active && (!mission || active!=mission.GetPtr()),"menu command did not cover the actual origin");
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(active,&refs);
        CPtr<NUI::CInterface> ui;CPtr<NGame::IMission> context;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(options?4:7,&ui);reader.Add(options?2:4,&context);}
        Require(IsValid(ui) && IsValid(ui->GetSound()) && (!previousUI || previousUI.GetPtr()!=ui.GetPtr()),"menu reused its previous UI instead of constructing the actual page");
        previousUI=ui.GetPtr();
        CStructureNetworkContext uiRefs;auto uiFields=BorrowedObjectFields(ui,&uiRefs);bool owns=false;
        {CStructureSaver reader(uiFields,CStructureSaver::READ,&uiRefs);reader.Add(23,&owns);}
        const auto initial=MenuMusicState(ui->GetSound());
        Require(owns && IsValid(initial.clock) && ui->GetSound()!=source.GetPtr() &&
            (!source || initial.clock.GetPtr()!=before.clock.GetPtr()) && !IsValid(initial.music) &&
            initial.current==NDb::MT_AMBIENT && initial.next==NDb::MT_AMBIENT && initial.stop==0,
            "menu copied a live source scene/clock/playback state instead of owning a fresh scene");
        const bool inherited=initial.ambient.GetPtr()==(source?ambientPool.GetPtr():nullptr) && initial.combat.GetPtr()==(source?combatPool.GetPtr():nullptr);
        const bool contextual=context.GetPtr()==mission.GetPtr();
        MarkNewDGFrame();ui->Draw(1000+observed*100);
        const auto playing=MenuMusicState(ui->GetSound());
        const bool started=MenuMusicRecord(playing)==(source?ambient.GetPtr():nullptr);
        bool combatStarted=true;
        if(source) {
            ui->GetSound()->SetMusic(NDb::MT_COMBAT);
            combatStarted=MenuMusicRecord(MenuMusicState(ui->GetSound()))==combat.GetPtr();
        }
        std::printf("native menu music origin=%d page=%s ambient=%d combat=%d context=%d inherited=%d started=%d combatStarted=%d own=%d\n",
            origin,label,initial.ambient?initial.ambient->GetRecordID():0,initial.combat?initial.combat->GetRecordID():0,
            int(contextual),int(inherited),int(started),int(combatStarted),int(owns));std::fflush(stdout);
        errors+=!inherited+!contextual+!started+!combatStarted;++observed;
        if(source) {
            const auto after=MenuMusicState(source);
            Require(after.clock.GetPtr()==before.clock.GetPtr() && after.music.GetPtr()==before.music.GetPtr() &&
                after.ambient.GetPtr()==before.ambient.GetPtr() && after.combat.GetPtr()==before.combat.GetPtr() &&
                after.current==before.current && after.next==before.next && after.start==before.start && after.stop==before.stop && after.random==before.random,
                "menu changed source scene pools, deadlines, random state or live music ownership");
        }
    };
    if(origin!=4) {
        event(origin==0?"options":"gamemenu");inspect(origin==0,origin==0?"profiles":"pause");
        if(origin!=0){event("options");inspect(true,"profiles");}
    } else inspect(true,"profiles");
    for(const char* page:{"video","audio","profiles","gameplay","controls"}) {event(page);inspect(true,page);}
    event("cancel");
    if(origin>0 && origin<4)event("cancel");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==mission.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"closing menus did not restore the original interface stack");
    std::printf("native menu music origin=%d checked=%d failures=%d\n",origin,observed,errors);std::fflush(stdout);
    Require(errors==0,"contextual menus lost source music pools/context and could not start their own ambient/combat tracks");
#else
    Require(false,"menu music fixture requires native audio");
#endif
}


static void BackdropUIAudioOwnership(const char* databasePath,const char* resourcePath)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS"),"backdrop UI ownership fixture requires headless native audio");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"backdrop UI fixture started with an existing interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm backdrop UI audio ownership regression");
    Require(NFMSound::Init(NFMSound::SStartInfo()),"backdrop UI fixture could not initialize native audio");cleanup.audio=true;
    NFMSound::SetMusicMasterVolume(0);
    CDBIterator<NDb::CSound> records(*NDatabase::GetTable<NDb::CSound>());
    Require(records.MoveNext(),"backdrop UI fixture has no world sound");CDBPtr<NDb::CSound> worldRecord=records.Get();
    Require(records.MoveNext(),"backdrop UI fixture has no UI sound");CDBPtr<NDb::CSound> uiRecord=records.Get();
    worldRecord->bLoop=true;uiRecord->bLoop=false;
    worldRecord->nStartSamples=worldRecord->nEndingSamples=uiRecord->nStartSamples=uiRecord->nEndingSamples=0;
    auto wave=SilentPCMFixture(44100);
    for(std::size_t i=44;i<wave.size();i+=2){wave[i]=0xa0;wave[i+1]=0x0f;}
    CObj<NFMSound::CSample2D> sample=NFMSound::LoadSample2D(wave.data(),int(wave.size()));
    Require(IsValid(sample),"backdrop UI fixture could not decode its non-silent PCM");
    CObj<NSound::CFileSample2D> worldLoader=NSound::share2DSamples.Get(worldRecord->GetRecordID());
    CObj<NSound::CFileSample2D> uiLoader=NSound::share2DSamples.Get(uiRecord->GetRecordID());
    (worldLoader.GetPtr()->*CachedChapterSampleAccess::ValueMember())=sample.GetPtr();
    (uiLoader.GetPtr()->*CachedChapterSampleAccess::ValueMember())=sample.GetPtr();
    CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    CObj<NGame::CMissionBase> menu=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(menu) && menu->GetWorld() && menu->GetRenderGame() && menu->GetInterface() && menu->GetSoundScene(),"backdrop UI fixture did not open the actual world-backed main menu");
    auto* ui=menu->GetInterface();auto* scene=menu->GetSoundScene();
    CTransformStack transform;transform.Init();
    auto channelExcept=[&](int except,int other=-1){int result=-1;
        for(int i=0;i<128;++i)if(i!=except && i!=other && FSOUND_IsPlaying(i)) {Require(result==-1,"backdrop UI fixture has an unexpected additional channel");result=i;}
        return result;};
    CObj<NSound::ISound2D> world=scene->Add2DSound(worldRecord);
    MarkNewDGFrame();scene->Draw(&transform);
    const int worldChannel=channelExcept(-1);Require(worldChannel>=0 && world->IsPlaying(),"world audio positive control never started");
    // Use the actual CWindow path, not a substitute UI sound dispatcher.
    ui->PlaySound(uiRecord);
    MarkNewDGFrame();scene->Draw(&transform);ui->Draw(1000);
    const int uiChannel=channelExcept(worldChannel);Require(uiChannel>=0,"real main-menu CWindow::PlaySound never started its UI channel");
    CObj<NFMSound::CSound2D> independent=NFMSound::PlaySound(sample);
    const int control=channelExcept(worldChannel,uiChannel);Require(IsValid(independent) && control>=0,"backdrop UI fixture has no independent native mixer control");
    vector<float> output(512*2);
    auto render=[&](){Require(S2FmodRenderForTest(output.data(),512)!=0 && std::any_of(output.begin(),output.end(),[](float v){return std::abs(v)>0.01f;}),"backdrop UI fixture did not render actual non-silent PCM");};
    render();
    const auto worldBefore=FSOUND_GetCurrentPosition(worldChannel),uiBefore=FSOUND_GetCurrentPosition(uiChannel),controlBefore=FSOUND_GetCurrentPosition(control);
    Require(worldBefore>0 && uiBefore>0 && controlBefore>0,"backdrop UI baseline channels did not advance");
    auto event=[&](const char* name){NInput::SEvent input;while(NInput::GetEvent(&input)){}NInput::PostEvent(name);
        Require(NInput::GetEvent(&input),"backdrop UI fixture lost its native binding");
        NMainLoop::GetCurrentInterfaceForHarness()->ProcessEvent(input);
        CObj<NMainLoop::CInterfaceCommand> command=NMainLoop::TakeInterfaceCommandForHarness();
        Require(IsValid(command) && !NMainLoop::HaveInterfaceCommand(),"backdrop UI binding did not queue exactly one native interface command");command->Exec();};
    event("options");Require(NMainLoop::GetInterfaceStackDepth()==2 && NMainLoop::GetCurrentInterfaceForHarness()!=menu.GetPtr(),"main menu did not open its real modal options");
    render();
    const auto worldCovered=FSOUND_GetCurrentPosition(worldChannel),uiCovered=FSOUND_GetCurrentPosition(uiChannel),controlCovered=FSOUND_GetCurrentPosition(control);
    Require(FSOUND_IsPlaying(worldChannel) && worldCovered==worldBefore && controlCovered>controlBefore,"native focus failed to pause only the backdrop world or the independent mixer stopped");
    std::printf("native backdrop UI covered: sharedScene=%d world=%u->%u UI=%u->%u control=%u->%u\n",int(ui->GetSound()==scene),worldBefore,worldCovered,uiBefore,uiCovered,controlBefore,controlCovered);std::fflush(stdout);
    Require(FSOUND_IsPlaying(uiChannel) && uiCovered>uiBefore,"opening options froze an already-playing UI sound with the backdrop world");
    CStructureNetworkContext references;auto fields=BorrowedObjectFields(ui,&references);bool owns=false;
    {CStructureSaver reader(fields,CStructureSaver::READ,&references);reader.Add(23,&owns);}
    const auto uiState=MenuMusicState(ui->GetSound()),worldState=MenuMusicState(scene);
    Require(owns && ui->GetSound()!=scene && uiState.clock.GetPtr()!=worldState.clock.GetPtr() && !uiState.ambient && !uiState.combat,"backdrop UI does not own a separate pool-less clock-backed scene");
    event("cancel");Require(NMainLoop::GetCurrentInterfaceForHarness()==menu.GetPtr() && NMainLoop::GetInterfaceStackDepth()==1,"closing options lost the original main menu");
    render();Require(FSOUND_GetCurrentPosition(worldChannel)>worldCovered && FSOUND_GetCurrentPosition(uiChannel)>uiCovered,"returning to the menu did not resume world audio and preserve UI playback");
#else
    Require(false,"backdrop UI ownership fixture requires native audio");
#endif
}

static void AudioOptionCheckboxes(const char* databasePath,const char* resourcePath,bool notificationOnly = false,bool resetDefaults = false)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_USER_DATA_DIR") && std::getenv("S2_AUDIO_HEADLESS"),"audio options fixture requires isolated user data and native headless audio");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"audio options fixture started with an active interface");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm audio option regression");
    Require(NFMSound::Init(NFMSound::SStartInfo()),"audio options fixture could not initialize native audio");
    cleanup.audio = true;
    const char* settings[] = {"sound_sfxvolume","sound_musicvolume","ui_charresponses","ui_charresponsessubtitles"};
    float defaults[4]{};
    if (resetDefaults) for (int i = 0; i < 4; ++i) {
        NGlobal::ResetVar(settings[i]); defaults[i] = NGlobal::GetVar(settings[i]).GetFloat();
    }
    NGlobal::SetVar("sound_sfxvolume",0.23f); NGlobal::SetVar("sound_musicvolume",0.61f);
    NGlobal::SetVar("ui_charresponses",resetDefaults ? 0.f : 1.f); NGlobal::SetVar("ui_charresponsessubtitles",0.f);
    const auto wave = SilentPCMFixture(44100);
    CObj<NFMSound::CSample2D> sample = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
    Require(IsValid(sample),"audio options fixture could not decode its native control sample");
    CObj<NFMSound::CSound2D> playing = NFMSound::PlaySound(sample,0,0,0,true);
    Require(IsValid(playing),"audio options fixture could not start its independent native sound");
    int channel = -1;
    for (int i = 0; i < 128; ++i) if (FSOUND_IsPlaying(i)) {
        Require(channel == -1,"audio options fixture started with unexpected sound channels"); channel = i;
    }
    Require(channel >= 0,"audio options fixture has no actual playing native channel");
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(1,1); pixels[0][0] = NGfx::SPixel8888(11,22,33,255); texture->Set(pixels);
    CObj<NGame::CICOptions> open = new NGame::CICOptions(NGame::OS_AUDIO,texture); open->Exec();
    auto* active = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(active),"audio options fixture did not open the actual native page");
    Require(NMainLoop::StepApp(true,false,false) && NMainLoop::GetCurrentInterfaceForHarness() == active && !NMainLoop::HaveInterfaceCommand(),
        "audio options fixture did not render its active native interface");
    CStructureNetworkContext interfaceReferences;
    auto interfaceFields = BorrowedObjectFields(active,&interfaceReferences);
    CPtr<NUI::CWindow> page;
    { CStructureSaver reader(interfaceFields,CStructureSaver::READ,&interfaceReferences); reader.Add(6,&page); }
    Require(IsValid(page) && pSSClasses->GetObjectTypeID(page) == int(0xB0815152),"audio options fixture did not retain the authored audio page");
    auto* responses = dynamic_cast<NUI::CCheckButton*>(page->GetChildByID("char_responses"));
    auto* subtitles = dynamic_cast<NUI::CCheckButton*>(page->GetChildByID("char_subtitles"));
    auto* sfx = dynamic_cast<NUI::CScroll*>(page->GetChildByID("sound_volume"));
    auto* music = dynamic_cast<NUI::CScroll*>(page->GetChildByID("music_volume"));
    Require(IsValid(responses) && IsValid(subtitles) && IsValid(sfx) && IsValid(music),"authored native audio options lack their real controls");
    CStructureNetworkContext pageReferences;
    auto pageFields = BorrowedObjectFields(page,&pageReferences);
    CPtr<NUI::CWindow> legacyApply, legacyDefault, legacySfx, legacyMusic, legacyOutput;
    { CStructureSaver reader(pageFields,CStructureSaver::READ,&pageReferences);
      reader.Add(2,&legacyApply); reader.Add(3,&legacyDefault); reader.Add(4,&legacySfx); reader.Add(5,&legacyMusic); reader.Add(6,&legacyOutput); }
    Require(IsValid(legacyApply) && legacyDefault.GetPtr() == page->GetChildByID("default") && legacySfx.GetPtr() == sfx && legacyMusic.GetPtr() == music && IsValid(legacyOutput),
        "audio option changes reinterpreted the fork's existing serialized field tags");
    Require(!legacyApply->GetStyle(NUI::STYLE_VISIBLE) && !legacyOutput->GetStyle(NUI::STYLE_VISIBLE) &&
        legacyApply->GetSize().x == 0 && legacyApply->GetSize().y == 0 && legacyOutput->GetSize().x == 0 && legacyOutput->GetSize().y == 0,
        "legacy audio option fields introduced active Apply/output controls into the authored page");
    auto volumes = [&]() {
        Require(fabs(NGlobal::GetVar("sound_sfxvolume").GetFloat()-0.23f) < 1e-6f && fabs(NGlobal::GetVar("sound_musicvolume").GetFloat()-0.61f) < 1e-6f &&
            sfx->GetMaxValue() == 100 && sfx->GetValue() == 23 && music->GetMaxValue() == 100 && music->GetValue() == 61,
            "audio checkbox initialization or notification changed the existing normalized volume controls");
    };
    volumes();
    if (notificationOnly) { responses->SetChecked(true); subtitles->SetChecked(false); }
    else Require(responses->IsChecked() == !resetDefaults && !subtitles->IsChecked(),"the native audio page did not initialize its acknowledgement checkboxes from config");
    auto click = [&](NUI::CWindow* control) {
        Require(IsValid(control),"audio options fixture lacks the requested button");
        NUI::SPoint position; NUI::SRect rect;
        Require(control->ClientToScreen(&position,&rect) && rect.x2 > rect.x1 && rect.y2 > rect.y1,"audio option button has no native hit-test rectangle");
        const int x = (rect.x1+rect.x2)/2, y = (rect.y1+rect.y2)/2;
        Require(control->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONDOWN,x,y)) && control->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONUP,x,y)),
            "audio options fixture could not click its real button");
    };
    if (resetDefaults) {
        Require(defaults[0] != 0.23f && defaults[1] != 0.61f && defaults[2] == 1.f && defaults[3] == 1.f,
            "audio defaults fixture did not start with independently changed settings");
        vector<float> output(512*2);
        Require(S2FmodRenderForTest(output.data(),512) != 0 && FSOUND_GetCurrentPosition(channel) > 0,
            "audio defaults fixture could not advance its existing native channel");
        const auto before = FSOUND_GetCurrentPosition(channel);
        auto restored = [&]() {
            for (int i = 0; i < 4; ++i) Require(fabs(NGlobal::GetVar(settings[i]).GetFloat()-defaults[i]) < 1e-6f,
                "the native audio Default button did not restore registered settings");
            Require(sfx->GetValue() == int(defaults[0]*100) && music->GetValue() == int(defaults[1]*100) && responses->IsChecked() && subtitles->IsChecked(),
                "the native audio Default button did not refresh its actual controls");
        };
        click(page->GetChildByID("default")); restored();
        Require(FSOUND_IsPlaying(channel) && S2FmodRenderForTest(output.data(),512) != 0 && FSOUND_GetCurrentPosition(channel) > before,
            "resetting native audio settings stopped or restarted an existing sound");
        for (const auto* setting : settings) NGlobal::SetVar(setting,0.f);
        NGlobal::LoadConfig(".\\cfg\\config.cfg"); restored();
        click(sfx->GetChildByID("plus"));
        Require(sfx->GetValue() == int(defaults[0]*100)+1 && fabs(NGlobal::GetVar("sound_sfxvolume").GetFloat()-(defaults[0]+0.01f)) < 1e-6f &&
            fabs(NGlobal::GetVar("sound_musicvolume").GetFloat()-defaults[1]) < 1e-6f,
            "resetting native audio settings broke subsequent live normalized slider changes");
        click(page->GetChildByID("default")); restored();
        std::printf("native audio Default restored registered values %.2f/%.2f and acknowledgements; existing PCM continued\n",defaults[0],defaults[1]);
        NMainLoop::DoneInterface(); return;
    }
    click(responses);
    Require(!responses->IsChecked() && NGlobal::GetVar("ui_charresponses").GetFloat() == 0.f && NGlobal::GetVar("ui_charresponsessubtitles").GetFloat() == 0.f,
        "the native response checkbox did not apply its change to config");
    click(subtitles);
    Require(subtitles->IsChecked() && NGlobal::GetVar("ui_charresponses").GetFloat() == 0.f && NGlobal::GetVar("ui_charresponsessubtitles").GetFloat() == 1.f,
        "the native subtitle checkbox did not apply its independent change to config");
    volumes();
    NGlobal::SetVar("ui_charresponses",1.f); NGlobal::SetVar("ui_charresponsessubtitles",0.f);
    NGlobal::LoadConfig(".\\cfg\\config.cfg");
    Require(NGlobal::GetVar("ui_charresponses").GetFloat() == 0.f && NGlobal::GetVar("ui_charresponsessubtitles").GetFloat() == 1.f,
        "native audio acknowledgement checkbox changes were not saved to isolated config");
    vector<float> output(512*2);
    Require(FSOUND_IsPlaying(channel) && S2FmodRenderForTest(output.data(),512) != 0 && FSOUND_GetCurrentPosition(channel) > 0,
        "audio checkbox changes interrupted an existing native sound");
    NMainLoop::DoneInterface();
    open = new NGame::CICOptions(NGame::OS_AUDIO,texture); open->Exec();
    CStructureNetworkContext reopenedReferences;
    auto reopened = BorrowedObjectFields(NMainLoop::GetCurrentInterfaceForHarness(),&reopenedReferences);
    { CStructureSaver reader(reopened,CStructureSaver::READ,&reopenedReferences); reader.Add(6,&page); }
    responses = dynamic_cast<NUI::CCheckButton*>(page->GetChildByID("char_responses"));
    subtitles = dynamic_cast<NUI::CCheckButton*>(page->GetChildByID("char_subtitles"));
    Require(IsValid(responses) && IsValid(subtitles) && !responses->IsChecked() && subtitles->IsChecked(),"reopening native audio options lost the saved acknowledgement state");
    std::printf("native audio acknowledgement click/config/reopen and legacy tags 2..6 preserved; PCM channel=%d\n",channel);
    NMainLoop::DoneInterface();
#else
    (void)databasePath; (void)resourcePath; (void)notificationOnly; (void)resetDefaults;
    Require(false,"native audio options fixture requires native SFX");
#endif
}

class BodyHoverMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(BodyHoverMission);
public:
    CPtr<CObjectBase> hover;
    bool fromWorld = true;
    CObjectBase* GetStateTarget(bool* world = nullptr) const override {
        if (world) *world = fromWorld;
        return hover;
    }
};
REGISTER_SAVELOAD_CLASS(0x7f550107,BodyHoverMission);

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

static void AIUnitFlash(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR"),"AI flash fixture requires isolated user data");
    struct SelectionModeRestore {NGlobal::CValue previous;~SelectionModeRestore(){NGlobal::SetVar("game_selectionmode",previous);}} mode{NGlobal::GetVar("game_selectionmode",0)};
    NGlobal::SetVar("game_selectionmode",2);
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm native AI flash regression");
    NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(-1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    auto* variant=NDb::GetTemplVariant(810);
    Require(variant && IsValid(variant->pTemplate),"AI flash fixture lacks its authored mission template");
    class Begin: public NMainLoop::CInterfaceCommand {
        OBJECT_NOCOPY_METHODS(Begin);
    public:
        CObj<NGame::CMission> mission=new NGame::CMission;
        CObj<NRPG::CGlobalGame> game;int templateID=-1;
        void Exec() override {
            Require(mission->Initialize(templateID,810,nullptr,vector<string>(),game),"AI flash fixture failed actual mission initialization");
            SetInterface(mission);
        }
    };
    CObj<Begin> begin=new Begin;begin->game=game;begin->templateID=variant->pTemplate->GetRecordID();begin->Exec();
    auto mission=begin->mission;
    auto* world=dynamic_cast<NWorld::CWorld*>(mission->GetWorld());auto* player=mission->GetActivePlayer();
    Require(world && player && mission->GetRenderGame() && mission->GetDesktop(),"AI flash fixture lacks its native world, renderer, player or desktop");
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(player->GetPlayer()));player->Update(true);
    vector<CPtr<NGame::IUnitTracker>> party;player->GetUnits(&party);
    Require(!party.empty(),"AI flash fixture lacks its real observer party");
    auto* observer=dynamic_cast<NWorld::CUnitServer*>(party.front()->GetUnit());
    Require(observer && observer->CanFight(),"AI flash fixture lacks its living observer");
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CPlayer* enemy=nullptr;
    for(auto unit: units) {
        auto* server=dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if(server && server->IsAIUnit() && server->CanFight() && world->GetDiplomacyState(player->GetPlayer(),server->GetPlayer())==NDb::DS_ENEMY) {
            enemy=dynamic_cast<NWorld::CPlayer*>(server->GetPlayer());break;
        }
    }
    Require(enemy,"AI flash fixture lacks an actual hostile AI commander");
    auto position=observer->GetPosition();bool placed=false;auto* network=world->GetPathNetwork();
    for(auto offset: {CTPoint<int>(2,0),CTPoint<int>(-2,0),CTPoint<int>(0,2),CTPoint<int>(0,-2)}) {
        const int x=int(position.pos.p.GetX())+offset.x,y=int(position.pos.p.GetY())+offset.y;
        if(x<0 || y<0 || x>255 || y>255)continue;
        auto point=position.pos.p;point.SetXY(x,y);point.SetPose(NAI::CM_STAND);point.SetMoving(0);
        if(network->IsPassable(point) && !network->IsLocked(point)) {position.pos.p=point;placed=true;break;}
    }
    Require(placed,"AI flash fixture has no free native tile beside its observer");
    CObj<NRPG::IUnitMission> rpg=NRPG::CreateUnit(observer->GetUnitRPG()->GetRPGUnit()->GetPers());
    CObj<NWorld::CUnitServer> target=new NWorld::CUnitServer(world,rpg,rpg->GetModel(),enemy,position);world->AddUnit(target);
    Require(target->IsAIUnit() && target->CanFight() && world->GetDiplomacyState(player->GetPlayer(),target->GetPlayer())==NDb::DS_ENEMY,
        "AI flash target is not a real hostile native unit");
    mission->PauseGame(true);mission->SetCheatVisibility(true);mission->GetDesktop()->SetStyle(NUI::STYLE_VISIBLE,false);
    while(NMainLoop::HaveInterfaceCommand()){CObj<NMainLoop::CInterfaceCommand> pending=NMainLoop::TakeInterfaceCommandForHarness();}
    // Freeze simulation ahead of the short presentation-clock interval; only
    // the real renderer's CCTime advances in the following color/lifetime checks.
    world->GetTime()->Set(1000000);world->GetAimTime()->Set(1000000);
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} clock{NMainLoop::SetTimeForHarness(100000)};
    Require(NMainLoop::StepApp(true,false,false),"AI flash fixture lost its native first frame");
    ICamera::SCameraPos placement;mission->GetCamera()->GetPlacement(&placement);
    placement.ptAnchor=target->GetPosition().GetCP()+CVec3(0,0,1.8f);placement.fRod=18;mission->GetCamera()->SetPlacement(placement);
    int frames=0;
    auto capture=[&](CArray2D<NGfx::SPixel8888>* image) {
        NMainLoop::SetTimeForHarness(100100+(++frames)*100);MarkNewDGFrame();
        Require(NMainLoop::StepApp(true,false,false) && NMainLoop::GetCurrentInterfaceForHarness()==mission.GetPtr(),"AI flash fixture lost its actual mission frame");
        NGfx::MakeScreenShot(image,false);
        Require(image->GetXSize()==320 && image->GetYSize()==240,"AI flash fixture did not read its actual GPU viewport");
    };
    CArray2D<NGfx::SPixel8888> bare,selected,stable,flashed;
    for(int warmup=0;warmup<24;++warmup)capture(&bare);
    NGlobal::SetVar("game_selectionmode",1);
    const CVec4 selectedColor(.25f,0,0,1);
    CObj<CObjectBase> selection=mission->GetRenderGame()->Select(target,selectedColor);
    Require(selection,"native visible AI target did not accept its ordinary selection control");
    for(int warmup=0;warmup<4;++warmup)capture(&selected);capture(&stable);
    vector<CTPoint<int>> mask;
    for(int y=35;y<205;++y)for(int x=50;x<270;++x) {
        const auto& a=bare[y][x];const auto& b=selected[y][x];
        if(int(b.r)-int(a.r)>8 && b.r<245 && a.g<220 && a.b<220)mask.emplace_back(x,y);
    }
    std::printf("native AI flash control: unit=%p modelPixels=%zu renderTime=%u worldTime=%u\n",target.GetPtr(),mask.size(),mission->GetRenderGame()->GetTime()->GetValue(),world->GetTime()->GetValue());std::fflush(stdout);
    Require(mask.size()>=8,"AI flash fixture did not observe an actual selected model on the GPU");
    for(auto p:mask)Require(std::abs(int(stable[p.y][p.x].r)-int(selected[p.y][p.x].r))<=1 &&
        std::abs(int(stable[p.y][p.x].g)-int(selected[p.y][p.x].g))<=1 && std::abs(int(stable[p.y][p.x].b)-int(selected[p.y][p.x].b))<=1,
        "AI flash fixture's paused native model control is not stable");
    // Idle animation also follows the renderer clock. Record the independent
    // no-flash control at the same future time used by the expiry assertion.
    CArray2D<NGfx::SPixel8888> expiryControl;
    mission->GetRenderGame()->GetTime()->Set(1501);
    for(int warmup=0;warmup<4;++warmup)capture(&expiryControl);
    mission->GetRenderGame()->GetTime()->Set(0);
    for(int warmup=0;warmup<4;++warmup)capture(&stable);
    NGlobal::SetVar("game_selectionmode",2);
    const int ap=target->GetAP(),vp=target->GetUnitRPG()->GetTotalVP();const auto unchanged=target->GetPosition();
    world->AddUICommand(new NWorld::CUICmdAIUnitWillMove(target));capture(&flashed);
    CObj<NWorld::CUICmd> remaining=world->GetUICommand();Require(!remaining,"AI flash fixture did not dispatch its native world UI command");
    NGlobal::SetVar("game_selectionmode",1);
    CObj<CObjectBase> preserved=mission->GetRenderGame()->Select(target,selectedColor);
    std::printf("native AI flash selection: preserved=%d\n",preserved.GetPtr()==selection.GetPtr());std::fflush(stdout);
    Require(preserved.GetPtr()==selection.GetPtr(),"AI movement hint replaced the player's ordinary selection instead of adding a temporary flash");
    int affected=0;
    for(auto p:mask)if(int(flashed[p.y][p.x].r)-int(selected[p.y][p.x].r)>4 || int(flashed[p.y][p.x].g)-int(selected[p.y][p.x].g)>4)++affected;
    std::printf("native AI flash GPU: changed model pixels=%d\n",affected);
    Require(affected>=8,"AI movement flash was suppressed by disabling ordinary selection visuals");
    // Read the native save fields without retaining the filter whose release is
    // under test. These are observations of the real renderer, not substitutes.
    struct Flash {CPtr<CObjectBase> node;STime end=0;int operator&(CStructureSaver& f){f.Add(2,&node);f.Add(3,&end);return 0;}};
    struct UnitSet {list<Flash> flashes;int operator&(CStructureSaver& f){f.Add(9,&flashes);return 0;}};
    auto flashes=[&]() {
        CStructureNetworkContext refs;auto saved=BorrowedObjectFields(mission->GetRenderGame(),&refs);UnitSet state;
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(7,&state);}
        return state.flashes;
    };
    auto live=flashes();Require(live.size()==1 && live.front().node && live.front().end==1500,
        "AI flash did not own exactly one native 1500ms filter");
    CPtr<CObjectBase> flashNode=live.front().node;
    auto strength=[&](const CArray2D<NGfx::SPixel8888>& image) {
        int total=0;for(auto p:mask)total+=Max(0,int(image[p.y][p.x].r)-int(selected[p.y][p.x].r))+
            Max(0,int(image[p.y][p.x].g)-int(selected[p.y][p.x].g));return total;
    };
    const int peak=strength(flashed);CArray2D<NGfx::SPixel8888> faded,expired;
    mission->GetRenderGame()->GetTime()->Set(300);capture(&faded);capture(&faded);
    const int after300=strength(faded);
    std::printf("native AI flash fade: peak=%d at300ms=%d\n",peak,after300);
    Require(after300>peak/10 && after300<peak*3/4,"native GPU flash did not fade with its presentation clock");
    mission->GetRenderGame()->GetTime()->Set(1500);capture(&expired);
    Require(flashes().size()==1 && IsValid(flashNode),"AI flash expired before the original 1500ms boundary");
    mission->GetRenderGame()->GetTime()->Set(1501);capture(&expired);capture(&expired);
    Require(flashes().empty() && !IsValid(flashNode),"expired AI flash retained its native scene filter");
    for(auto p:mask)Require(std::abs(int(expired[p.y][p.x].r)-int(expiryControl[p.y][p.x].r))<=1 &&
        std::abs(int(expired[p.y][p.x].g)-int(expiryControl[p.y][p.x].g))<=1 && std::abs(int(expired[p.y][p.x].b)-int(expiryControl[p.y][p.x].b))<=1,
        "expired AI flash left a tint on the native GPU model");
    mission->GetRenderGame()->FlashUnit(world,CVec4(1,1,1,0));
    Require(flashes().empty(),"AI flash accepted a non-unit object outside its visible unit set");
    CObj<CObjectBase> stillSelected=mission->GetRenderGame()->Select(target,selectedColor);
    Require(stillSelected.GetPtr()==selection.GetPtr(),"flash expiry removed the player's ordinary selection");
    // FastUpdate is the native renderer path that removes unit visuals (for
    // previews). A retained selection must not make an absent model flash.
    mission->GetRenderGame()->FastUpdate(1501);
    mission->GetRenderGame()->FlashUnit(target,CVec4(1,1,1,0));
    Require(flashes().empty(),"AI flash created a filter after the native renderer hid its unit set");
    std::printf("native AI flash lifetime/visibility: expiry1500/1501ms, hidden-set and non-unit controls passed\n");
    Require(target->GetAP()==ap && target->GetUnitRPG()->GetTotalVP()==vp && target->GetPosition()==unchanged && world->GetTime()->GetValue()==1000000,
        "AI flash presentation advanced native simulation, spent AP or moved/damaged the target");
}

static void BodyHoverLabels(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR"),"body hover fixture requires isolated user data");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm body hover label regression");
    NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(-1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    auto* variant = NDb::GetTemplVariant(810);
    Require(variant && IsValid(variant->pTemplate),"body hover fixture lacks its authored variant and parent template");
    class Begin: public NMainLoop::CInterfaceCommand {
        OBJECT_NOCOPY_METHODS(Begin);
    public:
        CObj<BodyHoverMission> mission = new BodyHoverMission;
        CObj<NRPG::CGlobalGame> game;
        int templateID = -1;
        void Exec() override {
            Require(mission->Initialize(templateID,810,nullptr,vector<string>(),game),"body hover fixture failed native mission initialization");
            SetInterface(mission);
        }
    };
    CObj<Begin> begin = new Begin;
    begin->game = game; begin->templateID = variant->pTemplate->GetRecordID(); begin->Exec();
    auto mission = begin->mission;
    auto* world = dynamic_cast<NWorld::CWorld*>(mission->GetWorld());
    auto* player = mission->GetActivePlayer();
    Require(world && player && mission->GetDesktop(),"body hover fixture lacks its actual world, player and HUD");
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(player->GetPlayer())); player->Update(true);
    vector<CPtr<NGame::IUnitTracker>> party; player->GetUnits(&party);
    Require(!party.empty(),"body hover fixture has no real party");
    auto* actor = dynamic_cast<NWorld::CUnitServer*>(party.front()->GetUnit());
    Require(actor && actor->CanFight() && !actor->IsEmptyPK(),"body hover fixture lacks its native living observer");
    mission->Select(actor); mission->RefreshPointerTarget();
    while (NMainLoop::HaveInterfaceCommand()) { CObj<NMainLoop::CInterfaceCommand> pending = NMainLoop::TakeInterfaceCommandForHarness(); }
    mission->PauseGame(true);
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(100000)};
    auto frame = [](NGame::CStateUse* state) {
        CStructureNetworkContext references; auto fields = BorrowedObjectFields(state,&references);
        CPtr<NUI::CTextFrame> result;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(4,&result); }
        return result;
    };
    auto initialize = [&](CObjectBase* target,bool expect) {
        mission->hover = target;
        CObj<NGame::CStateUse> state = new NGame::CStateUse;
        Require(state->Initialize(mission) == expect,"body hover changed native use-state eligibility");
        return state;
    };
    auto living = initialize(actor,false); Require(!IsValid(frame(living)),"a living human received a pickup label"); living->Terminate();
    auto absent = initialize(nullptr,false); Require(!IsValid(frame(absent)),"a missing target received a pickup label"); absent->Terminate();
    NDb::CRPGPers* record = nullptr;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while (people.MoveNext()) if (IsValid(people.Get()->pPanzerklein) && !people.Get()->pPanzerklein->bHasNoHead && IsValid(people.Get()->pModel)) { record = people.Get(); break; }
    Require(record,"body hover fixture has no authored ordinary PK persona");
    auto* network = world->GetPathNetwork(); vector<NAI::SPathPlace> places;
    network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),20.f),&places);
    NAI::SPathPlace anchor; bool found = false; float nearest = 1e30f;
    for (auto place: places) {
        place.SetPose(NAI::CM_STAND); place.SetDirection(0);
        if (!NAI::IsBigLockerPlacePassable(network,place)) continue;
        NAI::SUnitPosition position = actor->GetPosition(); position.pos.p = place;
        const float length = fabs2(position.GetCP()-actor->GetPosition().GetCP());
        if (length < nearest) { nearest = length; anchor = place; found = true; }
    }
    Require(found,"body hover fixture has no real empty PK footprint");
    CObj<NWorld::CUnitServer> shell = world->AddUnitInGame(anchor,NRPG::CreateUnit(record),nullptr);
    std::printf("native spawned PK: valid=%d persona=%d PK=%d fight=%d dead=%d unconscious=%d\n",IsValid(shell),record->GetRecordID(),shell ? shell->IsEmptyPK() : -1,shell ? shell->CanFight() : -1,shell ? shell->IsDead() : -1,shell ? shell->IsUnconscious() : -1);
    Require(shell && shell->IsEmptyPK() && !shell->CanFight(),"body hover fixture failed to spawn its real empty PK");
    NWorld::SEnemyInfo intactInfo; player->GetPlayer()->GetEnemyUnitInfo(shell,&intactInfo);
    Require(intactInfo.bPKInfo && !intactInfo.bUnitInfo && intactInfo.nPKHP > 0,
        "native empty PK control did not retain its intact armor health");
    Require(NMainLoop::StepApp(true,false,false),"body hover fixture lost its native first frame");
    ICamera::SCameraPos placement; mission->GetCamera()->GetPlacement(&placement);
    placement.ptAnchor = shell->GetPosition().GetCP()+CVec3(0,0,3.6f); placement.fRod = 30;
    mission->GetCamera()->SetPlacement(placement);
    NMainLoop::SetTimeForHarness(100100);
    Require(NMainLoop::StepApp(true,false,false),"body hover fixture lost its focused native frame");
    const int ap = actor->GetAP(), vp = actor->GetUnitRPG()->GetTotalVP(); const auto position = actor->GetPosition();
    auto transform = mission->GetCameraTransform(); CVec2 screen;
    CTransformStack direct; mission->GetCamera()->GetTransform(&direct,mission->GetScene()->GetScreenRect());
    const auto point = shell->GetPosition().GetCP(); const auto camera = mission->GetCamera()->GetCP();
    std::printf("native body projection: current=%d ready=%d screen=%.0fx%.0f point=%.2f,%.2f,%.2f camera=%.2f,%.2f,%.2f direct=%d cached=%d\n",
        NMainLoop::GetCurrentInterfaceForHarness() == mission.GetPtr(),mission->IsReady(),mission->GetScene()->GetScreenRect().x,mission->GetScene()->GetScreenRect().y,
        point.x,point.y,point.z,camera.x,camera.y,camera.z,
        TestRayInFrustrum(point+CVec3(0,0,3.6f),&direct,mission->GetScene()->GetScreenRect(),&screen),
        TestRayInFrustrum(point+CVec3(0,0,3.6f),&transform,mission->GetScene()->GetScreenRect(),&screen));
    Require(TestRayInFrustrum(shell->GetPosition().GetCP()+CVec3(0,0,3.6f),&transform,mission->GetScene()->GetScreenRect(),&screen),
        "native intact empty PK control was not projected into the real viewport");
    auto intact = initialize(shell,true); auto intactFrame = frame(intact);
    std::printf("native intact PK hover: HP=%d PKlabel=%d serverDead=%d\n",intactInfo.nPKHP,IsValid(intactFrame),shell->IsDead());
    Require(IsValid(intactFrame) && intactFrame->GetStyle(NUI::STYLE_VISIBLE),"native intact empty PK lost its projected label control");
    intact->Terminate(); Require(!IsValid(intactFrame),"terminating intact PK hover retained its label");
    shell->GetUnitRPG()->Kill(); shell->SetState(new NWorld::CUnitStateDeath(shell));
    Require(shell->IsDead() && shell->IsEmptyPK() && !shell->CanFight(),"body hover fixture failed its native destroyed PK state");
    auto destroyed = initialize(shell,true); auto destroyedFrame = frame(destroyed);
    std::printf("native empty PK hover: intactLabel=1 destroyedLabel=%d dead=%d unconscious=%d\n",IsValid(destroyedFrame),shell->IsDead(),shell->IsUnconscious());
    Require(IsValid(destroyedFrame) && destroyedFrame->GetStyle(NUI::STYLE_VISIBLE),"a destroyed empty Panzerklein lost its native pickup-hover label");
    NWorld::SEnemyInfo expected; player->GetPlayer()->GetEnemyUnitInfo(shell,&expected);
    Require(expected.bPKInfo && !expected.bUnitInfo && !expected.bCanSeePKHP && expected.nPKHP < 0,
        "destroyed PK label fixture changed its native information or visibility policy");
    list<CPtr<NUI::CWindow>> children; destroyedFrame->GetChildrenList(&children); NUI::CText* text = nullptr;
    for (auto child: children) if (auto* label = dynamic_cast<NUI::CText*>(child.GetPtr())) { text = label; break; }
    Require(text && text->GetText().find(NUI::GetDBString(19200)) != wstring::npos &&
        text->GetText().find(NUI::GetDBString(19208+int(expected.ePKCondition))) != wstring::npos,
        "destroyed PK hover lost its localized PK-only health condition");
    destroyed->Terminate(); Require(!IsValid(destroyedFrame),"terminating destroyed PK hover retained its label");
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units); NWorld::CUnitServer* human = nullptr;
    for (auto unit: units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (candidate && candidate != actor && candidate != shell.GetPtr() && candidate->CanFight() && !candidate->IsEmptyPK() && !candidate->IsWearingPK()) { human = candidate; break; }
    }
    Require(human,"body hover fixture lacks an ordinary second human");
    human->SetState(new NWorld::CUnitStateUnconscious(human));
    Require(human->IsUnconscious() && !human->CanFight(),"body hover fixture failed native unconsciousness");
    auto unconscious = initialize(human,true);
    Require(!IsValid(frame(unconscious)),"an ordinary unconscious human incorrectly received the empty-PK pickup label"); unconscious->Terminate();
    human->GetUnitRPG()->Kill(); human->SetState(new NWorld::CUnitStateDeath(human));
    auto corpse = initialize(human,true); Require(!IsValid(frame(corpse)),"an ordinary corpse received the empty-PK pickup label"); corpse->Terminate();
    mission->fromWorld = false; auto inventory = initialize(shell,false); Require(!IsValid(frame(inventory)),"a non-world target received a pickup label"); inventory->Terminate();
    Require(actor->GetAP() == ap && actor->GetUnitRPG()->GetTotalVP() == vp && actor->GetPosition() == position,
        "querying native body hover labels spent observer AP, changed health or moved the actor");
}

static void ForcedAttackTooltip(const char* databasePath,const char* resourcePath,bool cursorRouting = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"forced-attack tooltip fixture needs isolated user data");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm forced-attack tooltip regression");
    NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(-1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    auto* variant = NDb::GetTemplVariant(810);
    Require(variant,"forced-attack tooltip fixture has no authored variant");
    const int templateID = IsValid(variant->pTemplate) ? variant->pTemplate->GetRecordID() : -1;
    CObj<NGame::CICBeginMission> enter = new NGame::CICBeginMission(templateID,810,vector<string>(),game); enter->Exec();
    CObj<NGame::CMission> mission = dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(mission && mission->GetWorld() && mission->GetDesktop(),"forced-attack tooltip fixture did not open its actual tactical mission");
    auto* world = dynamic_cast<NWorld::CWorld*>(mission->GetWorld());
    auto* player = mission->GetActivePlayer();
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(player->GetPlayer()));
    player->Update(true);
    vector<CPtr<NGame::IUnitTracker>> party; player->GetUnits(&party);
    Require(!party.empty(),"forced-attack tooltip fixture has no real party tracker");
    auto* actor = dynamic_cast<NWorld::CUnitServer*>(party.front()->GetUnit());
    Require(actor && actor->CanFight(),"forced-attack tooltip fixture has no live native actor");
    mission->Select(actor);
    Require(mission->IsReady(),"forced-attack tooltip fixture is not in an interactive player turn");
    mission->RefreshPointerTarget();
    while (NMainLoop::HaveInterfaceCommand()) { CObj<NMainLoop::CInterfaceCommand> pending = NMainLoop::TakeInterfaceCommandForHarness(); }
    mission->PauseGame(true);
    mission->GetCamera()->FocusOnPlace(actor->GetPosition().GetCP(),actor->GetPosition().pos.GetFloor());
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(100000)};
    mission->Step();
    NGame::SActionInfo general; mission->GetActionInfo(NGame::UA_ATTACK,&general);
    std::printf("native attack-mode control: selected=%d result=%d ok=%d enoughAP=%d\n",
        mission->CountSelected(),int(general.eResult),general.bOk,general.bEnoughAP);
    Require(mission->CountSelected() == 1 && (general.eResult == NWorld::UCR_PENDING || (general.bOk && general.bEnoughAP)),
        "forced-attack tooltip fixture could not enter a native attack mode");
    auto tooltip = [](NGame::CStateAttack* state) {
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(state,&references);
        CPtr<NUI::CTextFrame> result;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(7,&result); }
        return result;
    };
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnit* other = nullptr;
    NWorld::CUnit* visible = nullptr;
    for (auto unit: units) {
        if (!other && unit->GetPlayer() && unit->GetPlayer() != player->GetPlayer()) other = unit;
        auto transform = mission->GetCameraTransform(); CVec2 screen;
        if (TestRayInFrustrum(unit->GetPosition().GetCP()+CVec3(0,0,3.6f),&transform,mission->GetScene()->GetScreenRect(),&screen)) {
            if (!visible || unit.GetPtr() == actor) visible = unit;
        }
    }
    std::printf("native tooltip targets: units=%zu other=%d visible=%d screen=%.0fx%.0f\n",units.size(),other != nullptr,visible != nullptr,mission->GetScene()->GetScreenRect().x,mission->GetScene()->GetScreenRect().y);
    Require(other && visible,"forced-attack tooltip fixture needs a non-owned hover target and a genuinely projected unit");
    mission->SetStateTarget(other);
    CObj<NGame::CStateAttack> hover = new NGame::CStateAttack(false);
    Require(hover->Initialize(mission) && IsValid(tooltip(hover)),"ordinary enemy hover failed its native tooltip allocation control");
    hover->Terminate();
    NWorld::SEnemyInfo expected; player->GetPlayer()->GetEnemyUnitInfo(visible,&expected);
    Require(!expected.wsName.empty(),"forced-attack tooltip fixture has no native fog-of-war display name");
    const int ap = actor->GetAP(), vp = actor->GetUnitRPG()->GetTotalVP();
    const auto position = actor->GetPosition();
    mission->SetStateTarget(visible);
    CObj<NGame::CStateAttack> forced = new NGame::CStateAttack(true);
    Require(cursorRouting ? mission->CommandState(forced) : forced->Initialize(mission),"forced-attack tooltip fixture did not initialize its native forced state");
    auto frame = tooltip(forced);
    std::printf("native forced attack tooltip: frame=%d projectedUnit=1\n",IsValid(frame));
    Require(IsValid(frame),"forced attack omitted the native unit tooltip frame");
    Require(frame->GetParent() == mission->GetDesktop()->GetClientWindow() && frame->GetStyle(NUI::STYLE_VISIBLE),
        "forced attack failed to show the tooltip above a genuinely projected unit");
    list<CPtr<NUI::CWindow>> children; frame->GetChildrenList(&children);
    NUI::CText* label = nullptr;
    for (auto child: children) if (auto* text = dynamic_cast<NUI::CText*>(child.GetPtr())) { label = text; break; }
    Require(label && label->GetText().find(expected.wsName) != wstring::npos,
        "forced attack lost the active player's fog-of-war unit caption");
    if (cursorRouting) {
        auto* hud = mission->GetDesktop();
        auto* ui = mission->GetInterface();
        const NUI::SCursorInfo expectedCursor = forced->GetCursorInfo();
        Require(IsValid(expectedCursor.pCursor) && mission->GetState() == forced.GetPtr(),
            "cursor routing fixture did not install its native forced state and cursor");
        CObj<NUI::CWindow> child = new NUI::CWindow(NUI::SWindowInfo(hud,NUI::SPoint(0,0),NUI::SPoint(80,80),
            "cursor-caption-control",NUI::STYLE_ENABLED|NUI::STYLE_VISIBLE|NUI::STYLE_TOPMOST));
        const NUI::SEvent move(NUI::EVENT_MOUSEMOVE,10,10);
        Require(child->HitTest(move.nX,move.nY),"cursor routing fixture did not hit its real child window");
        CStructureNetworkContext cursorReferences;
        auto cursorFields = BorrowedObjectFields(ui,&cursorReferences);
        CPtr<NUI::ICursor> nativeCursor;
        { CStructureSaver reader(cursorFields,CStructureSaver::READ,&cursorReferences); reader.Add(10,&nativeCursor); }
        Require(IsValid(nativeCursor),"cursor routing fixture lacks its real interface cursor");
        auto pendingCursor = [&]() {
            CStructureNetworkContext references;
            auto fields = BorrowedObjectFields(ui,&references);
            NUI::SCursorInfo pending;
            { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(6,&pending); }
            return pending;
        };
        STime frameTime = 100001;
        auto route = [&](NDb::CUICursor* icon,const wstring& expectedText) {
            const wstring childText = L"child caption differs from tactical state";
            Require(childText != expectedCursor.wsText,"cursor routing fixture cannot distinguish child and state captions");
            child->SetCursorInfo(NUI::SCursorInfo(icon,childText));
            child->ProcessMessage(move);
            const auto pending = pendingCursor();
            Require(pending.pCursor == icon && pending.wsText == childText,
                "native child cursor control did not publish its distinct caption");
            // GetCursorInfo observes the cursor submitted on the previous frame.
            // Step dispatches this frame's native mouse move, then submits its result.
            nativeCursor->SetCursor(NUI::SCursorInfo(icon,L"previous frame"));
            nativeCursor->SetPos(CVec2(10*S2UI::Scale(),10*S2UI::Scale()));
            ui->Step(frameTime++);
            std::printf("native tactical cursor after child: sameIcon=%d stateCaption=%d childCaption=%d\n",
                icon == expectedCursor.pCursor.GetPtr(),ui->GetCursorInfo().wsText == expectedText,ui->GetCursorInfo().wsText == childText);
            Require(ui->GetCursorInfo().pCursor == expectedCursor.pCursor && ui->GetCursorInfo().wsText == expectedText,
                "a child window replaced the forced state's caption while using the same cursor icon");
        };
        route(expectedCursor.pCursor,expectedCursor.wsText);
        auto* alternate = NDb::GetUICursor(expectedCursor.pCursor.GetPtr() == NDb::GetUICursor(1) ? 2 : 1);
        Require(alternate && alternate != expectedCursor.pCursor.GetPtr(),"cursor routing fixture lacks its different-icon control");
        route(alternate,L"");
        route(expectedCursor.pCursor,expectedCursor.wsText);
        hud->RemoveChild(child);
    }
    mission->SetStateTarget(nullptr);
    forced->Step();
    std::printf("native forced attack caption after leaving unit: visible=%d\n",frame->GetStyle(NUI::STYLE_VISIBLE));
    Require(!frame->GetStyle(NUI::STYLE_VISIBLE),"forced attack kept a stale unit caption after the pointer left its target");
    mission->SetStateTarget(visible);
    forced->Step();
    Require(frame->GetStyle(NUI::STYLE_VISIBLE) && label->GetText().find(expected.wsName) != wstring::npos,
        "forced attack failed to restore the native caption after returning to a unit");
    CObj<NWorld::CWindowDoor> nonUnit = new NWorld::CWindowDoor;
    mission->SetStateTarget(nonUnit);
    forced->Step();
    Require(!frame->GetStyle(NUI::STYLE_VISIBLE),"forced attack kept a stale unit caption over a non-unit object");
    forced->Terminate();
    Require(!IsValid(frame),"terminating forced attack retained its tooltip");
    mission->SetStateTarget(nullptr);
    CObj<NGame::CStateAttack> empty = new NGame::CStateAttack(true);
    Require(empty->Initialize(mission) && IsValid(tooltip(empty)) && !tooltip(empty)->GetStyle(NUI::STYLE_VISIBLE),
        "forced attack without a hovered unit failed to retain an initially hidden tooltip");
    empty->Terminate();
    Require(actor->GetAP() == ap && actor->GetUnitRPG()->GetTotalVP() == vp && actor->GetPosition() == position,
        "initializing attack tooltips spent AP, changed health or moved the selected actor");
}

static void HotseatDialogTemplate(const char* databasePath,const char* resourcePath,bool legacy = false,bool automatic = false,bool keyboard = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"hotseat dialog fixture requires isolated user data");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm hotseat dialog regression");
    NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(-1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{103}));
    CObj<NGame::CICBeginMission> enter = new NGame::CICBeginMission(810,-1,vector<string>(),game); enter->Exec();
    CObj<NGame::CMission> mission = dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(mission && mission->GetWorld(),"hotseat dialog fixture did not open its actual tactical mission");
    auto* hud = dynamic_cast<NUI::CMissionUI*>(mission->GetDesktop());
    Require(hud && mission->GetInterface(),"hotseat dialog fixture lacks its actual HUD and interface");
    vector<CPtr<NGame::IPlayerTracker>> players; mission->GetPlayers(&players);
    Require(players.size() == 2 && !players[0]->GetGlobalPlayer()->bAIPlayer && !players[1]->GetGlobalPlayer()->bAIPlayer,
        "hotseat dialog fixture did not create two real human player trackers");
    auto readDialog = [](NUI::CMissionUI* source) {
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(source,&references);
        CObj<NUI::CPlayerSwitchUI> result;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(25,&result); }
        Require(result,"hotseat dialog was not retained in the HUD's existing save field");
        return result;
    };
    auto dialog = readDialog(hud);
    if (legacy) {
        // Recreate the previous constructor's zero-sized HUD child using public
        // window APIs, then exercise the actual mission save graph in memory.
        CMObj<NUI::CPlayerSwitchUI> retained = dialog.GetPtr();
        dialog->GetParent()->RemoveChild(dialog);
        hud->AddChild(dialog);
        dialog->SetSize(NUI::SPoint(0,0));
        dialog->SetStyle(NUI::STYLE_ENABLED | NUI::STYLE_TOPMOST,false);
        CMemoryStream saved;
        { CStructureSaver writer(saved,CStructureSaver::WRITE_COMPRESSED); writer.Add(2,&mission); }
        saved.Seek(0);
        CObj<NGame::CMission> restored;
        { CStructureSaver reader(saved,CStructureSaver::READ); reader.Add(2,&restored); }
        Require(restored && restored.GetPtr() != mission.GetPtr() && restored->GetWorld() && restored->GetInterface(),
            "legacy hotseat dialog fixture failed to restore its actual mission graph");
        mission = restored;
        hud = dynamic_cast<NUI::CMissionUI*>(mission->GetDesktop());
        Require(hud,"legacy hotseat dialog fixture did not restore its mission desktop");
        dialog = readDialog(hud);
        mission->GetPlayers(&players);
        Require(players.size() == 2 && dialog->GetParent() == hud && dialog->GetSize() == NUI::SPoint(0,0),
            "legacy save control did not retain the prior zero-sized HUD dialog");
        std::printf("native restored legacy dialog: root=0 size=0x0 players=%zu\n",players.size());
        if (!automatic) {
        mission->PauseGame(false);
        dialog->Show();
        Require(dialog->GetParent() == mission->GetInterface() && dialog->GetSize().x > 0 && dialog->GetSize().y > 0,
            "showing the restored legacy hotseat dialog paused the mission with a zero-sized HUD child");
        dialog->OnOK();
        Require(!mission->IsGamePaused(),"repairing a legacy hotseat dialog changed its prior pause state");
        }
    }
    if (automatic) {
        STime time = 1000;
        auto frame = [&]() { hud->Update(++time,mission->GetInterface()->GetView()); };
        auto visible = [&]() { return dialog->GetStyle(NUI::STYLE_VISIBLE); };
        auto nameCorrect = [&]() {
            auto* text = dynamic_cast<NUI::CText*>(dialog->GetChildByID("text"));
            return text && text->GetText() == NUI::GetDBString(0x51F3)+mission->GetActivePlayer()->GetPlayer()->GetPlayerName();
        };
        int acceptedKeys = 0;
        auto accept = [&]() {
            if (!keyboard) { dialog->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY,string("ok"))); return; }
            NInput::SEvent other{};
            other.mMessage.cType = NInput::CT_KEY;
            other.mMessage.nAction = NInput::GetControlID("A");
            other.mMessage.bState = true;
            Require(mission->GetInterface()->ProcessEvent(other) && visible() && mission->IsGamePaused(),
                "a non-Enter key escaped the shown handoff dialog or accepted it");
            const char* keyName = acceptedKeys++ % 2 ? "NUM_ENTER" : "ENTER";
            NInput::SEvent enter{};
            enter.mMessage.cType = NInput::CT_KEY;
            enter.mMessage.nAction = NInput::GetControlID(keyName);
            enter.mMessage.bState = true;
            int virtualKey = 0;
            Require(NInput::GetKeyForMessage(enter.mMessage,&virtualKey) && virtualKey == VK_RETURN,
                "hotseat keyboard control did not map the native physical Enter message");
            const bool focused = dialog->IsActive();
            const bool consumed = mission->GetInterface()->ProcessEvent(enter);
            std::printf("native hotseat %s: legacy=%d focused=%d consumed=%d stillVisible=%d\n",keyName,legacy,focused,consumed,visible());
            Require(consumed && !visible(),"the shown hotseat dialog did not receive Enter through its native interface");
        };
        mission->PauseGame(false);
        frame();
        std::printf("native automatic handoff: legacy=%d visible=%d paused=%d root=%d size=%dx%d\n",
            legacy,visible(),mission->IsGamePaused(),dialog->GetParent() == mission->GetInterface(),dialog->GetSize().x,dialog->GetSize().y);
        Require(visible() && mission->IsGamePaused() && nameCorrect(),
            "a new human hotseat player did not automatically receive the handoff dialog");
        Require(dialog->GetParent() == mission->GetInterface() && dialog->GetSize().x > 0 && dialog->GetSize().y > 0,
            "automatic handoff retained an unusable legacy dialog layout");
        accept(); frame();
        Require(!visible() && !mission->IsGamePaused(),"the same active hotseat player reopened the accepted dialog");
        const int initialPlayer = mission->GetActivePlayer() == players[0] ? 0 : 1;
        for (int turn = 0; turn < 4; ++turn) {
            const bool paused = turn >= 2;
            mission->SetActivePlayer(players[(initialPlayer+turn+1)%2]); mission->PauseGame(paused);
            frame();
            Require(visible() && mission->IsGamePaused() && nameCorrect(),
                "a changed human hotseat player lost its automatic named handoff");
            accept(); frame();
            Require(!visible() && mission->IsGamePaused() == paused,
                "accepting automatic handoff changed the prior pause or reopened on the next UI update");
        }
        // This is the UI's persisted AI-player flag, not a fabricated commander.
        auto aiTurn = players[mission->GetActivePlayer() == players[0] ? 1 : 0];
        aiTurn->GetGlobalPlayer()->bAIPlayer = true;
        mission->SetActivePlayer(aiTurn); mission->PauseGame(false); frame();
        Require(!visible() && !mission->IsGamePaused(),"an AI hotseat slot was interrupted by a human handoff dialog");
        aiTurn->GetGlobalPlayer()->bAIPlayer = false;
        frame();
        Require(visible() && nameCorrect(),"skipping an AI slot incorrectly acknowledged its later human handoff");
        accept();
        CStructureNetworkContext latchReferences;
        auto latchFields = BorrowedObjectFields(hud,&latchReferences);
        CPtr<NGame::IPlayerTracker> acknowledged;
        { CStructureSaver reader(latchFields,CStructureSaver::READ,&latchReferences); reader.Add(26,&acknowledged); }
        Require(acknowledged == mission->GetActivePlayer(),"automatic hotseat handoff did not persist its existing player latch");
        CMemoryStream acknowledgedSave;
        { CStructureSaver writer(acknowledgedSave,CStructureSaver::WRITE_COMPRESSED); writer.Add(2,&mission); }
        acknowledgedSave.Seek(0);
        CObj<NGame::CMission> acknowledgedMission;
        { CStructureSaver reader(acknowledgedSave,CStructureSaver::READ); reader.Add(2,&acknowledgedMission); }
        Require(acknowledgedMission && acknowledgedMission.GetPtr() != mission.GetPtr(),
            "acknowledged handoff failed to restore its complete native mission graph");
        mission = acknowledgedMission;
        hud = dynamic_cast<NUI::CMissionUI*>(mission->GetDesktop());
        Require(hud,"acknowledged handoff lost its restored HUD");
        dialog = readDialog(hud);
        frame();
        Require(!visible() && !mission->IsGamePaused(),"loading an acknowledged handoff reopened the dialog or paused the mission");
        // An ordinary one-player campaign uses the same HUD without any handoff.
        NMainLoop::DoneInterface();
        while (NMainLoop::HaveInterfaceCommand()) { CObj<NMainLoop::CInterfaceCommand> command = NMainLoop::TakeInterfaceCommandForHarness(); }
        game = NRPG::CreateGlobalGame(-1);
        game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        enter = new NGame::CICBeginMission(810,-1,vector<string>(),game); enter->Exec();
        mission = dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
        Require(mission && mission->GetInterface(),"single-player handoff control did not create a native mission");
        hud = dynamic_cast<NUI::CMissionUI*>(mission->GetDesktop());
        Require(hud,"single-player handoff control lost its native HUD");
        dialog = readDialog(hud); mission->GetPlayers(&players);
        Require(players.size() == 1,"single-player handoff control has multiple trackers");
        mission->PauseGame(false);
        for (int update = 0; update < 3; ++update) frame();
        Require(!visible() && !mission->IsGamePaused(),"automatic hotseat handoff interrupted an ordinary one-player campaign");
        return;
    }
    auto* label = dynamic_cast<NUI::CText*>(dialog->GetChildByID("text"));
    auto* ok = dynamic_cast<NUI::CHoverButton*>(dialog->GetChildByID("ok"));
    std::printf("native hotseat dialog: root=%d text=%d ok=%d size=%dx%d\n",
        dialog->GetParent() == mission->GetInterface(),label != nullptr,ok != nullptr,dialog->GetSize().x,dialog->GetSize().y);
    Require(dialog->GetParent() == mission->GetInterface() && label && ok,
        "the native mission constructed the hotseat dialog from a missing HUD child instead of its own template");
    Require(!dialog->GetStyle(NUI::STYLE_VISIBLE) && dialog->GetStyle(NUI::STYLE_ENABLED) &&
        dialog->GetStyle(NUI::STYLE_TOPMOST) && dialog->GetSize().x > 0 && dialog->GetSize().y > 0,
        "the loaded hotseat dialog did not start hidden with a usable topmost layout");
    for (auto player: players) for (bool paused: {false,true}) {
        mission->SetActivePlayer(player); mission->PauseGame(paused);
        dialog->Show();
        Require(dialog->GetStyle(NUI::STYLE_VISIBLE) && mission->IsGamePaused() &&
            label->GetText() == NUI::GetDBString(0x51F3)+player->GetPlayer()->GetPlayerName(),
            "the loaded native hotseat dialog failed to show the active player's name and pause the mission");
        Require(dialog->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY,string("ok"))) &&
            !dialog->GetStyle(NUI::STYLE_VISIBLE) && mission->IsGamePaused() == paused,
            "accepting the native hotseat dialog did not hide it and restore the prior pause state");
    }
    list<CPtr<NUI::CWindow>> rootChildren; mission->GetInterface()->GetChildrenList(&rootChildren);
    int dialogs = 0;
    for (auto child: rootChildren) if (child->GetWindowID() == "playerswitch") ++dialogs;
    Require(dialogs == 1 && mission->GetInterface()->GetChildByID("playerswitch") == dialog.GetPtr(),
        "showing the hotseat dialog duplicated it or lost its root attachment");
}

static void MissionSavePolicy(const char* databasePath,const char* resourcePath,bool network = false)
{
    Require(std::getenv("S2_USER_DATA_DIR"),"mission save-policy fixture requires isolated user data");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm live save policy regression");
    NDb::CDBDifficulty* records[3]={};
    CDBIterator<NDb::CDBDifficulty> difficulties(*NDatabase::GetTable<NDb::CDBDifficulty>());
    while(difficulties.MoveNext()) {
        auto* record=difficulties.Get();
        std::printf("native save policy control: record=%d name=%s canSave=%d\n",record->GetRecordID(),record->szUserName.c_str(),int(record->canSave));
        if(int(record->canSave)>=0 && int(record->canSave)<3 && !records[int(record->canSave)])records[int(record->canSave)]=record;
    }
    std::fflush(stdout);
    Require(records[NDb::E_CS_ALWAYS] && records[NDb::E_CS_REALTIME],"save policy fixture lacks its authored realtime/always records");
    NMainLoop::CreateProfile(network?"UpstreamNetworkSavePolicy":"UpstreamMissionSavePolicy");
    NMainLoop::SetActiveProfile(network?"UpstreamNetworkSavePolicy":"UpstreamMissionSavePolicy");
    auto* manager=NMainLoop::GetSaveManager();
    struct SlotCleanup {NMainLoop::CSaveManager* manager;~SlotCleanup(){manager->DeleteSlot(NMainLoop::S_SLOT_ACTIVE);}} slots{manager};
    NGame::InitLoadingScreen();
    CObj<NGame::CMission> mission;CObj<NRPG::CGlobalGame> game;
    std::shared_ptr<S2Net::NetworkWorld> netWorld;std::shared_ptr<S2Net::NetworkSession> session;
    if(network) {
        S2Net::NetworkMatchPreset preset;preset.prepareOnBase=true;preset.squadSize=1;
        netWorld=std::make_shared<S2Net::NetworkWorld>(true,preset);netWorld->Initial();
        session=std::make_shared<S2Net::NetworkSession>(S2Net::Compatibility{"save-policy-regression","data"},netWorld->Callbacks());session->Host(0);
        NGame::OpenNetworkMission(netWorld,session);
        CObj<NMainLoop::CInterfaceCommand> open=NMainLoop::TakeInterfaceCommandForHarness();
        Require(open && !NMainLoop::HaveInterfaceCommand(),"save policy fixture lost its queued real LAN mission command");open->Exec();
        mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
        Require(mission && netWorld->IsPreparing() && mission->GetWorld()==netWorld->Match().world.GetPtr(),
            "save policy fixture did not initialize the actual LAN preparation mission");
        game=netWorld->Match().game;
    } else {
        game=NRPG::CreateGlobalGame(-1,records[NDb::E_CS_ALWAYS]);
        game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        CObj<NGame::CICBeginMission> enter=new NGame::CICBeginMission(810,-1,vector<string>(),game);enter->Exec();
        mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
        Require(mission,"save policy fixture failed to open its actual native tactical mission");
        CObj<NMainLoop::CInterfaceCommand> snapshot=NMainLoop::TakeInterfaceCommandForHarness();
        Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"save policy fixture did not isolate its actual restart snapshot");snapshot->Exec();
        Require(!mission->GetWorld()->IsBase(),"tactical save-policy fixture unexpectedly entered HQ");
        mission->PauseGame(true);
    }
    auto frame=[&]() {
        Require(NMainLoop::StepApp(true,false,false) && NMainLoop::GetCurrentInterfaceForHarness()==mission.GetPtr() && !NMainLoop::HaveInterfaceCommand(),
            "save policy fixture lost its actual native mission/frame");
    };
    auto allowed=[&]() {
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(mission,&refs);
        CMemoryStream base;
        {CStructureSaver writer(base,CStructureSaver::WRITE,&refs);mission->NGame::CMissionBase::operator&(writer);}
        base.Seek(0);bool value=false;
        {CStructureSaver reader(base,CStructureSaver::READ,&refs);reader.Add(30,&value);}return value;
    };
    auto check=[&](NDb::ECanSave policy,bool expected,const char* stage) {
        game->pDifficulty=records[policy];frame();
        const bool actual=allowed();
        std::printf("native save policy: network=%d stage=%s policy=%d realtime=%d sequence=%d actual=%d expected=%d\n",int(network),stage,
            int(policy),int(mission->IsRealTime()),int(mission->IsSequence()),int(actual),int(expected));std::fflush(stdout);
        Require(actual==expected,"actual mission frame did not recompute the original save policy or preserve the LAN save prohibition");
    };
    if(network) {
        Require(game->pDifficulty==NDb::GetDBDifficulty(2) && !allowed(),"actual LAN preparation lost its original fork difficulty2/save prohibition");
        for(auto policy:{NDb::E_CS_ALWAYS,NDb::E_CS_REALTIME,NDb::E_CS_ALWAYS})check(policy,false,"LAN-preparation");
        game->pDifficulty=NDb::GetDBDifficulty(2);frame();
        Require(!allowed(),"restoring the own LAN difficulty enabled mission saves");
        NMainLoop::DoneInterface();return;
    }
    auto checkMenu=[&](bool expected) {
        NInput::SEvent event;while(NInput::GetEvent(&event)) {}
        NInput::PostEvent("savemenu");
        Require(NInput::GetEvent(&event) && mission->ProcessEvent(event),"live save policy fixture did not dispatch its native Save-menu binding");
        CObj<NMainLoop::CInterfaceCommand> command=NMainLoop::TakeInterfaceCommandForHarness();
        Require(dynamic_cast<NGame::CICSaveLoadMenu*>(command.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"live save policy did not open the actual save/load menu");command->Exec();
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(NMainLoop::GetCurrentInterfaceForHarness(),&refs);
        bool permission=!expected;NGame::ESaveLoadType mode=NGame::SAVE;CPtr<NUI::CWindow> page;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&permission);reader.Add(3,&mode);reader.Add(6,&page);}
        Require(permission==expected && mode==(expected?NGame::SAVE:NGame::LOAD) && page,"live mission permission did not reach its actual Save/Load dialog");
        CStructureNetworkContext pageRefs;auto pageFields=BorrowedObjectFields(page,&pageRefs);CPtr<NUI::CWindow> view;
        {CStructureSaver reader(pageFields,CStructureSaver::READ,&pageRefs);reader.Add(4,&view);}
        Require(view && pSSClasses->GetObjectTypeID(view)==int(expected?0xB1014135:0xB1014134),"live save policy opened the wrong native Save/Load view");
        CObj<NMainLoop::CICExitModal> close=new NMainLoop::CICExitModal;close->Exec();
        Require(NMainLoop::GetCurrentInterfaceForHarness()==mission.GetPtr(),"closing the live policy dialog lost the original mission");
        std::printf("native live save dialog: allowed=%d view=%s\n",int(expected),expected?"SAVE":"LOAD");
    };
    Require(mission->IsRealTime() && !mission->IsSequence(),"save policy fixture did not start in native realtime outside a sequence");
    check(NDb::E_CS_ALWAYS,true,"realtime-positive");
    check(NDb::E_CS_REALTIME,true,"realtime");
    auto* world=dynamic_cast<NWorld::CWorld*>(mission->GetWorld());Require(world,"save policy fixture lost its real world");
    world->AddUICommand(new NWorld::CUICmdBeginSequence(true));frame();
    Require(mission->IsSequence() && mission->IsRealTime(),"save policy fixture did not enter the real native UI sequence");
    check(NDb::E_CS_REALTIME,false,"sequence");
    check(NDb::E_CS_ALWAYS,true,"always-in-sequence");
    world->AddUICommand(new NWorld::CUICmdEndSequence(true,true));frame();
    Require(!mission->IsSequence(),"save policy fixture did not leave the native UI sequence");
    check(NDb::E_CS_REALTIME,true,"after-sequence");
    auto* player=dynamic_cast<NWorld::CPlayer*>(mission->GetActivePlayer()->GetPlayer());Require(player,"save policy fixture lacks its actual native player");
    world->GivePlayerTurn(player);Require(!mission->IsRealTime(),"save policy fixture did not start a native player turn");
    check(NDb::E_CS_REALTIME,false,"turn-based");checkMenu(false);
    check(NDb::E_CS_ALWAYS,true,"always-in-turn");
    world->EndOfTurn();Require(mission->IsRealTime(),"save policy fixture did not return to real time after ending its native turn");
    check(NDb::E_CS_REALTIME,true,"after-turn");checkMenu(true);
    // The shipped DB has Easy=ALWAYS, Normal/Hard=REALTIME and no CHAPTER
    // record. Exercise enum0 on a separate full native in-memory record; never
    // mutate the authored DB or present this as an existing difficulty choice.
    CObj<NDb::CDBDifficulty> chapter=new NDb::CDBDifficulty;CMemoryStream savedDifficulty;
    {CStructureSaver writer(savedDifficulty,CStructureSaver::WRITE);records[NDb::E_CS_ALWAYS]->operator&(writer);}
    savedDifficulty.Seek(0);
    {CStructureSaver reader(savedDifficulty,CStructureSaver::READ);chapter->operator&(reader);}
    chapter->canSave=NDb::E_CS_CHAPTER;records[NDb::E_CS_CHAPTER]=chapter.GetPtr();
    Require(chapter.GetPtr()!=NDb::GetDBDifficulty(1) && NDb::GetDBDifficulty(1)->canSave==NDb::E_CS_ALWAYS &&
        chapter->fAPCoeff==NDb::GetDBDifficulty(1)->fAPCoeff && chapter->fVPCoeff==NDb::GetDBDifficulty(1)->fVPCoeff,
        "test-defined chapter policy changed an authored difficulty record or coefficients");
    check(NDb::E_CS_CHAPTER,false,"test-defined-chapter");
    check(NDb::E_CS_ALWAYS,true,"restored-authored-always");
    NMainLoop::DoneInterface();
    auto* campaign=NDb::GetGlobalMap(3);
    Require(campaign && IsValid(campaign->pScenario) && IsValid(campaign->pBaseZone),"save policy HQ fixture lacks its authored campaign/base");
    game=NRPG::CreateGlobalGame(campaign->pScenario->GetRecordID(),records[NDb::E_CS_REALTIME]);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    game->players.front()->pSide=NScenario::GetSideForScenario(game->pScenarioTracker);
    auto* base=game->pScenarioTracker->GetZoneByDBZone(campaign->pBaseZone);
    Require(base,"save policy HQ fixture did not construct its actual scenario base zone");
    CObj<NGame::CICBeginMission> enterBase=new NGame::CICBeginMission(base,-1,vector<string>(),game);enterBase->Exec();
    mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(mission && mission->GetWorld()->IsBase(),"save policy fixture did not enter an actual original HQ world");
    CObj<NMainLoop::CInterfaceCommand> snapshot=NMainLoop::TakeInterfaceCommandForHarness();
    Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"HQ save policy fixture lost its actual restart snapshot");snapshot->Exec();
    mission->PauseGame(true);
    check(NDb::E_CS_CHAPTER,true,"HQ-test-defined-chapter");
    world=dynamic_cast<NWorld::CWorld*>(mission->GetWorld());Require(world,"HQ save policy fixture lost its actual world");
    world->AddUICommand(new NWorld::CUICmdBeginSequence(true));frame();
    Require(mission->IsSequence(),"HQ save policy fixture did not enter a native UI sequence");
    check(NDb::E_CS_REALTIME,true,"HQ-realtime-in-sequence");
    check(NDb::E_CS_ALWAYS,true,"HQ-always-in-sequence");
    NMainLoop::DoneInterface();
}

static void MissionSaveMenu(const char* databasePath,const char* resourcePath,bool quickSave = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"mission save-menu fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"mission save-menu fixture started with an active interface");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm mission save-menu regression");
    const char* profile = quickSave ? "UpstreamMissionQuickSave" : "UpstreamMissionSaveMenu";
    NMainLoop::CreateProfile(profile); NMainLoop::SetActiveProfile(profile);
    auto* manager = NMainLoop::GetSaveManager();
    struct SlotCleanup {
        NMainLoop::CSaveManager* manager; bool visible;
        void ClearVisible() { list<string> slots; manager->GetSlotsList(&slots); for (const auto& slot : slots) manager->DeleteSlot(slot); }
        ~SlotCleanup() { if (visible) ClearVisible(); manager->DeleteSlot(NMainLoop::S_SLOT_ACTIVE); }
    } slotCleanup{manager,quickSave};
    if (quickSave) slotCleanup.ClearVisible();
    NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(-1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginMission> enter = new NGame::CICBeginMission(810,-1,vector<string>(),game); enter->Exec();
    CObj<NGame::CMission> mission = dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(mission) && IsValid(mission->GetWorld()),"mission save-menu fixture did not open its actual tactical mission");
    CObj<NMainLoop::CInterfaceCommand> snapshot = NMainLoop::TakeInterfaceCommandForHarness();
    Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"mission save-menu fixture did not isolate its real restart snapshot");
    snapshot->Exec();
    const auto* world = mission->GetWorld(); const STime worldTime = world->GetAimTime()->GetValue();
    const auto* difficulty = game->pDifficulty.GetPtr();
    NWorld::IPlayer::CUnitSet party; mission->GetActivePlayer()->GetPlayer()->GetUnits(&party);
    auto permission = [&](bool allowed) {
        // Supply an existing saved permission without changing its calculation.
        CMemoryStream fields;
        { CStructureSaver writer(fields,CStructureSaver::WRITE); writer.Add(30,&allowed); }
        fields.Seek(0);
        { CStructureSaver reader(fields,CStructureSaver::READ); mission->NGame::CMissionBase::operator&(reader); }
    };
    if (quickSave) {
        auto slots = [&]() { list<string> result; manager->GetSlotsList(&result); result.sort(); return result; };
        auto bytes = [&](const string& slot) {
            CFileStream file; file.OpenRead(manager->GetSlotFilePathW(slot,NMainLoop::S_SAVE_FILENAME).c_str());
            vector<unsigned char> data(file.GetSize()); if (!data.empty()) file.Read(data.data(),data.size()); return data;
        };
        Require(slots().empty(),"mission QuickSave fixture did not clear its own isolated slots");
        NInput::CBind save("save");
        for (bool allowed : {false,true,false}) {
            permission(allowed);
            const auto before = slots(); map<string,vector<unsigned char>> previous;
            for (const auto& slot : before) previous[slot] = bytes(slot);
            const string target = NMainLoop::GetQuickSaveSlot(false);
            NInput::SEvent event; while (NInput::GetEvent(&event)) {}
            NInput::PostEvent("save");
            // Exercise the real application dispatcher, including its global
            // fallback when the active mission does not consume this event.
            Require(NMainLoop::StepApp(false,false,true),"native mission QuickSave dispatch requested application exit");
            const auto after = slots();
            std::printf("native QuickSave permission=%d, slots before=%zu after=%zu\n",int(allowed),before.size(),after.size());
            if (!allowed) {
                Require(after == before,"a forbidden mission QuickSave created or changed a save slot");
                for (const auto& slot : before) Require(bytes(slot) == previous[slot],"a forbidden mission QuickSave overwrote an existing save");
            } else {
                Require(after.size() == before.size()+1 && std::find(after.begin(),after.end(),target) != after.end(),
                    "an explicitly permitted native mission QuickSave did not write its expected slot");
                CFileStream file; file.OpenRead(manager->GetSlotFilePathW(target,NMainLoop::S_SAVE_FILENAME).c_str());
                auto header = std::make_unique<NMainLoop::SSaveFileHeader>(); NMainLoop::ReadSaveFileHeader(file,header.get());
                Require(header->nMagic == NMainLoop::N_SAVE_MAGIC_NUMBER && header->nMods == 0 &&
                    file.GetSize() > 8+NMainLoop::N_SAVE_SCREENSHOT_X*NMainLoop::N_SAVE_SCREENSHOT_Y*4+1024,
                    "native mission QuickSave did not write its real header and serialized mission data");
            }
            NWorld::IPlayer::CUnitSet afterParty; mission->GetActivePlayer()->GetPlayer()->GetUnits(&afterParty);
            Require(NMainLoop::GetCurrentInterfaceForHarness() == mission.GetPtr() && !NMainLoop::HaveInterfaceCommand() && mission->GetWorld() == world &&
                world->GetAimTime()->GetValue() == worldTime && afterParty == party && game->pDifficulty.GetPtr() == difficulty,
                "native QuickSave dispatch changed the mission, world, party or difficulty");
        }
        std::puts("native mission QuickSave false/true/false: no forbidden slot or overwrite; permitted save header/data written");
        NMainLoop::DoneInterface(); return;
    }
    NInput::CBind saveMenu("savemenu");
    for (bool allowed : {false,true}) {
        permission(allowed);
        NInput::SEvent event; while (NInput::GetEvent(&event)) {}
        NInput::PostEvent("savemenu");
        Require(NInput::GetEvent(&event) && saveMenu.ProcessEvent(event) && mission->ProcessEvent(event) && NMainLoop::HaveInterfaceCommand(),
            "mission save-menu fixture did not dispatch its actual native bound command");
        CObj<NMainLoop::CInterfaceCommand> command = NMainLoop::TakeInterfaceCommandForHarness();
        Require(dynamic_cast<NGame::CICSaveLoadMenu*>(command.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"mission Save-menu action queued an unrelated command");
        command->Exec();
        CStructureNetworkContext dialogReferences; auto dialog = BorrowedObjectFields(NMainLoop::GetCurrentInterfaceForHarness(),&dialogReferences);
        bool dialogPermission = !allowed; int mode = -1; CPtr<NUI::CWindow> page;
        { CStructureSaver reader(dialog,CStructureSaver::READ,&dialogReferences); reader.Add(2,&dialogPermission); reader.Add(3,&mode); reader.Add(6,&page); }
        Require(dialogPermission == allowed && IsValid(page),"the actual mission did not forward its existing save permission to the native dialog");
        CStructureNetworkContext viewReferences; auto fields = BorrowedObjectFields(page,&viewReferences); CPtr<NUI::CWindow> view;
        { CStructureSaver reader(fields,CStructureSaver::READ,&viewReferences); reader.Add(4,&view); }
        Require(IsValid(view),"a forbidden mission Save-menu command opened an empty native save/load page");
        Require(mode == (allowed ? NGame::SAVE : NGame::LOAD) && pSSClasses->GetObjectTypeID(view) == int(allowed ? 0xB1014135 : 0xB1014134),
            "the mission save-menu command selected the wrong actual native view");
        CObj<NMainLoop::CICExitModal> close = new NMainLoop::CICExitModal; close->Exec();
        NWorld::IPlayer::CUnitSet after; mission->GetActivePlayer()->GetPlayer()->GetUnits(&after);
        Require(NMainLoop::GetCurrentInterfaceForHarness() == mission.GetPtr() && !NMainLoop::HaveInterfaceCommand() && mission->GetWorld() == world &&
            world->GetAimTime()->GetValue() == worldTime && after == party && game->pDifficulty.GetPtr() == difficulty,
            "opening and closing a mission save-menu changed the world, party or difficulty");
    }
    std::puts("actual tactical mission Save-menu false/true selected native Load/Save views; world/party/difficulty preserved");
    NMainLoop::DoneInterface();
}

static void MissionRestartMenu(const char* databasePath,const char* resourcePath,bool network = false)
{
    Require(std::getenv("S2_USER_DATA_DIR"),"restart permission fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"restart fixture has a pending interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm mission restart permission regression");
    const char* profile=network?"UpstreamNetworkRestart":"UpstreamMissionRestart";
    NMainLoop::CreateProfile(profile);NMainLoop::SetActiveProfile(profile);
    auto* manager=NMainLoop::GetSaveManager();
    struct Slots {NMainLoop::CSaveManager* manager;~Slots(){manager->DeleteSlot(NMainLoop::S_SLOT_ACTIVE);}} slots{manager};
    NGame::InitLoadingScreen();
    CObj<NGame::CMission> mission;CObj<NRPG::CGlobalGame> game;
    std::shared_ptr<S2Net::NetworkWorld> netWorld;std::shared_ptr<S2Net::NetworkSession> session;
    if(network) {
        S2Net::NetworkMatchPreset preset;preset.prepareOnBase=true;preset.squadSize=1;
        netWorld=std::make_shared<S2Net::NetworkWorld>(true,preset);netWorld->Initial();
        session=std::make_shared<S2Net::NetworkSession>(S2Net::Compatibility{"restart-permission-regression","data"},netWorld->Callbacks());session->Host(0);
        NGame::OpenNetworkMission(netWorld,session);
        CObj<NMainLoop::CInterfaceCommand> open=NMainLoop::TakeInterfaceCommandForHarness();
        Require(open && !NMainLoop::HaveInterfaceCommand(),"restart fixture lost its LAN preparation command");open->Exec();
        mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
        Require(mission && netWorld->IsPreparing(),"restart fixture did not open real LAN preparation");game=netWorld->Match().game;
        Require(!NMainLoop::HaveInterfaceCommand(),"LAN preparation unexpectedly queued a local restart snapshot");
    } else {
        game=NRPG::CreateGlobalGame(-1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        CObj<NGame::CICBeginMission> enter=new NGame::CICBeginMission(810,-1,vector<string>(),game);enter->Exec();
        mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
        Require(mission,"restart fixture did not open an actual tactical mission");
        CObj<NMainLoop::CInterfaceCommand> snapshot=NMainLoop::TakeInterfaceCommandForHarness();
        Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"restart fixture lost its actual queued snapshot");snapshot->Exec();
    }
    struct Flags {bool restart=false;int operator&(CStructureSaver& saver){saver.Add(32,&restart);return 0;}} flags;
    CStructureNetworkContext missionReferences;auto missionFields=BorrowedObjectFields(mission,&missionReferences);
    {CStructureSaver reader(missionFields,CStructureSaver::READ,&missionReferences);reader.Add(1,&flags);}
    std::printf("native restart initialization: network=%d saved permission=%d expected=%d\n",int(network),int(flags.restart),int(!network));std::fflush(stdout);
    const auto* world=mission->GetWorld();const STime worldTime=world->GetAimTime()->GetValue();
    NWorld::IPlayer::CUnitSet party;mission->GetActivePlayer()->GetPlayer()->GetUnits(&party);
    NInput::CBind menu("gamemenu");
    for(bool allowed:{false,true,false}) {
        if(network && allowed)continue;
        CMemoryStream fields;
        {CStructureSaver writer(fields,CStructureSaver::WRITE);writer.Add(32,&allowed);}fields.Seek(0);
        {CStructureSaver reader(fields,CStructureSaver::READ);mission->NGame::CMissionBase::operator&(reader);}
        NInput::SEvent event;while(NInput::GetEvent(&event)){}
        auto* networkMenu=network?mission->GetInterface()->GetChildByID("net_match_menu"):nullptr;
        const bool networkWasVisible=networkMenu && networkMenu->GetStyle(NUI::STYLE_VISIBLE);
        NInput::PostEvent("gamemenu");
        Require(NInput::GetEvent(&event) && menu.ProcessEvent(event) && mission->ProcessEvent(event),"mission did not consume its actual menu binding");
        if(network) {
            Require(networkMenu && networkMenu->GetStyle(NUI::STYLE_VISIBLE)!=networkWasVisible && !NMainLoop::HaveInterfaceCommand() &&
                !networkMenu->GetChildByID("restart") && NMainLoop::GetCurrentInterfaceForHarness()==mission.GetPtr(),
                "LAN menu binding lost its own overlay or opened a local restart menu");
            std::puts("native LAN restart control: own match overlay toggled; no local snapshot/menu command");
            continue;
        }
        CObj<NMainLoop::CInterfaceCommand> command=NMainLoop::TakeInterfaceCommandForHarness();
        Require(dynamic_cast<NGame::CICInGameMenu*>(command.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"mission did not queue its actual in-game menu");command->Exec();
        auto* active=NMainLoop::GetCurrentInterfaceForHarness();Require(active && active!=mission.GetPtr(),"in-game menu was not installed");
        CStructureNetworkContext refs;auto saved=BorrowedObjectFields(active,&refs);CPtr<NUI::CWindow> page;
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(9,&page);}
        auto* restart=IsValid(page)?page->GetChildByID("restart"):nullptr;
        Require(IsValid(restart),"native restart menu has no authored restart control");
        std::printf("native restart menu: network=%d permission=%d button=%d\n",int(network),int(allowed),int(restart->GetStyle(NUI::STYLE_ENABLED)));std::fflush(stdout);
        Require(restart->GetStyle(NUI::STYLE_ENABLED)==allowed,"native mission menu ignored its saved restart permission");
        CObj<NMainLoop::CICExitModal> close=new NMainLoop::CICExitModal;close->Exec();
        NWorld::IPlayer::CUnitSet after;mission->GetActivePlayer()->GetPlayer()->GetUnits(&after);
        Require(NMainLoop::GetCurrentInterfaceForHarness()==mission.GetPtr() && !NMainLoop::HaveInterfaceCommand() && mission->GetWorld()==world && world->GetAimTime()->GetValue()==worldTime && after==party,
            "opening the restart menu changed the mission, world time or party");
    }
    Require(flags.restart==!network,"fresh mission did not preserve the correct local-versus-network restart permission");
}

static void MissionLoadingInput(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"mission loading input fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"mission loading input fixture started with an interface or pending command");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm loading input regression");
    Require(NInput::InitInput(true),"mission loading input fixture could not initialize native SDL input");
    struct InputCleanup { ~InputCleanup() { NMainLoop::DoneInterface(); NInput::DoneInput(); } } inputCleanup;
    NMainLoop::CreateProfile("UpstreamLoadingInput"); NMainLoop::SetActiveProfile("UpstreamLoadingInput");
    auto* manager = NMainLoop::GetSaveManager();
    struct SlotCleanup { NMainLoop::CSaveManager* manager; ~SlotCleanup() { manager->DeleteSlot(NMainLoop::S_SLOT_ACTIVE); } } slotCleanup{manager};
    NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(-1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginMission> enter = new NGame::CICBeginMission(810,-1,vector<string>(),game);
    enter->Exec();
    CObj<NGame::CMission> mission = dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(mission) && IsValid(mission->GetWorld()),"loading input fixture did not open a real native tactical mission");
    CObj<NMainLoop::CInterfaceCommand> snapshot = NMainLoop::TakeInterfaceCommandForHarness();
    Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) != nullptr && !NMainLoop::HaveInterfaceCommand(),
        "loading input fixture did not isolate the actual deferred restart snapshot");
    NInput::CBind released("native_loading_released"), held("native_loading_held"), once("native_loading_once");
    NInput::SetSection("native_loading_fixture");
    NGlobal::ProcessCommand(L"bind +native_loading_released 'A'");
    NGlobal::ProcessCommand(L"bind +native_loading_held 'B'");
    NInput::UpdateBinds();
    auto pump = [&]() { S2Platform::Delay(2); S2Platform::PumpEvents(); NInput::PumpMessages(true); };
    auto drain = [&]() { NInput::SEvent event; while (NInput::GetEvent(&event)) { released.ProcessEvent(event); held.ProcessEvent(event); once.ProcessEvent(event); } };
    pump(); drain();
    SDL_Event focus{}; focus.type = SDL_EVENT_WINDOW_FOCUS_GAINED;
    focus.window.windowID = SDL_GetWindowID(S2Platform::Window());
    Require(SDL_PushEvent(&focus),"loading input fixture could not queue its native focus event");
    pump(); drain();
    auto key = [&](SDL_Scancode scan,bool down) {
        SDL_Event event{}; event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
        event.key.windowID = SDL_GetWindowID(S2Platform::Window()); event.key.scancode = scan; event.key.down = down;
        Require(SDL_PushEvent(&event),"loading input fixture could not queue a native SDL key");
    };
    key(SDL_SCANCODE_A,true); key(SDL_SCANCODE_B,true); pump(); drain();
    Require(released.IsActive() && held.IsActive(),"loading input fixture did not activate its actual native bound controls");
    key(SDL_SCANCODE_A,false); NInput::PostEvent("native_loading_once");
    const STime worldTime = mission->GetWorld()->GetAimTime()->GetValue();
    NWorld::IPlayer::CUnitSet party; mission->GetActivePlayer()->GetPlayer()->GetUnits(&party);
    snapshot->Exec();
    {
        CFileStream saved; saved.OpenRead(manager->GetSlotFilePathW(NMainLoop::S_SLOT_ACTIVE,"restart.sav").c_str());
        Require(saved.GetSize() > 1024,"native restart completion did not write its real mission snapshot");
    }
    Require(!released.IsActive(),"restart snapshot completion did not consume a key release accumulated during loading");
    Require(held.IsActive(),"restart completion discarded a legitimately held native control");
    NInput::SEvent event;
    Require(!NInput::GetEvent(&event),"restart snapshot completion leaked loading-time input into the mission");
    NWorld::IPlayer::CUnitSet after; mission->GetActivePlayer()->GetPlayer()->GetUnits(&after);
    Require(mission->GetWorld()->GetAimTime()->GetValue() == worldTime && after == party,
        "clearing loading input advanced the native world or replaced the party");
    key(SDL_SCANCODE_A,true); pump();
    bool fresh = false;
    while (NInput::GetEvent(&event)) fresh = released.ProcessEvent(event) || fresh;
    Require(fresh && released.IsActive() && held.IsActive(),"finishing loading disabled subsequent native input");
    NInput::PostEvent("native_loading_once");
    Require(NInput::GetEvent(&event) && once.ProcessEvent(event),"finishing loading disabled subsequent posted input events");
    // Our normal SDL polling deliberately skips dt==0 frames. An explicit purge
    // must still consume releases; the save itself usually masks this timing.
    int sameTickPurges = 0;
    for (int attempt = 0; attempt < 128 && sameTickPurges < 8; ++attempt) {
        key(SDL_SCANCODE_A,true); pump(); drain();
        Require(released.IsActive() && held.IsActive(),"immediate input purge fixture lost its held controls");
        const auto tick = S2Platform::Milliseconds();
        NInput::PumpMessages(true);
        key(SDL_SCANCODE_A,false); NInput::PostEvent("native_loading_once");
        NInput::PurgeEvents();
        const bool sameTick = S2Platform::Milliseconds() == tick;
        const bool empty = !NInput::GetEvent(&event);
        Require(!released.IsActive() && held.IsActive() && empty,
            "explicit native input purge skipped a same-tick release or leaked a loading action");
        if (sameTick) ++sameTickPurges;
    }
    Require(sameTickPurges == 8,"immediate input purge fixture did not exercise the native same-tick polling guard");
    std::printf("native restart snapshot and %d same-tick input purges completed without advancing the world\n",sameTickPurges);
    key(SDL_SCANCODE_A,false); key(SDL_SCANCODE_B,false); pump(); drain();
    Require(!released.IsActive() && !held.IsActive(),"native loading input controls did not release after their fresh input");
    NMainLoop::DoneInterface();
}
