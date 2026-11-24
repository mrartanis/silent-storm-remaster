static void ScriptPK(bool leave)
{
    CObj<NWorld::CWorld> world = new NWorld::CWorld;
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    CObj<ScriptActor> actor = new ScriptActor;
    CObj<ScriptActor> shell = new ScriptActor;
    shell->shell = true;
    NScript::luaPushCPtr(*script, actor); script->SetGlobal("upstreamActor");
    NScript::luaPushCPtr(*script, shell); script->SetGlobal("upstreamShell");
    Script::AutoBlock stack(*script);
    if (leave) {
        RunScript(script, "upstreamResult = UnitLeavePK(upstreamActor)");
        Require(script->GetGlobal("upstreamResult").IsNil(), "UnitLeavePK returned a pilot instead of its missing shell");
    } else {
        CObj<NWorld::CPlayer> owner = new NWorld::CPlayer;
        CObj<NAI::CAICommander> commander = new NAI::CAICommander;
        owner->SetCommander(commander);
        actor->owner = owner; shell->owner = owner;
        RunScript(script, "UnitWearPK(upstreamActor, upstreamShell)");
        Require(commander->HasCommands() && actor->orders == 0,
            "UnitWearPK bypassed its owning queue for a valid nonfighting empty shell");
        CObj<NWorld::CCommand> first = commander->GetCommand();
        Require(commander->HasCommands(),"UnitWearPK omitted its ordered Continue");
        CObj<NWorld::CCommand> second = commander->GetCommand();
        auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
        auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
        auto* enter = set ? dynamic_cast<NWorld::CCmdTakeCorpse*>(set->GetCmd()) : nullptr;
        Require(set && set->pUnit == actor.GetPtr() && enter && enter->pCorpse == shell.GetPtr() &&
            resume && resume->pUnit == actor.GetPtr() && dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) &&
            !commander->HasCommands(),"UnitWearPK lost its empty-shell target, actor or exact Continue order");
        RunScript(script, "UnitWearPK(upstreamActor, upstreamActor)");
        Require(!commander->HasCommands() && actor->orders == 0, "UnitWearPK accepted a living soldier as a shell");
        Require(NAI::GetAICommander(actor) == commander, "live pilot lost its AI commander");
        Require(!NAI::GetAICommander(shell), "empty PK shell acquired an AI commander");
    }
}

static void PKEquipmentTransfer()
{
    CObj<NRPG::IInventory> shell = NRPG::CreateInventory(nullptr);
    CObj<NRPG::IInventory> pilot = NRPG::CreateInventory(nullptr);
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem;
    Require(shell->Equip(NDb::SLOT_1, weapon), "invalid PK weapon test fixture");
    CObj<NDb::CPanzerklein> fixedSuit = new NDb::CPanzerklein;
    fixedSuit->bSingleSlot = true;
    pilot->SetPanzerklein(fixedSuit, shell);
    Require(pilot->Get(NDb::SLOT_1) == weapon, "putting on a fixed-weapon PK lost its weapon");
    Require(!shell->Get(NDb::SLOT_1), "PK weapon exists in both inventories");
    Require(!pilot->CanEquip(NDb::SLOT_1, weapon), "manual fixed-weapon replacement became allowed");
    pilot->SetPanzerklein(nullptr, shell);
    Require(shell->Get(NDb::SLOT_1) == weapon && !pilot->Get(NDb::SLOT_1),
        "removing the PK failed to return its weapon to the shell");
}

template<class T>
static auto CanRemovePKWeapon(T* inventory, int) -> decltype(inventory->CanTakeOff(NDb::SLOT_1))
{ return inventory->CanTakeOff(NDb::SLOT_1); }
template<class T>
static bool CanRemovePKWeapon(T*, long) { return true; }

static void PKFixedWeaponPolicy()
{
    CObj<NRPG::IInventory> inventory = NRPG::CreateInventory(nullptr);
    Require(CanRemovePKWeapon(inventory.GetPtr(), 0), "normal equipment removal was restricted");
    Require(inventory->Activate(NDb::SLOT_2) && inventory->GetActiveSlot() == NDb::SLOT_2,
        "ordinary inventory cannot select its second slot");
    CObj<NDb::CPanzerklein> suit = new NDb::CPanzerklein;
    suit->bSingleSlot = false;
    inventory->SetPanzerklein(suit, nullptr);
    Require(CanRemovePKWeapon(inventory.GetPtr(), 0), "removable PK equipment was restricted");
    suit->bSingleSlot = true;
    Require(!CanRemovePKWeapon(inventory.GetPtr(), 0), "manual removal of a fixed PK weapon is allowed");
    Require(inventory->Activate(NDb::SLOT_1) && inventory->GetActiveSlot() == NDb::SLOT_1,
        "a single-slot suit cannot select its fixed weapon");
    Require(!inventory->Activate(NDb::SLOT_2) && inventory->GetActiveSlot() == NDb::SLOT_1,
        "a single-slot suit switched to its unused weapon slot");
}

static void DoorSwingDirection(const char* databasePath,const char* resourcePath)
{
    auto world = FixtureWorld(databasePath,resourcePath);
    auto* map = world->GetAIMap(); map->Sync();
    int doors = 0;
    for (const auto& entry : world->GetObjectsForHarness()) {
        auto* door = dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr());
        if (!door || door->IsBroken()) continue;
        const bool wasOpen = door->IsOpen();
        struct Restore { NAI::IAIMap* map; NWorld::CWindowDoor* door; bool open;
            ~Restore() { map->FlipDoorWindow(door,open); } } restore{map,door,wasOpen};
        // Existing native state filtering exposes each real collision hull in turn.
        // No grid cells or synthetic hull bounds are used as the expected direction.
        map->FlipDoorWindow(door,false); CObj<CObjectBase> closedHull = map->GetHull(door);
        CVec3 closed;
        if (!IsValid(closedHull) || !map->GetUnitHLPos(&closed,closedHull,-1)) continue;
        map->FlipDoorWindow(door,true); CObj<CObjectBase> openHull = map->GetHull(door);
        CVec3 opened;
        if (!IsValid(openHull) || openHull == closedHull || !map->GetUnitHLPos(&opened,openHull,-1)) continue;
        CVec3 expected = opened-closed;
        if (fabs2(expected) < 0.0001f) continue;
        Normalize(&expected);
        for (bool visibleOpen : {false,true}) {
            map->FlipDoorWindow(door,visibleOpen);
            const auto forward = door->GetChangeStateDirection(true), backward = door->GetChangeStateDirection(false);
            std::printf("native door swing: door=%d visible-open=%d length=%.6f hull-error=%.6f reverse-error=%.6f\n",
                doors,int(visibleOpen),float(fabs(forward)),float(fabs(forward-expected)),float(fabs(backward+expected)));
            Require(fabs2(forward-expected) < 0.00001f && fabs2(backward+expected) < 0.00001f,
                "native door swing uses arbitrary grid locks instead of its normalized collision-hull direction");
            Require(door->IsOpen() == wasOpen,"measuring a native door swing changed its logical open state");
        }
        ++doors;
    }
    Require(doors > 0,"door swing fixture has no authored door with distinct closed and open hulls");
    std::printf("native door swing: checked %d authored doors in both collision states\n",doors);
}

static void DoorApproachHeading(const char* databasePath,const char* resourcePath)
{
    auto world = FixtureWorld(databasePath,resourcePath,true);
    auto* network = world->GetPathNetwork(); world->GetAIMap()->Sync();
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto unit : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (candidate && !candidate->IsAIUnit() && candidate->CanFight() && candidate->GetUnitRPG()->CanMove()) { actor = candidate; break; }
    }
    Require(actor != nullptr,"door approach fixture has no movable native roster actor");
    const auto original = actor->GetPosition();
    const auto ap = actor->GetAP();
    struct Restore { NWorld::CUnitServer* actor; NAI::SUnitPosition position;
        ~Restore() { actor->SetTemporaryPosition(position); } } restore{actor,original};
    int cases = 0;
    for (const auto& entry : world->GetObjectsForHarness()) {
        auto* door = dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr());
        if (!door || door->IsBroken() || door->IsOpen()) continue;
        vector<NAI::SPathPlace> approaches; door->GetApproaches(&approaches,network);
        const auto swing = door->GetChangeStateDirection(true);
        for (int i=0; i<approaches.size(); ++i) {
            auto from = approaches[i];
            if (!network->IsPassable(from)) continue;
            // These real authored Unit bones face 4 or 0. Check concrete nearby
            // headings and the circular 6 -> 7 approach to 0, without reproducing
            // the production candidate-selection algorithm in the expectation.
            struct Case { int authored, facing, expected; };
            const Case casesForBone[] = {{4,4,4},{4,5,4},{4,6,5},{0,4,4},{0,6,7}};
            for (const auto& test : casesForBone) {
                if (approaches[i].GetDirection() != test.authored) continue;
                const int facing = test.facing;
                from.SetDirection(facing);
                NAI::SPosition at(from,network); const float angle = at.GetDirection();
                Require(swing.x*cos(angle)+swing.y*sin(angle) <= 0,"door approach fixture no longer needs a separate opening action");
                auto placed = original; placed.pos.p = from; actor->SetTemporaryPosition(placed);
                if (test.expected != facing) {
                    CObj<NAI::CPath> controlPath = new NAI::CPath;
                    controlPath->pNet = network; controlPath->points.push_back(from);
                    auto turn = from; turn.SetDirection(test.expected); controlPath->points.push_back(turn);
                    CObj<NWorld::CExecQueue> control = new NWorld::CExecQueue(actor);
                    control->AddPath(controlPath,NAI::PF_DEFAULT,NWorld::ITEM_NO_MATTER);
                    auto* head = control->GetFrontExecutor();
                    CObj<NAI::CPath> rotated = head ? head->GetCurrentPath() : nullptr;
                    Require(IsValid(rotated) && rotated->points.back().GetDirection() == test.expected,
                        "native rotation control cannot prepare the same actor's expected turn");
                }
                CObj<NAI::CPath> path = new NAI::CPath;
                path->pNet = network; path->points.push_back(from);
                NAI::CPath::SPathAction action; action.where = from; action.action = NAI::PA_OPEN; action.pObject = door;
                path->actions.push_back(action);
                CObj<NWorld::CExecQueue> queue = new NWorld::CExecQueue(actor);
                queue->AddPath(path,NAI::PF_DEFAULT,NWorld::ITEM_NO_MATTER);
                auto* front = queue->GetFrontExecutor(); CObj<NAI::CPath> prepared = front ? front->GetCurrentPath() : nullptr;
                Require(IsValid(front),"native door queue did not create its first movement executor");
                std::printf("native door approach: approach=%d authored-dir=%d from=%d expected=%d prepared=%d last=%d pose=%d\n",
                    i,approaches[i].GetDirection(),facing,test.expected,int(IsValid(prepared)),
                    IsValid(prepared) ? prepared->points.back().GetDirection() : -1,from.GetPose());
                if (test.expected != facing)
                    Require(IsValid(prepared) && prepared->points.back().GetDirection() == test.expected,
                        "native door queue omits the authored near-door approach turn");
                else
                    Require(!IsValid(prepared),"native door queue adds an unwanted turn for an aligned or opposite heading");
                ++cases;
                Require(actor->GetAP() == ap && door->IsOpen() == false,"preparing the native door queue executed a world action");
            }
        }
    }
    Require(cases >= 5,"door approach fixture did not cover its authored aligned, near-door and wrap headings");
}

static void DoorPathStop(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    auto* network = world->GetPathNetwork(); world->GetAIMap()->Sync();
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto unit: units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (candidate && !candidate->IsAIUnit() && candidate->CanFight() && !candidate->IsWearingPK() &&
            candidate->GetUnitRPG()->CanMove()) { actor = candidate; break; }
    }
    Require(actor != nullptr,"door-stop fixture has no native movable human");
    NWorld::CWindowDoor* selected = nullptr;
    NAI::SUnitPosition start = actor->GetPosition();
    CObj<NAI::CPath> approachPath;
    int candidates[6] = {};
    for (const auto& entry: world->GetObjectsForHarness()) {
        auto* door = dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr());
        if (!door || door->IsBroken() || door->IsOpen()) continue;
        ++candidates[0];
        door->LockDoor(false,0,1); world->GetAIMap()->Sync();
        ++candidates[1];
        vector<NAI::SPathPlace> approaches; door->GetApproaches(&approaches,network);
        for (auto target: approaches) {
            if (target.GetDirection() != 4 || !network->IsPassable(target) || network->IsLocked(target,true)) continue;
            ++candidates[2];
            NAI::SPosition at(target,network);
            const auto swing = door->GetChangeStateDirection(true);
            if (swing.x*cos(at.GetDirection())+swing.y*sin(at.GetDirection()) > 0) continue;
            ++candidates[3];
            vector<NAI::SPathPlace> nearbyPlaces; network->GetNearPlaces(SSphere(at.GetCP(),3.f),&nearbyPlaces);
            for (auto source: nearbyPlaces) {
                source.SetPose(target.GetPose()); source.SetDirection(4);
                if (source.GetLayer() != target.GetLayer() || !network->IsPassable(source) ||
                    network->IsLocked(source,false) || source.GetX() <= target.GetX() || source.GetY() != target.GetY()) continue;
                ++candidates[4];
                start.pos.p = source; actor->SetPosition(start);
                CObj<NAI::CPath> path = NWorld::FindPath(network,actor,source,vector<NAI::SPathPlace>{target},nullptr,true);
                if (!path || path->points.size() < 2 || !path->actions.empty()) continue;
                ++candidates[5];
                const auto before = path->points[path->points.size()-2];
                if (before.GetX() == target.GetX() && before.GetY() == target.GetY()) continue;
                selected = door; approachPath = path; break;
            }
            if (selected) break;
        }
        if (selected) break;
    }
    std::printf("native door-stop candidates: %d %d %d %d %d %d\n",candidates[0],candidates[1],candidates[2],candidates[3],candidates[4],candidates[5]);
    Require(selected && approachPath,"door-stop fixture has no authored unlocked door with a native walking approach");
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    Require(!world->IsRealTime() && !world->IsSequence(),"door-stop fixture did not enter a native player turn");
    for (int scenario: {0,1,2}) {
        actor->SetPosition(start); actor->SetWalkWithoutWeapon(true);
        CObj<NAI::CPath> path = new NAI::CPath;
        path->pNet = network; path->points = approachPath->points;
        // An intermediate path point keeps its walking flag until split for the door.
        path->points.back().SetMoving(scenario != 1);
        if (scenario != 0) {
            NAI::CPath::SPathAction action;
            action.where = path->points.back(); action.action = NAI::PA_OPEN; action.pObject = selected;
            path->actions.push_back(action);
        }
        const int initialAP = actor->GetAP();
        CObj<NWorld::CExecQueue> queue = new NWorld::CExecQueue(actor);
        queue->AddPath(path,NAI::PF_DEFAULT,NWorld::ITEM_NO_MATTER);
        auto* front = queue->GetFrontExecutor();
        CObj<NAI::CPath> prepared = front ? front->GetCurrentPath() : nullptr;
        Require(prepared && prepared->points.back().GetDirection() == 4,
            "door-stop fixture lost its real walking segment or added an unexpected turn");
        const int moveCost = front->GetActionAP();
        Require(actor->GetAP() == initialAP && !selected->IsOpen(),
            "preparing a door approach executed a game action");
        front->Run();
        for (int callback=0;callback<32 && front->GetState() == NWorld::CCommandExecute::RUNNING;++callback) {
            front->TimeLabelReached(); front->AnimationFinished();
        }
        Require(front->GetState() == NWorld::CCommandExecute::FINISHED &&
            actor->GetPosition().pos.p.GetX() == path->points.back().GetX() &&
            actor->GetPosition().pos.p.GetY() == path->points.back().GetY(),
            "native door approach did not finish at its authored interaction tile");
        std::printf("native door path: scenario=%d points=%d moving=%d AP=%d expected-cost=%d\n",
            scenario,int(prepared->points.size()),int(actor->GetPosition().pos.p.IsMoving()),
            initialAP-actor->GetAP(),moveCost);
        Require(initialAP-actor->GetAP() == moveCost && !selected->IsOpen(),
            "native approach changed the movement AP formula or prematurely opened the door");
        Require(actor->GetPosition().pos.p.IsMoving() == (scenario == 0),
            "native door approach ended in a moving state before the separate opening action");
        queue = nullptr;
        Require(!world->IsAction(),"door approach retained its native action counter");
    }
}

static void DoorCollision()
{
    CObj<RenderFixture> first = new RenderFixture, second = new RenderFixture;
    NAI::SSourceInfo closedA(first, nullptr, 0, NWorld::TS_STATE_CLOSED);
    NAI::SSourceInfo openA(first, nullptr, 0, NWorld::TS_STATE_OPEN);
    NAI::SSourceInfo openB(second, nullptr, 0, NWorld::TS_STATE_OPEN);
    NAI::SDoorColliderAnalyzer analyzer;
    Require(!analyzer(NAI::SColliderUserInfo(&closedA, 0)), "one door hull blocked by itself");
    Require(!analyzer(NAI::SColliderUserInfo(&openB, 0)), "inactive hulls from different doors were combined");
    Require(analyzer(NAI::SColliderUserInfo(&openA, 0)), "both hulls of the same door did not block");
    analyzer.Clear();
    Require(!analyzer.IsCollided(), "door collision state survived Clear");
}

static void OverlappingDoors()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    auto* layer = network->GetLayer(0);
    for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x)
        layer->tiles[y][x].nPassable = NAI::CP_STAND;
    CObj<RenderFixture> first = new RenderFixture, second = new RenderFixture;
    NAI::SPathPlace key(3, 3, 0), position = key;
    position.SetPose(NAI::CM_STAND);
    NAI::CNodesLayer::STile blocked{}, open{};
    open.nPassable = NAI::CP_STAND;
    auto* door = network->GetFlipper(first);
    door->bOpen = false; door->nFixedFlags = NAI::CPathNetwork::F_LOCKED_DOOR;
    door->locksClosed[key] = blocked; door->locksOpen[key] = open;
    door = network->GetFlipper(second);
    door->bOpen = false; door->nFixedFlags = NAI::CPathNetwork::F_LOCKED_DOOR;
    door->locksClosed[key] = open; door->locksOpen[key] = blocked;
    layer->tiles[3][3].nFlipper = 2; // The legacy single-door index hides door A.
    Require(network->IsLocked(position, false), "overlapping door A lost its blocking restriction");
    Require(!network->IsLocked(position, true), "ignoring doors also blocked an unlocked tile");
    network->GetFlipper(first)->bOpen = true;
    Require(!network->IsLocked(position, false), "inactive overlapping door hulls blocked the passage");
    // Replication restores fields into a live grid. A derived door lookup
    // must not retain indices into the old serialized flipper arrays.
    CMemoryStream update;
    { CStructureSaver saver(update, CStructureSaver::WRITE);
      std::vector<NAI::CPathNetwork::SFlipper> empty;
      std::unordered_map<CPtr<CObjectBase>, int, SPtrHash> noDoors;
      saver.Add(9, &empty); saver.Add(10, &noDoors); }
    update.Seek(0);
    { CStructureSaver saver(update, CStructureSaver::READ);
      static_cast<CObjectBase*>(network.GetPtr())->operator&(saver); }
    Require(network->GetFlippersAt(position).empty(), "restored grid retained a stale door lookup");
}

static void AISelectedMedicalProbe(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* anchor=nullptr;
    for(auto entry:units){auto* unit=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());if(unit && unit->IsAIUnit() && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()){anchor=unit;break;}}
    Require(anchor,"AI medical probe lacks a real native AI player/anchor");auto* player=dynamic_cast<NWorld::CPlayer*>(anchor->GetPlayer());Require(player,"AI medical probe lacks its actual player");auto* network=world->GetPathNetwork();
    auto point=anchor->GetPosition().pos.p;bool placed=false;
    for(auto offset:{CTPoint<int>(2,0),CTPoint<int>(-2,0),CTPoint<int>(0,2),CTPoint<int>(0,-2)}){const int x=int(point.GetX())+offset.x,y=int(point.GetY())+offset.y;if(x<0 || y<0 || x>255 || y>255)continue;auto candidate=point;candidate.SetXY(x,y);candidate.SetPose(NAI::CM_STAND);candidate.SetMoving(0);if(network->IsPassable(candidate) && !network->IsLocked(candidate)){point=candidate;placed=true;break;}}
    Require(placed,"AI medical probe has no actual free deployment point");auto* persona=NDb::GetPers(34);auto* weaponDB=NDb::GetWeapon(65);auto* kitDB=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(1);Require(persona && weaponDB && kitDB && kitDB->nAPToUse==0 && kitDB->nRequiedSkill==0 && kitDB->nRequiredPerkID==0,"AI medical probe lacks the actual unrestricted zero-preparation kit/medic/weapon");
    CObj<NRPG::CUnit> rpg=new NRPG::CUnit(persona,0,false,0,0,0,true);CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(rpg);auto* inventory=mission->GetInventory();vector<CObj<NRPG::IInventoryItem>> previous;
    for(auto slot:{NDb::SLOT_1,NDb::SLOT_2})if(auto* item=inventory->TakeOff(slot))previous.push_back(item);const auto bag=inventory->GetItems();for(const auto& item:bag){previous.push_back(item.pItem.GetPtr());inventory->Take(item.pItem);}
    CObj<NRPG::CWeaponItem> gun=new NRPG::CWeaponItem(weaponDB);CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(kitDB);const bool active=mode==1 || mode==2 || mode==4 || mode==7 || mode==8;
    if(mode==6)while(!kit->IsEmpty())kit->SpendPotion();Require(inventory->Equip(NDb::SLOT_1,active?static_cast<NRPG::IInventoryItem*>(kit.GetPtr()):static_cast<NRPG::IInventoryItem*>(gun.GetPtr())) && inventory->Activate(NDb::SLOT_1),"AI medical probe cannot equip its actual active item");
    if(!active && mode!=5){CTPoint<int> location;Require(inventory->FindPlace(kit,&location) && inventory->Place(location,kit),"AI medical probe cannot put its selected kit in the real backpack");}
    auto placement=anchor->GetPosition();placement.pos.p=point;CObj<NWorld::CUnitServer> actor=new NWorld::CUnitServer(world,mission,rpg->pModel,player,placement);world->AddUnit(actor);Require(actor && actor->CanFight() && actor->IsAIUnit(),"AI medical probe failed native actor registration");
    if(mode!=4){auto& hp=rpg->Skills(NDb::ST_VP);hp.SetValue(hp.GetCurrentMaxValue()/4);mission->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_VP));mission->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,1));}
    auto* ai=NAI::GetAIUnit(actor);Require(ai && ai->IsUnderAIControl() && ai->GetUnitServer()==actor.GetPtr(),"AI medical probe has no actual owned AI wrapper");ai->Synchronize(false);auto* planning=ai->GetAIInventory();Require(planning && !ai->GetAIUnitState()->pEnemy,"AI medical probe unexpectedly has a selected enemy or missing native inventory");auto* selected=planning->GetBestFirstAid();
    Require(mode==5?!selected:selected && selected->GetItem()==kit.GetPtr() && planning->IsCurrentItem(selected)==active,"AI medical probe did not choose the actual held/backpack kit as expected");
    Require(mode==4?ai->GetHP()==ai->GetMaxHP():ai->GetHP()>0 && ai->GetHP()<=.3f*ai->GetMaxHP(),"AI medical probe lacks its actual injured/healthy threshold precondition");
    NAI::SPlaceWithAP query(ai->GetUnitPosition(),ai->GetAP());const bool future=mode==2 || mode==3 || mode==7 || mode==8;
    if(future){vector<NAI::SPathPlace> candidates;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),3.f),&candidates);bool found=false;
        for(auto candidate:candidates){candidate.SetPose(NAI::CM_STAND);if(!candidate.IsIntegral() || candidate.IsFinal() || !network->IsPassable(candidate) || network->IsLocked(candidate))continue;auto pos=actor->GetPosition();pos.pos=NAI::SPosition(candidate,network);const float distance=fabs(pos.GetCP()-actor->GetPosition().GetCP());if(distance<1.25f || distance>2.5f)continue;query.place=pos;found=true;break;}
        Require(found && !NWorld::IsWithinHumanReach(query.place.GetCP(),actor->GetPosition().GetCP(),.8f),"AI medical probe has no real future self-position beyond current-world heal reach");
    }
    if(mode==7){auto fly=query.place.pos;Require(NAI::MakeFlyPos(query.place.GetCP(),network,&fly),"AI medical probe cannot encode the real future fly point");query.place.pos=fly;}
    if(mode==8)query.place.pos.p.SetPose(NAI::CM_INACTIVE);
    const auto position=actor->GetPosition(),preview=query.place;const int ap=actor->GetAP(),aiAP=ai->GetAP(),hp=ai->GetHP(),maxHP=ai->GetMaxHP(),rawHP=rpg->Skills(NDb::ST_VP),dose=kit->GetIncQuantity(),medicine=rpg->Skills(NDb::ST_MEDICINE);auto* held=inventory->GetActive();auto random=GlobalGameRandom();
    CObj<NAI::CAIHealAction> action=new NAI::CAIHealAction(ai);NAI::CAIHealAction::SInfo info,repeat;action->GetInfoInner(query,&info);action->GetInfoInner(query,&repeat);const bool cached=action->CanDo(query),expected=mode<=3;auto afterRandom=GlobalGameRandom();
    std::printf("native AI medical probe: mode=%d activeKit=%d future=%d selected=%d currentSelected=%d HP=%d maxHP=%d medicine=%d actualAP=%d planningAP=%d budget=%d dose=%d distance=%g final=%d pose=%d canDo=%d repeat=%d cached=%d expected=%d\n",mode,int(active),int(future),selected?selected->GetItem()->GetDBFirstAid()->GetRecordID():0,int(selected && planning->IsCurrentItem(selected)),hp,maxHP,medicine,ap,aiAP,query.nUnitAP,dose,fabs(preview.GetCP()-position.GetCP()),int(preview.pos.p.IsFinal()),int(preview.pos.p.GetPose()),int(info.bCanDo),int(repeat.bCanDo),int(cached),int(expected));std::fflush(stdout);
    Require(actor->GetAP()==ap && ai->GetAP()==aiAP && ai->GetHP()==hp && ai->GetMaxHP()==maxHP && int(rpg->Skills(NDb::ST_VP))==rawHP && int(rpg->Skills(NDb::ST_MEDICINE))==medicine && kit->GetIncQuantity()==dose && inventory->GetActive()==held && actor->GetPosition()==position && query.place==preview && planning->GetBestFirstAid()==selected && !ai->GetAIUnitState()->pEnemy && random.Get()==afterRandom.Get(),"AI medical planning changed world/AP/HP/item/pose/skill/selection/enemy/RNG instead of probing");
    Require(info.bCanDo==expected && repeat.bCanDo==expected && cached==expected && (mode==5 || info.pFirstAid==selected && repeat.pFirstAid==selected),"AI medical probe ignores its actual selected backpack kit or future self-target position");
}

