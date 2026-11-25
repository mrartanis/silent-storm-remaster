static void LadderPassabilityLayerBounds()
{
    auto network=SimplePathNetwork(2);
    for(int layer=0;layer<network->GetNumLayers();++layer) {
        NAI::CNodesLayer::SLadder ladder{};
        ladder.eDir=NAI::LD_RIGHT;ladder.bNeedRecalc=false;ladder.bConsistent=true;
        ladder.pointPassable.assign(8,1);ladder.pointOnUpperHalf.assign(8,0);ladder.nLocks.assign(8,0);
        network->GetLayer(layer)->ladders.push_back(ladder);
        NAI::SPathPlace valid(0,0,layer,4,NAI::CM_STAND,0);valid.SetIntegral(0);
        Require(network->GetPassability(valid)==NAI::AIP_YES,"ladder layer-bounds fixture rejected its valid layer control");
    }
    for(int invalidLayer: {network->GetNumLayers(),255}) {
        NAI::SPathPlace ground(4,4,invalidLayer,0,NAI::CM_STAND,0);
        Require(network->GetPassability(ground)==NAI::AIP_NOT_PASSABLE,"ground query lost its existing invalid-layer guard");
        NAI::SPathPlace flight(0,0,invalidLayer,4,NAI::CM_STAND,0);flight.SetIntegral(0);flight.SetFinal(1);
        Require(network->GetPassability(flight)==NAI::AIP_YES && network->IsPassable(flight) && NAI::IsBigLockerPlacePassable(network,flight),
            "layer-bounds checking interpreted flying altitude as a ground-layer index");
    }
    for(int invalidLayer: {network->GetNumLayers(),255}) {
        NAI::SPathPlace invalid(0,0,invalidLayer,4,NAI::CM_STAND,0);invalid.SetIntegral(0);
        std::printf("native ladder layer-bounds: layer=%d available=%d valid/flying controls passed\n",invalidLayer,network->GetNumLayers());std::fflush(stdout);
        Require(network->GetPassability(invalid)==NAI::AIP_NOT_PASSABLE,"invalid ladder layer was dereferenced instead of rejected");
    }
}

static void LadderPassabilityFacing()
{
    auto network = SimplePathNetwork();
    auto* layer = network->GetLayer(0);
    const NAI::ELadderDirection directions[] = {NAI::LD_RIGHT,NAI::LD_FRONT,NAI::LD_LEFT,NAI::LD_BACK};
    // Independent values from the original executable: left, down, right, up.
    const int expectedFacing[] = {4,6,0,2};
    for (auto direction: directions) {
        NAI::CNodesLayer::SLadder ladder{};
        ladder.eDir = direction;
        ladder.pointPassable.assign(8,1);
        ladder.pointOnUpperHalf.assign(8,0);
        ladder.nLocks.assign(8,0);
        ladder.bNeedRecalc = false;
        ladder.bConsistent = true;
        layer->ladders.push_back(ladder);
    }
    auto place = [](int ladder,int step,int facing) {
        NAI::SPathPlace result(ladder,step,0,facing,NAI::CM_STAND,0);
        result.SetIntegral(0);result.SetFinal(0);return result;
    };
    for (int index=0;index<4;++index) {
        auto& ladder = layer->ladders[index];
        for (int step=0;step<8;++step) {
            const auto correct = place(index,step,expectedFacing[index]);
            Require(network->GetPassability(correct)==NAI::AIP_YES && network->IsPassable(correct),
                "native ladder fixture rejected a passable, unlocked rung facing the ladder");
        }
        ladder.pointPassable[3]=0;ladder.nLocks[3]=1;
        Require(network->GetPassability(place(index,3,expectedFacing[index]))==NAI::AIP_NOT_PASSABLE &&
            !network->IsPassable(place(index,3,expectedFacing[index])),"a broken ladder rung lost precedence over its lock");
        ladder.pointPassable[3]=1;
        for(int facing=0;facing<8;++facing)
            Require(network->GetPassability(place(index,3,facing))==NAI::AIP_LOCKED && !network->IsPassable(place(index,3,facing)),
                "native ladder locks lost precedence or legacy bool occupancy checks changed");
        ladder.nLocks[3]=0;
    }
    for(int index=0;index<4;++index) for(int step=0;step<8;++step) for(int facing=0;facing<8;++facing) {
        const auto point = place(index,step,facing);
        const auto expected = facing==expectedFacing[index] ? NAI::AIP_YES : NAI::AIP_NOT_PASSABLE;
        const auto actual = network->GetPassability(point);
        if(actual!=expected)std::printf("native ladder facing: ladder=%d step=%d facing=%d expectedFacing=%d verdict=%d\n",
            index,step,facing,expectedFacing[index],int(actual));
        Require(actual==expected,"granular ladder passability accepted a unit facing away from the ladder");
        Require(network->IsPassable(point),"granular ladder direction checking changed the legacy bool occupancy query");
    }
    for(const auto& ladder: layer->ladders)
        Require(!ladder.bNeedRecalc && ladder.bConsistent && ladder.pointPassable==vector<char>(8,1) && ladder.nLocks==vector<char>(8,0),
            "querying ladder facing mutated its native geometry or occupancy");
}

static void AnchorPassability()
{
    auto network = SimplePathNetwork(2);
    const NAI::SPathPlace prone(4,4,0,3,NAI::CM_LAY,0);
    auto standing = prone; standing.SetPose(NAI::CM_STAND);
    auto findPosePath = [&]() {
        return CObj<NAI::CPath>(NAI::FindSimplePath(standing,{prone},NAI::PF_USE_POSEDIR,network,false,false,false));
    };
    Require(network->GetPassability(prone) == NAI::AIP_YES && network->IsPassable(prone) && IsValid(findPosePath()),
        "anchor passability fixture failed its unlocked native pose-path control");
    vector<NAI::SPathPlace> footprint; network->GetLockArea(&footprint,prone,false);
    auto neighbour = std::find_if(footprint.begin(),footprint.end(),[&](const NAI::SPathPlace& point) {
        return point.GetLayer() == prone.GetLayer() && (point.GetX() != prone.GetX() || point.GetY() != prone.GetY());
    });
    Require(neighbour != footprint.end(),"anchor passability fixture has no real expanded prone footprint");
    auto occupied = *neighbour; occupied.SetPose(NAI::CM_STAND);
    CObj<LockerActor> first = new LockerActor, second = new LockerActor;
    network->Lock(first,occupied); network->Lock(second,occupied);
    const auto* layer = network->GetLayer(prone.GetLayer());
    Require(layer->tiles[prone.GetY()][prone.GetX()].nLocks == 0 &&
        layer->tiles[occupied.GetY()][occupied.GetX()].nLocks != 0 && network->IsLocked(occupied,true),
        "anchor passability fixture did not isolate an unlocked anchor from an occupied neighbour");
    for (int pose: {NAI::CM_STAND,NAI::CM_CROUCH}) {
        auto point = prone; point.SetPose(pose);
        Require(network->GetPassability(point) == NAI::AIP_YES && network->IsPassable(point),
            "an occupied neighbouring cell blocked the native standing/crouching anchor control");
    }
    const auto verdict = network->GetPassability(prone);
    std::printf("native anchor passability: anchorLocks=%d neighbourLocks=%d verdict=%d strictPassable=%d\n",
        int(layer->tiles[prone.GetY()][prone.GetX()].nLocks),int(layer->tiles[occupied.GetY()][occupied.GetX()].nLocks),int(verdict),network->IsPassable(prone));
    Require(verdict == NAI::AIP_YES,"the native anchor verdict incorrectly expanded the unit's neighbouring footprint");
    Require(!network->IsPassable(prone) && NAI::BigLockerPassableState(network,prone) == NAI::AIP_LOCKED &&
        !NAI::IsBigLockerPlacePassable(network,prone) && !IsValid(findPosePath()),
        "restoring anchor classification allowed an occupied prone/PK footprint or simple pose path");
    Require(network->GetPassability(occupied) == NAI::AIP_LOCKED,
        "an actually occupied anchor lost its native locked verdict");
    network->Unlock(first);
    Require(network->GetPassability(prone) == NAI::AIP_YES && network->IsLocked(occupied,true) && !network->IsPassable(prone),
        "releasing one overlapping owner cleared another owner's lock or changed anchor classification");
    network->Unlock(second);
    Require(!network->IsLocked(occupied,true) && network->GetPassability(prone) == NAI::AIP_YES &&
        network->IsPassable(prone) && NAI::BigLockerPassableState(network,prone) == NAI::AIP_YES && IsValid(findPosePath()),
        "releasing the final neighbour lock did not restore native full-footprint movement");
}

static void SimpleCrossLayerPoseEntry()
{
    auto network = SimplePathNetwork(2);
    auto find = [&](const NAI::SPathPlace& source,const NAI::SPathPlace& goal) {
        return CObj<NAI::CPath>(NAI::FindSimplePath(source,{goal},NAI::PF_USE_POSEDIR,network,false,false,false));
    };
    const NAI::SPathPlace standing(4,4,0,3,NAI::CM_STAND,0), prone(4,4,0,3,NAI::CM_LAY,0);
    Require(IsValid(find(standing,prone)), "unlocked prone footprint control rejected a direct pose path");
    vector<NAI::SPathPlace> footprint;
    network->GetLockArea(&footprint,prone,true);
    auto other = std::find_if(footprint.begin(),footprint.end(),[&](const NAI::SPathPlace& place) {
        return place.GetX() != prone.GetX() || place.GetY() != prone.GetY();
    });
    Require(other != footprint.end(), "prone footprint control has no neighbouring occupied cell");
    auto& blocked = network->GetLayer(other->GetLayer())->tiles[other->GetY()][other->GetX()];
    blocked.nLocks = 1;
    Require(network->GetPassability(prone) == NAI::AIP_YES && !network->IsPassable(prone) &&
        NAI::BigLockerPassableState(network,prone) == NAI::AIP_LOCKED && !IsValid(find(standing,prone)),
        "native anchor classification and explicit full-footprint pose checks no longer have distinct contracts");
    blocked.nLocks = 0;
    Require(IsValid(find(standing,prone)), "unlocking the prone footprint did not restore the simple pose path");
    for (int startPose: {NAI::CM_STAND,NAI::CM_LAY}) {
        const int endPose = startPose == NAI::CM_STAND ? NAI::CM_CROUCH : NAI::CM_STAND;
        const NAI::SPathPlace source(4,4,0,3,startPose,0), destination(4,4,1,3,endPose,0);
        auto sameLayer = destination;
        sameLayer.SetLayer(0);
        auto direct = find(source,sameLayer);
        Require(IsValid(direct) && direct->points.size() == 2,
            "same-layer pose control acquired an unnecessary intermediate point");
        auto path = find(source,destination);
        Require(IsValid(path) && path->points.front() == source && path->points.back() == destination,
            "cross-layer pose fixture did not find the requested native direct route");
        Require(path->points.size() == 3,
            "a simple cross-layer pose path omitted its layer-only entry point and combined the layer and pose change");
        auto entry = source;
        entry.SetLayer(destination.GetLayer());
        Require(path->points[1] == entry && path->actions.empty(),
            "a cross-layer entry changed source pose, heading, coordinates or unrelated path actions");
        for (std::size_t i = 1; i < path->points.size(); ++i)
            Require(NAI::GetTransitionType(network,path->points[i-1],path->points[i]) != NAI::TT_NO_WAY,
                "an emitted layer/pose path segment was not traversable by the native transition classifier");
    }
}

static void SimplePoseFacingPreference()
{
    auto network = SimplePathNetwork();
    auto find = [&](const NAI::SPathPlace& source,const vector<NAI::SPathPlace>& goals,
                    NAI::EFindPathParams params,bool pk = false) {
        return CObj<NAI::CPath>(NAI::FindSimplePath(source,goals,params,network,pk,pk,false));
    };
    const NAI::SPathPlace crouched(4,4,0,3,NAI::CM_CROUCH,0);
    const NAI::SPathPlace stand(4,4,0,3,NAI::CM_STAND,0), turn(4,4,0,4,NAI::CM_CROUCH,0);
    auto ranked = find(crouched,{stand,turn},NAI::PF_USE_POSEDIR);
    Require(IsValid(ranked) && ranked->points.back() == turn,
        "simple-path ranking no longer prefers an explicit turn over a pose change");
    auto pkPath = find(crouched,{stand,turn},NAI::PF_USE_POSEDIR,true);
    Require(IsValid(pkPath) && pkPath->points.back() == stand,
        "simple-path ranking bypassed the crouched Panzerklein turn restriction");
    const NAI::SPathPlace prone(4,4,0,3,NAI::CM_LAY,0);
    for (int pose: {NAI::CM_STAND,NAI::CM_CROUCH}) {
        const NAI::SPathPlace unspecified(4,4,0,0,pose,0);
        auto path = find(prone,{unspecified},NAI::PF_USE_POSE);
        Require(IsValid(path), "simple facing fixture did not find its direct pose path");
        Require(path->points.back().GetDirection() == prone.GetDirection(),
            "simple pose-path selection chose direction zero over an equally valid unchanged facing");
        auto explicitGoal = unspecified;
        explicitGoal.SetDirection(6);
        auto explicitPath = find(prone,{explicitGoal},NAI::PF_USE_POSEDIR);
        Require(IsValid(explicitPath) && explicitPath->points.back() == explicitGoal,
            "simple facing preference overrode the caller's explicit direction request");
    }
}

static void FlyingDynamicLocks()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    auto* layer = network->GetLayer(0);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            layer->tiles[y][x] = NAI::STile{};
            layer->tiles[y][x].nPassable = 7;
            layer->tiles[y][x].nFlags = NAI::TF_STAND_PASSABLE;
        }
    NAI::SPosition flying;
    Require(NAI::MakeFlyPos(CVec3(2,2,16), network, &flying), "flying-lock fixture could not encode its airborne position");
    Require(flying.p.IsFinal() && flying.p.IsIntegral() && flying.p.GetLayer() >= network->GetNumLayers(),
        "flying-lock fixture did not encode altitude in the layer bits");
    NAI::SPathPlace first(1,1,0), second(2,1,0);
    first.SetPose(NAI::CM_STAND); second.SetPose(NAI::CM_STAND);
    CObj<LockerActor> actor = new LockerActor;
    std::printf("Dynamic locks: airborne final=%d integral=%d altitude-bits=%d navigation-layers=%d\n",
        int(flying.p.IsFinal()), int(flying.p.IsIntegral()), flying.p.GetLayer(), network->GetNumLayers());
    std::fflush(stdout);
    network->ChangeDynamicLocks(actor, vector<NAI::SPathPlace>{first, flying.p, second, second});
    Require(layer->tiles[1][1].nDynLocks == 1 && layer->tiles[1][2].nDynLocks == 1,
        "skipping an airborne point changed the ground route's dynamic locks or duplicate filtering");
    network->ClearDynamicLocks(actor);
    Require(layer->tiles[1][1].nDynLocks == 0 && layer->tiles[1][2].nDynLocks == 0,
        "clearing a mixed airborne route retained ground dynamic locks");
    network->ChangeDynamicLocks(actor, vector<NAI::SPathPlace>{flying.p});
    network->ClearDynamicLocks(actor);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            Require(layer->tiles[y][x].nDynLocks == 0 && layer->tiles[y][x].nLocks == 0,
                "an airborne-only route locked a ground cell");
}

