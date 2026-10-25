#if defined(_WIN32)
#include "../Main/StdAfx.h"
#else
#include "../FileIO/StdAfx.h"
#include "../FileIO/BasicChunk1.h"
#include "../Misc/Geom.h"
#endif
#include "../Main/wMain.h"
#include "../Main/wBuilding.h"
#include "../Main/wUnitGroup.h"
#include "../Main/iCommonUI.h"
#include "../Main/PerkToolTip.h"
#include "../Main/UnitStepSound.h"
#include "../Main/InventoryUnit.h"
#include "../Main/UnitIconWeaponMode.h"
#include "../Main/iUnitIconBar.h"
#include "../Main/UIInterface.h"
#include "../Main/Console.h"
#include "../MiscDll/Commands.h"
#include "../DBFormat/DataInterface.h"
#include "../DBFormat/DataObject.h"
#include "../DBFormat/DataPerk.h"
#include "../Main/GView.h"
#include "../Main/iMission.h"
#include "../Main/iGameStates.h"
#include "../Input/BindInternal.h"
#include "../Main/iUnitPanel.h"
#include "../Main/iDesktopWindow.h"
#include "../Main/Sound.h"
#include "../FModSound/FMsound.h"
#include "../Main/iMissionDlgUI.h"
#include "../Main/iMissionUI.h"
#include "../Main/iMissionMovieUI.h"
#include "../Main/G2DView.h"
#include "../Main/UIML.h"
#include "../DBFormat/DataAck.h"
#include "../Main/wUnitCommands.h"
#include "../Main/wUICommands.h"
#include "../Main/wDialog.h"
#include "../Main/aiGrid.h"
#include "../Main/aiColourer.h"
#include "../Main/aiMap.h"
#include "../Main/aiStability.h"
#include "../Main/aiNearestPosition.h"
#include "../FileIO/PortableGameDatabase.h"
#include "../FileIO/PortableStructureChunks.h"
#include "../Main/aiLocker.h"
#include "../Main/wUnitServer.h"
#include "../Main/RPGItem.h"
#include "../Main/RPGItemSet.h"
#include "../Main/RPGCritical.h"
#include "../Main/wUnitAttackExec.h"
#include "../Main/wUnitStates.h"
#include "../Main/wUnitQueue.h"
#include "../Main/wUnitExec.h"
#include "../Main/wUnitMove.h"
#include "../Main/wMainPath.h"
#include "../Main/wGrenade.h"
#include "../Main/wDebris.h"
#include "../Main/wMisc.h"
#include "../Main/RPGAttackMech.h"
#include "../Main/aiPath.h"
#include "../Main/GAnimParticles.h"
#include "../Main/GAnimation.h"
#include "../Main/GDecalGeometry.h"
#include "../Main/GCombiner.h"
#include "../Main/GSceneUtils.h"
#include "../Main/GRenderCore.h"
#include "../Main/GAnimLight.h"
#include "../Main/GRenderLight.h"
#include "../Main/GLightmapStateWire.h"
#include "../Main/GInit.h"
#include "../Game/Platform.h"
#include "../Main/GGeometryPositionHash.h"
using GeometryTestHash = NGScene::SGeometryPosHash;
#include "../Main/rpgGlobal.h"
#include "../Main/RPGUnit.h"
#include "../Main/RPGUnitMission.h"
#include "../Main/rpgCheatConstants.h"
#include "../Main/aiCommander.h"
#include "../Main/aiRouteLogic.h"
#include "../Main/aiRoute.h"
#include "../Main/aiUnit.h"
#include "../Main/aiPlayer.h"
#include "../Main/aiReaction.h"
#include "../Main/aiEvent.h"
#include "../Main/aiThreatTracker.h"
#include "../Main/Sync.h"
#include "../Main/A5Script.h"
#include "../Main/scriptPtr.h"
#include "../Main/aiMisc.h"
#include "../Main/wOSBase.h"
#include "../Main/aiInterval.h"
#include "../Main/aiDoorCollider.h"
#include "../Main/MapBuild.h"
#include "../DBFormat/DataMap.h"
#include "../Main/scFlowChartItems.h"
#include "../Main/scFlowChart.h"
#include "../Main/scScenarioTracker.h"
#include "../Main/RPGGame.h"
#include "../Main/aiJob.h"
#include "../Main/iMissionExec.h"
#include "../Main/iMissionInternal.h"
#include "../Main/RPGStore.h"
#include "../Main/UnitTracker.h"
#include "../DBFormat/DataFormat.h"
#include "AIUnitFixture.h"
#include "LightingSceneFixture.h"
#include "GameViewFixture.h"
#include "../Main/Transform.h"
#include "../Main/aiInventory.h"
#include "../Main/aiWeapon.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <new>
#include <cstddef>
#include <cmath>
#include <limits>
#include <algorithm>
#include <type_traits>

extern CPtr<NScript::CScript> pScript;

REGISTER_SAVELOAD_CLASS_NM(0x7f550105, AIUnitFixture, NAI);

static void Require(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}



static bool optionalStringAccepted = false;

static int OptionalStringBridge(lua_State* state)
{
    Script script(state);
    vector<SLuaParams> params;
    optionalStringAccepted = false;
    if (!script.CheckArgs("ss[]", "OptionalStringBridge", &params)) return 0;
    optionalStringAccepted = true;
    script.PushString(params[1].s.c_str());
    return 1;
}

static void EmptyLuaDefault()
{
    Script script;
    script.Register("OptionalStringBridge", OptionalStringBridge);
    Require(script.DoString("result = OptionalStringBridge('first-visit-key')") == LUA_NOERR,
        "optional-string fixture did not compile");
    script.ExecuteThreads();
    Require(optionalStringAccepted, "Lua rejected an omitted optional string with an empty default");
    Require(script.GetGlobal("result").IsString() && std::string(script.GetGlobal("result").GetString()).empty(),
        "omitted optional Lua string did not receive its empty default");
    script.SetTop(0);
    script.DoString("result = OptionalStringBridge('key', 'explicit')");
    script.ExecuteThreads();
    Require(optionalStringAccepted && std::string(script.GetGlobal("result").GetString()) == "explicit",
        "empty-default handling changed an explicitly supplied Lua string");
    script.SetTop(0);
    script.DoString("result = OptionalStringBridge('key', '')");
    script.ExecuteThreads();
    Require(optionalStringAccepted && std::string(script.GetGlobal("result").GetString()).empty(),
        "empty-default handling changed an explicitly empty Lua string");
    script.SetTop(0);
    script.DoString("OptionalStringBridge()");
    script.ExecuteThreads();
    Require(!optionalStringAccepted, "empty-default handling accepted a missing required Lua argument");
}


DEFINE_DG_CONSTANT_NODE(AimerPoseProbe, NAnimation::SSkeletonPose);
DEFINE_DG_CONSTANT_NODE(AimerTimeProbe, STime);