static void AISelectedMedicalCost(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* anchor=nullptr;
    for(auto entry:units){auto* unit=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());if(unit && unit->IsAIUnit() && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()){anchor=unit;break;}}
    Require(anchor,"AI medical cost lacks a real native AI player/anchor");auto* player=dynamic_cast<NWorld::CPlayer*>(anchor->GetPlayer());Require(player,"AI medical cost lacks its actual player");auto* network=world->GetPathNetwork();
    auto point=anchor->GetPosition().pos.p;bool placed=false;
    for(auto offset:{CTPoint<int>(2,0),CTPoint<int>(-2,0),CTPoint<int>(0,2),CTPoint<int>(0,-2)}){const int x=int(point.GetX())+offset.x,y=int(point.GetY())+offset.y;if(x<0 || y<0 || x>255 || y>255)continue;auto candidate=point;candidate.SetXY(x,y);candidate.SetPose(NAI::CM_STAND);candidate.SetMoving(0);if(network->IsPassable(candidate) && !network->IsLocked(candidate)){point=candidate;placed=true;break;}}
    Require(placed,"AI medical cost has no actual free deployment point");auto* persona=NDb::GetPers(34);auto* weaponDB=NDb::GetWeapon(65);auto* kitDB=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(mode==7?1:3);Require(persona && weaponDB && kitDB && kitDB->nAPToUse==(mode==7?0:10) && kitDB->nRequiredPerkID==0,"AI medical cost lacks the actual authored critical-first/zero-preparation kit, medic and weapon");
    CObj<NRPG::CUnit> rpg=new NRPG::CUnit(persona,0,false,0,0,0,true);CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(rpg);auto* inventory=mission->GetInventory();vector<CObj<NRPG::IInventoryItem>> previous;
    for(auto slot:{NDb::SLOT_1,NDb::SLOT_2})if(auto* item=inventory->TakeOff(slot))previous.push_back(item);const auto bag=inventory->GetItems();for(const auto& item:bag){previous.push_back(item.pItem.GetPtr());inventory->Take(item.pItem);}
    CObj<NRPG::CWeaponItem> gun=new NRPG::CWeaponItem(weaponDB);CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(kitDB);const bool active=mode==3 || mode==4 || mode==6;
    Require(inventory->Equip(NDb::SLOT_1,active?static_cast<NRPG::IInventoryItem*>(kit.GetPtr()):static_cast<NRPG::IInventoryItem*>(gun.GetPtr())) && inventory->Activate(NDb::SLOT_1),"AI medical cost cannot equip its actual active item");
    if(!active){CTPoint<int> location;Require(inventory->FindPlace(kit,&location) && inventory->Place(location,kit),"AI medical cost cannot put its selected kit in the real backpack");}
    auto placement=anchor->GetPosition();placement.pos.p=point;CObj<NWorld::CUnitServer> actor=new NWorld::CUnitServer(world,mission,rpg->pModel,player,placement);world->AddUnit(actor);Require(actor && actor->CanFight() && actor->IsAIUnit(),"AI medical cost failed native actor registration");
    {auto& hp=rpg->Skills(NDb::ST_VP);hp.SetValue(hp.GetCurrentMaxValue()/4);mission->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_VP));mission->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,1));}
    auto* ai=NAI::GetAIUnit(actor);Require(ai && ai->IsUnderAIControl() && ai->GetUnitServer()==actor.GetPtr(),"AI medical cost has no actual owned AI wrapper");ai->Synchronize(false);auto* planning=ai->GetAIInventory();Require(planning && !ai->GetAIUnitState()->pEnemy,"AI medical cost unexpectedly has a selected enemy or missing native inventory");auto* selected=planning->GetBestFirstAid();

    Require(selected && selected->GetItem()==kit.GetPtr() && planning->IsCurrentItem(selected)==active,"AI medical cost did not select its actual held/backpack kit");
    const int price=kitDB->nAPToUse,heldPrice=mission->GetActionAP(NAI::WALK,NRPG::AC_FIRSTAID),medicine=rpg->Skills(NDb::ST_MEDICINE);
    Require(medicine+kitDB->nSkillModifier>=kitDB->nRequiedSkill && ai->GetHP()>0 && ai->GetHP()<=.3f*ai->GetMaxHP(),"AI medical cost lacks its native qualified injured actor");
    Require(heldPrice==(active?price:15),"AI medical cost does not distinguish selected and held-item prices");
    const bool spend=mode==5 || mode==6,expected=mode!=0 && mode!=3;
    const int budget=spend?ai->GetAP():mode==7?0:mode==0 || mode==3?price-1:mode==2?price+1:price;
    NAI::SPlaceWithAP query(ai->GetUnitPosition(),budget);const auto position=actor->GetPosition(),preview=query.place;
    const int ap=actor->GetAP(),aiAP=ai->GetAP(),hp=ai->GetHP(),maxHP=ai->GetMaxHP(),rawHP=rpg->Skills(NDb::ST_VP),dose=kit->GetIncQuantity();auto* held=inventory->GetActive();auto random=GlobalGameRandom();
    CObj<NAI::CAIHealAction> action=new NAI::CAIHealAction(ai);NAI::CAIHealAction::SInfo info,repeat;action->GetInfoInner(query,&info);action->GetInfoInner(query,&repeat);const bool cached=action->CanDo(query);auto afterRandom=GlobalGameRandom();
    std::printf("native AI medical cost: mode=%d kit=%d activeKit=%d selectedPrice=%d heldPrice=%d budget=%d physicalAP=%d planningAP=%d HP=%d maxHP=%d medicine=%d required=%d modifier=%d dose=%d canDo=%d repeat=%d cached=%d expected=%d\n",mode,kitDB->GetRecordID(),int(active),price,heldPrice,budget,ap,aiAP,hp,maxHP,medicine,kitDB->nRequiedSkill,kitDB->nSkillModifier,dose,int(info.bCanDo),int(repeat.bCanDo),int(cached),int(expected));std::fflush(stdout);
    Require(actor->GetAP()==ap && ai->GetAP()==aiAP && ai->GetHP()==hp && ai->GetMaxHP()==maxHP && int(rpg->Skills(NDb::ST_VP))==rawHP && int(rpg->Skills(NDb::ST_MEDICINE))==medicine && kit->GetIncQuantity()==dose && inventory->GetActive()==held && actor->GetPosition()==position && query.place==preview && planning->GetBestFirstAid()==selected && !ai->GetAIUnitState()->pEnemy && random.Get()==afterRandom.Get(),"AI medical cost query changed world/planning state");
    Require(info.bCanDo==expected && repeat.bCanDo==expected && cached==expected && info.pFirstAid==selected && repeat.pFirstAid==selected,"AI medical affordability uses the held item instead of the selected kit price");
    if(!spend)return;
    auto planRandom=GlobalGameRandom();CObj<NAI::CAILog> log=NAI::CreateAILog();action->Do(log);afterRandom=GlobalGameRandom();
    std::printf("native AI medical cost spend: mode=%d before=%d after=%d expected=%d physicalBefore=%d physicalAfter=%d selectedCurrent=%d\n",mode,aiAP,ai->GetAP(),aiAP-price,ap,actor->GetAP(),int(planning->IsCurrentItem(selected)));std::fflush(stdout);
    Require(!log->IsEmpty() && planning->IsCurrentItem(selected) && actor->GetAP()==ap && ai->GetHP()==hp && ai->GetMaxHP()==maxHP && int(rpg->Skills(NDb::ST_VP))==rawHP && int(rpg->Skills(NDb::ST_MEDICINE))==medicine && kit->GetIncQuantity()==dose && inventory->GetActive()==held && actor->GetPosition()==position && planRandom.Get()==afterRandom.Get(),"AI medical plan changed actual world/HP/doses/held item/pose/skill/RNG or failed to select its kit");
    Require(ai->GetAP()==aiAP-price,"AI medical plan spends the held item price instead of the selected kit preparation AP");
}

static void AISelectedReloadActionCost(const char* databasePath,const char* resourcePath,bool spend)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* anchor=nullptr;
    for(auto entry:units) {
        auto* actor=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(actor && actor->IsAIUnit() && actor->CanFight() && !actor->IsEmptyPK() && !actor->IsWearingPK()) {anchor=actor;break;}
    }
    Require(anchor,"AI reload fixture lacks a real native AI owner/player");
    auto* player=dynamic_cast<NWorld::CPlayer*>(anchor->GetPlayer());Require(player,"AI reload fixture lacks its native player");
    auto point=anchor->GetPosition().pos.p;bool placed=false;auto* network=world->GetPathNetwork();
    for(auto offset:{CTPoint<int>(2,0),CTPoint<int>(-2,0),CTPoint<int>(0,2),CTPoint<int>(0,-2)}) {
        const int x=int(point.GetX())+offset.x,y=int(point.GetY())+offset.y;
        if(x<0 || y<0 || x>255 || y>255)continue;
        auto candidate=point;candidate.SetXY(x,y);candidate.SetPose(NAI::CM_STAND);candidate.SetMoving(0);
        if(network->IsPassable(candidate) && !network->IsLocked(candidate)) {point=candidate;placed=true;break;}
    }
    Require(placed,"AI reload fixture lacks a real unoccupied tile beside its AI owner");
    auto* persona=NDb::GetPers(265);auto* currentDB=NDb::GetWeapon(65);auto* selectedDB=NDb::GetWeapon(127);auto* perk=NDb::GetDBPerk(33);
    Require(persona && currentDB && selectedDB && perk && !perk->params.empty() &&
        IsValid(currentDB->pInnerClip) && IsValid(selectedDB->pInnerClip) && currentDB->nReloadAP!=selectedDB->nReloadAP,
        "AI reload fixture lacks distinct authored current/selected firearms or its perk");
    CObj<NRPG::CUnit> rpg=new NRPG::CUnit(persona);CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(rpg);
    auto* inventory=mission->GetInventory();vector<CObj<NRPG::IInventoryItem>> original;
    for(auto slot:{NDb::SLOT_1,NDb::SLOT_2})if(auto* item=inventory->TakeOff(slot))original.push_back(item);
    const auto bag=inventory->GetItems();
    for(const auto& item:bag) {original.push_back(item.pItem.GetPtr());inventory->Take(item.pItem);}
    CObj<NRPG::CWeaponItem> current=new NRPG::CWeaponItem(currentDB),selected=new NRPG::CWeaponItem(selectedDB);
    selected->DiscardAmmo();
    Require(inventory->Equip(NDb::SLOT_1,current) && inventory->Equip(NDb::SLOT_2,selected) && inventory->Activate(NDb::SLOT_1),
        "AI reload fixture did not install its real held and holstered firearms");
    CObj<NRPG::IInventoryItem> spare=NRPG::CreateClipItem(selectedDB->pInnerClip);CTPoint<int> place;
    Require(spare && inventory->FindPlace(spare,&place) && inventory->Place(place,spare) && selected->CanReload(inventory),
        "AI reload fixture failed to place compatible real spare ammunition");
    auto placement=anchor->GetPosition();placement.pos.p=point;
    CObj<NWorld::CUnitServer> actor=new NWorld::CUnitServer(world,mission,rpg->pModel,player,placement);world->AddUnit(actor);
    auto* ai=NAI::GetAIUnit(actor);
    Require(actor && actor->IsAIUnit() && actor->CanFight() && ai && ai->IsUnderAIControl() && ai->GetUnitServer()==actor.GetPtr(),
        "AI reload fixture did not register its genuine owned runtime AI unit");
    ai->Synchronize();auto* planning=ai->GetAIInventory();auto* planned=planning?planning->GetBestWeaponForReload():nullptr;
    Require(planned && planned->GetItem()==selected.GetPtr() && !planning->IsCurrentItem(planned) &&
        IsValid(planned->GetNextClip()) && planned->GetNextClip()->GetAmmoCount()>0,
        "AI reload fixture did not resolve its sole reload-eligible holstered firearm and live spare");
    Require(!mission->HasPerk(33) && planned->GetReloadAP()==selectedDB->nReloadAP,"AI reload no-perk/raw-price control failed");
    CObj<NAI::CAIReloadAction> action=new NAI::CAIReloadAction(ai);NAI::SPlaceWithAP query;
    query.place=ai->GetUnitPosition();query.nUnitAP=selectedDB->nReloadAP;NAI::CAIReloadAction::SInfo info;
    action->GetInfoInner(query,&info);Require(info.bCanDo && info.pWeapon==planned,"AI reload positive budget control rejected its actual selected weapon");
    rpg->GetPerksTree()->AddPerkPoints(1);rpg->GetPerksTree()->TakePerk(33);float parameter=0;
    Require(mission->HasPerk(33,&parameter) && parameter==perk->params[0],"AI reload fixture did not acquire its genuine authored perk33");
    const int price=rpg->GetWeaponReloadActionAP(selected),held=rpg->GetWeaponReloadActionAP(current),physicalAP=actor->GetAP();
    Require(price==int(selectedDB->nReloadAP*parameter) && price>0 && price<selectedDB->nReloadAP && price!=held &&
        mission->GetActionAP(NAI::WALK,NRPG::AC_RELOAD)==held,"AI reload fixture does not distinguish the selected native action price from held/raw prices");
    query.nUnitAP=price;action->GetInfoInner(query,&info);
    std::printf("native AI reload budget: selected=%d held=%d raw=%d selectedAction=%d heldAction=%d budget=%d canDo=%d spendMode=%d\n",
        selectedDB->GetRecordID(),currentDB->GetRecordID(),selectedDB->nReloadAP,price,held,query.nUnitAP,int(info.bCanDo),int(spend));std::fflush(stdout);
    if(!spend) {
        Require(info.bCanDo && info.pWeapon==planned,"AI reload rejects an affordable selected weapon by using raw or held-weapon AP instead of its action price");
        query.nUnitAP=price-1;action->GetInfoInner(query,&info);Require(!info.bCanDo,"AI reload accepts a selected action without sufficient AP");
        Require(actor->GetAP()==physicalAP && selected->GetAmmoQuantity()==0 && inventory->GetActive()==current.GetPtr(),
            "AI availability query spent world AP, loaded ammunition or changed the held weapon");return;
    }
    const int budget=selectedDB->nReloadAP,maximum=ai->GetMaxAP();Require(budget<=maximum,"AI reload spend fixture exceeds its native maximum planning AP");
    ai->SetAP(budget,maximum);CObj<NAI::CAILog> log=NAI::CreateAILog();action->Do(log);
    std::printf("native AI reload spend: before=%d after=%d expected=%d physicalBefore=%d physicalAfter=%d\n",budget,ai->GetAP(),budget-price,physicalAP,actor->GetAP());std::fflush(stdout);
    Require(!log->IsEmpty() && ai->GetAP()==budget-price,"AI reload log spends raw or held-weapon AP instead of the original selected action price");
    Require(planning->IsCurrentItem(planned) && planned->IsCurrentClipFull() && actor->GetAP()==physicalAP &&
        selected->GetAmmoQuantity()==0 && inventory->GetActive()==current.GetPtr(),"AI planning reload changed the real world or failed to update its native inventory mirror");
}

static void RetailReloadActionCost(const char* databasePath)
{
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);
    auto* perk=NDb::GetDBPerk(33);
    Require(perk && !perk->params.empty() && std::isfinite(perk->params[0]) && perk->params[0]>0 && perk->params[0]<1,
        "reload action fixture lacks the real authored perk33 parameter");
    NDb::CRPGPers* persona=nullptr;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext()) {
        auto* candidate=people.Get();
        if(!IsValid(candidate->pClass) || !IsValid(candidate->pBaseValue) || !IsValid(candidate->pModel) ||
            IsValid(candidate->pPanzerklein) || IsValid(candidate->pDefaultWearsPanzerklein))continue;
        CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(candidate->pClass->nPerkTreeID);
        vector<CPtr<NRPG::CPerk>> entries;tree->GetAllPerks(&entries);
        for(auto entry:entries)if(entry->GetDBPerk()->GetRecordID()==33) {persona=candidate;break;}
        if(persona)break;
    }
    Require(persona,"reload action fixture lacks an authored persona with perk33");
    NDb::CRPGWeapon* weapon=nullptr;
    CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while(weapons.MoveNext()) {
        auto* candidate=weapons.Get();
        if(IsValid(candidate->pItem) && IsValid(candidate->pInnerClip) && candidate->nReloadAP>3 && int(candidate->nReloadAP*perk->params[0])!=int(candidate->nReloadAP*(1-perk->params[0]))) {
            weapon=candidate;break;
        }
    }
    Require(weapon,"reload action fixture lacks an authored firearm distinguishing the original multiplier from a discount");
    CObj<NRPG::CUnit> unit=new NRPG::CUnit(persona);
    CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(unit);
    CObj<NRPG::CWeaponItem> gun=new NRPG::CWeaponItem(weapon);unit->SetCannonItem(gun);
    Require(unit->GetWeaponItem()==gun.GetPtr() && !mission->HasPerk(33),"reload action fixture lacks its native effective weapon or starts with the perk");
    const int raw=weapon->nReloadAP,ap=unit->Skills(NDb::ST_AP);
    Require(unit->GetWeaponReloadAP()==raw && gun->GetReloadAP()==raw,"reload action positive control changed the authored raw weapon cost");
    for(auto pose:{NAI::WALK,NAI::CROUCH,NAI::CRAWL})Require(mission->GetActionAP(pose,NRPG::AC_RELOAD)==raw,"no-perk native reload cost differs from the authored firearm");
    std::printf("native reload action control: persona=%d weapon=%d raw=%d perkParam=%g noPerk=%d\n",persona->GetRecordID(),weapon->GetRecordID(),raw,
        perk->params[0],mission->GetActionAP(NAI::WALK,NRPG::AC_RELOAD));std::fflush(stdout);
    unit->GetPerksTree()->AddPerkPoints(1);unit->GetPerksTree()->TakePerk(33);
    float parameter=0;Require(mission->HasPerk(33,&parameter) && parameter==perk->params[0],"reload action fixture did not acquire its actual authored perk");
    const int expected=int(raw*parameter);
    for(auto pose:{NAI::WALK,NAI::CROUCH,NAI::CRAWL}) {
        const int actual=mission->GetActionAP(pose,NRPG::AC_RELOAD);
        std::printf("native reload action: pose=%d raw=%d param=%g actual=%d retailExpected=%d\n",int(pose),raw,parameter,actual,expected);std::fflush(stdout);
        Require(actual==expected,"native reload action ignores the original perk33 multiplier");
    }
    Require(unit->GetWeaponReloadActionAP()==expected && unit->GetWeaponReloadActionAP(gun)==expected,
        "shared reload action entry lost its effective or explicitly selected authored weapon");
    // Preserve the original integer-zero edge without mutating game.db: this is
    // a complete native in-memory record copy with only the raw reload AP set to1.
    CObj<NDb::CRPGWeapon> small=new NDb::CRPGWeapon;CMemoryStream fields;
    {CStructureSaver writer(fields,CStructureSaver::WRITE);weapon->operator&(writer);}
    fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ);small->operator&(reader);}
    small->nReloadAP=1;CObj<NRPG::CWeaponItem> smallGun=new NRPG::CWeaponItem(small);
    Require(unit->GetWeaponReloadActionAP(smallGun)==0 && unit->GetWeaponItem()==gun.GetPtr() &&
        unit->GetWeaponReloadActionAP()==expected,"planned reload query clamped the original zero price or replaced the effective weapon");
    unit->SetCannonItem(smallGun);
    for(auto pose:{NAI::WALK,NAI::CROUCH,NAI::CRAWL})Require(mission->GetActionAP(pose,NRPG::AC_RELOAD)==0,
        "native reload action introduced a one-AP floor after the original fractional multiplier");
    unit->SetCannonItem(nullptr);
    auto* inventory=unit->GetInventory();const auto activeSlot=NDb::ESlot(inventory->GetActiveSlot());
    CObj<NRPG::IInventoryItem> equipped=inventory->GetActive();
    CObj<NRPG::IInventoryItem> removed;
    if(equipped) {
        removed=inventory->TakeOff(activeSlot);
        Require(removed==equipped.GetPtr(),"unarmed control removed a different equipped item");
    }
    Require(!inventory->GetActive() && !unit->GetWeaponItem(),"unarmed control did not establish an actual empty active slot");
    Require(mission->GetActionAP(NAI::WALK,NRPG::AC_RELOAD)==0,"an unarmed unit acquired a nonzero native reload price");
    if(equipped)Require(inventory->Equip(activeSlot,equipped) && inventory->Activate(activeSlot) && inventory->GetActive()==equipped.GetPtr(),
        "reload edge fixture failed to restore the persona's actual inventory item");
    unit->SetCannonItem(gun);
    Require(unit->GetWeaponReloadAP()==raw && gun->GetReloadAP()==raw && weapon->nReloadAP==raw &&
        unit->GetWeaponReloadActionAP()==expected && int(unit->Skills(NDb::ST_AP))==ap,
        "querying a perk-adjusted action changed authored/raw forecast costs, the effective weapon or unit AP");
    std::printf("native reload edge controls: selectedRaw=1 action=0 unarmed=0 restoredRaw=%d restoredAction=%d APunchanged=%d\n",raw,expected,ap);
}

static void GlobalGameDifficulty(const char* databasePath)
{
    Require(std::getenv("S2_USER_DATA_DIR"),"difficulty campaign fixture requires isolated user data");
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);
    auto* originalDefault=NDb::GetDBDifficulty(1);auto* other=NDb::GetDBDifficulty(2);
    Require(originalDefault && other && originalDefault!=other,"difficulty fixture lacks distinct original DB records1/2");
    for(auto* record:{originalDefault,other})
        std::printf("native difficulty control: id=%d AP=%.3f VP=%.3f enemyDamage=%.3f ourDamage=%.3f AILevel=%d\n",
            record->GetRecordID(),record->fAPCoeff,record->fVPCoeff,record->fEnemyDamageMult,record->fOurDamageMult,record->nAIUnitsLevel);
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(-1);
    Require(game && IsValid(game->pScenarioTracker),"difficulty fixture failed native global-game/scenario construction");
    std::printf("native global difficulty: default=%d expected=%d\n",game->pDifficulty ? game->pDifficulty->GetRecordID() : -1,originalDefault->GetRecordID());std::fflush(stdout);
    Require(game->pDifficulty==originalDefault,"new global game ignored the original default difficulty1 and silently selected difficulty2");
    CObj<NRPG::CGlobalGame> nullChoice=NRPG::CreateGlobalGame(-1,nullptr);
    Require(nullChoice->pDifficulty==originalDefault,"explicit null difficulty did not use original fallback1");
    for(int id:{1,2,3}) {
        auto* selected=NDb::GetDBDifficulty(id);Require(selected,"difficulty fixture lacks its authored selected record");
        CObj<NRPG::CGlobalGame> chosen=NRPG::CreateGlobalGame(-1,selected);
        Require(chosen->pDifficulty==selected,"native global factory replaced the supplied difficulty record");
        std::printf("native chosen difficulty: requested=%d actual=%d\n",id,chosen->pDifficulty->GetRecordID());
    }
    CObj<NDb::CDBDifficulty> owner=new NDb::CDBDifficulty;CPtr<NDb::CDBDifficulty> expired=owner.GetPtr();
    owner=nullptr;
    Require(expired.GetPtr() && !IsValid(expired),"difficulty fixture failed to retain a native invalid weak record");
    CObj<NRPG::CGlobalGame> invalidChoice=NRPG::CreateGlobalGame(-1,expired.GetPtr());
    Require(invalidChoice->pDifficulty==originalDefault,"invalid native difficulty record did not use original fallback1");
    game->ChangeDifficulty(2);
    Require(game->pDifficulty==other,"existing ChangeDifficulty stopped retaining the requested authored record");
    CMemoryStream save;
    {CStructureSaver writer(save,CStructureSaver::WRITE);writer.Add(2,&game);}
    const int bytes=save.GetSize();save.Seek(0);CObj<NRPG::CGlobalGame> loaded;
    {CStructureSaver reader(save,CStructureSaver::READ);reader.Add(2,&loaded);}
    Require(loaded && loaded.GetPtr()!=game.GetPtr() && IsValid(loaded->pScenarioTracker) && loaded->pDifficulty==other,
        "restoring an existing difficulty2 global-game graph replaced its saved difficulty with the new-game default");
    Require(NDb::GetDBDifficulty(1)==originalDefault && NDb::GetDBDifficulty(2)==other,
        "global-game difficulty initialization replaced authored DB records");
    std::printf("native global difficulty controls: null/invalid fallback1, records1/2/3, saved difficulty2 graph=%d bytes\n",bytes);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "difficulty campaign fixture started with an interface or queued command");
    NMainLoop::CreateProfile("UpstreamDifficulty");NMainLoop::SetActiveProfile("UpstreamDifficulty");
    struct QueueCleanup {~QueueCleanup(){while(NMainLoop::HaveInterfaceCommand()){CObj<NMainLoop::CInterfaceCommand> command=NMainLoop::TakeInterfaceCommandForHarness();}}} cleanup;
    CDBIterator<NDb::CGlobalMap> maps(*NDatabase::GetTable<NDb::CGlobalMap>());int campaigns=0;
    while(maps.MoveNext()) {
        auto* map=maps.Get();if(!IsValid(map->pScenario) || !IsValid(map->pStartZone))continue;
        vector<CObj<NRPG::CGlobalPlayer>> players{NRPG::CreateGlobalPlayer(vector<int>{54})};
        for(int id:{-1,1,2,3}) {
            auto* chosen=id<0 ? nullptr : NDb::GetDBDifficulty(id);
            CObj<NGame::CICBeginGame> begin=new NGame::CICBeginGame(map->GetRecordID(),players,chosen);begin->Exec();
            CObj<NMainLoop::CInterfaceCommand> queued=NMainLoop::TakeInterfaceCommandForHarness();
            auto* mission=dynamic_cast<NGame::CICBeginMission*>(queued.GetPtr());
            Require(mission && !NMainLoop::HaveInterfaceCommand(),"native campaign begin did not queue exactly one intro mission");
            auto* global=(mission->*BeginMissionCommandAccess::Game()).GetPtr();
            Require(global && global->pDifficulty==(chosen ? chosen : originalDefault) && global->players==players &&
                global->bGlobalMapSet && global->nGlobalMapID==map->GetRecordID(),
                "native campaign begin lost its selected difficulty, roster or campaign map");
            auto* zone=(mission->*BeginMissionCommandAccess::Zone()).GetPtr();
            Require(zone && zone->GetDBZone()==map->pStartZone.GetPtr(),"difficulty forwarding changed the campaign intro destination");
            std::printf("native campaign difficulty: map=%d requested=%d intro=%d actual=%d\n",map->GetRecordID(),id,
                zone->GetDBZone()->GetRecordID(),global->pDifficulty->GetRecordID());
        }
        ++campaigns;
    }
    Require(campaigns>=2,"difficulty fixture did not exercise both authored campaigns");
}

static void WeaponFamiliarity()
{
    CObj<NRPG::CUnit> unit = new NRPG::CUnit;
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem;
    CObj<NRPG::CWeaponItem> other = new NRPG::CWeaponItem;
    unit->UseWeapon(weapon, 1.f, 100, .1f);
    unit->UseWeapon(weapon, 2.f, 100, .1f);
    Require(fabs(unit->GetWeaponAdaptation(weapon) - .2f) < .0001f,
        "familiarity bonus uses training rate instead of the DB multiplier");
    unit->UseWeapon(weapon, .5f, 100, .1f);
    Require(fabs(unit->GetWeaponAdaptation(weapon) - .25f) < .0001f,
        "changing training rate changed familiarity scaling");
    unit->UseWeapon(weapon, 200.f, 100, .1f);
    Require(fabs(unit->GetWeaponAdaptation(weapon) - 10.f) < .0001f,
        "retail familiarity cap changed");
    Require(unit->GetWeaponAdaptation(other) == 0.f, "familiarity leaked to another weapon");
}

static void ClueSlotSelection()
{
    for (auto type : {NDb::CT_ITEM, NDb::CT_PERSON}) {
        CObj<NDb::CDBScenarioZone> dbZone = new NDb::CDBScenarioZone;
        CObj<NScenario::CScenarioZone> zone = new NScenario::CScenarioZone(dbZone, 1);
        dbZone->templatesIDs = {0, 101, 102};
        std::unordered_map<int, NScenario::CScenarioZone::STemplate> slots;
        slots[102].nEmptyItemSlots = 1;
        slots[102].nEmptyPersonSlots = 1;
        CMemoryStream state;
        { CStructureSaver saver(state, CStructureSaver::WRITE); saver.Add(13, &slots); }
        state.Seek(0);
        { CStructureSaver saver(state, CStructureSaver::READ);
          static_cast<CObjectBase*>(zone.GetPtr())->operator&(saver); }
        CObj<NDb::CDBScenarioClue> record = new NDb::CDBScenarioClue;
        record->clueType = type;
        CObj<NScenario::CScenarioClue> clue = new NScenario::CScenarioClue(record, 1);
        zone->PlaceClue(clue);
        Require(clue->IsPlaced() && clue->GetTemplateID() == 102,
            "zero-capacity map selected for a clue instead of the available slot");
        zone->RemoveClue(clue);
        zone->PlaceClue(clue);
        Require(clue->GetTemplateID() == 102, "returned clue slot was not available for reuse");
    }
}

class FullScenarioGraph: public NScenario::CScenarioFlowChartBase
{
public:
    FullScenarioGraph(): CScenarioFlowChartBase(1, true) { Generate(); }
};

static void ConclusionRouting(const char* databasePath, const char* resourcePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<FullScenarioGraph> graph = new FullScenarioGraph;
    std::vector<CPtr<NScenario::CScenarioClue>> clues;
    graph->GetClues(&clues);
    int checked = 0;
    for (auto& clue : clues) {
        if (clue->GetDBClue()->clueType != NDb::CT_CONCLUSION) continue;
        ++checked;
        Require(clue->IsPlaced(), "abstract conclusion was dropped from the campaign graph");
        Require(clue->GetTemplateID() == 0 && clue->GetParentZones().empty(),
            "abstract conclusion consumed a physical map slot");
    }
    Require(checked > 0, "campaign fixture has no conclusion clues");
}

template<class Network>
static auto CanPlacePK(Network* network, const NAI::SPathPlace& place, int)
    -> decltype(IsBigLockerPlacePassable(network, place))
{ return IsBigLockerPlacePassable(network, place); }
template<class Network>
static bool CanPlacePK(Network* network, const NAI::SPathPlace& place, long)
{ return NAI::IsBigLockerPassable(network, place); }

static void PKPlacement()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    auto* layer = network->GetLayer(0);
    NAI::SPathPlace position(3, 3, 0);
    position.SetPose(NAI::CRAWL);
    position.SetDirection(0);
    Require(!CanPlacePK(network.GetPtr(), position, 0), "PK placement accepted an unlocked layer without a floor");
    layer->tiles[3][3].nPassable = NAI::CP_STAND;
    Require(!CanPlacePK(network.GetPtr(), position, 0), "PK placement ignored blocked footprint neighbors");
    for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x)
        layer->tiles[y][x].nPassable = NAI::CP_STAND;
    for (int direction = 0; direction < 8; ++direction) {
        position.SetDirection(direction);
        Require(CanPlacePK(network.GetPtr(), position, 0), "open standing footprint was rejected");
        NAI::SPathPlace edge(position);
        edge.SetXY(0, 0);
        Require(!CanPlacePK(network.GetPtr(), edge, 0), "PK footprint was clipped at the grid border");
    }
    position.SetLayer(200);
    position.SetFinal(1);
    position.SetPose(NAI::CM_STAND);
    Require(position.IsFinal() && CanPlacePK(network.GetPtr(), position, 0), "flying PK restoration was rejected");
}

