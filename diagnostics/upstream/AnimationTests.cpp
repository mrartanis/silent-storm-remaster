#include "NativeCannonFixture.h"
#include "../../Main/GAnimFormat.h"
#include "../../Main/GAnimPath.h"
#include "../../Main/GAnimTerrain.h"
#include "../../Main/wUICommands.h"
#include "../../Main/wUnitMove.h"
#include "../../Main/wUnitQueue.h"
#include "../../Main/aiPath.h"
#include "../../Main/aiGrid.h"
#include "../../Main/aiLocker.h"
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace UpstreamTests
{
namespace
{
void AnimationPathSmoothingOrder()
{
    using NAnimation::CPathInterpolator;
    const vector<CVec3> controls = {CVec3(0x1.833e14p+1f, 0, 0), CVec3(0x1.8146e0p+1f, 0, 0),
        CVec3(0x1.84c0dcp+1f, 0, 0)};
    CObj<CPathInterpolator> path = new CPathInterpolator(controls);
    CMemoryStream fields;
    { CStructureSaver writer(fields, CStructureSaver::WRITE); path->operator&(writer); }
    fields.Seek(0);
    vector<CPathInterpolator::SPathPoint> points;
    { CStructureSaver reader(fields, CStructureSaver::READ); reader.Add(1, &points); }
    Require(points.size() == 5 && points.front().ptPos == controls.front() && points.back().ptPos == controls.back(),
        "path smoothing fixture did not preserve the two endpoints in one subdivision");
    Require(points[2].ptPos.x == 0x1.822f14p+1f,
        "animation path smoothing stored the wrong center at the retail rounding boundary");
}

void ClipFrameDuration(const char* databasePath, const char* resourcePath)
{
    auto fixture = MakeNativeCannonFixture(databasePath, resourcePath);
    auto* skeleton = fixture.actor->animator.GetSkeletonAnimator();
    auto* record = NDb::GetAnimation(4437);
    Require(IsValid(record) && IsValid(skeleton), "clip duration fixture needs an authored runtime clip and skeleton");
    record->fSpeed = 1.f;
    CObj<NAnimation::CAnimation> clip = skeleton->CreateAnimation(record, 1000);
    Require(IsValid(clip), "native skeleton could not load the duration test clip");
    CStructureNetworkContext refs;
    auto fields = BorrowedObjectFields(clip, &refs);
    CDGPtr<CPtrFuncBase<NAnimation::CFileAnimationInfo>> payload;
    { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(5, &payload); }
    payload.Refresh();
    auto* info = payload->GetValue();
    Require(IsValid(info) && info->hdr.fLength > 0, "native duration test clip has no authored frame length");
    const float length = info->hdr.fLength;
    for (int milliseconds = 200; milliseconds < 1500; ++milliseconds) {
        float rate = std::nextafter(length * 1000.f / milliseconds, 0.f);
        for (int step = 0; step < 4; ++step, rate = std::nextafter(rate, rate + 1.f)) {
            const STime expected = STime((length / rate) * 1000.f);
            const STime previous = STime((length * 1000.f) / rate);
            if (expected == previous)
                continue;
            info->hdr.fFrameRate = rate;
            CObj<NAnimation::CAnimation> retimed = skeleton->CreateAnimation(record, 1000);
            Require(IsValid(retimed) && retimed->GetTime() == expected && clip->GetOriginalTime() == expected,
                "native clip constructor or duration query used the old multiply-before-divide order");
            return;
        }
    }
    Require(false, "authored clip length did not supply a distinguishing frame rate");
}

void UnoccupiedPKSkeleton(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    auto* actor = MovementPolicyActor(world);
    NDb::CRPGPers* persona = nullptr;
    CDBIterator<NDb::CRPGPers> rows(*NDatabase::GetTable<NDb::CRPGPers>());
    while (!persona && rows.MoveNext()) {
        auto* candidate = rows.Get();
        if (IsValid(candidate->pPanzerklein) && IsValid(candidate->pModel) &&
            IsValid(candidate->pBaseValue) && candidate->pPanzerklein->nMaxVP > 0)
            persona = candidate;
    }
    Require(IsValid(persona), "skeleton type fixture needs an authored Panzerklein shell");
    auto* network = world->GetPathNetwork();
    vector<NAI::SPathPlace> nearby;
    network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(), 15.f), &nearby);
    auto placement = std::find_if(nearby.begin(), nearby.end(), [&](auto& point) {
        point.SetPose(NAI::CM_STAND);
        return network->IsPassable(point) && !network->IsLocked(point) && NAI::IsBigLockerPlacePassable(network, point);
    });
    Require(placement != nearby.end(), "skeleton type fixture needs a free native heavy footprint");
    auto* shell = world->AddUnitInGame(*placement, NRPG::CreateUnit(persona), nullptr);
    Require(IsValid(shell) && shell->IsEmptyPK(), "the actual spawn did not create an empty Panzerklein");
    struct AnimatorSnapshot {
        bool pk = false;
        int operator&(CStructureSaver& reader) { reader.Add(38, &pk); return 0; }
    };
    struct UnitBodySnapshot {
        AnimatorSnapshot animator;
        int operator&(CStructureSaver& reader) { reader.Add(19, &animator); return 0; }
    };
    auto usesPKSkeleton = [](NWorld::CUnitServer* unit) {
        CStructureNetworkContext refs;
        auto fields = BorrowedObjectFields(unit, &refs);
        UnitBodySnapshot state;
        CStructureSaver reader(fields, CStructureSaver::READ, &refs);
        reader.Add(2, &state);
        return state.animator.pk;
    };
    Require(!usesPKSkeleton(actor), "ordinary soldier should keep its original skeleton type");
    Require(usesPKSkeleton(shell), "new empty Panzerklein entered the ordinary human animation state");
}

