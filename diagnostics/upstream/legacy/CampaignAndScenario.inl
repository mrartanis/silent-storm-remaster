static void MainMenuMapScript(const char* databasePath,const char* resourcePath,bool authored = false,bool cadence = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"main-menu script fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"main-menu script fixture started with an active interface");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm menu script regression");
    auto* mapScript = NDb::GetDBScript(10);
    auto* legacyScript = NDb::GetDBScript(85);
    Require(IsValid(mapScript) && IsValid(legacyScript),"main-menu script fixture has no authored menu scripts");
    std::printf("menu script10=%zu bytes; legacy85=%zu bytes; authored=%d\n",mapScript->strCode.size(),legacyScript->strCode.size(),int(authored));
    bool authoredVariant = false;
    CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
    while (variants.MoveNext()) if (IsValid(variants.Get()->pTemplate) && variants.Get()->pTemplate->GetRecordID() == 2425 &&
        variants.Get()->pScript.GetPtr() == mapScript) authoredVariant = true;
    Require(authoredVariant,"the authored menu template does not reference the expected map script");
    struct Restore {
        CPtr<NDb::CScript> map, legacy;
        string mapCode, legacyCode;
        ~Restore() { if (IsValid(map)) map->strCode = mapCode; if (IsValid(legacy)) legacy->strCode = legacyCode; }
    } restore{mapScript,legacyScript,mapScript->strCode,legacyScript->strCode};
    const string counter = "MenuMapRuns = (MenuMapRuns or 0) + 1; MenuEnters = 0; "
        "function OnEnterZone() MenuEnters = MenuEnters + 1 end; ";
    mapScript->strCode = counter;
#if defined(S2_UPSTREAM_NATIVE_SFX)
    CObj<NFMSound::CSample2D> sample;
    CObj<NSound::CFileSample2D> loader;
#endif
    if (authored) {
        mapScript->strCode += "MenuNativeAnimationCalls = 0; MenuNativePlay = ObjectPlayAnimation; "
            "function ObjectPlayAnimation(object,animation) MenuNativeAnimationCalls = MenuNativeAnimationCalls + 1; "
            "MenuLastNativeAnimation = animation; return MenuNativePlay(object,animation) end; "+restore.mapCode;
#if defined(S2_UPSTREAM_NATIVE_SFX)
        Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr,"authored menu fixture requires headless native audio");
        Require(NFMSound::Init(NFMSound::SStartInfo()),"authored menu fixture could not initialize native audio");
        cleanup.audio = true; NFMSound::SetMusicMasterVolume(0);
        auto* ambience = NDb::GetSound(15691);
        Require(ambience != nullptr,"authored menu fixture has no ambient sound record");
        const auto wave = SilentPCMFixture(44100);
        sample = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
        Require(IsValid(sample),"authored menu fixture could not decode native PCM");
        loader = NSound::share2DSamples.Get(ambience->GetRecordID());
        (loader.GetPtr()->*CachedChapterSampleAccess::ValueMember()) = sample.GetPtr();
#else
        Require(false,"authored menu sound test requires native SFX");
#endif
    }
    legacyScript->strCode = authored ? restore.legacyCode : "MenuLegacyRuns = (MenuLegacyRuns or 0) + 1";
    CObj<NGame::CICMainMenu> open = new NGame::CICMainMenu; open->Exec();
    CObj<NGame::CMissionBase> menu = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(menu) && pSSClasses->GetObjectTypeID(menu) == int(0xB2540912),"script fixture did not open the actual native main menu");
    auto* world = dynamic_cast<NWorld::CWorld*>(menu->GetWorld());
    auto* script = world ? world->GetOwnScript() : nullptr;
    Require(IsValid(script),"the native main menu has no initialized world script");
    auto number = [&](const char* name,int expected) {
        const int top = script->GetTop(); auto value = script->GetGlobal(name);
        const bool matches = value.IsNumber() && value.GetNumber() == expected; script->SetTop(top); return matches;
    };
    auto absent = [&](const char* name) {
        const int top = script->GetTop(); const bool nil = script->GetGlobal(name).IsNil(); script->SetTop(top); return nil;
    };
    Require(number("MenuMapRuns",1),"the native main-menu initializer did not execute its variant entry exactly once");
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(1050)};
    auto frame = [&](STime time) {
        NMainLoop::SetTimeForHarness(time);
        Require(NMainLoop::StepApp(true,false,false),"native main-menu script frame requested exit");
        Require(NMainLoop::GetCurrentInterfaceForHarness() == menu.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"the main-menu script fixture changed interfaces");
    };
    if (cadence) {
        frame(1050); // Establish the renderer/world baseline before the new coroutine.
        Require(script->DoString("MenuFrameRuns = 0; while 1 do MenuFrameRuns = MenuFrameRuns + 1; Sleep(1) end") == 0,
            "menu cadence fixture could not queue its native sleeping coroutine");
        const STime frozenWorld = world->GetAimTime()->GetValue();
        for (int tick = 0; tick < 4; ++tick) frame(1050);
        Require(world->GetAimTime()->GetValue() == frozenWorld,"menu cadence fixture unexpectedly advanced its frozen world time");
        Require(absent("MenuFrameRuns"),"the main menu executed a sleeping Lua coroutine between native world segments");
        for (int tick = 0; tick < 8; ++tick) frame(1100+tick*50);
        const int top = script->GetTop(); auto value = script->GetGlobal("MenuFrameRuns");
        const bool resumed = value.IsNumber() && value.GetNumber() >= 2; script->SetTop(top);
        Require(world->GetAimTime()->GetValue() > frozenWorld && resumed,"real main-menu world segments did not resume the Lua coroutine");
        Require(number("MenuMapRuns",1) && number("MenuEnters",1),"menu cadence change repeated map initialization");
        NMainLoop::DoneInterface(); return;
    }
    for (int tick = 0; tick < 4; ++tick) frame(1050);
    if (!authored) {
        Require(absent("MenuLegacyRuns"),"the native main menu launched an extra DB script after its map entry was already initialized");
        for (int tick = 0; tick < 8; ++tick) frame(1100+tick*50);
    } else {
        auto* man = dynamic_cast<NWorld::CAnimObjectServerBase*>(world->GetObjectByName("Man"));
        Require(IsValid(man),"authored menu fixture has no animated Man object");
        bool performing = man->IsPerformingAction(); int calls = 0;
        for (int tick = 0; tick < 200; ++tick) {
            frame(1100+tick*150); performing = performing || man->IsPerformingAction();
            const int top = script->GetTop(); auto value = script->GetGlobal("MenuNativeAnimationCalls");
            calls = value.IsNumber() ? int(value.GetNumber()) : 0; script->SetTop(top);
            if (calls >= 3) break;
        }
        std::printf("authored animation calls=%d, performing=%d\n",calls,int(performing));
        Require(performing && calls == 3 && number("MenuLastNativeAnimation",2521),
            "the extra menu script duplicated the authored native Man animation sequence");
#if defined(S2_UPSTREAM_NATIVE_SFX)
        int channel = -1;
        for (int i = 0; i < 128; ++i) if (FSOUND_IsPlaying(i)) {
            Require(channel == -1,"authored menu fixture has unexpected extra native sound channels"); channel = i;
        }
        Require(channel >= 0,"authored map script10 did not start its native ambient sound");
        vector<float> output(512*2);
        Require(S2FmodRenderForTest(output.data(),512) != 0 && FSOUND_GetCurrentPosition(channel) > 0,
            "authored menu ambience did not advance real native PCM");
#endif
    }
    Require(number("MenuMapRuns",1) && number("MenuEnters",1),"native main-menu frames reran their map entry or OnEnterZone");
    NMainLoop::DoneInterface();
}

static void ChapterActiveScript(const char* databasePath,const char* resourcePath,int commandMode = 0)
{
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "active chapter fixture started with an interface or pending command");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm chapter script regression");
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"active chapter fixture has no authored map");
    struct ScriptRecordRestore {
        CPtr<NDb::CChapterMap> map; CPtr<NDb::CScript> script;
        ~ScriptRecordRestore() { if (IsValid(map)) map->pScript = script; }
    } restore{map,map->pScript};
    CObj<NDb::CScript> entry = new NDb::CScript;
    entry->strCode = "ChapterStarts = (ChapterStarts or 0) + 1; ChapterEnters = 0; "
        "function OnEnterZone() ChapterEnters = ChapterEnters + 1 end; Sleep(2); ChapterResumed = 23";
    map->pScript = entry;
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CPtr<NScenario::CScenarioZone> destination;
#if defined(S2_UPSTREAM_NATIVE_SFX)
    CObj<NFMSound::CSample2D> audioSample2D;
    CObj<NFMSound::CSample3D> audioSample3D;
    CObj<NSound::CFileSample2D> audioLoader2D;
    CObj<NSound::CFileSample3D> audioLoader3D;
#endif
    string chapterPlay;
    if (commandMode >= 6) {
#if defined(S2_UPSTREAM_NATIVE_SFX)
        Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr,"active chapter sound fixture requires headless native audio");
        Require(NFMSound::Init(NFMSound::SStartInfo()),"active chapter sound fixture could not initialize native audio");
        cleanup.audio = true; NFMSound::SetMusicMasterVolume(0);
        CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
        Require(sounds.MoveNext(),"active chapter sound fixture has no native sound record");
        auto* record = sounds.Get(); record->bLoop = true; record->nStartSamples = record->nEndingSamples = 0;
        const auto wave = SilentPCMFixture(44100);
        audioSample2D = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
        audioSample3D = NFMSound::LoadSample3D(wave.data(),static_cast<int>(wave.size()),1.f,100.f,0);
        Require(IsValid(audioSample2D) && IsValid(audioSample3D),"active chapter sound fixture could not decode its native PCM samples");
        audioLoader2D = NSound::share2DSamples.Get(record->GetRecordID());
        audioLoader3D = NSound::share3DSamples.Get(record->GetRecordID());
        (audioLoader2D.GetPtr()->*CachedChapterSampleAccess::ValueMember()) = audioSample2D.GetPtr();
        (audioLoader3D.GetPtr()->*CachedEffectSampleAccess::ValueMember()) = audioSample3D.GetPtr();
        chapterPlay = commandMode % 2 == 0 ? "PlaySound("+std::to_string(record->GetRecordID())+")" :
            "Play3DSound("+std::to_string(record->GetRecordID())+",SoundPosition)";
        entry->strCode = "ChapterHandle = "+chapterPlay+"; ChapterRequested = 1";
        if (commandMode >= 8) {
            CDBIterator<NDb::CUIHint> hints(*NDatabase::GetTable<NDb::CUIHint>());
            int hintID = 0;
            while (hints.MoveNext()) if (IsValid(hints.Get()->pString)) { hintID = hints.Get()->GetRecordID(); break; }
            Require(hintID != 0,"queued chapter sound fixture has no authored hint");
            game->hintsSet.clear();
            entry->strCode += "; ChapterHint = ShowHint("+std::to_string(hintID)+")";
        }
#else
        Require(false,"active chapter sound test requires the native SFX backend");
