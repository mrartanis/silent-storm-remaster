#include "TestSupport.h"
#include "../../Main/aiGrid.h"
#include "../../Main/aiMap.h"
#include "../../Main/aiColourer.h"
#include "../../Main/aiMapProxy.h"
#include "../../Main/aiObject.h"
#include "../../Main/aiPath.h"
#include "../../Main/aiMoves.h"
#include "../../Main/aiNearestPosition.h"
#include "../../Main/PrecalcCollider.h"
#include "../../Main/RPGBullet.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wMine.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>

namespace UpstreamTests
{
namespace
{
class FootprintOwner : public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(FootprintOwner);
public:
    bool IsPathLocker() override { return true; }
    bool IsEmptyPK() const override { return false; }
    NDb::CPanzerklein* GetWearingDBPK() override { return nullptr; }
};

void KnownMinePathReservations()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    const float heights[] = {0.f, 0.2f, 0.4f};
    for (int floor = 0; floor < 3; ++floor) {
        const int layer = network->CreateLayer(8, 8, VNULL2, 0.f, floor);
        auto& tile = network->GetLayer(layer)->tiles[3][3];
        tile = NAI::STile{};
        tile.nHeight = NAI::GetIHeight(heights[floor]);
        tile.nPassable = NAI::CP_STAND;
    }
    network->Freeze(true);
    const NAI::SPathPlace base(3, 3, 0), triggering(3, 3, 1), nearby(3, 3, 2);
    auto center = network->GetCP(base);
    auto* nearLayer = network->GetLayer(nearby.GetLayer());
    vector<NAI::SPathPlace> candidates;
    nearLayer->AddNearPoints(SSphere(center, 0.5f), &candidates, false);
    Require(std::any_of(candidates.begin(), candidates.end(), [&](const auto& node) {
        return node.GetLayer() == nearby.GetLayer() && node.GetX() == nearby.GetX() && node.GetY() == nearby.GetY();
    }),
        "the non-triggering surface must be inside the navigation search sphere");

    CObj<NWorld::CMine> mine = new NWorld::CMine;
    CMemoryStream fields;
    { CStructureSaver saver(fields, CStructureSaver::WRITE); saver.Add(3, &center); }
    fields.Seek(0);
    { CStructureSaver saver(fields, CStructureSaver::READ);
      static_cast<CObjectBase*>(mine.GetPtr())->operator&(saver); }
    CObj<NWorld::CMineTracker> tracker = new NWorld::CMineTracker;
    tracker->AddMine(mine, center);
    vector<CPtr<NWorld::CMine>> found;
    Require(tracker->GetMines({network->GetCP(triggering)}, &found) &&
        !tracker->GetMines({network->GetCP(nearby)}, &found),
        "the candidate surfaces must straddle the native mine trigger");

    CObj<FootprintOwner> owner = new FootprintOwner;
    network->Lock(owner, base);
    network->LockSelected({owner, mine.GetPtr()});
    Require(network->IsLocked(base, true) && network->IsLocked(triggering, true),
        "the route query omitted a known mine's triggering surface");
    Require(!network->IsLocked(nearby, true), "the route query reserved a surface that cannot trigger the mine");
    network->Unlock(owner);
    Require(network->IsLocked(base, true), "releasing a unit footprint accidentally cleared the known mine constraint");
    network->Lock(owner, base);
    network->UnlockSelected();
    Require(network->IsLocked(base, true) && !network->IsLocked(triggering, true),
        "query cleanup lost the restored unit footprint or retained the temporary mine constraint");
    network->LockSelected({});
    Require(!network->IsLocked(triggering, true), "a later route query excluded a mine that it did not know about");
    network->UnlockSelected();
    network->Unlock(owner);
}

static void PrecalcSweepBounds()
{
    const vector<CVec3> points{
        CVec3(0,-.5f,-.5f),CVec3(.07f,-.5f,-.5f),CVec3(.07f,.5f,-.5f),CVec3(0,.5f,-.5f),
        CVec3(0,-.5f,.5f),CVec3(.07f,-.5f,.5f),CVec3(.07f,.5f,.5f),CVec3(0,.5f,.5f)};
    const vector<STriangle> triangles{
        STriangle(0,2,1),STriangle(0,3,2),STriangle(4,5,6),STriangle(4,6,7),
        STriangle(0,1,5),STriangle(0,5,4),STriangle(3,7,6),STriangle(3,6,2),
        STriangle(0,4,7),STriangle(0,7,3),STriangle(1,2,6),STriangle(1,6,5)};
    CObj<NAI::CPrecalcSpheres> grid = new NAI::CPrecalcSpheres;
    Require(!grid->Generate(points,triangles,0), "thin-wall fixture unexpectedly requires another voxel Z block");
    NCollider::CPrecalcCollider<int> collider;
    SBound sceneBound,geometryBound;
    sceneBound.BoxInit(CVec3(-4,-4,-4),CVec3(4,4,4));
    geometryBound.BoxInit(CVec3(0,-.5f,-.5f),CVec3(.07f,.5f,.5f));
    collider.PrepareCollider(sceneBound,1.f);
    SHMatrix identity; Identity(&identity);
    collider.AddEntity(identity,identity,vector<CPtr<NAI::CPrecalcSpheres>>{grid.GetPtr()},false,17,geometryBound);
    struct HitAnalyzer {
        int calls = 0;
        bool operator()(int owner) { Require(owner == 17, "precalc sweep changed its collision owner"); ++calls; return true; }
    };
    for (const auto& velocity: {VNULL3,CVec3(0,.05f,0)}) {
        for (float x: {-.30f,.37f}) {
            HitAnalyzer hit;
            Require(collider.CollideBool(SSphere(CVec3(x,0,0),NAI::F_TEST_SPHERE_RADIUS),velocity,&hit) && hit.calls == 1,
                "a genuine thin-wall collision was lost, including the neighbouring broad-phase cell");
        }
        const CVec3 outside(.381f,0,0);
        Require(outside.x - .07f > NAI::F_TEST_SPHERE_RADIUS && grid->IsMovingSphereCollided(outside,outside+velocity),
            "thin-wall fixture did not expose conservative voxel rounding outside the exact padded bound");
        HitAnalyzer hit;
        Require(!collider.CollideBool(SSphere(outside,NAI::F_TEST_SPHERE_RADIUS),velocity,&hit) && hit.calls == 0,
            "a precalculated thin wall reported a collision outside its radius-padded bound because the sweep radius was counted twice");
    }
}

