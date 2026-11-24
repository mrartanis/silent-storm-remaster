#include "TestSupport.h"
#include "../../Main/RPGGame.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/RPGAttackMech.h"
#include "../../Main/ModManager.h"
#include "../../Main/aiPosition.h"
#include "../../DBFormat/DataFormat.h"
#include "../../DBFormat/DataMisc.h"
#include "../../DBFormat/DataRPG.h"
#include "../../DBFormat/DataInterface.h"
#include "../../FileIO/PortableGameDatabase.h"
#include "NativeDatabaseFixtures.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <cstring>

namespace UpstreamTests
{
namespace
{
static void PicklockItemImport(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);

    auto* table = NDatabase::GetTable<NDb::CRPGPicklock>();
    Require(table != nullptr, "the database has no picklock table");
    CDBIterator<NDb::CRPGPicklock> records(*table);
    int checked = 0;
    while (records.MoveNext()) {
        auto* record = records.Get();
        if (!IsValid(record->pItem)) continue;
        Require(record->pItem->pSuccessor == record, "the imported base item does not link to its picklock record");
        CObj<NRPG::IInventoryItem> item = NRPG::CreateItem(record->pItem->pSuccessor);
        auto* picklock = dynamic_cast<NRPG::CPicklockItem*>(item.GetPtr());
        Require(picklock && picklock->GetDBPicklock() == record && picklock->GetDBItem() == record->pItem,
            "the inventory item factory failed to create the imported picklock");
        Require(picklock->GetIncQuantity() == record->nQuantity, "the imported picklock lost its authored charges");
        ++checked;
    }
    Require(checked > 0, "the database has no picklock with an authored item link");
}

static void CriticalTablesFollowMods(const char* databasePath)
{
    const char* userData = std::getenv("S2_USER_DATA_DIR");
    Require(userData && *userData, "critical table test requires an isolated user directory");
    const auto root = std::filesystem::absolute(userData) / "critical-db-reload";
    const auto modPath = root / "mod";
    std::filesystem::create_directories(modPath);
    std::filesystem::copy_file(databasePath, root / "game.db", std::filesystem::copy_options::overwrite_existing);
    {
        std::ofstream description(modPath / "description.txt");
        description << "Critical table reload fixture";
        Require(bool(description), "could not write mod description");
    }
    struct WorkingDirectory {
        std::filesystem::path original = std::filesystem::current_path();
        ~WorkingDirectory() { std::error_code error; std::filesystem::current_path(original, error); }
    } directory;
    std::filesystem::current_path(root);
    {
        CFileStream database;
        database.OpenRead("game.db");
        NDatabase::Serialize(database, CStructureSaver::READ);
    }

    auto applyHit = [] {
        auto* persona = NDb::GetPers(103);
        auto* armor = NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);
        Require(persona && armor && armor->pMaterial, "critical reload fixture lacks a persona or body armor");
        int damageType = -1;
        CDBIterator<NDb::CRPGDmgToArmor> damage(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());
        while (damage.MoveNext()) {
            if (damage.Get()->armors[armor->pMaterial->nDR] > 0) {
                damageType = damage.Get()->GetRecordID();
                break;
            }
        }
        Require(damageType >= 0, "critical reload fixture lacks a body damage type");
        CObj<NRPG::IUnitMission> target = NRPG::CreateUnit(persona);
        for (int type = 0; type < NDb::N_CRIT_TYPES; ++type)
            if (type != NDb::C_AP_REDUCTION)
                target->DisableCritical(static_cast<NDb::ECritical>(type), NRPG::CS_DISABLED);
        NRPG::CAttackPortion hit(100000, damageType, 0.f, 2, 2, 100, 100, nullptr, target, 1.f, -1);
        GlobalGameRandom().SeedForHarness(813);
        const auto result = target->ProcessAttack(nullptr, NAI::HL_BODY, &hit, armor);
        Require(result.nDmg > 0, "critical reload fixture did not apply a body hit");
        return target->HasCritical(NDb::C_AP_REDUCTION);
    };

