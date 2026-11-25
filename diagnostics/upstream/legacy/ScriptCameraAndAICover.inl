static void WorldScriptCameraClipping(const char* databasePath,const char* resourcePath,int context,int mode)
{
    Require(context>=0 && context<=1 && mode>=0 && mode<=2,"unknown shared-world camera clipping case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"world camera clipping fixture started with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm shared world camera clipping regression");
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} frameClock{NMainLoop::SetTimeForHarness(1000)};
    CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    struct RestoreScripts {vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> variants;CPtr<NDb::CChapterMap> chapter;CPtr<NDb::CScript> entry;
        ~RestoreScripts(){for(auto& p:variants)if(IsValid(p.first))p.first->pScript=p.second;if(IsValid(chapter))chapter->pScript=entry;}} restore;
    // This command fixture must not enter a randomly placed encounter while drawing the map.
    struct SectorRestore {
        CObj<CChapterInfo> info;
        vector<SChapterSector> original;
        ~SectorRestore() { if(IsValid(info)) info->sectorsSet.swap(original); }
    } sectors;
    CObj<NRPG::CGlobalGame> game;
    if(context==0){
        CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());while(maps.MoveNext())if(IsValid(maps.Get()->pBackground)){restore.chapter=maps.Get();break;}
        Require(IsValid(restore.chapter),"world camera clipping fixture has no authored chapter");restore.entry=restore.chapter->pScript;restore.chapter->pScript=quiet.GetPtr();
        game=NRPG::CreateGlobalGame(1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        list<CPtr<NScenario::CScenarioClue>> clues;game->pScenarioTracker->GetAvailableClues(&clues);for(auto clue:clues)if(IsValid(clue))clue->SetJustFound(false);
        for(auto merc:game->players.front()->mercs){vector<CDBPtr<NDb::CMedal>> medals;merc->GetJustFoundMedals(&medals);}
        CDGPtr<CPtrFuncBase<CChapterInfo>> info=shareChapterInfo.Get(restore.chapter->GetRecordID());
        info.Refresh();sectors.info=info->GetValue();
        Require(IsValid(sectors.info),"camera clipping fixture lacks its actual cached map info");
        sectors.original=sectors.info->sectorsSet;auto& isolated=sectors.info->sectorsSet;
        isolated.erase(std::remove_if(isolated.begin(),isolated.end(),
            [](const SChapterSector& sector){return sector.eType==RANDOM;}),isolated.end());
        Require(!isolated.empty(),"camera clipping isolation removed every authored zone and exit");
        std::printf("camera clipping isolation mode=%d map=%d randomRemoved=%zu retained=%zu\n",
            mode,restore.chapter->GetRecordID(),sectors.original.size()-isolated.size(),isolated.size());
        CObj<NGame::CICBeginChapter> enter=new NGame::CICBeginChapter(restore.chapter->GetRecordID(),game);enter->Exec();
    }else{
        CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
        while(variants.MoveNext())if(IsValid(variants.Get()->pTemplate) && variants.Get()->pTemplate->GetRecordID()==2425){restore.variants.push_back({variants.Get(),variants.Get()->pScript});variants.Get()->pScript=quiet.GetPtr();}
        Require(!restore.variants.empty(),"world camera clipping fixture has no authored menu variants");CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    }
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* world=owner?dynamic_cast<NWorld::CWorld*>(owner->GetWorld()):nullptr;auto* view=owner?owner->GetScene():nullptr;auto* renderer=owner?owner->GetRenderGame():nullptr;
    Require(world && view && renderer && world->GetOwnScript(),"world camera clipping fixture lacks actual chapter/menu world, view, renderer or Lua");
    Require(context==0?owner->GetDesktop()!=nullptr:owner->GetDesktop()==nullptr,"world camera clipping fixture lost its real chapter/null-menu desktop distinction");
    // Materialize authored static world effects before taking geometry baselines.
    NMainLoop::SetTimeForHarness(1050);Require(NMainLoop::StepApp(true,false,false),"world camera clipping warm-up frame exited");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"world camera clipping warm-up opened an unrelated interface");
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}

    if(context==0){
        auto* ui=dynamic_cast<NUI::CChapterMapUI*>(owner->GetInterface()->GetChildByID("chaptermapUI"));
        Require(ui && ui->GetTravelForDiagnostics().find(" type=1 template=")==string::npos,
            "camera clipping fixture UI retained a random encounter");
    }
    auto* camera = owner->GetCamera();
    Require(camera && (context==0 ? owner->GetActivePlayer()!=nullptr : owner->GetActivePlayer()==nullptr),
        "world camera clipping fixture lost its actual player/null-player distinction");
    float initialNear=-1, initialFar=-1;
    camera->GetClipDistance(&initialNear,&initialFar);
    Require(std::fabs(initialNear-0.1f)<1e-6f && std::fabs(initialFar-100.f)<1e-6f,
        "quiet chapter/menu camera has unexpected default clipping planes");
    auto* script = world->GetOwnScript();
    const int rounds = mode==2 ? 2 : 1;
    for(int round=0;round<rounds;++round) {
        const float wantedNear = mode==0 ? initialNear : mode==1 ? 0.75f : round==0 ? 0.25f : 1.25f;
        const float wantedFar = mode==0 ? initialFar : mode==1 ? 45.f : round==0 ? 80.f : 70.f;
        const string command = mode==0 ? "WorldClippingProduced = 1" :
            "CameraSetClipping("+std::to_string(wantedNear)+","+std::to_string(wantedFar)+"); WorldClippingProduced = 1";
        Require(script->DoString(command.c_str())==0,"camera clipping fixture could not execute its real Lua producer");
        world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        const int top=script->GetTop();auto produced=script->GetGlobal("WorldClippingProduced");
        const bool ran=produced.IsNumber() && produced.GetNumber()==1;script->SetTop(top);
        Require(ran,"camera clipping Lua producer never ran");
        NMainLoop::SetTimeForHarness(1100+round*50);
        Require(NMainLoop::StepApp(true,false,false),"camera clipping native interface frame exited");
        if(NMainLoop::GetCurrentInterfaceForHarness()!=owner.GetPtr() || NMainLoop::HaveInterfaceCommand()) {
            auto* current=NMainLoop::GetCurrentInterfaceForHarness();
            std::printf("camera clipping transition context=%d mode=%d round=%d active=%s queued=%d\n",context,mode,round,
                current?typeid(*current).name():"null",int(NMainLoop::HaveInterfaceCommand()));
            if(auto* ui=dynamic_cast<NUI::CChapterMapUI*>(owner->GetInterface()->GetChildByID("chaptermapUI")))
                std::printf("%s",ui->GetTravelForDiagnostics().c_str());
        }
        Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),
            "camera clipping fixture opened an unrelated interface");
        auto* selected=owner->GetCamera();float actualNear=-1,actualFar=-1;
        selected->GetClipDistance(&actualNear,&actualFar);
        std::printf("native world camera clipping context=%d mode=%d round=%d selected=%d near=%.6f far=%.6f wantedNear=%.6f wantedFar=%.6f\n",
            context,mode,round,int(selected==camera),actualNear,actualFar,wantedNear,wantedFar);std::fflush(stdout);
        Require(selected==camera && std::fabs(actualNear-wantedNear)<1e-6f && std::fabs(actualFar-wantedFar)<1e-6f,
            "non-tactical world discarded its camera clipping command");
        CTransformStack transform;selected->GetTransform(&transform,CVec2(320,240));
        const auto& projection=transform.GetProjection().forward;
        CVec4 nearPoint,farPoint,beforeNear,beyondFar;
        projection.RotateHVector(&nearPoint,CVec4(0,0,wantedNear,1));
        projection.RotateHVector(&farPoint,CVec4(0,0,wantedFar,1));
        projection.RotateHVector(&beforeNear,CVec4(0,0,wantedNear*0.5f,1));
        projection.RotateHVector(&beyondFar,CVec4(0,0,wantedFar*2,1));
        Require(nearPoint.w>0 && farPoint.w>0 && std::fabs(nearPoint.z/nearPoint.w)<1e-5f &&
            std::fabs(farPoint.z/farPoint.w-1)<1e-5f && beforeNear.z<0 && beyondFar.z>beyondFar.w,
            "camera clipping did not reach the actual near/far projection boundaries");
        Require(!world->GetUICommand(),"camera clipping command pump left a queued command");
    }
}

static void WorldScriptHints(const char* databasePath,const char* resourcePath,int context,int mode)
{
    Require(context>=0 && context<=1 && mode>=0 && mode<=3,"unknown shared-world hint case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"world hint fixture started with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm shared world hint regression");
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} frameClock{NMainLoop::SetTimeForHarness(1000)};
    CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    struct RestoreScripts {vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> variants;CPtr<NDb::CChapterMap> chapter;CPtr<NDb::CScript> entry;
        ~RestoreScripts(){for(auto& p:variants)if(IsValid(p.first))p.first->pScript=p.second;if(IsValid(chapter))chapter->pScript=entry;}} restore;
    CObj<NRPG::CGlobalGame> game;
    if(context==0){
        CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());while(maps.MoveNext())if(IsValid(maps.Get()->pBackground)){restore.chapter=maps.Get();break;}
        Require(IsValid(restore.chapter),"world hint fixture has no authored chapter");restore.entry=restore.chapter->pScript;restore.chapter->pScript=quiet.GetPtr();
        game=NRPG::CreateGlobalGame(1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        list<CPtr<NScenario::CScenarioClue>> clues;game->pScenarioTracker->GetAvailableClues(&clues);for(auto clue:clues)if(IsValid(clue))clue->SetJustFound(false);
        for(auto merc:game->players.front()->mercs){vector<CDBPtr<NDb::CMedal>> medals;merc->GetJustFoundMedals(&medals);}
        CObj<NGame::CICBeginChapter> enter=new NGame::CICBeginChapter(restore.chapter->GetRecordID(),game);enter->Exec();
    }else{
        CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
        while(variants.MoveNext())if(IsValid(variants.Get()->pTemplate) && variants.Get()->pTemplate->GetRecordID()==2425){restore.variants.push_back({variants.Get(),variants.Get()->pScript});variants.Get()->pScript=quiet.GetPtr();}
        Require(!restore.variants.empty(),"world hint fixture has no authored menu variants");CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    }
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* world=owner?dynamic_cast<NWorld::CWorld*>(owner->GetWorld()):nullptr;auto* view=owner?owner->GetScene():nullptr;auto* renderer=owner?owner->GetRenderGame():nullptr;
    Require(world && view && renderer && world->GetOwnScript(),"world hint fixture lacks actual chapter/menu world, view, renderer or Lua");
    Require(context==0?owner->GetDesktop()!=nullptr:owner->GetDesktop()==nullptr,"world hint fixture lost its real chapter/null-menu desktop distinction");
    // Materialize authored static world effects before taking geometry baselines.
    NMainLoop::SetTimeForHarness(1050);Require(NMainLoop::StepApp(true,false,false),"world hint warm-up frame exited");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"world hint warm-up opened an unrelated interface");
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}

    struct HintOptionRestore {NGlobal::CValue previous;~HintOptionRestore(){NGlobal::SetVar("ui_showhints",previous);}} option{NGlobal::GetVar("ui_showhints",1)};
    NGlobal::SetVar("ui_showhints",mode==2?1:0);
    auto* script=world->GetOwnScript();auto* scriptGame=script->GetGlobalGame();Require(scriptGame,"world hint fixture has no script global game");scriptGame->hintsSet.clear();
    Require(context==0?owner->GetRPGGame()==scriptGame:owner->GetRPGGame()==nullptr,"world hint fixture lost its chapter/campaignless-menu distinction");
    CDBIterator<NDb::CUIHint> hints(*NDatabase::GetTable<NDb::CUIHint>());CDBPtr<NDb::CUIHint> hint;
    while(hints.MoveNext())if(IsValid(hints.Get()->pString) && hints.Get()->nSequenceID>0 && (!IsValid(hint) || hints.Get()->nSequenceID<hint->nSequenceID))hint=hints.Get();
    Require(IsValid(hint),"world hint fixture lacks an authored positive-reward hint");
    auto xp=[](NRPG::CGlobalGame* current){float result=0;if(current)for(auto player:current->players)for(auto merc:player->mercs)if(IsValid(merc))result+=merc->fXP;return result;};
    const float originalXP=xp(scriptGame);const bool show=mode==0 || mode==2;const bool tutorial=mode==0;
    auto flag=[&](){struct Flag {bool enabled=false;int operator&(CStructureSaver& f){f.Add(31,&enabled);return 0;}} value;
        struct RenderBase {Flag* value;int operator&(CStructureSaver& f){f.Add(1,value);return 0;}} render{&value};
        CStructureNetworkContext references;auto fields=BorrowedObjectFields(owner,&references);
        {CStructureSaver reader(fields,CStructureSaver::READ,&references);if(context==0)reader.Add(1,&value);else reader.Add(1,&render);}return value.enabled;};
    auto number=[&](const char* name,int expected){const int top=script->GetTop();auto value=script->GetGlobal(name);const bool matches=value.IsNumber() && value.GetNumber()==expected;script->SetTop(top);return matches;};
    auto absent=[&](const char* name){const int top=script->GetTop();const bool nil=script->GetGlobal(name).IsNil();script->SetTop(top);return nil;};
    auto frame=[&](STime tick,bool active=true){if(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand())world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);NMainLoop::SetTimeForHarness(tick);Require(NMainLoop::StepApp(active,false,false),"world hint native frame exited");};
    const string toggle=mode==0?"SetTutorialMode(1); ":mode==3?"SetTutorialMode(1); SetTutorialMode(nil); ":"SetTutorialMode(nil); ";
    const string lua=toggle+"WorldHintID = ShowHint("+std::to_string(hint->GetRecordID())+"); WorldHintProduced = 1; while IsUIActionIDPresent(WorldHintID) do Sleep(1) end; WorldHintReleased = 1";
    Require(script->DoString(lua.c_str())==0,"world hint fixture could not execute actual Lua tutorial/hint producers");
    world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);Require(number("WorldHintProduced",1),"world hint fixture did not execute its native producers");
    const int top=script->GetTop();auto id=script->GetGlobal("WorldHintID");Require(id.IsNumber(),"native ShowHint returned no wait ID");const int eventID=int(id.GetNumber());script->SetTop(top);
    Require(script->IsUIActionIDPresent(eventID) && absent("WorldHintReleased"),"world hint wait completed before the native show/suppress consumer");
    frame(1100);Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr(),"world hint consumer changed the interface before executing its queued modal");
    std::printf("native world hints context=%d mode=%d stage=dispatch tutorial=%d expected=%d queued=%d show=%d event=%d hint=%d reward=%d xp=%.3f\n",context,mode,int(flag()),int(tutorial),int(NMainLoop::HaveInterfaceCommand()),int(show),eventID,hint->GetRecordID(),hint->nSequenceID,xp(scriptGame));std::fflush(stdout);
    Require(flag()==tutorial,"non-tactical world discarded its tutorial-mode command");
    Require(NMainLoop::HaveInterfaceCommand()==show,"non-tactical world ignored the hint visibility policy");
    if(show){frame(1150);auto* modal=NMainLoop::GetCurrentInterfaceForHarness();Require(modal && pSSClasses->GetObjectTypeID(modal)==int(0x1423130) && NMainLoop::GetInterfaceStackDepth()==2,"world hint did not open the actual native modal");
        CStructureNetworkContext references;auto fields=BorrowedObjectFields(modal,&references);CPtr<NGame::IMission> origin;int modalEvent=-1;
        {CStructureSaver reader(fields,CStructureSaver::READ,&references);reader.Add(2,&origin);reader.Add(3,&modalEvent);}Require(origin.GetPtr()==owner.GetPtr() && modalEvent==eventID,"world hint lost its origin or native pending event");
        const STime coveredTime=world->GetAimTime()->GetValue();frame(1200);Require(world->GetAimTime()->GetValue()==coveredTime && script->IsUIActionIDPresent(eventID) && absent("WorldHintReleased"),"covered hint owner advanced its world or released its Lua wait");
        Require(context==0?xp(scriptGame)>originalXP:xp(scriptGame)==originalXP,"world hint changed its existing campaign/null-campaign XP contract");
        NInput::SEvent input;while(NInput::GetEvent(&input)){}NInput::PostEvent("cancel");Require(NInput::GetEvent(&input) && modal->ProcessEvent(input),"native hint modal did not accept its close event");frame(1250,false);
        Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && NMainLoop::GetInterfaceStackDepth()==1,"hint close did not restore its actual owner");
    }else Require(xp(scriptGame)==originalXP,"suppressed hint awarded experience");
    for(int tick=0;tick<12 && (!number("WorldHintReleased",1) || script->IsUIActionIDPresent(eventID));++tick)frame(1300+tick*50);
    Require(!script->IsUIActionIDPresent(eventID) && number("WorldHintReleased",1),"world hint command never released its native Lua UI wait");
    const float afterXP=xp(scriptGame);Require(scriptGame->hintsSet.size()==1 && scriptGame->hintsSet[0].GetPtr()==hint.GetPtr(),"world hint changed native one-shot registration");
    Require(script->DoString(("WorldHintReplay = ShowHint("+std::to_string(hint->GetRecordID())+"); WorldHintReplayDone = 1").c_str())==0,"world hint could not execute its one-shot replay control");
    for(int tick=0;tick<8 && !number("WorldHintReplayDone",1);++tick)frame(2000+tick*50);
    Require(number("WorldHintReplayDone",1) && absent("WorldHintReplay") && !NMainLoop::HaveInterfaceCommand() && NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && scriptGame->hintsSet.size()==1 && xp(scriptGame)==afterXP,"replayed hint reopened a modal or awarded experience twice");
    std::printf("native world hints context=%d mode=%d stage=complete tutorial=%d modal=%d released=%d xpBefore=%.3f xpAfter=%.3f replay=1\n",context,mode,int(flag()),int(show),int(!script->IsUIActionIDPresent(eventID)),originalXP,afterXP);std::fflush(stdout);
}

static void WorldScriptLighting(const char* databasePath,const char* resourcePath,int context,int mode)
{
    Require(context>=0 && context<=1 && mode>=0 && mode<=5,"unknown shared-world lighting case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"world lighting fixture started with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm shared world lighting regression");
    struct ClockRestore {STime previous;~ClockRestore(){NMainLoop::SetTimeForHarness(previous);}} frameClock{NMainLoop::SetTimeForHarness(1000)};
    CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    struct RestoreScripts {vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> variants;CPtr<NDb::CChapterMap> chapter;CPtr<NDb::CScript> entry;
        ~RestoreScripts(){for(auto& p:variants)if(IsValid(p.first))p.first->pScript=p.second;if(IsValid(chapter))chapter->pScript=entry;}} restore;
    CObj<NRPG::CGlobalGame> game;
    if(context==0){
        CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());while(maps.MoveNext())if(IsValid(maps.Get()->pBackground)){restore.chapter=maps.Get();break;}
        Require(IsValid(restore.chapter),"world lighting fixture has no authored chapter");restore.entry=restore.chapter->pScript;restore.chapter->pScript=quiet.GetPtr();
        game=NRPG::CreateGlobalGame(1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        list<CPtr<NScenario::CScenarioClue>> clues;game->pScenarioTracker->GetAvailableClues(&clues);for(auto clue:clues)if(IsValid(clue))clue->SetJustFound(false);
        for(auto merc:game->players.front()->mercs){vector<CDBPtr<NDb::CMedal>> medals;merc->GetJustFoundMedals(&medals);}
        CObj<NGame::CICBeginChapter> enter=new NGame::CICBeginChapter(restore.chapter->GetRecordID(),game);enter->Exec();
    }else{
        CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
        while(variants.MoveNext())if(IsValid(variants.Get()->pTemplate) && variants.Get()->pTemplate->GetRecordID()==2425){restore.variants.push_back({variants.Get(),variants.Get()->pScript});variants.Get()->pScript=quiet.GetPtr();}
        Require(!restore.variants.empty(),"world lighting fixture has no authored menu variants");CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    }
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* world=owner?dynamic_cast<NWorld::CWorld*>(owner->GetWorld()):nullptr;auto* view=owner?owner->GetScene():nullptr;auto* renderer=owner?owner->GetRenderGame():nullptr;
    Require(world && view && renderer && world->GetOwnScript(),"world lighting fixture lacks actual chapter/menu world, view, renderer or Lua");
    Require(context==0?owner->GetDesktop()!=nullptr:owner->GetDesktop()==nullptr,"world lighting fixture lost its real chapter/null-menu desktop distinction");
    // Materialize authored static world effects before taking geometry baselines.
    NMainLoop::SetTimeForHarness(1050);Require(NMainLoop::StepApp(true,false,false),"world lighting warm-up frame exited");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"world lighting warm-up opened an unrelated interface");
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}
    world->SetTimeOfDay(NWorld::TOD_DAY);
    vector<CDBPtr<NDb::CAmbientLight>> records;CDBIterator<NDb::CAmbientLight> ambients(*NDatabase::GetTable<NDb::CAmbientLight>());
    while(ambients.MoveNext() && records.size()<3)records.push_back(ambients.Get());Require(records.size()==3,"world lighting fixture lacks registered ambient records");
    for(int i=0;i<3;++i){auto* light=records[i].GetPtr();const float k=0.1f*(i+1);
        light->vAmbientColor=CVec3(k,k+0.01f,k+0.02f);light->vGroundAmbientColor=CVec3(k+0.03f,k+0.04f,k+0.05f);
        light->vLightColor=CVec3(k+0.06f,k+0.07f,k+0.08f);light->vGlossColor=CVec3(k,k,k);
        light->vFogColor=CVec3(k+0.09f,k+0.10f,k+0.11f);light->vVapourColor=CVec3(k,k,k);
        light->vShadowColor=CVec3(k,k,k);light->vBackColor=CVec3(k+0.12f,k+0.13f,k+0.14f);
        light->fPitch=30+i;light->fYaw=45+i;light->fFogDistance=100+10*i;light->fVapourHeight=2+i;
        light->fVapourDensity=0.1f;light->fVapourNoiseParam=1;light->fVapourSpeed=1;light->fVapourSwitchTime=1;
        light->fFogStartDistance=10+i;light->fVapourStartHeight=1;light->fBlurStrength=0;
        light->pSky=0;light->pGF2Light=0;light->bInGameUse=(i==0);light->flags.clear();
    }
    vector<CDBPtr<NDb::CTAmbientLight>> pools;CDBIterator<NDb::CTAmbientLight> ambientPools(*NDatabase::GetTable<NDb::CTAmbientLight>());
    while(ambientPools.MoveNext()){auto* p=ambientPools.Get();p->variants.clear();p->variants.push_back(records[0].GetPtr());p->roulette=CRoulette();p->roulette.AddSector(1);pools.push_back(p);}
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(world,&refs);CDBPtr<NDb::CTAmbientLight> zonePool;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(18,&zonePool);}
    NDb::CTAmbientLight* baselinePool=nullptr;for(const auto& p:pools)if(p.GetPtr()!=zonePool.GetPtr()){baselinePool=p;break;}Require(baselinePool,"world lighting fixture lacks a baseline template");
    baselinePool->variants[0]=records[2].GetPtr();
    NDb::CTAmbientLight* explicitPool=nullptr;for(const auto& p:pools)if(p.GetPtr()!=zonePool.GetPtr() && p.GetPtr()!=baselinePool){explicitPool=p;break;}Require(explicitPool,"world lighting fixture lacks a separate explicit template");
    if(IsValid(zonePool))zonePool->variants[0]=records[1].GetPtr();
    Require(context==0?!IsValid(zonePool):IsValid(zonePool),"chapter/menu default-light distinction changed");
    SRand paletteRandom(SRandomSeed(123));CPtr<NDb::CAmbientLightReal> baseline=baselinePool->GetLight(&paletteRandom);view->SetAmbient(baseline);
    auto* scene=view->GetGScene();CStructureNetworkContext sceneRefs;auto sceneFields=BorrowedObjectFields(scene,&sceneRefs);CPtr<CFuncBase<CVec3>> mean,animated;
    {CStructureSaver reader(sceneFields,CStructureSaver::READ,&sceneRefs);reader.Add(18,&mean);reader.Add(33,&animated);}Require(IsValid(mean) && IsValid(animated),"world lighting fixture lacks native ambient graph");
    auto binding=[&](){CStructureNetworkContext context;auto saved=BorrowedObjectFields(animated,&context);CPtr<NGScene::CLightAnimator> animator;
        {CStructureSaver reader(saved,CStructureSaver::READ,&context);reader.Add(3,&animator);}return animator;};
    auto geometry=[&](){CStructureNetworkContext context;auto saved=BorrowedObjectFields(scene,&context);list<CPtr<CObjectBase>> lights,particles;
        {CStructureSaver reader(saved,CStructureSaver::READ,&context);reader.Add(7,&lights);reader.Add(10,&particles);}return std::make_pair(std::count_if(lights.begin(),lights.end(),[](const CPtr<CObjectBase>& p){return IsValid(p);}),std::count_if(particles.begin(),particles.end(),[](const CPtr<CObjectBase>& p){return IsValid(p);}));};
    const auto originalGeometry=geometry();auto stock=NativeAuthoredEffectLights();
    CDBIterator<NDb::CEffect> effects(*NDatabase::GetTable<NDb::CEffect>());Require(effects.MoveNext(),"world lighting fixture lacks a registered effect");CDBPtr<NDb::CEffect> effect=effects.Get();effect->instances.clear();effect->lights.clear();effect->flags.clear();effect->lights.push_back(stock[0].instance.GetPtr());
    CDBIterator<NDb::CTEffect> effectPools(*NDatabase::GetTable<NDb::CTEffect>());Require(effectPools.MoveNext(),"world lighting fixture lacks a registered effect pool");CDBPtr<NDb::CTEffect> effectPool=effectPools.Get();effectPool->variants.clear();effectPool->variants.push_back(effect.GetPtr());effectPool->roulette=CRoulette();effectPool->roulette.AddSector(1);
    CObj<NGScene::CLightAnimator> loadedSeed;
    if(mode>=2 && mode<=4){
        Require(stock[0].info->fTStart==0,"world lighting fixture needs an authored light active at its launch time");
        view->SetAmbientEffect(effect,renderer->GetTime()->GetValue(),renderer->GetTime());loadedSeed=binding().GetPtr();Require(IsValid(loadedSeed),"world lighting fixture could not prime its actual cached light resource");
        CDGPtr<NGScene::CLightAnimator> ready=loadedSeed.GetPtr();bool active=false;const auto deadline=SDL_GetTicks()+10000;
        do {MarkNewDGFrame();ready.Refresh();active=ready->GetValue()->bActive;if(active)break;SDL_Delay(1);}while(SDL_GetTicks()<deadline);
        Require(active,"world lighting fixture's real cached light resource did not become active");
        if(mode!=3){view->SetAmbientEffect(nullptr,renderer->GetTime()->GetValue(),renderer->GetTime());view->SetAmbient(baseline);Require(!IsValid(binding()),"resource priming left a scene animation bound");}
    }
    auto* script=world->GetOwnScript();CObj<NScript::CLUAObjectPosition> point=new NScript::CLUAObjectPosition(CVec3(2,3,4));NScript::luaPushCObj(script->GetState(),point);script->SetGlobal("WorldLightingPosition");
    const string command=mode==0?"SetupAmbientLight("+std::to_string(explicitPool->GetRecordID())+")":mode==1?"SetTimeOfDay(5)":mode==2?"SetAmbientEffect("+std::to_string(effectPool->GetRecordID())+")":mode==3?"SetAmbientEffect(-1)":mode==4?"WorldLightingHandle = PlayEffect("+std::to_string(effectPool->GetRecordID())+",WorldLightingPosition)":"SetupAmbientLight(-1); WorldLightingHandle = PlayEffect(-1,WorldLightingPosition)";
    Require(script->DoString((command+"; WorldLightingExecuted = 1").c_str())==0,"world lighting fixture could not execute its real Lua producer");world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
    const int top=script->GetTop();auto executed=script->GetGlobal("WorldLightingExecuted");const bool produced=executed.IsNumber() && executed.GetNumber()==1;script->SetTop(top);Require(produced,"world lighting Lua producer did not run");
    auto* clock=renderer->GetTime();CDGPtr<CFuncBase<STime>> clockRead=clock;clockRead.Refresh();const STime beforeFrame=clockRead->GetValue();
    NMainLoop::SetTimeForHarness(1100);Require(NMainLoop::StepApp(true,false,false),"world lighting native interface frame exited");
    Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"world lighting fixture was covered by an unrelated interface command");
    clockRead.Refresh();const STime begin=context==0?clockRead->GetValue():beforeFrame;
    const auto shape=geometry();auto current=binding();auto* actual=view->GetPrevLight();CDGPtr<CFuncBase<CVec3>> sample=mean.GetPtr();sample.Refresh();const CVec3 color=sample->GetValue();
    std::printf("native world lighting context=%d mode=%d owner=%s ambient=%.3f mean=%.3f animation=%d extraLights=%lld extraParticles=%lld renderer=%u\n",context,mode,typeid(*owner.GetPtr()).name(),actual?actual->vAmbientColor.x:-1.f,color.x,int(IsValid(current)),static_cast<long long>(shape.first-originalGeometry.first),static_cast<long long>(shape.second-originalGeometry.second),unsigned(begin));std::fflush(stdout);
    if(mode<=1){const int expected=mode==1 && context==1?1:0;auto* wanted=records[expected].GetPtr();Require(actual && fabs2(actual->vAmbientColor-wanted->vAmbientColor)<1e-10f && fabs2(color-(wanted->vAmbientColor+wanted->vGroundAmbientColor)*0.5f)<1e-10f,"non-tactical world discarded its ambient-light command");}
    else if(mode==2){Require(IsValid(current),"non-tactical world discarded its ambient-animation command");CStructureNetworkContext contextRefs;auto saved=BorrowedObjectFields(current,&contextRefs);CDBPtr<NDb::CLightInstance> instance;STime actualBegin=0;CDGPtr<CFuncBase<STime>> actualClock;
        {CStructureSaver reader(saved,CStructureSaver::READ,&contextRefs);reader.Add(3,&actualBegin);reader.Add(4,&instance);reader.Add(5,&actualClock);}Require(instance.GetPtr()==stock[0].instance.GetPtr() && actualBegin==begin && actualClock.GetPtr()==clock,"shared ambient routing lost native instance/current clock identity");}
    else if(mode==3)Require(!IsValid(current),"non-tactical world discarded its ambient-clear command");
    else if(mode==4){const int savedTop=script->GetTop();auto handle=script->GetGlobal("WorldLightingHandle");auto* play=handle.IsUserData()?dynamic_cast<NWorld::CUICmdPlayEffect*>(handle.GetUserData()):nullptr;const bool played=play && IsValid(play->pHandle);script->SetTop(savedTop);
        Require(played && shape.first==originalGeometry.first+1 && shape.second==originalGeometry.second,"non-tactical world discarded its ordinary effect command");}
    else Require(actual==baseline.GetPtr() && !IsValid(current),"invalid ambient/effect ID changed existing world lighting");
    if(mode!=4)Require(shape==originalGeometry,"shared ambient command created ordinary effect geometry");
    Require(!world->GetUICommand(),"world lighting command pump left a queued command");
}