static void CoverPerkMultiplier(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    auto* perk=NDb::GetDBPerk(72);
    Require(IsValid(perk) && !perk->params.empty() && std::isfinite(perk->params[0]),
        "cover perk fixture lacks its actual authored parameter");
    std::printf("native cover perk record: id=72 parameter=%g\n",perk->params[0]);std::fflush(stdout);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* anchor=nullptr;
    for(auto entry:units) {
        auto* candidate=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(candidate && !candidate->IsAIUnit() && candidate->CanFight() && !candidate->IsWearingPK() &&
            !candidate->GetUnitRPG()->HasPerk(17)) { anchor=candidate;break; }
    }
    Require(anchor!=nullptr,"cover perk fixture has no ordinary native target without the separate no-cover perk");
    NDb::CRPGPers* persona=nullptr;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext()) {
        auto* candidate=people.Get();
        if(!IsValid(candidate->pClass) || !IsValid(candidate->pBaseValue) || !IsValid(candidate->pModel) ||
            IsValid(candidate->pPanzerklein) || IsValid(candidate->pDefaultWearsPanzerklein))continue;
        CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(candidate->pClass->nPerkTreeID);
        vector<CPtr<NRPG::CPerk>> entries;tree->GetAllPerks(&entries);
        for(auto entry:entries)if(entry->GetDBPerk()->GetRecordID()==72) { persona=candidate;break; }
        if(persona)break;
    }
    Require(persona!=nullptr,"cover perk fixture has no authored ordinary class containing perk72");
    CObj<NRPG::CUnit> rpg=new NRPG::CUnit(persona);CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(rpg);
    const auto position=anchor->GetPosition();
    CObj<NWorld::CUnitServer> shooter=new NWorld::CUnitServer(world,mission,rpg->pModel,
        dynamic_cast<NWorld::CPlayer*>(anchor->GetPlayer()),position);
    Require(!mission->HasPerk(72),"cover perk control already owns the tested ability");
    auto measure=[&](int distance,float cover,bool unit) {
        const CVec3 from=position.GetCP()+CVec3(0,distance*FP_GRID_STEP,1.5f);
        CObj<NRPG::CToHitCalcer> calcer;
        if(unit)calcer=new NRPG::CUnitToHitCalcer(shooter,position.GetPose(),distance,from,position.pos,0,0,
            cover,false,false,VNULL3,NAI::HL_ANY,anchor,0,false);
        else calcer=new NRPG::CTileToHitCalcer(shooter,position.GetPose(),distance,from,0,cover,false,false,
            VNULL3,position.GetCP(),0);
        calcer->Prepare();
        const int ap=shooter->GetAP();const float xp=rpg->fXP;auto* inventory=mission->GetInventory();
        const float area=calcer->GetTArea(),factor=calcer->GetCA();
        Require(std::isfinite(area) && area>0 && std::isfinite(factor),"cover perk native geometry has no finite area/factor");
        Require(shooter->GetAP()==ap && rpg->fXP==xp && mission->GetInventory()==inventory &&
            shooter->GetPosition()==position,"cover calculation changed AP, XP, inventory or world placement");
        return factor;
    };
    struct Reference {int distance;bool unit;float floor,exposed;};vector<Reference> references;
    for(bool unit:{false,true})for(int distance:{5,20,60}) {
        const float floor=measure(distance,0,unit),exposed=measure(distance,100,unit);
        Require(exposed>floor && std::abs(measure(distance,25,unit)-(floor+(exposed-floor)*.25f))<1e-5f,
            "unperked native cover controls lost their real area contribution");
        references.push_back({distance,unit,floor,exposed});
    }
    std::printf("native cover control: persona=%d six tile/unit area-distance cases pass\n",persona->GetRecordID());std::fflush(stdout);
    rpg->GetPerksTree()->AddPerkPoints(1);rpg->GetPerksTree()->TakePerk(72);float multiplier=0;
    Require(mission->HasPerk(72,&multiplier) && multiplier==perk->params[0],"cover fixture did not acquire its authored perk");
    for(auto reference:references)for(float cover:{0.f,25.f,100.f}) {
        const float actual=measure(reference.distance,cover,reference.unit);
        const float expected=reference.floor+multiplier*(reference.exposed-reference.floor)*(cover/100.f);
        std::printf("native cover perk: unit=%d distance=%d cover=%g actual=%g expected=%g\n",
            int(reference.unit),reference.distance,cover,actual,expected);std::fflush(stdout);
        Require(std::abs(actual-expected)<1e-5f,"cover perk discards target area instead of applying its authored multiplier");
    }
}

static void MovingTargetPerk(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;
    for(auto entry:units) {
        auto* candidate=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(candidate && !candidate->IsAIUnit() && candidate->CanFight() && !candidate->IsWearingPK()) { actor=candidate;break; }
    }
    Require(actor!=nullptr,"moving-target fixture has no real ordinary party actor");
    auto* perk=NDb::GetDBPerk(14);
    Require(IsValid(perk) && !perk->params.empty() && std::isfinite(perk->params[0]),
        "moving-target fixture lacks the authored multiplier");
    NDb::CRPGPers* persona=nullptr;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext()) {
        auto* candidate=people.Get();
        if(!IsValid(candidate->pClass) || !IsValid(candidate->pBaseValue) || !IsValid(candidate->pModel) ||
            IsValid(candidate->pPanzerklein) || IsValid(candidate->pDefaultWearsPanzerklein))continue;
        CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(candidate->pClass->nPerkTreeID);
        vector<CPtr<NRPG::CPerk>> entries;tree->GetAllPerks(&entries);
        for(auto entry:entries)if(entry->GetDBPerk()->GetRecordID()==14) { persona=candidate;break; }
        if(persona)break;
    }
    Require(persona!=nullptr,"moving-target fixture has no authored persona with perk14");
    CObj<NRPG::CUnit> rpg=new NRPG::CUnit(persona);
    CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(rpg);
    const auto position=actor->GetPosition();
    CObj<NWorld::CUnitServer> target=new NWorld::CUnitServer(world,mission,rpg->pModel,
        dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()),position);
    Require(!mission->HasPerk(14) && target->CanFight(),"moving-target control unexpectedly starts with its perk or an invalid actor");
    const CVec3 end=position.GetCP();
    auto measure=[&](const CVec3& movement,const CVec3& shotOffset,bool firstRound=false) {
        mission->StartNewTurn(end-movement);
        CObj<NRPG::CUnitToHitCalcer> calcer=new NRPG::CUnitToHitCalcer(actor,actor->GetPosition().GetPose(),4,
            end+shotOffset,position.pos,0,0,100.f,firstRound,false,VNULL3,NAI::HL_BODY,target,0,false);
        const int sourceAP=actor->GetAP(),targetAP=target->GetAP();const float xp=rpg->fXP;
        auto* inventory=mission->GetInventory();const auto actualPosition=target->GetPosition();
        const float result=calcer->GetTMove();
        Require(actor->GetAP()==sourceAP && target->GetAP()==targetAP && rpg->fXP==xp &&
            mission->GetInventory()==inventory && target->GetPosition()==actualPosition,
            "moving-target calculation changed world position, AP, training or inventory");
        return result;
    };
    const CVec3 movement(2,0,0);
    const CVec3 angles[]={CVec3(0,-4,0),CVec3(-4,-4,0),CVec3(4,-4,0)};
    vector<float> baseline;
    for(auto angle:angles) {
        const float value=measure(movement,angle);
        Require(std::isfinite(value) && value>0,"unperked moving-target native geometry control has no projected movement penalty");
        baseline.push_back(value);
    }
    Require(measure(VNULL3,angles[0])==0 && measure(movement,angles[0],true)==0,
        "native stationary/first-round movement controls failed");
    const int maxMove=actor->GetUnitRPG()->GetToHitConstants()->nMaxMoveBonus;
    const CVec3 longMovement((maxMove+10)*FP_GRID_STEP,0,0);
    const float capped=measure(longMovement,angles[0]);
    Require(capped==float(maxMove) && capped>0,"native moving-target distance cap control failed");
    std::printf("native moving-target control: persona=%d authoredParam=%g baseline=%g/%g/%g\n",
        persona->GetRecordID(),perk->params[0],baseline[0],baseline[1],baseline[2]);std::fflush(stdout);
    rpg->GetPerksTree()->AddPerkPoints(1);rpg->GetPerksTree()->TakePerk(14);
    float multiplier=0;
    Require(mission->HasPerk(14,&multiplier) && multiplier==perk->params[0],"moving-target fixture did not acquire its actual authored perk");
    for(int i=0;i<3;++i) {
        const float actual=measure(movement,angles[i]),expected=baseline[i]*multiplier;
        std::printf("native moving-target perk: angle=%d actual=%g expected=%g\n",i,actual,expected);std::fflush(stdout);
        Require(std::abs(actual-expected)<1e-5f,"moving-target perk uses direction cosine instead of its original authored multiplier");
    }
    Require(std::abs(measure(longMovement,angles[0])-capped*multiplier)<1e-5f,
        "moving-target perk changed the original cap-before-multiplier ordering");
    Require(measure(VNULL3,angles[0])==0 && measure(movement,angles[0],true)==0,
        "moving-target perk changed stationary or first-round rules");
}

static void AttackerLevelCriticalRolls(const char* databasePath)
{
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);
    auto* persona=NDb::GetPers(103);auto* armor=NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);
    Require(IsValid(persona) && armor && IsValid(armor->pMaterial),"attacker-level fixture lacks its authored persona/armor");
    int damageType=-1;
    CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());
    while(types.MoveNext()) if(types.Get()->armors[armor->pMaterial->nDR]>0) { damageType=types.Get()->GetRecordID();break; }
    Require(damageType>=0,"attacker-level fixture has no authored flesh damage type");
    class Capture: public ILogNotify {
        OBJECT_NOCOPY_METHODS(Capture);
    public:
        wstring line;
        void OnAddConsoleLine(const SConsoleLine& value) override {
            if(value.eType==STREAM_RPG && value.szText.find(L"\tseverity=")!=wstring::npos)line=value.szText;
        }
    };
    CObj<Capture> log=new Capture;SetLogNotify(log);
    struct Cleanup { ~Cleanup() { SetLogNotify(nullptr); } } cleanup;
    auto hit=[&](int requested,bool critical) {
        CObj<NRPG::CUnit> attacker=new NRPG::CUnit(persona),victim=new NRPG::CUnit(persona);
        CObj<NRPG::IUnitMission> source=NRPG::CreateUnit(attacker),target=NRPG::CreateUnit(victim);
        int level=0;
        if(requested>0) {
            attacker->SetXPLevel(requested);level=source->GetSkillValue(NDb::ST_LEVEL);
            Require(level==requested,"attacker-level fixture did not assign its native requested level");
        }
        // Keep actual critical selection out of this argument/RNG test.
        target->DisableCriticals();
        NRPG::CAttackPortion portion(100000,damageType,0.f,1,1,critical?100:0,7,
            requested>0?source.GetPtr():nullptr,target,1.f,-1);
        const int before=victim->Skills(NDb::ST_VP);auto* inventory=victim->GetInventory();
        const int attackerAP=attacker->Skills(NDb::ST_AP);const float attackerXP=attacker->fXP;
        log->line.clear();
        const float victimXP=victim->fXP;
        auto blocked=portion;blocked.nK=0;auto idle=GlobalGameRandom();
        Require(target->ProcessAttack(nullptr,NAI::HL_BODY,&blocked,nullptr).nDmg==0 &&
            int(victim->Skills(NDb::ST_VP))==before && victim->fXP==victimXP && log->line.empty(),
            "blocked attack entered severity processing or trained/damaged the target");
        auto afterBlocked=GlobalGameRandom();
        for(int i=0;i<4;++i)Require(afterBlocked.Get()==idle.Get(),"blocked damage consumed attacker-level random rolls");
        auto expectedRandom=GlobalGameRandom();int expectedSeverity=7;
        for(int i=0;i<level;++i)expectedSeverity+=expectedRandom.Get(1,20);
        // Original RollCritical uses the supplied severity without a shared random draw.
        expectedRandom.Get(100); // Existing unconsciousness gate.
        const auto expectedNext=expectedRandom;
        const int damage=target->ProcessAttack(nullptr,NAI::HL_BODY,&portion,nullptr).nDmg;
        auto actual=GlobalGameRandom(),expected=expectedNext;
        bool randomMatches=true;
        for(int i=0;i<4;++i)randomMatches&=actual.Get()==expected.Get();
        Require(damage==1 && before-int(victim->Skills(NDb::ST_VP))==1 && !target->IsDead() &&
            !target->IsUnconscious() && inventory==victim->GetInventory() &&
            int(attacker->Skills(NDb::ST_AP))==attackerAP && attacker->fXP==attackerXP && portion.nCrticalDifficulty==7,
            "attacker-level bookkeeping changed base damage, attacker training, inventory or the input portion");
        std::printf("native attacker-level rolls: level=%d critical=%d expectedSeverity=%d sharedRNG=%d\n",
            level,int(critical),expectedSeverity,int(randomMatches));std::fflush(stdout);
        Require(randomMatches,"native damage omitted the attacker-level severity random prefix");
        if(critical) {
            const auto position=log->line.find(L"\tseverity=");
            Require(position!=wstring::npos,"actual critical calculator did not publish its severity input");
            const int received=int(std::wcstol(log->line.c_str()+position+10,nullptr,10));
            std::printf("native critical calculator severity input=%d expected=%d\n",received,expectedSeverity);std::fflush(stdout);
            Require(received==expectedSeverity,"attacker-level severity rolls never reached the actual critical calculator");
        } else Require(log->line.empty(),"zero-probability control unexpectedly entered critical selection");
    };
    hit(0,false);hit(0,true);
    std::puts("native attacker-level controls: anonymous damage/severity and existing shared draws preserved");std::fflush(stdout);
    hit(1,false);
    hit(1,true);hit(3,false);hit(3,true);hit(7,false);hit(7,true);
}

static void HeadBodyDamage(const char* databasePath)
{
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);
    auto* persona=NDb::GetPers(103);auto* armor=NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);
    Require(IsValid(persona) && IsValid(persona->pClass) && !IsValid(persona->pPanzerklein) &&
        armor && IsValid(armor->pMaterial),"head damage fixture lacks its native persona/body armor");
    int damageType=-1;
    CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());
    while(types.MoveNext()) if(types.Get()->armors[armor->pMaterial->nDR]>0) { damageType=types.Get()->GetRecordID();break; }
    Require(damageType>=0,"head damage fixture has no authored flesh damage type");
    auto hit=[&](NAI::EHitLocation location,int expected,int base=40) {
        CObj<NRPG::CUnit> unit=new NRPG::CUnit(persona);
        CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(unit);
        const int before=unit->Skills(NDb::ST_VP);auto* inventory=unit->GetInventory();
        Require(before>80 && unit->nHealedVP==0 && !mission->GetPanzerklein(),
            "head damage fixture needs an ordinary living target without healed HP");
        NRPG::CAttackPortion portion(100000,damageType,0.f,base,base,0,0,nullptr,mission,1.f,-1);
        Require(portion.CanDealDmg(armor) && portion.CalcStructDmg(nullptr,armor,0)==base,
            "head damage control did not retain its exact native base flesh damage");
        auto expectedRandom=GlobalGameRandom();expectedRandom.Get(100);
        const int damage=mission->ProcessAttack(nullptr,location,&portion,nullptr).nDmg;
        auto actualRandom=GlobalGameRandom();
        for(int next=0;next<4;++next)Require(actualRandom.Get()==expectedRandom.Get(),
            "head damage multiplier changed native noncritical damage random draws");
        Require(!mission->IsDead() && !mission->IsUnconscious() && unit->GetInventory()==inventory &&
            damage==before-int(unit->Skills(NDb::ST_VP)),
            "head damage fixture did not apply its reported nonfatal damage to actual VP");
        std::printf("native body-part damage: location=%d base=%d actual=%d expected=%d\n",int(location),base,damage,expected);
        std::fflush(stdout);
        Require(damage==expected,"native head damage does not match the original retail multiplier");
    };
    hit(NAI::HL_BODY,40);hit(NAI::HL_RHAND,28);hit(NAI::HL_LHAND,28);
    hit(NAI::HL_RLEG,34);hit(NAI::HL_LLEG,34);
    hit(NAI::HL_HEAD,60);
    hit(NAI::HL_HEAD,61,41);hit(NAI::HL_HEAD,1,1);
}

static void FreshSkillModifierValues(const char* databasePath)
{
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);
    auto* persona=NDb::GetPers(103);
    Require(IsValid(persona) && IsValid(persona->pClass) && !IsValid(persona->pPanzerklein),
        "fresh-skill fixture has no authored ordinary persona");
    CObj<NRPG::CUnit> unit=new NRPG::CUnit(persona);
    auto* tree=unit->GetPerksTree();tree->AddPerkPoints(1);tree->TakePerk(8);
    Require(unit->HasPerk(8),"fresh-skill fixture has no acquired authored survival perk");
    for(auto skill:{NDb::ST_VP,NDb::ST_AP}) {
        auto& value=unit->Skills(skill);const int cap=value.GetMaxValue();
        Require(cap>10,"fresh-skill fixture needs expendable VP/AP greater than ten");
        value.SetValue(cap/2);const int spent=value;
        { CObj<NRPG::CSkillModifier> bonus=new NRPG::CSkillModifier(&value,NRPG::SSkillModifyInfo(1.5f,0));
          Require(value.GetMaxValue()>cap && int(value)==spent,
              "native expendable VP/AP control refilled points when only the cap increased"); }
        Require(value.GetMaxValue()==cap && int(value)==spent,
            "removing an expendable-stat bonus refilled VP/AP");
    }
    std::puts("native fresh-skill control: VP/AP retain expended current values across cap modifiers");std::fflush(stdout);
    const int originalIC=unit->Skills(NDb::ST_IC);
    unit->Skills(NDb::ST_VP).SetValue(unit->Skills(NDb::ST_VP).GetTheoreticalMax()/5);
    CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(unit);
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_VP));
    const int cap=unit->Skills(NDb::ST_IC).GetMaxValue();
    Require(mission->HasCritical(NDb::C_VP) && cap>originalIC,
        "native survival perk control did not attach its real IC modifier");
    std::printf("native fresh survival modifier: effectiveIC=%d cap=%d original=%d\n",mission->GetIC(),cap,originalIC);
    std::fflush(stdout);
    Require(mission->GetIC()==cap,"fresh static skill retains a stale current value instead of its modified cap");
    mission->RemoveCritical(NDb::C_VP);
    Require(mission->GetIC()==originalIC,"removing the survival modifier did not restore effective IC");
    const float xp=unit->fXP;
    auto roundTrip=[](NRPG::CDynamicSkill* cell) {
        CMemoryStream fields;
        { CStructureSaver writer(fields,CStructureSaver::WRITE);static_cast<CObjectBase*>(cell)->operator&(writer); }
        fields.Seek(0);CObj<NRPG::CDynamicSkill> restored=new NRPG::CDynamicSkill;
        { CStructureSaver reader(fields,CStructureSaver::READ);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
        return restored;
    };
    int staticSkills=0;
    for(int skill=0;skill<NDb::SKILL_TYPE_NUMBERS;++skill) {
        if(skill==NDb::ST_VP || skill==NDb::ST_AP)continue;
        auto& value=unit->Skills(skill);
        const int original=value.GetMaxValue(),earned=value.GetXPPart();const float progress=value.GetProgress();
        for(float factor:{1.5f,0.5f}) {
            { CObj<NRPG::CSkillModifier> modifier=new NRPG::CSkillModifier(&value,NRPG::SSkillModifyInfo(factor,0));
              Require(int(value)==value.GetMaxValue(),"fresh static skill did not follow a positive/negative modifier"); }
            Require(int(value)==original && value.GetMaxValue()==original && value.GetXPPart()==earned &&
                value.GetProgress()==progress && unit->fXP==xp,
                "removing a static-skill modifier lost its effective value or changed training");
        }
        auto restored=roundTrip(&value);
        CObj<NRPG::CSkillModifier> bonus=new NRPG::CSkillModifier(restored,NRPG::SSkillModifyInfo(1.5f,0));
        Require(int(*restored)==restored->GetMaxValue(),"saved fresh skill lost its existing freeze flag");
        ++staticSkills;
    }
    Require(staticSkills==15,"fresh skill fixture did not exercise every non-expendable skill");
    CObj<NRPG::CDynamicSkill> legacy=new NRPG::CDynamicSkill(20,20);legacy->SetValue(7);
    auto restored=roundTrip(legacy);
    CObj<NRPG::CSkillModifier> legacyBonus=new NRPG::CSkillModifier(restored,NRPG::SSkillModifyInfo(1.5f,0));
    Require(restored->GetMaxValue()==30 && int(*restored)==7,
        "fresh-unit initialization retroactively changed an existing saved unfrozen skill");
    std::puts("native fresh-skill GREEN: 15 static skills follow modifiers and restore; saved old unfrozen cell stays expendable");
}

static void TimedCriticalTreatment(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(IsValid(unit) && !unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->HasCommand() && !unit->IsPerformingAction()) {actor=unit;break;}
    Require(actor,"timed critical fixture lacks a real ordinary roster actor");
    const bool medicDC = mode>=15 && mode<=19;
    const bool patientDC = mode>=20 && mode<=24;
    const bool criticalFirst = mode>=25 && mode<=27;
    CObj<NWorld::CUnitServer> physician;
    if(mode==5 || mode==12 || medicDC || criticalFirst) {
        const int perk=mode==5?62:mode==12?59:56;NDb::CRPGPers* persona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
        while(people.MoveNext()) {
            auto* candidate=people.Get();if(!IsValid(candidate->pClass) || !IsValid(candidate->pModel) || !IsValid(candidate->pBaseValue) || !IsValid(candidate->pName) || candidate->pClass->skills[NDb::ST_LEVEL]<=0 || IsValid(candidate->pPanzerklein) || IsValid(candidate->pDefaultWearsPanzerklein))continue;
            CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(candidate->pClass->nPerkTreeID);vector<CPtr<NRPG::CPerk>> entries;tree->GetAllPerks(&entries);
            for(auto entry:entries)if(entry->GetDBPerk()->GetRecordID()==perk) {persona=candidate;break;}if(persona)break;
        }
        Require(persona,"timed critical fixture lacks its authored healing-perk class");
        physician=world->AddUnitInGame(actor->GetPosition().pos.p,NRPG::CreateUnit(persona),actor->GetTBSPlayer());actor=physician;
        Require(actor && actor->CanFight(),"timed critical fixture could not deploy its authored physician");
        auto* unit=actor->GetUnitRPG()->GetRPGUnit();if(mode!=16 && !criticalFirst){unit->GetPerksTree()->AddPerkPoints(1);unit->GetPerksTree()->TakePerk(perk);}Require(unit->HasPerk(perk)==(mode!=16 && !criticalFirst),"timed critical fixture did not acquire or withhold its real healing ability");
    }
    NDb::CRPGPers* patientPersona=NDb::GetPers(103);
    if(patientDC) {
        patientPersona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
        while(people.MoveNext()) {
            auto* candidate=people.Get();if(!IsValid(candidate->pClass) || !IsValid(candidate->pModel) || !IsValid(candidate->pBaseValue) || !IsValid(candidate->pName) || candidate->pClass->skills[NDb::ST_LEVEL]<=0 || IsValid(candidate->pPanzerklein) || IsValid(candidate->pDefaultWearsPanzerklein))continue;
            CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(candidate->pClass->nPerkTreeID);vector<CPtr<NRPG::CPerk>> entries;tree->GetAllPerks(&entries);
            for(auto entry:entries)if(entry->GetDBPerk()->GetRecordID()==24){patientPersona=candidate;break;}if(patientPersona)break;
        }
        Require(patientPersona,"patient DC fixture lacks its authored perk24 class");
    }
    CObj<NWorld::CUnitServer> retained=world->AddUnitInGame(actor->GetPosition().pos.p,NRPG::CreateUnit(patientPersona),actor->GetTBSPlayer());NWorld::CUnitServer* patient=retained;
    if(patientDC) {
        auto* unit=patient->GetUnitRPG()->GetRPGUnit();
        if(mode!=21){unit->GetPerksTree()->AddPerkPoints(1);unit->GetPerksTree()->TakePerk(24);}
        Require(unit->HasPerk(24)==(mode!=21),"patient DC fixture did not acquire or withhold actual perk24");
    }
    Require(patient && patient->CanFight() && patient->GetPlayer()==actor->GetPlayer(),"timed critical fixture could not deploy a real friendly patient");
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),1.5f),&places);bool placed=false;
    for(auto place:places) {
        place.SetPose(NAI::CM_STAND);if(!network->IsPassable(place) || network->IsLocked(place,false))continue;
        auto position=patient->GetPosition();position.pos=NAI::SPosition(place,network);const float distance=fabs(position.GetCP()-actor->GetPosition().GetCP());
        if(distance<.3f || !NWorld::IsWithinHumanReach(actor->GetPosition().GetCP(),position.GetCP(),.8f))continue;
        world->GetTime()->Set(world->GetTime()->GetValue()+1);patient->SetPosition(position);patient->animator.AlignTime();patient->animator.PlaceUnit(position);placed=true;break;
    }
    Require(placed,"timed critical fixture has no reachable native patient placement");
    const int effect=criticalFirst && mode!=27?int(NDb::FAE_CRITICAL_FIRST):int(NDb::FAE_NORMAL);
    NDb::CRPGFirstAid* record=nullptr;CDBIterator<NDb::CRPGFirstAid> kits(*NDatabase::GetTable<NDb::CRPGFirstAid>());
    while(kits.MoveNext()){auto* candidate=kits.Get();if(IsValid(candidate->pItem) && (criticalFirst?candidate->GetRecordID()==(mode==27?1:3):int(candidate->effect)==effect && candidate->GetRecordID()==1) && candidate->nQuantity>0){record=candidate;break;}}
    Require(record,"timed critical fixture lacks its authored critical-only/critical-first/normal item");
    if(criticalFirst){std::printf("native critical-first item: mode=%d kit=%d actualKind=%d expectedKind=%d capacity=%d required=%d modifier=%d\n",mode,record->GetRecordID(),int(record->effect),effect,record->nTotalHealVP,record->nRequiedSkill,record->nSkillModifier);std::fflush(stdout);}
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);auto* healer=actor->GetUnitRPG()->GetRPGUnit();auto* target=patient->GetUnitRPG();auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous;
    if(inventory->Get(NDb::SLOT_1))previous=inventory->TakeOff(NDb::SLOT_1);
    if(mode!=6)Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"timed critical fixture could not activate its actual medical item");else inventory->Activate(NDb::SLOT_1);
    actor->animator.SetActiveItem(mode!=6);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());MarkNewDGFrame();world->GetAIMap()->Sync();
    auto readCriticals=[](NRPG::IUnitMission* mission){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(mission,&refs);vector<CObj<NRPG::CCritical>> stored;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(5,&stored);}vector<NRPG::SCritical> result;for(auto entry:stored)result.push_back(entry->GetCritical());return result;};
    const auto type=(mode==12 || mode==13)?NDb::C_BLEEDING:NDb::C_WEAPONSKILL_REDUCTION;
    auto hasSelected=[&](){for(const auto& entry:readCriticals(target))if(entry.eCritical==type && entry.eCl==NDb::CL_HEAD)return true;return false;};
    const bool medicCritical=mode==0 || mode==1 || mode==7 || mode==9 || mode==10 || mode==14;
    if(medicCritical)actor->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_LEGS,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,1));
    const int dcLimit=healer->GetFirstAidDC(kit);
    const float dcScale=float(NRPG::N_MAX_DC)/NRPG::N_MAX_SKILL;float dcBonus=0;
    const int medicine=healer->Skills(NDb::ST_MEDICINE);
    const int baseLimit=int(dcScale*(medicine+record->nSkillModifier));
    if(medicDC && mode!=16)Require(healer->HasPerk(56,&dcBonus) && dcBonus>0,"critical DC fixture lacks its real positive physician perk56");
    float patientMultiplier=1;
    if(patientDC) {
        Require(medicine==17 && !healer->HasPerk(56),"patient DC fixture needs the actual unmodified roster medic");
        if(mode!=21)Require(target->GetRPGUnit()->HasPerk(24,&patientMultiplier) && patientMultiplier>1,"patient DC fixture lacks its real positive multiplier");
    }
    const int improvedLimit=int(dcScale*(int(int(medicine+dcBonus)*patientMultiplier)+record->nSkillModifier));
    if(medicDC && mode!=16)Require(improvedLimit>baseLimit,"physician DC perk does not extend the native critical threshold");
    const int dc=patientDC?(mode==22?improvedLimit+1:mode==23?baseLimit:mode==24?improvedLimit:baseLimit+1):medicDC?(mode==17?improvedLimit+1:mode==18?baseLimit:mode==19?improvedLimit:baseLimit+1):(mode==3 || mode==11 || mode==12 || mode==13)?dcLimit+1:mode==2?1:20;
    if(patientDC){std::printf("native patient DC threshold: mode=%d medicine=%d multiplier=%g base=%d improved=%d generic=%d critical=%d\n",mode,medicine,patientMultiplier,baseLimit,improvedLimit,dcLimit,dc);std::fflush(stdout);}
    if(medicDC){std::printf("native physician DC threshold: mode=%d medicine=%d bonus=%g base=%d improved=%d actual=%d critical=%d\n",mode,medicine,dcBonus,baseLimit,improvedLimit,dcLimit,dc);std::fflush(stdout);}
    Require(dcLimit>=20,"timed critical fixture's actual medicine/kit cannot select its baseline critical");
    const int initialDC=(mode==16 || mode==17 || mode==21 || mode==22)?1:(mode==3 || mode==4 || mode==13)?20:dc;
    target->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,type,-1,2.f,initialDC));
    if(mode==14)target->ApplyCritical(NRPG::SCritical(NDb::CL_TORSO,type,-1,3.f,1));
    if(mode==9 || mode==11 || criticalFirst){auto& vp=target->GetRPGUnit()->Skills(NDb::ST_VP);vp.SetValue(vp.GetMaxValue()-3);target->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_VP));}
    const int rawVP=target->GetRPGUnit()->Skills(NDb::ST_VP),bandage=target->GetRPGUnit()->nHealedVP,ap=actor->GetAP(),quantity=kit->GetIncQuantity();
    const bool selected=mode!=3 && mode!=4 && mode!=9 && mode!=11 && mode!=13 && mode!=16 && mode!=17 && mode!=21 && mode!=22 && mode!=26 && mode!=27;
    float speed=1,parameter=0;if(healer->HasPerk(62,&parameter) && parameter>0)speed=parameter;
    const int total=selected?(std::max)(1,Float2Int(dc*50.f/(std::max)(1,int(healer->Skills(NDb::ST_MEDICINE)))/speed)):(effect==int(NDb::FAE_CRITICAL_ONLY)?1:0);
    struct Selection {NDb::ECritical type=NDb::C_NONE;NDb::ECriticalLocation location=NDb::CL_ANY;int dc=0;};
    struct Snapshot {bool pk=false;int hp=0,healed=0,maxHP=0,pkHP=0,maxPKHP=0,ap=0,maxAP=0,sight=0;bool unit=false;};
    struct Progress {int remaining=-999;Selection selection;Snapshot snapshot;};
    auto state=[](NWorld::CUnitServer* unit){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(unit,&refs);CObj<NWorld::CUnitState> result;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(10,&result);}return result;};
    auto progress=[](NWorld::CUnitState* unit){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(unit,&refs);Progress result;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(5,&result.remaining);reader.Add(6,&result.selection);reader.Add(7,&result.snapshot);}return result;};
    CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,patient);auto random=GlobalGameRandom();const auto result=executor->CanDoIt(actor->GetPosition());const int cost=executor->GetStartAP();
    std::printf("native timed critical query: mode=%d kit=%d effect=%d medicine=%d dcLimit=%d dc=%d selected=%d speed=%g total=%d result=%d AP=%d cost=%d\n",mode,record->GetRecordID(),effect,int(healer->Skills(NDb::ST_MEDICINE)),dcLimit,dc,int(selected),speed,total,int(result),ap,cost);std::fflush(stdout);
    if(mode==6){Require(result==NWorld::UCR_UNAVAILABLE && actor->GetAP()==ap && kit->GetIncQuantity()==quantity && hasSelected() && random.Get()==GlobalGameRandom().Get(),"missing timed-treatment item changed the patient/AP/charge/RNG");return;}
    Require(result==NWorld::UCR_OK && ap>=cost && !world->IsRealTime(),"timed critical fixture cannot afford its actual native turn-based medical action");
    executor->Run();
    if(mode==3 || mode==4 || mode==13 || mode==16 || mode==17 || mode==21 || mode==22 || mode==26) {Require(target->RemoveCritical(type),"in-flight critical control did not remove its initial eligible effect");if(mode!=4 && mode!=26)target->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,type,-1,2.f,dc));}
    const int available=actor->GetAP();int expectedRemaining=(std::max)(0,total-available);NRPG::SFirstAid woundPlan;int woundAP=0;
    if(mode==9 || mode==11 || mode==26 || mode==27)Require(healer->CreateFirstAid(&woundPlan,available,record->nTotalHealVP,kit,target->GetRPGUnit(),&woundAP),"wound-priority/fallback fixture cannot plan its real small dressing");
    executor->AnimationFinished();
    const int expectedAP=available-((mode==9 || mode==11 || mode==26 || mode==27)?woundAP:(std::min)(total,available));
    std::printf("native timed critical started: mode=%d AP=%d expectedAP=%d patient=%d marker=%d medic=%d expectedRemaining=%d charge=%d\n",mode,actor->GetAP(),expectedAP,int(hasSelected()),int(target->HasCritical(NDb::C_PATIENT)),int(actor->GetUnitRPG()->HasCritical(NDb::C_WEAPONSKILL_REDUCTION)),expectedRemaining,kit->GetIncQuantity());std::fflush(stdout);
    Require(actor->GetAP()==expectedAP && actor->GetUnitRPG()->HasCritical(NDb::C_WEAPONSKILL_REDUCTION)==medicCritical,criticalFirst?"first-aid effect import lost authored critical-first treatment priority":patientDC?"critical treatment ignored the original patient DC perk24":medicDC?"critical treatment ignored the original physician DC perk56":"timed critical treatment omitted original AP progress or cured its medic");
    Require(int(target->GetRPGUnit()->Skills(NDb::ST_VP))==rawVP,"timed critical treatment changed raw patient health");
    if(mode==9 || mode==11 || mode==26 || mode==27) {
        Require(hasSelected()==(mode!=26) && target->GetRPGUnit()->nHealedVP==bandage+3,"normal wound-first or critical-first wound fallback lost its native dressing/critical priority");
        actor->SetState(new NWorld::CUnitStateNormal(actor));Require(kit->GetIncQuantity()==quantity-1 && !target->HasCritical(NDb::C_PATIENT),"wound route failed native cancellation cleanup");return;
    }
    Require(hasSelected()==(selected?expectedRemaining>0:mode!=4) && target->GetRPGUnit()->nHealedVP==bandage,"timed critical treatment cured before its AP budget or lost an ineligible critical");
    Require(target->HasCritical(NDb::C_PATIENT)==(expectedRemaining>0) && kit->GetIncQuantity()==quantity-(expectedRemaining?0:1),"timed critical treatment ended its patient/charge ownership before the AP budget");
    if(expectedRemaining>0) {
        auto active=state(actor);Require(dynamic_cast<NWorld::CUnitStateHealer*>(active.GetPtr()),"timed critical action did not retain the native healer state");auto saved=progress(active);
        std::printf("native timed critical progress: mode=%d remaining=%d expected=%d type=%d location=%d dc=%d snapshotHP=%d expectedHP=%d\n",mode,saved.remaining,expectedRemaining,int(saved.selection.type),int(saved.selection.location),saved.selection.dc,saved.snapshot.hp,rawVP);std::fflush(stdout);
        Require(saved.remaining==expectedRemaining && saved.selection.type==type && saved.selection.location==NDb::CL_HEAD && saved.selection.dc==dc && saved.snapshot.hp==rawVP,"timed critical state did not save its original AP/selection/compatible target snapshot");
        actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);Require(progress(state(actor)).remaining==expectedRemaining && actor->GetAP()==0 && hasSelected(),"same-turn finish spent the timed treatment budget twice");
        if(mode==7){actor->SetState(new NWorld::CUnitStateNormal(actor));Require(hasSelected() && !target->HasCritical(NDb::C_PATIENT) && kit->GetIncQuantity()==quantity-1,"cancelling timed critical treatment cured the patient or leaked its dose/marker");return;}
        if(mode==1) {
            CStructureNetworkContext refs;auto fields=BorrowedObjectFields(active,&refs);CObj<NWorld::CUnitState> restored=dynamic_cast<NWorld::CUnitState*>(pSSClasses->CreateObject(pSSClasses->GetObjectTypeID(active)));
            Require(restored,"timed critical saved state factory failed");{CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);}
            const auto copy=progress(restored);Require(copy.remaining==expectedRemaining && copy.selection.dc==dc && copy.snapshot.hp==rawVP,"saved timed critical state lost its native progress");
            CStructureNetworkContext ownerRefs;auto ownerFields=BorrowedObjectFields(actor,&ownerRefs);const int id=ownerRefs.nextID++;ownerRefs.ids[restored]=id;ownerRefs.refs[id]=restored;
            CMemoryStream patch;{CStructureSaver writer(patch,CStructureSaver::WRITE,&ownerRefs);writer.Add(10,&restored);}patch.Seek(0);
            {CStructureSaver reader(patch,CStructureSaver::READ,&ownerRefs);static_cast<CObjectBase*>(actor)->operator&(reader);}
            Require(state(actor).GetPtr()==restored.GetPtr() && actor->GetAP()==0 && target->HasCritical(NDb::C_PATIENT) && kit->GetIncQuantity()==quantity,"restoring native healer state restarted or finished its treatment");
        }
        int turns=0;
        while(expectedRemaining>0 && turns<8) {
            actor->OnTBSEvent(NWorld::TBS_START_NEW_TURN);const int budget=actor->GetAP(),spent=(std::min)(budget,expectedRemaining);Require(budget>0,"native new turn did not replenish the medic's AP");
            actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);expectedRemaining-=spent;++turns;
            std::printf("native timed critical turn: mode=%d turn=%d budget=%d spent=%d remaining=%d AP=%d patient=%d\n",mode,turns,budget,spent,expectedRemaining,actor->GetAP(),int(hasSelected()));std::fflush(stdout);
            Require(actor->GetAP()==budget-spent && hasSelected()==(expectedRemaining>0) && target->HasCritical(NDb::C_PATIENT)==(expectedRemaining>0),"native timed treatment turn did not spend exactly its remaining AP before curing the patient");
            if(expectedRemaining>0)Require(progress(state(actor)).remaining==expectedRemaining,"native timed treatment lost its remaining budget between turns");
        }
        Require(expectedRemaining==0,"native timed critical treatment failed to finish within its actual finite AP budget");
    }
    if(mode==14){bool other=false;for(const auto& entry:readCriticals(target))if(entry.eCritical==type && entry.eCl==NDb::CL_TORSO)other=true;Require(other,"timed critical treatment cured another body location");}
    Require(kit->GetIncQuantity()==quantity-1 && !target->HasCritical(NDb::C_PATIENT) && !actor->IsPerformingAction() && !actor->HasCommand() && !patient->HasCommand(),"timed treatment did not finish native dose/patient/action ownership");
    std::printf("native timed critical complete: mode=%d total=%d selected=%d remaining=%d patient=%d charge=%d->%d\n",mode,total,int(selected),expectedRemaining,int(hasSelected()),quantity,kit->GetIncQuantity());std::fflush(stdout);
}

