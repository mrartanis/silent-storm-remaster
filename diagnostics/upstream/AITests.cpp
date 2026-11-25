#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wOSBase.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGCritical.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/rpgGlobal.h"
#include "../../Main/aiActions.h"
#include "../../Main/aiCommander.h"
#include "../../Main/aiJob.h"
#include "../../Main/aiMisc.h"
#include "../../Main/aiUnit.h"
#include "../../Main/aiInventory.h"
#include "../../Main/aiUnitState.h"
#include "../../Main/aiWeapon.h"
#include "../../Main/aiActionPlaceSource.h"
#include "../../Main/aiEvent.h"
#include "../../Main/aiGrid.h"
#include "../../Main/aiReaction.h"
#include "../../Main/aiRouteLogic.h"
#include "../../Main/aiRouteMisc.h"
#include "../../Main/aistate.h"
#include "../../Main/aiTaskCommand.h"
#include "../../Main/aiThreatTracker.h"
#include "../../Main/eventUnit.h"
#include "../../Misc/RandomGen.h"
#include "../AIUnitFixture.h"
#include "../../DBFormat/DataRPG.h"
#include "../../DBFormat/DataMap.h"
#include "../../DBFormat/DataFormat.h"
#include <cstring>
#include <stdexcept>

namespace UpstreamTests
{
namespace
{
void SelfHealingWithBandagedHealth(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    const auto candidate = std::find_if(units.begin(), units.end(), [](const auto& unit) {
        return IsValid(unit) && unit->IsAIUnit() && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK();
    });
    Require(candidate != units.end(), "the healing fixture has no live AI character");
    auto* actor = candidate->GetPtr();
    auto* mission = actor->GetUnitRPG();
    auto* rpg = mission->GetRPGUnit();
    auto* inventory = mission->GetInventory();
    auto* record = NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(1);
    Require(record && record->nAPToUse == 0 && record->nRequiedSkill == 0 && record->nRequiredPerkID == 0,
        "the healing fixture needs a usable first-aid kit");
    CObj<NRPG::IInventoryItem> previous = inventory->TakeOff(NDb::SLOT_1);
    CObj<NRPG::CFirstAidItem> kit = new NRPG::CFirstAidItem(record);
    Require(inventory->Equip(NDb::SLOT_1, kit) && inventory->Activate(NDb::SLOT_1) && !kit->IsEmpty(),
        "could not equip the first-aid kit");

    auto& health = rpg->Skills(NDb::ST_VP);
    health.SetValue(health.GetCurrentMaxValue() / 4);
    rpg->nHealedVP = 0;
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_ANY, NDb::C_VP));
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD, NDb::C_WEAPONSKILL_REDUCTION, -1, 2.f, 1));
    auto* ai = NAI::GetAIUnit(actor);
    Require(ai && ai->IsUnderAIControl(), "the wounded unit has no AI controller");
    ai->Synchronize(false);
    Require(!ai->GetAIUnitState()->pEnemy, "the AI is already responding to an enemy");
    const float threshold = ai->GetMaxHP() * 0.3f;
    Require(ai->GetHP() > 0 && ai->GetHP() <= threshold, "the raw health is outside the healing threshold");
    const NAI::SPlaceWithAP place(ai->GetUnitPosition(), ai->GetAP());
    CObj<NAI::CAIHealAction> action = new NAI::CAIHealAction(ai);
    NAI::CAIHealAction::SInfo before, after;
    action->GetInfoInner(place, &before);
    Require(before.bCanDo && IsValid(before.pFirstAid) && before.pFirstAid->GetItem() == kit,
        "the AI cannot use its equipped kit before bandaging");

    rpg->nHealedVP = ai->GetMaxHP() / 4;
    Require(mission->GetTotalVP() > threshold && ai->GetHP() <= threshold,
        "the bandaged and raw values must straddle the healing threshold");
    action->GetInfoInner(place, &after);
    Require(!after.bCanDo && after.pFirstAid == before.pFirstAid,
        "the AI still spends first aid when bandaged health exceeds its threshold");
}

class SequenceDeploymentWorld : public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(SequenceDeploymentWorld);
public:
    SequenceDeploymentWorld() = default;
    explicit SequenceDeploymentWorld(NRPG::CGlobalGame* game) : CWorld(game)
    {
        auto* variant = NDb::GetTemplVariant(810);
        Require(variant && !variant->pUnits.empty() && variant->pUnits.front(), "map has no authored units");
        CObj<NRPG::CGlobalPlayer> campaign = new NRPG::CGlobalPlayer;
        game->players.push_back(campaign);
        const int side = variant->pUnits.front()->nPlayer;
        auto* player = new NWorld::CPlayer(L"Sequence side", game, campaign, side);
        RegisterPlayer(player);
        CObj<NAI::CSequenceCommander> commander = new NAI::CSequenceCommander(this);
        commander->SetPlayer(player);
        player->SetCommander(commander);
    }
};