#endif
    } else if (commandMode == 4) {
        destination = game->pScenarioTracker->GetZoneByName("ffight");
        Require(IsValid(destination),"chapter command fixture has no authored final fight zone");
        entry->strCode = "BeginZone('ffight'); ChapterRequested = 1";
    } else if (commandMode == 5) {
        entry->strCode = "ChapterRequested = 1; uiShowTeamMngMenu()";
    } else if (commandMode != 0) {
        CDBIterator<NDb::CUIHint> hints(*NDatabase::GetTable<NDb::CUIHint>());
        int hintID = 0;
        while (hints.MoveNext()) if (IsValid(hints.Get()->pString)) { hintID = hints.Get()->GetRecordID(); break; }
        Require(hintID != 0,"chapter command fixture has no authored hint");
        game->hintsSet.clear();
        entry->strCode = "ChapterRequested = 1; ChapterHint = ShowHint(" + std::to_string(hintID) +
            "); while IsUIActionIDPresent(ChapterHint) do Sleep(1) end; ChapterCommandFinished = 9";
    }
    struct HintOptionRestore {
        NGlobal::CValue previous;
        ~HintOptionRestore() { NGlobal::SetVar("ui_showhints",previous); }
    } option{NGlobal::GetVar("ui_showhints",1)};
    if (commandMode != 0) NGlobal::SetVar("ui_showhints",commandMode == 2 || commandMode >= 8 ? 1 : 0);
    // Startup notifications would legitimately cover the chapter on its second
    // painted frame; this fixture isolates the native chapter's script pump.
    list<CPtr<NScenario::CScenarioClue>> clues; game->pScenarioTracker->GetAvailableClues(&clues);
    for (auto clue: clues) if (IsValid(clue)) clue->SetJustFound(false);
    for (auto merc: game->players.front()->mercs) { vector<CDBPtr<NDb::CMedal>> medals; merc->GetJustFoundMedals(&medals); }
    // This fixture tests command/audio lifetimes; random encounters can cover
    // the chapter while its UI is first drawn after closing a native hint.
    struct SectorRestore {
        CObj<CChapterInfo> info;
        vector<SChapterSector> original;
        ~SectorRestore() { if (IsValid(info)) info->sectorsSet.swap(original); }
    } sectors;
    CDGPtr<CPtrFuncBase<CChapterInfo>> info = shareChapterInfo.Get(map->GetRecordID());
    info.Refresh();
    sectors.info = info->GetValue();
    Require(IsValid(sectors.info),"chapter command fixture lacks its actual cached map info");
    sectors.original = sectors.info->sectorsSet;
    auto& isolated = sectors.info->sectorsSet;
    isolated.erase(std::remove_if(isolated.begin(),isolated.end(),
        [](const SChapterSector& sector) { return sector.eType == RANDOM; }),isolated.end());
    Require(!isolated.empty(),"chapter command isolation removed every authored zone and exit");
    std::printf("chapter command isolation mode=%d randomRemoved=%zu retained=%zu\n",
        commandMode,sectors.original.size()-isolated.size(),isolated.size());
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game); enter->Exec();
    CObj<NGame::CMissionBase> chapter = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* world = chapter ? dynamic_cast<NWorld::CWorld*>(chapter->GetWorld()) : nullptr;
    Require(world && IsValid(world->GetOwnScript()),"active chapter fixture did not create its native world/script");
    auto* commandUI=dynamic_cast<NUI::CChapterMapUI*>(chapter->GetInterface()->GetChildByID("chaptermapUI"));
    Require(commandUI && commandUI->GetTravelForDiagnostics().find(" type=1 template=")==string::npos,"chapter command fixture UI retained a random encounter");
    if (commandMode == 3) {
        bool tutorial = true; CMemoryStream base;
        { CStructureSaver writer(base,CStructureSaver::WRITE); writer.Add(31,&tutorial); }
        base.Seek(0);
        { CStructureSaver reader(base,CStructureSaver::READ); chapter->NGame::CMissionBase::operator&(reader); }
    }
    auto* script = world->GetOwnScript();
    if (commandMode >= 6) {
        CObj<NScript::CLUAObjectPosition> position = new NScript::CLUAObjectPosition(CVec3(0,0,0));
        NScript::luaPushCObj(script->GetState(),position); script->SetGlobal("SoundPosition");
    }
    auto number = [&](const char* name,int expected) {
        const int top = script->GetTop(); auto value = script->GetGlobal(name);
        const bool matches = value.IsNumber() && value.GetNumber() == expected; script->SetTop(top); return matches;
    };
    auto absent = [&](const char* name) {
        const int top = script->GetTop(); const bool nil = script->GetGlobal(name).IsNil(); script->SetTop(top); return nil;
    };
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(1000)};
    auto frame = [&](STime time,bool active) {
        NMainLoop::SetTimeForHarness(time);
        Require(NMainLoop::StepApp(active,false,false),"native chapter frame unexpectedly requested application exit");
        const bool stable = NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr() && !NMainLoop::HaveInterfaceCommand();
        if (!stable) {
            auto* current = NMainLoop::GetCurrentInterfaceForHarness();
            std::printf("chapter fixture mode=%d time=%u active=%s depth=%d queued=%d\n",commandMode,unsigned(time),
                current ? typeid(*current).name() : "null",NMainLoop::GetInterfaceStackDepth(),int(NMainLoop::HaveInterfaceCommand()));
            if(commandUI)std::printf("%s",commandUI->GetTravelForDiagnostics().c_str());
            if (NMainLoop::HaveInterfaceCommand()) {
                CObj<NMainLoop::CInterfaceCommand> command = NMainLoop::TakeInterfaceCommandForHarness();
                std::printf("chapter fixture unexpected command=%s\n",typeid(*command.GetPtr()).name());
            }
        }
        Require(stable,"active chapter fixture was covered by an unrelated interface command");
    };
    frame(1000,false);
    Require(world->GetAimTime()->GetValue() == 0 && absent("ChapterStarts"),"an inactive chapter executed its queued world script");
    if (commandMode >= 6) {
#if defined(S2_UPSTREAM_NATIVE_SFX)
        auto channelExcept = [&](int except) {
            int found = -1;
            for (int i = 0; i < 128; ++i) if (i != except && FSOUND_IsPlaying(i)) {
                Require(found == -1,"active chapter sound fixture has an unexpected extra native channel"); found = i;
            }
            return found;
        };
        Require(channelExcept(-1) == -1,"an inactive chapter started its scripted native audio");
        if (commandMode >= 8) {
            NMainLoop::SetTimeForHarness(1050);
            Require(NMainLoop::StepApp(true,false,false) && NMainLoop::HaveInterfaceCommand() &&
                NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr(),
                "the chapter did not queue its native hint alongside its scripted sound");
        } else frame(1050,true);
        Require(number("ChapterRequested",1),"the active chapter never executed its native Lua sound request");
        const int top = script->GetTop(); auto handle = script->GetGlobal("ChapterHandle");
        auto* soundCommand = handle.IsUserData() ? dynamic_cast<NWorld::CUICmdPlaySound*>(handle.GetUserData()) : nullptr;
        Require(soundCommand && IsValid(soundCommand->pChannel),
            "the native chapter sound dispatcher never created its scene-owned channel wrapper");
        script->SetTop(top);
        const int played = channelExcept(-1);
        Require(played >= 0,"the active chapter created a script sound wrapper but never started native PCM playback");
        Require(FSOUND_GetCurrentPosition(played) == 0,"fresh chapter script audio inherited absolute frame time as its sample offset");
        vector<float> output(512*2);
        auto render = [&]() { Require(S2FmodRenderForTest(output.data(),512) != 0,"active chapter sound fixture could not render native PCM"); };
        render();
        Require(FSOUND_GetCurrentPosition(played) > 0,"active chapter sound never advanced its real native PCM cursor");
        CObj<NFMSound::CSound2D> independent = NFMSound::PlaySound(audioSample2D);
        const int control = channelExcept(played);
        Require(IsValid(independent) && control >= 0,"active chapter sound fixture could not start its independent native control");
        const unsigned before = FSOUND_GetCurrentPosition(played), other = FSOUND_GetCurrentPosition(control);
        CObj<NGame::CICExitMenu> menu;
        if (commandMode >= 8) {
            NMainLoop::SetTimeForHarness(1100);
            Require(NMainLoop::StepApp(true,false,false),"queued chapter sound fixture could not open its native hint");
            auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
            Require(modal && pSSClasses->GetObjectTypeID(modal) == int(0x1423130),"queued chapter sound fixture opened the wrong interface");
        } else { menu = new NGame::CICExitMenu; menu->Exec(); }
        Require(NMainLoop::GetInterfaceStackDepth() == 2,"chapter sound fixture did not open its native modal");
        render();
        Require(FSOUND_IsPlaying(played) && FSOUND_GetCurrentPosition(played) == before && FSOUND_GetCurrentPosition(control) > other,
            "covering the active chapter failed to pause only its own playing audio");
        if (commandMode >= 8) {
            NInput::SEvent input; while (NInput::GetEvent(&input)) {}
            NInput::PostEvent("cancel");
            Require(NInput::GetEvent(&input) && NMainLoop::GetCurrentInterfaceForHarness()->ProcessEvent(input),
                "queued chapter sound fixture could not close its actual hint");
            NMainLoop::SetTimeForHarness(1150);
            Require(NMainLoop::StepApp(false,false,false),"queued chapter sound fixture did not restore its chapter");
        } else {
            CObj<NMainLoop::CICExitModal> close = new NMainLoop::CICExitModal; close->Exec();
        }
        render();
        Require(NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr() && FSOUND_GetCurrentPosition(played) > before,
            "returning from the native menu did not resume the chapter's real playback");
        Require(script->DoString("StopSound(ChapterHandle); ChapterStopped = 1") == 0,"chapter sound fixture could not queue its native Lua stop request");
        for (int tick = 0; tick < 8; ++tick) frame(1200+tick*50,true);
        render();
        Require(number("ChapterStopped",1) && !FSOUND_IsPlaying(played),"active chapter Lua StopSound left its actual native channel playing");
        Require(FSOUND_IsPlaying(control) && FSOUND_GetCurrentPosition(control) > other,
            "chapter StopSound interrupted the independent native control");
        Require(script->DoString(("ChapterHandle = "+chapterPlay+"; ChapterRestarted = 1").c_str()) == 0,
            "chapter sound fixture could not queue its native restart request");
        for (int tick = 0; tick < 8; ++tick) frame(1600+tick*50,true);
        const int restarted = channelExcept(control);
        Require(number("ChapterRestarted",1) && restarted >= 0,"chapter audio failed to start a new native channel after StopSound");
        render();
        Require(FSOUND_GetCurrentPosition(restarted) > 0,"restarted chapter audio did not advance native PCM");
        NMainLoop::DoneInterface(); chapter = nullptr; ClearHoldQueue(); render();
        Require(!FSOUND_IsPlaying(restarted) && FSOUND_IsPlaying(control),
            "closing the chapter leaked its scripted audio or stopped the independent control");
#endif
        return;
    }
    if (commandMode != 0) {
        auto step = [&](STime time) {
            NMainLoop::SetTimeForHarness(time);
            Require(NMainLoop::StepApp(true,false,false),"native chapter command frame requested application exit");
        };
        vector<CPtr<NWorld::CUnit>> originalUnits;
        world->GetAllUnits(&originalUnits);
        const auto originalMercs = game->players.front()->mercs;
        step(1050);
        Require(number("ChapterRequested",1),"chapter command fixture never executed its native Lua request");
        if (commandMode == 5) {
            Require(NMainLoop::HaveInterfaceCommand(),"the active chapter ignored its native Lua roster command");
            step(1100);
            auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
            Require(modal && pSSClasses->GetObjectTypeID(modal) == int(0xB0925140) && NMainLoop::GetInterfaceStackDepth() == 2,
                "the chapter roster command did not open the existing native team interface");
            NInput::SEvent input; while (NInput::GetEvent(&input)) {}
            NInput::PostEvent("cancel");
            Require(NInput::GetEvent(&input) && modal->ProcessEvent(input),"the native chapter roster did not consume its close event");
            for (int tick = 0; tick < 12; ++tick) step(1200+tick*50);
            vector<CPtr<NWorld::CUnit>> afterUnits; world->GetAllUnits(&afterUnits);
            Require(afterUnits == originalUnits && game->players.front()->mercs == originalMercs,
                "closing the native chapter roster duplicated or replaced the unchanged deployed party");
            vector<CPtr<NGame::IUnitTracker>> tracked; chapter->GetUnits(&tracked);
            Require(tracked.size() == originalUnits.size() && !tracked.empty(),
                "chapter roster dispatch left deployed party units without native player trackers");
            Require(NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr() && !NMainLoop::HaveInterfaceCommand(),
                "the closed native roster did not return to the original chapter");
        } else if (commandMode == 4) {
            Require(NMainLoop::HaveInterfaceCommand(),"the active chapter ignored its native Lua BeginZone transition");
            CObj<NMainLoop::CInterfaceCommand> queued = NMainLoop::TakeInterfaceCommandForHarness();
            auto* begin = dynamic_cast<NGame::CICBeginMission*>(queued.GetPtr());
            Require(begin && (begin->*BeginMissionCommandAccess::Zone()) == destination &&
                (begin->*BeginMissionCommandAccess::Game()) == game && (begin->*BeginMissionCommandAccess::Template()) == -1,
                "chapter BeginZone lost its actual scenario destination, campaign or default template");
            Require(!(begin->*BeginMissionCommandAccess::EmulatesChapter()),
                "chapter BeginZone would repeat tactical leave-zone healing/scenario/medal bookkeeping");
            Require((begin->*BeginMissionCommandAccess::Image()) == map->pPWLImage,
                "scripted chapter entry discarded the remaster's chapter loading background");
            Require(!NMainLoop::HaveInterfaceCommand(),"one chapter BeginZone queued multiple transitions");
            CObj<ChapterZoneDrainMission> tactical = new ChapterZoneDrainMission;
            tactical->Attach(world,game,chapter->GetDesktop());
            world->AddUICommand(new NWorld::CUICmdBeginZone("ffight")); tactical->Drain();
            queued = NMainLoop::TakeInterfaceCommandForHarness(); begin = dynamic_cast<NGame::CICBeginMission*>(queued.GetPtr());
            Require(begin && (begin->*BeginMissionCommandAccess::Zone()) == destination &&
                (begin->*BeginMissionCommandAccess::Game()) == game && (begin->*BeginMissionCommandAccess::EmulatesChapter()),
                "sharing BeginZone dispatch broke the existing tactical leave-zone transition");
            Require(!NMainLoop::HaveInterfaceCommand(),"one tactical BeginZone queued multiple transitions");
        } else {
            const int top = script->GetTop(); auto hint = script->GetGlobal("ChapterHint");
            Require(hint.IsNumber(),"the actual chapter ShowHint binding returned no wait ID");
            const int eventID = int(hint.GetNumber()); script->SetTop(top);
            Require(script->IsUIActionIDPresent(eventID) && absent("ChapterCommandFinished"),
                "chapter hint bypassed its actual asynchronous UI wait");
            if (commandMode >= 2) {
                Require(NMainLoop::HaveInterfaceCommand(),"the active chapter ignored its native Lua hint command");
                step(1100);
                auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
                Require(modal && pSSClasses->GetObjectTypeID(modal) == int(0x1423130) && NMainLoop::GetInterfaceStackDepth() == 2,
                    "chapter ShowHint did not open the existing native hint interface");
                CStructureNetworkContext references; auto fields = BorrowedObjectFields(modal,&references);
                CPtr<NGame::IMission> origin; int modalEvent = -1;
                { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(2,&origin); reader.Add(3,&modalEvent); }
                Require(origin.GetPtr() == chapter.GetPtr() && modalEvent == eventID,
                    "the native chapter hint lost its origin or pending script event");
                const STime coveredTime = world->GetAimTime()->GetValue(); step(1150); step(1200);
                Require(world->GetAimTime()->GetValue() == coveredTime && script->IsUIActionIDPresent(eventID),
                    "the covered chapter advanced its world or prematurely released its hint wait");
                NInput::SEvent input; while (NInput::GetEvent(&input)) {}
                NInput::PostEvent("cancel");
                Require(NInput::GetEvent(&input) && modal->ProcessEvent(input),"native chapter hint did not consume its close event");
                step(1250);
                Require(NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr() && NMainLoop::GetInterfaceStackDepth() == 1,
                    "closing the hint did not restore its actual chapter interface");
            } else Require(!NMainLoop::HaveInterfaceCommand(),"disabled chapter hints opened a modal");
            for (int tick = 0; tick < 12; ++tick) step(1300+tick*50);
            Require(!script->IsUIActionIDPresent(eventID) && number("ChapterCommandFinished",9),
                "the chapter hint command never released its real Lua UI wait");
        }
        Require(game->nChapterMapID == map->GetRecordID() && game->players.size() == 1,
            "chapter command dispatch changed campaign destination or party before a transition");
        NMainLoop::DoneInterface(); return;
    }
    frame(1050,true);
    Require(number("ChapterStarts",1) && world->GetAimTime()->GetValue() > 0,
        "the real active chapter Step never advanced its world or executed the assigned Lua script");
    Require(absent("ChapterResumed"),"the chapter script bypassed its native Sleep segment delay");
    const STime covered = world->GetAimTime()->GetValue();
    frame(1100,false); frame(1150,false);
    Require(world->GetAimTime()->GetValue() == covered && absent("ChapterResumed"),"inactive frames advanced a sleeping chapter's simulation");
    chapter->PauseGame(true); frame(1200,true); frame(1250,true);
    Require(world->GetAimTime()->GetValue() == covered && absent("ChapterResumed"),"active paused chapter frames advanced their sleeping world");
    chapter->PauseGame(false);
    for (int tick = 0; tick < 8; ++tick) frame(1300+tick*50,true);
    Require(number("ChapterResumed",23) && number("ChapterStarts",1) && number("ChapterEnters",1),
        "active chapter frames did not resume Sleep or reran initialization/OnEnterZone");
    Require(game->bChapterMapSet && game->nChapterMapID == map->GetRecordID() && game->players.size() == 1,
        "the native chapter pump changed campaign destination or party");
    NMainLoop::DoneInterface();
}

static void ChapterClockStep(const char* databasePath,const char* resourcePath,const char* legacyPath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NGScene::Is3DActive(),
        "chapter clock fixture needs an inactive native chapter with no render device");
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); } } cleanup;
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(1000)};
    int chapterID = 0;
    { CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
      while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { chapterID = maps.Get()->GetRecordID(); break; } }
    Require(chapterID != 0,"chapter clock fixture has no authored chapter map");
    auto roundTrip = [&](CObj<NRPG::CGlobalGame>& game,CObj<NGame::CMissionBase>& chapter) {
        CMemoryStream snapshot;
        { CStructureSaver writer(snapshot,CStructureSaver::WRITE); writer.Add(2,&game); writer.Add(3,&chapter); }
        snapshot.Seek(0);
        CObj<NRPG::CGlobalGame> loadedGame; CObj<NGame::CMissionBase> loaded;
        { CStructureSaver reader(snapshot,CStructureSaver::READ); reader.Add(2,&loadedGame); reader.Add(3,&loaded); }
        Require(IsValid(loadedGame) && IsValid(loaded) && loaded->GetWorld() != chapter->GetWorld(),
            "chapter clock snapshot did not restore an independent complete graph");
        NMainLoop::DoneInterface(); chapter = loaded; game = loadedGame;
        chapter->OnSnapshotRestored();
    };
    // Fresh chapter; actual old null-world graph; previous complete fork save
    // with a zero interval; and a saved custom nonzero interval to retain.
    for (int variant = 0; variant < 4; ++variant) {
        NMainLoop::DoneInterface(); NMainLoop::SetTimeForHarness(1000);
        CObj<NRPG::CGlobalGame> game; CObj<NGame::CMissionBase> chapter;
        const STime interval = variant == 3 ? 7 : 5;
        if (variant == 1) {
            CFileStream legacy; legacy.OpenRead(legacyPath);
            { CStructureSaver reader(legacy,CStructureSaver::READ); reader.Add(2,&game); reader.Add(3,&chapter); }
            Require(IsValid(chapter) && !chapter->GetWorld(),"chapter clock fixture did not load its historical null-world graph");
            chapter->OnSnapshotRestored();
        } else {
            game = NRPG::CreateGlobalGame(1);
            game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
            CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(chapterID,game);
            enter->Exec();
            chapter = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
            if (variant >= 2) {
                STime savedInterval = variant == 2 ? 0 : interval;
                CMemoryStream base;
                { CStructureSaver writer(base,CStructureSaver::WRITE); writer.Add(36,&savedInterval); }
                base.Seek(0);
                { CStructureSaver reader(base,CStructureSaver::READ); chapter->NGame::CMissionBase::operator&(reader); }
                roundTrip(game,chapter);
            }
        }
        Require(IsValid(chapter) && chapter->GetWorld() && chapter->GetWorld()->GetAimTime()->GetValue() == 0,
            "chapter clock fixture lacks its native world or ran a tactical warm-up");
        auto* ui = dynamic_cast<NUI::CChapterMapUI*>(chapter->GetInterface()->GetChildByID("chaptermapUI"));
        Require(ui != nullptr,"chapter clock fixture lost its actual saved chapter UI");
        const string travel = ui->GetTravelForDiagnostics();
        const int mapID = game->nChapterMapID, difficulty = game->nCurrentChapterDifficulty;
        const auto mercs = game->players.front()->mercs;
        auto step = [&](STime time) { NMainLoop::SetTimeForHarness(time); chapter->Step(); };
        step(1000);
        Require(chapter->GetGameTime() == 0 && chapter->GetUITime() == 0,"the first chapter clock step jumped instead of establishing a baseline");
        step(1003);
        Require(chapter->GetGameTime() == 0 && chapter->GetUITime() == 0,"the chapter frame gate accepted an update before its saved minimum interval");
        step(1000+interval);
        Require(chapter->GetGameTime() == interval && chapter->GetUITime() == interval,
            "the native chapter Step did not advance its actual game and interface clocks");
        chapter->PauseGame(true); step(1000+2*interval);
        Require(chapter->GetGameTime() == interval && chapter->GetUITime() == 2*interval,
            "pausing the native chapter froze its interface clock or advanced its game clock");
        chapter->PauseGame(false); step(1000+4*interval);
        Require(chapter->GetGameTime() == 3*interval && chapter->GetUITime() == 4*interval,
            "resuming the native chapter replayed paused time or lost its resumed interval");
        Require(game->nChapterMapID == mapID && game->nCurrentChapterDifficulty == difficulty && game->players.front()->mercs == mercs &&
            chapter->GetWorld()->GetAimTime()->GetValue() == 0 && ui->GetTravelForDiagnostics() == travel,
            "inactive chapter clock bookkeeping changed its world, campaign, party or saved travel");
        roundTrip(game,chapter);
        Require(chapter->GetGameTime() == 3*interval && chapter->GetUITime() == 4*interval,
            "loading a complete chapter reset its saved interface/game clock values");
        step(2000);
        Require(chapter->GetGameTime() == 3*interval && chapter->GetUITime() == 4*interval,
            "the restored chapter replayed an unsaved wall-time gap");
        step(2000+interval);
        Require(chapter->GetGameTime() == 4*interval && chapter->GetUITime() == 5*interval &&
            chapter->GetWorld()->GetAimTime()->GetValue() == 0,
            "the restored native chapter clocks failed to continue without simulating the inactive world");
    }
    NMainLoop::DoneInterface();
}

