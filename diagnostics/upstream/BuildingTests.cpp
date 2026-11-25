#include "TestSupport.h"
#include "../../Main/BuildingGrid.h"
#include "../../Main/BuildingInfo.h"
#include "../../Main/BuildingSchema.h"
#include "../../Main/MakeBuilding.h"
#include "../../Main/MakeBuildingInternal.h"
#include "../../Main/MapBuild.h"
#include "../../Main/GInit.h"
#include "../../Main/aiGrid.h"
#include "../../Main/aiMap.h"
#include "../../Main/aiStability.h"
#include "../../Main/A5Time.h"
#include "../../Main/rpgGlobal.h"
#include "../../Main/RPGGame.h"
#include "../../Main/RPGObject.h"
#include "../../Main/wMain.h"
#include "../../Main/wOSBase.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/RPGUnitMission.h"
#include "../../DBFormat/DataRPG.h"
#include "../../DBFormat/DataMap.h"
#include "../../DBFormat/DataObject.h"
#include "../../DBFormat/DataFormat.h"
#include "../../Misc/BasicShare.h"
#include <cstdio>
#include <cstring>

namespace NGScene
{
extern CBasicShare<int, NBuilding::CBuildInfoLoader> shareBuildings;
}

namespace NBuilding
{
void MarkCellarWalls(CArray2D<bool>* walls, const CArray2D<bool>& cellar, const CArray2D<float>& bottom);
}