static void ScriptPlayEffect(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=4,"unknown ordinary-effect case");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm native ordinary effect regression");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}
    world->SetTimeOfDay(NWorld::TOD_DAY);
    CObj<NGScene::IGameView> view=NGScene::CreateNewView();
    CObj<NRender::IRenderGame> renderer=NRender::CreateRenderGame(world,view,nullptr);
    CObj<NativeAmbientEffectMission> mission=new NativeAmbientEffectMission;mission->Attach(world,game,view,renderer);
    Require(view && renderer && world->GetOwnScript(),"ordinary-effect fixture lacks its real world/Lua/view/renderer");
    const CVec3 top(0.17f,0.31f,0.53f),bottom(0.07f,0.13f,0.23f),position(12,34,5);
    view->SetAmbient(bottom,top);auto* scene=view->GetGScene();
    CStructureNetworkContext meanRefs;auto sceneFields=BorrowedObjectFields(scene,&meanRefs);CPtr<CFuncBase<CVec3>> mean,ambient;
    {CStructureSaver reader(sceneFields,CStructureSaver::READ,&meanRefs);reader.Add(18,&mean);reader.Add(33,&ambient);}
    Require(IsValid(mean) && IsValid(ambient),"ordinary-effect fixture lost the existing ambient graph");
    auto geometry=[&](){CStructureNetworkContext refs;auto saved=BorrowedObjectFields(scene,&refs);list<CPtr<CObjectBase>> lights,particles;
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(7,&lights);reader.Add(10,&particles);}
        vector<CPtr<CObjectBase>> liveLights,liveParticles;
        for(auto p:lights)if(IsValid(p))liveLights.push_back(p);
        for(auto p:particles)if(IsValid(p))liveParticles.push_back(p);
        return std::make_pair(liveLights,liveParticles);};
    const auto initial=geometry();Require(initial.first.empty() && initial.second.empty(),"ordinary-effect fixture started with unrelated effect geometry");
    auto lights=NativeAuthoredEffectLights();
    CDBIterator<NDb::CEffect> effects(*NDatabase::GetTable<NDb::CEffect>());
    Require(effects.MoveNext(),"ordinary-effect fixture has no registered effect");CDBPtr<NDb::CEffect> effect=effects.Get();
    effect->instances.clear();effect->lights.clear();effect->flags.clear();effect->lights.push_back(lights[0].instance.GetPtr());
    NDb::SVariantFlags allowed;allowed.flags=world->GetCreateFlags();effect->flags.push_back(allowed);
    if(mode==1){effect->flags[0].flags={NWorld::TOD_NIGHT};Require(!IsSuitableVariant(world->GetCreateFlags(),effect->flags),"night-only ordinary effect matched a day world");}
    if(mode==2) {
        effect->lights.push_back(lights[1].instance.GetPtr());
        CDBIterator<NDb::CParticleInstance> particles(*NDatabase::GetTable<NDb::CParticleInstance>());NDb::CParticleInstance* particle=nullptr;
        while(particles.MoveNext())if(IsValid(particles.Get()->pParticle)){particle=particles.Get();break;}
        Require(particle!=nullptr,"ordinary mixed effect lacks a native particle instance");effect->instances.push_back(particle);
    }
    CDBIterator<NDb::CTEffect> templates(*NDatabase::GetTable<NDb::CTEffect>());
    Require(templates.MoveNext(),"ordinary-effect fixture has no registered effect template");CDBPtr<NDb::CTEffect> pool=templates.Get();
    pool->variants.clear();pool->variants.push_back(effect.GetPtr());pool->roulette=CRoulette();pool->roulette.AddSector(1);
    auto* script=world->GetOwnScript();CObj<NScript::CLUAObjectPosition> point=new NScript::CLUAObjectPosition(position);
    NScript::luaPushCObj(script->GetState(),point);script->SetGlobal("NativeEffectPosition");
    auto* clock=renderer->GetTime();const STime begin=mode>=3?0:1375;clock->Set(begin);
    const string lua="NativeEffectHandle = PlayEffect("+std::to_string(mode==4?-1:pool->GetRecordID())+",NativeEffectPosition)";
    Require(script->DoString(lua.c_str())==0,"ordinary-effect fixture could not execute its actual Lua producer");
    world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);CDGPtr<CFuncBase<STime>> clockEdge=clock;clockEdge.Refresh();mission->Drain();
    const int stackTop=script->GetTop();auto value=script->GetGlobal("NativeEffectHandle");
    CObj<NWorld::CUICmdPlayEffect> command=value.IsUserData()?dynamic_cast<NWorld::CUICmdPlayEffect*>(value.GetUserData()):nullptr;
    const bool nil=value.IsNil();script->SetTop(stackTop);
    const auto shape=geometry();
    std::printf("native ordinary effect mode=%d rendererBegin=%u worldTime=%u command=%d handle=%d lights=%zu particles=%zu\n",mode,unsigned(begin),unsigned(world->GetAimTime()->GetValue()),int(IsValid(command)),int(command && IsValid(command->pHandle)),shape.first.size(),shape.second.size());std::fflush(stdout);
    if(mode==4) {Require(nil && !command && shape.first.empty() && shape.second.empty(),"invalid effect token changed its native nil/no-geometry behavior");return;}
    Require(command && !nil && command->pEffect.GetPtr()==pool.GetPtr() && fabs2(command->vPos-position)<1e-10f,"ordinary effect lost its native command/position/template identity");
    if(mode==1) {
        Require(!IsValid(command->pHandle) && shape.first.empty() && shape.second.empty(),"ordinary effect ignored world variant flags and rendered an unsuitable effect");
    } else {
        Require(IsValid(command->pHandle) && shape.first.size()==(mode==2?2u:1u) && shape.second.size()==(mode==2?1u:0u),"ordinary effect no longer creates its complete authored light/particle set");
        CPtr<NGScene::CLightAnimator> first;STime observedBegin=0;bool correctTimes=true;
        for(auto node:shape.first) {
            Require(dynamic_cast<NGScene::CDynamicPointLight*>(node.GetPtr())!=nullptr,"ordinary effect created an unexpected light kind");
            CStructureNetworkContext refs;auto saved=BorrowedObjectFields(node,&refs);CPtr<NGScene::CLightAnimator> animation;
            {CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(2,&animation);}
            Require(IsValid(animation),"ordinary point light has no real native light animator");
            CStructureNetworkContext animationRefs;auto animationFields=BorrowedObjectFields(animation,&animationRefs);
            CDBPtr<NDb::CLightInstance> instance;STime start=0;CDGPtr<CFuncBase<STime>> time;
            {CStructureSaver reader(animationFields,CStructureSaver::READ,&animationRefs);reader.Add(3,&start);reader.Add(4,&instance);reader.Add(5,&time);}
            Require(time.GetPtr()==clock,"ordinary light animation is not driven by the actual renderer clock");correctTimes=correctTimes && start==begin;
            if(instance.GetPtr()==lights[0].instance.GetPtr()){first=animation.GetPtr();observedBegin=start;}
        }
        Require(IsValid(first),"ordinary effect lost its authored first light instance");
        const float phase=lights[0].info->fTStart+(lights[0].info->fTEnd-lights[0].info->fTStart)*0.25f;
        clock->Set(begin+STime(phase*1000));CDGPtr<NGScene::CLightAnimator> actual=first.GetPtr();bool active=false,ended=false;CVec3 color=VNULL3;
        const auto deadline=SDL_GetTicks()+10000;
        do {MarkNewDGFrame();actual.Refresh();auto* state=actual->GetValue();active=state->bActive;ended=state->bEnd;if(active)color=state->color;if(active || ended)break;SDL_Delay(1);}while(SDL_GetTicks()<deadline);
        CVec3 expected;lights[0].info->color.GetValue((clock->GetValue()-begin)*lights[0].info->fFrameRate/1000.f,&expected);
        std::printf("native ordinary light mode=%d expectedBegin=%u actualBegin=%u active=%d ended=%d allTimes=%d\n",mode,unsigned(begin),unsigned(observedBegin),int(active),int(ended),int(correctTimes));std::fflush(stdout);
        Require(correctTimes && active && !ended && fabs2(color-expected)<1e-7f,"ordinary effect used elapsed renderer time instead of its current launch time");
        Require(script->DoString("StopEffect(NativeEffectHandle); NativeEffectStopped = 1")==0,"ordinary-effect fixture could not execute native StopEffect");
        world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);mission->Drain();
        const int stopTop=script->GetTop();auto stop=script->GetGlobal("NativeEffectStopped");const bool stoppedLua=stop.IsNumber() && stop.GetNumber()==1;script->SetTop(stopTop);
        Require(stoppedLua,"ordinary-effect fixture did not execute its queued Lua stop body");
        Require(!IsValid(command->pHandle),"native StopEffect retained the command's live render handle");
        const auto stopped=geometry();Require(stopped.first.empty() && stopped.second.empty(),"ordinary StopEffect left its light or particle nodes alive");
    }
    CDGPtr<CFuncBase<CVec3>> meanEdge=mean.GetPtr();MarkNewDGFrame();meanEdge.Refresh();
    Require(fabs2(meanEdge->GetValue()-(top+bottom)*0.5f)<1e-10f,"ordinary effect was incorrectly routed into scene-wide ambient animation");
    CStructureNetworkContext ambientRefs;auto ambientFields=BorrowedObjectFields(ambient,&ambientRefs);CPtr<CObjectBase> ambientAnimation;
    {CStructureSaver reader(ambientFields,CStructureSaver::READ,&ambientRefs);reader.Add(3,&ambientAnimation);}
    Require(!IsValid(ambientAnimation) && !NMainLoop::HaveInterfaceCommand(),"ordinary effect changed ambient ownership or opened a new interface");
}

static void CorpseTargetInformation(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* observer = nullptr;
    for (auto unit: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (server && server->GetPlayer() && !NAI::IsAIPlayer(server->GetPlayer()) && server->CanFight()) { observer = server; break; }
    }
    Require(observer,"corpse information fixture lacks its native human viewer");
    auto* player = observer->GetPlayer();
    NWorld::CUnitServer* foreign = nullptr;
    for (auto unit: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (server && server->GetPlayer() != player && server->CanFight() && !server->IsEmptyPK() && !server->IsWearingPK()) { foreign = server; break; }
    }
    Require(foreign,"corpse information fixture lacks its native foreign human target");
    NWorld::SEnemyInfo ownLive,foreignLive;
    player->GetEnemyUnitInfo(observer,&ownLive); player->GetEnemyUnitInfo(foreign,&foreignLive);
    Require(ownLive.bUnitInfo && ownLive.bCanSeeUnitHP && ownLive.bCanSeePKHP && !ownLive.wsName.empty() && ownLive.nMaxUnitHP > 0,
        "corpse information own-unit live control did not expose its normal name and health");
    Require(foreignLive.bUnitInfo && !foreignLive.wsName.empty() && foreignLive.nMaxUnitHP > 0 &&
        !foreignLive.bCanSeeUnitHP && !foreignLive.bCanSeePKHP,
        "corpse information foreign live control did not preserve the viewer's normal HP restrictions");
    auto checkInvalid = [&](CObjectBase* target) {
        NWorld::SEnemyInfo stale = ownLive;
        player->GetEnemyUnitInfo(target,&stale);
        Require(stale.wsName.empty() && !stale.bUnitInfo && !stale.bPKInfo && !stale.bCanSeeUnitHP &&
            !stale.bCanSeePKHP && stale.nUnitHP == 0 && stale.nMaxUnitHP == 0,
            "an absent or non-unit target retained information from a prior unit");
    };
    CObj<NWorld::CWindowDoor> object = new NWorld::CWindowDoor;
    checkInvalid(nullptr); checkInvalid(object);
    const int ap = observer->GetAP(), health = observer->GetUnitRPG()->GetTotalVP();
    const auto observerPosition = observer->GetPosition();
    auto makeCorpse = [](NWorld::CUnitServer* unit) {
        unit->GetUnitRPG()->Kill(); unit->SetState(new NWorld::CUnitStateDeath(unit));
        Require(unit->IsDead() && unit->GetUnitRPG()->IsDead() && !unit->CanFight(),
            "corpse information fixture failed to enter real server and RPG death states");
    };
    makeCorpse(foreign);
    NWorld::SEnemyInfo corpse;
    player->GetEnemyUnitInfo(foreign,&corpse);
    std::printf("native corpse information: foreign liveName=%zu corpseName=%zu unitInfo=%d maxHP=%d HPvisibility=%d/%d\n",
        foreignLive.wsName.size(),corpse.wsName.size(),corpse.bUnitInfo,corpse.nMaxUnitHP,corpse.bCanSeeUnitHP,corpse.bCanSeePKHP);
    Require(corpse.wsName == foreignLive.wsName && corpse.bUnitInfo && corpse.nMaxUnitHP == foreignLive.nMaxUnitHP,
        "the native viewer discarded a corpse's existing name and health information");
    Require(corpse.nUnitHP < 0 && corpse.eUnitCondition == NWorld::SEnemyInfo::CND_UNCONSCIOUS_DESTROYED &&
        !corpse.bCanSeeUnitHP && !corpse.bCanSeePKHP,
        "restoring corpse information changed its actual dead condition or revealed restricted HP");
    Require(observer->GetAP() == ap && observer->GetUnitRPG()->GetTotalVP() == health && observer->GetPosition() == observerPosition,
        "querying corpse information altered the viewer's AP, health or placement");
    makeCorpse(observer);
    NWorld::SEnemyInfo ownCorpse;
    player->GetEnemyUnitInfo(observer,&ownCorpse);
    Require(ownCorpse.wsName == ownLive.wsName && ownCorpse.bUnitInfo && ownCorpse.nMaxUnitHP == ownLive.nMaxUnitHP &&
        ownCorpse.bCanSeeUnitHP && ownCorpse.bCanSeePKHP && ownCorpse.nUnitHP < 0,
        "the native player's own corpse lost its existing full health visibility");
    checkInvalid(nullptr); checkInvalid(object);
}

static void MountedGunEligibility(const char* databasePath,const char* resourcePath)
{
    auto world = FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NWorld::CUnitServer* target = nullptr;
    for (auto unit: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (!server || !server->CanFight() || server->IsWearingPK() || !NAI::GetAIUnit(server)) continue;
        actor = server; break;
    }
    Require(actor,"mounted-gun fixture has no native AI gunner");
    for(auto candidate:units) {
        auto* server=dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if(server && server->CanFight() && !server->IsWearingPK() && !server->IsEmptyPK() && actor->GetDiplomacyState(server)==NDb::DS_ENEMY){target=server;break;}
    }
    Require(target,"mounted-gun fixture has no actual hostile target under authored diplomacy");
    auto* gunner = NAI::GetAIUnit(actor);
    CObj<NAI::IAIUnit> enemyOwner=NAI::GetAIUnit(target);
    if(!enemyOwner)enemyOwner=NAI::CreateAIUnit(target,false);
    enemyOwner->Synchronize(false);auto* enemy=enemyOwner.GetPtr();
    auto* threat = gunner->GetAIUnitState();
    Require(threat != nullptr,"mounted-gun fixture has no native threat state");
    threat->AddEnemy(enemy); threat->pEnemy = enemy; threat->pPossibleEnemy = nullptr;
    const auto& known=threat->GetKnownEnemies();
    Require(std::find(known.begin(),known.end(),CPtr<NAI::IAIUnit>(enemy))!=known.end(),"native threat tracker rejected the fixture target as non-hostile");
    CObj<NAI::CAIShootFromHGAction> shoot = new NAI::CAIShootFromHGAction(gunner);
    CObj<NAI::CAIDockWithHGAction> dock = new NAI::CAIDockWithHGAction(gunner);
    CObj<NAI::CAIUndockFromHGAction> undock = new NAI::CAIUndockFromHGAction(gunner);
    vector<NWorld::CCannon*> cannons; world->GetCannons(&cannons);
    NWorld::CCannon* selected = nullptr;
    auto* net = world->GetPathNetwork();
    int examined = 0, attackable = 0;
    for (auto* cannon: cannons) {
        if (!cannon || cannon->IsBroken() || cannon->IsOccupied() || !cannon->GetItem() || !cannon->GetItem()->HasAmmo()) continue;
        vector<NAI::SPathPlace> approaches; cannon->GetApproaches(&approaches,net);
        vector<NAI::SPathPlace> targets; net->GetNearPlaces(SSphere(cannon->GetPosition(),25.f),&targets);
        for (auto approach: approaches) {
            if (!net->IsPassable(approach) || net->IsLocked(approach,false)) continue;
            auto position = actor->GetPosition(); position.pos.p = approach; actor->SetPosition(position);
            world->GetTime()->Set(world->GetTime()->GetValue()+1);actor->animator.AlignTime();actor->animator.PlaceUnit(position);
            gunner->Synchronize();
            for (auto place: targets) {
                place.SetPose(NAI::CM_STAND);
                if (!net->IsPassable(place) || net->IsLocked(place,false)) continue;
                NAI::SPosition at(place,net);
                if (fabs(at.GetCP()-cannon->GetPosition()) <= 6.f ||
                    NWorld::CanAttackWithCannon(cannon,at.GetCP()+CVec3(0,0,1)) != NWorld::UCR_OK) continue;
                auto targetPosition = target->GetPosition(); targetPosition.pos.p = place; target->SetPosition(targetPosition);
                world->GetTime()->Set(world->GetTime()->GetValue()+1);target->animator.AlignTime();target->animator.PlaceUnit(targetPosition);
                enemy->Synchronize(); MarkNewDGFrame();world->GetAIMap()->Sync(); ++examined;
                CVec3 aim;auto* hull=world->GetAIMap()->GetHull(target);const bool hasAim=world->GetAIMap()->GetUnitHLPos(&aim,hull,NAI::HL_BODY);
                if(examined<=3) {
                    const auto cp=target->GetPosition().GetCP(),gp=cannon->GetPosition();
                    std::printf("mounted native aim: person=%d pose=%d hull=%d aimValid=%d CP=(%g,%g,%g) aim=(%g,%g,%g) gun=(%g,%g,%g) result=%d\n",
                        target->GetUnitRPG()->GetRPGPersID(),int(target->GetPosition().GetPose()),int(hull!=nullptr),int(hasAim),double(cp.x),double(cp.y),double(cp.z),double(aim.x),double(aim.y),double(aim.z),double(gp.x),double(gp.y),double(gp.z),int(NWorld::CanAttackWithCannon(cannon,aim)));std::fflush(stdout);
                }
                if (NWorld::CanAttackWithCannon(cannon,aim) != NWorld::UCR_OK) continue;
                ++attackable;
                gunner->SetCannon(cannon);
                NAI::CAIShootFromHGAction::SInfo firing;
                shoot->GetInfoInner(NAI::SPlaceWithAP(actor->GetPosition(),gunner->GetAP()),&firing);
                if (firing.bCanDo && firing.pGun == cannon && firing.pTarget == enemy) { selected = cannon; break; }
            }
            if (selected) break;
        }
        if (selected) break;
    }
    std::printf("native mounted-gun candidates: guns=%zu examined=%d attackable=%d selected=%d\n",cannons.size(),examined,attackable,selected != nullptr);
    Require(selected,"mounted-gun fixture has no reachable native gun with a damageable known enemy");
    const NAI::SPlaceWithAP place(actor->GetPosition(),gunner->GetAP());
    gunner->SetCannon(nullptr);
    NAI::CAIDockWithHGAction::SInfo freeDock; dock->GetInfoInner(place,&freeDock);
    Require(freeDock.bCanDo && freeDock.pGun == selected,"unoccupied mounted-gun docking control failed");
    gunner->SetCannon(selected);
    NAI::CAIUndockFromHGAction::SInfo freeUndock; undock->GetInfoInner(place,&freeUndock);
    Require(!freeUndock.bCanDo,"working native gun unexpectedly required abandonment before occupancy");
    const int ap = actor->GetAP(), ammo = selected->GetItem()->GetAmmoQuantity();
    const auto actorPosition = actor->GetPosition();
    selected->SetCurrentUnit(actor);
    Require(selected->IsOccupied() && selected->GetCurrentUnit() == actor,"mounted-gun fixture did not establish native occupancy");
    NAI::CAIShootFromHGAction::SInfo mounted; shoot->GetInfoInner(place,&mounted);
    std::printf("native occupied gun: fire=%d target=%d ammo=%d\n",mounted.bCanDo,mounted.pTarget == enemy,ammo);
    Require(mounted.bCanDo && mounted.pGun == selected && mounted.pTarget == enemy,
        "a live occupied cannon prevented its own gunner from selecting a shot");
    NAI::CAIUndockFromHGAction::SInfo stay; undock->GetInfoInner(place,&stay);
    Require(!stay.bCanDo,"a working occupied cannon forced its gunner to abandon it");
    gunner->SetCannon(nullptr);
    NAI::CAIDockWithHGAction::SInfo occupiedDock; dock->GetInfoInner(place,&occupiedDock);
    Require(!occupiedDock.bCanDo || occupiedDock.pGun != selected,"another gunner could select an occupied cannon for docking");
    gunner->SetCannon(selected);
    Require(actor->GetAP() == ap && selected->GetItem()->GetAmmoQuantity() == ammo && actor->GetPosition() == actorPosition,
        "mounted-gun eligibility queries spent AP, ammunition or moved the gunner");
    auto* weapon = dynamic_cast<NRPG::CWeaponItem*>(selected->GetItem());
    Require(weapon != nullptr,"native cannon has no weapon item for the empty-ammunition control");
    weapon->DiscardAmmo();
    NAI::CAIShootFromHGAction::SInfo empty; shoot->GetInfoInner(place,&empty);
    NAI::CAIUndockFromHGAction::SInfo leave; undock->GetInfoInner(place,&leave);
    Require(!empty.bCanDo && leave.bCanDo,"an empty occupied cannon remained eligible to fire or failed to release its gunner");
    gunner->SetCannon(nullptr); selected->SetCurrentUnit(nullptr);
}