struct AimTurnSample
{
    CVec3 direction;
    STime duration;
};

AimTurnSample FindAimTurnSample(const CVec3& reference)
{
    for (int milliseconds = 300; milliseconds < 1800; ++milliseconds) {
        const double angle = double(milliseconds) / 1250.0;
        const float horizontal = float(std::cos(angle));
        float vertical = std::nextafter(float(std::sin(angle)), 0.f);
        for (int sample = 0; sample < 3; ++sample) {
            const CVec3 direction(reference.x * horizontal, reference.y * horizontal, vertical);
            CVec3 unit = direction;
            Normalize(&unit);
            const double turn = std::asin(double(fabs(reference ^ unit)));
            const STime expected = STime(turn * 1250.0);
            const STime previous = STime(turn / float(0.8f) * 1000.0);
            if (expected == previous + 1)
                return {direction, expected};
            vertical = std::nextafter(vertical, 1.f);
        }
    }
    throw std::runtime_error("native aim turn fixture lacks a ray straddling the millisecond boundary");
}

void AimTurnDuration(const char* databasePath, const char* resourcePath, int mode)
{
    auto fixture = MakeNativeCannonFixture(databasePath, resourcePath);
    auto* actor = fixture.actor.GetPtr();
    auto& animator = actor->animator;
    auto* cannon = fixture.cannon.GetPtr();
    const auto position = actor->GetPosition();
    if (mode >= 2) {
        fixture.world->GivePlayerTurn(actor->GetTBSPlayer());
        CObj<NWorld::CCmdCannon> order = new NWorld::CCmdCannon(cannon);
        CObj<NWorld::CExecCannon> entry = new NWorld::CExecCannon(actor, cannon, true, order);
        Require(entry->CanDoIt(position) == NWorld::UCR_OK, "native aim turn fixture cannot mount its cannon");
        entry->Run();
        fixture.world->GetTime()->Set(animator.GetTimeEnd());
        entry->AnimationFinished();
        Require(animator.GetCannon() == cannon, "native aim turn fixture lost its mounted cannon");
    }
    const float heading = mode >= 2 ? cannon->GetDirection() : position.GetDirection();
    CVec3 reference(std::cos(heading), std::sin(heading), 0);
    CRay ray;
    ray.ptOrigin = position.GetCP();
    if (mode < 2) {
        ray.ptDir = reference;
        animator.Attack(position, ray, false, true);
        Require(animator.IsAiming(), "native actor did not enter its authored aim animation");
        if (mode == 0) {
            Normalize(&reference);
        } else {
            animator.StartMove(true);
            animator.EndMove(position, position, false, false);
            Require(animator.IsAiming(), "native strafe stop did not preserve aimed posture");
        }
    }
    const auto sample = FindAimTurnSample(reference);
    ray.ptDir = sample.direction;
    if (mode < 2) {
        const STime start = animator.GetTimeEnd();
        animator.Attack(position, ray, false, true);
        const STime padding = mode == 1 ? 300 : 0;
        Require(animator.GetTimeEnd() == start + sample.duration + padding,
            "native aim preparation ended one millisecond before its retail animation time");
        return;
    }

    if (mode == 3)
        animator.Attack(position, ray);
    auto* body = cannon->GetSkeletonAnimator();
    const auto before = NativeCannonAnimationClips(body);
    vector<CObj<NAnimation::CAnimation>> existing;
    for (auto* clip : before)
        existing.emplace_back(clip);
    const STime start = animator.GetTimeEnd();
    if (mode == 3)
        animator.LeaveCannon(position);
    else
        animator.Attack(position, ray);
    const auto type = mode == 3 ? NDb::CAnimation::POSE : NDb::CAnimation::ATTACK_LD;
    for (auto* clip : NativeCannonAnimationClips(body)) {
        const auto* record = NDb::GetAnimation(clip->GetRecordID());
        if (!record || record->nType != type || std::find(before.begin(), before.end(), clip) != before.end())
            continue;
        CStructureNetworkContext refs;
        auto fields = BorrowedObjectFields(clip, &refs);
        STime actual = 0;
        { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(7, &actual); }
        Require(actual == start + sample.duration,
            "native cannon aim or return pose began one millisecond before its retail animation time");
        return;
    }
    Require(false, "native cannon aim turn did not schedule its authored animation");
}