static void ChapterFocusAudio(const char* databasePath,const char* resourcePath)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr,"chapter-focus fixture requires headless native audio");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "chapter-focus fixture started with an active interface or pending command");
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    Require(NFMSound::Init(NFMSound::SStartInfo()),"chapter-focus fixture could not initialize native audio");
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); ClearHoldQueue(); NFMSound::Done(); } } cleanup;
    NFMSound::SetMusicMasterVolume(0);
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"chapter-focus fixture has no authored chapter map");
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game);
    enter->Exec();
    CObj<NGame::CMissionBase> chapter = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(chapter) && chapter->GetSoundScene(),"chapter-focus fixture did not open its actual native chapter");
    CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
    Require(sounds.MoveNext(),"chapter-focus fixture has no native sound record");
    auto* record = sounds.Get(); record->bLoop = false; record->nStartSamples = record->nEndingSamples = 0;
    const auto wave = SilentPCMFixture(44100);
    CObj<NFMSound::CSample2D> sample = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
    Require(IsValid(sample),"chapter-focus fixture could not decode its PCM sample");
    CObj<NSound::CFileSample2D> loader = NSound::share2DSamples.Get(record->GetRecordID());
    (loader.GetPtr()->*CachedChapterSampleAccess::ValueMember()) = sample.GetPtr();
    CObj<NSound::ISound2D> sound = chapter->GetSoundScene()->Add2DSound(record);
    CObj<ICamera> camera = ::CreateCamera(CAMERA_PC);
    CTransformStack transform; camera->GetTransform(&transform,CVec2(1024,768));
    chapter->GetSoundScene()->Draw(&transform);
    auto findChannel = [&](int except) {
        int found = -1;
        for (int i = 0; i < 128; ++i) if (i != except && FSOUND_IsPlaying(i)) {
            Require(found == -1,"chapter-focus fixture has unexpected extra channels"); found = i;
        }
        return found;
    };
    const int channel = findChannel(-1);
    Require(IsValid(sound) && sound->IsPlaying() && channel >= 0,"chapter scene did not start an actual native PCM channel");
    CObj<NFMSound::CSound2D> unrelated = NFMSound::PlaySound(sample);
    const int control = findChannel(channel);
    Require(IsValid(unrelated) && control >= 0,"chapter-focus fixture did not start its unrelated audio control");
    vector<float> output(512*2);
    auto render = [&]() { Require(S2FmodRenderForTest(output.data(),512) != 0,"chapter-focus fixture could not render native PCM"); };
    render();
    Require(FSOUND_GetCurrentPosition(channel) > 0,"chapter-focus fixture has no observable live playback before its menu");
    for (int pass = 0; pass < 2; ++pass) {
        const unsigned before = FSOUND_GetCurrentPosition(channel), other = FSOUND_GetCurrentPosition(control);
        CObj<NGame::CICExitMenu> menu = new NGame::CICExitMenu;
        menu->Exec();
        Require(NMainLoop::GetInterfaceStackDepth() == 2 && NMainLoop::GetCurrentInterfaceForHarness() != chapter.GetPtr(),
            "chapter-focus fixture did not cover the chapter with its actual native menu");
        render();
        Require(FSOUND_IsPlaying(channel) && FSOUND_GetCurrentPosition(channel) == before && FSOUND_GetCurrentPosition(control) > other,
            "covering a chapter did not freeze only its own live audio channel");
        CObj<NMainLoop::CICExitModal> close = new NMainLoop::CICExitModal;
        close->Exec();
        Require(NMainLoop::GetInterfaceStackDepth() == 1 && NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr(),
            "closing the native menu did not return focus to the chapter");
        render();
        Require(FSOUND_IsPlaying(channel) && FSOUND_GetCurrentPosition(channel) > before,
            "returning from a native menu left the chapter audio channel permanently paused");
    }
    Require(game->bChapterMapSet && game->nChapterMapID == map->GetRecordID() && game->players.size() == 1,
        "chapter focus changed campaign destination or party");
    NMainLoop::DoneInterface();
#else
    Require(false,"chapter-focus test requires the native SFX backend");
#endif
}

static void ChapterScriptInitialization(const char* databasePath,const char* resourcePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "chapter-script fixture started with an active interface or pending command");
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"chapter-script fixture has no authored chapter-map record");
    struct ScriptRecordRestore {
        CPtr<NDb::CChapterMap> map; CPtr<NDb::CScript> script;
        ~ScriptRecordRestore() { if (IsValid(map)) map->pScript = script; }
    } restore{map,map->pScript};
    CObj<NDb::CScript> entry = new NDb::CScript;
    entry->strCode = "UpstreamChapterInit = 17; UpstreamChapterEnter = 0; "
        "function OnEnterZone() UpstreamChapterEnter = UpstreamChapterEnter + 1 end";
    CObj<NScript::CScript> foreign = new NScript::CScript;
    foreign->PushNumber(99);
    struct GlobalScriptRestore { CPtr<NScript::CScript> previous; ~GlobalScriptRestore() { pScript = previous; } } globalRestore{pScript};
    auto number = [&](NScript::CScript* script,const char* name,int expected) {
        const int top = script->GetTop(); auto value = script->GetGlobal(name);
        const bool matches = value.IsNumber() && value.GetNumber() == expected;
        script->SetTop(top); return matches;
    };
    for (bool withScript: {false,true}) {
        map->pScript = withScript ? entry.GetPtr() : nullptr;
        pScript = foreign;
        CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
        game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game);
        enter->Exec();
        CObj<NGame::CMissionBase> mission = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
        auto* world = mission ? dynamic_cast<NWorld::CWorld*>(mission->GetWorld()) : nullptr;
        Require(world && IsValid(world->GetOwnScript()),"chapter-script fixture did not create its actual native world and Lua state");
        auto* own = world->GetOwnScript();
        Require(own != foreign.GetPtr() && own->pWorld == world && own->GetGlobalGame() == game.GetPtr(),
            "the chapter script was not bound to its own native world/global-game context");
        Require(world->GetAimTime()->GetValue() == 0,"chapter script initialization ran the tactical first-segments warm-up");
        if (withScript) {
            const int top = own->GetTop(); auto pending = own->GetGlobal("UpstreamChapterInit");
            Require(pending.IsNil(),"chapter initialization eagerly executed its queued Lua thread before the first live segment");
            own->SetTop(top);
            // DoString queues the body. Segment pumps it, then queues OnEnterZone;
            // that callback runs when the next native segment pumps Lua again.
            world->UpdateWorld(0,nullptr);
            Require(number(own,"UpstreamChapterInit",17),"the native chapter never evaluated its assigned ScriptID entry point");
            Require(number(own,"UpstreamChapterEnter",0),"the chapter bypassed the native deferred OnEnterZone scheduling");
            world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
            Require(number(own,"UpstreamChapterEnter",1),"the chapter did not execute its deferred OnEnterZone callback on the next native segment");
            world->UpdateWorld(world->GetAimTime()->GetValue()+50,nullptr);
            Require(number(own,"UpstreamChapterEnter",1) && number(own,"UpstreamChapterInit",17),
                "later world segments reran chapter initialization or the OnEnterZone one-shot");
        }
        const int top = foreign->GetTop(); auto other = foreign->GetGlobal("UpstreamChapterInit");
        Require(other.IsNil() && top == 1,"chapter initialization executed in an unrelated Lua state's globals or changed its stack");
        foreign->SetTop(top);
        Require(game->bChapterMapSet && game->nChapterMapID == map->GetRecordID() && game->players.size() == 1,
            "chapter-script setup changed campaign destination or party");
        NMainLoop::DoneInterface(); mission = nullptr;
    }
}

static void ChapterWorldGraph(const char* databasePath,const char* resourcePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "chapter-world fixture started with an active interface or pending command");
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"chapter-world fixture has no authored campaign chapter");
    const int chapterID = map->GetRecordID();
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game);
    enter->Exec();
    CObj<NGame::CMissionBase> mission = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(mission) && pSSClasses->GetObjectTypeID(mission) == int(0xB2301230),
        "chapter-world fixture did not open the actual native campaign chapter");
    auto check = [&](NGame::CMissionBase* chapter,NRPG::CGlobalGame* global) {
        auto* world = chapter->GetWorld();
        Require(IsValid(world),"the native chapter initialized without the world required by its saved mission state");
        Require(world->GetGlobalGame() == global && IsValid(world->GetAIMap()) && IsValid(world->GetPathNetwork()),
            "the native chapter world lost its global-game or AI/path infrastructure");
        auto* player = chapter->GetActivePlayer();
        Require(IsValid(player) && player->GetGlobalPlayer() == global->players.front() && IsValid(player->GetPlayer()) &&
            IsValid(player->GetCommander()) && IsValid(player->GetCamera()),
            "the native chapter did not retain its real world player, commander and player camera");
        Require(IsValid(chapter->GetScene()) && IsValid(chapter->GetRenderGame()) && IsValid(chapter->GetEnemyTurnCamera()) &&
            chapter->GetEnemyTurnCamera() != player->GetCamera(),"the native chapter did not own its scene/render and separate cinematic camera");
        auto* ui = chapter->GetInterface();
        Require(IsValid(ui) && IsValid(chapter->GetSoundScene()) && ui->GetSound() == chapter->GetSoundScene(),
            "the native chapter interface did not share its world-clock sound scene");
        Require(IsValid(chapter->GetDesktop()) && chapter->GetDesktop() == ui->GetChildByID("chaptermapUI"),
            "the native chapter UI was absent from the mission desktop stack");
        vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
        int aliveMercs = 0;
        for (const auto& merc: global->players.front()->mercs) if (!merc->IsDead()) ++aliveMercs;
        Require(int(units.size()) == aliveMercs && global->players.size() == 1 && global->bChapterMapSet && global->nChapterMapID == chapterID,
            "initializing the chapter world changed campaign destination or the deployed party roster");
        for (const auto& unit: units) {
            auto* server = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
            Require(server && std::find(global->players.front()->mercs.begin(),global->players.front()->mercs.end(),
                server->GetUnitRPG()->GetRPGUnit()) != global->players.front()->mercs.end(),
                "the native chapter world did not retain its actual global-party RPG units");
        }
    };
    check(mission,game);
    CMemoryStream snapshot;
    { CStructureSaver writer(snapshot,CStructureSaver::WRITE); writer.Add(2,&game); writer.Add(3,&mission); }
    CObj<NRPG::CGlobalGame> restoredGame;
    CObj<NGame::CMissionBase> restored;
    snapshot.Seek(0);
    { CStructureSaver reader(snapshot,CStructureSaver::READ); reader.Add(2,&restoredGame); reader.Add(3,&restored); }
    Require(IsValid(restoredGame) && IsValid(restored) && restored.GetPtr() != mission.GetPtr() &&
        restored->GetWorld() != mission->GetWorld(),"chapter-world native snapshot did not restore an independent object graph");
    check(restored,restoredGame);
    const STime before = restored->GetWorld()->GetAimTime()->GetValue();
    restored->GetWorld()->UpdateWorld(before+20,nullptr);
    Require(restored->GetWorld()->GetAimTime()->GetValue() > before,
        "the restored native chapter world could not resume its simulation clock");
    NMainLoop::DoneInterface();
    restored = nullptr; restoredGame = nullptr; mission = nullptr;
}

static void GlobalSectorDescription(const char* databasePath,const char* resourcePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    NDb::CUIContainer* sectorTemplate = nullptr;
    NDb::CUIControl* descriptionControl = nullptr;
    CDBIterator<NDb::CUIContainer> templates(*NDatabase::GetTable<NDb::CUIContainer>());
    while (templates.MoveNext()) {
        auto* candidate = templates.Get();
        bool normal = false, highlighted = false, normalZone = false, disabledZone = false;
        NDb::CUIControl* description = nullptr;
        for (const auto& control: candidate->controls) if (IsValid(control)) {
            normal |= control->szID == "text_normal"; highlighted |= control->szID == "text_hilighted";
            normalZone |= control->szID == "zone_normal"; disabledZone |= control->szID == "zone_disabled";
            if (control->szID == "description") description = control;
        }
        if (normal && highlighted && normalZone && description && description->type == NDb::UI_TEXT &&
            description->rect.Width() > 8 && !NUI::GetDBString(description->pString).empty()) {
            sectorTemplate = candidate; descriptionControl = description; break;
        }
    }
    Require(sectorTemplate && descriptionControl,"global description fixture has no authored sector template and localized panel caption");
    const auto caption = NUI::GetDBString(descriptionControl->pString);
    const auto originalRect = descriptionControl->rect;
    std::printf("native global description template=%d control=%d string=%d\n",sectorTemplate->GetRecordID(),
        descriptionControl->GetRecordID(),descriptionControl->pString->GetRecordID());
    const float oldScale = S2Platform::Display().uiScale;
    struct DisplayCleanup { float scale; ~DisplayCleanup() { S2Platform::UpdateDisplay(scale*100); } } cleanup{oldScale};
    for (float scale: {1.f,1.5f,2.f}) {
        S2Platform::UpdateDisplay(scale*100); MarkNewDGFrame();
        CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false);
        CObj<NUI::CInterface> interface = new NUI::CInterface(cursor);
        CObj<NUI::CWindow> sector = dynamic_cast<NUI::CWindow*>(pSSClasses->CreateObject(int(0xB0912172)));
        Require(IsValid(sector),"global description fixture could not create its native zone-sector control");
        sector->SetInterface(interface); sector->SetPosition(NUI::SPoint(0,0)); sector->SetSize(NUI::SPoint(1024,768));
        NUI::LoadTemplate(sector,sectorTemplate);
        auto* description = sector->GetChildByID("description");
        Require(IsValid(description),"native zone-sector template did not create the description panel");
        list<CPtr<NUI::CWindow>> children; description->GetChildrenList(&children);
        NUI::CText* text = dynamic_cast<NUI::CText*>(description);
        int captions = text && text->GetText() == caption ? 1 : 0;
        for (const auto& child: children) if (auto* candidate = dynamic_cast<NUI::CText*>(child.GetPtr())) {
            if (candidate->GetText() == caption) { text = candidate; ++captions; }
        }
        Require(captions == 1 && text,
            "the native global-sector description panel lost its authored localized caption");
        NUI::SPoint measured; text->GetRealSize(&measured);
        std::printf("native description scale=%.1f panel=(%d,%d) text=(%d,%d) measured=(%d,%d)\n",double(scale),
            description->GetSize().x,description->GetSize().y,text->GetSize().x,text->GetSize().y,measured.x,measured.y);
        Require(measured.y > 0 && text->GetSize().y == measured.y && description->GetSize().y == measured.y+8,
            "the native global-sector description kept its fixed height instead of fitting its actual caption and border");
        Require(text->GetPosition() == NUI::SPoint(4,4) && text->GetSize().x == description->GetSize().x-8 &&
            text->GetStyle(NUI::STYLE_VISIBLE) && text->GetText() == caption,
            "the global-sector caption lost its inset, width, visibility or authored font markup");
        Require(descriptionControl->rect.x1 == originalRect.x1 && descriptionControl->rect.y1 == originalRect.y1 &&
            descriptionControl->rect.x2 == originalRect.x2 && descriptionControl->rect.y2 == originalRect.y2 && NUI::GetDBString(descriptionControl->pString) == caption,
            "building the global-sector caption modified the authored UI rectangle or localized text");
    }
}