    Require(applyHit(), "base database did not produce the authored torso AP critical");
    SModInfo mod;
    mod.szDirectory = "mod";
    mod.szName = "Critical table reload fixture";
    Require(CModManager::Activate(vector<SModInfo>{mod}), "activating the mod did not reload the real database");
    Require(applyHit(), "mod activation left the critical selector pointing to the old database");
    Require(CModManager::Activate(vector<SModInfo>{}), "removing the mod did not reload the real database");
    Require(applyHit(), "mod removal left the critical selector pointing to the old database");
}

static void RetailAmmoSubtypes(const char* databasePath)
{
    S2FileIO::PortableGameDatabase source;
    string error;
    Require(S2FileIO::LoadPortableGameDatabase(databasePath, &source, &error), "could not decode native item columns");
    const auto tableID = NDatabase::GetRecordTypes().GetTypeID(static_cast<NDb::CRPGItem*>(nullptr));
    const auto table = std::find_if(source.tables.begin(), source.tables.end(),
        [tableID](const auto& value) { return value.tableId == tableID; });
    Require(table != source.tables.end(), "database does not contain the item table");
    const auto id = std::find(table->intNames.begin(), table->intNames.end(), "ID");
    const auto subtype = std::find(table->stringNames.begin(), table->stringNames.end(), "SubType");
    Require(id != table->intNames.end() && subtype != table->stringNames.end(), "item table lacks ID or SubType");

    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    auto* items = NDatabase::GetTable<NDb::CRPGItem>();
    Require(items != nullptr, "item table did not load");
    const auto idColumn = id - table->intNames.begin();
    const auto subtypeColumn = subtype - table->stringNames.begin();
    const std::pair<const wchar_t*, int> expected[] = {
        {L"SMGAmmoAllies", 15}, {L"SMGAmmoAxis", 16}, {L"RifleAmmoAllies", 17}, {L"RifleAmmoAxis", 18},
        {L"MGAmmo", NDb::SUBTYPE_AMMO_MG}, {L"RifleAmmo", NDb::SUBTYPE_AMMO_RIFLE},
    };
    int matches = 0;
    for (std::size_t row = 0; row < table->intRows.size(); ++row) {
        const auto& name = table->stringRows[row][subtypeColumn];
        for (const auto& entry : expected) {
            if (name != entry.first) continue;
            const auto* item = items->GetRecord(table->intRows[row][idColumn]);
            Require(item && int(item->subType) == entry.second, "item import did not retain the authored ammunition subtype");
            ++matches;
        }
    }
    Require(matches > 0, "database has no authored ammunition subtype to check");
}

static void UITextureFallbackDimensions()
{
    auto storage = [](vector<string> fields, vector<vector<int>> rows) {
        CObj<CObjectBase> table = pSSClasses->CreateObject(int(0xa1843130u));
        Require(IsValid(table), "columnar database storage is missing");
        vector<NativeContextColumn> columns;
        for (const auto& field : fields) columns.push_back({field, 0});
        vector<vector<float>> floats;
        vector<vector<wstring>> strings;
        vector<string> emptyNames;
        CMemoryStream serialized;
        {
            CStructureSaver writer(serialized, CStructureSaver::WRITE);
            writer.Add(2, &rows);
            writer.Add(3, &floats);
            writer.Add(4, &strings);
            writer.Add(5, &columns);
            writer.Add(6, &fields);
            writer.Add(7, &emptyNames);
            writer.Add(8, &emptyNames);
        }
        serialized.Seek(0);
        CStructureSaver reader(serialized, CStructureSaver::READ);
        table->operator&(reader);
        return table;
    };
    unordered_map<int, CObj<CObjectBase>> tables;
    tables[2] = storage({"ID"}, {});
    tables[3] = storage({"ID", "Width", "Height"}, {{1, 100, 100}, {2, 240, 180}});
    tables[49] = storage({"ID", "R_800x600", "R_1024x768", "R_1280x960", "R_1600x1200"},
        {{1, 0, 0, 2, 0}, {2, 0, 1, 2, 0}});
    list<NativeContextEmptyRelation> relations;
    CMemoryStream database;
    {
        CStructureSaver writer(database, CStructureSaver::WRITE);
        writer.Add(1, &tables);
        writer.Add(2, &relations);
    }
    database.Seek(0);
    try {
        NDatabase::Serialize(database, CStructureSaver::READ, false, false);
    } catch (const SFileIOError& error) {
        throw std::runtime_error(error.szError);
    }
    const auto* preferred = NDb::GetUITexture(2);
    Require(preferred && preferred->nWidth == 100 && preferred->nHeight == 100,
        "the 1024x768 texture did not retain priority and its authored size");
    const auto* fallback = NDb::GetUITexture(1);
    Require(fallback && fallback->pTextures[NDb::UIM_1280x1024] &&
        fallback->pTextures[NDb::UIM_1280x1024]->nHeight == 180,
        "the database did not link the 1280x960 texture");
    Require(fallback->nWidth == 192 && fallback->nHeight == 144,
        "the UI texture fallback calculated logical dimensions from the wrong resolution");
}