static void SmartAimerBoundary()
{
    NAnimation::SSkeletonPose target(1), output(1);
    CObj<AimerPoseProbe> pose = new AimerPoseProbe(target);
    CObj<AimerTimeProbe> time = new AimerTimeProbe(200);
    CObj<NAnimation::CASmartAimer> aimer = new NAnimation::CASmartAimer;
    aimer->pNextPose = pose;
    aimer->pNextPoseTime = time;
    aimer->GetFrame(100, &output);
    Require(fabs2(output[0].pos) < 0.000001f, "smart-aimer fixture did not start at its initial pose");
    target[0].pos.x = 10.f;
    pose->Set(target);
    aimer->GetFrame(200, &output);
    Require(fabs2(output[0].pos - target[0].pos) < 0.000001f,
        "smart aimer did not return the exact target at the transition endpoint");
    // Retail returns the endpoint without reseeding the interpolator. The next
    // target must still blend from the last sampled interior pose/time.
    target[0].pos.x = 25.f;
    pose->Set(target);
    time->Set(300);
    aimer->GetFrame(250, &output);
    Require(std::fabs(output[0].pos.x - 18.75f) < 0.0001f,
        "exact animation endpoint reseeded smart-aimer interpolation for the next transition");
    aimer->GetFrame(301, &output);
    Require(fabs2(output[0].pos - target[0].pos) < 0.000001f,
        "smart aimer changed its completed-transition output");
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

static void FreshNavigationColouring()
{
    CObj<NAI::CNodesLayer> layer = new NAI::CNodesLayer;
    layer->nLayer = layer->nFloor = 0;
    layer->tiles.SetSizes(8, 8);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) layer->tiles[y][x] = NAI::STile{};
    CObj<NAI::CMapColourer> full = new NAI::CMapColourer;
    full->SetLayer(layer, nullptr, 0);
    full->ConstructColouring(nullptr);
    Require(full->IsReady() && full->GetPointColor(0, 0) != full->GetPointColor(7, 7),
        "fresh-colouring fixture did not build separate blocked regions");
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

class RagdollRestorationProbe: public NAnimation::CAParticle
{
    OBJECT_BASIC_METHODS(RagdollRestorationProbe);
public:
    RagdollRestorationProbe() { nParticleBones = {0, 1, 2}; }
    using CAParticle::RestoreLine;
    using CAParticle::RestoreTriangle;
};

static void RagdollRestoration()
{
    CObj<RagdollRestorationProbe> particle = new RagdollRestorationProbe;
    const CVec3 shift(7, 8, 9);
    for (bool triangle : {false, true}) {
        // Reversing the first edge exercises the second alignment; flipping
        // the plane additionally exercises the first triangle alignment.
        for (int shape = 0; shape < 5; ++shape) {
            NAnimation::SSkeletonPose pose(3);
            pose[0].pos = VNULL3;
            pose[1].pos = CVec3(1, 0, 0);
            pose[2].pos = CVec3(0, 1, 0);
            vector<CVec3> parts = {shift, shift + CVec3(-1, 0, 0), shift + CVec3(0, -1, 0)};
            if (shape == 1) parts[1] = shift + CVec3(1, 0, 0);
            if (shape == 2) parts[1] = shift; // collapsed destination edge
            if (shape == 3) pose[1].pos = VNULL3; // collapsed source edge
            if (shape == 4) parts[2] = parts[1]; // collapsed triangle plane
            const CVec3 oldEdge = pose[1].pos - pose[0].pos;
            const CVec3 oldOther = pose[2].pos - pose[0].pos;
            if (triangle) particle->RestoreTriangle(parts, 0, 1, 2, 0, &pose);
            else particle->RestoreLine(parts, 0, 1, 0, &pose);
            float rotation[4];
            pose[0].rot.GetComponentsForWire(rotation);
            for (float value : rotation)
                Require(std::isfinite(value), "degenerate ragdoll restoration produced a non-finite rotation");
            Require(fabs2(pose[0].pos - shift) < 1e-8f,
                "ragdoll restoration lost the particle translation");
            if (shape < 2) {
                Require(fabs2(pose[0].rot.Rotate(oldEdge) - (parts[1] - parts[0])) < 1e-8f,
                    "half-turn ragdoll restoration failed to align the bone");
                if (triangle)
                    Require(fabs2(pose[0].rot.Rotate(oldOther) - (parts[2] - parts[0])) < 1e-8f,
                        "half-turn ragdoll restoration failed to align the triangle plane");
            }
        }
    }
}

static void PartialReferenceImport(const char* databasePath)
{
    CFileStream source;
    source.OpenRead(databasePath);
    vector<unsigned char> bytes(source.GetSize());
    source.Read(bytes.data(), static_cast<unsigned int>(bytes.size()));
    auto load = [](const vector<unsigned char>& data, bool overlay) {
        CMemoryStream stream;
        stream.Write(data.data(), static_cast<unsigned int>(data.size()));
        stream.Seek(0);
        NDatabase::Serialize(stream, CStructureSaver::READ, overlay);
    };
    load(bytes, false);
    auto* map = NDb::GetGlobalMap(3);
    Require(map && IsValid(map->pStartZone), "fixture intro zone reference is missing");
    CPtr<NDb::CDBScenarioZone> previous = map->pStartZone;
    vector<unsigned char> partial = bytes;
    const string column = "StartZoneID", absent = "AbsentZoneX";
    int replaced = 0;
    for (auto it = partial.begin();;) {
        it = std::search(it, partial.end(), column.begin(), column.end());
        if (it == partial.end()) break;
        std::copy(absent.begin(), absent.end(), it);
        it += column.size(); ++replaced;
    }
    Require(replaced == 2, "fixture reference column is not uniquely described");
    load(partial, true);
    Require(NDb::GetGlobalMap(3) == map && map->pStartZone == previous,
        "partial database import cleared a reference whose column was absent");

    // A present reference must still be imported, including an explicit null.
    S2FileIO::PortableGameDatabase database;
    string error;
    Require(S2FileIO::LoadPortableGameDatabaseBytes(bytes.data(), bytes.size(), &database, &error),
        "fixture database decoder failed");
    bool changed = false;
    for (const auto& table : database.tables) {
        if (table.tableId != 16) continue;
        const auto id = std::find(table.intNames.begin(), table.intNames.end(), "ID") - table.intNames.begin();
        const auto ref = std::find(table.intNames.begin(), table.intNames.end(), column) - table.intNames.begin();
        Require(id < table.intNames.size() && ref < table.intNames.size(), "fixture reference indices are missing");
        for (const auto& row : table.intRows) {
            if (row[id] != 3) continue;
            vector<unsigned char> pattern;
            for (int value : row)
                for (unsigned shift = 0; shift < 32; shift += 8)
                    pattern.push_back(static_cast<unsigned char>(static_cast<unsigned>(value) >> shift));
            auto it = std::search(bytes.begin(), bytes.end(), pattern.begin(), pattern.end());
            Require(it != bytes.end() && std::search(it + 1, bytes.end(), pattern.begin(), pattern.end()) == bytes.end(),
                "fixture database row bytes are not unique");
            std::fill(it + 4 * ref, it + 4 * ref + 4, 0);
            changed = true;
        }
    }
    Require(changed, "fixture reference row was not patched");
    load(bytes, true);
    Require(!IsValid(map->pStartZone), "explicit null reference was ignored during partial database import");
}


static void PermanentClueSingleMap()
{
    CObj<NDb::CDBScenarioZone> record = new NDb::CDBScenarioZone;
    record->templatesIDs = {710, 0, 0, 0};
    record->nItemSlots = 1;
    record->nPersonSlots = 0;
    record->nCluesMaxNumber = 1;
    CObj<NScenario::CScenarioZone> zone = new NScenario::CScenarioZone(record, 1);
    unordered_map<int, NScenario::CScenarioZone::STemplate> slots;
    slots[710].nItemSlots = slots[710].nEmptyItemSlots = 1;
    CMemoryStream state;
    { CStructureSaver writer(state, CStructureSaver::WRITE); writer.Add(13, &slots); }
    state.Seek(0);
    { CStructureSaver reader(state, CStructureSaver::READ); static_cast<CObjectBase*>(zone.GetPtr())->operator&(reader); }
    CObj<NDb::CDBScenarioClue> ordinaryRecord = new NDb::CDBScenarioClue;
    ordinaryRecord->clueType = NDb::CT_ITEM;
    ordinaryRecord->bPermanent = false;
    CObj<NDb::CDBScenarioClue> permanentRecord = new NDb::CDBScenarioClue;
    permanentRecord->clueType = NDb::CT_ITEM;
    permanentRecord->bPermanent = true;
    CObj<NScenario::CScenarioClue> ordinary = new NScenario::CScenarioClue(ordinaryRecord, 1);
    CObj<NScenario::CScenarioClue> permanent = new NScenario::CScenarioClue(permanentRecord, 2);
    Require(zone->CanPlaceClue(NDb::CT_ITEM), "fixture has no ordinary clue capacity");
    zone->PlaceClue(ordinary);
    Require(ordinary->GetTemplateID() == 710, "ordinary clue lost its map");
    Require(!zone->CanPlaceClue(NDb::CT_ITEM), "ordinary clue admission exceeded its retail limit");
    zone->PlaceClue(permanent);
    Require(permanent->GetTemplateID() == 710,
        "permanent clue lost its single map after ordinary clue capacity was consumed");
    Require(!zone->CanPlaceClue(NDb::CT_ITEM), "permanent clue changed ordinary clue admission limits");
    Require(zone->GetClues().size() == 2 && permanent->GetParentZones().front() == zone.GetPtr(),
        "permanent clue was detached from its scenario zone");
}

class DeferredClickTarget: public CObjectBase, public NRPG::IAttackable
{
    OBJECT_NOCOPY_METHODS(DeferredClickTarget);
public:
    NWorld::IWorld* expectedWorld = nullptr;
    int calls = 0;
    int ProcessAttack(NWorld::IWorld* world, int user, NRPG::CAttackPortion* attack,
        const CVec3& direction, NDb::CRPGArmor*) override {
        ++calls;
        Require(world == expectedWorld, "deferred ClickOfDeath damage lost its creating world");
        Require(user == 42 && attack->atkType == NRPG::AT_CLICK_OF_DEATH &&
                attack->nDmgMin == 10000000 && attack->nDmgMax == 10000001,
            "deferred ClickOfDeath changed its original damage payload");
        Require(direction == CVec3(0, 1, 0), "deferred ClickOfDeath changed its trajectory");
        return 3;
    }
};

template<class Factory>
static NWorld::IDynamicObject* CreateDeferredClick(Factory factory, NWorld::IWorld* world,
    NWorld::CActionCounter* action, CObjectBase* target, const CRay& ray)
{
    // Exercise the pre-port factory too, before it accepts the missing world.
    if constexpr (std::is_invocable_v<Factory, NWorld::IWorld*, NWorld::CActionCounter*, CObjectBase*, int, const CRay&>)
        return factory(world, action, target, 42, ray);
    else
        return factory(action, target, 42, ray);
}

static void DeferredClickWorld()
{
    CObj<NWorld::CWorld> world = new NWorld::CWorld;
    CObj<DeferredClickTarget> target = new DeferredClickTarget;
    target->expectedWorld = world;
    CRay ray;
    ray.ptOrigin = CVec3(1, 2, 3);
    ray.ptDir = CVec3(0, 1, 0);
    CObj<NWorld::IDynamicObject> click = CreateDeferredClick(&NWorld::CreateClickOfDeath,
        world, world->GetActiveCounter(), target, ray);
    Require(world->IsAction(), "deferred ClickOfDeath did not hold its active action counter");
    Require(click->Segment() && target->calls == 1, "deferred ClickOfDeath did not complete one target callback");
    click = nullptr;
    Require(!world->IsAction(), "completed ClickOfDeath retained its action counter");
    click = CreateDeferredClick(&NWorld::CreateClickOfDeath, world, world->GetActiveCounter(), nullptr, ray);
    Require(click->Segment() && target->calls == 1, "targetless ClickOfDeath invoked a damage callback");
    click = nullptr;
    Require(!world->IsAction(), "targetless ClickOfDeath retained its action counter");
}

static void InventoryWithoutUniform()
{
    CObj<NRPG::CUnit> owner = new NRPG::CUnit;
    CObj<NRPG::IInventory> inventory = NRPG::CreateInventory(owner);
    CObj<NDb::CRPGUniform> uniform = new NDb::CRPGUniform;
    for (auto& slot: uniform->subTypes)
        for (auto& priority: slot.priorities) priority = NDb::SUBTYPE_NONE;
    uniform->subTypes[1].priorities[1] = NDb::SUBTYPE_KNIFE;
    uniform->subTypes[2].priorities[0] = NDb::SUBTYPE_KNIFE;
    owner->pUniform = uniform;
    Require(inventory->GetPlaceBySubType(NDb::SUBTYPE_KNIFE) == 2,
        "uniform placement control did not preserve preferred-subtype-only matching");
    owner->pUniform = nullptr;
    Require(inventory->GetUniform() == nullptr, "uniform-less fixture unexpectedly retained a uniform");
    Require(inventory->GetPlaceBySubType(NDb::SUBTYPE_KNIFE) == -1,
        "uniform-less NPC inventory invented a belt placement");
    CObj<NRPG::IInventory> ownerless = NRPG::CreateInventory(nullptr);
    Require(ownerless->GetPlaceBySubType(NDb::SUBTYPE_HEAVY) == -1,
        "ownerless inventory invented a belt placement");
    owner->pUniform = uniform;
    Require(inventory->GetPlaceBySubType(NDb::SUBTYPE_KNIFE) == 2,
        "restoring a uniform did not restore its original belt placement");
}

static void DynamicAmbientFallback()
{
    Require(S2Platform::Window() == nullptr, "ambient fallback fixture unexpectedly owns a display window");
    // The real mode setter takes its documented failure fallback without a
    // platform window; no GPU or display is initialized by this test.
    NGlobal::SetVar("gfx_fullscreen", 0);
    Require(!NGScene::SetModeFromConfig(false) && NGScene::GetCLSkyTexturesNumber() == 0,
        "ambient fixture did not enter the real no-sky-texture fallback");
    Require(S2Platform::Window() == nullptr, "ambient fallback fixture created a display window");
    CObj<NGScene::LightingSceneFixture> scene = new NGScene::LightingSceneFixture;
    NGScene::CLightState light;
    light.vAmbientColor = CVec3(0.3f, 0.4f, 0.5f);
    light.vUpDifColor = CVec3(0.1f, 0.2f, 0.3f);
    light.skyDirections.emplace_back(0, 0, -1);
    NGScene::SDynamicAmbientInfo result;
    light.TraceDynamicLM(&result, SSphere(VNULL3, 0.1f), scene);
    Require(fabs2(result.vXPos.v - light.vAmbientColor) < 1e-10f
            && fabs2(result.vXNeg.v - light.vAmbientColor) < 1e-10f
            && fabs2(result.vYPos.v - light.vAmbientColor) < 1e-10f
            && fabs2(result.vYNeg.v - light.vAmbientColor) < 1e-10f
            && fabs2(result.vZPos.v - (light.vAmbientColor + light.vUpDifColor)) < 1e-10f
            && fabs2(result.vZNeg.v - (light.vAmbientColor - light.vUpDifColor)) < 1e-10f,
        "dynamic objects lost the full ambient fallback when sky textures were unavailable");
    Require(scene->traceCalls == 0, "no-sky-texture fallback still traced unavailable sky occlusion");
    light.skyDirections.clear();
    light.TraceDynamicLM(&result, SSphere(VNULL3, 0.1f), scene);
    Require(fabs2(result.vXPos.v) + fabs2(result.vXNeg.v) + fabs2(result.vYPos.v)
            + fabs2(result.vYNeg.v) + fabs2(result.vZPos.v) + fabs2(result.vZNeg.v) == 0,
        "ambient fallback injected sky light into a cache without sky samples");
}

static void PointLightWithoutScene()
{
    NGScene::CLightState light;
    light.points.emplace_back(CVec3(1, 2, 3), VNULL3, 12.f);
    NGScene::SDynamicAmbientInfo result;
    light.TraceDynamicLM(&result, SSphere(CVec3(1, 0, 0), 0.1f), nullptr);
    // Original falloff at distance 1 with radius 12 is 1/(1+6/12)^2.
    Require(fabs2(result.vXNeg.v - CVec3(4.f/9.f, 8.f/9.f, 12.f/9.f)) < 1e-10f,
        "unshadowed point light changed its original colour or distance attenuation");
    Require(fabs2(result.vXPos.v) + fabs2(result.vYPos.v) + fabs2(result.vYNeg.v)
            + fabs2(result.vZPos.v) + fabs2(result.vZNeg.v) == 0,
        "unshadowed point light illuminated the wrong receiver direction");
    light.TraceDynamicLM(&result, SSphere(CVec3(20, 0, 0), 0.1f), nullptr);
    Require(fabs2(result.vXNeg.v) == 0, "out-of-radius point light contributed to the receiver");
    light.points.clear();
    light.semiPoints.emplace_back(CVec3(1, 2, 3), VNULL3, CVec3(1, 0, 0), 12.f);
    light.TraceDynamicLM(&result, SSphere(CVec3(-1, 0, 0), 0.1f), nullptr);
    Require(fabs2(result.vXPos.v) == 0, "semi-point light illuminated its excluded hemisphere");
    light.TraceDynamicLM(&result, SSphere(CVec3(1, 0, 0), 0.1f), nullptr);
    Require(fabs2(result.vXNeg.v - CVec3(4.f/9.f, 8.f/9.f, 12.f/9.f)) < 1e-10f,
        "unshadowed semi-point light lost its allowed hemisphere");
}

struct RetailPointLightFixture
{
    float color[3], center[3], radius;
    bool shadow;
    unsigned char padding[3];
};
static_assert(sizeof(RetailPointLightFixture) == 32, "retail cached point-light record size");

static void PointLightSaveLayout(bool legacy = false)
{
    vector<RetailPointLightFixture> original(2);
    std::memset(original.data(), 0, original.size() * sizeof(original[0]));
    original[0].color[0] = 0.25f; original[0].color[1] = 0.5f; original[0].color[2] = 0.75f;
    original[0].center[0] = -3.f; original[0].center[1] = 7.f; original[0].center[2] = 1.f;
    original[0].radius = 12.f; original[0].shadow = false;
    original[1].color[0] = 1.f; original[1].color[1] = 0.125f; original[1].color[2] = 0.625f;
    original[1].center[0] = 9.f; original[1].center[1] = -8.f; original[1].center[2] = 2.f;
    original[1].radius = 19.f; original[1].shadow = true;
    CMemoryStream saved;
    if (legacy) {
        struct OldPoint { float values[7]; };
        vector<OldPoint> old(2);
        for (int i = 0; i < 2; ++i) std::memcpy(old[i].values, &original[i], 28);
        { CStructureSaver writer(saved, CStructureSaver::WRITE); writer.Add(4, &old); }
        original[0].shadow = original[1].shadow = true; // Previous remaster always traced shadows.
    } else {
        CStructureSaver writer(saved, CStructureSaver::WRITE); writer.Add(4, &original);
    }
    saved.Seek(0);
    NGScene::CLightState light;
    light.vAmbientColor = light.vUpDifColor = VNULL3;
    { CStructureSaver reader(saved, CStructureSaver::READ); light.operator&(reader); }
    Require(light.points.size() == 2, "retail save lost cached point-light records");
    Require(light.points[1].vCenter == CVec3(9, -8, 2) && light.points[1].fRadius == 19.f,
        "retail 32-byte point-light records were loaded with the wrong array stride");
    CMemoryStream resaved;
    { CStructureSaver writer(resaved, CStructureSaver::WRITE); light.operator&(writer); }
    resaved.Seek(0);
    vector<RetailPointLightFixture> roundtrip;
    { CStructureSaver reader(resaved, CStructureSaver::READ); reader.Add(4, &roundtrip); }
    Require(roundtrip.size() == original.size(), "point-light resave changed the record count");
    for (int i = 0; i < 2; ++i) {
        Require(roundtrip[i].shadow == original[i].shadow && roundtrip[i].radius == original[i].radius,
            "point-light resave lost the original shadow flag or radius");
        for (int axis = 0; axis < 3; ++axis)
            Require(roundtrip[i].center[axis] == original[i].center[axis] && roundtrip[i].color[axis] == original[i].color[axis],
                "point-light resave changed saved position or colour");
    }
}

static void PointLightShadowFlag()
{
    CObj<NGScene::LightingSceneFixture> scene = new NGScene::LightingSceneFixture;
    NGScene::CLightState light;
    light.points.emplace_back(CVec3(1, 2, 3), VNULL3, 12.f, false);
    const SSphere receiver(CVec3(1, 0, 0), 0.1f);
    NGScene::SDynamicAmbientInfo result;
    light.TraceDynamicLM(&result, receiver, scene);
    Require(scene->traceCalls == 0 && result.vXNeg.v.x > 0.f,
        "point light with shadows disabled still performed an occlusion test");
    light.points[0].bCastShadow = true;
    light.TraceDynamicLM(&result, receiver, scene);
    Require(scene->traceCalls == 1 && fabs2(result.vXNeg.v) == 0,
        "shadow-casting point light ignored an intervening occluder");
    scene->blocked = false;
    light.TraceDynamicLM(&result, receiver, scene);
    Require(scene->traceCalls == 2 && fabs2(result.vXNeg.v - CVec3(4.f/9.f, 8.f/9.f, 12.f/9.f)) < 1e-10f,
        "visible shadow-casting point light changed its original attenuation");
}

static void ConsoleVisibilityToggle()
{
    CObj<NUI::CInterface> ui = new NUI::CInterface;
    CObj<NUI::CConsole> console = new NUI::CConsole(NUI::SWindowInfo(nullptr,
        NUI::SPoint(0, 0), NUI::SPoint(1024, 300), "console", NUI::STYLE_ENABLED));
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[console.GetPtr()] = 1; references.refs[1] = console.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CMemoryStream controls;
    { CStructureSaver writer(controls, CStructureSaver::WRITE, &references); writer.Add(11, &console); }
    controls.Seek(0);
    { CStructureSaver reader(controls, CStructureSaver::READ, &references); static_cast<CObjectBase*>(ui.GetPtr())->operator&(reader); }
    auto toggle = [&] {
        NInput::PostEvent("console");
        NInput::SEvent event;
        Require(NInput::GetEvent(&event), "console fixture did not receive its posted toggle");
        Require(ui->ProcessEvent(event), "console toggle was not consumed by its interface");
    };
    toggle();
    Require(!console->GetStyle(NUI::STYLE_VISIBLE), "locked console became visible without unlocking");
    NGlobal::ProcessCommand(L"wirbelwind");
    toggle();
    Require(console->GetStyle(NUI::STYLE_VISIBLE), "unlocked console did not open");
    toggle();
    Require(!console->GetStyle(NUI::STYLE_VISIBLE),
        "console close retained visible sliding state until a later render");
    toggle();
    Require(console->GetStyle(NUI::STYLE_VISIBLE) && console->GetPosition().y == 0,
        "reopened console remained hidden or inherited an offscreen sliding position");
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
    CObj<NGScene::CPointLight> light = new NGScene::CPointLight(CVec3(4, 2, 1), CVec3(3, -2, 5), 3.f, nullptr, true);
    NGScene::CPointLight::SRadianceInfo info;
    light->GetRadianceInfo(&info, NGScene::RP_GF3_CL);
    Require(info.vColor == CVec3(1, 0.5f, 0.25f) && info.fRadius == 6.f,
        "point-light radiance failed to fold colour strength into its cached radius");
    Require(info.vCenter == CVec3(3, -2, 5) && !info.bIsRendered,
        "radiance conversion changed light position or lightmap-only eligibility");
    CObj<NGScene::CPointLight> dim = new NGScene::CPointLight(CVec3(0.25f, 0.125f, 0), VNULL3, 8.f, nullptr, true);
    dim->GetRadianceInfo(&info, NGScene::RP_GF3_CL);
    Require(info.vColor == CVec3(1, 0.5f, 0) && info.fRadius == 4.f,
        "dim point-light radiance used the wrong radius scaling");
    CObj<NGScene::CPointLight> dark = new NGScene::CPointLight(VNULL3, VNULL3, 3.f, nullptr, true);
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

static CObj<NWorld::CWorld> FixtureWorld(const char* databasePath, const char* resourcePath,
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
    world->CreateRandom(810, {}, true, {}, 0, &post, SRandomSeed(123));
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

static bool SavedWorldUIRefresh(NWorld::CWorld* world)
{
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE); static_cast<CObjectBase*>(world)->operator&(writer); }
    const auto* bytes = static_cast<const std::uint8_t*>(saved.GetBuffer());
    const auto size = static_cast<std::size_t>(saved.GetSize());
    S2FileIO::StructureChunk root;
    bool foundRoot = false;
    for (std::size_t offset = 0; offset < size;) {
        S2FileIO::StructureChunk chunk;
        Require(S2FileIO::DecodeStructureChunkAt(bytes, size, offset, &chunk), "invalid world save chunk");
        if (chunk.id == 1) { root = chunk; foundRoot = true; break; }
        offset = static_cast<std::size_t>(chunk.payloadOffset + chunk.length);
    }
    Require(foundRoot, "world save has no root payload");
    const auto* fields = bytes + root.payloadOffset;
    for (std::size_t offset = 0; offset < root.length;) {
        S2FileIO::StructureChunk chunk;
        Require(S2FileIO::DecodeStructureChunkAt(fields, root.length, offset, &chunk), "invalid world save field");
        if (chunk.id == 47) {
            Require(chunk.length == 1, "world UI-refresh latch has an unexpected wire size");
            return fields[chunk.payloadOffset] != 0;
        }
        offset = static_cast<std::size_t>(chunk.payloadOffset + chunk.length);
    }
    return false;
}

static void WorldUIRefreshLatch(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    Require(SavedWorldUIRefresh(world), "fresh world did not preserve its pending UI refresh in save tag 47");
    NWorld::IWorld* api = world.GetPtr();
    Require(api->IsUINeedUpdate() && !api->IsUINeedUpdate(), "fresh-world UI refresh was not consumed exactly once");
    Require(!SavedWorldUIRefresh(world), "consumed UI refresh was still armed in an ordinary save");
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& unit: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (server && server->CanFight() && !server->IsEmptyPK()) { actor = server; break; }
    }
    Require(actor != nullptr, "UI-refresh fixture has no living actor");
    actor->Hide(false, false);
    api->IsUINeedUpdate();
    actor->EnableHide();
    Require(api->IsUINeedUpdate() && !api->IsUINeedUpdate(), "re-arming Hide did not request exactly one UI refresh");
    actor->EnableHide();
    Require(!api->IsUINeedUpdate(), "already enabled Hide requested an unnecessary UI refresh");
    auto networkBytes = [&]() {
        CStructureNetworkContext context;
        context.include = [](CObjectBase*) { return false; };
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &context);
          static_cast<CObjectBase*>(world.GetPtr())->operator&(writer); }
        const auto* bytes = static_cast<const std::uint8_t*>(saved.GetBuffer());
        return vector<std::uint8_t>(bytes, bytes + saved.GetSize());
    };
    const auto cleanNetwork = networkBytes();
    api->UINeedUpdate();
    Require(SavedWorldUIRefresh(world), "explicit UI-refresh request was missing from an ordinary save");
    Require(networkBytes() == cleanNetwork, "local UI refresh changed the synchronized world payload");
    Require(api->IsUINeedUpdate() && !api->IsUINeedUpdate(), "explicit UI-refresh request was not consumed exactly once");
    for (bool pending: {true, false}) {
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE); writer.Add(47, &pending); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ);
          static_cast<CObjectBase*>(world.GetPtr())->operator&(reader); }
        Require(api->IsUINeedUpdate() == pending && !api->IsUINeedUpdate(),
            "ordinary save restore lost the pending/consumed UI-refresh state");
    }

}

class PainSoundVisitor: public NWorld::ISoundVisitor
{
public:
    vector<NDb::CSound*> sounds;
    void Add3DSound(STime, NDb::CSound* sound, CFuncBase<CVec3>*) override { sounds.push_back(sound); }
    void AddEffect(STime, NDb::CSoundEffect*, CFuncBase<CVec3>*, const vector<int>&) override {
        throw std::runtime_error("unexpected effect in pain-sound fixture");
    }
    void Add2DSound(STime, NDb::CSound*) override { throw std::runtime_error("unexpected 2D pain sound"); }
};

static vector<NDb::CSound*> UnitAttachedSounds(NWorld::CUnitServer* actor)
{
    vector<CPtr<NWorld::C3DSound>> attached;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* object) {
        if (auto* sound = dynamic_cast<NWorld::C3DSound*>(object)) attached.push_back(sound);
        return false;
    };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
      actor->NWorld::CDumbUnitServer::operator&(writer); }
    PainSoundVisitor visitor;
    for (auto& sound: attached) sound->Visit(&visitor);
    return visitor.sounds;
}

