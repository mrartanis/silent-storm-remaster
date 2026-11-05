#include <SDL3/SDL.h>
#if defined(__ANDROID__)
#include <SDL3/SDL_main.h>
#include "../../Game/AndroidSwapChain.h"
#include "../../Game/Platform.h"
#include "../../Input/TouchGestures.h"
#endif
#include <bgfx/bgfx.h>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace {
constexpr int kWidth = 640;
constexpr int kHeight = 360;
constexpr uint32_t clearColor = 0x25314bff;
#if defined(__ANDROID__)
bgfx::SwapChain AndroidSwapChain()
{
  int width = 0, height = 0;
  S2Platform::SurfaceSize(&width, &height);
  bgfx::SwapChain swapChain;
  swapChain.nwh = S2Platform::NativeWindow();
  swapChain.width = width;
  swapChain.height = height;
  swapChain.formatColor = bgfx::TextureFormat::RGBA8;
  swapChain.flags = BGFX_SWAP_CHAIN_TRANSPARENT_BACKBUFFER;
  return swapChain;
}

void RebindAndroidSurface()
{
  const auto swapChain = AndroidSwapChain();
  S2Android::RebindSwapChain(BGFX_RESET_VSYNC, swapChain);
  bgfx::setViewRect(0, 0, 0, swapChain.width, swapChain.height);
}
#endif

bool attachWindow(SDL_Window* window, bgfx::Init& init)
{
  const SDL_PropertiesID props = SDL_GetWindowProperties(window);
  const char* driver = SDL_GetCurrentVideoDriver();
  if (std::strcmp(driver, "windows") == 0) {
    init.swapChain.nwh = SDL_GetPointerProperty(
        props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
  } else if (std::strcmp(driver, "x11") == 0) {
    init.swapChain.ndt = SDL_GetPointerProperty(
        props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
    init.swapChain.nwh = reinterpret_cast<void*>(static_cast<uintptr_t>(
        SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0)));
  } else if (std::strcmp(driver, "wayland") == 0) {
    init.swapChain.ndt = SDL_GetPointerProperty(
        props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
    init.swapChain.nwh = SDL_GetPointerProperty(
        props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
  } else if (std::strcmp(driver, "cocoa") == 0) {
    init.swapChain.nwh = SDL_GetPointerProperty(
        props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#if defined(__ANDROID__)
  } else if (std::strcmp(driver, "android") == 0) {
    init.swapChain.nwh = S2Platform::NativeWindow();
#endif
  } else {
    return false;
  }
  return init.swapChain.nwh != nullptr;
}
} // namespace

int main(int argc, char** argv)
{
  bool headless = false;
#if defined(__ANDROID__)
  int frames = 3600;
#else
  int frames = 120;
#endif
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--headless") == 0) {
      headless = true;
    } else if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
      frames = std::atoi(argv[++i]);
      if (frames < 1 || frames > 3600) return 2;
    } else {
      std::fprintf(stderr, "usage: S2SDL3BgfxProbe [--headless] [--frames 1..3600]\n");
      return 2;
    }
  }
#if defined(__ANDROID__)
  if (!S2Platform::Init("Silent Storm Android probe")) return 1;
  SDL_Window* window = S2Platform::Window();
#else
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    return 1;
  }
  SDL_Window* window = SDL_CreateWindow("Silent Storm SDL3 + bgfx probe",
      kWidth, kHeight, headless ? SDL_WINDOW_HIDDEN : 0);
  if (!window) {
    std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }
#endif
  const auto destroyWindow = [window] {
#if defined(__ANDROID__)
    S2Platform::Done();
#else
    SDL_DestroyWindow(window);
    SDL_Quit();
#endif
  };

  bgfx::Init init;
  init.type = headless ? bgfx::RendererType::Noop : bgfx::RendererType::Count;
  int width = kWidth, height = kHeight;
#if defined(__ANDROID__)
  init.type = bgfx::RendererType::Vulkan;
  init.swapChain = AndroidSwapChain();
  width = init.swapChain.width;
  height = init.swapChain.height;
#else
  init.swapChain.width = width;
  init.swapChain.height = height;