void MapDeploymentReactions(const char* databasePath, const char* resourcePath, bool sequence)
{
    NWorld::CWorld* (*sequenceFactory)(NRPG::CGlobalGame*) = [](NRPG::CGlobalGame* game) -> NWorld::CWorld* {
        return new SequenceDeploymentWorld(game);
    };
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game, 810, sequence ? sequenceFactory : nullptr);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    size_t checked = 0;
    for (const auto& unit : units) {
        if (!unit || !unit->CanFight() || unit->IsEmptyPK() || unit->IsAIUnit() == sequence) continue;
        auto* brain = NAI::GetAIUnit(unit);
        Require(brain, "authored unit has no world-owned control object");
        Require(bool(brain->GetReaction()) == !sequence,
            sequence ? "map deployment installed an AI reaction on a sequence-controlled unit"
                     : "map deployment omitted the reaction of an AI-controlled unit");
        ++checked;
    }
    Require(checked != 0, "map has no eligible unit on the requested side");
}

class AICollectionCallbackException: public std::runtime_error
{
public:
    AICollectionCallbackException(): std::runtime_error("expected AI collection callback exception") {}
};

class ThreatOwnerFixture: public NAI::AIUnitFixture
{
    OBJECT_NOCOPY_METHODS(ThreatOwnerFixture);
public:
    NAI::SAIUnitState threat;
    NAI::SAIState* shared = nullptr;
    bool exposeThreat = true;
    int sharedReads = 0;
    NAI::SAIState* GetAIState() override { ++sharedReads; return shared; }
    NAI::SAIUnitState* GetAIUnitState() override { return exposeThreat ? &threat : nullptr; }
};

class AIOwnerEventProbe: public NAI::IAIEvent
{
    OBJECT_NOCOPY_METHODS(AIOwnerEventProbe);
public:
    int calls = 0;
    NAI::SAIUnitState* received = nullptr;
    void Modify(NAI::SAIUnitState* state) override { ++calls; received = state; }
};

struct InlineAITrackerFields
{
    CPtr<NAI::CAIEventTrackerImpl> impl;
    int operator&(CStructureSaver& saver) { saver.Add(2, &impl); return 0; }
};

static CPtr<NAI::CAIEventTrackerImpl> ReadNativeAITracker(NAI::IAIUnit* owner)
{
    CStructureNetworkContext references;
    auto saved = BorrowedObjectFields(owner, &references);
    InlineAITrackerFields tracker;
    { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(2, &tracker); }
    Require(IsValid(tracker.impl), "native AI wrapper did not retain its inline event tracker");
    return tracker.impl;
}