static void PainVoiceOwner(const char* databasePath, const char* resourcePath, bool unconscious = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (server && server->CanFight() && !server->IsEmptyPK() && !server->IsWearingPK()
                && server->IsAIUnit()) { actor = server; break; }
    }
    Require(actor != nullptr, "pain-voice fixture has no ordinary conscious NPC");
    CDBIterator<NDb::CSound> rows(*NDatabase::GetTable<NDb::CSound>());
    Require(rows.MoveNext(), "pain-voice fixture has no first sound");
    NDb::CSound* ownSound = rows.Get();
    Require(rows.MoveNext(), "pain-voice fixture has no donor sound");
    NDb::CSound* donorSound = rows.Get();
    CObj<NDb::CTSound> ownTemplate = new NDb::CTSound, donorTemplate = new NDb::CTSound;
    CObj<NDb::CSoundVariant> ownVariant = new NDb::CSoundVariant, donorVariant = new NDb::CSoundVariant;
    ownVariant->pSound = ownSound; donorVariant->pSound = donorSound;
    ownTemplate->variants.push_back(ownVariant.GetPtr()); ownTemplate->roulette.AddSector(1);
    donorTemplate->variants.push_back(donorVariant.GetPtr()); donorTemplate->roulette.AddSector(1);
    CObj<NDb::CRPGPers> donor = new NDb::CRPGPers;
    donor->pSoundHit = donorTemplate.GetPtr();
    auto* persona = actor->GetUnitRPG()->GetRPGPers();
    persona->pSoundHit = ownTemplate.GetPtr();
    persona->pAcksHolder = donor.GetPtr();
    Require(actor->GetUnitRPG()->GetRPGUnit()->GetAckHolder() == donor.GetPtr(),
        "pain-voice fixture did not install a different acknowledgement owner");
    auto before = UnitAttachedSounds(actor);
    const int vp = actor->GetUnitRPG()->GetTotalVP();
    auto strike = [&]() {
        NRPG::CAttackPortion attack(0, 2, 0, 0, 0, 0, 0, nullptr, actor->GetUnitRPG());
        attack.rTtrajectory.ptOrigin = actor->GetPosition().GetCP();
        attack.rTtrajectory.ptDir = CVec3(0, 1, 0);
        Require(actor->ProcessAttack(world, NAI::HL_BODY, &attack, CVec3(0, 1, 0), nullptr) == 0,
            "pain-voice fixture unexpectedly dealt damage");
    };
    strike();
    auto sounds = UnitAttachedSounds(actor);
    Require(sounds.size() == before.size() + 1 && sounds.back() == donorSound,
        "native hit processing used the persona voice instead of its acknowledgement owner");
    persona->pAcksHolder = nullptr;
    strike();
    sounds = UnitAttachedSounds(actor);
    Require(sounds.size() == before.size() + 2 && sounds.back() == ownSound,
        "missing acknowledgement owner did not fall back to the persona pain voice");
    persona->pSoundHit = nullptr;
    strike();
    Require(UnitAttachedSounds(actor).size() == sounds.size(), "missing pain voice created a spurious sound");
    Require(actor->CanFight() && actor->GetUnitRPG()->GetTotalVP() == vp,
        "pain-voice selection changed combat health or consciousness");
    if (unconscious) {
        persona->pSoundHit = ownTemplate.GetPtr();
        actor->MakeUnconscious(VNULL3, true, false);
        Require(actor->IsUnconscious() && !actor->IsDead() && !actor->CanFight(),
            "pain-eligibility fixture did not enter the native unconscious state");
        const auto quiet = UnitAttachedSounds(actor);
        strike();
        Require(UnitAttachedSounds(actor).size() == quiet.size(),
            "an already unconscious actor emitted a new pain voice on a hit");
        Require(actor->GetUnitRPG()->GetTotalVP() == vp && actor->IsUnconscious(),
            "pain eligibility changed health or the native unconscious state");
    }

}

class VisibleIconPlayer: public NWorld::CPlayer
{
    OBJECT_NOCOPY_METHODS(VisibleIconPlayer);
public:
    void See(NWorld::CUnitServer* actor) { NWorld::CPlayerBaseVision<NWorld::CUnitServer>::visible.push_back(actor); }
    int GetScenarioPlayerID() const override { return 0; }
};

class EnemyIconMission: public ReadinessMission
{
    OBJECT_NOCOPY_METHODS(EnemyIconMission);
public:
    CObj<NGScene::IGameView> view;
    CTransformStack transform;
    EnemyIconMission() { transform.Init(); }
    NGScene::IGameView* GetScene() const override { return view; }
    const CTransformStack& GetCameraTransform() const override { return transform; }
};

static void EnemyIconVisibility(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    CObj<VisibleIconPlayer> visiblePlayer = new VisibleIconPlayer;
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (server && server->CanFight() && !server->IsClueUnit() && !server->IsEmptyPK()
                && world->GetDiplomacyState(visiblePlayer.GetPtr(), server->GetPlayer()) == NDb::DS_ENEMY) {
            actor = server; break;
        }
    }
    Require(actor != nullptr, "enemy-icon fixture has no visible hostile actor");
    visiblePlayer->See(actor);
    CObj<NGame::IPlayerTracker> tracker = dynamic_cast<NGame::IPlayerTracker*>(pSSClasses->CreateObject(int(0xB2022800)));
    CObj<EnemyIconMission> mission = new EnemyIconMission;
    mission->view = new NGScene::GameViewFixture;
    CObj<MissionIconProbe> desktop = new MissionIconProbe;
    mission->Attach(world, desktop); mission->AttachActive(tracker);
    CObj<NUI::CWindow> client = new NUI::CWindow(NUI::SWindowInfo(nullptr,
        NUI::SPoint(0, 0), NUI::SPoint(1024, 768), "view", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE));
    CObj<NUI::CText> pause = new NUI::CText;
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.include = [](CObjectBase*) { return false; };
    auto retain = [&](CObjectBase* object, unsigned id) { references.ids[object] = id; references.refs[id] = object; };
    retain(mission.GetPtr(), 1); retain(visiblePlayer.GetPtr(), 2); retain(client.GetPtr(), 3); retain(pause.GetPtr(), 4);
    CPtr<NWorld::IPlayer> worldPlayer = visiblePlayer.GetPtr();
    CMemoryStream playerSaved;
    { CStructureSaver writer(playerSaved, CStructureSaver::WRITE, &references); writer.Add(5, &worldPlayer); }
    playerSaved.Seek(0);
    { CStructureSaver reader(playerSaved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(tracker.GetPtr())->operator&(reader); }
    struct DesktopClientWire {
        CObj<NUI::CWindow> client;
        int operator&(CStructureSaver& saver) { saver.Add(2, &client); return 0; }
    } base;
    base.client = client.GetPtr();
    CPtr<NGame::IMission> missionRef = mission.GetPtr();
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
      writer.Add(1, &base); writer.Add(2, &missionRef); writer.Add(27, &pause); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(desktop.GetPtr())->operator&(reader); }
    auto markerCount = [&](int typeID) {
        int count = 0;
        CStructureNetworkContext graph;
        graph.include = [&](CObjectBase* object) {
            if (pSSClasses->GetObjectTypeID(object) == typeID) ++count;
            return false;
        };
        CMemoryStream traversal;
        { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
          static_cast<CObjectBase*>(desktop.GetPtr())->operator&(writer); }
        return count;
    };
    NGlobal::SetVar("ui_showicons", 1);
    desktop->RefreshEnemies();
    Require(markerCount(int(0xB0241948)) == 1, "visible hostile actor did not receive its normal enemy icon");
    NGlobal::SetVar("ui_showicons", 0);
    desktop->RefreshEnemies();
    Require(markerCount(int(0xB0241948)) == 0, "disabled enemy icons were rebuilt for a visible hostile actor");
    NGlobal::SetVar("ui_showicons", 1);
    desktop->RefreshEnemies();
    Require(markerCount(int(0xB0241948)) == 1, "re-enabling enemy icons did not restore the visible hostile marker");
    actor->nClueCount = 1;
    NGlobal::SetVar("ui_showicons", 0);
    desktop->RefreshEnemies();
    Require(markerCount(int(0xB0241948)) == 0 && markerCount(int(0xB3123180)) == 1,
        "enemy-icon visibility suppressed the independently visible quest-clue marker");

}

static void FinishedMusicSilence(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    CDBIterator<NDb::CMusic> rows(*NDatabase::GetTable<NDb::CMusic>());
    Require(rows.MoveNext(), "finished-music fixture has no track record");
    auto* track = rows.Get();
    track->szFileName = "__upstream_missing_music_fixture__.ogg";
    track->nFadeIn = 0; track->nRndPlayTime = 0;
    track->nSilence = 10000; track->nRndSilence = 0;
    CObj<NDb::CTMusic> pool = new NDb::CTMusic;
    pool->variants.push_back(track); pool->roulette.AddSector(1);
    CObj<CCTime> clock = new CCTime(0);
    Require(NFMSound::Init(NFMSound::SStartInfo()), "headless native audio initialization failed");
    struct AudioCleanup { ~AudioCleanup() { NFMSound::Done(); } } cleanup;
    CObj<NSound::ISoundScene> scene = NSound::CreateSoundScene(pool, nullptr, clock);
    CTransformStack transform; transform.Init();
    scene->Draw(&transform); // Starts the native stream and leaves the silence state.
    CPtr<NFMSound::CStream> stream = NFMSound::PlayStream(track->szFileName.c_str(), true, 0, true, 0);
    Require(IsValid(stream), "finished-music fixture did not retain a native stream");
    CPtr<CObjectBase> music;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* object) {
        if (pSSClasses->GetObjectTypeID(object) == int(0xa1063160)) music = object;
        return false;
    };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(scene.GetPtr())->operator&(writer); }
    Require(IsValid(music), "native sound scene did not retain its current music record");
    NFMSound::Done();
    Require(stream.GetPtr() != nullptr && !IsValid(stream), "native stream teardown did not leave its retained invalid reference");
    unsigned current = 777;
    CMemoryStream position;
    { CStructureSaver writer(position, CStructureSaver::WRITE); writer.Add(4, &current); }
    position.Seek(0);
    { CStructureSaver reader(position, CStructureSaver::READ); music->operator&(reader); }
    clock->Set(1000);
    scene->Draw(&transform);
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(scene.GetPtr())->operator&(writer); }
    saved.Seek(0);
    __int64 nextTrack = -1;
    { CStructureSaver reader(saved, CStructureSaver::READ, &graph); reader.Add(2, &nextTrack); }
    Require(nextTrack == 11000,
        "finished retained music stream entered restore logic and lost its authored silence interval");
    // A genuinely restored music object has no runtime stream. With audio
    // unavailable it must still attempt restoration and drop the failed track.
    CObj<CObjectBase> restored = pSSClasses->CreateObject(int(0xa1063160));
    Require(IsValid(restored), "native restored-music class is not registered");
    CDBPtr<NDb::CMusic> record = track;
    bool loop = true;
    CMemoryStream trackSave;
    { CStructureSaver writer(trackSave, CStructureSaver::WRITE);
      writer.Add(2, &record); writer.Add(3, &loop); writer.Add(4, &current); }
    trackSave.Seek(0);
    { CStructureSaver reader(trackSave, CStructureSaver::READ); restored->operator&(reader); }
    CObj<NSound::ISoundScene> loaded = NSound::CreateSoundScene(nullptr, nullptr, clock);
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.ids[restored.GetPtr()] = 1; references.refs[1] = restored.GetPtr();
    references.include = [](CObjectBase*) { return false; };
    CMemoryStream sceneSave;
    { CStructureSaver writer(sceneSave, CStructureSaver::WRITE, &references); writer.Add(5, &restored); }
    sceneSave.Seek(0);
    { CStructureSaver reader(sceneSave, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(loaded.GetPtr())->operator&(reader); }
    auto retainsRestored = [&]() {
        bool retained = false;
        CStructureNetworkContext inspect;
        inspect.include = [&](CObjectBase* object) { if (object == restored.GetPtr()) retained = true; return false; };
        CMemoryStream traversal;
        { CStructureSaver writer(traversal, CStructureSaver::WRITE, &inspect);
          static_cast<CObjectBase*>(loaded.GetPtr())->operator&(writer); }
        return retained;
    };
    Require(retainsRestored(), "restored sound scene lost its saved track before drawing");
    loaded->Draw(&transform);
    Require(!retainsRestored(), "genuinely missing stream no longer entered failed-resume cleanup");

}

static void HolsteredHeavyWeaponMesh(const char* databasePath, const char* resourcePath)
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
    Require(actor != nullptr, "heavy-mesh fixture has no ordinary actor");
    NDb::CRPGWeapon* heavy = nullptr;
    CDBIterator<NDb::CRPGWeapon> rows(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while (rows.MoveNext()) {
        auto* candidate = rows.Get();
        if (IsValid(candidate->pItem) && candidate->pItem->subType == NDb::SUBTYPE_HEAVY
                && IsValid(candidate->pItem->pModel) && IsValid(candidate->pAnimWeaponType)) {
            heavy = candidate; break;
        }
    }
    Require(heavy != nullptr, "heavy-mesh fixture has no modelled heavy weapon");
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(heavy);
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    CObj<NRPG::IInventoryItem> oldFirst = inventory->Get(NDb::SLOT_1) ? inventory->TakeOff(NDb::SLOT_1) : nullptr;
    CObj<NRPG::IInventoryItem> oldSecond = inventory->Get(NDb::SLOT_2) ? inventory->TakeOff(NDb::SLOT_2) : nullptr;
    Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1),
        "heavy-mesh fixture could not equip its active weapon");
    const char* bone = NWorld::GetBoneName(NWorld::UIT_WEAPON_HEAVY, weapon);
    Require(bone && bone[0], "heavy-mesh fixture weapon has no heavy bind bone");
    auto heavyCount = [&](bool holstered, bool noHeavy = false) {
        vector<NWorld::IRenderVisitor::SBoundMesh> meshes;
        NWorld::GetItemsBindPlaces(&meshes, actor->GetUnitRPG(), holstered, nullptr, noHeavy);
        return std::count_if(meshes.begin(), meshes.end(), [&](const auto& mesh) {
            return mesh.pszBindBone && !std::strcmp(mesh.pszBindBone, bone);
        });
    };
    Require(heavyCount(false) == 1, "drawn heavy weapon lost its normal mesh");
    Require(heavyCount(false, true) == 0, "no-heavy render mode did not suppress the weapon");
    Require(heavyCount(true) == 0, "holstered active heavy weapon remained in the native render bindings");
    Require(inventory->Activate(NDb::SLOT_2) && heavyCount(true) == 1,
        "active-weapon holster gate incorrectly hid an inactive carried heavy weapon");
}

class AckPriorityDesktop: public NUI::CDesktopWindow
{
    OBJECT_NOCOPY_METHODS(AckPriorityDesktop);
public:
    vector<CPtr<NWorld::CAckEvent>> played;
    AckPriorityDesktop(): CDesktopWindow(NUI::SWindowInfo(nullptr, NUI::SPoint(0, 0),
        NUI::SPoint(100, 100), "ack-priority-probe")) {}
protected:
    NUI::CAckEvent* PlayAckEvent(const STime&, NWorld::CAckEvent* event) override {
        played.push_back(event);
        return new NUI::CAckEvent(event);
    }
};

static void ScriptedDialoguePriority(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    CObj<NWorld::CUnitServer> actor;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight() || unit->IsEmptyPK()) continue;
        int personID = unit->GetUnitRPG()->GetRPGPers()->GetRecordID();
        if (personID != game->GetDialogHeroPersID() && world->GetUnitServerByPersID(personID) == unit) {
            actor = unit; break;
        }
    }
    Require(IsValid(actor), "dialogue-priority fixture has no existing persona speaker");
    CObj<NDb::CDBAckInfo> info = new NDb::CDBAckInfo;
    info->nRPGPersID = actor->GetUnitRPG()->GetRPGPers()->GetRecordID();
    info->voices.resize(6);
    const int dialogID = 900001;
    CDBIterator<NDb::CDBDialogSeq> rows(*NDatabase::GetTable<NDb::CDBDialogSeq>());
    int count = 0;
    while (count < 4 && rows.MoveNext()) {
        rows.Get()->nDialogID = dialogID;
        rows.Get()->pAckInfo = info.GetPtr();
        ++count;
    }
    Require(count == 4, "dialogue-priority fixture has fewer than four sequence rows");
    while (auto* pending = world->GetUICommand()) { CObj<NWorld::CUICmd> release = pending; }
    NWorld::PlayDialogAsAcks(world, dialogID);
    CObj<NWorld::CUICmd> command = world->GetUICommand();
    auto* packet = dynamic_cast<NWorld::CUICmdPlayAck*>(command.GetPtr());
    Require(packet && packet->phrases.size() == NDb::N_ACKINFO_MAX_COUNT,
        "scripted dialogue did not queue its original capped phrase sequence");
    CObj<NWorld::CAckEvent> ordinary = new NWorld::CAckEvent(10, actor, info);
    CObj<AckPriorityDesktop> desktop = new AckPriorityDesktop;
    desktop->PlayAck(ordinary);
    desktop->Draw(0, nullptr);
    Require(desktop->played.size() == 1 && desktop->played.back() == ordinary.GetPtr(),
        "dialogue-priority fixture did not start its ordinary acknowledgement");
    desktop->PlayAck(packet->phrases.front());
    desktop->Draw(1, nullptr);
    Require(desktop->played.size() == 2 && desktop->played.back() == packet->phrases.front().GetPtr(),
        "scripted dialogue was rejected behind an active ordinary acknowledgement");
    desktop->PlayAck(ordinary);
    desktop->Draw(2, nullptr);
    Require(desktop->played.size() == 2 && ordinary->nPriority == 10,
        "ordinary acknowledgement interrupted scripted dialogue or changed its own priority");
}

static void CorpseBackpackDrop(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) { actor = unit; break; }
    }
    Require(actor != nullptr, "backpack-drop fixture has no ordinary living actor");
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    while (!inventory->GetItems().empty()) inventory->Take(inventory->GetItems().front().pItem);
    NDb::CRPGItem* itemTemplate = nullptr;
    CDBIterator<NDb::CRPGItem> items(*NDatabase::GetTable<NDb::CRPGItem>());
    while (items.MoveNext()) {
        auto* item = items.Get();
        if (IsValid(item->pModel) && IsValid(item->pSuccessor) && item->sSize.x > 0
                && item->sSize.y > 0 && item->sSize.x <= 2 && item->sSize.y <= 2) {
            itemTemplate = item; break;
        }
    }
    Require(itemTemplate != nullptr, "backpack-drop fixture has no small modelled inventory item");
    vector<CObj<NRPG::IInventoryItem>> expected;
    for (int i = 0; i < 3; ++i) {
        CObj<NRPG::IInventoryItem> item = NRPG::CreateItem(itemTemplate->pSuccessor);
        CTPoint<int> place;
        Require(IsValid(item) && inventory->FindPlace(item, &place) && inventory->Place(place, item),
            "backpack-drop fixture could not store its three distinct items");
        expected.push_back(item);
    }
    CObj<NDb::CRPGItem> inertTemplate = new NDb::CRPGItem;
    inertTemplate->sSize = CTPoint<int>(1, 1);
    inertTemplate->subType = NDb::SUBTYPE_NONE;
    inertTemplate->nWeight = inertTemplate->nSubTypePriority = 0;
    CObj<NDb::CRPGKey> inertKey = new NDb::CRPGKey;
    inertKey->pItem = inertTemplate.GetPtr();
    inertKey->nKeyID = 0;
    CObj<NRPG::IInventoryItem> inert = NRPG::CreateItem(inertKey);
    CTPoint<int> inertPlace;
    Require(IsValid(inert) && inventory->FindPlace(inert, &inertPlace) && inventory->Place(inertPlace, inert),
        "backpack-drop fixture could not store its unmodelled item");
    Require(inventory->GetItems().size() == 4, "backpack-drop fixture merged distinct inventory entries");
    actor->SetBloodyDeath(false);
    actor->GetUnitRPG()->GetRPGUnit()->Kill();
    actor->SyncConscious();
    Require(actor->IsDead(), "backpack-drop fixture did not take the ordinary death path");
    Require(inventory->GetItems().empty(), "ordinary death skipped a backpack item while removing earlier entries");
    vector<CPtr<NRPG::IInventoryItem>> ground;
    world->GetDebris()->GetGroundItemsForDiagnostics(&ground);
    for (const auto& item: expected)
        Require(std::count_if(ground.begin(), ground.end(), [&](const auto& entry) { return entry.GetPtr() == item.GetPtr(); }) == 1,
            "ordinary death lost or duplicated a backpack item instead of launching it once");
    Require(std::none_of(ground.begin(), ground.end(), [&](const auto& item) { return item.GetPtr() == inert.GetPtr(); }),
        "unmodelled backpack record changed retail's nonvisual cleanup policy");
}

