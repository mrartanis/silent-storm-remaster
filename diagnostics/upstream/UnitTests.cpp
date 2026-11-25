#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/wMainPath.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wUnitCommands.h"
#include "../../Main/wUnitExec.h"
#include "../../Main/wUnitStates.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGCritical.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemMap.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/wUnitAttackExec.h"
#include "../../Main/aiGrid.h"
#include "../../Main/aiMoves.h"
#include "../../Main/aiPath.h"
#include "../../Main/A5Time.h"
#include "../../Main/GAnimation.h"
#include "../../Main/rpgPerk.h"
#include "../../Main/rpgPerkConstants.h"
#include "../../Main/rpgGlobal.h"
#include "../../Main/A5Script.h"
#include "../../Main/scriptPtr.h"
#include "../../Main/wAckBase.h"
#include "../../DBFormat/DataRPG.h"
#include "../../MiscDll/LogStream.h"
#include "../../Misc/RandomGen.h"
#include <cstring>

namespace UpstreamTests
{
namespace
{
void CorpseDensity(const char* databasePath, const char* resourcePath, bool crowded)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* anchor = MovementPolicyActor(world);
    const auto position = anchor->GetPosition().pos.p;
    vector<CObj<NWorld::CUnitServer>> bodies;
    const int count = crowded ? 17 : 15;
    auto add = [&](bool clue, const string& name) {
        auto* unit = world->AddUnitInGame(position, NRPG::CreateUnit(NDb::GetPers(103)), anchor->GetTBSPlayer(), name);
        Require(IsValid(unit), "corpse density fixture could not add a unit to the native world");
        unit->nClueCount = clue ? 1 : 0;
        return unit;
    };
    for (int i = 0; i < count; ++i) {
        auto* unit = add(false, "CorpseDensity" + std::to_string(i));
        unit->GetUnitRPG()->InitAsCorpse(true);
        unit->Die();
        unit->tDeathTime = 1000 + i;
        unit->SetPosition(anchor->GetPosition());
        bodies.emplace_back(unit);
    }
    auto* clue = add(true, "ProtectedDensityClue");
    clue->GetUnitRPG()->InitAsCorpse(true);
    clue->Die();
    clue->tDeathTime = 10;
    clue->SetPosition(anchor->GetPosition());
    auto* unconscious = add(false, "ProtectedDensityUnconscious");
    unconscious->GetUnitRPG()->InitAsCorpse(false);
    unconscious->SetState(new NWorld::CUnitStateUnconscious(unconscious));
    unconscious->tDeathTime = 5;
    unconscious->SetPosition(anchor->GetPosition());
    Require(clue->IsDead() && clue->IsClueUnit() && unconscious->IsUnconscious() && !unconscious->IsDead() && anchor->CanFight(),
        "native corpse density fixture lacks living or protected-body controls");

    for (int pass = 0; pass < 2; ++pass) {
        world->UpdateVisible(true);
        for (int i = 0; i < count; ++i)
            Require(bodies[i]->IsDead() && bodies[i]->IsAddedToVisitor() == (i >= count - 15),
                "native visibility refresh failed to keep exactly the fifteen newest ordinary corpses");
        Require(clue->IsAddedToVisitor() && unconscious->IsAddedToVisitor() && anchor->IsAddedToVisitor(),
            "corpse density cleanup hid a living unit, unconscious unit or quest clue");
    }
}

void PathCommandGoal(const char* databasePath, const char* resourcePath, bool approximate)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    auto* network = dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());
    Require(IsValid(network), "path command fixture has no native path network");
    const auto start = actor->GetPosition();
    const auto origin = start.pos.p;
    const auto* layer = network->GetLayer(origin.GetLayer());
    const int ap = actor->GetAP();
    for (int dy = -6; dy <= 6; ++dy) {
        for (int dx = -6; dx <= 6; ++dx) {
            const int x = int(origin.GetX()) + dx, y = int(origin.GetY()) + dy;
            if (dx * dx + dy * dy < 9 || x <= 0 || y <= 0 ||
                x >= layer->tiles.GetXSize() - 1 || y >= layer->tiles.GetYSize() - 1)
                continue;
            auto destination = origin;
            destination.SetXY(x, y);
            if (network->GetPassability(destination) != NAI::AIP_NOT_PASSABLE)
                continue;
            const NAI::SPosition target(destination, network);
            CObj<NWorld::CCmdPath> command = new NWorld::CCmdPath(target, NAI::PF_USE_POSEDIR);
            const vector<NAI::SPathPlace> goals{destination};
            CObj<NAI::CPath> fallback = NWorld::FindPath(network, actor, origin, goals, nullptr, true,
                command->eParams, actor->IsStrafing(), true);
            CObj<NAI::CPath> exact = NWorld::FindPath(network, actor, origin, goals, nullptr, true,
                command->eParams, actor->IsStrafing(), false);
            if (!IsValid(fallback) || IsValid(exact))
                continue;
            command->bCanFindNotExact = approximate;
            NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
            CObj<NWorld::CCommandExecute> action = actor->CreateExecutor(command, &result);
            if (approximate)
                Require(action && result == NWorld::UCR_OK, "explicit approximate path command lost its reachable fallback");
            else
                Require(!action && result == NWorld::UCR_PATH_NOT_FOUND, "exact path command accepted a nearby tile for a blocked goal");
            Require(actor->GetPosition() == start && actor->GetAP() == ap, "path command validation moved the unit or spent AP");
            return;
        }
    }
    Require(false, "native path command fixture lacks a blocked goal with a reachable approximation");
}