void AttackAnimationFallback(const char* databasePath, const char* resourcePath, int mode)
{
    auto fixture = MakeNativeCannonFixture(databasePath, resourcePath);
    auto* actor = fixture.actor.GetPtr();
    auto& animator = actor->animator;
    auto position = actor->GetPosition();
    CDBPtr<NDb::CSkeleton> skeleton;
    CMemoryStream fields;
    CStructureNetworkContext refs;
    refs.include = [](CObjectBase*) { return false; };
    { CStructureSaver writer(fields, CStructureSaver::WRITE, &refs); animator.operator&(writer); }
    fields.Seek(0);
    { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(31, &skeleton); }
    Require(skeleton, "attack animation fixture has no native skeleton");

    CRay ray;
    ray.ptOrigin = position.GetCP();
    ray.ptDir = CVec3(0.8f, 0.f, 0.6f);
    if (mode == 0) {
        Require(!animator.IsAiming() && skeleton->pAnimations.erase(NDb::CAnimation::ATTACK) == 1,
            "attack fallback fixture could not remove the real attack clip choices");
        animator.Attack(position, ray, false, true);
        CMemoryStream after;
        { CStructureSaver writer(after, CStructureSaver::WRITE, &refs); animator.operator&(writer); }
        after.Seek(0);
        CVec3 aim;
        bool strafing = false;
        { CStructureSaver reader(after, CStructureSaver::READ, &refs); reader.Add(16, &strafing); reader.Add(21, &aim); }
        Require(animator.IsAiming() && strafing && fabs2(aim - ray.ptDir) < 1e-6f,
            "missing native firing animation discarded the aim direction or strafe state");
        return;
    }

    NDb::CAnimation* record = nullptr;
    if (mode == 1) {
        const auto& clips = skeleton->pAnimations[NDb::CAnimation::ATTACK].anims;
        for (const auto& clip : clips) {
            if (clip->szParams == "Height1" && clip->fRndWeight > 0) {
                record = clip;
                break;
            }
        }
        Require(record, "attack fixture lacks a native low strike clip");
        position.SetPose(NAI::CRAWL);
        animator.PlaceUnit(position);
    } else {
        record = NDb::GetAnimation(0x925);
        Require(record && record->nType == NDb::CAnimation::ATTACK, "native special attack record is missing");
    }
    record->nPoseWeaponFlags = 0x7fffffff;
    record->nClassSexFlags = 0x7fffffff & ~NDb::CAnimation::BOSS;
    record->pSide = nullptr;
    skeleton->pAnimations[NDb::CAnimation::ATTACK].anims = {record};
    auto* graph = animator.GetSkeletonAnimator();
    if (mode == 1)
        animator.CloseAttack(position, NAI::BH_TOP);
    else
        animator.Attack(position, ray, false, true);

    bool hasClip = false;
    bool usesCrawlTerrain = false;
    for (auto* node : NativeAnimationNodes(graph)) {
        if (auto* clip = dynamic_cast<NAnimation::CAnimation*>(node))
            hasClip |= clip->GetRecordID() == record->GetRecordID();
        usesCrawlTerrain |= dynamic_cast<NAnimation::CATerrainCrawl*>(node) != nullptr;
    }
    Require(hasClip, "native attack path did not schedule its low or special clip");
    if (mode == 2)
        Require(usesCrawlTerrain, "native special attack did not use the crawl terrain transform");
}