#endif
  init.reset = BGFX_RESET_VSYNC;
  if (!headless && !attachWindow(window, init)) {
    std::fprintf(stderr, "No native window handle for SDL driver %s\n",
                 SDL_GetCurrentVideoDriver());
    destroyWindow();
    return 1;
  }
  if (!bgfx::init(init)) {
    std::fprintf(stderr, "bgfx::init failed\n");
    destroyWindow();
    return 1;
  }
  SDL_Log("SDL driver=%s, bgfx renderer=%s",
          SDL_GetCurrentVideoDriver(), bgfx::getRendererName(bgfx::getRendererType()));
#if defined(__ANDROID__)
  if (bgfx::getRendererType() != bgfx::RendererType::Vulkan) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Vulkan did not initialize");
    bgfx::shutdown();
    destroyWindow();
    return 1;
  }
  NInput::TouchGestures touch;
  using Phase = NInput::TouchGestures::Phase;
  using Action = NInput::TouchGestures::Action;
  bool resumePending = false;
  const auto* caps = bgfx::getCaps();
  for (const auto format : {bgfx::TextureFormat::BC1, bgfx::TextureFormat::BC2, bgfx::TextureFormat::BC3,
                            bgfx::TextureFormat::ETC2A, bgfx::TextureFormat::ASTC4x4}) {
    SDL_Log("Texture format %d caps: 0x%x", static_cast<int>(format), caps->formats[format]);
  }
#endif
  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColor, 1.0f, 0);
  bgfx::setViewRect(0, 0, 0, width, height);
  int submitted = 0;
  bool exiting = false;
  while (!exiting && submitted < (headless ? 3 : frames)) {
#if defined(__ANDROID__)
    S2Platform::PumpEvents();
    if (S2Platform::Exiting()) break;
    if (S2Platform::TakeBackgroundSaveRequest()) {
      resumePending = true;
      touch.Reset();
      SDL_Log("Android probe background");
    }
    if (S2Platform::Backgrounded() ||
        (resumePending && !S2Platform::RefreshAndroidSurface(RebindAndroidSurface))) {
      SDL_Delay(20);
      continue;
    }
    S2Platform::InputEvent input;
    while (S2Platform::PollInput(&input)) {
      const SDL_Event& event = input.event;
      if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) touch.Reset();
      if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_AC_BACK) exiting = true;
      if (event.type < SDL_EVENT_FINGER_DOWN || event.type > SDL_EVENT_FINGER_CANCELED) continue;
      const auto& finger = event.tfinger;
      const auto phase = event.type == SDL_EVENT_FINGER_DOWN ? Phase::Down
          : event.type == SDL_EVENT_FINGER_UP ? Phase::Up
          : event.type == SDL_EVENT_FINGER_CANCELED ? Phase::Cancel : Phase::Move;
      const auto gesture = touch.Update(phase, finger.fingerID, {finger.x * width, finger.y * height},
                                        finger.timestamp / SDL_NS_PER_MS, true);
      if (gesture.action == Action::Tap)
        SDL_Log("Android probe tap");
      if (gesture.action == Action::Camera)
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x43825bff, 1.0f, 0);
      if (phase == Phase::Up || phase == Phase::Cancel)
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColor, 1.0f, 0);
    }
    if (touch.PollHold(SDL_GetTicks())) SDL_Log("Android probe hold");
#else
    SDL_Event event;
    while (SDL_PollEvent(&event))
      if (event.type == SDL_EVENT_QUIT) exiting = true;
#endif
    if (exiting) break;
    bgfx::touch(0);
    bgfx::frame();
    ++submitted;
#if defined(__ANDROID__)
    if (submitted == 1) SDL_Log("Android probe first frame");
    if (resumePending) {
      SDL_Log("Android probe resumed");
      resumePending = false;
    }
#endif
    if (!headless) SDL_Delay(16);
  }
  std::printf("submitted %d frames\n", submitted);
  bgfx::shutdown();
  destroyWindow();
#if defined(__ANDROID__)
  SDL_Log("Android probe normal exit");
#endif
  return 0;
}