static void GlobalMapReturnDestination(const char* databasePath,const char* resourcePath,bool missingRecord = false,bool retainFailedChapter = false)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "global-map return fixture must begin without interfaces or queued commands");
    struct InterfaceCleanup { ~InterfaceCleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CDBIterator<NDb::CGlobalMap> maps(*NDatabase::GetTable<NDb::CGlobalMap>());
    NDb::CGlobalMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pScenario)) { map = maps.Get(); break; }
    Require(map != nullptr, "global-map return fixture has no authored campaign map");
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(map->pScenario->GetRecordID());
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    game->bGlobalMapSet = true; game->nGlobalMapID = map->GetRecordID();
    game->bChapterMapSet = true; game->nChapterMapID = 987654;
    game->vChapterPos = CVec2(12,34);
    CObj<NGame::CICShowGlobal> overview = new NGame::CICShowGlobal(game);
    overview->Exec();
    auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(modal) && pSSClasses->GetObjectTypeID(modal) == int(0xB1808180) && game->bChapterMapSet,
        "modal global-map overview did not preserve its chapter return destination");
    if (missingRecord) {
        const int absentID = 987654;
        Require(!IsValid(NDb::GetGlobalMap(absentID)),"missing-map fixture record unexpectedly exists");
        CObj<NRPG::CGlobalGame> invalid = NRPG::CreateGlobalGame(map->pScenario->GetRecordID());
        invalid->players = game->players; invalid->bGlobalMapSet = true;
        invalid->nGlobalMapID = absentID;
        invalid->bChapterMapSet = retainFailedChapter;
        invalid->nChapterMapID = 24680; invalid->vChapterPos = CVec2(7,8);
        for (bool overviewMode: {true,false}) {
            std::printf("native missing-map control: valid=%d absent=%d overview=%d existing-interface=%d\n",
                map->GetRecordID(),absentID,int(overviewMode),int(NMainLoop::GetCurrentInterfaceForHarness() == modal));
            std::fflush(stdout);
            if (overviewMode) {
                CObj<NGame::CICShowGlobal> command = new NGame::CICShowGlobal(invalid);
                command->Exec();
            } else {
                CObj<NGame::CICContinueGlobal> command = new NGame::CICContinueGlobal(invalid);
                command->Exec();
            }
            Require(NMainLoop::GetCurrentInterfaceForHarness() == modal && !NMainLoop::HaveInterfaceCommand() &&
                game->bGlobalMapSet && game->nGlobalMapID == map->GetRecordID() && game->bChapterMapSet &&
                game->nChapterMapID == 987654 && game->vChapterPos == CVec2(12,34) &&
                invalid->bGlobalMapSet && invalid->nGlobalMapID == absentID,
                "a missing global-map record replaced the active interface or changed campaign state");
            if (retainFailedChapter) {
                std::printf("native failed-map return: overview=%d retained-chapter=%d\n",
                    int(overviewMode),int(invalid->bChapterMapSet));
                Require(invalid->bChapterMapSet && invalid->nChapterMapID == 24680 && invalid->vChapterPos == CVec2(7,8),
                    "failed global-map continuation discarded the existing chapter return destination");
            }
        }
        if (retainFailedChapter) {
            invalid->bGlobalMapSet = false;
            CObj<NGame::CICContinueGlobal> command = new NGame::CICContinueGlobal(invalid);
            command->Exec();
            Require(invalid->bChapterMapSet && invalid->nChapterMapID == 24680 && invalid->vChapterPos == CVec2(7,8) &&
                NMainLoop::GetCurrentInterfaceForHarness() == modal && !NMainLoop::HaveInterfaceCommand(),
                "continuation without a global-map destination discarded its current chapter or interface");
        }
    }
    NMainLoop::DoneInterface();
    CObj<NGame::CICContinueGlobal> leaveChapter = new NGame::CICContinueGlobal(game);
    leaveChapter->Exec();
    auto* active = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(active) && pSSClasses->GetObjectTypeID(active) == int(0xB1808180),
        "continuing the campaign did not open the actual native global map");
    Require(!game->bChapterMapSet,
        "continuing to the global map retained the old chapter return destination");
    Require(game->bGlobalMapSet && game->nGlobalMapID == map->GetRecordID() &&
        game->nChapterMapID == 987654 && game->vChapterPos == CVec2(12,34) && game->players.size() == 1,
        "clearing the chapter return flag also changed saved map IDs, position or party");
}

static void CheckChapterMarkerToolTipTransitions(NUI::CWindow* marker,NUI::CText* text);
static void CheckChapterExitMarkerHint(NGame::CMissionBase* mission,NUI::CWindow* chapter,NUI::CWindow* marker,NUI::CText* text);
static void CheckChapterClueHeading(NGame::CMissionBase* mission,NUI::CWindow* chapter);

static void ChapterTeamMarkerToolTip(const char* databasePath,const char* resourcePath,int transitions = 0)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "chapter marker fixture must begin without interfaces or queued commands");
    struct InterfaceCleanup { ~InterfaceCleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) {
        if (transitions == 3) {
            CObj<CChapterInfoLoader> loader = new CChapterInfoLoader;
            loader->SetKey(maps.Get()->GetRecordID());
            CDGPtr<CPtrFuncBase<CChapterInfo>> info = loader.GetPtr(); info.Refresh();
            bool usable = false;
            for (const auto& sector : info->GetValue()->sectorsSet) {
                if (sector.eType != ZONE || sector.nDescriptionID == -1 || sector.pointsSet.empty()) continue;
                auto* zone = game->pScenarioTracker->GetZoneByDBZone(NDb::GetDBScenarioZone(sector.nTemplate));
                if (!IsValid(zone)) continue;
                for (auto clue : zone->GetClues())
                    if (!clue->IsCompound() && clue->IsPlaced() && !clue->IsDestroyed() &&
                        !clue->GetParentZones().empty() && clue->GetParentZones().front() == zone &&
                        !NUI::GetDBString(clue->GetDBClue()->pDescription).empty()) usable = true;
            }
            if (!usable) continue;
        }
        map = maps.Get(); break;
    }
    Require(map != nullptr, "chapter marker fixture has no authored chapter map");
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game);
    enter->Exec();
    auto* mission = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(mission) && pSSClasses->GetObjectTypeID(mission) == int(0xB2301230) && mission->GetInterface(),
        "chapter marker fixture did not open the actual native chapter interface");
    auto* chapter = mission->GetInterface()->GetChildByID("chaptermapUI");
    auto* view = chapter ? chapter->GetChildByID("view") : nullptr;
    auto* marker = view ? view->GetChildByID("action") : nullptr;
    Require(IsValid(marker), "chapter marker fixture did not create its native party marker");
    auto* tooltip = marker->GetToolTip();
    Require(IsValid(tooltip), "the chapter party marker never created its saved tooltip");
    Require(tooltip->GetParent() == mission->GetInterface() && tooltip->GetStyle(NUI::STYLE_TOPMOST),
        "the chapter marker tooltip is not owned by the native topmost interface");
    CStructureNetworkContext markerRefs;
    auto markerFields = BorrowedObjectFields(marker,&markerRefs);
    CPtr<NUI::CToolTip> retained;
    { CStructureSaver reader(markerFields,CStructureSaver::READ,&markerRefs); reader.Add(11,&retained); }
    Require(retained.GetPtr() == tooltip, "the chapter marker did not retain the same tooltip in its existing saved field");
    CStructureNetworkContext tooltipRefs;
    auto tooltipFields = BorrowedObjectFields(tooltip,&tooltipRefs);
    CPtr<NUI::CText> text;
    { CStructureSaver reader(tooltipFields,CStructureSaver::READ,&tooltipRefs); reader.Add(2,&text); }
    Require(IsValid(text) && !NUI::GetDBString(11124).empty() && text->GetText() == NUI::GetDBString(11124),
        "the chapter marker tooltip lost its authored initial text");
    if (transitions == 1) CheckChapterMarkerToolTipTransitions(marker,text);
    if (transitions == 2) CheckChapterExitMarkerHint(mission,chapter,marker,text);
    if (transitions == 3) CheckChapterClueHeading(mission,chapter);
    Require(game->bChapterMapSet && game->nChapterMapID == map->GetRecordID() && game->players.size() == 1,
        "creating a chapter marker tooltip changed the campaign destination or party");
}

static void MainMenuLoadSaveTab(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr, "main-menu load fixture needs isolated user data");
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "main-menu load fixture started with an active interface or command");
    struct InterfaceCleanup { ~InterfaceCleanup() { NMainLoop::DoneInterface(); } } cleanup;
    auto saveTab = [&]() {
        auto* active = NMainLoop::GetCurrentInterfaceForHarness();
        Require(IsValid(active), "native load command did not open a modal interface");
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(active,&references);
        int type = -1;
        CPtr<NUI::CWindow> menu;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(3,&type); reader.Add(6,&menu); }
        Require(type == NGame::LOAD && IsValid(menu), "native load command opened the wrong mode or lost its dialog");
        CStructureNetworkContext menuReferences;
        auto menuFields = BorrowedObjectFields(menu,&menuReferences);
        CPtr<NUI::CWindow> view;
        { CStructureSaver reader(menuFields,CStructureSaver::READ,&menuReferences); reader.Add(4,&view); }
        Require(IsValid(view), "native load dialog did not construct its mode view");
        CStructureNetworkContext viewReferences;
        auto viewFields = BorrowedObjectFields(view,&viewReferences);
        CPtr<NUI::CWindow> tab;
        { CStructureSaver reader(viewFields,CStructureSaver::READ,&viewReferences); reader.Add(4,&tab); }
        Require(IsValid(tab) && tab->GetWindowID() == "tab_save", "native load view did not construct its actual Save tab");
        return tab->GetStyle(NUI::STYLE_VISIBLE);
    };
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(2,2);
    for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x) pixels[y][x] = NGfx::SPixel8888(12,23,34,255);
    texture->Set(pixels);
    CObj<NGame::CICSaveLoadMenu> allowed = new NGame::CICSaveLoadMenu(NGame::LOAD,texture,true);
    allowed->Exec();
    Require(saveTab(), "a load dialog with explicit save permission hid its Save tab");
    NMainLoop::DoneInterface();
    CObj<NGame::CMissionBase> mainMenu = dynamic_cast<NGame::CMissionBase*>(pSSClasses->CreateObject(int(0xB2540912)));
    Require(IsValid(mainMenu), "main-menu load fixture has no native menu interface class");
    CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false);
    CObj<NUI::CInterface> interface = new NUI::CInterface(cursor);
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.include = [](CObjectBase*) { return false; };
    references.ids[cursor.GetPtr()] = 1; references.refs[1] = cursor.GetPtr();
    references.ids[interface.GetPtr()] = 2; references.refs[2] = interface.GetPtr();
    references.nextID = 100;
    CMemoryStream saved;
    { CStructureSaver writer(saved,CStructureSaver::WRITE,&references); writer.Add(15,&cursor); writer.Add(16,&interface); }
    saved.Seek(0);
    { CStructureSaver reader(saved,CStructureSaver::READ,&references); mainMenu->NGame::CMissionBase::operator&(reader); }
    NInput::SEvent event;
    while (NInput::GetEvent(&event)) {}
    NInput::PostEvent("loadgame");
    Require(NInput::GetEvent(&event) && mainMenu->ProcessEvent(event) && NMainLoop::HaveInterfaceCommand(),
        "native main menu did not consume its Load Game command");
    Require(NMainLoop::StepApp(false,false,false), "main-menu load command did not execute through the application queue");
    Require(!saveTab(), "loading from the main menu offered a Save tab without an active game");
}

static void ExitMenuSavePermission(const char* databasePath,const char* resourcePath,bool shortcut)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    Require(NMainLoop::GetCurrentInterfaceForHarness() == nullptr && !NMainLoop::HaveInterfaceCommand(),
        "exit save-gate fixture must start without an interface or queued command");
    struct InterfaceCleanup { ~InterfaceCleanup() { NMainLoop::DoneInterface(); } } cleanup;
    // The existing saved UI permission must still permit Save when explicitly true.
    CObj<NUI::ICursor> controlCursor = NUI::ICursor::Create(false);
    CObj<NUI::CInterface> controlInterface = new NUI::CInterface(controlCursor);
    CObj<NUI::CWindow> permitted = dynamic_cast<NUI::CWindow*>(pSSClasses->CreateObject(int(0xB1203151)));
    Require(IsValid(permitted), "exit save-gate fixture has no native dialog class");
    permitted->SetInterface(controlInterface);
    bool allow = true;
    CMemoryStream fields;
    { CStructureSaver writer(fields,CStructureSaver::WRITE); writer.Add(2,&allow); }
    fields.Seek(0);
    { CStructureSaver reader(fields,CStructureSaver::READ); static_cast<CObjectBase*>(permitted.GetPtr())->operator&(reader); }
    NUI::LoadTemplate(permitted,NDb::GetUIContainer(357));
    auto* allowedSave = permitted->GetChildByID("save");
    Require(IsValid(allowedSave) && allowedSave->GetStyle(NUI::STYLE_VISIBLE),
        "exit dialog control did not show Save for an explicitly permitted saved UI");
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(1,1);
    pixels[0][0] = NGfx::SPixel8888(12,23,34,255); texture->Set(pixels);
    CObj<NGame::CICExitMenu> command = new NGame::CICExitMenu(texture);
    command->Exec();
    auto* active = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(active) && !NMainLoop::HaveInterfaceCommand(),
        "exit save-gate fixture did not initialize the actual exit menu cleanly");
    if (shortcut) {
        NInput::SEvent event;
        while (NInput::GetEvent(&event)) {}
        NInput::PostEvent("save");
        Require(NInput::GetEvent(&event), "exit save-gate fixture did not receive the native save command");
        active->ProcessEvent(event);
        Require(!NMainLoop::HaveInterfaceCommand(),
            "the default exit dialog queued a Save menu through its forbidden save shortcut");
    } else {
        CStructureNetworkContext references;
        auto saved = BorrowedObjectFields(active,&references);
        CPtr<NUI::CWindow> dialog;
        { CStructureSaver reader(saved,CStructureSaver::READ,&references); reader.Add(5,&dialog); }
        Require(IsValid(dialog), "exit save-gate fixture did not retain the native dialog");
        auto* save = dialog->GetChildByID("save");
        auto* cancel = dialog->GetChildByID("cancel");
        Require(IsValid(save) && IsValid(cancel), "exit save-gate fixture lacks save or cancel buttons");
        Require(!save->GetStyle(NUI::STYLE_VISIBLE),
            "the default exit dialog displayed Save despite its disabled save permission");
        Require(cancel->GetStyle(NUI::STYLE_VISIBLE), "disabling Save also hid the exit dialog's Cancel button");
    }
    CheckPermittedExitMenu<NGame::CICExitMenu>(texture,shortcut);
}

static void SuppliedMenuScreenshot(const char* databasePath,const char* resourcePath, bool lose = false)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    Require(NMainLoop::GetCurrentInterfaceForHarness() == nullptr,
        "supplied screenshot fixture must start without an active interface");
    struct InterfaceCleanup { ~InterfaceCleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels;
    pixels.SetSizes(2,2);
    pixels[0][0] = NGfx::SPixel8888(11,22,33,255);
    pixels[0][1] = NGfx::SPixel8888(44,55,66,255);
    pixels[1][0] = NGfx::SPixel8888(77,88,99,255);
    pixels[1][1] = NGfx::SPixel8888(111,122,133,255);
    texture->Set(pixels);
    if (lose) {
        CObj<NGame::CICLoseMenu> command = new NGame::CICLoseMenu(-1,texture);
        command->Exec();
    } else {
        CObj<NGame::CICOptions> command = new NGame::CICOptions(NGame::OS_VIDEO,texture);
        command->Exec();
    }
    auto* active = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(active), "supplied screenshot fixture did not open the native options interface");
    CStructureNetworkContext references;
    auto interfaceFields = BorrowedObjectFields(active,&references);
    CPtr<NUI::CScreenShot> widget;
    { CStructureSaver reader(interfaceFields,CStructureSaver::READ,&references); reader.Add(5,&widget); }
    Require(IsValid(widget) && widget->GetTexture() == texture.GetPtr(),
        "native options interface replaced or failed to adopt the supplied screenshot");
    CMemoryStream fields;
    { CStructureSaver writer(fields,CStructureSaver::WRITE); static_cast<CObjectBase*>(texture.GetPtr())->operator&(writer); }
    fields.Seek(0);
    NGScene::CScreenshotTexture::EMode mode = NGScene::CScreenshotTexture::COLOR;
    CVec4 coefficient(0,0,0,0);
    { CStructureSaver reader(fields,CStructureSaver::READ); reader.Add(2,&mode); reader.Add(3,&coefficient); }
    Require(mode == NGScene::CScreenshotTexture::BLACKANDWHITE,
        lose ? "opening the defeat menu with a supplied screenshot left it in default color mode"
             : "opening options with a supplied screenshot left it in the widget's default color mode");
    const CVec4 expected = lose ? CVec4(1.0f,0.677f,0.877f,1.0f) : CVec4(0.677f,0.877f,1.0f,1.0f);
    Require(fabs(coefficient.x-expected.x) < 1e-6f && fabs(coefficient.y-expected.y) < 1e-6f &&
        fabs(coefficient.z-expected.z) < 1e-6f && coefficient.w == expected.w,
        lose ? "supplied defeat screenshot did not receive the original loss tint"
             : "supplied options screenshot did not receive the original menu tint");
    CArray2D<NGfx::SPixel8888> retained;
    texture->Get(&retained);
    Require(retained.GetXSize() == 2 && retained.GetYSize() == 2,
        "opening options resized the supplied screenshot pixels");
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 2; ++x)
            Require(retained[y][x].r == pixels[y][x].r && retained[y][x].g == pixels[y][x].g &&
                retained[y][x].b == pixels[y][x].b && retained[y][x].a == pixels[y][x].a,
                "options screenshot treatment overwrote the original pixels");
}