static void RestoreActionState(const char* databasePath, const char* resourcePath, bool zoneReentry)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    Require(!world->GetBuildingsForHarness().empty() && !world->IsAction(), "fixture has no settled buildings");
    vector<int> segments;
    for (const auto& building : world->GetBuildingsForHarness()) segments.push_back(building->nSegemntCnt);
    const bool realtime = world->IsRealTime();
    auto* player = world->GetCurrentPlayer();
    if (zoneReentry) {
        world->CreateRestored(game);
        Require(world->IsAction(), "zone reentry no longer resumed its building simulation");
        for (const auto& building : world->GetBuildingsForHarness())
            Require(building->nSegemntCnt == 0, "zone reentry did not rearm its building updates");
    } else {
        world->RestoreRuntimeCaches(game);
        Require(!world->IsAction(), "restoring save runtime caches started a new building action");
        int index = 0;
        for (const auto& building : world->GetBuildingsForHarness())
            Require(building->nSegemntCnt == segments[index++], "save cache restoration reset building segment timing");
        Require(world->IsRealTime() == realtime && world->GetCurrentPlayer() == player,
            "save cache restoration restarted the active turn");
    }
}

static void RestoreBuildingHulls(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    Require(!world->GetBuildingsForHarness().empty(), "fixture has no building geometry");
    auto* map = world->GetAIMap();
    map->Sync();
    SBound area;
    area.SphereInit(VNULL3, 10000.f);
    vector<CPtr<CObjectBase>> hulls;
    map->SelectHullPointers(&hulls, area, ~0, 0);
    Require(!hulls.empty(), "fixture has no live AI geometry hulls");
    for (const auto& hull : hulls) Require(IsValid(hull), "fixture retained an invalid AI hull");
    // Stability catchers retain weak hull references too. Reconnecting derived
    // save caches must leave unchanged support geometry alive through AI Sync.
    world->RestoreRuntimeCaches(game);
    map->Sync();
    for (const auto& hull : hulls)
        Require(IsValid(hull), "save cache restoration destroyed existing AI support hulls");
    Require(!world->IsAction(), "cache-only building restoration started simulation");
}

class RetiredSupportFixture: public CObjectBase
{
    OBJECT_BASIC_METHODS(RetiredSupportFixture);
};

struct TrackedCorpseFixture
{
    CPtr<NWorld::CUnitServer> unit;
    vector<CPtr<CObjectBase>> catchers;
    SBound bound;
    int operator&(CStructureSaver& saver) {
        saver.Add(2, &unit); saver.Add(3, &catchers); saver.Add(4, &bound); return 0;
    }
};

static void CarriedCorpseStability(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> humans;
    for (auto& actor : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(actor.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) humans.push_back(unit);
        if (humans.size() == 2) break;
    }
    Require(humans.size() == 2, "fixture needs a body and its carrier");
    auto* body = humans[0];
    auto* carrier = humans[1];
    body->GetUnitRPG()->GetRPGUnit()->Kill();
    body->SetState(new NWorld::CUnitStateDeath(body));
    body->animator.BeTaken(carrier, &carrier->animator, body);
    Require(body->animator.bIsCarried, "fixture did not attach the corpse to its carrier");

    // Restore the existing tracker wire with the former resting support absent
    // from the current map. No private production accessor or map copy is needed.
    CObj<RetiredSupportFixture> formerSupport = new RetiredSupportFixture;
    TrackedCorpseFixture record;
    record.unit = body;
    record.catchers.push_back(formerSupport.GetPtr());
    record.bound.SphereInit(body->GetPosition().GetCP(), 1.f);
    vector<TrackedCorpseFixture> corpses{record};
    CPtr<NWorld::IWorld> worldRef = world.GetPtr();
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[world.GetPtr()] = 1; references.refs[1] = world.GetPtr();
    references.ids[body] = 2; references.refs[2] = body;
    references.ids[formerSupport.GetPtr()] = 3; references.refs[3] = formerSupport.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CObj<NAI::IAIMapTracker> tracker = dynamic_cast<NAI::IAIMapTracker*>(pSSClasses->CreateObject(int(0x71812130)));
    Require(tracker != nullptr, "corpse stability tracker class is not registered");
    CMemoryStream state;
    { CStructureSaver writer(state, CStructureSaver::WRITE, &references);
      writer.Add(2, &corpses); writer.Add(5, &worldRef); writer.Add(6, &record.bound); }
    state.Seek(0);
    { CStructureSaver reader(state, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(tracker.GetPtr())->operator&(reader); }
    tracker->OnChange();
    Require(body->animator.bIsCarried, "old ground stability tracking dropped a corpse already attached to its carrier");
    tracker->OnChange();
    Require(body->animator.bIsCarried, "repeated stability notification detached a carried corpse");
}

class TileAttackMission: public AttackCursorMission
{
    OBJECT_NOCOPY_METHODS(TileAttackMission);
public:
    CVec3 target = VNULL3;
    void AttachWorld(NWorld::CWorld* world) { pWorld = world; }
    bool GetTracePosition(CVec3* result) const override { *result = target; return true; }
    bool GetTracePosition(NAI::SPosition*) const override { return false; }
};

static void ExactAttackPreviewTarget(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnit* shooter = nullptr;
    CVec3 target = VNULL3;
    int chance = 0;
    for (auto& actor : units) {
        auto* weapon = dynamic_cast<NRPG::IWeaponItemInfo*>(actor->GetRPG()->GetInventoryInfo()->GetActive());
        if (!weapon || weapon->GetDBWeapon()->bBazookaLogic) continue;
        for (float distance : {1.f, 2.f, 4.f}) {
            target = actor->GetPosition().GetCP() + CVec3(distance, 0.f, 0.97f);
            chance = world->GetGame()->GetTileCompositeToHit(actor, target, NAI::THL_MIDDLE, world->IsFirstTurn());
            if (chance >= 2) { shooter = actor; break; }
        }
        if (shooter) break;
    }
    Require(shooter != nullptr, "fixture has no positive firearm chance at a 3D target");
    CObj<TileAttackMission> mission = new TileAttackMission;
    mission->AttachWorld(world);
    mission->target = target;
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission, shooter);
    mission->selection.push_back(tracker.GetPtr());
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    NGame::SActionInfo info;
    info.SetValid();
    SetAttackPreview(state, info);
    const int ap = shooter->GetRPG()->GetAP();
    state->RefreshCaption();
    const auto expected = NUI::GetDBString(20276) + NUI::GetDBString(19808) + L"0"
        + NUI::GetDBString(20276) + NUI::GetDBString(19809) + std::to_wstring(chance) + L"%";
    Require(state->GetCursorInfo().wsText == expected,
        "attack preview lost the exact 3D target when no movement tile was available");
    Require(shooter->GetRPG()->GetAP() == ap, "attack preview spent action points");
}

static void GroupAttackPreview(const char* databasePath, const char* resourcePath, bool mixedChance)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units, candidates;
    world->GetAllUnits(&units);
    for (auto& actor : units) {
        auto* weapon = dynamic_cast<NRPG::IWeaponItemInfo*>(actor->GetRPG()->GetInventoryInfo()->GetActive());
        if (weapon && !weapon->GetDBWeapon()->bBazookaLogic) candidates.push_back(actor);
    }
    Require(candidates.size() >= 2, "fixture needs two distinct firearm users");
    vector<CPtr<NWorld::CUnit>> shooters{candidates[0], candidates[1]};
    auto chances = [&](const CVec3& target) {
        const int a = world->GetGame()->GetTileCompositeToHit(shooters[0], target, NAI::THL_MIDDLE, world->IsFirstTurn());
        const int b = world->GetGame()->GetTileCompositeToHit(shooters[1], target, NAI::THL_MIDDLE, world->IsFirstTurn());
        return std::make_pair((std::min)(a, b), (std::max)(a, b));
    };
    CVec3 target(1000, 1000, 1);
    auto range = chances(target);
    if (mixedChance) {
        // Exercise the existing native-client preview cache with two server
        // results. This controls UI inputs without changing combat formulas.
        world->bNetworkReplica = true;
        world->networkHitChance = [&](int kind, NWorld::CUnit* actor, NWorld::CUnit*, CVec3, int, bool) {
            Require(kind == 2, "group preview requested the wrong cached attack kind");
            return actor == shooters[0].GetPtr() ? 73 : 0;
        };
        range = chances(target);
    } else {
        Require(range.first == 0 && range.second == 0, "fixture distant target is not out of range for both shooters");
    }
    CObj<TileAttackMission> mission = new TileAttackMission;
    mission->AttachWorld(world);
    mission->target = target;
    vector<CObj<NGame::CUnitTracker>> trackers;
    for (auto& shooter : shooters) {
        trackers.emplace_back(new NGame::CUnitTracker(mission, shooter));
        mission->selection.push_back(trackers.back().GetPtr());
    }
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    NGame::SActionInfo info;
    info.SetValid();
    SetAttackPreview(state, info);
    state->RefreshCaption();
    auto percent = std::to_wstring(range.first);
    if (range.first != range.second) percent += L"-" + std::to_wstring(range.second);
    percent += L"%";
    const auto expected = NUI::GetDBString(20276) + NUI::GetDBString(19808) + L"0"
        + NUI::GetDBString(range.second >= 2 ? 20276 : 19807) + NUI::GetDBString(19809) + percent;
    Require(state->GetCursorInfo().wsText == expected, mixedChance
        ? "group attack caption was coloured by the worst chance despite a valid selected shooter"
        : "equal selected-unit hit chances were displayed as a redundant percentage range");
}

static NWorld::CExecShootUnit* FindQueuedUnitShot(NWorld::CCommandExecute* executor)
{
    if (auto* shot = dynamic_cast<NWorld::CExecShootUnit*>(executor)) return shot;
    NWorld::CExecShootUnit* found = nullptr;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* object) {
        if (auto* shot = dynamic_cast<NWorld::CExecShootUnit*>(object)) {
            found = shot;
            return false;
        }
        return dynamic_cast<NWorld::CSimpleExecQueue*>(object) != nullptr;
    };
    CObj<NWorld::CCommandExecute> root = executor;
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph); writer.Add(1, &root); }
    return found;
}

struct ShotBaseSnapshot
{
    int extraAP = 0;
    bool onlyPrepare = false;
    int operator&(CStructureSaver& saver) {
        saver.Add(2, &extraAP); saver.Add(12, &onlyPrepare); return 0;
    }
};
struct ShotCommandSnapshot
{
    ShotBaseSnapshot base;
    CPtr<NWorld::CUnitServer> target;
    NAI::EHitLocation hitLocation = NAI::HL_ANY;
};

static ShotCommandSnapshot ReadStartedShot(NWorld::CExecShootUnit* shot,
    NWorld::CUnitServer* actor, NWorld::CUnitServer* target)
{
    // Start fills the shot/ray bookkeeping before inspecting a valid in-flight
    // snapshot. No animation labels or world segments fire a bullet here.
    shot->Start();
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[actor] = 1; references.refs[1] = actor;
    references.ids[target] = 2; references.refs[2] = target;
    references.nextID = 100;
    references.include = [](CObjectBase*) { return false; };
    CMemoryStream state;
    { CStructureSaver writer(state, CStructureSaver::WRITE, &references);
      static_cast<CObjectBase*>(shot)->operator&(writer); }
    state.Seek(0);
    ShotCommandSnapshot snapshot;
    { CStructureSaver reader(state, CStructureSaver::READ, &references);
      reader.Add(1, &snapshot.base); reader.Add(2, &snapshot.target); reader.Add(3, &snapshot.hitLocation); }
    return snapshot;
}

static void SniperHeldAim(const char* databasePath, const char* resourcePath, bool targetlessProbe = false)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NDb::CRPGWeapon* rifle = nullptr;
    CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while (weapons.MoveNext()) {
        auto* candidate = weapons.Get();
        if (candidate->shootModes[NDb::SM_Snipe] && !candidate->bBazookaLogic &&
                IsValid(candidate->pItem) && IsValid(candidate->pInnerClip)) {
            rifle = candidate; break;
        }
    }
    Require(rifle != nullptr, "fixture database has no sniper-capable firearm");
    NWorld::CUnitServer* actor = nullptr;
    NWorld::CUnitServer* target = nullptr;
    CObj<NWorld::CUnitStateSniping> state;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (!unit || !unit->CanFight() || unit->IsEmptyPK() || unit->IsWearingPK()) continue;
        CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(rifle);
        auto* inventory = unit->GetUnitRPG()->GetInventory();
        if (!inventory->Equip(NDb::SLOT_1, weapon) || !inventory->Activate(NDb::SLOT_1) ||
                !weapon->SetShootMode(NDb::SM_Snipe)) continue;
        for (auto& other : units) {
            auto* victim = dynamic_cast<NWorld::CUnitServer*>(other.GetPtr());
            if (!victim || victim == unit || !victim->CanFight() || victim->GetPlayer() != unit->GetPlayer()) continue;
            CObj<NWorld::CUnitStateSniping> probe = new NWorld::CUnitStateSniping(unit, victim, 30, NAI::HL_HEAD);
            unit->SetState(probe);
            CObj<NWorld::CCmdShootObject> explicitShot = new NWorld::CCmdShootObject(victim, 0, NAI::HL_HEAD);
            NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
            CObj<NWorld::CCommandExecute> executor = probe->CreateExecutor(explicitShot, &result);
            auto* shot = executor ? FindQueuedUnitShot(executor) : nullptr;
            if (shot && result == NWorld::UCR_OK && shot->CanDoIt(unit->GetPosition()) == NWorld::UCR_OK) {
                actor = unit; target = victim; state = probe; break;
            }
        }
        if (actor) break;
        unit->SetState(new NWorld::CUnitStateNormal(unit));
    }
    Require(actor && target && state, "fixture contains no currently shootable held sniper target");
    if (targetlessProbe) {
        auto check = [&](CObjectBase* requestedTarget, NAI::EHitLocation point, int extraAP,
                         bool onlyPrepare, NAI::EHitLocation expectedPoint) {
            CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(rifle);
            auto* inventory = actor->GetUnitRPG()->GetInventory();
            Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1) &&
                weapon->SetShootMode(NDb::SM_Snipe), "fixture could not reset the sniper magazine");
            CObj<NWorld::CCmdShootObject> command = new NWorld::CCmdShootObject(requestedTarget, extraAP, point);
            command->bOnlyPrepareToShoot = onlyPrepare;
            NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
            CObj<NWorld::CCommandExecute> executor = state->CreateExecutor(command, &result);
            auto* shot = executor ? FindQueuedUnitShot(executor) : nullptr;
            Require(shot && result == NWorld::UCR_OK, "targetless sniper Fire did not resolve the held target");
            CObj<NWorld::CExecShootUnit> reference = new NWorld::CExecShootUnit(actor, target, expectedPoint, extraAP, onlyPrepare);
            Require(shot->GetStartAP() == reference->GetStartAP(), "sniper Fire lost the extra-AP preview cost");
            const int beforeAP = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
            const auto snapshot = ReadStartedShot(shot, actor, target);
            Require(snapshot.target.GetPtr() == target && snapshot.hitLocation == expectedPoint,
                "sniper Fire resolved the wrong target or aim point");
            Require(snapshot.base.onlyPrepare == onlyPrepare, "sniper Fire lost its prepare-only command flag");
            if (onlyPrepare) {
                Require(snapshot.base.extraAP == extraAP, "sniper preparation lost its extra-AP parameter");
                Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == beforeAP,
                    "sniper preparation spent firing action points");
            }
        };
        check(nullptr, NAI::HL_ANY, 7, true, NAI::HL_HEAD);
        check(target, NAI::HL_ANY, 5, true, NAI::HL_HEAD);
        check(nullptr, NAI::HL_BODY, 3, true, NAI::HL_BODY);
        CObj<NWorld::CCmdShootObject> wrongTarget = new NWorld::CCmdShootObject(actor, 0);
        NWorld::EUnitCommandResult result = NWorld::UCR_OK;
        CObj<NWorld::CCommandExecute> rejected = state->CreateExecutor(wrongTarget, &result);
        Require(!rejected && result == NWorld::UCR_UNAVAILABLE, "sniper Fire retargeted an unrelated explicit target");
        state = new NWorld::CUnitStateSniping(actor, target, 30, NAI::HL_ANY);
        actor->SetState(state);
        check(nullptr, NAI::HL_ANY, 0, true, NAI::HL_ANY);
        check(nullptr, NAI::HL_HEAD, 0, false, NAI::HL_HEAD);
        return;
    }
    CObj<NWorld::CCmdSnipeAttack> command = new NWorld::CCmdSnipeAttack;
    NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
    CObj<NWorld::CCommandExecute> executor = state->CreateExecutor(command, &result);
    auto* shot = executor ? FindQueuedUnitShot(executor) : nullptr;
    Require(shot && result == NWorld::UCR_OK, "sniper Fire did not create a unit shot");
    const auto snapshot = ReadStartedShot(shot, actor, target);
    Require(snapshot.hitLocation == NAI::HL_HEAD, "sniper Fire discarded the held called-shot location");
    Require(snapshot.target.GetPtr() == target, "sniper Fire changed its held target");
}