struct SRepairFinishSnapshot
{
    int idle=0,flags=0,weaponFlags=0,welder=0;
    bool healing=false,walking=false,activeItem=false;
    STime end=0;
    vector<CObj<NAnimation::CAnimation>> clips;
};

static SRepairFinishSnapshot RepairFinishSnapshot(NWorld::CUnitServer* actor)
{
    SRepairFinishSnapshot result;
    auto* state=dynamic_cast<NAnimation::CSkeletonState*>(actor->animator.GetSkeletonState());Require(state,"repair finish has no actual skeleton state");
    result.idle=state->state.pIdleAnimation?state->state.pIdleAnimation->GetRecordID():0;result.flags=state->state.cIdleBannedFlags;
    PainSoundVisitor sound;actor->NWorld::CDumbUnitServer::Visit(&sound);
    for(auto* entry:sound.sounds)if(entry && entry->GetRecordID()==0x4057)++result.welder;
    CStructureNetworkContext refs;refs.include=[](CObjectBase*){return false;};CMemoryStream fields;
    {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);actor->animator.operator&(writer);}fields.Seek(0);
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(3,&result.end);reader.Add(4,&result.weaponFlags);reader.Add(9,&result.walking);reader.Add(22,&result.activeItem);reader.Add(25,&result.healing);}
    CStructureNetworkContext graph;graph.include=[&](CObjectBase* object){if(auto* clip=dynamic_cast<NAnimation::CAnimation*>(object))result.clips.push_back(clip);return dynamic_cast<NAnimation::CAnimator*>(object)!=nullptr || dynamic_cast<NAnimation::CSkeletonAnimator*>(object)!=nullptr;};
    CMemoryStream traversal;{CStructureSaver writer(traversal,CStructureSaver::WRITE,&graph);static_cast<CObjectBase*>(actor->animator.GetSkeletonAnimator())->operator&(writer);}
    return result;
}

// Observe the real global dispatcher without replacing its existing DB-backed listeners.
class TreatmentAckObserver: public NWorld::IAck
{
    OBJECT_NOCOPY_METHODS(TreatmentAckObserver);
public:
    int good=0,bad=0;
    NWorld::CUnitServer* healer=nullptr;
    NWorld::CUnitServer* patient=nullptr;
    void OnCannotFinishHeal(NWorld::CUnitServer* who,NWorld::CUnitServer* target) override {++bad;healer=who;patient=target;}
    void OnHealFinished(NWorld::CUnitServer* who,NWorld::CUnitServer* target) override {++good;healer=who;patient=target;}
};

static CObj<TreatmentAckObserver> ObserveTreatmentAck(NWorld::CWorld* world)
{
    auto* dispatcher=world->GetGlobalAck();CStructureNetworkContext refs;auto fields=BorrowedObjectFields(dispatcher,&refs);
    vector<CObj<NWorld::IAck>> listeners;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&listeners);}
    Require(!listeners.empty(),"treatment ack fixture lost the actual native listeners");
    CObj<TreatmentAckObserver> observer=new TreatmentAckObserver;
    const int id=refs.nextID++;refs.ids[observer.GetPtr()]=id;refs.refs[id]=observer.GetPtr();listeners.push_back(observer.GetPtr());
    CMemoryStream patch;{CStructureSaver writer(patch,CStructureSaver::WRITE,&refs);writer.Add(2,&listeners);}patch.Seek(0);
    {CStructureSaver reader(patch,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(dispatcher)->operator&(reader);}
    return observer;
}

// Build a genuine medic of the same player, earning perk58 through its native prerequisites.
static CObj<NWorld::CUnitServer> QualifiedMedicalActor(NWorld::CWorld* world,NWorld::CUnitServer* anchor)
{
    auto* oldRPG=anchor->GetUnitRPG()->GetRPGUnit();NDb::CRPGPers* persona=nullptr;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext()){
        auto* p=people.Get();if(!p->pClass || !p->pModel || !p->pBaseValue || !p->pName || p->pClass->skills[NDb::ST_LEVEL]<=0 || p->pPanzerklein || p->pDefaultWearsPanzerklein || (!oldRPG->bDisabled && p->pSide!=oldRPG->GetPers()->pSide))continue;
        CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(p->pClass->nPerkTreeID);vector<CPtr<NRPG::CPerk>> perks;tree->GetAllPerks(&perks);bool found=false;for(auto perk:perks)if(perk->GetDBPerk()->GetRecordID()==58)found=true;
        if(found && (!persona || p->GetRecordID()<persona->GetRecordID()))persona=p;
    }
    Require(persona,"medical requirements lack an authored medic class with perk58");
    CObj<NRPG::CUnit> rpg=new NRPG::CUnit(persona,0,false,0,0,0,oldRPG->bDisabled);CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(rpg);
    auto* tree=rpg->GetPerksTree();vector<CPtr<NRPG::CPerk>> perks;tree->GetAllPerks(&perks);NRPG::CPerk* required=nullptr;for(auto perk:perks)if(perk->GetDBPerk()->GetRecordID()==58)required=perk;
    Require(required && !required->IsTaken(),"medical requirements lost its untaken native ability");vector<CPtr<NRPG::CPerk>> path;
    auto collect=[&](auto&& self,NRPG::CPerk* perk)->void{for(int i=0;i<perk->GetParents().GetSize();++i)self(self,perk->GetParents()[i]);for(auto known:path)if(known.GetPtr()==perk)return;path.push_back(perk);};collect(collect,required);
    for(int level=1;tree->GetPerkPoints()<int(path.size()) && level<=20;++level)rpg->SetXPLevel(level);
    for(auto perk:path){if(perk->IsTaken())continue;Require(perk->CanTake(),"medical requirements could not satisfy a native prerequisite");tree->TakePerk(perk->GetDBPerk()->GetRecordID());}
    Require(rpg->HasPerk(58) && rpg->bDisabled==oldRPG->bDisabled,"medical requirements did not earn perk58 or changed medal eligibility");
    auto result=world->AddUnitInGame(anchor->GetPosition().pos.p,mission,anchor->GetTBSPlayer());Require(result && result->CanFight() && result->GetPlayer()==anchor->GetPlayer(),"medical requirements could not deploy its actual qualified medic under the same player");
    // Preserve a real held weapon for the existing wrong-item rejection controls.
    auto* source=anchor->GetUnitRPG()->GetInventory();if(dynamic_cast<NRPG::CWeaponItem*>(source->GetActive())){
        CObj<NRPG::IInventoryItem> held=source->TakeOff(static_cast<NDb::ESlot>(source->GetActiveSlot()));auto* target=mission->GetInventory();CObj<NRPG::IInventoryItem> previous;
        if(target->Get(NDb::SLOT_1))previous=target->TakeOff(NDb::SLOT_1);Require(target->Equip(NDb::SLOT_1,held) && target->Activate(NDb::SLOT_1),"qualified medic could not retain the real rejection-control weapon");
    }
    std::printf("native qualified medical actor: persona=%d medicine=%d engineering=%d level=%d path=%d remainingPoints=%d perk58=%d medalsDisabled=%d\n",persona->GetRecordID(),int(rpg->Skills(NDb::ST_MEDICINE)),int(rpg->Skills(NDb::ST_ENGINEERING)),int(rpg->Skills(NDb::ST_LEVEL)),int(path.size()),tree->GetPerkPoints(),int(rpg->HasPerk(58)),int(rpg->bDisabled));std::fflush(stdout);
    return result;
}

static void ProneMedicalReach(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->HasCommand() && !unit->IsPerformingAction()){actor=unit;break;}
    Require(actor,"prone medical reach lacks its actual ordinary medic");CObj<NWorld::CUnitServer> qualified;if(mode==6){qualified=QualifiedMedicalActor(world,actor);actor=qualified;}
    const bool healerProne=mode>=7,prone=mode==2 || mode==3 || mode==4 || mode==6 || mode==8;
    if(healerProne){auto pos=actor->GetPosition();pos.pos.p.SetPose(NAI::CM_LAY);world->GetTime()->Set(world->GetTime()->GetValue()+1);actor->SetPosition(pos);actor->animator.AlignTime();actor->animator.PlaceUnit(pos);}
    CObj<NWorld::CUnitServer> patient;
    const int zone=mode==0 || mode==2?0:mode==4?2:1;const float desired=zone==0?.625f:zone==1?.9f:1.25f;
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),2.f),&places);bool found=false;float best=1e30f;unsigned bestBits=0xffffffffu;NAI::SUnitPosition targetPosition;int ground=0,standing=0,all=0,flat=0;
    for(auto point:places){
        if(!point.IsIntegral() || point.IsFinal())continue;++ground;point.SetPose(NAI::CM_STAND);
        if(!network->IsPassable(point) || network->IsLocked(point,false))continue;++standing;bool allPoses=false;
        for(int direction=0;direction<8 && !allPoses;++direction){point.SetDirection(direction);bool clear=true;for(auto pose:{NAI::CM_STAND,NAI::CM_LAY,NAI::CM_CROUCH}){auto candidate=point;candidate.SetPose(pose);if(!network->IsPassable(candidate) || network->IsLocked(candidate,false))clear=false;}if(clear)allPoses=true;}if(!allPoses)continue;++all;
        auto position=actor->GetPosition();position.pos=NAI::SPosition(point,network);const auto delta=position.GetCP()-actor->GetPosition().GetCP();const float distance=std::sqrt(delta.x*delta.x+delta.y*delta.y);
        if(!NWorld::IsWithinHumanReach(actor->GetPosition().GetCP(),position.GetCP(),2.f))continue;++flat;if(zone==0?distance<=.3f || distance>.8f:zone==1?distance<=.8f || distance>1.2f:distance<=1.2f || distance>1.5f)continue;
        const float error=std::fabs(distance-desired);if(error>best || error==best && point.GetBits()>=bestBits)continue;
        position.pos.p.SetPose(prone?NAI::CM_LAY:mode==5?NAI::CM_CROUCH:NAI::CM_STAND);targetPosition=position;best=error;bestBits=point.GetBits();found=true;
    }
    std::printf("native prone medical setup: mode=%d actorLayer=%d places=%d ground=%d standing=%d allPoses=%d verticalAllowed=%d found=%d\n",mode,int(actor->GetPosition().pos.p.GetLayer()),int(places.size()),ground,standing,all,flat,int(found));std::fflush(stdout);
    Require(found,"prone medical reach has no actual free all-pose-passable point in the required distance band");patient=world->AddUnitInGame(targetPosition.pos.p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer());Require(patient && patient->CanFight(),"prone medical reach cannot deploy its real friendly patient at the selected free point");world->GetTime()->Set(world->GetTime()->GetValue()+1);patient->SetPosition(targetPosition);patient->animator.AlignTime();patient->animator.PlaceUnit(targetPosition);
    Require(!patient->IsDead() && !patient->IsUnconscious() && patient->GetPosition().pos.p.GetPose()==(prone?NAI::CM_LAY:mode==5?NAI::CM_CROUCH:NAI::CM_STAND),"prone medical patient is not in its actual conscious authored pose");
    auto* record=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(1);Require(record && record->nRequiedSkill==0 && record->nRequiredPerkID==0,"prone medical reach lost its unrestricted authored normal kit");CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);
    auto* inventory=actor->GetUnitRPG()->GetInventory();vector<CObj<NRPG::IInventoryItem>> previous;for(auto slot:{NDb::SLOT_1,NDb::SLOT_2})if(inventory->Get(slot))previous.push_back(inventory->TakeOff(slot));Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"prone medical reach cannot activate its actual kit");
    auto* target=patient->GetUnitRPG();target->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,20));Require(target->HasCurableCriticals(),"prone medical reach lacks its actual curable patient condition");
    MarkNewDGFrame();world->GetAIMap()->Sync();auto* healer=actor->GetUnitRPG()->GetRPGUnit();const auto from=actor->GetPosition(),to=patient->GetPosition();const auto delta=to.GetCP()-from.GetCP();const float distance=std::sqrt(delta.x*delta.x+delta.y*delta.y);const int ap=actor->GetAP(),patientAP=patient->GetAP(),hp=target->GetTotalVP(),healerHP=actor->GetUnitRPG()->GetTotalVP(),dose=kit->GetIncQuantity(),medicine=healer->Skills(NDb::ST_MEDICINE),points=healer->GetPerksTree()->GetPerkPoints();const float xp=healer->fXP;auto random=GlobalGameRandom();
    const bool close=NWorld::IsWithinHumanReach(from.GetCP(),to.GetCP(),.8f),wide=NWorld::IsWithinHumanReach(from.GetCP(),to.GetCP(),1.2f);Require((zone==0?close:zone==1?!close && wide:!wide),"prone medical reach changed its real geometry before the order probe");
    const auto expected=mode==1 || mode==4 || mode==5 || mode==7?NWorld::UCR_TARGET_OUT_OF_RANGE:NWorld::UCR_OK;CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,patient);const auto result=executor->CanDoIt(from),repeat=executor->CanDoIt(from);auto afterRandom=GlobalGameRandom();
    std::printf("native prone medical reach: mode=%d actorPose=%d targetPose=%d has58=%d distance=%g dz=%g close=%d wide=%d zone=%d result=%d repeat=%d expected=%d AP=%d patientAP=%d dose=%d rawHP=%d medicine=%d points=%d\n",mode,int(from.pos.p.GetPose()),int(to.pos.p.GetPose()),int(healer->HasPerk(58)),distance,delta.z,int(close),int(wide),zone,int(result),int(repeat),int(expected),ap,patientAP,dose,hp,medicine,points);std::fflush(stdout);
    Require(actor->GetAP()==ap && patient->GetAP()==patientAP && target->GetTotalVP()==hp && actor->GetUnitRPG()->GetTotalVP()==healerHP && kit->GetIncQuantity()==dose && inventory->GetActive()==kit.GetPtr() && actor->GetPosition()==from && patient->GetPosition()==to && int(healer->Skills(NDb::ST_MEDICINE))==medicine && healer->GetPerksTree()->GetPerkPoints()==points && healer->fXP==xp && random.Get()==afterRandom.Get() && target->HasCurableCriticals(),"prone medical reach query spent AP/dose or changed health/item/positions/skills/points/XP/RNG/critical state");
    Require(result==expected && repeat==expected,"medical reach ignores the actual prone patient pose or changes standing/crouched/outer-range controls");
}

static void ProneMedicalApproach(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->HasCommand() && !unit->IsPerformingAction()){actor=unit;break;}
    Require(actor,"prone medical approach lacks its actual ordinary medic");CObj<NWorld::CUnitServer> qualified;if(mode==6){qualified=QualifiedMedicalActor(world,actor);actor=qualified;}
    const bool healerProne=mode>=7,prone=mode==2 || mode==3 || mode==4 || mode==6 || mode==8;
    if(healerProne){auto pos=actor->GetPosition();pos.pos.p.SetPose(NAI::CM_LAY);world->GetTime()->Set(world->GetTime()->GetValue()+1);actor->SetPosition(pos);actor->animator.AlignTime();actor->animator.PlaceUnit(pos);}
    CObj<NWorld::CUnitServer> patient;
    const int zone=mode==0 || mode==2?0:mode==4?2:1;const float desired=zone==0?.625f:zone==1?.9f:1.25f;
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),2.f),&places);bool found=false;float best=1e30f;unsigned bestBits=0xffffffffu;NAI::SUnitPosition targetPosition;int ground=0,standing=0,all=0,flat=0;
    for(auto point:places){
        if(!point.IsIntegral() || point.IsFinal())continue;++ground;point.SetPose(NAI::CM_STAND);
        if(!network->IsPassable(point) || network->IsLocked(point,false))continue;++standing;bool allPoses=false;
        for(int direction=0;direction<8 && !allPoses;++direction){point.SetDirection(direction);bool clear=true;for(auto pose:{NAI::CM_STAND,NAI::CM_LAY,NAI::CM_CROUCH}){auto candidate=point;candidate.SetPose(pose);if(!network->IsPassable(candidate) || network->IsLocked(candidate,false))clear=false;}if(clear)allPoses=true;}if(!allPoses)continue;++all;
        auto position=actor->GetPosition();position.pos=NAI::SPosition(point,network);const auto delta=position.GetCP()-actor->GetPosition().GetCP();const float distance=std::sqrt(delta.x*delta.x+delta.y*delta.y);
        if(!NWorld::IsWithinHumanReach(actor->GetPosition().GetCP(),position.GetCP(),2.f))continue;++flat;if(zone==0?distance<=.3f || distance>.8f:zone==1?distance<=.8f || distance>1.2f:distance<=1.2f || distance>1.5f)continue;
        const float error=std::fabs(distance-desired);if(error>best || error==best && point.GetBits()>=bestBits)continue;
        position.pos.p.SetPose(prone?NAI::CM_LAY:mode==5?NAI::CM_CROUCH:NAI::CM_STAND);targetPosition=position;best=error;bestBits=point.GetBits();found=true;
    }
    std::printf("native prone medical approach setup: mode=%d actorLayer=%d places=%d ground=%d standing=%d allPoses=%d verticalAllowed=%d found=%d\n",mode,int(actor->GetPosition().pos.p.GetLayer()),int(places.size()),ground,standing,all,flat,int(found));std::fflush(stdout);
    Require(found,"prone medical approach has no actual free all-pose-passable point in the required distance band");patient=world->AddUnitInGame(targetPosition.pos.p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer());Require(patient && patient->CanFight(),"prone medical approach cannot deploy its real friendly patient at the selected free point");world->GetTime()->Set(world->GetTime()->GetValue()+1);patient->SetPosition(targetPosition);patient->animator.AlignTime();patient->animator.PlaceUnit(targetPosition);
    Require(!patient->IsDead() && !patient->IsUnconscious() && patient->GetPosition().pos.p.GetPose()==(prone?NAI::CM_LAY:mode==5?NAI::CM_CROUCH:NAI::CM_STAND),"prone medical patient is not in its actual conscious authored pose");
    auto* record=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(1);Require(record && record->nRequiedSkill==0 && record->nRequiredPerkID==0,"prone medical approach lost its unrestricted authored normal kit");CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);
    auto* inventory=actor->GetUnitRPG()->GetInventory();vector<CObj<NRPG::IInventoryItem>> previous;for(auto slot:{NDb::SLOT_1,NDb::SLOT_2})if(inventory->Get(slot))previous.push_back(inventory->TakeOff(slot));Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"prone medical approach cannot activate its actual kit");
    auto* target=patient->GetUnitRPG();target->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,20));Require(target->HasCurableCriticals(),"prone medical approach lacks its actual curable patient condition");
    MarkNewDGFrame();world->GetAIMap()->Sync();auto* healer=actor->GetUnitRPG()->GetRPGUnit();const auto from=actor->GetPosition(),to=patient->GetPosition();const auto delta=to.GetCP()-from.GetCP();const float distance=std::sqrt(delta.x*delta.x+delta.y*delta.y);const int ap=actor->GetAP(),patientAP=patient->GetAP(),hp=target->GetTotalVP(),healerHP=actor->GetUnitRPG()->GetTotalVP(),dose=kit->GetIncQuantity(),medicine=healer->Skills(NDb::ST_MEDICINE),points=healer->GetPerksTree()->GetPerkPoints();const float xp=healer->fXP;auto random=GlobalGameRandom();
    const bool close=NWorld::IsWithinHumanReach(from.GetCP(),to.GetCP(),.8f),wide=NWorld::IsWithinHumanReach(from.GetCP(),to.GetCP(),1.2f);Require((zone==0?close:zone==1?!close && wide:!wide),"prone medical approach changed its real geometry before the order probe");

    CObj<NWorld::CExecHeal> direct=new NWorld::CExecHeal(actor,patient);const auto current=direct->CanDoIt(from);
    const bool atGoal=current==NWorld::UCR_OK && NWorld::IsWithinHumanReach(from.GetCP(),to.GetEyePosition(),prone?1.2f:.8f);
    NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCmdHeal> command=new NWorld::CCmdHeal(patient);
    CObj<NWorld::CCommandExecute> executor=NWorld::CreateActionExecutor(actor,command,&result);auto* mover=dynamic_cast<NWorld::IExecMove*>(executor.GetPtr());
    NAI::SPathPlace destination=from.pos.p;NAI::EFindPathParams params=NAI::PF_DEFAULT;NWorld::ENeedActiveItem active=NWorld::ITEM_NO_MATTER;
    if(mover)mover->GetDesiredPlace(&destination,&params,&active);CObj<NAI::CPath> path=executor?executor->GetCurrentPath():nullptr;
    std::printf("native prone medical approach: mode=%d actorPose=%d targetPose=%d has58=%d distance=%g current=%d executor=%d result=%d mover=%d path=%d points=%d sourceX=%d sourceY=%d targetX=%d targetY=%d goalX=%d goalY=%d atGoal=%d expectedAtGoal=%d AP=%d patientAP=%d dose=%d medicine=%d\n",mode,int(from.pos.p.GetPose()),int(to.pos.p.GetPose()),int(healer->HasPerk(58)),distance,int(current),int(bool(executor)),int(result),int(bool(mover)),int(bool(path)),path?int(path->points.size()):0,int(from.pos.p.GetX()),int(from.pos.p.GetY()),int(to.pos.p.GetX()),int(to.pos.p.GetY()),int(destination.GetX()),int(destination.GetY()),int(mover && destination.GetX()==from.pos.p.GetX() && destination.GetY()==from.pos.p.GetY()),int(atGoal),ap,patientAP,dose,medicine);std::fflush(stdout);
    std::printf("native prone medical approach context: mode=%d eyeDistance=%g eyeClose=%d eyeWide=%d params=%d active=%d goalBits=%u pathBits=%u\n",mode,fabs(to.GetEyePosition()-from.GetCP()),int(NWorld::IsWithinHumanReach(from.GetCP(),to.GetEyePosition(),.8f)),int(NWorld::IsWithinHumanReach(from.GetCP(),to.GetEyePosition(),1.2f)),int(params),int(active),destination.GetBits(),path && !path->points.empty()?path->points.back().GetBits():0);std::fflush(stdout);
    Require(executor && mover && path && !path->points.empty() && result==NWorld::UCR_OK,"prone medical approach cannot create a real native heal queue/path on its legal geometry");
    Require(destination.GetX()==path->points.back().GetX() && destination.GetY()==path->points.back().GetY() && destination.GetLayer()==path->points.back().GetLayer(),"prone medical approach lost its actual native path endpoint");
    auto endpoint=from;endpoint.pos=NAI::SPosition(destination,network);Require(direct->CanDoIt(endpoint)==NWorld::UCR_OK,"prone medical approach queues a destination where actual healing is unavailable");
    auto afterRandom=GlobalGameRandom();Require(actor->GetAP()==ap && patient->GetAP()==patientAP && target->GetTotalVP()==hp && actor->GetUnitRPG()->GetTotalVP()==healerHP && kit->GetIncQuantity()==dose && inventory->GetActive()==kit.GetPtr() && actor->GetPosition()==from && patient->GetPosition()==to && int(healer->Skills(NDb::ST_MEDICINE))==medicine && healer->GetPerksTree()->GetPerkPoints()==points && healer->fXP==xp && random.Get()==afterRandom.Get() && target->HasCurableCriticals() && !actor->HasCommand(),"prone medical approach planning changed actual world/AP/HP/item/positions/skills/XP/RNG/orders");
    const bool same=destination.GetX()==from.pos.p.GetX() && destination.GetY()==from.pos.p.GetY() && destination.GetLayer()==from.pos.p.GetLayer();
    Require(same==atGoal,"heal command unnecessarily moves an already-in-reach medic instead of using the original prone-patient approach radius");
}