static void UnitDiplomacyIdentity(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> before; world->GetAllUnits(&before);
    NWorld::CUnitServer* first = nullptr;
    NWorld::CUnitServer* npc = nullptr;
    for (auto unit: before) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (!server || !server->GetPlayer()) continue;
        if (!NAI::IsAIPlayer(server->GetPlayer())) first = server;
        else if (server->GetPlayer()->GetScenarioPlayerID() > 0) npc = server;
    }
    Require(first && npc,"unit diplomacy fixture needs a deployed human and an authored NPC side");
    auto* firstPlayer = first->GetPlayer();
    const auto originalDiplomacy = first->GetUnitRPG()->GetDiplomacy();
    const int npcID = npc->GetPlayer()->GetScenarioPlayerID();
    // Per-unit overrides must survive routing through the world. Player-to-player
    // diplomacy is insufficient: it does not consult this unit's individual mask.
    for (auto state: {NDb::DS_ENEMY,NDb::DS_NEUTRAL,NDb::DS_ALLY}) {
        auto individual = originalDiplomacy;
        individual.SetDiplomacyState(npcID,state);
        first->GetUnitRPG()->SetDiplomacy(individual);
        Require(world->GetDiplomacyState(static_cast<NWorld::CUnit*>(first),npc->GetPlayer()) == state &&
            first->GetDiplomacyState(npc) == state,"unit diplomacy lost an individual authored-side override");
    }
    first->GetUnitRPG()->SetDiplomacy(originalDiplomacy);
    Require(first->GetDiplomacyState(first) == NDb::DS_ALLY,"unit diplomacy self-alliance control failed");
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NAI::CSequenceCommander> commander = new NAI::CSequenceCommander(world);
    auto* secondPlayer = dynamic_cast<NWorld::CPlayer*>(world->AddPlayer(L"Second diplomacy player",game->players.back(),commander));
    Require(secondPlayer && secondPlayer != firstPlayer,"unit diplomacy did not create a distinct native player");
    commander->SetPlayer(secondPlayer);
    vector<CPtr<NWorld::CUnit>> deployed; world->GetAllUnits(&deployed);
    NWorld::CUnitServer* second = nullptr;
    for (auto unit: deployed) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (server && server->GetPlayer() == secondPlayer) { second = server; break; }
    }
    Require(second && firstPlayer->GetScenarioPlayerID() == 0 && secondPlayer->GetScenarioPlayerID() == 0,
        "unit diplomacy did not deploy two real hotseat parties sharing scenario ID zero");
    Require(originalDiplomacy.GetDiplomacyState(0) == NDb::DS_ALLY &&
        world->GetDiplomacyState(static_cast<NWorld::CUnit*>(first),secondPlayer) == NDb::DS_ENEMY,
        "unit diplomacy control did not distinguish player identity from the scenario mask");
    std::printf("native diplomacy controls: individual enemy/neutral/ally, self ally; distinct players share scenario 0\n");
    const auto firstPosition = first->GetPosition(), secondPosition = second->GetPosition();
    const int firstAP = first->GetAP(), secondAP = second->GetAP();
    const int firstVP = first->GetUnitRPG()->GetTotalVP(), secondVP = second->GetUnitRPG()->GetTotalVP();
    std::printf("native diplomacy opposing parties: first-to-second=%d, world=%d\n",
        int(first->GetDiplomacyState(second)),int(NDb::DS_ENEMY));
    Require(first->GetDiplomacyState(second) == NDb::DS_ENEMY &&
        second->GetDiplomacyState(first) == NDb::DS_ENEMY,
        "distinct hotseat players were treated as allies because their scenario IDs matched");
    Require(second->GetDiplomacyState(second) == NDb::DS_ALLY &&
        first->GetDiplomacyState(nullptr) == NDb::DS_NEUTRAL,
        "unit diplomacy lost self alliance or failed to handle an absent target");
    Require(first->GetAP() == firstAP && second->GetAP() == secondAP &&
        first->GetUnitRPG()->GetTotalVP() == firstVP && second->GetUnitRPG()->GetTotalVP() == secondVP &&
        first->GetPosition() == firstPosition && second->GetPosition() == secondPosition &&
        first->GetPlayer() == firstPlayer && second->GetPlayer() == secondPlayer &&
        first->GetUnitRPG()->GetDiplomacy().GetDiplomacy() == originalDiplomacy.GetDiplomacy(),
        "a unit diplomacy query changed player identity, placement, AP, health or individual diplomacy");
}

class ScriptSoundDispatchMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(ScriptSoundDispatchMission);
    CObj<NUI::CDesktopWindow> desktop;
public:
    void Attach(NWorld::CWorld* world,NRPG::CGlobalGame* game,NSound::ISoundScene* scene) {
        pWorld = world; pGlobalGame = game; pSoundScene = scene; desktop = new NUI::CDesktopWindow;
    }
    NUI::CDesktopWindow* GetDesktop() const override { return desktop; }
    void Drain() { ExecWorldCommands(); }
};

static void ScriptSoundStop(const char* databasePath,const char* resourcePath,bool spatial,bool unassigned = false,STime interfaceTime = 0)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr,"script sound fixture requires headless native audio");
    Require(NFMSound::Init(NFMSound::SStartInfo()),"script sound fixture could not initialize native audio");
    struct AudioCleanup { ~AudioCleanup() { ClearHoldQueue(); NFMSound::Done(); } } audioCleanup;
    NFMSound::SetMusicMasterVolume(0);
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    while (auto* pending = world->GetUICommand()) { CObj<NWorld::CUICmd> release = pending; }
    CObj<NSound::ISoundScene> scene = NSound::CreateSoundScene(nullptr,nullptr,world->GetAimTime());
    CObj<ScriptSoundDispatchMission> mission = new ScriptSoundDispatchMission;
    mission->Attach(world,game,scene);
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(interfaceTime)};
    CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
    Require(sounds.MoveNext(),"script sound fixture has no native sound record");
    auto* record = sounds.Get(); record->bLoop = true; record->nStartSamples = record->nEndingSamples = 0;
    const auto wave = SilentPCMFixture(44100);
    CObj<NFMSound::CSample2D> sample2D = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
    CObj<NFMSound::CSample3D> sample3D = NFMSound::LoadSample3D(wave.data(),static_cast<int>(wave.size()),1.f,100.f,0);
    Require(IsValid(sample2D) && IsValid(sample3D),"script sound fixture could not decode its native PCM samples");
    CObj<NSound::CFileSample2D> loader2D = NSound::share2DSamples.Get(record->GetRecordID());
    CObj<NSound::CFileSample3D> loader3D = NSound::share3DSamples.Get(record->GetRecordID());
    (loader2D.GetPtr()->*CachedChapterSampleAccess::ValueMember()) = sample2D.GetPtr();
    (loader3D.GetPtr()->*CachedEffectSampleAccess::ValueMember()) = sample3D.GetPtr();
    auto* script = world->GetOwnScript();
    Require(IsValid(script),"script sound fixture has no actual world Lua state");
    CObj<NScript::CLUAObjectPosition> position = new NScript::CLUAObjectPosition(CVec3(0,0,0));
    NScript::luaPushCObj(script->GetState(),position); script->SetGlobal("SoundPosition");
    // A second live userdata exercises commands outside the first root's adjacent GC slot.
    CObj<NScript::CLUAObjectPosition> guard = new NScript::CLUAObjectPosition(CVec3(4,5,6));
    NScript::luaPushCObj(script->GetState(),guard); script->SetGlobal("SoundPositionGuard");
    CObj<ICamera> camera = ::CreateCamera(CAMERA_PC);
    CTransformStack transform; camera->GetTransform(&transform,CVec2(1024,768));
    vector<float> output(512*2);
    auto render = [&]() { Require(S2FmodRenderForTest(output.data(),512) != 0,"script sound fixture could not render native PCM"); };
    auto run = [&](const string& code) {
        Require(script->DoString(code.c_str()) == 0,"script sound fixture could not queue its native Lua command");
        world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        mission->Drain(); scene->Draw(&transform);
    };
    const string play = spatial ? "Play3DSound("+std::to_string(record->GetRecordID())+",SoundPosition)" :
        "PlaySound("+std::to_string(record->GetRecordID())+")";
    auto channelExcept = [&](int except) {
        int found = -1;
        for (int i = 0; i < 128; ++i) if (i != except && FSOUND_IsPlaying(i)) {
            Require(found == -1,"script sound fixture has an unexpected extra channel"); found = i;
        }
        return found;
    };
    auto collect = [&]() {
        Require(script->GetState()->nGCAvoid == 0,"full GC fixture is running inside a collection-avoid scope");
        CObj<NScript::CLUAObjectPosition> orphan = new NScript::CLUAObjectPosition(CVec3(7,8,9));
        CPtr<NScript::CLUAObjectPosition> witness = orphan.GetPtr();
        NScript::luaPushCObj(script->GetState(),orphan); script->Pop(); orphan = nullptr;
        // Drive the VM's full collector, including marking of live globals.
        // luaC_collect alone is the sweep phase and is not a complete GC cycle.
        for (int check = 0; check < 2001; ++check) luaC_checkGC(script->GetState());
        Require(!IsValid(witness),"the GC fixture never collected its unreachable native userdata");
        const int top = script->GetTop(); auto live = script->GetGlobal("SoundPosition");
        Require(live.IsUserData() && live.GetUserData() == position.GetPtr(),"a full Lua collection destroyed a live fixture global");
        script->SetTop(top);
    };
    if (unassigned) {
        Require(script->DoString((play+"; Pause()").c_str()) == 0,"unassigned sound fixture could not queue its Lua producer");
        world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        collect(); // A full collection may occur before the interface consumes the queue.
        mission->Drain(); scene->Draw(&transform);
        const int pending = channelExcept(-1);
        Require(pending >= 0,"an unassigned Lua PlaySound result was collected before its native UI command could start playback");
        Require(mission->IsGamePaused(),"a collected unassigned sound blocked the following native UI command");
        render();
        Require(FSOUND_GetCurrentPosition(pending) > 0,"an unassigned scripted sound did not advance native PCM playback");
        collect(); scene->Draw(&transform); render();
        Require(FSOUND_IsPlaying(pending),"a dispatched unassigned sound did not survive a subsequent full Lua collection");
        mission = nullptr; render();
        Require(!FSOUND_IsPlaying(pending),"mission teardown leaked its unassigned script sound");
        return;
    }
    run("PlayHandle = "+play);
    const int played = channelExcept(-1);
    Require(played >= 0,"native Lua PlaySound dispatch did not start a real PCM channel");
    if (interfaceTime != 0) Require(FSOUND_GetCurrentPosition(played) == 0,
        "a fresh scripted sound inherited absolute interface time as a sample start offset");
    render();
    Require(FSOUND_GetCurrentPosition(played) > 0,"native scripted sound never advanced before StopSound");
    CObj<NFMSound::CSound2D> independent = NFMSound::PlaySound(sample2D);
    const int control = channelExcept(played);
    Require(IsValid(independent) && control >= 0,"script sound fixture did not start its unrelated control");
    run("StopSound(PlayHandle)");
    render();
    Require(!FSOUND_IsPlaying(played),"native Lua StopSound cleared its weak handle but left the actual sound playing");
    Require(FSOUND_IsPlaying(control) && FSOUND_GetCurrentPosition(control) > 0,
        "stopping a scripted sound stopped an unrelated native channel");
    const unsigned beforeControl = FSOUND_GetCurrentPosition(control);
    run("StopSound(PlayHandle)"); render();
    Require(channelExcept(control) == -1 && FSOUND_GetCurrentPosition(control) > beforeControl,
        "repeated StopSound replayed the sample or interrupted unrelated playback");
    run("PlayHandle = "+play);
    const int retained = channelExcept(control);
    Require(retained >= 0,"a new scripted sound failed after stopping the preceding channel");
    run("PlayHandle = nil");
    collect();
    render();
    Require(FSOUND_IsPlaying(retained) && FSOUND_GetCurrentPosition(retained) > 0,
        "collecting a dispatched PlaySound userdata stopped its scene-owned playback");
    mission = nullptr;
    render();
    Require(!FSOUND_IsPlaying(retained) && FSOUND_IsPlaying(control),
        "releasing the mission left its scripted playback alive or stopped unrelated audio");
#else
    Require(false,"script sound test requires the native SFX backend");
#endif
}

class ZoneSnapshotMissionFixture: public NGame::CMission
{
public:
    static auto SaveWorldMethod() { return &ZoneSnapshotMissionFixture::SaveWorld; }
};

static void ZoneSnapshotExcludesHUD(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr, "zone snapshot fixture needs isolated user data");
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    Require(IsValid(world->GetOwnScript()), "zone snapshot fixture has no native world script");
    NMainLoop::CreateProfile("UpstreamZoneSnapshot");
    NMainLoop::SetActiveProfile("UpstreamZoneSnapshot");
    auto* manager = NMainLoop::GetSaveManager();
    struct Cleanup {
        NMainLoop::CSaveManager* manager;
        ~Cleanup() { manager->DeleteSlot(NMainLoop::S_SLOT_ACTIVE); }
    } cleanup{manager};
    CObj<NGame::CMission> mission = new NGame::CMission;
    CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false);
    CObj<NUI::CInterface> interface = new NUI::CInterface(cursor);
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.include = [](CObjectBase*) { return false; };
    const vector<CObjectBase*> objects{game.GetPtr(),world.GetPtr(),cursor.GetPtr(),interface.GetPtr()};
    for (std::size_t i = 0; i < objects.size(); ++i) {
        references.ids[objects[i]] = unsigned(i+1); references.refs[unsigned(i+1)] = objects[i];
    }
    references.nextID = 100;
    CMemoryStream base;
    { CStructureSaver writer(base,CStructureSaver::WRITE,&references);
      writer.Add(2,&game); writer.Add(10,&world); writer.Add(15,&cursor); writer.Add(16,&interface); }
    base.Seek(0);
    { CStructureSaver reader(base,CStructureSaver::READ,&references); mission->NGame::CMissionBase::operator&(reader); }
    CMObj<NUI::CMissionUI> hud = new NUI::CMissionUI(
        NUI::SWindowInfo(interface,NUI::SPoint(0,0),NUI::SPoint(1024,768),"snapshot-hud",NUI::STYLE_ENABLED),mission);
    world->GetOwnScript()->SetScriptInterface(interface);
    Require(world->GetOwnScript()->GetScriptInterface() == interface.GetPtr() &&
        interface->GetChildByID("snapshot-hud") == hud.GetPtr(),
        "zone snapshot fixture did not connect its real world script, HUD and mission");
    {
        // Normal mission saves must keep the UI bridge and the HUD's mission link.
        CObj<NGame::CMission> savedMission = mission.GetPtr(), restoredMission;
        CMemoryStream full;
        { CStructureSaver writer(full,CStructureSaver::WRITE_COMPRESSED); writer.Add(2,&savedMission); }
        full.Seek(0);
        CStructureSaver reader(full,CStructureSaver::READ);
        reader.Add(2,&restoredMission);
        Require(IsValid(restoredMission) && IsValid(restoredMission->GetInterface()),
            "normal native mission save did not restore its registered mission root and interface");
        auto* restoredWorld = dynamic_cast<NWorld::CWorld*>(restoredMission->GetWorld());
        Require(IsValid(restoredWorld) && IsValid(restoredWorld->GetOwnScript()) &&
            restoredWorld->GetOwnScript()->GetScriptInterface() == restoredMission->GetInterface() &&
            IsValid(restoredMission->GetInterface()->GetChildByID("snapshot-hud")),
            "normal native mission save did not retain its script-to-HUD bridge");
    }
    const auto saveWorld = ZoneSnapshotMissionFixture::SaveWorldMethod();
    (mission.GetPtr()->*saveWorld)("upstream-zone-ui.sav");
    Require(world->GetOwnScript()->GetScriptInterface() == interface.GetPtr(),
        "writing a zone snapshot did not restore the live world's HUD bridge");
    CFileStream snapshot;
    snapshot.OpenRead(manager->GetSlotFilePathW(NMainLoop::S_SLOT_ACTIVE,"upstream-zone-ui.sav").c_str());
    CObj<NWorld::IWorld> restored;
    CStructureSaver reader(snapshot,CStructureSaver::READ);
    reader.Add(2,&restored);
    auto* restoredWorld = dynamic_cast<NWorld::CWorld*>(restored.GetPtr());
    Require(IsValid(restoredWorld) && IsValid(restoredWorld->GetOwnScript()),
        "native zone snapshot did not restore its world and script");
    Require(restoredWorld->GetOwnScript()->GetScriptInterface() == nullptr,
        "a native zone snapshot retained the HUD and mission graph through its world-script interface bridge");
}

class PassageGateProbe: public NWorld::CObjectServerBase, public NWorld::IPassageObject
{
    OBJECT_NOCOPY_METHODS(PassageGateProbe);
public:
    bool nearby = false;
    int proximityChecks = 0;
    bool IsBroken() const override { return false; }
    bool CanPass(NWorld::CUnitServer*) override { ++proximityChecks; return nearby; }
    bool UsePassageObject(NWorld::CUnitServer*) override { return false; }
    void GetObjectApproaches(vector<NAI::SPathPlace>*) override {}
    int GetAPRadius() const override { return 4; }
    int GetPassageZoneID() const override { return 0; }
    int GetPassageObjectID() const override { return 123; }
};

// Only the per-unit reach provider is controlled: the actual world party,
// native command preview and real transition gate remain production code.
class PartyPassageProbe: public NWorld::CObjectServerBase, public NWorld::IPassageObject
{
    OBJECT_NOCOPY_METHODS(PartyPassageProbe);
public:
    std::set<NWorld::CUnitServer*> eligible;
    int checks=0,id=1001;
    bool IsBroken() const override { return false; }
    bool CanPass(NWorld::CUnitServer* unit) override { ++checks;return eligible.count(unit)!=0; }
    bool UsePassageObject(NWorld::CUnitServer*) override { return false; }
    void GetObjectApproaches(vector<NAI::SPathPlace>*) override {}
    int GetAPRadius() const override { return 4; }
    int GetPassageZoneID() const override { return 0; }
    int GetPassageObjectID() const override { return id; }
};

static void PassagePreview(const char* databasePath,const char* resourcePath,int mode)
{
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    auto match=S2Net::CreateNetworkMatch(S2Net::NetworkMatchPreset());auto* world=match.world.GetPtr();const int side=mode==8?1:0;
    auto* player=match.players[side].GetPtr();vector<NWorld::CUnitServer*> party;for(auto unit:player->GetPlayerUnits())party.push_back(unit.GetPtr());Require(party.size()==6 && player->GetScenarioPlayerID()==side,"passage preview lacks its actual native six-unit owning party");
    auto* actor=party[0];auto* partner=party[1];Require(actor->CanFight() && partner->CanFight(),"passage preview has no real conscious actor and companion");
    CObj<NDb::CDBScenarioZone> zoneRecord=new NDb::CDBScenarioZone;CObj<NScenario::CScenarioZone> zone=new NScenario::CScenarioZone(zoneRecord,1);match.game->pCurrentZone=mode==9?nullptr:zone.GetPtr();
    CObj<PartyPassageProbe> clicked=new PartyPassageProbe,alternate=new PartyPassageProbe;alternate->id=1002;
    for(auto* unit:party)clicked->eligible.insert(unit);
    if(mode==0 || mode==5 || mode==6 || mode==7)clicked->eligible.erase(partner);
    if(mode==2)clicked->eligible.erase(actor);
    if(mode==3){clicked->eligible.erase(partner);alternate->eligible.insert(partner);}
    if(mode==4){clicked->eligible.clear();for(auto* unit:party)alternate->eligible.insert(unit);}
    if(mode==5){partner->MakeUnconscious(VNULL3,true,false);Require(partner->IsUnconscious() && !partner->GetUnitRPG()->IsDead(),"passage preview could not incapacitate its actual companion");}
    if(mode==6){partner->GetUnitRPG()->Kill();partner->SetState(new NWorld::CUnitStateDeath(partner));Require(partner->IsDead() && partner->GetUnitRPG()->IsDead(),"passage preview could not enter its native companion death state");}
    auto objects=world->GetObjectsForHarness();if(mode!=10)objects.push_back(clicked.GetPtr());if(mode==3 || mode==4)objects.push_back(alternate.GetPtr());
    CStructureNetworkContext refs;auto native=BorrowedObjectFields(world,&refs);for(auto* object:{clicked.GetPtr(),alternate.GetPtr()}){const int id=refs.nextID++;refs.ids[object]=id;refs.refs[id]=object;}
    CMemoryStream replacement;{CStructureSaver writer(replacement,CStructureSaver::WRITE,&refs);writer.Add(20,&objects);}replacement.Seek(0);{CStructureSaver reader(replacement,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(world)->operator&(reader);}
    Require(std::count(world->GetObjectsForHarness().begin(),world->GetObjectsForHarness().end(),clicked.GetPtr())==int(mode!=10),"passage preview did not arrange its actual world object list");
    auto drain=[&](){int count=0;vector<CObj<CObjectBase>> events;world->DrainNetworkEvents(&events);for(auto event:events)if(dynamic_cast<NWorld::CUICmdContinueChapter*>(event.GetPtr()))++count;return count;};drain();
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);vector<std::pair<int,int>> stats;vector<NAI::SUnitPosition> positions;for(auto unit:units){stats.push_back({unit->GetAP(),unit->GetUnitRPG()->GetTotalVP()});positions.push_back(unit->GetPosition());}
    const bool action=world->IsAction();auto random=GlobalGameRandom();const bool deploy=player->GetGlobalPlayer()->deployData.bPassage;const int deployZone=player->GetGlobalPlayer()->deployData.nPassageZoneID;const auto deployCount=player->GetGlobalPlayer()->deployData.unitsDeployData.size();
    CObj<NWorld::CCmdUsePassage> command=new NWorld::CCmdUsePassage(mode==11?nullptr:clicked.GetPtr());CObj<NWorld::CExecUsePassage> executor=new NWorld::CExecUsePassage(actor,command);
    const auto result=executor->CanDoIt(actor->GetPosition(),false),ignored=executor->CanDoIt(actor->GetPosition(),true);const int previewTransitions=drain();
    auto afterRandom=GlobalGameRandom();bool unchanged=action==world->IsAction() && random.Get()==afterRandom.Get() && deploy==player->GetGlobalPlayer()->deployData.bPassage && deployZone==player->GetGlobalPlayer()->deployData.nPassageZoneID && deployCount==player->GetGlobalPlayer()->deployData.unitsDeployData.size();int index=0;for(auto unit:units){unchanged&=stats[index].first==unit->GetAP() && stats[index].second==unit->GetUnitRPG()->GetTotalVP() && positions[index]==unit->GetPosition();++index;}
    const bool expected=mode==1 || mode==3 || mode==4 || mode==5 || mode==6 || mode==8;const auto expectedResult=mode==11?NWorld::UCR_GENERAL_FAILURE:expected?NWorld::UCR_OK:NWorld::UCR_NOT_ALL_UNITS_NEAR_PASSAGE;
    int actual=-1,transitions=0,forced=-1;if(mode!=11){actual=int(world->UsePassageObject(actor,0));transitions=drain();Require(actual==int(expected) && transitions==int(expected),"actual native passage transition did not enforce the existing owning-party rule");}
    if(mode==7){const int checks=clicked->checks+alternate->checks;forced=int(world->UsePassageObject(actor,0,true));Require(forced==1 && drain()==1 && checks==clicked->checks+alternate->checks,"scripted forced passage lost its existing no-proximity bypass");}
    std::printf("native passage preview: mode=%d owner=%d party=%u units=%u result=%d ignored=%d expected=%d actual=%d transitions=%d forced=%d previewTransitions=%d unchanged=%d checks=%d\n",mode,side,unsigned(party.size()),unsigned(stats.size()),int(result),int(ignored),int(expectedResult),actual,transitions,forced,previewTransitions,int(unchanged),clicked->checks+alternate->checks);std::fflush(stdout);
    Require(unchanged && previewTransitions==0,"passage preview changed actual unit AP/HP/position, deployment, actions, RNG or queued a transition");
    Require(result==expectedResult && ignored==expectedResult,"passage command preview disagreed with the real owning-party transition gate");
}

