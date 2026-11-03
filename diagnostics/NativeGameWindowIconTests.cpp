#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "../Game/Platform.h"
#include "../Game/resource.h"
#include <cstdint>
#include <cstdio>
#include <cstring>

static std::uint64_t IconPixels(HICON icon)
{
    if (!icon) return 0;
    HDC dc=CreateCompatibleDC(nullptr);
    if (!dc) return 0;
    BITMAPINFO info{};
    info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=32;info.bmiHeader.biHeight=-32;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    info.bmiHeader.biCompression=BI_RGB;
    void* bytes=nullptr;
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bytes,nullptr,0);
    if (!bitmap || !bytes) { if (bitmap) DeleteObject(bitmap);DeleteDC(dc);return 0; }
    HGDIOBJ previous=SelectObject(dc,bitmap);
    std::memset(bytes,0,32*32*4);
    std::uint64_t hash=0;
    if (DrawIconEx(dc,0,0,icon,32,32,0,nullptr,DI_NORMAL)) {
        hash=UINT64_C(14695981039346656037);
        for (int i=0;i<32*32*4;++i) { hash^=static_cast<const unsigned char*>(bytes)[i];hash*=UINT64_C(1099511628211); }
    }
    SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);
    return hash;
}

int main()
{
    // Fingerprint independently bound to original Steam resource101 and every
    // frame of the unchanged donor ICO. The original executable is not run.
    constexpr std::uint64_t retail=UINT64_C(0x8973e7b08c5fa1e4);
    S2Platform::SetErrorDialogs(false);
    if (!S2Platform::Init("Silent Storm icon regression",128,128,true)) return 2;
    HWND window=static_cast<HWND>(S2Platform::NativeWindow());
    HINSTANCE module=GetModuleHandleW(nullptr);
    HICON resource32=static_cast<HICON>(LoadImageW(module,MAKEINTRESOURCEW(IDI_ICON9),IMAGE_ICON,32,32,0));
    HICON resourceDefault=LoadIconW(module,MAKEINTRESOURCEW(IDI_ICON9));
    HICON actual=window?reinterpret_cast<HICON>(SendMessageW(window,WM_GETICON,ICON_BIG,0)):nullptr;
    if (!actual && window) actual=reinterpret_cast<HICON>(GetClassLongPtrW(window,GCLP_HICON));
    const auto resourceHash=IconPixels(resource32),defaultHash=IconPixels(resourceDefault),windowHash=IconPixels(actual);
    const bool loaded=window && resource32 && resourceDefault && actual;
    const bool selection=loaded && windowHash==defaultHash;
    const bool image=loaded && resourceHash==retail;
    std::printf("native game window icon loaded=%d resource101=%016llx expected=%016llx window=%016llx default=%016llx selection=%d image=%d\n",int(loaded),static_cast<unsigned long long>(resourceHash),static_cast<unsigned long long>(retail),static_cast<unsigned long long>(windowHash),static_cast<unsigned long long>(defaultHash),int(selection),int(image));
    if (resource32) DestroyIcon(resource32);
    // Default and class icons are shared/SDL-owned, never destroyed by this observer.
    S2Platform::Done();
    if (!loaded || !selection || !image) { std::fprintf(stderr,"FAIL: actual SDL window or compiled Game resource101 still uses the A5 icon instead of the original game icon\n");return 1; }
    return 0;
}