static void NewLayerCompactSave()
{
    auto roundtrip = [](NAI::CNodesLayer* layer) {
        CObj<NAI::CNodesLayer> original = layer, restored;
        CMemoryStream fields;
        { CStructureSaver writer(fields, CStructureSaver::WRITE); writer.Add(2, &original); }
        fields.Seek(0);
        { CStructureSaver reader(fields, CStructureSaver::READ); reader.Add(2, &restored); }
        Require(IsValid(restored), "navigation save fixture did not restore the layer graph");
        return restored;
    };
    auto inspect = [&](NAI::CNodesLayer* layer) {
        auto restored = roundtrip(layer);
        return std::make_pair(restored->bHappySave, IsValid(restored->pColourer));
    };
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    const int normal = network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->GetColourer(normal);
    const auto ordinary = inspect(network->GetLayer(normal));
    Require(!ordinary.first && ordinary.second, "ordinary layer save did not retain its existing navigation cache");
    network->SetSmallSaveSize(true);
    const int added = network->CreateLayer(8, 8, VNULL2, 0.f, 1);
    network->GetColourer(added);
    const auto fresh = inspect(network->GetLayer(added));
    Require(fresh.first, "navigation layer created after enabling compact saves did not inherit the small-save flag");
    Require(!fresh.second && !inspect(network->GetLayer(normal)).second,
        "compact layer save retained a cache on a new or existing layer");
    Require(IsValid(network->GetLayer(added)->pColourer) && IsValid(network->GetLayer(normal)->pColourer),
        "compact serialization destroyed the live navigation caches");
    auto restored = roundtrip(network->GetLayer(added));
    Require(restored->bHappySave && !IsValid(restored->pColourer),
        "compact layer save did not round-trip the flag and omitted-cache state");
    network->SetSmallSaveSize(false);
    for (int index: {normal, added}) {
        const auto full = inspect(network->GetLayer(index));
        Require(!full.first && full.second, "disabling compact saves did not restore full cache serialization on all layers");
    }
}

static void ComboPopupClipping(bool itemWidth = false)
{
    CObj<NUI::CInterface> root = new NUI::CInterface;
    root->SetInterface(root); root->SetSize(NUI::SPoint(640,480)); root->SetPosition(NUI::SPoint(20,10));
    root->SetStyle(NUI::STYLE_VISIBLE | NUI::STYLE_ENABLED,true);
    CObj<NUI::CWindow> panel = new NUI::CWindow(NUI::SWindowInfo(root,NUI::SPoint(100,80),NUI::SPoint(300,100),"panel"));
    CObj<NUI::CWindow> row = new NUI::CWindow(NUI::SWindowInfo(panel,NUI::SPoint(25,20),NUI::SPoint(230,20),"row"));
    CObj<NUI::CComboBox> combo = new NUI::CComboBox(NUI::SWindowInfo(row,NUI::SPoint(10,2),NUI::SPoint(100,18),"combo"));
    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(combo,&references);
    CPtr<NUI::CListView> popup;
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(4,&popup); }
    Require(IsValid(popup),"native combo did not create its actual popup");
    combo->AddItem(7,NUI::CComboBox::SInfo(L"First"));
    combo->AddItem(9,NUI::CComboBox::SInfo(L"Second"));
    for (int id : {7,9}) popup->GetItem(id)->SetSize(NUI::SPoint(100,22));
    popup->SetSize(NUI::SPoint(160,44)); // Existing remaster artwork can exceed the closed field.
    auto update = [&]() { combo->Update(1,nullptr); popup->Update(1,nullptr); };
    auto open = [&]() { Require(combo->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY,"drop_list")) &&
        popup->GetStyle(NUI::STYLE_VISIBLE),"native combo did not open its actual popup"); };
    auto check = [&]() {
        NUI::SPoint field; NUI::SRect clip;
        combo->ClientToScreen(&field,&clip);
        const NUI::SPoint expected(field.x+combo->GetSize().x-160,field.y+combo->GetSize().y);
        NUI::SPoint position; popup->ClientToScreen(&position,&clip);
        std::printf("native combo parent=%s popup=%d,%d expected=%d,%d hit=%d\n",popup->GetParent()->GetWindowID().c_str(),
            position.x,position.y,expected.x,expected.y,int(popup->HitTest(expected.x+10,expected.y+10)));
        Require(popup->HitTest(expected.x+10,expected.y+10),"native combo popup is clipped by its owning row");
        Require(popup->GetParent() == root.GetPtr() && position == expected && popup->GetSize().x == 160,
            "native combo lost interface coordinates, right alignment or remaster artwork width");
    };
    update(); open(); check();
    if (itemWidth) {
        auto* wide = popup->GetItem(9);
        wide->SetSize(NUI::SPoint(250,22)); update();
        NUI::SPoint position; NUI::SRect clip; wide->ClientToScreen(&position,&clip);
        std::printf("native combo row width=%d popup width=%d row-end hit=%d\n",wide->GetSize().x,popup->GetSize().x,
            int(wide->HitTest(position.x+240,position.y+10)));
        Require(wide->HitTest(position.x+240,position.y+10),"native combo popup clips an item wider than its closed field");
        auto width = [&](int expected) {
            NUI::SPoint field, list; NUI::SRect clip;
            combo->ClientToScreen(&field,&clip); popup->ClientToScreen(&list,&clip);
            Require(popup->GetSize() == NUI::SPoint(expected,44) && list.x+expected == field.x+combo->GetSize().x,
                "native item-width layout lost the field right edge, list height or remaster artwork minimum");
        };
        width(250);
        combo->SetSize(NUI::SPoint(300,18)); update(); width(300);
        combo->SetSize(NUI::SPoint(80,18)); update(); width(250);
        wide->SetSize(NUI::SPoint(100,22)); update(); width(160);
        combo->RemoveAllItems(); update();
        Require(popup->GetSize() == NUI::SPoint(160,0) && combo->GetSelectedItem() == -1,
            "empty native combo lost the remaster artwork width or retained rows");
        return;
    }
    panel->SetPosition(NUI::SPoint(150,90)); combo->SetSize(NUI::SPoint(60,18)); update(); check();
    // An older native UI graph can still carry the popup below its row.
    { CMObj<NUI::CWindow> hold = popup.GetPtr(); root->RemoveChild(popup); row->AddChild(popup); }
    update(); check();
    row->SetStyle(NUI::STYLE_ENABLED,false); update();
    Require(!popup->GetStyle(NUI::STYLE_ENABLED),"root-level popup remained interactive below a disabled owner");
    row->SetStyle(NUI::STYLE_ENABLED,true); panel->SetStyle(NUI::STYLE_VISIBLE,false); update();
    Require(!popup->GetStyle(NUI::STYLE_VISIBLE),"root-level popup remained visible below a hidden owner");
    panel->SetStyle(NUI::STYLE_VISIBLE,true); update(); open(); check();
    popup->SetSelectedItem(9);
    Require(combo->GetSelectedItem() == 9 && !popup->GetStyle(NUI::STYLE_VISIBLE),
        "native popup selection did not notify its combo and close");
    open();
    combo->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONDOWN,0,0));
    Require(!popup->GetStyle(NUI::STYLE_VISIBLE),"native popup did not close on an outside click");
    combo = nullptr;
    Require(!IsValid(popup),"destroying a combo left its root-level popup alive");
}

static void WindowLazyUpdate()
{
    auto requiresUpdate = [](NUI::CWindow* window) {
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(window, &references);
        bool flag = true;
        { CStructureSaver reader(fields, CStructureSaver::READ, &references); reader.Add(4, &flag); }
        return flag;
    };
    CObj<NUI::CWindow> idle = new NUI::CWindow;
    Require(requiresUpdate(idle), "new UI window did not start with an update pending");
    idle->Update(1, nullptr);
    Require(!requiresUpdate(idle), "idle UI branch never cleared its serialized update latch");
    idle->Update(2, nullptr);
    Require(!requiresUpdate(idle), "sleeping UI branch spontaneously resumed its per-frame traversal");
    CMObj<NUI::CWindow> child = new NUI::CWindow(NUI::SWindowInfo(idle,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "late-child"));
    Require(requiresUpdate(idle), "adding a child did not wake its sleeping parent");
    idle->Update(3, nullptr);
    Require(!requiresUpdate(idle) && !requiresUpdate(child), "static child kept its UI branch awake forever");
    CObj<NUI::CWindow> animated = new NUI::CWindow(NUI::SWindowInfo(nullptr,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "always-update", 0x80));
    animated->Update(1, nullptr); animated->Update(2, nullptr);
    Require(requiresUpdate(animated), "retail always-update style did not keep its UI window awake");
    CObj<NUI::CWindow> active = new NUI::CWindow;
    CMObj<NUI::CSlider> slider = new NUI::CSlider(NUI::SWindowInfo(active,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "active-slider"));
    active->Update(1, nullptr); active->Update(2, nullptr);
    Require(requiresUpdate(active), "active slider's update result did not keep its parent branch awake");
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE); bool pending = false; writer.Add(4, &pending); }
    saved.Seek(0);
    CObj<NUI::CWindow> restored = new NUI::CWindow;
    { CStructureSaver reader(saved, CStructureSaver::READ); restored->operator&(reader); }
    restored->Update(4, nullptr);
    Require(!requiresUpdate(restored), "restored sleeping UI window lost the retail tag-4 latch");
}

static void WindowNestedWakeup()
{
    auto requiresUpdate = [](NUI::CWindow* window) {
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(window, &references);
        bool flag = true;
        { CStructureSaver reader(fields, CStructureSaver::READ, &references); reader.Add(4, &flag); }
        return flag;
    };
    CObj<NUI::CWindow> root = new NUI::CWindow;
    CMObj<NUI::CWindow> branch = new NUI::CWindow(NUI::SWindowInfo(root,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "sleeping-branch"));
    root->Update(1, nullptr);
    Require(!requiresUpdate(root) && !requiresUpdate(branch), "nested UI wakeup fixture did not become idle");
    CMObj<NUI::CSlider> active = new NUI::CSlider(NUI::SWindowInfo(branch,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "late-active-descendant"));
    Require(requiresUpdate(root) && requiresUpdate(branch),
        "new active descendant did not wake all sleeping ancestor windows");
    root->Update(2, nullptr); root->Update(3, nullptr);
    Require(requiresUpdate(root) && requiresUpdate(branch), "active descendant stopped updating beneath a sleeping ancestor");
    branch->RemoveChild(active);
    active = nullptr;
    root->Update(4, nullptr);
    Require(!requiresUpdate(root) && !requiresUpdate(branch), "removing the active descendant left idle ancestors permanently awake");
    CMObj<NUI::CWindow> passive = new NUI::CWindow(NUI::SWindowInfo(branch,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "late-passive-descendant"));
    Require(requiresUpdate(root), "a passive descendant did not request its initial ancestor traversal");
    root->Update(5, nullptr);
    Require(!requiresUpdate(root) && !requiresUpdate(branch) && !requiresUpdate(passive),
        "passive descendant insertion disabled lazy UI updates");
}

static void WindowTemplateScript(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    CDBIterator<NDb::CScript> scripts(*NDatabase::GetTable<NDb::CScript>());
    NDb::CScript* record = nullptr;
    while (scripts.MoveNext()) if (scripts.Get()->GetRecordID() > 0) { record = scripts.Get(); break; }
    Require(record && NDb::GetDBScript(record->GetRecordID()) == record,
        "template script fixture did not resolve its native database script record");
    record->strCode = "local w=GetWindow('template-child'); if w then w.x=84 end; templateReady=1";
    CObj<NDb::CUIContainer> layout = new NDb::CUIContainer;
    layout->pScript = record;
    CObj<NUI::CInterface> interface = new NUI::CInterface;
    interface->SetInterface(interface);
    CMObj<NUI::CWindow> window = new NUI::CWindow(NUI::SWindowInfo(interface,
        NUI::SPoint(1, 2), NUI::SPoint(20, 20), "template-child"));
    CObj<NScript::CScript> unrelated = NScript::CreateScript(nullptr);
    unrelated->PushNumber(777);
    struct GlobalScriptRestore {
        CPtr<NScript::CScript> previous;
        ~GlobalScriptRestore() { pScript = previous; }
    } restore{pScript};
    pScript = unrelated;
    window->ProcessMessage(NUI::SEvent(NUI::EVENT_TEMPLATELOAD, nullptr, layout.GetPtr()));
    Require(window->GetPosition() == NUI::SPoint(1, 2), "template script ran before the native window update phase");
    window->Update(123, nullptr);
    Require(window->GetPosition() == NUI::SPoint(84, 2),
        "window template Lua script was not created and executed in its own interface during Update");
    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(window, &references);
    CPtr<NScript::CScript> script;
    { CStructureSaver reader(fields, CStructureSaver::READ, &references); reader.Add(17, &script); }
    Require(IsValid(script) && !IsValid(script->pWorld) && script->GetScriptInterface() == interface.GetPtr()
            && script->GetGlobal("templateReady").IsNumber() && script->GetGlobal("templateReady").GetNumber() == 1
            && unrelated->GetTop() == 1,
        "template Lua lifecycle lost its saved script/context or modified an unrelated script stack");
    CObj<NDb::CUIContainer> plainLayout = new NDb::CUIContainer;
    CMObj<NUI::CWindow> plain = new NUI::CWindow(NUI::SWindowInfo(interface,
        NUI::SPoint(5, 6), NUI::SPoint(10, 10), "unscripted-child"));
    plain->ProcessMessage(NUI::SEvent(NUI::EVENT_TEMPLATELOAD, nullptr, plainLayout.GetPtr()));
    plain->Update(124, nullptr);
    CStructureNetworkContext plainReferences;
    auto plainFields = BorrowedObjectFields(plain, &plainReferences);
    CPtr<NScript::CScript> absent;
    { CStructureSaver reader(plainFields, CStructureSaver::READ, &plainReferences); reader.Add(17, &absent); }
    Require(!IsValid(absent) && plain->GetPosition() == NUI::SPoint(5, 6), "unscripted templates acquired a script or changed their native window");
}

static std::map<unsigned char, vector<unsigned char>> OuterSaveChunks(CDataStream& stream)
{
    std::map<unsigned char, vector<unsigned char>> chunks;
    stream.Seek(0);
    while (stream.GetPosition() < stream.GetSize()) {
        unsigned char tag = 0, encoded[4] = {};
        stream.Read(&tag, 1); stream.Read(encoded, 1);
        const std::size_t prefix = encoded[0] & 1 ? 4 : 1;
        if (prefix == 4) stream.Read(encoded + 1, 3);
        std::uint32_t length = 0;
        Require(S2FileIO::DecodeStructureLength(encoded, prefix, stream.GetSize() - stream.GetPosition(), &length),
            "compressed-save fixture found an invalid outer chunk length");
        auto& payload = chunks[tag]; payload.resize(length);
        if (length) stream.Read(payload.data(), length);
    }
    return chunks;
}

static void LoadSuccessFeedback(const char* databasePath,const char* resourcePath,bool encodedName = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr, "load feedback fixture needs isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "load feedback fixture started with a live interface or command");
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    const char* profile = encodedName ? "UpstreamEncodedLoad" : "UpstreamLoadFeedback";
    NMainLoop::CreateProfile(profile);
    NMainLoop::SetActiveProfile(profile);
    const string slot = encodedName ? S2FileIO::EncodeWindowsSaveName(L"Load \u0417\u0430\u043F\u0438\u0441\u044C \u65E5\u672C") : "Feedback";
    auto* manager = NMainLoop::GetSaveManager();
    struct Cleanup {
        NMainLoop::CSaveManager* manager;
        string slot;
        ~Cleanup() { NMainLoop::DoneInterface(); manager->DeleteSlot(slot); manager->DeleteSlot(NMainLoop::S_SLOT_ACTIVE); }
    } cleanup{manager,slot};
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(2,2);
    for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x) pixels[y][x] = NGfx::SPixel8888(12,23,34,255);
    texture->Set(pixels);
    CObj<NGame::CICExitMenu> menu = new NGame::CICExitMenu(texture);
    menu->Exec();
    CObj<NMainLoop::IInterfaceBase> original = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(original) && NMainLoop::GetInterfaceStackDepth() == 1,
        "load feedback fixture did not create its native interface stack");
    CObj<NMainLoop::CICSave> save = new NMainLoop::CICSave(slot,texture,true);
    save->Exec();
    const auto localized = NUI::GetDBString(20260);
    Require(!localized.empty(), "load feedback fixture has no localized success message");
    csGame << localized << endl;
    Require(!consoleLines.empty() && consoleLines.front().eType == STREAM_GAME && !consoleLines.front().szText.empty(),
        "load feedback log control did not receive its localized game-stream message");
    const auto expected = consoleLines.front().szText;
    auto messages = [&]() {
        return std::count_if(consoleLines.begin(),consoleLines.end(),[&](const SConsoleLine& line) {
            return line.eType == STREAM_GAME && line.szText == expected;
        });
    };
    const auto before = messages();
    CObj<NMainLoop::CICLoad> load = new NMainLoop::CICLoad(slot,true);
    load->Exec();
    CObj<NMainLoop::IInterfaceBase> restored = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(restored) && restored.GetPtr() != original.GetPtr() && NMainLoop::GetInterfaceStackDepth() == 1,
        "load feedback control did not restore a new native interface from the actual slot");
    Require(messages() == before + 1, "a completed silent native load omitted its localized success feedback");
    if (encodedName) {
        list<string> written; manager->GetSlotsList(&written);
        for (const auto* invalid : {L"bad.name",L"bad^name",L"bad:name",L"bad/name",L".."}) {
            CObj<NMainLoop::CICLoad> rejected = new NMainLoop::CICLoad(S2FileIO::EncodeWindowsSaveName(invalid),true);
            rejected->Exec();
            list<string> after; manager->GetSlotsList(&after);
            Require(after == written && NMainLoop::GetCurrentInterfaceForHarness() == restored.GetPtr() && messages() == before+1,
                "encoded invalid load name changed the active interface or reported success");
        }
        std::puts("native encoded Unicode load: restored a new interface from the actual saved file; invalid names rejected");
    }
    Require(!std::filesystem::exists(std::filesystem::path(manager->GetSlotFilePathW("Missing",NMainLoop::S_SAVE_FILENAME))),
        "load failure control unexpectedly has an existing slot");
    const auto beforeFailure = messages();
    CObj<NMainLoop::CICLoad> failed = new NMainLoop::CICLoad("Missing",true);
    try { failed->Exec(); } catch (...) {} // Debug builds can propagate the expected read failure.
    Require(messages() == beforeFailure && NMainLoop::GetCurrentInterfaceForHarness() == restored.GetPtr(),
        "a missing native slot reported success or replaced the active interface");
}