static void RemovedSelectedAIContact(int kind)
{
    CObj<ThreatOwnerFixture> owner = new ThreatOwnerFixture;
    CObj<NAI::AIUnitFixture> first = new NAI::AIUnitFixture, second = new NAI::AIUnitFixture;
    auto& state = owner->threat;
    state.SetUnit(owner);
    auto* selected = kind == 0 ? &state.pEnemy : kind == 1 ? &state.pPossibleEnemy : &state.pAlly;
    auto* collection = kind == 0 ? &state.enemies : kind == 1 ? &state.possibleEnemies : &state.allies;
    auto remove = [&](NAI::IAIUnit* unit) {
        if (kind == 0) state.RemoveEnemy(unit);
        else if (kind == 1) state.RemovePossibleEnemy(unit);
        else state.RemoveAlly(unit);
    };
    collection->data.units = {first.GetPtr(), second.GetPtr()}; *selected = first.GetPtr();
    state.selfModified.data = false;
    collection->bModified = false;
    remove(second);
    Require(selected->GetPtr() == first.GetPtr() && collection->data.units.size() == 1,
        "removing an unrelated AI contact cleared the selected contact");
    Require(collection->bModified && !state.selfModified.data,
        "removing an unrelated contact lost collection invalidation or reported a new threat");
    collection->bModified = false;
    remove(first);
    Require(!IsValid(*selected), "removing the selected AI contact left a stale target for subsequent reactions");
    Require(collection->data.units.empty() && collection->bModified && !state.selfModified.data,
        "removing a selected contact lost collection invalidation or synthesized a threat change");
    // Removal must also clear a selected reference absent from the member list,
    // as can happen while an event consumes a contact with updates deferred.
    *selected = second.GetPtr();
    remove(second);
    Require(!IsValid(*selected) && !state.selfModified.data,
        "removing an already-consumed AI contact retained its selected reference");
}

class ThreatActionProbe: public NAI::CAIAction
{
    OBJECT_NOCOPY_METHODS(ThreatActionProbe);
public:
    explicit ThreatActionProbe(NAI::IAIUnit* unit = nullptr): CAIAction(unit) {}
    using NAI::CAIAction::GetEnemy;
    void Do(NAI::CAILog*) const override {}
    bool ComparePlaces(const NAI::SPlaceWithAP&, const NAI::SPlaceWithAP&) const override { return false; }
    bool CanDo(const NAI::SPlaceWithAP&) override { return false; }
    void ResetInfoHash() override {}
};

class ThreatPlaceSourceProbe: public NAI::CAIActionPlaceSource
{
    OBJECT_NOCOPY_METHODS(ThreatPlaceSourceProbe);
public:
    explicit ThreatPlaceSourceProbe(NAI::IAIUnit* unit = nullptr): CAIActionPlaceSource(unit) {}
    using NAI::CAIActionPlaceSource::GetEnemy;
    void Prepare() override {}
};

static void OwnedAIEnemySelection(bool placeSource)
{
    CObj<ThreatOwnerFixture> owner = new ThreatOwnerFixture;
    CObj<NAI::AIUnitFixture> first = new NAI::AIUnitFixture, second = new NAI::AIUnitFixture;
    NAI::SAIState shared;
    owner->shared = &shared;
    owner->threat.pEnemy = first.GetPtr();
    shared.SetCurrentAIEnemy(second);
    CObj<ThreatActionProbe> action = new ThreatActionProbe(owner);
    CObj<ThreatPlaceSourceProbe> source = new ThreatPlaceSourceProbe(owner);
    auto selected = [&]() { return placeSource ? source->GetEnemy() : action->GetEnemy(); };
    Require(selected() == first.GetPtr(), "AI action selected another unit's shared target instead of its own contact");
    shared.SetCurrentAIEnemy(nullptr);
    Require(selected() == first.GetPtr(), "clearing the shared chooser context erased the acting unit's target");
    owner->threat.pEnemy = second.GetPtr();
    shared.SetCurrentAIEnemy(first);
    Require(selected() == second.GetPtr(), "AI action did not follow a change to its own threat target");
    owner->shared = nullptr;
    Require(selected() == second.GetPtr(), "own target lookup required a shared commander context");
    owner->threat.pEnemy = nullptr;
    Require(selected() == nullptr, "cleared own contact fell back to a shared target");
    owner->exposeThreat = false;
    Require(selected() == nullptr, "AI unit without a threat-state implementation was not handled safely");
    Require(owner->sharedReads == 0, "own-target lookup queried shared commander state");
    owner->Invalidate();
    Require(selected() == nullptr, "own-target lookup queried an invalid AI unit");
}

static void MountedAILogCommands()
{
    CObj<LockerActor> actor = new LockerActor;
    CObj<NAI::AIUnitFixture> unit = new NAI::AIUnitFixture;
    unit->server = actor.GetPtr();
    CObj<NWorld::CCannon> cannon = new NWorld::CCannon;
    CObj<NAI::CAILogUseCannon> enter = new NAI::CAILogUseCannon(unit, cannon);
    CObj<NAI::CAILogExitCannon> leave = new NAI::CAILogExitCannon(unit, cannon);
    list<CPtr<NWorld::CCommand>> commands;
    enter->GetCommands(&commands);
    Require(commands.size() == 2, "AI cannon entry lost its pose and docking commands during an incomplete-type cast");
    auto entry = commands.begin();
    auto* poseOrder = dynamic_cast<NWorld::CCmdSetCommand*>(entry++->GetPtr());
    auto* cannonOrder = dynamic_cast<NWorld::CCmdSetCommand*>(entry->GetPtr());
    Require(poseOrder && cannonOrder, "AI cannon entry emitted an unexpected world command");
    auto* pose = dynamic_cast<NWorld::CCmdWishPose*>(poseOrder->GetCmd());
    auto* dock = dynamic_cast<NWorld::CCmdCannon*>(cannonOrder->GetCmd());
    Require(pose && pose->pose == NAI::RUN && dock && dock->pObject.GetPtr() == static_cast<NWorld::IObject*>(cannon.GetPtr()),
        "AI cannon entry did not preserve its pose or exact cannon target");
    commands.clear();
    leave->GetCommands(&commands);
    Require(commands.size() == 1, "AI cannon exit lost its command during an incomplete-type cast");
    auto* exitOrder = dynamic_cast<NWorld::CCmdSetCommand*>(commands.front().GetPtr());
    auto* exit = exitOrder ? dynamic_cast<NWorld::CCmdExitCannon*>(exitOrder->GetCmd()) : nullptr;
    Require(exit && exit->pCannon.GetPtr() == static_cast<NWorld::IObject*>(cannon.GetPtr()),
        "AI cannon exit targeted another object");
    commands.clear();
    cannon->Invalidate();
    enter->GetCommands(&commands); leave->GetCommands(&commands);
    Require(commands.empty(), "invalid cannon generated an AI entry or exit command");
}

static void NavigationColouringOccupancy(int unavailable = 0)
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    auto* layer = network->GetLayer(0);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            auto& tile = layer->tiles[y][x];
            tile = NAI::STile{};
            tile.nMoveStand = static_cast<char>(0xff);
            tile.nPassable = NAI::CP_STAND;
            tile.nFlags = NAI::TF_STAND_PASSABLE;
        }
    CObj<NAI::CMapColourer> control = new NAI::CMapColourer;
    control->SetLayer(layer, nullptr, 0);
    control->ConstructColouring(network);
    Require(control->GetPointColor(3, 3) == control->GetPointColor(4, 3),
        "occupancy-colouring fixture has no connected open floor");
    CObj<LockerActor> actor = new LockerActor;
    NAI::SPathPlace occupied(3, 3, 0);
    occupied.SetPose(NAI::CM_STAND);
    network->Lock(actor, occupied);
    Require(network->IsLocked(occupied, true) && network->GetWhoLocksThisPlace(occupied) == actor.GetPtr(),
        "occupancy-colouring fixture did not register a live actor footprint");
    CObj<NAI::IAIJob> job = dynamic_cast<NAI::IAIJob*>(pSSClasses->CreateObject(0x00513131));
    Require(IsValid(job), "native recolouring job class is not registered");
    CObj<NAI::CLayersGroup> group = layer->pGroup;
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[group.GetPtr()] = 1; references.refs[1] = group.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); writer.Add(3, &group); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(job.GetPtr())->operator&(reader); }
    if (unavailable) {
        if (unavailable == 1) layer->pColourer = nullptr;
        else network->GetColourer(0)->Invalidate();
        Require(!IsValid(layer->pColourer), "unavailable colouring fixture retained a valid cache");
    }
    job->DoJob();
    auto* actual = network->GetColourer(0);
    Require(actual->IsReady(), "native recolouring job did not build its layer cache");
    Require(actual->GetPointColor(3, 3) == actual->GetPointColor(4, 3),
        "navigation rebuild cached a standing actor's temporary footprint as disconnected scenery");
    Require(network->IsLocked(occupied, true) && network->GetWhoLocksThisPlace(occupied) == actor.GetPtr()
            && layer->tiles[3][3].nLocks == 1,
        "navigation rebuild failed to restore the actor's original occupancy lock");
    network->Unlock(actor);
    Require(!network->IsLocked(occupied, true), "recolouring changed occupancy ownership or duplicated its lock");
}

class ReactionProbe: public NAI::CAIReaction
{
    OBJECT_NOCOPY_METHODS(ReactionProbe);
public:
    int updates = 0;
    ReactionProbe() {}
    explicit ReactionProbe(NAI::IAIUnit* unit): CAIReaction(unit) {}
    void Update() override { ++updates; }
};

static void StaleAIReaction()
{
    for (int staleReason = 0; staleReason < 4; ++staleReason) {
        CObj<LockerActor> actor = new LockerActor, liveActor = new LockerActor;
        CObj<NAI::AIUnitFixture> unit = new NAI::AIUnitFixture, liveUnit = new NAI::AIUnitFixture;
        unit->server = actor; liveUnit->server = liveActor;
        CObj<ReactionProbe> stale = new ReactionProbe(unit), live = new ReactionProbe(liveUnit);
        NAI::SAIUpdateTracker tracker;
        tracker.Add(stale); tracker.Add(live);
        // State changes after the reaction has already been enqueued.
        if (staleReason == 0) actor->active = false;
        if (staleReason == 1) unit->underAIControl = false;
        if (staleReason == 2) unit->server = nullptr;
        if (staleReason == 3) unit->Invalidate();
        tracker.Update();
        Require(stale->updates == 0, "queued AI reaction updated an actor that can no longer operate");
        Require(live->updates == 0 && !tracker.CanGetCommand(), "AI update drained more than one queued reaction");
        tracker.Update();
        Require(live->updates == 1 && tracker.CanGetCommand(), "live AI reaction did not resume on the next segment");
    }
}

static void OverlappingUnitLocks()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    NAI::SPathPlace shared(3, 3, 0), other(4, 3, 0);
    shared.SetPose(NAI::CM_STAND); other.SetPose(NAI::CM_STAND);
    auto* layer = network->GetLayer(0);
    layer->tiles[3][3].nPassable = layer->tiles[3][4].nPassable = NAI::CP_STAND;
    CObj<LockerActor> first = new LockerActor, second = new LockerActor;
    network->LockMovingObject(first, shared, other);
    network->Lock(second, shared);
    Require(network->IsLocked(shared, true) && network->IsLocked(other, true), "invalid lock fixture");
    network->Unlock(first);
    Require(network->IsLocked(shared, true), "unlocking one unit cleared another unit's overlapping footprint");
    Require(!network->IsLocked(other, true), "exclusive footprint remained locked after its owner left");
    network->Unlock(second);
    Require(!network->IsLocked(shared, true), "last footprint owner did not release the tile");

    // Selected-only queries deliberately disable everyone except the selection.
    network->Lock(first, shared);
    network->Lock(second, shared);
    network->LockSelected(std::list<CObjectBase*>{first.GetPtr()});
    network->Unlock(first);
    Require(!network->IsLocked(shared, true), "disabled unselected owner was incorrectly relocked");
    network->UnlockSelected();
    Require(network->IsLocked(shared, true), "selection query lost the surviving unit's lock");
    second->Invalidate();
    network->Lock(first, shared);
    network->Unlock(first);
    Require(!network->IsLocked(shared, true), "overlap restoration revived an invalid owner's footprint");
    network->Unlock(second);
}

static void CheckStaleUnitLocks(NWorld::CUnitServer* actor, int path, bool dead)
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    NAI::SPathPlace shared(3, 3, 0), other(4, 3, 0);
    shared.SetPose(NAI::CM_STAND); other.SetPose(NAI::CM_STAND);
    auto* layer = network->GetLayer(0);
    layer->tiles[3][3].nPassable = layer->tiles[3][4].nPassable = NAI::CP_STAND;
    network->Lock(actor, shared);
    Require(network->IsLocked(shared, true), "live actor did not lock its footprint");
    if (path == 2) network->LockSelected(std::list<CObjectBase*>{actor});
    if (dead) {
        actor->GetUnitRPG()->GetRPGUnit()->Kill();
        actor->SetState(new NWorld::CUnitStateDeath(actor));
        Require(actor->IsDead() && !actor->IsWearingPK(), "fixture actor is not an ordinary corpse");
    } else actor->Invalidate();
    Require(!NAI::IsLockerUnit(actor), "stale actor unexpectedly remains an eligible locker");
    if (path == 0) network->Lock(actor, shared);
    if (path == 1) network->LockMovingObject(actor, shared, other);
    if (path == 2) network->UnlockSelected();
    Require(!network->IsLocked(shared, true) && !network->IsLocked(other, true),
        "path query restored an ineligible unit's movement footprint");
    network->Unlock(actor);
}

static void InvalidUnitLocks()
{
    for (int path = 0; path < 3; ++path) {
        CObj<LockerActor> actor = new LockerActor;
        CheckStaleUnitLocks(actor, path, false);
    }
}

static void DeadUnitLocks(const char* databasePath, const char* resourcePath, int path)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    for (auto& candidate : units) {
        auto* actor = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (actor && actor->CanFight() && !actor->IsEmptyPK() && !actor->IsWearingPK()) {
            CheckStaleUnitLocks(actor, path, true);
            return;
        }
    }
    throw std::runtime_error("dead-lock mission fixture contains no ordinary live actor");
}

class ScriptActor: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(ScriptActor);
public:
    bool shell = false;
    int orders = 0;
    CPtr<NWorld::CPlayer> owner;
    bool IsEmptyPK() const override { return shell; }
    bool CanFight() const override { return !shell; }
    NWorld::IPlayer* GetPlayer() const override { return owner; }
    void Do(NWorld::CCommand* command) override { CObj<NWorld::CCommand> hold = command; ++orders; }
};

static void RunScript(NScript::CScript* script, const char* code)
{
    Require(script->DoString(code) == 0, "script compilation failed");
    pScript = script;
    script->ExecuteThreads();
    pScript = nullptr;
}

static void ScriptHideState(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) { actor = unit; break; }
    }
    Require(actor != nullptr, "script Hide state fixture has no human actor");
    auto* commander = actor->GetPlayer()->GetCommander();
    Require(commander && !commander->HasCommands() && !actor->HasCommand(),
        "script Hide state fixture started with native orders");
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    Require(!world->IsRealTime() && !world->IsSequence(), "script Hide AP fixture did not enter its native player turn");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
    RunScript(script,"UnitHide(nil); UpstreamHideReturned=7");
    auto returned = script->GetGlobal("UpstreamHideReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 7 && !commander->HasCommands(),
        "script Hide lost our nil-actor return-without-command behavior");
    vector<CObj<NWorld::CCmdHide>> requests;
    for (int option=0;option<3;++option)
        for (bool initial: {false,true}) {
            const bool requested = option != 1;
            actor->EnableHide(); actor->Hide(initial,false); actor->EnableHide();
            const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
            const auto position = actor->GetPosition();
            const string call = string("UnitHide(upstreamActor")+(option == 0 ? "" : option == 1 ? ",nil" : ",1")+
                "); UpstreamHideReturned=11";
            RunScript(script,call.c_str());
            returned = script->GetGlobal("UpstreamHideReturned");
            Require(returned.IsNumber() && returned.GetNumber() == 11 && commander->HasCommands(),
                "script Hide state call did not return with a queued request");
            CObj<NWorld::CCommand> first = commander->GetCommand();
            auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
            auto* hide = set ? dynamic_cast<NWorld::CCmdHide*>(set->GetCmd()) : nullptr;
            Require(hide && set->pUnit == actor && commander->HasCommands(),
                "script Hide state lost its actor, payload or Continue");
            CObj<NWorld::CCommand> second = commander->GetCommand();
            auto* next = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
            Require(next && next->pUnit == actor && dynamic_cast<NWorld::CCmdContinue*>(next->GetCmd()) &&
                !commander->HasCommands() && !actor->HasCommand(), "script Hide state changed native queue order or added commands");
            Require(actor->IsHiding() == initial && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap &&
                actor->GetPosition().pos.p == position.pos.p, "script Hide changed native state before consumption");
            NWorld::EUnitCommandResult result;
            CObj<NWorld::CCommandExecute> executor = NWorld::CreateExecutor(actor,hide,&result);
            Require(executor && result == NWorld::UCR_OK, "script Hide native executor rejected a valid request");
            const int cost = !initial && requested ? actor->GetActionAP(NRPG::AC_HIDE) : 0;
            executor->Run();
            std::printf("Lua Hide option=%d initial=%d requested=%d actual=%d explicit=%d\n",
                option,int(initial),int(requested),int(actor->IsHiding()),int(hide->bExplicitState));
            Require(actor->IsHiding() == requested, "Lua UnitHide toggled hiding instead of applying its requested/default state");
            Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap-cost,
                "Lua Hide changed the original native AP cost");
            requests.emplace_back(hide);
        }
    for (std::size_t i=0;i<requests.size();++i)
        Require(requests[i]->bExplicitState && requests[i]->bState == (i/2 != 1),
            "Lua Hide did not preserve its explicit/default request independently of live state");
    Require(!world->IsAction() && !actor->HasCommand(), "script Hide left an executor or action counter");
}