static void MedicalOrderPose(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game,mode==5 || mode==8?4517:810);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->HasCommand() && !unit->IsPerformingAction()){actor=unit;break;}
    Require(actor,"medical pose lacks its actual ordinary actor");CObj<NWorld::CUnitServer> qualified;
    if(mode>=6){qualified=QualifiedMedicalActor(world,actor);actor=qualified;}
    auto* network=dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());Require(network,"medical pose lacks the actual native path network");auto from=actor->GetPosition();int ladderLayer=-1,ladderIndex=-1,ladderCount=0;
    const bool ladderCase=mode==5 || mode==8;
    if(ladderCase){
        for(int layer=0;layer<network->GetNumLayers() && ladderLayer<0;++layer){
            auto* nodes=network->GetLayer(layer);ladderCount+=int(nodes->ladders.size());
            for(int index=0;index<int(nodes->ladders.size());++index){
                const auto& ladder=nodes->ladders[index];auto bottom=ladder.placeOnBottom;bottom.SetPose(NAI::CM_STAND);
                if(ladder.pointPassable.size()<2 || !bottom.IsIntegral() || bottom.IsFinal() || !network->IsPassable(bottom) || network->IsLocked(bottom,false))continue;
                NAI::SPathPlace rung(index,0,layer,(4+2*int(ladder.eDir))&7,NAI::CM_STAND,0);rung.SetIntegral(0);
                if(network->GetPassability(rung)!=NAI::AIP_YES)continue;
                NAI::SPosition rungPosition(rung,network),bottomPosition(bottom,network);if(fabs(rungPosition.GetCP()-bottomPosition.GetCP())>1e-4f)continue;
                auto placed=actor->GetPosition();placed.pos=bottomPosition;world->GetTime()->Set(world->GetTime()->GetValue()+1);actor->SetPosition(placed);actor->animator.AlignTime();actor->animator.PlaceUnit(placed);
                from=placed;from.pos=rungPosition;ladderLayer=layer;ladderIndex=index;break;
            }
        }
        std::printf("native medical ladder setup: mode=%d seen=%d layer=%d index=%d\n",mode,ladderCount,ladderLayer,ladderIndex);std::fflush(stdout);
        Require(ladderLayer>=0,"medical pose fixture has no actual passable map ladder with a free bottom landing");
    }
    if(mode==1)from.pos.p.SetPose(NAI::CM_CROUCH);
    if(mode==2)from.pos.p.SetPose(NAI::CM_LAY);
    if(mode==3 || mode==6){auto fly=actor->GetPosition().pos;Require(NAI::MakeFlyPos(actor->GetPosition().GetCP(),network,&fly),"medical pose could not encode an actual fly preview");from.pos=fly;}
    if(mode==4 || mode==7)from.pos.p.SetPose(NAI::CM_INACTIVE);
    Require(NWorld::IsWithinHumanReach(from.GetCP(),actor->GetPosition().GetCP(),.8f),"medical pose preview is outside actual self-treatment reach");
    Require(from.pos.p.IsFinal()==(mode==3 || mode==6) && from.pos.p.IsIntegral()!=ladderCase,"medical pose lost its actual ground/fly/ladder precondition");
    auto* record=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(1);Require(record && record->nRequiedSkill==0 && record->nRequiredPerkID==0,"medical pose lost the unrestricted authored normal kit");
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);auto* patient=actor->GetUnitRPG();auto* rpg=patient->GetRPGUnit();auto* inventory=patient->GetInventory();vector<CObj<NRPG::IInventoryItem>> previous;
    for(auto slot:{NDb::SLOT_1,NDb::SLOT_2})if(inventory->Get(slot))previous.push_back(inventory->TakeOff(slot));Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"medical pose could not activate the actual kit");
    patient->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,20));Require(patient->HasCurableCriticals(),"medical pose has no actual curable self-target critical");
    MarkNewDGFrame();world->GetAIMap()->Sync();const auto original=actor->GetPosition();const auto bits=from.pos.p.GetBits();const int ap=actor->GetAP(),hp=patient->GetTotalVP(),dose=kit->GetIncQuantity(),medicine=rpg->Skills(NDb::ST_MEDICINE),points=rpg->GetPerksTree()->GetPerkPoints();const float xp=rpg->fXP;auto random=GlobalGameRandom();
    const auto expected=mode>=3 && mode<=8?NWorld::UCR_GENERAL_FAILURE:NWorld::UCR_OK;CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,actor);const auto result=executor->CanDoIt(from),repeat=executor->CanDoIt(from);auto afterRandom=GlobalGameRandom();
    std::printf("native medical pose: mode=%d final=%d integral=%d pose=%d has58=%d layer=%d ladder=%d delta=%g result=%d repeat=%d expected=%d AP=%d dose=%d rawHP=%d medicine=%d points=%d\n",mode,int(from.pos.p.IsFinal()),int(from.pos.p.IsIntegral()),int(from.pos.p.GetPose()),int(rpg->HasPerk(58)),ladderLayer,ladderIndex,fabs(from.GetCP()-actor->GetPosition().GetCP()),int(result),int(repeat),int(expected),ap,dose,hp,medicine,points);std::fflush(stdout);
    Require(actor->GetAP()==ap && patient->GetTotalVP()==hp && kit->GetIncQuantity()==dose && inventory->GetActive()==kit.GetPtr() && actor->GetPosition()==original && from.pos.p.GetBits()==bits && int(rpg->Skills(NDb::ST_MEDICINE))==medicine && rpg->GetPerksTree()->GetPerkPoints()==points && rpg->fXP==xp && random.Get()==afterRandom.Get() && patient->HasCurableCriticals(),"medical pose query spent AP/dose or changed health/item/position/preview/skills/perks/XP/RNG/critical state");
    Require(result==expected && repeat==expected,"medical order accepts a fly, ladder or inactive pose, or perk58 bypasses the position gate");
}

static void MedicalRequirements(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->HasCommand() && !unit->IsPerformingAction()){actor=unit;break;}
    Require(actor && int(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_MEDICINE))==17,"medical requirements lack the native unmodified Medicine17 roster actor");
    CObj<NWorld::CUnitServer> qualified;if(mode==5 || mode==6){qualified=QualifiedMedicalActor(world,actor);actor=qualified;}
    auto* healer=actor->GetUnitRPG()->GetRPGUnit();const int ids[]={10,10,7,6,8,14,26,10,10,1,7};const int id=ids[mode];auto* record=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(id);
    Require(record && record->pItem,"medical requirements lost its authored kit");
    if(mode==1 || mode==2 || mode==4)for(int level=1;int(healer->Skills(NDb::ST_MEDICINE))+record->nSkillModifier<record->nRequiedSkill && level<=20;++level)healer->SetXPLevel(level);
    const int medicine=healer->Skills(NDb::ST_MEDICINE),engineering=healer->Skills(NDb::ST_ENGINEERING);const bool bypass=healer->HasPerk(58);
    if(mode==1 || mode==2 || mode==3)Require(medicine+record->nSkillModifier==record->nRequiedSkill,"medical requirements missed its actual skill/modifier equality boundary");
    if(mode==4)Require(medicine+record->nSkillModifier>=record->nRequiedSkill && record->nRequiredPerkID==58 && !bypass,"medical requirements missing-perk control still fails the skill precondition");
    if(mode==5)Require(bypass && medicine+record->nSkillModifier<record->nRequiedSkill,"medical requirements perk58 bypass control already meets the high native skill requirement");
    if(mode==6)Require(bypass && engineering+record->nSkillModifier<record->nRequiedSkill && record->effect==NDb::FAE_REPAIR_PK,"medical requirements repair control does not separate engineering from perk58");
    if(mode==0 || mode==10)Require(!bypass && medicine+record->nSkillModifier<record->nRequiedSkill,"medical requirements low-skill control already qualifies");
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);auto* inventory=actor->GetUnitRPG()->GetInventory();vector<CObj<NRPG::IInventoryItem>> previous;
    for(auto slot:{NDb::SLOT_1,NDb::SLOT_2})if(inventory->Get(slot))previous.push_back(inventory->TakeOff(slot));
    if(mode!=8)Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"medical requirements cannot equip its actual kit");else inventory->Activate(NDb::SLOT_1);
    if(mode==7)while(!kit->IsEmpty())kit->SpendPotion();
    auto* patient=actor->GetUnitRPG();patient->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,20));Require(patient->HasCurableCriticals(),"medical requirements lacks its actual curable self-target condition");
    MarkNewDGFrame();world->GetAIMap()->Sync();const auto position=actor->GetPosition();const int ap=actor->GetAP(),hp=patient->GetTotalVP(),quantity=kit->GetIncQuantity(),points=healer->GetPerksTree()->GetPerkPoints();const float xp=healer->fXP;auto random=GlobalGameRandom();
    const auto expected=mode==0 || mode==6 || mode==10?NWorld::UCR_NEED_HIGHER_SKILL:mode==4 || mode==7?NWorld::UCR_NO_EQUIPMENT:mode==8?NWorld::UCR_UNAVAILABLE:NWorld::UCR_OK;
    CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,actor);const auto result=executor->CanDoIt(position),repeat=executor->CanDoIt(position);auto afterRandom=GlobalGameRandom();
    std::printf("native medical requirements: mode=%d kit=%d effect=%d medicine=%d engineering=%d modifier=%d required=%d requiredPerk=%d has58=%d level=%d result=%d repeat=%d expected=%d AP=%d dose=%d rawHP=%d points=%d\n",mode,id,int(record->effect),medicine,engineering,record->nSkillModifier,record->nRequiedSkill,record->nRequiredPerkID,int(bypass),int(healer->Skills(NDb::ST_LEVEL)),int(result),int(repeat),int(expected),ap,quantity,hp,points);std::fflush(stdout);
    Require(actor->GetAP()==ap && patient->GetTotalVP()==hp && kit->GetIncQuantity()==quantity && int(healer->Skills(NDb::ST_MEDICINE))==medicine && int(healer->Skills(NDb::ST_ENGINEERING))==engineering && healer->HasPerk(58)==bypass && healer->GetPerksTree()->GetPerkPoints()==points && healer->fXP==xp && actor->GetPosition()==position && random.Get()==afterRandom.Get() && patient->HasCurableCriticals(),"medical requirements query changed native AP/HP/item/skills/perks/points/XP/position/RNG/critical state");
    Require(result==expected && repeat==expected,"medical order ignores skill/modifier/item-perk requirements or perk58 rules");
}

static void MedicalEffectEligibility(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->HasCommand() && !unit->IsPerformingAction()){actor=unit;break;}
    Require(actor,"medical eligibility lacks its real ordinary medic");
    const int kitID=mode>=4 && mode<=6?3:mode>=7 && mode<=9 || mode==20 || mode==22?9:mode>=10 && mode<=12?17:mode==13?12:mode==14?15:mode==15 || mode==19?7:1;
    auto* record=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(kitID);Require(record && IsValid(record->pItem),"medical eligibility lost its actual authored kit");
    CObj<NWorld::CUnitServer> physician;
    if(record->nRequiredPerkID>0){
        NDb::CRPGPers* persona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
        while(people.MoveNext()){
            auto* p=people.Get();if(!p->pClass || !p->pModel || !p->pBaseValue || !p->pName || p->pClass->skills[NDb::ST_LEVEL]<=0 || p->pPanzerklein || p->pDefaultWearsPanzerklein)continue;
            CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(p->pClass->nPerkTreeID);vector<CPtr<NRPG::CPerk>> perks;tree->GetAllPerks(&perks);bool found=false;for(auto perk:perks)if(perk->GetDBPerk()->GetRecordID()==record->nRequiredPerkID)found=true;
            if(found && (!persona || p->GetRecordID()<persona->GetRecordID()))persona=p;
        }
        Require(persona,"medical eligibility has no real class with the actual kit ability");physician=world->AddUnitInGame(actor->GetPosition().pos.p,NRPG::CreateUnit(persona),actor->GetTBSPlayer());actor=physician;Require(actor && actor->CanFight(),"medical eligibility cannot deploy its native qualified physician");
        auto* rpg=actor->GetUnitRPG()->GetRPGUnit();auto* tree=rpg->GetPerksTree();vector<CPtr<NRPG::CPerk>> perks;tree->GetAllPerks(&perks);NRPG::CPerk* required=nullptr;for(auto perk:perks)if(perk->GetDBPerk()->GetRecordID()==record->nRequiredPerkID)required=perk;
        Require(required,"medical eligibility lost the real class ability");vector<CPtr<NRPG::CPerk>> path;
        auto collect=[&](auto&& self,NRPG::CPerk* perk)->void{for(int i=0;i<perk->GetParents().GetSize();++i)self(self,perk->GetParents()[i]);for(auto known:path)if(known.GetPtr()==perk)return;path.push_back(perk);};collect(collect,required);
        for(int level=1;tree->GetPerkPoints()<int(path.size()) && level<=20;++level)rpg->SetXPLevel(level);
        for(auto perk:path){if(perk->IsTaken())continue;Require(perk->CanTake(),"medical eligibility cannot satisfy a native prerequisite");tree->TakePerk(perk->GetDBPerk()->GetRecordID());}
        Require(rpg->HasPerk(record->nRequiredPerkID),"medical eligibility did not acquire its actual item ability");
    }
    CObj<NWorld::CUnitServer> retained;NWorld::CUnitServer* patient=nullptr;
    if(mode==18){for(auto unit:units)if(unit && unit->CanFight() && unit->GetDiplomacyState(actor)==NDb::DS_ENEMY){patient=unit;break;}}
    else {retained=world->AddUnitInGame(actor->GetPosition().pos.p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer());patient=retained;}
    Require(patient && patient->CanFight(),"medical eligibility lacks its real patient/rejection target");
    if(mode!=18){
        auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),1.5f),&places);bool placed=false;
        for(auto place:places){place.SetPose(NAI::CM_STAND);if(!network->IsPassable(place) || network->IsLocked(place,false))continue;auto pos=patient->GetPosition();pos.pos=NAI::SPosition(place,network);if(fabs(pos.GetCP()-actor->GetPosition().GetCP())<.3f || !NWorld::IsWithinHumanReach(actor->GetPosition().GetCP(),pos.GetCP(),.8f))continue;world->GetTime()->Set(world->GetTime()->GetValue()+1);patient->SetPosition(pos);patient->animator.AlignTime();patient->animator.PlaceUnit(pos);placed=true;break;}
        Require(placed,"medical eligibility has no free native patient approach");
    }
    auto* healer=actor->GetUnitRPG()->GetRPGUnit();
    for(int level=1;int(healer->Skills(NDb::ST_MEDICINE))+record->nSkillModifier<record->nRequiedSkill && level<=20;++level)healer->SetXPLevel(level);
    const int medicine=healer->Skills(NDb::ST_MEDICINE);
    Require(medicine+record->nSkillModifier>=record->nRequiedSkill,"medical eligibility native class progression did not qualify for the actual kit");
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous;
    if(inventory->Get(NDb::SLOT_1))previous=inventory->TakeOff(NDb::SLOT_1);
    if(mode!=17)Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"medical eligibility cannot equip its actual kit");else inventory->Activate(NDb::SLOT_1);
    if(mode==16)while(!kit->IsEmpty())kit->SpendPotion();
    auto* target=patient->GetUnitRPG();auto* unit=target->GetRPGUnit();auto& hp=unit->Skills(NDb::ST_VP);
    const int baseQuota=int(float(NRPG::N_MAX_VP)/NRPG::N_MAX_SKILL*medicine),itemQuota=int(float(NRPG::N_MAX_VP)/NRPG::N_MAX_SKILL*(medicine+record->nSkillModifier));
    const int wound=mode==15 || mode==19?itemQuota+10:mode==3 || mode==20?hp.GetMaxValue()/2:mode==24?3:0;
    Require(wound<hp.GetMaxValue(),"medical eligibility wound exceeds its actual patient health");hp.SetValue(hp.GetMaxValue()-wound);
    if(wound)target->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_VP));
    if(mode==15 || mode==19 || mode==24){NRPG::SFirstAid prior;prior.fdVP=mode==15?baseQuota:mode==19?itemQuota:3;prior.nMaxVP=999;target->HealVP(prior);}
    const bool noncurable=mode==2 || mode==6 || mode==8 || mode==23;
    const bool curable=mode==1 || mode==5 || mode==9 || mode==11 || mode==21 || mode==22 || mode==23;
    if(noncurable)target->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_PATIENT));
    if(curable)target->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,mode==21?999:20));
    if(mode==12)target->ApplyCritical(NRPG::SCritical(NDb::CL_TORSO,NDb::C_BLEEDING,-1,1.f,1));
    if(mode==22)target->SuspendCriticals(2);
    auto criticals=[&](){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(target,&refs);vector<CObj<NRPG::CCritical>> result;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(5,&result);}return result;};
    const auto before=criticals();int suspendable=0;for(auto entry:before)if(entry->CanBeSuspended())++suspendable;
    Require((suspendable>0)==(curable && mode!=22 || mode==12),"medical eligibility lacks its actual suspendable/non-suspendable critical precondition");
    Require(!noncurable || target->HasCritical(NDb::C_PATIENT),"medical eligibility lost its noncurable patient status control");
    MarkNewDGFrame();world->GetAIMap()->Sync();const auto position=actor->GetPosition(),targetPosition=patient->GetPosition();
    const int ap=actor->GetAP(),patientAP=patient->GetAP(),rawHP=hp,dressed=unit->nHealedVP,quantity=kit->GetIncQuantity();const float xp=healer->fXP;auto random=GlobalGameRandom();
    const bool cannot=mode==0 || mode==2 || mode==4 || mode==6 || mode==7 || mode==8 || mode==10 || mode==11 || mode==19 || mode==20 || mode==22;
    const auto expected=mode==16?NWorld::UCR_NO_EQUIPMENT:mode==17?NWorld::UCR_UNAVAILABLE:mode==18?NWorld::UCR_GENERAL_FAILURE:cannot?NWorld::UCR_CANT_HEAL:NWorld::UCR_OK;
    CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,patient);const auto result=executor->CanDoIt(position),repeat=executor->CanDoIt(position);auto afterRandom=GlobalGameRandom();const auto after=criticals();
    std::printf("native medical eligibility: mode=%d kit=%d effect=%d medicine=%d modifier=%d required=%d requiredPerk=%d hasRequiredPerk=%d level=%d wound=%d dressed=%d baseQuota=%d itemQuota=%d suspendable=%d bleeding=%d patientStatus=%d result=%d repeat=%d expected=%d AP=%d patientAP=%d dose=%d rawHP=%d criticals=%d\n",mode,kitID,int(record->effect),medicine,record->nSkillModifier,record->nRequiedSkill,record->nRequiredPerkID,int(record->nRequiredPerkID==0 || healer->HasPerk(record->nRequiredPerkID)),int(healer->Skills(NDb::ST_LEVEL)),wound,dressed,baseQuota,itemQuota,suspendable,int(target->HasCritical(NDb::C_BLEEDING)),int(target->HasCritical(NDb::C_PATIENT)),int(result),int(repeat),int(expected),ap,patientAP,quantity,rawHP,int(before.size()));std::fflush(stdout);
    Require(actor->GetAP()==ap && patient->GetAP()==patientAP && int(hp)==rawHP && unit->nHealedVP==dressed && kit->GetIncQuantity()==quantity && int(healer->Skills(NDb::ST_MEDICINE))==medicine && healer->fXP==xp && actor->GetPosition()==position && patient->GetPosition()==targetPosition && random.Get()==afterRandom.Get() && before.size()==after.size(),"medical eligibility query changed native AP/HP/dressing/item/skill/XP/position/RNG/criticals");
    for(int i=0;i<int(before.size());++i)Require(before[i].GetPtr()==after[i].GetPtr(),"medical eligibility query replaced an actual critical");
    Require(result==expected && repeat==expected,"medical eligibility ignores the authored effect or kit-adjusted wound capacity");
}

static void TreatmentResultAck(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->HasCommand() && !unit->IsPerformingAction()){actor=unit;break;}
    Require(actor && int(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_MEDICINE))==17,"treatment ack lacks its native unmodified roster medic");
    CObj<NWorld::CUnitServer> retained;NWorld::CUnitServer* patient=actor;
    if(mode!=4){
        retained=world->AddUnitInGame(actor->GetPosition().pos.p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer());patient=retained;
        Require(patient && patient->CanFight(),"treatment ack lacks its native friendly patient");
        auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),1.5f),&places);bool placed=false;
        for(auto place:places){place.SetPose(NAI::CM_STAND);if(!network->IsPassable(place) || network->IsLocked(place,false))continue;auto pos=patient->GetPosition();pos.pos=NAI::SPosition(place,network);if(fabs(pos.GetCP()-actor->GetPosition().GetCP())<.3f || !NWorld::IsWithinHumanReach(actor->GetPosition().GetCP(),pos.GetCP(),.8f))continue;world->GetTime()->Set(world->GetTime()->GetValue()+1);patient->SetPosition(pos);patient->animator.AlignTime();patient->animator.PlaceUnit(pos);placed=true;break;}
        Require(placed,"treatment ack has no free native patient approach");
    }
    auto* record=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(1);Require(record && record->effect==NDb::FAE_NORMAL && record->nAPToUse==0,"treatment ack lost the authored normal kit");
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous;
    if(inventory->Get(NDb::SLOT_1))previous=inventory->TakeOff(NDb::SLOT_1);Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"treatment ack cannot activate its actual kit");
    actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    auto* target=patient->GetUnitRPG();auto* unit=target->GetRPGUnit();auto& hp=unit->Skills(NDb::ST_VP);
    const bool critical=mode>=6;const int wound=critical?0:mode==1?30:mode==2?31:mode==4?4:mode==5?32:29;
    hp.SetValue(hp.GetMaxValue()-wound);
    if(wound)target->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_VP));
    if(mode==5){NRPG::SFirstAid prior;prior.fdVP=2;prior.nMaxVP=999;target->HealVP(prior);}
    if(critical)target->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,20));
    if(mode==3 || mode==7){const int before=patient->GetAP(),cost=patient->GetActionAP(NRPG::AC_ROTATE);patient->DoAction(NRPG::AC_ROTATE);Require(cost>0 && patient->GetAP()==before-cost,"treatment ack did not spend real patient rotation AP");}
    MarkNewDGFrame();world->GetAIMap()->Sync();auto observer=ObserveTreatmentAck(world);
    const int ap=actor->GetAP(),quantity=kit->GetIncQuantity(),rawHP=hp,already=unit->nHealedVP,initialDeficit=wound-already;
    CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,patient);Require(executor->CanDoIt(actor->GetPosition())==NWorld::UCR_OK && !world->IsRealTime(),"treatment ack actual native order is unavailable");
    executor->Run();executor->AnimationFinished();
    const int firstDressed=unit->nHealedVP;
    if(critical){
        Require(target->HasCritical(NDb::C_WEAPONSKILL_REDUCTION) && actor->GetAP()==0 && target->HasCritical(NDb::C_PATIENT),"critical ack fixture did not enter its actual multi-turn treatment");
        if(mode==7)target->RemoveCritical(NDb::C_WEAPONSKILL_REDUCTION);
        if(mode==8){int turns=0;while(target->HasCritical(NDb::C_PATIENT) && turns++<3){actor->OnTBSEvent(NWorld::TBS_START_NEW_TURN);actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);}Require(!target->HasCritical(NDb::C_WEAPONSKILL_REDUCTION) && !target->HasCritical(NDb::C_PATIENT),"critical ack fixture failed to pay/cure across actual turns");}
    } else Require(firstDressed==already+(mode==4?4:15),"wound ack fixture did not apply its native first dressing segment");
    actor->SetState(new NWorld::CUnitStateNormal(actor));
    const bool good=mode==0 || mode==3 || mode==4 || mode==7;
    std::printf("native treatment ack: mode=%d initialDeficit=%d dressedBefore=%d dressedFirst=%d remaining=%d AP=%d->%d patientAP=%d good=%d bad=%d expectedGood=%d expectedBad=%d critical=%d dose=%d->%d\n",mode,initialDeficit,already,firstDressed,wound-unit->nHealedVP,ap,actor->GetAP(),patient->GetAP(),observer->good,observer->bad,int(good),int(!good),int(target->HasCritical(NDb::C_WEAPONSKILL_REDUCTION)),quantity,kit->GetIncQuantity());std::fflush(stdout);
    Require(int(hp)==rawHP && kit->GetIncQuantity()==quantity-1 && !target->HasCritical(NDb::C_PATIENT) && !actor->IsPerformingAction(),"treatment ack changed paid health/dose/patient/action ownership");
    Require(observer->good==int(good) && observer->bad==int(!good) && observer->healer==actor && observer->patient==patient,"treatment result acknowledgment depends on patient AP instead of original healing progress");
}

