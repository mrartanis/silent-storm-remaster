#include "TestSupport.h"
#include "../../Main/wMain.h"
#include "../../Main/wUnitServer.h"
#include "../../Main/wUnitCommands.h"
#include "../../Main/wUnitExec.h"
#include "../../Main/RPGUnit.h"
#include "../../Main/RPGUnitMission.h"
#include "../../Main/RPGCritical.h"
#include "../../Main/RPGItem.h"
#include "../../Main/RPGItemMap.h"
#include "../../Main/RPGItemSet.h"
#include "../../Main/wUnitAttackExec.h"
#include "../../Main/rpgPerk.h"
#include "../../Main/rpgGlobal.h"
#include "../../DBFormat/DataRPG.h"
#include "../../MiscDll/LogStream.h"
#include "../../Misc/RandomGen.h"
#include <cstring>

namespace UpstreamTests
{
namespace
{
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

}

bool RunUnitTests(const char* mode, int argc, char** argv)
{
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
    return false;
}
}