enum class DeferredScriptProducer { Animation, Direction, Pose, WishPose, ShootMode, Reload, ActivateWeapon, Hide };

static void DeferredUnitScriptCommand(const char* databasePath,const char* resourcePath,
    DeferredScriptProducer producer = DeferredScriptProducer::Animation,bool mixedOnly = false)
{
    const bool direction = producer == DeferredScriptProducer::Direction;
    const bool pose = producer == DeferredScriptProducer::Pose;
    const bool wishPose = producer == DeferredScriptProducer::WishPose;
    const bool shootMode = producer == DeferredScriptProducer::ShootMode;
    const bool reload = producer == DeferredScriptProducer::Reload;
    const bool activateWeapon = producer == DeferredScriptProducer::ActivateWeapon;
    const bool hide = producer == DeferredScriptProducer::Hide;
    const bool animationMode = producer == DeferredScriptProducer::Animation;
    auto world = FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto unit : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (!candidate || !candidate->CanFight()) continue;
        if (shootMode || reload || activateWeapon) {
            auto* weapon = candidate->GetUnitRPG()->GetWeaponItem();
            if (candidate->IsEmptyPK() || candidate->IsWearingPK() || candidate->animator.GetCannon() || !weapon) continue;
            if (shootMode && (!weapon->IsShootModeSupported(NDb::SM_Snap) || !weapon->IsShootModeSupported(NDb::SM_Aimed))) continue;
            if (reload) {
                auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(weapon);
                if (!firearm || firearm->GetAmmoQuantity() <= 0) continue;
                firearm->DiscardAmmo();
                if (!weapon->CanReload(candidate->GetUnitRPG()->GetInventory())) continue;
            }
        } else if (candidate->IsAIUnit()) continue;
        actor = candidate; break;
    }
    Require(actor != nullptr,"deferred animation fixture has no actual roster actor");
    if (hide) {
        Require(!actor->IsHiding(),"deferred hide fixture started with an already hidden actor");
        actor->EnableHide();
    }
    auto* commander = actor->GetPlayer()->GetCommander();
    Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
        "deferred animation fixture started with a pending native order");
    auto* animation = NDb::GetAnimation(4437);
    if (animationMode) Require(IsValid(animation),"deferred animation fixture has no authored script-animation record");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
    const auto position = actor->GetPosition(); const int ap = actor->GetAP();
    const auto originalWishPose = actor->GetWishPose();
    const bool originalActiveItem = actor->animator.IsActiveItem();
    const bool originalHiding = actor->IsHiding();
    auto* weapon = (shootMode || reload || activateWeapon) ? actor->GetUnitRPG()->GetWeaponItem() : nullptr;
    auto* clip = weapon ? weapon->GetInnerClip() : nullptr;
    const int ammo = weapon ? weapon->GetAmmoQuantity() : 0;
    vector<std::pair<CPtr<NRPG::IInventoryItem>,int>> backpack;
    if (reload) for (const auto& entry : actor->GetUnitRPG()->GetInventory()->GetItems()) {
        auto* item = dynamic_cast<NRPG::CClipItem*>(entry.pItem.GetPtr());
        backpack.emplace_back(entry.pItem,item ? item->GetIncQuantity() : -1);
    }
    if (shootMode) Require(weapon && weapon->SetShootMode(NDb::SM_Snap),"deferred fire-mode fixture cannot select its native weapon's snap mode");
    const vector<bool> options = mixedOnly ? vector<bool>{} : (reload || hide) ? vector<bool>{false} : vector<bool>{false,true};
    for (bool freeze : options) {
        const int requestedDirection = (position.pos.p.GetDirection()+(freeze ? 7 : 1))&7;
        const auto requestedPose = freeze ? NAI::WALK : NAI::CRAWL;
        const auto requestedMode = freeze ? NDb::SM_Snap : NDb::SM_Aimed;
        string call;
        if (direction) call = "UnitSetDirection(upstreamActor,"+std::to_string(requestedDirection)+")";
        else if (pose) call = "UnitSetPose(upstreamActor,"+std::to_string(int(requestedPose))+")";
        else if (wishPose) call = "UnitSetWishPose(upstreamActor,"+std::to_string(int(requestedPose))+")";
        else if (shootMode) call = "UnitSetShootMode(upstreamActor,"+std::to_string(int(requestedMode))+")";
        else if (reload) call = "UnitReload(upstreamActor)";
        else if (activateWeapon) call = "UnitActivateWeapon(upstreamActor,"+string(freeze ? "1" : "nil")+")";
        else if (hide) call = "UnitHide(upstreamActor)";
        else call = "UnitPlayAnimation(upstreamActor,4437,nil,"+string(freeze ? "1" : "nil")+")";
        const string code = call+"; UpstreamAnimationReturned = 19";
        RunScript(script,code.c_str());
        auto returned = script->GetGlobal("UpstreamAnimationReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 19,"native animation script did not finish its call");
        if (direction)
            std::printf("native script direction dispatch: requested=%d commander-pending=%d unit-command=%d\n",
                requestedDirection,int(commander->HasCommands()),int(actor->HasCommand()));
        else if (pose)
            std::printf("native script pose dispatch: requested=%d commander-pending=%d unit-command=%d\n",
                int(requestedPose),int(commander->HasCommands()),int(actor->HasCommand()));
        else if (wishPose)
            std::printf("native script preferred-pose dispatch: requested=%d commander-pending=%d unit-command=%d before=%d after=%d\n",
                int(requestedPose),int(commander->HasCommands()),int(actor->HasCommand()),int(originalWishPose),int(actor->GetWishPose()));
        else if (shootMode)
            std::printf("native script fire-mode dispatch: requested=%d commander-pending=%d unit-command=%d live-mode=%d\n",
                int(requestedMode),int(commander->HasCommands()),int(actor->HasCommand()),int(weapon->GetShootMode()));
        else if (reload)
            std::printf("native script reload dispatch: commander-pending=%d unit-command=%d ammo-before=%d ammo-after=%d\n",
                int(commander->HasCommands()),int(actor->HasCommand()),ammo,weapon->GetAmmoQuantity());
        else if (activateWeapon)
            std::printf("native script weapon activation: requested=%d commander-pending=%d unit-command=%d walk-without-weapon=%d\n",
                int(freeze),int(commander->HasCommands()),int(actor->HasCommand()),int(actor->IsWalkingWithoutWeapon()));
        else if (hide)
            std::printf("native script hide dispatch: commander-pending=%d unit-command=%d before=%d after=%d\n",
                int(commander->HasCommands()),int(actor->HasCommand()),int(originalHiding),int(actor->IsHiding()));
        else
            std::printf("native script animation dispatch: freeze=%d commander-pending=%d unit-command=%d\n",
                int(freeze),int(commander->HasCommands()),int(actor->HasCommand()));
        Require(commander->HasCommands() && !actor->HasCommand(),
            direction ? "native script direction executes immediately instead of entering the owning commander's queue" :
            pose ? "native script pose bypasses the owning commander's queue" :
            wishPose ? "native script preferred pose bypasses the owning commander's queue" :
            shootMode ? "native script fire mode bypasses the owning commander's queue" :
            reload ? "native script reload bypasses the owning commander's queue" :
            activateWeapon ? "native script weapon activation bypasses the owning commander's queue" :
            hide ? "native script hide bypasses the owning commander's queue" :
            "native script animation executes immediately instead of entering the owning commander's queue");
        CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
        auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
        auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
        Require(set && set->pUnit == actor,"queued native script command lost its target actor");
        if (direction) {
            auto* look = dynamic_cast<NWorld::CCmdLook*>(set->GetCmd());
            auto expected = position.pos; expected.p.SetDirection(requestedDirection);
            Require(look && look->ptDst.p == expected.p && look->ptDst.pNet == expected.pNet,
                "queued native script turn lost its LOOK type, direction, tile, pose or network");
        } else if (pose) {
            auto* path = dynamic_cast<NWorld::CCmdPath*>(set->GetCmd());
            auto expected = position; expected.SetPose(requestedPose);
            Require(path && path->ptDst.p == expected.pos.p && path->ptDst.pNet == expected.pos.pNet &&
                path->eParams == NAI::PF_USE_POSEDIR,
                "queued native script pose lost its PATH type, pose/direction option, tile, heading or network");
        } else if (wishPose) {
            auto* preference = dynamic_cast<NWorld::CCmdWishPose*>(set->GetCmd());
            Require(preference && preference->pose == requestedPose,
                "queued native script preferred pose lost its WISHPOSE type or requested value");
        } else if (shootMode) {
            auto* mode = dynamic_cast<NWorld::CCmdShootMode*>(set->GetCmd());
            Require(mode && mode->eMode == requestedMode,"queued native script fire mode lost its type or requested value");
        } else if (reload) {
            auto* request = dynamic_cast<NWorld::CCmdReload*>(set->GetCmd());
            Require(request && request->nSlot == -1,"queued native script reload lost its type or active-weapon slot");
        } else if (activateWeapon) {
            auto* path = dynamic_cast<NWorld::CCmdPath*>(set->GetCmd());
            Require(path && path->ptDst.p == position.pos.p && path->ptDst.pNet == position.pos.pNet &&
                path->eParams == NAI::PF_DEFAULT && !path->bLeaveZone && !path->bCanFindNotExact &&
                path->needActiveItem == (freeze ? NWorld::ITEM_ACTIVE : NWorld::ITEM_INACTIVE),
                "queued native weapon activation lost its PATH destination, flags or requested hand state");
        } else if (hide) {
            Require(dynamic_cast<NWorld::CCmdHide*>(set->GetCmd()) != nullptr,
                "queued native script hide lost its original command type");
        } else {
            auto* play = dynamic_cast<NWorld::CCmdPlayAnimation*>(set->GetCmd());
            Require(play && play->nDBAnimationID == 4437 && play->bFreezeAfterLastFrame == freeze,
                "queued native animation lost its clip or freeze parameter");
        }
        Require(resume && resume->pUnit == actor && dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) &&
            !commander->HasCommands(),"native animation did not queue exactly one ordered Continue");
        Require(!actor->HasCommand() && actor->GetAP() == ap &&
            actor->GetPosition().pos.p == position.pos.p,"queueing a script animation executed an actor action");
        if (wishPose) Require(actor->GetWishPose() == originalWishPose,
            "native preferred pose changed before its queued command was consumed");
        if (shootMode) Require(actor->GetUnitRPG()->GetWeaponItem() == weapon && weapon->GetShootMode() == NDb::SM_Snap,
            "native weapon mode changed before its queued script command was consumed");
        if (activateWeapon) Require(actor->IsWalkingWithoutWeapon() == !freeze &&
            actor->animator.IsActiveItem() == originalActiveItem &&
            actor->GetUnitRPG()->GetWeaponItem() == weapon && weapon->GetAmmoQuantity() == ammo,
            "queued weapon activation lost the immediate persistent holster preference or changed the live weapon");
        if (hide) Require(actor->IsHiding() == originalHiding,"native script changed hiding state before queued command consumption");
        if (reload) {
            const auto& items = actor->GetUnitRPG()->GetInventory()->GetItems();
            Require(actor->GetUnitRPG()->GetWeaponItem() == weapon && weapon->GetInnerClip() == clip &&
                weapon->GetAmmoQuantity() == ammo && items.size() == backpack.size(),
                "native script reload changed the weapon, inner clip or backpack before command consumption");
            for (std::size_t i=0; i<items.size(); ++i) {
                auto* item = dynamic_cast<NRPG::CClipItem*>(items[i].pItem.GetPtr());
                Require(items[i].pItem == backpack[i].first && (item ? item->GetIncQuantity() : -1) == backpack[i].second,
                    "native script reload consumed or replaced backpack ammunition before queued command consumption");
            }
        }
    }
    if (animationMode) {
        RunScript(script,"UnitPlayAnimation(upstreamActor,4437,1); UpstreamAnimationReturned = 23");
        auto returned = script->GetGlobal("UpstreamAnimationReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 23 && !commander->HasCommands() && !actor->HasCommand(),
            "native custom-idle animation was incorrectly converted into a queued action");
    }
    if (mixedOnly) {
        Require(shootMode && IsValid(animation),"mixed script fixture has no native weapon or authored clip");
        RunScript(script,"UnitSetDirection(upstreamActor,1); UnitSetShootMode(upstreamActor,1); "
            "UnitSetWishPose(upstreamActor,0); UnitPlayAnimation(upstreamActor,4437,nil,nil); UpstreamMixedReturned = 31");
        auto returned = script->GetGlobal("UpstreamMixedReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 31,"mixed native script did not finish its four calls");
        vector<CObj<NWorld::CCommand>> pending;
        while (commander->HasCommands()) pending.push_back(commander->GetCommand());
        std::printf("native mixed script order: commands=%zu expected=8 live-mode=%d live-wish=%d\n",
            pending.size(),int(weapon->GetShootMode()),int(actor->GetWishPose()));
        Require(pending.size() == 8 && weapon->GetShootMode() == NDb::SM_Snap &&
            actor->GetWishPose() == originalWishPose && !actor->HasCommand(),
            "native mixed script loses a queued request or changes live state ahead of its command order");
        for (int i=0; i<8; ++i) {
            auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(pending[i].GetPtr());
            Require(set && set->pUnit == actor,"mixed native queue lost its command wrapper or target actor");
            auto* payload = set->GetCmd();
            bool matches = false;
            if (i%2) matches = dynamic_cast<NWorld::CCmdContinue*>(payload) != nullptr;
            else if (i == 0) { auto* look = dynamic_cast<NWorld::CCmdLook*>(payload); matches = look && look->ptDst.p.GetDirection() == 1; }
            else if (i == 2) { auto* mode = dynamic_cast<NWorld::CCmdShootMode*>(payload); matches = mode && mode->eMode == NDb::SM_Aimed; }
            else if (i == 4) { auto* wish = dynamic_cast<NWorld::CCmdWishPose*>(payload); matches = wish && wish->pose == NAI::CRAWL; }
            else if (i == 6) { auto* play = dynamic_cast<NWorld::CCmdPlayAnimation*>(payload); matches = play && play->nDBAnimationID == 4437 && !play->bFreezeAfterLastFrame; }
            Require(matches,"native mixed script commands arrived in a different order or lost their parameters");
        }
        Require(actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p,
            "queueing the mixed native script moved its actor or spent action points");
    }
}