void DifficultySkillScaling()
{
    CObj<NRPG::CDynamicSkill> reduced = new NRPG::CDynamicSkill(27, 20);
    reduced->ScaleForDifficulty(0.8f);
    Require(int(*reduced) == 21 && reduced->GetMaxValue() == 21 && reduced->GetXPPart() == 7,
        "AI difficulty rounded up a fractional skill cap");

    CObj<NRPG::CDynamicSkill> increased = new NRPG::CDynamicSkill(27, 20);
    increased->ScaleForDifficulty(1.25f);
    Require(int(*increased) == 27 && increased->GetMaxValue() == 33 && increased->GetXPPart() == 7,
        "raising the difficulty cap rounded up or increased the unit's current value");
}

void TakePerkDuringTreatment(const char* databasePath, const char* resourcePath)
{
    using namespace NWorld;
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    world->GivePlayerTurn(actor->GetTBSPlayer());
    auto* mission = actor->GetUnitRPG();
    auto* unit = mission->GetRPGUnit();
    auto* inventory = mission->GetInventory();
    auto* record = NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(1);
    Require(record && record->effect == NDb::FAE_NORMAL, "fixture lacks the ordinary first-aid kit");
    CObj<NRPG::CFirstAidItem> kit = new NRPG::CFirstAidItem(record);
    if (inventory->Get(NDb::SLOT_1))
        inventory->TakeOff(NDb::SLOT_1);
    Require(inventory->Equip(NDb::SLOT_1, kit) && inventory->Activate(NDb::SLOT_1),
        "fixture could not ready the first-aid kit");
    actor->animator.SetActiveItem(true);
    actor->animator.SetWeaponAnimation(mission->GetWeaponType());
    auto& hp = unit->Skills(NDb::ST_VP);
    hp.SetValue(hp.GetMaxValue() / 2);
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_ANY, NDb::C_VP));
    world->GetAIMap()->Sync();

    auto* perks = unit->GetPerksTree();
    perks->AddPerkPoints(1);
    vector<CPtr<NRPG::CPerk>> available;
    perks->GetAvailablePerks(&available);
    Require(!available.empty(), "fixture has no purchasable perk");
    const int id = available.front()->GetDBPerk()->GetRecordID();
    const int points = perks->GetPerkPoints();
    CObj<CCmdTakePerk> purchase = new CCmdTakePerk(id);
    EUnitCommandResult result = UCR_UNAVAILABLE;
    CObj<CCommandExecute> order = actor->CreateExecutor(purchase, &result);
    Require(order && result == UCR_OK && !unit->HasPerk(id), "idle unit cannot validate its available perk");

    CObj<CExecHeal> treatment = new CExecHeal(actor, actor);
    Require(treatment->CanDoIt(actor->GetPosition()) == UCR_OK, "fixture cannot begin treating its wound");
    treatment->Run();
    treatment->AnimationFinished();
    Require(mission->HasCritical(NDb::C_PATIENT), "fixture did not remain in the treatment state");
    const int ap = actor->GetAP();
    const int doses = kit->GetIncQuantity();
    order = actor->CreateExecutor(purchase, &result);
    Require(order && result == UCR_OK, "active treatment blocked an available perk purchase");
    order->Run();
    Require(unit->HasPerk(id) && perks->GetPerkPoints() == points - 1 && actor->GetAP() == ap &&
        kit->GetIncQuantity() == doses && mission->HasCritical(NDb::C_PATIENT),
        "purchasing a perk did not spend precisely one point or interrupted the treatment");
    order = actor->CreateExecutor(purchase, &result);
    Require(!order && result == UCR_GENERAL_FAILURE, "treatment bypassed ordinary perk eligibility");
}

void SelectedCriticalTreatmentLog(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    auto* persona = NDb::GetPers(103);
    Require(IsValid(persona), "treatment log fixture lacks an authored patient");
    CObj<NRPG::IUnitMission> patient = NRPG::CreateUnit(persona);
    CObj<NRPG::IUnitMission> control = NRPG::CreateUnit(persona);
    NRPG::SCritical injury(NDb::CL_HEAD, NDb::C_BLEEDING, -1, 2.f, 3);
    NRPG::SCritical separate(NDb::CL_LEGS, NDb::C_BLEEDING, -1, 4.f, 5);
    patient->ApplyCritical(injury);
    patient->ApplyCritical(separate);
    control->ApplyCritical(injury);

    class Capture : public ILogNotify {
        OBJECT_NOCOPY_METHODS(Capture);
    public:
        vector<wstring> cured;
        void OnAddConsoleLine(const SConsoleLine& line) override {
            if (line.eType == STREAM_RPG && line.szText.find(L"Cured:") != wstring::npos)
                cured.push_back(line.szText);
        }
    };
    CObj<Capture> log = new Capture;
    SetLogNotify(log);
    struct RestoreNotify { ~RestoreNotify() { SetLogNotify(nullptr); } } restore;
    Require(control->HealCriticals(3, true) == 3 && log->cured.size() == 1,
        "the ordinary critical treatment control did not log its cure");
    const auto expected = log->cured.front();
    log->cured.clear();
    const auto choice = patient->TryHealCritical(3, true);
    Require(choice.eCritical == NDb::C_BLEEDING && choice.eCl == NDb::CL_HEAD && choice.nDC == 3,
        "treatment did not select the intended head injury");
    Require(patient->HealCritical(choice) == 3 && patient->TryHealCritical(5, true).eCl == NDb::CL_LEGS,
        "treatment changed the result or removed the separate leg injury");
    Require(log->cured.size() == 1 && log->cured.front() == expected,
        "targeted treatment did not log the cured injury with its location and difficulty");
    Require(patient->HealCritical(choice) == 0 && log->cured.size() == 1,
        "targeted treatment logged a cure when the requested injury was absent");
}