static void UnsupportedNavigationFloor()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    const int first = network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    auto* layer = network->GetLayer(first);
    const int supporting = network->CreateLayer(8, 8, VNULL2, 0.f, 1, layer->pGroup);
    auto* same = network->GetLayer(supporting);
    auto& empty = layer->tiles[2][3];
    auto& support = same->tiles[2][3];
    empty = NAI::STile{}; support = NAI::STile{};
    empty.nFloor = 100;
    support.nFloor = -1; support.nPassable = 7;
    const NAI::SPathPlace place(3, 2, first);
    Require(network->GetFloor(place) == -1, "loaded empty navigation slot did not resolve its real supporting surface floor");
    Require(empty.nFloor == 100 && empty.nHeight == 0 && empty.nPassable == 0,
        "navigation floor lookup rewrote the saved empty tile instead of resolving its support");
    empty.nFloor = 2;
    Require(network->GetFloor(place) == 2, "native tile floor was replaced by another layer's surface floor");
    empty.nFloor = 100;
    support.nHeight = 1;
    Require(network->GetFloor(place) == 100, "unrelated height was accepted as navigation support");
    support.nHeight = 0; support.nDisplacement = 1;
    Require(network->GetFloor(place) == 100, "displaced navigation tile was accepted as identical support");
    support.nDisplacement = 0; support.nPassable = 0;
    Require(network->GetFloor(place) == 100, "impassable navigation surface was accepted as support");
    const int foreign = network->CreateLayer(8, 8, CVec2(8, 0), 0.f, 0);
    auto& foreignTile = network->GetLayer(foreign)->tiles[2][3];
    foreignTile = NAI::STile{}; foreignTile.nFloor = 3; foreignTile.nPassable = 7;
    Require(network->GetFloor(place) == 100, "navigation support leaked across unrelated layer groups");
    support.nFloor = 100; support.nPassable = 7;
    Require(network->GetFloor(place) == 100, "another empty navigation slot invented a supporting floor");
    support.nFloor = -1; empty.nHeight = support.nHeight = 123;
    Require(network->GetFloor(place) == -1, "saved duplicate support resolution failed above zero height");
    NAI::CNodesLayer::SLadder ladder{};
    ladder.placeOnBottom = place;
    layer->ladders.push_back(ladder);
    NAI::SPathPlace rung(0, 4, first); rung.SetIntegral(0);
    Require(network->GetFloor(rung) == 0, "ladder floor calculation did not resolve its bottom support before adding rung floors");
    NAI::SPathPlace flying(0, 0, 200); flying.SetFinal(1);
    Require(network->GetFloor(flying) == 4, "navigation support correction changed flying-position floor handling");
}

static void LazyNavigationColourer()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    auto* layer = network->GetLayer(0);
    const bool allocatedBeforeFirstRequest = IsValid(layer->pColourer);
    layer->pColourer = nullptr; // Same missing-cache state as a compact save.
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            auto& tile = layer->tiles[y][x];
            tile = NAI::STile{}; tile.nPassable = NAI::CP_STAND;
            tile.nMoveStand = static_cast<char>(0xff); tile.nFlags = NAI::TF_STAND_PASSABLE;
        }
    const NAI::CPathNetwork* query = network.GetPtr();
    auto* cache = query->GetColourer(0);
    Require(IsValid(cache) && cache == layer->pColourer.GetPtr(), "native navigation lookup did not reconstruct a cache omitted by a compact save");
    cache->ConstructColouring(network);
    Require(cache->IsReady() && cache->GetPointColor(3, 3) == cache->GetPointColor(4, 3),
        "lazily allocated navigation cache did not build the real open-floor connectivity");
    Require(query->GetColourer(0) == cache && cache->IsReady(), "repeated navigation cache lookup discarded a completed cache");
    CPtr<NAI::CMapColourer> previous = cache;
    layer->pColourer = nullptr;
    cache = query->GetColourer(0);
    Require(IsValid(cache) && !IsValid(previous) && cache != previous.GetPtr(),
        "navigation cache lookup failed to restore a cache omitted by a compact save");
    cache->ConstructColouring(network);
    cache->Invalidate();
    Require(!IsValid(layer->pColourer), "stale navigation cache fixture remained valid");
    cache = query->GetColourer(0);
    Require(IsValid(cache), "navigation lookup returned an invalidated colour cache");
    cache->ConstructColouring(network);
    Require(cache->IsReady() && cache->GetPointColor(3, 3) == cache->GetPointColor(4, 3),
        "replacement navigation cache lost its layer binding or connectivity");
    CObj<NAI::IAIMapTracker> tracker = dynamic_cast<NAI::IAIMapTracker*>(pSSClasses->CreateObject(0x72952150));
    Require(IsValid(tracker), "native navigation change tracker is not registered");
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.include = [](CObjectBase*) { return false; };
    references.ids[layer->pGroup.GetPtr()] = 1; references.refs[1] = layer->pGroup.GetPtr();
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
      CPtr<NAI::CLayersGroup> group = layer->pGroup.GetPtr(); CTPoint<int> cell(0, 0);
      writer.Add(2, &group); writer.Add(3, &cell); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references); static_cast<CObjectBase*>(tracker.GetPtr())->operator&(reader); }
    layer->pColourer = nullptr;
    layer->pGroup->squareLevel[0][0] = 2;
    tracker->OnChange();
    Require(IsValid(layer->pColourer) && !layer->pColourer->zonesToRecalc.empty() && layer->pGroup->squareLevel[0][0] == 0,
        "native scenery change did not reconstruct an omitted navigation cache before marking its dirty zone");
    Require(!allocatedBeforeFirstRequest, "fresh navigation layer allocated its colour cache before the first request");
}

