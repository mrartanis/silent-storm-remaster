#include "StdAfx.h"
#undef for
#include "VectorFonts.h"
#include "VectorFontRasterizer.h"
#include "GfxBuffers.h"
#include "GPixelFormat.h"
#include "../MiscDll/LogStream.h"
#include <SDL3/SDL_filesystem.h>
#if defined(__ANDROID__)
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>
#include <memory>
#endif
#include <map>

namespace {
unsigned deviceGeneration = 1;
class VectorTexture: public CPtrFuncBase<NGfx::CTexture> {
  unsigned generation = 0;
  S2Fonts::Atlas atlas;
protected:
  bool NeedUpdate() override { return generation != deviceGeneration || !IsValid(pValue); }
  void Recalc() override {
    pValue = NGfx::MakeTexture(atlas.width, atlas.height, 1, NGfx::SPixel8888::ID, NGfx::REGULAR, NGfx::CLAMP);
    if (!IsValid(pValue)) return;
    NGfx::I2DBufferLock* lock = pValue->Lock(0, NGfx::INPLACE);
    if (!lock) { pValue = 0; return; }
    for (int y = 0; y < atlas.height; ++y)
      memcpy(static_cast<char*>(lock->GetBuffer()) + y * lock->GetStride(), &atlas.rgba[y * atlas.width * 4], atlas.width * 4);
    delete lock;
    generation = deviceGeneration;
  }
public:
  explicit VectorTexture(S2Fonts::Atlas value): atlas(std::move(value)) {}
};
class VectorFormat: public CPtrFuncBase<CFontFormatInfo> {
protected:
  bool NeedUpdate() override { return false; }
  void Recalc() override {}
public:
  explicit VectorFormat(CFontFormatInfo* value) { pValue = value; }
};
// Lives outside the serialized locale. Shutdown releases it before the device.
std::map<std::pair<std::string,int>, CObj<NGScene::CFontInfo>> cache;
}

class CVectorFontBuilder {
public:
  static CFontFormatInfo* Format(const S2Fonts::Atlas& atlas) {
    auto* info = new CFontFormatInfo;
    info->nHeight = atlas.lineHeight; info->nExternalLeading = 0;
    info->nAveCharWidth = atlas.averageWidth; info->nMaxCharWidth = atlas.maxWidth;
    info->cCharSet = 1; info->wDefaultChar = '?';
    for (const auto& g : atlas.glyphs)
      info->chars[g.code] = STFCharacter{g.x, g.y, g.x+g.width, g.y+g.height,
                                       g.bearing, g.advance-g.bearing, g.advance};
    for (const auto& k : atlas.kerns)
      info->kerns[(DWORD(k.previous)<<16)|DWORD(k.current)] = k.adjustment;
    return info;
  }
};
namespace NGScene {
CFontInfo* GetVectorFont(const SFont& font) {
  const auto file = S2Fonts::FamilyFile(font.szName);
  if (file.empty()) return nullptr;
  const int pixels = Max(1, Min(512, font.nSize));
  const auto key = std::make_pair(file, pixels);
  const auto old = cache.find(key);
  if (old != cache.end()) return old->second;
  S2Fonts::Atlas atlas; std::string error;
#if defined(__ANDROID__)
  const std::string path = "fonts/" + file;
  std::size_t size = 0;
  std::unique_ptr<void, decltype(&SDL_free)> data(SDL_LoadFile(path.c_str(), &size), SDL_free);
  if (!data || !S2Fonts::RasterizeMemory(data.get(), size, pixels, &atlas, &error)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", path.c_str(), data ? error.c_str() : SDL_GetError());
    return nullptr;
  }
#else
  const char* base = SDL_GetBasePath();
  if (!base || !S2Fonts::Rasterize(std::string(base) + "fonts/" + file, pixels, &atlas, &error)) {
    csSystem << "VECTOR-FONT: " << error << endl;
    return nullptr;
  }
#endif
  CObj<CFontFormatInfo> format = CVectorFontBuilder::Format(atlas);
  CObj<CFontInfo> result = new CFontInfo(SFont(atlas.lineHeight, font.szName),
      new VectorTexture(std::move(atlas)), new VectorFormat(format));
  result->SetRuntimeVector(font);
  cache[key] = result;
  return result;
}
bool IsVectorTexture(CPtrFuncBase<NGfx::CTexture>* texture) { return dynamic_cast<VectorTexture*>(texture) != nullptr; }
void ResetVectorFontDevice() { ++deviceGeneration; }
void ClearVectorFonts() { cache.clear(); }
}
