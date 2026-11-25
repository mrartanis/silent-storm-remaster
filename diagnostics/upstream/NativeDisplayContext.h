#pragma once

namespace UpstreamTests
{
// Construct before the interface graph so partial initialization is cleaned up.
struct NativeDisplayContext
{
    bool platform = false, graphics = false, audio = false;
    ~NativeDisplayContext();
    void Initialize(const char* databasePath, const char* resourcePath, const char* title);
};
}