static void FreshNavigationTransitions()
{
    // Full attachment is the control; the incremental entry point must produce
    // the same real inter-layer neighbours even with no dirty rectangles.
    for (bool fullFirst: {true, false}) {
        CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
        const int first = network->CreateLayer(8, 8, CVec2(0, 0), 0.f, 0);
        const int second = network->CreateLayer(8, 8, CVec2(8, 0), 0.f, 0);
        Require(first == 0 && second == 1, "transition fixture did not create its two layers");
        for (int index: {first, second}) {
            auto* layer = network->GetLayer(index);
            for (int y = 0; y < 8; ++y)
                for (int x = 0; x < 8; ++x) {
                    layer->tiles[y][x] = NAI::STile{};
                    layer->tiles[y][x].nPassable = 7;
                    layer->tiles[y][x].nFlags = NAI::TF_STAND_PASSABLE;
                }
        }
        const NAI::SPathPlace source(1, 1, first), destination(1, 1, second);
        auto* sourceLayer = network->GetLayer(first);
        sourceLayer->tiles[1][1].nFlags |= NAI::TF_HAS_INTERGRID;
        sourceLayer->transitions[source].links.emplace_back(destination, 0);
        auto* colourer = network->GetColourer(first);
        colourer->ConstructColouring(network);
        network->GetColourer(second)->ConstructColouring(network);
        if (fullFirst) colourer->AttachTransitions(network);
        else colourer->RecalcTransitions(network);
        auto neighbours = [&](NAI::EPathfinderMode mode) {
            vector<unsigned char> xs, ys;
            vector<int> layers;
            colourer->GetColorNeighbourCenters(network, colourer->GetPointColor(1, 1), &xs, &ys, &layers, mode);
            Require(xs.size() == layers.size() && ys.size() == layers.size(), "transition neighbour coordinates were truncated");
            return layers;
        };
        for (auto mode: {NAI::PM_STAND_ONLY, NAI::PM_ANY_MOVE}) {
            const auto result = neighbours(mode);
            Require(result.size() == 1 && result[0] == second,
                fullFirst ? "full attachment control did not connect the two layers"
                          : "first incremental transition update omitted existing inter-layer links");
        }
        sourceLayer->transitions[source].links.emplace_back(NAI::SPathPlace(2, 2, second), 0);
        colourer->RecalcTransitions(network);
        Require(neighbours(NAI::PM_STAND_ONLY).size() == 1 && neighbours(NAI::PM_ANY_MOVE).size() == 1,
            "already attached transition cache was rebuilt without a dirty region");
    }
}

// Keep the native call boundary out of MSVC's test-loop inliner.
#if defined(_MSC_VER)
__declspec(noinline)
#else
__attribute__((noinline))
#endif
static unsigned char ColouringMoveCost(const NAI::CNodesLayerProxy* proxy,
    unsigned char x,unsigned char y,NAI::EDirection direction)
{
    return proxy->GetOneMoveCost(x,y,direction);
}

static void InvalidRegionColouring()
{
    auto layer = []() {
        CObj<NAI::CNodesLayer> result = new NAI::CNodesLayer;
        result->nLayer = result->nFloor = 0; result->tiles.SetSizes(3,3);
        for (int y=0;y<3;++y) for (int x=0;x<3;++x) result->tiles[y][x] = NAI::STile{};
        return result;
    };
    // Preserve existing valid-edge weights: cardinal/diagonal and all four
    // supported movement types, with the stand-only restriction unchanged.
    const int poses[] = {2,1,0,3}, weights[] = {1,2,3,5};
    for (auto direction: {NAI::RIGHT,NAI::UPRIGHT})
        for (int kind=0;kind<4;++kind) {
            auto grid = layer(); const int opposite = (int(direction)+4)&7;
            auto& source = grid->tiles[1][1];
            auto& target = grid->tiles[1+NAI::cTranslations[direction][1]][1+NAI::cTranslations[direction][0]];
            source.nPassable = target.nPassable = 7;
            source.nFlags = target.nFlags = NAI::TF_STAND_PASSABLE;
            source.nMove[poses[kind]] = static_cast<char>(1<<direction);
            target.nMove[poses[kind]] = static_cast<char>(1<<opposite);
            CObj<NAI::CNodesLayerProxy> proxy = new NAI::CNodesLayerProxy(grid);
            const int step = direction == NAI::RIGHT ? 2 : 3;
            proxy->SetPathfinderMode(NAI::PM_ANY_MOVE);
            Require(ColouringMoveCost(proxy.GetPtr(),1,1,direction) == step*weights[kind],
                "valid navigation edge changed its existing movement cost");
            proxy->SetPathfinderMode(NAI::PM_STAND_ONLY);
            Require(ColouringMoveCost(proxy.GetPtr(),1,1,direction) == (kind == 0 ? step : 0),
                "stand-only navigation changed its original pose restriction");
            proxy->SetPathfinderMode(NAI::PM_ANY_MOVE); target.nLocks = 1;
            Require(ColouringMoveCost(proxy.GetPtr(),1,1,direction) == 0, "valid source entered a locked target");
        }
    auto grid = layer();
    auto& blocked = grid->tiles[1][1]; auto& walkable = grid->tiles[1][0];
    blocked.nMoveStand = static_cast<char>(1<<NAI::LEFT);
    walkable.nMoveStand = static_cast<char>(1<<NAI::RIGHT);
    walkable.nPassable = 7; walkable.nFlags = NAI::TF_STAND_PASSABLE;
    CObj<NAI::CNodesLayerProxy> proxy = new NAI::CNodesLayerProxy(grid);
    Require(!proxy->IsValidPoint(1,1) && proxy->IsValidPoint(0,1),
        "navigation region fixture did not retain one invalid source and one valid neighbour");
    CObj<NAI::CMapColourer> colourer = new NAI::CMapColourer;
    colourer->SetLayer(grid,nullptr,0); colourer->ConstructColouring(nullptr);
    std::printf("native recolouring: invalid=%u valid=%u internal-invalid=%u edge-cost=%d\n",
        unsigned(colourer->GetPointColor(1,1)),unsigned(colourer->GetPointColor(0,1)),
        unsigned(colourer->GetPointColor(0,0)),int(ColouringMoveCost(proxy.GetPtr(),1,1,NAI::LEFT)));
    Require(colourer->GetPointColor(1,1) != colourer->GetPointColor(0,1),
        "native colouring joined an invalid tile to a walkable region through a stale movement flag");
    Require(colourer->GetPointColor(1,1) == colourer->GetPointColor(0,0),
        "native colouring failed to connect the internal invalid region");
    for (auto mode: {NAI::PM_STAND_ONLY,NAI::PM_ANY_MOVE}) {
        proxy->SetPathfinderMode(mode);
        Require(ColouringMoveCost(proxy.GetPtr(),1,1,NAI::LEFT) == 0,
            "invalid source still connects to a valid neighbour");
        Require(ColouringMoveCost(proxy.GetPtr(),1,1,NAI::UP) == 2 && ColouringMoveCost(proxy.GetPtr(),1,1,NAI::UPRIGHT) == 3,
            "internal invalid-region colouring changed cardinal/diagonal weights");
    }
}