void PanzerkleinRegenerationBleeding(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    auto* persona = NDb::GetPers(103);
    Require(IsValid(persona), "PK regeneration fixture lacks an authored pilot");
    CObj<NRPG::IUnitMission> mission = NRPG::CreateUnit(persona);
    auto& hp = mission->GetRPGUnit()->Skills(NDb::ST_VP);
    hp.SetValue(hp.GetMaxValue() - 10);

    CObj<NDb::CPanzerklein> suit = new NDb::CPanzerklein;
    suit->fRegenerationValue = 4.f;
    suit->bSingleSlot = false;
    CObj<NRPG::CDynamicSkill> shell = new NRPG::CDynamicSkill(100, 100);
    mission->SetPanzerklein(suit, shell, nullptr);
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD, NDb::C_BLEEDING, -1, 2.f, NRPG::N_MAX_DC));
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_LEGS, NDb::C_BLEEDING, -1, 2.f, NRPG::N_MAX_DC + 1));
    const auto first = mission->TryHealCritical(NRPG::N_MAX_DC, false);
    Require(first.eCl == NDb::CL_HEAD && first.nDC == NRPG::N_MAX_DC,
        "fixture did not retain bleeding at the normal difficulty limit");

    int bleeding = -1;
    mission->DoRegenerations(nullptr, &bleeding);
    Require(mission->GetHealedVP() == 4 && int(*shell) == 100 && bleeding > 0,
        "regeneration did not calculate bleeding and heal the pilot as before");
    Require(mission->TryHealCritical(NRPG::N_MAX_DC, false).eCritical == NDb::C_NONE,
        "regeneration did not cure bleeding at the normal difficulty limit");
    const auto remaining = mission->TryHealCritical(NRPG::N_MAX_DC + 1, false);
    Require(remaining.eCritical == NDb::C_BLEEDING && remaining.eCl == NDb::CL_LEGS && remaining.nDC == NRPG::N_MAX_DC + 1,
        "regeneration bypassed the critical difficulty limit for bleeding");
}

static void LookActionPointCost(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    world->GivePlayerTurn(actor->GetTBSPlayer());
    const auto original = actor->GetPosition();
    const int ap = actor->GetAP();
    const int rotatePrice = actor->GetActionAP(NRPG::AC_ROTATE);
    Require(rotatePrice > 0 && ap >= rotatePrice, "look test requires a priced turn and sufficient AP");
    auto turned = original.pos;
    turned.p.SetDirection((turned.p.GetDirection() + 2) & 7);
    int start = -1, total = -1;
    CObj<NWorld::CCmdLook> change = new NWorld::CCmdLook(turned);
    Require(actor->CanDo(change, &start, &total) == NWorld::UCR_OK && total >= rotatePrice,
        "a real change of facing must have a rotation cost");

    CObj<NWorld::CCmdLook> same = new NWorld::CCmdLook(original.pos);
    Require(actor->CanDo(same, &start, &total) == NWorld::UCR_OK,
        "the fixture did not prepare a valid unchanged-facing look order");
    Require(total == 0, "an unchanged-facing look order charges AP");
    Require(actor->GetAP() == ap && actor->GetPosition() == original, "querying look cost changed the actor");
}

void ForcedActionReplacement(const char* databasePath, const char* resourcePath, bool stale)
{
    using namespace NWorld;
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    world->GivePlayerTurn(actor->GetTBSPlayer());
    const auto previousPose = actor->GetWishPose();
    const auto desiredPose = previousPose == NAI::CROUCH ? NAI::WALK : NAI::CROUCH;
    const auto submit = [&] {
        actor->Do(new CCmdSetCommand(actor, new CCmdWishPose(desiredPose)));
        if (actor->HasCommand())
            actor->Do(new CCmdSetCommand(actor, new CCmdContinue));
    };
    const auto current = [&] {
        CStructureNetworkContext refs;
        auto fields = BorrowedObjectFields(actor, &refs);
        CObj<CCommandExecute> executor;
        bool forced = false;
        CStructureSaver reader(fields, CStructureSaver::READ, &refs);
        reader.Add(7, &executor);
        reader.Add(12, &forced);
        return std::make_pair(executor, forced);
    };
    Require(!current().first && !current().second, "fixture starts with an existing command");
    if (stale) {
        bool forced = true;
        CMemoryStream fields;
        { CStructureSaver writer(fields, CStructureSaver::WRITE); writer.Add(12, &forced); }
        fields.Seek(0);
        { CStructureSaver reader(fields, CStructureSaver::READ); static_cast<CObjectBase*>(actor)->operator&(reader); }
        Require(!current().first && current().second, "legacy state fixture did not restore the orphaned latch");
    } else {
        Require(IsValid(NDb::GetAnimation(4437)), "fixture lacks the authored animation");
        CObj<CExecPlayAnimation> animation = new CExecPlayAnimation(actor, 4437, false);
        actor->RunCriticalExecutor(animation);
        Require(current().first == animation && current().second && actor->IsPerformingAction(),
            "the forced animation could not start");
        submit();
        Require(current().first == animation && current().second && actor->GetWishPose() == previousPose,
            "an ordinary command replaced the running forced action");
        world->GetTime()->Set(actor->animator.GetTimeEnd());
        actor->Segment();
    }
    submit();
    Require(actor->GetWishPose() == desiredPose && !current().second,
        "the forced-action flag blocked or outlived the next ordinary command");
}

