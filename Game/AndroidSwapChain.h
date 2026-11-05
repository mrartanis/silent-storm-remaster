#pragma once
#include <bgfx/bgfx.h>

namespace S2Android {
inline void RebindSwapChain(uint32_t resetFlags, const bgfx::SwapChain& swapChain)
{
  bgfx::reset(resetFlags, &swapChain);
  // The second submission waits for the render thread to apply the reset.
  // The platform retains its reference to the old surface until this returns.
  bgfx::frame();
  bgfx::frame();
}
}