static void AIEventOwnerBinding(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NAI::IAIUnit* owner = nullptr;
    for (const auto& unit: units) {
        auto* wrapper = NAI::GetAIUnit(unit);
        if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && IsValid(wrapper)) {
            actor = unit.GetPtr(); owner = wrapper; break;
        }
    }
    Require(actor && owner && owner->GetAIUnitState(), "event-owner fixture has no native AI actor");
    CObj<NAI::CAIEventTrackerImpl> restored;
    CObj<NAI::CAIEventTracker> legacyAdapter;
    CPtr<NAI::CAIEventTrackerImpl> impl = ReadNativeAITracker(owner);
    CObj<ThreatOwnerFixture> boundOwner;
    if (mode == 0) {
        // The serialized owner must win over a fresh global lookup for the same
        // server, including retained callbacks across wrapper replacement.
        boundOwner = new ThreatOwnerFixture;
        boundOwner->server = actor;
        boundOwner->threat.SetUnit(boundOwner);
        owner = boundOwner.GetPtr();
    }

    if (mode != 1) {
        restored = new NAI::CAIEventTrackerImpl;
        CStructureNetworkContext references;
        references.externalReferences = true;
        references.ids[actor] = 1; references.refs[1] = actor;
        references.ids[owner] = 2; references.refs[2] = owner;
        references.ids[restored.GetPtr()] = 3; references.refs[3] = restored.GetPtr();
        references.nextID = 100; references.include = [](CObjectBase*) { return false; };
        CPtr<NWorld::CUnitServer> server = actor;
        CPtr<NAI::IAIUnit> ownerRef = owner;
        if (mode == 2) {
            legacyAdapter = new NAI::CAIEventTracker(actor);
            references.ids[legacyAdapter.GetPtr()] = 4; references.refs[4] = legacyAdapter.GetPtr();
        }
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
          writer.Add(2, &server);
          if (mode == 2) writer.Add(3, &legacyAdapter); else writer.Add(3, &ownerRef); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); restored->operator&(reader); }
        Require(restored->GetUnit() == actor, "restored AI event tracker lost its server reference");
        impl = restored.GetPtr();
        if (mode == 2) {
            InlineAITrackerFields tracker; tracker.impl = restored.GetPtr();
            CMemoryStream legacyUnit;
            { CStructureSaver writer(legacyUnit, CStructureSaver::WRITE, &references); writer.Add(2, &tracker); }
            legacyUnit.Seek(0);
            { CStructureSaver reader(legacyUnit, CStructureSaver::READ, &references);
              static_cast<CObjectBase*>(owner)->operator&(reader); }
            impl = ReadNativeAITracker(owner);
        }
    }
    if (mode == 1) {
        CStructureNetworkContext references;
        auto saved = BorrowedObjectFields(impl, &references);
        CPtr<NAI::IAIUnit> serializedOwner;
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(3, &serializedOwner); }
        Require(serializedOwner.GetPtr() == owner,
            "fresh AI event tracker serialized its adapter instead of the owning AI unit identity");
    }
    const int ap = actor->GetRPG()->GetAP();
    CObj<AIOwnerEventProbe> event = new AIOwnerEventProbe;
    impl->ThrowAIEvent(event);
    Require(event->calls == 1 && event->received == owner->GetAIUnitState(),
        "restored owner-identity AI event tracker did not deliver to its bound unit threat state");
    impl->ThrowAIEvent(nullptr);
    Require(event->calls == 1 && actor->GetRPG()->GetAP() == ap,
        "AI event owner binding duplicated delivery or changed live action points");
    Require(event->received->enemies.nLock == 0 && event->received->possibleEnemies.nLock == 0
            && event->received->allies.nLock == 0, "AI owner dispatch retained contact locks after returning");
    if (mode == 0) {
        boundOwner->Invalidate();
        impl->ThrowAIEvent(event);
        Require(event->calls == 1, "invalid bound AI owner was silently replaced by a global lookup");
    }

}

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

class AIImmediateEventProbe: public NAI::IAIEvent
{
    OBJECT_NOCOPY_METHODS(AIImmediateEventProbe);
public:
    CPtr<NAI::CAIEventTrackerImpl> tracker;
    CObj<NAI::IAIEvent> nested;
    int expectedDepth = 0, calls = 0;
    bool throwAfterDelivery = false;
    void Modify(NAI::SAIUnitState* state) override {
        ++calls;
        std::printf("world AI callback depth=%d expected=%d\n", state->enemies.nLock, expectedDepth);
        Require(state->enemies.nLock == expectedDepth && state->possibleEnemies.nLock == expectedDepth
                && state->allies.nLock == expectedDepth,
            "world AI event incorrectly acquired reaction-side contact collection locks");
        state->enemies.SetModified(); state->possibleEnemies.SetModified(); state->allies.SetModified();
        Require(state->enemies.bModified == (expectedDepth < 1)
                && state->possibleEnemies.bModified == (expectedDepth < 1)
                && state->allies.bModified == (expectedDepth < 1),
            "world AI event did not respect its inherited contact lock depth");
        if (IsValid(nested)) tracker->ThrowAIEvent(nested);
        Require(state->enemies.nLock == expectedDepth && state->possibleEnemies.nLock == expectedDepth
                && state->allies.nLock == expectedDepth, "nested world AI event changed caller lock depths");
        if (throwAfterDelivery) throw AICollectionCallbackException();
    }
};

