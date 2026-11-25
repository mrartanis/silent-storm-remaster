#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/rpgGlobal.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/aiCommander.h"
#include "../../Main/GInit.h"
#include "../../Main/rpgPerk.h"
#include "../../DBFormat/DataPerk.h"
#include <stdexcept>

namespace UpstreamTests
{
void Require(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}

CMemoryStream BorrowedObjectFields(CObjectBase* object, CStructureNetworkContext* references)
{
    vector<CObjectBase*> referenced;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* child) { referenced.push_back(child); return false; };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph); object->operator&(writer); }
    references->externalReferences = true;
    references->nextID = 100;
    references->include = [](CObjectBase*) { return false; };
    for (auto* child : referenced) {
        if (references->ids.count(child)) continue;
        const auto id = references->nextID++;
        references->ids[child] = id; references->refs[id] = child;
    }
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, references); object->operator&(writer); }
    saved.Seek(0);
    return saved;
}

void RestoreSinglePerk(NRPG::CUnit* unit, int id)
{
    auto* tree = unit->GetPerksTree();
    auto* perk = NDb::GetDBPerk(id);
    Require(tree && perk, "fixture lacks its native perk tree or perk record");
    CObj<NRPG::CPerk> learned = new NRPG::CPerk(perk);
    learned->Take();
    unordered_map<int, CObj<NRPG::CPerk>> perks{{id, learned}};
    CMemoryStream fields;
    { CStructureSaver writer(fields, CStructureSaver::WRITE); writer.Add(2, &perks); }
    fields.Seek(0);
    { CStructureSaver reader(fields, CStructureSaver::READ); static_cast<CObjectBase*>(tree)->operator&(reader); }
    Require(unit->HasPerk(id), "native RPG unit did not recover its saved perk");
}

CObj<NWorld::CWorld> FixtureWorld(const char* databasePath, const char* resourcePath,
    bool deployParty, CObj<NRPG::CGlobalGame>* retainedGame, int variantID,
    NWorld::CWorld* (*factory)(NRPG::CGlobalGame*))
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    if (retainedGame) *retainedGame = game;
    if (deployParty) game->players.push_back(NRPG::CreateGlobalPlayer(std::vector<int>{54}));
    CObj<NWorld::CWorld> world = factory ? factory(game.GetPtr()) : new NWorld::CWorld(game);
    CObj<NWorld::CPostWorldCreateInfo> post;
    world->CreateRandom(variantID, {}, true, {}, 0, &post, SRandomSeed(123));
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

NWorld::CUnitServer* MovementPolicyActor(NWorld::CWorld* world)
{
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    for (auto actor : units)
        if (IsValid(actor) && !actor->IsAIUnit() && actor->CanFight() &&
            actor->GetUnitRPG()->CanMove() && actor->GetPosition().pos.p.GetPose() == NAI::CM_STAND)
            return actor;
    throw std::runtime_error("fixture lacks a standing, mobile party member");
}
}
