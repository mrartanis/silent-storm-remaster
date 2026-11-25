#include "TestSupport.h"
#include "NativeDisplayContext.h"
#include "../../ADOImport/BasicDB.h"
#include "../../FModSound/FMsound.h"
#include "../../Game/Platform.h"
#include "../../Main/DG.h"
#include "../../Main/Gfx.h"
#include "../../Main/GInit.h"
#include "../../Main/GResource.h"
#include "../../Main/GView.h"
#include "../../Main/iMain.h"

namespace UpstreamTests
{
NativeDisplayContext::~NativeDisplayContext()
{
    NGScene::StopResourceLoadingThread();
    NMainLoop::DoneInterface();
    while (NMainLoop::HaveInterfaceCommand()) {
        CObj<NMainLoop::CInterfaceCommand> command = NMainLoop::TakeInterfaceCommandForHarness();
    }
    ClearHoldQueue();
#if defined(S2_UPSTREAM_NATIVE_SFX)
    if (audio) NFMSound::Done();
#endif
    if (graphics) NGfx::Done3D();
    if (platform) S2Platform::Done();
}

void NativeDisplayContext::Initialize(const char* databasePath, const char* resourcePath, const char* title)
{
    S2Platform::SetErrorDialogs(false);
    Require(S2Platform::Init(title, 320, 240, true), "native display fixture could not create its hidden SDL window");
    platform = true;
    Require(NGfx::Init3D(static_cast<HWND>(S2Platform::NativeWindow())), "native display fixture has no graphics window handle");
    graphics = true;
    NGfx::SRenderTargetsInfo targets;
    Require(NGfx::SetMode(NGfx::SVideoMode(320, 240, 32, NGfx::WINDOWED), targets) &&
        NGScene::Is3DActive() && S2Platform::Display().drawable,
        "native display fixture could not initialize its real healthy bgfx renderer");
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    NGScene::RunResourceLoadingThread();
}
}