static void FreshNavigationColouring()
{
    CObj<NAI::CNodesLayer> layer = new NAI::CNodesLayer;
    layer->nLayer = layer->nFloor = 0;
    layer->tiles.SetSizes(8, 8);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) layer->tiles[y][x] = NAI::STile{};
    // Two genuine walkable islands; blocked-cell label identities are not
    // part of this cache-initialization regression's contract.
    layer->tiles[0][0].nFlags = layer->tiles[7][7].nFlags = NAI::TF_STAND_PASSABLE;
    layer->tiles[0][0].nPassable = layer->tiles[7][7].nPassable = 7;
    CObj<NAI::CMapColourer> full = new NAI::CMapColourer;
    full->SetLayer(layer, nullptr, 0);
    full->ConstructColouring(nullptr);
    Require(full->IsReady() && full->GetPointColor(0, 0) != full->GetPointColor(7, 7),
        "fresh-colouring fixture did not build separate walkable islands");
    CObj<NAI::CMapColourer> incremental = new NAI::CMapColourer;
    incremental->SetLayer(layer, nullptr, 0);
    incremental->RecalcColouring(nullptr, 0);
    Require(incremental->IsReady(),
        "first incremental navigation update left a new layer without its connectivity cache");
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            Require(incremental->GetPointColor(x, y) == full->GetPointColor(x, y),
                "first incremental navigation update differs from a full connectivity build");
    const auto first = incremental->GetPointColor(0, 0), last = incremental->GetPointColor(7, 7);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            auto& tile = layer->tiles[y][x];
            for (auto& moves: tile.nMove) moves = static_cast<char>(0xff);
            tile.nFlags = NAI::TF_STAND_PASSABLE;
            tile.nPassable = 7;
        }
    full->ConstructColouring(nullptr);
    Require(full->GetPointColor(0, 0) == full->GetPointColor(7, 7),
        "modified colouring fixture did not form a connected region");
    incremental->RecalcColouring(nullptr, 0);
    Require(incremental->GetPointColor(0, 0) == first && incremental->GetPointColor(7, 7) == last,
        "already constructed colourer repeated a full build without a dirty region");
}

class EmptySearchNetwork: public NAI::CPathNetwork
{
    OBJECT_BASIC_METHODS(EmptySearchNetwork);
public:
    int queries = 0;
    void GetNearPlaces(const SSphere&, vector<NAI::SPathPlace>* places, bool) override {
        Require(++queries <= 8, "nearest-position search continued beyond its maximum radius on an empty grid");
        places->clear();
    }
};

static void EmptyNearestPosition()
{
    CObj<EmptySearchNetwork> network = new EmptySearchNetwork;
    const auto result = NAI::GetNearestPosition(VNULL3, network);
    Require(result.p == NAI::SPathPlace() && result.GetNetwork() == network.GetPtr(),
        "empty nearest-position search returned an unrelated grid or a valid tile");
    Require(network->queries == 8, "nearest-position search skipped its bounded fallback attempts");
}

class FiniteSearchNetwork: public NAI::CPathNetwork
{
    OBJECT_BASIC_METHODS(FiniteSearchNetwork);
public:
    int queries = 0;
    void GetNearPlaces(const SSphere& sphere, vector<NAI::SPathPlace>* places, bool) override {
        ++queries;
        Require(std::isfinite(sphere.ptCenter.x) && std::isfinite(sphere.ptCenter.y) && std::isfinite(sphere.ptCenter.z),
            "non-finite nearest-position coordinates reached navigation grid conversion");
        places->clear();
    }
};

static void InvalidNearestPosition()
{
    CObj<FiniteSearchNetwork> network = new FiniteSearchNetwork;
    for (float invalid : {std::numeric_limits<float>::quiet_NaN(),
            std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()}) {
        for (int axis = 0; axis < 3; ++axis) {
            CVec3 point = VNULL3;
            if (axis == 0) point.x = invalid;
            if (axis == 1) point.y = invalid;
            if (axis == 2) point.z = invalid;
            for (bool native : {false, true}) {
                const auto result = native ? NAI::GetNearestNativePosition(point, network)
                    : NAI::GetNearestPosition(point, network);
                Require(result.p == NAI::SPathPlace(), "invalid coordinates produced a navigation tile");
            }
        }
    }
    Require(network->queries == 0, "invalid nearest-position search queried navigation tiles");
    Require(NAI::GetNearestPosition(VNULL3, nullptr).p == NAI::SPathPlace(),
        "missing navigation network produced a valid tile");
    network->Invalidate();
    Require(NAI::GetNearestPosition(VNULL3, network).p == NAI::SPathPlace(),
        "invalid navigation network produced a valid tile");
}