static void ScriptedPassageTransition(const char* databasePath,const char* resourcePath, bool emptyPassage = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    CObj<NDb::CDBScenarioZone> zoneRecord = new NDb::CDBScenarioZone;
    CObj<NScenario::CScenarioZone> zone = new NScenario::CScenarioZone(zoneRecord,1);
    game->pCurrentZone = zone;
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (const auto& unit: units)
        if (IsValid(unit) && unit->CanFight() && !unit->IsAIUnit()) { actor = unit; break; }
    Require(actor != nullptr, "scripted passage fixture has no living human party");
    CObj<PassageGateProbe> passage = new PassageGateProbe;
    list<CObj<NWorld::CObjectServerBase>> objects = world->GetObjectsForHarness();
    if (emptyPassage) {
        objects.remove_if([](const CObj<NWorld::CObjectServerBase>& object) {
            auto* exit = dynamic_cast<NWorld::IPassageObject*>(object.GetPtr());
            return exit && exit->GetPassageZoneID() == 0;
        });
    } else objects.push_back(passage.GetPtr());
    CStructureNetworkContext references;
    auto nativeFields = BorrowedObjectFields(world,&references);
    auto id = references.nextID++;
    references.ids[passage.GetPtr()] = id; references.refs[id] = passage.GetPtr();
    CMemoryStream fields;
    { CStructureSaver writer(fields,CStructureSaver::WRITE,&references); writer.Add(20,&objects); }
    fields.Seek(0);
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); static_cast<CObjectBase*>(world.GetPtr())->operator&(reader); }
    const auto& restoredObjects = world->GetObjectsForHarness();
    Require(std::count(restoredObjects.begin(),restoredObjects.end(),passage.GetPtr()) == int(!emptyPassage),
        "scripted passage fixture did not register its passage in the native world");
    auto drainTransitions = [&]() {
        int count = 0;
        while (auto* raw = world->GetUICommand()) {
            CObj<NWorld::CUICmd> command = raw;
            if (dynamic_cast<NWorld::CUICmdContinueChapter*>(raw)) ++count;
        }
        return count;
    };
    drainTransitions();
    if (emptyPassage) {
        actor->MakeUnconscious(VNULL3,true,false);
        Require(actor->IsUnconscious() && !actor->IsDead(), "empty-passage fixture failed to incapacitate its human actor");
        for (const auto& unit: units)
            Require(unit->GetPlayer() != actor->GetPlayer() || unit->IsUnconscious() || unit->GetUnitRPG()->IsDead(),
                "empty-passage fixture still has a conscious party member");
        const int ap = actor->GetAP();
        Require(!world->UsePassageObject(actor,0) && drainTransitions() == 0,
            "an entirely unconscious party left through a nonexistent passage");
        CObj<NScript::CScript> script = NScript::CreateScript(world);
        Require(script->DoString("LeaveToSubZone(0)") == 0, "empty-passage script fixture failed to compile");
        script->ExecuteThreads();
        Require(drainTransitions() == 0, "scripted exit fabricated a missing passage for an unconscious party");
        Require(actor->GetAP() == ap, "rejecting an empty passage spent action points");
        return;
    }
    const int ap = actor->GetAP();
    Require(!world->UsePassageObject(actor,0) && passage->proximityChecks > 0 && drainTransitions() == 0,
        "ordinary passage control accepted a party outside the passage radius");
    const int checks = passage->proximityChecks;
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    Require(script->DoString("LeaveToSubZone(0)") == 0, "scripted passage fixture failed to compile");
    script->ExecuteThreads();
    Require(drainTransitions() == 1,
        "scripted LeaveToSubZone failed to leave after a cutscene while the party was away from the passage");
    Require(passage->proximityChecks == checks,
        "scripted forced passage still performed an ordinary proximity check");
    Require(!world->UsePassageObject(actor,0) && drainTransitions() == 0,
        "scripted forced passage weakened later ordinary passage eligibility");
    passage->nearby = true;
    Require(world->UsePassageObject(actor,0) && drainTransitions() == 1,
        "ordinary passage control rejected a party within the passage radius");
    Require(!world->UsePassageObject(actor,999999) && drainTransitions() == 0,
        "missing passage produced a transition command");
    Require(actor->GetAP() == ap, "scripted or previewed passage transition spent action points");
}

static void FlyingAttackQueue(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (const auto& unit: units) {
        if (!IsValid(unit) || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK()) continue;
        auto* weapon = unit->GetUnitRPG()->GetWeaponItem();
        if (IsValid(weapon) && weapon->GetAmmoQuantity() > 0 && weapon->SetShootMode(NDb::SM_Snap) &&
            NWorld::GetActionType(unit) == NWorld::AT_SHOOT && unit->GetUnitRPG()->CanMove()) {
            actor = unit; break;
        }
    }
    Require(actor != nullptr, "airborne attack fixture has no mobile armed actor");
    const NAI::SUnitPosition original = actor->GetPosition();
    struct PositionRestore {
        NWorld::CUnitServer* unit;
        NAI::SUnitPosition position;
        ~PositionRestore() { unit->SetTemporaryPosition(position); }
    } restore{actor,original};
    NAI::SUnitPosition airborne = original;
    Require(NAI::MakeFlyPos(original.GetCP()+CVec3(0,0,30),world->GetPathNetwork(),&airborne.pos),
        "airborne attack fixture could not encode its flight position");
    airborne.pos.p.SetPose(NAI::CM_STAND);
    actor->SetTemporaryPosition(airborne);
    auto* weapon = actor->GetUnitRPG()->GetWeaponItem();
    const int ap = actor->GetAP(), ammo = weapon->GetAmmoQuantity();
    const CVec3 target = airborne.GetCP()+CVec3(10,0,0);
    CObj<NWorld::CExecShootTile> shot = new NWorld::CExecShootTile(actor,target);
    Require(shot->CanDoIt(airborne) == NWorld::UCR_OK,
        "airborne attack fixture cannot shoot from its unmodified flight position");
    CObj<NWorld::CCmdShootTile> command = new NWorld::CCmdShootTile(target);
    NWorld::EUnitCommandResult error = NWorld::UCR_GENERAL_FAILURE;
    CObj<NWorld::CCommandExecute> executor = NWorld::CreateActionExecutor(actor,command,&error);
    std::printf("Airborne attack queue: final=%d altitude=%d result=%d executor=%d\n",
        int(airborne.pos.p.IsFinal()),airborne.pos.p.GetLayer(),int(error),int(IsValid(executor)));
    Require(IsValid(executor) && error == NWorld::UCR_OK,
        "airborne attack pose candidates lost the flight bit and prevented a valid shot queue");
    auto* mover = dynamic_cast<NWorld::IExecMove*>(executor.GetPtr());
    Require(mover != nullptr, "airborne shot queue has no movement plan");
    NAI::SPathPlace desired;
    NAI::EFindPathParams parameters;
    NWorld::ENeedActiveItem active;
    mover->GetDesiredPlace(&desired,&parameters,&active);
    Require(desired.IsFinal() && desired.GetX() == airborne.pos.p.GetX() &&
        desired.GetY() == airborne.pos.p.GetY() && desired.GetLayer() == airborne.pos.p.GetLayer() &&
        desired.GetPose() == NAI::CM_STAND && desired.GetDirection() ==
            NRPG::GetShootDirection(world->GetPathNetwork(),airborne.pos.p,target),
        "airborne shot queue changed its altitude, place, pose, or target facing");
    Require(actor->GetAP() == ap && weapon->GetAmmoQuantity() == ammo && !actor->HasCommand(),
        "preparing an airborne shot queue spent AP/ammunition or installed a command");
}

static void OwnedAICommanderRosters(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CPlayer>> players;
    world->GetPlayersList(&players);
    list<CPtr<NWorld::CUnitServer>> allUnits;
    world->GetAllUnits(&allUnits);
    Require(players.size() > 1 && !allUnits.empty(), "AI roster fixture has no opposing parties");
    int checked = 0, hostileViews = 0;
    for (const auto& player: players) {
        auto* commander = dynamic_cast<NAI::CAICommander*>(player->GetCommander());
        if (!commander) continue;
        ++checked;
        auto verifyOwnership = [&]() {
            for (const auto& unit: commander->GetUnitsList())
                Require(IsValid(unit) && IsValid(unit->GetUnitServer()) && unit->GetUnitServer()->GetPlayer() == player.GetPtr(),
                    "AI commander retained a foreign unit in its strongly owned roster");
        };
        verifyOwnership();
        auto* state = commander->GetAIState();
        state->Synchronize();
        const auto& allies = *state->GetAllyAIPlayer()->GetUnits();
        const auto& enemies = *state->GetEnemyAIPlayer()->GetUnits();
        vector<CPtr<NAI::IAIUnit>> expectedAllies,expectedEnemies;
        state->GetUnits(&expectedAllies,true,true); state->GetUnits(&expectedEnemies,false,true);
        Require(allies==expectedAllies && enemies==expectedEnemies,
            "AI tactical views mixed ownership with current global non-enemy/enemy diplomacy");
        for (const auto& server: allUnits) {
            if (!IsValid(server)) continue;
            auto* wrapper=NAI::GetAIUnit(server);
            if (IsValid(wrapper) && server->GetPlayer()==player.GetPtr())
                Require(wrapper->GetAIState()==state,"commander failed to reconnect its owned unit's tactical state");
        }
        if (!expectedEnemies.empty()) ++hostileViews;
    }
    Require(checked > 1 && hostileViews > 0, "AI roster fixture did not verify multiple commanders and a hostile view");
}

namespace NAI
{
    // Existing external implementation; no new production API is needed for this probe.
    IAIUnit* FindNearestUnit(IAIUnit* self, vector<CPtr<IAIUnit>>& units);
}
class CoverOriginMapProbe: public NAI::IAIMap
{
    OBJECT_NOCOPY_METHODS(CoverOriginMapProbe);
public:
    CPtr<NAI::IAIMap> native;
    struct TraceRecord { CVec3 origin; int mask,width,height; };
    vector<TraceRecord> traces;
    explicit CoverOriginMapProbe(NAI::IAIMap* map=nullptr): native(map) {}
    void TraceGrid(NAI::CFastRenderer* grid,int mask,ESort sort,const NAI::CFloorsSet& floors,ESplitTerrainHGroups split,bool doors) override {
        Require(grid->resGrid.GetXSize()>0 && grid->resGrid.GetYSize()>0,"cover origin probe received no native projection grid");
        NAI::CFastRenderer::SResult interval{}; interval.fEnter=1; interval.fExit=2;
        auto* saved=grid->resGrid[0][0]; grid->resGrid[0][0]=&interval;
        vector<CVec3> entries,exits; grid->GetPoints(&entries,&exits,0,0); grid->resGrid[0][0]=saved;
        Require(entries.size()==1 && exits.size()==1,"cover origin probe could not read the native projective conversion");
        traces.push_back({entries.front()*2.f-exits.front(),mask,grid->resGrid.GetXSize(),grid->resGrid.GetYSize()});
        native->TraceGrid(grid,mask,sort,floors,split,doors); // Keep actual native geometry and intersections.
    }
    void Sync(ESyncType type) override {native->Sync(type);}
    void GetEntities(list<NAI::SObjectInfo>* out,int mask,const NAI::CFloorsSet& floors) override {native->GetEntities(out,mask,floors);}
    void Trace(const CRay& ray,vector<NAI::SInterval>* out,int mask,const NAI::CFloorsSet& floors,ESplitTerrainHGroups split) override {native->Trace(ray,out,mask,floors,split);}
    void TraceVoxelGrid(NAI::CExplVoxelRenderer* grid,int mask,const NAI::CFloorsSet& floors,bool doors) override {native->TraceVoxelGrid(grid,mask,floors,doors);}
    void TraceVisionGrid(NAI::CVisionVoxelRenderer* grid,int mask,const NAI::CFloorsSet& floors,bool doors) override {native->TraceVisionGrid(grid,mask,floors,doors);}
    bool GetUnitHLPos(CVec3* out,CObjectBase* hull,int id) override {return native->GetUnitHLPos(out,hull,id);}
    void GetAccessibleUnitHL(vector<int>* out,const CVec3& from,CObjectBase* hull,float distance) override {native->GetAccessibleUnitHL(out,from,hull,distance);}
    CObjectBase* GetHull(CObjectBase* object) override {return native->GetHull(object);}
    bool GetObjectBound(SBound* out,CObjectBase* object) override {return native->GetObjectBound(out,object);}
    bool CalcIntersection(const CVec3& centre,float radius,int flags,CObjectBase* ignore) override {return native->CalcIntersection(centre,radius,flags,ignore);}
    void PrepareCollider(NAI::IPrepareCollider* out,const SBound& bound,float size,int mask,bool doors) override {native->PrepareCollider(out,bound,size,mask,doors);}
    void AddTracker(NAI::IAIMapTracker* tracker,const SBound& bound,int mask,bool doors) override {native->AddTracker(tracker,bound,mask,doors);}
    void FlipDoorWindow(CObjectBase* object,bool open) override {native->FlipDoorWindow(object,open);}
    NAI::IStabilityTrackers* GetStabilityTrackers() override {return native->GetStabilityTrackers();}
    void SelectHullPointers(vector<CPtr<CObjectBase>>* out,const SBound& bound,int include,int exclude) override {native->SelectHullPointers(out,bound,include,exclude);}
    bool GetWindowPos(CObjectBase* object,CVec3* closed,CVec3* open) override {return native->GetWindowPos(object,closed,open);}
};
static void SetCoverProbeMap(NRPG::IGame* game,NAI::IAIMap* map)
{
    CStructureNetworkContext refs; auto traversal=BorrowedObjectFields(game,&refs);
    if (!refs.ids.count(map)) { const auto id=refs.nextID++; refs.ids[map]=id; refs.refs[id]=map; }
    CPtr<NAI::IAIMap> restored=map; CMemoryStream fields;
    {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs); writer.Add(2,&restored);}
    fields.Seek(0); {CStructureSaver reader(fields,CStructureSaver::READ,&refs); game->operator&(reader);}
}
namespace NRPG
{
    // Existing native implementation, kept out of the public gameplay interface.
    int GetHitCover(const NWorld::CUnit* attacker,CCoverInfo* cover);
}
using TerrainTraceRow=std::tuple<int,int,std::uintptr_t,int,int,int,float,float>;
static vector<TerrainTraceRow> TerrainTraceRows(NAI::CFastRenderer& renderer,bool cap)
{
    vector<TerrainTraceRow> rows;
    for(int y=0;y<renderer.resGrid.GetYSize();++y)for(int x=0;x<renderer.resGrid.GetXSize();++x)for(auto* p=renderer.resGrid[y][x];p;p=p->pNext){Require(p->pSrc && p->pSrc->pSrc,"terrain trace lost its native source ownership");const auto& info=p->GetInfo();const bool terrain=info.pUserData==nullptr;rows.emplace_back(y,x,reinterpret_cast<std::uintptr_t>(info.pUserData.GetPtr()),p->pSrc->nUserID,info.nFloor,info.nTSFlags,cap && terrain?p->fExit-0.01f:p->fEnter,p->fExit);}
    std::sort(rows.begin(),rows.end());return rows;
}
static void InitTerrainPassGrid(NAI::CFastRenderer& renderer,NAI::CLayersGroup* group,const CTRect<int>& region)
{
    CTRect<int> expanded(region);expanded.x1=expanded.x1*3-1;expanded.y1=expanded.y1*3-1;expanded.x2=expanded.x2*3+1;expanded.y2=expanded.y2*3+1;
    renderer.InitParallel(group->ptOrigin,atan2(group->ptXDir.y,group->ptXDir.x),FP_GRID_STEP/3,expanded);
}
class TerrainReductionProbe: public CoverOriginMapProbe
{
    OBJECT_NOCOPY_METHODS(TerrainReductionProbe);
public:
    NAI::CLayersGroup* group=nullptr;CTRect<int> region;bool reduce=true,matched=false,ordered=false;int calls=0,observed=-1,terrain=0,objects=0,changed=0,capped=0;
    explicit TerrainReductionProbe(NAI::IAIMap* map=nullptr): CoverOriginMapProbe(map) {}
    void TraceGrid(NAI::CFastRenderer* grid,int mask,ESort sort,const NAI::CFloorsSet& floors,ESplitTerrainHGroups split,bool doors) override {
        ++calls;observed=int(sort);NAI::CFastRenderer control;InitTerrainPassGrid(control,group,region);Require(control.resGrid.GetXSize()==grid->resGrid.GetXSize() && control.resGrid.GetYSize()==grid->resGrid.GetYSize(),"terrain probe did not clone the actual native supersampled projection");
        native->TraceGrid(&control,mask,STH_SORT_INTERVALS,floors,split,doors);const auto baseline=TerrainTraceRows(control,false),expected=TerrainTraceRows(control,reduce);
        for(const auto& row:baseline)if(std::get<2>(row)==0){++terrain;if(std::get<6>(row)!=std::get<7>(row)-0.01f)++changed;}else ++objects;
        native->TraceGrid(grid,mask,sort,floors,split,doors);matched=TerrainTraceRows(*grid,false)==expected;ordered=true;
        for(int y=0;y<grid->resGrid.GetYSize();++y)for(int x=0;x<grid->resGrid.GetXSize();++x){bool first=true;float last=0;for(auto* p=grid->resGrid[y][x];p;p=p->pNext){if(!first && p->fEnter<last)ordered=false;first=false;last=p->fEnter;if(p->GetInfo().pUserData==nullptr && p->fEnter==p->fExit-0.01f)++capped;}}
    }
};
static void TerrainReduction(const char* databasePath,const char* resourcePath,int mode)
{
    const int variant=mode==3?4517:810;CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game,variant);auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());Require(network && network->GetNumLayers()>0,"terrain reduction lacks its actual authored navigation network");auto* layer=network->GetLayer(0);auto* group=layer->pGroup.GetPtr();auto* map=world->GetAIMap();Require(map,"terrain reduction lacks its actual native map");map->Sync(NAI::IAIMap::ST_NORMAL);
    CTRect<int> region;bool found=false;
    for(const auto& object:world->GetObjectsForHarness()){CVec2 cell;const auto& pos=object->GetPosition();group->GetExactPoint(&cell,CVec2(pos.ptPos.x,pos.ptPos.y));int x=Float2Int(cell.x),y=Float2Int(cell.y);if(x<3 || y<3 || x>=layer->tiles.GetXSize()-3 || y>=layer->tiles.GetYSize()-3)continue;CTRect<int> candidate(x-2,y-2,x+3,y+3);NAI::CFastRenderer sample;InitTerrainPassGrid(sample,group,candidate);map->TraceGrid(&sample,NWorld::TS_PASS_BLOCKER,NAI::IAIMap::STH_SORT_INTERVALS,NAI::CFloorsSet(),NAI::IAIMap::STH_SPLIT_TERR_HG,true);int terrain=0,objects=0;for(const auto& row:TerrainTraceRows(sample,false))if(std::get<2>(row)==0)++terrain;else ++objects;if(terrain && objects){region=candidate;found=true;break;}}
    Require(found,"terrain reduction lacks real authored terrain and object intervals in a common projection");
    CObj<TerrainReductionProbe> probe=new TerrainReductionProbe(map);probe->group=group;probe->region=region;probe->reduce=mode==0 || mode==3;
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);vector<std::pair<int,int>> stats;vector<NAI::SUnitPosition> positions;for(auto unit:units){stats.push_back({unit->GetAP(),unit->GetUnitRPG()->GetTotalVP()});positions.push_back(unit->GetPosition());}const bool action=world->IsAction();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    if(probe->reduce){NAI::STempArrayGroup<NAI::STile> buffers;buffers.region=region;NAI::CPassCalcer calcer(network,group,probe,buffers,region);calcer.Calc();}
    else{NAI::CFastRenderer renderer;InitTerrainPassGrid(renderer,group,region);probe->TraceGrid(&renderer,NWorld::TS_PASS_BLOCKER,mode==1?NAI::IAIMap::STH_SORT_INTERVALS:NAI::IAIMap::STH_NOSORT,NAI::CFloorsSet(),NAI::IAIMap::STH_SPLIT_TERR_HG,true);}
    auto afterRandom=GlobalGameRandom();bool unchanged=action==world->IsAction() && time==world->GetTime()->GetValue() && random.Get()==afterRandom.Get();int i=0;for(auto unit:units){unchanged&=stats[i].first==unit->GetAP() && stats[i].second==unit->GetUnitRPG()->GetTotalVP() && positions[i]==unit->GetPosition();++i;}
    std::printf("native terrain reduction: mode=%d variant=%d region=%d,%d,%d,%d sort=%d calls=%d terrain=%d objects=%d changed=%d capped=%d matched=%d ordered=%d units=%u unchanged=%d\n",mode,variant,region.minx,region.miny,region.maxx,region.maxy,probe->observed,probe->calls,probe->terrain,probe->objects,probe->changed,probe->capped,int(probe->matched),int(probe->ordered),unsigned(stats.size()),int(unchanged));std::fflush(stdout);
    Require(probe->calls==1 && probe->terrain>0 && probe->objects>0 && probe->changed>0 && unchanged,"terrain reduction lost its real mixed geometry, query-only state or call boundary");
    Require(probe->observed==(probe->reduce?2:mode==1?1:0) && probe->matched && (mode==2 || probe->ordered) && probe->capped==(probe->reduce?probe->terrain:0),"native passability tracing retained terrain volume or changed object/default trace intervals instead of reducing only terrain before sorting");
}