static void AIContactEventDispatch(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer *actor = nullptr, *enemy = nullptr, *ally = nullptr;
    NAI::IAIUnit* owner = nullptr;
    for (const auto& unit: units) {
        auto* ai = NAI::GetAIUnit(unit);
        if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && IsValid(ai)) {
            actor = unit.GetPtr(); owner = ai; break;
        }
    }
    Require(actor && owner && owner->GetAIUnitState(), "contact event fixture has no native AI actor");
    for (const auto& unit: units) {
        if (!IsValid(unit) || unit.GetPtr() == actor || !unit->CanFight() || !IsValid(NAI::GetAIUnit(unit))) continue;
        if (actor->GetDiplomacyState(unit) == NDb::DS_ENEMY) enemy = unit.GetPtr();
        else ally = unit.GetPtr();
    }
    Require(enemy && ally, "contact event fixture needs native hostile and allied contacts");
    auto* state = owner->GetAIUnitState();
    CObj<NAI::CAIEventTrackerImpl> impl = ReadNativeAITracker(owner).GetPtr();
    CObj<NAI::CAIEventTracker> reactionTracker = new NAI::CAIEventTracker(actor);
    auto clear = [&]() {
        state->enemies.data.units.clear(); state->possibleEnemies.data.units.clear(); state->allies.data.units.clear();
        state->enemies.data.positions.clear(); state->possibleEnemies.data.positions.clear(); state->allies.data.positions.clear();
        state->enemies.bModified = state->possibleEnemies.bModified = state->allies.bModified = false;
        state->enemies.nLock = state->possibleEnemies.nLock = state->allies.nLock = 0;
        state->pEnemy = state->pPossibleEnemy = state->pAlly = nullptr;
        state->selfModified.data = state->selfModified.bModified = false;
    };
    const int ap = actor->GetRPG()->GetAP(), hp = actor->GetUnitRPG()->GetTotalVP();
    const auto position = actor->GetPosition();
    clear();
    if (mode == 0) {
        CObj<AIImmediateEventProbe> outer = new AIImmediateEventProbe, inner = new AIImmediateEventProbe;
        outer->tracker = inner->tracker = impl.GetPtr(); outer->nested = inner.GetPtr();
        impl->ThrowAIEvent(outer);
        Require(outer->calls == 1 && inner->calls == 1, "nested immediate event was lost or duplicated");
        state->Update();
        Require(!state->enemies.bModified && !state->possibleEnemies.bModified && !state->allies.bModified,
            "immediate event left dirty contact flags unconsumed after Update");
        clear(); outer->nested = nullptr; outer->throwAfterDelivery = true;
        bool caught = false;
        try { impl->ThrowAIEvent(outer); } catch (const AICollectionCallbackException&) { caught = true; }
        Require(caught && state->enemies.nLock == 0 && state->possibleEnemies.nLock == 0 && state->allies.nLock == 0,
            "throwing immediate event corrupted collection locks");
        clear(); outer->throwAfterDelivery = false; outer->expectedDepth = 2;
        state->enemies.nLock = state->possibleEnemies.nLock = state->allies.nLock = 2;
        impl->ThrowAIEvent(outer);
        Require(state->enemies.nLock == 2 && state->possibleEnemies.nLock == 2 && state->allies.nLock == 2,
            "immediate delivery erased inherited locks");
        state->enemies.nLock = state->possibleEnemies.nLock = state->allies.nLock = 0;
    } else if (mode == 1) {
        CObj<NAI::IAIEvent> confirmed = NAI::CreateAIEnemyEvent(NAI::GetAIUnit(enemy));
        CObj<NAI::IAIEvent> allied = NAI::CreateAIAllyNeedHelpEvent(NAI::GetAIUnit(ally));
        reactionTracker->Notify(confirmed); reactionTracker->Notify(allied);
        std::printf("reaction contact flags=%d,%d,%d\n", state->enemies.bModified,
            state->possibleEnemies.bModified, state->allies.bModified);
        Require(state->enemies.data.units.size() == 1 && state->allies.data.units.size() == 1,
            "reaction notification failed to edit actual native contact membership");
        Require(!state->enemies.bModified && !state->possibleEnemies.bModified && !state->allies.bModified,
            "reaction-side contact edits incorrectly armed new collection dirty flags");
        state->Update();
        Require(!IsValid(state->pEnemy) && !IsValid(state->pAlly) && !state->IsModified(),
            "reaction-side edits unexpectedly triggered threat selection or another reaction");
        state->enemies.bModified = state->possibleEnemies.bModified = state->allies.bModified = true;
        reactionTracker->Notify(confirmed);
        Require(state->enemies.bModified && state->possibleEnemies.bModified && state->allies.bModified,
            "reaction-side notification discarded pre-existing contact dirty flags");
        state->Update();
        Require(state->pEnemy.GetPtr() == NAI::GetAIUnit(enemy) && state->pAlly.GetPtr() == NAI::GetAIUnit(ally)
                && !state->enemies.bModified && !state->possibleEnemies.bModified && !state->allies.bModified,
            "pre-existing contact updates failed to derive native targets after notification");
    } else if (mode == 2) {
        // Use the actual globally registered world producer/consumer path, not a probe event.
        NGlobal::ThrowEvent(NWorld::CEventOnSeeNewEnemy(actor, enemy, world->IsRealTime()));
        Require(state->enemies.data.units.size() == 1 && state->enemies.bModified,
            "new world sighting failed to arm its actual contact update");
        state->Update();
        Require(state->pEnemy.GetPtr() == NAI::GetAIUnit(enemy) && state->IsModified() && !state->enemies.bModified,
            "world sighting failed to select the native hostile and arm its reaction");
        std::printf("global sighting: contact selected, reaction armed, locks=%d\n", state->enemies.nLock);
    } else {
        CObj<ThreatOwnerFixture> bound = new ThreatOwnerFixture;
        bound->server = actor; bound->threat.SetUnit(bound);
        CObj<NAI::CAIEventTrackerImpl> guarded = new NAI::CAIEventTrackerImpl(bound, actor);
        CObj<AIOwnerEventProbe> event = new AIOwnerEventProbe;
        guarded->ThrowAIEvent(event);
        Require(event->calls == 1 && event->received == &bound->threat, "valid bound state did not receive event");
        guarded->ThrowAIEvent(nullptr);
        bound->threat.SetUnit(nullptr); guarded->ThrowAIEvent(event);
        bound->threat.SetUnit(bound); bound->server = nullptr; guarded->ThrowAIEvent(event);
        bound->server = actor; bound->exposeThreat = false; guarded->ThrowAIEvent(event);
        bound->exposeThreat = true;
        CObj<AIOwnerEventProbe> invalid = new AIOwnerEventProbe; invalid->Invalidate();
        guarded->ThrowAIEvent(invalid);
        Require(event->calls == 1 && invalid->calls == 0, "invalid event or state owner passed immediate delivery guard");
        bound->Invalidate(); guarded->ThrowAIEvent(event);
        Require(event->calls == 1, "invalid bound owner was replaced during immediate event delivery");
    }
    Require(state->enemies.nLock == 0 && state->possibleEnemies.nLock == 0 && state->allies.nLock == 0,
        "contact delivery retained locks after returning");
    Require(actor->GetRPG()->GetAP() == ap && actor->GetUnitRPG()->GetTotalVP() == hp && actor->GetPosition() == position,
        "contact event delivery changed native AP, health or position");
}