namespace UpstreamTests
{
namespace
{
void ObjectDestroyStageVitality()
{
    struct Case { int stages, maximum, stage, expected; };
    for (auto test : {Case{4, 15, 1, 11}, Case{3, 15, 1, 9}, Case{3, 12, 1, 8},
        Case{3, 15, 0, 15}, Case{0, 15, 0, 15}, Case{3, 15, 3, -1}}) {
        CObj<NRPG::IObject> object = NRPG::CreateObject(test.stages, nullptr);
        CMemoryStream saved;
        int initialStage = 0;
        {
            CStructureSaver writer(saved, CStructureSaver::WRITE);
            writer.Add(2, &test.maximum);
            writer.Add(3, &test.stages);
            writer.Add(4, &initialStage);
            writer.Add(5, &test.maximum);
        }
        saved.Seek(0);
        {
            CStructureSaver reader(saved, CStructureSaver::READ);
            object->operator&(reader);
        }
        object->SetDestroyStage(test.stage);
        std::printf("object vitality stages=%d maximum=%d stage=%d actualStage=%d vitality=%d expected=%d\n",
            test.stages, test.maximum, test.stage, object->GetDestroyStage(), object->GetHP(), test.expected);
        Require(object->GetDestroyStage() == test.stage && object->GetHP() == test.expected,
            "native object stage did not round the vitality loss like the retail x87 calculation");
    }
}

static void BuildingIndestructibleHP()
{
    CObj<NBuilding::CBuildingGrid> grid = new NBuilding::CBuildingGrid;
    grid->Setup(2, 2, 0, 0, CVec2(0, 0));
    const NBuilding::SPoint3 ordinary(1, 1, 0), protectedCell(1, 1, 1);
    grid->AddHP(ordinary, 20);
    grid->AddHP(ordinary, 30);
    Require(grid->GetHP(ordinary) == 50, "ordinary building cells must accumulate HP");
    grid->AddHP(ordinary, 255);
    Require(grid->GetHP(ordinary) == 253, "ordinary building HP must saturate below sentinel values");
    Require(grid->DamageSpot(ordinary), "ordinary building cells must remain destructible");

    grid->SetIndestructible(protectedCell);
    const int sentinel = grid->GetHP(protectedCell);
    Require(!grid->DamageSpot(protectedCell), "the protected cell must start indestructible");
    grid->AddHP(protectedCell, 20);
    Require(grid->GetHP(protectedCell) == sentinel, "overlapping geometry removed a building cell's indestructible flag");
    Require(!grid->DamageSpot(protectedCell), "a protected building cell was destroyed after adding HP");
}

static void BuildingDamageParts()
{
    for (int z : {-2, 1, -3}) {
        CObj<NBuilding::CBuildingGrid> grid = new NBuilding::CBuildingGrid;
        grid->Setup(3, 3, -2, 1, CVec2(0, 0));
        const NBuilding::SPoint3 cell(2, 2, z);
        grid->AddHP(cell, 1);
        Require(grid->DamageSpot(cell), "the building voxel must be destroyed before refreshing parts");

        vector<NBuilding::SPart> parts;
        grid->GetUpdatedParts(&parts);
        const vector<int> floors = z == -2 ? vector<int>{-1}
            : z == 1 ? vector<int>{-1, 0} : vector<int>{-2, -1};
        Require(parts.size() == floors.size() * 4, "voxel damage refreshed the wrong number of building parts");
        for (int floor : floors)
            for (int x : {0, 1})
                for (int y : {0, 1})
                    Require(std::find(parts.begin(), parts.end(), NBuilding::SPart(floor, x, y)) != parts.end(),
                        "voxel damage missed a neighboring building part");
    }
}

static void ZeroScaleMapObjects(const char* databasePath, const char* resourcePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    const int variantID = 810;
    auto* variant = NDb::GetTemplVariant(variantID);
    Require(IsValid(variant), "zero-scale test lacks an authored map");

    auto build = [variantID] {
        CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
        SMapInfo result;
        Require(BuildMap(variantID, {}, network, &result, -1, SRandomSeed(123), 0), "could not build the authored map");
        return result;
    };
    const char* objectLabel = "UpstreamZeroScaleObject";
    auto hasObject = [objectLabel](const SMapInfo& map) {
        return std::any_of(map.items.begin(), map.items.end(),
            [objectLabel](const SMapElement& entry) { return entry.szName == objectLabel; });
    };
    for (auto entry : variant->pFinalElements) {
        if (!IsValid(entry) || !IsValid(entry->pObject) || !IsValid(entry->pObject->pObject)) continue;
        const CVec3 originalScale = entry->ptScale;
        if (std::abs(originalScale.x) < 1.e-12f || std::abs(originalScale.y) < 1.e-12f ||
            std::abs(originalScale.z) < 1.e-12f) continue;
        const string originalName = entry->szName;
        entry->szName = objectLabel;
        const auto baseline = build();
        if (!hasObject(baseline)) {
            entry->szName = originalName;
            continue;
        }
        entry->ptScale = CVec3(originalScale.x, 0, originalScale.z);
        const auto scaled = build();
        entry->ptScale = originalScale;
        entry->szName = originalName;
        Require(scaled.deploySpots.size() == baseline.deploySpots.size(), "zero-scale filtering removed deployment markers");
        Require(!hasObject(scaled), "native map build instantiated a model with a zero-scale axis");
        return;
    }
    Require(false, "zero-scale fixture could not identify a modeled object in the authored map");
}

class FixedBuildingInfo : public CPtrFuncBase<NBuilding::CBuildInfo>
{
    OBJECT_NOCOPY_METHODS(FixedBuildingInfo);
public:
    explicit FixedBuildingInfo(NBuilding::CBuildInfo* info = nullptr) { pValue = info; }
};

void RetailSolidGroupOrder(const char* databasePath, const char* resourcePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    SMapInfo map;
    Require(BuildMap(4517, {}, network, &map, -1, SRandomSeed(123), 0), "solid order fixture could not build its authored map");

    const vector<int> ids = {2, 6, 14, 1, 27};
    for (const auto& building : map.buildings) {
        if (!building.pVariant)
            continue;
        CDGPtr<CPtrFuncBase<NBuilding::CBuildInfo>> loader = NGScene::shareBuildings.Get(building.pVariant->GetRecordID());
        loader.Refresh();
        auto* donor = loader->GetValue();
        if (!donor)
            continue;
        for (const auto& original : donor->solidFragments) {
            if (original.ptPos.z < 0)
                continue;
            CObj<NBuilding::CBuildInfo> info = new NBuilding::CBuildInfo;
            info->nMaxX = 54;
            info->nMaxY = 10;
            info->nMinFloor = 0;
            info->nMaxFloor = 2;
            for (std::size_t index = 0; index < ids.size(); ++index) {
                auto fragment = original;
                fragment.nFragmentID = ids[index];
                fragment.ptPos = CVec3(3 + index * 10, 3, 0);
                fragment.nRotationID = 0;
                info->solidFragments.push_back(fragment);
            }
            CObj<FixedBuildingInfo> data = new FixedBuildingInfo(info);
            CObj<NBuilding::CSolidAndWallMap> solids = new NBuilding::CSolidAndWallMap(data, SRandomSeed(123));
            CObj<NBuilding::CBuildingGrid> grid = new NBuilding::CBuildingGrid;
            grid->Setup(info->nMaxX, info->nMaxY, info->nMinFloor, info->nMaxFloor, CVec2(0, 0));
            NBuilding::BuildingHP(info, grid, solids);
            CObj<NBuilding::CBuildingSchema> schema = new NBuilding::CBuildingSchema;
            NBuilding::MakeBuildingSchema(schema, grid, info, solids);

            vector<int> order;
            for (const auto& junction : schema->GetJuncs()) {
                const int column = (junction.ptJ.x + 2) / 10;
                if (column >= 0 && column < int(ids.size()) &&
                    (order.empty() || order.back() != ids[column]))
                    order.push_back(ids[column]);
            }
            if (order.size() != ids.size())
                continue;
            Require(order == vector<int>({27, 1, 14, 2, 6}), "native building stability changed the retail solid group order");
            return;
        }
    }
    Require(false, "solid order fixture could not isolate native junctions for five authored solid groups");
}

void TransformedCellarJunctions(const char* databasePath, const char* resourcePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    SMapInfo map;
    Require(BuildMap(4517, {}, network, &map, -1, SRandomSeed(123), 0), "fixture could not build its authored map");

    int farEnds = 0, unmarked = 0, aboveGround = 0;
    for (const auto& building : map.buildings) {
        if (!building.pVariant || !building.pGrid || !building.pSWMap) continue;
        CDGPtr<CPtrFuncBase<NBuilding::CBuildInfo>> loader = NGScene::shareBuildings.Get(building.pVariant->GetRecordID());
        loader.Refresh();
        auto* info = loader->GetValue();
        if (!info || !info->cellar.GetXSize() || !info->cellar.GetYSize()) continue;

        CArray2D<bool> walls;
        NBuilding::MarkCellarWalls(&walls, info->cellar, building.pSWMap->GetBottom());
        CObj<NBuilding::CBuildingSchema> schema = new NBuilding::CBuildingSchema;
        NBuilding::MakeBuildingSchema(schema, building.pGrid, info, building.pSWMap);
        for (const auto& junction : schema->GetJuncs()) {
            const auto point = junction.ptJ;
            if (point.z > 0) {
                ++aboveGround;
                Require(!junction.IsCellarWall(), "a transformed above-ground junction became a cellar wall");
                continue;
            }
            if (!junction.IsFilled() && point.x >= 0 && point.x < walls.GetXSize() &&
                point.y >= 0 && point.y < walls.GetYSize() && walls[point.y][point.x]) {
                ++farEnds;
                if (!junction.IsCellarWall()) ++unmarked;
            }
        }
    }
    Require(farEnds > 0 && aboveGround > 0, "authored fixtures have no distinct cellar rod ends and above-ground junctions");
    Require(unmarked == 0, "a rod's unfilled end lost the cellar flag after transforming to a cellar boundary");
}

struct StabilityGridFields
{
    vector<CPtr<CObjectBase>> cells;
    int width = 0, depth = 0, height = 0;
    int operator&(CStructureSaver& reader)
    {
        reader.Add(2, &cells);
        reader.Add(3, &width);
        reader.Add(4, &depth);
        reader.Add(5, &height);
        return 0;
    }
};

struct StabilityObjectFields
{
    CPtr<NWorld::CObjectServerBase> object;
    int operator&(CStructureSaver& reader)
    {
        reader.Add(2, &object);
        return 0;
    }
};

void StabilityObjectCells(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game, 4517);
    NWorld::CObjectServerBase* object = nullptr;
    SBound envelope;
    for (const auto& candidate : world->GetObjectsForHarness()) {
        NDb::CModel* model = nullptr;
        SHMatrix transform;
        if (!IsValid(candidate) || !candidate->GetCheckStabilityParams(&model, &transform))
            continue;
        CVec3 centerOfMass;
        vector<SMassSphere> spheres;
        NAI::GetSpheres(model, &spheres, &centerOfMass);
        if (spheres.empty())
            continue;
        SBoundCalcer bounds;
        for (const auto& sphere : spheres) {
            CVec3 center;
            transform.RotateHVector(&center, sphere.ptCenter);
            bounds.Add(center, sphere.fRadius);
        }
        bounds.Make(&envelope);
        if (envelope.ptHalfBox.x > 0.6f && envelope.ptHalfBox.x < 1.4f &&
            envelope.ptHalfBox.y < 1.4f && envelope.ptHalfBox.z < 1.4f) {
            object = candidate;
            break;
        }
    }
    Require(object, "stability fixture lacks an authored object that fits the grid boundaries");
    const bool spanning = mode != 2;
    const float centerOffset = spanning ? 1.5f : 0.5f;
    const CVec3 minimum = envelope.s.ptCenter - CVec3(centerOffset, 1.45f, 1.45f);
    const CVec3 maximum = minimum + CVec3(8.f, 2.9f, 2.9f);
    CObj<NAI::IStabilityTrackers> trackers = NAI::CreateStabilityTrackers(world);
    trackers->EnlargeMap(minimum, maximum);
    if (mode != 0)
        trackers->FinishConstruction();
    object->RegisterForStability(trackers);
    if (mode == 0)
        trackers->FinishConstruction();

    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(trackers, &references);
    StabilityGridFields grid;
    { CStructureSaver reader(fields, CStructureSaver::READ, &references); reader.Add(2, &grid); }
    Require(grid.width == 4 && grid.depth == 1 && grid.height == 1 && grid.cells.size() == 4,
        "stability grid did not construct its four cell intervals");
    for (size_t index = 0; index < grid.cells.size(); ++index) {
        CStructureNetworkContext cellReferences;
        auto cellFields = BorrowedObjectFields(grid.cells[index], &cellReferences);
        vector<StabilityObjectFields> objects;
        { CStructureSaver reader(cellFields, CStructureSaver::READ, &cellReferences); reader.Add(4, &objects); }
        const size_t expected = index == 0 ? 1 : index == 1 && spanning ? 2 : 0;
        Require(objects.size() == expected, "stability registration missed the containing cell or its upper corner");
        for (const auto& registered : objects)
            Require(registered.object == object, "stability registration recorded a different world object");
    }
}

class DebrisFixtureWorld : public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(DebrisFixtureWorld);
public:
    explicit DebrisFixtureWorld(NRPG::CGlobalGame* game = nullptr) : CWorld(game) {}
    bool AdvanceDebris(STime time)
    {
        GetTime()->Set(time);
        MarkNewDGFrame();
        SSphere invalidation;
        CDebrisController::Segment(GetAIMap(), &invalidation);
        return HasDynamicItems();
    }
    NWorld::CDFrozenItem* FindCosmetic(NDb::CModel* model) const
    {
        for (const auto& item : showFrozenItems) {
            if (item->GetModel().pModel == model && !item->GetInvItem())
                return item;
        }
        return nullptr;
    }
};