static void SaveSuccessFeedback(const char* databasePath,bool encodedName = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr, "save feedback fixture needs isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "save feedback fixture started with a live interface or command");
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    const char* profile = encodedName ? "UpstreamEncodedSave" : "UpstreamSaveFeedback";
    NMainLoop::CreateProfile(profile);
    NMainLoop::SetActiveProfile(profile);
    const string slot = encodedName ? S2FileIO::EncodeWindowsSaveName(L"Quick \u0417\u0430\u043F\u0438\u0441\u044C \u65E5\u672C") : "Feedback";
    auto* manager = NMainLoop::GetSaveManager();
    struct SlotCleanup {
        NMainLoop::CSaveManager* manager;
        string slot;
        ~SlotCleanup() { manager->DeleteSlot(slot); }
    } cleanup{manager,slot};
    const auto localized = NUI::GetDBString(20259);
    Require(!localized.empty(), "save feedback fixture has no localized success message");
    csGame << localized << endl;
    Require(!consoleLines.empty() && consoleLines.front().eType == STREAM_GAME && !consoleLines.front().szText.empty(),
        "save feedback log control did not receive its localized game-stream message");
    const auto expected = consoleLines.front().szText;
    auto messages = [&]() {
        return std::count_if(consoleLines.begin(),consoleLines.end(),[&](const SConsoleLine& line) {
            return line.eType == STREAM_GAME && line.szText == expected;
        });
    };
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(2,2);
    for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x) pixels[y][x] = NGfx::SPixel8888(12,23,34,255);
    texture->Set(pixels);
    const auto before = messages();
    CObj<NMainLoop::CICSave> save = new NMainLoop::CICSave(slot,texture,true);
    save->Exec();
    list<string> written; manager->GetSlotsList(&written);
    Require(!encodedName || std::find(written.begin(),written.end(),slot) != written.end(),
        "native save command rejected its platform-encoded valid Unicode slot");
    {
        CFileStream file;
        file.OpenRead(manager->GetSlotFilePathW(slot,NMainLoop::S_SAVE_FILENAME).c_str());
        NMainLoop::SSaveFileHeader header;
        NMainLoop::ReadSaveFileHeader(file,&header);
        Require(header.nMagic == NMainLoop::N_SAVE_MAGIC_NUMBER && header.nMods == 0,
            "save feedback control did not write a complete native save header");
        list<CObj<NMainLoop::IInterfaceBase>> restored;
        { CStructureSaver reader(file,CStructureSaver::READ); reader.Add(2,&restored); }
        Require(restored.empty(), "save feedback control did not round-trip its empty native interface stack");
    }
    Require(messages() == before + 1, "a completed silent native save omitted its localized success feedback");
    if (encodedName) {
        for (const auto* invalid : {L"bad.name",L"bad^name",L"bad:name",L"bad/name",L".."}) {
            const auto count = messages();
            CObj<NMainLoop::CICSave> rejected = new NMainLoop::CICSave(S2FileIO::EncodeWindowsSaveName(invalid),texture,true);
            rejected->Exec();
            list<string> after; manager->GetSlotsList(&after);
            Require(after == written && messages() == count,"encoded invalid save name wrote a slot or reported success");
        }
#if !defined(_WIN32)
        for (const char* invalid : {"S2U8:GG","S2U8:0","S2U8:00"}) {
            CObj<NMainLoop::CICSave> rejected = new NMainLoop::CICSave(invalid,texture,true);
            rejected->Exec();
            list<string> after; manager->GetSlotsList(&after);
            Require(after == written && messages() == before+1,"malformed Linux save-name encoding wrote a slot or reported success");
        }
#endif
        std::puts("native encoded Unicode save: real header/data round-trip; invalid decoded names rejected");
        return;
    }
    const auto blockedPath = std::filesystem::path(manager->GetSlotFilePathW(NMainLoop::S_SLOT_ACTIVE,NMainLoop::S_SAVE_FILENAME));
    std::filesystem::create_directories(blockedPath.parent_path());
    Require(std::filesystem::create_directory(blockedPath), "save failure control could not block its isolated target with a directory");
    struct BlockerCleanup {
        std::filesystem::path path;
        ~BlockerCleanup() { std::error_code error; std::filesystem::remove(path,error); }
    } blocker{blockedPath};
    const auto beforeFailure = messages();
    CObj<NMainLoop::CICSave> failed = new NMainLoop::CICSave(NMainLoop::S_SLOT_ACTIVE,texture,true);
    try { failed->Exec(); } catch (...) {} // Debug builds propagate the expected write failure.
    Require(std::filesystem::is_directory(blockedPath), "save failure control unexpectedly replaced its blocked target");
    Require(messages() == beforeFailure, "a failed native save emitted a false localized success message");
}

static void CompressedSaveWriting()
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr, "compressed-save fixture needs an isolated user-data directory");
    NMainLoop::CreateProfile("UpstreamCompression");
    NMainLoop::SetActiveProfile("UpstreamCompression");
    CObj<NMainLoop::CICSaveFile> snapshot = new NMainLoop::CICSaveFile("upstream-compression.sav");
    snapshot->Exec();
    auto* manager = NMainLoop::GetSaveManager();
    const auto path = manager->GetSlotFilePathW(NMainLoop::S_SLOT_ACTIVE, "upstream-compression.sav");
    CFileStream file; file.OpenRead(path.c_str());
    auto snapshotChunks = OuterSaveChunks(file);
    const vector<unsigned char> marker = {'A', '3', 0};
    Require(snapshotChunks[3] == marker, "native snapshot command wrote an uncompressed save without the retail A3 marker");
    Require(snapshotChunks[4] == vector<unsigned char>({1, 0, 0, 0}), "compressed snapshot lost its unpacked format version");
    file.Seek(0);
    list<CObj<NMainLoop::IInterfaceBase>> interfaces;
    { CStructureSaver reader(file, CStructureSaver::READ); reader.Add(2, &interfaces); }
    Require(interfaces.empty(), "empty native snapshot did not round-trip its interface list");
    vector<CObj<NWorld::CCmdReload>> commands;
    for (int i = 0; i < 1024; ++i) commands.emplace_back(new NWorld::CCmdReload(i % NDb::N_SLOTS));
    CMemoryStream plain, packed;
    { CStructureSaver writer(plain, CStructureSaver::WRITE); writer.Add(2, &commands); }
    // The new compressed writer occupies the next EMode value. Using its wire
    // value keeps this behavioral regression compilable before the API exists.
    { CStructureSaver writer(packed, static_cast<CStructureSaver::EMode>(2)); writer.Add(2, &commands); }
    auto plainChunks = OuterSaveChunks(plain), packedChunks = OuterSaveChunks(packed);
    Require(!plainChunks.count(3) && packedChunks[3] == marker && plainChunks[4] == packedChunks[4],
        "compressed writing changed plain-stream compatibility or the version envelope");
    Require(packed.GetSize() < plain.GetSize() / 2, "repetitive native command save was not actually compressed");
    for (unsigned char tag : {0, 1, 2}) {
        Require(packedChunks.count(tag) && plainChunks.count(tag), "compressed save omitted a required payload chunk");
        CMemoryStream payload, unpacked;
        payload.Write(packedChunks[tag].data(), static_cast<unsigned int>(packedChunks[tag].size()));
        payload.Seek(0);
        CNetCompressor decoder; decoder.Unpack(payload, unpacked);
        Require(unpacked.GetSize() == static_cast<int>(plainChunks[tag].size())
                && (unpacked.GetSize() == 0 || !std::memcmp(unpacked.GetBuffer(), plainChunks[tag].data(), unpacked.GetSize())),
            "a compressed payload did not independently decompress to its native plain contents");
    }
    vector<CObj<NWorld::CCmdReload>> restored;
    packed.Seek(0);
    { CStructureSaver reader(packed, CStructureSaver::READ); reader.Add(2, &restored); }
    Require(restored.size() == commands.size(), "compressed save lost referenced command objects");
    for (std::size_t i = 0; i < restored.size(); ++i)
        Require(IsValid(restored[i]) && restored[i]->nSlot == commands[i]->nSlot,
            "compressed save changed a restored command's slot");
}

static void FreshAmbientGraph()
{
    Require(S2Platform::Window() == nullptr, "fresh ambient fixture unexpectedly owns a display window");
    CObj<NGScene::IGScene> scene = NGScene::CreateScene();
    CStructureNetworkContext references;
    auto saved = BorrowedObjectFields(scene, &references);
    CPtr<CFuncBase<CVec3>> mean, top, bottom, animated;
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      reader.Add(18, &mean); reader.Add(31, &top); reader.Add(32, &bottom); reader.Add(33, &animated); }
    Require(IsValid(animated) && pSSClasses->GetObjectTypeID(animated.GetPtr()) == int(0x01123131),
        "fresh scene omitted its retail animated top-ambient node");
    Require(IsValid(mean) && IsValid(top) && IsValid(bottom), "fresh scene lost its ambient endpoints");
    CStructureNetworkContext meanReferences;
    auto meanFields = BorrowedObjectFields(mean, &meanReferences);
    CPtr<CFuncBase<CVec3>> meanTop, meanBottom;
    { CStructureSaver reader(meanFields, CStructureSaver::READ, &meanReferences);
      reader.Add(2, &meanTop); reader.Add(3, &meanBottom); }
    Require(meanTop == animated && meanBottom == bottom, "fresh ambient mean bypassed its animated top edge");
    const CVec3 topColor(0.8f, 0.6f, 0.4f), bottomColor(0.2f, 0.1f, 0.3f);
    scene->SetAmbient(bottomColor, topColor);
    CDGPtr<CFuncBase<CVec3>> refreshed(mean);
    MarkNewDGFrame(); refreshed.Refresh();
    Require(fabs2(refreshed->GetValue() - (topColor + bottomColor) * 0.5f) < 1e-10f,
        "fresh animated ambient node changed the unanimated scene mean");
    CObj<FixedAnimatedLight> animation = new FixedAnimatedLight;
    animation->Light()->color = CVec3(0.4f, 0.8f, 0.2f); animation->Light()->bEnd = false;
    references.ids[animation.GetPtr()] = 1; references.refs[1] = animation.GetPtr();
    CPtr<CPtrFuncBase<NGScene::CAnimLight>> animationEdge = animation.GetPtr();
    CMemoryStream binding;
    { CStructureSaver writer(binding, CStructureSaver::WRITE, &references); writer.Add(3, &animationEdge); }
    binding.Seek(0);
    { CStructureSaver reader(binding, CStructureSaver::READ, &references); animated->operator&(reader); }
    animated->NetworkFieldsChanged();
    MarkNewDGFrame(); refreshed.Refresh();
    Require(fabs2(refreshed->GetValue() - (animation->Light()->color + bottomColor) * 0.5f) < 1e-10f,
        "live animated ambient did not reach the scene mean");
    animation->Light()->color = CVec3(0.9f, 0.3f, 0.7f); animation->Updated();
    MarkNewDGFrame(); refreshed.Refresh();
    Require(fabs2(refreshed->GetValue() - (animation->Light()->color + bottomColor) * 0.5f) < 1e-10f,
        "animated ambient changes did not invalidate the native dependency graph");
    animation->Light()->bEnd = true; animation->Updated();
    MarkNewDGFrame(); refreshed.Refresh();
    Require(fabs2(refreshed->GetValue() - (topColor + bottomColor) * 0.5f) < 1e-10f,
        "ended ambient animation did not restore the configured top color");
    CObj<NGScene::CCVec3> sunlight = new NGScene::CCVec3(CVec3(1, 1, 1));
    CObj<NGScene::CCVec3> gloss = new NGScene::CCVec3(VNULL3);
    CPtr<CObjectBase> directional = scene->AddDirectionalLight(sunlight, gloss, VNULL3,
        CVec3(0, 0, 1), VNULL3, CVec2(8, 8), 16.f, true, 0);
    Require(IsValid(directional), "fresh ambient fixture did not create a native directional light");
    CStructureNetworkContext directionalReferences;
    auto directionalFields = BorrowedObjectFields(directional, &directionalReferences);
    CPtr<CFuncBase<CVec3>> directionalTop;
    { CStructureSaver reader(directionalFields, CStructureSaver::READ, &directionalReferences);
      reader.Add(15, &directionalTop); }
    Require(directionalTop == animated, "directional lighting bypassed the scene's animated ambient edge");
    Require(S2Platform::Window() == nullptr, "ambient dependency test unexpectedly created a display window");
}

static void AnimatedPointLightGuard(bool inactive)
{
    CObj<FixedAnimatedLight> animator = new FixedAnimatedLight;
    auto* state = animator->Light();
    state->position = VNULL3; state->color = CVec3(1, 1, 1);
    state->fRadius = 4; state->bActive = true; state->bEnd = false;
    CObj<NGScene::CDynamicPointLight> light = new NGScene::CDynamicPointLight(animator);
    CObj<AnimatedLightMaterial> material = new AnimatedLightMaterial;
    NGScene::CSceneFragments scene;
    NGScene::CPartFlags parts; parts.Clear(); parts.Set(0);
    scene.AddElement(0, parts, material, nullptr, nullptr);
    NGScene::SParticleLMRenderTargetInfo particles;
    CTransformStack transform;
    auto reachesMaterials = [&]() {
        // Observe the real render traversal before any graphics calls; the
        // material sentinel also proves that a valid light is not skipped.
        try { light->Render(&transform, &transform, nullptr, NGScene::RP_GF3_CL, nullptr, scene, particles); }
        catch (const AnimatedLightRenderReached&) { return true; }
        return false;
    };
    Require(reachesMaterials(), "animated-light fixture did not enter rendering for a live ordinary light");
    if (inactive) {
        state->bActive = false;
        Require(!reachesMaterials(), "inactive animated point light entered scene rendering before its parameters were ready");
        state->bActive = true;
    } else {
        for (float radius : {0.f, -1.f, 0.0999f}) {
            state->fRadius = radius;
            Require(!reachesMaterials(), "zero/tiny animated point-light radius reached the rendering pipeline");
        }
        state->fRadius = 0.1f;
        Require(reachesMaterials(), "minimum valid animated point-light radius stopped rendering");
    }
    state->bEnd = true;
    Require(!reachesMaterials(), "ended animated point light still rendered");
    state->bEnd = false; state->color = VNULL3;
    Require(!reachesMaterials(), "dark animated point light still rendered");
}

static void TinyPointLightCache()
{
    NGScene::SGlobalIlluminationInfo global;
    global.vUpDifColor = VNULL3;
    const float radii[] = {0.f, 0.25f, 0.5f, 0.5001f, 12.f};
    for (float radius: radii)
        global.points.emplace_back(CVec3(1, 0.5f, 0.25f), VNULL3, radius, false, false);
    NGScene::SLightStateCalcSeed seed;
    NGScene::CLightState cached;
    cached.CreateSimple(&seed, global);
    Require(cached.points.size() == 2,
        "point-light cache retained zero/tiny radii that retail discards before lighting");
    Require(cached.points[0].fRadius == 0.5001f && cached.points[1].fRadius == 12.f
            && cached.points[1].vColor == CVec3(1, 0.5f, 0.25f) && !cached.points[1].bCastShadow,
        "tiny-light cutoff changed surviving radii, colour, ordering or shadow settings");
    cached.skyDirections.clear();
    NGScene::SDynamicAmbientInfo result;
    cached.TraceDynamicLM(&result, SSphere(CVec3(1, 0, 0), 0.1f), nullptr);
    Require(fabs2(result.vXNeg.v - CVec3(4.f/9.f, 2.f/9.f, 1.f/9.f)) < 1e-10f,
        "surviving point light changed its distance attenuation after tiny-light filtering");
}