static void DeferredScriptShot(const char* databasePath,const char* resourcePath,bool prepare = false)
{
    auto world = FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NWorld::CUnitServer* target = nullptr;
    for (auto entry : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!candidate || !candidate->CanFight() || candidate->IsEmptyPK() || candidate->IsWearingPK() ||
            candidate->animator.GetCannon() || !dynamic_cast<NRPG::CWeaponItem*>(candidate->GetUnitRPG()->GetWeaponItem())) continue;
        for (auto other : units) {
            auto* enemy = dynamic_cast<NWorld::CUnitServer*>(other.GetPtr());
            if (enemy && enemy != candidate && enemy->CanFight() && !enemy->IsEmptyPK() && !enemy->IsWearingPK() &&
                world->GetDiplomacyState(candidate->GetPlayer(),enemy->GetPlayer()) == NDb::DS_ENEMY) {
                actor = candidate; target = enemy; break;
            }
        }
        if (actor) break;
    }
    Require(actor && target,"deferred shot fixture has no real armed actor and distinct living enemy");
    auto* weapon = actor->GetUnitRPG()->GetWeaponItem();
    const int ammo = weapon->GetAmmoQuantity(), ap = actor->GetAP();
    const auto position = actor->GetPosition();
    NRPG::SUnitInfo before; target->GetInfo(&before);
    auto* commander = actor->GetPlayer()->GetCommander();
    Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
        "deferred shot fixture started with an existing native order");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
    NScript::luaPushCPtr(*script,target); script->SetGlobal("upstreamTarget");
    const string function = prepare ? "UnitShootPrepare" : "UnitShoot";
    const string nilControl = function+"(upstreamActor,nil,0); "+function+"(nil,upstreamTarget,0); UpstreamShotReturned = 13";
    RunScript(script,nilControl.c_str());
    auto returned = script->GetGlobal("UpstreamShotReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 13 && !commander->HasCommands() && !actor->HasCommand(),
        "deferred shot lost the fork's nil hardening or issued an order for a null actor/target");
    for (auto location : {NAI::HL_BODY,NAI::HL_HEAD}) {
        const string call = function+"(upstreamActor,upstreamTarget,"+std::to_string(int(location))+"); UpstreamShotReturned = 17";
        RunScript(script,call.c_str());
        returned = script->GetGlobal("UpstreamShotReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 17,"deferred shot Lua callback did not finish");
        std::printf("native script shot dispatch: prepare=%d location=%d commander-pending=%d unit-command=%d\n",
            int(prepare),int(location),int(commander->HasCommands()),int(actor->HasCommand()));
        Require(commander->HasCommands() && !actor->HasCommand(),
            "native script shot bypasses the owning commander's queue");
        CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
        auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
        auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
        auto* shot = set ? dynamic_cast<NWorld::CCmdShootObject*>(set->GetCmd()) : nullptr;
        Require(set && set->pUnit == actor && shot && shot->pTarget == target && shot->eHL == location &&
            shot->nExtraAttackAP == 0 && shot->bOnlyPrepareToShoot == prepare && !shot->bCanBeReplacedByReload &&
            resume && resume->pUnit == actor && dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) &&
            !commander->HasCommands(),"queued native shot lost its target, hit location, flags, actor or ordered Continue");
        NRPG::SUnitInfo after; target->GetInfo(&after);
        Require(!actor->HasCommand() && actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p &&
            actor->GetUnitRPG()->GetWeaponItem() == weapon && weapon->GetAmmoQuantity() == ammo &&
            target->CanFight() && after.nHP == before.nHP,
            "native script shot changed actor/AP/ammunition or damaged the target before queued command consumption");
    }
}

static void DeferredWaypointCommand(const char* databasePath,const char* resourcePath,bool fly = true)
{
    auto world = FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr; NRPG::CWeaponItem* weapon = nullptr;
    for (auto entry : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!candidate || !candidate->CanFight() || candidate->IsEmptyPK() || candidate->IsWearingPK() ||
            candidate->animator.GetCannon() || candidate->GetPosition().pos.GetFloor() != 0) continue;
        auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
        if (firearm) { actor = candidate; weapon = firearm; break; }
    }
    Require(actor && weapon,"deferred waypoint fixture has no ordinary armed actor on the ground floor");
    const auto position = actor->GetPosition(); const int ap = actor->GetAP(), ammo = weapon->GetAmmoQuantity();
    auto* commander = actor->GetPlayer()->GetCommander();
    Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
        "deferred waypoint fixture started with an existing native order");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
    const string function = fly ? "UnitFlyToWaypoint" : "UnitAttackWaypoint";
    for (bool threeD : {false,true}) {
        CObj<NDb::CWaypointName> name = new NDb::CWaypointName;
        name->szName = threeD ? "upstream_dispatch_probe_fly" : "upstream_dispatch_probe_ground";
        CObj<CMapWaypoint> authored = new CMapWaypoint;
        authored->pName = name; authored->b3DWaypoint = threeD;
        authored->pos.ptPos = position.GetCP()+CVec3(2,0,threeD ? 2.f : 0.f);
        world->AddWaypoint(authored);
        auto* waypoint = world->GetWaypoint(name->szName);
        Require(waypoint && waypoint->pos.p.IsFinal() == threeD,
            "deferred waypoint fixture did not load its actual ground/fly point through the world's registry");
        const auto target = waypoint->pos;
        const string controls = function+"(nil,'"+name->szName+"'); "+function+
            "(upstreamActor,'upstream_dispatch_probe_missing'); UpstreamWaypointReturned = 23";
        RunScript(script,controls.c_str());
        auto returned = script->GetGlobal("UpstreamWaypointReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 23 && !commander->HasCommands() && !actor->HasCommand(),
            "native waypoint nil-actor/missing-point controls issued an order or stopped the script");
        const string call = function+"(upstreamActor,'"+name->szName+"'); UpstreamWaypointReturned = 29";
        RunScript(script,call.c_str());
        returned = script->GetGlobal("UpstreamWaypointReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 29,"native waypoint Lua callback did not finish");
        std::printf("native script waypoint dispatch: fly=%d three-dimensional=%d queued=%d unit-command=%d\n",
            int(fly),int(threeD),int(commander->HasCommands()),int(actor->HasCommand()));
        Require(commander->HasCommands() && !actor->HasCommand(),
            "native script waypoint command bypasses the owning commander's queue");
        CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
        auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
        auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
        Require(set && set->pUnit == actor && resume && resume->pUnit == actor &&
            dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) && !commander->HasCommands(),
            "queued waypoint command lost its actor or exact ordered Continue");
        if (fly) {
            auto* request = dynamic_cast<NWorld::CCmdFly*>(set->GetCmd());
            Require(request && request->dst.pos.p.IsFinal() && !request->dst.bRun,
                "queued fly request lost its command type, fly-place encoding or walk/run flag");
            const auto actual = request->dst.GetCP(), expected = target.GetCP();
            Require(std::fabs(actual.x-expected.x)<0.001f && std::fabs(actual.y-expected.y)<0.001f &&
                std::fabs(actual.z-expected.z)<=NAI::F_3D_STEP+0.001f,
                "queued fly destination differs from its native waypoint beyond fly-height quantization");
            if (threeD) Require(request->dst.pos.p == target.p && request->dst.pos.pNet == target.pNet,
                "queued fly request reinterpreted an already loaded 3D waypoint's position, altitude or heading bits");
        } else {
            auto* request = dynamic_cast<NWorld::CCmdShootTile*>(set->GetCmd());
            Require(request && fabs2(request->ptTarget-target.GetCP())<0.000001f && !request->bCanBeReplacedByReload,
                "queued waypoint attack lost its native tile destination, type or reload replacement flag");
        }
        Require(!actor->HasCommand() && actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p &&
            actor->GetUnitRPG()->GetInventory()->GetActive() == weapon && weapon->GetAmmoQuantity() == ammo &&
            waypoint->pos.p == target.p && waypoint->pos.pNet == target.pNet,
            "native waypoint producer moved the actor, changed AP/ammo or rewrote its loaded point before consumption");
    }
}

static void DeferredCorpseCommand(const char* databasePath,const char* resourcePath,bool drop = false)
{
    auto world = FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> ordinary;
    for (auto entry : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (candidate && candidate->CanFight() && !candidate->IsEmptyPK() && !candidate->IsWearingPK() &&
            !candidate->animator.GetCannon()) ordinary.push_back(candidate);
        if (ordinary.size() == 2) break;
    }
    Require(ordinary.size() == 2,"deferred corpse fixture has no two living ordinary actors");
    auto* actor = ordinary[0]; auto* body = ordinary[1];
    auto* commander = actor->GetPlayer()->GetCommander();
    Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
        "deferred corpse fixture started with an existing order");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
    NScript::luaPushCPtr(*script,body); script->SetGlobal("upstreamBody");
    RunScript(script,"UnitTakeCorpse(upstreamActor,upstreamBody); UpstreamCorpseReturned = 7");
    auto returned = script->GetGlobal("UpstreamCorpseReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 7 && !commander->HasCommands() && !actor->HasCommand() && body->CanFight(),
        "native script corpse pickup accepted a living target or stopped the callback");
    body->GetUnitRPG()->GetRPGUnit()->Kill();
    body->SetState(new NWorld::CUnitStateDeath(body));
    Require(actor->CanFight() && !actor->IsDead() && body->IsDead() && !body->CanFight() && !body->GetCorpseCarrier(),
        "deferred corpse fixture did not prepare an actual uncarried dead actor");
    CObj<NWorld::CExecCorpse> take;
    if (drop) {
        take = new NWorld::CExecCorpse(actor,body,true);
        take->Run();
        take->TimeLabelReached();
        Require(actor->GetCorpse() == body && actor->IsCarryingCorpse() && body->GetCorpseCarrier() == actor,
            "deferred drop fixture did not enter the native corpse-carry state and body attachment");
    }
    Require(!commander->HasCommands() && !actor->HasCommand(),"corpse fixture preparation issued an actor order");
    const auto position = actor->GetPosition(), bodyPosition = body->GetPosition(); const int ap = actor->GetAP();
    auto* previousCorpse = actor->GetCorpse(); auto* previousCarrier = body->GetCorpseCarrier();
    const bool carrying = actor->IsCarryingCorpse();
    const string controls = drop ? "UnitDropCorpse(nil); UpstreamCorpseReturned = 11" :
        "UnitTakeCorpse(nil,upstreamBody); UnitTakeCorpse(upstreamActor,nil); UpstreamCorpseReturned = 11";
    RunScript(script,controls.c_str());
    returned = script->GetGlobal("UpstreamCorpseReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 11 && !commander->HasCommands() && !actor->HasCommand(),
        "native corpse nil controls issued an order or stopped the callback");
    RunScript(script,drop ? "UnitDropCorpse(upstreamActor); UpstreamCorpseReturned = 13" :
        "UnitTakeCorpse(upstreamActor,upstreamBody); UpstreamCorpseReturned = 13");
    returned = script->GetGlobal("UpstreamCorpseReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 13,"native corpse Lua callback did not finish");
    std::printf("native script corpse dispatch: drop=%d queued=%d unit-command=%d carrying=%d\n",
        int(drop),int(commander->HasCommands()),int(actor->HasCommand()),int(actor->IsCarryingCorpse()));
    Require(commander->HasCommands() && !actor->HasCommand(),
        "native script corpse action bypasses the owning commander's queue");
    CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
    auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
    auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
    Require(set && set->pUnit == actor && resume && resume->pUnit == actor &&
        dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) && !commander->HasCommands(),
        "queued corpse command lost its actor or exact ordered Continue");
    if (drop) {
        auto* request = dynamic_cast<NWorld::CCmdDropCorpse*>(set->GetCmd());
        Require(request && !IsValid(request->pCorpse),
            "queued corpse drop lost its type or resolved the carrier-state body before consumption");
    } else {
        auto* request = dynamic_cast<NWorld::CCmdTakeCorpse*>(set->GetCmd());
        Require(request && request->pCorpse == body,"queued corpse pickup lost its actual target body");
    }
    Require(!actor->HasCommand() && actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p &&
        body->GetPosition().pos.p == bodyPosition.pos.p && actor->GetCorpse() == previousCorpse &&
        body->GetCorpseCarrier() == previousCarrier && actor->IsCarryingCorpse() == carrying && body->IsDead(),
        "native corpse producer changed position/AP/carry state or body attachment before queued consumption");
}

static void DeferredPKCommand(const char* databasePath,const char* resourcePath,bool leave = true,bool queryRoute = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* pilot = nullptr;
    for (auto entry : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (candidate && !candidate->IsAIUnit() && candidate->CanFight() && !candidate->IsEmptyPK() &&
            !candidate->IsWearingPK() && !candidate->animator.GetCannon()) { pilot = candidate; break; }
    }
    Require(pilot != nullptr,"deferred PK fixture has no ordinary living human pilot");
    NDb::CRPGPers* suitRecord = nullptr;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while (people.MoveNext()) if (IsValid(people.Get()->pPanzerklein) &&
        !people.Get()->pPanzerklein->bHasNoHead && IsValid(people.Get()->pModel)) { suitRecord = people.Get(); break; }
    Require(suitRecord != nullptr,"deferred PK fixture has no regular authored suit record");
    auto* network = world->GetPathNetwork();
    vector<NAI::SPathPlace> places;
    network->GetNearPlaces(SSphere(pilot->GetPosition().GetCP(),20.f),&places);
    NAI::SPathPlace anchor; bool found = false;
    for (auto place : places) {
        place.SetPose(NAI::CM_STAND); place.SetDirection(0);
        if (NAI::IsBigLockerPlacePassable(network,place)) { anchor = place; found = true; break; }
    }
    Require(found,"deferred PK fixture has no free native suit footprint");
    CObj<NWorld::CUnitServer> shell = world->AddUnitInGame(anchor,NRPG::CreateUnit(suitRecord),nullptr);
    Require(shell && shell->IsEmptyPK() && !shell->CanFight(),"deferred PK fixture did not create an actual nonfighting empty shell");
    auto* commander = pilot->GetPlayer()->GetCommander();
    Require(IsValid(commander) && !commander->HasCommands() && !pilot->HasCommand(),
        "deferred PK fixture started with an existing pilot order");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,pilot); script->SetGlobal("upstreamPilot");
    NScript::luaPushCPtr(*script,shell.GetPtr()); script->SetGlobal("upstreamShell");
    if (queryRoute) {
        Require(!IsValid(shell->GetPlayer()), "empty PK route fixture unexpectedly has a player owner");
        RunScript(script,"UpstreamNilRoute=UnitGetRoute(nil); UpstreamHumanRoute=UnitGetRoute(upstreamPilot); UpstreamRouteReturned=7");
        auto returned = script->GetGlobal("UpstreamRouteReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 7 &&
            script->GetGlobal("UpstreamNilRoute").IsNil() && script->GetGlobal("UpstreamHumanRoute").IsNil(),
            "route lookup lost nil/non-AI controls");
        NWorld::CUnitServer* actor = nullptr; NAI::IAIUnit* ai = nullptr;
        for (auto& entry: units) {
            auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
            if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) {
                auto* candidate = NAI::GetAIUnit(unit);
                if (candidate) { actor = unit; ai = candidate; break; }
            }
        }
        Require(actor && ai, "route lookup fixture has no real AI-controlled actor");
        CObj<NAI::CAIRouteLogic> route = new NAI::CAIRouteLogic(ai);
        route->AddCommand(new NAI::CTaskCommandWait(100));
        ai->SetRouteLogic(route);
        Require(ai->GetRouteLogic() == route.GetPtr(), "route lookup fixture did not install its real route logic");
        NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamRouteActor");
        RunScript(script,"UpstreamRealRoute=UnitGetRoute(upstreamRouteActor); UpstreamRouteFinished=RouteIsFinished(UpstreamRealRoute)");
        auto actual = script->GetGlobal("UpstreamRealRoute");
        Require(actual.IsUserData() && NScript::luaGetPtr(actual) == route.GetPtr() &&
            script->GetGlobal("UpstreamRouteFinished").IsNil(),
            "route lookup lost the actual route identity or reported its unfinished wait complete");
        route->Finish();
        RunScript(script,"UpstreamRouteFinished=RouteIsFinished(UpstreamRealRoute)");
        actual = script->GetGlobal("UpstreamRouteFinished");
        Require(actual.IsNumber() && actual.GetNumber() == 1, "route lookup stopped observing our own completion latch");
        std::puts("native UnitGetRoute: nil/non-AI and real route identity/completion controls passed; querying ownerless empty PK");
        std::fflush(stdout);
        RunScript(script,"UpstreamShellRoute=UnitGetRoute(upstreamShell); UpstreamRouteReturned=11");
        returned = script->GetGlobal("UpstreamRouteReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 11 && script->GetGlobal("UpstreamShellRoute").IsNil() &&
            !commander->HasCommands() && !pilot->HasCommand(), "empty PK route lookup did not return nil without a command");
        return;
    }
    if (leave) {
        RunScript(script,"UpstreamNilShell = UnitLeavePK(nil); UpstreamMissingShell = UnitLeavePK(upstreamPilot); UpstreamPKReturned = 11");
        Require(script->GetGlobal("UpstreamNilShell").IsNil() && script->GetGlobal("UpstreamMissingShell").IsNil(),
            "native PK exit returned a shell for a nil or unarmoured pilot");
        Require(shell->WearAsPK(true),"deferred PK exit fixture could not hide its shell for entry");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell,false,false);
        Require(pilot->IsWearingPK() && pilot->GetWearingPK() == shell.GetPtr(),
            "deferred PK exit fixture did not attach the real suit to its pilot");
    } else RunScript(script,"UnitWearPK(nil,upstreamShell); UnitWearPK(upstreamPilot,nil); "
        "UnitWearPK(upstreamPilot,upstreamPilot); UpstreamPKReturned = 11");
    auto returned = script->GetGlobal("UpstreamPKReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 11 && !commander->HasCommands() && !pilot->HasCommand(),
        "native PK controls or fixture preparation issued an order or stopped the script");
    auto inventoryItems = [](NRPG::IInventory* inventory) {
        vector<CObj<NRPG::IInventoryItem>> items;
        for (auto slot : {NDb::SLOT_1,NDb::SLOT_2}) items.push_back(inventory->Get(slot));
        for (const auto& entry : inventory->GetItems()) items.push_back(entry.pItem.GetPtr());
        return items;
    };
    auto* pilotInventory = pilot->GetUnitRPG()->GetInventory();
    auto* shellInventory = shell->GetUnitRPG()->GetInventory();
    const auto pilotItems = inventoryItems(pilotInventory), shellItems = inventoryItems(shellInventory);
    const auto pilotSlot = pilotInventory->GetActiveSlot(), shellSlot = shellInventory->GetActiveSlot();
    const auto position = pilot->GetPosition(), shellPosition = shell->GetPosition(); const int ap = pilot->GetAP();
    auto* worn = pilot->GetWearingPK(); const bool empty = shell->IsEmptyPK();
    RunScript(script,leave ? "UpstreamExitedShell = UnitLeavePK(upstreamPilot); UpstreamPKReturned = 17" :
        "UnitWearPK(upstreamPilot,upstreamShell); UpstreamPKReturned = 17");
    returned = script->GetGlobal("UpstreamPKReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 17,"native PK Lua callback did not finish");
    if (leave) {
        auto result = script->GetGlobal("UpstreamExitedShell");
        Require(result.IsUserData() && NScript::luaGetPtr(result) == shell.GetPtr(),
            "native PK exit did not return its original shell through the fork's Lua pointer adapter");
    }
    std::printf("native script PK dispatch: leave=%d queued=%d unit-command=%d wearing=%d shell-empty=%d\n",
        int(leave),int(commander->HasCommands()),int(pilot->HasCommand()),int(pilot->IsWearingPK()),int(shell->IsEmptyPK()));
    Require(commander->HasCommands() && !pilot->HasCommand(),
        "native scripted PK transition bypasses the owning commander's queue");
    CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
    auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
    auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
    Require(set && set->pUnit == pilot && resume && resume->pUnit == pilot &&
        dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) && !commander->HasCommands(),
        "queued PK transition lost its pilot or exact ordered Continue");
    if (leave) Require(dynamic_cast<NWorld::CCmdExitPK*>(set->GetCmd()) != nullptr,"queued PK exit lost its original command type");
    else {
        auto* request = dynamic_cast<NWorld::CCmdTakeCorpse*>(set->GetCmd());
        Require(request && request->pCorpse == shell.GetPtr(),"queued PK entry lost its original empty-shell target");
    }
    Require(!pilot->HasCommand() && pilot->GetAP() == ap && pilot->GetPosition().pos.p == position.pos.p &&
        shell->GetPosition().pos.p == shellPosition.pos.p && pilot->GetWearingPK() == worn &&
        pilot->IsWearingPK() == leave && shell->IsEmptyPK() == empty &&
        inventoryItems(pilotInventory) == pilotItems && inventoryItems(shellInventory) == shellItems &&
        pilotInventory->GetActiveSlot() == pilotSlot && shellInventory->GetActiveSlot() == shellSlot,
        "native PK producer changed suit binding, positions, AP or either inventory before queued consumption");
}