static void AIContactMembership(const char* databasePath, const char* resourcePath, int kind, int check)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer *actor = nullptr, *hostile = nullptr, *friendly = nullptr;
    NAI::IAIUnit* owner = nullptr;
    for (const auto& unit: units) {
        auto* ai = NAI::GetAIUnit(unit);
        if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && IsValid(ai)) {
            actor = unit.GetPtr(); owner = ai; break;
        }
    }
    Require(actor && owner, "contact membership fixture has no native AI owner");
    for (const auto& unit: units) {
        if (!IsValid(unit) || unit.GetPtr() == actor || !unit->CanFight() || !IsValid(NAI::GetAIUnit(unit))) continue;
        if (actor->GetDiplomacyState(unit) == NDb::DS_ENEMY) hostile = unit.GetPtr();
        else friendly = unit.GetPtr();
    }
    Require(hostile && friendly, "contact membership fixture needs native hostile and friendly units");
    // SetPosition legitimately publishes world visibility events, promoting suspects
    // in the owner's actual state. Isolate only the pose-refresh consumer so that
    // those separate, working callbacks cannot replace the cache under test.
    NAI::SAIUnitState refreshState;
    auto* state = check == 1 ? &refreshState : owner->GetAIUnitState();
    if (check == 1) state->SetUnit(owner);
    Require(state != nullptr, "contact membership fixture has no threat state");
    state->enemies.data.units.clear(); state->enemies.data.positions.clear();
    state->possibleEnemies.data.units.clear(); state->possibleEnemies.data.positions.clear();
    state->allies.data.units.clear(); state->allies.data.positions.clear();
    auto* list = kind == 0 ? &state->enemies : kind == 1 ? &state->possibleEnemies : &state->allies;
    auto* selected = kind == 0 ? &state->pEnemy : kind == 1 ? &state->pPossibleEnemy : &state->pAlly;
    auto* target = kind == 2 ? friendly : hostile;
    auto* contact = NAI::GetAIUnit(target);
    auto* other = NAI::GetAIUnit(kind == 2 ? hostile : friendly);
    const auto original = target->GetPosition(), actorPosition = actor->GetPosition();
    const int ap = actor->GetRPG()->GetAP(), hp = actor->GetUnitRPG()->GetTotalVP();
    auto add = [&](NAI::IAIUnit* unit) {
        if (kind == 0) state->AddEnemy(unit);
        else if (kind == 1) state->AddPossibleEnemy(unit);
        else state->AddAlly(unit);
    };
    auto remove = [&](NAI::IAIUnit* unit) {
        if (kind == 0) state->RemoveEnemy(unit);
        else if (kind == 1) state->RemovePossibleEnemy(unit);
        else state->RemoveAlly(unit);
    };
    auto cached = [&]() {
        auto it = list->data.positions.find(contact);
        return it != list->data.positions.end() && it->second == target->GetPosition();
    };
    list->nLock = 0; list->bModified = false;
    state->selfModified.data = state->selfModified.bModified = false;
    if (check == 0) {
        add(contact);
        std::printf("contact kind=%d members=%zu poses=%zu\n", kind, list->data.units.size(), list->data.positions.size());
        Require(list->data.units.size() == 1 && cached() && list->bModified,
            "adding native AI contact failed to record its complete current pose");
        CStructureNetworkContext references;
        references.externalReferences = true; references.include = [](CObjectBase*) { return false; };
        references.ids[contact] = 1; references.refs[1] = contact;
        auto* network = original.pos.pNet.GetPtr();
        Require(network != nullptr, "contact pose has no native navigation network");
        references.ids[network] = 2; references.refs[2] = network; references.nextID = 100;
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); writer.Add(2, &list->data); }
        NAI::SUnitsAndPositions restored;
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(2, &restored); }
        Require(restored.units.size() == 1 && restored.units.front().GetPtr() == contact
                && restored.positions.size() == 1 && restored.positions.find(contact)->second == original,
            "native contact cache round trip lost unit identity, network, pose or run flag");
        // Old saves can contain the member list but no populated pose map.
        restored.positions.clear(); list->data = restored; list->bModified = false;
        add(contact);
        Require(list->bModified && cached() && list->data.units.size() == 1,
            "refreshing a legacy member-only contact failed to rebuild its missing pose cache");
    } else if (check == 1) {
        list->data.units.push_back(contact); list->data.positions[contact] = original;
        auto noChange = [&]() {
            list->bModified = false; add(contact);
            Require(!list->bModified && list->data.units.size() == 1 && cached(),
                "unchanged AI contact dirtied its collection or duplicated its membership");
        };
        noChange();
        auto plan = original.pos; plan.p.SetDirection((plan.p.GetDirection()+3)&7);
        contact->SetPosition(plan);
        Require(target->GetPosition() == original, "planning control moved the native server");
        noChange();
        for (int change = 0; change != 4; ++change) {
            auto moved = target->GetPosition();
            if (change == 0) moved.pos.p.SetDirection((moved.pos.p.GetDirection()+1)&7);
            if (change == 1) moved.SetPose(moved.GetPose() == NAI::CRAWL ? NAI::CROUCH : NAI::CRAWL);
            if (change == 2) moved.bRun = !moved.bRun;
            if (change == 3) list->data.positions[contact].pos.pNet = nullptr;
            if (change != 3) target->SetPosition(moved);
            list->bModified = false; add(contact);
            Require(list->bModified && cached() && list->data.units.size() == 1,
                "AI contact refresh missed live direction/pose/run/network changes");
            noChange();
        }
        list->nLock = 2;
        auto moved = target->GetPosition(); moved.bRun = !moved.bRun; target->SetPosition(moved);
        list->bModified = false; add(contact);
        Require(!list->bModified && cached() && list->nLock == 2,
            "locked contact refresh lost the new pose or synthesized a dirty mark");
        list->bModified = true; add(contact);
        Require(list->bModified && list->nLock == 2, "unchanged contact erased a pre-existing locked dirty mark");
        list->nLock = 0;
        target->SetPosition(original);
    } else if (check == 2) {
        list->data.units = {contact, other};
        list->data.positions[contact] = original; list->data.positions[other] = other->GetUnitPosition();
        *selected = other; remove(contact);
        Require(list->data.units.size() == 1 && list->data.positions.size() == 1
                && list->data.positions.find(contact) == list->data.positions.end()
                && list->data.positions.find(other) != list->data.positions.end()
                && selected->GetPtr() == other && list->bModified && !state->IsModified(),
            "removing AI contact retained its pose cache or changed the unrelated selected contact");
        list->bModified = false; *selected = contact; remove(contact);
        Require(!list->bModified && !IsValid(*selected) && !state->IsModified(),
            "removing an absent AI contact dirtied the collection or retained a consumed selection");
        *selected = other; list->bModified = false; remove(other);
        Require(list->data.units.empty() && list->data.positions.empty() && !IsValid(*selected) && list->bModified,
            "removing the last AI contact retained membership, pose or selection");
    } else {
        add(other);
        Require(list->data.units.empty() && list->data.positions.empty() && !list->bModified,
            "AI contact membership accepted a unit with incompatible diplomacy");
        add(nullptr); add(owner);
        Require(list->data.units.empty() && !list->bModified, "AI contact membership accepted null or itself");
        state->SetUnit(nullptr); add(contact); state->SetUnit(owner);
        Require(list->data.units.empty() && !list->bModified, "ownerless threat state accepted a contact");
        CObj<NAI::AIUnitFixture> missingServer = new NAI::AIUnitFixture;
        add(missingServer);
        Require(list->data.units.empty() && !list->bModified, "contact membership accepted a missing native server");
        if (kind == 1) {
            state->enemies.data.units.push_back(contact);
            add(contact);
            Require(list->data.units.empty() && !list->bModified, "confirmed hostile was also accepted as a possible enemy");
            state->enemies.data.units.clear();
        }
        target->MakeUnconscious(VNULL3, true, false);
        Require(!target->CanFight(), "inactive contact control remained fight-capable");
        add(contact);
        Require(list->data.units.empty() && list->data.positions.empty() && !list->bModified,
            "AI contact membership accepted an incapacitated unit");
    }
    Require(actor->GetRPG()->GetAP() == ap && actor->GetUnitRPG()->GetTotalVP() == hp
            && actor->GetPosition() == actorPosition, "contact bookkeeping changed owner AP, HP or position");
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
    for (int routine = 0; routine < 4; ++routine) {
        CObj<NAI::CAIRouteLogic> route = new NAI::CAIRouteLogic(ai);
        route->SetCircled(true);
        if (routine == 1) route->AddCommand(new NAI::CTaskCommandChangePose(NAI::WALK));
        if (routine == 2) route->AddCommand(new NAI::CTaskCommandGoto(original.pos));
        if (routine == 3) route->AddCommand(new NAI::CTaskCommandRoaming(original.pos.p, 1));
        route->AddCommand(new NAI::CTaskCommandWait(100));
        route->AddCommand(new NAI::CTaskCommandChangeDirection(0));
        ai->SetLogic(route);
        actor->SetTemporaryPosition(original);
        Require(!commander->IsEndOfTurn(), "normal AI route was incorrectly skipped");
        actor->SetTemporaryPosition(inactive);
        if (routine == 3) {
            vector<NAI::SPathPlace> reachable;
            NAI::GetNearestPlaces(actor, inactive.pos.p, 1, NAI::WALK, &reachable);
            Require(reachable.empty(), "inactive roaming fixture unexpectedly has a legal move");
        }
        Require(commander->IsEndOfTurn() == (routine != 2),
            routine == 2 ? "a route with movement was incorrectly skipped"
            : routine == 3 ? "inactive roaming with no legal move blocks the AI turn"
            : "inactive stationary AI route blocks the end of turn");
    }
    actor->SetTemporaryPosition(original);
}