static void PointLightRadiance()
{
    CObj<NGScene::CPointLight> light = new NGScene::CPointLight(CVec3(4, 2, 1), CVec3(3, -2, 5), 3.f, nullptr, true, false);
    NGScene::CPointLight::SRadianceInfo info;
    light->GetRadianceInfo(&info, NGScene::RP_GF3_CL);
    Require(info.vColor == CVec3(1, 0.5f, 0.25f) && info.fRadius == 6.f,
        "point-light radiance failed to fold colour strength into its cached radius");
    Require(info.vCenter == CVec3(3, -2, 5) && !info.bIsRendered,
        "radiance conversion changed light position or lightmap-only eligibility");
    CObj<NGScene::CPointLight> dim = new NGScene::CPointLight(CVec3(0.25f, 0.125f, 0), VNULL3, 8.f, nullptr, true, false);
    dim->GetRadianceInfo(&info, NGScene::RP_GF3_CL);
    Require(info.vColor == CVec3(1, 0.5f, 0) && info.fRadius == 4.f,
        "dim point-light radiance used the wrong radius scaling");
    CObj<NGScene::CPointLight> dark = new NGScene::CPointLight(VNULL3, VNULL3, 3.f, nullptr, true, false);
    dark->GetRadianceInfo(&info, NGScene::RP_GF3_CL);
    Require(info.vColor == VNULL3 && info.fRadius == 3.f,
        "zero-strength point light generated nonfinite radiance or changed its radius");
}

struct RetailGlobalPointFixture
{
    float color[3], center[3], radius;
    bool rendered, shadow;
    unsigned char padding[2];
};
static_assert(sizeof(RetailGlobalPointFixture) == 32, "retail global point-light record size");

static void GlobalPointLightShadowFlag()
{
    vector<RetailGlobalPointFixture> original(3);
    std::memset(original.data(), 0, original.size() * sizeof(original[0]));
    for (int i = 0; i < 3; ++i) {
        original[i].color[0] = 0.25f;
        original[i].color[1] = 0.5f;
        original[i].color[2] = 0.75f;
        original[i].center[0] = float(i * 4);
        original[i].radius = float(12 + i);
        original[i].shadow = i != 0;
        original[i].rendered = i == 2;
    }
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE); writer.Add(5, &original); }
    saved.Seek(0);
    NGScene::SGlobalIlluminationInfo global;
    global.vUpDifColor = VNULL3;
    { CStructureSaver reader(saved, CStructureSaver::READ); global.operator&(reader); }
    NGScene::SLightStateCalcSeed seed;
    NGScene::CLightState cached;
    cached.CreateSimple(&seed, global);
    Require(cached.points.size() == 2, "global lighting cache lost points or duplicated an already rendered light");
    Require(!cached.points[0].bCastShadow && cached.points[1].bCastShadow,
        "global illumination discarded the saved point-light shadow flags while building its cache");
    Require(cached.points[1].vCenter == CVec3(4, 0, 0) && cached.points[1].fRadius == 13.f
            && cached.points[1].vColor == CVec3(0.25f, 0.5f, 0.75f),
        "global lighting cache changed saved colour, radius or position");
    CMemoryStream resaved;
    { CStructureSaver writer(resaved, CStructureSaver::WRITE); global.operator&(writer); }
    resaved.Seek(0);
    vector<RetailGlobalPointFixture> roundtrip;
    { CStructureSaver reader(resaved, CStructureSaver::READ); reader.Add(5, &roundtrip); }
    Require(roundtrip.size() == 3 && !roundtrip[0].shadow && roundtrip[1].shadow
            && roundtrip[2].shadow && roundtrip[2].rendered,
        "global lighting resave lost retail rendered/shadow flags");
}

class CameraExecutorReached: public std::runtime_error
{
public:
    CameraExecutorReached(): std::runtime_error("automatic action camera escaped cinematic desktop filtering") {}
};

class QueuedCameraWorld: public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(QueuedCameraWorld);
public:
    vector<CObj<NWorld::CUICmd>> commands;
    std::size_t next = 0;
    NWorld::CUICmd* GetUICommand() override {
        return next < commands.size() ? commands[next++].GetPtr() : nullptr;
    }
};

class CinematicDrainMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(CinematicDrainMission);
    CObj<NUI::CDesktopWindow> desktop;
public:
    void Attach(NWorld::CWorld* world, NUI::CDesktopWindow* top) { pWorld = world; desktop = top; }
    NUI::CDesktopWindow* GetDesktop() const override { return desktop; }
    ICamera* GetCamera() const override { throw CameraExecutorReached(); }
    void Drain() { ExecWorldCommands(); }
    bool TutorialEnabled() const { return bTutorialMode; }
};

class ReadinessWorld: public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(ReadinessWorld);
public:
    bool sequence = false;
    bool IsSequence() const override { return sequence; }
};

class ClueDispatchMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(ClueDispatchMission);
    CObj<NUI::CDesktopWindow> desktop;
public:
    void Attach(NWorld::CWorld* world,NRPG::CGlobalGame* game) {
        pWorld = world; pGlobalGame = game; desktop = new NUI::CDesktopWindow;
    }
    void Step() override {}
    void OnGetFocus() override {}
    void OnLostFocus() override {}
    NUI::CDesktopWindow* GetDesktop() const override { return desktop; }
    void DoEvent(NWorld::CCommand* command) override { dynamic_cast<NWorld::CWorld*>(pWorld.GetPtr())->ExecuteCommand(command); }
    void Drain() { ExecWorldCommands(); }
};

static void FreshClueWaitRelease(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "fresh clue fixture must begin without an active interface or command");
    struct Cleanup { ~Cleanup() { NMainLoop::StepApp(false,false,false); NMainLoop::DoneInterface(); } } cleanup;
    auto* clue = game->pScenarioTracker->GetClueByName("Find_Cipher_key");
    Require(clue != nullptr, "fresh clue fixture has no authored campaign clue");
    auto* script = world->GetOwnScript();
    Require(script != nullptr, "fresh clue fixture has no native mission script");
    CObj<NWorld::CUICmdShowClue> command = new NWorld::CUICmdShowClue(clue);
    const int event = command->GetID();
    Require(script->AddUICommandWithID(command) == 1 && script->IsUIActionIDPresent(event),
        "fresh clue fixture did not register its actual pending script UI action");
    CObj<ClueDispatchMission> mission = new ClueDispatchMission;
    mission->Attach(world,game); mission->Drain();
    Require(NMainLoop::HaveInterfaceCommand() && script->IsUIActionIDPresent(event),
        "dispatching a clue skipped its modal or prematurely released its script wait");
    Require(NMainLoop::StepApp(false,false,false), "the native queued clue modal did not open");
    auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(modal) && pSSClasses->GetObjectTypeID(modal) == int(0xB1001180),
        "fresh clue dispatch opened an unrelated interface");
    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(modal,&references);
    CPtr<NGame::IMission> origin;
    int restoredEvent = -1;
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(2,&restoredEvent); reader.Add(3,&origin); }
    Require(origin.GetPtr() == mission.GetPtr() && restoredEvent == event,
        "fresh clue dispatch dropped the originating mission and its waiting script event ID");
    NInput::SEvent input;
    while (NInput::GetEvent(&input)) {}
    NInput::PostEvent("cancel");
    Require(NInput::GetEvent(&input) && modal->ProcessEvent(input) && NMainLoop::HaveInterfaceCommand(),
        "the fresh clue modal did not consume its native close command");
    Require(!script->IsUIActionIDPresent(event), "closing a fresh clue left its actual script UI action waiting forever");
    NMainLoop::StepApp(false,false,false);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "the fresh clue fixture retained its modal after closing");
}

class ReadinessMission: public NGame::CMissionBase
{
    OBJECT_NOCOPY_METHODS(ReadinessMission);
    CObj<NUI::CDesktopWindow> desktop;
public:
    bool clientSequence = false;
    void Attach(NWorld::CWorld* world, NUI::CDesktopWindow* top) { pWorld = world; desktop = top; }
    void AttachActive(NGame::IPlayerTracker* player) { pActivePlayer = player; }
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    bool IsSequence() const override { return clientSequence; }
    bool IsRealTime() const override { return true; }
    NUI::CDesktopWindow* GetDesktop() const override { return desktop; }
};

class IconSceneReached: public std::exception {};

class IconGateMission: public ReadinessMission
{
    OBJECT_NOCOPY_METHODS(IconGateMission);
public:
    mutable int sceneQueries = 0;
    NGScene::IGameView* GetScene() const override { ++sceneQueries; throw IconSceneReached(); }
};

class MissionIconProbe: public NUI::CMissionUI
{
    OBJECT_NOCOPY_METHODS(MissionIconProbe);
public:
    void RefreshIcons(bool traps) { if (traps) UpdateTraps(); else UpdateClues(); }
    void RefreshEnemies() { UpdateEnemies(); }
    void RefreshHits(STime time) { UpdateHits(time); }
};

static void OverlayIconVisibility(bool traps)
{
    CObj<IconGateMission> mission = new IconGateMission;
    CObj<MissionIconProbe> desktop = new MissionIconProbe;
    CObj<NUI::CText> pause = new NUI::CText;
    CObj<CObjectBase> icon = pSSClasses->CreateObject(int(traps ? 0xB3618130u : 0xB3131150u));
    Require(IsValid(icon), "native overlay icon class is not registered");
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[mission.GetPtr()] = 1; references.refs[1] = mission.GetPtr();
    references.ids[pause.GetPtr()] = 2; references.refs[2] = pause.GetPtr();
    references.ids[icon.GetPtr()] = 3; references.refs[3] = icon.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CPtr<NGame::IMission> missionRef = mission.GetPtr();
    list<CObj<CObjectBase>> icons; icons.push_back(icon);
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
      writer.Add(2, &missionRef); writer.Add(27, &pause); writer.Add(traps ? 10 : 11, &icons); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(desktop.GetPtr())->operator&(reader); }
    auto countRetained = [&]() {
        int count = 0;
        CStructureNetworkContext graph;
        graph.include = [&](CObjectBase* object) { if (object == icon.GetPtr()) ++count; return false; };
        CMemoryStream traversal;
        { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
          static_cast<CObjectBase*>(desktop.GetPtr())->operator&(writer); }
        return count;
    };
    Require(countRetained() == 1, "overlay fixture did not restore its existing native icon");
    NGlobal::SetVar("ui_showicons", 0);
    bool queriedScene = false;
    try { desktop->RefreshIcons(traps); } catch (const IconSceneReached&) { queriedScene = true; }
    Require(!queriedScene, "disabled overlay icons still queried the scene to rebuild markers");
    Require(countRetained() == 0, "disabling overlay icons retained a stale marker");
    NGlobal::SetVar("ui_showicons", 1);
    try { desktop->RefreshIcons(traps); } catch (const IconSceneReached&) { queriedScene = true; }
    Require(queriedScene && mission->sceneQueries == 1, "enabling overlay icons did not restore native marker updates");
}

static void AIPlayerMissionReadiness()
{
    CObj<ReadinessWorld> world = new ReadinessWorld;
    CObj<ReadinessMission> mission = new ReadinessMission;
    CObj<NUI::CDesktopWindow> gameplay = dynamic_cast<NUI::CDesktopWindow*>(
        pSSClasses->CreateObject(int(0xB0241942)));
    CObj<NGame::IPlayerTracker> player = dynamic_cast<NGame::IPlayerTracker*>(
        pSSClasses->CreateObject(int(0xB2022800))); // Actual CPlayerTracker.
    Require(IsValid(gameplay) && IsValid(player), "AI-player readiness fixture classes are not registered");
    CObj<NRPG::CGlobalPlayer> global = new NRPG::CGlobalPlayer;
    global->bAIPlayer = false;
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[global.GetPtr()] = 1; references.refs[1] = global.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); writer.Add(4, &global); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(player.GetPtr())->operator&(reader); }
    Require(player->GetGlobalPlayer() == global.GetPtr(), "native player tracker lost its restored global player");
    mission->Attach(world, gameplay);
    mission->AttachActive(player);
    Require(mission->IsReady(), "human player was not ready in the ordinary mission desktop");
    global->bAIPlayer = true;
    Require(!mission->IsReady(), "AI-controlled active player remained eligible for manual mission interaction");
    global->bAIPlayer = false;
    Require(mission->IsReady(), "returning to a human active player did not restore mission readiness");
    mission->AttachActive(nullptr);
    Require(mission->IsReady(), "AI-player readiness gate broke the pre-player initialization path");
}

static void MissionOverlayReadiness()
{
    CObj<ReadinessWorld> world = new ReadinessWorld;
    CObj<ReadinessMission> mission = new ReadinessMission;
    mission->Attach(world, nullptr);
    Require(mission->IsReady(), "readiness fixture without an overlay is not interactive");
    CObj<NUI::CDesktopWindow> gameplay = dynamic_cast<NUI::CDesktopWindow*>(
        pSSClasses->CreateObject(int(0xB0241942))); // Actual registered CMissionUI.
    Require(IsValid(gameplay), "native mission desktop class is not registered");
    mission->Attach(world, gameplay);
    Require(mission->IsReady(), "ordinary mission desktop incorrectly blocks interaction");
    vector<CObj<NUI::CDesktopWindow>> overlays;
    overlays.emplace_back(new NUI::CDesktopWindow);
    overlays.emplace_back(new NUI::CMissionDlgUI);
    overlays.emplace_back(new NUI::CMissionMovieUI);
    overlays.emplace_back(new NUI::CMissionFadeUI);
    for (const auto& overlay: overlays) {
        mission->Attach(world, overlay);
        Require(!mission->IsReady(), "mission remained interactive behind a noninteractive desktop overlay");
    }
    mission->Attach(world, gameplay);
    Require(mission->IsReady(), "closing an overlay did not restore ordinary mission interaction");
    mission->clientSequence = true;
    Require(!mission->IsReady(), "desktop readiness bypassed a client-side cinematic sequence");
    mission->clientSequence = false;
    world->sequence = true;
    Require(!mission->IsReady(), "desktop readiness bypassed the remaster's world-sequence guard");
    world->sequence = false;
    Require(mission->IsReady(), "sequence completion did not restore mission readiness");
}

class FollowCameraDrainMission: public CinematicDrainMission
{
    OBJECT_NOCOPY_METHODS(FollowCameraDrainMission);
    CObj<ICamera> selectedCamera = ::CreateCamera(CAMERA_PC);
public:
    int completionEvents = 0;
    ICamera* GetCamera() const override { return selectedCamera; }
    bool HasLocator() const { return IsValid(pExecLocator); }
    bool AdvanceLocator(STime time) { Require(IsValid(pExecLocator),"camera command fixture has no executor"); return pExecLocator->Update(time); }
    void FinishLocator() { if(IsValid(pExecLocator)){pExecLocator->Finished();pExecLocator=nullptr;} }
    void DoEvent(NWorld::CCommand* command) override {
        CObj<NWorld::CCommand> release=command;
        if(dynamic_cast<NWorld::CCmdInterfaceEvent*>(command))++completionEvents;
    }
};