void MovementPresentationGuards(const char* databasePath, const char* resourcePath, int mode)
{
    NativeCannonFixture fixture;
    if (mode == 1) {
        fixture.world = FixtureWorld(databasePath, resourcePath, true, &fixture.game);
        fixture.actor = MovementPolicyActor(fixture.world);
    } else {
        fixture = MakeNativeCannonFixture(databasePath, resourcePath);
    }
    auto* actor = fixture.actor.GetPtr();
    auto* world = fixture.world.GetPtr();
    world->GivePlayerTurn(actor->GetTBSPlayer());
    actor->GetUnitRPG()->StartNewTurn(actor->GetPosition().GetCP());
    auto* cannon = fixture.cannon.GetPtr();
    CObj<NWorld::CCmdCannon> command = new NWorld::CCmdCannon(cannon);
    CObj<NWorld::CExecCannon> mounted = new NWorld::CExecCannon(actor, cannon, true, command);
    if (mode != 1) {
        Require(mounted->CanDoIt(actor->GetPosition()) == NWorld::UCR_OK, "mounted movement fixture has no legal cannon order");
        mounted->Run();
        Require(actor->animator.GetCannon() == cannon, "mounted movement fixture did not mount its cannon");
    }
    auto from = actor->GetPosition().pos.p;
    CObj<NAI::CPath> path = new NAI::CPath;
    path->pNet = world->GetPathNetwork();
    path->points.push_back(from);
    if (mode == 1) {
        auto to = from;
        to.SetPose(NAI::CM_CROUCH);
        path->pNet->Unlock(actor);
        const auto pass = path->pNet->GetPassability(to);
        path->pNet->Lock(actor, from);
        Require(pass == NAI::AIP_YES, "movement fixture has no native crouch pose");
        path->points.push_back(to);
    }
    if (mode == 0) {
        CObj<NWorld::CExecQueue> queue = new NWorld::CExecQueue(actor);
        queue->AddExecutor(mounted);
        queue->SetNewPath(path, NAI::PF_USE_POSEDIR);
        Require(queue->GetFrontExecutor() == mounted.GetPtr(), "a new path displaced the mounted cannon executor");
        return;
    }

    while (CObj<NWorld::CUICmd> command = world->GetUICommand()) {}
    CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(actor, path, NAI::PF_USE_POSEDIR,
        NWorld::ITEM_NO_MATTER, true);
    Require(move, "movement fixture failed to create the actual path executor");
    move->Run();

    Require(actor->GetPosition().pos.p.GetPose() == NAI::CM_CROUCH, "stance fixture did not execute the native crouch step");
    int requests = 0;
    while (CObj<NWorld::CUICmd> command = world->GetUICommand()) {
        auto* camera = dynamic_cast<NWorld::CUICmdUnitCamera*>(command.GetPtr());
        if (camera && camera->pUnit == actor && camera->nPriority == NWorld::PR_UNIT_ACTION)
            ++requests;
    }
    Require(requests == 1, "native pose change did not issue exactly one action camera request");
}
}

bool RunAnimationTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "animation-path-smoothing-order") && argc == 2) { AnimationPathSmoothingOrder(); return true; }
    if (!std::strcmp(mode, "clip-frame-duration") && argc == 4) { ClipFrameDuration(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "unoccupied-pk-skeleton") && argc == 4) { UnoccupiedPKSkeleton(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "aim-turn-duration") && argc == 5) { AimTurnDuration(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "attack-animation-fallback") && argc == 5) { AttackAnimationFallback(argv[2], argv[3], std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "movement-presentation-guards") && argc == 5) { MovementPresentationGuards(argv[2], argv[3], std::atoi(argv[4])); return true; }
    return false;
}
}