static void SimplePronePosePath()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8,8,VNULL2,0.f,0);
    network->Freeze(true);
    for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x) {
        auto& tile = network->GetLayer(0)->tiles[y][x];
        tile = NAI::STile{};
        tile.nPassable = 0xff;
        tile.nMoveStand = tile.nMoveCrouch = tile.nMoveLay = 0xff;
        tile.nFlags = NAI::TF_STAND_PASSABLE;
    }
    NAI::SPathPlace prone(4,4,0,0,NAI::CM_LAY,0), standing(4,4,0,0,NAI::CM_STAND,0);
    auto find = [&](const NAI::SPathPlace& source,const vector<NAI::SPathPlace>& goals,bool corpse = false) {
        return CObj<NAI::CPath>(NAI::FindSimplePath(source,goals,NAI::PF_USE_POSE,network,false,false,corpse));
    };
    auto crouched = standing;
    crouched.SetPose(NAI::CM_CROUCH);
    Require(IsValid(find(standing,{crouched})), "ordinary direct pose-change control failed");
    auto path = find(prone,{standing});
    Require(IsValid(path), "simple native pathfinding rejected a direct pose change from prone");
    Require(path->points.size() == 2 && path->points.front() == prone && path->points.back() == standing && path->actions.empty(),
        "direct prone pose path changed its coordinates, heading or action list");
    Require(IsValid(find(prone,{crouched})), "simple native pathfinding rejected a direct crouch transition from prone");
    Require(!IsValid(find(prone,{})) && !IsValid(find(prone,{prone})),
        "simple prone pathfinding changed empty-goal or prone-only rejection");
    auto inactive = prone;
    inactive.SetPose(NAI::CM_INACTIVE);
    Require(!IsValid(find(inactive,{standing})) && !IsValid(find(prone,{standing},true)),
        "simple pose paths bypassed inactive-source or corpse-carrying restrictions");
    auto displaced = standing;
    displaced.SetXY(5,4);
    Require(!IsValid(find(prone,{displaced})), "simple prone pose handling incorrectly replaced a movement route");
}

static void PronePoseChangeFacing()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8,8,VNULL2,0.f,0);
    network->Freeze(true);
    for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x) {
        auto& tile = network->GetLayer(0)->tiles[y][x];
        tile = NAI::STile{};
        tile.nPassable = 0xff;
        tile.nMoveStand = tile.nMoveCrouch = tile.nMoveLay = 0xff;
        tile.nFlags = NAI::TF_STAND_PASSABLE;
    }
    NAI::SPathPlace prone(4,4,0,3,NAI::CM_LAY,0);
    CObj<NAI::CPath> turn = new NAI::CPath;
    turn->pNet = network.GetPtr();
    auto turned = prone;
    turned.SetDirection(4);
    Require(turn->AddNewPoint(prone) && turn->AddNewPoint(turned) && turn->points.back() == turned,
        "explicit same-pose prone turn control lost its requested heading");
    for (int pose: {NAI::CM_STAND,NAI::CM_CROUCH}) {
        CObj<NAI::CPath> path = new NAI::CPath;
        path->pNet = network.GetPtr();
        NAI::SPathPlace canonical(4,4,0,0,pose,0);
        Require(path->AddNewPoint(prone) && path->AddNewPoint(canonical),
            "native path rejected a valid prone-to-upright pose transition");
        Require(path->points.size() == 2 && path->points.back().GetPose() == pose &&
            path->points.back().GetDirection() == prone.GetDirection(),
            "leaving prone adopted the wave's canonical direction instead of retaining the actual facing");
        Require(path->points.front() == prone && path->points.back().GetX() == 4 &&
            path->points.back().GetY() == 4 && path->points.back().GetLayer() == 0 && path->actions.empty(),
            "preserving pose-change facing moved the unit or introduced an unrelated path action");
        auto finalTurn = path->points.back();
        finalTurn.SetDirection(6);
        Require(path->AddNewPoint(finalTurn) && path->points.back().GetDirection() == 6,
            "the following explicit upright turn lost its requested heading");
    }
}

static void IsolatedSourcePath()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8,8,VNULL2,0.f,0);
    network->CreateLayer(8,8,VNULL2,0.f,1,network->GetLayer(0)->pGroup);
    // Keep the real pathfinder and colour caches active; only geometry refresh
    // is frozen because these native tiles are the complete fixture map.
    network->GetLayer(0)->pGroup->bFreeze = true;
    NAI::SPathPlace source(2,2,0), goal(5,2,1);
    source.SetPose(NAI::CM_STAND); goal.SetPose(NAI::CM_STAND);
    const int right = network->GetDir(NAI::SPathPlace(2,2,1),NAI::SPathPlace(3,2,1));
    const int left = (right + 4) & 7;
    for (int x = 1; x <= 6; ++x) {
        auto& tile = network->GetLayer(1)->tiles[2][x];
        tile = NAI::STile{};
        tile.nHeight = 100;
        tile.nPassable = 0xff;
        tile.nFlags = NAI::TF_STAND_PASSABLE;
        tile.nMoveStand = (x > 1 ? 1 << left : 0) | (x < 6 ? 1 << right : 0);
        tile.nMoveCrouch = tile.nMoveLay = tile.nMoveStand;
    }
    auto& isolated = network->GetLayer(0)->tiles[2][2];
    isolated = network->GetLayer(1)->tiles[2][2];
    isolated.nMoveStand = isolated.nMoveCrouch = isolated.nMoveLay = 0;
    int costs[NAI::N_MOVE_TYPES];
    std::fill(std::begin(costs),std::end(costs),1);
    costs[NAI::MT_ZERO] = 0;
    costs[NAI::MT_MOVE_STAND] = 2;
    costs[NAI::MT_MOVE_CROUCH] = 3;
    costs[NAI::MT_MOVE_CRAWL] = 4;
    const list<CObjectBase*> occupants;
    auto find = [&](int limit, NAI::EFindPathParams params) {
        return CObj<NAI::CPath>(NAI::FindPath(network,source,vector<NAI::SPathPlace>{goal},
            costs,limit,false,occupants,nullptr,true,false,params,false));
    };
    auto reachesGoal = [&](NAI::CPath* path) {
        if (!IsValid(path) || path->points.empty()) return false;
        const auto& end = path->points.back();
        return end.GetX() == goal.GetX() && end.GetY() == goal.GetY() && end.GetLayer() == goal.GetLayer();
    };
    const auto shortSearch = find(39,NAI::PF_USE_POSE);
    Require(reachesGoal(shortSearch), "local-wave control could not leave the isolated source and reach its exact standing goal");
    vector<unsigned char> xs,ys;
    vector<int> neighbours;
    auto* colourer = network->GetColourer(0);
    colourer->GetColorNeighbourCenters(network,colourer->GetPointColor(2,2),&xs,&ys,&neighbours,NAI::PM_ANY_MOVE);
    Require(neighbours.empty(), "path fixture unexpectedly had a cached upper-net escape route");
    const auto route = find(100,NAI::PF_DEFAULT);
    if (IsValid(route) && !route->points.empty()) {
        const auto& end = route->points.back();
        std::printf("Full isolated-source route ends at (%u,%u,layer %u,pose %u); expected (%u,%u,layer %u)\n",
            unsigned(end.GetX()),unsigned(end.GetY()),unsigned(end.GetLayer()),unsigned(end.GetPose()),
            unsigned(goal.GetX()),unsigned(goal.GetY()),unsigned(goal.GetLayer()));
    } else std::puts("Full isolated-source route was absent despite the successful local-wave control");
    Require(reachesGoal(route), "upper-net zone pruning rejected a reachable route from an isolated source or stopped before its goal");
    Require(route->points.front().GetLayer() == source.GetLayer() &&
        route->points.front().GetX() == source.GetX() && route->points.front().GetY() == source.GetY(),
        "isolated-source path changed its starting position");
    auto& escape = network->GetLayer(1)->tiles[2][2];
    escape.nMoveStand = escape.nMoveCrouch = escape.nMoveLay = 0;
    Require(!IsValid(find(100,NAI::PF_DEFAULT)), "isolated-source zone bypass invented a route without a connected escape");
}