void FallDamageMinimum(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    auto* mission = actor->GetUnitRPG();
    auto expectedRandom = GlobalGameRandom();
    int expected = 0;
    for (int n = 0; n < 3; ++n)
        expected += expectedRandom.Get(0, 25);
    Require(mission->GetFallDamage(4.8f) == Max(2, expected), "the ordinary fall changed its three damage rolls");
    auto actualRandom = GlobalGameRandom();
    Require(actualRandom.Get() == expectedRandom.Get(), "the ordinary fall changed the shared RNG consumption");

    const int health = mission->GetTotalVP();
    actor->FallFromHigh(2.8f);
    Require(mission->GetTotalVP() == health, "a nonqualifying fall inflicted damage");
    expectedRandom = GlobalGameRandom();
    Require(mission->GetFallDamage(2.81f) == 2, "a qualifying fall did not impose the minimum damage");
    actualRandom = GlobalGameRandom();
    Require(actualRandom.Get() == expectedRandom.Get(), "a zero-range damage roll advanced the RNG");
    actor->FallFromHigh(2.81f);
    Require(mission->GetTotalVP() < health, "the minimum fall did not reach normal unit damage");
}

void RealTimeTurnReset(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    auto* mission = actor->GetUnitRPG();
    actor->OnTBSEvent(NWorld::TBS_START_NEW_TURN);
    actor->OnTBSEvent(NWorld::TBS_START_NEW_TURN);
    Require(!mission->IsFirstTurn(), "fixture did not advance the unit turn counter");
    const int ap = actor->GetAP();
    mission->SpendAP(1);
    Require(actor->GetAP() == ap - 1, "fixture could not exercise the usual AP reset");
    actor->OnTBSEvent(NWorld::TBS_START_REAL_TIME);
    Require(actor->GetAP() == ap, "entering real time failed to reset ordinary AP accounting");
    Require(mission->IsFirstTurn(), "entering real time kept the previous encounter's turn counter");
    actor->OnTBSEvent(NWorld::TBS_START_NEW_TURN);
    Require(mission->IsFirstTurn(), "the new encounter did not get its first turn");
    actor->OnTBSEvent(NWorld::TBS_START_NEW_TURN);
    Require(!mission->IsFirstTurn(), "the new encounter did not advance the normal turn counter");
}

static void WishPoseStrafe(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    world->GivePlayerTurn(actor->GetTBSPlayer());
    const auto position = actor->GetPosition();
    for (auto pose : {NAI::WALK, NAI::CROUCH, NAI::RUN, NAI::CRAWL}) {
        actor->SetStrafe(true);
        CObj<NWorld::CCmdWishPose> cmd = new NWorld::CCmdWishPose(pose);
        NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
        CObj<NWorld::CCommandExecute> order = actor->CreateExecutor(cmd, &result);
        Require(order && result == NWorld::UCR_OK, "the ordinary wish-pose command was unavailable");
        order->Run();
        auto expectedPosition = position;
        expectedPosition.bRun = pose == NAI::RUN;
        Require(actor->GetWishPose() == pose && actor->GetPosition() == expectedPosition,
            "a wish-pose order failed to update the desired movement mode");
        Require(actor->IsStrafing() == (pose == NAI::WALK || pose == NAI::CROUCH),
            "the wish-pose executor applied the wrong strafe policy");
    }
}

static void HideWithEmptyRosterSlot(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    actor->EnableHide();
    CObj<NWorld::CCmdHide> hide = new NWorld::CCmdHide;
    NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
    CObj<NWorld::CCommandExecute> ordinary = NWorld::CreateExecutor(actor, hide, &result);
    Require(ordinary && result == NWorld::UCR_OK, "the ordinary hide order was ineligible before adding the empty slot");
    vector<CPtr<NWorld::CPlayer>> players;
    world->GetPlayersList(&players);
    for (auto opponent : players) {
        if (!IsValid(opponent) || opponent == actor->GetTBSPlayer()) continue;
        opponent->AddUnit(nullptr);
        CObj<NWorld::CCommandExecute> withEmpty = NWorld::CreateExecutor(actor, hide, &result);
        opponent->RemoveUnit(nullptr);
        Require(withEmpty && result == NWorld::UCR_OK, "an empty opposing roster slot blocked the hide order");
        return;
    }
    Require(false, "hide fixture lacks an opposing roster");
}

static void NormalStateCommandLifetime(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    const auto initial = actor->GetWishPose();
    const auto wanted = initial == NAI::CROUCH ? NAI::WALK : NAI::CROUCH;
    const auto position = actor->GetPosition();
    const int ap = actor->GetAP();
    NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
    CObj<NWorld::CCommandExecute> executor = actor->CreateExecutor(new NWorld::CCmdWishPose(wanted), &result);
    Require(executor && result == NWorld::UCR_OK, "normal-state validation released a newly issued native command");
    Require(actor->GetWishPose() == initial && actor->GetAP() == ap && actor->GetPosition() == position,
        "creating a pose executor changed unit state");
    executor->Run();
    Require(actor->GetWishPose() == wanted, "the retained command lost its requested pose");
}

struct LadderDescentFixture
{
    CObj<NRPG::CGlobalGame> game;
    CObj<NWorld::CWorld> world;
    NWorld::CUnitServer* actor;
    NAI::SUnitPosition approach, ledge, upperRung, lowerRung, bottom;
    bool realClimb = false;