static void FirearmAttackCreationResult(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    auto* persona = NDb::GetPers(103);
    Require(IsValid(persona), "attack creation fixture lacks its persona");
    CObj<NRPG::IUnitMission> actor = NRPG::CreateUnit(persona);
    auto* inventory = actor->GetInventory();
    CObj<NRPG::IInventoryItem> previous = inventory->TakeOff(NDb::SLOT_1);
    NDb::CRPGWeapon* rifle = nullptr;
    CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while (weapons.MoveNext()) {
        auto* record = weapons.Get();
        if (record->pItem && record->pWeaponType && record->pWeaponType->bTwoHanded &&
            record->pInnerClip && record->shootModes[NDb::SM_Snap]) {
            rifle = record;
            break;
        }
    }
    Require(rifle != nullptr, "attack creation fixture lacks an authored two-handed firearm");
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(rifle);
    weapon->nQuantity = 1;
    Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1) &&
        weapon->SetShootMode(NDb::SM_Snap) && weapon->GetAmmoQuantity() > 0,
        "attack creation fixture cannot equip a loaded firearm");
    const int ammo = weapon->GetAmmoQuantity();

    vector<NRPG::CAttackPortion> portions;
    actor->UseTwoHanded(false);
    Require(!actor->CreateAttack(&portions, false) && portions.empty(),
        "a disabled two-handed firearm reported a successful attack without a portion");
    actor->UseTwoHanded(true);
    Require(actor->CreateAttack(&portions, false) && portions.size() == 1,
        "a working two-handed firearm failed to create a preview portion");
    portions.front().nCrtical = 7;
    portions.front().nCrticalDifficulty = 11;
    Require(actor->CreateAttack(&portions, false) && portions.size() == 2,
        "creating a second firearm portion failed");
    Require(portions.front().nCrtical == 7 && portions.front().nCrticalDifficulty == 11,
        "creating a second firearm portion modified the caller's existing portion");
    Require(weapon->GetAmmoQuantity() == ammo, "firearm previews spent ammunition");
}
}

bool RunDatabaseTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "picklock-item-import") && argc == 3) { PicklockItemImport(argv[2]); return true; }
    if (!std::strcmp(mode, "critical-tables-follow-mods") && argc == 3) { CriticalTablesFollowMods(argv[2]); return true; }
    if (!std::strcmp(mode, "retail-ammo-subtypes") && argc == 3) { RetailAmmoSubtypes(argv[2]); return true; }
    if (!std::strcmp(mode, "ui-texture-fallback-dimensions")) { UITextureFallbackDimensions(); return true; }
    if (!std::strcmp(mode, "firearm-attack-creation") && argc == 3) { FirearmAttackCreationResult(argv[2]); return true; }
    return false;
}
}