class CoverReachMapProbe: public CoverOriginMapProbe
{
    OBJECT_NOCOPY_METHODS(CoverReachMapProbe);
public:
    CVec3 source,aim,fullAim;
    CPtr<NWorld::CUnitServer> target;
    bool shortenAim=false,inspectTarget=true;
    int targetIntervals=0,reachableIntervals=0,beyondIntervals=0;
    explicit CoverReachMapProbe(NAI::IAIMap* map=nullptr): CoverOriginMapProbe(map) {}
    bool GetUnitHLPos(CVec3* out,CObjectBase* hull,int id) override {
        if(!native->GetUnitHLPos(out,hull,id))return false;
        fullAim=*out;
        if(shortenAim) {
            CVec3 direction=*out-source;
            Require(fabs(direction)>4.f,"cover reach fixture has no separated aim point and target geometry");
            Normalize(&direction);*out=source+direction;
        }
        aim=*out;return true;
    }
    void TraceGrid(NAI::CFastRenderer* grid,int mask,ESort sort,const NAI::CFloorsSet& floors,ESplitTerrainHGroups split,bool doors) override {
        if(shortenAim) {
            // Retain the real target frustum while independently exercising a
            // shorter penetration cutoff. A near frustum would cull the target
            // before the native cover walk can examine its actual intervals.
            const CVec3 direction=fullAim-source;
            const float square=fabs(direction.x,direction.y)*tan(FP_PI8);
            const int half=grid->resGrid.GetXSize()/2;
            const CVec2 view=half==5 ? CVec2(square,Max(1.f,square)) : CVec2(Min(1.f,square),1.f);
            grid->InitProjective(source,fullAim,view,half);
        }
        CoverOriginMapProbe::TraceGrid(grid,mask,sort,floors,split,doors);
        if(!inspectTarget)return;
        CVec3 direction=aim-source;const float distance=fabs(direction)+1.f;Normalize(&direction);
        for(int y=0;y<grid->resGrid.GetYSize();++y)for(int x=0;x<grid->resGrid.GetXSize();++x) {
            CVec3 ray;grid->GetDir(&ray,x,y);const float projection=direction*ray;
            Require(projection>0,"native cover projection contains a backward ray");
            for(auto* hit=grid->resGrid[y][x];hit;hit=hit->pNext)if(hit->GetInfo().pUserData==target.GetPtr()) {
                ++targetIntervals;
                if(hit->fEnter>=distance/projection)++beyondIntervals;
                else if(hit->fExit>=1.f && (hit->GetInfo().nTSFlags&NWorld::TS_COVER))++reachableIntervals;
            }
        }
    }
    void Reset(){traces.clear();targetIntervals=reachableIntervals=beyondIntervals=0;}
};
class CoverRangeMapProbe: public CoverOriginMapProbe
{
    OBJECT_NOCOPY_METHODS(CoverRangeMapProbe);
public:
    CVec3 source,aim;
    CPtr<NWorld::CUnitServer> target;
    int targetIntervals=0,insideCap=0,outsideCap=0,insideNaturalRange=0;
    explicit CoverRangeMapProbe(NAI::IAIMap* map=nullptr): CoverOriginMapProbe(map) {}
    bool GetUnitHLPos(CVec3* out,CObjectBase* hull,int id) override {
        const bool result=native->GetUnitHLPos(out,hull,id);if(result)aim=*out;return result;
    }
    void TraceGrid(NAI::CFastRenderer* grid,int mask,ESort sort,const NAI::CFloorsSet& floors,ESplitTerrainHGroups split,bool doors) override {
        CoverOriginMapProbe::TraceGrid(grid,mask,sort,floors,split,doors);
        CVec3 direction=aim-source;const float naturalDistance=fabs(direction)+1.f;Normalize(&direction);
        for(int y=0;y<grid->resGrid.GetYSize();++y)for(int x=0;x<grid->resGrid.GetXSize();++x) {
            CVec3 ray;grid->GetDir(&ray,x,y);const float projection=direction*ray;
            Require(projection>0,"range fixture contains a backward native projection ray");
            for(auto* hit=grid->resGrid[y][x];hit;hit=hit->pNext)if(hit->GetInfo().pUserData==target.GetPtr()) {
                ++targetIntervals;
                Require(hit->fExit>=1.f && (hit->GetInfo().nTSFlags&NWorld::TS_COVER),"range fixture target lacks eligible native cover geometry");
                if(hit->fEnter<30.f/projection)++insideCap;else ++outsideCap;
                if(hit->fEnter<naturalDistance/projection)++insideNaturalRange;
            }
        }
    }
};
// Controlled closed-box collision geometry is rasterized by the native renderer.
// No interval, cover score or hit decision is supplied by this map fixture.
class CoverProjectionMapProbe: public CoverOriginMapProbe
{
    OBJECT_NOCOPY_METHODS(CoverProjectionMapProbe);
public:
    vector<CVec3> points;
    NAI::CEdgesInfo edges;
    SFBTransform transform;
    NAI::SSourceInfo surface;
    CVec3 source;
    float cutoff=30.f;
    int intersections=0,directInside=0,projectedInside=0,primaryHits=0,secondaryHits=0;
    explicit CoverProjectionMapProbe(NAI::IAIMap* map=nullptr): CoverOriginMapProbe(map) {
        Identity(&transform.forward);Identity(&transform.backward);
    }
    void Box(NWorld::CUnitServer* owner,const CVec3& a,const CVec3& z) {
        surface=NAI::SSourceInfo(owner,owner->GetUnitRPG()->GetRPGArmor(),owner->GetFloor(),NWorld::TS_COVER|NWorld::TS_UNITS);
        points={CVec3(a.x,a.y,a.z),CVec3(z.x,a.y,a.z),CVec3(z.x,z.y,a.z),CVec3(a.x,z.y,a.z),
            CVec3(a.x,a.y,z.z),CVec3(z.x,a.y,z.z),CVec3(z.x,z.y,z.z),CVec3(a.x,z.y,z.z)};
        const vector<STriangle> triangles={STriangle(0,2,1),STriangle(0,3,2),STriangle(4,5,6),STriangle(4,6,7),
            STriangle(0,1,5),STriangle(0,5,4),STriangle(3,7,6),STriangle(3,6,2),
            STriangle(0,4,7),STriangle(0,7,3),STriangle(1,2,6),STriangle(1,6,5)};
        edges.GenerateEdgeList(triangles,points);
        Require(edges.IsClosed(),"projection fixture box is not closed native collision geometry");
    }
    void TraceGrid(NAI::CFastRenderer* grid,int mask,ESort sort,const NAI::CFloorsSet&,ESplitTerrainHGroups,bool) override {
        Require((mask&NWorld::TS_COVER) && sort==STH_SORT_INTERVALS,"projection fixture lost native sorted cover tracing");
        NAI::CFastRenderer::SResult measurement{};measurement.fEnter=1;measurement.fExit=2;
        auto* saved=grid->resGrid[0][0];grid->resGrid[0][0]=&measurement;
        vector<CVec3> enters,exits;grid->GetPoints(&enters,&exits,0,0);grid->resGrid[0][0]=saved;
        Require(enters.size()==1 && exits.size()==1,"projection fixture could not inspect the native projection origin");
        traces.push_back({enters.front()*2.f-exits.front(),mask,grid->resGrid.GetXSize(),grid->resGrid.GetYSize()});
        NAI::SConvexHull hull(points,edges,transform,surface,NAI::HL_BODY,vector<CPtr<NAI::CPrecalcSpheres>>{});
        grid->TraceEntity(hull,false);grid->SortIntervals(); // Actual native rasterization and distance conversion.
        for(int y=0;y<grid->resGrid.GetYSize();++y)for(int x=0;x<grid->resGrid.GetXSize();++x) {
            CVec3 direction;grid->GetDir(&direction,x,y);const float projection=direction.y;
            Require(projection>0,"projection fixture has a backward native ray");
            for(auto* hit=grid->resGrid[y][x];hit;hit=hit->pNext) {
                Require(hit->GetInfo().pUserData==surface.pUserData && hit->fExit>=hit->fEnter,"native box rasterization lost surface ownership or ordered intervals");
                ++intersections;if(traces.size()==1)++primaryHits;else ++secondaryHits;
                if(hit->fEnter<cutoff)++directInside;
                if(hit->fEnter<cutoff/projection)++projectedInside;
            }
        }
    }
};
static void AICoverProjection(const char* databasePath,const char* resourcePath,int mode)
{
    const bool human=mode==2 || mode==3,tile=mode>=4 && mode<=6,sliver=mode==0 || mode==2 || mode==4,pk=mode==7;
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> all;world->GetAllUnits(&all);
    NWorld::CUnitServer *actor=nullptr,*target=nullptr,*ally=nullptr;NRPG::IWeaponItem* firearm=nullptr;
    for(const auto& candidate:all) {
        if(!IsValid(candidate) || !candidate->CanFight() || candidate->HasCommand() || candidate->IsEmptyPK() || candidate->IsWearingPK() || candidate->animator.GetCannon())continue;
        auto* weapon=dynamic_cast<NRPG::IWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
        if(weapon && weapon->GetAmmoQuantity()>0 && weapon->SetShootMode(NDb::SM_Snap)){actor=candidate.GetPtr();firearm=weapon;break;}
    }
    Require(actor && firearm,"projection fixture has no armed native actor");
    for(const auto& unit:all)if(IsValid(unit) && unit.GetPtr()!=actor && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) {
        if(!target && actor->GetDiplomacyState(unit)==NDb::DS_ENEMY)target=unit.GetPtr();
        if(!ally && actor->GetDiplomacyState(unit)==NDb::DS_ALLY)ally=unit.GetPtr();
    }
    Require(target && ally,"projection fixture has no native enemy/ally ownership controls");
    CObj<NWorld::CUnitServer> shell;
    if(pk) {
        NDb::CRPGPers* persona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
        while(people.MoveNext())if(people.Get()->pPanzerklein && !people.Get()->pPanzerklein->bHasNoHead && people.Get()->pModel){persona=people.Get();break;}
        Require(persona,"projection fixture has no regular native PK persona");
        auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(target->GetPosition().GetCP(),20.f),&places);
        NAI::SPathPlace anchor;bool found=false;
        for(auto place:places){place.SetPose(NAI::CM_STAND);place.SetDirection(0);if(NAI::IsBigLockerPlacePassable(network,place)){anchor=place;found=true;break;}}
        Require(found,"projection fixture has no free native PK footprint");
        shell=world->AddUnitInGame(anchor,NRPG::CreateUnit(persona),nullptr);
        Require(shell && shell->IsEmptyPK() && shell->WearAsPK(true),"projection fixture could not prepare its real PK shell");
        target->SetPosition(shell->GetPosition());target->FlipPanzerklein(shell,false,false);world->GetAIMap()->Sync();
        Require(target->IsWearingPK() && target->GetDecalsRef()==shell.GetPtr(),"projection fixture lost the fork's native pilot/surface-owner relation");
    }
    NRPG::IGame* game=world->GetGame();CPtr<NAI::IAIMap> original;
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(game,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&original);}
    CVec3 aim;Require(original.GetPtr()==world->GetAIMap() && original->GetUnitHLPos(&aim,original->GetHull(target),NAI::HL_ANY),"projection fixture has no actual native target locator");
    const float distance=human?8.f:pk?6.f:30.f;
    const CVec3 source=tile?VNULL3:aim-CVec3(0,distance,0),tileTarget(0,30.25f,0);
    CObj<CoverProjectionMapProbe> probe=new CoverProjectionMapProbe(original);probe->source=source;probe->cutoff=tile?29.625f:Min(distance+1.f,30.f);
    CVec3 lo,hi;
    if(tile) {
        // Tile target30.25 minus original0.625 pullback gives the actual29.625 plane.
        const float plane=mode==4?29.62495f:mode==5?29.626f:29.615f;
        lo=CVec3(-.25f,plane,-.25f);hi=CVec3(.25f,plane+.01f,.25f);
    } else if(human) {
        const float plane=sliver?7.999f:7.8f;
        lo=source+CVec3(2.8f,plane,2.8f);hi=source+CVec3(3.2f,plane+.01f,3.2f);
    } else {
        const float plane=sliver?29.999f:pk?5.5f:29.f;
        lo=source+CVec3(.5f,plane,-.125f);hi=source+CVec3(1.f,plane+.01f,.125f);
    }
    probe->Box(tile?ally:target,lo,hi);
    vector<NRPG::CAttackPortion> attacks;actor->GetUnitRPG()->CreateAttack(&attacks,false);
    Require(!attacks.empty() && attacks.front().nK>0 && !attacks.front().IsArmorIgnored(probe->surface.pArmor),"projection fixture has no eligible native attack/surface material");
    if(tile)Require(attacks.front().CanDealDmg(probe->surface.pArmor) && actor->GetDiplomacyState(ally)==NDb::DS_ALLY,"projected tile control lacks its native ally blocker");
    struct Restore{NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}}restore{game,original};SetCoverProbeMap(game,probe);
    const auto actorPosition=actor->GetPosition(),targetPosition=target->GetPosition(),allyPosition=ally->GetPosition();
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),targetHP=target->GetUnitRPG()->GetTotalVP(),allyHP=ally->GetUnitRPG()->GetTotalVP(),ammo=firearm->GetAmmoQuantity();
    const int shellHP=shell?shell->GetUnitRPG()->GetTotalVP():0;const bool command=actor->HasCommand();GlobalGameRandom().SeedForHarness(0x73da);auto beforeRandom=GlobalGameRandom();
    int score;
    if(tile || human) {
        CObj<NRPG::CCoverInfo> cover=tile?game->CalcCoversForTile(source,attacks.front(),actor,tileTarget,0.f):
            game->CalcCovers(source,attacks.front(),actor,target,NAI::HL_ANY,1.f,false);
        score=NRPG::GetHitCover(actor,cover);
    } else score=game->GetCoverForAIUnit(source,actor,target,attacks.front(),NAI::HL_ANY);
    std::printf("native cover projection: mode=%d human=%d tile=%d pk=%d score=%d intersections=%d direct=%d projected=%d primary=%d secondary=%d grids=%zu cutoff=%.2f\n",
        mode,int(human),int(tile),int(pk),score,probe->intersections,probe->directInside,probe->projectedInside,probe->primaryHits,probe->secondaryHits,probe->traces.size(),probe->cutoff);std::fflush(stdout);
    Require(probe->intersections>0 && probe->traces.size()==(human?2u:1u),"projection fixture did not rasterize actual box intersections in the expected native grids");
    for(const auto& trace:probe->traces)Require(fabs(trace.origin-source)<2e-4f,"projection fixture changed the native source or projection");
    if(human)Require(probe->primaryHits==0 && probe->secondaryHits>0,"non-AI projection fixture did not exercise the native low-resolution grid");
    if(sliver)Require(probe->directInside==0 && probe->projectedInside==probe->intersections,"projection fixture did not separate radial and projected ray cutoffs");
    else if(mode==5)Require(probe->directInside==0 && probe->projectedInside==0,"far tile control did not put its actual ally surface beyond both cutoffs");
    else Require(probe->directInside==probe->intersections && probe->projectedInside==probe->intersections,"near projection control is not actually within both ray cutoffs");
    const bool blocked=sliver || mode==6;
    Require(blocked?score<=0:score>0,"unit ray used a projected cutoff or tile aiming lost its projected ally/default-open behavior");
    Require(GlobalGameRandom().Get()==beforeRandom.Get(),"projection queries consumed native RNG");
    Require(actor->GetPosition()==actorPosition && target->GetPosition()==targetPosition && ally->GetPosition()==allyPosition && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp
        && target->GetUnitRPG()->GetTotalVP()==targetHP && ally->GetUnitRPG()->GetTotalVP()==allyHP && firearm->GetAmmoQuantity()==ammo && actor->HasCommand()==command
        && (!pk || (target->IsWearingPK() && target->GetDecalsRef()==shell.GetPtr() && shell->GetUnitRPG()->GetTotalVP()==shellHP)),"projection queries changed gameplay state or the fork's PK ownership");
}

namespace NWorld
{
    int GetMask(NDb::CAIGeometry* geometry,NDb::CRPGArmor* armor);
}
class CoverMaskObstacle: public CObjectBase
{
    OBJECT_NOCOPY_METHODS(CoverMaskObstacle);
};
// Both boxes use the actual native rasterizer; the real world supplies the
// units, weapon, armor, target locator and projection. No hit result is injected.
class CoverMaskMapProbe: public CoverProjectionMapProbe
{
    OBJECT_NOCOPY_METHODS(CoverMaskMapProbe);
public:
    CObj<CoverProjectionMapProbe> obstacle;
    CObj<CoverMaskObstacle> owner;
    bool enabled=false;
    int obstacleHits=0,nearHits=0,selectedGrids=0;
    explicit CoverMaskMapProbe(NAI::IAIMap* map=nullptr): CoverProjectionMapProbe(map) {
        obstacle=new CoverProjectionMapProbe(map);owner=new CoverMaskObstacle;
    }
    void TraceGrid(NAI::CFastRenderer* grid,int mask,ESort sort,const NAI::CFloorsSet& floors,ESplitTerrainHGroups split,bool doors) override {
        CoverProjectionMapProbe::TraceGrid(grid,mask,sort,floors,split,doors);
        if(!enabled || !(mask&obstacle->surface.nTSFlags))return;
        ++selectedGrids;
        NAI::SConvexHull hull(obstacle->points,obstacle->edges,obstacle->transform,obstacle->surface,NAI::HL_BODY,vector<CPtr<NAI::CPrecalcSpheres>>{});
        grid->TraceEntity(hull,false);grid->SortIntervals();
        for(int y=0;y<grid->resGrid.GetYSize();++y)for(int x=0;x<grid->resGrid.GetXSize();++x)
            for(auto* hit=grid->resGrid[y][x];hit;hit=hit->pNext)if(&hit->GetInfo()==&obstacle->surface) {
                ++obstacleHits;if(hit->fEnter<1.f)++nearHits;
            }
    }
    void Reset(){traces.clear();intersections=directInside=projectedInside=primaryHits=secondaryHits=obstacleHits=nearHits=selectedGrids=0;}
};
static void AICoverMask(const char* databasePath,const char* resourcePath,int mode)
{
    const int role=mode<12?mode/4:mode==12?0:2,kind=mode<12?mode%4:0;
    const bool human=role!=0,low=role==2,ignored=mode==12,farObstacle=mode==13;
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> all;world->GetAllUnits(&all);
    NWorld::CUnitServer *actor=nullptr,*target=nullptr;NRPG::IWeaponItem* firearm=nullptr;
    for(const auto& unit:all) {
        if(!IsValid(unit) || !unit->CanFight() || unit->HasCommand() || unit->IsEmptyPK() || unit->IsWearingPK() || unit->animator.GetCannon())continue;
        auto* weapon=dynamic_cast<NRPG::IWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        if(weapon && weapon->GetAmmoQuantity()>0 && weapon->SetShootMode(NDb::SM_Snap)){actor=unit.GetPtr();firearm=weapon;break;}
    }
    Require(actor && firearm,"mask fixture has no armed native actor");
    for(const auto& unit:all)if(IsValid(unit) && unit.GetPtr()!=actor && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()
        && actor->GetDiplomacyState(unit)==NDb::DS_ENEMY){target=unit.GetPtr();break;}
    Require(target,"mask fixture has no native enemy target");
    NRPG::IGame* game=world->GetGame();CPtr<NAI::IAIMap> original;
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(game,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&original);}
    CVec3 aim;Require(original.GetPtr()==world->GetAIMap() && original->GetUnitHLPos(&aim,original->GetHull(target),NAI::HL_ANY),"mask fixture has no native target locator");
    const CVec3 source=aim-CVec3(0,8.f,0);
    CObj<CoverMaskMapProbe> probe=new CoverMaskMapProbe(original);probe->source=source;probe->cutoff=9.f;
    probe->Box(target,source+(low?CVec3(2.8f,7.8f,2.8f):CVec3(-.4f,7.8f,-.4f)),
        source+(low?CVec3(3.2f,7.81f,3.2f):CVec3(.4f,7.81f,.4f)));
    const float plane=farObstacle?2.f:.3f;
    probe->obstacle->Box(target,source+CVec3(-.25f,plane-.2f,-.25f),source+CVec3(.25f,plane+.2f,.25f));
    auto& surface=probe->obstacle->surface;
    CObj<NDb::CAIGeometry> geometry=new NDb::CAIGeometry;
    geometry->traficability=static_cast<NDb::ETrafic>(kind==1?NDb::TR_PASS:kind==2?NDb::TR_ITEM_BLOCKER:NDb::TR_PASS|NDb::TR_ITEM_BLOCKER);
    surface.nTSFlags=NWorld::GetMask(geometry,surface.pArmor);
    if(kind==3)surface.nTSFlags=NWorld::TS_FRAGMENTED|NWorld::TS_WEAPON_BLOCKER;
    surface.pUserData=ignored?static_cast<CObjectBase*>(actor):probe->owner.GetPtr();
    Require(surface.nTSFlags==(kind==0?0x8508:kind==1?0x508:kind==2?0x8008:0x410),"native geometry classification changed the original item/pass/cover categories");
    vector<NRPG::CAttackPortion> attacks;actor->GetUnitRPG()->CreateAttack(&attacks,false);
    Require(!attacks.empty() && attacks.front().nK>0 && !attacks.front().IsArmorIgnored(probe->surface.pArmor),"mask fixture has no native attack/target material");
    struct Restore{NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}}restore{game,original};SetCoverProbeMap(game,probe);
    const auto actorPosition=actor->GetPosition(),targetPosition=target->GetPosition();
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),targetHP=target->GetUnitRPG()->GetTotalVP(),ammo=firearm->GetAmmoQuantity();
    const bool command=actor->HasCommand();GlobalGameRandom().SeedForHarness(0x74da);auto beforeRandom=GlobalGameRandom();
    auto query=[&]() {
        if(!human)return game->GetCoverForAIUnit(source,actor,target,attacks.front(),NAI::HL_ANY);
        CObj<NRPG::CCoverInfo> cover=game->CalcCovers(source,attacks.front(),actor,target,NAI::HL_ANY,1.f,false);
        return NRPG::GetHitCover(actor,cover);
    };
    const int baseline=query();
    Require(baseline>0 && probe->intersections>0,"mask fixture has no positive native target baseline");
    if(low)Require(probe->primaryHits==0 && probe->secondaryHits>0,"mask fixture did not isolate the low-resolution target grid");
    probe->Reset();probe->enabled=true;const int actual=query();
    std::printf("native cover mask: mode=%d role=%d flags=%x baseline=%d score=%d target=%d primary=%d secondary=%d obstacle=%d near=%d selected=%d grids=%zu ignored=%d farObstacle=%d\n",
        mode,role,surface.nTSFlags,baseline,actual,probe->intersections,probe->primaryHits,probe->secondaryHits,probe->obstacleHits,probe->nearHits,probe->selectedGrids,probe->traces.size(),int(ignored),int(farObstacle));std::fflush(stdout);
    Require(probe->traces.size()==(human?2u:1u) && probe->intersections>0,"mask query lost native target intersections or projection grids");
    for(const auto& trace:probe->traces)Require(fabs(trace.origin-source)<2e-4f,"mask query changed the native projection origin");
    if(low)Require(probe->primaryHits==0 && probe->secondaryHits>0,"mask query no longer exercises the low-resolution target grid");
    if(kind==1 || kind==3)Require(probe->selectedGrids==0 && probe->obstacleHits==0,"preview/AI selected an original-excluded pass-only or fragmented-only obstacle");
    if(probe->selectedGrids)Require(probe->obstacleHits>0 && (farObstacle?probe->nearHits==0:probe->nearHits==probe->obstacleHits),"native obstacle rasterization missed the intended muzzle-clearance interval");
    Require(GlobalGameRandom().Get()==beforeRandom.Get() && actor->GetPosition()==actorPosition && target->GetPosition()==targetPosition
        && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && target->GetUnitRPG()->GetTotalVP()==targetHP
        && firearm->GetAmmoQuantity()==ammo && actor->HasCommand()==command,"mask queries changed native gameplay state or RNG");
    Require(kind==0 && !ignored && !farObstacle?actual<=0:actual==baseline,"unit cover sampling missed an item-blocking muzzle obstacle or changed an original category/ignore/distance control");
}

class CoverFragmentParityMapProbe: public CoverOriginMapProbe
{
    OBJECT_NOCOPY_METHODS(CoverFragmentParityMapProbe);
public:
    bool includeFragments=false;
    int extraIntervals=0;
    CPtr<NWorld::CUnitServer> target;
    explicit CoverFragmentParityMapProbe(NAI::IAIMap* map=nullptr): CoverOriginMapProbe(map) {}
    void TraceGrid(NAI::CFastRenderer* grid,int mask,ESort sort,const NAI::CFloorsSet& floors,ESplitTerrainHGroups split,bool doors) override {
        CoverOriginMapProbe::TraceGrid(grid,mask|(includeFragments?NWorld::TS_FRAGMENTED:0),sort,floors,split,doors);
        for(int y=0;y<grid->resGrid.GetYSize();++y)for(int x=0;x<grid->resGrid.GetXSize();++x)
            for(auto* hit=grid->resGrid[y][x];hit;hit=hit->pNext) {
                const auto& surface=hit->GetInfo();
                if(surface.nTSFlags&(NWorld::TS_COVER|NWorld::TS_ITEM_BLOCKER))continue;
                ++extraIntervals;
                Require(includeFragments && (surface.nTSFlags&NWorld::TS_FRAGMENTED)
                    && !(surface.nTSFlags&NWorld::TS_WEAPON_BLOCKER) && surface.pUserData!=target.GetPtr()
                    && !CDynamicCast<NWorld::CUnit>(surface.pUserData),
                    "actual fragment-only provider can affect muzzle clearance or unit-target classification");
            }
    }
};
static bool SameCoverRays(const vector<CoverParitySnapshot::Ray>& a,const vector<CoverParitySnapshot::Ray>& b)
{
    if(a.size()!=b.size())return false;
    for(size_t i=0;i<a.size();++i)if(a[i].direction!=b[i].direction || a[i].penetrate!=b[i].penetrate || a[i].deviation!=b[i].deviation)return false;
    return true;
}
static void AICoverNativeFragmentParity(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;NRPG::IWeaponItem* firearm=nullptr;
    for(const auto& unit:units) {
        if(!IsValid(unit) || !unit->CanFight() || unit->HasCommand() || unit->IsEmptyPK() || unit->IsWearingPK() || unit->animator.GetCannon())continue;
        auto* weapon=dynamic_cast<NRPG::IWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        if(weapon && weapon->GetAmmoQuantity()>0 && weapon->SetShootMode(NDb::SM_Snap)){actor=unit.GetPtr();firearm=weapon;break;}
    }
    Require(actor && firearm,"fragment parity fixture has no armed native actor");
    NRPG::IGame* game=world->GetGame();CPtr<NAI::IAIMap> original;
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(game,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&original);}
    Require(original.GetPtr()==world->GetAIMap(),"fragment parity fixture lost the actual collision map");
    CObj<CoverFragmentParityMapProbe> probe=new CoverFragmentParityMapProbe(original);
    struct Restore{NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}}restore{game,original};SetCoverProbeMap(game,probe);
    vector<NRPG::CAttackPortion> attacks;actor->GetUnitRPG()->CreateAttack(&attacks,false);
    Require(!attacks.empty(),"fragment parity fixture has no native attack portion");
    const auto position=actor->GetPosition();const CVec3 source=actor->GetAttackOrigin(position);
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),ammo=firearm->GetAmmoQuantity();const bool command=actor->HasCommand();
    GlobalGameRandom().SeedForHarness(0x75da);auto rng=GlobalGameRandom();
    int compared=0,positive=0,fragments=0;
    for(const auto& target:units)if(IsValid(target) && target.GetPtr()!=actor && target->CanFight() && !target->IsEmptyPK()) {
        probe->target=target;
        for(const auto location:{NAI::HL_ANY,NAI::HL_BODY}) {
            const auto targetPosition=target->GetPosition();const int targetHP=target->GetUnitRPG()->GetTotalVP();
            auto query=[&](bool include){probe->includeFragments=include;probe->extraIntervals=0;probe->traces.clear();
                return CObj<NRPG::CCoverInfo>(game->CalcCovers(source,attacks.front(),actor,target,location,actor->GetMinClearDistance(),false));};
            auto current=query(false);const auto a=CoverSnapshot(current);const int score=NRPG::GetHitCover(actor,current);
            Require(probe->extraIntervals==0 && probe->traces.size()==2,"current native unit query lost its high/low grids or selected fragment-only geometry");
            auto reference=query(true);const auto z=CoverSnapshot(reference);const int expected=NRPG::GetHitCover(actor,reference);
            Require(probe->traces.size()==2 && a.source==z.source && a.piercing==z.piercing && SameCoverRays(a.hits,z.hits)
                && SameCoverRays(a.loose,z.loose) && SameCoverRays(a.obstacles,z.obstacles) && score==expected,
                "native fragment-only sampling changes a unit ray, classification or armor-piercing result");
            ++compared;if(score>0)++positive;fragments+=probe->extraIntervals;
            std::printf("native cover fragment parity: profile=%d location=%d score=%d fragments=%d hits=%zu loose=%zu obstacles=%zu\n",
                compared,int(location),score,probe->extraIntervals,a.hits.size(),a.loose.size(),a.obstacles.size());std::fflush(stdout);
            Require(target->GetPosition()==targetPosition && target->GetUnitRPG()->GetTotalVP()==targetHP,"fragment parity queries changed the native target");
        }
    }
    Require(compared>1 && positive>0 && fragments>0,"native parity fixture did not encounter both real fragment-only geometry and a positive unit shot");
    Require(actor->GetPosition()==position && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && firearm->GetAmmoQuantity()==ammo
        && actor->HasCommand()==command && rng.Get()==GlobalGameRandom().Get(),"fragment parity queries changed native gameplay or RNG");
}