static void PersonClueCreation(const char* databasePath, const char* resourcePath, bool withInventoryClue = false)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    NScenario::CScenarioClue* clue = nullptr;
    CDBIterator<NDb::CDBScenarioClue> clues(*NDatabase::GetTable<NDb::CDBScenarioClue>());
    while (clues.MoveNext()) {
        auto* record = clues.Get();
        if (record->clueType == NDb::CT_PERSON && record->nPersID > 0 && NDb::GetPers(record->nPersID)) {
            clue = game->pScenarioTracker->GetClueByPersID(record->nPersID);
            if (clue) break;
        }
    }
    Require(clue != nullptr, "fixture has no scenario person clue");
    auto* itemClue = game->pScenarioTracker->GetClueByName("Find_Cipher_key");
    auto* item = itemClue ? NDb::GetRPGItem(itemClue->GetDBClue()->nItemID) : nullptr;
    Require(item != nullptr, "fixture has no inventory clue item");
    item->pSuccessor = nullptr;
    item->sSize = CTPoint<int>(1, 1);
    // Force the authored human spawn points to offer person-clue slots. All
    // changes stay in the loaded database, without writing any game assets.
    CDBIterator<NDb::CUnit> records(*NDatabase::GetTable<NDb::CUnit>());
    while (records.MoveNext()) {
        auto* unit = records.Get();
        unit->bClueSlot = IsValid(unit->pMonster) && !IsValid(unit->pMonster->pPanzerklein);
        unit->bClueInventorySlot = withInventoryClue;
    }
    CDBIterator<NDb::CPlacableObject> objects(*NDatabase::GetTable<NDb::CPlacableObject>());
    while (objects.MoveNext()) {
        auto* object = objects.Get();
        if (IsValid(object->pRPGItem) && object->pRPGItem->GetRecordID() == 188)
            object->pRPGItem = nullptr;
    }
    CObj<NWorld::CWorld> world = new NWorld::CWorld(game);
    CObj<NWorld::CPostWorldCreateInfo> post;
    std::list<CPtr<NScenario::CScenarioClue>> requested{clue};
    if (withInventoryClue) requested.push_back(itemClue);
    world->CreateRandom(810, {}, true, requested, 0, &post, SRandomSeed(123));
    vector<CPtr<NWorld::CUnit>> actors;
    world->GetAllUnits(&actors);
    int found = 0;
    for (auto& actor : actors) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(actor.GetPtr());
        if (actor->GetRPG()->GetRPGUnit()->GetPers()->GetRecordID() == clue->GetDBClue()->nPersID) {
            ++found;
            if (withInventoryClue) {
                int carried = 0;
                for (const auto& entry : actor->GetRPG()->GetInventoryInfo()->GetItems())
                    if (dynamic_cast<NRPG::IClueItem*>(entry.pItem.GetPtr()) && entry.pItem->GetDBItem() == item)
                        ++carried;
                Require(carried == 1, "inventory clue was lost when its slot's authored person was replaced");
                Require(server->nClueCount == 2, "replacement person did not retain both scenario clues");
            } else {
                Require(server->IsClueUnit() && server->nClueCount == 1,
                    "spawned scenario person lost its clue marker and corpse-cleanup protection");
            }
        } else {
            Require(!server->IsClueUnit(), "ordinary actor was marked as a scenario person");
        }
    }
    Require(found == 1, "fixture did not spawn exactly one scenario person");
}

static void InventoryClueCreation(const char* databasePath, const char* resourcePath, bool checkCount = false, bool noSpace = false)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    auto* clue = game->pScenarioTracker->GetClueByName("Find_Cipher_key");
    Require(clue && clue->GetDBClue()->clueType == NDb::CT_ITEM, "fixture has no item clue");
    auto* record = NDb::GetRPGItem(clue->GetDBClue()->nItemID);
    Require(record != nullptr, "fixture item clue has no RPG record");
    // A pure quest item has no ordinary-item successor. Keep fixture changes
    // in the loaded database only; no game.db or resource copy is written.
    record->pSuccessor = nullptr;
    record->sSize = noSpace ? CTPoint<int>(100, 100) : CTPoint<int>(1, 1);
    CDBIterator<NDb::CUnit> units(*NDatabase::GetTable<NDb::CUnit>());
    while (units.MoveNext()) {
        auto* unit = units.Get();
        unit->bClueSlot = false;
        unit->bClueInventorySlot = IsValid(unit->pMonster) && !IsValid(unit->pMonster->pPanzerklein);
    }
    // MapBuild's item 188 is the floor clue-slot marker. This fixture routes
    // every candidate slot through a normal human's backpack instead.
    CDBIterator<NDb::CPlacableObject> objects(*NDatabase::GetTable<NDb::CPlacableObject>());
    while (objects.MoveNext()) {
        auto* object = objects.Get();
        if (IsValid(object->pRPGItem) && object->pRPGItem->GetRecordID() == 188)
            object->pRPGItem = nullptr;
    }
    CObj<NWorld::CWorld> world = new NWorld::CWorld(game);
    CObj<NWorld::CPostWorldCreateInfo> post;
    world->CreateRandom(810, {}, true, std::list<CPtr<NScenario::CScenarioClue>>{clue}, 0, &post, SRandomSeed(123));
    vector<CPtr<NWorld::CUnit>> actors;
    world->GetAllUnits(&actors);
    int found = 0;
    for (auto& actor : actors) {
        auto* inventory = actor->GetRPG()->GetInventoryInfo();
        int carried = 0;
        for (const auto& entry : inventory->GetItems())
            if (dynamic_cast<NRPG::IClueItem*>(entry.pItem.GetPtr()) && entry.pItem->GetDBItem() == record)
                ++carried;
        found += carried;
        if (checkCount)
            Require(dynamic_cast<NWorld::CUnitServer*>(actor.GetPtr())->nClueCount == carried,
                "inventory clue placement did not count the clue on its carrier");
    }
    if (noSpace)
        Require(found == 0, "inventory clue placement inserted a quest item without room in the backpack");
    else
        Require(found == 1, "inventory clue placement lost a quest item without an ordinary successor");
}


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


class DialogueLayoutView: public NGScene::I2DGameView
{
    OBJECT_BASIC_METHODS(DialogueLayoutView);
    CObj<NGScene::CTextLocaleInfo> locale;
    CVec2 viewport = CVec2(1024, 768);
public:
    int queries = 0;
    int maxQueries = 100000;
    DialogueLayoutView(): locale(new NGScene::CTextLocaleInfo) { locale->Setup(viewport); }
    void CreateDynamicRects(CFuncBase<NGScene::SText>*, const CTPoint<int>&, const CTRect<int>&) override {}
    void CreateDynamicRects(NDb::CTexture*, const CRectLayout&, const CTPoint<int>&, const CTRect<int>&) override {}
    void CreateDynamicRects(CPtrFuncBase<NGfx::CTexture>*, const CRectLayout&, const CTPoint<int>&, const CTRect<int>&) override {}
    CFuncBase<NGScene::SText>* CreateText(CFuncBase<wstring>*, CFuncBase<CTPoint<int>>*, bool) override { return nullptr; }
    void CreateDynamicClearRects(const CRectLayout&, const CTPoint<int>&, const CTRect<int>&, float) override {}
    NGScene::CTextLocaleInfo* GetLocaleInfo() const override { return locale; }
    const CVec2& GetViewportSize() override {
        Require(++queries <= maxQueries, "dialogue pagination made no progress with markup-only content");
        return viewport;
    }
    void StartNewFrame() override {}
    void Flush() override {}
};

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

static void OversizedDialogueLine(const char* databasePath, const char* resourcePath, int scenario = 0)
{
    const bool markupOnly = scenario == 1;
    const bool normalPages = scenario == 2;
    auto world = FixtureWorld(databasePath, resourcePath, true);
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
        NUI::SWindowInfo(nullptr, NUI::SPoint(0, 0), NUI::SPoint(normalPages ? 120 : 500, normalPages ? 50 : markupOnly ? 10 : 1), "dialogue-text"));
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
    layout->SetText(texts.front()->szStr, 0); layout->Generate(view, text->GetSize().x);
    Require(layout->GetSize().y > text->GetSize().y, "dialogue fixture did not overflow its text box");
    if (markupOnly) {
        list<NUI::SRect> bounds;
        layout->Render(&bounds, NUI::SPoint(0, 0), NUI::SRect(0, 0, 0, 0));
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

class AICollectionCallbackException: public std::runtime_error
{
public:
    AICollectionCallbackException(): std::runtime_error("expected AI collection callback exception") {}
};

class AICollectionEventProbe: public NAI::IAIEvent
{
    OBJECT_NOCOPY_METHODS(AICollectionEventProbe);
public:
    CPtr<NAI::CAIEventTracker> tracker;
    CObj<NAI::IAIEvent> nested;
    int expectedDepth = 1, calls = 0;
    bool throwAfterUpdate = false;
    void Modify(NAI::SAIUnitState* state) override {
        ++calls;
        Require(state->enemies.nLock == expectedDepth && state->possibleEnemies.nLock == expectedDepth
                && state->allies.nLock == expectedDepth,
            "AI event callback ran without holding all three contact collection locks");
        state->enemies.bModified = state->possibleEnemies.bModified = state->allies.bModified = true;
        state->Update();
        Require(state->enemies.bModified && state->possibleEnemies.bModified && state->allies.bModified,
            "AI event callback consumed contact dirty flags before notification completed");
        if (IsValid(nested)) tracker->Notify(nested);
        Require(state->enemies.nLock == expectedDepth && state->possibleEnemies.nLock == expectedDepth
                && state->allies.nLock == expectedDepth,
            "nested AI notification corrupted its caller's collection lock depth");
        if (throwAfterUpdate) throw AICollectionCallbackException();
    }
};

static void AIEventCollectionLocks(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NAI::SAIUnitState* state = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight()) continue;
        auto* ai = NAI::GetAIUnit(unit);
        if (ai && ai->IsUnderAIControl() && ai->GetAIUnitState()) {
            actor = unit; state = ai->GetAIUnitState(); break;
        }
    }
    Require(actor && state, "event-lock fixture has no live AI threat state");
    CObj<NAI::CAIEventTracker> tracker = new NAI::CAIEventTracker(actor);
    CObj<AICollectionEventProbe> outer = new AICollectionEventProbe;
    CObj<AICollectionEventProbe> inner = new AICollectionEventProbe;
    outer->tracker = inner->tracker = tracker.GetPtr();
    inner->expectedDepth = 2;
    outer->nested = inner.GetPtr();
    tracker->Notify(outer);
    Require(outer->calls == 1 && inner->calls == 1, "nested AI notification was lost or duplicated");
    Require(state->enemies.nLock == 0 && state->possibleEnemies.nLock == 0 && state->allies.nLock == 0,
        "completed AI notification retained contact collection locks");
    state->Update();
    Require(!state->enemies.bModified && !state->possibleEnemies.bModified && !state->allies.bModified,
        "contact collection dirty flags were not consumed after leaving notification");
    outer->nested = nullptr;
    outer->throwAfterUpdate = true;
    bool caught = false;
    try { tracker->Notify(outer); } catch (const AICollectionCallbackException&) { caught = true; }
    Require(caught && state->enemies.nLock == 0 && state->possibleEnemies.nLock == 0 && state->allies.nLock == 0,
        "throwing AI callback leaked contact collection locks");
}

static void AIReactionResetContacts()
{
    CObj<NAI::AIUnitFixture> contact = new NAI::AIUnitFixture;
    NAI::SAIUnitState state;
    state.enemies.data.units.push_back(contact.GetPtr());
    state.possibleEnemies.data.units.push_back(contact.GetPtr());
    state.allies.data.units.push_back(contact.GetPtr());
    state.knownCorpses.push_back(contact.GetPtr());
    state.pEnemy = state.pPossibleEnemy = state.pAlly = contact.GetPtr();
    state.bHelpCalled = state.bScared = true;
    CObj<NAI::IAIEvent> event = NAI::CreateAIUpdateEvent();
    event->Modify(&state);
    Require(state.IsModified(), "AI update event did not arm a reaction before reset");
    state.Reset();
    Require(state.enemies.data.units.size() == 1 && state.possibleEnemies.data.units.size() == 1
            && state.allies.data.units.size() == 1 && state.pEnemy == contact.GetPtr()
            && state.pPossibleEnemy == contact.GetPtr() && state.pAlly == contact.GetPtr()
            && state.knownCorpses.size() == 1 && state.bHelpCalled && state.bScared,
        "consuming an AI reaction notification erased contacts or morale state");
    Require(!state.IsModified(), "AI notification reset failed to consume its boolean value");
}

static void AIReactionValueLatch(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NAI::IAIUnit* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight()) continue;
        auto* ai = NAI::GetAIUnit(unit);
        if (ai && ai->IsUnderAIControl() && IsValid(ai->GetReaction())) { actor = ai; break; }
    }
    Require(actor != nullptr, "reaction fixture has no AI-controlled actor with a reaction");
    auto* state = actor->GetAIUnitState();
    auto* reaction = actor->GetReaction();
    auto restoreLatch = [&](bool value, bool dirty, int locks) {
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE);
          writer.Add(2, &locks); writer.Add(3, &dirty); writer.Add(4, &value); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ); state->selfModified.operator&(reader); }
    };
    restoreLatch(true, false, 1);
    Require(actor->GetReactionForUpdate() == reaction,
        "AI reaction pump ignored a restored notification value when its tracking flag was clear");
    Require(actor->GetReactionForUpdate() == nullptr && !state->selfModified.bModified,
        "AI reaction pump repeated a consumed value or dirtied a locked tracking flag");
    CObj<NAI::IAIEvent> event = NAI::CreateAIUpdateEvent();
    event->Modify(state);
    Require(actor->GetReactionForUpdate() == reaction && actor->GetReactionForUpdate() == nullptr,
        "AI update event did not deliver exactly one notification under a tracking lock");
    restoreLatch(false, true, 0);
    Require(actor->GetReactionForUpdate() == nullptr,
        "AI reaction pump treated a dirty tracking flag as a new notification");
    actor->SetReaction(reaction);
    Require(actor->GetReactionForUpdate() == reaction && actor->GetReactionForUpdate() == nullptr,
        "installing an AI reaction did not arm and consume exactly one notification");
}

static void IdleAI(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NAI::CAICommander* commander = nullptr;
    for (auto& entry : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight()) continue;
        unit->GetUnitRPG()->GetRPGUnit()->SetCheat(NRPG::CHEAT_NOAI, true);
        if (!actor) {
            commander = dynamic_cast<NAI::CAICommander*>(unit->GetPlayer()->GetCommander());
            if (commander) actor = unit;
        }
    }
    Require(actor && commander, "mission fixture contains no AI actor");
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    Require(!world->IsRealTime() && !world->IsSequence() && !world->IsAction(), "AI fixture is not idle in its turn");
    actor->GetUnitRPG()->GetRPGUnit()->SetCheat(NRPG::CHEAT_NOAI, false);
    auto* ai = commander->GetAIUnit(actor);
    Require(ai != nullptr, "AI actor wrapper missing");
    auto original = actor->GetPosition();
    auto inactive = original;
    inactive.pos.p.SetPose(NAI::CM_INACTIVE);
    for (int routine = 0; routine < 3; ++routine) {
        CObj<NAI::CAIRouteLogic> route = new NAI::CAIRouteLogic(ai);
        route->SetCircled(true);
        if (routine == 1) route->AddCommand(new NAI::CTaskCommandChangePose(NAI::WALK));
        if (routine == 2) route->AddCommand(new NAI::CTaskCommandGoto(original.pos));
        route->AddCommand(new NAI::CTaskCommandWait(100));
        route->AddCommand(new NAI::CTaskCommandChangeDirection(0));
        ai->SetLogic(route);
        actor->SetTemporaryPosition(original);
        Require(!commander->IsEndOfTurn(), "normal AI route was incorrectly skipped");
        actor->SetTemporaryPosition(inactive);
        Require(commander->IsEndOfTurn() == (routine != 2),
            routine == 2 ? "a route with movement was incorrectly skipped" : "inactive stationary AI route blocks the end of turn");
    }
    actor->SetTemporaryPosition(original);
}

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

static void EffectiveWeaponMode(const char* databasePath, const char* resourcePath, int checkIcons = 0)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    CPtr<NRPG::CWeaponItem> personal;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (!unit || !unit->CanFight() || unit->IsEmptyPK() || unit->IsWearingPK()) continue;
        auto* item = dynamic_cast<NRPG::CWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        if (item && item->IsShootModeSupported(NDb::SM_Snap) && item->IsShootModeSupported(NDb::SM_Aimed)) {
            actor = unit; personal = item; break;
        }
    }
    Require(actor && IsValid(personal), "mission fixture has no suitable personal firearm");
    CObj<NRPG::CWeaponItem> mounted = new NRPG::CWeaponItem(personal->GetDBWeapon());
    Require(personal->SetShootMode(NDb::SM_Snap) && mounted->SetShootMode(NDb::SM_Snap), "fixture modes are unsupported");
    actor->GetUnitRPG()->SetCannonItem(mounted);
    CObj<NWorld::CCmdShootMode> command = new NWorld::CCmdShootMode(NDb::SM_Aimed);
    NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
    CObj<NWorld::CCommandExecute> executor = NWorld::CreateExecutor(actor, command, &result);
    Require(executor && result == NWorld::UCR_OK, "could not create a fire-mode executor");
    executor->Run();
    Require(mounted->GetShootMode() == NDb::SM_Aimed, "fire-mode executor changed the personal weapon instead of the mounted gun");
    Require(personal->GetShootMode() == NDb::SM_Snap, "mounted fire-mode change modified the holstered personal weapon");
    if (checkIcons) {
        CObj<WeaponIconMission> mission = new WeaponIconMission;
        CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission, actor);
        mission->selected.push_back(tracker.GetPtr());
        if (checkIcons == 3) MountedModeButton(mission);
        if (checkIcons == 2)
            Require(NUI::GetSelectedWeaponForIcons(mission) == mounted.GetPtr(),
                "fire-mode submenu enumerated the personal firearm instead of the mounted gun");
        Require(NUI::GetUnitsGroupWeaponMode(mission) == NDb::SM_Aimed,
            "mounted-gun mode indicator displayed the personal firearm mode");
        Require(NUI::GetSelectedWeaponForIcons(mission) == mounted.GetPtr(),
            "fire-mode submenu enumerated the personal firearm instead of the mounted gun");
        actor->GetUnitRPG()->SetCannonItem(nullptr);
        Require(NUI::GetUnitsGroupWeaponMode(mission) == NDb::SM_Snap &&
                NUI::GetSelectedWeaponForIcons(mission) == personal.GetPtr(),
            "personal fire-mode UI did not resume after leaving the mounted gun");
        mission->selected.push_back(tracker.GetPtr());
        Require(!NUI::GetSelectedWeaponForIcons(mission), "multi-selection enabled the single-weapon submenu");
        mission->selected.clear();
        Require(NUI::GetUnitsGroupWeaponMode(mission) == -2 && !NUI::GetSelectedWeaponForIcons(mission),
            "empty selection exposed a weapon mode");
    }
    actor->GetUnitRPG()->SetCannonItem(nullptr);
}