static void RepairOrderGate(const char* databasePath,const char* resourcePath,int mode,int repairAction=-1,int repairFinish=-1,int repairAck=-1)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* anchor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK()){anchor=unit;break;}
    Require(anchor,"repair order has no actual player anchor");
    CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(anchor->GetPosition().pos.p,NRPG::CreateUnit(NDb::GetPers(3)),anchor->GetTBSPlayer());
    Require(actor && actor->CanFight() && !actor->IsWearingPK(),"repair order could not deploy its real engineer");auto* rpg=actor->GetUnitRPG()->GetRPGUnit();
    if(mode!=5)for(int level=1;int(rpg->Skills(NDb::ST_ENGINEERING))<100 && level<=20;++level)rpg->SetXPLevel(level);
    if(mode!=5 && mode!=6)rpg->GetPerksTree()->TakePerk(79);
    Require(mode==5?int(rpg->Skills(NDb::ST_ENGINEERING))<100:int(rpg->Skills(NDb::ST_ENGINEERING))>=100,"repair order native class progression failed");
    Require(rpg->HasPerk(79)==(mode!=5 && mode!=6),"repair order acquired the wrong native ability");
    NDb::CRPGPers* shellPersona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext()){auto* p=people.Get();if(p->pPanzerklein && !p->pPanzerklein->bHasNoHead && p->pModel && (!shellPersona || p->GetRecordID()<shellPersona->GetRecordID()))shellPersona=p;}
    Require(shellPersona,"repair order has no real ordinary chassis");
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(anchor->GetPosition().GetCP(),20.f),&places);NAI::SPathPlace shellPlace;bool found=false;
    for(auto place:places){place.SetPose(NAI::CM_STAND);place.SetDirection(0);if(NAI::IsBigLockerPlacePassable(network,place)){shellPlace=place;found=true;break;}}
    Require(found,"repair order has no free big native footprint");
    CObj<NWorld::CUnitServer> shell=world->AddUnitInGame(shellPlace,NRPG::CreateUnit(shellPersona),actor->GetTBSPlayer());
    Require(shell && shell->IsEmptyPK(),"repair order could not deploy the actual empty PK");
    CObj<NWorld::CUnitServer> pilot;NWorld::CUnitServer* target=shell;
    if(mode==1 || mode==3 || mode==4 || mode==10 || mode==13 || mode==8) {
        pilot=world->AddUnitInGame(anchor->GetPosition().pos.p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer());Require(pilot && pilot->CanFight(),"repair order has no friendly pilot");
        if(mode!=8){Require(shell->WearAsPK(true),"repair order could not prepare the real suit");pilot->SetPosition(shell->GetPosition());pilot->FlipPanzerklein(shell,false,false);Require(pilot->IsWearingPK() && pilot->GetWearingPK()==shell.GetPtr(),"repair order lost its actual pilot/suit ownership");}
        target=pilot;
    }
    auto& shellHP=shell->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP);Require(shellHP.GetMaxValue()>100,"repair order chassis has insufficient VP");
    shellHP.SetValue(shellHP.GetMaxValue()-((mode==2 || mode==10)?0:100));
    if(repairAction>=0)shellHP.SetValue(shellHP.GetMaxValue()-((repairAction==2 || repairAction==3)?3:(repairAction==4 || repairAction==5)?650:100));
    if(repairAck>=0)shellHP.SetValue(shellHP.GetMaxValue()-(repairAck==1?494:repairAck==2?495:493));
    if(pilot){
        auto* pilotUnit=pilot->GetUnitRPG()->GetRPGUnit();auto& hp=pilotUnit->Skills(NDb::ST_VP);
        const int woundedHP=hp.GetMaxValue()-7,oldBase=hp.GetTheoreticalMax()-hp.GetXPPart();
        // Wearing the real suit changes primary attributes. Stabilize their derived values
        // before injuring the pilot: later sound/Spot practice must not rebase a stale VP cell.
        // Retain the same wounded raw HP and every repair/AP/dose/ack assertion below.
        pilotUnit->UpdateSkills();hp.SetValue(woundedHP);
        Require(int(hp)==woundedHP && hp.GetTheoreticalMax()==10*(int(pilotUnit->Skills(NDb::ST_STR))+5)+hp.GetXPPart(),"repair pilot did not establish canonical worn stats and original wounded raw HP");
        std::printf("REPAIR_PILOT_BASELINE persona=%d oldBase=%d canonicalBase=%d maximum=%d woundedHP=%d STR=%d AP=%d\n",pilotUnit->pPers->GetRecordID(),oldBase,hp.GetTheoreticalMax()-hp.GetXPPart(),hp.GetMaxValue(),woundedHP,int(pilotUnit->Skills(NDb::ST_STR)),int(pilotUnit->Skills(NDb::ST_AP)));
        pilot->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_VP));
    }
    // Put the actual engineer on a free repair ring. Inner invalid previews never relocate him into the suit footprint.
    places.clear();network->GetNearPlaces(SSphere(shell->GetPosition().GetCP(),5.f),&places);NAI::SUnitPosition ring=actor->GetPosition(),farPosition=ring,innerPosition=ring;bool gotRing=false,gotFar=false,gotInner=false;
    for(auto place:places) {
        place.SetPose(NAI::CM_STAND);if(!place.IsIntegral() || place.IsFinal() || !network->IsPassable(place))continue;
        auto position=actor->GetPosition();position.pos=NAI::SPosition(place,network);const float distance=fabs(position.GetCP()-shell->GetPosition().GetCP());
        if(!gotInner && distance>.3f && NWorld::IsWithinHumanReach(position.GetCP(),shell->GetPosition().GetCP(),.8f)){innerPosition=position;gotInner=true;}
        if(network->IsLocked(place,false))continue;
        if(!gotRing && NWorld::IsWithinHumanReach(position.GetCP(),shell->GetPosition().GetCP(),1.8f) && !NWorld::IsWithinHumanReach(position.GetCP(),shell->GetPosition().GetCP(),.8f)){ring=position;gotRing=true;}
        if(!gotFar && distance<3.f && !NWorld::IsWithinHumanReach(position.GetCP(),shell->GetPosition().GetCP(),1.8f)){farPosition=position;gotFar=true;}
    }
    Require(gotRing && gotFar && gotInner,"repair order lacks native outer/innerPosition/farPosition preview positions");
    world->GetTime()->Set(world->GetTime()->GetValue()+1);actor->SetPosition(mode==3?farPosition:ring);actor->animator.AlignTime();actor->animator.PlaceUnit(actor->GetPosition());
    auto queryFrom=mode==4?innerPosition:actor->GetPosition();
    if(mode==9 || mode>=14)target=nullptr;
    if(mode==14)queryFrom.pos.p.SetFinal(1);
    if(mode==15)queryFrom.pos.p.SetIntegral(0);
    if(mode==16)queryFrom.pos.p.SetPose(NAI::CM_INACTIVE);
    if(mode==11){target=nullptr;for(auto unit:units)if(unit && unit->CanFight() && unit->GetDiplomacyState(actor)==NDb::DS_ENEMY){target=unit;break;}Require(target,"repair order lacks a real hostile rejection control");}
    auto* record=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(mode==13 || mode==19?1:mode==18 || repairAction==6?27:26);Require(record && record->effect==(mode==13 || mode==19?NDb::FAE_NORMAL:NDb::FAE_REPAIR_PK),"repair order lost its actual kit type");
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous;
    if(inventory->Get(NDb::SLOT_1))previous=inventory->TakeOff(NDb::SLOT_1);
    if(mode!=7)Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"repair order cannot equip its real kit");else inventory->Activate(NDb::SLOT_1);
    if(mode==12)while(!kit->IsEmpty())kit->SpendPotion();
    if(repairFinish>=5)while(kit->GetIncQuantity()>1)kit->SpendPotion();
    MarkNewDGFrame();world->GetAIMap()->Sync();
    const int ap=actor->GetAP(),quantity=kit->GetIncQuantity(),rawShellHP=shellHP,pilotHP=pilot?int(pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP)):-1;const auto position=actor->GetPosition();auto random=GlobalGameRandom();
    if(repairAction>=0) {
        CObj<TreatmentAckObserver> ack;if(repairAck>=0)ack=ObserveTreatmentAck(world);
        actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
        CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,target);
        Require(executor->CanDoIt(actor->GetPosition())==NWorld::UCR_OK && !world->IsRealTime(),"repair action lacks its actual valid TBS order");
        const int prep=record->nAPToUse;executor->Run();const int available=actor->GetAP();
        Require(available==ap-prep && int(shellHP)==rawShellHP && kit->GetIncQuantity()==quantity,"repair animation start changed shell HP/dose or charged the wrong preparation AP");
        CObj<NRPG::IInventoryItem> removed;
        if(repairAction==7)shellHP.SetValue(shellHP.GetMaxValue());
        if(repairAction==8)removed=inventory->TakeOff(NDb::SLOT_1);
        const int initialHP=shellHP,deficit=shellHP.GetMaxValue()-initialHP,engineering=rpg->Skills(NDb::ST_ENGINEERING);
        auto quote=[&](int missing,int budget,int* spent){int hp=missing;*spent=int(float(missing)/engineering*20.f);if(*spent>budget){*spent=budget;hp=Float2Int(budget*engineering*.05f);}return hp;};
        int spent=0;const int expectedHP=repairAction==8?initialHP:initialHP+quote(deficit,available,&spent);
        std::printf("native repair action before: mode=%d kit=%d shell=%d worn=%d engineering=%d medicine=%d deficit=%d AP=%d prep=%d budget=%d pilotHP=%d\n",repairAction,record->GetRecordID(),shellPersona->GetRecordID(),int(bool(pilot)),engineering,int(rpg->Skills(NDb::ST_MEDICINE)),deficit,ap,prep,available,pilotHP);std::fflush(stdout);
        const auto medals=rpg->medalInfos;const bool pilotWound=pilot && pilot->GetUnitRPG()->HasCritical(NDb::C_VP);const int pilotBandage=pilot?pilot->GetUnitRPG()->GetRPGUnit()->nHealedVP:0;
        std::printf("native repair pilot before: mode=%d rawHP=%d bandage=%d penalty=%d\n",repairAction,pilotHP,pilotBandage,int(pilotWound));std::fflush(stdout);executor->AnimationFinished();
        const bool active=repairAction<7;
        std::printf("native repair action first: mode=%d shellHP=%d expectedHP=%d AP=%d expectedAP=%d marker=%d expectedActive=%d dose=%d expectedDose=%d state=%d\n",repairAction,int(shellHP),expectedHP,actor->GetAP(),available-spent,int(target->GetUnitRPG()->HasCritical(NDb::C_PATIENT)),int(active),kit->GetIncQuantity(),quantity-(repairAction==7?1:0),int(actor->GetState()));std::fflush(stdout);
        Require(int(shellHP)==expectedHP && actor->GetAP()==available-spent,"repair state did not apply the original engineering repair to the actual chassis");
        Require(target->GetUnitRPG()->HasCritical(NDb::C_PATIENT)==active && kit->GetIncQuantity()==quantity-(repairAction==7?1:0) && (actor->GetState()==NWorld::CUnit::ST_HEALER)==active,"repair state lost its native pending capacity, patient marker or dose ownership");
        if(repairAck>=0){
            Require(expectedHP-initialHP==247,"repair ack fixture did not apply its native engineering budget");
            if(repairAck==3){const int before=target->GetAP(),cost=target->GetActionAP(NRPG::AC_ROTATE);target->DoAction(NRPG::AC_ROTATE);Require(cost>0 && target->GetAP()==before-cost,"repair ack did not spend actual pilot AP");}
            NRPG::SUnitInfo info={};target->GetInfo(&info);Require(info.bPKInfo && info.nPKHP==int(shellHP) && info.nMaxPKHP==shellHP.GetMaxValue(),"repair ack lost the fork server shell-health adapter");
            actor->SetState(new NWorld::CUnitStateNormal(actor));const bool silent=repairAck==4,good=repairAck==0 || repairAck==3;
            std::printf("native repair ack: mode=%d worn=%d deficit=%d repaired=%d remaining=%d AP=%d->%d patientAP=%d canFight=%d good=%d bad=%d expectedGood=%d expectedBad=%d dose=%d->%d pilotHP=%d\n",repairAck,int(bool(pilot)),deficit,int(shellHP)-initialHP,shellHP.GetMaxValue()-int(shellHP),ap,actor->GetAP(),target->GetAP(),int(target->CanFight()),ack->good,ack->bad,int(good && !silent),int(!good && !silent),quantity,kit->GetIncQuantity(),pilot?int(pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP)):-1);std::fflush(stdout);
            Require(kit->GetIncQuantity()==quantity-1 && !target->GetUnitRPG()->HasCritical(NDb::C_PATIENT) && actor->GetAP()==available-spent && int(shellHP)==expectedHP,"repair ack changed paid shell/AP/dose ownership");
            Require(!pilot || int(pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP))==pilotHP,"repair ack changed pilot health");
            Require(ack->good==int(good && !silent) && ack->bad==int(!good && !silent) && (silent || (ack->healer==actor && ack->patient==target)),"repair result acknowledgment depends on pilot AP instead of original shell progress");return;
        }
        auto verifyPilot=[&](){if(pilot)Require(int(pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP))==pilotHP && pilot->GetUnitRPG()->GetRPGUnit()->nHealedVP==pilotBandage && pilot->GetUnitRPG()->HasCritical(NDb::C_VP)==pilotWound,"PK repair healed or bandaged its wounded pilot");};
        auto capacity=[&](){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(actor,&refs);CObj<NWorld::CUnitState> state;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(10,&state);}Require(dynamic_cast<NWorld::CUnitStateHealer*>(state.GetPtr()),"repair action lost its actual healer state");CStructureNetworkContext values;auto saved=BorrowedObjectFields(state,&values);float result=-999;{CStructureSaver reader(saved,CStructureSaver::READ,&values);reader.Add(4,&result);}return result;};
        SRepairFinishSnapshot finishBefore;
        if(repairFinish>=0) {
            finishBefore=RepairFinishSnapshot(actor);
            std::printf("native repair finish before: mode=%d action=%d kit=%d idle=%d flags=%d welder=%d healing=%d activeItem=%d clips=%d dose=%d end=%lld now=%lld\n",repairFinish,repairAction,record->GetRecordID(),finishBefore.idle,finishBefore.flags,finishBefore.welder,int(finishBefore.healing),int(finishBefore.activeItem),int(finishBefore.clips.size()),kit->GetIncQuantity(),(long long)finishBefore.end,(long long)world->GetTime()->GetValue());std::fflush(stdout);
            Require(finishBefore.idle==(record->GetRecordID()==26?0x1183:0x1181) && (finishBefore.flags&NAnimation::E_CUSTOM_ANIMATION_IDLE)!=0 && finishBefore.welder==(record->GetRecordID()==26?1:0) && finishBefore.healing,"repair finish fixture never entered its actual custom repair idle and dedicated welder state");
        }
        auto verifyFinish=[&](){
            const auto after=RepairFinishSnapshot(actor);const int wanted=record->GetRecordID()==26?0x1185:0x1184;int clips=0;STime length=0;
            for(auto clip:after.clips){bool old=false;for(auto previous:finishBefore.clips)if(previous.GetPtr()==clip.GetPtr())old=true;if(!old && clip->GetRecordID()==wanted){++clips;length=clip->GetTime();}}
            const bool removed=repairFinish>=5;
            std::printf("native repair finish after: mode=%d kit=%d idle=%d flags=%d welder=%d healing=%d walking=%d activeItem=%d slotEmpty=%d wanted=%d newFinishClips=%d length=%lld end=%lld now=%lld dose=%d\n",repairFinish,record->GetRecordID(),after.idle,after.flags,after.welder,int(after.healing),int(after.walking),int(after.activeItem),int(inventory->Get(NDb::SLOT_1)==nullptr),wanted,clips,(long long)length,(long long)after.end,(long long)world->GetTime()->GetValue(),kit->GetIncQuantity());std::fflush(stdout);
            Require(after.idle==0 && !(after.flags&NAnimation::E_CUSTOM_ANIMATION_IDLE) && after.welder==0 && !after.healing && after.walking,"repair completion retained its custom idle or live welding effect");
            Require(clips==1 && length>0 && after.end==world->GetTime()->GetValue()+length,"repair completion omitted or mis-timed the original kit-specific finish clip");
            Require((inventory->Get(NDb::SLOT_1)==nullptr)==removed && after.activeItem!=removed,"repair finish lost the fork active-item cleanup for the last kit charge");
        };
        if(repairFinish==3 || repairFinish==4){const int hp=shellHP,ap=actor->GetAP();actor->SetState(new NWorld::CUnitStateNormal(actor));Require(int(shellHP)==hp && actor->GetAP()==ap && kit->GetIncQuantity()==quantity-1 && !target->GetUnitRPG()->HasCritical(NDb::C_PATIENT),"cancelling repair changed paid HP/AP or lost its single dose/marker cleanup");verifyPilot();verifyFinish();return;}
        verifyPilot();Require(rpg->nHealedVP==0 && shell->GetUnitRPG()->GetRPGUnit()->nHealedVP==0,"repair applied human wound dressing to engineer or chassis");
        float remaining=record->nTotalHealVP-(expectedHP-initialHP);
        if(active){Require(capacity()==remaining,"repair did not initialize/subtract its authored kit capacity");const int hp=shellHP,currentAP=actor->GetAP();actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);Require(int(shellHP)==hp && actor->GetAP()==currentAP && capacity()==remaining && kit->GetIncQuantity()==quantity,"same-turn repair finish spent its engineering budget twice");}
        int turns=0;
        while(target->GetUnitRPG()->HasCritical(NDb::C_PATIENT) && turns<6){
            actor->OnTBSEvent(NWorld::TBS_START_NEW_TURN);const int budget=actor->GetAP(),before=shellHP,missing=shellHP.GetMaxValue()-before;int used=0;const int healed=quote(missing,budget,&used);
            actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);remaining-=healed;++turns;
            const bool pending=missing>0;
            std::printf("native repair action turn: mode=%d turn=%d budget=%d spent=%d repaired=%d shellHP=%d expectedHP=%d AP=%d remaining=%g pending=%d dose=%d\n",repairAction,turns,budget,used,healed,int(shellHP),before+healed,actor->GetAP(),remaining,int(pending),kit->GetIncQuantity());std::fflush(stdout);
            Require(int(shellHP)==before+healed && actor->GetAP()==budget-used && target->GetUnitRPG()->HasCritical(NDb::C_PATIENT)==pending && kit->GetIncQuantity()==quantity-(pending?0:1),"repair turn did not apply/pay the original engineering segment or finish only after the native healthy recheck");
            if(pending)Require(capacity()==remaining,"repair progress lost its actual remaining capacity");verifyPilot();
        }
        Require(!target->GetUnitRPG()->HasCritical(NDb::C_PATIENT) && kit->GetIncQuantity()==quantity-(repairAction==8?0:1) && !actor->IsPerformingAction() && !actor->HasCommand(),"repair action did not finish its native patient/action/dose ownership");
        if(repairAction!=8)Require(int(shellHP)==shellHP.GetMaxValue(),"repair action stopped before the actual chassis was repaired");
        Require(rpg->medalInfos.size()==medals.size(),"repair changed the engineer medal chain");for(int i=0;i<int(medals.size());++i)Require(rpg->medalInfos[i].fPoints==medals[i].fPoints,"repair awarded human wound-treatment medal progress");
        const int finalAP=actor->GetAP(),finalHP=shellHP,finalDose=kit->GetIncQuantity();actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);
        Require(actor->GetAP()==finalAP && int(shellHP)==finalHP && kit->GetIncQuantity()==finalDose,"finished repair consumed another dose or AP");
        if(repairFinish>=0)verifyFinish();
        std::printf("native repair action complete: mode=%d turns=%d shellHP=%d pilotHP=%d AP=%d dose=%d\n",repairAction,turns,int(shellHP),pilot?int(pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP)):-1,actor->GetAP(),kit->GetIncQuantity());std::fflush(stdout);return;
    }
    if(mode>=17) {
        const auto action=mode==19?NRPG::AC_FIRSTAID:NRPG::AC_REPAIR_PK;
        const int quoted=actor->GetActionAP(action),medical=actor->GetActionAP(NRPG::AC_FIRSTAID),expected=record->nAPToUse;
        Require(actor->GetAP()==ap && kit->GetIncQuantity()==quantity && int(shellHP)==rawShellHP,"repair preparation cost query changed AP, kit or shell HP");
        actor->DoAction(action);
        std::printf("native repair preparation: mode=%d kit=%d kind=%d action=%d expected=%d quoted=%d medical=%d AP=%d->%d dose=%d->%d shellHP=%d\n",mode,record->GetRecordID(),int(record->effect),int(action),expected,quoted,medical,ap,actor->GetAP(),quantity,kit->GetIncQuantity(),int(shellHP));std::fflush(stdout);
        Require(quoted==expected && medical==expected && actor->GetAP()==ap-expected && kit->GetIncQuantity()==quantity && int(shellHP)==rawShellHP,"repair preparation omitted the original active kit AP cost");return;
    }
    const auto expected=mode>=14?NWorld::UCR_GENERAL_FAILURE:mode==2 || mode==10?NWorld::UCR_CANT_HEAL:mode==3 || mode==4?NWorld::UCR_TARGET_OUT_OF_RANGE:mode==5?NWorld::UCR_NEED_HIGHER_SKILL:mode==6 || mode==12?NWorld::UCR_NO_EQUIPMENT:mode==7?NWorld::UCR_UNAVAILABLE:mode==8 || mode==13?NWorld::UCR_INVALID_COMMAND:mode==9?NWorld::UCR_NO_TARGET:mode==11?NWorld::UCR_GENERAL_FAILURE:NWorld::UCR_OK;
    CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,target);const auto result=executor->CanDoIt(queryFrom);const auto repeat=executor->CanDoIt(queryFrom);
    std::printf("native repair order gate: mode=%d engineering=%d medicine=%d perk79=%d kit=%d kind=%d targetPK=%d targetEmpty=%d result=%d repeat=%d expected=%d distance=%g queryPreview=%d actorAP=%d dose=%d shellHP=%d pilotHP=%d\n",mode,int(rpg->Skills(NDb::ST_ENGINEERING)),int(rpg->Skills(NDb::ST_MEDICINE)),int(rpg->HasPerk(79)),record->GetRecordID(),int(record->effect),target?int(target->IsWearingPK()):-1,target?int(target->IsEmptyPK()):-1,int(result),int(repeat),int(expected),target?fabs(queryFrom.GetCP()-target->GetPosition().GetCP()):-1,int(mode==4),actor->GetAP(),kit->GetIncQuantity(),int(shellHP),pilotHP);std::fflush(stdout);
    Require(actor->GetAP()==ap && kit->GetIncQuantity()==quantity && int(shellHP)==rawShellHP && (!pilot || int(pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP))==pilotHP) && fabs(actor->GetPosition().GetCP()-position.GetCP())<1e-6f && random.Get()==GlobalGameRandom().Get(),"repair availability query changed AP, item, suit/pilot health, position or RNG");
    Require(result==expected && repeat==expected,"repair order used medical reach or ignored original repair target/engineer gates");
}

static void WoundHealerPerk(const char* databasePath,const char* resourcePath,int mode,bool speedTest=false)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* anchor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK()){anchor=unit;break;}Require(anchor,"wound healer perk has no actual player anchor");
    const int targetPerk=speedTest && mode!=6 && mode!=7?63:57;
    NDb::CRPGPers* persona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext()) {
        auto* p=people.Get();if(!p->pClass || !p->pModel || !p->pBaseValue || !p->pName || p->pClass->skills[NDb::ST_LEVEL]<=0 || p->pPanzerklein || p->pDefaultWearsPanzerklein)continue;
        CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(p->pClass->nPerkTreeID);vector<CPtr<NRPG::CPerk>> perks;tree->GetAllPerks(&perks);bool found=false;for(auto perk:perks)if(perk->GetDBPerk()->GetRecordID()==targetPerk)found=true;
        if(found && (!persona || p->GetRecordID()<persona->GetRecordID()))persona=p;
    }
    Require(persona,"wound healer perk has no authored ordinary physician class");
    CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(anchor->GetPosition().pos.p,NRPG::CreateUnit(persona),anchor->GetTBSPlayer());Require(actor && actor->CanFight(),"wound healer perk could not deploy its actual physician");
    auto* healer=actor->GetUnitRPG()->GetRPGUnit();auto* tree=healer->GetPerksTree();vector<CPtr<NRPG::CPerk>> perks;tree->GetAllPerks(&perks);NRPG::CPerk* bonus=nullptr;
    for(auto perk:perks)if(perk->GetDBPerk()->GetRecordID()==targetPerk)bonus=perk;Require(bonus && !bonus->IsTaken(),"wound healer perk fixture lacks an unacquired real ability");
    vector<CPtr<NRPG::CPerk>> path;
    auto collect=[&](auto&& self,NRPG::CPerk* perk)->void{for(int i=0;i<perk->GetParents().GetSize();++i)self(self,perk->GetParents()[i]);for(auto known:path)if(known.GetPtr()==perk)return;path.push_back(perk);};collect(collect,bonus);
    for(int level=1;tree->GetPerkPoints()<int(path.size()) && level<=20;++level)healer->SetXPLevel(level);
    Require(tree->GetPerkPoints()>=int(path.size()),"wound healer perk native progression did not earn its actual prerequisite points");
    const bool acquired=mode!=2 && mode!=3;
    for(auto perk:path){if(perk.GetPtr()==bonus && !acquired)continue;Require(perk->CanTake(),"wound healer perk tried to bypass a real prerequisite");tree->TakePerk(perk->GetDBPerk()->GetRecordID());}
    float parameter=0;Require(healer->HasPerk(targetPerk,&parameter)==acquired && (!acquired || parameter>0),"wound healer perk did not acquire/withhold its actual positive bonus");
    CObj<NWorld::CUnitServer> patient=world->AddUnitInGame(anchor->GetPosition().pos.p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer());Require(patient && patient->CanFight() && !patient->GetUnitRPG()->HasPerk(24),"wound healer perk lacks its ordinary unmodified patient");
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),1.5f),&places);bool placed=false;
    for(auto place:places){place.SetPose(NAI::CM_STAND);if(!network->IsPassable(place) || network->IsLocked(place,false))continue;auto pos=patient->GetPosition();pos.pos=NAI::SPosition(place,network);if(fabs(pos.GetCP()-actor->GetPosition().GetCP())<.3f || !NWorld::IsWithinHumanReach(actor->GetPosition().GetCP(),pos.GetCP(),.8f))continue;world->GetTime()->Set(world->GetTime()->GetValue()+1);patient->SetPosition(pos);patient->animator.AlignTime();patient->animator.PlaceUnit(pos);placed=true;break;}Require(placed,"wound healer perk has no free real patient approach");
    const int kitID=speedTest?(mode==5?3:1):(mode==1 || mode==3?4:3);
    auto* record=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(kitID);Require(record && record->effect==(kitID==1?NDb::FAE_NORMAL:NDb::FAE_CRITICAL_FIRST) && record->nSkillModifier==(kitID==1?0:kitID==4?20:10) && record->nAPToUse==(kitID==1?0:10),"wound perk fixture lost its actual normal or critical-first kit");
    float strength=0,speedParameter=0;healer->HasPerk(57,&strength);const bool hasSpeed=healer->HasPerk(63,&speedParameter);const float speed=record->effect==NDb::FAE_NORMAL && hasSpeed?speedParameter:1.f;
    Require(speed>0 && (!speedTest || mode==2 || mode==3 || hasSpeed),"wound speed fixture has no actual positive speed ability");
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous;if(inventory->Get(NDb::SLOT_1))previous=inventory->TakeOff(NDb::SLOT_1);Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"wound healer perk cannot equip its actual kit");
    actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());MarkNewDGFrame();world->GetAIMap()->Sync();
    auto* target=patient->GetUnitRPG();auto* unit=target->GetRPGUnit();auto& hp=unit->Skills(NDb::ST_VP);const int wound=mode==1 || mode==3?hp.GetMaxValue()/2:mode==4?12:speedTest && mode==7?1:4;hp.SetValue(hp.GetMaxValue()-wound);target->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_VP));
    if(mode==4){NRPG::SFirstAid prior;prior.fdVP=8;prior.nMaxVP=999;target->HealVP(prior);}
    const int raw=hp,already=unit->nHealedVP,ap=actor->GetAP(),quantity=kit->GetIncQuantity(),medicine=healer->Skills(NDb::ST_MEDICINE),skill=int(medicine+record->nSkillModifier+strength);
    const int budget=ap-record->nAPToUse;Require(budget>0,"wound healer perk has insufficient native AP after its authored preparation");
    const int quota=int(float(NRPG::N_MAX_VP)/NRPG::N_MAX_SKILL*skill);int expectedHP=(std::min)((std::max)(0,quota-already),wound-already),expectedAP=int(expectedHP/(skill*speed)*60.f);
    if(expectedAP>budget){expectedAP=budget;expectedHP=Float2Int(budget*skill*speed/60.f);}expectedHP=(std::min)(expectedHP,record->nTotalHealVP);
    NRPG::SFirstAid plan;int required=-999;auto random=GlobalGameRandom();const bool planned=healer->CreateFirstAid(&plan,budget,record->nTotalHealVP,kit,unit,&required);
    Require(actor->GetAP()==ap && kit->GetIncQuantity()==quantity && int(hp)==raw && unit->nHealedVP==already && int(healer->Skills(NDb::ST_MEDICINE))==medicine && random.Get()==GlobalGameRandom().Get(),"wound healer perk planning mutated actual AP/item/health/medicine/RNG");
    std::printf((speedTest?"native wound speed perk query: mode=%d persona=%d level=%d parents=%d acquired=%d parameter=%g medicine=%d skill=%d quota=%d planned=%d actualQuota=%d required=%d expectedAP=%d plannedHP=%g expectedHP=%d wound=%d dressed=%d AP=%d budget=%d speed63=%d kit=%d targetPerk=%d speed=%g strength=%g\n":"native wound healer perk query: mode=%d persona=%d level=%d parents=%d acquired=%d parameter=%g medicine=%d skill=%d quota=%d planned=%d actualQuota=%d required=%d expectedAP=%d plannedHP=%g expectedHP=%d wound=%d dressed=%d AP=%d budget=%d speed63=%d kit=%d targetPerk=%d speed=%g strength=%g\n"),mode,persona->GetRecordID(),int(healer->Skills(NDb::ST_LEVEL)),int(path.size())-1,int(acquired),parameter,medicine,skill,quota,int(planned),planned?plan.nMaxVP:-1,required,expectedAP,planned?plan.fdVP:-1,expectedHP,wound,already,ap,budget,int(healer->HasPerk(63)),record->GetRecordID(),targetPerk,speed,strength);std::fflush(stdout);
    CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,patient);Require(executor->CanDoIt(actor->GetPosition())==NWorld::UCR_OK && !world->IsRealTime(),"wound healer perk actual order is unavailable");executor->Run();Require(actor->GetAP()==budget && unit->nHealedVP==already && kit->GetIncQuantity()==quantity,"wound treatment start changed its authored preparation AP");executor->AnimationFinished();
    std::printf((speedTest?"native wound speed perk action: mode=%d dressed=%d expectedDressed=%d AP=%d expectedAP=%d rawHP=%d medicine=%d dose=%d\n":"native wound healer perk action: mode=%d dressed=%d expectedDressed=%d AP=%d expectedAP=%d rawHP=%d medicine=%d dose=%d\n"),mode,unit->nHealedVP,already+expectedHP,actor->GetAP(),budget-expectedAP,int(hp),int(healer->Skills(NDb::ST_MEDICINE)),kit->GetIncQuantity());std::fflush(stdout);
    Require(planned && plan.nMaxVP==quota && required==expectedAP && int(plan.fdVP)==expectedHP && unit->nHealedVP==already+expectedHP && actor->GetAP()==budget-expectedAP,speedTest?"wound treatment ignored the original normal-only speed perk63":"wound treatment ignored the original healer skill perk57");
    Require(int(hp)==raw && int(healer->Skills(NDb::ST_MEDICINE))==medicine,"local wound bonus changed raw health or the physician medicine statistic");
    actor->SetState(new NWorld::CUnitStateNormal(actor));Require(kit->GetIncQuantity()==quantity-1 && !target->HasCritical(NDb::C_PATIENT) && !actor->IsPerformingAction(),"wound healer perk lost native charge/patient ownership cleanup");
}

