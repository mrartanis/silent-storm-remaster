#include "upstream/TestSupport.h"
#include <cstdio>
#include <stdexcept>

int main(int argc, char** argv)
{
    using namespace UpstreamTests;
    try {
        const char* mode = argc > 1 ? argv[1] : "temporary-groups";
        if (!RunBuildingTests(mode, argc, argv) && !RunDatabaseTests(mode, argc, argv) &&
            !RunRenderTests(mode, argc, argv) && !RunUnitTests(mode, argc, argv) &&
            !RunProjectileTests(mode, argc, argv) && !RunLegacyTests(mode, argc, argv))
            throw std::runtime_error("unknown upstream regression mode");
        std::printf("PASS: %s\n", mode);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
