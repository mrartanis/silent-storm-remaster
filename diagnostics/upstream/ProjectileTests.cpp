#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/wExplTracker.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wDebris.h"
#include "../../Main/wMisc.h"
#include "../../Main/RPGAttackMech.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemSet.h"
#include "../../DBFormat/DataRPG.h"
#include "../../DBFormat/DataFormat.h"
#include "../../DBFormat/DataSound.h"
#include "../../DBFormat/DataGeometry.h"
#include "../../DBFormat/DataObject.h"
#include "SoundVisitor.h"
#include <set>
#include <cstring>

namespace UpstreamTests
{
namespace
{
static void FrozenItemArmorResistance(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    auto* book = NDb::GetRPGItem(438);
    Require(book && IsValid(book->pModel), "test requires the authored book model");
    CObj<NRPG::IInventoryItem> item = NRPG::CreateClueItem(book);
    CObj<NWorld::CDFrozenItem> body =
        world->AddFrozenItem(world->GetAIMap(), CVec3(2, 2, 0), QNULL, item, false, 0);
    Require(IsValid(body) && body->GetModel().pModel, "the dropped item has no world body");
    auto* armor = body->GetModel().pModel->pRPGArmor.GetPtr();
    Require(IsValid(armor) && IsValid(armor->pMaterial), "the dropped item has no authored armor material");
    struct RestoreResistance {
        NDb::CRPGMaterial* material;
        int value;
        int threshold;
        ~RestoreResistance() { material->nDR = value; material->nThreshold = threshold; }
    } restore{armor->pMaterial, armor->pMaterial->nDR, armor->pMaterial->nThreshold};

    NRPG::CAttackPortion hit(100000, 0, 0.f, 5, 5, 0);
    armor->pMaterial->nThreshold = 0;
    Require(hit.CalcStructDmg(world, armor, 0) == 5, "test hit does not deal its fixed amount of damage");
    armor->pMaterial->nDR = 0;
    Require(body->ProcessAttack(world, 0, &hit, CVec3(1, 0, 0), armor) == 0,
        "the dropped item took damage from its zero-resistance material");
    armor->pMaterial->nDR = 1;
    Require(body->ProcessAttack(world, 0, &hit, CVec3(1, 0, 0), armor) == 5,
        "a positive-resistance material stopped applying damage");
}

NDb::CRPGGrenade* FindModelledGrenade()
{
    NDb::CRPGGrenade* authored = nullptr;
    CDBIterator<NDb::CRPGGrenade> grenades(*NDatabase::GetTable<NDb::CRPGGrenade>());
    while (grenades.MoveNext()) {
        auto* candidate = grenades.Get();
        if (IsValid(candidate->pItem) && IsValid(candidate->pItem->pModel) &&
            (!authored || candidate->GetRecordID() < authored->GetRecordID()))
            authored = candidate;
    }
    Require(authored, "the fixture needs a modelled grenade");
    return authored;
}

static void FlyingGrenadeLifetime(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    SRand random;
    CObj<NDb::CModel> model = FindModelledGrenade()->pItem->pModel->CreateModel(&random);
    Require(IsValid(model) && IsValid(model->pGeometry), "grenade lifetime fixture needs valid projectile geometry");
    CObj<NDb::CRPGGrenade> testGrenade = new NDb::CRPGGrenade;
    testGrenade->nMaxDelay = 20;
    const auto release = world->GetTime()->GetValue();
    const size_t beforeLaunch = world->GetMiscObjects()->size();
    CObj<NWorld::IDynamicObject> grenade = dynamic_cast<NWorld::IDynamicObject*>(
        world->ThrowGrenade(actor->GetPosition().GetEyePosition(), CVec3(1, 0, 1), release,
            0.f, model, testGrenade, actor));
    Require(grenade && world->GetMiscObjects()->size() == beforeLaunch + 1,
        "the real world did not create a flying grenade");
    world->GetTime()->Set(release + 15000);
    MarkNewDGFrame();
    Require(!grenade->Segment(), "a flying grenade expired before the lifetime boundary");
    world->GetTime()->Set(release + 15001);
    MarkNewDGFrame();
    Require(grenade->Segment(), "the flying grenade's expiry still depends on its fuse");
    Require(world->GetMiscObjects()->size() == beforeLaunch + 1,
        "the expired grenade emitted a new world object before its fuse");
}

static void BloodyDeathSoundPosition(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    CObj<NWorld::CUnitServer> actor = MovementPolicyActor(world);
    auto* persona = actor->GetUnitRPG()->GetRPGUnit()->GetAckHolder();
    if (!persona) persona = actor->GetUnitRPG()->GetRPGPers();
    const auto* deathSound = persona->pSoundDeath.GetPtr();
    Require(deathSound && !deathSound->variants.empty() && NDb::GetTEffect(822),
        "bloody-death fixture requires the authored sound and effect");
    const auto position = actor->GetPosition();
    const auto eye = position.GetEyePosition();
    Require(fabs2(eye - position.GetCenter()) > 0.01f, "fixture needs distinct eye and center positions");
    std::set<NWorld::IDynamicObject*> existing;
    for (const auto& object : *world->GetMiscObjects()) existing.insert(object.GetPtr());

    actor->SetBloodyDeath(true);
    actor->GetUnitRPG()->GetRPGUnit()->Kill();
    actor->SyncConscious();

    ObjectStageAudio audio;
    for (const auto& object : *world->GetMiscObjects())
        if (!existing.count(object.GetPtr()))
            if (auto* sound = dynamic_cast<NWorld::C3DSound*>(object.GetPtr())) sound->Visit(&audio);
    const auto matches = [deathSound](auto* sound) {
        return sound && std::any_of(deathSound->variants.begin(), deathSound->variants.end(),
            [sound](const auto& variant) { return IsValid(variant) && variant->pSound == sound; });
    };
    Require(std::count_if(audio.sounds.begin(), audio.sounds.end(), matches) == 1,
        "bloody death did not emit exactly one detached persona death sound");
    const auto index = std::find_if(audio.sounds.begin(), audio.sounds.end(), matches) - audio.sounds.begin();
    CDGPtr<CFuncBase<CVec3>> soundPosition(audio.positions[index].GetPtr());
    soundPosition.Refresh();
    Require(fabs2(soundPosition->GetValue() - eye) < 0.00001f,
        "bloody death emitted its detached sound from the unit center");
}

static void ExplosionVoxelBudget(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    const auto center = actor->GetPosition().GetEyePosition();
    CObj<NWorld::CExplosionSpace> space = NWorld::CreateExplosionSpace(world->GetAIMap());
    CObj<NDb::CRPGGrenade> grenade = new NDb::CRPGGrenade;
    grenade->nWaveNumber = 1;
    int budgets[2] = {};
    for (int radius = 1; radius <= 2; ++radius) {
        grenade->fWaveRadius = radius * NWorld::F_VOXEL_SIZE / FP_GRID_STEP;
        CObj<NWorld::CVoxelExpl> wave = new NWorld::CVoxelExpl(
            world, center, nullptr, 0, grenade, actor, nullptr, space);
        CStructureNetworkContext refs;
        auto fields = BorrowedObjectFields(wave, &refs);
        CStructureSaver reader(fields, CStructureSaver::READ, &refs);
        reader.Add(9, &budgets[radius - 1]);
    }
    Require(budgets[0] == 4, "single-voxel-radius control changed its budget");
    Require(budgets[1] == 33, "a blast rounded its fractional voxel budget upward");
}

void InvalidProjectileResources(const char* databasePath, const char* resourcePath, int mode)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    const auto start = actor->GetPosition().GetEyePosition();
    const auto when = world->GetTime()->GetValue();
    const CVec3 velocity(1, 0, 1);
    auto* grenade = FindModelledGrenade();
    SRand random;
    CObj<NDb::CModel> model = grenade->pItem->pModel->CreateModel(&random);
    CObj<NDb::CModel> noGeometry = new NDb::CModel;
    Require(IsValid(model) && IsValid(model->pGeometry) && !noGeometry->pGeometry,
        "the projectile fixture did not supply contrasting geometries");
    const auto before = world->GetMiscObjects()->size();
    if (mode == 0) {
        Require(world->ThrowGrenade(start, velocity, when, 0.f, model, grenade, actor) &&
            world->GetMiscObjects()->size() == before + 1, "the fixture cannot launch a valid grenade");
        Require(!world->ThrowGrenade(start, velocity, when, 0.f, noGeometry, grenade, actor),
            "the world accepted a grenade without geometry");
    } else {
        CObj<NRPG::IInventoryItem> ammunition;
        CDBIterator<NDb::CRPGAmmo> ammo(*NDatabase::GetTable<NDb::CRPGAmmo>());
        while (!ammunition && ammo.MoveNext()) {
            if (!IsValid(ammo.Get()->pExplosiveBullet)) continue;
            CDBIterator<NDb::CRPGClip> clips(*NDatabase::GetTable<NDb::CRPGClip>());
            while (clips.MoveNext()) {
                if (IsValid(clips.Get()->pItem) && clips.Get()->nAmmoGroup == ammo.Get()->nAmmoGroup) {
                    ammunition = NRPG::CreateClipItem(clips.Get(), ammo.Get(), 1);
                    break;
                }
            }
        }
        auto* rocket = dynamic_cast<NRPG::IClipItem*>(ammunition.GetPtr());
        Require(rocket && rocket->GetIncQuantity() == 1, "the fixture needs a live rocket ammunition record");
        NRPG::CAttackPortion hit(100000, 0, 0.f, 5, 5, 0);
        Require(world->LaunchRocket(start, velocity, when, 3.f, model, hit, rocket, actor, nullptr) &&
            world->GetMiscObjects()->size() == before + 1, "the fixture cannot launch a valid rocket");
        Require(!world->LaunchRocket(start, velocity, when, 3.f, mode == 1 ? noGeometry.GetPtr() : model.GetPtr(),
            hit, mode == 2 ? nullptr : rocket, actor, nullptr), "the world accepted a rocket with missing resources");
    }
    Require(world->GetMiscObjects()->size() == before + 1, "the rejected launch changed the world projectile list");
}
}

bool RunProjectileTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "frozen-item-armor-resistance") && argc == 4) { FrozenItemArmorResistance(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "flying-grenade-lifetime") && argc == 4) { FlyingGrenadeLifetime(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "bloody-death-sound-position") && argc == 4) { BloodyDeathSoundPosition(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "explosion-voxel-budget") && argc == 4) { ExplosionVoxelBudget(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "invalid-projectile-resources") && argc == 5) { InvalidProjectileResources(argv[2], argv[3], std::atoi(argv[4])); return true; }
    return false;
}
}