class RenderCache;
class RenderFixture: public CObjectBase
{
    OBJECT_BASIC_METHODS(RenderFixture);
public:
    void Visit(RenderCache*) {}
};

class RenderCache: public COrdinarySyncDst<RenderFixture, RenderCache>
{
protected:
    void PostVisit(int, RenderFixture* object) override { cached = object; }
public:
    CPtr<RenderFixture> cached;
    explicit RenderCache(CSyncSrc<RenderFixture>* source): COrdinarySyncDst(source) {}
};

static void RenderRemoval()
{
    CObj<CSetSyncSrc<RenderFixture>> source = new CSetSyncSrc<RenderFixture>;
    CObj<RenderFixture> object = new RenderFixture;
    RenderCache cache(source);
    source->Set(std::vector<RenderFixture*>{object});
    cache.Sync();
    Require(cache.cached == object, "invalid render cache test fixture");
    source->Set(std::vector<RenderFixture*>{});
    cache.Sync();
    Require(!cache.cached, "removed render slot retained a live object reference");
}

static void RenderPassCompaction()
{
    // Start with two low passes so the old wrong-destination write fails
    // deterministically without needing an out-of-bounds memory access.
    for (const auto& passes : std::vector<std::vector<int>>{
            {1, 2, 8, 9}, {8, 1, 9, 2}, {1, 8}, {8, 9}, {1, 2}, {}, {8}}) {
        NGScene::CRenderCmdList source, lower;
        std::vector<float> expectedLower, expectedRetained;
        for (int i = 0; i < int(passes.size()); ++i) {
            source.ops.emplace_back(nullptr, NGScene::RO_NOP,
                static_cast<unsigned char>(passes[i]), 0, 0,
                NGScene::CRenderCmdList::UParameter(float(i + 1)));
            (passes[i] < 8 ? expectedLower : expectedRetained).push_back(float(i + 1));
        }
        NGScene::SplitOps(&lower, &source, 8);
        Require(lower.ops.size() == expectedLower.size(), "render split changed lower-pass count");
        Require(source.ops.size() == expectedRetained.size(), "render split changed retained-pass count");
        for (int i = 0; i < int(expectedLower.size()); ++i)
            Require(lower.ops[i].p1.f == expectedLower[i] && lower.ops[i].nPass < 8,
                "retained render passes overwrote lower passes");
        for (int i = 0; i < int(expectedRetained.size()); ++i)
            Require(source.ops[i].p1.f == expectedRetained[i] && source.ops[i].nPass >= 8,
                "render split lost or reordered retained passes");
    }
}

class PassiveUnitState: public NWorld::CUnitState
{
    OBJECT_NOCOPY_METHODS(PassiveUnitState);
public:
    NWorld::CCommandExecute* CreateExecutor(NWorld::CCmd*, NWorld::EUnitCommandResult*) override { return nullptr; }
    void FilterCriticals() override {}
};

class ReplacingUnitState: public PassiveUnitState
{
    OBJECT_NOCOPY_METHODS(ReplacingUnitState);
    bool* callbackAlive = nullptr;
    int marker = 0;
public:
    ReplacingUnitState() {}
    ReplacingUnitState(NWorld::CUnitServer* unit, bool* alive): callbackAlive(alive), marker(42) { pUS = unit; }
    void OnStateStarted() override {
        // Keep the allocation to observe destroyed contents without a UAF.
        CPtr<ReplacingUnitState> weak(this);
        bool* result = callbackAlive;
        pUS->SetState(new PassiveUnitState);
        *result = IsValid(this) && marker == 42;
    }
};

static void ReentrantStateLifetime()
{
    CObj<NWorld::CUnitServer> unit = new NWorld::CUnitServer;
    bool callbackAlive = false;
    unit->SetState(new ReplacingUnitState(unit, &callbackAlive));
    Require(callbackAlive, "self-replacing unit state was destroyed before its callback returned");
}

class MountedStateProbe: public NWorld::CUnitStateUsingCannon
{
    OBJECT_NOCOPY_METHODS(MountedStateProbe);
public:
    bool blocked = false;
    explicit MountedStateProbe(NWorld::CUnitServer* unit = nullptr): CUnitStateUsingCannon(unit, nullptr) {}
    bool IsCriticalsFailCommand(NWorld::CCmd*, NWorld::EUnitCommandResult* result) override {
        if (blocked) *result = NWorld::UCR_CRITICALS_BAN;
        return blocked;
    }
};

static void MountedModeCommand()
{
    CObj<NWorld::CUnitServer> unit = new NWorld::CUnitServer;
    CObj<MountedStateProbe> state = new MountedStateProbe(unit);
    for (auto mode : {NDb::SM_Snap, NDb::SM_Aimed, NDb::SM_Careful, NDb::SM_ShortBurst, NDb::SM_LongBurst}) {
        CObj<NWorld::CCmdShootMode> command = new NWorld::CCmdShootMode(mode);
        NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
        CObj<NWorld::CCommandExecute> executor = state->CreateExecutor(command, &result);
        Require(executor && result == NWorld::UCR_OK, "mounted-gun state rejected a fire-mode command");
    }
    state->blocked = true;
    CObj<NWorld::CCmdShootMode> blocked = new NWorld::CCmdShootMode(NDb::SM_Aimed);
    NWorld::EUnitCommandResult result = NWorld::UCR_OK;
    CObj<NWorld::CCommandExecute> executor = state->CreateExecutor(blocked, &result);
    Require(!executor && result == NWorld::UCR_CRITICALS_BAN, "mounted fire-mode command bypassed injury restrictions");
    state->blocked = false;
    CObj<NWorld::CCmdSetActiveItem> inventory = new NWorld::CCmdSetActiveItem(0);
    executor = state->CreateExecutor(inventory, &result);
    Require(!executor && result == NWorld::UCR_UNAVAILABLE, "mounted state accepted an unrelated inventory command");
}

class CallbackQueue: public NWorld::CSimpleExecQueue
{
    OBJECT_NOCOPY_METHODS(CallbackQueue);
public:
    void Replace(NWorld::CCommandExecute* replacement) { execList.clear(); AddExecutor(replacement); }
};

class WaitingExecutor: public NWorld::CCommandExecute
{
    OBJECT_NOCOPY_METHODS(WaitingExecutor);
};

class ReplacingExecutor: public NWorld::CCommandExecute
{
    OBJECT_NOCOPY_METHODS(ReplacingExecutor);
    CPtr<CallbackQueue> queue;
    CPtr<WaitingExecutor> replacement;
    void Callback() {
        CPtr<ReplacingExecutor> weak(this);
        queue->Replace(replacement);
        Require(IsValid(this), "queue destroyed its executor while the executor callback was running");
    }
public:
    ReplacingExecutor() {}
    ReplacingExecutor(CallbackQueue* owner, WaitingExecutor* next): queue(owner), replacement(next) {}
    void Run() override { Callback(); }
    void AnimationFinished() override { Callback(); }
};

static void ReentrantQueueLifetime(bool animationFinished)
{
    CObj<CallbackQueue> queue = new CallbackQueue;
    CObj<WaitingExecutor> replacement = new WaitingExecutor;
    queue->AddExecutor(new ReplacingExecutor(queue, replacement));
    if (animationFinished) queue->AnimationFinished(); else queue->Run();
    Require(queue->GetFrontExecutor() == replacement.GetPtr(),
        "callback replacement was removed from the action queue");
    Require(queue->GetState() == NWorld::CCommandExecute::RUNNING,
        "callback replacement incorrectly finished the action queue");
}

class InventoryFirearm: public NAI::CAIFireArmsWeapon
{
    OBJECT_NOCOPY_METHODS(InventoryFirearm);
public:
    void SetItem(NRPG::CWeaponItem* item) { pWeaponItem = item; }
};

static void InvalidAIWeapon()
{
    CObj<NAI::AIUnitFixture> owner = new NAI::AIUnitFixture;
    CObj<NAI::AIUnitFixture> target = new NAI::AIUnitFixture;
    CObj<NAI::CAIInventory> inventory = new NAI::CAIInventory;
    CObj<InventoryFirearm> firearm = new InventoryFirearm;
    inventory->RestoreItem(firearm);
    // Inject the inventory's owner through its existing serialized field.
    CMemoryStream stream;
    CObj<NAI::IAIUnit> ownerReference = owner.GetPtr();
    { CStructureSaver saver(stream, CStructureSaver::WRITE); saver.Add(2, &ownerReference); }
    stream.Seek(0);
    CObj<NAI::IAIUnit> loadedOwner;
    { CStructureSaver saver(stream, CStructureSaver::READ);
      saver.Add(2, &loadedOwner);
      static_cast<CObjectBase*>(inventory.GetPtr())->operator&(saver); }
    owner = dynamic_cast<NAI::AIUnitFixture*>(loadedOwner.GetPtr());
    Require(IsValid(owner), "fixture inventory has no live owner");
    for (bool stale : {false, true}) {
        CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem;
        firearm->SetItem(stale ? weapon.GetPtr() : nullptr);
        weapon = nullptr;
        Require(!IsValid(firearm->GetItem()), "fixture weapon should be missing or invalid");
        int cover = 123, quality = 123, hit = 123;
        NDb::EShootMode mode = NDb::SM_Snap;
        auto* best = inventory->GetBestFireArms(NAI::SUnitPosition(), target, 30,
            &cover, &quality, &mode, &hit);
        Require(best == nullptr && cover == 0 && hit == 0 && quality == -0xFFFF,
            "invalid-only AI inventory produced a weapon quote");
        Require(owner->coverCalls == 0, "invalid AI weapon entered attack evaluation");
    }
}

class FirearmTypeProbe: public NRPG::CWeaponItem
{
    OBJECT_NOCOPY_METHODS(FirearmTypeProbe);
public:
    NDb::EWeaponType GetWeaponType() const override {
        Require(!IsRefInvalid(), "invalid firearm reached weapon-type lookup");
        return NDb::WT_PISTOL;
    }
};

class MeleeTypeProbe: public NRPG::CMeleeWeaponItem
{
    OBJECT_NOCOPY_METHODS(MeleeTypeProbe);
public:
    NDb::EWeaponType GetWeaponType() const override {
        Require(!IsRefInvalid(), "invalid melee weapon reached weapon-type lookup");
        return NDb::WT_KNIFE;
    }
};

static void StaleWeaponReferences(int kind)
{
    CObj<NRPG::CUnit> unit = new NRPG::CUnit;
    unit->pInventory = NRPG::CreateInventory(unit);
    if (kind < 2) {
        CObj<FirearmTypeProbe> personal = new FirearmTypeProbe;
        Require(unit->pInventory->Equip(NDb::SLOT_1, personal) && unit->pInventory->Activate(NDb::SLOT_1),
            "could not equip the weapon-reference fixture");
        Require(unit->GetWeaponType() == NDb::WT_PISTOL, "live personal weapon was not selected");
        if (kind == 0) {
            CObj<FirearmTypeProbe> cannon = new FirearmTypeProbe;
            unit->pCannonItem = cannon;
            cannon = nullptr;
            Require(!IsValid(unit->pCannonItem), "fixture cannon must be invalid");
            Require(unit->GetWeaponItem() == personal.GetPtr(), "invalid mounted weapon masked the equipped firearm");
        } else {
            personal->Invalidate();
            Require(unit->GetWeaponType() == NDb::WT_DEFAULT, "invalid firearm changed the unit's weapon type");
        }
    } else {
        CObj<MeleeTypeProbe> melee = new MeleeTypeProbe;
        unit->pDefaultWeapon = melee;
        Require(unit->GetWeaponType() == NDb::WT_KNIFE, "live default melee weapon was not selected");
        melee->Invalidate();
        Require(unit->GetWeaponType() == NDb::WT_DEFAULT, "invalid melee weapon changed the unit's weapon type");
    }
}

class LockerActor: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(LockerActor);
public:
    bool active = true;
    bool shell = false;
    bool CanFight() const override { return active; }
    bool IsEmptyPK() const override { return shell; }
};

class ColouringNetworkProbe: public NAI::CPathNetwork
{
    OBJECT_NOCOPY_METHODS(ColouringNetworkProbe);
public:
    bool allowRebuild = true;
    void LockSelected(const list<CObjectBase*>& selected) override {
        Require(allowRebuild,
            "recolour job started changing occupancy for a missing or invalid navigation cache");
        CPathNetwork::LockSelected(selected);
    }
};

static void NavigationColouringOccupancy(int unavailable = 0)
{
    CObj<ColouringNetworkProbe> network = new ColouringNetworkProbe;
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
        else layer->pColourer->Invalidate();
        Require(!IsValid(network->GetColourer(0)), "unavailable colouring fixture retained a valid cache");
        network->allowRebuild = false;
        for (int step = 0; step < 4 && !job->IsJobFinished(); ++step) job->DoJob();
        Require(job->IsJobFinished(), "recolouring job did not finish after skipping an unavailable cache");
        Require(network->IsLocked(occupied, true) && layer->tiles[3][3].nLocks == 1
                && network->GetWhoLocksThisPlace(occupied) == actor.GetPtr(),
            "skipping an unavailable colourer altered a live occupancy lock");
        return;
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
    auto executor = equipment(1);
    executor->AnimationFinished();
    const bool reset = !actor->IsWalkingWithoutWeapon();
    actor->SetWalkWithoutWeapon(true); // A newer script request while the animation completes.
    executor->AnimationFinished();
    Require(reset, "explicit equipment activation retained the scripted holster flag");
    Require(actor->IsWalkingWithoutWeapon() && executor->GetState() == NWorld::CCommandExecute::FINISHED,
        "equipment completion overwrote a later scripted holster request");
    CObj<NRPG::IInventoryItem> removed = actor->GetUnitRPG()->GetInventory()->TakeOff(slot);
    auto empty = equipment(1);
    empty->AnimationFinished();
    Require(actor->IsWalkingWithoutWeapon() && empty->GetState() == NWorld::CCommandExecute::FINISHED,
        "empty equipment slot cancelled scripted holstering without activating an item");
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
        RunScript(script, "UnitWearPK(upstreamActor, upstreamShell)");
        Require(actor->orders > 0, "UnitWearPK rejected an empty shell because it cannot fight");
        const int before = actor->orders;
        RunScript(script, "UnitWearPK(upstreamActor, upstreamActor)");
        Require(actor->orders == before, "UnitWearPK accepted a living soldier as a shell");
        CObj<NWorld::CPlayer> owner = new NWorld::CPlayer;
        CObj<NAI::CAICommander> commander = new NAI::CAICommander;
        owner->SetCommander(commander);
        actor->owner = owner; shell->owner = owner;
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
    CObj<NDb::CPanzerklein> suit = new NDb::CPanzerklein;
    suit->bSingleSlot = false;
    inventory->SetPanzerklein(suit, nullptr);
    Require(CanRemovePKWeapon(inventory.GetPtr(), 0), "removable PK equipment was restricted");
    suit->bSingleSlot = true;
    Require(!CanRemovePKWeapon(inventory.GetPtr(), 0), "manual removal of a fixed PK weapon is allowed");
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

static void CriticalIDs(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    CObj<NRPG::CUnit> unit = new NRPG::CUnit;
    CObj<NRPG::IUnitMission> mission = NRPG::CreateUnit(unit);
    for (bool enable : {false, true}) {
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE);
          std::vector<int> empty; writer.Add(20, &empty); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ);
          static_cast<CObjectBase*>(mission.GetPtr())->operator&(reader); }
        if (enable) mission->EnableCriticals(); else mission->DisableCriticals();
        CMemoryStream result;
        { CStructureSaver writer(result, CStructureSaver::WRITE);
          static_cast<CObjectBase*>(mission.GetPtr())->operator&(writer); }
        result.Seek(0);
        std::vector<int> states;
        { CStructureSaver reader(result, CStructureSaver::READ); reader.Add(20, &states); }
        Require(states.size() == NDb::N_CRIT_TYPES, "critical toggle did not repair a restored short state vector");
        for (int state : states)
            Require(state == (enable ? NRPG::CS_ENABLED : NRPG::CS_DISABLED), "critical toggle left a stale status");
    }
    Require(int(NDb::C_PANZERKLEIN_AXIS) == 16 && int(NDb::C_PANZERKLEIN_BROKEN) == 19,
        "PK critical identifiers do not match the retail database and save format");
}

class PathProbeExecutor: public NWorld::CCommandExecute
{
    OBJECT_BASIC_METHODS(PathProbeExecutor);
public:
    static int probes;
    bool running = false;
    int cost = 0;
    int operator&(CStructureSaver& saver) override {
        saver.Add(2, &running); saver.Add(3, &cost); return 0;
    }
    int GetStartAP() const override { return cost; }
    bool IsExecuting() override { return running; }
    NWorld::CPathConflictsRemover* GetPathConflictsRemover() override { ++probes; return nullptr; }
};
int PathProbeExecutor::probes = 0;
REGISTER_SAVELOAD_CLASS(0x7f550101, PathProbeExecutor);

static void IdlePathBlocker(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && unit->CanFight()) { actor = unit; break; }
    }
    Require(actor != nullptr, "path blocker fixture has no actor");
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    Require(!world->IsRealTime() && actor->CanSpendAP(0), "path blocker fixture is not in its turn");
    for (int state = 0; state < 3; ++state) {
        CObj<PathProbeExecutor> executor = new PathProbeExecutor;
        executor->running = state != 0;
        executor->cost = state == 2 ? 100000 : 0;
        CMemoryStream saved;
        { CStructureSaver saver(saved, CStructureSaver::WRITE); saver.Add(7, &executor); }
        saved.Seek(0);
        { CStructureSaver saver(saved, CStructureSaver::READ);
          static_cast<CObjectBase*>(actor)->operator&(saver); }
        PathProbeExecutor::probes = 0;
        actor->GetPathConflictsRemover();
        Require(PathProbeExecutor::probes == (state == 1 ? 1 : 0),
            state == 0 ? "idle unit promises to free its movement tile" :
            state == 2 ? "unaffordable move promises to free its movement tile" : "running affordable mover was ignored");
    }
}