static void ForcedActionRecalc(const char* databasePath,const char* resourcePath)
{
    for (bool forced : {false,true}) {
        auto world = FixtureWorld(databasePath,resourcePath,true);
        vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr;
        for (auto unit : units) {
            auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
            if (candidate && !candidate->IsAIUnit() && candidate->CanFight()) { actor = candidate; break; }
        }
        Require(actor != nullptr && IsValid(NDb::GetAnimation(4437)),"forced recalc fixture has no native actor or authored animation");
        auto read = [&]() {
            CStructureNetworkContext refs; auto fields = BorrowedObjectFields(actor,&refs);
            CObj<NWorld::CCommandExecute> executor; bool flag = false;
            { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(7,&executor); reader.Add(12,&flag); }
            return std::make_pair(executor,flag);
        };
        Require(!read().first && !read().second && !actor->IsPerformingAction(),
            "forced recalc fixture started with a running action");
        const int ap = actor->GetAP(); const auto position = actor->GetPosition();
        if (forced) {
            CObj<NWorld::CExecPlayAnimation> animation = new NWorld::CExecPlayAnimation(actor,4437,false);
            actor->RunCriticalExecutor(animation);
        } else {
            actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdPlayAnimation(4437,false)));
            actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));
        }
        auto before = read();
        Require(IsValid(before.first) && dynamic_cast<NWorld::CExecPlayAnimation*>(before.first.GetPtr()) &&
            before.first->GetState() == NWorld::CCommandExecute::RUNNING && before.second == forced && actor->IsPerformingAction(),
            "native animation entry did not establish the expected running/forced action");
        actor->OnTBSEvent(NWorld::TBS_RECALC_COMMAND);
        auto after = read();
        std::printf("native action recalc: forced=%d retained=%d forced-flag=%d executing=%d\n",
            int(forced),int(after.first.GetPtr() == before.first.GetPtr()),int(after.second),int(before.first->IsExecuting()));
        if (forced) {
            Require(after.first.GetPtr() == before.first.GetPtr() && after.second && actor->IsPerformingAction(),
                "native command recalc drops a running forced action while its forced flag remains latched");
            world->GetTime()->Set(actor->animator.GetTimeEnd());
            actor->Segment(); // Reach the existing animation edge and native retirement path.
            auto finished = read();
            Require(!IsValid(finished.first) && !finished.second && !actor->IsPerformingAction(),
                "native forced animation did not retire and release its flag at the ordinary animation edge");
        } else {
            Require(!IsValid(after.first) && !after.second,"native recalc stopped retiring an ordinary cancelable action");
            before.first->Cancel(); // The retained witness owns the detached control until cleanup.
        }
        Require(actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p,
            "native recalc lifecycle changed logical position or action points");
    }
}

static void ScriptDrawWeaponSlot(const char* databasePath,const char* resourcePath,bool requireQueued = false,bool grenade = false)
{
    for (auto active : {NDb::SLOT_1,NDb::SLOT_2}) {
        auto world = FixtureWorld(databasePath,resourcePath);
        vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr; NDb::CRPGWeapon* record = nullptr;
        for (auto unit : units) {
            auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
            if (!candidate || !candidate->CanFight() || candidate->IsEmptyPK() || candidate->IsWearingPK() ||
                candidate->animator.GetCannon()) continue;
            auto* weapon = dynamic_cast<NRPG::CWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
            if (weapon) { actor = candidate; record = weapon->GetDBWeapon(); break; }
        }
        Require(actor && IsValid(record),"script draw fixture has no native armed actor");
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        vector<CObj<NRPG::IInventoryItem>> removed;
        while (!inventory->GetItems().empty()) {
            CObj<NRPG::IInventoryItem> item = inventory->GetItems().back().pItem;
            removed.push_back(item); inventory->Take(item);
        }
        for (auto slot : {NDb::SLOT_1,NDb::SLOT_2})
            if (inventory->Get(slot)) removed.push_back(inventory->TakeOff(slot));
        CObj<NRPG::CWeaponItem> first = new NRPG::CWeaponItem(record), second = new NRPG::CWeaponItem(record);
        CObj<NRPG::IInventoryItem> spare;
        if (grenade) {
            NDb::CRPGGrenade* grenadeRecord = nullptr;
            CDBIterator<NDb::CRPGGrenade> rows(*NDatabase::GetTable<NDb::CRPGGrenade>());
            while (rows.MoveNext()) if (IsValid(rows.Get()->pItem) && IsValid(rows.Get()->pItem->pModel) &&
                rows.Get()->pItem->nWeight > 0) { grenadeRecord = rows.Get(); break; }
            Require(grenadeRecord != nullptr,"script draw fixture has no authored grenade");
            CObj<NRPG::CGrenadeItem> item = new NRPG::CGrenadeItem(grenadeRecord);
            item->nQuantity = 1; spare = item;
        } else spare = new NRPG::CWeaponItem(record);
        Require(inventory->Equip(NDb::SLOT_1,first) && inventory->Equip(NDb::SLOT_2,second) && inventory->Activate(active),
            "script draw fixture could not equip and select its two native hand slots");
        auto* commander = actor->GetPlayer()->GetCommander();
        Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
            "script draw fixture started with a pending order");
        CObj<NScript::CScript> script = NScript::CreateScript(world);
        NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
        const string call = grenade ? "UnitSwitchToGrenade(upstreamActor)" : "UnitDrawWeapon(upstreamActor)";
        RunScript(script,(call+"; UpstreamDrawReturned = 1").c_str());
        Require(!commander->HasCommands() && !actor->HasCommand() && inventory->GetActiveSlot() == active,
            "native script draw without a backpack weapon changed a hand or issued a command");
        CTPoint<int> place;
        Require(inventory->FindPlace(spare,&place) && inventory->Place(place,spare),
            "script draw fixture could not store its real backpack weapon");
        const auto targetSlot = active == NDb::SLOT_1 ? NDb::SLOT_2 : NDb::SLOT_1;
        auto* targetItem = inventory->Get(targetSlot);
        const auto ap = actor->GetAP();
        RunScript(script,(call+"; UpstreamDrawReturned = 7").c_str());
        auto returned = script->GetGlobal("UpstreamDrawReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 7,"native draw script did not finish its callback");
        const bool queued = commander->HasCommands();
        std::printf("native script inventory dispatch: grenade=%d active=%d queued=%d unit-command=%d\n",
            int(grenade),int(active),int(queued),int(actor->HasCommand()));
        if (requireQueued) Require(queued && !actor->HasCommand(),
            "native script inventory draw executes immediately instead of entering its owning commander's queue");
        CObj<NWorld::CCmd> requested;
        if (queued) {
            CObj<NWorld::CCommand> firstOrder = commander->GetCommand(), resume = commander->GetCommand();
            auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(firstOrder.GetPtr());
            auto* next = dynamic_cast<NWorld::CCmdSetCommand*>(resume.GetPtr());
            Require(set && set->pUnit == actor && next && next->pUnit == actor &&
                dynamic_cast<NWorld::CCmdContinue*>(next->GetCmd()) && !commander->HasCommands(),
                "native draw did not issue its exact actor command and ordered Continue");
            requested = set->GetCmd();
        } else {
            CStructureNetworkContext refs; auto fields = BorrowedObjectFields(actor,&refs);
            { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(9,&requested); }
        }
        auto* exchange = dynamic_cast<NWorld::CCmdExchangeInventoryItems*>(requested.GetPtr());
        Require(exchange != nullptr,"native script draw did not retain its actual inventory exchange request");
        std::printf("native script draw slot: active=%d expected=%d actual=%d target=%d queued=%d\n",
            int(active),int(targetSlot),exchange->GetSlot(),exchange->GetTarget().nSlot,int(queued));
        Require(exchange->GetSlot() == targetSlot && exchange->GetTarget().nSlot == targetSlot,
            "native script draw chooses the active hand instead of the opposite active-slot index");
        Require(exchange->GetActivate() && exchange->GetSource().pUnit == actor && exchange->GetTarget().pUnit == actor &&
            exchange->GetSource().eType == NWorld::SItem::BACKPACK && exchange->GetSource().pItem == spare.GetPtr() &&
            exchange->GetTarget().eType == NWorld::SItem::SLOT && exchange->GetTarget().pItem == targetItem,
            "native script draw lost its real source item, target item, owner or activation intent");
        if (requireQueued) Require(actor->GetAP() == ap && inventory->GetActiveSlot() == active &&
            inventory->Get(NDb::SLOT_1) == first.GetPtr() && inventory->Get(NDb::SLOT_2) == second.GetPtr() &&
            inventory->GetItems().size() == 1 && inventory->GetItems().front().pItem == spare.GetPtr(),
            "native draw changed inventory or AP before queued command consumption");
        CStructureNetworkContext refs; auto fields = BorrowedObjectFields(actor,&refs);
        CObj<NWorld::CCommandExecute> executor;
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(7,&executor); }
        if (IsValid(executor)) executor->Cancel();
    }
}

static void DeferredCreateAndActivateItem(const char* databasePath,const char* resourcePath)
{
    for (int scenario=0; scenario<3; ++scenario) {
        const bool occupied = scenario != 0, full = scenario == 2;
        auto world = FixtureWorld(databasePath,resourcePath);
        vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr; NDb::CRPGWeapon* record = nullptr;
        for (auto unit : units) {
            auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
            if (!candidate || !candidate->CanFight() || candidate->IsEmptyPK() || candidate->IsWearingPK() ||
                candidate->animator.GetCannon()) continue;
            auto* weapon = dynamic_cast<NRPG::CWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
            if (weapon) { actor = candidate; record = weapon->GetDBWeapon(); break; }
        }
        Require(actor && IsValid(record) && IsValid(record->pItem),"script create fixture has no native armed actor/item record");
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        vector<CObj<NRPG::IInventoryItem>> removed;
        while (!inventory->GetItems().empty()) {
            CObj<NRPG::IInventoryItem> item = inventory->GetItems().back().pItem;
            removed.push_back(item); inventory->Take(item);
        }
        for (auto slot : {NDb::SLOT_1,NDb::SLOT_2})
            if (inventory->Get(slot)) removed.push_back(inventory->TakeOff(slot));
        CObj<NRPG::CWeaponItem> old = new NRPG::CWeaponItem(record), active = new NRPG::CWeaponItem(record);
        Require(inventory->Equip(NDb::SLOT_2,active) && inventory->Activate(NDb::SLOT_2) &&
            (!occupied || inventory->Equip(NDb::SLOT_1,old)),"script create fixture could not prepare its actual hand slots");
        if (full) for (int i=0; ; ++i) {
            CObj<NRPG::CWeaponItem> filler = new NRPG::CWeaponItem(record);
            CTPoint<int> place;
            if (!inventory->FindPlace(filler,&place)) break;
            Require(i<100 && inventory->Place(place,filler),"script create fixture could not fill its native backpack");
        }
        CTPoint<int> freePlace;
        Require(inventory->FindPlace(old,&freePlace) == !full,"script create fixture has the wrong real backpack capacity");
        const auto backpack = inventory->GetItems();
        const auto position = actor->GetPosition(); const int ap = actor->GetAP();
        auto* commander = actor->GetPlayer()->GetCommander();
        Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
            "script create fixture started with an existing order");
        CObj<NScript::CScript> script = NScript::CreateScript(world);
        NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
        const string id = std::to_string(record->pItem->GetRecordID());
        RunScript(script,("CreateAndActivateItem(nil,"+id+"); UpstreamCreateReturned = 5").c_str());
        auto returned = script->GetGlobal("UpstreamCreateReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 5 && !commander->HasCommands() && !actor->HasCommand(),
            "script create nil-actor control failed to finish or issued an order");
        RunScript(script,("CreateAndActivateItem(upstreamActor,"+id+"); UpstreamCreateReturned = 9").c_str());
        returned = script->GetGlobal("UpstreamCreateReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 9,"native create Lua callback did not finish");
        std::printf("native script create dispatch: occupied=%d full=%d queued=%d unit-command=%d\n",
            int(occupied),int(full),int(commander->HasCommands()),int(actor->HasCommand()));
        Require(commander->HasCommands() && !actor->HasCommand(),
            "native script create-and-activate bypasses the owning commander's queue");
        CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
        auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
        auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
        auto* create = set ? dynamic_cast<NWorld::CCmdCreateAndActivateInventoryItem*>(set->GetCmd()) : nullptr;
        Require(set && set->pUnit == actor && create && create->GetItem() == record->pItem &&
            create->GetSlot() == NDb::SLOT_1 && create->GetNeedMove() == occupied &&
            resume && resume->pUnit == actor && dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) &&
            !commander->HasCommands(),"queued script create lost its record, fixed hand slot, move flag, actor or Continue");
        if (occupied) {
            const auto& source = create->GetMoveSource(); const auto& target = create->GetMoveTarget();
            Require(source.pUnit == actor && source.eType == NWorld::SItem::SLOT && source.nSlot == NDb::SLOT_1 &&
                source.pItem == old.GetPtr() && target.pUnit == actor &&
                target.eType == (full ? NWorld::SItem::GROUND : NWorld::SItem::BACKPACK) &&
                target.sPosition.x == -1 && target.sPosition.y == -1,
                "queued create lost the old held item or changed backpack/ground stash selection");
        }
        Require(!actor->HasCommand() && actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p &&
            inventory->GetActiveSlot() == NDb::SLOT_2 && inventory->Get(NDb::SLOT_2) == active.GetPtr() &&
            inventory->Get(NDb::SLOT_1) == (occupied ? old.GetPtr() : nullptr) &&
            inventory->GetItems().size() == backpack.size(),"script create mutated actor/hands/backpack/AP before command consumption");
        for (std::size_t i=0; i<backpack.size(); ++i)
            Require(inventory->GetItems()[i].pItem == backpack[i].pItem,"script create replaced backpack items before command consumption");
    }
}