static void FollowCameraCommandPolicy(int mode)
{
    Require(mode>=0 && mode<=4,"unknown follow-camera policy mode");
    struct OptionRestore {NGlobal::CValue value;~OptionRestore(){NGlobal::SetVar("ui_followcamera",value);}}
        restore{NGlobal::GetVar("ui_followcamera",1)};
    const bool follow=mode==2;
    NGlobal::SetVar("ui_followcamera",follow?1:0);
    CObj<QueuedCameraWorld> world=new QueuedCameraWorld;
    CObj<FollowCameraDrainMission> mission=new FollowCameraDrainMission;
    CObj<NUI::CMissionUI> desktop=new NUI::CMissionUI(NUI::SWindowInfo(),mission);
    mission->Attach(world,desktop);
    Require(mission->GetFollowCameraState()==follow,"native follow-camera option handler did not update its actual mission getter");
    auto* camera=mission->GetCamera();Require(camera,"follow-camera fixture did not create a real native PC camera");
    const ICamera::SCameraPos start(CVec3(10,20,5),20,.7f,0,0,.7f);
    const CVec3 delta(1,2,0);camera->SetPlacement(start);
    camera->ScrollAnchor(delta,true,false);ICamera::SCameraPos control;camera->GetPlacement(&control);
    Require(fabs2(control.ptAnchor-start.ptAnchor-delta)<1e-10f,"native camera did not scroll before any command was queued");
    camera->SetPlacement(start);
    auto target=start;target.ptAnchor+=CVec3(2,1,0);
    if(mode==0 || mode==2)world->commands.emplace_back(new NWorld::CUICmdUnitCamera(nullptr,1,false,0.f,nullptr));
    else if(mode==1)world->commands.emplace_back(new NWorld::CUICmdPointCamera(VNULL3,2,false,0.f,false,1,0));
    else if(mode==3)world->commands.emplace_back(new NWorld::CUICmdScriptMoveCamera(vector<ICamera::SCameraPos>{target},1000));
    world->commands.emplace_back(new NWorld::CUICmdTutorialMode(true));
    mission->Drain();
    Require(mission->TutorialEnabled() && world->next==world->commands.size(),"camera filtering discarded an unrelated UI command or stalled the real queue drain");
    if(mode==3) {
        Require(mission->HasLocator(),"disabled automatic follow discarded a scripted camera move");
        Require(!mission->AdvanceLocator(1000) && !mission->AdvanceLocator(1500),"script camera move ended before its native transition time");
        ICamera::SCameraPos middle;camera->GetPlacement(&middle);
        Require(fabs2(middle.ptAnchor-(start.ptAnchor+target.ptAnchor)*.5f)<1e-10f,"script camera move lost its native interpolation with follow disabled");
        Require(mission->AdvanceLocator(2000),"script camera move did not finish at its native transition boundary");
        ICamera::SCameraPos end;camera->GetPlacement(&end);mission->FinishLocator();
        Require(fabs2(end.ptAnchor-target.ptAnchor)<1e-10f && mission->completionEvents==1 && !mission->HasLocator(),
            "script camera move lost its final pose or single completion event");
        std::printf("native follow-camera mode=3 follow=0 script=accepted interpolation/final/completion passed\n");std::fflush(stdout);
        return;
    }
    const bool locator=mission->HasLocator();
    camera->ScrollAnchor(delta,true,false);ICamera::SCameraPos after;camera->GetPlacement(&after);
    const bool moved=fabs2(after.ptAnchor-start.ptAnchor-delta)<1e-10f;
    std::printf("native follow-camera mode=%d follow=%d executor=%d scrollMoved=%d expectedExecutor=%d\n",mode,int(follow),int(locator),int(moved),int(follow));std::fflush(stdout);
    mission->FinishLocator();
    Require(locator==follow && moved!=follow,"automatic camera command locked the native camera despite follow being disabled");
    camera->SetPlacement(start);camera->ScrollAnchor(delta,true,false);camera->GetPlacement(&after);
    Require(fabs2(after.ptAnchor-start.ptAnchor-delta)<1e-10f,"finishing the native automatic camera command failed to release its scroll lock");
}

static void CinematicCameraCommands(int kind)
{
    CObj<NUI::CDesktopWindow> desktop;
    if (kind == 0) desktop = new NUI::CMissionDlgUI;
    else if (kind == 1) desktop = new NUI::CMissionMovieUI;
    else desktop = new NUI::CMissionFadeUI;
    CObj<QueuedCameraWorld> world = new QueuedCameraWorld;
    CObj<CinematicDrainMission> mission = new CinematicDrainMission;
    mission->Attach(world, desktop);
    world->commands.emplace_back(new NWorld::CUICmdUnitCamera(nullptr, 1, false, 0.f, nullptr));
    world->commands.emplace_back(new NWorld::CUICmdTutorialMode(true));
    // Exercise the actual mission queue drain: automatic locators must be
    // rejected before they instantiate a camera executor or lock the camera.
    mission->Drain();
    Require(mission->TutorialEnabled(), "cinematic command filter discarded an unrelated script command");
    world->commands.emplace_back(new NWorld::CUICmdScriptMoveCamera({}, 0));
    bool scriptedCameraAccepted = false;
    try { mission->Drain(); }
    catch (const CameraExecutorReached&) { scriptedCameraAccepted = true; }
    Require(scriptedCameraAccepted, "cinematic command filter blocked a scripted camera move");

    CObj<NUI::CDesktopWindow> ordinary = new NUI::CDesktopWindow;
    CObj<QueuedCameraWorld> ordinaryWorld = new QueuedCameraWorld;
    CObj<CinematicDrainMission> ordinaryMission = new CinematicDrainMission;
    ordinaryMission->Attach(ordinaryWorld, ordinary);
    ordinaryWorld->commands.emplace_back(new NWorld::CUICmdUnitCamera(nullptr, 1, false, 0.f, nullptr));
    bool ordinaryCameraAccepted = false;
    try { ordinaryMission->Drain(); }
    catch (const CameraExecutorReached&) { ordinaryCameraAccepted = true; }
    Require(ordinaryCameraAccepted, "ordinary mission desktop lost automatic camera commands");
}

static void CompletedZoneRecommendation()
{
    CObj<NRPG::CGlobalPlayer> player = new NRPG::CGlobalPlayer;
    CObj<NRPG::CUnit> mercenary = new NRPG::CUnit;
    mercenary->skills[NDb::ST_VP] = new NRPG::CDynamicSkill(100);
    mercenary->skills[NDb::ST_LEVEL] = new NRPG::CDynamicSkill(5);
    player->mercs.push_back(mercenary);
    CObj<NScenario::CScenarioTracker> tracker = new NScenario::CScenarioTracker;
    CObj<NDb::CDBScenarioClue> clueRecord = new NDb::CDBScenarioClue;
    clueRecord->clueType = NDb::CT_PERSON;
    CObj<NScenario::CScenarioClue> firstClue = new NScenario::CScenarioClue(clueRecord, 1);
    CObj<NScenario::CScenarioClue> secondClue = new NScenario::CScenarioClue(clueRecord, 2);
    vector<CObj<NDb::CDBScenarioZone>> zoneRecords;
    auto makeZone = [&](NScenario::CScenarioClue* clue, int difficulty) {
        CObj<NDb::CDBScenarioZone> record = new NDb::CDBScenarioZone;
        zoneRecords.push_back(record);
        CObj<NScenario::CScenarioZone> zone = new NScenario::CScenarioZone(record, difficulty);
        // Populate one person-clue slot through the existing save field. No
        // map resource is needed for the recommendation's scenario graph.
        record->templatesIDs.push_back(1);
        unordered_map<int, NScenario::CScenarioZone::STemplate> slots;
        slots[1].nPersonSlots = slots[1].nEmptyPersonSlots = 1;
        CMemoryStream state;
        { CStructureSaver writer(state, CStructureSaver::WRITE); writer.Add(13, &slots); }
        state.Seek(0);
        { CStructureSaver reader(state, CStructureSaver::READ); static_cast<CObjectBase*>(zone.GetPtr())->operator&(reader); }
        zone->PlaceClue(clue);
        zone->SetDifficulty(difficulty);
        tracker->OpenZone(zone);
        return zone;
    };
    auto first = makeZone(firstClue, 1), second = makeZone(secondClue, 3);
    Require(tracker->GetRecommendedZone(player) == first.GetPtr(), "fixture zone priority is unexpected");
    first->SetPassed();
    Require(tracker->GetRecommendedZone(player) == second.GetPtr(),
        "completed mission with a leftover clue was recommended again");
    second->SetPassed();
    Require(!tracker->GetRecommendedZone(player), "all completed missions still produced a recommendation");
    first->SetPassed(false);
    Require(tracker->GetRecommendedZone(player) == first.GetPtr(), "unfinished mission disappeared from recommendations");
    firstClue->SetDestroyed(true);
    Require(!tracker->GetRecommendedZone(player), "destroyed clue produced a recommendation");
    firstClue->SetDestroyed(false);
    tracker->BlockZone(first);
    Require(!tracker->GetRecommendedZone(player), "blocked mission produced a recommendation");
}

static void PerkTooltipParameters()
{
    CObj<NDb::CDBPerk> perk = new NDb::CDBPerk;
    CObj<NDb::CString> description = new NDb::CString;
    description->szStr = L"<value paramonepoint> <value paramoneperc> <value paramtwopoint> <value paramtwoperc>";
    perk->pToolTip = description;
    perk->params = {1.256f, .346f};
    CObj<NUI::CToolTip> tooltip = NUI::CreatePerkToolTip(nullptr, perk);
    auto* text = dynamic_cast<NUI::CText*>(tooltip->GetChildByID(""));
    Require(text && text->GetText() == description->szStr, "perk tooltip lost its localized description");
    wstring value;
    Require(text->GetVal(L"paramonepoint", &value) && value == L"1",
        "perk tooltip omitted its numeric point parameter");
    Require(text->GetVal(L"paramoneperc", &value) && value == L"26", "perk tooltip percentage did not round its multiplier delta");
    Require(text->GetVal(L"paramtwopoint", &value) && value == L"0", "perk tooltip point parameter did not truncate");
    Require(text->GetVal(L"paramtwoperc", &value) && value == L"35", "perk tooltip fractional percentage is incorrect");
    Require(perk->params == vector<float>({1.256f, .346f}), "tooltip formatting changed perk mechanics");
    perk->params = {2.f, -.4f};
    tooltip = NUI::CreatePerkToolTip(nullptr, perk);
    text = dynamic_cast<NUI::CText*>(tooltip->GetChildByID(""));
    Require(text->GetVal(L"paramoneperc", &value) && value == L"100", "perk tooltip multiplier percentage is incorrect");
    Require(text->GetVal(L"paramtwoperc", &value) && value == L"-40", "perk tooltip negative percentage is incorrect");
    perk->params = {1.f};
    tooltip = NUI::CreatePerkToolTip(nullptr, perk);
    text = dynamic_cast<NUI::CText*>(tooltip->GetChildByID(""));
    Require(text->GetVal(L"paramoneperc", &value) && value == L"100", "perk tooltip changed the exact-one percentage boundary");
    Require(!text->GetVal(L"paramtwopoint", &value), "single-parameter perk acquired a second substitution");
    perk->params.clear();
    tooltip = NUI::CreatePerkToolTip(nullptr, perk);
    text = dynamic_cast<NUI::CText*>(tooltip->GetChildByID(""));
    Require(!text->GetVal(L"paramonepoint", &value), "parameter-free perk inherited a numeric substitution");
    perk->pToolTip = nullptr;
    perk->szUserName = "Fallback perk";
    tooltip = NUI::CreatePerkToolTip(nullptr, perk);
    text = dynamic_cast<NUI::CText*>(tooltip->GetChildByID(""));
    Require(text && text->GetText() == L"Fallback perk", "perk tooltip lost its name fallback");
}


static void AmbientVariantEmpty(int mode)
{
    Require(mode>=0 && mode<=5,"unknown ambient variant case");
    CObj<NDb::CAmbientLight> primary=new NDb::CAmbientLight,fallback=new NDb::CAmbientLight;
    CObj<NDb::CTAmbientLight> pool=new NDb::CTAmbientLight,alternate=new NDb::CTAmbientLight;
    for(int i=0;i<2;++i){auto* p=i?fallback.GetPtr():primary.GetPtr();const float k=0.1f*(i+1);
        p->vAmbientColor=CVec3(k,k+0.01f,k+0.02f);p->vGroundAmbientColor=CVec3(k+0.03f,k+0.04f,k+0.05f);
        p->vLightColor=CVec3(k+0.06f,k+0.07f,k+0.08f);p->vGlossColor=CVec3(k,k,k);p->vFogColor=CVec3(k,k,k);
        p->vVapourColor=CVec3(k,k,k);p->vShadowColor=CVec3(k,k,k);p->vBackColor=CVec3(k,k,k);
        p->fPitch=30+i;p->fYaw=45+i;p->fFogDistance=100+i;p->fVapourHeight=2+i;p->fVapourDensity=k;
        p->fVapourNoiseParam=1+i;p->fVapourSpeed=2+i;p->fVapourSwitchTime=3+i;
        p->fFogStartDistance=10+i;p->fVapourStartHeight=1+i;p->fBlurStrength=k;p->bInGameUse=true;p->pSky=0;p->pGF2Light=0;
        NDb::SVariantFlags flags;flags.flags={int(NWorld::TOD_DAY)};p->flags.push_back(flags);
    }
    pool->variants.push_back(primary.GetPtr());pool->roulette.AddSector(3);
    alternate->variants.push_back(fallback.GetPtr());alternate->roulette.AddSector(7);
    if(mode!=2)primary->pGF2Light=alternate.GetPtr();
    if(mode==0 || mode>=4)primary->flags[0].flags={int(NWorld::TOD_NIGHT)};
    if(mode==1 || mode>=4)fallback->flags[0].flags={int(NWorld::TOD_NIGHT)};
    const vector<int> params=mode==4?vector<int>():vector<int>{int(NWorld::TOD_DAY)};
    auto copied=[](NDb::CAmbientLightReal* actual,NDb::CAmbientLight* expected){
        Require(actual && fabs2(actual->vAmbientColor-expected->vAmbientColor)<1e-10f && fabs2(actual->vGroundAmbientColor-expected->vGroundAmbientColor)<1e-10f && fabs2(actual->vLightColor-expected->vLightColor)<1e-10f,"ambient factory lost primary/fallback colors");
        Require(actual->fFogDistance==expected->fFogDistance && actual->fFogStartDistance==expected->fFogStartDistance && actual->fPitch==expected->fPitch && actual->fYaw==expected->fYaw && actual->fVapourDensity==expected->fVapourDensity && actual->fBlurStrength==expected->fBlurStrength && actual->bInGameUse==expected->bInGameUse,"ambient factory lost copied light properties");};
    for(int seed: {1,123,90210}){
        SRand random{SRandomSeed(seed)},reference{SRandomSeed(seed)};const int draws=mode==0?0:mode<=2?1:2;
        for(int i=0;i<draws;++i)reference.GetFloat(0,1);
        std::printf("native ambient variant mode=%d seed=%d entering draws=%d\n",mode,seed,draws);std::fflush(stdout);
        CPtr<NDb::CAmbientLightReal> result=mode==5?pool->GetLight(&random):pool->GetLight(&random,params);
        std::printf("native ambient variant mode=%d seed=%d result=%d fallback=%d state=%d expected=%d\n",mode,seed,int(IsValid(result)),int(IsValid(result) && IsValid(result->pGF2Light)),random.seed.nSeed,reference.seed.nSeed);std::fflush(stdout);
        Require(random.seed.nSeed==reference.seed.nSeed,"empty ambient selection rerolled or consumed unexpected random draws");
        if(mode==0)Require(!IsValid(result),"no matching ambient variant did not return null");
        else {copied(result,primary);const bool hasFallback=mode>=3;
            Require(IsValid(result->pGF2Light)==hasFallback,"missing optional ambient fallback did not preserve only the primary light");
            if(hasFallback)copied(result->pGF2Light,fallback);}
    }
    Require(pool->variants.size()==1 && alternate->variants.size()==1 && pool->roulette.GetSectorValue(0)==3 && alternate->roulette.GetSectorValue(0)==7,"ambient selection changed authored variant weights");
}

static void MechanicalStepSound()
{
    CObj<NDb::CRPGArmor> armor = new NDb::CRPGArmor;
    armor->nAISoundType = 17;
    CObj<NDb::CPanzerklein> suit = new NDb::CPanzerklein;
    CObj<NDb::CTSound> ordinary = new NDb::CTSound, mechanical = new NDb::CTSound, empty = new NDb::CTSound;
    CObj<NDb::CSoundVariant> ordinaryVariant = new NDb::CSoundVariant, mechanicalVariant = new NDb::CSoundVariant;
    CObj<NDb::CSound> ordinarySound = new NDb::CSound, mechanicalSound = new NDb::CSound;
    ordinaryVariant->pSound = ordinarySound.GetPtr(); mechanicalVariant->pSound = mechanicalSound.GetPtr();
    ordinary->variants.push_back(ordinaryVariant.GetPtr()); ordinary->roulette.AddSector(1.f);
    mechanical->variants.push_back(mechanicalVariant.GetPtr()); mechanical->roulette.AddSector(1.f);
    armor->pSoundStep = ordinary.GetPtr(); suit->pStepSound = mechanical.GetPtr();
    SRand random(SRandomSeed(123));
    Require(NWorld::SelectUnitStepSound(armor, suit, &random) == mechanicalSound.GetPtr(),
        "worn Panzerklein played the pilot armor's ordinary footsteps");
    Require(NWorld::SelectUnitStepSound(nullptr, suit, &random) == mechanicalSound.GetPtr(), "mechanical footsteps depended on pilot armor");
    Require(NWorld::SelectUnitStepSound(armor, nullptr, &random) == ordinarySound.GetPtr(), "ordinary footsteps changed for an unsuited unit");
    suit->pStepSound = nullptr;
    Require(NWorld::SelectUnitStepSound(armor, suit, &random) == ordinarySound.GetPtr(), "missing mechanical footsteps lost the armor fallback");
    suit->pStepSound = empty.GetPtr();
    Require(!NWorld::SelectUnitStepSound(armor, suit, &random), "empty mechanical sound set produced a sound");
    armor->pSoundStep = empty.GetPtr();
    Require(!NWorld::SelectUnitStepSound(armor, nullptr, &random), "empty ordinary sound set produced a sound");
    Require(!NWorld::SelectUnitStepSound(nullptr, nullptr, &random), "missing footstep definitions produced a sound");
    Require(armor->nAISoundType == 17, "mechanical sound selection changed AI noise classification");
}