static void PKRepairApproach(const char* databasePath,const char* resourcePath,int mode)
{
    const bool worn=mode>=8 && mode<16 || mode==17 || mode==19;const int direction=mode<16?mode%8:mode-16;
    const bool full=mode==16 || mode==17,wrong=mode==18,empty=mode==19,expected=!full && !wrong && !empty;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* anchor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsWearingPK() && !unit->IsEmptyPK()){anchor=unit;break;}
    Require(anchor,"PK repair approach has no real player anchor");CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(anchor->GetPosition().pos.p,NRPG::CreateUnit(NDb::GetPers(3)),anchor->GetTBSPlayer());Require(actor && actor->CanFight(),"PK repair approach cannot deploy its real engineer");
    auto* rpg=actor->GetUnitRPG()->GetRPGUnit();for(int level=1;int(rpg->Skills(NDb::ST_ENGINEERING))<100 && level<=20;++level)rpg->SetXPLevel(level);rpg->GetPerksTree()->TakePerk(79);Require(int(rpg->Skills(NDb::ST_ENGINEERING))>=100 && rpg->HasPerk(79),"PK repair approach lacks native engineering/perk qualification");
    NDb::CRPGPers* shellPersona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());while(people.MoveNext()){auto* p=people.Get();if(p->pPanzerklein && !p->pPanzerklein->bHasNoHead && p->pModel && (!shellPersona || p->GetRecordID()<shellPersona->GetRecordID()))shellPersona=p;}Require(shellPersona,"PK repair approach has no real ordinary chassis");
    const int dx=NAI::nMoveShift[direction][0],dy=NAI::nMoveShift[direction][1];const bool cardinal=dx==0 || dy==0;auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(anchor->GetPosition().GetCP(),20.f),&places);NAI::SPathPlace shellPlace,startPlace;bool found=false;int candidates=0;
    for(auto place:places){
        const int x=int(place.GetX()),y=int(place.GetY());if(x<5 || y<5 || x>250 || y>250 || !place.IsIntegral() || place.IsFinal())continue;place.SetPose(NAI::CM_STAND);place.SetDirection(direction);
        if(!NAI::IsBigLockerPlacePassable(network,place))continue;++candidates;auto start=place;start.SetXY(x-4*dx,y-4*dy);if(!network->IsPassable(start) || network->IsLocked(start,false))continue;
        const auto center=NAI::SPosition(place,network).GetCP();vector<CTPoint<int>> offsets=cardinal?vector<CTPoint<int>>{CTPoint<int>(-2*dx,-2*dy)}:vector<CTPoint<int>>{CTPoint<int>(-2*dx,0),CTPoint<int>(2*dx,0),CTPoint<int>(0,-2*dy),CTPoint<int>(0,2*dy)};bool clear=true;
        for(auto offset:offsets){auto point=place;point.SetXY(x+offset.x,y+offset.y);if(!cardinal)point.SetDirection(NRPG::GetShootDirection(network,point,center));const auto cp=NAI::SPosition(point,network).GetCP();if(!network->IsPassable(point) || network->IsLocked(point,false) || !NWorld::IsWithinHumanReach(cp,center,1.8f) || NWorld::IsWithinHumanReach(cp,center,.8f))clear=false;}
        if(!clear || NWorld::IsWithinHumanReach(NAI::SPosition(start,network).GetCP(),center,1.8f))continue;shellPlace=place;startPlace=start;found=true;break;
    }
    std::printf("native PK repair approach setup: mode=%d worn=%d direction=%d candidates=%d found=%d\n",mode,int(worn),direction,candidates,int(found));std::fflush(stdout);Require(found,"PK repair approach has no genuine free big-footprint/approach/start layout");
    CObj<NWorld::CUnitServer> shell=world->AddUnitInGame(shellPlace,NRPG::CreateUnit(shellPersona),actor->GetTBSPlayer());Require(shell && shell->IsEmptyPK(),"PK repair approach cannot deploy its actual empty suit");
    auto shellPosition=shell->GetPosition();shellPosition.pos=NAI::SPosition(shellPlace,network);world->GetTime()->Set(world->GetTime()->GetValue()+1);shell->SetPosition(shellPosition);shell->animator.AlignTime();shell->animator.PlaceUnit(shellPosition);
    CObj<NWorld::CUnitServer> pilot;NWorld::CUnitServer* target=shell;
    if(worn){pilot=world->AddUnitInGame(anchor->GetPosition().pos.p,NRPG::CreateUnit(NDb::GetPers(103)),actor->GetTBSPlayer());Require(pilot && pilot->CanFight() && shell->WearAsPK(true),"PK repair approach cannot prepare actual pilot and suit");pilot->SetPosition(shell->GetPosition());pilot->FlipPanzerklein(shell,false,false);Require(pilot->IsWearingPK() && pilot->GetWearingPK()==shell.GetPtr(),"PK repair approach lost actual pilot/suit ownership");target=pilot;}
    auto targetPosition=target->GetPosition();targetPosition.pos=NAI::SPosition(shellPlace,network);world->GetTime()->Set(world->GetTime()->GetValue()+1);target->SetPosition(targetPosition);target->animator.AlignTime();target->animator.PlaceUnit(targetPosition);
    Require(target->GetPosition().pos.p.GetDirection()==direction && target->IsEmptyPK()!=worn,"PK repair approach changed its actual target facing/kind");auto& shellHP=shell->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP);Require(shellHP.GetMaxValue()>100,"PK repair approach has insufficient native chassis health");shellHP.SetValue(shellHP.GetMaxValue()-(full?0:100));
    auto actorPosition=actor->GetPosition();actorPosition.pos=NAI::SPosition(startPlace,network);world->GetTime()->Set(world->GetTime()->GetValue()+1);actor->SetPosition(actorPosition);actor->animator.AlignTime();actor->animator.PlaceUnit(actorPosition);
    auto* record=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(wrong?1:26);Require(record && record->effect==(wrong?NDb::FAE_NORMAL:NDb::FAE_REPAIR_PK),"PK repair approach lost its authored kit");CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous;if(inventory->Get(NDb::SLOT_1))previous=inventory->TakeOff(NDb::SLOT_1);Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"PK repair approach cannot equip its real kit");if(empty)while(!kit->IsEmpty())kit->SpendPotion();
    MarkNewDGFrame();world->GetAIMap()->Sync();CObj<NWorld::CExecHeal> direct=new NWorld::CExecHeal(actor,target);int legal=0;
    const auto center=target->GetPosition().pos.p;vector<CTPoint<int>> offsets=cardinal?vector<CTPoint<int>>{CTPoint<int>(-2*dx,-2*dy)}:vector<CTPoint<int>>{CTPoint<int>(-2*dx,0),CTPoint<int>(2*dx,0),CTPoint<int>(0,-2*dy),CTPoint<int>(0,2*dy)};
    for(auto offset:offsets){auto point=center;point.SetXY(int(center.GetX())+offset.x,int(center.GetY())+offset.y);if(!cardinal){point.SetPose(NAI::CM_STAND);point.SetDirection(NRPG::GetShootDirection(network,point,target->GetPosition().GetCP()));}auto pos=actor->GetPosition();pos.pos=NAI::SPosition(point,network);if(network->IsPassable(point) && !network->IsLocked(point,false) && direct->CanDoIt(pos)==NWorld::UCR_OK)++legal;}
    Require(expected?legal>0:legal==0,"PK repair approach lacks its genuine usable/rejected destination precondition");
    const int ap=actor->GetAP(),dose=kit->GetIncQuantity(),rawHP=shellHP,pilotHP=pilot?pilot->GetUnitRPG()->GetTotalVP():-1,engineering=rpg->Skills(NDb::ST_ENGINEERING);const auto from=actor->GetPosition(),to=target->GetPosition(),shellFrom=shell->GetPosition();auto random=GlobalGameRandom();
    NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCmdHeal> command=new NWorld::CCmdHeal(target);CObj<NWorld::CCommandExecute> executor=NWorld::CreateActionExecutor(actor,command,&result);auto* mover=dynamic_cast<NWorld::IExecMove*>(executor.GetPtr());NAI::SPathPlace destination=from.pos.p;NAI::EFindPathParams params=NAI::PF_DEFAULT;NWorld::ENeedActiveItem active=NWorld::ITEM_NO_MATTER;if(mover)mover->GetDesiredPlace(&destination,&params,&active);CObj<NAI::CPath> path=executor?executor->GetCurrentPath():nullptr;auto afterRandom=GlobalGameRandom();
    std::printf("native PK repair approach: mode=%d worn=%d direction=%d cardinal=%d full=%d wrongKit=%d emptyKit=%d legal=%d executor=%d result=%d mover=%d points=%d sourceX=%d sourceY=%d targetX=%d targetY=%d goalX=%d goalY=%d goalPose=%d goalDirection=%d expected=%d AP=%d dose=%d shellHP=%d pilotHP=%d engineering=%d\n",mode,int(worn),direction,int(cardinal),int(full),int(wrong),int(empty),legal,int(bool(executor)),int(result),int(bool(mover)),path?int(path->points.size()):0,int(from.pos.p.GetX()),int(from.pos.p.GetY()),int(center.GetX()),int(center.GetY()),int(destination.GetX()),int(destination.GetY()),int(destination.GetPose()),int(destination.GetDirection()),int(expected),ap,dose,rawHP,pilotHP,engineering);std::fflush(stdout);
    Require(actor->GetAP()==ap && kit->GetIncQuantity()==dose && inventory->GetActive()==kit.GetPtr() && int(shellHP)==rawHP && (pilot?pilot->GetUnitRPG()->GetTotalVP():-1)==pilotHP && actor->GetPosition()==from && target->GetPosition()==to && shell->GetPosition()==shellFrom && int(rpg->Skills(NDb::ST_ENGINEERING))==engineering && rpg->HasPerk(79) && !actor->HasCommand() && random.Get()==afterRandom.Get() && (!worn || pilot->GetWearingPK()==shell.GetPtr()),"PK repair approach planning changed physical state, skill, items, ownership or RNG");
    Require(bool(executor)==expected,"PK repair command cannot plan a reachable original directional approach despite a qualified engineer and damaged real suit");if(!expected)return;
    Require(mover && path && !path->points.empty() && result==NWorld::UCR_OK,"PK repair approach lost its actual native movement/heal queue");Require(destination.GetX()==path->points.back().GetX() && destination.GetY()==path->points.back().GetY() && destination.GetLayer()==path->points.back().GetLayer(),"PK repair approach differs from its native path endpoint");
    auto end=from;end.pos=NAI::SPosition(destination,network);Require(direct->CanDoIt(end)==NWorld::UCR_OK,"PK repair approach queues an invalid actual repair position");
    const int gx=int(destination.GetX())-int(center.GetX()),gy=int(destination.GetY())-int(center.GetY());if(cardinal)Require(gx==-2*dx && gy==-2*dy && destination.GetDirection()==direction && destination.GetPose()==center.GetPose(),"cardinal PK repair does not approach two tiles behind the suit with its original pose/facing");
    else Require((std::abs(gx)==2 && gy==0 || gx==0 && std::abs(gy)==2) && destination.GetPose()==NAI::CM_STAND && destination.GetDirection()==NRPG::GetShootDirection(network,destination,target->GetPosition().GetCP()),"diagonal PK repair does not use one of four standing side approaches facing the suit");
}

static void RepairPlanner(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    auto* record=NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(26);
    Require(record && record->effect==NDb::FAE_REPAIR_PK && record->nRequiedSkill==100 && record->nRequiredPerkID==79,"repair planner needs its genuine qualified repair-kit definition");
    NDb::CRPGPers* engineerPersona=nullptr;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext()) {
        auto* persona=people.Get();if(!IsValid(persona->pClass) || !IsValid(persona->pModel) || !IsValid(persona->pBaseValue) || !IsValid(persona->pName) || persona->pClass->skills[NDb::ST_LEVEL]<=0 || IsValid(persona->pPanzerklein) || IsValid(persona->pDefaultWearsPanzerklein))continue;
        CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(persona->pClass->nPerkTreeID);vector<CPtr<NRPG::CPerk>> perks;tree->GetAllPerks(&perks);bool hasRepairAbility=false;
        for(auto perk:perks)if(perk->GetDBPerk()->GetRecordID()==79){hasRepairAbility=true;break;}
        if(hasRepairAbility && (!engineerPersona || persona->GetRecordID()<engineerPersona->GetRecordID()))engineerPersona=persona;
    }
    Require(engineerPersona,"repair planner has no real engineer class with perk79");
    CObj<NRPG::IUnitMission> engineer=NRPG::CreateUnit(engineerPersona);auto* unit=engineer->GetRPGUnit();
    const int initialEngineering=unit->Skills(NDb::ST_ENGINEERING);
    for(int level=1;int(unit->Skills(NDb::ST_ENGINEERING))<record->nRequiedSkill && level<=20;++level)unit->SetXPLevel(level);
    unit->GetPerksTree()->TakePerk(79);
    const int bestID=engineerPersona->GetRecordID();
    std::printf("native repair planner selection: mode=%d persona=%d initialEngineering=%d level=%d engineering=%d medicine=%d perk79=%d\n",mode,bestID,initialEngineering,int(unit->Skills(NDb::ST_LEVEL)),int(unit->Skills(NDb::ST_ENGINEERING)),int(unit->Skills(NDb::ST_MEDICINE)),int(unit->HasPerk(79)));std::fflush(stdout);
    Require(unit->HasPerk(79) && int(unit->Skills(NDb::ST_ENGINEERING))>=100,"repair planner native class progression did not reach actual item requirements");
    NDb::CRPGPers* shellPersona=nullptr;CDBIterator<NDb::CRPGPers> shells(*NDatabase::GetTable<NDb::CRPGPers>());
    while(shells.MoveNext()){auto* persona=shells.Get();if(IsValid(persona->pPanzerklein) && IsValid(persona->pModel) && (!shellPersona || persona->GetRecordID()<shellPersona->GetRecordID()))shellPersona=persona;}
    Require(shellPersona,"repair planner lacks a real authored PK chassis");CObj<NRPG::IUnitMission> shell=NRPG::CreateUnit(shellPersona);
    auto* target=shell->GetRPGUnit();auto& hp=target->Skills(NDb::ST_VP);Require(hp.GetMaxValue()>100 && target->nHealedVP==0,"repair planner chassis has insufficient real VP or preexisting dressing");
    const int deficit=mode==0?3:mode==3?0:100;hp.SetValue(hp.GetMaxValue()-deficit);
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);const int skill=unit->Skills(NDb::ST_ENGINEERING),medicine=unit->Skills(NDb::ST_MEDICINE);
    const int budget=mode==2?1:1000;const float capacity=mode==4?1.f:1000.f;
    const int ap=unit->Skills(NDb::ST_AP),rawHP=hp,quantity=kit->GetIncQuantity();NRPG::SFirstAid plan;plan.fdVP=-999;plan.nMaxVP=-999;int required=-777;
    const bool result=unit->CreateFirstAid(&plan,budget,capacity,kit,target,&required);
    const int expectedHP=mode==2?Float2Int(skill*.05f):deficit;
    const int expectedAP=mode==2?1:mode==3?-777:mode==0?0:int(float(deficit)/skill*20.f);
    std::printf("native repair planner: mode=%d actor=%d engineering=%d medicine=%d perk79=%d kit=%d kind=%d shell=%d deficit=%d budget=%d capacity=%g result=%d planned=%g quota=%d quotedAP=%d expectedHP=%d expectedAP=%d\n",mode,bestID,skill,medicine,int(unit->HasPerk(79)),record->GetRecordID(),int(record->effect),shellPersona->GetRecordID(),deficit,budget,capacity,int(result),plan.fdVP,plan.nMaxVP,required,expectedHP,expectedAP);std::fflush(stdout);
    Require(int(unit->Skills(NDb::ST_AP))==ap && int(hp)==rawHP && kit->GetIncQuantity()==quantity && target->nHealedVP==0,"pure repair planning spent AP/dose or changed shell HP/dressing");
    Require(result==(deficit>0) && (deficit==0?(required==-777 && plan.fdVP==-999):(plan.fdVP==expectedHP && required==expectedAP)),"repair planner used medical bandaging instead of original engineering repair costs");
}

static void RepairKindImport(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    auto* table=NDatabase::GetTable<NDb::CRPGFirstAid>();Require(table,"repair import has no actual first-aid table");
    if(mode<2) {
        const int id=26+mode;auto* record=table->GetRecord(id);Require(record && IsValid(record->pItem),"repair import lost an authored repair item");
        std::printf("native repair kind import: kit=%d actual=%d expected=6 capacity=%d required=%d perk=%d quantity=%d\n",id,int(record->effect),record->nTotalHealVP,record->nRequiedSkill,record->nRequiredPerkID,record->nQuantity);std::fflush(stdout);
        Require(int(record->effect)==6 && record->nTotalHealVP==1000 && record->nRequiedSkill==100 && record->nRequiredPerkID==79 && record->nQuantity==10,"first-aid importer ignored the authored RepairPK type");
    } else {
        for(int id=1;id<=17;++id) {
            const int expected=id>=3 && id<=5?7:id<=8?0:id<=11?2:id<=14?3:id<=16?4:5;
            auto* record=table->GetRecord(id);Require(record && IsValid(record->pItem),"repair import control lost a stock medical item");
            std::printf("native repair kind control: kit=%d actual=%d expected=%d\n",id,int(record->effect),expected);std::fflush(stdout);
            Require(int(record->effect)==expected,"repair type import changed another authored medical effect");
        }
    }
}

static void CriticalFirstImport(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    auto* table=NDatabase::GetTable<NDb::CRPGFirstAid>();Require(table,"critical-first import has no actual first-aid table");
    bool correct=true;
    for(int id:{3,4,5,1,2,6,7,8}) {
        auto* record=table->GetRecord(id);Require(record && IsValid(record->pItem),"critical-first import lost a stock item");
        const int expected=id>=3 && id<=5?7:0;
        std::printf("native critical-first import: kit=%d actual=%d expected=%d capacity=%d\n",id,int(record->effect),expected,record->nTotalHealVP);std::fflush(stdout);
        correct=correct && int(record->effect)==expected;
    }
    Require(correct,"first-aid importer ignored the authored NormalCriticalFirst type");
}

static void WoundCapacityPlan(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit->GetUnitRPG()->GetRPGUnit()->GetPers()->GetRecordID()==54 && !unit->IsAIUnit() && !unit->IsWearingPK()){actor=unit;break;}
    Require(actor,"capacity planner fixture has no actual roster hero54");auto* healer=actor->GetUnitRPG()->GetRPGUnit();
    Require(int(healer->Skills(NDb::ST_MEDICINE))==17,"capacity planner fixture requires its actual Medicine17");
    NDb::CRPGFirstAid* record=nullptr;CDBIterator<NDb::CRPGFirstAid> kits(*NDatabase::GetTable<NDb::CRPGFirstAid>());
    while(kits.MoveNext()){auto* candidate=kits.Get();if(IsValid(candidate->pItem) && candidate->effect==NDb::FAE_NORMAL && candidate->GetRecordID()==1 && candidate->nQuantity>0 && candidate->nSkillModifier==0){record=candidate;break;}}
    Require(record,"capacity planner fixture has no actual unmodified normal kit");CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);
    CObj<NRPG::IUnitMission> target=NRPG::CreateUnit(NDb::GetPers(103));auto* patient=target->GetRPGUnit();auto& hp=patient->Skills(NDb::ST_VP);hp.SetValue(hp.GetMaxValue()-3);
    Require(patient->nHealedVP==0,"capacity planner fixture patient already has dressing");
    const int ap=actor->GetAP(),quantity=kit->GetIncQuantity(),rawHP=hp;Require(ap>=10,"capacity planner fixture cannot cover its native full quote");
    struct Case {int capacity,expected;};const Case cases[]={{0,0},{1,1},{2,2},{3,3},{100,3}};bool correct=true;
    for(const auto& test:cases) {
        NRPG::SFirstAid plan;int quoted=-999;const bool result=healer->CreateFirstAid(&plan,ap,float(test.capacity),kit,patient,&quoted);
        std::printf("native wound capacity plan: capacity=%d result=%d planned=%g expected=%d quotedAP=%d maxVP=%d actorAP=%d dose=%d rawHP=%d dressed=%d\n",test.capacity,int(result),result?plan.fdVP:-1,test.expected,quoted,result?plan.nMaxVP:-1,actor->GetAP(),kit->GetIncQuantity(),int(hp),patient->nHealedVP);std::fflush(stdout);
        // Original quotes AP before clamping to remaining capacity, including the zero-capacity query.
        correct=correct && result && plan.fdVP==test.expected && plan.nMaxVP==30 && quoted==10;
        Require(actor->GetAP()==ap && kit->GetIncQuantity()==quantity && int(hp)==rawHP && patient->nHealedVP==0,"pure wound planning changed AP, dose or patient health");
    }
    Require(correct,"wound planner ignored remaining kit capacity or changed the original pre-clamp AP quote");
}

static void HealingMedalAwards(const char* databasePath,const char* resourcePath,int mode,bool critical=false,bool woundBudget=false,bool woundFinish=false,int authoredKit=0)
{
    const bool patientWound = !critical && mode>=9 && mode<=13;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    auto ordinary=[](NWorld::CUnitServer* unit){return unit && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->HasCommand() && !unit->IsPerformingAction() && unit->GetState()!=NWorld::CUnit::ST_MACHINE_GUN;};
    for(auto unit:units)if(ordinary(unit) && !unit->IsAIUnit() && !unit->GetUnitRPG()->GetRPGUnit()->bDisabled && int(unit->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_MEDICINE))>0) {actor=unit;break;}
    Require(actor,"wound award fixture has no actual roster medic with medal collection enabled");
    if(mode==6) {
        actor=nullptr;
        for(auto unit:units)if(ordinary(unit) && unit->GetUnitRPG()->GetRPGUnit()->bDisabled && int(unit->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_MEDICINE))>0) {actor=unit;break;}
        Require(actor,"wound award fixture has no actual NPC with medal collection disabled");
    }
    CObj<NWorld::CUnitServer> qualified;if(critical){qualified=QualifiedMedicalActor(world,actor);actor=qualified;}
    NDb::CRPGPers* patientPersona=NDb::GetPers(103);
    if(patientWound) {
        patientPersona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
        while(people.MoveNext()) {
            auto* candidate=people.Get();if(!IsValid(candidate->pClass) || !IsValid(candidate->pModel) || !IsValid(candidate->pBaseValue) || !IsValid(candidate->pName) || candidate->pClass->skills[NDb::ST_LEVEL]<=0 || IsValid(candidate->pPanzerklein) || IsValid(candidate->pDefaultWearsPanzerklein))continue;
            CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(candidate->pClass->nPerkTreeID);vector<CPtr<NRPG::CPerk>> entries;tree->GetAllPerks(&entries);
            for(auto entry:entries)if(entry->GetDBPerk()->GetRecordID()==24){patientPersona=candidate;break;}if(patientPersona)break;
        }
        Require(patientPersona,"wound patient fixture lacks its real perk24 class");
    }
    CObj<NWorld::CUnitServer> retained;NWorld::CUnitServer* patient=actor;
    if(mode==5) {
        patient=nullptr;
        for(auto unit:units)if(ordinary(unit) && unit->GetDiplomacyState(actor)==NDb::DS_ENEMY) {patient=unit;break;}
        Require(patient,"wound award fixture has no real enemy rejection target");
    } else if(mode!=2) {
        retained=world->AddUnitInGame(actor->GetPosition().pos.p,NRPG::CreateUnit(patientPersona),actor->GetTBSPlayer());patient=retained;
        Require(ordinary(patient) && patient->GetPlayer()==actor->GetPlayer(),"wound award fixture could not deploy its actual friendly patient");
        auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),1.5f),&places);bool placed=false;
        for(auto place:places) {
            place.SetPose(NAI::CM_STAND);if(!network->IsPassable(place) || network->IsLocked(place,false))continue;
            auto position=patient->GetPosition();position.pos=NAI::SPosition(place,network);const float distance=fabs(position.GetCP()-actor->GetPosition().GetCP());
            if(distance<.3f || !NWorld::IsWithinHumanReach(actor->GetPosition().GetCP(),position.GetCP(),.8f))continue;
            world->GetTime()->Set(world->GetTime()->GetValue()+1);patient->SetPosition(position);patient->animator.AlignTime();patient->animator.PlaceUnit(position);placed=true;break;
        }
        Require(placed,"wound award fixture has no reachable native patient position");
    }
    float patientWoundMultiplier=1;
    if(patientWound) {
        auto* unit=patient->GetUnitRPG()->GetRPGUnit();const bool acquired=mode!=11 && mode!=12;
        if(acquired){unit->GetPerksTree()->AddPerkPoints(1);unit->GetPerksTree()->TakePerk(24);}
        Require(unit->HasPerk(24)==acquired,"wound patient fixture failed real perk acquisition/without-perk control");
        if(acquired)Require(unit->HasPerk(24,&patientWoundMultiplier) && patientWoundMultiplier==1.5f,"wound patient fixture needs its authored1.5 multiplier");
    }
    NDb::CRPGFirstAid* record=nullptr;CDBIterator<NDb::CRPGFirstAid> kits(*NDatabase::GetTable<NDb::CRPGFirstAid>());
    while(kits.MoveNext()){auto* candidate=kits.Get();if(IsValid(candidate->pItem) && candidate->effect==(critical?NDb::FAE_REMOVE_BLEEDING:NDb::FAE_NORMAL) && candidate->nQuantity>0 && (critical || candidate->nSkillModifier>=0) && (authoredKit?candidate->GetRecordID()==authoredKit:critical || candidate->GetRecordID()==1)) {record=candidate;break;}}
    Require(record,"wound award fixture has no authored ordinary first-aid kit");
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);auto* healer=actor->GetUnitRPG()->GetRPGUnit();auto* target=patient->GetUnitRPG();auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous;
    if(inventory->Get(NDb::SLOT_1))previous=inventory->TakeOff(NDb::SLOT_1);
    if(mode!=4)Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"wound award fixture could not activate its real medical item");else inventory->Activate(NDb::SLOT_1);
    actor->animator.SetActiveItem(mode!=4);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());MarkNewDGFrame();world->GetAIMap()->Sync();
    auto& vp=target->GetRPGUnit()->Skills(NDb::ST_VP);const int wound=critical?(mode==9?vp.GetMaxValue()/2:0):patientWound?((mode==10 || mode==12)?vp.GetMaxValue()/2:3):mode==8?vp.GetMaxValue()/2:(mode==1 || mode==7)?3:1;vp.SetValue(vp.GetMaxValue()-wound);target->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_VP));
    auto bleeds=[](NRPG::IUnitMission* mission) {
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(mission,&refs);vector<CObj<NRPG::CCritical>> stored;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(5,&stored);}vector<NRPG::SCritical> result;
        for(auto entry:stored)if(entry->GetCritical().eCritical==NDb::C_BLEEDING)result.push_back(entry->GetCritical());return result;
    };
    if(critical) {
        target->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_BLEEDING,mode==3?0:-1,2.f,mode==8?0:1));
        if(mode==1)target->ApplyCritical(NRPG::SCritical(NDb::CL_TORSO,NDb::C_BLEEDING,-1,3.f,2));
        if(mode==7)actor->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_LEGS,NDb::C_BLEEDING,-1,4.f,2));
        if(mode==3 || mode==9)target->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,1));
    }
    auto initialBleeds=bleeds(target);const auto healerBleeds=bleeds(actor->GetUnitRPG());int criticalDC=0;
    for(const auto& entry:initialBleeds)criticalDC+=entry.nDC;
    if(critical)Require(initialBleeds.size()==(mode==1?2:1) && criticalDC==(mode==1?3:mode==8?0:1),"critical medal fixture did not retain its actual independent bleed locations/DC");
    const int rawVP=vp,ap=actor->GetAP(),quantity=kit->GetIncQuantity();const auto before=healer->medalInfos;const auto patientAwards=target->GetRPGUnit()->medalInfos;
    vector<bool> eligible(before.size(),false);int collecting=0;
    for(int i=0;i<int(before.size());++i) {
        const auto& info=before[i];if(!info.bIsCollectingPoints || info.bIsGained || info.bWillBeGiven || !IsValid(info.pMedal) || (info.pMedal->bIsRussianOnly && !game->IsActiveZoneRussian()))continue;
        const float threshold=info.nProbability==0?info.pMedal->nPointsToStart:info.pMedal->nPointsToAddProbability;
        Require(info.fPoints+NRPG::GetMedalPoints(critical?NRPG::MPC_HEALED_CRITICAL:NRPG::MPC_HEALED_WOUND,patientWound?22:mode==8?15:3)<threshold,"wound award fixture would cross an unrelated medal RNG threshold");eligible[i]=true;++collecting;
    }
    Require(mode==6?healer->bDisabled:!healer->bDisabled && collecting>0,"wound award fixture has no eligible actual medal chain");
    CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,patient);auto random=GlobalGameRandom();const auto result=executor->CanDoIt(actor->GetPosition());const int cost=executor->GetStartAP();
    std::printf(critical?"native critical heal award query: mode=%d actor=%d kit=%d result=%d AP=%d cost=%d medicine=%d collecting=%d disabled=%d wound=%d\n":"native wound award query: mode=%d actor=%d kit=%d result=%d AP=%d cost=%d medicine=%d collecting=%d disabled=%d wound=%d\n",mode,healer->GetPers()->GetRecordID(),record->GetRecordID(),int(result),ap,cost,int(healer->Skills(NDb::ST_MEDICINE)),collecting,int(healer->bDisabled),wound);std::fflush(stdout);
    if(mode==4 || mode==5) {
        Require(result==(mode==4?NWorld::UCR_UNAVAILABLE:NWorld::UCR_GENERAL_FAILURE) && actor->GetAP()==ap && kit->GetIncQuantity()==quantity && target->GetRPGUnit()->nHealedVP==0 && random.Get()==GlobalGameRandom().Get(),"rejected wound treatment changed AP, charges, dressing or RNG");return;
    }
    Require(result==NWorld::UCR_OK && ap>=cost && !world->IsRealTime(),"wound award fixture cannot afford its real turn-based medical action");
    executor->Run();const int afterRun=actor->GetAP();
    if(critical) {
        if(mode==3) {int bleed=-999;target->DoRegenerations(world,&bleed);initialBleeds=bleeds(target);criticalDC=0;Require(initialBleeds.empty() && bleed==0,"finite bleeding did not expire through native regeneration before medical completion");}
        executor->AnimationFinished();const auto afterBleeds=bleeds(target);const auto afterHealer=bleeds(actor->GetUnitRPG());
        Require(afterBleeds.empty() && (patient==actor?afterHealer.empty():afterHealer.size()==healerBleeds.size()),"critical blood-stop treatment changed the wrong recipient or retained a cured bleed");
        Require(int(vp)==rawVP && target->GetRPGUnit()->nHealedVP==0 && actor->GetAP()==afterRun && afterRun==ap-cost && kit->GetIncQuantity()==quantity-1 && !target->HasCritical(NDb::C_PATIENT) && !actor->IsPerformingAction() && !actor->HasCommand() && !patient->HasCommand(),"critical blood-stop medal path changed native health, AP, charge or state ownership");
        if(mode==3 || mode==9)Require(target->HasCritical(NDb::C_WEAPONSKILL_REDUCTION),"critical blood-stop medal path removed a separate nonbleeding effect");
        if(mode==9)Require(target->HasCritical(NDb::C_VP),"critical blood-stop medal path removed its ordinary wound penalty control");
        const float expectedEach=mode==6?0:NRPG::GetMedalPoints(NRPG::MPC_HEALED_CRITICAL,float(criticalDC));float total=0;bool correct=healer->medalInfos.size()==before.size();
        for(int n=0;n<int(before.size());++n) {
            const auto& old=before[n];const auto& current=healer->medalInfos[n];const float delta=current.fPoints-old.fPoints;total+=delta;
            correct=correct && std::abs(delta-(eligible[n]?expectedEach:0))<1e-5f && current.nProbability==old.nProbability && current.bWillBeGiven==old.bWillBeGiven && current.bIsGained==old.bIsGained;
        }
        if(patient!=actor)for(int n=0;n<int(patientAwards.size());++n)Require(target->GetRPGUnit()->medalInfos[n].fPoints==patientAwards[n].fPoints,"critical blood-stop treatment awarded the patient instead of its medic");
        std::printf("native critical heal award result: mode=%d bleeds=%d cured=%d criticalDC=%d healerBleeds=%d->%d collecting=%d actualTotal=%g expectedEach=%g expectedTotal=%g AP=%d->%d charges=%d->%d wound=%d\n",mode,int(initialBleeds.size()),int(initialBleeds.size()-afterBleeds.size()),criticalDC,int(healerBleeds.size()),int(afterHealer.size()),collecting,total,expectedEach,expectedEach*collecting,ap,actor->GetAP(),quantity,kit->GetIncQuantity(),int(target->HasCritical(NDb::C_VP)));std::fflush(stdout);
        Require(correct,"native critical blood-stop treatment omitted original medal progress for its cured critical DC");return;
    }
    // Another completed dressing can precede the current animation-completion callback.
    if(mode==3 || mode==7 || (!critical && mode==13)) {NRPG::SFirstAid prior;prior.fdVP=mode==3?1:2;prior.nMaxVP=999;target->HealVP(prior);}
    const int dressedBefore=target->GetRPGUnit()->nHealedVP;NRPG::SFirstAid plan;int requiredAP=0;
    const bool planned=healer->CreateFirstAid(&plan,afterRun,100,kit,target->GetRPGUnit(),&requiredAP);
    executor->AnimationFinished();const int dressedAfter=target->GetRPGUnit()->nHealedVP,effective=dressedAfter-dressedBefore;
    const int expectedAmount=patientWound?(mode==10?(afterRun==53?22:18):mode==12?(afterRun==53?15:12):mode==13?1:3):mode==8?(authoredKit==6?14:afterRun==53?15:12):mode==3?0:mode==1?3:1;
    if(patientWound) {
        Require(int(healer->Skills(NDb::ST_MEDICINE))==17 && record->nSkillModifier==0 && (afterRun==53 || afterRun==43),"patient wound fixture needs its actual stock kit/Medicine17/AP budget");
        const int expectedQuota=patientWoundMultiplier==1.5f?44:30;
        const int expectedAP=(mode==10 || mode==12)?afterRun:mode==13?2:mode==9?7:10;
        std::printf("native patient wound perk: mode=%d multiplier=%g wound=%d dressedBefore=%d planned=%g quota=%d quotedAP=%d effective=%d expectedHP=%d expectedQuota=%d expectedAP=%d AP=%d->%d dose=%d->%d\n",mode,patientWoundMultiplier,wound,dressedBefore,planned?plan.fdVP:-1,planned?plan.nMaxVP:-1,requiredAP,effective,expectedAmount,expectedQuota,expectedAP,afterRun,actor->GetAP(),quantity,kit->GetIncQuantity());std::fflush(stdout);
        Require(planned && plan.fdVP==expectedAmount && plan.nMaxVP==expectedQuota && requiredAP==expectedAP && effective==expectedAmount && actor->GetAP()==afterRun-expectedAP,"wound treatment ignored the original patient healing multiplier24");
    }
    Require(effective==expectedAmount && int(vp)==rawVP && actor->GetAP()==afterRun-(planned?requiredAP:0) && afterRun==ap-cost,"wound award fixture did not perform its actual small native dressing with unchanged AP accounting/raw health");
    if(woundFinish) {
        if(mode==8)Require(afterRun==53 || afterRun==43,"unfinished wound fixture needs its native stock-kit AP budget");
        const bool finished=mode!=8;
        const bool marker=target->HasCritical(NDb::C_PATIENT);
        std::printf("native wound completion: mode=%d wound=%d before=%d dressed=%d marker=%d dose=%d->%d expectedFinished=%d AP=%d\n",mode,wound,dressedBefore,dressedAfter,int(marker),quantity,kit->GetIncQuantity(),int(finished),actor->GetAP());std::fflush(stdout);
        Require(marker!=finished && kit->GetIncQuantity()==quantity-int(finished),"wound treatment did not finish its native patient/dose state when dressing was complete");
        if(finished) {
            const int finishedAP=actor->GetAP();actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);
            Require(actor->GetAP()==finishedAP && kit->GetIncQuantity()==quantity-1 && !target->HasCritical(NDb::C_PATIENT),"completed wound treatment was repeated at own-turn finish");
        }
    }
    if(authoredKit) {
        Require(mode==8 && int(healer->Skills(NDb::ST_MEDICINE))==17 && record->nSkillModifier==0,"authored capacity fixture needs its real pending Medicine17 dressing");
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(actor,&refs);CObj<NWorld::CUnitState> active;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(10,&active);}
        Require(dynamic_cast<NWorld::CUnitStateHealer*>(active.GetPtr()),"authored capacity treatment did not retain its native pending healer state");
        CStructureNetworkContext stateRefs;auto saved=BorrowedObjectFields(active,&stateRefs);float remaining=-999;
        {CStructureSaver reader(saved,CStructureSaver::READ,&stateRefs);reader.Add(4,&remaining);}
        const float expected=record->nTotalHealVP-effective;
        std::printf("native authored wound capacity: kit=%d total=%d effective=%d remaining=%g expected=%g AP=%d->%d dose=%d->%d marker=%d\n",record->GetRecordID(),record->nTotalHealVP,effective,remaining,expected,ap,actor->GetAP(),quantity,kit->GetIncQuantity(),int(target->HasCritical(NDb::C_PATIENT)));std::fflush(stdout);
        Require(remaining==expected && kit->GetIncQuantity()==quantity && target->HasCritical(NDb::C_PATIENT),"normal wound treatment replaced its authored kit capacity with a fixed budget");
    }
    // Native cancellation finishes a still-active legacy healer state and spends its dose.
    actor->SetState(new NWorld::CUnitStateNormal(actor));
    Require(kit->GetIncQuantity()==quantity-1 && !target->HasCritical(NDb::C_PATIENT) && !actor->IsPerformingAction() && !actor->HasCommand() && !patient->HasCommand(),"wound award fixture did not finish native kit/patient/action ownership");
    Require(healer->medalInfos.size()==before.size(),"wound award treatment changed the authored medal chain");
    const float expectedPoints=mode==6?0:NRPG::GetMedalPoints(NRPG::MPC_HEALED_WOUND,float(effective));float total=0;bool correct=true;
    for(int i=0;i<int(before.size());++i) {
        const auto& old=before[i];const auto& current=healer->medalInfos[i];const float delta=current.fPoints-old.fPoints;total+=delta;
        correct=correct && std::abs(delta-(eligible[i]?expectedPoints:0))<1e-5f && current.nProbability==old.nProbability && current.bWillBeGiven==old.bWillBeGiven && current.bIsGained==old.bIsGained;
    }
    if(patient!=actor) {
        Require(target->GetRPGUnit()->medalInfos.size()==patientAwards.size(),"wound treatment changed the patient's medal chain");
        for(int i=0;i<int(patientAwards.size());++i)Require(target->GetRPGUnit()->medalInfos[i].fPoints==patientAwards[i].fPoints,"wound treatment awarded the patient instead of the medic");
    }
    std::printf("native wound award result: mode=%d planned=%g effective=%d alreadyDressed=%d requiredAP=%d collecting=%d actualTotal=%g expectedEach=%g expectedTotal=%g charges=%d->%d\n",mode,planned?plan.fdVP:0,effective,dressedBefore,planned?requiredAP:0,collecting,total,expectedPoints,expectedPoints*collecting,quantity,kit->GetIncQuantity());std::fflush(stdout);
    Require(correct,"native wound treatment omitted the original medal progress for its actual effective healing");
    if(woundBudget) {
        Require(int(healer->Skills(NDb::ST_MEDICINE))==17 && record->nSkillModifier==0,"wound budget fixture requires its actual Medicine17, unmodified stock kit");
        const int expectedAP=mode==1?10:mode==3?0:3;
        const bool expectedPlan=expectedAmount>0;
        std::printf("native wound budget result: mode=%d wound=%d dressed=%d planned=%g effective=%d AP=%d->%d spent=%d expectedAP=%d charges=%d->%d\n",mode,wound,dressedBefore,planned?plan.fdVP:0,effective,afterRun,actor->GetAP(),afterRun-actor->GetAP(),expectedAP,quantity,kit->GetIncQuantity());std::fflush(stdout);
        Require(planned==expectedPlan && (!planned || (plan.fdVP==expectedAmount && requiredAP==expectedAP)) && actor->GetAP()==afterRun-expectedAP,"wound treatment charged AP for wounds already dressed before completion");
    }
}