static void DeferredTakeObject(const char* databasePath,const char* resourcePath)
{
    auto world = FixtureWorld(databasePath,resourcePath);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr; NRPG::CWeaponItem* weapon = nullptr;
    for (auto entry : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!candidate || !candidate->CanFight() || candidate->IsEmptyPK() || candidate->IsWearingPK() ||
            candidate->animator.GetCannon()) continue;
        auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
        if (firearm) { actor = candidate; weapon = firearm; break; }
    }
    Require(actor && weapon,"script pickup fixture has no native armed actor");
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    vector<CObj<NRPG::IInventoryItem>> removed;
    while (!inventory->GetItems().empty()) {
        CObj<NRPG::IInventoryItem> item = inventory->GetItems().back().pItem;
        removed.push_back(item); inventory->Take(item);
    }
    CObj<NRPG::CWeaponItem> item = new NRPG::CWeaponItem(weapon->GetDBWeapon());
    CTPoint<int> place;
    Require(inventory->FindPlace(item,&place),"script pickup fixture has no real backpack space");
    auto* body = world->AddFrozenItem(world->GetAIMap(),actor->GetPosition().GetCP()+CVec3(0.25f,0,0.25f),QNULL,item);
    Require(IsValid(body) && body->GetInvItem() == item.GetPtr() && world->GetFrozenItem(item) == body,
        "script pickup fixture did not register its actual world-body/item pair");
    const auto position = actor->GetPosition(); const int ap = actor->GetAP();
    const auto activeSlot = inventory->GetActiveSlot(); const int ammo = weapon->GetAmmoQuantity();
    auto* commander = actor->GetPlayer()->GetCommander();
    Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
        "script pickup fixture started with an existing order");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
    NScript::luaPushCPtr(*script,body); script->SetGlobal("upstreamGroundItem");
    NScript::luaPushCPtr(*script,item.GetPtr()); script->SetGlobal("upstreamInventoryItem");
    RunScript(script,"UnitTakeObject(nil,upstreamGroundItem); UnitTakeObject(upstreamActor,nil); "
        "UnitTakeObject(upstreamActor,upstreamInventoryItem); UpstreamPickupReturned = 13");
    auto returned = script->GetGlobal("UpstreamPickupReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 13 && !commander->HasCommands() && !actor->HasCommand(),
        "pickup nil controls or the fork's world-body argument contract issued an invalid order");
    RunScript(script,"UnitTakeObject(upstreamActor,upstreamGroundItem); UpstreamPickupReturned = 19");
    returned = script->GetGlobal("UpstreamPickupReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 19,"native pickup Lua callback did not finish");
    std::printf("native script pickup dispatch: queued=%d unit-command=%d ground-item-retained=%d\n",
        int(commander->HasCommands()),int(actor->HasCommand()),int(world->GetFrozenItem(item) == body));
    Require(commander->HasCommands() && !actor->HasCommand(),
        "native script pickup bypasses the owning commander's queue");
    CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
    auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
    auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
    auto* move = set ? dynamic_cast<NWorld::CCmdMoveInventoryItem*>(set->GetCmd()) : nullptr;
    Require(set && set->pUnit == actor && move && resume && resume->pUnit == actor &&
        dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) && !commander->HasCommands(),
        "queued pickup lost its actor, move type or exact ordered Continue");
    const auto& source = move->GetSource(); const auto& target = move->GetTarget();
    Require(!IsValid(source.pUnit) && source.eType == NWorld::SItem::GROUND && source.nSlot == -1 &&
        source.pItem == item.GetPtr() && source.pWorldItem == body && target.pUnit == actor &&
        target.eType == NWorld::SItem::BACKPACK && target.sPosition.x == -1 && target.sPosition.y == -1,
        "queued pickup lost the fork's ground-body/item adapter or original ground-to-backpack payload");
    Require(!actor->HasCommand() && actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p &&
        inventory->GetItems().empty() && inventory->GetActiveSlot() == activeSlot && inventory->GetActive() == weapon &&
        weapon->GetAmmoQuantity() == ammo && world->GetFrozenItem(item) == body && body->GetInvItem() == item.GetPtr(),
        "native script pickup changed actor/AP/inventory or removed the world body before queued command consumption");
}

static void NativePKHideGate(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<8,"unknown native PK hide gate mode");NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false,true);auto& world=fixture.world;auto* actor=fixture.actor;const bool worn=mode!=0 && mode!=7,canHideRecord=mode==1 || mode==3 || mode==4 || mode==5,cooldown=mode==3 || mode==7;CObj<NWorld::CUnitServer> shell;NDb::CRPGPers* suit=nullptr;
    if(worn){auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),20.f),&places);NAI::SPathPlace place;bool found=false;for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetMoving(0);if(network->IsPassable(p) && !network->IsLocked(p) && NAI::IsBigLockerPlacePassable(network,p)){place=p;found=true;break;}}Require(found,"PK hide lacks an actual free big-locker placement");CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());while(people.MoveNext()){auto* p=people.Get();if(p->pPanzerklein && p->pModel && p->pPanzerklein->nMaxVP>0 && p->pPanzerklein->bCanHide==canHideRecord && (!suit || p->GetRecordID()<suit->GetRecordID()))suit=p;}Require(suit,"PK hide lacks its authored allowed/forbidden model");shell=world->AddUnitInGame(place,NRPG::CreateUnit(suit),nullptr);Require(shell && shell->IsEmptyPK() && shell->WearAsPK(true),"PK hide cannot prepare its actual empty shell");actor->SetPosition(shell->GetPosition());actor->FlipPanzerklein(shell,false,false);Require(actor->IsWearingPK() && actor->GetWearingDBPK()==suit->pPanzerklein.GetPtr(),"PK hide did not establish its actual worn identity");}
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));actor->GetUnitRPG()->GetRPGUnit()->UpdateSkills();Require(!world->IsRealTime() && !world->IsSequence(),"PK hide fixture did not establish its actual player turn");const bool initial=mode!=5 && !cooldown;actor->EnableHide();actor->Hide(initial,false);actor->EnableHide();if(cooldown)actor->Hide(false,false);Require(actor->IsHiding()==initial && actor->CanHide()==!cooldown,"PK hide cannot prepare actual concealment/cooldown state");
    CObj<NWorld::CCmdHide> command=mode==4 || mode==6?new NWorld::CCmdHide:new NWorld::CCmdHide(false);const bool accepted=(mode==0 || mode==1 || mode==4 || mode==5);const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();auto* active=actor->GetUnitRPG()->GetInventory()->GetActive();auto random=GlobalGameRandom();NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCommandExecute> executor=NWorld::CreateExecutor(actor,command,&result);
    std::printf("native PK hide mode=%d worn=%d persona=%d PK=%d DBcanHide=%d cooldown=%d initial=%d explicit=%d requested=%d result=%d expected=%d executor=%d\n",mode,int(worn),suit?suit->GetRecordID():0,suit?suit->pPanzerklein->GetRecordID():0,int(worn && canHideRecord),int(cooldown),int(initial),int(command->bExplicitState),int(command->bState),int(result),int(accepted?NWorld::UCR_OK:NWorld::UCR_GENERAL_FAILURE),int(bool(executor)));std::fflush(stdout);
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && world->GetTime()->GetValue()==time && actor->IsHiding()==initial && actor->GetUnitRPG()->GetInventory()->GetActive()==active && random.Get()==GlobalGameRandom().Get(),"PK hide availability changed AP/HP/position/time/concealment/item/RNG");
    Require(result==(accepted?NWorld::UCR_OK:NWorld::UCR_GENERAL_FAILURE) && bool(executor)==accepted,"native Hide executor rejects an authored stealth-capable PK or bypasses forbidden/cooldown gates");
    if(executor){Require(executor->GetStartAP()==0,"PK unhide/toggle acquired a hide-entry AP cost");executor->Run();Require(!actor->IsHiding() && executor->GetState()==NWorld::CCommandExecute::FINISHED && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && actor->GetUnitRPG()->GetInventory()->GetActive()==active,"actual accepted PK unhide/toggle changed AP/HP/item/position or failed completion");}
}


class CameraFloorBridgeMissionFixture: public NGame::CMission
{
public:
    void Attach(NWorld::IWorld* world,NGScene::IGameView* view){pWorld=world;pScene=view;}
    void Detach(){pWorld=nullptr;pScene=nullptr;}
};

static void NativeCameraFramingBridge(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<3,"unknown camera framing bridge mode");Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"camera bridge starts with an interface");NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm camera floor bridge regression");
    struct ClockRestore{STime old;~ClockRestore(){NMainLoop::SetTimeForHarness(old);}} clock{NMainLoop::SetTimeForHarness(1000)};CObj<NDb::CScript> quiet=new NDb::CScript;quiet->strCode="function OnEnterZone() end";
    struct Restore{vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> variants;~Restore(){for(auto& p:variants)if(IsValid(p.first))p.first->pScript=p.second;}} restore;
    CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());while(variants.MoveNext())if(variants.Get()->pTemplate && variants.Get()->pTemplate->GetRecordID()==2425){restore.variants.push_back({variants.Get(),variants.Get()->pScript});variants.Get()->pScript=quiet.GetPtr();}Require(!restore.variants.empty(),"camera bridge lacks authored menu variants");CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    CObj<NGame::CMissionBase> owner=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());auto* world=owner?dynamic_cast<NWorld::CWorld*>(owner->GetWorld()):nullptr;auto* view=owner?owner->GetScene():nullptr;auto* camera=owner?owner->GetCamera():nullptr;Require(world && view && camera && owner->GetRenderGame(),"camera bridge lacks its actual initialized mission/world/view/camera/renderer");
    NMainLoop::SetTimeForHarness(1050);Require(NMainLoop::StepApp(true,false,false),"camera bridge warm-up frame exited");Require(NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"camera bridge opened an unrelated interface");while(auto* pending=world->GetUICommand()){CObj<NWorld::CUICmd> release=pending;}
    // Exercise concrete tactical-mission forwarding and its real factory/accessor,
    // sharing the initialized native view/world; the menu base floor methods are no-ops.
    CObj<CameraFloorBridgeMissionFixture> bridgeOwner=new CameraFloorBridgeMissionFixture;bridgeOwner->Attach(world,view);
    CObj<ICamera> bridgeCamera=bridgeOwner->CreateCamera(CAMERA_PC);Require(bridgeCamera,"floor bridge cannot create its actual tactical camera");camera=bridgeCamera;
    struct Detach{CameraFloorBridgeMissionFixture* owner;~Detach(){owner->Detach();}} detach{bridgeOwner};
    // Use the supported no-world framing context to isolate floor publication from terrain/LOS.
    // The factory-installed native handles and render view remain attached; only
    // the real mission step publishes its selected camera floor.
    camera->SetWorld(nullptr);struct RestoreWorld{ICamera* camera;NWorld::IWorld* world;~RestoreWorld(){camera->SetWorld(world);}} restoreWorld{camera,world};camera->SetPlacement(ICamera::SCameraPos(CVec3(0,0,0),20.f,-FP_PI/4,.125f,0,35.f));camera->SetScreenRect(CTRect<float>(0,0,1,1));camera->SetCutFloorRange(0,2);view->SetCutFloorRange(0,4);camera->SetCutFloor(1);view->SetCutFloor(1);if(mode==1)camera->FreezeCamera(true);
    const int request=mode==0?9:mode==1?2:1,expected=mode==0?2:1;const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();camera->ShowPlacesFromBestPoint(CVec3(-.125f,0,0),CVec3(-.125f,0,0),request,24.f,1,1.f,true,true);
    const int cameraFloor=camera->GetCutFloor(),viewFloor=view->GetCutFloor();std::printf("native camera floor bridge mode=%d frozen=%d request=%d cameraFloor=%d viewFloor=%d expectedCamera=%d expectedView=1\n",mode,int(mode==1),request,cameraFloor,viewFloor,expected);std::fflush(stdout);
    Require(world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get() && NMainLoop::GetCurrentInterfaceForHarness()==owner.GetPtr(),"floor framing advanced world time/RNG or changed interface ownership");if(mode==1)camera->FreezeCamera(false);
    Require(cameraFloor==expected && viewFloor==1,"camera framing bypassed its clamp/freeze or published the render floor before the real selected-camera mission step");
}