    LadderDescentFixture(const char* databasePath, const char* resourcePath)
        : world(FixtureWorld(databasePath, resourcePath, true, &game, 4517)),
          actor(MovementPolicyActor(world))
    {
        auto* network = dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());
        Require(network, "ladder descent fixture lacks the mission path network");
        for (int layer = 0; layer < network->GetNumLayers(); ++layer) {
            const auto& ladders = network->GetLayer(layer)->ladders;
            for (size_t index = 0; index < ladders.size(); ++index) {
                const auto& ladder = ladders[index];
                if (ladder.GetHeight() < 5 || ladder.placeOnTop.IsFinal() || ladder.placeOnBottom.IsFinal() ||
                    !ladder.placeOnTop.IsIntegral() || !ladder.placeOnBottom.IsIntegral())
                    continue;
                auto upper = ladder.placeOnTop;
                auto lower = ladder.placeOnBottom;
                upper.SetPose(NAI::CM_INACTIVE);
                lower.SetPose(NAI::CM_STAND);
                NAI::SPathPlace first(index, ladder.GetHeight() - 4, layer,
                    (4 + 2 * int(ladder.eDir)) & 7, NAI::CM_STAND, 0);
                upper.SetDirection(first.GetDirection());
                first.SetIntegral(0);
                auto last = first;
                last.SetY(0);
                if (!network->IsPassable(upper) || !network->IsPassable(lower) ||
                    !network->IsPassable(first) || !network->IsPassable(last) ||
                    NAI::GetTransitionType(network, upper, first) != NAI::TT_LADDER_UP ||
                    NAI::GetTransitionType(network, last, lower) != NAI::TT_LADDER_DOWN)
                    continue;
                vector<NAI::SPathPlace> approaches;
                network->GetNearPlaces(SSphere(network->GetCP(upper), 2.f), &approaches, true);
                for (auto candidate : approaches) {
                    candidate.SetPose(NAI::CM_CROUCH);
                    candidate.SetDirection(upper.GetDirection());
                    if (candidate.GetLayer() != upper.GetLayer() || !network->IsPassable(candidate))
                        continue;
                    const auto transition = NAI::GetTransitionType(network, candidate, upper);
                    realClimb = transition >= NAI::TT_CLIMB_1 && transition <= NAI::TT_CLIMB_4;
                    if (!realClimb && transition != NAI::TT_MOVE && transition != NAI::TT_MOVE_DIAGONAL)
                        continue;
                    approach.pos = NAI::SPosition(candidate, network);
                    ledge.pos = NAI::SPosition(upper, network);
                    upperRung.pos = NAI::SPosition(first, network);
                    lowerRung.pos = NAI::SPosition(last, network);
                    bottom.pos = NAI::SPosition(lower, network);
                    return;
                }
            }
        }
        Require(false, "ladder descent fixture lacks a usable authored ladder");
    }

    void Enter()
    {
        auto& animation = actor->animator;
        Require(animation.CanActivateItem(), "ordinary standing actor cannot activate an item");
        animation.Climb(approach, ledge, realClimb);
        Require(!animation.CanActivateItem(), "climb approach did not enter the inactive animation pose");
        animation.EnterLadder(ledge, upperRung, false);
        Require(!animation.CanActivateItem(), "actor can activate an item while still on the ladder");
    }

    void Ascend()
    {
        auto& animation = actor->animator;
        animation.EnterLadder(bottom, lowerRung, true);
        auto rung = lowerRung;
        while (rung.pos.p.GetLadderStep() < upperRung.pos.p.GetLadderStep()) {
            auto next = rung;
            next.pos.p.SetY(rung.pos.p.GetLadderStep() + 1);
            Require(world->GetPathNetwork()->IsPassable(next.pos.p), "ascent crossed an unavailable rung");
            animation.MoveLadder(rung, next);
            rung = next;
        }
        animation.LeaveLadder(rung, ledge, true);
    }
};

void LadderDescentItemActivation(const char* databasePath, const char* resourcePath, int mode)
{
    LadderDescentFixture fixture(databasePath, resourcePath);
    auto* actor = fixture.actor;
    auto& animation = actor->animator;
    if (mode == 3) {
        fixture.Ascend();
        Require(!animation.CanActivateItem(), "leaving the ladder onto an inactive ledge enabled item activation");
        return;
    }
    fixture.Enter();
    if (mode == 1) {
        animation.JumpFromLadder(fixture.upperRung, fixture.bottom);
        Require(animation.CanActivateItem(), "jumping off the ladder left the landing pose inactive");
        return;
    }
    if (mode == 2) {
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        auto* record = NDatabase::GetTable<NDb::CRPGFirstAid>()->GetRecord(1);
        Require(record, "ladder item fixture lacks an authored hand item");
        CObj<NRPG::CFirstAidItem> kit = new NRPG::CFirstAidItem(record);
        for (auto slot : {NDb::SLOT_1, NDb::SLOT_2})
            if (inventory->Get(slot))
                inventory->TakeOff(slot);
        Require(inventory->Equip(NDb::SLOT_1, kit) && inventory->Activate(NDb::SLOT_1) &&
            !inventory->Get(NDb::SLOT_2), "ladder item fixture could not prepare a drawn item and empty slot");
        animation.SetActiveItem(true);
        animation.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
        actor->SetUndrawItem(false);
        CObj<NWorld::CCmdSetActiveItem> select = new NWorld::CCmdSetActiveItem(NDb::SLOT_2);
        NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
        CObj<NWorld::CCommandExecute> event = NWorld::CreateExecutor(actor, select, &result);
        Require(event, "native empty-slot switch has no executor");
        event->Run();
        Require(inventory->GetActiveSlot() == NDb::SLOT_2 && !animation.CanActivateItem(),
            "inactive slot switch failed to select the empty slot or changed the ladder state");
        Require(!animation.IsActiveItem() && actor->GetUndrawItem(),
            "empty slot retained a drawn item while the actor was on a ladder");
        return;
    }
    auto rung = fixture.upperRung;
    while (rung.pos.p.GetLadderStep() > 0) {
        auto next = rung;
        next.pos.p.SetY(rung.pos.p.GetLadderStep() - 1);
        Require(fixture.world->GetPathNetwork()->IsPassable(next.pos.p), "descent crossed an unavailable rung");
        animation.MoveLadder(rung, next);
        rung = next;
    }
    Require(rung.pos.p == fixture.lowerRung.pos.p && !animation.CanActivateItem(),
        "ladder descent did not reach the lowest rung while keeping item activation blocked");
    animation.LeaveLadder(rung, fixture.bottom, false);
    Require(animation.CanActivateItem(), "descending the ladder left the landing pose inactive");
}