static void ConnectedAliasMoves(bool isolatedSource = false, bool lockedDestination = false)
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    for (int i = 0; i < 3; ++i)
        network->CreateLayer(4,4,VNULL2,0.f,i,i ? network->GetLayer(0)->pGroup.GetPtr() : nullptr);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 100;
    connected.nPassable = NAI::CP_STAND | NAI::CP_CROUCH;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE | NAI::TF_HAS_SAME;
    for (int i = 0; i < 3; ++i) network->GetLayer(i)->tiles[1][1] = connected;
    auto& isolated = network->GetLayer(1)->tiles[1][1];
    isolated.nMoveStand = 0;
    isolated.nFlags = NAI::TF_STAND_PASSABLE;
    auto& destination = network->GetLayer(2)->tiles[1][1];
    if (lockedDestination) destination.nLocks = 1;
    NAI::SPathPlace source(1,1,0);
    source.SetPose(NAI::CM_STAND);
    auto& sourceTile = network->GetLayer(0)->tiles[1][1];
    if (isolatedSource) {
        sourceTile.nMoveStand = 0;
        sourceTile.nFlags = NAI::TF_STAND_PASSABLE;
    }
    auto aliases = [&]() {
        vector<NAI::SMove> moves(512);
        vector<char> locks(512,-1);
        int count = 0;
        NAI::GetNonStandartMoves(network,source,true,true,true,&moves,&count,&locks);
        Require(count >= 0 && count < 512, "alias move enumeration exceeded its fixture capacity");
        vector<int> result;
        for (int i = 0; i < count; ++i) {
            if (moves[i].type != NAI::MT_ZERO) continue;
            const auto& place = moves[i].dest;
            Require(place.GetX() == source.GetX() && place.GetY() == source.GetY() &&
                place.GetPose() == source.GetPose() && locks[i] == 0,
                "a zero-distance alias changed coordinates, pose or dynamic-lock accounting");
            result.push_back(place.GetLayer());
        }
        return result;
    };
    Require(aliases() == (lockedDestination ? vector<int>{} : vector<int>{2}), lockedDestination
        ? "native move enumeration offered a zero-distance step into a locked connected alias"
        : isolatedSource ? "native move enumeration could not escape an isolated source lacking the same-point flag"
                         : "native move enumeration offered a zero-distance step into an isolated alias");
    if (lockedDestination) {
        destination.nLocks = 0;
        Require(aliases() == vector<int>({2}), "unlocking a connected alias did not restore its zero-distance step");
        destination.nDynLocks = 1;
        Require(aliases() == vector<int>({2}) && destination.nDynLocks == 1,
            "a dynamic counter alone was treated as a hard lock or was changed by enumeration");
        destination.nLocks = 1;
        Require(aliases().empty() && destination.nLocks == 1 && destination.nDynLocks == 1,
            "an occupied alias was accepted or its lock counters were modified");
        destination.nLocks = 0; destination.nDynLocks = 0;
    }
    if (isolatedSource) {
        sourceTile.nMoveStand = 1;
        Require(aliases().empty(), "a connected source invented a same-point step without its declared flag");
        sourceTile.nMoveStand = 0;
    }
    isolated.nMoveCrouch = 1;
    Require(aliases() == vector<int>({1,2}), "move enumeration rejected a linked crouch alias");
    isolated.nPassable = NAI::CP_CROUCH;
    Require(aliases() == vector<int>({2}), "move enumeration ignored the current pose on a linked alias");
    ++destination.nHeight;
    Require(aliases().empty(), "move enumeration offered an alias at a different height");
}

static void IsolatedTransitionLinkMatch()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    for (int i = 0; i < 3; ++i)
        network->CreateLayer(4,4,VNULL2,0.f,i,i ? network->GetLayer(0)->pGroup.GetPtr() : nullptr);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 200;
    connected.nPassable = NAI::CP_STAND | NAI::CP_CROUCH;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE | NAI::TF_HAS_INTERGRID;
    const NAI::SPathPlace source(1,1,0), target(2,1,1), linkTarget(2,1,2);
    network->GetLayer(0)->tiles[1][1] = connected;
    network->GetLayer(0)->tiles[1][1].nHeight = 100;
    network->GetLayer(1)->tiles[1][2] = connected;
    auto& alias = network->GetLayer(2)->tiles[1][2];
    alias = connected;
    alias.nFlags = NAI::TF_STAND_PASSABLE;
    network->GetLayer(0)->transitions[source].links.emplace_back(linkTarget,10);
    network->GetLayer(1)->transitions[target];
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_INTERGRID,
        "connected transition link matching control failed");
    alias.nMoveStand = 0;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a recorded transition ending at an isolated copy matched a connected destination on another layer");
    alias.nMoveStand = 1;
    alias.nPassable = 0;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a recorded transition ending at an impassable copy matched a connected destination");
    alias = connected;
    ++alias.nHeight;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a recorded transition ending at a different height matched the destination");
}