static void AIToHitWeaponOwner(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    for(const auto& server:units)if(IsValid(server) && server->CanFight() && IsValid(NAI::GetAIUnit(server)))NAI::GetAIUnit(server)->Synchronize(false);
    NAI::IAIUnit *actor=nullptr,*target=nullptr;NAI::CAIFireArmsWeapon *ownWeapon=nullptr,*enemyWeapon=nullptr;
    int ownReference=0,enemyReference=0;
    auto reference=[](NAI::IAIUnit* shooter,NAI::IAIUnit* victim,NAI::CAIFireArmsWeapon* weapon) {
        const auto pos=shooter->GetUnitPosition();
        const int cover=shooter->GetCoverForFixedUnit(pos,victim->GetUnitServer(),weapon->GetItem(),NAI::HL_ANY);
        CObj<NRPG::CAIUnitToHitCalcer> calc=new NRPG::CAIUnitToHitCalcer(shooter,pos,victim,cover,NAI::HL_ANY,0,weapon->GetItem(),0);
        return calc->GetToHit();
    };
    for(const auto& server:units) {
        if(!IsValid(server) || !server->CanFight() || server->HasCommand() || !server->IsAIUnit() || server->IsWearingPK() || server->IsEmptyPK() || server->animator.GetCannon())continue;
        auto* shooter=NAI::GetAIUnit(server);if(!IsValid(shooter) || !IsValid(shooter->GetAIInventory()))continue;
        auto* weapon=shooter->GetAIInventory()->GetCurrentFireArms();if(!IsValid(weapon) || !IsValid(weapon->GetItem()))continue;
        for(const auto& other:units) {
            if(!IsValid(other) || other.GetPtr()==server.GetPtr() || !other->CanFight() || other->IsEmptyPK() || other->IsWearingPK())continue;
            auto* victim=NAI::GetAIUnit(other);if(!IsValid(victim) || !IsValid(victim->GetAIInventory()))continue;
            auto* victimWeapon=victim->GetAIInventory()->GetCurrentFireArms();if(!IsValid(victimWeapon) || !IsValid(victimWeapon->GetItem()))continue;
            const int own=reference(shooter,victim,weapon),enemy=reference(shooter,victim,victimWeapon);
            if(own<=0 || enemy<=0)continue;
            actor=shooter;target=victim;ownWeapon=weapon;enemyWeapon=victimWeapon;ownReference=own;enemyReference=enemy;break;
        }
        if(actor)break;
    }
    Require(actor && target && ownWeapon && enemyWeapon,"weapon-owner fixture has no pair with independently positive native shooter/target weapon calculations");
    auto* actorInventory=actor->GetAIInventory();auto* targetInventory=target->GetAIInventory();
    CObj<NAI::IAIInventoryItem> ownCurrent=actorInventory->GetCurrentItem(),targetCurrent=targetInventory->GetCurrentItem();
    struct Restore{NAI::CAIInventory *own,*target;CObj<NAI::IAIInventoryItem> a,b;~Restore(){own->SetCurrentItem(a);target->SetCurrentItem(b);}}restore{actorInventory,targetInventory,ownCurrent,targetCurrent};
    // Speculative AI hands are separate from the real native equipment. Empty
    // one planning hand without replacing weapons, combat data or collision.
    const bool emptyOwn=mode==1 || mode==3,emptyTarget=mode==0 || mode==3;
    if(emptyOwn)actorInventory->SetCurrentItem(nullptr);
    if(emptyTarget)targetInventory->SetCurrentItem(nullptr);
    auto* actorServer=actor->GetUnitServer();auto* targetServer=target->GetUnitServer();
    const auto actorPosition=actorServer->GetPosition(),targetPosition=targetServer->GetPosition();
    const int ap=actorServer->GetAP(),hp=actorServer->GetUnitRPG()->GetTotalVP(),targetHP=targetServer->GetUnitRPG()->GetTotalVP();
    const int ownAmmo=ownWeapon->GetItem()->GetAmmoQuantity(),targetAmmo=enemyWeapon->GetItem()->GetAmmoQuantity();
    auto* equipped=actorServer->GetUnitRPG()->GetInventory()->GetActive();auto* targetEquipped=targetServer->GetUnitRPG()->GetInventory()->GetActive();
    const bool command=actorServer->HasCommand();GlobalGameRandom().SeedForHarness(0x76da);auto rng=GlobalGameRandom();
    const int expected=emptyOwn?0:ownReference;
    const int actual=actor->GetToHit(target,actor->GetUnitPosition(),NAI::HL_ANY);
    std::printf("native AI to-hit owner: mode=%d ownEmpty=%d targetEmpty=%d ownReference=%d targetWeaponReference=%d actual=%d expected=%d\n",
        mode,int(emptyOwn),int(emptyTarget),ownReference,enemyReference,actual,expected);std::fflush(stdout);
    Require(actorServer->GetPosition()==actorPosition && targetServer->GetPosition()==targetPosition && actorServer->GetAP()==ap
        && actorServer->GetUnitRPG()->GetTotalVP()==hp && targetServer->GetUnitRPG()->GetTotalVP()==targetHP
        && ownWeapon->GetItem()->GetAmmoQuantity()==ownAmmo && enemyWeapon->GetItem()->GetAmmoQuantity()==targetAmmo
        && actorServer->GetUnitRPG()->GetInventory()->GetActive()==equipped && targetServer->GetUnitRPG()->GetInventory()->GetActive()==targetEquipped
        && actorServer->HasCommand()==command && rng.Get()==GlobalGameRandom().Get(),"AI hit prediction changed native equipment, ammo, gameplay state or RNG");
    Require(actual==expected,"AI hit prediction used the target's current firearm instead of the shooter's own planning hand");
}

static void AIToHitTargetValidity(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    for(const auto& server:units)if(IsValid(server) && server->CanFight() && IsValid(NAI::GetAIUnit(server)))NAI::GetAIUnit(server)->Synchronize(false);
    CObj<NAI::IAIUnit> actor,target;int baseline=0;
    for(const auto& server:units) {
        if(!IsValid(server) || !server->CanFight() || server->HasCommand() || !server->IsAIUnit() || server->IsWearingPK() || server->IsEmptyPK() || server->animator.GetCannon())continue;
        auto* shooter=NAI::GetAIUnit(server);
        if(!IsValid(shooter) || !IsValid(shooter->GetAIInventory()) || !IsValid(shooter->GetAIInventory()->GetCurrentFireArms()))continue;
        for(const auto& other:units) {
            if(!IsValid(other) || other.GetPtr()==server.GetPtr() || !other->CanFight() || other->IsEmptyPK() || other->IsWearingPK())continue;
            auto* victim=NAI::GetAIUnit(other);if(!IsValid(victim))continue;
            const int score=shooter->GetToHit(victim,shooter->GetUnitPosition(),NAI::HL_ANY);
            if(score>0){actor=shooter;target=victim;baseline=score;break;}
        }
        if(actor)break;
    }
    Require(actor && target && baseline>0,"target-validity fixture has no independently positive native armed hit prediction");
    CPtr<NWorld::CUnitServer> actorServer=actor->GetUnitServer(),targetServer=target->GetUnitServer();
    auto* inventory=actor->GetAIInventory();CObj<NAI::IAIInventoryItem> held=inventory->GetCurrentItem();
    struct Restore{NAI::CAIInventory* inventory;CObj<NAI::IAIInventoryItem> held;~Restore(){inventory->SetCurrentItem(held);}}restore{inventory,held};
    const bool empty=mode==2 || mode==4,invalid=mode==1 || mode==4,valid=mode==3;
    if(empty)inventory->SetCurrentItem(nullptr);
    if(invalid){target->Invalidate();Require(!IsValid(target) && target.GetPtr()!=nullptr,"target-validity fixture did not retain an actually invalid native AI wrapper");}
    NAI::IAIUnit* argument=invalid || valid?target.GetPtr():nullptr;
    const auto actorPosition=actorServer->GetPosition(),targetPosition=targetServer->GetPosition();
    const int ap=actorServer->GetAP(),hp=actorServer->GetUnitRPG()->GetTotalVP(),targetHP=targetServer->GetUnitRPG()->GetTotalVP();
    auto* equipped=actorServer->GetUnitRPG()->GetInventory()->GetActive();const bool command=actorServer->HasCommand();
    GlobalGameRandom().SeedForHarness(0x77da);auto rng=GlobalGameRandom();
    std::printf("native AI target validity entry: mode=%d empty=%d invalid=%d valid=%d baseline=%d\n",mode,int(empty),int(invalid),int(valid),baseline);std::fflush(stdout);
    const int actual=actor->GetToHit(argument,actor->GetUnitPosition(),NAI::HL_ANY),expected=valid?baseline:0;
    std::printf("native AI target validity result: mode=%d actual=%d expected=%d\n",mode,actual,expected);std::fflush(stdout);
    Require(actorServer->GetPosition()==actorPosition && targetServer->GetPosition()==targetPosition && actorServer->GetAP()==ap
        && actorServer->GetUnitRPG()->GetTotalVP()==hp && targetServer->GetUnitRPG()->GetTotalVP()==targetHP
        && actorServer->GetUnitRPG()->GetInventory()->GetActive()==equipped && actorServer->HasCommand()==command
        && rng.Get()==GlobalGameRandom().Get(),"AI target prediction changed native gameplay, equipment or RNG");
    Require(actual==expected,"AI hit prediction evaluated a missing or invalid target instead of returning zero");
}

static void SetToHitProbeServer(NAI::IAIUnit* unit,NWorld::CUnitServer* server)
{
    CStructureNetworkContext refs;auto original=BorrowedObjectFields(unit,&refs);
    if(server && !refs.ids.count(server)){const auto id=refs.nextID++;refs.ids[server]=id;refs.refs[id]=server;}
    CPtr<NWorld::CUnitServer> value=server;CMemoryStream fields;
    {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);writer.Add(3,&value);}
    fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ,&refs);unit->operator&(reader);}
}
static void AIToHitServerValidity(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    for(const auto& server:units)if(IsValid(server) && server->CanFight() && IsValid(NAI::GetAIUnit(server)))NAI::GetAIUnit(server)->Synchronize(false);
    CObj<NAI::IAIUnit> actor,target;int baseline=0;
    for(const auto& server:units) {
        if(!IsValid(server) || !server->CanFight() || server->HasCommand() || !server->IsAIUnit() || server->IsWearingPK() || server->IsEmptyPK() || server->animator.GetCannon())continue;
        auto* shooter=NAI::GetAIUnit(server);
        if(!IsValid(shooter) || !IsValid(shooter->GetAIInventory()) || !IsValid(shooter->GetAIInventory()->GetCurrentFireArms()))continue;
        for(const auto& other:units) {
            if(!IsValid(other) || other.GetPtr()==server.GetPtr() || !other->CanFight() || other->IsEmptyPK() || other->IsWearingPK())continue;
            auto* victim=NAI::GetAIUnit(other);if(!IsValid(victim))continue;
            const int score=shooter->GetToHit(victim,shooter->GetUnitPosition(),NAI::HL_ANY);
            if(score>0){actor=shooter;target=victim;baseline=score;break;}
        }
        if(actor)break;
    }
    Require(actor && target && baseline>0,"server-validity fixture has no independently positive native hit prediction");
    CPtr<NWorld::CUnitServer> originalServer=actor->GetUnitServer(),targetServer=target->GetUnitServer();
    auto* inventory=actor->GetAIInventory();CObj<NAI::IAIInventoryItem> held=inventory->GetCurrentItem();
    const auto position=actor->GetUnitPosition(),targetPosition=targetServer->GetPosition();
    struct Restore{NAI::IAIUnit* unit;CPtr<NWorld::CUnitServer> server;NAI::CAIInventory* inventory;CObj<NAI::IAIInventoryItem> held;
        ~Restore(){SetToHitProbeServer(unit,server);inventory->SetCurrentItem(held);}}restore{actor,originalServer,inventory,held};
    const bool empty=mode>=3,invalid=mode==1 || mode==4,valid=mode==2;
    if(empty)inventory->SetCurrentItem(nullptr);
    CObj<NWorld::CUnitServer> tombstone;
    if(invalid){tombstone=new NWorld::CUnitServer;SetToHitProbeServer(actor,tombstone);tombstone->Invalidate();}
    else if(!valid)SetToHitProbeServer(actor,nullptr);
    Require(IsValid(actor) && IsValid(target) && actor->GetAIInventory()==inventory
        && actor->GetUnitServer()==(invalid?tombstone.GetPtr():valid?originalServer.GetPtr():nullptr)
        && IsValid(actor->GetUnitServer())==valid && (empty?!IsValid(inventory->GetCurrentFireArms()):IsValid(inventory->GetCurrentFireArms())),
        "server-validity fixture changed another field or failed to create its controlled missing/ref-invalid server reference");
    const int ap=originalServer->GetAP(),hp=originalServer->GetUnitRPG()->GetTotalVP(),targetHP=targetServer->GetUnitRPG()->GetTotalVP();
    auto* equipped=originalServer->GetUnitRPG()->GetInventory()->GetActive();const bool command=originalServer->HasCommand();
    GlobalGameRandom().SeedForHarness(0x78da);auto rng=GlobalGameRandom();
    std::printf("native AI server validity entry: mode=%d empty=%d invalid=%d valid=%d baseline=%d\n",mode,int(empty),int(invalid),int(valid),baseline);std::fflush(stdout);
    // The supplied planning pose is captured before the controlled reference
    // restore. Any failure must come from the query, not argument evaluation.
    const int actual=actor->GetToHit(target,position,NAI::HL_ANY),expected=valid?baseline:0;
    std::printf("native AI server validity result: mode=%d actual=%d expected=%d\n",mode,actual,expected);std::fflush(stdout);
    Require(originalServer->GetPosition()==position && targetServer->GetPosition()==targetPosition && originalServer->GetAP()==ap
        && originalServer->GetUnitRPG()->GetTotalVP()==hp && targetServer->GetUnitRPG()->GetTotalVP()==targetHP
        && originalServer->GetUnitRPG()->GetInventory()->GetActive()==equipped && originalServer->HasCommand()==command
        && rng.Get()==GlobalGameRandom().Get(),"AI server-validity query changed native gameplay, equipment or RNG");
    Require(actual==expected,"AI hit prediction used a missing or ref-invalid shooter server instead of returning zero");
    SetToHitProbeServer(actor,originalServer);
    Require(actor->GetUnitServer()==originalServer.GetPtr() && actor->GetAIInventory()==inventory,"server-validity fixture did not restore native wrapper ownership");
}

static CObj<NWorld::CWorld> SnipeFixtureWorld(const char* databasePath, const char* resourcePath,
    bool deployParty = false, CObj<NRPG::CGlobalGame>* retainedGame = nullptr)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    if (retainedGame) *retainedGame = game;
    if (deployParty) game->players.push_back(NRPG::CreateGlobalPlayer(std::vector<int>{54}));
    CObj<NWorld::CWorld> world = new NWorld::CWorld(game);
    CObj<NWorld::CPostWorldCreateInfo> post;
    world->CreateRandom(3791, {}, true, {}, 0, &post, SRandomSeed(123));
    Require(post && world->GetPathNetwork(), "mission fixture construction failed");
    if (deployParty) {
        CObj<NAI::CSequenceCommander> commander = new NAI::CSequenceCommander(world);
        auto* player = dynamic_cast<NWorld::CPlayer*>(world->AddPlayer(L"Clue fixture", game->players.front(), commander));
        Require(player != nullptr, "human party deployment failed");
        commander->SetPlayer(player);
    }
    // Deliberately exclude mission scripts: this fixture isolates AI scheduling.
    world->RunPostInit(nullptr);
    return world;
}