void ForcedFallPose(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    auto& animation = actor->animator;
    auto landing = actor->GetPosition();
    Require(animation.CanActivateItem(), "standing actor cannot activate an item before he falls");
    auto ledge = landing;
    ledge.pos.p.SetPose(NAI::CM_INACTIVE);
    animation.Climb(landing, ledge, false);
    Require(!animation.CanActivateItem(), "fall fixture did not start from an inactive ledge pose");
    const auto previousHeight = landing.GetCP().z + 100.f;
    const STime now = world->GetTime()->GetValue();
    animation.Fall(landing, previousHeight);
    Require(animation.GetTimeEnd() > now + 5000, "forced fall did not run its animated branch");
    Require(animation.CanActivateItem(), "animated fall onto an ordinary tile retained the inactive pose");
    animation.Fall(ledge, previousHeight);
    Require(animation.GetTimeEnd() > now + 5000 && !animation.CanActivateItem(),
        "animated fall onto an inactive ledge enabled item activation");
}

class PlacementWorld : public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(PlacementWorld);
public:
    explicit PlacementWorld(NRPG::CGlobalGame* game = nullptr) : CWorld(game) {}
    int visibilityUpdates = 0;

    void UpdateVisible(bool force) override
    {
        ++visibilityUpdates;
        CWorld::UpdateVisible(force);
    }
};

void OrdinaryPlacementAnchor(const char* databasePath, const char* resourcePath, bool anchorBlocked)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game, 810,
        [](NRPG::CGlobalGame* game) -> NWorld::CWorld* { return new PlacementWorld(game); });
    auto* placementWorld = static_cast<PlacementWorld*>(world.GetPtr());
    auto* actor = MovementPolicyActor(world);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    const auto other = std::find_if(units.begin(), units.end(), [&](const auto& unit) {
        return IsValid(unit) && unit.GetPtr() != actor && unit->CanFight() &&
            !unit->IsWearingPK() && !unit->IsEmptyPK();
    });
    Require(other != units.end(), "prone placement fixture lacks a second ordinary unit");
    auto* blocker = other->GetPtr();
    auto* network = world->GetPathNetwork();
    const auto from = actor->GetPosition();
    network->Unlock(actor);
    network->Unlock(blocker);

    NAI::SPathPlace prone, adjacent;
    vector<NAI::SPathPlace> nearby;
    network->GetNearPlaces(SSphere(from.GetCP(), 6.f), &nearby);
    bool found = false;
    for (auto point : nearby) {
        if (!point.IsIntegral() || point.GetLayer() != from.pos.p.GetLayer())
            continue;
        point.SetPose(NAI::CM_LAY);
        point.SetDirection(NAI::RIGHT);
        if (network->GetPassability(point) != NAI::AIP_YES || !network->IsPassable(point))
            continue;
        vector<NAI::SPathPlace> footprint;
        network->GetLockArea(&footprint, point, false);
        for (auto tile : footprint) {
            if (tile.GetLayer() != point.GetLayer() || tile.GetX() == point.GetX())
                continue;
            tile.SetPose(NAI::CM_STAND);
            if (network->GetPassability(tile) == NAI::AIP_YES) {
                prone = point;
                adjacent = tile;
                found = true;
                break;
            }
        }
        if (found)
            break;
    }
    Require(found, "native placement fixture lacks a prone anchor with an open neighboring footprint tile");
    auto position = from;
    position.pos = NAI::SPosition(prone, network);
    actor->animator.PlaceUnit(position);
    actor->SetPosition(position);
    network->Unlock(actor);
    auto blocking = blocker->GetPosition();
    if (anchorBlocked) {
        adjacent = prone;
        adjacent.SetPose(NAI::CM_STAND);
    }
    blocking.pos = NAI::SPosition(adjacent, network);
    blocker->animator.PlaceUnit(blocking);
    blocker->SetPosition(blocking);
    Require(network->GetPassability(prone) == (anchorBlocked ? NAI::AIP_LOCKED : NAI::AIP_YES) &&
        !network->IsPassable(prone), "native fixture failed to lock the intended part of the prone footprint");

    const int before = placementWorld->visibilityUpdates;
    actor->PlaceOnPassablePlace();
    if (anchorBlocked) {
        Require(actor->GetPosition().pos.p.GetBits() != prone.GetBits() && placementWorld->visibilityUpdates > before,
            "native placement kept a prone actor on a locked anchor");
    } else {
        Require(actor->GetPosition() == position && placementWorld->visibilityUpdates == before,
            "native placement repositioned an ordinary unit whose anchor was available");
    }
}