static void HiddenPortraitAck()
{
    CObj<AttackMission> mission = new AttackMission;
    CObj<NUI::CUnitPanel> panel = new NUI::CUnitPanel(NUI::SWindowInfo(), mission);
    for (int selected : {0, 2}) {
        mission->selected = selected;
        CObj<NUI::CAckEvent> event = new NUI::CAckEvent;
        panel->PlayAckEvent(STime(100), event);
        Require(event->IsReady(), "voice acknowledgement waits forever for a hidden portrait");
        const STime end = event->GetEndTime();
        panel->PlayAckEvent(STime(200), event);
        Require(event->GetEndTime() == end, "releasing a hidden portrait extended acknowledgement lifetime");
        event->Cancel();
        panel->PlayAckEvent(STime(300), event);
        Require(event->IsComplete(STime(300)), "hidden portrait revived a cancelled acknowledgement");
    }
}

static void ClueInitialization()
{
    CObj<NDb::CDBScenarioClue> record = new NDb::CDBScenarioClue;
    for (unsigned char pattern : {0xff, 0x11}) {
        void* memory = ::operator new(sizeof(NScenario::CScenarioClue));
        std::memset(memory, pattern, sizeof(NScenario::CScenarioClue));
        CObj<NScenario::CScenarioClue> clue = new(memory) NScenario::CScenarioClue(record, 1);
        Require(clue->GetTemplateID() == 0, "new clue inherits a stale map template ID");
    }
}

template<class Player>
static auto DropInStore(Player* player, const CTPoint<int>& point, NRPG::IInventoryItem* item, int)
    -> decltype(player->PlaceStoreItem(point, item), void())
{ player->PlaceStoreItem(point, item); }
template<class Player>
static void DropInStore(Player* player, const CTPoint<int>&, NRPG::IInventoryItem* item, long)
{ player->PlaceStoreItem(item); }

static void StoreDropPosition()
{
    CObj<NRPG::CGlobalPlayer> global = new NRPG::CGlobalPlayer;
    CObj<NWorld::CPlayer> player = new NWorld::CPlayer(L"Store fixture", nullptr, global, 0);
    CObj<NDb::CRPGItem> record = new NDb::CRPGItem;
    record->sSize = CTPoint<int>(1, 1);
    record->nWeight = 0;
    CObj<NRPG::IInventoryItem> first = NRPG::CreateDummyItem(record);
    DropInStore(player.GetPtr(), CTPoint<int>(5, 5), first, 0);
    auto* items = player->GetStoreItems();
    Require(items->size() == 1 && items->front().sPos == CTPoint<int>(5, 5),
        "store discarded a valid manual drop position");
    CObj<NRPG::IInventoryItem> second = NRPG::CreateDummyItem(record);
    DropInStore(player.GetPtr(), CTPoint<int>(5, 5), second, 0);
    Require(items->size() == 2 && items->back().sPos != CTPoint<int>(5, 5),
        "occupied store cell did not use a free fallback");
    CObj<NRPG::IInventoryItem> third = NRPG::CreateDummyItem(record);
    DropInStore(player.GetPtr(), CTPoint<int>(-1, -1), third, 0);
    Require(items->size() == 3 && items->back().sPos.x >= 0 && items->back().sPos.y >= 0,
        "automatic store placement lost the returned item");
}

class TransferActor: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(TransferActor);
public:
    int operator&(CStructureSaver&) override { return 0; }
};
REGISTER_SAVELOAD_CLASS(0x7f550102, TransferActor);

class TransferMission: public AttackMission
{
    OBJECT_BASIC_METHODS(TransferMission);
public:
    static CPtr<NWorld::CUnit> actor;
    static CObj<NWorld::CCmd> command;
    int operator&(CStructureSaver&) override { return 0; }
    void Command(NWorld::CUnit* unit, NWorld::CCmd* order, bool) override {
        actor = unit; command = order;
    }
};
CPtr<NWorld::CUnit> TransferMission::actor;
CObj<NWorld::CCmd> TransferMission::command;
REGISTER_SAVELOAD_CLASS(0x7f550103, TransferMission);

class TransferTracker: public NGame::CUnitTracker
{
    OBJECT_BASIC_METHODS(TransferTracker);
public:
    CObj<TransferActor> unit;
    int operator&(CStructureSaver& saver) override { saver.Add(2, &unit); return 0; }
    NWorld::CUnit* GetUnit() const override { return unit; }
};
REGISTER_SAVELOAD_CLASS(0x7f550104, TransferTracker);

static void InventoryTransferActor(bool backpack)
{
    CObj<TransferMission> mission = new TransferMission;
    CObj<TransferActor> source = new TransferActor, destination = new TransferActor;
    CObj<NUI::CSlot> slot = dynamic_cast<NUI::CSlot*>(pSSClasses->CreateObject(
        backpack ? int(0xB0521142) : int(0xB0521155)));
    Require(slot != nullptr, "inventory slot class is not registered");
    CObj<TransferTracker> tracker = new TransferTracker;
    tracker->unit = destination;
    CMemoryStream state;
    { CStructureSaver saver(state, CStructureSaver::WRITE);
      saver.Add(2, &mission);
      if (backpack) {
          saver.Add(3, &tracker);
      } else {
          NDb::ESlot hand = NDb::SLOT_1;
          saver.Add(3, &hand); saver.Add(4, &destination);
      } }
    state.Seek(0);
    { CStructureSaver saver(state, CStructureSaver::READ);
      static_cast<CObjectBase*>(slot.GetPtr())->operator&(saver); }
    if (backpack) {
        CMemoryStream base;
        CObj<NUI::CWindow> view = new NUI::CWindow(NUI::SWindowInfo());
        view->SetSize(NUI::SPoint(100, 100));
        { CStructureSaver saver(base, CStructureSaver::WRITE);
          int width = 10, height = 10;
          saver.Add(3, &width); saver.Add(4, &height); saver.Add(8, &view); }
        base.Seek(0);
        CStructureSaver saver(base, CStructureSaver::READ);
        slot->NUI::CSlot::operator&(saver);
    }
    slot->SetPosition(NUI::SPoint(0, 0));
    slot->SetSize(NUI::SPoint(100, 100));
    CObj<NDb::CRPGItem> record = new NDb::CRPGItem;
    record->sSize = CTPoint<int>(1, 1);
    CObj<NRPG::IInventoryItem> item = NRPG::CreateDummyItem(record);
    NWorld::SItem drag(source, NWorld::SItem::HAND, item);
    slot->Place(10, 10, drag);
    Require(TransferMission::actor == source, "inventory transfer executes on the recipient instead of the hand owner");
    auto* order = dynamic_cast<NWorld::CCmdMoveInventoryItem*>(TransferMission::command.GetPtr());
    Require(order && order->GetSource().pUnit == source && order->GetTarget().pUnit == slot->GetUnit(),
        "inventory transfer lost its source or destination");
    drag.pUnit = nullptr;
    slot->Place(10, 10, drag);
    Require(TransferMission::actor == slot->GetUnit(), "ownerless hand did not use the destination actor");
    TransferMission::actor = nullptr; TransferMission::command = nullptr;
}

static void GeometryHashBuckets()
{
    int buckets[4096] = {};
    GeometryTestHash hash;
    for (int i = 0; i < 4096; ++i)
        ++buckets[static_cast<unsigned>(hash(CVec3(i * .25f, 0.f, 0.f))) & 4095];
    int occupied = 0, largest = 0;
    for (int count : buckets) { occupied += count != 0; largest = Max(largest, count); }
    Require(occupied > 2000 && largest < 32,
        "grid-aligned geometry collapses into masked hash buckets");
    Require(hash(CVec3(0.f, 0.f, 0.f)) == hash(CVec3(-0.f, 0.f, 0.f)),
        "equal signed-zero positions have different merge hashes");
    std::vector<CVec3> positions = {VNULL3, CVec3(-0.f, 0.f, 0.f), CVec3(1,2,3), CVec3(1,2,3), CVec3(2,1,3)};
    std::vector<WORD> matches;
    NGScene::MergePositions(&matches, &positions);
    Require(positions.size() == 3 && matches == std::vector<WORD>({0,0,1,1,2}),
        "geometry hash changed vertex merge identity or ordering");
}

static void ImmediateClueNotification(const char* databasePath, const char* resourcePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NScenario::CScenarioTracker> tracker = NScenario::CreateScenarioTracker(1);
    CPtr<NScenario::CScenarioClue> selected, other;
    auto* table = NDatabase::GetTable<NDb::CDBScenarioClue>();
    CDBIterator<NDb::CDBScenarioClue> it(*table);
    while (it.MoveNext()) {
        auto* record = it.Get();
        auto* clue = tracker->GetClueByName(record->sSmallDescription);
        if (!clue) continue;
        if (!selected && record->bGiveImmediately && clue->GetObjectiveByType(NDb::OT_CAPTURE) && !tracker->IsClueFound(clue))
            selected = clue;
        else if (!other) other = clue;
    }
    Require(selected && other && selected != other, "campaign fixture has no immediate capturable clue");
    other->SetJustFound(true);
    tracker->CheatTakeClue(selected, true);
    Require(tracker->IsClueFound(selected), "immediate clue capture did not advance the scenario");
    Require(!selected->IsJustFound(), "immediate clue is announced again on the chapter map");
    Require(other->IsJustFound(), "clearing an immediate clue discarded another pending notification");
}

static void ImmediateCluePickup(const char* databasePath, const char* resourcePath, bool occupiedHand)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight()) { actor = unit; break; }
    }
    Require(actor != nullptr, "clue fixture has no human actor");
    auto* tracker = world->GetGlobalGame()->pScenarioTracker.GetPtr();
    auto* clue = tracker->GetClueByName("Find_Cipher_key");
    Require(clue && clue->GetDBClue()->bGiveImmediately && !tracker->IsClueFound(clue), "immediate clue fixture is unavailable");
    CObj<NRPG::IInventoryItem> item = NRPG::CreateClueItem(NDb::GetRPGItem(clue->GetDBClue()->nItemID));
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    CTPoint<int> position;
    Require(inventory->FindPlace(item, &position) && inventory->Place(position, item), "clue source does not fit the fixture backpack");
    actor->nClueCount = 1;
    CObj<NRPG::IInventoryItem> held;
    if (occupiedHand) {
        held = NRPG::CreateDummyItem(item->GetDBItem());
        actor->GetTBSPlayer()->SetInHandItem(NWorld::SItem(actor, NWorld::SItem::HAND, held));
        CObj<NWorld::CCmdMoveInventoryItem> ordinaryCommand = new NWorld::CCmdMoveInventoryItem(
            NWorld::SItem(actor, NWorld::SItem::BACKPACK, held), NWorld::SItem(actor, NWorld::SItem::HAND));
        CObj<NWorld::CExecMoveInventoryItem> ordinary = new NWorld::CExecMoveInventoryItem(actor, ordinaryCommand);
        Require(ordinary->CanDoIt(actor->GetPosition()) != NWorld::UCR_OK,
            "ordinary item with a clue DB identity bypassed occupied-hand checks");
    }
    CObj<NWorld::CCmdMoveInventoryItem> command = new NWorld::CCmdMoveInventoryItem(
        NWorld::SItem(actor, NWorld::SItem::BACKPACK, item), NWorld::SItem(actor, NWorld::SItem::HAND));
    CObj<NWorld::CExecMoveInventoryItem> executor = new NWorld::CExecMoveInventoryItem(actor, command);
    Require(executor->CanDoIt(actor->GetPosition()) == NWorld::UCR_OK, "an immediate clue requires an empty drag hand");
    executor->Run();
    Require(tracker->IsClueFound(clue), "immediate clue transfer did not advance the campaign on pickup");
    NWorld::SItem after;
    bool hasHand = actor->GetTBSPlayer()->GetInHandItem(&after);
    Require(occupiedHand ? hasHand && after.pItem == held : !hasHand,
        "immediate clue entered the drag hand or replaced its previous item");
    Require(actor->nClueCount == 0, "consumed immediate clue retained a carrier count");
    for (const auto& entry : inventory->GetItems())
        Require(entry.pItem != item, "consumed clue remained in the backpack");
    int modals = 0;
    while (auto* raw = world->GetUICommand()) {
        CObj<NWorld::CUICmd> event = raw;
        auto* show = dynamic_cast<NWorld::CUICmdShowClue*>(event.GetPtr());
        if (show && show->pClue == clue) ++modals;
    }
    Require(modals == 1, "immediate clue pickup did not open exactly one clue modal");
    if (auto* goal = clue->GetGoal()) for (auto& task : goal->GetTasks()) {
        auto tag = task->GetDBTask()->eTag;
        if (tag == NDb::TT_FIND_ITEM || tag == NDb::TT_TAKE_ITEM || tag == NDb::TT_CARRY_OUT_ITEM)
            Require(task->GetState() == NScenario::TS_COMPLETED, "immediate clue task did not complete");
    }
}

static CPtr<NAnimation::CASequence> ReadAnimationSequence(NAnimation::CSkeletonAnimator* animator)
{
    CPtr<NAnimation::CASequence> sequence;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* object) {
        if (auto* found = dynamic_cast<NAnimation::CASequence*>(object)) sequence = found;
        return false;
    };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(animator)->operator&(writer); }
    Require(IsValid(sequence), "fixture has no current animation sequence");
    return sequence;
}

static void PKShellWorld(const char* databasePath, const char* resourcePath, bool extractPilot, bool checkModel = false, int checkFeedback = 0)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer *pilot = nullptr, *rescuer = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (!unit || !unit->CanFight()) continue;
        if (!unit->IsAIUnit()) pilot = unit; else if (!rescuer) rescuer = unit;
    }
    Require(pilot && rescuer, "dead-pilot fixture has no pilot or rescuer");
    NDb::CRPGPers* suitRecord = nullptr;
    auto* table = NDatabase::GetTable<NDb::CRPGPers>();
    CDBIterator<NDb::CRPGPers> it(*table);
    while (it.MoveNext()) {
        auto* record = it.Get();
        if (record->pPanzerklein && !record->pPanzerklein->bHasNoHead && record->pModel) { suitRecord = record; break; }
    }
    Require(suitRecord != nullptr, "fixture database has no regular PK shell");
    auto* network = world->GetPathNetwork();
    std::vector<NAI::SPathPlace> places;
    network->GetNearPlaces(SSphere(pilot->GetPosition().GetCP(), 20.f), &places);
    NAI::SPathPlace anchor;
    bool found = false;
    for (auto place : places) {
        place.SetPose(NAI::CM_STAND); place.SetDirection(0);
        if (NAI::IsBigLockerPlacePassable(network, place)) { anchor = place; found = true; break; }
    }
    Require(found, "fixture has no unobstructed PK footprint");
    CObj<NWorld::CUnitServer> shell = world->AddUnitInGame(anchor, NRPG::CreateUnit(suitRecord), nullptr);
    Require(shell && shell->IsEmptyPK(), "could not create a PK shell");
    if (checkFeedback == 3) {
        Require(shell->WearAsPK(true), "turn-animation fixture could not prepare its shell");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell, false, false);
        Require(pilot->IsWearingPK(), "turn-animation fixture did not put on the suit");
        const int ap = pilot->GetRPG()->GetAP();
        auto sequence = ReadAnimationSequence(pilot->animator.GetSkeletonAnimator());
        const auto time = pilot->animator.GetTimeEnd();
        CPtr<NAnimation::CAnimator> before = sequence->GetAnimator(time);
        Require(IsValid(before), "fixture PK has no active animation track");
        pilot->animator.StartNewTurn(pilot->GetPosition());
        Require(sequence->GetAnimator(time) == before.GetPtr(),
            "new turn replaced the Panzerklein animation with a normal standing track");
        Require(pilot->GetRPG()->GetAP() == ap, "PK turn animation changed action points");
        pilot->FlipPanzerklein(nullptr, false, false);
        pilot->animator.PlaceUnit(pilot->GetPosition());
        sequence = ReadAnimationSequence(pilot->animator.GetSkeletonAnimator());
        const auto ordinaryTime = pilot->animator.GetTimeEnd();
        before = sequence->GetAnimator(ordinaryTime);
        pilot->animator.StartNewTurn(pilot->GetPosition());
        Require(sequence->GetAnimator(ordinaryTime) != before.GetPtr(),
            "ordinary human lost the original turn-start standing refresh");
        return;
    }
    if (checkFeedback) {
        CObj<NWorld::CUnitStateNormal> state = new NWorld::CUnitStateNormal(pilot);
        if (checkFeedback == 1) {
            CObj<NWorld::CCmdHeal> command = new NWorld::CCmdHeal(nullptr);
            for (int value = NDb::C_PANZERKLEIN_AXIS; value <= NDb::C_PANZERKLEIN_TERRORS_HWG; ++value) {
                const auto status = static_cast<NDb::ECritical>(value);
                pilot->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_ANY, status));
                Require(pilot->GetUnitRPG()->HasCritical(status), "PK feedback fixture did not apply a status");
                auto result = NWorld::UCR_NO_TARGET;
                Require(state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_PK_BAN,
                    "PK command ban reported an ordinary critical-injury rejection");
                Require(pilot->GetUnitRPG()->RemoveCritical(status), "PK fixture status could not be removed");
            }
            pilot->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD, NDb::C_BLIND, 3));
            auto result = NWorld::UCR_NO_TARGET;
            Require(state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_CRITICALS_BAN,
                "ordinary injury incorrectly reported a PK restriction");
            pilot->GetUnitRPG()->RemoveCritical(NDb::C_BLIND);
            result = NWorld::UCR_NO_TARGET;
            Require(!state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_NO_TARGET,
                "successful command probe did not preserve its prior result");
            return;
        }
        Require(shell->WearAsPK(true), "PK feedback fixture could not prepare its shell");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell, false, false);
        Require(pilot->IsWearingPK(), "PK feedback fixture did not put on the suit");
        const int ap = pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
        for (auto pose : {NAI::CRAWL, NAI::RUN}) {
            CObj<NWorld::CCmdWishPose> command = new NWorld::CCmdWishPose(pose);
            auto result = NWorld::UCR_NO_TARGET;
            Require(state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_PK_BAN,
                "worn-PK pose ban reported an ordinary critical-injury rejection");
        }
        CObj<NWorld::CCmdWishPose> allowed = new NWorld::CCmdWishPose(NAI::WALK);
        auto result = NWorld::UCR_NO_TARGET;
        Require(!state->IsCriticalsFailCommand(allowed, &result) && result == NWorld::UCR_NO_TARGET,
            "allowed PK pose did not preserve its prior eligibility result");
        Require(pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
            "PK eligibility feedback changed action points");
        return;
    }

    if (checkModel) {
        auto verifyPrivateModel = [](NWorld::CUnitServer* unit, NDb::CModel* source) {
            auto* live = unit->GetModel();
            Require(live && live != source, "PK mode change reused the shared RPG model");
            Require(live->pGeometry.GetPtr() == source->pGeometry.GetPtr() &&
                live->pSkeleton.GetPtr() == source->pSkeleton.GetPtr(), "private PK model lost its geometry or skeleton");
            auto geometry = source->pGeometry;
            Require(IsValid(geometry), "PK model fixture has no geometry");
            live->pGeometry = nullptr;
            Require(source->pGeometry.GetPtr() == geometry.GetPtr(), "live model mutation changed the RPG source model");
            live->pGeometry = geometry;
        };
        auto* humanModel = pilot->GetUnitRPG()->GetRPGUnit()->pModel.GetPtr();
        auto* suitModel = shell->GetUnitRPG()->GetRPGUnit()->pModel.GetPtr();
        Require(shell->WearAsPK(true), "could not prepare the model ownership fixture");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell, false, false);
        verifyPrivateModel(pilot, suitModel);
        pilot->FlipPanzerklein(nullptr, false, false);
        verifyPrivateModel(pilot, humanModel);
        return;
    }
    if (!extractPilot) {
        NAI::SPosition target = shell->GetPosition().pos;
        bool targetFound = false;
        for (auto place : places) {
            place.SetPose(NAI::CM_STAND); place.SetDirection(0);
            if (!NAI::IsBigLockerPlacePassable(network, place)) continue;
            target.p = place;
            if (fabs2(target.GetCP() - shell->GetPosition().GetCP()) > 9.f) { targetFound = true; break; }
        }
        Require(targetFound, "fixture has no distant PK waypoint");
        CObj<NDb::CWaypointName> name = new NDb::CWaypointName;
        name->szName = "upstream_pk";
        CObj<CMapWaypoint> authored = new CMapWaypoint;
        authored->pName = name;
        authored->pos.ptPos = target.GetCP();
        world->AddWaypoint(authored);
        auto* waypoint = world->GetWaypoint(name->szName);
        Require(waypoint != nullptr, "PK waypoint was not registered");
        waypoint->pos = target;
        CObj<NScript::CScript> script = NScript::CreateScript(world);
        NScript::luaPushCPtr(*script, shell); script->SetGlobal("upstreamShell");
        RunScript(script, "UnitSetToWaypoint(upstreamShell, 'upstream_pk')");
        Require(shell->GetPosition().GetPose() == NAI::CRAWL, "scripted PK relocation lost the parked pose");
        Require(fabs2(shell->GetPosition().GetCP() - target.GetCP()) < .0025f, "scripted empty PK did not reach its waypoint");
        return;
    }
    Require(shell->WearAsPK(true), "could not prepare a worn PK shell");
    pilot->SetPosition(shell->GetPosition());
    pilot->FlipPanzerklein(shell, false, false);
    pilot->GetUnitRPG()->GetRPGUnit()->Kill();
    pilot->SetState(new NWorld::CUnitStateDeath(pilot));
    pilot->SetPosition(pilot->GetPosition());
    Require(pilot->IsDead() && pilot->IsWearingPK() && network->IsLocked(anchor, true),
        "dead pilot did not retain the worn footprint");
    CObj<NWorld::CExecCorpse> spill = new NWorld::CExecCorpse(rescuer, pilot, true);
    spill->Run();
    Require(!pilot->IsWearingPK() && shell->IsEmptyPK(), "corpse extraction left the suit occupied");
    Require(shell->GetPosition().pos.p.GetX() == anchor.GetX() && shell->GetPosition().pos.p.GetY() == anchor.GetY(),
        "restoring the shell treated its dead pilot as an obstacle and relocated it");
}

