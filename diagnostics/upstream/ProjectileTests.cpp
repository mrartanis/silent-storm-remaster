#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/wExplTracker.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wDebris.h"
#include "../../Main/wOSBase.h"
#include "../../Main/wMisc.h"
#include "../../Main/wTSFlags.h"
#include "../../Main/aiMap.h"
#include "../../Main/aiInterval.h"
#include "../../Main/RPGAttackMech.h"
#include "../../Main/RPGBullet.h"
#include "../../Main/RPGGame.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemSet.h"
#include "../../DBFormat/DataRPG.h"
#include "../../DBFormat/DataFormat.h"
#include "../../DBFormat/DataSound.h"
#include "../../DBFormat/DataGeometry.h"
#include "../../DBFormat/DataObject.h"
#include "../../Misc/RandomGen.h"
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

void StuckKnifeFloor(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, false, nullptr, 4517);
    auto* map = world->GetAIMap();
    map->Sync();
    int floor = 0;
    float distance = 0.f;
    CRay shot;
    for (const auto& object : world->GetObjectsForHarness()) {
        SBound bounds;
        if (!map->GetObjectBound(&bounds, object)) continue;
        for (int heading = 0; heading < 16 && !floor; ++heading) {
            const float angle = heading * FP_2PI / 16;
            CRay ray;
            ray.ptDir = CVec3(std::cos(angle), std::sin(angle), 0);
            ray.ptOrigin = bounds.s.ptCenter - ray.ptDir * (bounds.s.fRadius + 0.4f);
            vector<NAI::SInterval> hits;
            map->Trace(ray, &hits, NWorld::TS_FRAGMENTED | NWorld::TS_ITEM_BLOCKER);
            const NAI::SInterval* first = nullptr;
            for (const auto& hit : hits)
                if (hit.enter.fT > 0.f && (!first || hit.enter.fT < first->enter.fT))
                    first = &hit;
            if (!first || first->pSrc->pUserData != object.GetPtr() || !first->pSrc->nFloor ||
                (first->pSrc->nTSFlags & (NWorld::TS_STATE_OPEN | NWorld::TS_STATE_CLOSED)) ||
                !first->pSrc->pArmor || !first->pSrc->pArmor->pMaterial)
                continue;
            const int resistance = first->pSrc->pArmor->pMaterial->nDR;
            if (resistance != 1 && resistance != 2) continue;
            floor = first->pSrc->nFloor;
            distance = first->enter.fT;
            shot = ray;
        }
        if (floor) break;
    }
    Require(floor != 0, "the authored world has no clear knife ray to a non-ground wood surface");
    NDb::CRPGMeleeWeapon* record = nullptr;
    CDBIterator<NDb::CRPGMeleeWeapon> blades(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
    while (blades.MoveNext()) {
        if (blades.Get()->bThrowing && IsValid(blades.Get()->pItem) && IsValid(blades.Get()->pItem->pModel)) {
            record = blades.Get();
            break;
        }
    }
    Require(record, "the fixture has no authored throwing knife");
    SRand random;
    CObj<NDb::CModel> model = record->pItem->pModel->CreateModel(&random);
    CObj<NRPG::CMeleeWeaponItem> item = new NRPG::CMeleeWeaponItem(record);
    item->nQuantity = 1;
    NRPG::CAttackPortion hit(100000, 0, 0.f, 0, 0, 0);
    NRPG::SAttackRayInfo ray(hit, shot.ptOrigin, shot.ptDir, false, 0.f, distance + 1.f, nullptr);
    const auto start = world->GetTime()->GetValue();
    CObj<NWorld::IDynamicObject> knife = dynamic_cast<NWorld::IDynamicObject*>(
        world->ThrowKnife(ray, 10.f, start, distance + 1.f, model, item));
    Require(knife && !knife->Segment() && !world->GetFrozenItem(item),
        "the knife fixture hit before the projectile advanced");
    world->GetTime()->Set(start + STime(std::ceil((distance + 0.05f) * 100)));
    MarkNewDGFrame();
    Require(knife->Segment(), "the knife did not retire on the wood surface");
    const auto* frozen = world->GetFrozenItem(item);
    Require(frozen && frozen->GetFloor() == floor, "the stuck knife lost the floor of the surface it hit");
}