static void IsolatedTransitionShortcut()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(4,4,VNULL2,0.f,0);
    network->CreateLayer(4,4,VNULL2,0.f,1,network->GetLayer(0)->pGroup);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 200;
    connected.nPassable = NAI::CP_STAND | NAI::CP_CROUCH;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE;
    NAI::SPathPlace source(1,1,0), target(2,1,1), intermediate(2,1,0);
    source.SetPose(NAI::CM_STAND); target.SetPose(NAI::CM_STAND);
    auto& start = network->GetLayer(0)->tiles[1][1];
    auto& alias = network->GetLayer(0)->tiles[1][2];
    start = alias = network->GetLayer(1)->tiles[1][2] = connected;
    start.nHeight = 100;
    start.nMoveStand = 1 << network->GetDir(source,intermediate);
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_MOVE,
        "connected cross-layer shortcut control failed");
    alias.nMoveStand = 0;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a cross-layer shortcut borrowed movement through an isolated intermediate copy");
    alias.nMoveCrouch = 1;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_MOVE,
        "a crouch-linked intermediate copy no longer supported its structural shortcut");
    ++alias.nHeight;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a cross-layer shortcut accepted an intermediate copy at a different height");
}

static void IsolatedTransitionEntry()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(4,4,VNULL2,0.f,0);
    network->CreateLayer(4,4,VNULL2,0.f,1,network->GetLayer(0)->pGroup);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 100;
    connected.nPassable = NAI::CP_STAND | NAI::CP_CROUCH;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE | NAI::TF_HAS_SAME;
    auto& sourceTile = network->GetLayer(0)->tiles[1][1];
    auto& targetTile = network->GetLayer(1)->tiles[1][1];
    sourceTile = targetTile = connected;
    NAI::SPathPlace source(1,1,0), target(1,1,1);
    source.SetPose(NAI::CM_STAND); target.SetPose(NAI::CM_STAND);
    Require(NAI::GetTransitionType(network,source,target) != NAI::TT_NO_WAY,
        "connected same-point transition control failed");
    targetTile.nMoveStand = 0;
    Require(NAI::GetTransitionType(network,target,source) != NAI::TT_NO_WAY,
        "an occupied isolated source could not escape to a connected same-point copy");
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a reverse same-point shortcut allowed entry into an isolated destination");
    targetTile.nMoveCrouch = 1;
    Require(NAI::GetTransitionType(network,source,target) != NAI::TT_NO_WAY,
        "a structurally connected crouch alias was rejected by transition classification");
    targetTile.nMoveCrouch = 0;
    targetTile.nFlags |= NAI::TF_HAS_INTERGRID;
    Require(NAI::GetTransitionType(network,source,target) != NAI::TT_NO_WAY,
        "an intergrid-connected alias was rejected by transition classification");
}

static void ConnectedDirectTransitions()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    for (int i = 0; i < 3; ++i)
        network->CreateLayer(4,4,VNULL2,0.f,i,i ? network->GetLayer(0)->pGroup.GetPtr() : nullptr);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 100;
    connected.nPassable = 7;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE;
    const NAI::SPathPlace source(1,1,1), destination(2,1,2);
    auto& lower = network->GetLayer(0)->tiles[1][1];
    lower = network->GetLayer(1)->tiles[1][1] = connected;
    network->GetLayer(1)->tiles[1][1].nFlags |= NAI::TF_HAS_SAME | NAI::TF_HAS_INTERGRID;
    auto& target = network->GetLayer(2)->tiles[1][2];
    target = connected;
    target.nHeight = 110;
    NAI::CNodesLayer::CTransitionsHash incoming;
    incoming[source].links.push_back(NAI::CNodesLayer::SLink(destination,10));
    auto install = [&]() {
        for (int i = 0; i < 3; ++i) network->GetLayer(i)->transitions.clear();
        network->AddDirectTransitions(incoming);
    };
    auto requireTransition = [&]() {
        const auto& forward = network->GetLayer(1)->transitions;
        const auto& reverse = network->GetLayer(2)->transitions;
        const auto f = forward.find(source), r = reverse.find(destination);
        Require(f != forward.end() && r != reverse.end() && f->second.links.size() == 1 &&
            r->second.links.size() == 1 && f->second.links.front().dst == destination &&
            r->second.links.front().dst == source && f->second.links.front().nHeight == 10 &&
            r->second.links.front().nHeight == 10,
            "an isolated lower copy suppressed a connected bidirectional navigation transition");
        Require(NAI::GetTransitionType(network,source,destination) == NAI::TT_INTERGRID &&
            NAI::GetTransitionType(network,destination,source) == NAI::TT_INTERGRID,
            "the retained direct transition could not be traversed in both directions");
    };
    lower.nMoveStand = 0;
    install();
    requireTransition();
    lower.nMoveStand = 1;
    install();
    Require(network->GetLayer(1)->transitions.empty() && network->GetLayer(2)->transitions.empty(),
        "a connected canonical lower copy no longer suppressed its duplicate transition");
    lower.nPassable = 0;
    install();
    requireTransition();
    lower = connected;
    ++lower.nDisplacement;
    install();
    requireTransition();
}