static bool RunRegressionMode(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "temporary-groups")) { TemporaryGroups(); return true; }
    if (!std::strcmp(mode, "scroll-initialization")) { ScrollInitialization(); return true; }
    if (!std::strcmp(mode, "attack-location")) { AttackLocationRefresh(); return true; }
    if (!std::strcmp(mode, "bind-initialization")) { BindInitialization(); return true; }
    if (!std::strcmp(mode, "flying-restoration")) { FlyingRestoration(); return true; }
    if (!std::strcmp(mode, "sniping-aim")) { SnipingAim(); return true; }
    if (!std::strcmp(mode, "decal-rebuild")) { DecalRebuild(); return true; }
    if (!std::strcmp(mode, "decal-projection")) { DecalProjection(); return true; }
    if (!std::strcmp(mode, "partial-reference-import") && argc == 3) { PartialReferenceImport(argv[2]); return true; }
    if (!std::strcmp(mode, "permanent-clue-single-map")) { PermanentClueSingleMap(); return true; }
    if (!std::strcmp(mode, "deferred-click-world")) { DeferredClickWorld(); return true; }
    if (!std::strcmp(mode, "point-light-without-scene")) { PointLightWithoutScene(); return true; }
    if (!std::strcmp(mode, "dynamic-ambient-fallback")) { DynamicAmbientFallback(); return true; }
    if (!std::strcmp(mode, "inventory-without-uniform")) { InventoryWithoutUniform(); return true; }
    if (!std::strcmp(mode, "corpse-backpack-drop") && argc == 4) { CorpseBackpackDrop(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "scripted-dialogue-priority") && argc == 4) { ScriptedDialoguePriority(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "point-light-save-layout")) { PointLightSaveLayout(); return true; }
    if (!std::strcmp(mode, "legacy-point-light-save-layout")) { PointLightSaveLayout(true); return true; }
    if (!std::strcmp(mode, "point-light-shadow-flag")) { PointLightShadowFlag(); return true; }
    if (!std::strcmp(mode, "global-point-light-shadow-flag")) { GlobalPointLightShadowFlag(); return true; }
    if (!std::strcmp(mode, "point-light-radiance")) { PointLightRadiance(); return true; }
    if (!std::strcmp(mode, "tiny-point-light-cache")) { TinyPointLightCache(); return true; }
    if (!std::strcmp(mode, "console-visibility-toggle")) { ConsoleVisibilityToggle(); return true; }
    if (!std::strcmp(mode, "ai-reaction-reset-contacts")) { AIReactionResetContacts(); return true; }
    if (!std::strcmp(mode, "ai-reaction-value-latch") && argc == 4) { AIReactionValueLatch(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "script-ai-activation") && argc == 4) { ScriptAIActivation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-event-collection-locks") && argc == 4) { AIEventCollectionLocks(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "mission-overlay-readiness")) { MissionOverlayReadiness(); return true; }
    if (!std::strcmp(mode, "ai-player-mission-readiness")) { AIPlayerMissionReadiness(); return true; }
    if (!std::strcmp(mode, "sound-icon-visibility")) { OverlayIconVisibility(false); return true; }
    if (!std::strcmp(mode, "trap-icon-visibility")) { OverlayIconVisibility(true); return true; }
    if (!std::strcmp(mode, "enemy-icon-visibility")) { Require(argc == 4, "enemy icons require db and res"); EnemyIconVisibility(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "finished-music-silence")) { Require(argc == 3, "finished music requires db"); FinishedMusicSilence(argv[2]); return true; }
    if (!std::strcmp(mode, "holstered-heavy-weapon-mesh")) { Require(argc == 4, "heavy mesh requires db and res"); HolsteredHeavyWeaponMesh(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "script-holster-persistence")) { Require(argc == 4, "script holster requires db and res"); ScriptHolsterPersistence(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "movement-holster-policy")) { Require(argc == 4, "movement holster requires db and res"); MovementHolsterPolicy(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "short-inactive-movement")) { Require(argc == 4, "inactive movement requires db and res"); MovementHolsterPolicy(argv[2], argv[3], MovementHolsterCheck::ShortInactive); return true; }
    if (!std::strcmp(mode, "hidden-active-movement")) { Require(argc == 4, "hidden active movement requires db and res"); MovementHolsterPolicy(argv[2], argv[3], MovementHolsterCheck::HiddenActive); return true; }
    if (!std::strcmp(mode, "equipment-holster-reset")) { Require(argc == 4, "equipment activation requires db and res"); ExplicitWeaponActivation(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "movement-holster-reset")) { Require(argc == 4, "movement activation requires db and res"); ExplicitWeaponActivation(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "movement-first-step-wait")) { Require(argc == 4, "movement wait requires db and res"); MovementFirstStepWaitTimer(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "movement-terminal-finished")) { Require(argc == 4, "terminal movement requires db and res"); MovementTerminalCompletion(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "movement-terminal-failed")) { Require(argc == 4, "terminal movement requires db and res"); MovementTerminalCompletion(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "pain-voice-owner")) { Require(argc == 4, "pain voice requires db and res"); PainVoiceOwner(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "unconscious-pain-voice")) { Require(argc == 4, "pain eligibility requires db and res"); PainVoiceOwner(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "world-ui-refresh-latch")) { Require(argc == 4, "world UI refresh requires db and res"); WorldUIRefreshLatch(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "dialogue-camera-filter")) { CinematicCameraCommands(0); return true; }
    if (!std::strcmp(mode, "movie-camera-filter")) { CinematicCameraCommands(1); return true; }
    if (!std::strcmp(mode, "fade-camera-filter")) { CinematicCameraCommands(2); return true; }
    if (!std::strcmp(mode, "completed-zone-recommendation")) { CompletedZoneRecommendation(); return true; }
    if (!std::strcmp(mode, "perk-tooltip-parameters")) { PerkTooltipParameters(); return true; }
    if (!std::strcmp(mode, "mechanical-step-sound")) { MechanicalStepSound(); return true; }
    if (!std::strcmp(mode, "nearest-invalid")) { InvalidNearestPosition(); return true; }
    if (!std::strcmp(mode, "lua-empty-default")) { EmptyLuaDefault(); return true; }
    if (!std::strcmp(mode, "smart-aimer-boundary")) { SmartAimerBoundary(); return true; }
    if (!std::strcmp(mode, "nearest-empty")) { EmptyNearestPosition(); return true; }
    if (!std::strcmp(mode, "fresh-navigation-colouring")) { FreshNavigationColouring(); return true; }
    if (!std::strcmp(mode, "fresh-navigation-transitions")) { FreshNavigationTransitions(); return true; }
    if (!std::strcmp(mode, "navigation-colouring-occupancy")) { NavigationColouringOccupancy(); return true; }
    if (!std::strcmp(mode, "missing-navigation-colourer")) { NavigationColouringOccupancy(1); return true; }
    if (!std::strcmp(mode, "stale-navigation-colourer")) { NavigationColouringOccupancy(2); return true; }
    if (!std::strcmp(mode, "ragdoll-restoration")) { RagdollRestoration(); return true; }
    if (!std::strcmp(mode, "negative-mesh-bounds")) { NegativeMeshBounds(); return true; }
    if (!std::strcmp(mode, "sniper-targetless-probe") && argc == 4) { SniperHeldAim(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "sniper-held-aim") && argc == 4) { SniperHeldAim(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "person-clue-creation") && argc == 4) { PersonClueCreation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "melee-attack-cursor") && argc == 4) { MeleeAttackCursor(argv[2]); return true; }
    if (!std::strcmp(mode, "blocked-attack-caption") && argc == 4) { AttackCaptionWithoutHitChance(argv[2], false); return true; }
    if (!std::strcmp(mode, "reload-attack-caption") && argc == 4) { AttackCaptionWithoutHitChance(argv[2], true); return true; }
    if (!std::strcmp(mode, "exact-attack-preview-target") && argc == 4) { ExactAttackPreviewTarget(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "equal-group-attack-caption") && argc == 4) { GroupAttackPreview(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "mixed-group-attack-caption") && argc == 4) { GroupAttackPreview(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "missing-attack-target-caption") && argc == 4) { AttackCaptionWithoutHitChance(argv[2], false, true); return true; }
    if (!std::strcmp(mode, "restore-action-state") && argc == 4) { RestoreActionState(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "zone-reentry-action-state") && argc == 4) { RestoreActionState(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "restore-building-hulls") && argc == 4) { RestoreBuildingHulls(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "carried-corpse-stability") && argc == 4) { CarriedCorpseStability(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "replacement-person-inventory-clue") && argc == 4) { PersonClueCreation(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "inventory-clue-creation") && argc == 4) { InventoryClueCreation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "inventory-clue-count") && argc == 4) { InventoryClueCreation(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "inventory-clue-capacity") && argc == 4) { InventoryClueCreation(argv[2], argv[3], true, true); return true; }
    if (!std::strcmp(mode, "corpse-snap-failure") && argc == 4) { FailedCorpseSnap(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "movement-critical-probe") && argc == 4) { MovementCriticalProbe(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "latest-route-command") && argc == 4) { LatestRouteCommand(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "oversized-dialogue-line") && argc == 4) { OversizedDialogueLine(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "markup-dialogue-progress") && argc == 4) { OversizedDialogueLine(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "dialogue-normal-pagination") && argc == 4) { OversizedDialogueLine(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "movement-first-step-cancel") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "movement-next-step-cancel") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "movement-first-step-release") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 1, true); return true; }
    if (!std::strcmp(mode, "movement-next-step-release") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 2, true); return true; }
    if (!std::strcmp(mode, "hide-cooldown") && argc == 4) { HideCooldown(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-idle") && argc == 4) { IdleAI(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "mixed-hide-button") && argc == 4) { MixedActionButton(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "mixed-strafe-button") && argc == 4) { MixedActionButton(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "mounted-mode-button") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "mounted-weapon-submenu") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "mounted-weapon-mode-ui") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "effective-weapon-mode") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "render-removal")) { RenderRemoval(); return true; }
    if (!std::strcmp(mode, "render-pass-compaction")) { RenderPassCompaction(); return true; }
    if (!std::strcmp(mode, "state-lifetime")) { ReentrantStateLifetime(); return true; }
    if (!std::strcmp(mode, "mounted-mode-command")) { MountedModeCommand(); return true; }
    if (!std::strcmp(mode, "invalid-ai-weapon")) { InvalidAIWeapon(); return true; }
    if (!std::strcmp(mode, "stale-cannon-selection")) { StaleWeaponReferences(0); return true; }
    if (!std::strcmp(mode, "stale-firearm-type")) { StaleWeaponReferences(1); return true; }
    if (!std::strcmp(mode, "stale-melee-type")) { StaleWeaponReferences(2); return true; }
    if (!std::strcmp(mode, "stale-ai-reaction")) { StaleAIReaction(); return true; }
    if (!std::strcmp(mode, "overlapping-unit-locks")) { OverlappingUnitLocks(); return true; }
    if (!std::strcmp(mode, "invalid-unit-locks")) { InvalidUnitLocks(); return true; }
    if (!std::strcmp(mode, "dead-unit-locks-single") && argc == 4) { DeadUnitLocks(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "dead-unit-locks-moving") && argc == 4) { DeadUnitLocks(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "dead-unit-locks-selection") && argc == 4) { DeadUnitLocks(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "queue-lifetime")) { ReentrantQueueLifetime(false); return true; }
    if (!std::strcmp(mode, "queue-animation-lifetime")) { ReentrantQueueLifetime(true); return true; }
    if (!std::strcmp(mode, "pk-leave")) { ScriptPK(true); return true; }
    if (!std::strcmp(mode, "pk-wear")) { ScriptPK(false); return true; }
    if (!std::strcmp(mode, "pk-equipment")) { PKEquipmentTransfer(); return true; }
    if (!std::strcmp(mode, "pk-fixed-weapon")) { PKFixedWeaponPolicy(); return true; }
    if (!std::strcmp(mode, "door-collision")) { DoorCollision(); return true; }
    if (!std::strcmp(mode, "overlapping-doors")) { OverlappingDoors(); return true; }
    if (!std::strcmp(mode, "weapon-familiarity")) { WeaponFamiliarity(); return true; }
    if (!std::strcmp(mode, "clue-slots")) { ClueSlotSelection(); return true; }
    if (!std::strcmp(mode, "conclusions") && argc == 4) { ConclusionRouting(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "pk-placement")) { PKPlacement(); return true; }
    if (!std::strcmp(mode, "critical-ids") && argc == 3) { CriticalIDs(argv[2]); return true; }
    if (!std::strcmp(mode, "idle-blocker") && argc == 4) { IdlePathBlocker(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "hidden-portrait")) { HiddenPortraitAck(); return true; }
    if (!std::strcmp(mode, "clue-initialization")) { ClueInitialization(); return true; }
    if (!std::strcmp(mode, "store-drop")) { StoreDropPosition(); return true; }
    if (!std::strcmp(mode, "inventory-actor")) { InventoryTransferActor(true); return true; }
    if (!std::strcmp(mode, "slot-actor")) { InventoryTransferActor(false); return true; }
    if (!std::strcmp(mode, "geometry-hash")) { GeometryHashBuckets(); return true; }
    if (!std::strcmp(mode, "clue-notification") && argc == 4) { ImmediateClueNotification(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "clue-pickup") && argc == 4) { ImmediateCluePickup(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "clue-capacity") && argc == 4) { ImmediateCluePickup(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "dead-pilot") && argc == 4) { PKShellWorld(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "pk-waypoint") && argc == 4) { PKShellWorld(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "pk-critical-feedback") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 1); return true; }
    if (!std::strcmp(mode, "pk-pose-feedback") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 2); return true; }
    if (!std::strcmp(mode, "pk-turn-animation") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 3); return true; }
    if (!std::strcmp(mode, "pk-model-ownership") && argc == 4) { PKShellWorld(argv[2], argv[3], false, true); return true; }
    return false;
}

int main(int argc, char** argv)
{
    try {
        const char* mode = argc > 1 ? argv[1] : "temporary-groups";
        if (!RunRegressionMode(mode, argc, argv)) throw std::runtime_error("unknown upstream regression mode");
        std::printf("PASS: %s\n", mode);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