static void NativeCameraCandidate(int mode)
{
    Require(mode>=0 && mode<8,"unknown camera candidate mode");CObj<ICamera> camera=::CreateCamera(CAMERA_PC);Require(camera,"candidate fixture lacks actual camera");
    const float yaws[]={.125f,-.75f,2.4f,-2.2f,.5235988f,3.66519165f,.125f,.125f};const float yaw=yaws[mode],rod=mode==6?20.f:mode==7?std::nextafter(35.f,0.f):40.f;
    camera->SetPlacement(ICamera::SCameraPos(CVec3(0,0,0),rod,-FP_PI/4,yaw,0,35.f));camera->SetScreenRect(CTRect<float>(0,0,1,1));camera->SetCutFloorRange(0,2);camera->SetCutFloor(0);ICamera::SCameraPos liveBefore;camera->GetPlacement(&liveBefore);auto random=GlobalGameRandom();const CVec3 A(-.125f,0,0),B(.125f,0,0);
    camera->ShowPlacesFromBestPoint(A,B,0,24.f,1,1.f,true,true);ICamera::SCameraPos desired,liveAfter;{CStructureNetworkContext refs;auto fields=BorrowedObjectFields(camera,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(4,&desired);}camera->GetPlacement(&liveAfter);const float expectedRod=mode>=6?rod:15.f;
    std::printf("native camera candidate mode=%d initialRod=%.9g initialYaw=%.9g rod=%.9g expectedRod=%.9g yaw=%.9g expectedYaw=%.9g floor=%d ratio=%d\n",mode,rod,yaw,desired.fRod,expectedRod,desired.fYaw,yaw,camera->GetCutFloor(),camera->GetSloMoRatio());std::fflush(stdout);
    Require(std::memcmp(&liveBefore,&liveAfter,sizeof(liveBefore))==0 && camera->GetCutFloor()==0 && camera->GetSloMoRatio()==1 && random.Get()==GlobalGameRandom().Get(),"candidate selection changed live pose/floor/ratio/gameplay RNG");
    Require(desired.fRod==expectedRod && desired.fYaw==yaw,"framing rotated away before trying the current viewing angle");
}

static void NativeCameraSloMoWindow(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<6,"unknown camera slow-motion window mode");CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);Require(campaign && world->GetGlobalGame()==campaign.GetPtr(),"camera window lacks actual campaign/world binding");
    const float divisor=mode==3?1.f:mode==4?1.5f:1.6f;const bool cheat=mode!=2;const int requestedRatio=mode==5?1:3,initialCount=3,expectedWindow=divisor>1.f?int(3.f/divisor):3;
    struct Restore {NRPG::CGlobalGame* game;int count;NGlobal::CValue option;~Restore(){game->nSloMoTimes=count;NGlobal::SetVar("cheat_slomo",option);}} restore{campaign,campaign->nSloMoTimes,NGlobal::GetVar("cheat_slomo",0)};campaign->nSloMoTimes=initialCount;NGlobal::SetVar("cheat_slomo",cheat?1:0);
    unsigned seed=0;int roll=-1,legacyRoll=-1;for(unsigned trial=1;trial<10000;++trial){GlobalGameRandom().SeedForHarness(trial);SRand a,b;const int current=a.Get(expectedWindow),old=b.Get(divisor>1.f?int(3.f/divisor+.5f):3);if((mode==1?current==0 && old==0:mode==0?current==0 && old!=0:true)){seed=trial;roll=current;legacyRoll=old;break;}}Require(seed,"camera window cannot isolate its deterministic native local roll");
    CObj<ICamera> camera=::CreateCamera(CAMERA_PC);camera->SetWorld(world);camera->SetScreenRect(CTRect<float>(0,0,1,1));camera->SetCutFloorRange(0,4);const CVec3 point(0,0,1000);camera->SetPlacement(ICamera::SCameraPos(point,20.f,-FP_PI/4,.125f,0,35.f));ICamera::SCameraPos liveBefore;camera->GetPlacement(&liveBefore);const auto time=world->GetTime()->GetValue();GlobalGameRandom().SeedForHarness(seed);auto random=GlobalGameRandom();
    camera->ShowPlacesFromBestPoint(point,point,0,24.f,requestedRatio,divisor,true,true);const bool fired=requestedRatio>1 && roll==0 && cheat;const int expectedRatio=1;S2CameraWire::SCameraSloMo state;{CStructureNetworkContext refs;auto fields=BorrowedObjectFields(camera,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(10,&state);}ICamera::SCameraPos liveAfter;camera->GetPlacement(&liveAfter);
    std::printf("native camera slomo window mode=%d count=%d divisor=%.9g window=%d seed=%u roll=%d legacyRoll=%d cheat=%d requested=%d ratio=%d expectedRatio=%d afterCount=%d expectedCount=%d lease=%d\n",mode,initialCount,divisor,expectedWindow,seed,roll,legacyRoll,int(cheat),requestedRatio,camera->GetSloMoRatio(),expectedRatio,campaign->nSloMoTimes,initialCount+int(fired),state.tMaxLen);std::fflush(stdout);
    Require(std::memcmp(&liveBefore,&liveAfter,sizeof(liveBefore))==0 && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"camera slow-motion query changed live pose/world time/gameplay ISAAC");
    Require(camera->GetSloMoRatio()==expectedRatio && campaign->nSloMoTimes==initialCount+int(fired) && state.tMaxLen==(fired?4000:0),"camera slow-motion probability window rounded instead of truncating");
}

static void NativeCameraFraming(int mode)
{
    Require(mode>=0 && mode<10,"unknown native camera framing mode");CObj<ICamera> camera=::CreateCamera(CAMERA_PC);Require(camera,"framing fixture lacks its actual native camera");
    const float rods[]={20.f,std::nextafter(35.f,0.f),35.f,std::nextafter(35.f,100.f),40.f};const bool single=mode>=5;const float rod=single?20.f:rods[mode];
    const ICamera::SCameraPos initial(CVec3(0,0,0),rod,-FP_PI/4,.125f,0,35.f);camera->SetPlacement(initial);camera->SetScreenRect(CTRect<float>(0,0,1,1));camera->SetCutFloorRange(0,2);camera->SetCutFloor(single?1:0);
    if(mode==6)camera->FreezeCamera(true);if(mode==7){camera->FreezeCamera(true);camera->FreezeCamera(true);camera->FreezeCamera(false);}if(mode==8){camera->FreezeCamera(true);camera->FreezeCamera(false);}
    const int requested=mode==5?9:mode==6 || mode==7?2:mode==8?-3:mode==9?1:0;const int expectedFloor=mode==5?2:mode==6 || mode==7 || mode==9?1:0;
    struct BaseState {int floor=-99,minimum=-99,maximum=-99,locks=-99,frozen=-99;int operator&(CStructureSaver& f){f.Add(2,&locks);f.Add(3,&floor);f.Add(11,&minimum);f.Add(12,&maximum);f.Add(14,&frozen);return 0;}};
    auto state=[&](ICamera::SCameraPos* desired){BaseState base;CStructureNetworkContext refs;auto fields=BorrowedObjectFields(camera,&refs);{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,&base);reader.Add(4,desired);}return base;};
    ICamera::SCameraPos before;const auto baseBefore=state(&before);ICamera::SCameraPos liveBefore;camera->GetPlacement(&liveBefore);auto random=GlobalGameRandom();const CVec3 A(-.125f,0,0),B=single?A:CVec3(.125f,0,0);
    camera->ShowPlacesFromBestPoint(A,B,requested,24.f,1,1.f,true,true);ICamera::SCameraPos desired;const auto after=state(&desired);ICamera::SCameraPos liveAfter;camera->GetPlacement(&liveAfter);const bool keep=single || mode<2;const float expectedRod=keep?rod:15.f,expectedYaw=.125f /* source1b85 now tries the current angle first */;
    std::printf("native camera framing mode=%d single=%d initialRod=%.9g requestedFloor=%d beforeFloor=%d frozen=%d desiredRod=%.9g expectedRod=%.9g yaw=%.9g expectedYaw=%.9g floor=%d expectedFloor=%d savedFloor=%d\n",mode,int(single),rod,requested,baseBefore.floor,baseBefore.frozen,desired.fRod,expectedRod,desired.fYaw,expectedYaw,camera->GetCutFloor(),expectedFloor,after.floor);std::fflush(stdout);
    Require(std::memcmp(&liveBefore,&liveAfter,sizeof(liveBefore))==0 && after.minimum==0 && after.maximum==2 && after.locks==baseBefore.locks && after.frozen==baseBefore.frozen && random.Get()==GlobalGameRandom().Get(),"camera framing changed live placement/clamp range/freeze counts or gameplay RNG");
    Require(desired.fRod==expectedRod && std::fabs(desired.fYaw-expectedYaw)<1e-6f && camera->GetCutFloor()==expectedFloor && after.floor==expectedFloor,"native framing uses the wrong strict35 rod gate or bypasses frozen/clamped virtual floor updates");
}


static void NativeHeldGrenadePriority(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<9,"unknown held grenade priority mode");NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false,true);auto* actor=fixture.actor;auto* inventory=actor->GetUnitRPG()->GetInventory();
    NDb::CRPGGrenade* regular=nullptr;NDb::CRPGEngGrenade* engineer=nullptr;NDb::CRPGWeapon* gun=nullptr;
    CDBIterator<NDb::CRPGGrenade> regularRows(*NDatabase::GetTable<NDb::CRPGGrenade>());while(regularRows.MoveNext()){auto* p=regularRows.Get();if(p->pItem && p->pItem->pModel && p->pItem->nWeight>0 && (!regular || p->GetRecordID()<regular->GetRecordID()))regular=p;}
    CDBIterator<NDb::CRPGEngGrenade> engRows(*NDatabase::GetTable<NDb::CRPGEngGrenade>());while(engRows.MoveNext()){auto* p=engRows.Get();if(p->pItem && p->pItem->pModel && p->pItem->nWeight>0 && (p->nRequiredPerkID<=0 || actor->GetUnitRPG()->HasPerk(p->nRequiredPerkID)) && (!engineer || p->GetRecordID()<engineer->GetRecordID()))engineer=p;}
    vector<CPtr<NWorld::CUnit>> units;fixture.world->GetAllUnits(&units);for(auto u:units){auto* s=dynamic_cast<NWorld::CUnitServer*>(u.GetPtr());auto* w=s?s->GetUnitRPG()->GetWeaponItem():nullptr;if(w && w->GetDBWeapon() && (!gun || w->GetDBWeapon()->GetRecordID()<gun->GetRecordID()))gun=w->GetDBWeapon();}
    Require(regular && engineer && gun,"held priority lacks actual authored regular/engineer/firearm records");
    auto make=[&](int kind)->CObj<NRPG::IInventoryItem>{if(kind==0){CObj<NRPG::CGrenadeItem> p=new NRPG::CGrenadeItem(regular);p->nQuantity=1;return p.GetPtr();}if(kind==1){CObj<NRPG::CGrenadeItem> p=new NRPG::CGrenadeItem(engineer);p->nQuantity=1;return p.GetPtr();}return new NRPG::CWeaponItem(gun);};
    int firstKind=0,secondKind=0;NDb::ESlot active=mode==0?NDb::SLOT_1:NDb::SLOT_2;
    if(mode>=2 && mode<=5){firstKind=mode<4?1:2;secondKind=mode<4?2:1;active=(mode%2)==0?NDb::SLOT_1:NDb::SLOT_2;}
    if(mode==6){firstKind=secondKind=1;active=NDb::SLOT_1;}if(mode==7){firstKind=1;secondKind=0;active=NDb::SLOT_1;}if(mode==8){firstKind=0;secondKind=1;active=NDb::SLOT_2;}
    vector<CObj<NRPG::IInventoryItem>> removed;for(auto slot:{NDb::SLOT_1,NDb::SLOT_2})if(inventory->Get(slot))removed.push_back(inventory->TakeOff(slot));auto first=make(firstKind),second=make(secondKind),spare=make(0);CTPoint<int> place;
    Require(inventory->Equip(NDb::SLOT_1,first) && inventory->Equip(NDb::SLOT_2,second) && inventory->Activate(active) && inventory->FindPlace(spare,&place) && inventory->Place(place,spare),"held priority cannot prepare its actual hands/backpack");
    auto* commander=actor->GetPlayer()->GetCommander();Require(commander && !commander->HasCommands() && !actor->HasCommand(),"held priority starts with pending orders");CObj<NScript::CScript> script=NScript::CreateScript(fixture.world);NScript::luaPushCPtr(*script,actor);script->SetGlobal("upstreamActor");
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();const auto time=fixture.world->GetTime()->GetValue();auto random=GlobalGameRandom();RunScript(script,"UnitSwitchToGrenade(upstreamActor); UpstreamHeldPriority=19");auto returned=script->GetGlobal("UpstreamHeldPriority");Require(returned.IsNumber() && returned.GetNumber()==19,"held priority Lua callback did not finish");
    const int held=firstKind==0?int(NDb::SLOT_1):secondKind==0?int(NDb::SLOT_2):-1;const bool expectedQueue=held<0 || held!=int(active);const int target=held<0?int(active==NDb::SLOT_1?NDb::SLOT_2:NDb::SLOT_1):held;const bool queued=commander->HasCommands();
    std::printf("native held grenade priority mode=%d firstKind=%d secondKind=%d active=%d regular=%d engineer=%d held=%d expectedTarget=%d queued=%d expectedQueue=%d\n",mode,firstKind,secondKind,int(active),regular->GetRecordID(),engineer->GetRecordID(),held,target,int(queued),int(expectedQueue));std::fflush(stdout);
    Require(!actor->HasCommand() && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && fixture.world->GetTime()->GetValue()==time && inventory->GetActiveSlot()==active && inventory->Get(NDb::SLOT_1)==first.GetPtr() && inventory->Get(NDb::SLOT_2)==second.GetPtr() && inventory->GetItems().size()==1 && inventory->GetItems().front().pItem==spare.GetPtr() && random.Get()==GlobalGameRandom().Get(),"held priority changed AP/HP/item/position/time/RNG before queue consumption");
    Require(queued==expectedQueue,"native held grenade selection blocks engineer fallback or loses first-hand priority");
    if(!queued)return;CObj<NWorld::CCommand> one=commander->GetCommand(),two=commander->GetCommand();auto* set=dynamic_cast<NWorld::CCmdSetCommand*>(one.GetPtr());auto* resume=dynamic_cast<NWorld::CCmdSetCommand*>(two.GetPtr());Require(set && resume && set->pUnit==actor && resume->pUnit==actor && dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) && !commander->HasCommands(),"held priority lost actual actor/ordered Continue or queued extra orders");
    if(held>=0){auto* select=dynamic_cast<NWorld::CCmdSetActiveItem*>(set->GetCmd());Require(select && int(select->nSlot)==target,"held priority selected wrong held grenade or drew the backpack instead");}
    else{auto* exchange=dynamic_cast<NWorld::CCmdExchangeInventoryItems*>(set->GetCmd());Require(exchange && exchange->GetSlot()==target && exchange->GetActivate() && exchange->GetSource().pUnit==actor && exchange->GetSource().eType==NWorld::SItem::BACKPACK && exchange->GetSource().pItem==spare.GetPtr() && exchange->GetTarget().pUnit==actor && exchange->GetTarget().eType==NWorld::SItem::SLOT && exchange->GetTarget().nSlot==target && exchange->GetTarget().pItem==inventory->Get(NDb::ESlot(target)),"engineer fallback lost its exact opposite-slot exchange payload/owner/activation");}
}


static void ScriptHeldGrenadeActivation(const char* databasePath,const char* resourcePath)
{
    for (bool engineer : {true,false}) for (auto heldSlot : {NDb::SLOT_1,NDb::SLOT_2}) for (bool alreadyActive : {true,false}) {
        NativeGrenadeThrowFixture fixture(databasePath,resourcePath,engineer);
        auto* actor = fixture.actor;
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        NDb::CRPGWeapon* weaponRecord = nullptr;
        vector<CPtr<NWorld::CUnit>> units; fixture.world->GetAllUnits(&units);
        for (auto unit : units) {
            auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
            auto* weapon = candidate ? candidate->GetUnitRPG()->GetWeaponItem() : nullptr;
            if (weapon && IsValid(weapon->GetDBWeapon())) { weaponRecord = weapon->GetDBWeapon(); break; }
        }
        Require(weaponRecord != nullptr,"held-grenade fixture has no authored firearm donor");
        vector<CObj<NRPG::IInventoryItem>> removed;
        for (auto slot : {NDb::SLOT_1,NDb::SLOT_2})
            if (inventory->Get(slot)) removed.push_back(inventory->TakeOff(slot));
        const auto otherSlot = heldSlot == NDb::SLOT_1 ? NDb::SLOT_2 : NDb::SLOT_1;
        CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(weaponRecord);
        NDb::CRPGGrenade* spareRecord = fixture.regular;
        if (!spareRecord) {
            CDBIterator<NDb::CRPGGrenade> rows(*NDatabase::GetTable<NDb::CRPGGrenade>());
            while (rows.MoveNext()) if (IsValid(rows.Get()->pItem) && IsValid(rows.Get()->pItem->pModel) &&
                rows.Get()->pItem->nWeight > 0) { spareRecord = rows.Get(); break; }
        }
        Require(spareRecord != nullptr,"held-grenade fixture has no regular backpack grenade");
        CObj<NRPG::CGrenadeItem> spare = new NRPG::CGrenadeItem(spareRecord);
        spare->nQuantity = 1;
        const auto active = alreadyActive ? heldSlot : otherSlot;
        CTPoint<int> place;
        Require(inventory->Equip(heldSlot,fixture.grenade) && inventory->Equip(otherSlot,weapon) &&
            inventory->Activate(active) && inventory->FindPlace(spare,&place) && inventory->Place(place,spare),
            "held-grenade fixture could not equip its actual hands and spare backpack grenade");
        auto* commander = actor->GetPlayer()->GetCommander();
        Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
            "held-grenade fixture started with an existing order");
        const auto ap = actor->GetAP();
        CObj<NScript::CScript> script = NScript::CreateScript(fixture.world);
        NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
        RunScript(script,"UnitSwitchToGrenade(upstreamActor); UpstreamHeldReturned = 11");
        auto returned = script->GetGlobal("UpstreamHeldReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 11,"held-grenade Lua callback did not finish");
        std::printf("native held grenade: engineer=%d held=%d active=%d already-active=%d queued=%d unit-command=%d\n",
            int(engineer),int(heldSlot),int(active),int(alreadyActive),int(commander->HasCommands()),int(actor->HasCommand()));
        if (engineer) {
            Require(commander->HasCommands(),"held engineer grenade blocked the original ordinary-backpack fallback");
            CObj<NWorld::CCommand> first=commander->GetCommand(),second=commander->GetCommand();
            auto* set=dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());auto* resume=dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
            auto* exchange=set?dynamic_cast<NWorld::CCmdExchangeInventoryItems*>(set->GetCmd()):nullptr;
            const auto target=active==NDb::SLOT_1?NDb::SLOT_2:NDb::SLOT_1;
            Require(set && set->pUnit==actor && exchange && exchange->GetSlot()==target && exchange->GetActivate() &&
                exchange->GetSource().pUnit==actor && exchange->GetSource().eType==NWorld::SItem::BACKPACK && exchange->GetSource().pItem==spare.GetPtr() &&
                exchange->GetTarget().pUnit==actor && exchange->GetTarget().eType==NWorld::SItem::SLOT && exchange->GetTarget().nSlot==target && exchange->GetTarget().pItem==inventory->Get(target) &&
                resume && resume->pUnit==actor && dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) && !commander->HasCommands(),
                "engineer held fallback lost its opposite-slot exchange, actual actor/item or ordered Continue");
        } else if (alreadyActive) Require(!commander->HasCommands(),"active ordinary grenade issued a new command");
        else {
            Require(commander->HasCommands(),"native grenade switch leaves the held grenade inactive and queues no activation");
            CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
            auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
            auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
            auto* select = set ? dynamic_cast<NWorld::CCmdSetActiveItem*>(set->GetCmd()) : nullptr;
            Require(set && set->pUnit == actor && select && select->nSlot == heldSlot &&
                resume && resume->pUnit == actor && dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) &&
                !commander->HasCommands(),"held-grenade activation lost its slot/actor/Continue or drew the spare instead");
        }
        Require(!actor->HasCommand() && actor->GetAP() == ap && inventory->GetActiveSlot() == active &&
            inventory->Get(heldSlot) == fixture.grenade.GetPtr() && inventory->Get(otherSlot) == weapon.GetPtr() &&
            inventory->GetItems().size() == 1 && inventory->GetItems().front().pItem == spare.GetPtr(),
            "held-grenade script changed inventory or AP before queued activation consumption");
    }
}