void ManualFirstTurnInterrupt(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* actor = MovementPolicyActor(world);
    auto* first = actor->GetTBSPlayer();
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    auto candidate = std::find_if(units.begin(), units.end(), [&](const auto& unit) {
        return IsValid(unit) && unit->CanFight() && unit->GetTBSPlayer() != first && unit->GetTBSPlayer() &&
            unit->GetUnitRPG()->GetRPGUnit()->GetPerksTree();
    });
    Require(candidate != units.end(), "first-turn fixture lacks the second player and perk-bearing unit");
    auto* reacting = candidate->GetPtr();
    auto* second = reacting->GetTBSPlayer();
    RestoreSinglePerk(reacting->GetUnitRPG()->GetRPGUnit(), 0x1b);
    Require(reacting->HasAutoFirstTurnInterrupt(), "native unit did not acquire the first-turn interrupt");

    vector<CPtr<NWorld::CPlayer>> players;
    world->GetPlayersList(&players);
    for (const auto& player : players)
        player->SetCommander(new NWorld::CCommander);
    auto& turns = static_cast<NWorld::CWorld::TTBSWorld&>(*world.GetPtr());
    list<NWorld::STBSEvent> empty;
    bool firstTurn = false;
    CMemoryStream fields;
    {
        CStructureSaver writer(fields, CStructureSaver::WRITE);
        writer.Add(2, &empty);
        writer.Add(6, &firstTurn);
        writer.Add(7, &empty);
    }
    fields.Seek(0);
    { CStructureSaver reader(fields, CStructureSaver::READ); turns.operator&(reader); }
    vector<CPtr<NWorld::IPlayer>> interrupts;
    world->GetInterrupts(&interrupts);
    Require(interrupts.empty() && world->IsRealTime(), "first-turn fixture did not enter an empty turn stack");
    first->GetCommander()->Do(new NWorld::CCmdSetCommand(actor,
        new NWorld::CCmdShootTile(actor->GetPosition().GetCP() + CVec3(2.f, 0.f, 0.5f))));
    Require(first->GetCommander()->IsRequestInterrupt(),
        "native commander did not request a turn for its non-skippable order");
    turns.Segment();
    world->GetInterrupts(&interrupts);
    std::printf("native manual first-turn stack=%zu\n", interrupts.size());
    Require(interrupts.size() == 2 && interrupts[0].GetPtr() == first && interrupts[1].GetPtr() == second &&
        world->GetCurrentPlayer() == second && world->IsFirstTurn(),
        "manual first-turn request omitted the opposing unit's automatic interrupt");
}

void OwnerWoundPolicy(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* party = MovementPolicyActor(world);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    const auto enemy = std::find_if(units.begin(), units.end(), [](const auto& unit) {
        return IsValid(unit) && unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK();
    });
    Require(enemy != units.end(), "wound policy fixture lacks a live AI unit");
    auto* actor = mode == 1 ? party : enemy->GetPtr();
    auto* owner = mode == 2 ? party->GetTBSPlayer() : (*enemy)->GetTBSPlayer();
    if (mode != 0)
        world->ChangeUnitPlayer(actor, owner);
    Require(actor->GetTBSPlayer() == owner && actor->IsAIUnit() == (mode != 2),
        "native world failed to assign the expected unit owner");

    auto* mission = actor->GetUnitRPG();
    auto* rpg = mission->GetRPGUnit();
    Require(!rpg->HasPerk(NRPG::N_PERK_LESS_INFLUENCE_OF_WOUNDS_FOR_TOHIT),
        "native wound fixture unexpectedly has the wound tolerance perk");
    rpg->Skills(NDb::ST_VP).SetConst(100);
    rpg->Skills(NDb::ST_VP).SetValue(40);
    rpg->Skills(NDb::ST_SHOOTING).SetConst(100);
    rpg->nHealedVP = 0;
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_ANY, NDb::C_VP));
    const int shooting = rpg->Skills(NDb::ST_SHOOTING).GetCurrentMaxValue();
    std::printf("native wound owner case=%d ai=%d wounded skill=%d\n", mode, actor->IsAIUnit(), shooting);
    Require(shooting == (mode == 2 ? 75 : 60),
        "native wound penalties did not follow the current owner");
}