void ShotRayWithoutCoverCandidates(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* map = world->GetAIMap();
    map->Sync();
    const CVec3 source = actor->GetAttackOrigin(actor->GetPosition());
    const CVec3 target = source - CVec3(0.3f, 0.2f, 10.f);
    NRPG::CAttackPortion attack(100000, 0, 0.f, 5, 5, 0);
    const int mask = NWorld::TS_COVER | NWorld::TS_ITEM_BLOCKER | NWorld::TS_FRAGMENTED;

    CObj<NRPG::CCoverInfo> unrestricted = world->GetGame()->CalcCoversForTile(source, attack, actor, target, 0.f, mask);
    Require(NRPG::CanShoot(unrestricted), "fixture has no usable rays without the muzzle clearance gate");
    NRPG::SAttackRayInfo ray(attack, VNULL3, VNULL3, false, 0.f, 30.f, nullptr);
    Require(NRPG::PrepareAttackRay(map, unrestricted, &ray, 1.f) && fabs2(ray.vDir - V3_AXIS_Z) > 1.f,
        "control shot did not select a real trajectory toward the target");

    CObj<NRPG::CCoverInfo> blocked = world->GetGame()->CalcCoversForTile(source, attack, actor, target, 4.f, mask);
    Require(!NRPG::CanShoot(blocked), "nearby terrain did not block the entire muzzle cone");
    ray.vOrigin = VNULL3;
    Require(!NRPG::PrepareAttackRay(map, blocked, &ray, 1.f) && !ray.bTargetIsHit && ray.trailPoints.empty(),
        "a blocked muzzle yielded a selected ray, a hit or a stale hit trail");
    Require(ray.vOrigin == source && ray.vDir == V3_AXIS_Z,
        "a blocked muzzle did not return the finite upward miss from the attack origin");
}

void RangedHitRollRange(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    const auto shooter = std::find_if(units.begin(), units.end(), [](const auto& unit) {
        if (!unit || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK()) return false;
        auto* weapon = dynamic_cast<NRPG::IWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        return weapon && weapon->GetAmmoQuantity() > 0 && weapon->SetShootMode(NDb::SM_Snap);
    });
    Require(shooter != units.end(), "hit roll fixture lacks an equipped firearm");
    auto* actor = shooter->GetPtr();
    auto* mission = actor->GetUnitRPG();
    vector<NRPG::CAttackPortion> attack;
    Require(mission->CreateAttack(&attack, false) && !attack.empty(), "fixture could not quote a native shot");
    const auto position = actor->GetPosition();
    const auto source = actor->GetAttackOrigin(position);
    CObj<NRPG::CCoverInfo> cover;
    CVec3 target;
    const bool firstTurn = world->IsFirstTurn();
    int expectedChance = 0;
    for (float distance : {2.f, 4.f, 8.f, 12.f}) {
        target = position.GetCP() + CVec3(distance, 0.f, 0.1f);
        cover = world->GetGame()->CalcCoversForTile(source, attack.front(), actor, target, actor->GetMinClearDistance());
        if (!NRPG::CanShoot(cover)) continue;
        NRPG::CheckTileToHit(actor, target, 0, NAI::THL_LOWER, cover, firstTurn, &expectedChance);
        if (expectedChance > 0 && expectedChance < 100) break;
    }
    Require(expectedChance > 0 && expectedChance < 100, "fixture has no shot with a nondegenerate hit chance");

    GlobalGameRandom().SeedForHarness(0x5201);
    int previous = -10;
    for (int sample = 0; sample < 400; ++sample) {
        int chance = -1;
        const float result = NRPG::CheckTileToHit(actor, target, 0, NAI::THL_LOWER, cover, firstTurn, &chance);
        Require(chance == expectedChance && std::isfinite(result) && result > 0.f,
            "the native hit check divided by a zero roll");
        const int roll = int(std::lround(chance / result));
        Require(roll >= 1 && roll <= 99 && std::abs(roll - previous) >= 6,
            "hit roll left the original range or repeated a nearby value");
        previous = roll;
    }
}
}

bool RunProjectileTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "frozen-item-armor-resistance") && argc == 4) { FrozenItemArmorResistance(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "flying-grenade-lifetime") && argc == 4) { FlyingGrenadeLifetime(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "bloody-death-sound-position") && argc == 4) { BloodyDeathSoundPosition(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "explosion-voxel-budget") && argc == 4) { ExplosionVoxelBudget(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "invalid-projectile-resources") && argc == 5) { InvalidProjectileResources(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "stuck-knife-floor") && argc == 4) { StuckKnifeFloor(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "shot-ray-without-cover-candidates") && argc == 4) { ShotRayWithoutCoverCandidates(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ranged-hit-roll-range") && argc == 4) { RangedHitRollRange(argv[2], argv[3]); return true; }
    return false;
}
}
