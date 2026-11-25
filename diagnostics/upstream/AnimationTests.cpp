#include "NativeCannonFixture.h"
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace UpstreamTests
{
namespace
{
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
}

bool RunAnimationTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "aim-turn-duration") && argc == 5) { AimTurnDuration(argv[2], argv[3], std::atoi(argv[4])); return true; }
    return false;
}
}
