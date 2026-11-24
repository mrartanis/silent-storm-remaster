#include "TestSupport.h"
#include "../../Main/GParticleFormat.h"
#include "../../Main/Render.h"
#include <array>
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

void FixedRasterEdgeReciprocal()
{
    struct RowSink : CRasterizer<RowSink> {
        std::array<std::pair<int, int>, 3> spans = {};
        void ClipVertical(int*, int*, int*) const {}
        void ClipHorizontal(int*, int*) const {}
        bool DoRenderBackface() const { return true; }
        void RasterSpan(int y, int first, int end, float, float, int) {
            if (y >= 51 && y <= 53)
                spans[y - 51] = {first, end};
        }
    } raster;
    // The diagonal crosses the pixel-60 boundary on row 52. Rounding the reciprocal
    // before multiplying changes the 16.16 step from 64270 to 64269.
    raster.RasterNoClip(CVec3(10.262948f, 1.27278805f, 1.f), CVec3(98.4306564f, 91.1779404f, 1.f),
        CVec3(10.262948f, 91.1779404f, 1.f));
    Require(raster.spans[0] == std::make_pair(10, 60) && raster.spans[2] == std::make_pair(10, 61),
        "neighboring control spans do not match the native fixed edge");
    Require(raster.spans[1] == std::make_pair(10, 60),
        "direct division added a pixel to the boundary span");
}
}

bool RunRenderTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "packed-particle-color")) { PackedParticleColor(); return true; }
    if (!std::strcmp(mode, "fixed-raster-edge-reciprocal")) { FixedRasterEdgeReciprocal(); return true; }
    return false;
}
}