static void RoamingUsesCurrentPosition(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    auto* network = world->GetPathNetwork();
    const int radius = 10;
    for (auto actor : units) {
        if (!IsValid(actor) || !actor->CanFight()) continue;
        auto* owner = actor->GetTBSPlayer();
        if (!owner || !dynamic_cast<NAI::CAICommander*>(owner->GetCommander())) continue;
        world->GivePlayerTurn(owner);
        auto current = actor->GetPosition();
        vector<NAI::SPathPlace> nearby;
        NAI::GetNearestPlaces(actor, current.pos.p, radius, NAI::WALK, &nearby);
        std::printf("roaming fixture actor=%d pose=%d AP=%d nearby=%zu\n",
            actor->GetUnitRPG()->GetRPGPersID(), int(current.pos.p.GetPose()), actor->GetAP(), nearby.size());
        if (nearby.empty()) continue;
        vector<NAI::SPathPlace> distant;
        network->GetNearPlaces(SSphere(current.GetCP(), 20.f), &distant);
        for (auto stored : distant) {
            if (!stored.IsIntegral() || stored.IsFinal()) continue;
            stored.SetPose(NAI::CM_STAND);
            stored.SetMoving(0);
            if (!network->IsPassable(stored) || fabs2(NAI::SPosition(stored, network).GetCP() - current.GetCP()) < 16.f)
                continue;

            CObj<NAI::CTaskCommandRoaming> step = new NAI::CTaskCommandRoaming(stored, radius);
            step->SetUnitServer(actor);
            const auto wishPose = actor->GetWishPose();
            const auto ap = actor->GetAP();
            auto random = GlobalGameRandom();
            auto destination = nearby[random.Get(static_cast<unsigned>(nearby.size()))];
            destination.SetPose(NAI::CM_STAND);
            step->Do();
            CObj<NWorld::CCmd> command = step->GetCommand();
            auto* path = dynamic_cast<NWorld::CCmdPath*>(command.GetPtr());
            Require(path && path->ptDst.p == destination, "AI roaming searched around the saved origin instead of the current position");
            auto used = GlobalGameRandom();
            Require(actor->GetPosition() == current && actor->GetAP() == ap && actor->GetWishPose() == wishPose &&
                used.Get() == random.Get(), "planning a roaming step changed the actor or consumed extra random draws");
            return;
        }
    }
    Require(false, "roaming fixture lacks a moving actor with two valid, disjoint origins");
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
}

