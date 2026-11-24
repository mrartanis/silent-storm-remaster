#include "TestSupport.h"
#include "../../Main/GParticleFormat.h"
#include <cstring>

namespace UpstreamTests
{
namespace
{
static void PackedParticleColor()
{
    NGScene::TParticleKey<DWORD> keys[] = {{0, 0xfe012ba7u}, {8, 0x1fa2f4dbu}};
    const NGScene::TKeyTrack<DWORD> track{2, keys};
    const std::pair<float, DWORD> expected[] = {
        {0.f, 0xfe012ba7u}, {1.f, 0xe21544aeu}, {2.f, 0xc6295db4u},
        {4.f, 0x8e5190c1u}, {6.f, 0x577ac2ceu}, {7.f, 0x3b8edbd5u}, {8.f, 0x1fa2f4dbu},
    };
    for (const auto& sample : expected) {
        DWORD actual = 0;
        track.GetValue(sample.first, &actual);
        Require(actual == sample.second, "particle color sampling did not match the packed channel interpolation");
    }
}
}

bool RunRenderTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "packed-particle-color")) { PackedParticleColor(); return true; }
    return false;
}
}