static std::pair<int,int> NativeSnipeEscapeVisibility(NAI::IAIUnit* actor,NAI::IAIUnit* target,int cost)
{
    auto* world=actor->GetUnitServer()->GetWorld();auto* game=world->GetGame();auto* network=world->GetPathNetwork();
    const auto pose=actor->GetUnitPosition();const float range=game->GetUnitSightDistance(actor->GetRPGUnit()),fov=actor->GetRPGUnit()->GetSightFOV();
    vector<NAI::SPathPlace> places;NAI::GetNearestPlaces(target->GetUnitServer(),target->GetPosition().p,cost,NAI::RUN,&places);
    int blocked=0;for(const auto& point:places)if(!game->CheckPositionVisibility(pose,NAI::SPosition(point,network),range,fov))++blocked;
    return {int(places.size()),blocked};
}
static bool PlaceNativeVisibleEscapeFixture(NAI::IAIUnit* actor,NAI::IAIUnit* target,
    NAI::CAIFireArmsWeapon* weapon,NAI::CAIBeginSnipeAction* action)
{
    auto* server=actor->GetUnitServer();auto* victim=target->GetUnitServer();auto* world=server->GetWorld();
    auto* network=world->GetPathNetwork();auto* game=world->GetGame();
    const auto original=server->GetPosition(),targetOriginal=victim->GetPosition();
    vector<NAI::SPathPlace> anchors;NAI::GetNearestPlaces(victim,targetOriginal.pos.p,target->GetMaxAP(),NAI::RUN,&anchors);
    const float range=game->GetUnitSightDistance(actor->GetRPGUnit()),fov=actor->GetRPGUnit()->GetSightFOV();
    auto place=[&](NWorld::CUnitServer* unit,const NAI::SUnitPosition& position) {
        unit->SetPosition(position);unit->animator.AlignTime();unit->animator.PlaceUnit(position);
        MarkNewDGFrame();world->GetAIMap()->Sync();
    };
    for(int facing=0;facing<8;++facing) {
        auto turned=original;turned.pos.p.SetDirection(facing);place(server,turned);
        for(auto anchor:anchors) {
            if(!network->IsPassable(anchor) || network->IsLocked(anchor))continue;
            NAI::SPosition position(anchor,network);
            if(fabs(position.GetCP()-original.GetCP())*1.6f<15.f
                || !game->CheckPositionVisibility(server->GetPosition(),position,range,fov))continue;
            vector<NAI::SPathPlace> four;NAI::GetNearestPlaces(victim,anchor,4,NAI::RUN,&four);
            if(four.empty())continue;
            bool visible=true;for(const auto& point:four)if(!game->CheckPositionVisibility(server->GetPosition(),NAI::SPosition(point,network),range,fov)){visible=false;break;}
            if(!visible)continue;
            auto moved=targetOriginal;moved.pos.p=anchor;place(victim,moved);
            NAI::CAIBeginSnipeAction::SInfo info;action->GetInfoInner(NAI::SPlaceWithAP(server->GetPosition(),actor->GetAP()),&info);
            const auto actualFour=NativeSnipeEscapeVisibility(actor,target,4),full=NativeSnipeEscapeVisibility(actor,target,target->GetMaxAP());
            if(info.hitLocation==NAI::HL_HEAD && info.pWeapon==weapon && actualFour.first>0 && actualFour.second==0 && full.second>0
                && actor->GetToHitWithWeapon(target,actor->GetUnitPosition(),NAI::HL_HEAD,weapon)>0) {
                std::printf("native escape placement: direction=%d distance=%.2f four=%d blocked4=%d full=%d blockedFull=%d\n",
                    facing,fabs(server->GetPosition().GetCP()-victim->GetPosition().GetCP()),actualFour.first,actualFour.second,full.first,full.second);std::fflush(stdout);
                return true;
            }
            place(victim,targetOriginal);
        }
    }
    place(victim,targetOriginal);place(server,original);return false;
}
// Audit an upstream assignment against the fork's actual reachable poses.
// This is an equivalence check, not a manufactured RED for unchanged behavior.
static void NativeSnipeFacingParity(NAI::IAIUnit* actor,NAI::IAIUnit* target,bool canSnipe)
{
    auto* world=actor->GetUnitServer()->GetWorld();auto* network=world->GetPathNetwork();auto* game=world->GetGame();
    const auto source=actor->GetUnitPosition(),destination=target->GetUnitPosition();
    const int ap=actor->GetAP(),targetAP=target->GetAP(),hp=actor->GetRPGUnit()->Skills(NDb::ST_VP),targetHP=target->GetRPGUnit()->Skills(NDb::ST_VP);
    auto rng=GlobalGameRandom();const float range=game->GetUnitSightDistance(actor->GetRPGUnit()),fov=actor->GetRPGUnit()->GetSightFOV();
    const auto closest=network->GetClosestDir(source.pos.p,destination.pos.p);
    int total=0,poseMask=0,changed=0,alternatives=0,visibility=0,hidden=0,fourCount=0;
    for(int budget:{4,target->GetMaxAP()}) {
        vector<NAI::SPathPlace> places;NAI::GetNearestPlaces(target->GetUnitServer(),destination.pos.p,budget,NAI::RUN,&places);
        Require(!places.empty(),"facing audit lost its nonempty actual native escape flood");
        if(budget==4)fourCount=int(places.size());
        for(auto point:places) {
            const auto pose=point.GetPose();Require(pose==NAI::CM_CROUCH || pose==NAI::CM_STAND,"native escape flood gained a direction-sensitive or inactive pose");
            poseMask|=1<<pose;NAI::SPosition baseline(point,network);vector<CVec3> cubes;NRPG::GetOccupiedCubes(&cubes,baseline);
            Require(!cubes.empty(),"facing audit has no actual occupied target cubes");
            const bool seen=budget==4?game->CheckPositionVisibility(source,baseline,range,fov):false;
            if(budget==4 && !seen)++hidden;
            if(point.GetDirection()!=closest)++changed;
            for(int direction=0;direction<8;++direction) {
                if(direction!=point.GetDirection())++alternatives;
                auto candidate=baseline;candidate.p.SetDirection(direction);vector<CVec3> actual;NRPG::GetOccupiedCubes(&actual,candidate);
                Require(candidate.GetCP()==baseline.GetCP() && actual==cubes,"target escape facing changed native coordinates or occupied cubes");
                if(budget==4) {
                    Require(game->CheckPositionVisibility(source,candidate,range,fov)==seen,"target escape facing changed the native snipe visibility decision");++visibility;
                }
                ++total;
            }
        }
    }
    std::printf("native snipe facing parity: can=%d four=%d hidden=%d directions=8 poseMask=%d comparisons=%d visibility=%d alternateDirections=%d originalDirectionChanges=%d\n",
        int(canSnipe),fourCount,hidden,poseMask,total,visibility,alternatives,changed);std::fflush(stdout);
    Require(total>8 && alternatives>0 && visibility==fourCount*8 && canSnipe==(hidden==0),"native facing audit did not exercise real changed direction bits and the expected positive/blocked decision");
    Require(actor->GetUnitPosition()==source && target->GetUnitPosition()==destination && actor->GetAP()==ap && target->GetAP()==targetAP
        && int(actor->GetRPGUnit()->Skills(NDb::ST_VP))==hp && int(target->GetRPGUnit()->Skills(NDb::ST_VP))==targetHP
        && rng.Get()==GlobalGameRandom().Get(),"native facing equivalence queries changed position, health, AP or RNG");
}
static void AISnipeSelectedWeapon(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> campaign;auto world=SnipeFixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NDb::CRPGWeapon* rifle=nullptr;CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while(weapons.MoveNext()) {
        auto* record=weapons.Get();
        if(record->eWeaponType==NDb::WT_RIFLE && record->shootModes[NDb::SM_Snipe] && record->shootModes[NDb::SM_Snap]
            && !record->bBazookaLogic && record->pItem && record->pInnerClip && record->pInnerClip->nQuantity>0){rifle=record;break;}
    }
    Require(rifle,"selected-snipe fixture has no authored loaded sniper rifle");
    CObj<NAI::IAIUnit> actor,target;CPtr<NAI::CAIFireArmsWeapon> selected;NAI::CAIBeginSnipeAction::SInfo baseline;
    int selectedHead=0;float distance=0;
    const bool visibleEscape=mode==3 || mode==5 || mode==7;
    for(const auto& server:units) {
        if(!IsValid(server) || !server->CanFight() || server->HasCommand() || server->IsSniping() || !server->IsAIUnit()
            || server->IsEmptyPK() || server->IsWearingPK() || server->animator.GetCannon())continue;
        auto* shooter=NAI::GetAIUnit(server);if(!IsValid(shooter))continue;
        CObj<NRPG::CWeaponItem> item=new NRPG::CWeaponItem(rifle);auto* equipment=server->GetUnitRPG()->GetInventory();
        if(!equipment->Equip(NDb::SLOT_1,item) || !equipment->Activate(NDb::SLOT_1) || !item->SetShootMode(NDb::SM_Snap))continue;
        shooter->Synchronize(false);auto* inventory=shooter->GetAIInventory();auto* choice=inventory->GetBestWeaponForSnipe();
        if(!IsValid(choice) || !IsValid(choice->GetCurrentClip()) || choice->GetCurrentClip()->IsEmpty())continue;
        auto* state=shooter->GetAIUnitState();if(!state)continue;
        CPtr<NAI::IAIUnit> oldEnemy=state->pEnemy,oldPossible=state->pPossibleEnemy;
        struct RestoreState{NAI::SAIUnitState* state;CPtr<NAI::IAIUnit> enemy,possible;~RestoreState(){state->pEnemy=enemy;state->pPossibleEnemy=possible;}}restore{state,oldEnemy,oldPossible};
        CObj<NAI::CAIBeginSnipeAction> action=new NAI::CAIBeginSnipeAction(shooter);
        for(const auto& other:units) {
            if(!IsValid(other) || other.GetPtr()==server.GetPtr() || !other->CanFight() || other->IsEmptyPK() || other->IsWearingPK()
                || server->GetDiplomacyState(other)!=NDb::DS_ENEMY)continue;
            auto* victim=NAI::GetAIUnit(other);if(!IsValid(victim))continue;
            float range=fabs(server->GetPosition().GetCP()-other->GetPosition().GetCP());
            if(range*1.6f<15.f)continue;
            state->pEnemy=victim;state->pPossibleEnemy=nullptr;
            if(visibleEscape && !PlaceNativeVisibleEscapeFixture(shooter,victim,choice,action))continue;
            range=fabs(server->GetPosition().GetCP()-other->GetPosition().GetCP());
            const auto pose=shooter->GetUnitPosition();
            const int cover=shooter->GetCoverForFixedUnit(pose,other,choice->GetItem(),NAI::HL_HEAD);
            CObj<NRPG::CAIUnitToHitCalcer> reference=new NRPG::CAIUnitToHitCalcer(shooter,pose,victim,cover,NAI::HL_HEAD,0,choice->GetItem(),0);
            const int expectedHead=reference->GetToHit();NAI::CAIBeginSnipeAction::SInfo info;
            action->GetInfoInner(NAI::SPlaceWithAP(server->GetPosition(),shooter->GetAP()),&info);
            std::printf("native snipe calibration: distance=%.2f selectedHead=%d location=%d can=%d\n",range,expectedHead,int(info.hitLocation),int(info.bCanDo));std::fflush(stdout);
            if(mode>=3 && expectedHead>0 && info.hitLocation==NAI::HL_HEAD && info.pWeapon==choice) {
                const auto four=NativeSnipeEscapeVisibility(shooter,victim,4),full=NativeSnipeEscapeVisibility(shooter,victim,victim->GetMaxAP());
                std::printf("native escape calibration: mode=%d distance=%.2f four=%d blocked4=%d full=%d blockedFull=%d maxAP=%d\n",
                    mode,range,four.first,four.second,full.first,full.second,victim->GetMaxAP());std::fflush(stdout);
                if(four.first==0 || (visibleEscape?four.second!=0 || full.second==0:four.second==0))continue;
            }
            if(expectedHead>0 && info.hitLocation==NAI::HL_HEAD && info.pWeapon==choice) {
                actor=shooter;target=victim;selected=choice;baseline=info;selectedHead=expectedHead;distance=range;break;
            }
        }
        if(actor)break;
    }
    Require(actor && target && selected && selectedHead>0 && baseline.hitLocation==NAI::HL_HEAD,
        "selected-snipe fixture has no native distant enemy with independently positive selected-rifle head prediction and reached head-choice stage");
    auto* inventory=actor->GetAIInventory();auto* state=actor->GetAIUnitState();CObj<NAI::IAIInventoryItem> current=inventory->GetCurrentItem();
    struct Restore{NAI::CAIInventory* inventory;CObj<NAI::IAIInventoryItem> current;NAI::SAIUnitState* state;CPtr<NAI::IAIUnit> enemy,possible;
        vector<std::pair<CPtr<NAI::CAIFireArmsWeaponClip>,int>> clips;
        ~Restore(){for(auto& entry:clips)entry.first->SetAmmoCount(entry.second);inventory->SetCurrentItem(current);state->pEnemy=enemy;state->pPossibleEnemy=possible;}
    }restore{inventory,current,state,state->pEnemy,state->pPossibleEnemy,{}};
    state->pEnemy=target;state->pPossibleEnemy=nullptr;
    if(mode==0 || mode==2)inventory->SetCurrentItem(nullptr);
    if(mode==2) {
        while(auto* weapon=inventory->GetBestWeaponForSnipe()) {
            auto* clip=weapon->GetCurrentClip();Require(clip && clip->GetAmmoCount()>0 && restore.clips.size()<128,"snipe ammo control did not advance through native planning clips");
            restore.clips.push_back({clip,clip->GetAmmoCount()});clip->SetAmmoCount(0);
        }
    } else Require(inventory->GetBestWeaponForSnipe()==selected.GetPtr(),"empty planning hand removed the actual selected sniper rifle");
    auto* server=actor->GetUnitServer();auto* victim=target->GetUnitServer();const auto position=server->GetPosition(),targetPosition=victim->GetPosition();
    auto* equipped=server->GetUnitRPG()->GetInventory()->GetActive();const int ap=server->GetAP(),hp=server->GetUnitRPG()->GetTotalVP(),targetHP=victim->GetUnitRPG()->GetTotalVP();
    const int ammo=selected->GetItem()->GetAmmoQuantity(),planningAmmo=selected->GetCurrentClip()->GetAmmoCount();const bool command=server->HasCommand();
    auto* planningHand=inventory->GetCurrentItem();GlobalGameRandom().SeedForHarness(0x79da);auto rng=GlobalGameRandom();
    CObj<NAI::CAIBeginSnipeAction> action=new NAI::CAIBeginSnipeAction(actor);NAI::CAIBeginSnipeAction::SInfo info;
    action->GetInfoInner(NAI::SPlaceWithAP(position,actor->GetAP()),&info);
    std::printf("native selected snipe: mode=%d emptyHand=%d noAmmo=%d distance=%.2f selectedHead=%d location=%d baselineLocation=%d can=%d baselineCan=%d selected=%d planningAmmo=%d\n",
        mode,int(mode==0 || mode==2),int(mode==2),distance,selectedHead,int(info.hitLocation),int(baseline.hitLocation),int(info.bCanDo),int(baseline.bCanDo),int(info.pWeapon==selected),planningAmmo);std::fflush(stdout);
    Require(server->GetPosition()==position && victim->GetPosition()==targetPosition && server->GetAP()==ap && server->GetUnitRPG()->GetTotalVP()==hp
        && victim->GetUnitRPG()->GetTotalVP()==targetHP && server->GetUnitRPG()->GetInventory()->GetActive()==equipped
        && selected->GetItem()->GetAmmoQuantity()==ammo && selected->GetCurrentClip()->GetAmmoCount()==planningAmmo
        && inventory->GetCurrentItem()==planningHand && server->HasCommand()==command && rng.Get()==GlobalGameRandom().Get(),
        "snipe prediction changed native equipment/ammo/AP/HP/positions/commands, planning hand/clip or RNG");
    if (mode == 7)
    {
        Require(info.bCanDo, "snipe fixture lacks a visible target from the current firing position");
        auto* net = world->GetPathNetwork();
        auto* game = world->GetGame();
        vector<NRPG::CAttackPortion> attacks;
        selected->GetItem()->CreateNewAttackPortion(&attacks, false);
        Require(!attacks.empty(), "snipe fixture lacks a real attack");
        vector<NAI::SPathPlace> nearby;
        net->GetNearPlaces(SSphere(position.GetCP(), 7.f), &nearby);
        bool foundBlockedPose = false;
        for (auto p : nearby)
        {
            if (!p.IsIntegral() || p.IsFinal() || p.GetLayer() != position.pos.p.GetLayer())
                continue;
            p.SetPose(NAI::CM_CROUCH);
            p.SetDirection(net->GetClosestDir(p, targetPosition.pos.p));
            if (net->GetPassability(p) != NAI::AIP_YES || fabs2(NAI::SPosition(p, net).GetCP() - position.GetCP()) < 1.f)
                continue;
            auto proposed = position;
            proposed.pos.p = p;
            proposed.bRun = false;
            const float cover = game->GetCoverForAIUnitPrecise(server->GetAttackOrigin(proposed, false),
                server, victim, attacks.front(), NAI::HL_ANY);
            if (cover > 0)
                continue;
            Require(actor->GetToHitWithWeapon(target, proposed, NAI::HL_ANY, selected) == 0,
                "AI shot calculation ignored the fully blocked proposed position");
            NAI::CAIBeginSnipeAction::SInfo blocked;
            action->GetInfoInner(NAI::SPlaceWithAP(proposed, actor->GetAP()), &blocked);
            Require(!blocked.bCanDo && server->GetPosition() == position,
                "AI chose the blocked firing position by testing its current position");
            foundBlockedPose = true;
            break;
        }
        Require(foundBlockedPose, "snipe fixture lacks a nearby crouching position behind real cover");

        world->GivePlayerTurn(server->GetTBSPlayer());
        server->GetUnitRPG()->StartNewTurn(position.GetCP());
        // Refresh the planner's AP without replacing the inventory used by the fixture.
        actor->SetAP(server->GetAP(), actor->GetMaxAP());
        state->pEnemy = target;
        state->pPossibleEnemy = nullptr;
        CDynamicCast<NRPG::CWeaponItem> liveGun(server->GetUnitRPG()->GetInventory()->GetActive());
        Require(liveGun && liveGun->SetShootMode(NDb::SM_Snipe), "unable to arm the real sniper rifle");
        const auto location = game->GetCompositeToHit(server, victim, NAI::HL_HEAD, false) > 0 ?
            NAI::HL_HEAD : NAI::HL_ANY;
        CObj<NWorld::CUnitStateSniping> aiming = new NWorld::CUnitStateSniping(server, victim, 12, location);
        server->SetState(aiming);
        CObj<NAI::CAISnipeShotAction> shot = new NAI::CAISnipeShotAction(actor);
        CObj<NAI::CAICollectSnipeAPAction> collect = new NAI::CAICollectSnipeAPAction(actor);
        CObj<NAI::CAICancelSnipeAction> cancel = new NAI::CAICancelSnipeAction(actor);
        const NAI::SPlaceWithAP here(position, actor->GetAP());
        NAI::CAISnipeShotAction::SInfo shotInfo;
        NAI::CAICollectSnipeAPAction::SInfo collectInfo;
        NAI::CAICancelSnipeAction::SInfo cancelInfo;
        shot->GetInfoInner(here, &shotInfo);
        collect->GetInfoInner(here, &collectInfo);
        cancel->GetInfoInner(here, &cancelInfo);
        std::printf("stored snipe: location=%d chance=%d AP=%d shot=%d collect=%d cancel=%d\n",
            int(location), game->GetCompositeToHit(server, victim, location, false), actor->GetAP(),
            int(shotInfo.bCanDo), int(collectInfo.bCanDo), int(cancelInfo.bCanDo));
        Require(shotInfo.bCanDo && collectInfo.bCanDo && !cancelInfo.bCanDo,
            "the sniper could not continue aiming at a live, hittable target");
        victim->Die();
        Require(victim->IsDead() && server->IsSniping(), "the stored target did not die while aim remained active");
        shot->GetInfoInner(here, &shotInfo);
        collect->GetInfoInner(here, &collectInfo);
        cancel->GetInfoInner(here, &cancelInfo);
        Require(!shotInfo.bCanDo && !collectInfo.bCanDo && cancelInfo.bCanDo,
            "the sniper kept planning shots or saving AP for a dead target");
        return;
    }
    if(mode>=3) {
        const auto four=NativeSnipeEscapeVisibility(actor,target,4),full=NativeSnipeEscapeVisibility(actor,target,target->GetMaxAP());
        std::printf("native snipe escape: mode=%d four=%d blocked4=%d full=%d blockedFull=%d maxAP=%d can=%d expected=%d location=%d\n",
            mode,four.first,four.second,full.first,full.second,target->GetMaxAP(),int(info.bCanDo),int(four.second==0),int(info.hitLocation));std::fflush(stdout);
        Require(four.first>0 && info.pWeapon==selected && info.hitLocation==NAI::HL_HEAD,"escape fixture did not reach its native visibility decision with a positive selected rifle");
        Require(visibleEscape?four.second==0 && full.second>0:four.second>0,"escape fixture did not separate four-AP visibility from a genuinely blocked control");
        Require(info.bCanDo==(four.second==0),"AI snipe eligibility scanned beyond the original four-AP escape budget");
        if(mode>=5)NativeSnipeFacingParity(actor,target,info.bCanDo);
        return;
    }
    if(mode==2)Require(!info.bCanDo && !IsValid(info.pWeapon) && info.hitLocation==NAI::HL_ANY,"snipe selection ignored empty native planning magazines");
    else Require(info.pWeapon==selected && info.hitLocation==NAI::HL_HEAD && info.bCanDo==baseline.bCanDo && info.pTarget==baseline.pTarget,
        "AI snipe prediction ignored the selected loaded rifle when its current planning hand was empty");
}

// Native calculator references differ only in the supplied cover input.
// Geometry, equipment, skills and the hit formula all remain the real game's.
static void AICoverPrecision(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> campaign;auto world=SnipeFixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);auto* game=world->GetGame();
    for(const auto& server:units) {
        if(!IsValid(server) || !server->CanFight() || server->HasCommand() || !server->IsAIUnit()
            || server->IsEmptyPK() || server->IsWearingPK() || server->animator.GetCannon())continue;
        auto* actor=NAI::GetAIUnit(server);if(!IsValid(actor))continue;actor->Synchronize(false);
        auto* inventory=actor->GetAIInventory();auto* weapon=inventory?inventory->GetCurrentFireArms():nullptr;
        if(!IsValid(weapon) || !IsValid(weapon->GetItem()) || weapon->GetItem()->GetAmmoQuantity()<=0
            || !weapon->GetItem()->SetShootMode(NDb::SM_Snap))continue;
        vector<NRPG::CAttackPortion> attacks;weapon->GetItem()->CreateNewAttackPortion(&attacks,false);
        if(attacks.empty() || attacks.front().nK<=0)continue;
        for(const auto& other:units) {
            if(!IsValid(other) || other.GetPtr()==server.GetPtr() || !other->CanFight() || other->IsEmptyPK() || other->IsWearingPK())continue;
            auto* target=NAI::GetAIUnit(other);if(!IsValid(target))continue;
            for(auto location:{NAI::HL_ANY,NAI::HL_BODY,NAI::HL_HEAD}) {
                const auto position=server->GetPosition(),targetPosition=other->GetPosition();
                float precise=mode==1?0.5f:mode==2?37.f:mode==3?0.f:100.f;int legacy=int(precise),rays=0,penetrating=0;
                if(mode==0 || mode==4) {
                    CObj<NRPG::CCoverInfo> cover=game->CalcCovers(server->GetAttackOrigin(position,false),attacks.front(),server,other,location,1.f,true);
                    const auto snapshot=CoverSnapshot(cover);rays=int(snapshot.hits.size());
                    for(const auto& ray:snapshot.hits)if(ray.penetrate)++penetrating;
                    if(rays==0 || penetrating==0)continue;
                    // Original floating ratio: no integer conversion before the caller's boundary.
                    const float average=(snapshot.piercing/float(penetrating))/attacks.front().nK;
                    precise=(100.f*penetrating/float(rays))*average;
                    if(!std::isfinite(precise) || precise<=0 || precise>=100.f)continue;
                    legacy=int(int(100*penetrating/rays)*average);
                    if(mode==0 && int(precise)==legacy)continue;
                }
                auto calculator=[&](int cover){return CObj<NRPG::CAIUnitToHitCalcer>(new NRPG::CAIUnitToHitCalcer(actor,position,target,cover,location,0,weapon->GetItem(),0));};
                auto reference=calculator(0);reference->fHitCover=precise;const int expectedHit=reference->GetToHit();
                auto old=calculator(legacy);const int legacyHit=old->GetToHit();
                if(mode!=0 && mode!=3 && expectedHit<=0)continue;
                if(mode==4 && expectedHit==legacyHit)continue;
                const int ap=server->GetAP(),hp=server->GetUnitRPG()->GetTotalVP(),targetHP=other->GetUnitRPG()->GetTotalVP();
                const int ammo=weapon->GetItem()->GetAmmoQuantity();auto* equipped=server->GetUnitRPG()->GetInventory()->GetActive();
                const bool command=server->HasCommand();GlobalGameRandom().SeedForHarness(0x7ada);auto rng=GlobalGameRandom();
                int actual=0,expected=mode==0?int(precise):expectedHit;
                if(mode==0)actual=game->GetCoverForAIUnit(server->GetAttackOrigin(position,false),server,other,attacks.front(),location);
                else if(mode==4)actual=actor->GetToHitWithWeapon(target,position,location,weapon);
                else {
                    CObj<NRPG::CAIUnitToHitCalcer> query=new NRPG::CAIUnitToHitCalcer(actor,position,target,precise,location,0,weapon->GetItem(),0);
                    actual=query->GetToHit();
                }
                std::printf("native AI cover precision: mode=%d location=%d distance=%.2f rays=%d penetrating=%d precise=%.7f legacy=%d legacyHit=%d actual=%d expected=%d\n",
                    mode,int(location),fabs(position.GetCP()-targetPosition.GetCP()),rays,penetrating,precise,legacy,legacyHit,actual,expected);std::fflush(stdout);
                Require(server->GetPosition()==position && other->GetPosition()==targetPosition && server->GetAP()==ap && server->GetUnitRPG()->GetTotalVP()==hp
                    && other->GetUnitRPG()->GetTotalVP()==targetHP && weapon->GetItem()->GetAmmoQuantity()==ammo
                    && server->GetUnitRPG()->GetInventory()->GetActive()==equipped && server->HasCommand()==command && rng.Get()==GlobalGameRandom().Get(),
                    "fractional cover query changed native gameplay, equipment or RNG");
                Require(actual==expected,"AI cover lost fractional precision before the original hit calculation or caller conversion");return;
            }
        }
    }
    Require(false,"fractional cover fixture found no native geometry/weapon case separating the original floating calculation from early truncation");
}

static void AIFirstIncapacitation(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> campaign;auto world=SnipeFixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    CObj<NWorld::CUnitServer> target,attacker,observer;CObj<NAI::IAIUnit> targetAI,attackerAI,observerAI;
    auto eligible=[](NWorld::CUnitServer* unit){return IsValid(unit) && unit->CanFight() && !unit->HasCommand()
        && !unit->IsEmptyPK() && !unit->IsWearingPK() && !unit->animator.GetCannon() && unit->GetUnitRPG()->GetTotalVP()>16 && IsValid(NAI::GetAIUnit(unit));};
    for(const auto& victim:units) {
        if(!eligible(victim))continue;
        for(const auto& shooter:units) {
            if(!eligible(shooter) || shooter==victim || victim->GetDiplomacyState(shooter)!=NDb::DS_ENEMY)continue;
            for(const auto& watcher:units)if(eligible(watcher) && watcher!=victim && watcher!=shooter
                && watcher->GetDiplomacyState(shooter)==NDb::DS_ENEMY){target=victim.GetPtr();attacker=shooter.GetPtr();observer=watcher.GetPtr();break;}
            if(target)break;
        }
        if(target)break;
    }
    Require(target && attacker && observer,"incapacitation fixture has no actual enemy attacker and allied observer");
    targetAI=NAI::GetAIUnit(target);attackerAI=NAI::GetAIUnit(attacker);observerAI=NAI::GetAIUnit(observer);
    Require(!target->pKiller && target->GetDeathTime()==0 && target->CanFight(),"fresh native target already has corpse metadata or cannot fight");
    const bool knockout=mode==1 || mode==6,repeat=mode==5 || mode==6,blocked=mode==3,nonlethal=mode==2,anonymous=mode==4;
    const STime firstTime=1234,laterTime=2345;world->GetTime()->Set(firstTime);target->animator.AlignTime();
    auto hit=[&](NWorld::CUnitServer* source,bool ko,bool lethal,bool stop) {
        NRPG::CAttackPortion attack(stop?0:100000,0,0.f,lethal?100000:ko?8:1,lethal?100000:ko?8:1,0,0,
            source?source->GetUnitRPG():nullptr,target->GetUnitRPG(),1.f,ko?100:-1);
        attack.rTtrajectory.ptOrigin=source?source->GetPosition().GetCP():target->GetPosition().GetCP()+CVec3(0,0,1);
        attack.rTtrajectory.ptDir=CVec3(0,1,0);GlobalGameRandom().SeedForHarness(0x7bda);
        return target->ProcessAttack(world,NAI::HL_BODY,&attack,CVec3(0,1,0),target->GetUnitRPG()->GetRPGArmor());
    };
    if(knockout) {
        // Preserve the real enemy difficulty multiplier: fixed input8 deals5.
        // This metadata fixture needs an actual ordinary-hit KO, not a healthy
        // target whose knockout chance is suppressed by original health scaling.
        auto* mission=target->GetUnitRPG();auto& health=mission->GetRPGUnit()->Skills(NDb::ST_VP);
        NRPG::CAttackPortion probe(100000,0,0.f,8,8,0,0,attacker->GetUnitRPG(),mission,1.f,100);
        const int expectedDamage=probe.CalcStructDmg(world,NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR),0),before=health;
        Require(expectedDamage==5 && health.GetMaxValue()==80,"AI incapacitation native enemy damage/health cap changed");
        health.Modify(expectedDamage+1-int(health));
        Require(mission->GetTotalVP()==6 && !target->IsDead() && !target->IsUnconscious(),"AI incapacitation lacks its real wounded living target");
        std::printf("AI_KO_HEALTH mode=%d before=%d prepared=%d cap=%d expectedDamage=5 expectedRemaining=1\n",mode,before,mission->GetTotalVP(),health.GetMaxValue());std::fflush(stdout);
    }
    const int hp=target->GetUnitRPG()->GetTotalVP(),attackerHP=attacker->GetUnitRPG()->GetTotalVP(),observerHP=observer->GetUnitRPG()->GetTotalVP();
    const int damage=hit(anonymous?nullptr:attacker.GetPtr(),knockout,!knockout && !blocked && !nonlethal,blocked);
    if(blocked || nonlethal)Require(target->CanFight() && !target->GetUnitRPG()->IsDead() && !target->GetUnitRPG()->IsUnconscious()
        && (blocked?damage==0:damage>0),"incapacitation control accidentally downed the target or missed its intended native damage branch");
    else Require(IsValid(target) && (knockout?target->IsUnconscious() && !target->IsDead():target->IsDead())
        && damage>0 && (!knockout || (damage==5 && target->GetUnitRPG()->GetTotalVP()==1)),"native attack did not produce the intended death or unconscious transition");
    if(repeat) {
        // A loaded old corpse already has recorded metadata. This is setup for
        // the repeat-hit control, not evidence that the first-hit writer ran.
        target->pKiller=attacker.GetPtr();target->tDeathTime=firstTime;
        world->GetTime()->Set(laterTime);target->animator.AlignTime();hit(observer,false,knockout,false);
        Require(target->IsDead(),"repeat-hit control did not remain dead or finish its real unconscious target");
    }
    Require(attacker->GetUnitRPG()->GetTotalVP()==attackerHP && observer->GetUnitRPG()->GetTotalVP()==observerHP,
        "native single-target attack changed another unit's health");
    if(mode==7) {
        Require(IsValid(targetAI) && IsValid(attackerAI) && IsValid(observerAI),"real death invalidated the retained native AI contact fixture");
        NAI::SAIUnitState state;state.SetUnit(observerAI);CObj<NAI::IAIEvent> event=NAI::CreateAICorpseEvent(targetAI);
        Require(event,"native corpse event factory rejected the actual retained corpse contact");event->Modify(&state);
        const bool possible=state.possibleEnemies.data.IsContain(attackerAI,false);
        std::printf("native corpse killer event: known=%d possible=%d killer=%d deathTime=%u\n",int(state.IsKnownCorpse(targetAI)),int(possible),int(target->pKiller==attacker.GetPtr()),unsigned(target->GetDeathTime()));std::fflush(stdout);
        Require(state.IsKnownCorpse(targetAI),"actual corpse event did not record its contact");
        Require(possible,"actual corpse event could not promote the real attacker because the first hit did not record its killer");return;
    }
    const auto* expectedKiller=(blocked || nonlethal || anonymous)?nullptr:attacker.GetPtr();
    const STime expectedTime=(blocked || nonlethal)?0:firstTime;
    std::printf("native first incapacitation: mode=%d damage=%d HP=%d->%d dead=%d unconscious=%d killerMatches=%d expectedKiller=%d deathTime=%u expectedTime=%u repeat=%d\n",
        mode,damage,hp,target->GetUnitRPG()->GetTotalVP(),int(target->IsDead()),int(target->IsUnconscious()),int(target->pKiller.GetPtr()==expectedKiller),int(expectedKiller!=nullptr),unsigned(target->GetDeathTime()),unsigned(expectedTime),int(repeat));std::fflush(stdout);
    Require(target->pKiller.GetPtr()==expectedKiller && target->GetDeathTime()==expectedTime,
        "first native incapacitating hit lost its attacker/time or a later corpse hit replaced the original metadata");
}