static void ScriptedEnemyDeath(const char* databasePath,const char* resourcePath, bool spareUnconscious = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    auto* human = world->GetPlayerByID(0);
    Require(IsValid(human) && !NAI::IsAIPlayer(human), "scripted death fixture has no scenario-player-zero human");
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    vector<CPtr<NWorld::CUnitServer>> enemies;
    vector<std::pair<CPtr<NWorld::CUnitServer>,int>> protectedUnits;
    for (const auto& unit: units) {
        if (!IsValid(unit) || !unit->CanFight()) continue;
        if (world->GetDiplomacyState(human,unit->GetPlayer()) == NDb::DS_ENEMY)
            enemies.push_back(unit);
        else protectedUnits.push_back({unit,unit->GetAP()});
    }
    Require(!enemies.empty() && !protectedUnits.empty(), "scripted death fixture needs hostile and protected units");
    CPtr<NWorld::CUnitServer> unconscious;
    if (spareUnconscious) {
        Require(enemies.size() > 1, "scripted unconscious gate fixture needs two hostile actors");
        unconscious = enemies.front();
        enemies.erase(enemies.begin());
        unconscious->MakeUnconscious(VNULL3,true,false);
        Require(unconscious->IsUnconscious() && !unconscious->IsDead() && !unconscious->CanFight() &&
            !unconscious->GetUnitRPG()->IsDead(), "scripted unconscious gate fixture did not preserve a living incapacitated hostile");
    }
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    RunScript(script,"KillEmAll()");
    if (spareUnconscious)
        Require(unconscious->IsUnconscious() && !unconscious->IsDead() && !unconscious->GetUnitRPG()->IsDead(),
            "KillEmAll killed an incapacitated hostile outside its fight-capable target set");
    for (const auto& enemy: enemies) {
        Require(enemy->GetUnitRPG()->IsDead(),
            "KillEmAll left a world-dead hostile's RPG persona alive");
        Require(enemy->IsDead() && !enemy->CanFight(), "KillEmAll did not complete the hostile's world death");
    }
    for (const auto& item: protectedUnits)
        Require(item.first->CanFight() && !item.first->GetUnitRPG()->IsDead() && item.first->GetAP() == item.second,
            "KillEmAll harmed a friendly or neutral unit or spent its AP");
}

static bool SavedWalkWithoutWeapon(NWorld::CUnitServer* actor)
{
    CStructureNetworkContext graph;
    graph.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(actor)->operator&(writer); }
    saved.Seek(0);
    bool holstered = false;
    { CStructureSaver reader(saved, CStructureSaver::READ, &graph); reader.Add(26, &holstered); }
    return holstered;
}

static void ScriptHolsterPersistence(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (server && server->CanFight() && !server->IsEmptyPK() && !server->IsWearingPK()) { actor = server; break; }
    }
    Require(actor != nullptr, "scripted holster fixture has no living actor");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script, actor); script->SetGlobal("upstreamActor");
    const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    Require(!SavedWalkWithoutWeapon(actor), "new actor unexpectedly requested persistent holstering");
    RunScript(script, "UnitActivateWeapon(upstreamActor, nil)");
    Require(SavedWalkWithoutWeapon(actor), "scripted holstering did not persist the walk-without-weapon flag");
    RunScript(script, "UnitActivateWeapon(upstreamActor, 1)");
    Require(!SavedWalkWithoutWeapon(actor), "scripted weapon activation did not clear persistent holstering");
    RunScript(script, "UnitActivateWeapon(upstreamActor, nil)");
    RunScript(script, "UnitActivateWeapon(upstreamActor)");
    Require(!SavedWalkWithoutWeapon(actor), "default scripted activation retained the holster flag");
    Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "recording scripted holster intent spent action points before queued execution");
}

enum class MovementHolsterCheck { Persistent, ShortInactive, HiddenActive };

static void MovementHolsterPolicy(const char* databasePath, const char* resourcePath,
    MovementHolsterCheck check = MovementHolsterCheck::Persistent)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (server && server->CanFight() && !server->IsEmptyPK() && !server->IsWearingPK()
                && server->GetUnitRPG()->CanMove() && server->GetPosition().pos.p.GetPose() != NAI::CM_INACTIVE
                && IsValid(server->GetUnitRPG()->GetInventoryInfo()->GetActive())) { actor = server; break; }
    }
    Require(actor != nullptr, "movement holster fixture has no mobile armed actor");
    CObj<NAI::CPath> path = new NAI::CPath;
    path->pNet = world->GetPathNetwork();
    auto place = actor->GetPosition().pos.p;
    const int initialDirection = place.GetDirection();
    for (int turn = 0; turn < 4; ++turn) {
        place.SetDirection((initialDirection + turn) & 7);
        path->points.push_back(place);
    }
    const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    int deactivations = 0;
    auto activationCount = [&](bool persistent, NWorld::ENeedActiveItem constraint, int pointCount = 4,
        bool animationActive = false, bool modelHidden = true) {
        actor->SetWalkWithoutWeapon(persistent);
        actor->animator.SetActiveItem(animationActive);
        actor->SetUndrawItem(modelHidden);
        CObj<NAI::CPath> planned = new NAI::CPath;
        planned->pNet = path->pNet.GetPtr();
        planned->points.assign(path->points.begin(), path->points.begin() + pointCount);
        CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(actor, planned, NAI::PF_DEFAULT, constraint);
        Require(IsValid(move), "movement holster fixture could not create its native path executor");
        int activations = 0, rotations = 0;
        deactivations = 0;
        CStructureNetworkContext graph;
        graph.include = [&](CObjectBase* object) {
            const int type = pSSClasses->GetObjectTypeID(object);
            if (type == int(0x113B1130)) ++activations;
            if (type == int(0x114B1130)) ++deactivations;
            if (type == int(0x02731143)) ++rotations;
            return false;
        };
        CMemoryStream traversal;
        { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
          static_cast<CObjectBase*>(move.GetPtr())->operator&(writer); }
        Require(rotations >= pointCount - 1, "movement holster fixture lost its native turn commands");
        return activations;
    };
    Require(activationCount(false, NWorld::ITEM_NO_MATTER) == 1,
        "ordinary movement did not retain its normal weapon activation");
    Require(activationCount(true, NWorld::ITEM_NO_MATTER) == 0,
        "native movement queued automatic weapon activation despite persistent scripted holstering");
    Require(activationCount(true, NWorld::ITEM_ACTIVE) == 1,
        "persistent holstering suppressed an explicit active-weapon path constraint");
    if (check == MovementHolsterCheck::ShortInactive) {
        for (int count: {2, 3}) {
            Require(activationCount(false, NWorld::ITEM_INACTIVE, count) == 0 && deactivations == 0,
                "short inactive path queued an unnecessary weapon draw and holster cycle");
        }
        Require(activationCount(false, NWorld::ITEM_INACTIVE, 4) == 1 && deactivations == 1,
            "short inactive-path exception changed activation policy for a longer path");
    }
    if (check == MovementHolsterCheck::HiddenActive) {
        Require(activationCount(false, NWorld::ITEM_NO_MATTER, 4, true, true) == 1,
            "hidden weapon was treated as drawn solely because its animator flag was active");
        Require(activationCount(false, NWorld::ITEM_NO_MATTER, 4, true, false) == 0,
            "a genuinely drawn weapon received a redundant activation command");
        Require(activationCount(true, NWorld::ITEM_NO_MATTER, 4, true, true) == 0,
            "drawn-state reconciliation bypassed persistent scripted holstering");
    }
    Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "constructing the movement activation policy spent action points");
}

static void ExplicitWeaponActivation(const char* databasePath, const char* resourcePath, bool movement)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (server && server->CanFight() && !server->IsEmptyPK() && !server->IsWearingPK()
                && server->GetUnitRPG()->CanMove() && IsValid(server->GetUnitRPG()->GetInventoryInfo()->GetActive())) {
            actor = server; break;
        }
    }
    Require(actor != nullptr, "explicit activation fixture has no mobile armed actor");
    actor->SetWalkWithoutWeapon(true);
    actor->animator.SetActiveItem(false);
    actor->SetUndrawItem(true);
    if (movement) {
        CObj<NAI::CPath> path = new NAI::CPath;
        path->pNet = world->GetPathNetwork(); path->points.push_back(actor->GetPosition().pos.p);
        CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(actor, path, NAI::PF_DEFAULT, NWorld::ITEM_ACTIVE);
        Require(IsValid(move), "explicit activation fixture could not create its movement executor");
        move->Run();
        const bool reset = !actor->IsWalkingWithoutWeapon();
        move->TimeLabelReached(); move->AnimationFinished(); move = nullptr;
        Require(reset, "explicit movement weapon activation retained the scripted holster flag");
        Require(!world->IsAction(), "completed movement activation retained its action counter");
        return;
    }
    const auto slot = static_cast<NDb::ESlot>(actor->GetUnitRPG()->GetInventoryInfo()->GetActiveSlot());
    auto equipment = [&](int stage) {
        CObj<NWorld::CCommandExecute> executor = dynamic_cast<NWorld::CCommandExecute*>(
            pSSClasses->CreateObject(int(0x01122133))); // Native CExecSetActiveItem.
        Require(IsValid(executor), "native equipment activation class is not registered");
        struct OwnerWire {
            CPtr<NWorld::CUnitServer> actor;
            int operator&(CStructureSaver& saver) { saver.Add(4, &actor); return 0; }
        } owner;
        owner.actor = actor;
        auto savedSlot = slot;
        bool twoHeavy = false;
        CStructureNetworkContext references;
        references.externalReferences = true;
        references.ids[actor] = 1; references.refs[1] = actor;
        references.nextID = 100; references.include = [](CObjectBase*) { return false; };
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
          writer.Add(1, &owner); writer.Add(2, &savedSlot); writer.Add(3, &stage); writer.Add(4, &twoHeavy); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ, &references);
          static_cast<CObjectBase*>(executor.GetPtr())->operator&(reader); }
        Require(executor->GetUnitServer() == actor, "restored equipment callback lost its native actor");
        return executor;
    };
    auto transientFlags = [&]() {
        struct BaseFlags {
            bool undraw = false, noHeavy = false;
            int operator&(CStructureSaver& saver) { saver.Add(10,&undraw); saver.Add(11,&noHeavy); return 0; }
        } flags;
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(actor,&references);
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(2,&flags); }
        return std::make_pair(flags.undraw,flags.noHeavy);
    };
    const int ap = actor->GetAP();
    auto executor = equipment(1);
    executor->AnimationFinished();
    const bool reset = !actor->IsWalkingWithoutWeapon();
    actor->SetWalkWithoutWeapon(true); // A newer script request while the animation completes.
    actor->SetUndrawItem(true,true);
    Require(transientFlags() == std::make_pair(true,true), "equipment fixture did not seed both native temporary holster flags");
    executor->AnimationFinished();
    Require(reset, "explicit equipment activation retained the scripted holster flag");
    Require(actor->IsWalkingWithoutWeapon() && executor->GetState() == NWorld::CCommandExecute::FINISHED,
        "equipment completion overwrote a later scripted holster request");
    Require(!actor->GetUndrawItem() && transientFlags() == std::make_pair(false,false),
        "completed equipment activation retained temporary item or heavy-weapon hiding flags");
    CObj<NRPG::IInventoryItem> removed = actor->GetUnitRPG()->GetInventory()->TakeOff(slot);
    actor->SetUndrawItem(true,true);
    auto empty = equipment(1);
    empty->AnimationFinished();
    Require(actor->IsWalkingWithoutWeapon() && empty->GetState() == NWorld::CCommandExecute::FINISHED,
        "empty equipment slot cancelled scripted holstering without activating an item");
    Require(!actor->GetUndrawItem() && transientFlags() == std::make_pair(false,false),
        "completing activation of an empty slot retained temporary item hiding flags");
    Require(actor->GetAP() == ap && !actor->HasCommand(), "clearing equipment visibility flags spent AP or installed an action");
}

static void ScriptAIActivation(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NAI::IAIUnit* ai = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight()) continue;
        auto* candidate = NAI::GetAIUnit(unit);
        if (candidate && candidate->IsUnderAIControl() && IsValid(candidate->GetReaction())) {
            actor = unit; ai = candidate; break;
        }
    }
    Require(actor && ai, "script activation fixture has no controlled AI actor");
    CObj<NAI::CAILogic> route = new NAI::CAILogic(ai);
    ai->SetRouteLogic(route);
    Require(ai->GetRouteLogic() == route.GetPtr() && route->IsActive(), "script activation fixture route is not active");
    CObj<NAI::CAIReaction> original = ai->GetReaction();
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script, actor); script->SetGlobal("upstreamActor");
    const int beforeAP = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    RunScript(script, "UnitCheat(upstreamActor, 32, 1)");
    Require(actor->GetUnitRPG()->GetRPGUnit()->IsCheatEnabled(NRPG::CHEAT_NOAI), "script failed to set the no-AI flag");
    Require(ai->GetRouteLogic() == route.GetPtr() && !route->IsActive(),
        "scripted AI deactivation left the current native route running");
    CObj<NAI::CAIReaction> disabled = ai->GetReaction();
    Require(IsValid(disabled) && disabled != original.GetPtr(), "scripted AI deactivation did not replace the active reaction");
    RunScript(script, "UnitCheat(upstreamActor, 32, nil)");
    Require(!actor->GetUnitRPG()->GetRPGUnit()->IsCheatEnabled(NRPG::CHEAT_NOAI)
            && ai->GetRouteLogic() == route.GetPtr() && route->IsActive(),
        "scripted AI activation did not resume the preserved route");
    Require(IsValid(ai->GetReaction()) && ai->GetReaction() != disabled.GetPtr(),
        "scripted AI activation retained the disabled reaction");
    Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == beforeAP,
        "scripted AI activation changed the actor's action points");
}
