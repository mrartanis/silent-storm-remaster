#if defined(_WIN32)
#include "../Main/StdAfx.h"
#else
#include "../FileIO/StdAfx.h"
#include "../FileIO/BasicChunk1.h"
#include "../Misc/Geom.h"
#endif
#include "../Main/Bound.h"
#include <cstdint>
#include <cstdio>

static bool Check(const CVec3* points, size_t count, const CVec3& minimum, const CVec3& maximum)
{
    CVec3 low(1e38f, 1e38f, 1e38f), high(-1e38f, -1e38f, -1e38f);
    StartMMXBound(&low, &high);
    // The production x86 contract forbids x87 work between Start and Store.
    for (size_t i = 0; i < count; ++i) AddMMXBoundPoint(points + i);
    StoreMMXBoundResult(&low, &high);
    return low.x == minimum.x && low.y == minimum.y && low.z == minimum.z &&
           high.x == maximum.x && high.y == maximum.y && high.z == maximum.z;
}

int main()
{
    const CVec3 negative[] = {CVec3(-3, -5, -7), CVec3(-10, -1, -2), CVec3(-2, -8, -4)};
    if (!Check(negative, 3, CVec3(-10, -8, -7), CVec3(-2, -1, -2))) {
        std::fprintf(stderr, "FAIL: bound accumulator reversed negative floating-point coordinates\n");
        return 1;
    }
    const CVec3 mixed[] = {CVec3(-7, 4, 0), CVec3(2, -8, 3), CVec3(-1, 0, -5)};
    if (!Check(mixed, 3, CVec3(-7, -8, -5), CVec3(2, 4, 3)) ||
        !Check(negative, 1, negative[0], negative[0]) ||
        !Check(nullptr, 0, CVec3(1e38f, 1e38f, 1e38f), CVec3(-1e38f, -1e38f, -1e38f))) {
        std::fprintf(stderr, "FAIL: bound accumulator changed mixed, single-point or empty bounds\n");
        return 1;
    }
    CVec3 cloud[257];
    SBoundCalcer scalar;
    std::uint32_t seed = 0x7a31u;
    for (auto& point : cloud) {
        float* coordinates[] = {&point.x, &point.y, &point.z};
        for (auto* coordinate : coordinates) {
            seed = seed * 1664525u + 1013904223u;
            *coordinate = (int(seed % 20001u) - 10000) * 0.125f;
        }
        scalar.Add(point);
    }
    if (!Check(cloud, 257, scalar.ptMin, scalar.ptMax)) {
        std::fprintf(stderr, "FAIL: bound accumulator disagreed with scalar bounds for a signed point cloud\n");
        return 1;
    }
    std::printf("PASS: signed bound accumulator (%zu-bit)\n", sizeof(void*) * 8);
    return 0;
}