bool RunAITests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "ai-self-healing-bandaged-health") && argc == 4) { SelfHealingWithBandagedHealth(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "map-deployment-reactions") && argc == 5) { MapDeploymentReactions(argv[2], argv[3], std::atoi(argv[4]) != 0); return true; }
    if (!std::strcmp(mode, "ai-reaction-reset-contacts")) { AIReactionResetContacts(); return true; }
    if (!std::strcmp(mode, "ai-reaction-value-latch") && argc == 4) { AIReactionValueLatch(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-event-collection-locks") && argc == 4) { AIEventCollectionLocks(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-immediate-event-dispatch") && argc == 4) { AIContactEventDispatch(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "ai-reaction-contact-edits") && argc == 4) { AIContactEventDispatch(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "ai-world-sighting-dispatch") && argc == 4) { AIContactEventDispatch(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "ai-immediate-event-guards") && argc == 4) { AIContactEventDispatch(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "ai-contact-cache-enemy") && argc == 4) { AIContactMembership(argv[2], argv[3], 0, 0); return true; }
    if (!std::strcmp(mode, "ai-contact-cache-possible") && argc == 4) { AIContactMembership(argv[2], argv[3], 1, 0); return true; }
    if (!std::strcmp(mode, "ai-contact-cache-ally") && argc == 4) { AIContactMembership(argv[2], argv[3], 2, 0); return true; }
    if (!std::strcmp(mode, "ai-contact-refresh-enemy") && argc == 4) { AIContactMembership(argv[2], argv[3], 0, 1); return true; }
    if (!std::strcmp(mode, "ai-contact-refresh-possible") && argc == 4) { AIContactMembership(argv[2], argv[3], 1, 1); return true; }
    if (!std::strcmp(mode, "ai-contact-refresh-ally") && argc == 4) { AIContactMembership(argv[2], argv[3], 2, 1); return true; }
    if (!std::strcmp(mode, "ai-contact-remove-enemy") && argc == 4) { AIContactMembership(argv[2], argv[3], 0, 2); return true; }
    if (!std::strcmp(mode, "ai-contact-remove-possible") && argc == 4) { AIContactMembership(argv[2], argv[3], 1, 2); return true; }
    if (!std::strcmp(mode, "ai-contact-remove-ally") && argc == 4) { AIContactMembership(argv[2], argv[3], 2, 2); return true; }
    if (!std::strcmp(mode, "ai-contact-eligibility-enemy") && argc == 4) { AIContactMembership(argv[2], argv[3], 0, 3); return true; }
    if (!std::strcmp(mode, "ai-contact-eligibility-possible") && argc == 4) { AIContactMembership(argv[2], argv[3], 1, 3); return true; }
    if (!std::strcmp(mode, "ai-contact-eligibility-ally") && argc == 4) { AIContactMembership(argv[2], argv[3], 2, 3); return true; }
    if (!std::strcmp(mode, "restored-ai-event-owner") && argc == 4) { AIEventOwnerBinding(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "fresh-ai-event-owner") && argc == 4) { AIEventOwnerBinding(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "legacy-ai-event-owner") && argc == 4) { AIEventOwnerBinding(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "removed-selected-enemy")) { RemovedSelectedAIContact(0); return true; }
    if (!std::strcmp(mode, "removed-selected-possible")) { RemovedSelectedAIContact(1); return true; }
    if (!std::strcmp(mode, "removed-selected-ally")) { RemovedSelectedAIContact(2); return true; }
    if (!std::strcmp(mode, "owned-ai-action-enemy")) { OwnedAIEnemySelection(false); return true; }
    if (!std::strcmp(mode, "owned-ai-place-enemy")) { OwnedAIEnemySelection(true); return true; }
    if (!std::strcmp(mode, "ai-idle") && argc == 4) { IdleAI(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-roaming-current-position") && argc == 4) { RoamingUsesCurrentPosition(argv[2], argv[3]); return true; }
    return false;
}
}
