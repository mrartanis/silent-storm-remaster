#include "TestSupport.h"
#include "../../Main/BuildingGrid.h"
#include "../../Main/BuildingInfo.h"
#include "../../Main/BuildingSchema.h"
#include "../../Main/MakeBuilding.h"
#include "../../Main/MakeBuildingInternal.h"
#include "../../Main/MapBuild.h"
#include "../../Main/GInit.h"
#include "../../Main/aiGrid.h"
#include "../../DBFormat/DataMap.h"
#include "../../DBFormat/DataObject.h"
#include "../../DBFormat/DataFormat.h"
#include "../../Misc/BasicShare.h"
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
}

bool RunBuildingTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "building-indestructible-hp")) { BuildingIndestructibleHP(); return true; }
    if (!std::strcmp(mode, "building-damage-parts")) { BuildingDamageParts(); return true; }
    if (!std::strcmp(mode, "zero-scale-map-objects") && argc == 4) { ZeroScaleMapObjects(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "transformed-cellar-junctions") && argc == 4) { TransformedCellarJunctions(argv[2], argv[3]); return true; }
    return false;
}
}
