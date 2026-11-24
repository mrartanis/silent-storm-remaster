#include "TestSupport.h"
#include "../../Main/aiGrid.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wMine.h"
#include <cstring>

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
}

bool RunNavigationTests(const char* mode)
{
    if (std::strcmp(mode, "known-mine-path-reservations") != 0) return false;
    KnownMinePathReservations();
    return true;
}
}