void ScriptedAckSpeaker(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* speaker = nullptr;
    for (const auto& unit : units) {
        if (!IsValid(unit))
            continue;
        const auto* rpgUnit = unit->GetUnitRPG()->GetRPGUnit();
        if (rpgUnit->GetRPGPersID() != rpgUnit->GetAckPersID()) {
            speaker = unit;
            break;
        }
    }
    Require(speaker, "native acknowledgement fixture lacks a speaker with a separate voice donor");
    const auto* rpgUnit = speaker->GetUnitRPG()->GetRPGUnit();
    auto* table = NDatabase::GetTable<NDb::CDBAck>();
    Require(table, "native acknowledgement fixture lacks its authored table");
    vector<NDb::CDBAck*> rows;
    CDBIterator<NDb::CDBAck> next(*table);
    while (rows.size() < 2 && next.MoveNext()) {
        auto* row = next.Get();
        if (IsValid(row) && IsValid(row->pCondition) && IsValid(row->pAckSequence))
            rows.push_back(row);
    }
    Require(rows.size() == 2, "native acknowledgement fixture needs separate speaker and donor rows");
    auto* own = rows[0];
    auto* donor = rows[1];
    const int condition = own->pCondition->GetRecordID();
    own->nRPGPersID = rpgUnit->GetRPGPersID();
    own->nConditionID = -1;
    donor->nRPGPersID = rpgUnit->GetAckPersID();
    donor->nConditionID = condition;
    donor->pCondition = own->pCondition;

    struct QueuedAck {
        CPtr<NWorld::CUnitServer> unit;
        CDBPtr<NDb::CDBAck> row;
        int operator&(CStructureSaver& fields) { fields.Add(2, &unit); fields.Add(3, &row); return 0; }
    };
    auto queueCounts = [&] {
        CStructureNetworkContext refs;
        auto fields = BorrowedObjectFields(world->GetGlobalAck(), &refs);
        list<QueuedAck> queued;
        CStructureSaver reader(fields, CStructureSaver::READ, &refs);
        reader.Add(3, &queued);
        std::pair<int, int> counts;
        for (const auto& ack : queued) {
            if (ack.unit.GetPtr() != speaker)
                continue;
            counts.first += ack.row.GetPtr() == own;
            counts.second += ack.row.GetPtr() == donor;
        }
        return counts;
    };
    world->GetGlobalAck()->AddAckSequence(speaker, donor);
    const auto before = queueCounts();
    Require(before.second >= 1, "serialized acknowledgement queue lost its control speaker or row");
    auto* script = world->GetOwnScript();
    Require(IsValid(script), "native acknowledgement fixture lacks its Lua state");
    NScript::luaPushCPtr(script->GetState(), speaker);
    script->SetGlobal("TestAckSpeaker");
    const string command = "UnitSayAck(TestAckSpeaker," + std::to_string(condition) + "); TestAckRan = 1";
    Require(script->DoString(command.c_str()) == 0, "UnitSayAck could not run through Lua");
    world->ExecuteOwnScript();
    const auto ran = script->GetGlobal("TestAckRan");
    Require(ran.IsNumber() && ran.GetNumber() == 1, "native Lua script did not reach the acknowledgement");
    script->Pop(1);
    const auto after = queueCounts();
    std::printf("scripted acknowledgement own=%d donor=%d\n", after.first - before.first, after.second - before.second);
    Require(after.first == before.first + 1 && after.second == before.second,
        "UnitSayAck skipped its speaker's linked condition or queued the voice donor's phrase");
}

void CrouchedClimbIdle(const char* databasePath, const char* resourcePath)
{
    LadderDescentFixture fixture(databasePath, resourcePath);
    auto& animation = fixture.actor->animator;
    animation.PlaceUnit(fixture.bottom);
    fixture.Ascend();
    auto* idleState = dynamic_cast<NAnimation::CSkeletonState*>(animation.GetSkeletonState());
    Require(idleState, "actor lacks a native skeleton idle state");
    auto flags = [&] { return idleState->state.nAnimFlagsPoseWeapon; };
    Require(!animation.CanActivateItem() && (flags() & NDb::CAnimation::POSE_STAND),
        "authored ladder approach did not retain its pre-climb standing idle flags");
    const auto transition = NAI::GetTransitionType(fixture.world->GetPathNetwork(),
        fixture.ledge.pos.p, fixture.approach.pos.p);
    const bool jump = transition == NAI::TT_JUMP || transition == NAI::TT_JUMP_BACK;
    Require(jump || transition == NAI::TT_MOVE || transition == NAI::TT_MOVE_DIAGONAL,
        "fixture lacks the native transition off the upper ledge");
    animation.Jump(fixture.ledge, fixture.approach, jump, transition == NAI::TT_JUMP_BACK);
    Require(animation.CanActivateItem(), "completing the climb did not release the inactive ledge pose");
    Require((flags() & NDb::CAnimation::POSE_CROUCH) && !(flags() & NDb::CAnimation::POSE_STAND),
        "completed crouched climb retained standing idle flags");
}

}

bool RunUnitTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "corpse-density") && argc == 5) { CorpseDensity(argv[2], argv[3], std::atoi(argv[4]) != 0); return true; }
    if (!std::strcmp(mode, "path-command-goal") && argc == 5) { PathCommandGoal(argv[2], argv[3], std::atoi(argv[4]) != 0); return true; }
    if (!std::strcmp(mode, "difficulty-skill-scaling") && argc == 2) { DifficultySkillScaling(); return true; }
    if (!std::strcmp(mode, "take-perk-during-treatment") && argc == 4) { TakePerkDuringTreatment(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "selected-critical-treatment-log") && argc == 3) { SelectedCriticalTreatmentLog(argv[2]); return true; }
    if (!std::strcmp(mode, "panzerklein-regeneration-bleeding") && argc == 3) { PanzerkleinRegenerationBleeding(argv[2]); return true; }
    if (!std::strcmp(mode, "look-action-point-cost") && argc == 4) { LookActionPointCost(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "forced-action-replacement") && argc == 5) { ForcedActionReplacement(argv[2], argv[3], std::atoi(argv[4]) != 0); return true; }
    if (!std::strcmp(mode, "fall-damage-minimum") && argc == 4) { FallDamageMinimum(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "realtime-turn-reset") && argc == 4) { RealTimeTurnReset(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "wish-pose-strafe") && argc == 4) { WishPoseStrafe(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "hide-empty-roster-slot") && argc == 4) { HideWithEmptyRosterSlot(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "normal-state-command-lifetime") && argc == 4) { NormalStateCommandLifetime(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ladder-descent-item-activation") && argc == 5) { LadderDescentItemActivation(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "forced-fall-pose") && argc == 4) { ForcedFallPose(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "crouched-climb-idle") && argc == 4) { CrouchedClimbIdle(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "scripted-ack-speaker") && argc == 4) { ScriptedAckSpeaker(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "owner-wound-policy") && argc == 5) { OwnerWoundPolicy(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "ordinary-placement-anchor") && argc == 5) { OrdinaryPlacementAnchor(argv[2], argv[3], std::atoi(argv[4]) != 0); return true; }
    if (!std::strcmp(mode, "manual-first-turn-interrupt") && argc == 4) { ManualFirstTurnInterrupt(argv[2], argv[3]); return true; }
    return false;
}
}