static void ScenarioDiscovery(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    vector<CPtr<NWorld::CPlayer>> players;world->GetPlayersList(&players);NWorld::CPlayer* human=nullptr;
    for(const auto& player:players)if(IsValid(player) && !NAI::IsAIPlayer(player)){human=player;break;}
    Require(human && IsValid(campaign->pScenarioTracker),"scenario discovery fixture has no real human player/tracker");
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* hero=nullptr;
    for(const auto& unit:units)if(IsValid(unit) && unit->GetPlayer()==human && unit->CanFight()){hero=unit.GetPtr();break;}
    Require(hero,"scenario discovery fixture has no actual deployed party unit");
    NWorld::CUnitServer* carrier=nullptr;NRPG::IInventoryItem* item=nullptr;list<CPtr<NWorld::CUnit>> visible;
    for(int direction=0;direction<8 && !carrier;++direction) {
        auto position=hero->GetPosition();position.pos.p.SetDirection(direction);hero->SetPosition(position);hero->animator.AlignTime();hero->animator.PlaceUnit(position);
        MarkNewDGFrame();world->GetAIMap()->Sync();world->UpdateVisible(true);human->GetVisible(&visible);
        for(const auto& candidate:visible) {
            auto* unit=dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
            if(!unit || unit->GetPlayer()==human || !unit->CanFight() || unit->IsEmptyPK() || unit->IsWearingPK())continue;
            auto* active=unit->GetUnitRPG()->GetInventory()->GetActive();
            if(IsValid(active) && IsValid(active->GetDBItem())){carrier=unit;item=active;break;}
        }
    }
    Require(carrier && item,"scenario discovery fixture has no naturally visible NPC carrying a real authored item");
    NDb::CString* description=nullptr;CDBIterator<NDb::CScenarioTask> authored(*NDatabase::GetTable<NDb::CScenarioTask>());
    while(authored.MoveNext())if(IsValid(authored.Get()->pDescription)){description=authored.Get()->pDescription;break;}
    Require(description,"scenario discovery fixture has no actual authored task description");
    const bool unmatched=mode==3 || mode==4,latched=mode==4,carrierTask=mode==0 || unmatched || mode==6;
    int itemID=item->GetDBItem()->GetRecordID();
    if(unmatched) {
        std::set<int> present;
        auto inspect=[&](NRPG::IInventoryItem* entry){if(IsValid(entry) && IsValid(entry->GetDBItem()))present.insert(entry->GetDBItem()->GetRecordID());};
        for(const auto& unit:visible)if(IsValid(unit) && IsValid(unit->GetRPG())) {
            auto* inventory=unit->GetRPG()->GetRPGUnit()->pInventory.GetPtr();
            for(int slot=0;slot<NDb::N_SLOTS;++slot)inspect(inventory->Get(NDb::ESlot(slot)));
            for(const auto& entry:inventory->GetItems())inspect(entry.pItem);
        }
        list<CPtr<CObjectBase>> objects;human->GetVisibleObjects(&objects);
        for(const auto& object:objects){CDynamicCast<NWorld::IItem> ground(object.GetPtr());if(IsValid(ground))inspect(ground->GetInvItem());}
        itemID=0;CDBIterator<NDb::CRPGItem> records(*NDatabase::GetTable<NDb::CRPGItem>());
        while(records.MoveNext())if(records.Get()->GetRecordID()>0 && !present.count(records.Get()->GetRecordID())){itemID=records.Get()->GetRecordID();break;}
        Require(itemID>0,"scenario discovery fixture has no authored item absent from merged visibility");
    }
    CObj<NDb::CScenarioTask> taskDB=new NDb::CScenarioTask;taskDB->pDescription=description;
    taskDB->eTag=carrierTask?NDb::TT_DESTROY_ITEM_CARRIER:mode==1?NDb::TT_FIND_ITEM:mode==2?NDb::TT_FIND_PERSON:NDb::TT_COMPLETE_BY_SCRIPT;
    CObj<NDb::CScenarioGoal> goalDB=new NDb::CScenarioGoal;goalDB->tasks.push_back(taskDB.GetPtr());
    CObj<NDb::CDBScenarioClue> clueDB=new NDb::CDBScenarioClue;clueDB->clueType=mode==2?NDb::CT_PERSON:NDb::CT_ITEM;
    clueDB->nItemID=mode==2?0:itemID;clueDB->nPersID=mode==2?carrier->GetUnitRPG()->GetRPGPersID():0;clueDB->pGoal=goalDB.GetPtr();
    CObj<NScenario::CScenarioClue> clue=new NScenario::CScenarioClue(clueDB,91001);clue->SetPlaced();clue->SetDetected(latched);
    auto* goal=clue->GetGoal();Require(goal && goal->GetTasks().size()==1,"typed scenario fixture did not create its real goal/task");
    auto* task=goal->GetTasks().front().GetPtr();const bool initialVisible=!carrierTask || latched;
    if(mode==6) {
        std::printf("native scenario initial visibility: mode=%d tag=%d visible=%d expected=0 detected=%d state=%d parent=%d\n",mode,int(taskDB->eTag),int(task->IsVisible()),int(task->IsDetected()),int(task->GetState()),int(task->GetParentClue()==clue.GetPtr()));std::fflush(stdout);
        Require(!task->IsVisible() && !task->IsDetected() && task->GetState()==NScenario::TS_UNKNOWN && task->GetParentClue()==clue.GetPtr(),
            "new carrier task exposes its clue objective before actual carrier discovery");
    }
    if(mode==7) {
        int controls=0;
        for(int tag=0;tag<=int(NDb::TT_COMPLETE_BY_SCRIPT);++tag)if(tag!=int(NDb::TT_DESTROY_ITEM_CARRIER)) {
            CObj<NDb::CScenarioTask> controlDB=new NDb::CScenarioTask;controlDB->eTag=NDb::ETaskTag(tag);controlDB->pDescription=description;
            CObj<NDb::CScenarioGoal> controlGoalDB=new NDb::CScenarioGoal;controlGoalDB->tasks.push_back(controlDB.GetPtr());
            CObj<NScenario::CScenarioGoal> control=new NScenario::CScenarioGoal(controlGoalDB,clue);
            auto* entry=control->GetTasks().front().GetPtr();
            Require(entry->IsVisible() && !entry->IsDetected() && entry->GetState()==NScenario::TS_UNKNOWN && entry->GetParentClue()==clue.GetPtr(),
                "carrier initialization changed another task tag's visibility/state/parent");++controls;
        }
        std::printf("native scenario initial visibility controls: tags=%d visible=1 detected=0 state=0 parent=1\n",controls);std::fflush(stdout);
        Require(controls==8,"initial visibility control did not cover every other task tag");
    }
    if(mode!=6) {CMemoryStream fields;{CStructureSaver writer(fields,CStructureSaver::WRITE);bool value=initialVisible;writer.Add(6,&value);}
     fields.Seek(0);CStructureSaver reader(fields,CStructureSaver::READ);static_cast<CObjectBase*>(task)->operator&(reader);}
    CObj<NScenario::CScenarioZone> zone=campaign->pCurrentZone.GetPtr();
    if(!zone) {
        CDBIterator<NDb::CDBScenarioZone> zones(*NDatabase::GetTable<NDb::CDBScenarioZone>());Require(zones.MoveNext(),"scenario fixture has no real zone record");
        zone=NScenario::CreateScenarioZone(zones.Get(),91002);campaign->pCurrentZone=zone.GetPtr();
    }
    {vector<CPtr<NScenario::CScenarioClue>> clues{clue.GetPtr()};CStructureNetworkContext refs;refs.externalReferences=true;
     refs.ids[clue.GetPtr()]=100;refs.refs[100]=clue.GetPtr();refs.include=[](CObjectBase*){return false;};CMemoryStream fields;
     {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);writer.Add(11,&clues);}fields.Seek(0);CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(zone.GetPtr())->operator&(reader);}
    Require(zone->GetClues().size()==1 && zone->GetClues().front()==clue.GetPtr() && task->IsVisible()==initialVisible
        && task->GetDBTask()==taskDB.GetPtr(),"loaded-state scenario fixture changed its typed references or visibility input");
    vector<CPtr<NWorld::CUnit>> party;for(const auto& unit:units)if(unit->GetPlayer()==human)party.push_back(unit.GetPtr());
    NScenario::SGoalDescription before;campaign->pScenarioTracker->GetGoalDescription(&before,party,goal);
    world->UpdateVisible(true);human->GetVisible(&visible);
    Require(std::find(visible.begin(),visible.end(),CPtr<NWorld::CUnit>(carrier))!=visible.end(),"native carrier left merged visibility during scenario refresh");
    NScenario::SGoalDescription after;campaign->pScenarioTracker->GetGoalDescription(&after,party,goal);
    const bool expectedVisible=!unmatched || latched,expectedDetected=mode==1 || mode==2;
    std::printf("native scenario discovery: mode=%d visibleUnits=%zu item=%d person=%d tag=%d beforeRows=%zu afterRows=%zu visible=%d expectedVisible=%d taskDetected=%d expectedDetected=%d clueDetected=%d state=%d\n",
        mode,visible.size(),clueDB->nItemID,clueDB->nPersID,int(taskDB->eTag),before.tasks.size(),after.tasks.size(),int(task->IsVisible()),int(expectedVisible),int(task->IsDetected()),int(expectedDetected),int(clue->IsDetected()),after.tasks.empty()?-1:int(after.tasks.front().state));std::fflush(stdout);
    Require(task->IsVisible()==expectedVisible && after.tasks.size()==size_t(expectedVisible),"scenario visibility refresh did not reveal a loaded carrier task from the real merged visible inventory");
    Require(task->IsDetected()==expectedDetected && (!expectedDetected || (clue->IsDetected() && after.tasks.front().state==NScenario::STS_COMPLETED)),
        "scenario visibility refresh did not complete discovery through the native objective description");
    if(unmatched)Require(clue->IsDetected()==latched,"scenario discovery exposed an unseen item or forgot a previously detected clue");
    if(mode==5 || mode==7)Require(after.tasks.front().state==NScenario::STS_UNKNOWN,"visibility incorrectly completed an unrelated scripted task");
}

struct NativeCarrierClueInput
{
    CObj<NScenario::CScenarioClue> clue;CPtr<NScenario::CScenarioTask> task;CObj<NRPG::IInventoryItem> item;
};
static vector<NativeCarrierClueInput> NativeAuthoredCarrierClues(NScenario::CScenarioTracker* tracker,NRPG::IInventory* inventory,int count)
{
    vector<NativeCarrierClueInput> result;std::set<int> used;CDBIterator<NDb::CDBScenarioClue> rows(*NDatabase::GetTable<NDb::CDBScenarioClue>());
    while(rows.MoveNext() && int(result.size())<count){auto* record=rows.Get();if(record->clueType!=NDb::CT_ITEM || record->nItemID<=0 || used.count(record->nItemID))continue;auto* clue=tracker->GetClueByItemID(record->nItemID);auto* itemDB=NDb::GetRPGItem(record->nItemID);if(!clue || !clue->GetGoal() || !itemDB || tracker->IsClueFound(clue) || tracker->IsClueDestroyed(clue))continue;NScenario::CScenarioTask* task=nullptr;for(auto entry:clue->GetGoal()->GetTasks())if(entry && entry->GetDBTask() && entry->GetDBTask()->eTag==NDb::TT_DESTROY_ITEM_CARRIER && entry->GetState()==NScenario::TS_UNKNOWN && entry->GetParentClue()==clue){task=entry;break;}if(!task)continue;
        bool present=false;for(int slot=0;slot<NDb::N_SLOTS;++slot){auto* old=inventory->Get(NDb::ESlot(slot));present|=old && old->GetDBItem() && old->GetDBItem()->GetRecordID()==record->nItemID;}for(const auto& old:inventory->GetItems())present|=old.pItem && old.pItem->GetDBItem() && old.pItem->GetDBItem()->GetRecordID()==record->nItemID;if(present)continue;CObj<NRPG::IInventoryItem> item=NRPG::CreateClueItem(itemDB);CTPoint<int> place;if(!item || !inventory->FindPlace(item,&place))continue;result.push_back({clue,task,item});used.insert(record->nItemID);
    }
    std::printf("native authored carrier clue inputs found=%zu required=%d\n",result.size(),count);std::fflush(stdout);Require(int(result.size())==count,"native carrier fixture lacks actual authored flowchart item goals/carrier tasks; not a notification RED");return result;
}
static void ScenarioCarrierDestroyed(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=8,"unknown native carrier notification mode");CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,false,&campaign);auto* tracker=campaign->pScenarioTracker.GetPtr();Require(tracker && tracker->IsScenarioAvailable(),"native carrier fixture has no active real scenario");list<CPtr<NWorld::CUnitServer>> all;world->GetAllUnits(&all);NWorld::CUnitServer* anchor=nullptr;for(auto candidate:all)if(candidate && candidate->CanFight() && candidate->GetTBSPlayer() && !candidate->IsEmptyPK() && !candidate->IsWearingPK()){anchor=candidate;break;}Require(anchor,"native carrier fixture lacks actual world owner");CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(NDb::GetPers(103));CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(anchor->GetPosition().pos.p,mission,anchor->GetTBSPlayer(),"NativeCarrier");Require(actor && actor->CanFight() && actor->GetUnitRPG()==mission.GetPtr(),"native carrier fixture cannot deploy its ordinary actor");
    CObj<NWorld::CUnitServer> body;CObj<NRPG::IUnitMission> bodyMission;if(mode==7 || mode==8){bodyMission=NRPG::CreateUnit(NDb::GetPers(103));body=world->AddUnitInGame(anchor->GetPosition().pos.p,bodyMission,anchor->GetTBSPlayer(),"NativeCarrierPayload");Require(body && body->CanFight(),"native carrier fixture cannot deploy its real corpse payload");bodyMission->GetRPGUnit()->Kill();body->SetState(new NWorld::CUnitStateDeath(body));Require(body->IsDead() && !body->GetCorpseCarrier(),"native loaded corpse fixture did not enter real death state");}
    auto* owner=body?body.GetPtr():actor.GetPtr();auto* inventory=owner->GetUnitRPG()->GetInventory();auto inputs=NativeAuthoredCarrierClues(tracker,inventory,mode==5?2:1);vector<std::pair<NScenario::CScenarioTask*,NScenario::ETaskState>> unrelated;
    for(auto& input:inputs){Require(tracker->GetClueByItemID(input.item->GetDBItem()->GetRecordID())==input.clue.GetPtr() && input.task->GetParentClue()==input.clue.GetPtr(),"native carrier clue identity was not the actual flowchart lookup");input.clue->SetDetected(true);input.task->SetVisible(true);if(mode==6)input.task->SetState(NScenario::TS_COMPLETED);for(auto task:input.clue->GetGoal()->GetTasks())if(task && task->GetDBTask()->eTag!=NDb::TT_DESTROY_ITEM_CARRIER)unrelated.push_back({task,task->GetState()});if(mode!=3){CTPoint<int> place;Require(inventory->FindPlace(input.item,&place) && inventory->Place(place,input.item),"native carrier cannot place its actual authored plot item in the real backpack");}}
    if(mode==7){CObj<NWorld::CExecCorpse> pickup=new NWorld::CExecCorpse(actor,body,true);pickup->Run();pickup->TimeLabelReached();Require(actor->GetCorpse()==body.GetPtr() && body->GetCorpseCarrier()==actor.GetPtr(),"native carrier pickup failed to establish actual reciprocal carrying");}else Require(!actor->GetCorpse(),"native carrier unexpectedly has a carried body");vector<CPtr<NWorld::CUnit>> party;for(const auto& input:inputs)Require(tracker->GetTaskState(party,input.task)==(mode==6?NScenario::STS_COMPLETED:NScenario::STS_UNKNOWN),"native carrier task is already completed by an unintended party/clue input");
    if(mode==4){NRPG::CAttackPortion click(100000,0,0.f,4,4,0,0,nullptr,mission.GetPtr(),1.f,-1);CRay ray;ray.ptOrigin=actor->GetPosition().GetCP();ray.ptDir=CVec3(0,1,0);click.MakeClickOfDeath(ray);click.pTarget=mission;const auto hit=mission->ProcessAttack(world,NAI::HL_BODY,&click,nullptr);Require(hit.nDmg==0 && mission->IsUnconscious(),"native carrier knockout prerequisite did not execute actual RPG click");actor->MakeUnconscious(VNULL3,true,false);Require(actor->IsUnconscious() && inputs[0].task->GetState()==NScenario::TS_UNKNOWN,"scripted knockout prerequisite changed the carrier task before the tested death");}
    if(mode==1)world->RemoveUnit(actor);else if(mode!=2){mission->Kill();actor->KillUnit(VNULL3);Require(actor->IsDead() && mission->IsDead(),"native carrier did not execute the actual world death path");}list<CPtr<NWorld::CUnitServer>> after;world->GetAllUnits(&after);const bool present=std::find(after.begin(),after.end(),CPtr<NWorld::CUnitServer>(actor))!=after.end();Require(present==(mode!=1),"native carrier removal control changed world membership unexpectedly");const bool completed=mode!=2 && mode!=3 && mode!=8;int matched=0;for(const auto& input:inputs){const auto state=input.task->GetState();const auto displayed=tracker->GetTaskState(party,input.task);std::printf("native carrier destroyed mode=%d item=%d clue=%d tasks=%zu state=%d expected=%d display=%d expectedDisplay=%d removed=%d carried=%d\n",mode,input.item->GetDBItem()->GetRecordID(),input.clue->GetDBClue()->GetRecordID(),inputs.size(),int(state),int(completed?NScenario::TS_COMPLETED:NScenario::TS_UNKNOWN),int(displayed),int(completed?NScenario::STS_COMPLETED:NScenario::STS_UNKNOWN),int(!present),int(mode==7));std::fflush(stdout);matched+=state==(completed?NScenario::TS_COMPLETED:NScenario::TS_UNKNOWN) && displayed==(completed?NScenario::STS_COMPLETED:NScenario::STS_UNKNOWN);Require(input.task->GetParentClue()==input.clue.GetPtr() && tracker->GetClueByItemID(input.item->GetDBItem()->GetRecordID())==input.clue.GetPtr(),"native carrier notification replaced task/clue identities");}
    Require(matched==int(inputs.size()),"native death/removal did not complete actual native carrier tasks through the scenario and objective-state query");for(const auto& item:unrelated)Require(item.first->GetState()==item.second,"carrier notification changed an unrelated authored task tag");
}

static void ScenarioAttackKnockout(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=9,"unknown native scenario attack knockout mode");
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,false,&campaign);auto* tracker=campaign->pScenarioTracker.GetPtr();Require(tracker && tracker->IsScenarioAvailable(),"knockout fixture lacks active native scenario");
    CObj<NScenario::CScenarioClue> clue;CPtr<NScenario::CScenarioTask> task;NDb::CRPGPers* persona=nullptr;CDBIterator<NDb::CDBScenarioClue> rows(*NDatabase::GetTable<NDb::CDBScenarioClue>());
    while(rows.MoveNext()) {auto* row=rows.Get();if(row->clueType!=NDb::CT_PERSON || row->nPersID<=0)continue;auto* p=NDb::GetPers(row->nPersID);auto* c=tracker->GetClueByPersID(row->nPersID);if(!p || !p->pClass || !p->pModel || !p->pBaseValue || !p->pName || p->pClass->skills[NDb::ST_LEVEL]<=0 || p->pPanzerklein || p->pDefaultWearsPanzerklein || !c || !c->GetGoal())continue;for(auto t:c->GetGoal()->GetTasks())if(t && t->GetDBTask() && t->GetDBTask()->eTag==NDb::TT_STUN_PERSON && t->GetState()==NScenario::TS_UNKNOWN && t->GetParentClue()==c){clue=c;task=t;persona=p;break;}if(task)break;}
    Require(persona && clue && task,"knockout fixture lacks actual authored person/STUN goal; not a notification RED");
    list<CPtr<NWorld::CUnitServer>> all;world->GetAllUnits(&all);NWorld::CUnitServer* anchor=nullptr;for(auto u:all)if(u && u->CanFight() && u->GetTBSPlayer() && !u->IsEmptyPK() && !u->IsWearingPK()){anchor=u;break;}Require(anchor,"knockout fixture lacks world owner");
    NDb::CRPGPers* victim=persona;if(mode==4){victim=NDb::GetPers(103);Require(victim && victim!=persona,"knockout other-person control lacks different persona");}CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(victim);auto* unit=mission->GetRPGUnit();unit->Skills(NDb::ST_VP).SetConst(500);unit->nHealedVP=0;unit->Skills(NDb::ST_IC).SetConst(21);unit->Skills(NDb::ST_IC).SetProgress(0);CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(anchor->GetPosition().pos.p,mission,anchor->GetTBSPlayer(),"NativeScenarioKnockout");Require(actor && actor->CanFight() && !mission->GetPanzerklein() && mission->GetTotalVP()==500,"knockout fixture lacks healthy actual victim");
    clue->SetDetected(mode!=1);task->SetVisible(mode!=1);if(mode==8)task->SetState(NScenario::TS_COMPLETED);const bool detected=clue->IsDetected(),visible=task->IsVisible();vector<std::pair<NScenario::CScenarioTask*,NScenario::ETaskState>> unrelated;for(auto t:clue->GetGoal()->GetTasks())if(t && t->GetDBTask()->eTag!=NDb::TT_STUN_PERSON && t->GetDBTask()->eTag!=NDb::TT_DESTROY_ITEM_CARRIER)unrelated.push_back({t,t->GetState()});
    vector<NativeCarrierClueInput> items;if(mode==5){auto* bag=mission->GetInventory();items=NativeAuthoredCarrierClues(tracker,bag,1);CTPoint<int> place;Require(bag->FindPlace(items[0].item,&place) && bag->Place(place,items[0].item),"knockout fixture cannot place native clue");}
    NRPG::CAttackPortion portion(100000,0,0.f,0,0,0,0,nullptr,mission.GetPtr(),1.f,-1);CRay ray;ray.ptOrigin=actor->GetPosition().GetCP();ray.ptDir=CVec3(0,1,0);portion.MakeClickOfDeath(ray);portion.pTarget=mode==6?nullptr:mission.GetPtr();
    if(mode==3 || mode==9){auto hit=mission->ProcessAttack(world,NAI::HL_BODY,&portion,nullptr);Require(hit.nDmg==0 && mission->IsUnconscious(),"knockout script prerequisite failed in actual RPG");actor->MakeUnconscious(VNULL3,true,false);Require(actor->IsUnconscious() && task->GetState()==NScenario::TS_UNKNOWN,"knockout script prerequisite notified the scenario unexpectedly");}
    if(mode==2 || mode==3){portion.atkType=NRPG::AT_NORMAL;portion.nK=0;portion.nDmgMin=portion.nDmgMax=0;}
    int first=-1;if(mode!=9){Require(actor->ProcessAttack(world,NAI::HL_BODY,&portion,CVec3(0,1,0),mission->GetRPGArmor())==0,"native knockout attack changed hit damage");first=int(task->GetState());}if(mode==7){task->SetState(NScenario::TS_UNKNOWN);portion.atkType=NRPG::AT_NORMAL;portion.nK=0;portion.nDmgMin=portion.nDmgMax=0;actor->ProcessAttack(world,NAI::HL_BODY,&portion,CVec3(0,1,0),mission->GetRPGArmor());}
    const bool completed=mode==0 || mode==1 || mode==5 || mode==6 || mode==8;vector<CPtr<NWorld::CUnit>> party;const auto state=task->GetState();const auto display=tracker->GetTaskState(party,task);const auto expected=completed?NScenario::TS_COMPLETED:NScenario::TS_UNKNOWN;
    std::printf("native scenario attack knockout mode=%d persona=%d victim=%d clue=%d first=%d state=%d expected=%d display=%d KO=%d HP=%d detected=%d visible=%d carrier=%d\n",mode,persona->GetRecordID(),victim->GetRecordID(),clue->GetDBClue()->GetRecordID(),first,int(state),int(expected),int(display),int(actor->IsUnconscious()),mission->GetTotalVP(),int(clue->IsDetected()),int(task->IsVisible()),items.empty()?-1:int(items[0].task->GetState()));std::fflush(stdout);
    Require(actor->IsUnconscious()==(mode!=2) && mission->IsUnconscious()==(mode!=2) && !mission->IsDead() && mission->GetTotalVP()==500 && unit->Skills(NDb::ST_IC).GetProgress()==0,"native scenario attack lost zero-damage KO/blocked control or practiced IC");Require(state==expected && display==(completed?NScenario::STS_COMPLETED:NScenario::STS_UNKNOWN) && (mode!=7 || first==int(NScenario::TS_COMPLETED)),"actual new attack knockout did not complete the native person task once");Require(clue->IsDetected()==detected && task->IsVisible()==visible && tracker->GetClueByPersID(persona->GetRecordID())==clue.GetPtr() && task->GetParentClue()==clue.GetPtr(),"knockout changed visibility or native scenario identities");for(auto entry:unrelated)Require(entry.first->GetState()==entry.second,"knockout changed unrelated task tag");if(mode==5)Require(items[0].task->GetState()==NScenario::TS_COMPLETED,"new attack knockout omitted carrier task foundation");
}