static void PenaltyTreatment(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;NWorld::CUnitServer* patient=nullptr;
    auto ordinary=[](NWorld::CUnitServer* unit) {return unit && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->HasCommand() && !unit->IsPerformingAction() && unit->GetState()!=NWorld::CUnit::ST_MACHINE_GUN;};
    for(auto first:units) {
        if(!ordinary(first) || int(first->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_MEDICINE))<=0 || !dynamic_cast<NRPG::CWeaponItem*>(first->GetUnitRPG()->GetInventory()->GetActive()))continue;
        for(auto second:units)if(ordinary(second) && first!=second && first->GetPlayer()==second->GetPlayer()) {actor=first;patient=second;break;}
        if(actor)break;
    }
    Require(actor && patient,"penalty treatment lacks two idle authored friendly actors and a medic");
    CObj<NWorld::CUnitServer> spawned;
    if(mode==7) {
        NDb::CRPGPers* persona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
        while(people.MoveNext()) {
            auto* candidate=people.Get();if(!IsValid(candidate->pClass) || !IsValid(candidate->pBaseValue) || !IsValid(candidate->pModel) || !IsValid(candidate->pName) || candidate->pClass->skills[NDb::ST_LEVEL]<=0 || IsValid(candidate->pPanzerklein) || IsValid(candidate->pDefaultWearsPanzerklein))continue;
            CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(candidate->pClass->nPerkTreeID);vector<CPtr<NRPG::CPerk>> entries;tree->GetAllPerks(&entries);
            for(auto entry:entries)if(entry->GetDBPerk()->GetRecordID()==64) {persona=candidate;break;}
            if(persona)break;
        }
        Require(persona,"penalty duration fixture lacks an actual ordinary class containing perk64");
        CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(persona);
        spawned=world->AddUnitInGame(actor->GetPosition().pos.p,mission,actor->GetTBSPlayer());actor=spawned;
        Require(ordinary(actor),"penalty duration fixture could not deploy its authored medic");
        auto* unit=mission->GetRPGUnit();Require(!unit->HasPerk(64),"penalty duration control already owns perk64");
        unit->GetPerksTree()->AddPerkPoints(1);unit->GetPerksTree()->TakePerk(64);
        float parameter=0;Require(unit->HasPerk(64,&parameter) && parameter>1,"penalty duration fixture could not acquire its authored duration ability");
    }
    const bool self=mode==2 || mode==3 || mode>=7;
    if(self)patient=actor;
    if(mode==5) {
        patient=nullptr;
        for(auto candidate:units)if(ordinary(candidate) && candidate!=actor && candidate->GetDiplomacyState(actor)==NDb::DS_ENEMY) {patient=candidate;break;}
        Require(patient,"penalty treatment lacks a real hostile rejection target");
    }
    if(patient!=actor && mode!=5) {
        auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),1.5f),&places);bool placed=false;
        for(auto place:places) {
            place.SetPose(NAI::CM_STAND);if(!network->IsPassable(place) || network->IsLocked(place,false))continue;
            auto position=patient->GetPosition();position.pos=NAI::SPosition(place,network);const float distance=fabs(position.GetCP()-actor->GetPosition().GetCP());
            if(distance<.3f || !NWorld::IsWithinHumanReach(actor->GetPosition().GetCP(),position.GetCP(),.8f))continue;
            world->GetTime()->Set(world->GetTime()->GetValue()+1);patient->SetPosition(position);patient->animator.AlignTime();patient->animator.PlaceUnit(position);placed=true;break;
        }
        Require(placed,"penalty treatment has no native reachable free patient position");
    }
    NDb::CRPGFirstAid* record=nullptr;CDBIterator<NDb::CRPGFirstAid> kits(*NDatabase::GetTable<NDb::CRPGFirstAid>());
    while(kits.MoveNext()) {auto* candidate=kits.Get();if(IsValid(candidate->pItem) && candidate->effect==NDb::FAE_TEMP_REMOVE_PENALTIES && candidate->nQuantity>0 && candidate->nDuration>1 && candidate->fPower>0 && candidate->fPower<100) {record=candidate;break;}}
    Require(record,"penalty treatment lacks an actual authored timed penalty drug");
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous;
    if(mode!=6) {
        if(inventory->Get(NDb::SLOT_1))previous=inventory->TakeOff(NDb::SLOT_1);
        if(mode!=4)Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"penalty treatment could not equip its actual drug");else inventory->Activate(NDb::SLOT_1);
        actor->animator.SetActiveItem(mode!=4);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    }
    MarkNewDGFrame();world->GetAIMap()->Sync();auto* healer=actor->GetUnitRPG();auto* target=patient->GetUnitRPG();
    if(mode!=1 && mode!=9)healer->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,20));
    if(patient!=actor)target->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_WEAPONSKILL_REDUCTION,-1,3.f,40));
    if(mode==0) {auto& vp=target->GetRPGUnit()->Skills(NDb::ST_VP);vp.SetValue(vp.GetTheoreticalMax()/2);target->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_VP));Require(target->HasCritical(NDb::C_VP),"penalty treatment did not install its native wound penalty control");}
    const bool healerBefore=healer->HasCritical(NDb::C_WEAPONSKILL_REDUCTION),patientBefore=target->HasCritical(NDb::C_WEAPONSKILL_REDUCTION),woundBefore=target->HasCritical(NDb::C_VP);
    const int ap=actor->GetAP(),quantity=kit->GetIncQuantity(),healerVP=healer->GetTotalVP(),patientVP=target->GetTotalVP(),healerCap=healer->GetRPGUnit()->Skills(NDb::ST_VP).GetMaxValue(),patientCap=target->GetRPGUnit()->Skills(NDb::ST_VP).GetMaxValue();
    const int patientBandage=target->GetRPGUnit()->nHealedVP;float multiplier=1,parameter=0;if(healer->GetRPGUnit()->HasPerk(64,&parameter))multiplier=parameter;const int duration=int(record->nDuration*multiplier);
    Require(duration>1 && duration<100,"penalty treatment actual duration is outside its finite test bound");
    struct Suspended {vector<CObj<NRPG::CCritical>> criticals;int left=0;int operator&(CStructureSaver& f){f.Add(2,&criticals);f.Add(3,&left);return 0;}};
    struct Postponed {CPtr<NRPG::CDynamicSkill> target;NRPG::SSkillModifyInfo info;int left=0;int operator&(CStructureSaver& f){f.Add(2,&target);f.Add(3,&info);f.Add(4,&left);return 0;}};
    struct State {list<Suspended> suspended;list<Postponed> postponed;vector<CObj<NRPG::CSkillModifier>> active;int operator&(CStructureSaver& f){f.Add(34,&suspended);f.Add(36,&postponed);f.Add(37,&active);return 0;}};
    auto state=[](NRPG::IUnitMission* mission) {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(mission,&refs);State result;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);result.operator&(reader);}return result;};
    Require(state(target).suspended.empty() && state(target).postponed.empty() && state(healer).suspended.empty(),"penalty treatment fixture already has a pending drug");
    // The no-critical completion control starts valid, then loses its injury during preparation.
    if(mode==9)target->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,1));
    CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,patient);auto random=GlobalGameRandom();const auto result=executor->CanDoIt(actor->GetPosition());const int cost=executor->GetStartAP();
    std::printf("native penalty drug query: mode=%d kit=%d power=%g duration=%d multiplier=%g expectedDuration=%d result=%d AP=%d cost=%d self=%d\n",mode,record->GetRecordID(),record->fPower,record->nDuration,multiplier,duration,int(result),ap,cost,int(self));std::fflush(stdout);
    if(mode>=4 && mode<=6) {
        Require(result==(mode==5?NWorld::UCR_GENERAL_FAILURE:NWorld::UCR_UNAVAILABLE) && actor->GetAP()==ap && kit->GetIncQuantity()==quantity && healer->HasCritical(NDb::C_WEAPONSKILL_REDUCTION)==healerBefore && target->HasCritical(NDb::C_WEAPONSKILL_REDUCTION)==patientBefore && state(target).postponed.empty() && random.Get()==GlobalGameRandom().Get(),"rejected penalty drug changed criticals, AP, charges, deferred effects or RNG");return;
    }
    Require(result==NWorld::UCR_OK && cost>=0 && ap>=cost,"penalty drug failed its real availability/AP gate");
    executor->Run();const int afterRun=actor->GetAP();
    if(mode==9)target->RemoveCritical(NDb::C_WEAPONSKILL_REDUCTION);
    executor->AnimationFinished();
    const bool healerAfter=healer->HasCritical(NDb::C_WEAPONSKILL_REDUCTION),patientAfter=target->HasCritical(NDb::C_WEAPONSKILL_REDUCTION);auto initial=state(target);
    std::printf("native penalty drug initial: mode=%d healer=%d->%d patient=%d->%d suspended=%d left=%d postponed=%d AP=%d->%d->%d charges=%d->%d wound=%d\n",mode,int(healerBefore),int(healerAfter),int(patientBefore),int(patientAfter),int(initial.suspended.size()),initial.suspended.empty()?-1:initial.suspended.front().left,int(initial.postponed.size()),ap,afterRun,actor->GetAP(),quantity,kit->GetIncQuantity(),int(target->HasCritical(NDb::C_VP)));std::fflush(stdout);
    Require(actor->GetAP()==afterRun && (world->IsRealTime() || afterRun==ap-cost) && kit->GetIncQuantity()==quantity-1 && healer->GetTotalVP()==healerVP && target->GetTotalVP()==patientVP && healer->GetRPGUnit()->Skills(NDb::ST_VP).GetMaxValue()==healerCap && target->GetRPGUnit()->Skills(NDb::ST_VP).GetMaxValue()==patientCap && target->GetRPGUnit()->nHealedVP==patientBandage && !target->HasCritical(NDb::C_PATIENT) && !actor->IsPerformingAction() && !actor->HasCommand() && !patient->HasCommand(),"penalty drug changed immediate health, cost, charge or native completion ownership");
    Require(!patientAfter && healerAfter==(self?false:healerBefore) && target->HasCritical(NDb::C_VP)==woundBefore,"penalty drug suspended the healer instead of its patient or erased the wound control");
    const float expectedMul=1-record->fPower*.01f;
    if(mode!=3 && mode!=8 && mode!=9)Require(initial.suspended.size()==1 && initial.suspended.front().left==duration && initial.postponed.size()==1 && initial.postponed.front().left==duration && initial.postponed.front().target.GetPtr()==&target->GetRPGUnit()->Skills(NDb::ST_VP) && std::abs(initial.postponed.front().info.fMul-expectedMul)<1e-6f && initial.postponed.front().info.fAdd==0,"penalty drug omitted its original patient duration or postponed health modifier");
    CObj<NRPG::IUnitMission> restored;auto* measured=target;
    if(mode==8) {
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(target,&refs);restored=dynamic_cast<NRPG::IUnitMission*>(pSSClasses->CreateObject(pSSClasses->GetObjectTypeID(target)));Require(restored,"penalty drug mission restore factory failed");
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);}measured=restored;
    }
    const int expectedCap=Float2Int(measured->GetRPGUnit()->Skills(NDb::ST_VP).GetTheoreticalMax()*expectedMul),expectedVP=(std::min)(patientVP,expectedCap);
    for(int tick=1;tick<=duration;++tick) {
        int bleed=-999;measured->DoRegenerations(world,&bleed);const int cap=measured->GetRPGUnit()->Skills(NDb::ST_VP).GetMaxValue(),vp=measured->GetTotalVP();const bool critical=measured->HasCritical(NDb::C_WEAPONSKILL_REDUCTION);
        if(tick<duration)Require(!critical && cap==patientCap && vp==patientVP,"penalty drug expired or changed health before its original duration");
        else {
            auto final=state(measured);
            std::printf("native penalty drug expiry: mode=%d tick=%d critical=%d expectedCritical=%d cap=%d expectedCap=%d VP=%d expectedVP=%d postponed=%d active=%d restored=%d\n",mode,tick,int(critical),int(patientBefore),cap,expectedCap,vp,expectedVP,int(final.postponed.size()),int(final.active.size()),int(bool(restored)));std::fflush(stdout);
            Require(critical==patientBefore && cap==expectedCap && vp==expectedVP && final.postponed.empty() && final.active.size()==initial.active.size()+1,"penalty drug did not apply its original delayed health cost when criticals returned");
        }
    }
    if(!self)Require(healer->GetTotalVP()==healerVP && healer->GetRPGUnit()->Skills(NDb::ST_VP).GetMaxValue()==healerCap && healer->HasCritical(NDb::C_WEAPONSKILL_REDUCTION)==healerBefore,"patient penalty drug changed the independent medic after expiry");
}

static void BleedingTreatmentRecipient(const char* databasePath,const char* resourcePath,int mode,bool temporary=false)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;NWorld::CUnitServer* patient=nullptr;
    auto ordinary=[](NWorld::CUnitServer* unit) { return unit && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->HasCommand() && !unit->IsPerformingAction() && unit->GetState()!=NWorld::CUnit::ST_MACHINE_GUN; };
    for(auto first:units) {
        if(!ordinary(first) || int(first->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_MEDICINE))<=0 ||
            !dynamic_cast<NRPG::CWeaponItem*>(first->GetUnitRPG()->GetInventory()->GetActive()))continue;
        for(auto second:units)if(ordinary(second) && first!=second && first->GetPlayer()==second->GetPlayer()) { actor=first;patient=second;break; }
        if(actor)break;
    }
    Require(actor && patient,"bleeding recipient fixture lacks two idle native friendly actors and an armed medic");
    CObj<NWorld::CUnitServer> qualified;if(!temporary){qualified=QualifiedMedicalActor(world,actor);actor=qualified;}
    if(mode==2)patient=actor;
    if(mode==5) {
        patient=nullptr;
        for(auto candidate:units)if(ordinary(candidate) && candidate!=actor && candidate->GetDiplomacyState(actor)==NDb::DS_ENEMY) {patient=candidate;break;}
        Require(patient!=nullptr,"bleeding recipient rejection lacks a real hostile patient");
    }
    if(patient!=actor && mode!=5) {
        auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;
        network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),1.5f),&places);bool placed=false;
        for(auto place:places) {
            place.SetPose(NAI::CM_STAND);
            if(!network->IsPassable(place) || network->IsLocked(place,false))continue;
            auto position=patient->GetPosition();position.pos=NAI::SPosition(place,network);
            const float distance=fabs(position.GetCP()-actor->GetPosition().GetCP());
            if(distance<.3f || !NWorld::IsWithinHumanReach(actor->GetPosition().GetCP(),position.GetCP(),.8f))continue;
            world->GetTime()->Set(world->GetTime()->GetValue()+1);patient->SetPosition(position);patient->animator.AlignTime();patient->animator.PlaceUnit(position);placed=true;break;
        }
        Require(placed,"bleeding recipient fixture has no native reachable free patient placement");
    }
    NDb::CRPGFirstAid* record=nullptr;CDBIterator<NDb::CRPGFirstAid> kits(*NDatabase::GetTable<NDb::CRPGFirstAid>());
    while(kits.MoveNext()) { auto* candidate=kits.Get();if(IsValid(candidate->pItem) && candidate->effect==(temporary?NDb::FAE_TEMP_STOP_BLEEDING:NDb::FAE_REMOVE_BLEEDING) && candidate->nQuantity>0 && (!temporary || candidate->GetRecordID()==15)) {record=candidate;break;} }
    Require(record!=nullptr,"bleeding recipient fixture lacks an authored blood-stop item");
    CObj<NRPG::CFirstAidItem> kit=new NRPG::CFirstAidItem(record);auto* inventory=actor->GetUnitRPG()->GetInventory();
    CObj<NRPG::IInventoryItem> previous;
    if(mode!=6) {
        if(inventory->Get(NDb::SLOT_1))previous=inventory->TakeOff(NDb::SLOT_1);
        if(mode!=4)Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),"bleeding recipient fixture could not activate its real medical item");
        else inventory->Activate(NDb::SLOT_1);
        actor->animator.SetActiveItem(mode!=4);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    }
    MarkNewDGFrame();world->GetAIMap()->Sync();
    auto* healer=actor->GetUnitRPG();auto* target=patient->GetUnitRPG();
    if(mode!=1)healer->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_BLEEDING,-1,2.f,20));
    if(patient!=actor)target->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_BLEEDING,-1,3.f,40));
    if(mode==3) {healer->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,1));target->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_WEAPONSKILL_REDUCTION,-1,2.f,1));}
    const bool healerBefore=healer->HasCritical(NDb::C_BLEEDING),patientBefore=target->HasCritical(NDb::C_BLEEDING);
    Require(patientBefore && healerBefore==(mode!=1),"bleeding recipient fixture did not install its real independent criticals");
    auto stopped=[](NRPG::IUnitMission* mission) {
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(mission,&refs);int value=-999;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(35,&value);}return value;
    };
    const int beforeHealerStop=stopped(healer),beforePatientStop=stopped(target);
    Require(beforeHealerStop==0 && beforePatientStop==0,"native blood-stop fixture unexpectedly starts with saved suppression");
    const int ap=actor->GetAP(),quantity=kit->GetIncQuantity(),healerVP=healer->GetTotalVP(),patientVP=target->GetTotalVP();
    const int healerBandage=healer->GetRPGUnit()->nHealedVP,patientBandage=target->GetRPGUnit()->nHealedVP;
    CObj<NWorld::CExecHeal> executor=new NWorld::CExecHeal(actor,patient);auto random=GlobalGameRandom();
    const auto result=executor->CanDoIt(actor->GetPosition());const int cost=executor->GetStartAP();
    std::printf("native bleeding recipient query: mode=%d kit=%d charges=%d medicine=%d result=%d AP=%d cost=%d realtime=%d distance=%g\n",mode,record->GetRecordID(),quantity,int(healer->GetRPGUnit()->Skills(NDb::ST_MEDICINE)),int(result),ap,cost,int(world->IsRealTime()),fabs(actor->GetPosition().GetCP()-patient->GetPosition().GetCP()));std::fflush(stdout);
    if(mode>=4 && mode<=6) {
        const auto expected=mode==5?NWorld::UCR_GENERAL_FAILURE:NWorld::UCR_UNAVAILABLE;
        Require(result==expected && healer->HasCritical(NDb::C_BLEEDING)==healerBefore && target->HasCritical(NDb::C_BLEEDING)==patientBefore && actor->GetAP()==ap && kit->GetIncQuantity()==quantity && random.Get()==GlobalGameRandom().Get(),"rejected medical use changed bleeding, AP, charges or RNG");return;
    }
    Require(result==NWorld::UCR_OK && cost>=0 && ap>=cost,"bleeding recipient native action failed its actual availability/AP gate");
    executor->Run();const int afterRun=actor->GetAP();
    // Drive the public native completion callback; this does not claim to play the animation clock.
    executor->AnimationFinished();
    const bool healerAfter=healer->HasCritical(NDb::C_BLEEDING),patientAfter=target->HasCritical(NDb::C_BLEEDING);
    std::printf("native bleeding recipient effect: mode=%d healer=%d->%d patient=%d->%d AP=%d->%d->%d charges=%d->%d state=%d\n",mode,int(healerBefore),int(healerAfter),int(patientBefore),int(patientAfter),ap,afterRun,actor->GetAP(),quantity,kit->GetIncQuantity(),int(actor->GetState()));std::fflush(stdout);
    Require(actor->GetAP()==afterRun && (world->IsRealTime() || afterRun==ap-cost) && kit->GetIncQuantity()==quantity-1 && !target->HasCritical(NDb::C_PATIENT) && !actor->HasCommand() && !patient->HasCommand() && !actor->IsPerformingAction(),"instant blood-stop completion changed AP cost, charge use, patient marker or native action lifecycle");
    Require(healer->GetTotalVP()==healerVP && target->GetTotalVP()==patientVP && healer->GetRPGUnit()->nHealedVP==healerBandage && target->GetRPGUnit()->nHealedVP==patientBandage,"blood-stop effect changed raw health or wound dressing");
    if(mode==3)Require(healer->HasCritical(NDb::C_WEAPONSKILL_REDUCTION) && target->HasCritical(NDb::C_WEAPONSKILL_REDUCTION),"blood-stop item removed a separate nonbleeding critical");
    if(temporary) {
        Require(healerAfter==healerBefore && patientAfter==patientBefore,"temporary blood suppression erased its bleeding critical");
        const int power=Float2Int(record->fPower);Require(power>0,"native authored temporary blood-stop item has no positive power");
        int uses=1;
        if(mode==7) {
            CObj<NRPG::CFirstAidItem> second=new NRPG::CFirstAidItem(record);
            if(inventory->Get(NDb::SLOT_1))inventory->TakeOff(NDb::SLOT_1);
            Require(inventory->Equip(NDb::SLOT_1,second) && inventory->Activate(NDb::SLOT_1),"repeat suppression could not activate a second authored dose");
            actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
            CObj<NWorld::CExecHeal> again=new NWorld::CExecHeal(actor,patient);
            const int nextAP=actor->GetAP(),nextCost=again->GetStartAP(),nextQuantity=second->GetIncQuantity();
            Require(again->CanDoIt(actor->GetPosition())==NWorld::UCR_OK && nextAP>=nextCost,"second temporary blood-stop use is not available");
            again->Run();again->AnimationFinished();uses=2;
            Require(actor->GetAP()==nextAP-nextCost && second->GetIncQuantity()==nextQuantity-1 && !actor->IsPerformingAction() && !target->HasCritical(NDb::C_PATIENT),"repeated temporary blood suppression changed its cost, charge or completion lifecycle");
        }
        CObj<NRPG::IUnitMission> restored;auto* measured=target;
        if(mode==8) {
            CStructureNetworkContext refs;auto fields=BorrowedObjectFields(target,&refs);
            restored=dynamic_cast<NRPG::IUnitMission*>(pSSClasses->CreateObject(pSSClasses->GetObjectTypeID(target)));
            Require(restored,"temporary blood-stop saved mission factory failed");
            {CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);}measured=restored;
        }
        const int healerStop=stopped(healer),patientStop=stopped(measured);
        int healerBleed=-999,patientBleed=-999;healer->DoRegenerations(world,&healerBleed);measured->DoRegenerations(world,&patientBleed);
        const int expectedHealer=mode==2?power:0,expectedPatient=power*uses;
        const int expectedDamage=(std::max)(0,(mode==2?2:3)-expectedPatient);
        std::printf("native temporary bleeding effect: mode=%d kit=%d power=%d uses=%d healerStop=%d patientStop=%d expectedHealer=%d expectedPatient=%d healerBleed=%d patientBleed=%d expectedDamage=%d restored=%d\n",mode,record->GetRecordID(),power,uses,healerStop,patientStop,expectedHealer,expectedPatient,healerBleed,patientBleed,expectedDamage,int(bool(restored)));std::fflush(stdout);
        Require(healer->GetTotalVP()==healerVP && measured->GetTotalVP()==patientVP && measured->HasCritical(NDb::C_BLEEDING) && !actor->HasCommand() && !patient->HasCommand(),"temporary bleeding regeneration changed health, removed the wound or issued an order");
        Require(healerStop==expectedHealer && patientStop==expectedPatient && patientBleed==expectedDamage && healerBleed==(mode==2?expectedDamage:mode==1?0:2),"temporary blood-stop item did not add its original suppression to the selected patient");return;
    }
    Require(!patientAfter && healerAfter==(mode==2?false:healerBefore),"blood-stop item cured the medic instead of its selected patient");
}