static void NearbyConnectedDestination()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(4,4,VNULL2,0.f,0);
    network->CreateLayer(4,4,VNULL2,0.f,1,network->GetLayer(0)->pGroup);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 100;
    connected.nPassable = 7;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE;
    auto* upper = network->GetLayer(1);
    auto& lowerTile = network->GetLayer(0)->tiles[1][1];
    auto& upperTile = upper->tiles[1][1];
    upperTile = lowerTile = connected;
    lowerTile.nMoveStand = 0;
    const NAI::SPathPlace destination(1,1,1);
    const SSphere sphere(network->GetCP(destination),0.05f);
    auto nearby = [&](bool all) {
        vector<NAI::SPathPlace> result;
        upper->AddNearPoints(sphere,&result,all);
        vector<int> layers;
        for (const auto& place: result) {
            Require(place.IsIntegral() && place.GetX() == 1 && place.GetY() == 1,
                "nearby fixture returned a point outside the requested sphere");
            layers.push_back(place.GetLayer());
        }
        return layers;
    };
    Require(nearby(true) == vector<int>({1}),
        "nearby fixture did not locate exactly its connected upper destination");
    Require(nearby(false) == vector<int>({1}),
        "nearby destination deduplication discarded a connected tile in favor of an isolated lower copy");
    lowerTile.nMoveStand = 1;
    Require(nearby(false).empty(), "nearby search did not retain canonical lower connected copies");
    lowerTile.nPassable = 1;
    Require(nearby(false) == vector<int>({1}),
        "a lower copy with fewer supported poses suppressed the better destination");
    lowerTile = connected;
    ++lowerTile.nHeight;
    Require(nearby(false) == vector<int>({1}),
        "a lower tile at a different height suppressed the destination");
    lowerTile = connected;
    lowerTile.nLocks = 1;
    lowerTile.nDynLocks = 1;
    Require(nearby(false).empty() && lowerTile.nLocks == 1 && lowerTile.nDynLocks == 1,
        "temporary occupancy changed structural deduplication or its lock counters");
}

static void FlyingShootDirection()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8,8,VNULL2,0.f,0);
    network->CreateLayer(8,8,VNULL2,FP_PI2,1);
    network->Freeze(true);
    for (int n = 0; n < 2; ++n)
        for (int y = 0; y < 8; ++y)
            for (int x = 0; x < 8; ++x) {
                network->GetLayer(n)->tiles[y][x] = NAI::STile{};
                network->GetLayer(n)->tiles[y][x].nPassable = 7;
                network->GetLayer(n)->tiles[y][x].nFlags = NAI::TF_STAND_PASSABLE;
            }
    NAI::SPathPlace ground(2,2,0), rotated(2,2,1);
    ground.SetPose(NAI::CM_STAND); rotated.SetPose(NAI::CM_CROUCH);
    Require(NRPG::GetShootDirection(network,ground,network->GetCP(ground)+CVec3(10,0,0)) == NAI::RIGHT,
        "shoot-direction control did not face east on the ordinary layer");
    Require(NRPG::GetShootDirection(network,rotated,network->GetCP(rotated)+CVec3(10,0,0)) == NAI::DOWN,
        "shoot-direction control ignored the rotated ground layer");
    NAI::SPosition flying;
    Require(NAI::MakeFlyPos(CVec3(2,2,16),network,&flying), "shoot-direction fixture could not encode an airborne position");
    Require(flying.p.IsFinal() && flying.p.GetLayer() >= network->GetNumLayers(),
        "shoot-direction fixture altitude must differ from a valid navigation layer");
    const auto before = flying.p.GetBits();
    std::printf("Shoot direction: final=%d altitude-bits=%d navigation-layers=%d\n",
        int(flying.p.IsFinal()),flying.p.GetLayer(),network->GetNumLayers());
    std::fflush(stdout);
    const CVec3 origin = flying.GetCP();
    Require(NRPG::GetShootDirection(network,flying.p,origin+CVec3(10,0,0)) == NAI::RIGHT,
        "an airborne shot did not face east using the base-layer axes");
    Require(NRPG::GetShootDirection(network,flying.p,origin+CVec3(0,10,0)) == NAI::UP,
        "an airborne shot did not face north using the base-layer axes");
    Require(NRPG::GetShootDirection(network,flying.p,origin+CVec3(-10,0,0)) == NAI::LEFT,
        "an airborne shot did not face west using the base-layer axes");
    Require(flying.p.GetBits() == before, "shoot-direction lookup changed the airborne position or pose");
}
}

bool RunNavigationTests(const char* mode)
{
    if (!std::strcmp(mode, "known-mine-path-reservations")) { KnownMinePathReservations(); return true; }
    if (!std::strcmp(mode, "nearest-invalid")) { InvalidNearestPosition(); return true; }
    if (!std::strcmp(mode, "precalc-sweep-bounds")) { PrecalcSweepBounds(); return true; }
    if (!std::strcmp(mode, "nearest-empty")) { EmptyNearestPosition(); return true; }
    if (!std::strcmp(mode, "fresh-navigation-colouring")) { FreshNavigationColouring(); return true; }
    if (!std::strcmp(mode, "invalid-region-colouring")) { InvalidRegionColouring(); return true; }
    if (!std::strcmp(mode, "fresh-navigation-transitions")) { FreshNavigationTransitions(); return true; }
    if (!std::strcmp(mode, "lazy-navigation-colourer")) { LazyNavigationColourer(); return true; }
    if (!std::strcmp(mode, "unsupported-navigation-floor")) { UnsupportedNavigationFloor(); return true; }
    if (!std::strcmp(mode, "connected-alias-moves")) { ConnectedAliasMoves(); return true; }
    if (!std::strcmp(mode, "isolated-source-moves")) { ConnectedAliasMoves(true); return true; }
    if (!std::strcmp(mode, "locked-alias-moves")) { ConnectedAliasMoves(false,true); return true; }
    if (!std::strcmp(mode, "isolated-source-path")) { IsolatedSourcePath(); return true; }
    if (!std::strcmp(mode, "prone-pose-change-facing")) { PronePoseChangeFacing(); return true; }
    if (!std::strcmp(mode, "simple-prone-pose-path")) { SimplePronePosePath(); return true; }
    if (!std::strcmp(mode, "isolated-transition-link-match")) { IsolatedTransitionLinkMatch(); return true; }
    if (!std::strcmp(mode, "isolated-transition-shortcut")) { IsolatedTransitionShortcut(); return true; }
    if (!std::strcmp(mode, "isolated-transition-entry")) { IsolatedTransitionEntry(); return true; }
    if (!std::strcmp(mode, "connected-direct-transitions")) { ConnectedDirectTransitions(); return true; }
    if (!std::strcmp(mode, "nearby-connected-destination")) { NearbyConnectedDestination(); return true; }
    if (!std::strcmp(mode, "flying-shoot-direction")) { FlyingShootDirection(); return true; }
    return false;
}
}