void StabilityCosmeticDebris(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true, nullptr, 810,
        [](NRPG::CGlobalGame* game) -> NWorld::CWorld* { return new DebrisFixtureWorld(game); });
    auto* fixture = static_cast<DebrisFixtureWorld*>(world.GetPtr());
    auto* actor = MovementPolicyActor(world);
    auto* weapon = NDatabase::GetTable<NDb::CRPGWeapon>()->GetRecord(26);
    Require(IsValid(weapon), "debris fixture lacks a real weapon record");
    auto* model = weapon->GetModel();
    Require(IsValid(model), "debris fixture could not resolve its native model variant");
    auto* map = world->GetAIMap();
    const auto ground = actor->GetPosition().GetCP();
    const STime now = world->GetTime()->GetValue();
    world->AddDebris(model, map, ground + CVec3(0, 0, 0.6f), QNULL, VNULL3, world->GetTime(),
        false, nullptr, nullptr, actor->GetFloor());
    for (int step = 1; step <= 100 && fixture->AdvanceDebris(now + 100 * step); ++step) {}
    auto* frozen = fixture->FindCosmetic(model);
    Require(IsValid(frozen), "itemless debris did not settle through the native physics controller");

    struct DebrisEntry {
        CPtr<NWorld::CDFrozenItem> body;
        int operator&(CStructureSaver& reader) { reader.Add(2, &body); return 0; }
    };
    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(map->GetStabilityTrackers(), &references);
    StabilityGridFields grid;
    { CStructureSaver reader(fields, CStructureSaver::READ, &references); reader.Add(2, &grid); }
    for (const auto& cell : grid.cells) {
        CStructureNetworkContext cellReferences;
        auto state = BorrowedObjectFields(cell, &cellReferences);
        vector<DebrisEntry> entries;
        { CStructureSaver reader(state, CStructureSaver::READ, &cellReferences); reader.Add(3, &entries); }
        if (std::any_of(entries.begin(), entries.end(), [&](const auto& entry) { return entry.body == frozen; }))
            return;
    }
    Require(false, "settled cosmetic debris was missing from the support stability grid");
}
}

bool RunBuildingTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "retail-solid-group-order") && argc == 4) { RetailSolidGroupOrder(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "object-destroy-stage-vitality")) { ObjectDestroyStageVitality(); return true; }
    if (!std::strcmp(mode, "building-indestructible-hp")) { BuildingIndestructibleHP(); return true; }
    if (!std::strcmp(mode, "building-damage-parts")) { BuildingDamageParts(); return true; }
    if (!std::strcmp(mode, "zero-scale-map-objects") && argc == 4) { ZeroScaleMapObjects(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "transformed-cellar-junctions") && argc == 4) { TransformedCellarJunctions(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "stability-object-cells") && argc == 5) { StabilityObjectCells(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "stability-cosmetic-debris") && argc == 4) { StabilityCosmeticDebris(argv[2], argv[3]); return true; }
    return false;
}
}
