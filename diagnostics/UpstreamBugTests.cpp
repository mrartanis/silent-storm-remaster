#if defined(_WIN32)
#include "../Main/StdAfx.h"
#else
#include "../FileIO/StdAfx.h"
#include "../FileIO/BasicChunk1.h"
#include "../Misc/Geom.h"
#endif
#include "../Main/wMain.h"
#include "../Main/wBuilding.h"
#include "../Main/wObject.h"
#include "../Main/wMine.h"
#include "../Main/wUnitGroup.h"
#include "../Main/iCommonUI.h"
#include "../Main/PerkToolTip.h"
#include "../Main/UnitStepSound.h"
#include "../Main/InventoryUnit.h"
#include "../Main/UnitIconWeaponMode.h"
#include "../Main/iUnitIconBar.h"
#include "../Main/UIInterface.h"
#include "../Main/Console.h"
#include "../FileIO/WindowsSaveNames.h"
#include <typeinfo>
#include "../MiscDll/Commands.h"
#include "../DBFormat/DataInterface.h"
#include "../DBFormat/DataObject.h"
#include "../DBFormat/DataMisc.h"
#include "../DBFormat/DataPerk.h"
#include "../Main/GView.h"
#include "../Main/iMission.h"
#include "../Main/iGameStates.h"
#include "../Input/BindInternal.h"
#include "../Main/iUnitPanel.h"
#include "../Main/iCharacterPanel.h"
#include "../Main/DisplayLayout.h"
#include "../Main/iDesktopWindow.h"
#include "../Main/Sound.h"
#include "../Main/RWSound.h"
#include "../Main/RWGame.h"
#include "../Main/LSController.h"
#include "../Main/LSHead.h"
#include "../Main/SoundFormat.h"
#include "../Misc/BasicShare.h"
#include "../FModSound/FMsound.h"
#include "../DBFormat/DataSound.h"
#include "../Main/SoundEffect.h"
#include "../third_party/fmod/include/fmod.h"
#if defined(S2_UPSTREAM_NATIVE_SFX)
extern "C" int S2FmodRenderForTest(float* output,std::size_t frames);
#endif
#include "../Main/iMissionDlgUI.h"
#include "../Main/iMissionUI.h"
#include "../Main/iMissionMovieUI.h"
#include "../Main/G2DView.h"
#include "../Main/UIML.h"
#include "../DBFormat/DataAck.h"
#include "../Main/wUnitCommands.h"
#include "../Main/wUICommands.h"
#include "../Main/wDialog.h"
#include "../Main/aiGrid.h"
#include "../Main/aiMoves.h"
#include "../Main/aiPath.h"
#include "../Main/PrecalcCollider.h"
#include "../Main/aiColourer.h"
#include "../Main/aiMap.h"
#include "../Main/aiRender.h"
#include "../Main/aiStability.h"
#include "../Main/aiNearestPosition.h"
#include "../FileIO/PortableGameDatabase.h"
#include "../FileIO/PortableStructureChunks.h"
#include "../FileIO/Cruncher.h"
#include "../Main/iMain.h"
#include "../Main/iGlobalMap.h"
#include "../Main/iChapterMap.h"
#include "../Main/iRenderWorld.h"
#include "../Main/iMainMenu.h"
#include "../Main/iLoading.h"
#include "../Main/iChapterMapUI.h"
#include "../Main/iShowMedal.h"
#include "../Main/iMedalsPanel.h"
#include "../Main/UIWrap.h"
#include "../Main/PlayerTracker.h"
#include "../Main/iOptionsMenu.h"
#include "../Main/iInGameMenu.h"
#include "../Main/iLoseFake.h"
#include "../Main/iExitMenu.h"
#include "../Main/ScreenShot.h"
#include "../Main/iSaveManager.h"
#include "../Main/iSaveLoad.h"
#include "../MiscDll/LogStream.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include "../Main/aiLocker.h"
#include "../Main/wUnitServer.h"
#include "../Main/RPGItem.h"
#include "../Main/RPGItemSet.h"
#include "../Main/RPGCritical.h"
#include "../Main/wUnitAttackExec.h"
#include "../Main/wUnitAttack.h"
#include "../Main/wUnitStates.h"
#include "../Main/wUnitQueue.h"
#include "../Main/wUnitExec.h"
#include "../Main/wUnitMove.h"
#include "../Main/wMainPath.h"
#include "../Main/wGrenade.h"
#include "../Main/wBullet.h"
#include "../Main/wAckBase.h"
#include "../Main/wDynObject.h"
#include "../Main/wDebris.h"
#include "../Main/wMisc.h"
#include "../Main/RPGAttackMech.h"
#include "../Main/aiPath.h"
#include "../Main/GAnimParticles.h"
#include "../Main/GParticleInfo.h"
#include "../Main/GAnimation.h"
#include "../Main/GDecalGeometry.h"
#include "../Main/GCombiner.h"
#include "../Main/GSceneUtils.h"
#include "../Main/GRenderCore.h"
#include "../Main/GAnimLight.h"
#include "../Main/GRenderLight.h"
#include "../Main/GLightmapStateWire.h"
#include "../Main/GInit.h"
#include "../Main/Gfx.h"
#include "../Game/Platform.h"
#include <SDL3/SDL.h>
#include "../Main/GGeometryPositionHash.h"
using GeometryTestHash = NGScene::SGeometryPosHash;
#include "../Main/rpgGlobal.h"
#include "../Main/RPGDiplomacy.h"
#include "../Main/RPGUnit.h"
#include "../Main/RPGUnitMission.h"
#include "../Main/rpgCheatConstants.h"
#include "../Main/aiCommander.h"
#include "../Main/aiRouteLogic.h"
#include "../Main/aiRoute.h"
#include "../Main/aiUnit.h"
#include "../Main/aiPlayer.h"
#include "../Main/aiReaction.h"
#include "../Main/aiActionPlaceSource.h"
#include "../Main/aiCombatLogic.h"
#include "../Main/aiReactions.h"
#include "../Main/aiGuardReaction.h"
#include "../Main/aiUnitState.h"
#include "../Main/aiEvent.h"
#include "../Main/aiThreatTracker.h"
#include "../Main/Sync.h"
#include "../Main/A5Script.h"
#include "../Main/scriptCommon.h"
#include "../Main/scriptUI.h"
#include "../Script/lstate.h"
#include "../Script/lgc.h"
#include "../Main/scriptPtr.h"
#include "../Main/scriptPosition.h"
#include "../Main/aiMisc.h"
#include "../Main/wOSBase.h"
#include "../Main/aiInterval.h"
#include "../Main/aiDoorCollider.h"
#include "../Main/MapBuild.h"
#include "../DBFormat/DataMap.h"
#include "../DBFormat/DataCamera.h"
#include "../Main/scFlowChartItems.h"
#include "../Main/scFlowChart.h"
#include "../Main/scScenarioTracker.h"
#include "../Main/RPGGame.h"
#include "../Main/aiJob.h"
#include "../Main/iMissionExec.h"
#include "../Main/iMissionInternal.h"
#include "../Main/RPGStore.h"
#include "../Main/UnitTracker.h"
#include "../DBFormat/DataFormat.h"
#include "AIUnitFixture.h"
#include "LightingSceneFixture.h"
#include "GameViewFixture.h"
#include "../Main/Transform.h"
#include "../Main/aiInventory.h"
#include "../Main/aiWeapon.h"
#include "../Main/AILog.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <new>
#include <cstddef>
#include <cmath>
#include <limits>
#include <algorithm>
#include <type_traits>
#include <map>

extern CPtr<NScript::CScript> pScript;

REGISTER_SAVELOAD_CLASS_NM(0x7f550105, AIUnitFixture, NAI);

static void Require(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}



struct ReloadLogFixture
{
    CObj<NAI::CAIFireArmsWeapon> weapon;
    CObj<NAI::CAIFireArmsWeaponClip> current, next;
};

static CStructureNetworkContext ReloadReferences(const ReloadLogFixture& fixture)
{
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[fixture.weapon.GetPtr()] = 1; references.refs[1] = fixture.weapon.GetPtr();
    references.ids[fixture.current.GetPtr()] = 2; references.refs[2] = fixture.current.GetPtr();
    references.ids[fixture.next.GetPtr()] = 3; references.refs[3] = fixture.next.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    return references;
}

static ReloadLogFixture MakeReloadLogFixture()
{
    ReloadLogFixture fixture;
    fixture.weapon = new NAI::CAIFireArmsWeapon;
    fixture.current = new NAI::CAIFireArmsWeaponClip; fixture.current->SetAmmoCount(3);
    fixture.next = new NAI::CAIFireArmsWeaponClip; fixture.next->SetAmmoCount(12);
    struct FirearmClipsWire {
        CObj<NAI::CAIFireArmsWeaponClip> current;
        vector<CObj<NAI::CAIFireArmsWeaponClip>> spare;
        int operator&(CStructureSaver& saver) { saver.Add(3, &current); saver.Add(4, &spare); return 0; }
    } base;
    base.current = fixture.current.GetPtr(); base.spare.push_back(fixture.next.GetPtr());
    auto references = ReloadReferences(fixture);
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); writer.Add(2, &base); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(fixture.weapon.GetPtr())->operator&(reader); }
    Require(fixture.weapon->GetCurrentClip() == fixture.current.GetPtr()
            && fixture.weapon->GetNextClip() == fixture.next.GetPtr(), "native AI reload clip fixture failed to restore");
    return fixture;
}

static CObj<NAI::CAILogReloadWeapon> RestoreRetailReloadLog(const ReloadLogFixture& fixture)
{
    CObj<NAI::CAILogReloadWeapon> restored = new NAI::CAILogReloadWeapon;
    auto references = ReloadReferences(fixture);
    CPtr<NAI::CAIFireArmsWeapon> weapon = fixture.weapon.GetPtr();
    CPtr<NAI::CAIFireArmsWeaponClip> next = fixture.next.GetPtr();
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); writer.Add(3, &weapon); writer.Add(4, &next); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
    return restored;
}

static void AIReloadLogRecord(int check)
{
    auto fixture = MakeReloadLogFixture();
    if (check == 0) {
        CObj<NAI::CAILogReloadWeapon> created = new NAI::CAILogReloadWeapon(nullptr, fixture.weapon);
        auto references = ReloadReferences(fixture);
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
          static_cast<CObjectBase*>(created.GetPtr())->operator&(writer); }
        saved.Seek(0);
        CPtr<NAI::CAIFireArmsWeaponClip> next, obsolete;
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(4, &next); reader.Add(5, &obsolete); }
        Require(next == fixture.next.GetPtr(), "AI reload record did not write the next clip in retail save tag 4");
        Require(obsolete == nullptr, "AI reload record retained its obsolete clip save tag 5");
        auto restored = RestoreRetailReloadLog(fixture);
        CPtr<NAI::CAIFireArmsWeaponClip> expected = fixture.next.GetPtr();
        fixture.next = nullptr;
        restored->Commit();
        Require(IsValid(expected) && fixture.weapon->GetCurrentClip() == expected.GetPtr()
                && expected->GetAmmoCount() == 12 && fixture.weapon->GetNextClip() == nullptr,
            "restored retail reload record failed to transfer spare-clip ownership before removing it");
    } else if (check == 1) {
        auto restored = RestoreRetailReloadLog(fixture);
        restored->RollBack();
        Require(fixture.weapon->GetCurrentClip() == fixture.current.GetPtr()
                && fixture.current->GetAmmoCount() == 3 && fixture.next->GetAmmoCount() == 12,
            "restored AI reload rollback changed the loaded magazine without committing");
        restored->Commit();
        Require(fixture.weapon->GetCurrentClip() == fixture.next.GetPtr(), "AI reload commit did not install the next clip");
    } else {
        CObj<NAI::CAILogReloadWeapon> created = new NAI::CAILogReloadWeapon(nullptr, fixture.weapon);
        CPtr<NAI::CAIFireArmsWeaponClip> old = fixture.current.GetPtr(), next = fixture.next.GetPtr();
        fixture.current = nullptr; fixture.next = nullptr;
        created->Commit();
        Require(!IsValid(old), "committed AI reload log retained ownership of the obsolete magazine");
        Require(IsValid(next) && fixture.weapon->GetCurrentClip() == next.GetPtr() && next->GetAmmoCount() == 12,
            "AI reload log lost the loaded spare or changed its ammunition during ownership transfer");
    }
}

static bool optionalStringAccepted = false;

static int OptionalStringBridge(lua_State* state)
{
    Script script(state);
    vector<SLuaParams> params;
    optionalStringAccepted = false;
    if (!script.CheckArgs("ss[]", "OptionalStringBridge", &params)) return 0;
    optionalStringAccepted = true;
    script.PushString(params[1].s.c_str());
    return 1;
}

static int PerStateArgumentCount(lua_State* state)
{
    const int count = NScript::luaGetParamCount(state);
    lua_pushnumber(state, count);
    return 1;
}

static void LuaArgumentState()
{
    CObj<NScript::CScript> executing = new NScript::CScript, other = new NScript::CScript;
    executing->Register("PerStateArgumentCount", PerStateArgumentCount);
    other->PushNumber(99);
    struct GlobalScriptRestore {
        CPtr<NScript::CScript> previous;
        ~GlobalScriptRestore() { pScript = previous; }
    } restore{pScript};
    pScript = other;
    Require(executing->DoString("count = PerStateArgumentCount(7, 8, 9)") == LUA_NOERR,
        "Lua argument-state fixture did not compile");
    executing->ExecuteThreads();
    Require(executing->GetGlobal("count").IsNumber() && executing->GetGlobal("count").GetNumber() == 3,
        "native Lua argument count came from another script's stack");
    Require(other->GetTop() == 1, "counting another Lua state's arguments changed the global script stack");
    pScript = nullptr;
    for (const char* code : {"count = PerStateArgumentCount()", "count = PerStateArgumentCount(1, 2, 3, 4, 5)"}) {
        Require(executing->DoString(code) == LUA_NOERR, "Lua argument-state boundary did not compile");
        executing->ExecuteThreads();
        const int expected = std::strstr(code, "1, 2") ? 5 : 0;
        Require(executing->GetGlobal("count").IsNumber() && executing->GetGlobal("count").GetNumber() == expected,
            "native Lua argument count still depended on a globally active script");
    }
}

static void LuaWindowUserdata()
{
    CMObj<NUI::CWindow> borrowed = new NUI::CWindow(NUI::SWindowInfo());
    {
        CObj<NWorld::CCmdReload> command = new NWorld::CCmdReload(2);
        CObj<NScript::CScript> script = new NScript::CScript;
        script->PushUserTag(borrowed.GetPtr(), tagLuaWindow);
        const int slot = script->GetTop();
        Require(lua_tag(script->GetState(), slot) == tagLuaWindow
                && lua_touserdata(script->GetState(), slot) == borrowed.GetPtr(),
            "Lua window userdata discarded its borrowed native window reference");
        script->PushUserTag(command.GetPtr(), tagLuaCPtr);
        Require(lua_touserdata(script->GetState(), -1) == command.GetPtr(), "window userdata adaptation broke ordinary Lua CPtr values");
        script->PushUserTag(command.GetPtr(), tagLuaCObj);
        Require(lua_touserdata(script->GetState(), -1) == command.GetPtr(), "window userdata adaptation broke owning Lua CObj values");
        CObj<NUI::CWindow> legacy = new NUI::CWindow(NUI::SWindowInfo());
        script->PushUserTag(legacy.GetPtr(), tagLuaWindow);
        UserData* record = nullptr;
        auto* state = script->GetState();
        for (int i = 0; i < state->userdatas.size(); ++i) {
            auto* candidate = state->userdatas[i];
            if (candidate && candidate->tag == tagLuaWindow &&
                    (candidate->pPtr == legacy.GetPtr() || candidate->pObj == legacy.GetPtr())) record = candidate;
        }
        Require(record != nullptr, "window userdata fixture could not find its native record");
        // Existing save fields also permit an owning tag-8 handle. Preserve
        // its read path while creating fresh UI handles as borrowed references.
        UserData owning; owning.tag = tagLuaWindow; owning.marked = 0; owning.pObj = legacy.GetPtr();
        CStructureNetworkContext references;
        references.externalReferences = true; references.include = [](CObjectBase*) { return false; };
        references.ids[legacy.GetPtr()] = 1; references.refs[1] = legacy.GetPtr();
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); owning.operator&(writer); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); record->operator&(reader); }
        Require(lua_touserdata(state, -1) == legacy.GetPtr(), "Lua stopped reading existing owning window-userdata save records");
    }
    Require(IsValid(borrowed), "destroying a Lua script invalidated a window still owned by the UI");
    {
        CObj<NScript::CScript> script = new NScript::CScript;
        CMObj<NUI::CWindow> dying = new NUI::CWindow(NUI::SWindowInfo());
        script->PushUserTag(dying.GetPtr(), tagLuaWindow);
        dying = nullptr;
        Require(!IsValid(lua_touserdata(script->GetState(), -1)), "Lua window userdata kept a removed window alive");
    }
    CObj<NUI::CInterface> interface = new NUI::CInterface;
    interface->SetInterface(interface);
    CMObj<NUI::CWindow> child = new NUI::CWindow(NUI::SWindowInfo(interface,
        NUI::SPoint(3, 4), NUI::SPoint(20, 20), "userdata-child"));
    {
        CObj<NScript::CScript> script = NScript::CreateScript(nullptr);
        script->SetScriptInterface(interface);
        struct GlobalScriptRestore {
            CPtr<NScript::CScript> previous;
            ~GlobalScriptRestore() { pScript = previous; }
        } restore{pScript};
        pScript = script;
        Require(script->DoString("w=GetWindow('userdata-child'); if w then windowSetProperty(w, 'x', 42); windowSetProperty(w, 'y', 17) end") == LUA_NOERR,
            "native window-userdata fixture did not compile");
        script->ExecuteThreads();
        Require(child->GetPosition() == NUI::SPoint(42, 17), "Lua window lookup/property bindings did not reach the borrowed native control");
    }
    Require(IsValid(child) && interface->GetChildByID("userdata-child") == child.GetPtr(),
        "closing a window script invalidated a control retained by its interface");
}

static void LuaWindowSetterStack()
{
    CObj<NUI::CInterface> interface = new NUI::CInterface;
    interface->SetInterface(interface);
    CMObj<NUI::CWindow> child = new NUI::CWindow(NUI::SWindowInfo(interface,
        NUI::SPoint(3, 4), NUI::SPoint(20, 20), "setter-child"));
    CObj<NScript::CScript> script = NScript::CreateScript(nullptr);
    script->SetScriptInterface(interface);
    struct GlobalScriptRestore {
        CPtr<NScript::CScript> previous;
        ~GlobalScriptRestore() { pScript = previous; }
    } restore{pScript};
    pScript = script;
    Require(script->DoString("local w=GetWindow('setter-child'); w.x=42; windowSetProperty(w, 'y', 17); afterSetter=123") == LUA_NOERR,
        "native Lua setter-stack fixture did not compile");
    script->ExecuteThreads();
    Require(script->GetGlobal("afterSetter").IsNumber() && script->GetGlobal("afterSetter").GetNumber() == 123
            && child->GetPosition() == NUI::SPoint(42, 17),
        "Lua dotted window assignment corrupted the stack before the following native call");
    script->SetTop(0);
    script->PushUserTag(child.GetPtr(), tagLuaWindow);
    const int originalTop = script->GetTop();
    lua_pushstring(script->GetState(), "x"); lua_pushnumber(script->GetState(), 51);
    lua_settable(script->GetState(), -3);
    Require(script->GetTop() == originalTop && child->GetPosition() == NUI::SPoint(51, 17),
        "Lua VM setter correction changed the C API's explicit stack-pop contract");
    script->SetTop(0);
    Require(script->DoString("local t={}; t.x=7; plainSetter=t.x+1; afterPlain=99") == LUA_NOERR,
        "plain Lua setter control did not compile");
    script->ExecuteThreads();
    Require(script->GetGlobal("plainSetter").GetNumber() == 8 && script->GetGlobal("afterPlain").GetNumber() == 99,
        "window setter correction changed ordinary Lua table assignment");
}

static void LuaWindowStateContext()
{
    CObj<NUI::CInterface> first = new NUI::CInterface, second = new NUI::CInterface;
    first->SetInterface(first); second->SetInterface(second);
    CMObj<NUI::CWindow> own = new NUI::CWindow(NUI::SWindowInfo(first,
        NUI::SPoint(3, 4), NUI::SPoint(20, 20), "context-child"));
    CMObj<NUI::CWindow> foreign = new NUI::CWindow(NUI::SWindowInfo(second,
        NUI::SPoint(7, 8), NUI::SPoint(20, 20), "context-child"));
    CObj<NScript::CScript> executing = NScript::CreateScript(nullptr), other = NScript::CreateScript(nullptr);
    executing->SetScriptInterface(first); other->SetScriptInterface(second);
    other->PushNumber(999);
    struct GlobalScriptRestore {
        CPtr<NScript::CScript> previous;
        ~GlobalScriptRestore() { pScript = previous; }
    } restore{pScript};
    pScript = other;
    Require(executing->DoString("local w=GetWindow('context-child'); if w then w.x=31 end; afterOwnContext=1") == LUA_NOERR,
        "per-state UI script fixture did not compile");
    executing->ExecuteThreads();
    Require(own->GetPosition() == NUI::SPoint(31, 4) && foreign->GetPosition() == NUI::SPoint(7, 8),
        "Lua window lookup/property dispatch ignored its owning state in favor of an unrelated global script");
    Require(executing->GetState()->pContext == executing.GetPtr() && other->GetTop() == 1,
        "per-state script context or the unrelated script stack was changed incorrectly");
    executing->SetTop(0);
    pScript = nullptr;
    Require(executing->DoString("local w=GetWindow('context-child'); if w then w.y=47 end; afterNullContext=1") == LUA_NOERR,
        "UI script without a global context did not compile");
    executing->ExecuteThreads();
    Require(own->GetPosition() == NUI::SPoint(31, 47) && foreign->GetPosition() == NUI::SPoint(7, 8),
        "UI dispatch could not resolve its own state when the global script was empty");
    executing->SetTop(0);
    executing->GetState()->pContext = nullptr;
    pScript = executing;
    Require(executing->DoString("windowSetProperty(GetWindow('context-child'), 'x', 59)") == LUA_NOERR,
        "legacy global-context control did not compile");
    executing->ExecuteThreads();
    Require(own->GetPosition() == NUI::SPoint(59, 47), "UI dispatch lost the existing global fallback for unbound Lua states");
    pScript = nullptr;
    Script unbound;
    unbound.Register("GetWindow", NScript::luaGetWindow);
    Require(unbound.DoString("missing=GetWindow('context-child')") == LUA_NOERR,
        "unbound Lua-state control did not compile");
    unbound.ExecuteThreads();
    Require(unbound.GetGlobal("missing").IsNil(), "an unbound Lua state acquired a foreign window context");
}

static void WindowMessageCallbacks()
{
    CObj<NUI::CInterface> interface = new NUI::CInterface;
    interface->SetInterface(interface);
    CMObj<NUI::CWindow> window = new NUI::CWindow(NUI::SWindowInfo(interface,
        NUI::SPoint(3, 4), NUI::SPoint(20, 20), "callback-child"));
    CObj<NScript::CScript> script = NScript::CreateScript(nullptr), unrelated = NScript::CreateScript(nullptr);
    script->SetScriptInterface(interface);
    unrelated->PushNumber(888);
    struct GlobalScriptRestore {
        CPtr<NScript::CScript> previous;
        ~GlobalScriptRestore() { pScript = previous; }
    } restore{pScript};
    pScript = unrelated;
    const bool baselineHandled = window->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY, std::string("baseline")));
    Require(script->DoString("calls=0; local w=GetWindow('callback-child'); unset=(w.onmessage==nil) and 1 or 0; handler=function(w,event,id) calls=calls+1; kind=event; notice=id; w.x=w.x+1; return 1 end; w.onmessage=handler; sameHandler=(w.onmessage==handler) and 1 or 0") == LUA_NOERR,
        "window callback fixture did not compile");
    script->ExecuteThreads();
    auto number = [&](const char* name) {
        Script::AutoBlock stack(*script);
        auto value = script->GetGlobal(name);
        Require(value.IsNumber(), "window callback did not publish its numeric result");
        return value.GetNumber();
    };
    auto text = [&](const char* name) {
        Script::AutoBlock stack(*script);
        auto value = script->GetGlobal(name);
        Require(value.IsString(), "window callback did not publish its event name");
        return std::string(value.GetString());
    };
    const bool handled = window->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY, std::string("ready")));
    Require(number("calls") == 1 && window->GetPosition() == NUI::SPoint(4, 4),
        "registered Lua onmessage handler did not receive the native window notification");
    Require(handled == baselineHandled, "Lua callback changed native notification handling");
    Require(text("kind") == "notify" && text("notice") == "ready", "Lua callback received incorrect notification arguments");
    Require(number("unset") == 1, "an unset window callback was not nil");
    Require(number("sameHandler") == 1, "window callback getter did not return the registered closure");
    Require(unrelated->GetTop() == 1, "window callback changed an unrelated script stack");
    const std::pair<int, const char*> mouseEvents[] = {
        {NUI::EVENT_LBUTTONDOWN, "lbtndown"}, {NUI::EVENT_LBUTTONUP, "lbtnup"},
        {NUI::EVENT_LBUTTONDBLCLK, "lbtndblclk"}, {NUI::EVENT_RBUTTONDOWN, "rbtndown"},
        {NUI::EVENT_RBUTTONUP, "rbtnup"}, {NUI::EVENT_RBUTTONDBLCLK, "rbtndblclk"}
    };
    int expected = 1;
    for (const auto& event : mouseEvents) {
        Require(!window->ProcessMessage(NUI::SEvent(event.first, -100, -100)),
            "Lua callback return value consumed an unhandled mouse event");
        Require(number("calls") == ++expected && text("kind") == event.second,
            "native mouse event was not forwarded under its retail Lua callback name");
        Script::AutoBlock stack(*script);
        Require(script->GetGlobal("notice").IsNil(), "mouse callback received a spurious notification ID");
    }
    window->ProcessMessage(NUI::SEvent(NUI::EVENT_ACTIVATE, NUI::EAF_DEACTIVATE));
    Require(!window->IsActive() && number("calls") == expected,
        "window callback intercepted an unsupported event or prevented built-in activation handling");
    Require(script->DoString("GetWindow('callback-child').onmessage=nil; cleared=(GetWindow('callback-child').onmessage==nil) and 1 or 0") == LUA_NOERR,
        "window callback clear control did not compile");
    script->ExecuteThreads();
    window->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY, std::string("after-clear")));
    Require(number("calls") == expected && number("cleared") == 1 && IsValid(window),
        "clearing a Lua callback retained its handler or invalidated the UI-owned window");
}

static void WindowCallbackLifetime()
{
    CObj<NUI::CInterface> interface = new NUI::CInterface;
    interface->SetInterface(interface);
    CMObj<NUI::CWindow> first = new NUI::CWindow(NUI::SWindowInfo(interface,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "gc-first"));
    CMObj<NUI::CWindow> second = new NUI::CWindow(NUI::SWindowInfo(interface,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "gc-second"));
    CObj<NScript::CScript> script = NScript::CreateScript(nullptr);
    script->SetScriptInterface(interface);
    lua_State* state = script->GetState();
    auto execute = [&](const char* code) {
        Require(script->DoString(code) == LUA_NOERR, "window callback lifetime fixture did not compile");
        script->ExecuteThreads();
        script->SetTop(0);
        luaC_checkGC(state);
    };
    auto alive = [&](int handle) {
        return handle > 0 && handle <= state->closures.size() && state->closures[handle - 1] != nullptr;
    };
    auto calls = [&]() {
        Script::AutoBlock stack(*script);
        return script->GetGlobal("calls").GetNumber();
    };
    execute("calls=0; GetWindow('gc-first').onmessage=function(w,event,id) calls=calls+1; w.y=15 end");
    const int anonymous = first->GetEventHandler(script);
    Require(alive(anonymous), "anonymous window callback was reclaimed while still registered");
    first->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY, std::string("anonymous")));
    Require(calls() == 1 && first->GetPosition().y == 15,
        "anonymous window callback could not execute after Lua garbage collection");
    execute("GetWindow('gc-first').onmessage=nil");
    Require(!alive(anonymous), "cleared anonymous window callback remained rooted in Lua");
    execute("shared=function(w,event,id) calls=calls+1 end; GetWindow('gc-first').onmessage=shared; GetWindow('gc-second').onmessage=shared; shared=nil");
    const int shared = second->GetEventHandler(script);
    Require(alive(shared) && shared == first->GetEventHandler(script),
        "two windows could not share a callback without a Lua global root");
    execute("GetWindow('gc-first').onmessage=nil");
    Require(alive(shared), "clearing one window reclaimed another window's shared callback");
    second->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY, std::string("shared")));
    Require(calls() == 2, "shared window callback did not survive removal from another window");
    CPtr<NUI::CWindow> released = second.GetPtr();
    interface->RemoveChild(second);
    second = nullptr;
    luaC_checkGC(state);
    Require(!IsValid(released) && !alive(shared), "destroying a window retained its otherwise unreachable callback");
    Require(script->GetTop() == 0, "window callback rooting changed the owning Lua stack");
}

static void WorldlessUIScript()
{
    CObj<NScript::CScript> script = NScript::CreateScript(nullptr);
    Require(IsValid(script), "native script factory rejected a UI script without a tactical world");
    Require(!IsValid(script->pWorld), "worldless UI script unexpectedly acquired a tactical world");
    CObj<NUI::CInterface> interface = new NUI::CInterface;
    interface->SetInterface(interface);
    script->SetScriptInterface(interface);
    struct GlobalScriptRestore {
        CPtr<NScript::CScript> previous;
        ~GlobalScriptRestore() { pScript = previous; }
    } restore{pScript};
    pScript = script;
    Require(script->DoString("cursor=GetCursorPos(); factoryValue=40+2; cursorX=cursor.x; cursorY=cursor.y") == LUA_NOERR,
        "worldless UI script did not compile its native cursor binding");
    script->ExecuteThreads();
    Require(script->GetScriptInterface() == interface.GetPtr()
            && script->GetGlobal("factoryValue").IsNumber() && script->GetGlobal("factoryValue").GetNumber() == 42
            && script->GetGlobal("cursorX").IsNumber() && script->GetGlobal("cursorX").GetNumber() == interface->GetCursorPos().x
            && script->GetGlobal("cursorY").IsNumber() && script->GetGlobal("cursorY").GetNumber() == interface->GetCursorPos().y,
        "worldless UI script lost its interface, Lua execution or native cursor access");
    CObj<NWorld::CWorld> world = new NWorld::CWorld;
    CObj<NScript::CScript> tactical = NScript::CreateScript(world);
    Require(IsValid(tactical) && tactical->pWorld == world.GetPtr(),
        "allowing UI scripts changed the native tactical script factory path");
}

static void PrecalcSweepBounds()
{
    const vector<CVec3> points{
        CVec3(0,-.5f,-.5f),CVec3(.07f,-.5f,-.5f),CVec3(.07f,.5f,-.5f),CVec3(0,.5f,-.5f),
        CVec3(0,-.5f,.5f),CVec3(.07f,-.5f,.5f),CVec3(.07f,.5f,.5f),CVec3(0,.5f,.5f)};
    const vector<STriangle> triangles{
        STriangle(0,2,1),STriangle(0,3,2),STriangle(4,5,6),STriangle(4,6,7),
        STriangle(0,1,5),STriangle(0,5,4),STriangle(3,7,6),STriangle(3,6,2),
        STriangle(0,4,7),STriangle(0,7,3),STriangle(1,2,6),STriangle(1,6,5)};
    CObj<NAI::CPrecalcSpheres> grid = new NAI::CPrecalcSpheres;
    Require(!grid->Generate(points,triangles,0), "thin-wall fixture unexpectedly requires another voxel Z block");
    NCollider::CPrecalcCollider<int> collider;
    SBound sceneBound,geometryBound;
    sceneBound.BoxInit(CVec3(-4,-4,-4),CVec3(4,4,4));
    geometryBound.BoxInit(CVec3(0,-.5f,-.5f),CVec3(.07f,.5f,.5f));
    collider.PrepareCollider(sceneBound,1.f);
    SHMatrix identity; Identity(&identity);
    collider.AddEntity(identity,identity,vector<CPtr<NAI::CPrecalcSpheres>>{grid.GetPtr()},false,17,geometryBound);
    struct HitAnalyzer {
        int calls = 0;
        bool operator()(int owner) { Require(owner == 17, "precalc sweep changed its collision owner"); ++calls; return true; }
    };
    for (const auto& velocity: {VNULL3,CVec3(0,.05f,0)}) {
        for (float x: {-.30f,.37f}) {
            HitAnalyzer hit;
            Require(collider.CollideBool(SSphere(CVec3(x,0,0),NAI::F_TEST_SPHERE_RADIUS),velocity,&hit) && hit.calls == 1,
                "a genuine thin-wall collision was lost, including the neighbouring broad-phase cell");
        }
        const CVec3 outside(.381f,0,0);
        Require(outside.x - .07f > NAI::F_TEST_SPHERE_RADIUS && grid->IsMovingSphereCollided(outside,outside+velocity),
            "thin-wall fixture did not expose conservative voxel rounding outside the exact padded bound");
        HitAnalyzer hit;
        Require(!collider.CollideBool(SSphere(outside,NAI::F_TEST_SPHERE_RADIUS),velocity,&hit) && hit.calls == 0,
            "a precalculated thin wall reported a collision outside its radius-padded bound because the sweep radius was counted twice");
    }
}

static void MissingLuaGlobalWarning()
{
    Script script;
    auto warnings = [&]() {
        return std::count_if(consoleLines.begin(),consoleLines.end(),[](const SConsoleLine& line) {
            return line.eType == STREAM_SYSTEM && line.szText.find(L"[Script warning!] Value was NIL when getting global with name ") != wstring::npos;
        });
    };
    const auto before = warnings();
    Require(script.GetGlobal("UpstreamMissingGlobalDiagnostic").IsNil(), "missing-global lookup changed its nil result");
    Require(warnings() == before + 1 &&
        consoleLines.front().szText.find(L"UpstreamMissingGlobalDiagnostic") != wstring::npos,
        "native Lua silently returned nil for a missing global without naming it in the diagnostic log");
    script.SetTop(0);
    const auto afterMissing = warnings();
    Require(script.GetGlobal("FALSE").IsNil() && warnings() == afterMissing,
        "Lua's deliberately nil FALSE constant generated a missing-global warning");
    script.SetTop(0);
    Require(script.DoString("UpstreamDeclaredGlobalDiagnostic=42") == LUA_NOERR,
        "missing-global fixture could not compile its declared-variable control");
    script.ExecuteThreads();
    Require(script.GetGlobal("UpstreamDeclaredGlobalDiagnostic").GetNumber() == 42 && warnings() == afterMissing,
        "a declared Lua global changed value or generated a missing-global warning");
}

static void EmptyLuaDefault()
{
    Script script;
    script.Register("OptionalStringBridge", OptionalStringBridge);
    Require(script.DoString("result = OptionalStringBridge('first-visit-key')") == LUA_NOERR,
        "optional-string fixture did not compile");
    script.ExecuteThreads();
    Require(optionalStringAccepted, "Lua rejected an omitted optional string with an empty default");
    Require(script.GetGlobal("result").IsString() && std::string(script.GetGlobal("result").GetString()).empty(),
        "omitted optional Lua string did not receive its empty default");
    script.SetTop(0);
    script.DoString("result = OptionalStringBridge('key', 'explicit')");
    script.ExecuteThreads();
    Require(optionalStringAccepted && std::string(script.GetGlobal("result").GetString()) == "explicit",
        "empty-default handling changed an explicitly supplied Lua string");
    script.SetTop(0);
    script.DoString("result = OptionalStringBridge('key', '')");
    script.ExecuteThreads();
    Require(optionalStringAccepted && std::string(script.GetGlobal("result").GetString()).empty(),
        "empty-default handling changed an explicitly empty Lua string");
    script.SetTop(0);
    script.DoString("OptionalStringBridge()");
    script.ExecuteThreads();
    Require(!optionalStringAccepted, "empty-default handling accepted a missing required Lua argument");
}


DEFINE_DG_CONSTANT_NODE(AimerPoseProbe, NAnimation::SSkeletonPose);
DEFINE_DG_CONSTANT_NODE(AimerTimeProbe, STime);

static void SmartAimerBoundary()
{
    NAnimation::SSkeletonPose target(1), output(1);
    CObj<AimerPoseProbe> pose = new AimerPoseProbe(target);
    CObj<AimerTimeProbe> time = new AimerTimeProbe(200);
    CObj<NAnimation::CASmartAimer> aimer = new NAnimation::CASmartAimer;
    aimer->pNextPose = pose;
    aimer->pNextPoseTime = time;
    aimer->GetFrame(100, &output);
    Require(fabs2(output[0].pos) < 0.000001f, "smart-aimer fixture did not start at its initial pose");
    target[0].pos.x = 10.f;
    pose->Set(target);
    aimer->GetFrame(200, &output);
    Require(fabs2(output[0].pos - target[0].pos) < 0.000001f,
        "smart aimer did not return the exact target at the transition endpoint");
    // Retail returns the endpoint without reseeding the interpolator. The next
    // target must still blend from the last sampled interior pose/time.
    target[0].pos.x = 25.f;
    pose->Set(target);
    time->Set(300);
    aimer->GetFrame(250, &output);
    Require(std::fabs(output[0].pos.x - 18.75f) < 0.0001f,
        "exact animation endpoint reseeded smart-aimer interpolation for the next transition");
    aimer->GetFrame(301, &output);
    Require(fabs2(output[0].pos - target[0].pos) < 0.000001f,
        "smart aimer changed its completed-transition output");
}

static void UnsupportedNavigationFloor()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    const int first = network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    auto* layer = network->GetLayer(first);
    const int supporting = network->CreateLayer(8, 8, VNULL2, 0.f, 1, layer->pGroup);
    auto* same = network->GetLayer(supporting);
    auto& empty = layer->tiles[2][3];
    auto& support = same->tiles[2][3];
    empty = NAI::STile{}; support = NAI::STile{};
    empty.nFloor = 100;
    support.nFloor = -1; support.nPassable = 7;
    const NAI::SPathPlace place(3, 2, first);
    Require(network->GetFloor(place) == -1, "loaded empty navigation slot did not resolve its real supporting surface floor");
    Require(empty.nFloor == 100 && empty.nHeight == 0 && empty.nPassable == 0,
        "navigation floor lookup rewrote the saved empty tile instead of resolving its support");
    empty.nFloor = 2;
    Require(network->GetFloor(place) == 2, "native tile floor was replaced by another layer's surface floor");
    empty.nFloor = 100;
    support.nHeight = 1;
    Require(network->GetFloor(place) == 100, "unrelated height was accepted as navigation support");
    support.nHeight = 0; support.nDisplacement = 1;
    Require(network->GetFloor(place) == 100, "displaced navigation tile was accepted as identical support");
    support.nDisplacement = 0; support.nPassable = 0;
    Require(network->GetFloor(place) == 100, "impassable navigation surface was accepted as support");
    const int foreign = network->CreateLayer(8, 8, CVec2(8, 0), 0.f, 0);
    auto& foreignTile = network->GetLayer(foreign)->tiles[2][3];
    foreignTile = NAI::STile{}; foreignTile.nFloor = 3; foreignTile.nPassable = 7;
    Require(network->GetFloor(place) == 100, "navigation support leaked across unrelated layer groups");
    support.nFloor = 100; support.nPassable = 7;
    Require(network->GetFloor(place) == 100, "another empty navigation slot invented a supporting floor");
    support.nFloor = -1; empty.nHeight = support.nHeight = 123;
    Require(network->GetFloor(place) == -1, "saved duplicate support resolution failed above zero height");
    NAI::CNodesLayer::SLadder ladder{};
    ladder.placeOnBottom = place;
    layer->ladders.push_back(ladder);
    NAI::SPathPlace rung(0, 4, first); rung.SetIntegral(0);
    Require(network->GetFloor(rung) == 0, "ladder floor calculation did not resolve its bottom support before adding rung floors");
    NAI::SPathPlace flying(0, 0, 200); flying.SetFinal(1);
    Require(network->GetFloor(flying) == 4, "navigation support correction changed flying-position floor handling");
}

static void LazyNavigationColourer()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    auto* layer = network->GetLayer(0);
    const bool allocatedBeforeFirstRequest = IsValid(layer->pColourer);
    layer->pColourer = nullptr; // Same missing-cache state as a compact save.
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            auto& tile = layer->tiles[y][x];
            tile = NAI::STile{}; tile.nPassable = NAI::CP_STAND;
            tile.nMoveStand = static_cast<char>(0xff); tile.nFlags = NAI::TF_STAND_PASSABLE;
        }
    const NAI::CPathNetwork* query = network.GetPtr();
    auto* cache = query->GetColourer(0);
    Require(IsValid(cache) && cache == layer->pColourer.GetPtr(), "native navigation lookup did not reconstruct a cache omitted by a compact save");
    cache->ConstructColouring(network);
    Require(cache->IsReady() && cache->GetPointColor(3, 3) == cache->GetPointColor(4, 3),
        "lazily allocated navigation cache did not build the real open-floor connectivity");
    Require(query->GetColourer(0) == cache && cache->IsReady(), "repeated navigation cache lookup discarded a completed cache");
    CPtr<NAI::CMapColourer> previous = cache;
    layer->pColourer = nullptr;
    cache = query->GetColourer(0);
    Require(IsValid(cache) && !IsValid(previous) && cache != previous.GetPtr(),
        "navigation cache lookup failed to restore a cache omitted by a compact save");
    cache->ConstructColouring(network);
    cache->Invalidate();
    Require(!IsValid(layer->pColourer), "stale navigation cache fixture remained valid");
    cache = query->GetColourer(0);
    Require(IsValid(cache), "navigation lookup returned an invalidated colour cache");
    cache->ConstructColouring(network);
    Require(cache->IsReady() && cache->GetPointColor(3, 3) == cache->GetPointColor(4, 3),
        "replacement navigation cache lost its layer binding or connectivity");
    CObj<NAI::IAIMapTracker> tracker = dynamic_cast<NAI::IAIMapTracker*>(pSSClasses->CreateObject(0x72952150));
    Require(IsValid(tracker), "native navigation change tracker is not registered");
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.include = [](CObjectBase*) { return false; };
    references.ids[layer->pGroup.GetPtr()] = 1; references.refs[1] = layer->pGroup.GetPtr();
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
      CPtr<NAI::CLayersGroup> group = layer->pGroup.GetPtr(); CTPoint<int> cell(0, 0);
      writer.Add(2, &group); writer.Add(3, &cell); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references); static_cast<CObjectBase*>(tracker.GetPtr())->operator&(reader); }
    layer->pColourer = nullptr;
    layer->pGroup->squareLevel[0][0] = 2;
    tracker->OnChange();
    Require(IsValid(layer->pColourer) && !layer->pColourer->zonesToRecalc.empty() && layer->pGroup->squareLevel[0][0] == 0,
        "native scenery change did not reconstruct an omitted navigation cache before marking its dirty zone");
    Require(!allocatedBeforeFirstRequest, "fresh navigation layer allocated its colour cache before the first request");
}

static void FreshNavigationTransitions()
{
    // Full attachment is the control; the incremental entry point must produce
    // the same real inter-layer neighbours even with no dirty rectangles.
    for (bool fullFirst: {true, false}) {
        CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
        const int first = network->CreateLayer(8, 8, CVec2(0, 0), 0.f, 0);
        const int second = network->CreateLayer(8, 8, CVec2(8, 0), 0.f, 0);
        Require(first == 0 && second == 1, "transition fixture did not create its two layers");
        for (int index: {first, second}) {
            auto* layer = network->GetLayer(index);
            for (int y = 0; y < 8; ++y)
                for (int x = 0; x < 8; ++x) {
                    layer->tiles[y][x] = NAI::STile{};
                    layer->tiles[y][x].nPassable = 7;
                    layer->tiles[y][x].nFlags = NAI::TF_STAND_PASSABLE;
                }
        }
        const NAI::SPathPlace source(1, 1, first), destination(1, 1, second);
        auto* sourceLayer = network->GetLayer(first);
        sourceLayer->tiles[1][1].nFlags |= NAI::TF_HAS_INTERGRID;
        sourceLayer->transitions[source].links.emplace_back(destination, 0);
        auto* colourer = network->GetColourer(first);
        colourer->ConstructColouring(network);
        network->GetColourer(second)->ConstructColouring(network);
        if (fullFirst) colourer->AttachTransitions(network);
        else colourer->RecalcTransitions(network);
        auto neighbours = [&](NAI::EPathfinderMode mode) {
            vector<unsigned char> xs, ys;
            vector<int> layers;
            colourer->GetColorNeighbourCenters(network, colourer->GetPointColor(1, 1), &xs, &ys, &layers, mode);
            Require(xs.size() == layers.size() && ys.size() == layers.size(), "transition neighbour coordinates were truncated");
            return layers;
        };
        for (auto mode: {NAI::PM_STAND_ONLY, NAI::PM_ANY_MOVE}) {
            const auto result = neighbours(mode);
            Require(result.size() == 1 && result[0] == second,
                fullFirst ? "full attachment control did not connect the two layers"
                          : "first incremental transition update omitted existing inter-layer links");
        }
        sourceLayer->transitions[source].links.emplace_back(NAI::SPathPlace(2, 2, second), 0);
        colourer->RecalcTransitions(network);
        Require(neighbours(NAI::PM_STAND_ONLY).size() == 1 && neighbours(NAI::PM_ANY_MOVE).size() == 1,
            "already attached transition cache was rebuilt without a dirty region");
    }
}

static void FreshNavigationColouring()
{
    CObj<NAI::CNodesLayer> layer = new NAI::CNodesLayer;
    layer->nLayer = layer->nFloor = 0;
    layer->tiles.SetSizes(8, 8);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) layer->tiles[y][x] = NAI::STile{};
    CObj<NAI::CMapColourer> full = new NAI::CMapColourer;
    full->SetLayer(layer, nullptr, 0);
    full->ConstructColouring(nullptr);
    Require(full->IsReady() && full->GetPointColor(0, 0) != full->GetPointColor(7, 7),
        "fresh-colouring fixture did not build separate blocked regions");
    CObj<NAI::CMapColourer> incremental = new NAI::CMapColourer;
    incremental->SetLayer(layer, nullptr, 0);
    incremental->RecalcColouring(nullptr, 0);
    Require(incremental->IsReady(),
        "first incremental navigation update left a new layer without its connectivity cache");
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            Require(incremental->GetPointColor(x, y) == full->GetPointColor(x, y),
                "first incremental navigation update differs from a full connectivity build");
    const auto first = incremental->GetPointColor(0, 0), last = incremental->GetPointColor(7, 7);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            auto& tile = layer->tiles[y][x];
            for (auto& moves: tile.nMove) moves = static_cast<char>(0xff);
            tile.nFlags = NAI::TF_STAND_PASSABLE;
            tile.nPassable = 7;
        }
    full->ConstructColouring(nullptr);
    Require(full->GetPointColor(0, 0) == full->GetPointColor(7, 7),
        "modified colouring fixture did not form a connected region");
    incremental->RecalcColouring(nullptr, 0);
    Require(incremental->GetPointColor(0, 0) == first && incremental->GetPointColor(7, 7) == last,
        "already constructed colourer repeated a full build without a dirty region");
}

class EmptySearchNetwork: public NAI::CPathNetwork
{
    OBJECT_BASIC_METHODS(EmptySearchNetwork);
public:
    int queries = 0;
    void GetNearPlaces(const SSphere&, vector<NAI::SPathPlace>* places, bool) override {
        Require(++queries <= 8, "nearest-position search continued beyond its maximum radius on an empty grid");
        places->clear();
    }
};

static void EmptyNearestPosition()
{
    CObj<EmptySearchNetwork> network = new EmptySearchNetwork;
    const auto result = NAI::GetNearestPosition(VNULL3, network);
    Require(result.p == NAI::SPathPlace() && result.GetNetwork() == network.GetPtr(),
        "empty nearest-position search returned an unrelated grid or a valid tile");
    Require(network->queries == 8, "nearest-position search skipped its bounded fallback attempts");
}

class FiniteSearchNetwork: public NAI::CPathNetwork
{
    OBJECT_BASIC_METHODS(FiniteSearchNetwork);
public:
    int queries = 0;
    void GetNearPlaces(const SSphere& sphere, vector<NAI::SPathPlace>* places, bool) override {
        ++queries;
        Require(std::isfinite(sphere.ptCenter.x) && std::isfinite(sphere.ptCenter.y) && std::isfinite(sphere.ptCenter.z),
            "non-finite nearest-position coordinates reached navigation grid conversion");
        places->clear();
    }
};

static void InvalidNearestPosition()
{
    CObj<FiniteSearchNetwork> network = new FiniteSearchNetwork;
    for (float invalid : {std::numeric_limits<float>::quiet_NaN(),
            std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()}) {
        for (int axis = 0; axis < 3; ++axis) {
            CVec3 point = VNULL3;
            if (axis == 0) point.x = invalid;
            if (axis == 1) point.y = invalid;
            if (axis == 2) point.z = invalid;
            for (bool native : {false, true}) {
                const auto result = native ? NAI::GetNearestNativePosition(point, network)
                    : NAI::GetNearestPosition(point, network);
                Require(result.p == NAI::SPathPlace(), "invalid coordinates produced a navigation tile");
            }
        }
    }
    Require(network->queries == 0, "invalid nearest-position search queried navigation tiles");
    Require(NAI::GetNearestPosition(VNULL3, nullptr).p == NAI::SPathPlace(),
        "missing navigation network produced a valid tile");
    network->Invalidate();
    Require(NAI::GetNearestPosition(VNULL3, network).p == NAI::SPathPlace(),
        "invalid navigation network produced a valid tile");
}

class RagdollRestorationProbe: public NAnimation::CAParticle
{
    OBJECT_BASIC_METHODS(RagdollRestorationProbe);
public:
    RagdollRestorationProbe() { nParticleBones = {0, 1, 2}; }
    using CAParticle::RestoreLine;
    using CAParticle::RestoreTriangle;
};

static void RagdollRestoration()
{
    CObj<RagdollRestorationProbe> particle = new RagdollRestorationProbe;
    const CVec3 shift(7, 8, 9);
    for (bool triangle : {false, true}) {
        // Reversing the first edge exercises the second alignment; flipping
        // the plane additionally exercises the first triangle alignment.
        for (int shape = 0; shape < 5; ++shape) {
            NAnimation::SSkeletonPose pose(3);
            pose[0].pos = VNULL3;
            pose[1].pos = CVec3(1, 0, 0);
            pose[2].pos = CVec3(0, 1, 0);
            vector<CVec3> parts = {shift, shift + CVec3(-1, 0, 0), shift + CVec3(0, -1, 0)};
            if (shape == 1) parts[1] = shift + CVec3(1, 0, 0);
            if (shape == 2) parts[1] = shift; // collapsed destination edge
            if (shape == 3) pose[1].pos = VNULL3; // collapsed source edge
            if (shape == 4) parts[2] = parts[1]; // collapsed triangle plane
            const CVec3 oldEdge = pose[1].pos - pose[0].pos;
            const CVec3 oldOther = pose[2].pos - pose[0].pos;
            if (triangle) particle->RestoreTriangle(parts, 0, 1, 2, 0, &pose);
            else particle->RestoreLine(parts, 0, 1, 0, &pose);
            float rotation[4];
            pose[0].rot.GetComponentsForWire(rotation);
            for (float value : rotation)
                Require(std::isfinite(value), "degenerate ragdoll restoration produced a non-finite rotation");
            Require(fabs2(pose[0].pos - shift) < 1e-8f,
                "ragdoll restoration lost the particle translation");
            if (shape < 2) {
                Require(fabs2(pose[0].rot.Rotate(oldEdge) - (parts[1] - parts[0])) < 1e-8f,
                    "half-turn ragdoll restoration failed to align the bone");
                if (triangle)
                    Require(fabs2(pose[0].rot.Rotate(oldOther) - (parts[2] - parts[0])) < 1e-8f,
                        "half-turn ragdoll restoration failed to align the triangle plane");
            }
        }
    }
}

static void PartialReferenceImport(const char* databasePath)
{
    CFileStream source;
    source.OpenRead(databasePath);
    vector<unsigned char> bytes(source.GetSize());
    source.Read(bytes.data(), static_cast<unsigned int>(bytes.size()));
    auto load = [](const vector<unsigned char>& data, bool overlay) {
        CMemoryStream stream;
        stream.Write(data.data(), static_cast<unsigned int>(data.size()));
        stream.Seek(0);
        NDatabase::Serialize(stream, CStructureSaver::READ, overlay);
    };
    load(bytes, false);
    auto* map = NDb::GetGlobalMap(3);
    Require(map && IsValid(map->pStartZone), "fixture intro zone reference is missing");
    CPtr<NDb::CDBScenarioZone> previous = map->pStartZone;
    vector<unsigned char> partial = bytes;
    const string column = "StartZoneID", absent = "AbsentZoneX";
    int replaced = 0;
    for (auto it = partial.begin();;) {
        it = std::search(it, partial.end(), column.begin(), column.end());
        if (it == partial.end()) break;
        std::copy(absent.begin(), absent.end(), it);
        it += column.size(); ++replaced;
    }
    Require(replaced == 2, "fixture reference column is not uniquely described");
    load(partial, true);
    Require(NDb::GetGlobalMap(3) == map && map->pStartZone == previous,
        "partial database import cleared a reference whose column was absent");

    // A present reference must still be imported, including an explicit null.
    S2FileIO::PortableGameDatabase database;
    string error;
    Require(S2FileIO::LoadPortableGameDatabaseBytes(bytes.data(), bytes.size(), &database, &error),
        "fixture database decoder failed");
    bool changed = false;
    for (const auto& table : database.tables) {
        if (table.tableId != 16) continue;
        const auto id = std::find(table.intNames.begin(), table.intNames.end(), "ID") - table.intNames.begin();
        const auto ref = std::find(table.intNames.begin(), table.intNames.end(), column) - table.intNames.begin();
        Require(id < table.intNames.size() && ref < table.intNames.size(), "fixture reference indices are missing");
        for (const auto& row : table.intRows) {
            if (row[id] != 3) continue;
            vector<unsigned char> pattern;
            for (int value : row)
                for (unsigned shift = 0; shift < 32; shift += 8)
                    pattern.push_back(static_cast<unsigned char>(static_cast<unsigned>(value) >> shift));
            auto it = std::search(bytes.begin(), bytes.end(), pattern.begin(), pattern.end());
            Require(it != bytes.end() && std::search(it + 1, bytes.end(), pattern.begin(), pattern.end()) == bytes.end(),
                "fixture database row bytes are not unique");
            std::fill(it + 4 * ref, it + 4 * ref + 4, 0);
            changed = true;
        }
    }
    Require(changed, "fixture reference row was not patched");
    load(bytes, true);
    Require(!IsValid(map->pStartZone), "explicit null reference was ignored during partial database import");
}


static void PermanentClueSingleMap()
{
    CObj<NDb::CDBScenarioZone> record = new NDb::CDBScenarioZone;
    record->templatesIDs = {710, 0, 0, 0};
    record->nItemSlots = 1;
    record->nPersonSlots = 0;
    record->nCluesMaxNumber = 1;
    CObj<NScenario::CScenarioZone> zone = new NScenario::CScenarioZone(record, 1);
    unordered_map<int, NScenario::CScenarioZone::STemplate> slots;
    slots[710].nItemSlots = slots[710].nEmptyItemSlots = 1;
    CMemoryStream state;
    { CStructureSaver writer(state, CStructureSaver::WRITE); writer.Add(13, &slots); }
    state.Seek(0);
    { CStructureSaver reader(state, CStructureSaver::READ); static_cast<CObjectBase*>(zone.GetPtr())->operator&(reader); }
    CObj<NDb::CDBScenarioClue> ordinaryRecord = new NDb::CDBScenarioClue;
    ordinaryRecord->clueType = NDb::CT_ITEM;
    ordinaryRecord->bPermanent = false;
    CObj<NDb::CDBScenarioClue> permanentRecord = new NDb::CDBScenarioClue;
    permanentRecord->clueType = NDb::CT_ITEM;
    permanentRecord->bPermanent = true;
    CObj<NScenario::CScenarioClue> ordinary = new NScenario::CScenarioClue(ordinaryRecord, 1);
    CObj<NScenario::CScenarioClue> permanent = new NScenario::CScenarioClue(permanentRecord, 2);
    Require(zone->CanPlaceClue(NDb::CT_ITEM), "fixture has no ordinary clue capacity");
    zone->PlaceClue(ordinary);
    Require(ordinary->GetTemplateID() == 710, "ordinary clue lost its map");
    Require(!zone->CanPlaceClue(NDb::CT_ITEM), "ordinary clue admission exceeded its retail limit");
    zone->PlaceClue(permanent);
    Require(permanent->GetTemplateID() == 710,
        "permanent clue lost its single map after ordinary clue capacity was consumed");
    Require(!zone->CanPlaceClue(NDb::CT_ITEM), "permanent clue changed ordinary clue admission limits");
    Require(zone->GetClues().size() == 2 && permanent->GetParentZones().front() == zone.GetPtr(),
        "permanent clue was detached from its scenario zone");
}

class DeferredClickTarget: public CObjectBase, public NRPG::IAttackable
{
    OBJECT_NOCOPY_METHODS(DeferredClickTarget);
public:
    NWorld::IWorld* expectedWorld = nullptr;
    int calls = 0;
    int ProcessAttack(NWorld::IWorld* world, int user, NRPG::CAttackPortion* attack,
        const CVec3& direction, NDb::CRPGArmor*) override {
        ++calls;
        Require(world == expectedWorld, "deferred ClickOfDeath damage lost its creating world");
        Require(user == 42 && attack->atkType == NRPG::AT_CLICK_OF_DEATH &&
                attack->nDmgMin == 10000000 && attack->nDmgMax == 10000001,
            "deferred ClickOfDeath changed its original damage payload");
        Require(direction == CVec3(0, 1, 0), "deferred ClickOfDeath changed its trajectory");
        return 3;
    }
};

template<class Factory>
static NWorld::IDynamicObject* CreateDeferredClick(Factory factory, NWorld::IWorld* world,
    NWorld::CActionCounter* action, CObjectBase* target, const CRay& ray)
{
    // Exercise the pre-port factory too, before it accepts the missing world.
    if constexpr (std::is_invocable_v<Factory, NWorld::IWorld*, NWorld::CActionCounter*, CObjectBase*, int, const CRay&>)
        return factory(world, action, target, 42, ray);
    else
        return factory(action, target, 42, ray);
}

static void DeferredClickWorld()
{
    CObj<NWorld::CWorld> world = new NWorld::CWorld;
    CObj<DeferredClickTarget> target = new DeferredClickTarget;
    target->expectedWorld = world;
    CRay ray;
    ray.ptOrigin = CVec3(1, 2, 3);
    ray.ptDir = CVec3(0, 1, 0);
    CObj<NWorld::IDynamicObject> click = CreateDeferredClick(&NWorld::CreateClickOfDeath,
        world, world->GetActiveCounter(), target, ray);
    Require(world->IsAction(), "deferred ClickOfDeath did not hold its active action counter");
    Require(click->Segment() && target->calls == 1, "deferred ClickOfDeath did not complete one target callback");
    click = nullptr;
    Require(!world->IsAction(), "completed ClickOfDeath retained its action counter");
    click = CreateDeferredClick(&NWorld::CreateClickOfDeath, world, world->GetActiveCounter(), nullptr, ray);
    Require(click->Segment() && target->calls == 1, "targetless ClickOfDeath invoked a damage callback");
    click = nullptr;
    Require(!world->IsAction(), "targetless ClickOfDeath retained its action counter");
}

static void InventoryWithoutUniform()
{
    CObj<NRPG::CUnit> owner = new NRPG::CUnit;
    CObj<NRPG::IInventory> inventory = NRPG::CreateInventory(owner);
    CObj<NDb::CRPGUniform> uniform = new NDb::CRPGUniform;
    for (auto& slot: uniform->subTypes)
        for (auto& priority: slot.priorities) priority = NDb::SUBTYPE_NONE;
    uniform->subTypes[1].priorities[1] = NDb::SUBTYPE_KNIFE;
    uniform->subTypes[2].priorities[0] = NDb::SUBTYPE_KNIFE;
    owner->pUniform = uniform;
    Require(inventory->GetPlaceBySubType(NDb::SUBTYPE_KNIFE) == 2,
        "uniform placement control did not preserve preferred-subtype-only matching");
    owner->pUniform = nullptr;
    Require(inventory->GetUniform() == nullptr, "uniform-less fixture unexpectedly retained a uniform");
    Require(inventory->GetPlaceBySubType(NDb::SUBTYPE_KNIFE) == -1,
        "uniform-less NPC inventory invented a belt placement");
    CObj<NRPG::IInventory> ownerless = NRPG::CreateInventory(nullptr);
    Require(ownerless->GetPlaceBySubType(NDb::SUBTYPE_HEAVY) == -1,
        "ownerless inventory invented a belt placement");
    owner->pUniform = uniform;
    Require(inventory->GetPlaceBySubType(NDb::SUBTYPE_KNIFE) == 2,
        "restoring a uniform did not restore its original belt placement");
}

static void DynamicAmbientFallback()
{
    Require(S2Platform::Window() == nullptr, "ambient fallback fixture unexpectedly owns a display window");
    // The real mode setter takes its documented failure fallback without a
    // platform window; no GPU or display is initialized by this test.
    NGlobal::SetVar("gfx_fullscreen", 0);
    Require(!NGScene::SetModeFromConfig(false) && NGScene::GetCLSkyTexturesNumber() == 0,
        "ambient fixture did not enter the real no-sky-texture fallback");
    Require(S2Platform::Window() == nullptr, "ambient fallback fixture created a display window");
    CObj<NGScene::LightingSceneFixture> scene = new NGScene::LightingSceneFixture;
    NGScene::CLightState light;
    light.vAmbientColor = CVec3(0.3f, 0.4f, 0.5f);
    light.vUpDifColor = CVec3(0.1f, 0.2f, 0.3f);
    light.skyDirections.emplace_back(0, 0, -1);
    NGScene::SDynamicAmbientInfo result;
    light.TraceDynamicLM(&result, SSphere(VNULL3, 0.1f), scene);
    Require(fabs2(result.vXPos.v - light.vAmbientColor) < 1e-10f
            && fabs2(result.vXNeg.v - light.vAmbientColor) < 1e-10f
            && fabs2(result.vYPos.v - light.vAmbientColor) < 1e-10f
            && fabs2(result.vYNeg.v - light.vAmbientColor) < 1e-10f
            && fabs2(result.vZPos.v - (light.vAmbientColor + light.vUpDifColor)) < 1e-10f
            && fabs2(result.vZNeg.v - (light.vAmbientColor - light.vUpDifColor)) < 1e-10f,
        "dynamic objects lost the full ambient fallback when sky textures were unavailable");
    Require(scene->traceCalls == 0, "no-sky-texture fallback still traced unavailable sky occlusion");
    light.skyDirections.clear();
    light.TraceDynamicLM(&result, SSphere(VNULL3, 0.1f), scene);
    Require(fabs2(result.vXPos.v) + fabs2(result.vXNeg.v) + fabs2(result.vYPos.v)
            + fabs2(result.vYNeg.v) + fabs2(result.vZPos.v) + fabs2(result.vZNeg.v) == 0,
        "ambient fallback injected sky light into a cache without sky samples");
}

static void PointLightWithoutScene()
{
    NGScene::CLightState light;
    light.points.emplace_back(CVec3(1, 2, 3), VNULL3, 12.f);
    NGScene::SDynamicAmbientInfo result;
    light.TraceDynamicLM(&result, SSphere(CVec3(1, 0, 0), 0.1f), nullptr);
    // Original falloff at distance 1 with radius 12 is 1/(1+6/12)^2.
    Require(fabs2(result.vXNeg.v - CVec3(4.f/9.f, 8.f/9.f, 12.f/9.f)) < 1e-10f,
        "unshadowed point light changed its original colour or distance attenuation");
    Require(fabs2(result.vXPos.v) + fabs2(result.vYPos.v) + fabs2(result.vYNeg.v)
            + fabs2(result.vZPos.v) + fabs2(result.vZNeg.v) == 0,
        "unshadowed point light illuminated the wrong receiver direction");
    light.TraceDynamicLM(&result, SSphere(CVec3(20, 0, 0), 0.1f), nullptr);
    Require(fabs2(result.vXNeg.v) == 0, "out-of-radius point light contributed to the receiver");
    light.points.clear();
    light.semiPoints.emplace_back(CVec3(1, 2, 3), VNULL3, CVec3(1, 0, 0), 12.f);
    light.TraceDynamicLM(&result, SSphere(CVec3(-1, 0, 0), 0.1f), nullptr);
    Require(fabs2(result.vXPos.v) == 0, "semi-point light illuminated its excluded hemisphere");
    light.TraceDynamicLM(&result, SSphere(CVec3(1, 0, 0), 0.1f), nullptr);
    Require(fabs2(result.vXNeg.v - CVec3(4.f/9.f, 8.f/9.f, 12.f/9.f)) < 1e-10f,
        "unshadowed semi-point light lost its allowed hemisphere");
}

struct RetailPointLightFixture
{
    float color[3], center[3], radius;
    bool shadow;
    unsigned char padding[3];
};
static_assert(sizeof(RetailPointLightFixture) == 32, "retail cached point-light record size");

static void PointLightSaveLayout(bool legacy = false)
{
    vector<RetailPointLightFixture> original(2);
    std::memset(original.data(), 0, original.size() * sizeof(original[0]));
    original[0].color[0] = 0.25f; original[0].color[1] = 0.5f; original[0].color[2] = 0.75f;
    original[0].center[0] = -3.f; original[0].center[1] = 7.f; original[0].center[2] = 1.f;
    original[0].radius = 12.f; original[0].shadow = false;
    original[1].color[0] = 1.f; original[1].color[1] = 0.125f; original[1].color[2] = 0.625f;
    original[1].center[0] = 9.f; original[1].center[1] = -8.f; original[1].center[2] = 2.f;
    original[1].radius = 19.f; original[1].shadow = true;
    CMemoryStream saved;
    if (legacy) {
        struct OldPoint { float values[7]; };
        vector<OldPoint> old(2);
        for (int i = 0; i < 2; ++i) std::memcpy(old[i].values, &original[i], 28);
        { CStructureSaver writer(saved, CStructureSaver::WRITE); writer.Add(4, &old); }
        original[0].shadow = original[1].shadow = true; // Previous remaster always traced shadows.
    } else {
        CStructureSaver writer(saved, CStructureSaver::WRITE); writer.Add(4, &original);
    }
    saved.Seek(0);
    NGScene::CLightState light;
    light.vAmbientColor = light.vUpDifColor = VNULL3;
    { CStructureSaver reader(saved, CStructureSaver::READ); light.operator&(reader); }
    Require(light.points.size() == 2, "retail save lost cached point-light records");
    Require(light.points[1].vCenter == CVec3(9, -8, 2) && light.points[1].fRadius == 19.f,
        "retail 32-byte point-light records were loaded with the wrong array stride");
    CMemoryStream resaved;
    { CStructureSaver writer(resaved, CStructureSaver::WRITE); light.operator&(writer); }
    resaved.Seek(0);
    vector<RetailPointLightFixture> roundtrip;
    { CStructureSaver reader(resaved, CStructureSaver::READ); reader.Add(4, &roundtrip); }
    Require(roundtrip.size() == original.size(), "point-light resave changed the record count");
    for (int i = 0; i < 2; ++i) {
        Require(roundtrip[i].shadow == original[i].shadow && roundtrip[i].radius == original[i].radius,
            "point-light resave lost the original shadow flag or radius");
        for (int axis = 0; axis < 3; ++axis)
            Require(roundtrip[i].center[axis] == original[i].center[axis] && roundtrip[i].color[axis] == original[i].color[axis],
                "point-light resave changed saved position or colour");
    }
}

static void PointLightShadowFlag()
{
    CObj<NGScene::LightingSceneFixture> scene = new NGScene::LightingSceneFixture;
    NGScene::CLightState light;
    light.points.emplace_back(CVec3(1, 2, 3), VNULL3, 12.f, false);
    const SSphere receiver(CVec3(1, 0, 0), 0.1f);
    NGScene::SDynamicAmbientInfo result;
    light.TraceDynamicLM(&result, receiver, scene);
    Require(scene->traceCalls == 0 && result.vXNeg.v.x > 0.f,
        "point light with shadows disabled still performed an occlusion test");
    light.points[0].bCastShadow = true;
    light.TraceDynamicLM(&result, receiver, scene);
    Require(scene->traceCalls == 1 && fabs2(result.vXNeg.v) == 0,
        "shadow-casting point light ignored an intervening occluder");
    scene->blocked = false;
    light.TraceDynamicLM(&result, receiver, scene);
    Require(scene->traceCalls == 2 && fabs2(result.vXNeg.v - CVec3(4.f/9.f, 8.f/9.f, 12.f/9.f)) < 1e-10f,
        "visible shadow-casting point light changed its original attenuation");
}

static void ConsoleVisibilityToggle()
{
    CObj<NUI::CInterface> ui = new NUI::CInterface;
    CObj<NUI::CConsole> console = new NUI::CConsole(NUI::SWindowInfo(nullptr,
        NUI::SPoint(0, 0), NUI::SPoint(1024, 300), "console", NUI::STYLE_ENABLED));
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[console.GetPtr()] = 1; references.refs[1] = console.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CMemoryStream controls;
    { CStructureSaver writer(controls, CStructureSaver::WRITE, &references); writer.Add(11, &console); }
    controls.Seek(0);
    { CStructureSaver reader(controls, CStructureSaver::READ, &references); static_cast<CObjectBase*>(ui.GetPtr())->operator&(reader); }
    auto toggle = [&] {
        NInput::PostEvent("console");
        NInput::SEvent event;
        Require(NInput::GetEvent(&event), "console fixture did not receive its posted toggle");
        Require(ui->ProcessEvent(event), "console toggle was not consumed by its interface");
    };
    toggle();
    Require(!console->GetStyle(NUI::STYLE_VISIBLE), "locked console became visible without unlocking");
    NGlobal::ProcessCommand(L"wirbelwind");
    toggle();
    Require(console->GetStyle(NUI::STYLE_VISIBLE), "unlocked console did not open");
    toggle();
    Require(!console->GetStyle(NUI::STYLE_VISIBLE),
        "console close retained visible sliding state until a later render");
    toggle();
    Require(console->GetStyle(NUI::STYLE_VISIBLE) && console->GetPosition().y == 0,
        "reopened console remained hidden or inherited an offscreen sliding position");
}

class AnimatedLightRenderReached: public std::exception {};

class AnimatedLightMaterial: public NGScene::IMaterial
{
    OBJECT_NOCOPY_METHODS(AnimatedLightMaterial);
public:
    bool DoesCastShadow() const override { return false; }
    const NGScene::SMaterialInfo& GetMaterialInfo() override { throw AnimatedLightRenderReached(); }
};

class FixedAnimatedLight: public CPtrFuncBase<NGScene::CAnimLight>
{
    OBJECT_NOCOPY_METHODS(FixedAnimatedLight);
    bool NeedUpdate() override { return false; }
    void Recalc() override {}
public:
    FixedAnimatedLight() { pValue = new NGScene::CAnimLight; }
    NGScene::CAnimLight* Light() { return pValue; }
};

static void WindowInterfacePropagation()
{
    CObj<NUI::CInterface> first = new NUI::CInterface, second = new NUI::CInterface;
    CObj<NUI::CWindow> parent = new NUI::CWindow(NUI::SWindowInfo());
    parent->SetInterface(first);
    CObj<NUI::CWindow> child = new NUI::CWindow(NUI::SWindowInfo(parent, NUI::SPoint(0, 0), NUI::SPoint(20, 20), "child"));
    CObj<NUI::CWindow> grandchild = new NUI::CWindow(NUI::SWindowInfo(child, NUI::SPoint(0, 0), NUI::SPoint(10, 10), "grandchild"));
    CObj<NUI::CWindow> unrelated = new NUI::CWindow(NUI::SWindowInfo());
    unrelated->SetInterface(first);
    Require(parent->GetInterface() == first.GetPtr() && child->GetInterface() == first.GetPtr()
            && grandchild->GetInterface() == first.GetPtr(), "window-interface fixture did not inherit its initial context");
    parent->SetInterface(second);
    Require(child->GetInterface() == second.GetPtr() && grandchild->GetInterface() == second.GetPtr(),
        "changing a window's interface left its descendants attached to the old context");
    Require(unrelated->GetInterface() == first.GetPtr() && child->GetParent() == parent.GetPtr()
            && grandchild->GetParent() == child.GetPtr(), "interface propagation changed another branch or its parent links");
    parent->SetInterface(nullptr);
    Require(!parent->GetInterface() && !child->GetInterface() && !grandchild->GetInterface(),
        "clearing a window's interface left stale descendant contexts");
}

static void WindowChildRegistration()
{
    CObj<NUI::CInterface> context = new NUI::CInterface;
    CObj<NUI::CWindow> parent = new NUI::CWindow(NUI::SWindowInfo());
    parent->SetInterface(context);
    CMObj<NUI::CWindow> child = new NUI::CWindow(NUI::SWindowInfo(nullptr,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "registered-child"));
    CObj<NUI::CWindow> grandchild = new NUI::CWindow(NUI::SWindowInfo(child,
        NUI::SPoint(0, 0), NUI::SPoint(10, 10), "registered-grandchild"));
    Require(!child->GetParent() && grandchild->GetParent() == child.GetPtr()
            && !grandchild->GetInterface(), "child-registration fixture is not a detached window tree");
    parent->AddChild(child);
    Require(child->GetParent() == parent.GetPtr(), "AddChild retained a window without installing its parent link");
    Require(child->GetInterface() == context.GetPtr() && grandchild->GetInterface() == context.GetPtr(),
        "AddChild failed to install the parent's interface throughout its new subtree");
    Require(parent->GetChildByID("registered-child") == child.GetPtr()
            && grandchild->GetParent() == child.GetPtr(), "AddChild changed the registered subtree's identity or nested parent");
    CPtr<NUI::CWindow> retained = child.GetPtr();
    child = nullptr;
    Require(IsValid(retained) && grandchild->GetParent() == retained.GetPtr(),
        "the parent did not retain ownership of its registered child tree");
}

static CMemoryStream BorrowedObjectFields(CObjectBase* object, CStructureNetworkContext* references)
{
    vector<CObjectBase*> referenced;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* child) { referenced.push_back(child); return false; };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph); object->operator&(writer); }
    references->externalReferences = true;
    references->nextID = 100;
    references->include = [](CObjectBase*) { return false; };
    for (auto* child : referenced) {
        if (references->ids.count(child)) continue;
        const auto id = references->nextID++;
        references->ids[child] = id; references->refs[id] = child;
    }
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, references); object->operator&(writer); }
    saved.Seek(0);
    return saved;
}


class KeyboardEdgeEdit: public NUI::CEdit
{
    OBJECT_NOCOPY_METHODS(KeyboardEdgeEdit);
public:
    vector<int> edges;
    KeyboardEdgeEdit() {}
    explicit KeyboardEdgeEdit(NUI::CInterface* owner): NUI::CEdit(NUI::SWindowInfo(owner,NUI::SPoint(5,5),NUI::SPoint(300,40),
        "keyboard-edit",NUI::STYLE_VISIBLE | NUI::STYLE_ENABLED)) {}
    bool ProcessMessage(const NUI::SEvent& event) override {
        if (event.nEvent == NUI::EVENT_CHAR) edges.push_back(event.nVal);
        return NUI::CEdit::ProcessMessage(event);
    }
};

static void EditKeyboardRouting(bool escape)
{
    S2Platform::SetErrorDialogs(false);
    struct Cleanup {
        ~Cleanup() { NMainLoop::DoneInterface(); NInput::DoneInput(); S2Platform::Done(); }
    } cleanup;
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "edit-keyboard fixture started with another interface or pending command");
    Require(S2Platform::Init("edit keyboard routing regression",640,480,true) && NInput::InitInput(true),
        "edit-keyboard fixture could not start its hidden SDL input window");
    Require(std::strcmp(SDL_GetCurrentVideoDriver(),"dummy") == 0,
        "edit-keyboard regression requires the dummy backend to avoid changing desktop input");
    SDL_Event focus{}; focus.type = SDL_EVENT_WINDOW_FOCUS_GAINED;
    focus.window.windowID = SDL_GetWindowID(S2Platform::Window());
    Require(SDL_PushEvent(&focus),"edit-keyboard fixture could not queue window focus");
    auto pump = [&]() { S2Platform::Delay(2); S2Platform::PumpEvents(); NInput::PumpMessages(S2Platform::Active()); };
    pump();
    NInput::SEvent event;
    while (NInput::GetEvent(&event)) {}
    CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false);
    CObj<NUI::CInterface> interface = new NUI::CInterface(cursor);
    auto* edit = new KeyboardEdgeEdit(interface);
    edit->SetText(L"Save name"); edit->SetCursorPosition(9);
    edit->ProcessMessage(NUI::SEvent(NUI::EVENT_ACTIVATE,NUI::EAF_ACTIVATE));
    Require(edit->IsActive(),"edit-keyboard fixture did not focus its native edit control");
    auto key = [&](SDL_Scancode scan,bool down,bool repeat = false) {
        SDL_Event input{}; input.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
        input.key.windowID = SDL_GetWindowID(S2Platform::Window());
        input.key.scancode = scan; input.key.down = down; input.key.repeat = repeat;
        Require(SDL_PushEvent(&input),"edit-keyboard fixture could not queue a native SDL key");
    };
    if (escape) {
        NInput::SetSection("menu");
        NGlobal::ProcessCommand(L"bind 'cancel' 'ESC'"); NInput::UpdateBinds();
        CObj<NMainLoop::IInterfaceBase> modal = dynamic_cast<NMainLoop::IInterfaceBase*>(pSSClasses->CreateObject(int(0xB1014130)));
        Require(IsValid(modal),"edit-keyboard fixture could not create the native save/load modal");
        CStructureNetworkContext refs; refs.externalReferences = true; refs.include = [](CObjectBase*) { return false; };
        refs.ids[cursor] = 100; refs.refs[100] = cursor; refs.ids[interface] = 101; refs.refs[101] = interface;
        CMemoryStream saved;
        { CStructureSaver writer(saved,CStructureSaver::WRITE,&refs);
          bool allowSave = true; auto type = NGame::SAVE;
          writer.Add(2,&allowSave); writer.Add(3,&type); writer.Add(4,&cursor); writer.Add(5,&interface); }
        saved.Seek(0);
        { CStructureSaver reader(saved,CStructureSaver::READ,&refs); modal->operator&(reader); }
        class OpenModal: public NMainLoop::CInterfaceCommand {
            OBJECT_NOCOPY_METHODS(OpenModal);
        public:
            CPtr<NMainLoop::IInterfaceBase> modal;
            void Exec() override { PushInterface(modal); }
        };
        CObj<OpenModal> open = new OpenModal; open->modal = modal; open->Exec();
        key(SDL_SCANCODE_ESCAPE,true); key(SDL_SCANCODE_ESCAPE,false); pump();
        int physical = 0, virtualKeys = 0;
        while (NInput::GetEvent(&event)) {
            if (event.mMessage.cType == NInput::CT_KEY) ++physical;
            if (event.mMessage.cType == NInput::CT_WIN_KEY) ++virtualKeys;
            modal->ProcessEvent(event);
        }
        Require(physical == 2 && virtualKeys == 1,"edit-keyboard fixture did not deliver the actual SDL Escape down/up and virtual key");
        Require(NMainLoop::HaveInterfaceCommand(),
            "the focused native edit swallowed Escape before the save/load modal's cancel binding");
        Require(edit->edges.empty() && edit->GetText() == L"Save name",
            "Escape entered the edit's discardable-key stream or changed its text");
        Require(!NMainLoop::StepApp(false,false,false) && !NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
            "the native Escape cancellation did not close the restored save/load modal exactly once");
    } else {
        key(SDL_SCANCODE_A,true); key(SDL_SCANCODE_A,false);
        key(SDL_SCANCODE_LANG3,true); key(SDL_SCANCODE_LANG3,false);
        SDL_Event text{}; text.type = SDL_EVENT_TEXT_INPUT;
        text.text.windowID = SDL_GetWindowID(S2Platform::Window()); text.text.text = "Z\xD0\x96";
        Require(SDL_PushEvent(&text),"edit-keyboard fixture could not queue native Unicode text");
        pump();
        while (NInput::GetEvent(&event)) interface->ProcessEvent(event);
        Require(edit->edges == vector<int>{'A',0,0,0},
            "the native edit lost physical key releases or keyboard edges without virtual-key translations");
        Require(edit->GetText() == L"Save nameZ\u0416",
            "routing physical keyboard edges changed the SDL Unicode text-input contract");
        const auto edges = edit->edges;
        NInput::SEvent mouse{}; mouse.mMessage.cType = NInput::CT_KEY; mouse.mMessage.nAction = NInput::GetControlID("MOUSE_BUTTON0"); mouse.mMessage.bState = true;
        interface->ProcessEvent(mouse);
        NInput::SEvent unknown{}; unknown.mMessage.cType = NInput::CT_KEY; unknown.mMessage.nAction = -1; unknown.mMessage.bState = true;
        interface->ProcessEvent(unknown);
        Require(edit->edges == edges,"mouse or unknown controls were routed as discardable keyboard edges");
    }
}

static void NativeCursorPositioning()
{
    S2Platform::SetErrorDialogs(false);
    struct PlatformCleanup { ~PlatformCleanup() { S2Platform::Done(); } } cleanup;
    Require(S2Platform::Init("native cursor position regression",640,480,true),
        "native cursor position fixture could not initialize its hidden SDL window");
    Require(std::strcmp(SDL_GetCurrentVideoDriver(),"dummy") == 0,
        "native cursor position regression requires the dummy backend to avoid moving the desktop pointer");
    S2Platform::UseNativeCursor(true);
    SDL_Event focus{};
    focus.type = SDL_EVENT_WINDOW_FOCUS_GAINED;
    focus.window.windowID = SDL_GetWindowID(S2Platform::Window());
    Require(SDL_PushEvent(&focus), "native cursor position fixture could not queue focus");
    S2Platform::PumpEvents();
    Require(S2Platform::Active(), "native cursor position fixture did not become active");
    SDL_WarpMouseInWindow(S2Platform::Window(),5,7);
    S2Platform::PumpEvents();
    float x = 0, y = 0;
    S2Platform::CursorPosition(&x,&y);
    Require(x == 5 && y == 7, "dummy SDL cursor fixture does not support observable pointer warps");
    CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false,CVec2(50,60));
    S2Platform::PumpEvents();
    S2Platform::CursorPosition(&x,&y);
    Require(x == 50 && y == 60,
        "creating a native cursor at a requested position left SDL at the previous pointer location");
    cursor->Update();
    Require(cursor->GetPos().x == 50 && cursor->GetPos().y == 60,
        "the first native cursor update discarded its requested initial position");
    cursor->SetPos(CVec2(130,140));
    S2Platform::PumpEvents();
    S2Platform::CursorPosition(&x,&y);
    Require(x == 130 && y == 140, "setting a native cursor position did not synchronize SDL");
    cursor->Update();
    Require(cursor->GetPos().x == 130 && cursor->GetPos().y == 140,
        "native pointer polling undid an explicit cursor position change");
    S2Platform::UseNativeCursor(false);
    cursor->SetPos(CVec2(21,31));
    S2Platform::PumpEvents();
    S2Platform::CursorPosition(&x,&y);
    Require(x == 130 && y == 140 && cursor->GetPos().x == 21 && cursor->GetPos().y == 31,
        "logical software cursor positioning unexpectedly warped the SDL pointer");
    S2Platform::UseNativeCursor(true);
    focus.type = SDL_EVENT_WINDOW_FOCUS_LOST;
    Require(SDL_PushEvent(&focus), "native cursor position fixture could not queue focus loss");
    S2Platform::PumpEvents();
    Require(!S2Platform::Active(), "native cursor position fixture retained input focus");
    cursor->SetPos(CVec2(41,51));
    S2Platform::PumpEvents();
    S2Platform::CursorPosition(&x,&y);
    Require(x == 130 && y == 140, "an inactive native cursor warped the SDL pointer");
}

static void DisabledCursorInput()
{
    struct CursorModeRestore {
        bool previous = S2Platform::NativeCursorEnabled();
        ~CursorModeRestore() { S2Platform::UseNativeCursor(previous); }
    } restore;
    S2Platform::UseNativeCursor(true);
    CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false,CVec2(17,23));
    auto setDisableCount = [&](int count) {
        CMemoryStream fields;
        { CStructureSaver writer(fields,CStructureSaver::WRITE); writer.Add(2,&count); }
        fields.Seek(0);
        { CStructureSaver reader(fields,CStructureSaver::READ); static_cast<CObjectBase*>(cursor.GetPtr())->operator&(reader); }
    };
    NInput::SEvent event;
    event.mMessage.hasPointer = true;
    event.mMessage.pointerX = 91;
    event.mMessage.pointerY = 57;
    setDisableCount(2);
    cursor->ProcessEvent(event);
    Require(cursor->GetPos().x == 17 && cursor->GetPos().y == 23,
        "a disabled native cursor consumed pointer input while its serialized nesting counter was positive");
    setDisableCount(1);
    cursor->ProcessEvent(event);
    Require(cursor->GetPos().x == 17 && cursor->GetPos().y == 23,
        "reducing nested cursor disable count enabled input before the last lock was released");
    setDisableCount(0);
    cursor->ProcessEvent(event);
    Require(cursor->GetPos().x == 91 && cursor->GetPos().y == 57,
        "releasing the final cursor lock did not restore native pointer input");
    setDisableCount(-1);
    event.mMessage.pointerX = 45;
    event.mMessage.pointerY = 67;
    cursor->ProcessEvent(event);
    Require(cursor->GetPos().x == 45 && cursor->GetPos().y == 67,
        "cursor input no longer follows the original count-below-one enable rule");
}

template<class Command,class = void>
struct HasExitPermissionArgument: std::false_type {};
template<class Command>
struct HasExitPermissionArgument<Command,std::void_t<decltype(new Command(std::declval<NGScene::CScreenshotTexture*>(),true))>>: std::true_type {};

template<class Command>
static void CheckPermittedExitMenu(NGScene::CScreenshotTexture* texture,bool shortcut)
{
    // Keep the RED fixture compilable with the old one-argument command API.
    constexpr bool hasPermissionArgument = HasExitPermissionArgument<Command>::value;
    Require(hasPermissionArgument, "exit menu command cannot carry an explicit true save permission");
    if constexpr (hasPermissionArgument) {
        NMainLoop::DoneInterface();
        CObj<Command> permitted = new Command(texture,true);
        permitted->Exec();
        auto* active = NMainLoop::GetCurrentInterfaceForHarness();
        Require(IsValid(active), "explicitly permitted exit command did not open its menu");
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(active,&references);
        bool allow = false;
        CPtr<NUI::CWindow> dialog;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(2,&allow); reader.Add(5,&dialog); }
        Require(allow && IsValid(dialog), "exit menu command did not forward its true save permission");
        auto* save = dialog->GetChildByID("save");
        Require(IsValid(save) && save->GetStyle(NUI::STYLE_VISIBLE),
            "an explicitly permitted exit command hid its Save button");
        if (shortcut) {
            NInput::SEvent event;
            NInput::PostEvent("save");
            Require(NInput::GetEvent(&event), "permitted exit save shortcut was not queued");
            active->ProcessEvent(event);
            Require(NMainLoop::HaveInterfaceCommand(), "an explicitly permitted exit command rejected its Save shortcut");
        }
    }
}

class ClueCloseMission: public NGame::CMissionBase
{
    OBJECT_NOCOPY_METHODS(ClueCloseMission);
public:
    vector<int> notifications;
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    void OnGetFocus() override {}
    void OnLostFocus() override {}
    void DoEvent(NWorld::CCommand* command) override {
        CObj<NWorld::CCommand> hold = command;
        auto* event = dynamic_cast<NWorld::CCmdInterfaceEvent*>(command);
        Require(event != nullptr, "clue close delivered an unrelated mission event");
        notifications.push_back(event->nID);
    }
};

static void RestoredClueCloseNotification()
{
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "clue close fixture must begin without an active interface or command");
    struct Cleanup { ~Cleanup() { NMainLoop::StepApp(false,false,false); NMainLoop::DoneInterface(); } } cleanup;
    CObj<ClueCloseMission> mission = new ClueCloseMission;
    auto open = [&](bool withMission) {
        CObj<NMainLoop::IInterfaceBase> modal = dynamic_cast<NMainLoop::IInterfaceBase*>(pSSClasses->CreateObject(int(0xB1001180)));
        Require(IsValid(modal), "clue close fixture could not create its native restored clue dialog");
        CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false);
        CObj<NUI::CInterface> interface = new NUI::CInterface(cursor);
        CPtr<NGame::IMission> sender = withMission ? mission.GetPtr() : nullptr;
        int event = 73;
        CStructureNetworkContext references;
        references.externalReferences = true; references.include = [](CObjectBase*) { return false; }; references.nextID = 100;
        for (auto* object: vector<CObjectBase*>{mission.GetPtr(),cursor.GetPtr(),interface.GetPtr()}) {
            auto id = references.nextID++; references.ids[object] = id; references.refs[id] = object;
        }
        CMemoryStream saved;
        {
            CStructureSaver writer(saved,CStructureSaver::WRITE,&references);
            writer.Add(2,&event); writer.Add(3,&sender); writer.Add(7,&cursor); writer.Add(8,&interface);
        }
        saved.Seek(0);
        { CStructureSaver reader(saved,CStructureSaver::READ,&references); modal->operator&(reader); }
        class InstallClue: public NMainLoop::CInterfaceCommand {
            OBJECT_NOCOPY_METHODS(InstallClue);
        public:
            NMainLoop::IInterfaceBase* owner = nullptr;
            NMainLoop::IInterfaceBase* modal = nullptr;
            void Exec() override { SetInterface(owner); PushInterface(modal); }
        };
        CObj<InstallClue> install = new InstallClue;
        install->owner = mission; install->modal = modal; install->Exec();
        return modal;
    };
    auto send = [&](NMainLoop::IInterfaceBase* modal,const char* name) {
        NInput::SEvent event;
        while (NInput::GetEvent(&event)) {}
        NInput::PostEvent(name);
        Require(NInput::GetEvent(&event), "clue close fixture did not receive its native input command");
        return modal->ProcessEvent(event);
    };
    auto chapterClue = open(false);
    Require(send(chapterClue,"cancel") && NMainLoop::HaveInterfaceCommand() && mission->notifications.empty(),
        "a chapter clue without an originating mission should close without a notification");
    Require(NMainLoop::StepApp(false,false,false) && NMainLoop::GetCurrentInterfaceForHarness() == mission.GetPtr(),
        "the native clue dialog did not leave its actual modal stack");
    NMainLoop::DoneInterface();
    auto missionClue = open(true);
    Require(!send(missionClue,"unrelated_clue_event") && mission->notifications.empty() && !NMainLoop::HaveInterfaceCommand(),
        "an unrelated input command released the mission's clue wait");
    Require(send(missionClue,"cancel") && NMainLoop::HaveInterfaceCommand(), "the restored mission clue did not close");
    Require(mission->notifications == vector<int>{73},
        "closing a restored clue modal never notified the mission of its waiting UI event");
    Require(NMainLoop::StepApp(false,false,false) && NMainLoop::GetCurrentInterfaceForHarness() == mission.GetPtr() &&
        mission->notifications == vector<int>{73}, "closing a clue sent duplicate events or failed to restore the mission");
}

static void ExitMenuConfirmation(const char* databasePath,const char* resourcePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "exit confirmation fixture started with an active interface or command");
    struct InterfaceCleanup { ~InterfaceCleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(2,2);
    for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x) pixels[y][x] = NGfx::SPixel8888(12,23,34,255);
    texture->Set(pixels);
    auto open = [&]() {
        CObj<NGame::CICExitMenu> command = new NGame::CICExitMenu(texture);
        command->Exec();
        Require(NMainLoop::GetInterfaceStackDepth() == 1 && !NMainLoop::HaveInterfaceCommand(),
            "exit confirmation fixture did not open exactly one clean native modal");
    };
    auto send = [&](const char* name) {
        NInput::SEvent event;
        while (NInput::GetEvent(&event)) {}
        NInput::PostEvent(name);
        Require(NInput::GetEvent(&event), "exit confirmation fixture did not receive its native input command");
        return NMainLoop::GetCurrentInterfaceForHarness()->ProcessEvent(event);
    };
    open();
    Require(send("exitgame") && NMainLoop::HaveInterfaceCommand(), "existing exitgame confirmation control did not queue exit");
    Require(!NMainLoop::StepApp(false,false,false) && NMainLoop::GetInterfaceStackDepth() == 1,
        "existing exitgame control did not produce the application exit sentinel");
    NMainLoop::DoneInterface();
    open();
    Require(send("confirm") && NMainLoop::HaveInterfaceCommand(),
        "the native exit dialog ignored its standard confirmation command");
    Require(!NMainLoop::StepApp(false,false,false) && NMainLoop::GetInterfaceStackDepth() == 1,
        "standard exit confirmation did not produce the same application exit sentinel");
}

static vector<unsigned char> SilentPCMFixture(unsigned frames = 1000,unsigned sampleRate = 44100)
{
    vector<unsigned char> wave(44+frames*2,0);
    auto put = [&](std::size_t offset,unsigned value,int bytes) {
        for (int i = 0; i < bytes; ++i) wave[offset+i] = static_cast<unsigned char>(value >> (i*8));
    };
    std::memcpy(wave.data(),"RIFF",4); put(4,36+frames*2,4);
    std::memcpy(wave.data()+8,"WAVEfmt ",8); put(16,16,4);
    put(20,1,2); put(22,1,2); put(24,sampleRate,4); put(28,sampleRate*2,4);
    put(32,2,2); put(34,16,2); std::memcpy(wave.data()+36,"data",4); put(40,frames*2,4);
    return wave;
}

struct TemporaryAudioFile
{
    std::filesystem::path path;
    explicit TemporaryAudioFile(const char* filename = "native-music-fixture.wav")
    {
        const char* directory = std::getenv("S2_USER_DATA_DIR");
        Require(directory != nullptr, "native music fixture needs an isolated temporary directory");
        path = std::filesystem::path(directory)/filename;
        std::filesystem::create_directories(path.parent_path());
        const auto wave = SilentPCMFixture(88200);
        std::ofstream output(path,std::ios::binary|std::ios::trunc);
        output.write(reinterpret_cast<const char*>(wave.data()),static_cast<std::streamsize>(wave.size()));
        Require(output.good(), "native music fixture could not write its temporary PCM file");
    }
    ~TemporaryAudioFile() { std::error_code error; std::filesystem::remove(path,error); }
};

static void ReloadedMusicRecordContinuity()
{
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr, "music-record fixture requires headless audio");
    TemporaryAudioFile wave, otherWave("native-music-other.wav");
    Require(NFMSound::Init(NFMSound::SStartInfo()), "music-record fixture could not initialize native audio");
    struct AudioCleanup { ~AudioCleanup() { NFMSound::Done(); } } cleanup;
    CObj<NFMSound::CStream> stream = NFMSound::PlayStream(wave.path.string().c_str(),false,250,true,0);
    Require(IsValid(stream) && NFMSound::IsPlaying(stream) && NFMSound::GetStreamTime(stream) == 250,
        "music-record control did not start and seek an actual decoded stream");
    CObj<CCTime> clock = new CCTime(0);
    vector<CObj<NDb::CTMusic>> pools;
    auto track = [](const string& path) {
        CObj<NDb::CMusic> record = new NDb::CMusic;
        record->szFileName = path; record->nFadeIn = 0; record->nPlayTime = 60000; record->nRndPlayTime = 0;
        return record;
    };
    auto scene = [&](NDb::CMusic* record) {
        CObj<NDb::CTMusic> pool = new NDb::CTMusic;
        pool->variants.push_back(record); pool->roulette.AddSector(1); pools.push_back(pool);
        CObj<NSound::ISoundScene> result = NSound::CreateSoundScene(pool,nullptr,clock);
        return result;
    };
    auto original = track(wave.path.string());
    auto owner = scene(original);
    CTransformStack transform; transform.Init();
    owner->Draw(&transform);
    Require(NFMSound::IsPlaying(stream) && NFMSound::GetStreamTime(stream) == 250,
        "the original sound scene did not adopt its already-playing track");
    original = nullptr;
    Require(!IsValid(pools.front()->variants.front()) && NFMSound::IsPlaying(stream),
        "music-record fixture failed to invalidate the old database record independently of the live stream");
    auto replacement = track(wave.path.string());
    auto resumed = scene(replacement);
    resumed->Draw(&transform);
    Require(NFMSound::IsPlaying(stream) && NFMSound::GetStreamTime(stream) == 250 &&
        NFMSound::PlayStream(wave.path.string().c_str(),true,0,true,0) == stream.GetPtr(),
        "replacing a music database record restarted its already-playing file instead of adopting the live stream");
    auto different = track(otherWave.path.string());
    auto switched = scene(different);
    switched->Draw(&transform);
    CObj<NFMSound::CStream> current = NFMSound::PlayStream(otherWave.path.string().c_str(),true,0,true,0);
    Require(!NFMSound::IsPlaying(stream) && IsValid(current) && current.GetPtr() != stream.GetPtr() &&
        NFMSound::IsPlaying(current) && NFMSound::GetStreamTime(current) == 0,
        "preserving a same-file stream prevented an actual different music file from replacing it");
}

static void TracklessSceneMusicContinuity()
{
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr, "music continuity fixture requires headless audio");
    TemporaryAudioFile wave;
    CObj<NDb::CMusic> track = new NDb::CMusic;
    track->szFileName = wave.path.string();
    track->nFadeIn = 0; track->nRndPlayTime = 0; track->nPlayTime = 60000;
    CObj<NDb::CTMusic> pool = new NDb::CTMusic;
    pool->variants.push_back(track.GetPtr()); pool->roulette.AddSector(1);
    CObj<CCTime> clock = new CCTime(0);
    Require(NFMSound::Init(NFMSound::SStartInfo()), "music continuity fixture could not initialize audio");
    struct AudioCleanup { ~AudioCleanup() { NFMSound::Done(); } } cleanup;
    CObj<NSound::ISoundScene> owner = NSound::CreateSoundScene(pool,nullptr,clock);
    CTransformStack transform; transform.Init();
    owner->Draw(&transform);
    CObj<NFMSound::CStream> stream = NFMSound::PlayStream(track->szFileName.c_str(),true,0,true,0);
    Require(IsValid(stream) && NFMSound::IsPlaying(stream) && NFMSound::GetStreamTime(stream) == 0,
        "music continuity control did not open and retain a genuinely playing decoded stream");
    CObj<NSound::ISoundScene> overlay = NSound::CreateSoundScene(nullptr,nullptr,clock);
    const auto deadline = SDL_GetTicks()+4500;
    do { overlay->Draw(&transform); SDL_Delay(25); } while (SDL_GetTicks() < deadline);
    Require(NFMSound::IsPlaying(stream),
        "a sound scene without its own music faded out another scene's playing track");
    NFMSound::FadeOut(stream,0.05f);
    SDL_Delay(100);
    NFMSound::SListener listener;
    listener.vPosition = CVec3(0,0,0); listener.toProjective = transform.Get().forward;
    NFMSound::Update(listener);
    Require(!NFMSound::IsPlaying(stream), "explicit music fade-out control failed to stop actual playback");
}

static void HeadlessMusicStream()
{
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr, "native music fixture requires headless audio");
    TemporaryAudioFile wave;
    Require(NFMSound::Init(NFMSound::SStartInfo()), "native music fixture could not initialize audio");
    struct AudioCleanup { ~AudioCleanup() { NFMSound::Done(); } } cleanup;
    CObj<NFMSound::CStream> stream = NFMSound::PlayStream(wave.path.string().c_str(),false,250,true,0);
    Require(IsValid(stream) && NFMSound::IsPlaying(stream),
        "headless native music still required a physical audio device to open a valid PCM stream");
    Require(NFMSound::GetStreamTime(stream) == 250,
        "headless native music did not retain its exact initial PCM cursor");
    SDL_Delay(100);
    Require(NFMSound::IsPlaying(stream) && NFMSound::GetStreamTime(stream) == 250,
        "headless native music advanced on a physical device instead of waiting for explicit rendering");
}

static void LateLoopingSoundOffset()
{
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr, "late-loop fixture requires headless native audio");
    NFMSound::SStartInfo info;
    info.nMaxChannels = 4;
    Require(NFMSound::Init(info), "late-loop fixture could not initialize the native audio engine");
    struct AudioCleanup { ~AudioCleanup() { NFMSound::Done(); } } cleanup;
    const auto wave = SilentPCMFixture();
    CObj<NFMSound::CSample3D> sample = NFMSound::LoadSample3D(wave.data(),static_cast<int>(wave.size()),1.f,100.f,0);
    Require(IsValid(sample), "late-loop fixture could not load its real PCM sample");
    auto activeChannel = [&]() {
        int channel = -1;
        for (int i = 0; i < info.nMaxChannels; ++i) if (FSOUND_IsPlaying(i)) {
            Require(channel == -1, "late-loop fixture unexpectedly has multiple live channels");
            channel = i;
        }
        return channel;
    };
    NFMSound::SPlayParams params;
    params.pSample = sample.GetPtr();
    params.nStartMs = 10;
    {
        CObj<NFMSound::CSound3D> control = NFMSound::Play3DSound(params);
        const int channel = activeChannel();
        Require(IsValid(control) && NFMSound::IsPlaying(control) && channel >= 0 && FSOUND_GetFrequency(channel) == 44100 &&
            FSOUND_GetCurrentPosition(channel) == 441,
            "non-looping late-start control did not seek the actual decoded channel");
    }
    Require(activeChannel() == -1, "releasing the control sound did not stop its native channel");
    params.bLoop = true;
    for (int start: {35,1000001}) {
        params.nStartMs = start;
        CObj<NFMSound::CSound3D> loop = NFMSound::Play3DSound(params);
        const int channel = activeChannel();
        Require(IsValid(loop) && NFMSound::IsPlaying(loop) && channel >= 0,
            "late-loop fixture did not start an actual looping channel");
        const unsigned expected = static_cast<unsigned>((std::int64_t(44100)*start/1000)%1000);
        Require(FSOUND_GetCurrentPosition(channel) == expected,
            "a late looping native sound restarted at frame zero instead of wrapping its elapsed seek offset");
    }
    Require(activeChannel() == -1, "loop test retained a native playback channel");
    params.bLoop = false;
    params.nStartMs = 35;
    CObj<NFMSound::CSound3D> expired = NFMSound::Play3DSound(params);
    Require(!NFMSound::IsPlaying(expired) && activeChannel() == -1,
        "an expired non-looping event restarted while correcting looping offsets");
}

static void FailedSoundChannelLifetime()
{
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr, "failed-channel fixture requires headless native audio");
    NFMSound::SStartInfo info;
    info.nMaxChannels = 1;
    Require(NFMSound::Init(info), "failed-channel fixture could not initialize native audio");
    struct AudioCleanup { ~AudioCleanup() { NFMSound::Done(); } } cleanup;
    const auto wave = SilentPCMFixture();
    auto sample = [&]() { return NFMSound::LoadSample3D(wave.data(),static_cast<int>(wave.size()),1.f,100.f,0); };
    CPtr<NFMSound::CSample3D> expiredSample = sample();
    Require(IsValid(expiredSample), "failed-channel fixture could not decode its expired sample");
    NFMSound::SPlayParams params;
    params.pSample = expiredSample.GetPtr(); params.nStartMs = 35;
    {
        // Only observe the returned handle: no caller owns an already-expired sound.
        CPtr<NFMSound::CSound3D> expired = NFMSound::Play3DSound(params);
        Require(IsValid(expired) && !NFMSound::IsPlaying(expired), "expired-channel control unexpectedly played");
    }
    Require(!IsValid(expiredSample),
        "the channel table retained an expired sound and its decoded sample under the invalid channel ID");
    CObj<NFMSound::CSample3D> liveSample = sample();
    params.pSample = liveSample.GetPtr(); params.nStartMs = 0; params.bLoop = true;
    CObj<NFMSound::CSound3D> live = NFMSound::Play3DSound(params);
    Require(IsValid(live) && NFMSound::IsPlaying(live), "failed-channel fixture did not fill its single native voice");
    CPtr<NFMSound::CSample3D> rejectedSample = sample();
    params.pSample = rejectedSample.GetPtr();
    {
        CPtr<NFMSound::CSound3D> rejected = NFMSound::Play3DSound(params);
        Require(IsValid(rejected) && !NFMSound::IsPlaying(rejected) && NFMSound::IsPlaying(live),
            "a saturated mixer stole the existing voice or played the rejected sound");
    }
    Require(!IsValid(rejectedSample), "the channel table retained a rejected voice's decoded sample");
    Require(NFMSound::IsPlaying(live), "releasing a failed channel interrupted an unrelated live sound");
}

class HotseatVisibilityWorld: public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(HotseatVisibilityWorld);
public:
    using NWorld::CWorld::RegisterPlayer;
    using NWorld::CWorld::UnregisterPlayer;
};

static void HotseatOpponentVisibility()
{
    CObj<HotseatVisibilityWorld> world = new HotseatVisibilityWorld;
    CObj<NRPG::CGlobalDiplomacy> diplomacy = NRPG::CreateGlobalDiplomacy();
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.ids[diplomacy.GetPtr()] = 1; references.refs[1] = diplomacy.GetPtr();
    references.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved,CStructureSaver::WRITE,&references); writer.Add(40,&diplomacy); }
    saved.Seek(0);
    { CStructureSaver reader(saved,CStructureSaver::READ,&references); static_cast<CObjectBase*>(world.GetPtr())->operator&(reader); }
    Require(world->GetDiplomacy() == diplomacy.GetPtr(), "hotseat vision fixture did not bind its native diplomacy table");
    auto player = [&](const wchar_t* name,int scenario) {
        CObj<NWorld::CPlayer> result = new NWorld::CPlayer(name,nullptr,nullptr,scenario);
        result->SetCommander(new NAI::CSequenceCommander(world));
        world->RegisterPlayer(result);
        Require(!NAI::IsAIPlayer(result.GetPtr()), "hotseat vision fixture unexpectedly created an AI-controlled player");
        return result;
    };
    auto first = player(L"first hotseat player",0);
    auto second = player(L"second hotseat player",0);
    Require(world->GetDiplomacyState(static_cast<NWorld::IPlayer*>(first.GetPtr()),
        static_cast<NWorld::IPlayer*>(second.GetPtr())) == NDb::DS_ENEMY &&
        diplomacy->GetDiplomacyState(0,0) == NDb::DS_ALLY,
        "hotseat vision control did not reproduce distinct opponents sharing scenario ID zero");
    CObj<NWorld::CUnitServer> firstContact = new NWorld::CUnitServer;
    CObj<NWorld::CUnitServer> secondContact = new NWorld::CUnitServer;
    CObj<NWorld::CWindowDoor> object = new NWorld::CWindowDoor, trap = new NWorld::CWindowDoor;
    auto setVision = [&](NWorld::CPlayer* owner,NWorld::CUnitServer* contact,bool extras) {
        CStructureNetworkContext graph;
        graph.externalReferences = true; graph.nextID = 100;
        vector<CObjectBase*> objects{contact,object.GetPtr(),trap.GetPtr()};
        for (std::size_t i = 0; i < objects.size(); ++i) {
            graph.ids[objects[i]] = unsigned(i+1); graph.refs[unsigned(i+1)] = objects[i];
        }
        graph.include = [](CObjectBase*) { return false; };
        list<CPtr<NWorld::CUnitServer>> visible{contact};
        list<CPtr<CObjectBase>> visibleObjects, traps;
        if (extras) { visibleObjects.push_back(object.GetPtr()); traps.push_back(trap.GetPtr()); }
        CMemoryStream state;
        { CStructureSaver writer(state,CStructureSaver::WRITE,&graph);
          writer.Add(3,&visible); writer.Add(4,&visibleObjects); writer.Add(6,&traps); }
        state.Seek(0);
        { CStructureSaver reader(state,CStructureSaver::READ,&graph);
          static_cast<NWorld::CPlayerBaseVision<NWorld::CUnitServer>*>(owner)->operator&(reader); }
    };
    setVision(first,firstContact,true); setVision(second,secondContact,false);
    Require(first->CanSee(firstContact) && !second->CanSee(firstContact) && second->CanSee(secondContact),
        "hotseat vision fixture did not restore distinct native contact sets");
    world->MergeFriendlyPlayersVisibleSets();
    Require(!second->CanSee(firstContact) && !first->CanSee(secondContact) &&
        !second->CanSeeObject(object) && !second->CanSeeTrap(trap),
        "opposing hotseat players shared contacts, objects or traps merely because both used scenario ID zero");
    Require(first->CanSee(firstContact) && second->CanSee(secondContact) &&
        first->CanSeeObject(object) && first->CanSeeTrap(trap), "preventing opponent vision sharing erased existing sightings");
    world->UnregisterPlayer(second);
    auto ally = player(L"distinct allied side",1);
    const vector<CPtr<NRPG::IUnitMission>> units;
    diplomacy->SetDiplomacyState(0,units,1,NDb::DS_ALLY);
    diplomacy->SetDiplomacyState(1,units,0,NDb::DS_ALLY);
    Require(world->GetDiplomacyState(static_cast<NWorld::IPlayer*>(first.GetPtr()),
        static_cast<NWorld::IPlayer*>(ally.GetPtr())) == NDb::DS_ALLY,
        "allied-visibility control did not establish an actual native alliance");
    setVision(ally,secondContact,false);
    world->MergeFriendlyPlayersVisibleSets();
    Require(first->CanSee(secondContact) && ally->CanSee(firstContact) &&
        ally->CanSeeObject(object) && ally->CanSeeTrap(trap),
        "the opponent-visibility correction also prevented actual allies from sharing sightings");
}

struct MissionSoundClockCaptured {};

class MissionSoundClockRenderer: public NRender::IRenderGame
{
    OBJECT_NOCOPY_METHODS(MissionSoundClockRenderer);
public:
    STime time = 0;
    bool advance = false;
    int calls = 0;
    CObjectBase* Select(CObjectBase*,const CVec4&,bool) override { throw std::runtime_error("unexpected sound-clock selection"); }
    CCTime* GetTime() override { throw std::runtime_error("unexpected sound-clock render timer query"); }
    NLSHead::CHeadsController* GetHeadController() const override { throw std::runtime_error("unexpected sound-clock head query"); }
    void UpdateViewWorld(bool,STime,NWorld::IPlayer*,bool) override { throw std::runtime_error("unexpected sound-clock world update"); }
    void FastUpdate(STime) override { throw std::runtime_error("unexpected sound-clock fast update"); }
    void ResetTiming() override { throw std::runtime_error("unexpected sound-clock reset"); }
    void UpdateSound(bool shouldAdvance,CTransformStack* transform,STime current) override {
        Require(transform != nullptr, "native mission did not supply its camera transform to sound");
        ++calls; time = current; advance = shouldAdvance;
        // Observe the real caller, then stop before any GPU rendering or presentation.
        throw MissionSoundClockCaptured();
    }
};

class MissionSoundClockMission: public NGame::CMissionBase
{
    OBJECT_NOCOPY_METHODS(MissionSoundClockMission);
protected:
    const STime GetTime() override { return 10000; }
public:
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    void Attach(NRender::IRenderGame* renderer,NGScene::IGameView* scene) {
        pRender = renderer; pScene = scene; sTimeCounter.SetCurrent(10000);
    }
    void SetSkippedTime(STime skipped) { nDeltaTime = skipped; }
    void DrawSound(ICamera* camera,bool advance) { RenderFrame(NGame::N_RENDERMODE_3D|8,advance,camera,false); }
};

static void MissionSoundGameClock()
{
    CObj<MissionSoundClockMission> mission = new MissionSoundClockMission;
    CObj<MissionSoundClockRenderer> renderer = new MissionSoundClockRenderer;
    CObj<NGScene::GameViewFixture> scene = new NGScene::GameViewFixture;
    CObj<ICamera> camera = ::CreateCamera(CAMERA_PC);
    Require(IsValid(camera), "sound-clock fixture could not create a real native camera");
    mission->Attach(renderer,scene);
    auto draw = [&](bool advance) {
        bool reached = false;
        try { mission->DrawSound(camera,advance); }
        catch (const MissionSoundClockCaptured&) { reached = true; }
        Require(reached, "native mission frame did not reach its sound update before rendering");
    };
    draw(true);
    Require(renderer->calls == 1 && renderer->advance && renderer->time == 10000,
        "ordinary native mission frame did not pass its baseline sound clock");
    mission->SetSkippedTime(5000);
    Require(mission->GetGameTime() == 15000, "sound-clock fixture did not retain the native skipped-game-time offset");
    draw(true);
    Require(renderer->calls == 2 && renderer->advance && renderer->time == 15000,
        "the native mission passed raw wall time to sound and omitted the fast-forwarded game interval");
    draw(false);
    Require(renderer->calls == 3 && !renderer->advance && renderer->time == 15000,
        "using the game clock lost the native sound pause flag or skipped-time offset");
}

namespace NSound { extern CBasicShare<int,CFileSample3D> share3DSamples; extern CBasicShare<int,CFileSample2D> share2DSamples; }

class CachedEffectSampleAccess: public NSound::CFileSample3D
{
public:
    static auto ValueMember() { return &CachedEffectSampleAccess::pValue; }
};

class EffectSoundSource: public NWorld::IVisObj
{
    OBJECT_NOCOPY_METHODS(EffectSoundSource);
public:
    CPtr<NDb::CSoundEffect> effect;
    CObj<NGScene::CCVec3> position = new NGScene::CCVec3(CVec3(0,0,0));
    STime start;
    EffectSoundSource(): start(0) {}
    EffectSoundSource(NDb::CSoundEffect* record,STime time): effect(record),start(time) {}
    void Visit(NWorld::ISoundVisitor* visitor) override { visitor->AddEffect(start,effect,position,vector<int>()); }
};

static void LegacySoundEffectFields(NDb::CSoundInstance* record,CFuncBase<STime>* time,CFuncBase<CVec3>* position)
{
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.ids[time] = 1; references.refs[1] = time;
    references.ids[position] = 2; references.refs[2] = position;
    references.include = [](CObjectBase*) { return false; };
    STime begin = 12000, last = 11000;
    CDBPtr<NDb::CSoundInstance> data = record;
    CDGPtr<CFuncBase<STime>> clock = time;
    CDGPtr<CFuncBase<CVec3>> placement = position;
    CObj<NFMSound::CSound3D> channel;
    vector<int> flags{21,34};
    CMemoryStream legacy;
    { CStructureSaver writer(legacy,CStructureSaver::WRITE,&references);
      writer.Add(2,&begin); writer.Add(3,&last); writer.Add(4,&data); writer.Add(5,&clock);
      writer.Add(6,&placement); writer.Add(7,&channel); writer.Add(8,&flags); }
    CObj<NSound::CSoundInstance> restored = new NSound::CSoundInstance;
    legacy.Seek(0);
    { CStructureSaver reader(legacy,CStructureSaver::READ,&references);
      static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
    CStructureNetworkContext roundReferences;
    auto saved = BorrowedObjectFields(restored,&roundReferences);
    STime savedBegin = 0, savedLast = 0;
    CDBPtr<NDb::CSoundInstance> savedRecord;
    CDGPtr<CFuncBase<STime>> savedTime;
    CDGPtr<CFuncBase<CVec3>> savedPosition;
    vector<int> savedFlags;
    { CStructureSaver reader(saved,CStructureSaver::READ,&roundReferences);
      reader.Add(2,&savedBegin); reader.Add(3,&savedLast); reader.Add(4,&savedRecord);
      reader.Add(5,&savedTime); reader.Add(6,&savedPosition); reader.Add(8,&savedFlags); }
    Require(savedBegin == begin && savedLast == last && savedRecord.GetPtr() == record &&
        savedTime.Get() == time && savedPosition.Get() == position && savedFlags == flags,
        "sound-effect ageing changed the existing saved field numbers or their original meanings");
}

static void DelayedSoundEffectAge(const char* databasePath,bool infiniteLoop = false)
{
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr, "sound-effect fixture requires headless native audio");
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
    Require(sounds.MoveNext(), "sound-effect fixture has no actual sample record");
    auto* soundRecord = sounds.Get(); soundRecord->bLoop = false;
    NFMSound::SStartInfo info; info.nMaxChannels = 4;
    Require(NFMSound::Init(info), "sound-effect fixture could not initialize audio");
    struct AudioCleanup { ~AudioCleanup() { ClearHoldQueue(); NFMSound::Done(); } } cleanup;
    const auto wave = SilentPCMFixture();
    CObj<NFMSound::CSample3D> sample = NFMSound::LoadSample3D(wave.data(),static_cast<int>(wave.size()),1.f,100.f,0);
    Require(IsValid(sample), "sound-effect fixture could not decode its real PCM sample");
    CObj<NSound::CFileSample3D> loader = NSound::share3DSamples.Get(soundRecord->GetRecordID());
    (loader.GetPtr()->*CachedEffectSampleAccess::ValueMember()) = sample.GetPtr();
    CDGPtr<CPtrFuncBase<NFMSound::CSample3D>> cached = loader.GetPtr();
    cached.Refresh();
    Require(cached->GetValue() == sample.GetPtr(), "sound-effect fixture did not seed the actual native sample cache");
    CObj<NDb::CSoundVariant> variant = new NDb::CSoundVariant;
    variant->pSound = soundRecord;
    CObj<NDb::CTSound> pool = new NDb::CTSound;
    pool->variants.push_back(variant.GetPtr()); pool->roulette.AddSector(1);
    CDBIterator<NDb::CSoundInstance> instances(*NDatabase::GetTable<NDb::CSoundInstance>());
    Require(instances.MoveNext(), "sound-effect fixture has no native instance record for save compatibility");
    auto* instance = instances.Get();
    instance->pSound = pool; instance->nStartTime = 200; instance->nCycleCount = 1;
    instance->bFadeIn = instance->bFadeOut = false; instance->nFadeSamples = 64;
    instance->eSoundType = NDb::ST_PERMANENT; instance->fSoundAvgInterval = 1.f; instance->nVolume = 128;
    CObj<NDb::CSoundEffect> effect = new NDb::CSoundEffect;
    effect->instances.push_back(instance);
    CObj<CSetSyncSrc<NWorld::IVisObj>> source = new CSetSyncSrc<NWorld::IVisObj>;
    CTimeCounter clock; clock.SetCurrent(10000);
    CObj<NSound::ISoundScene> scene = NSound::CreateSoundScene(nullptr,nullptr,clock.GetTime());
    CObj<NRender::IRenderSound> mixer = dynamic_cast<NRender::IRenderSound*>(pSSClasses->CreateObject(int(0x03081142)));
    Require(IsValid(mixer), "sound-effect fixture has no registered native render mixer");
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    const vector<CObjectBase*> objects{source.GetPtr(),scene.GetPtr(),clock.GetTime()};
    for (std::size_t i = 0; i < objects.size(); ++i) {
        references.ids[objects[i]] = unsigned(i+1); references.refs[unsigned(i+1)] = objects[i];
    }
    references.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved,CStructureSaver::WRITE,&references);
      writer.Add(1,&source); writer.Add(2,&scene); writer.Add(3,&clock); }
    saved.Seek(0);
    { CStructureSaver reader(saved,CStructureSaver::READ,&references); static_cast<CObjectBase*>(mixer.GetPtr())->operator&(reader); }
    CObj<ICamera> camera = ::CreateCamera(CAMERA_PC);
    CTransformStack transform; camera->GetTransform(&transform,CVec2(1024,768));
    auto tick = [&](STime time) { MarkNewDGFrame(); mixer->Update(true,&transform,time); };
    auto playingChannel = [&]() {
        int channel = -1;
        // Native handles increase across sequential starts; maxChannels only caps live voices.
        // This isolated fixture performs fewer than 16 starts, with handles beginning at one.
        for (int i = 0; i < 32; ++i) if (FSOUND_IsPlaying(i)) {
            Require(channel == -1, "sound-effect fixture unexpectedly has multiple playing channels"); channel = i;
        }
        return channel;
    };
    if (infiniteLoop) {
        instance->nStartTime = 0; instance->nCycleCount = -1;
        CObj<EffectSoundSource> loop = new EffectSoundSource(effect,10000);
        source->Set(vector<NWorld::IVisObj*>{loop.GetPtr()}); tick(1000);
        const int loopChannel = playingChannel();
        Require(loopChannel >= 0, "infinite-effect fixture did not start an actual native channel");
        Require(FSOUND_GetLoopMode(loopChannel) == FSOUND_LOOP_NORMAL,
            "an effect with infinite cycle count played once because its sample record was not itself looped");
#if defined(S2_UPSTREAM_NATIVE_SFX)
        vector<float> output(4096*2);
        Require(S2FmodRenderForTest(output.data(),4096) != 0, "infinite-effect fixture could not render real native PCM frames");
        Require(FSOUND_IsPlaying(loopChannel) && FSOUND_GetCurrentPosition(loopChannel) > 0 &&
            FSOUND_GetCurrentPosition(loopChannel) < 1000,
            "an infinitely repeated effect stopped instead of wrapping after more than four actual sample lengths");
#endif
        source->Set(vector<NWorld::IVisObj*>()); tick(1000);
        Require(playingChannel() == -1, "removing an infinite effect did not release its actual native channel");
        instance->nCycleCount = 1;
        CObj<EffectSoundSource> once = new EffectSoundSource(effect,10000);
        source->Set(vector<NWorld::IVisObj*>{once.GetPtr()}); tick(1000);
        const int onceChannel = playingChannel();
        Require(onceChannel >= 0 && FSOUND_GetLoopMode(onceChannel) == FSOUND_LOOP_OFF,
            "correcting infinite effects also looped an explicitly single-cycle effect");
#if defined(S2_UPSTREAM_NATIVE_SFX)
        Require(S2FmodRenderForTest(output.data(),4096) != 0, "single-cycle control could not render its native PCM frames");
        Require(!FSOUND_IsPlaying(onceChannel), "an explicitly single-cycle effect continued after its actual sample ended");
#endif
        source->Set(vector<NWorld::IVisObj*>()); tick(1000);
        Require(playingChannel() == -1, "infinite-effect fixture retained a native playback channel");
        return;
    }
    CObj<EffectSoundSource> expired = new EffectSoundSource(effect,9500);
    LegacySoundEffectFields(instance,clock.GetTime(),expired->position);
    source->Set(vector<NWorld::IVisObj*>{expired.GetPtr()});
    tick(1000);
    Require(clock.GetTime()->GetValue() == 10000 && playingChannel() == -1,
        "sound-effect initial-delay control unexpectedly played or moved its baseline clock");
    tick(1300);
    Require(clock.GetTime()->GetValue() == 10300, "sound-effect fixture did not advance its real mixer clock");
    Require(playingChannel() == -1,
        "an expired world sound effect restarted after visibility was restored instead of seeking past its sample end");
    source->Set(vector<NWorld::IVisObj*>()); tick(1300);
    instance->nStartTime = 0;
    CObj<EffectSoundSource> tail = new EffectSoundSource(effect,10240);
    source->Set(vector<NWorld::IVisObj*>{tail.GetPtr()}); tick(1300);
    const int tailChannel = playingChannel();
    Require(tailChannel >= 0 && FSOUND_GetCurrentPosition(tailChannel) == 441,
        "a still-audible late effect did not retain its ten-millisecond sample offset");
    source->Set(vector<NWorld::IVisObj*>()); tick(1300);
    Require(playingChannel() == -1, "removing an effect source retained its actual native channel");
    {
        // Save a not-yet-started runtime node so only the new seek state, not a live channel,
        // must survive the native object-field round trip.
        CObj<NSound::CSoundInstance> node = new NSound::CSoundInstance(instance,10,clock.GetTime(),tail->position,vector<int>());
        CStructureNetworkContext graph;
        auto snapshot = BorrowedObjectFields(node,&graph);
        CObj<NSound::CSoundInstance> restored = new NSound::CSoundInstance;
        { CStructureSaver reader(snapshot,CStructureSaver::READ,&graph);
          static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
        CDGPtr<CFuncBase<bool>> refreshed = restored.GetPtr();
        MarkNewDGFrame(); refreshed.Refresh();
        const int restoredChannel = playingChannel();
        Require(restoredChannel >= 0 && FSOUND_GetCurrentPosition(restoredChannel) == 441,
            "restoring a late sound-effect node discarded its saved sample seek offset");
    }
    Require(playingChannel() == -1, "releasing the restored effect node retained its native channel");
    instance->nStartTime = 200;
    CObj<EffectSoundSource> future = new EffectSoundSource(effect,10300);
    source->Set(vector<NWorld::IVisObj*>{future.GetPtr()}); tick(1300);
    Require(playingChannel() == -1, "a fresh effect ignored its authored start delay");
    tick(1499);
    Require(playingChannel() == -1, "a delayed effect started before its scheduled native clock value");
    tick(1500);
    const int freshChannel = playingChannel();
    Require(freshChannel >= 0 && FSOUND_GetCurrentPosition(freshChannel) == 0,
        "an ordinary delayed effect failed to begin at its authored time and sample start");
    source->Set(vector<NWorld::IVisObj*>()); tick(1500);
    Require(playingChannel() == -1, "sound-effect fixture retained a playback channel after source removal");
}

static void SoundMixerClockCatchUp()
{
    CObj<NWorld::CWorldSyncSrc> source = new NWorld::CWorldSyncSrc;
    CObj<CCTime> sceneTime = new CCTime(0);
    CObj<NSound::ISoundScene> scene = NSound::CreateSoundScene(nullptr,nullptr,sceneTime);
    CObj<NRender::IRenderSound> mixer = dynamic_cast<NRender::IRenderSound*>(pSSClasses->CreateObject(int(0x03081142)));
    Require(IsValid(mixer), "sound-clock fixture has no registered native render mixer");
    CTimeCounter clock;
    clock.SetCurrent(5000);
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.include = [](CObjectBase*) { return false; };
    const vector<CObjectBase*> objects{source.GetPtr(),scene.GetPtr(),clock.GetTime()};
    for (std::size_t i = 0; i < objects.size(); ++i) {
        references.ids[objects[i]] = unsigned(i+1); references.refs[unsigned(i+1)] = objects[i];
    }
    references.nextID = 100;
    CMemoryStream saved;
    { CStructureSaver writer(saved,CStructureSaver::WRITE,&references);
      writer.Add(1,&source); writer.Add(2,&scene); writer.Add(3,&clock); }
    saved.Seek(0);
    { CStructureSaver reader(saved,CStructureSaver::READ,&references); static_cast<CObjectBase*>(mixer.GetPtr())->operator&(reader); }
    CTransformStack transform; transform.Init();
    mixer->Update(true,&transform,1000);
    Require(clock.GetTime()->GetValue() == 5000, "first mixer update advanced before establishing its restored clock baseline");
    mixer->Update(true,&transform,1100);
    Require(clock.GetTime()->GetValue() == 5100, "ordinary mixer-frame control did not advance its actual shared clock");
    mixer->Update(true,&transform,6100);
    Require(clock.GetTime()->GetValue() == 10100,
        "the native sound mixer lost most of a skipped interval by clamping its clock advance to one 150ms step");
    mixer->Update(false,&transform,9000);
    Require(clock.GetTime()->GetValue() == 10100, "a paused mixer advanced while consuming its clock baseline");
    mixer->Update(true,&transform,9300);
    Require(clock.GetTime()->GetValue() == 10400, "resuming the mixer replayed paused time or lost the resumed interval");
    mixer->ResetTiming();
    mixer->Update(true,&transform,20000);
    Require(clock.GetTime()->GetValue() == 10400, "resetting mixer timing caused a spurious catch-up jump");
}




static void WriteLegacyChapterFixture(const char* databasePath,const char* resourcePath,const char* fixturePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"legacy chapter fixture has no authored map");
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game);
    enter->Exec();
    CObj<NGame::CMissionBase> chapter = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(chapter) && !chapter->GetWorld() && !chapter->GetRenderGame() && !chapter->GetScene(),
        "legacy fixture writer must run with the historical pre-world chapter implementation");
    auto* ui = dynamic_cast<NUI::CChapterMapUI*>(chapter->GetInterface()->GetChildByID("chaptermapUI"));
    Require(ui && !chapter->GetDesktop() && !chapter->GetActivePlayer(),"historical chapter graph has an unexpected shape");
    ui->SetTarget(CVec2(113,227));
    CFileStream snapshot; snapshot.OpenWrite(fixturePath);
    { CStructureSaver writer(snapshot,CStructureSaver::WRITE); writer.Add(2,&game); writer.Add(3,&chapter); }
    NMainLoop::DoneInterface();
}

static void LegacyChapterWorldMigration(const char* databasePath,const char* resourcePath,const char* fixturePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness(),"legacy chapter test started with an active interface");
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CObj<NRPG::CGlobalGame> game;
    CObj<NGame::CMissionBase> chapter;
    CFileStream snapshot; snapshot.OpenRead(fixturePath);
    { CStructureSaver reader(snapshot,CStructureSaver::READ); reader.Add(2,&game); reader.Add(3,&chapter); }
    Require(IsValid(game) && IsValid(chapter) && pSSClasses->GetObjectTypeID(chapter) == int(0xB2301230) &&
        !chapter->GetWorld() && !chapter->GetRenderGame() && !chapter->GetScene(),
        "legacy fixture did not restore the actual historical null-world chapter graph");
    auto* ui = dynamic_cast<NUI::CChapterMapUI*>(chapter->GetInterface()->GetChildByID("chaptermapUI"));
    Require(ui && !chapter->GetDesktop() && !chapter->GetActivePlayer(),"legacy chapter fixture lost its original UI graph");
    auto* interface = chapter->GetInterface(); auto* cursor = chapter->GetCursor();
    const string travel = ui->GetTravelForDiagnostics();
    const int chapterID = game->nChapterMapID, difficulty = game->nCurrentChapterDifficulty;
    const auto mercs = game->players.front()->mercs;
    chapter->OnSnapshotRestored();
    Require(IsValid(chapter->GetWorld()) && IsValid(chapter->GetRenderGame()) && IsValid(chapter->GetScene()),
        "loading a historical chapter save left its native world/render graph missing");
    auto check = [&](NGame::CMissionBase* restored,NRPG::CGlobalGame* global) {
        auto* world = restored->GetWorld(); auto* player = restored->GetActivePlayer();
        auto* mapUI = dynamic_cast<NUI::CChapterMapUI*>(restored->GetInterface()->GetChildByID("chaptermapUI"));
        Require(IsValid(world) && world->GetGlobalGame() == global && IsValid(world->GetAIMap()) && IsValid(world->GetPathNetwork()) &&
            IsValid(player) && player->GetGlobalPlayer() == global->players.front() && IsValid(player->GetCommander()) &&
            IsValid(player->GetPlayer()) && IsValid(player->GetCamera()) && IsValid(restored->GetEnemyTurnCamera()),
            "legacy chapter migration did not rebuild its complete native world/player/camera links");
        Require(mapUI && restored->GetDesktop() == mapUI && mapUI->GetTravelForDiagnostics() == travel &&
            restored->GetInterface()->GetSound() == restored->GetSoundScene(),
            "legacy chapter migration discarded saved travel/UI state or failed to share its new sound scene");
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(restored->GetInterface(),&references);
        bool ownsSound = true;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(23,&ownsSound); }
        Require(!ownsSound,"migrated chapter UI still pumps its old independently clocked sound scene");
        Require(global->bChapterMapSet && global->nChapterMapID == chapterID && global->nCurrentChapterDifficulty == difficulty &&
            global->players.size() == 1 && global->players.front()->mercs.size() == mercs.size(),
            "legacy chapter migration changed campaign destination, difficulty or party size");
    };
    check(chapter,game);
    Require(chapter->GetInterface() == interface && chapter->GetCursor() == cursor && chapter->GetDesktop() == ui &&
        game->players.front()->mercs == mercs,"legacy migration replaced the original UI, cursor or RPG party objects");
    auto* world = chapter->GetWorld();
    const STime initial = world->GetAimTime()->GetValue();
    Require(initial == 0,"legacy migration ran a tactical warm-up");
    world->UpdateWorld(initial+100,nullptr);
    const STime savedTime = world->GetAimTime()->GetValue();
    Require(savedTime > initial,"migrated historical chapter world cannot advance native simulation");
    chapter->OnSnapshotRestored();
    Require(chapter->GetWorld() == world && world->GetAimTime()->GetValue() == savedTime,
        "a second restoration hook recreated the migrated world or reset its simulation");
    CMemoryStream current;
    { CStructureSaver writer(current,CStructureSaver::WRITE); writer.Add(2,&game); writer.Add(3,&chapter); }
    current.Seek(0);
    CObj<NRPG::CGlobalGame> loadedGame; CObj<NGame::CMissionBase> loaded;
    { CStructureSaver reader(current,CStructureSaver::READ); reader.Add(2,&loadedGame); reader.Add(3,&loaded); }
    auto* savedWorld = loaded->GetWorld(); auto* savedRender = loaded->GetRenderGame();
    Require(savedWorld && savedWorld != world && savedWorld->GetAimTime()->GetValue() == savedTime,
        "a migrated chapter did not save a complete independent world graph");
    loaded->OnSnapshotRestored(); loaded->OnSnapshotRestored();
    check(loaded,loadedGame);
    Require(loaded->GetWorld() == savedWorld && loaded->GetRenderGame() == savedRender && savedWorld->GetAimTime()->GetValue() == savedTime,
        "restoring an already complete chapter recreated its world/render or reset the saved clock");
    savedWorld->UpdateWorld(savedTime+50,nullptr);
    Require(savedWorld->GetAimTime()->GetValue() > savedTime,"a complete migrated chapter failed to resume after its next snapshot load");
    // Real focus and rendering clocks must now be safe for both generations.
    chapter->OnLostFocus(); chapter->OnGetFocus(); loaded->OnLostFocus(); loaded->OnGetFocus();
}

class ChapterZoneDrainMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(ChapterZoneDrainMission);
    CPtr<NUI::CDesktopWindow> desktop;
public:
    void Attach(NWorld::CWorld* world,NRPG::CGlobalGame* game,NUI::CDesktopWindow* top) {
        pWorld = world; pGlobalGame = game; desktop = top;
    }
    NUI::CDesktopWindow* GetDesktop() const override { return desktop; }
    void Drain() { ExecWorldCommands(); }
};

class BeginMissionCommandAccess: public NGame::CICBeginMission
{
public:
    static auto Zone() { return &BeginMissionCommandAccess::pZone; }
    static auto Game() { return &BeginMissionCommandAccess::pGlobalGame; }
    static auto Template() { return &BeginMissionCommandAccess::nTemplateID; }
    static auto EmulatesChapter() { return &BeginMissionCommandAccess::bEmulateChapter; }
    static auto Image() { return &BeginMissionCommandAccess::pPWLImage; }
};

class CachedChapterSampleAccess: public NSound::CFileSample2D
{
public:
    static auto ValueMember() { return &CachedChapterSampleAccess::pValue; }
};

// Shared native display lifetime for world-backed interface regressions.
// Construct before the interface graph so it also handles partial initialization.
struct NativeDisplayContext
{
    bool platform = false, graphics = false, audio = false;
    ~NativeDisplayContext() {
        NGScene::StopResourceLoadingThread(); NMainLoop::DoneInterface();
        while (NMainLoop::HaveInterfaceCommand()) { CObj<NMainLoop::CInterfaceCommand> command = NMainLoop::TakeInterfaceCommandForHarness(); }
        ClearHoldQueue();
#if defined(S2_UPSTREAM_NATIVE_SFX)
        if (audio) NFMSound::Done();
#endif
        if (graphics) NGfx::Done3D();
        if (platform) S2Platform::Done();
    }
    void Initialize(const char* databasePath,const char* resourcePath,const char* title) {
        S2Platform::SetErrorDialogs(false);
        Require(S2Platform::Init(title,320,240,true),"native display fixture could not create its hidden SDL window");
        platform = true;
        Require(NGfx::Init3D(static_cast<HWND>(S2Platform::NativeWindow())),"native display fixture has no graphics window handle");
        graphics = true;
        NGfx::SRenderTargetsInfo targets;
        Require(NGfx::SetMode(NGfx::SVideoMode(320,240,32,NGfx::WINDOWED),targets) && NGScene::Is3DActive() && S2Platform::Display().drawable,
            "native display fixture could not initialize its real healthy bgfx renderer");
        CFileStream database; database.OpenRead(databasePath);
        NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
        NGScene::RunResourceLoadingThread();
    }
};

class NativeBackdropInterface: public NGame::CRenderBaseInterface
{
    OBJECT_NOCOPY_METHODS(NativeBackdropInterface);
public:
    void Step() override {
        NGame::CRenderBaseInterface::Step();
        if (CanRender()) RenderFrame(GetTime(),GetCamera());
    }
};

static void BackdropScriptAudio(const char* databasePath,const char* resourcePath,bool spatial)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr,"backdrop sound fixture requires headless native audio");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"backdrop sound fixture started with an interface or pending command");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm backdrop sound regression");
    Require(NFMSound::Init(NFMSound::SStartInfo()),"backdrop sound fixture could not initialize native audio");
    cleanup.audio = true; NFMSound::SetMusicMasterVolume(0);
    CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
    Require(sounds.MoveNext(),"backdrop sound fixture has no native sound record");
    auto* record = sounds.Get(); record->bLoop = true; record->nStartSamples = record->nEndingSamples = 0;
    const auto wave = SilentPCMFixture(44100);
    CObj<NFMSound::CSample2D> sample2D = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
    CObj<NFMSound::CSample3D> sample3D = NFMSound::LoadSample3D(wave.data(),static_cast<int>(wave.size()),1.f,100.f,0);
    Require(IsValid(sample2D) && IsValid(sample3D),"backdrop sound fixture could not decode its native PCM samples");
    CObj<NSound::CFileSample2D> loader2D = NSound::share2DSamples.Get(record->GetRecordID());
    CObj<NSound::CFileSample3D> loader3D = NSound::share3DSamples.Get(record->GetRecordID());
    (loader2D.GetPtr()->*CachedChapterSampleAccess::ValueMember()) = sample2D.GetPtr();
    (loader3D.GetPtr()->*CachedEffectSampleAccess::ValueMember()) = sample3D.GetPtr();
    CObj<NDb::CScript> entry = new NDb::CScript;
    const string play = spatial ? "Play3DSound("+std::to_string(record->GetRecordID())+",GetWaypointPos('__native_backdrop_audio__'))" :
        "PlaySound("+std::to_string(record->GetRecordID())+")";
    entry->strCode = "BackdropStarts = (BackdropStarts or 0) + 1; BackdropEnters = 0; "
        "function OnEnterZone() BackdropEnters = BackdropEnters + 1 end; BackdropHandle = "+play+"; BackdropRequested = 1";
    struct VariantRestore {
        vector<std::pair<CPtr<NDb::CTemplVariant>,CPtr<NDb::CScript>>> records;
        ~VariantRestore() { for (auto& pair: records) if (IsValid(pair.first)) pair.first->pScript = pair.second; }
    } restore;
    CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
    while (variants.MoveNext()) {
        auto* variant = variants.Get();
        if (IsValid(variant->pTemplate) && variant->pTemplate->GetRecordID() == 2425) {
            std::printf("backdrop template2425 variant=%d originalScript=%d\n",variant->GetRecordID(),IsValid(variant->pScript) ? variant->pScript->GetRecordID() : 0);
            restore.records.push_back({variant,variant->pScript}); variant->pScript = entry;
        }
    }
    Require(!restore.records.empty(),"backdrop sound fixture has no authored main-menu map variant");
    CObj<NativeBackdropInterface> backdrop = new NativeBackdropInterface;
    backdrop->Initialize(2425);
    auto* placement = NDb::GetDBCamera(26);
    Require(placement != nullptr,"backdrop sound fixture has no authored menu camera");
    backdrop->GetCamera()->SetPlacement(ICamera::SCameraPos(placement->vAnchor,placement->fDistance,placement->fPitch,placement->fYaw,placement->fRoll,placement->fFOV));
    backdrop->GetCamera()->SetScreenRect(CTRect<float>(0.f,0.f,1.f,1.f));
    class InstallBackdrop: public NMainLoop::CInterfaceCommand {
        OBJECT_NOCOPY_METHODS(InstallBackdrop);
    public:
        CPtr<NMainLoop::IInterfaceBase> owner;
        void Exec() override { SetInterface(owner); }
    };
    CObj<InstallBackdrop> install = new InstallBackdrop; install->owner = backdrop; install->Exec();
    auto* world = dynamic_cast<NWorld::CWorld*>(backdrop->GetWorld());
    auto* script = world ? world->GetOwnScript() : nullptr;
    Require(IsValid(script),"backdrop sound fixture has no actual initialized world script");
    auto number = [&](const char* name,int expected) {
        const int top = script->GetTop(); auto value = script->GetGlobal(name);
        const bool matches = value.IsNumber() && value.GetNumber() == expected; script->SetTop(top); return matches;
    };
    Require(number("BackdropRequested",1) && number("BackdropStarts",1),
        "the existing native backdrop initializer never ran its map-variant script exactly once");
    const int top = script->GetTop(); auto handle = script->GetGlobal("BackdropHandle");
    auto* command = handle.IsUserData() ? dynamic_cast<NWorld::CUICmdPlaySound*>(handle.GetUserData()) : nullptr;
    Require(command != nullptr,"the backdrop variant script did not create its native Lua sound command");
    script->SetTop(top);
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(1000)};
    auto frame = [&](STime time,bool active) {
        NMainLoop::SetTimeForHarness(time);
        Require(NMainLoop::StepApp(active,false,false),"native backdrop frame requested application exit");
        Require(NMainLoop::GetCurrentInterfaceForHarness() == backdrop.GetPtr() && !NMainLoop::HaveInterfaceCommand(),
            "backdrop sound fixture was covered by an unrelated interface");
    };
    frame(1000,false);
    Require(!IsValid(command->pChannel),"inactive backdrop frames consumed a queued script sound");
    frame(1050,true);
    Require(IsValid(command->pChannel),"the active native backdrop ignored its initialized map script's sound command");
    auto channelExcept = [&](int except) {
        int found = -1;
        for (int i = 0; i < 128; ++i) if (i != except && FSOUND_IsPlaying(i)) {
            Require(found == -1,"backdrop sound fixture has an unexpected extra native channel"); found = i;
        }
        return found;
    };
    const int played = channelExcept(-1);
    Require(played >= 0 && FSOUND_GetCurrentPosition(played) == 0,"backdrop dispatch did not start its real native PCM channel at offset zero");
    vector<float> output(512*2);
    auto render = [&]() { Require(S2FmodRenderForTest(output.data(),512) != 0,"backdrop sound fixture could not render native PCM"); };
    render();
    Require(FSOUND_GetCurrentPosition(played) > 0,"backdrop sound never advanced native PCM playback");
    CObj<NFMSound::CSound2D> independent = NFMSound::PlaySound(sample2D);
    const int control = channelExcept(played);
    Require(IsValid(independent) && control >= 0,"backdrop sound fixture could not start its independent control");
    Require(script->DoString("StopSound(BackdropHandle); BackdropStopped = 1") == 0,"backdrop sound fixture could not queue its native Lua stop");
    for (int tick = 0; tick < 8; ++tick) frame(1100+tick*50,true);
    render();
    Require(number("BackdropStopped",1) && !FSOUND_IsPlaying(played) && FSOUND_IsPlaying(control),
        "the native backdrop ignored Lua StopSound or interrupted an unrelated channel");
    Require(script->DoString(("BackdropHandle = "+play+"; BackdropRestarted = 1").c_str()) == 0,"backdrop sound fixture could not queue its native Lua restart");
    for (int tick = 0; tick < 8; ++tick) frame(1500+tick*50,true);
    const int restarted = channelExcept(control);
    Require(number("BackdropRestarted",1) && restarted >= 0 && number("BackdropStarts",1) && number("BackdropEnters",1),
        "the native backdrop failed to restart audio or repeated initialization/OnEnterZone");
    render();
    Require(FSOUND_GetCurrentPosition(restarted) > 0,"restarted backdrop sound did not advance native PCM");
    NMainLoop::DoneInterface(); backdrop = nullptr; ClearHoldQueue(); render();
    Require(!FSOUND_IsPlaying(restarted) && FSOUND_IsPlaying(control),"backdrop teardown leaked its sound or stopped the independent control");
#else
    Require(false,"backdrop sound test requires the native SFX backend");
#endif
}

static void InGameSaveButton(const char* databasePath,const char* resourcePath,bool shortcut = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"in-game save button fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"in-game save button fixture started with an active interface");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm in-game save permission regression");
    NMainLoop::CreateProfile("UpstreamInGameSave"); NMainLoop::SetActiveProfile("UpstreamInGameSave");
    CObj<NRPG::CGlobalPlayer> player = NRPG::CreateGlobalPlayer(vector<int>{54});
    player->pSide = NDb::GetDBSide(1);
    Require(IsValid(player->pSide),"in-game save button fixture has no authored player side");
    NInput::CBind saveBind("savegame"), loadBind("loadgame");
    auto click = [&](NUI::CWindow* control) {
        NUI::SPoint position; NUI::SRect rect;
        Require(IsValid(control) && control->ClientToScreen(&position,&rect) && rect.x2 > rect.x1 && rect.y2 > rect.y1,
            "in-game menu button has no native hit-test rectangle");
        const int x = (rect.x1+rect.x2)/2, y = (rect.y1+rect.y2)/2;
        Require(control->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONDOWN,x,y)) && control->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONUP,x,y)),
            "in-game menu fixture could not click its native button");
    };
    for (bool allowed : {false,true}) {
        CObj<NGame::CICInGameMenu> open = new NGame::CICInGameMenu(player,false,allowed); open->Exec();
        auto* active = NMainLoop::GetCurrentInterfaceForHarness();
        Require(IsValid(active) && pSSClasses->GetObjectTypeID(active) == int(0xB0905130) && NMainLoop::StepApp(true,false,false),
            "in-game save button fixture did not open and draw its actual native menu");
        CStructureNetworkContext references; auto fields = BorrowedObjectFields(active,&references);
        bool menuPermission = !allowed; CPtr<NUI::CWindow> page;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(2,&menuPermission); reader.Add(9,&page); }
        Require(menuPermission == allowed && IsValid(page),"in-game menu lost its incoming save permission");
        auto* save = dynamic_cast<NUI::CButton*>(page->GetChildByID("savegame"));
        auto* load = dynamic_cast<NUI::CButton*>(page->GetChildByID("loadgame"));
        auto* restart = page->GetChildByID("restart");
        Require(IsValid(save) && IsValid(load) && IsValid(restart) && load->GetStyle(NUI::STYLE_ENABLED) && !restart->GetStyle(NUI::STYLE_ENABLED),
            "in-game save button fixture has no independent Load/Restart controls");
        Require(save->GetStyle(NUI::STYLE_ENABLED) == allowed,"the in-game Save button ignored its explicit save permission");
        CStructureNetworkContext pageReferences; auto pageFields = BorrowedObjectFields(page,&pageReferences);
        bool pagePermission = !allowed, pageRestart = true; CPtr<NRPG::CGlobalPlayer> savedPlayer; CPtr<NUI::CWindow> savedButton;
        { CStructureSaver reader(pageFields,CStructureSaver::READ,&pageReferences);
          reader.Add(2,&pagePermission); reader.Add(3,&pageRestart); reader.Add(4,&savedPlayer); reader.Add(8,&savedButton); }
        Require(pagePermission == allowed && !pageRestart && savedPlayer.GetPtr() == player.GetPtr() && savedButton.GetPtr() == save,
            "in-game menu changed its existing saved permission or button fields");
        NInput::SEvent event; while (NInput::GetEvent(&event)) {}
        if (shortcut) {
            NInput::PostEvent("savegame");
            Require(NInput::GetEvent(&event) && saveBind.ProcessEvent(event),"in-game save shortcut fixture did not receive its actual bound command");
        } else if (!allowed) {
            click(save);
            Require(!NInput::GetEvent(&event) && !NMainLoop::HaveInterfaceCommand(),"a disabled in-game Save button still emitted a save action");
            click(load);
            Require(NInput::GetEvent(&event) && loadBind.ProcessEvent(event),"disabling in-game Save also disabled the real Load button");
        } else {
            click(save);
            Require(NInput::GetEvent(&event) && saveBind.ProcessEvent(event),"an explicitly permitted in-game Save button did not emit its action");
        }
        Require(active->ProcessEvent(event) && NMainLoop::HaveInterfaceCommand(),"the in-game menu did not dispatch its actual save/load button action");
        CObj<NMainLoop::CInterfaceCommand> command = NMainLoop::TakeInterfaceCommandForHarness();
        Require(dynamic_cast<NGame::CICSaveLoadMenu*>(command.GetPtr()) != nullptr,"the in-game button queued an unrelated command");
        command->Exec();
        CStructureNetworkContext dialogReferences; auto dialog = BorrowedObjectFields(NMainLoop::GetCurrentInterfaceForHarness(),&dialogReferences);
        bool dialogPermission = !allowed; int mode = -1; CPtr<NUI::CWindow> dialogPage;
        { CStructureSaver reader(dialog,CStructureSaver::READ,&dialogReferences); reader.Add(2,&dialogPermission); reader.Add(3,&mode); reader.Add(6,&dialogPage); }
        Require(IsValid(dialogPage),"in-game save/load command did not create its native dialog page");
        CStructureNetworkContext viewReferences; auto viewFields = BorrowedObjectFields(dialogPage,&viewReferences);
        CPtr<NUI::CWindow> view;
        { CStructureSaver reader(viewFields,CStructureSaver::READ,&viewReferences); reader.Add(4,&view); }
        Require(IsValid(view),"a forbidden in-game Save shortcut opened an empty native save/load page");
        Require(dialogPermission == allowed && mode == (allowed ? NGame::SAVE : NGame::LOAD),"the in-game button lost its save/load destination or permission");
        Require(pSSClasses->GetObjectTypeID(view) == int(allowed ? 0xB1014135 : 0xB1014134),"the in-game command created the wrong native save/load view");
        NMainLoop::DoneInterface();
    }
    std::printf("actual in-game Save permission false/true, shortcut=%d, native save/load views and saved UI fields verified\n",int(shortcut));
}

static void LuaUnusedStackInitialization()
{
    alignas(TObject) unsigned char storage[sizeof(TObject)];
    std::memset(storage,0xa5,sizeof(storage));
    TObject* fresh = new(storage) TObject;
    Require(fresh->GetType() == LUA_TNIL,"a native Lua value created over dirty storage did not start as nil");
    fresh->~TObject();
    CObj<CLuaThread> thread = new CLuaThread("unused stack initialization audit");
    thread->stack.resize(thread->stack.size()+37);
    for (const auto& value : thread->stack) Require(value.GetType() == LUA_TNIL,"new or expanded native Lua stack slots have invalid tags");
    thread->top = 2; thread->stack[0].SetN(19); thread->stack[1].SetN(23);
    // A popped slot can retain an already collected string. The existing
    // thread saver must clear spare slots before serializing the whole vector.
    thread->stack[5].SetS(reinterpret_cast<TString*>(std::uintptr_t(1)));
    CMemoryStream wire;
    { CStructureSaver writer(wire,CStructureSaver::WRITE); writer.Add(2,&thread); }
    Require(thread->stack[5].GetType() == LUA_TNIL,"saving a Lua thread dereferenced or retained a stale spare stack slot");
    wire.Seek(0); CObj<CLuaThread> restored;
    { CStructureSaver reader(wire,CStructureSaver::READ); reader.Add(2,&restored); }
    Require(IsValid(restored) && restored.GetPtr() != thread.GetPtr() && restored->top == 2 && restored->stack.size() == thread->stack.size() &&
        restored->stack[0].GetType() == LUA_TNUMBER && restored->stack[0].GetN() == 19 && restored->stack[1].GetType() == LUA_TNUMBER && restored->stack[1].GetN() == 23,
        "native Lua thread serialization lost its live numeric slots");
    for (std::size_t i = restored->top; i < restored->stack.size(); ++i)
        Require(restored->stack[i].GetType() == LUA_TNIL,"native Lua thread serialization restored an invalid spare slot");
    std::printf("existing native Lua nil constructor and %zu serialized spare slots verified\n",restored->stack.size()-restored->top);
}

static void AudioOptionCheckboxes(const char* databasePath,const char* resourcePath,bool notificationOnly = false,bool resetDefaults = false)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_USER_DATA_DIR") && std::getenv("S2_AUDIO_HEADLESS"),"audio options fixture requires isolated user data and native headless audio");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"audio options fixture started with an active interface");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm audio option regression");
    Require(NFMSound::Init(NFMSound::SStartInfo()),"audio options fixture could not initialize native audio");
    cleanup.audio = true;
    const char* settings[] = {"sound_sfxvolume","sound_musicvolume","ui_charresponses","ui_charresponsessubtitles"};
    float defaults[4]{};
    if (resetDefaults) for (int i = 0; i < 4; ++i) {
        NGlobal::ResetVar(settings[i]); defaults[i] = NGlobal::GetVar(settings[i]).GetFloat();
    }
    NGlobal::SetVar("sound_sfxvolume",0.23f); NGlobal::SetVar("sound_musicvolume",0.61f);
    NGlobal::SetVar("ui_charresponses",resetDefaults ? 0.f : 1.f); NGlobal::SetVar("ui_charresponsessubtitles",0.f);
    const auto wave = SilentPCMFixture(44100);
    CObj<NFMSound::CSample2D> sample = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
    Require(IsValid(sample),"audio options fixture could not decode its native control sample");
    CObj<NFMSound::CSound2D> playing = NFMSound::PlaySound(sample,0,0,0,true);
    Require(IsValid(playing),"audio options fixture could not start its independent native sound");
    int channel = -1;
    for (int i = 0; i < 128; ++i) if (FSOUND_IsPlaying(i)) {
        Require(channel == -1,"audio options fixture started with unexpected sound channels"); channel = i;
    }
    Require(channel >= 0,"audio options fixture has no actual playing native channel");
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(1,1); pixels[0][0] = NGfx::SPixel8888(11,22,33,255); texture->Set(pixels);
    CObj<NGame::CICOptions> open = new NGame::CICOptions(NGame::OS_AUDIO,texture); open->Exec();
    auto* active = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(active),"audio options fixture did not open the actual native page");
    Require(NMainLoop::StepApp(true,false,false) && NMainLoop::GetCurrentInterfaceForHarness() == active && !NMainLoop::HaveInterfaceCommand(),
        "audio options fixture did not render its active native interface");
    CStructureNetworkContext interfaceReferences;
    auto interfaceFields = BorrowedObjectFields(active,&interfaceReferences);
    CPtr<NUI::CWindow> page;
    { CStructureSaver reader(interfaceFields,CStructureSaver::READ,&interfaceReferences); reader.Add(6,&page); }
    Require(IsValid(page) && pSSClasses->GetObjectTypeID(page) == int(0xB0815152),"audio options fixture did not retain the authored audio page");
    auto* responses = dynamic_cast<NUI::CCheckButton*>(page->GetChildByID("char_responses"));
    auto* subtitles = dynamic_cast<NUI::CCheckButton*>(page->GetChildByID("char_subtitles"));
    auto* sfx = dynamic_cast<NUI::CScroll*>(page->GetChildByID("sound_volume"));
    auto* music = dynamic_cast<NUI::CScroll*>(page->GetChildByID("music_volume"));
    Require(IsValid(responses) && IsValid(subtitles) && IsValid(sfx) && IsValid(music),"authored native audio options lack their real controls");
    CStructureNetworkContext pageReferences;
    auto pageFields = BorrowedObjectFields(page,&pageReferences);
    CPtr<NUI::CWindow> legacyApply, legacyDefault, legacySfx, legacyMusic, legacyOutput;
    { CStructureSaver reader(pageFields,CStructureSaver::READ,&pageReferences);
      reader.Add(2,&legacyApply); reader.Add(3,&legacyDefault); reader.Add(4,&legacySfx); reader.Add(5,&legacyMusic); reader.Add(6,&legacyOutput); }
    Require(IsValid(legacyApply) && legacyDefault.GetPtr() == page->GetChildByID("default") && legacySfx.GetPtr() == sfx && legacyMusic.GetPtr() == music && IsValid(legacyOutput),
        "audio option changes reinterpreted the fork's existing serialized field tags");
    Require(!legacyApply->GetStyle(NUI::STYLE_VISIBLE) && !legacyOutput->GetStyle(NUI::STYLE_VISIBLE) &&
        legacyApply->GetSize().x == 0 && legacyApply->GetSize().y == 0 && legacyOutput->GetSize().x == 0 && legacyOutput->GetSize().y == 0,
        "legacy audio option fields introduced active Apply/output controls into the authored page");
    auto volumes = [&]() {
        Require(fabs(NGlobal::GetVar("sound_sfxvolume").GetFloat()-0.23f) < 1e-6f && fabs(NGlobal::GetVar("sound_musicvolume").GetFloat()-0.61f) < 1e-6f &&
            sfx->GetMaxValue() == 100 && sfx->GetValue() == 23 && music->GetMaxValue() == 100 && music->GetValue() == 61,
            "audio checkbox initialization or notification changed the existing normalized volume controls");
    };
    volumes();
    if (notificationOnly) { responses->SetChecked(true); subtitles->SetChecked(false); }
    else Require(responses->IsChecked() == !resetDefaults && !subtitles->IsChecked(),"the native audio page did not initialize its acknowledgement checkboxes from config");
    auto click = [&](NUI::CWindow* control) {
        Require(IsValid(control),"audio options fixture lacks the requested button");
        NUI::SPoint position; NUI::SRect rect;
        Require(control->ClientToScreen(&position,&rect) && rect.x2 > rect.x1 && rect.y2 > rect.y1,"audio option button has no native hit-test rectangle");
        const int x = (rect.x1+rect.x2)/2, y = (rect.y1+rect.y2)/2;
        Require(control->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONDOWN,x,y)) && control->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONUP,x,y)),
            "audio options fixture could not click its real button");
    };
    if (resetDefaults) {
        Require(defaults[0] != 0.23f && defaults[1] != 0.61f && defaults[2] == 1.f && defaults[3] == 1.f,
            "audio defaults fixture did not start with independently changed settings");
        vector<float> output(512*2);
        Require(S2FmodRenderForTest(output.data(),512) != 0 && FSOUND_GetCurrentPosition(channel) > 0,
            "audio defaults fixture could not advance its existing native channel");
        const auto before = FSOUND_GetCurrentPosition(channel);
        auto restored = [&]() {
            for (int i = 0; i < 4; ++i) Require(fabs(NGlobal::GetVar(settings[i]).GetFloat()-defaults[i]) < 1e-6f,
                "the native audio Default button did not restore registered settings");
            Require(sfx->GetValue() == int(defaults[0]*100) && music->GetValue() == int(defaults[1]*100) && responses->IsChecked() && subtitles->IsChecked(),
                "the native audio Default button did not refresh its actual controls");
        };
        click(page->GetChildByID("default")); restored();
        Require(FSOUND_IsPlaying(channel) && S2FmodRenderForTest(output.data(),512) != 0 && FSOUND_GetCurrentPosition(channel) > before,
            "resetting native audio settings stopped or restarted an existing sound");
        for (const auto* setting : settings) NGlobal::SetVar(setting,0.f);
        NGlobal::LoadConfig(".\\cfg\\config.cfg"); restored();
        click(sfx->GetChildByID("plus"));
        Require(sfx->GetValue() == int(defaults[0]*100)+1 && fabs(NGlobal::GetVar("sound_sfxvolume").GetFloat()-(defaults[0]+0.01f)) < 1e-6f &&
            fabs(NGlobal::GetVar("sound_musicvolume").GetFloat()-defaults[1]) < 1e-6f,
            "resetting native audio settings broke subsequent live normalized slider changes");
        click(page->GetChildByID("default")); restored();
        std::printf("native audio Default restored registered values %.2f/%.2f and acknowledgements; existing PCM continued\n",defaults[0],defaults[1]);
        NMainLoop::DoneInterface(); return;
    }
    click(responses);
    Require(!responses->IsChecked() && NGlobal::GetVar("ui_charresponses").GetFloat() == 0.f && NGlobal::GetVar("ui_charresponsessubtitles").GetFloat() == 0.f,
        "the native response checkbox did not apply its change to config");
    click(subtitles);
    Require(subtitles->IsChecked() && NGlobal::GetVar("ui_charresponses").GetFloat() == 0.f && NGlobal::GetVar("ui_charresponsessubtitles").GetFloat() == 1.f,
        "the native subtitle checkbox did not apply its independent change to config");
    volumes();
    NGlobal::SetVar("ui_charresponses",1.f); NGlobal::SetVar("ui_charresponsessubtitles",0.f);
    NGlobal::LoadConfig(".\\cfg\\config.cfg");
    Require(NGlobal::GetVar("ui_charresponses").GetFloat() == 0.f && NGlobal::GetVar("ui_charresponsessubtitles").GetFloat() == 1.f,
        "native audio acknowledgement checkbox changes were not saved to isolated config");
    vector<float> output(512*2);
    Require(FSOUND_IsPlaying(channel) && S2FmodRenderForTest(output.data(),512) != 0 && FSOUND_GetCurrentPosition(channel) > 0,
        "audio checkbox changes interrupted an existing native sound");
    NMainLoop::DoneInterface();
    open = new NGame::CICOptions(NGame::OS_AUDIO,texture); open->Exec();
    CStructureNetworkContext reopenedReferences;
    auto reopened = BorrowedObjectFields(NMainLoop::GetCurrentInterfaceForHarness(),&reopenedReferences);
    { CStructureSaver reader(reopened,CStructureSaver::READ,&reopenedReferences); reader.Add(6,&page); }
    responses = dynamic_cast<NUI::CCheckButton*>(page->GetChildByID("char_responses"));
    subtitles = dynamic_cast<NUI::CCheckButton*>(page->GetChildByID("char_subtitles"));
    Require(IsValid(responses) && IsValid(subtitles) && !responses->IsChecked() && subtitles->IsChecked(),"reopening native audio options lost the saved acknowledgement state");
    std::printf("native audio acknowledgement click/config/reopen and legacy tags 2..6 preserved; PCM channel=%d\n",channel);
    NMainLoop::DoneInterface();
#else
    (void)databasePath; (void)resourcePath; (void)notificationOnly; (void)resetDefaults;
    Require(false,"native audio options fixture requires native SFX");
#endif
}

static void MissionSaveMenu(const char* databasePath,const char* resourcePath,bool quickSave = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"mission save-menu fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"mission save-menu fixture started with an active interface");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm mission save-menu regression");
    const char* profile = quickSave ? "UpstreamMissionQuickSave" : "UpstreamMissionSaveMenu";
    NMainLoop::CreateProfile(profile); NMainLoop::SetActiveProfile(profile);
    auto* manager = NMainLoop::GetSaveManager();
    struct SlotCleanup {
        NMainLoop::CSaveManager* manager; bool visible;
        void ClearVisible() { list<string> slots; manager->GetSlotsList(&slots); for (const auto& slot : slots) manager->DeleteSlot(slot); }
        ~SlotCleanup() { if (visible) ClearVisible(); manager->DeleteSlot(NMainLoop::S_SLOT_ACTIVE); }
    } slotCleanup{manager,quickSave};
    if (quickSave) slotCleanup.ClearVisible();
    NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(-1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginMission> enter = new NGame::CICBeginMission(810,-1,vector<string>(),game); enter->Exec();
    CObj<NGame::CMission> mission = dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(mission) && IsValid(mission->GetWorld()),"mission save-menu fixture did not open its actual tactical mission");
    CObj<NMainLoop::CInterfaceCommand> snapshot = NMainLoop::TakeInterfaceCommandForHarness();
    Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"mission save-menu fixture did not isolate its real restart snapshot");
    snapshot->Exec();
    const auto* world = mission->GetWorld(); const STime worldTime = world->GetAimTime()->GetValue();
    const auto* difficulty = game->pDifficulty.GetPtr();
    NWorld::IPlayer::CUnitSet party; mission->GetActivePlayer()->GetPlayer()->GetUnits(&party);
    auto permission = [&](bool allowed) {
        // Supply an existing saved permission without changing its calculation.
        CMemoryStream fields;
        { CStructureSaver writer(fields,CStructureSaver::WRITE); writer.Add(30,&allowed); }
        fields.Seek(0);
        { CStructureSaver reader(fields,CStructureSaver::READ); mission->NGame::CMissionBase::operator&(reader); }
    };
    if (quickSave) {
        auto slots = [&]() { list<string> result; manager->GetSlotsList(&result); result.sort(); return result; };
        auto bytes = [&](const string& slot) {
            CFileStream file; file.OpenRead(manager->GetSlotFilePathW(slot,NMainLoop::S_SAVE_FILENAME).c_str());
            vector<unsigned char> data(file.GetSize()); if (!data.empty()) file.Read(data.data(),data.size()); return data;
        };
        Require(slots().empty(),"mission QuickSave fixture did not clear its own isolated slots");
        NInput::CBind save("save");
        for (bool allowed : {false,true,false}) {
            permission(allowed);
            const auto before = slots(); map<string,vector<unsigned char>> previous;
            for (const auto& slot : before) previous[slot] = bytes(slot);
            const string target = NMainLoop::GetQuickSaveSlot(false);
            NInput::SEvent event; while (NInput::GetEvent(&event)) {}
            NInput::PostEvent("save");
            // Exercise the real application dispatcher, including its global
            // fallback when the active mission does not consume this event.
            Require(NMainLoop::StepApp(false,false,true),"native mission QuickSave dispatch requested application exit");
            const auto after = slots();
            std::printf("native QuickSave permission=%d, slots before=%zu after=%zu\n",int(allowed),before.size(),after.size());
            if (!allowed) {
                Require(after == before,"a forbidden mission QuickSave created or changed a save slot");
                for (const auto& slot : before) Require(bytes(slot) == previous[slot],"a forbidden mission QuickSave overwrote an existing save");
            } else {
                Require(after.size() == before.size()+1 && std::find(after.begin(),after.end(),target) != after.end(),
                    "an explicitly permitted native mission QuickSave did not write its expected slot");
                CFileStream file; file.OpenRead(manager->GetSlotFilePathW(target,NMainLoop::S_SAVE_FILENAME).c_str());
                auto header = std::make_unique<NMainLoop::SSaveFileHeader>(); NMainLoop::ReadSaveFileHeader(file,header.get());
                Require(header->nMagic == NMainLoop::N_SAVE_MAGIC_NUMBER && header->nMods == 0 &&
                    file.GetSize() > 8+NMainLoop::N_SAVE_SCREENSHOT_X*NMainLoop::N_SAVE_SCREENSHOT_Y*4+1024,
                    "native mission QuickSave did not write its real header and serialized mission data");
            }
            NWorld::IPlayer::CUnitSet afterParty; mission->GetActivePlayer()->GetPlayer()->GetUnits(&afterParty);
            Require(NMainLoop::GetCurrentInterfaceForHarness() == mission.GetPtr() && !NMainLoop::HaveInterfaceCommand() && mission->GetWorld() == world &&
                world->GetAimTime()->GetValue() == worldTime && afterParty == party && game->pDifficulty.GetPtr() == difficulty,
                "native QuickSave dispatch changed the mission, world, party or difficulty");
        }
        std::puts("native mission QuickSave false/true/false: no forbidden slot or overwrite; permitted save header/data written");
        NMainLoop::DoneInterface(); return;
    }
    NInput::CBind saveMenu("savemenu");
    for (bool allowed : {false,true}) {
        permission(allowed);
        NInput::SEvent event; while (NInput::GetEvent(&event)) {}
        NInput::PostEvent("savemenu");
        Require(NInput::GetEvent(&event) && saveMenu.ProcessEvent(event) && mission->ProcessEvent(event) && NMainLoop::HaveInterfaceCommand(),
            "mission save-menu fixture did not dispatch its actual native bound command");
        CObj<NMainLoop::CInterfaceCommand> command = NMainLoop::TakeInterfaceCommandForHarness();
        Require(dynamic_cast<NGame::CICSaveLoadMenu*>(command.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"mission Save-menu action queued an unrelated command");
        command->Exec();
        CStructureNetworkContext dialogReferences; auto dialog = BorrowedObjectFields(NMainLoop::GetCurrentInterfaceForHarness(),&dialogReferences);
        bool dialogPermission = !allowed; int mode = -1; CPtr<NUI::CWindow> page;
        { CStructureSaver reader(dialog,CStructureSaver::READ,&dialogReferences); reader.Add(2,&dialogPermission); reader.Add(3,&mode); reader.Add(6,&page); }
        Require(dialogPermission == allowed && IsValid(page),"the actual mission did not forward its existing save permission to the native dialog");
        CStructureNetworkContext viewReferences; auto fields = BorrowedObjectFields(page,&viewReferences); CPtr<NUI::CWindow> view;
        { CStructureSaver reader(fields,CStructureSaver::READ,&viewReferences); reader.Add(4,&view); }
        Require(IsValid(view),"a forbidden mission Save-menu command opened an empty native save/load page");
        Require(mode == (allowed ? NGame::SAVE : NGame::LOAD) && pSSClasses->GetObjectTypeID(view) == int(allowed ? 0xB1014135 : 0xB1014134),
            "the mission save-menu command selected the wrong actual native view");
        CObj<NMainLoop::CICExitModal> close = new NMainLoop::CICExitModal; close->Exec();
        NWorld::IPlayer::CUnitSet after; mission->GetActivePlayer()->GetPlayer()->GetUnits(&after);
        Require(NMainLoop::GetCurrentInterfaceForHarness() == mission.GetPtr() && !NMainLoop::HaveInterfaceCommand() && mission->GetWorld() == world &&
            world->GetAimTime()->GetValue() == worldTime && after == party && game->pDifficulty.GetPtr() == difficulty,
            "opening and closing a mission save-menu changed the world, party or difficulty");
    }
    std::puts("actual tactical mission Save-menu false/true selected native Load/Save views; world/party/difficulty preserved");
    NMainLoop::DoneInterface();
}

static void MissionLoadingInput(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"mission loading input fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"mission loading input fixture started with an interface or pending command");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm loading input regression");
    Require(NInput::InitInput(true),"mission loading input fixture could not initialize native SDL input");
    struct InputCleanup { ~InputCleanup() { NMainLoop::DoneInterface(); NInput::DoneInput(); } } inputCleanup;
    NMainLoop::CreateProfile("UpstreamLoadingInput"); NMainLoop::SetActiveProfile("UpstreamLoadingInput");
    auto* manager = NMainLoop::GetSaveManager();
    struct SlotCleanup { NMainLoop::CSaveManager* manager; ~SlotCleanup() { manager->DeleteSlot(NMainLoop::S_SLOT_ACTIVE); } } slotCleanup{manager};
    NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(-1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginMission> enter = new NGame::CICBeginMission(810,-1,vector<string>(),game);
    enter->Exec();
    CObj<NGame::CMission> mission = dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(mission) && IsValid(mission->GetWorld()),"loading input fixture did not open a real native tactical mission");
    CObj<NMainLoop::CInterfaceCommand> snapshot = NMainLoop::TakeInterfaceCommandForHarness();
    Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) != nullptr && !NMainLoop::HaveInterfaceCommand(),
        "loading input fixture did not isolate the actual deferred restart snapshot");
    NInput::CBind released("native_loading_released"), held("native_loading_held"), once("native_loading_once");
    NInput::SetSection("native_loading_fixture");
    NGlobal::ProcessCommand(L"bind +native_loading_released 'A'");
    NGlobal::ProcessCommand(L"bind +native_loading_held 'B'");
    NInput::UpdateBinds();
    auto pump = [&]() { S2Platform::Delay(2); S2Platform::PumpEvents(); NInput::PumpMessages(true); };
    auto drain = [&]() { NInput::SEvent event; while (NInput::GetEvent(&event)) { released.ProcessEvent(event); held.ProcessEvent(event); once.ProcessEvent(event); } };
    pump(); drain();
    SDL_Event focus{}; focus.type = SDL_EVENT_WINDOW_FOCUS_GAINED;
    focus.window.windowID = SDL_GetWindowID(S2Platform::Window());
    Require(SDL_PushEvent(&focus),"loading input fixture could not queue its native focus event");
    pump(); drain();
    auto key = [&](SDL_Scancode scan,bool down) {
        SDL_Event event{}; event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
        event.key.windowID = SDL_GetWindowID(S2Platform::Window()); event.key.scancode = scan; event.key.down = down;
        Require(SDL_PushEvent(&event),"loading input fixture could not queue a native SDL key");
    };
    key(SDL_SCANCODE_A,true); key(SDL_SCANCODE_B,true); pump(); drain();
    Require(released.IsActive() && held.IsActive(),"loading input fixture did not activate its actual native bound controls");
    key(SDL_SCANCODE_A,false); NInput::PostEvent("native_loading_once");
    const STime worldTime = mission->GetWorld()->GetAimTime()->GetValue();
    NWorld::IPlayer::CUnitSet party; mission->GetActivePlayer()->GetPlayer()->GetUnits(&party);
    snapshot->Exec();
    {
        CFileStream saved; saved.OpenRead(manager->GetSlotFilePathW(NMainLoop::S_SLOT_ACTIVE,"restart.sav").c_str());
        Require(saved.GetSize() > 1024,"native restart completion did not write its real mission snapshot");
    }
    Require(!released.IsActive(),"restart snapshot completion did not consume a key release accumulated during loading");
    Require(held.IsActive(),"restart completion discarded a legitimately held native control");
    NInput::SEvent event;
    Require(!NInput::GetEvent(&event),"restart snapshot completion leaked loading-time input into the mission");
    NWorld::IPlayer::CUnitSet after; mission->GetActivePlayer()->GetPlayer()->GetUnits(&after);
    Require(mission->GetWorld()->GetAimTime()->GetValue() == worldTime && after == party,
        "clearing loading input advanced the native world or replaced the party");
    key(SDL_SCANCODE_A,true); pump();
    bool fresh = false;
    while (NInput::GetEvent(&event)) fresh = released.ProcessEvent(event) || fresh;
    Require(fresh && released.IsActive() && held.IsActive(),"finishing loading disabled subsequent native input");
    NInput::PostEvent("native_loading_once");
    Require(NInput::GetEvent(&event) && once.ProcessEvent(event),"finishing loading disabled subsequent posted input events");
    // Our normal SDL polling deliberately skips dt==0 frames. An explicit purge
    // must still consume releases; the save itself usually masks this timing.
    int sameTickPurges = 0;
    for (int attempt = 0; attempt < 128 && sameTickPurges < 8; ++attempt) {
        key(SDL_SCANCODE_A,true); pump(); drain();
        Require(released.IsActive() && held.IsActive(),"immediate input purge fixture lost its held controls");
        const auto tick = S2Platform::Milliseconds();
        NInput::PumpMessages(true);
        key(SDL_SCANCODE_A,false); NInput::PostEvent("native_loading_once");
        NInput::PurgeEvents();
        const bool sameTick = S2Platform::Milliseconds() == tick;
        const bool empty = !NInput::GetEvent(&event);
        Require(!released.IsActive() && held.IsActive() && empty,
            "explicit native input purge skipped a same-tick release or leaked a loading action");
        if (sameTick) ++sameTickPurges;
    }
    Require(sameTickPurges == 8,"immediate input purge fixture did not exercise the native same-tick polling guard");
    std::printf("native restart snapshot and %d same-tick input purges completed without advancing the world\n",sameTickPurges);
    key(SDL_SCANCODE_A,false); key(SDL_SCANCODE_B,false); pump(); drain();
    Require(!released.IsActive() && !held.IsActive(),"native loading input controls did not release after their fresh input");
    NMainLoop::DoneInterface();
}

static void MainMenuMapScript(const char* databasePath,const char* resourcePath,bool authored = false,bool cadence = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"main-menu script fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"main-menu script fixture started with an active interface");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm menu script regression");
    auto* mapScript = NDb::GetDBScript(10);
    auto* legacyScript = NDb::GetDBScript(85);
    Require(IsValid(mapScript) && IsValid(legacyScript),"main-menu script fixture has no authored menu scripts");
    std::printf("menu script10=%zu bytes; legacy85=%zu bytes; authored=%d\n",mapScript->strCode.size(),legacyScript->strCode.size(),int(authored));
    bool authoredVariant = false;
    CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
    while (variants.MoveNext()) if (IsValid(variants.Get()->pTemplate) && variants.Get()->pTemplate->GetRecordID() == 2425 &&
        variants.Get()->pScript.GetPtr() == mapScript) authoredVariant = true;
    Require(authoredVariant,"the authored menu template does not reference the expected map script");
    struct Restore {
        CPtr<NDb::CScript> map, legacy;
        string mapCode, legacyCode;
        ~Restore() { if (IsValid(map)) map->strCode = mapCode; if (IsValid(legacy)) legacy->strCode = legacyCode; }
    } restore{mapScript,legacyScript,mapScript->strCode,legacyScript->strCode};
    const string counter = "MenuMapRuns = (MenuMapRuns or 0) + 1; MenuEnters = 0; "
        "function OnEnterZone() MenuEnters = MenuEnters + 1 end; ";
    mapScript->strCode = counter;
#if defined(S2_UPSTREAM_NATIVE_SFX)
    CObj<NFMSound::CSample2D> sample;
    CObj<NSound::CFileSample2D> loader;
#endif
    if (authored) {
        mapScript->strCode += "MenuNativeAnimationCalls = 0; MenuNativePlay = ObjectPlayAnimation; "
            "function ObjectPlayAnimation(object,animation) MenuNativeAnimationCalls = MenuNativeAnimationCalls + 1; "
            "MenuLastNativeAnimation = animation; return MenuNativePlay(object,animation) end; "+restore.mapCode;
#if defined(S2_UPSTREAM_NATIVE_SFX)
        Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr,"authored menu fixture requires headless native audio");
        Require(NFMSound::Init(NFMSound::SStartInfo()),"authored menu fixture could not initialize native audio");
        cleanup.audio = true; NFMSound::SetMusicMasterVolume(0);
        auto* ambience = NDb::GetSound(15691);
        Require(ambience != nullptr,"authored menu fixture has no ambient sound record");
        const auto wave = SilentPCMFixture(44100);
        sample = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
        Require(IsValid(sample),"authored menu fixture could not decode native PCM");
        loader = NSound::share2DSamples.Get(ambience->GetRecordID());
        (loader.GetPtr()->*CachedChapterSampleAccess::ValueMember()) = sample.GetPtr();
#else
        Require(false,"authored menu sound test requires native SFX");
#endif
    }
    legacyScript->strCode = authored ? restore.legacyCode : "MenuLegacyRuns = (MenuLegacyRuns or 0) + 1";
    CObj<NGame::CICMainMenu> open = new NGame::CICMainMenu; open->Exec();
    CObj<NGame::CMissionBase> menu = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(menu) && pSSClasses->GetObjectTypeID(menu) == int(0xB2540912),"script fixture did not open the actual native main menu");
    auto* world = dynamic_cast<NWorld::CWorld*>(menu->GetWorld());
    auto* script = world ? world->GetOwnScript() : nullptr;
    Require(IsValid(script),"the native main menu has no initialized world script");
    auto number = [&](const char* name,int expected) {
        const int top = script->GetTop(); auto value = script->GetGlobal(name);
        const bool matches = value.IsNumber() && value.GetNumber() == expected; script->SetTop(top); return matches;
    };
    auto absent = [&](const char* name) {
        const int top = script->GetTop(); const bool nil = script->GetGlobal(name).IsNil(); script->SetTop(top); return nil;
    };
    Require(number("MenuMapRuns",1),"the native main-menu initializer did not execute its variant entry exactly once");
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(1050)};
    auto frame = [&](STime time) {
        NMainLoop::SetTimeForHarness(time);
        Require(NMainLoop::StepApp(true,false,false),"native main-menu script frame requested exit");
        Require(NMainLoop::GetCurrentInterfaceForHarness() == menu.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"the main-menu script fixture changed interfaces");
    };
    if (cadence) {
        frame(1050); // Establish the renderer/world baseline before the new coroutine.
        Require(script->DoString("MenuFrameRuns = 0; while 1 do MenuFrameRuns = MenuFrameRuns + 1; Sleep(1) end") == 0,
            "menu cadence fixture could not queue its native sleeping coroutine");
        const STime frozenWorld = world->GetAimTime()->GetValue();
        for (int tick = 0; tick < 4; ++tick) frame(1050);
        Require(world->GetAimTime()->GetValue() == frozenWorld,"menu cadence fixture unexpectedly advanced its frozen world time");
        Require(absent("MenuFrameRuns"),"the main menu executed a sleeping Lua coroutine between native world segments");
        for (int tick = 0; tick < 8; ++tick) frame(1100+tick*50);
        const int top = script->GetTop(); auto value = script->GetGlobal("MenuFrameRuns");
        const bool resumed = value.IsNumber() && value.GetNumber() >= 2; script->SetTop(top);
        Require(world->GetAimTime()->GetValue() > frozenWorld && resumed,"real main-menu world segments did not resume the Lua coroutine");
        Require(number("MenuMapRuns",1) && number("MenuEnters",1),"menu cadence change repeated map initialization");
        NMainLoop::DoneInterface(); return;
    }
    for (int tick = 0; tick < 4; ++tick) frame(1050);
    if (!authored) {
        Require(absent("MenuLegacyRuns"),"the native main menu launched an extra DB script after its map entry was already initialized");
        for (int tick = 0; tick < 8; ++tick) frame(1100+tick*50);
    } else {
        auto* man = dynamic_cast<NWorld::CAnimObjectServerBase*>(world->GetObjectByName("Man"));
        Require(IsValid(man),"authored menu fixture has no animated Man object");
        bool performing = man->IsPerformingAction(); int calls = 0;
        for (int tick = 0; tick < 200; ++tick) {
            frame(1100+tick*150); performing = performing || man->IsPerformingAction();
            const int top = script->GetTop(); auto value = script->GetGlobal("MenuNativeAnimationCalls");
            calls = value.IsNumber() ? int(value.GetNumber()) : 0; script->SetTop(top);
            if (calls >= 3) break;
        }
        std::printf("authored animation calls=%d, performing=%d\n",calls,int(performing));
        Require(performing && calls == 3 && number("MenuLastNativeAnimation",2521),
            "the extra menu script duplicated the authored native Man animation sequence");
#if defined(S2_UPSTREAM_NATIVE_SFX)
        int channel = -1;
        for (int i = 0; i < 128; ++i) if (FSOUND_IsPlaying(i)) {
            Require(channel == -1,"authored menu fixture has unexpected extra native sound channels"); channel = i;
        }
        Require(channel >= 0,"authored map script10 did not start its native ambient sound");
        vector<float> output(512*2);
        Require(S2FmodRenderForTest(output.data(),512) != 0 && FSOUND_GetCurrentPosition(channel) > 0,
            "authored menu ambience did not advance real native PCM");
#endif
    }
    Require(number("MenuMapRuns",1) && number("MenuEnters",1),"native main-menu frames reran their map entry or OnEnterZone");
    NMainLoop::DoneInterface();
}

static void ChapterActiveScript(const char* databasePath,const char* resourcePath,int commandMode = 0)
{
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "active chapter fixture started with an interface or pending command");
    NativeDisplayContext cleanup;
    cleanup.Initialize(databasePath,resourcePath,"Silent Storm chapter script regression");
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"active chapter fixture has no authored map");
    struct ScriptRecordRestore {
        CPtr<NDb::CChapterMap> map; CPtr<NDb::CScript> script;
        ~ScriptRecordRestore() { if (IsValid(map)) map->pScript = script; }
    } restore{map,map->pScript};
    CObj<NDb::CScript> entry = new NDb::CScript;
    entry->strCode = "ChapterStarts = (ChapterStarts or 0) + 1; ChapterEnters = 0; "
        "function OnEnterZone() ChapterEnters = ChapterEnters + 1 end; Sleep(2); ChapterResumed = 23";
    map->pScript = entry;
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CPtr<NScenario::CScenarioZone> destination;
#if defined(S2_UPSTREAM_NATIVE_SFX)
    CObj<NFMSound::CSample2D> audioSample2D;
    CObj<NFMSound::CSample3D> audioSample3D;
    CObj<NSound::CFileSample2D> audioLoader2D;
    CObj<NSound::CFileSample3D> audioLoader3D;
#endif
    string chapterPlay;
    if (commandMode >= 6) {
#if defined(S2_UPSTREAM_NATIVE_SFX)
        Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr,"active chapter sound fixture requires headless native audio");
        Require(NFMSound::Init(NFMSound::SStartInfo()),"active chapter sound fixture could not initialize native audio");
        cleanup.audio = true; NFMSound::SetMusicMasterVolume(0);
        CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
        Require(sounds.MoveNext(),"active chapter sound fixture has no native sound record");
        auto* record = sounds.Get(); record->bLoop = true; record->nStartSamples = record->nEndingSamples = 0;
        const auto wave = SilentPCMFixture(44100);
        audioSample2D = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
        audioSample3D = NFMSound::LoadSample3D(wave.data(),static_cast<int>(wave.size()),1.f,100.f,0);
        Require(IsValid(audioSample2D) && IsValid(audioSample3D),"active chapter sound fixture could not decode its native PCM samples");
        audioLoader2D = NSound::share2DSamples.Get(record->GetRecordID());
        audioLoader3D = NSound::share3DSamples.Get(record->GetRecordID());
        (audioLoader2D.GetPtr()->*CachedChapterSampleAccess::ValueMember()) = audioSample2D.GetPtr();
        (audioLoader3D.GetPtr()->*CachedEffectSampleAccess::ValueMember()) = audioSample3D.GetPtr();
        chapterPlay = commandMode % 2 == 0 ? "PlaySound("+std::to_string(record->GetRecordID())+")" :
            "Play3DSound("+std::to_string(record->GetRecordID())+",SoundPosition)";
        entry->strCode = "ChapterHandle = "+chapterPlay+"; ChapterRequested = 1";
        if (commandMode >= 8) {
            CDBIterator<NDb::CUIHint> hints(*NDatabase::GetTable<NDb::CUIHint>());
            int hintID = 0;
            while (hints.MoveNext()) if (IsValid(hints.Get()->pString)) { hintID = hints.Get()->GetRecordID(); break; }
            Require(hintID != 0,"queued chapter sound fixture has no authored hint");
            game->hintsSet.clear();
            entry->strCode += "; ChapterHint = ShowHint("+std::to_string(hintID)+")";
        }
#else
        Require(false,"active chapter sound test requires the native SFX backend");
#endif
    } else if (commandMode == 4) {
        destination = game->pScenarioTracker->GetZoneByName("ffight");
        Require(IsValid(destination),"chapter command fixture has no authored final fight zone");
        entry->strCode = "BeginZone('ffight'); ChapterRequested = 1";
    } else if (commandMode == 5) {
        entry->strCode = "ChapterRequested = 1; uiShowTeamMngMenu()";
    } else if (commandMode != 0) {
        CDBIterator<NDb::CUIHint> hints(*NDatabase::GetTable<NDb::CUIHint>());
        int hintID = 0;
        while (hints.MoveNext()) if (IsValid(hints.Get()->pString)) { hintID = hints.Get()->GetRecordID(); break; }
        Require(hintID != 0,"chapter command fixture has no authored hint");
        game->hintsSet.clear();
        entry->strCode = "ChapterRequested = 1; ChapterHint = ShowHint(" + std::to_string(hintID) +
            "); while IsUIActionIDPresent(ChapterHint) do Sleep(1) end; ChapterCommandFinished = 9";
    }
    struct HintOptionRestore {
        NGlobal::CValue previous;
        ~HintOptionRestore() { NGlobal::SetVar("ui_showhints",previous); }
    } option{NGlobal::GetVar("ui_showhints",1)};
    if (commandMode != 0) NGlobal::SetVar("ui_showhints",commandMode == 2 || commandMode >= 8 ? 1 : 0);
    // Startup notifications would legitimately cover the chapter on its second
    // painted frame; this fixture isolates the native chapter's script pump.
    list<CPtr<NScenario::CScenarioClue>> clues; game->pScenarioTracker->GetAvailableClues(&clues);
    for (auto clue: clues) if (IsValid(clue)) clue->SetJustFound(false);
    for (auto merc: game->players.front()->mercs) { vector<CDBPtr<NDb::CMedal>> medals; merc->GetJustFoundMedals(&medals); }
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game); enter->Exec();
    CObj<NGame::CMissionBase> chapter = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* world = chapter ? dynamic_cast<NWorld::CWorld*>(chapter->GetWorld()) : nullptr;
    Require(world && IsValid(world->GetOwnScript()),"active chapter fixture did not create its native world/script");
    if (commandMode == 3) {
        bool tutorial = true; CMemoryStream base;
        { CStructureSaver writer(base,CStructureSaver::WRITE); writer.Add(31,&tutorial); }
        base.Seek(0);
        { CStructureSaver reader(base,CStructureSaver::READ); chapter->NGame::CMissionBase::operator&(reader); }
    }
    auto* script = world->GetOwnScript();
    if (commandMode >= 6) {
        CObj<NScript::CLUAObjectPosition> position = new NScript::CLUAObjectPosition(CVec3(0,0,0));
        NScript::luaPushCObj(script->GetState(),position); script->SetGlobal("SoundPosition");
    }
    auto number = [&](const char* name,int expected) {
        const int top = script->GetTop(); auto value = script->GetGlobal(name);
        const bool matches = value.IsNumber() && value.GetNumber() == expected; script->SetTop(top); return matches;
    };
    auto absent = [&](const char* name) {
        const int top = script->GetTop(); const bool nil = script->GetGlobal(name).IsNil(); script->SetTop(top); return nil;
    };
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(1000)};
    auto frame = [&](STime time,bool active) {
        NMainLoop::SetTimeForHarness(time);
        Require(NMainLoop::StepApp(active,false,false),"native chapter frame unexpectedly requested application exit");
        const bool stable = NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr() && !NMainLoop::HaveInterfaceCommand();
        if (!stable) {
            auto* current = NMainLoop::GetCurrentInterfaceForHarness();
            std::printf("chapter fixture mode=%d time=%u active=%s depth=%d queued=%d\n",commandMode,unsigned(time),
                current ? typeid(*current).name() : "null",NMainLoop::GetInterfaceStackDepth(),int(NMainLoop::HaveInterfaceCommand()));
            if (NMainLoop::HaveInterfaceCommand()) {
                CObj<NMainLoop::CInterfaceCommand> command = NMainLoop::TakeInterfaceCommandForHarness();
                std::printf("chapter fixture unexpected command=%s\n",typeid(*command.GetPtr()).name());
            }
        }
        Require(stable,"active chapter fixture was covered by an unrelated interface command");
    };
    frame(1000,false);
    Require(world->GetAimTime()->GetValue() == 0 && absent("ChapterStarts"),"an inactive chapter executed its queued world script");
    if (commandMode >= 6) {
#if defined(S2_UPSTREAM_NATIVE_SFX)
        auto channelExcept = [&](int except) {
            int found = -1;
            for (int i = 0; i < 128; ++i) if (i != except && FSOUND_IsPlaying(i)) {
                Require(found == -1,"active chapter sound fixture has an unexpected extra native channel"); found = i;
            }
            return found;
        };
        Require(channelExcept(-1) == -1,"an inactive chapter started its scripted native audio");
        if (commandMode >= 8) {
            NMainLoop::SetTimeForHarness(1050);
            Require(NMainLoop::StepApp(true,false,false) && NMainLoop::HaveInterfaceCommand() &&
                NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr(),
                "the chapter did not queue its native hint alongside its scripted sound");
        } else frame(1050,true);
        Require(number("ChapterRequested",1),"the active chapter never executed its native Lua sound request");
        const int top = script->GetTop(); auto handle = script->GetGlobal("ChapterHandle");
        auto* soundCommand = handle.IsUserData() ? dynamic_cast<NWorld::CUICmdPlaySound*>(handle.GetUserData()) : nullptr;
        Require(soundCommand && IsValid(soundCommand->pChannel),
            "the native chapter sound dispatcher never created its scene-owned channel wrapper");
        script->SetTop(top);
        const int played = channelExcept(-1);
        Require(played >= 0,"the active chapter created a script sound wrapper but never started native PCM playback");
        Require(FSOUND_GetCurrentPosition(played) == 0,"fresh chapter script audio inherited absolute frame time as its sample offset");
        vector<float> output(512*2);
        auto render = [&]() { Require(S2FmodRenderForTest(output.data(),512) != 0,"active chapter sound fixture could not render native PCM"); };
        render();
        Require(FSOUND_GetCurrentPosition(played) > 0,"active chapter sound never advanced its real native PCM cursor");
        CObj<NFMSound::CSound2D> independent = NFMSound::PlaySound(audioSample2D);
        const int control = channelExcept(played);
        Require(IsValid(independent) && control >= 0,"active chapter sound fixture could not start its independent native control");
        const unsigned before = FSOUND_GetCurrentPosition(played), other = FSOUND_GetCurrentPosition(control);
        CObj<NGame::CICExitMenu> menu;
        if (commandMode >= 8) {
            NMainLoop::SetTimeForHarness(1100);
            Require(NMainLoop::StepApp(true,false,false),"queued chapter sound fixture could not open its native hint");
            auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
            Require(modal && pSSClasses->GetObjectTypeID(modal) == int(0x1423130),"queued chapter sound fixture opened the wrong interface");
        } else { menu = new NGame::CICExitMenu; menu->Exec(); }
        Require(NMainLoop::GetInterfaceStackDepth() == 2,"chapter sound fixture did not open its native modal");
        render();
        Require(FSOUND_IsPlaying(played) && FSOUND_GetCurrentPosition(played) == before && FSOUND_GetCurrentPosition(control) > other,
            "covering the active chapter failed to pause only its own playing audio");
        if (commandMode >= 8) {
            NInput::SEvent input; while (NInput::GetEvent(&input)) {}
            NInput::PostEvent("cancel");
            Require(NInput::GetEvent(&input) && NMainLoop::GetCurrentInterfaceForHarness()->ProcessEvent(input),
                "queued chapter sound fixture could not close its actual hint");
            NMainLoop::SetTimeForHarness(1150);
            Require(NMainLoop::StepApp(false,false,false),"queued chapter sound fixture did not restore its chapter");
        } else {
            CObj<NMainLoop::CICExitModal> close = new NMainLoop::CICExitModal; close->Exec();
        }
        render();
        Require(NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr() && FSOUND_GetCurrentPosition(played) > before,
            "returning from the native menu did not resume the chapter's real playback");
        Require(script->DoString("StopSound(ChapterHandle); ChapterStopped = 1") == 0,"chapter sound fixture could not queue its native Lua stop request");
        for (int tick = 0; tick < 8; ++tick) frame(1200+tick*50,true);
        render();
        Require(number("ChapterStopped",1) && !FSOUND_IsPlaying(played),"active chapter Lua StopSound left its actual native channel playing");
        Require(FSOUND_IsPlaying(control) && FSOUND_GetCurrentPosition(control) > other,
            "chapter StopSound interrupted the independent native control");
        Require(script->DoString(("ChapterHandle = "+chapterPlay+"; ChapterRestarted = 1").c_str()) == 0,
            "chapter sound fixture could not queue its native restart request");
        for (int tick = 0; tick < 8; ++tick) frame(1600+tick*50,true);
        const int restarted = channelExcept(control);
        Require(number("ChapterRestarted",1) && restarted >= 0,"chapter audio failed to start a new native channel after StopSound");
        render();
        Require(FSOUND_GetCurrentPosition(restarted) > 0,"restarted chapter audio did not advance native PCM");
        NMainLoop::DoneInterface(); chapter = nullptr; ClearHoldQueue(); render();
        Require(!FSOUND_IsPlaying(restarted) && FSOUND_IsPlaying(control),
            "closing the chapter leaked its scripted audio or stopped the independent control");
#endif
        return;
    }
    if (commandMode != 0) {
        auto step = [&](STime time) {
            NMainLoop::SetTimeForHarness(time);
            Require(NMainLoop::StepApp(true,false,false),"native chapter command frame requested application exit");
        };
        vector<CPtr<NWorld::CUnit>> originalUnits;
        world->GetAllUnits(&originalUnits);
        const auto originalMercs = game->players.front()->mercs;
        step(1050);
        Require(number("ChapterRequested",1),"chapter command fixture never executed its native Lua request");
        if (commandMode == 5) {
            Require(NMainLoop::HaveInterfaceCommand(),"the active chapter ignored its native Lua roster command");
            step(1100);
            auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
            Require(modal && pSSClasses->GetObjectTypeID(modal) == int(0xB0925140) && NMainLoop::GetInterfaceStackDepth() == 2,
                "the chapter roster command did not open the existing native team interface");
            NInput::SEvent input; while (NInput::GetEvent(&input)) {}
            NInput::PostEvent("cancel");
            Require(NInput::GetEvent(&input) && modal->ProcessEvent(input),"the native chapter roster did not consume its close event");
            for (int tick = 0; tick < 12; ++tick) step(1200+tick*50);
            vector<CPtr<NWorld::CUnit>> afterUnits; world->GetAllUnits(&afterUnits);
            Require(afterUnits == originalUnits && game->players.front()->mercs == originalMercs,
                "closing the native chapter roster duplicated or replaced the unchanged deployed party");
            vector<CPtr<NGame::IUnitTracker>> tracked; chapter->GetUnits(&tracked);
            Require(tracked.size() == originalUnits.size() && !tracked.empty(),
                "chapter roster dispatch left deployed party units without native player trackers");
            Require(NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr() && !NMainLoop::HaveInterfaceCommand(),
                "the closed native roster did not return to the original chapter");
        } else if (commandMode == 4) {
            Require(NMainLoop::HaveInterfaceCommand(),"the active chapter ignored its native Lua BeginZone transition");
            CObj<NMainLoop::CInterfaceCommand> queued = NMainLoop::TakeInterfaceCommandForHarness();
            auto* begin = dynamic_cast<NGame::CICBeginMission*>(queued.GetPtr());
            Require(begin && (begin->*BeginMissionCommandAccess::Zone()) == destination &&
                (begin->*BeginMissionCommandAccess::Game()) == game && (begin->*BeginMissionCommandAccess::Template()) == -1,
                "chapter BeginZone lost its actual scenario destination, campaign or default template");
            Require(!(begin->*BeginMissionCommandAccess::EmulatesChapter()),
                "chapter BeginZone would repeat tactical leave-zone healing/scenario/medal bookkeeping");
            Require((begin->*BeginMissionCommandAccess::Image()) == map->pPWLImage,
                "scripted chapter entry discarded the remaster's chapter loading background");
            Require(!NMainLoop::HaveInterfaceCommand(),"one chapter BeginZone queued multiple transitions");
            CObj<ChapterZoneDrainMission> tactical = new ChapterZoneDrainMission;
            tactical->Attach(world,game,chapter->GetDesktop());
            world->AddUICommand(new NWorld::CUICmdBeginZone("ffight")); tactical->Drain();
            queued = NMainLoop::TakeInterfaceCommandForHarness(); begin = dynamic_cast<NGame::CICBeginMission*>(queued.GetPtr());
            Require(begin && (begin->*BeginMissionCommandAccess::Zone()) == destination &&
                (begin->*BeginMissionCommandAccess::Game()) == game && (begin->*BeginMissionCommandAccess::EmulatesChapter()),
                "sharing BeginZone dispatch broke the existing tactical leave-zone transition");
            Require(!NMainLoop::HaveInterfaceCommand(),"one tactical BeginZone queued multiple transitions");
        } else {
            const int top = script->GetTop(); auto hint = script->GetGlobal("ChapterHint");
            Require(hint.IsNumber(),"the actual chapter ShowHint binding returned no wait ID");
            const int eventID = int(hint.GetNumber()); script->SetTop(top);
            Require(script->IsUIActionIDPresent(eventID) && absent("ChapterCommandFinished"),
                "chapter hint bypassed its actual asynchronous UI wait");
            if (commandMode >= 2) {
                Require(NMainLoop::HaveInterfaceCommand(),"the active chapter ignored its native Lua hint command");
                step(1100);
                auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
                Require(modal && pSSClasses->GetObjectTypeID(modal) == int(0x1423130) && NMainLoop::GetInterfaceStackDepth() == 2,
                    "chapter ShowHint did not open the existing native hint interface");
                CStructureNetworkContext references; auto fields = BorrowedObjectFields(modal,&references);
                CPtr<NGame::IMission> origin; int modalEvent = -1;
                { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(2,&origin); reader.Add(3,&modalEvent); }
                Require(origin.GetPtr() == chapter.GetPtr() && modalEvent == eventID,
                    "the native chapter hint lost its origin or pending script event");
                const STime coveredTime = world->GetAimTime()->GetValue(); step(1150); step(1200);
                Require(world->GetAimTime()->GetValue() == coveredTime && script->IsUIActionIDPresent(eventID),
                    "the covered chapter advanced its world or prematurely released its hint wait");
                NInput::SEvent input; while (NInput::GetEvent(&input)) {}
                NInput::PostEvent("cancel");
                Require(NInput::GetEvent(&input) && modal->ProcessEvent(input),"native chapter hint did not consume its close event");
                step(1250);
                Require(NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr() && NMainLoop::GetInterfaceStackDepth() == 1,
                    "closing the hint did not restore its actual chapter interface");
            } else Require(!NMainLoop::HaveInterfaceCommand(),"disabled chapter hints opened a modal");
            for (int tick = 0; tick < 12; ++tick) step(1300+tick*50);
            Require(!script->IsUIActionIDPresent(eventID) && number("ChapterCommandFinished",9),
                "the chapter hint command never released its real Lua UI wait");
        }
        Require(game->nChapterMapID == map->GetRecordID() && game->players.size() == 1,
            "chapter command dispatch changed campaign destination or party before a transition");
        NMainLoop::DoneInterface(); return;
    }
    frame(1050,true);
    Require(number("ChapterStarts",1) && world->GetAimTime()->GetValue() > 0,
        "the real active chapter Step never advanced its world or executed the assigned Lua script");
    Require(absent("ChapterResumed"),"the chapter script bypassed its native Sleep segment delay");
    const STime covered = world->GetAimTime()->GetValue();
    frame(1100,false); frame(1150,false);
    Require(world->GetAimTime()->GetValue() == covered && absent("ChapterResumed"),"inactive frames advanced a sleeping chapter's simulation");
    chapter->PauseGame(true); frame(1200,true); frame(1250,true);
    Require(world->GetAimTime()->GetValue() == covered && absent("ChapterResumed"),"active paused chapter frames advanced their sleeping world");
    chapter->PauseGame(false);
    for (int tick = 0; tick < 8; ++tick) frame(1300+tick*50,true);
    Require(number("ChapterResumed",23) && number("ChapterStarts",1) && number("ChapterEnters",1),
        "active chapter frames did not resume Sleep or reran initialization/OnEnterZone");
    Require(game->bChapterMapSet && game->nChapterMapID == map->GetRecordID() && game->players.size() == 1,
        "the native chapter pump changed campaign destination or party");
    NMainLoop::DoneInterface();
}

static void ChapterClockStep(const char* databasePath,const char* resourcePath,const char* legacyPath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NGScene::Is3DActive(),
        "chapter clock fixture needs an inactive native chapter with no render device");
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); } } cleanup;
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(1000)};
    int chapterID = 0;
    { CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
      while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { chapterID = maps.Get()->GetRecordID(); break; } }
    Require(chapterID != 0,"chapter clock fixture has no authored chapter map");
    auto roundTrip = [&](CObj<NRPG::CGlobalGame>& game,CObj<NGame::CMissionBase>& chapter) {
        CMemoryStream snapshot;
        { CStructureSaver writer(snapshot,CStructureSaver::WRITE); writer.Add(2,&game); writer.Add(3,&chapter); }
        snapshot.Seek(0);
        CObj<NRPG::CGlobalGame> loadedGame; CObj<NGame::CMissionBase> loaded;
        { CStructureSaver reader(snapshot,CStructureSaver::READ); reader.Add(2,&loadedGame); reader.Add(3,&loaded); }
        Require(IsValid(loadedGame) && IsValid(loaded) && loaded->GetWorld() != chapter->GetWorld(),
            "chapter clock snapshot did not restore an independent complete graph");
        NMainLoop::DoneInterface(); chapter = loaded; game = loadedGame;
        chapter->OnSnapshotRestored();
    };
    // Fresh chapter; actual old null-world graph; previous complete fork save
    // with a zero interval; and a saved custom nonzero interval to retain.
    for (int variant = 0; variant < 4; ++variant) {
        NMainLoop::DoneInterface(); NMainLoop::SetTimeForHarness(1000);
        CObj<NRPG::CGlobalGame> game; CObj<NGame::CMissionBase> chapter;
        const STime interval = variant == 3 ? 7 : 5;
        if (variant == 1) {
            CFileStream legacy; legacy.OpenRead(legacyPath);
            { CStructureSaver reader(legacy,CStructureSaver::READ); reader.Add(2,&game); reader.Add(3,&chapter); }
            Require(IsValid(chapter) && !chapter->GetWorld(),"chapter clock fixture did not load its historical null-world graph");
            chapter->OnSnapshotRestored();
        } else {
            game = NRPG::CreateGlobalGame(1);
            game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
            CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(chapterID,game);
            enter->Exec();
            chapter = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
            if (variant >= 2) {
                STime savedInterval = variant == 2 ? 0 : interval;
                CMemoryStream base;
                { CStructureSaver writer(base,CStructureSaver::WRITE); writer.Add(36,&savedInterval); }
                base.Seek(0);
                { CStructureSaver reader(base,CStructureSaver::READ); chapter->NGame::CMissionBase::operator&(reader); }
                roundTrip(game,chapter);
            }
        }
        Require(IsValid(chapter) && chapter->GetWorld() && chapter->GetWorld()->GetAimTime()->GetValue() == 0,
            "chapter clock fixture lacks its native world or ran a tactical warm-up");
        auto* ui = dynamic_cast<NUI::CChapterMapUI*>(chapter->GetInterface()->GetChildByID("chaptermapUI"));
        Require(ui != nullptr,"chapter clock fixture lost its actual saved chapter UI");
        const string travel = ui->GetTravelForDiagnostics();
        const int mapID = game->nChapterMapID, difficulty = game->nCurrentChapterDifficulty;
        const auto mercs = game->players.front()->mercs;
        auto step = [&](STime time) { NMainLoop::SetTimeForHarness(time); chapter->Step(); };
        step(1000);
        Require(chapter->GetGameTime() == 0 && chapter->GetUITime() == 0,"the first chapter clock step jumped instead of establishing a baseline");
        step(1003);
        Require(chapter->GetGameTime() == 0 && chapter->GetUITime() == 0,"the chapter frame gate accepted an update before its saved minimum interval");
        step(1000+interval);
        Require(chapter->GetGameTime() == interval && chapter->GetUITime() == interval,
            "the native chapter Step did not advance its actual game and interface clocks");
        chapter->PauseGame(true); step(1000+2*interval);
        Require(chapter->GetGameTime() == interval && chapter->GetUITime() == 2*interval,
            "pausing the native chapter froze its interface clock or advanced its game clock");
        chapter->PauseGame(false); step(1000+4*interval);
        Require(chapter->GetGameTime() == 3*interval && chapter->GetUITime() == 4*interval,
            "resuming the native chapter replayed paused time or lost its resumed interval");
        Require(game->nChapterMapID == mapID && game->nCurrentChapterDifficulty == difficulty && game->players.front()->mercs == mercs &&
            chapter->GetWorld()->GetAimTime()->GetValue() == 0 && ui->GetTravelForDiagnostics() == travel,
            "inactive chapter clock bookkeeping changed its world, campaign, party or saved travel");
        roundTrip(game,chapter);
        Require(chapter->GetGameTime() == 3*interval && chapter->GetUITime() == 4*interval,
            "loading a complete chapter reset its saved interface/game clock values");
        step(2000);
        Require(chapter->GetGameTime() == 3*interval && chapter->GetUITime() == 4*interval,
            "the restored chapter replayed an unsaved wall-time gap");
        step(2000+interval);
        Require(chapter->GetGameTime() == 4*interval && chapter->GetUITime() == 5*interval &&
            chapter->GetWorld()->GetAimTime()->GetValue() == 0,
            "the restored native chapter clocks failed to continue without simulating the inactive world");
    }
    NMainLoop::DoneInterface();
}

static void ChapterFocusAudio(const char* databasePath,const char* resourcePath)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr,"chapter-focus fixture requires headless native audio");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "chapter-focus fixture started with an active interface or pending command");
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    Require(NFMSound::Init(NFMSound::SStartInfo()),"chapter-focus fixture could not initialize native audio");
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); ClearHoldQueue(); NFMSound::Done(); } } cleanup;
    NFMSound::SetMusicMasterVolume(0);
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"chapter-focus fixture has no authored chapter map");
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game);
    enter->Exec();
    CObj<NGame::CMissionBase> chapter = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(chapter) && chapter->GetSoundScene(),"chapter-focus fixture did not open its actual native chapter");
    CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
    Require(sounds.MoveNext(),"chapter-focus fixture has no native sound record");
    auto* record = sounds.Get(); record->bLoop = false; record->nStartSamples = record->nEndingSamples = 0;
    const auto wave = SilentPCMFixture(44100);
    CObj<NFMSound::CSample2D> sample = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
    Require(IsValid(sample),"chapter-focus fixture could not decode its PCM sample");
    CObj<NSound::CFileSample2D> loader = NSound::share2DSamples.Get(record->GetRecordID());
    (loader.GetPtr()->*CachedChapterSampleAccess::ValueMember()) = sample.GetPtr();
    CObj<NSound::ISound2D> sound = chapter->GetSoundScene()->Add2DSound(record);
    CObj<ICamera> camera = ::CreateCamera(CAMERA_PC);
    CTransformStack transform; camera->GetTransform(&transform,CVec2(1024,768));
    chapter->GetSoundScene()->Draw(&transform);
    auto findChannel = [&](int except) {
        int found = -1;
        for (int i = 0; i < 128; ++i) if (i != except && FSOUND_IsPlaying(i)) {
            Require(found == -1,"chapter-focus fixture has unexpected extra channels"); found = i;
        }
        return found;
    };
    const int channel = findChannel(-1);
    Require(IsValid(sound) && sound->IsPlaying() && channel >= 0,"chapter scene did not start an actual native PCM channel");
    CObj<NFMSound::CSound2D> unrelated = NFMSound::PlaySound(sample);
    const int control = findChannel(channel);
    Require(IsValid(unrelated) && control >= 0,"chapter-focus fixture did not start its unrelated audio control");
    vector<float> output(512*2);
    auto render = [&]() { Require(S2FmodRenderForTest(output.data(),512) != 0,"chapter-focus fixture could not render native PCM"); };
    render();
    Require(FSOUND_GetCurrentPosition(channel) > 0,"chapter-focus fixture has no observable live playback before its menu");
    for (int pass = 0; pass < 2; ++pass) {
        const unsigned before = FSOUND_GetCurrentPosition(channel), other = FSOUND_GetCurrentPosition(control);
        CObj<NGame::CICExitMenu> menu = new NGame::CICExitMenu;
        menu->Exec();
        Require(NMainLoop::GetInterfaceStackDepth() == 2 && NMainLoop::GetCurrentInterfaceForHarness() != chapter.GetPtr(),
            "chapter-focus fixture did not cover the chapter with its actual native menu");
        render();
        Require(FSOUND_IsPlaying(channel) && FSOUND_GetCurrentPosition(channel) == before && FSOUND_GetCurrentPosition(control) > other,
            "covering a chapter did not freeze only its own live audio channel");
        CObj<NMainLoop::CICExitModal> close = new NMainLoop::CICExitModal;
        close->Exec();
        Require(NMainLoop::GetInterfaceStackDepth() == 1 && NMainLoop::GetCurrentInterfaceForHarness() == chapter.GetPtr(),
            "closing the native menu did not return focus to the chapter");
        render();
        Require(FSOUND_IsPlaying(channel) && FSOUND_GetCurrentPosition(channel) > before,
            "returning from a native menu left the chapter audio channel permanently paused");
    }
    Require(game->bChapterMapSet && game->nChapterMapID == map->GetRecordID() && game->players.size() == 1,
        "chapter focus changed campaign destination or party");
    NMainLoop::DoneInterface();
#else
    Require(false,"chapter-focus test requires the native SFX backend");
#endif
}

static void ChapterScriptInitialization(const char* databasePath,const char* resourcePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "chapter-script fixture started with an active interface or pending command");
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"chapter-script fixture has no authored chapter-map record");
    struct ScriptRecordRestore {
        CPtr<NDb::CChapterMap> map; CPtr<NDb::CScript> script;
        ~ScriptRecordRestore() { if (IsValid(map)) map->pScript = script; }
    } restore{map,map->pScript};
    CObj<NDb::CScript> entry = new NDb::CScript;
    entry->strCode = "UpstreamChapterInit = 17; UpstreamChapterEnter = 0; "
        "function OnEnterZone() UpstreamChapterEnter = UpstreamChapterEnter + 1 end";
    CObj<NScript::CScript> foreign = new NScript::CScript;
    foreign->PushNumber(99);
    struct GlobalScriptRestore { CPtr<NScript::CScript> previous; ~GlobalScriptRestore() { pScript = previous; } } globalRestore{pScript};
    auto number = [&](NScript::CScript* script,const char* name,int expected) {
        const int top = script->GetTop(); auto value = script->GetGlobal(name);
        const bool matches = value.IsNumber() && value.GetNumber() == expected;
        script->SetTop(top); return matches;
    };
    for (bool withScript: {false,true}) {
        map->pScript = withScript ? entry.GetPtr() : nullptr;
        pScript = foreign;
        CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
        game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game);
        enter->Exec();
        CObj<NGame::CMissionBase> mission = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
        auto* world = mission ? dynamic_cast<NWorld::CWorld*>(mission->GetWorld()) : nullptr;
        Require(world && IsValid(world->GetOwnScript()),"chapter-script fixture did not create its actual native world and Lua state");
        auto* own = world->GetOwnScript();
        Require(own != foreign.GetPtr() && own->pWorld == world && own->GetGlobalGame() == game.GetPtr(),
            "the chapter script was not bound to its own native world/global-game context");
        Require(world->GetAimTime()->GetValue() == 0,"chapter script initialization ran the tactical first-segments warm-up");
        if (withScript) {
            const int top = own->GetTop(); auto pending = own->GetGlobal("UpstreamChapterInit");
            Require(pending.IsNil(),"chapter initialization eagerly executed its queued Lua thread before the first live segment");
            own->SetTop(top);
            // DoString queues the body. Segment pumps it, then queues OnEnterZone;
            // that callback runs when the next native segment pumps Lua again.
            world->UpdateWorld(0,nullptr);
            Require(number(own,"UpstreamChapterInit",17),"the native chapter never evaluated its assigned ScriptID entry point");
            Require(number(own,"UpstreamChapterEnter",0),"the chapter bypassed the native deferred OnEnterZone scheduling");
            world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
            Require(number(own,"UpstreamChapterEnter",1),"the chapter did not execute its deferred OnEnterZone callback on the next native segment");
            world->UpdateWorld(world->GetAimTime()->GetValue()+50,nullptr);
            Require(number(own,"UpstreamChapterEnter",1) && number(own,"UpstreamChapterInit",17),
                "later world segments reran chapter initialization or the OnEnterZone one-shot");
        }
        const int top = foreign->GetTop(); auto other = foreign->GetGlobal("UpstreamChapterInit");
        Require(other.IsNil() && top == 1,"chapter initialization executed in an unrelated Lua state's globals or changed its stack");
        foreign->SetTop(top);
        Require(game->bChapterMapSet && game->nChapterMapID == map->GetRecordID() && game->players.size() == 1,
            "chapter-script setup changed campaign destination or party");
        NMainLoop::DoneInterface(); mission = nullptr;
    }
}

static void ChapterWorldGraph(const char* databasePath,const char* resourcePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "chapter-world fixture started with an active interface or pending command");
    struct Cleanup { ~Cleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"chapter-world fixture has no authored campaign chapter");
    const int chapterID = map->GetRecordID();
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game);
    enter->Exec();
    CObj<NGame::CMissionBase> mission = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(mission) && pSSClasses->GetObjectTypeID(mission) == int(0xB2301230),
        "chapter-world fixture did not open the actual native campaign chapter");
    auto check = [&](NGame::CMissionBase* chapter,NRPG::CGlobalGame* global) {
        auto* world = chapter->GetWorld();
        Require(IsValid(world),"the native chapter initialized without the world required by its saved mission state");
        Require(world->GetGlobalGame() == global && IsValid(world->GetAIMap()) && IsValid(world->GetPathNetwork()),
            "the native chapter world lost its global-game or AI/path infrastructure");
        auto* player = chapter->GetActivePlayer();
        Require(IsValid(player) && player->GetGlobalPlayer() == global->players.front() && IsValid(player->GetPlayer()) &&
            IsValid(player->GetCommander()) && IsValid(player->GetCamera()),
            "the native chapter did not retain its real world player, commander and player camera");
        Require(IsValid(chapter->GetScene()) && IsValid(chapter->GetRenderGame()) && IsValid(chapter->GetEnemyTurnCamera()) &&
            chapter->GetEnemyTurnCamera() != player->GetCamera(),"the native chapter did not own its scene/render and separate cinematic camera");
        auto* ui = chapter->GetInterface();
        Require(IsValid(ui) && IsValid(chapter->GetSoundScene()) && ui->GetSound() == chapter->GetSoundScene(),
            "the native chapter interface did not share its world-clock sound scene");
        Require(IsValid(chapter->GetDesktop()) && chapter->GetDesktop() == ui->GetChildByID("chaptermapUI"),
            "the native chapter UI was absent from the mission desktop stack");
        vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
        int aliveMercs = 0;
        for (const auto& merc: global->players.front()->mercs) if (!merc->IsDead()) ++aliveMercs;
        Require(int(units.size()) == aliveMercs && global->players.size() == 1 && global->bChapterMapSet && global->nChapterMapID == chapterID,
            "initializing the chapter world changed campaign destination or the deployed party roster");
        for (const auto& unit: units) {
            auto* server = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
            Require(server && std::find(global->players.front()->mercs.begin(),global->players.front()->mercs.end(),
                server->GetUnitRPG()->GetRPGUnit()) != global->players.front()->mercs.end(),
                "the native chapter world did not retain its actual global-party RPG units");
        }
    };
    check(mission,game);
    CMemoryStream snapshot;
    { CStructureSaver writer(snapshot,CStructureSaver::WRITE); writer.Add(2,&game); writer.Add(3,&mission); }
    CObj<NRPG::CGlobalGame> restoredGame;
    CObj<NGame::CMissionBase> restored;
    snapshot.Seek(0);
    { CStructureSaver reader(snapshot,CStructureSaver::READ); reader.Add(2,&restoredGame); reader.Add(3,&restored); }
    Require(IsValid(restoredGame) && IsValid(restored) && restored.GetPtr() != mission.GetPtr() &&
        restored->GetWorld() != mission->GetWorld(),"chapter-world native snapshot did not restore an independent object graph");
    check(restored,restoredGame);
    const STime before = restored->GetWorld()->GetAimTime()->GetValue();
    restored->GetWorld()->UpdateWorld(before+20,nullptr);
    Require(restored->GetWorld()->GetAimTime()->GetValue() > before,
        "the restored native chapter world could not resume its simulation clock");
    NMainLoop::DoneInterface();
    restored = nullptr; restoredGame = nullptr; mission = nullptr;
}

static void GlobalSectorDescription(const char* databasePath,const char* resourcePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ); NGScene::AddResourceDir(resourcePath);
    NDb::CUIContainer* sectorTemplate = nullptr;
    NDb::CUIControl* descriptionControl = nullptr;
    CDBIterator<NDb::CUIContainer> templates(*NDatabase::GetTable<NDb::CUIContainer>());
    while (templates.MoveNext()) {
        auto* candidate = templates.Get();
        bool normal = false, highlighted = false, normalZone = false, disabledZone = false;
        NDb::CUIControl* description = nullptr;
        for (const auto& control: candidate->controls) if (IsValid(control)) {
            normal |= control->szID == "text_normal"; highlighted |= control->szID == "text_hilighted";
            normalZone |= control->szID == "zone_normal"; disabledZone |= control->szID == "zone_disabled";
            if (control->szID == "description") description = control;
        }
        if (normal && highlighted && normalZone && description && description->type == NDb::UI_TEXT &&
            description->rect.Width() > 8 && !NUI::GetDBString(description->pString).empty()) {
            sectorTemplate = candidate; descriptionControl = description; break;
        }
    }
    Require(sectorTemplate && descriptionControl,"global description fixture has no authored sector template and localized panel caption");
    const auto caption = NUI::GetDBString(descriptionControl->pString);
    const auto originalRect = descriptionControl->rect;
    std::printf("native global description template=%d control=%d string=%d\n",sectorTemplate->GetRecordID(),
        descriptionControl->GetRecordID(),descriptionControl->pString->GetRecordID());
    const float oldScale = S2Platform::Display().uiScale;
    struct DisplayCleanup { float scale; ~DisplayCleanup() { S2Platform::UpdateDisplay(scale*100); } } cleanup{oldScale};
    for (float scale: {1.f,1.5f,2.f}) {
        S2Platform::UpdateDisplay(scale*100); MarkNewDGFrame();
        CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false);
        CObj<NUI::CInterface> interface = new NUI::CInterface(cursor);
        CObj<NUI::CWindow> sector = dynamic_cast<NUI::CWindow*>(pSSClasses->CreateObject(int(0xB0912172)));
        Require(IsValid(sector),"global description fixture could not create its native zone-sector control");
        sector->SetInterface(interface); sector->SetPosition(NUI::SPoint(0,0)); sector->SetSize(NUI::SPoint(1024,768));
        NUI::LoadTemplate(sector,sectorTemplate);
        auto* description = sector->GetChildByID("description");
        Require(IsValid(description),"native zone-sector template did not create the description panel");
        list<CPtr<NUI::CWindow>> children; description->GetChildrenList(&children);
        NUI::CText* text = dynamic_cast<NUI::CText*>(description);
        int captions = text && text->GetText() == caption ? 1 : 0;
        for (const auto& child: children) if (auto* candidate = dynamic_cast<NUI::CText*>(child.GetPtr())) {
            if (candidate->GetText() == caption) { text = candidate; ++captions; }
        }
        Require(captions == 1 && text,
            "the native global-sector description panel lost its authored localized caption");
        NUI::SPoint measured; text->GetRealSize(&measured);
        std::printf("native description scale=%.1f panel=(%d,%d) text=(%d,%d) measured=(%d,%d)\n",double(scale),
            description->GetSize().x,description->GetSize().y,text->GetSize().x,text->GetSize().y,measured.x,measured.y);
        Require(measured.y > 0 && text->GetSize().y == measured.y && description->GetSize().y == measured.y+8,
            "the native global-sector description kept its fixed height instead of fitting its actual caption and border");
        Require(text->GetPosition() == NUI::SPoint(4,4) && text->GetSize().x == description->GetSize().x-8 &&
            text->GetStyle(NUI::STYLE_VISIBLE) && text->GetText() == caption,
            "the global-sector caption lost its inset, width, visibility or authored font markup");
        Require(descriptionControl->rect.x1 == originalRect.x1 && descriptionControl->rect.y1 == originalRect.y1 &&
            descriptionControl->rect.x2 == originalRect.x2 && descriptionControl->rect.y2 == originalRect.y2 && NUI::GetDBString(descriptionControl->pString) == caption,
            "building the global-sector caption modified the authored UI rectangle or localized text");
    }
}

static void GlobalMapReturnDestination(const char* databasePath,const char* resourcePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "global-map return fixture must begin without interfaces or queued commands");
    struct InterfaceCleanup { ~InterfaceCleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CDBIterator<NDb::CGlobalMap> maps(*NDatabase::GetTable<NDb::CGlobalMap>());
    NDb::CGlobalMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pScenario)) { map = maps.Get(); break; }
    Require(map != nullptr, "global-map return fixture has no authored campaign map");
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(map->pScenario->GetRecordID());
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    game->bGlobalMapSet = true; game->nGlobalMapID = map->GetRecordID();
    game->bChapterMapSet = true; game->nChapterMapID = 987654;
    game->vChapterPos = CVec2(12,34);
    CObj<NGame::CICShowGlobal> overview = new NGame::CICShowGlobal(game);
    overview->Exec();
    auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(modal) && pSSClasses->GetObjectTypeID(modal) == int(0xB1808180) && game->bChapterMapSet,
        "modal global-map overview did not preserve its chapter return destination");
    NMainLoop::DoneInterface();
    CObj<NGame::CICContinueGlobal> leaveChapter = new NGame::CICContinueGlobal(game);
    leaveChapter->Exec();
    auto* active = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(active) && pSSClasses->GetObjectTypeID(active) == int(0xB1808180),
        "continuing the campaign did not open the actual native global map");
    Require(!game->bChapterMapSet,
        "continuing to the global map retained the old chapter return destination");
    Require(game->bGlobalMapSet && game->nGlobalMapID == map->GetRecordID() &&
        game->nChapterMapID == 987654 && game->vChapterPos == CVec2(12,34) && game->players.size() == 1,
        "clearing the chapter return flag also changed saved map IDs, position or party");
}

static void CheckChapterMarkerToolTipTransitions(NUI::CWindow* marker,NUI::CText* text);
static void CheckChapterExitMarkerHint(NGame::CMissionBase* mission,NUI::CWindow* chapter,NUI::CWindow* marker,NUI::CText* text);
static void CheckChapterClueHeading(NGame::CMissionBase* mission,NUI::CWindow* chapter);

static void ChapterTeamMarkerToolTip(const char* databasePath,const char* resourcePath,int transitions = 0)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "chapter marker fixture must begin without interfaces or queued commands");
    struct InterfaceCleanup { ~InterfaceCleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) {
        if (transitions == 3) {
            CObj<CChapterInfoLoader> loader = new CChapterInfoLoader;
            loader->SetKey(maps.Get()->GetRecordID());
            CDGPtr<CPtrFuncBase<CChapterInfo>> info = loader.GetPtr(); info.Refresh();
            bool usable = false;
            for (const auto& sector : info->GetValue()->sectorsSet) {
                if (sector.eType != ZONE || sector.nDescriptionID == -1 || sector.pointsSet.empty()) continue;
                auto* zone = game->pScenarioTracker->GetZoneByDBZone(NDb::GetDBScenarioZone(sector.nTemplate));
                if (!IsValid(zone)) continue;
                for (auto clue : zone->GetClues())
                    if (!clue->IsCompound() && clue->IsPlaced() && !clue->IsDestroyed() &&
                        !clue->GetParentZones().empty() && clue->GetParentZones().front() == zone &&
                        !NUI::GetDBString(clue->GetDBClue()->pDescription).empty()) usable = true;
            }
            if (!usable) continue;
        }
        map = maps.Get(); break;
    }
    Require(map != nullptr, "chapter marker fixture has no authored chapter map");
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game);
    enter->Exec();
    auto* mission = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(mission) && pSSClasses->GetObjectTypeID(mission) == int(0xB2301230) && mission->GetInterface(),
        "chapter marker fixture did not open the actual native chapter interface");
    auto* chapter = mission->GetInterface()->GetChildByID("chaptermapUI");
    auto* view = chapter ? chapter->GetChildByID("view") : nullptr;
    auto* marker = view ? view->GetChildByID("action") : nullptr;
    Require(IsValid(marker), "chapter marker fixture did not create its native party marker");
    auto* tooltip = marker->GetToolTip();
    Require(IsValid(tooltip), "the chapter party marker never created its saved tooltip");
    Require(tooltip->GetParent() == mission->GetInterface() && tooltip->GetStyle(NUI::STYLE_TOPMOST),
        "the chapter marker tooltip is not owned by the native topmost interface");
    CStructureNetworkContext markerRefs;
    auto markerFields = BorrowedObjectFields(marker,&markerRefs);
    CPtr<NUI::CToolTip> retained;
    { CStructureSaver reader(markerFields,CStructureSaver::READ,&markerRefs); reader.Add(11,&retained); }
    Require(retained.GetPtr() == tooltip, "the chapter marker did not retain the same tooltip in its existing saved field");
    CStructureNetworkContext tooltipRefs;
    auto tooltipFields = BorrowedObjectFields(tooltip,&tooltipRefs);
    CPtr<NUI::CText> text;
    { CStructureSaver reader(tooltipFields,CStructureSaver::READ,&tooltipRefs); reader.Add(2,&text); }
    Require(IsValid(text) && !NUI::GetDBString(11124).empty() && text->GetText() == NUI::GetDBString(11124),
        "the chapter marker tooltip lost its authored initial text");
    if (transitions == 1) CheckChapterMarkerToolTipTransitions(marker,text);
    if (transitions == 2) CheckChapterExitMarkerHint(mission,chapter,marker,text);
    if (transitions == 3) CheckChapterClueHeading(mission,chapter);
    Require(game->bChapterMapSet && game->nChapterMapID == map->GetRecordID() && game->players.size() == 1,
        "creating a chapter marker tooltip changed the campaign destination or party");
}

static void MainMenuLoadSaveTab(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr, "main-menu load fixture needs isolated user data");
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "main-menu load fixture started with an active interface or command");
    struct InterfaceCleanup { ~InterfaceCleanup() { NMainLoop::DoneInterface(); } } cleanup;
    auto saveTab = [&]() {
        auto* active = NMainLoop::GetCurrentInterfaceForHarness();
        Require(IsValid(active), "native load command did not open a modal interface");
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(active,&references);
        int type = -1;
        CPtr<NUI::CWindow> menu;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(3,&type); reader.Add(6,&menu); }
        Require(type == NGame::LOAD && IsValid(menu), "native load command opened the wrong mode or lost its dialog");
        CStructureNetworkContext menuReferences;
        auto menuFields = BorrowedObjectFields(menu,&menuReferences);
        CPtr<NUI::CWindow> view;
        { CStructureSaver reader(menuFields,CStructureSaver::READ,&menuReferences); reader.Add(4,&view); }
        Require(IsValid(view), "native load dialog did not construct its mode view");
        CStructureNetworkContext viewReferences;
        auto viewFields = BorrowedObjectFields(view,&viewReferences);
        CPtr<NUI::CWindow> tab;
        { CStructureSaver reader(viewFields,CStructureSaver::READ,&viewReferences); reader.Add(4,&tab); }
        Require(IsValid(tab) && tab->GetWindowID() == "tab_save", "native load view did not construct its actual Save tab");
        return tab->GetStyle(NUI::STYLE_VISIBLE);
    };
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(2,2);
    for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x) pixels[y][x] = NGfx::SPixel8888(12,23,34,255);
    texture->Set(pixels);
    CObj<NGame::CICSaveLoadMenu> allowed = new NGame::CICSaveLoadMenu(NGame::LOAD,texture,true);
    allowed->Exec();
    Require(saveTab(), "a load dialog with explicit save permission hid its Save tab");
    NMainLoop::DoneInterface();
    CObj<NGame::CMissionBase> mainMenu = dynamic_cast<NGame::CMissionBase*>(pSSClasses->CreateObject(int(0xB2540912)));
    Require(IsValid(mainMenu), "main-menu load fixture has no native menu interface class");
    CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false);
    CObj<NUI::CInterface> interface = new NUI::CInterface(cursor);
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.include = [](CObjectBase*) { return false; };
    references.ids[cursor.GetPtr()] = 1; references.refs[1] = cursor.GetPtr();
    references.ids[interface.GetPtr()] = 2; references.refs[2] = interface.GetPtr();
    references.nextID = 100;
    CMemoryStream saved;
    { CStructureSaver writer(saved,CStructureSaver::WRITE,&references); writer.Add(15,&cursor); writer.Add(16,&interface); }
    saved.Seek(0);
    { CStructureSaver reader(saved,CStructureSaver::READ,&references); mainMenu->NGame::CMissionBase::operator&(reader); }
    NInput::SEvent event;
    while (NInput::GetEvent(&event)) {}
    NInput::PostEvent("loadgame");
    Require(NInput::GetEvent(&event) && mainMenu->ProcessEvent(event) && NMainLoop::HaveInterfaceCommand(),
        "native main menu did not consume its Load Game command");
    Require(NMainLoop::StepApp(false,false,false), "main-menu load command did not execute through the application queue");
    Require(!saveTab(), "loading from the main menu offered a Save tab without an active game");
}

static void ExitMenuSavePermission(const char* databasePath,const char* resourcePath,bool shortcut)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    Require(NMainLoop::GetCurrentInterfaceForHarness() == nullptr && !NMainLoop::HaveInterfaceCommand(),
        "exit save-gate fixture must start without an interface or queued command");
    struct InterfaceCleanup { ~InterfaceCleanup() { NMainLoop::DoneInterface(); } } cleanup;
    // The existing saved UI permission must still permit Save when explicitly true.
    CObj<NUI::ICursor> controlCursor = NUI::ICursor::Create(false);
    CObj<NUI::CInterface> controlInterface = new NUI::CInterface(controlCursor);
    CObj<NUI::CWindow> permitted = dynamic_cast<NUI::CWindow*>(pSSClasses->CreateObject(int(0xB1203151)));
    Require(IsValid(permitted), "exit save-gate fixture has no native dialog class");
    permitted->SetInterface(controlInterface);
    bool allow = true;
    CMemoryStream fields;
    { CStructureSaver writer(fields,CStructureSaver::WRITE); writer.Add(2,&allow); }
    fields.Seek(0);
    { CStructureSaver reader(fields,CStructureSaver::READ); static_cast<CObjectBase*>(permitted.GetPtr())->operator&(reader); }
    NUI::LoadTemplate(permitted,NDb::GetUIContainer(357));
    auto* allowedSave = permitted->GetChildByID("save");
    Require(IsValid(allowedSave) && allowedSave->GetStyle(NUI::STYLE_VISIBLE),
        "exit dialog control did not show Save for an explicitly permitted saved UI");
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(1,1);
    pixels[0][0] = NGfx::SPixel8888(12,23,34,255); texture->Set(pixels);
    CObj<NGame::CICExitMenu> command = new NGame::CICExitMenu(texture);
    command->Exec();
    auto* active = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(active) && !NMainLoop::HaveInterfaceCommand(),
        "exit save-gate fixture did not initialize the actual exit menu cleanly");
    if (shortcut) {
        NInput::SEvent event;
        while (NInput::GetEvent(&event)) {}
        NInput::PostEvent("save");
        Require(NInput::GetEvent(&event), "exit save-gate fixture did not receive the native save command");
        active->ProcessEvent(event);
        Require(!NMainLoop::HaveInterfaceCommand(),
            "the default exit dialog queued a Save menu through its forbidden save shortcut");
    } else {
        CStructureNetworkContext references;
        auto saved = BorrowedObjectFields(active,&references);
        CPtr<NUI::CWindow> dialog;
        { CStructureSaver reader(saved,CStructureSaver::READ,&references); reader.Add(5,&dialog); }
        Require(IsValid(dialog), "exit save-gate fixture did not retain the native dialog");
        auto* save = dialog->GetChildByID("save");
        auto* cancel = dialog->GetChildByID("cancel");
        Require(IsValid(save) && IsValid(cancel), "exit save-gate fixture lacks save or cancel buttons");
        Require(!save->GetStyle(NUI::STYLE_VISIBLE),
            "the default exit dialog displayed Save despite its disabled save permission");
        Require(cancel->GetStyle(NUI::STYLE_VISIBLE), "disabling Save also hid the exit dialog's Cancel button");
    }
    CheckPermittedExitMenu<NGame::CICExitMenu>(texture,shortcut);
}

static void SuppliedMenuScreenshot(const char* databasePath,const char* resourcePath, bool lose = false)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    Require(NMainLoop::GetCurrentInterfaceForHarness() == nullptr,
        "supplied screenshot fixture must start without an active interface");
    struct InterfaceCleanup { ~InterfaceCleanup() { NMainLoop::DoneInterface(); } } cleanup;
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels;
    pixels.SetSizes(2,2);
    pixels[0][0] = NGfx::SPixel8888(11,22,33,255);
    pixels[0][1] = NGfx::SPixel8888(44,55,66,255);
    pixels[1][0] = NGfx::SPixel8888(77,88,99,255);
    pixels[1][1] = NGfx::SPixel8888(111,122,133,255);
    texture->Set(pixels);
    if (lose) {
        CObj<NGame::CICLoseMenu> command = new NGame::CICLoseMenu(-1,texture);
        command->Exec();
    } else {
        CObj<NGame::CICOptions> command = new NGame::CICOptions(NGame::OS_VIDEO,texture);
        command->Exec();
    }
    auto* active = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(active), "supplied screenshot fixture did not open the native options interface");
    CStructureNetworkContext references;
    auto interfaceFields = BorrowedObjectFields(active,&references);
    CPtr<NUI::CScreenShot> widget;
    { CStructureSaver reader(interfaceFields,CStructureSaver::READ,&references); reader.Add(5,&widget); }
    Require(IsValid(widget) && widget->GetTexture() == texture.GetPtr(),
        "native options interface replaced or failed to adopt the supplied screenshot");
    CMemoryStream fields;
    { CStructureSaver writer(fields,CStructureSaver::WRITE); static_cast<CObjectBase*>(texture.GetPtr())->operator&(writer); }
    fields.Seek(0);
    NGScene::CScreenshotTexture::EMode mode = NGScene::CScreenshotTexture::COLOR;
    CVec4 coefficient(0,0,0,0);
    { CStructureSaver reader(fields,CStructureSaver::READ); reader.Add(2,&mode); reader.Add(3,&coefficient); }
    Require(mode == NGScene::CScreenshotTexture::BLACKANDWHITE,
        lose ? "opening the defeat menu with a supplied screenshot left it in default color mode"
             : "opening options with a supplied screenshot left it in the widget's default color mode");
    const CVec4 expected = lose ? CVec4(1.0f,0.677f,0.877f,1.0f) : CVec4(0.677f,0.877f,1.0f,1.0f);
    Require(fabs(coefficient.x-expected.x) < 1e-6f && fabs(coefficient.y-expected.y) < 1e-6f &&
        fabs(coefficient.z-expected.z) < 1e-6f && coefficient.w == expected.w,
        lose ? "supplied defeat screenshot did not receive the original loss tint"
             : "supplied options screenshot did not receive the original menu tint");
    CArray2D<NGfx::SPixel8888> retained;
    texture->Get(&retained);
    Require(retained.GetXSize() == 2 && retained.GetYSize() == 2,
        "opening options resized the supplied screenshot pixels");
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 2; ++x)
            Require(retained[y][x].r == pixels[y][x].r && retained[y][x].g == pixels[y][x].g &&
                retained[y][x].b == pixels[y][x].b && retained[y][x].a == pixels[y][x].a,
                "options screenshot treatment overwrote the original pixels");
}

static void NewLayerCompactSave()
{
    auto roundtrip = [](NAI::CNodesLayer* layer) {
        CObj<NAI::CNodesLayer> original = layer, restored;
        CMemoryStream fields;
        { CStructureSaver writer(fields, CStructureSaver::WRITE); writer.Add(2, &original); }
        fields.Seek(0);
        { CStructureSaver reader(fields, CStructureSaver::READ); reader.Add(2, &restored); }
        Require(IsValid(restored), "navigation save fixture did not restore the layer graph");
        return restored;
    };
    auto inspect = [&](NAI::CNodesLayer* layer) {
        auto restored = roundtrip(layer);
        return std::make_pair(restored->bHappySave, IsValid(restored->pColourer));
    };
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    const int normal = network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->GetColourer(normal);
    const auto ordinary = inspect(network->GetLayer(normal));
    Require(!ordinary.first && ordinary.second, "ordinary layer save did not retain its existing navigation cache");
    network->SetSmallSaveSize(true);
    const int added = network->CreateLayer(8, 8, VNULL2, 0.f, 1);
    network->GetColourer(added);
    const auto fresh = inspect(network->GetLayer(added));
    Require(fresh.first, "navigation layer created after enabling compact saves did not inherit the small-save flag");
    Require(!fresh.second && !inspect(network->GetLayer(normal)).second,
        "compact layer save retained a cache on a new or existing layer");
    Require(IsValid(network->GetLayer(added)->pColourer) && IsValid(network->GetLayer(normal)->pColourer),
        "compact serialization destroyed the live navigation caches");
    auto restored = roundtrip(network->GetLayer(added));
    Require(restored->bHappySave && !IsValid(restored->pColourer),
        "compact layer save did not round-trip the flag and omitted-cache state");
    network->SetSmallSaveSize(false);
    for (int index: {normal, added}) {
        const auto full = inspect(network->GetLayer(index));
        Require(!full.first && full.second, "disabling compact saves did not restore full cache serialization on all layers");
    }
}

static void ComboPopupClipping(bool itemWidth = false)
{
    CObj<NUI::CInterface> root = new NUI::CInterface;
    root->SetInterface(root); root->SetSize(NUI::SPoint(640,480)); root->SetPosition(NUI::SPoint(20,10));
    root->SetStyle(NUI::STYLE_VISIBLE | NUI::STYLE_ENABLED,true);
    CObj<NUI::CWindow> panel = new NUI::CWindow(NUI::SWindowInfo(root,NUI::SPoint(100,80),NUI::SPoint(300,100),"panel"));
    CObj<NUI::CWindow> row = new NUI::CWindow(NUI::SWindowInfo(panel,NUI::SPoint(25,20),NUI::SPoint(230,20),"row"));
    CObj<NUI::CComboBox> combo = new NUI::CComboBox(NUI::SWindowInfo(row,NUI::SPoint(10,2),NUI::SPoint(100,18),"combo"));
    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(combo,&references);
    CPtr<NUI::CListView> popup;
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(4,&popup); }
    Require(IsValid(popup),"native combo did not create its actual popup");
    combo->AddItem(7,NUI::CComboBox::SInfo(L"First"));
    combo->AddItem(9,NUI::CComboBox::SInfo(L"Second"));
    for (int id : {7,9}) popup->GetItem(id)->SetSize(NUI::SPoint(100,22));
    popup->SetSize(NUI::SPoint(160,44)); // Existing remaster artwork can exceed the closed field.
    auto update = [&]() { combo->Update(1,nullptr); popup->Update(1,nullptr); };
    auto open = [&]() { Require(combo->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY,"drop_list")) &&
        popup->GetStyle(NUI::STYLE_VISIBLE),"native combo did not open its actual popup"); };
    auto check = [&]() {
        NUI::SPoint field; NUI::SRect clip;
        combo->ClientToScreen(&field,&clip);
        const NUI::SPoint expected(field.x+combo->GetSize().x-160,field.y+combo->GetSize().y);
        NUI::SPoint position; popup->ClientToScreen(&position,&clip);
        std::printf("native combo parent=%s popup=%d,%d expected=%d,%d hit=%d\n",popup->GetParent()->GetWindowID().c_str(),
            position.x,position.y,expected.x,expected.y,int(popup->HitTest(expected.x+10,expected.y+10)));
        Require(popup->HitTest(expected.x+10,expected.y+10),"native combo popup is clipped by its owning row");
        Require(popup->GetParent() == root.GetPtr() && position == expected && popup->GetSize().x == 160,
            "native combo lost interface coordinates, right alignment or remaster artwork width");
    };
    update(); open(); check();
    if (itemWidth) {
        auto* wide = popup->GetItem(9);
        wide->SetSize(NUI::SPoint(250,22)); update();
        NUI::SPoint position; NUI::SRect clip; wide->ClientToScreen(&position,&clip);
        std::printf("native combo row width=%d popup width=%d row-end hit=%d\n",wide->GetSize().x,popup->GetSize().x,
            int(wide->HitTest(position.x+240,position.y+10)));
        Require(wide->HitTest(position.x+240,position.y+10),"native combo popup clips an item wider than its closed field");
        auto width = [&](int expected) {
            NUI::SPoint field, list; NUI::SRect clip;
            combo->ClientToScreen(&field,&clip); popup->ClientToScreen(&list,&clip);
            Require(popup->GetSize() == NUI::SPoint(expected,44) && list.x+expected == field.x+combo->GetSize().x,
                "native item-width layout lost the field right edge, list height or remaster artwork minimum");
        };
        width(250);
        combo->SetSize(NUI::SPoint(300,18)); update(); width(300);
        combo->SetSize(NUI::SPoint(80,18)); update(); width(250);
        wide->SetSize(NUI::SPoint(100,22)); update(); width(160);
        combo->RemoveAllItems(); update();
        Require(popup->GetSize() == NUI::SPoint(160,0) && combo->GetSelectedItem() == -1,
            "empty native combo lost the remaster artwork width or retained rows");
        return;
    }
    panel->SetPosition(NUI::SPoint(150,90)); combo->SetSize(NUI::SPoint(60,18)); update(); check();
    // An older native UI graph can still carry the popup below its row.
    { CMObj<NUI::CWindow> hold = popup.GetPtr(); root->RemoveChild(popup); row->AddChild(popup); }
    update(); check();
    row->SetStyle(NUI::STYLE_ENABLED,false); update();
    Require(!popup->GetStyle(NUI::STYLE_ENABLED),"root-level popup remained interactive below a disabled owner");
    row->SetStyle(NUI::STYLE_ENABLED,true); panel->SetStyle(NUI::STYLE_VISIBLE,false); update();
    Require(!popup->GetStyle(NUI::STYLE_VISIBLE),"root-level popup remained visible below a hidden owner");
    panel->SetStyle(NUI::STYLE_VISIBLE,true); update(); open(); check();
    popup->SetSelectedItem(9);
    Require(combo->GetSelectedItem() == 9 && !popup->GetStyle(NUI::STYLE_VISIBLE),
        "native popup selection did not notify its combo and close");
    open();
    combo->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONDOWN,0,0));
    Require(!popup->GetStyle(NUI::STYLE_VISIBLE),"native popup did not close on an outside click");
    combo = nullptr;
    Require(!IsValid(popup),"destroying a combo left its root-level popup alive");
}

static void WindowLazyUpdate()
{
    auto requiresUpdate = [](NUI::CWindow* window) {
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(window, &references);
        bool flag = true;
        { CStructureSaver reader(fields, CStructureSaver::READ, &references); reader.Add(4, &flag); }
        return flag;
    };
    CObj<NUI::CWindow> idle = new NUI::CWindow;
    Require(requiresUpdate(idle), "new UI window did not start with an update pending");
    idle->Update(1, nullptr);
    Require(!requiresUpdate(idle), "idle UI branch never cleared its serialized update latch");
    idle->Update(2, nullptr);
    Require(!requiresUpdate(idle), "sleeping UI branch spontaneously resumed its per-frame traversal");
    CMObj<NUI::CWindow> child = new NUI::CWindow(NUI::SWindowInfo(idle,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "late-child"));
    Require(requiresUpdate(idle), "adding a child did not wake its sleeping parent");
    idle->Update(3, nullptr);
    Require(!requiresUpdate(idle) && !requiresUpdate(child), "static child kept its UI branch awake forever");
    CObj<NUI::CWindow> animated = new NUI::CWindow(NUI::SWindowInfo(nullptr,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "always-update", 0x80));
    animated->Update(1, nullptr); animated->Update(2, nullptr);
    Require(requiresUpdate(animated), "retail always-update style did not keep its UI window awake");
    CObj<NUI::CWindow> active = new NUI::CWindow;
    CMObj<NUI::CSlider> slider = new NUI::CSlider(NUI::SWindowInfo(active,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "active-slider"));
    active->Update(1, nullptr); active->Update(2, nullptr);
    Require(requiresUpdate(active), "active slider's update result did not keep its parent branch awake");
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE); bool pending = false; writer.Add(4, &pending); }
    saved.Seek(0);
    CObj<NUI::CWindow> restored = new NUI::CWindow;
    { CStructureSaver reader(saved, CStructureSaver::READ); restored->operator&(reader); }
    restored->Update(4, nullptr);
    Require(!requiresUpdate(restored), "restored sleeping UI window lost the retail tag-4 latch");
}

static void WindowNestedWakeup()
{
    auto requiresUpdate = [](NUI::CWindow* window) {
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(window, &references);
        bool flag = true;
        { CStructureSaver reader(fields, CStructureSaver::READ, &references); reader.Add(4, &flag); }
        return flag;
    };
    CObj<NUI::CWindow> root = new NUI::CWindow;
    CMObj<NUI::CWindow> branch = new NUI::CWindow(NUI::SWindowInfo(root,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "sleeping-branch"));
    root->Update(1, nullptr);
    Require(!requiresUpdate(root) && !requiresUpdate(branch), "nested UI wakeup fixture did not become idle");
    CMObj<NUI::CSlider> active = new NUI::CSlider(NUI::SWindowInfo(branch,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "late-active-descendant"));
    Require(requiresUpdate(root) && requiresUpdate(branch),
        "new active descendant did not wake all sleeping ancestor windows");
    root->Update(2, nullptr); root->Update(3, nullptr);
    Require(requiresUpdate(root) && requiresUpdate(branch), "active descendant stopped updating beneath a sleeping ancestor");
    branch->RemoveChild(active);
    active = nullptr;
    root->Update(4, nullptr);
    Require(!requiresUpdate(root) && !requiresUpdate(branch), "removing the active descendant left idle ancestors permanently awake");
    CMObj<NUI::CWindow> passive = new NUI::CWindow(NUI::SWindowInfo(branch,
        NUI::SPoint(0, 0), NUI::SPoint(20, 20), "late-passive-descendant"));
    Require(requiresUpdate(root), "a passive descendant did not request its initial ancestor traversal");
    root->Update(5, nullptr);
    Require(!requiresUpdate(root) && !requiresUpdate(branch) && !requiresUpdate(passive),
        "passive descendant insertion disabled lazy UI updates");
}

static void WindowTemplateScript(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    CDBIterator<NDb::CScript> scripts(*NDatabase::GetTable<NDb::CScript>());
    NDb::CScript* record = nullptr;
    while (scripts.MoveNext()) if (scripts.Get()->GetRecordID() > 0) { record = scripts.Get(); break; }
    Require(record && NDb::GetDBScript(record->GetRecordID()) == record,
        "template script fixture did not resolve its native database script record");
    record->strCode = "local w=GetWindow('template-child'); if w then w.x=84 end; templateReady=1";
    CObj<NDb::CUIContainer> layout = new NDb::CUIContainer;
    layout->pScript = record;
    CObj<NUI::CInterface> interface = new NUI::CInterface;
    interface->SetInterface(interface);
    CMObj<NUI::CWindow> window = new NUI::CWindow(NUI::SWindowInfo(interface,
        NUI::SPoint(1, 2), NUI::SPoint(20, 20), "template-child"));
    CObj<NScript::CScript> unrelated = NScript::CreateScript(nullptr);
    unrelated->PushNumber(777);
    struct GlobalScriptRestore {
        CPtr<NScript::CScript> previous;
        ~GlobalScriptRestore() { pScript = previous; }
    } restore{pScript};
    pScript = unrelated;
    window->ProcessMessage(NUI::SEvent(NUI::EVENT_TEMPLATELOAD, nullptr, layout.GetPtr()));
    Require(window->GetPosition() == NUI::SPoint(1, 2), "template script ran before the native window update phase");
    window->Update(123, nullptr);
    Require(window->GetPosition() == NUI::SPoint(84, 2),
        "window template Lua script was not created and executed in its own interface during Update");
    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(window, &references);
    CPtr<NScript::CScript> script;
    { CStructureSaver reader(fields, CStructureSaver::READ, &references); reader.Add(17, &script); }
    Require(IsValid(script) && !IsValid(script->pWorld) && script->GetScriptInterface() == interface.GetPtr()
            && script->GetGlobal("templateReady").IsNumber() && script->GetGlobal("templateReady").GetNumber() == 1
            && unrelated->GetTop() == 1,
        "template Lua lifecycle lost its saved script/context or modified an unrelated script stack");
    CObj<NDb::CUIContainer> plainLayout = new NDb::CUIContainer;
    CMObj<NUI::CWindow> plain = new NUI::CWindow(NUI::SWindowInfo(interface,
        NUI::SPoint(5, 6), NUI::SPoint(10, 10), "unscripted-child"));
    plain->ProcessMessage(NUI::SEvent(NUI::EVENT_TEMPLATELOAD, nullptr, plainLayout.GetPtr()));
    plain->Update(124, nullptr);
    CStructureNetworkContext plainReferences;
    auto plainFields = BorrowedObjectFields(plain, &plainReferences);
    CPtr<NScript::CScript> absent;
    { CStructureSaver reader(plainFields, CStructureSaver::READ, &plainReferences); reader.Add(17, &absent); }
    Require(!IsValid(absent) && plain->GetPosition() == NUI::SPoint(5, 6), "unscripted templates acquired a script or changed their native window");
}

static std::map<unsigned char, vector<unsigned char>> OuterSaveChunks(CDataStream& stream)
{
    std::map<unsigned char, vector<unsigned char>> chunks;
    stream.Seek(0);
    while (stream.GetPosition() < stream.GetSize()) {
        unsigned char tag = 0, encoded[4] = {};
        stream.Read(&tag, 1); stream.Read(encoded, 1);
        const std::size_t prefix = encoded[0] & 1 ? 4 : 1;
        if (prefix == 4) stream.Read(encoded + 1, 3);
        std::uint32_t length = 0;
        Require(S2FileIO::DecodeStructureLength(encoded, prefix, stream.GetSize() - stream.GetPosition(), &length),
            "compressed-save fixture found an invalid outer chunk length");
        auto& payload = chunks[tag]; payload.resize(length);
        if (length) stream.Read(payload.data(), length);
    }
    return chunks;
}

static void LoadSuccessFeedback(const char* databasePath,const char* resourcePath,bool encodedName = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr, "load feedback fixture needs isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "load feedback fixture started with a live interface or command");
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    const char* profile = encodedName ? "UpstreamEncodedLoad" : "UpstreamLoadFeedback";
    NMainLoop::CreateProfile(profile);
    NMainLoop::SetActiveProfile(profile);
    const string slot = encodedName ? S2FileIO::EncodeWindowsSaveName(L"Load \u0417\u0430\u043F\u0438\u0441\u044C \u65E5\u672C") : "Feedback";
    auto* manager = NMainLoop::GetSaveManager();
    struct Cleanup {
        NMainLoop::CSaveManager* manager;
        string slot;
        ~Cleanup() { NMainLoop::DoneInterface(); manager->DeleteSlot(slot); manager->DeleteSlot(NMainLoop::S_SLOT_ACTIVE); }
    } cleanup{manager,slot};
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(2,2);
    for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x) pixels[y][x] = NGfx::SPixel8888(12,23,34,255);
    texture->Set(pixels);
    CObj<NGame::CICExitMenu> menu = new NGame::CICExitMenu(texture);
    menu->Exec();
    CObj<NMainLoop::IInterfaceBase> original = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(original) && NMainLoop::GetInterfaceStackDepth() == 1,
        "load feedback fixture did not create its native interface stack");
    CObj<NMainLoop::CICSave> save = new NMainLoop::CICSave(slot,texture,true);
    save->Exec();
    const auto localized = NUI::GetDBString(20260);
    Require(!localized.empty(), "load feedback fixture has no localized success message");
    csGame << localized << endl;
    Require(!consoleLines.empty() && consoleLines.front().eType == STREAM_GAME && !consoleLines.front().szText.empty(),
        "load feedback log control did not receive its localized game-stream message");
    const auto expected = consoleLines.front().szText;
    auto messages = [&]() {
        return std::count_if(consoleLines.begin(),consoleLines.end(),[&](const SConsoleLine& line) {
            return line.eType == STREAM_GAME && line.szText == expected;
        });
    };
    const auto before = messages();
    CObj<NMainLoop::CICLoad> load = new NMainLoop::CICLoad(slot,true);
    load->Exec();
    CObj<NMainLoop::IInterfaceBase> restored = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(restored) && restored.GetPtr() != original.GetPtr() && NMainLoop::GetInterfaceStackDepth() == 1,
        "load feedback control did not restore a new native interface from the actual slot");
    Require(messages() == before + 1, "a completed silent native load omitted its localized success feedback");
    if (encodedName) {
        list<string> written; manager->GetSlotsList(&written);
        for (const auto* invalid : {L"bad.name",L"bad^name",L"bad:name",L"bad/name",L".."}) {
            CObj<NMainLoop::CICLoad> rejected = new NMainLoop::CICLoad(S2FileIO::EncodeWindowsSaveName(invalid),true);
            rejected->Exec();
            list<string> after; manager->GetSlotsList(&after);
            Require(after == written && NMainLoop::GetCurrentInterfaceForHarness() == restored.GetPtr() && messages() == before+1,
                "encoded invalid load name changed the active interface or reported success");
        }
        std::puts("native encoded Unicode load: restored a new interface from the actual saved file; invalid names rejected");
    }
    Require(!std::filesystem::exists(std::filesystem::path(manager->GetSlotFilePathW("Missing",NMainLoop::S_SAVE_FILENAME))),
        "load failure control unexpectedly has an existing slot");
    const auto beforeFailure = messages();
    CObj<NMainLoop::CICLoad> failed = new NMainLoop::CICLoad("Missing",true);
    try { failed->Exec(); } catch (...) {} // Debug builds can propagate the expected read failure.
    Require(messages() == beforeFailure && NMainLoop::GetCurrentInterfaceForHarness() == restored.GetPtr(),
        "a missing native slot reported success or replaced the active interface");
}

static void SaveSuccessFeedback(const char* databasePath,bool encodedName = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr, "save feedback fixture needs isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "save feedback fixture started with a live interface or command");
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    const char* profile = encodedName ? "UpstreamEncodedSave" : "UpstreamSaveFeedback";
    NMainLoop::CreateProfile(profile);
    NMainLoop::SetActiveProfile(profile);
    const string slot = encodedName ? S2FileIO::EncodeWindowsSaveName(L"Quick \u0417\u0430\u043F\u0438\u0441\u044C \u65E5\u672C") : "Feedback";
    auto* manager = NMainLoop::GetSaveManager();
    struct SlotCleanup {
        NMainLoop::CSaveManager* manager;
        string slot;
        ~SlotCleanup() { manager->DeleteSlot(slot); }
    } cleanup{manager,slot};
    const auto localized = NUI::GetDBString(20259);
    Require(!localized.empty(), "save feedback fixture has no localized success message");
    csGame << localized << endl;
    Require(!consoleLines.empty() && consoleLines.front().eType == STREAM_GAME && !consoleLines.front().szText.empty(),
        "save feedback log control did not receive its localized game-stream message");
    const auto expected = consoleLines.front().szText;
    auto messages = [&]() {
        return std::count_if(consoleLines.begin(),consoleLines.end(),[&](const SConsoleLine& line) {
            return line.eType == STREAM_GAME && line.szText == expected;
        });
    };
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(2,2);
    for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x) pixels[y][x] = NGfx::SPixel8888(12,23,34,255);
    texture->Set(pixels);
    const auto before = messages();
    CObj<NMainLoop::CICSave> save = new NMainLoop::CICSave(slot,texture,true);
    save->Exec();
    list<string> written; manager->GetSlotsList(&written);
    Require(!encodedName || std::find(written.begin(),written.end(),slot) != written.end(),
        "native save command rejected its platform-encoded valid Unicode slot");
    {
        CFileStream file;
        file.OpenRead(manager->GetSlotFilePathW(slot,NMainLoop::S_SAVE_FILENAME).c_str());
        NMainLoop::SSaveFileHeader header;
        NMainLoop::ReadSaveFileHeader(file,&header);
        Require(header.nMagic == NMainLoop::N_SAVE_MAGIC_NUMBER && header.nMods == 0,
            "save feedback control did not write a complete native save header");
        list<CObj<NMainLoop::IInterfaceBase>> restored;
        { CStructureSaver reader(file,CStructureSaver::READ); reader.Add(2,&restored); }
        Require(restored.empty(), "save feedback control did not round-trip its empty native interface stack");
    }
    Require(messages() == before + 1, "a completed silent native save omitted its localized success feedback");
    if (encodedName) {
        for (const auto* invalid : {L"bad.name",L"bad^name",L"bad:name",L"bad/name",L".."}) {
            const auto count = messages();
            CObj<NMainLoop::CICSave> rejected = new NMainLoop::CICSave(S2FileIO::EncodeWindowsSaveName(invalid),texture,true);
            rejected->Exec();
            list<string> after; manager->GetSlotsList(&after);
            Require(after == written && messages() == count,"encoded invalid save name wrote a slot or reported success");
        }
#if !defined(_WIN32)
        for (const char* invalid : {"S2U8:GG","S2U8:0","S2U8:00"}) {
            CObj<NMainLoop::CICSave> rejected = new NMainLoop::CICSave(invalid,texture,true);
            rejected->Exec();
            list<string> after; manager->GetSlotsList(&after);
            Require(after == written && messages() == before+1,"malformed Linux save-name encoding wrote a slot or reported success");
        }
#endif
        std::puts("native encoded Unicode save: real header/data round-trip; invalid decoded names rejected");
        return;
    }
    const auto blockedPath = std::filesystem::path(manager->GetSlotFilePathW(NMainLoop::S_SLOT_ACTIVE,NMainLoop::S_SAVE_FILENAME));
    std::filesystem::create_directories(blockedPath.parent_path());
    Require(std::filesystem::create_directory(blockedPath), "save failure control could not block its isolated target with a directory");
    struct BlockerCleanup {
        std::filesystem::path path;
        ~BlockerCleanup() { std::error_code error; std::filesystem::remove(path,error); }
    } blocker{blockedPath};
    const auto beforeFailure = messages();
    CObj<NMainLoop::CICSave> failed = new NMainLoop::CICSave(NMainLoop::S_SLOT_ACTIVE,texture,true);
    try { failed->Exec(); } catch (...) {} // Debug builds propagate the expected write failure.
    Require(std::filesystem::is_directory(blockedPath), "save failure control unexpectedly replaced its blocked target");
    Require(messages() == beforeFailure, "a failed native save emitted a false localized success message");
}

static void CompressedSaveWriting()
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr, "compressed-save fixture needs an isolated user-data directory");
    NMainLoop::CreateProfile("UpstreamCompression");
    NMainLoop::SetActiveProfile("UpstreamCompression");
    CObj<NMainLoop::CICSaveFile> snapshot = new NMainLoop::CICSaveFile("upstream-compression.sav");
    snapshot->Exec();
    auto* manager = NMainLoop::GetSaveManager();
    const auto path = manager->GetSlotFilePathW(NMainLoop::S_SLOT_ACTIVE, "upstream-compression.sav");
    CFileStream file; file.OpenRead(path.c_str());
    auto snapshotChunks = OuterSaveChunks(file);
    const vector<unsigned char> marker = {'A', '3', 0};
    Require(snapshotChunks[3] == marker, "native snapshot command wrote an uncompressed save without the retail A3 marker");
    Require(snapshotChunks[4] == vector<unsigned char>({1, 0, 0, 0}), "compressed snapshot lost its unpacked format version");
    file.Seek(0);
    list<CObj<NMainLoop::IInterfaceBase>> interfaces;
    { CStructureSaver reader(file, CStructureSaver::READ); reader.Add(2, &interfaces); }
    Require(interfaces.empty(), "empty native snapshot did not round-trip its interface list");
    vector<CObj<NWorld::CCmdReload>> commands;
    for (int i = 0; i < 1024; ++i) commands.emplace_back(new NWorld::CCmdReload(i % NDb::N_SLOTS));
    CMemoryStream plain, packed;
    { CStructureSaver writer(plain, CStructureSaver::WRITE); writer.Add(2, &commands); }
    // The new compressed writer occupies the next EMode value. Using its wire
    // value keeps this behavioral regression compilable before the API exists.
    { CStructureSaver writer(packed, static_cast<CStructureSaver::EMode>(2)); writer.Add(2, &commands); }
    auto plainChunks = OuterSaveChunks(plain), packedChunks = OuterSaveChunks(packed);
    Require(!plainChunks.count(3) && packedChunks[3] == marker && plainChunks[4] == packedChunks[4],
        "compressed writing changed plain-stream compatibility or the version envelope");
    Require(packed.GetSize() < plain.GetSize() / 2, "repetitive native command save was not actually compressed");
    for (unsigned char tag : {0, 1, 2}) {
        Require(packedChunks.count(tag) && plainChunks.count(tag), "compressed save omitted a required payload chunk");
        CMemoryStream payload, unpacked;
        payload.Write(packedChunks[tag].data(), static_cast<unsigned int>(packedChunks[tag].size()));
        payload.Seek(0);
        CNetCompressor decoder; decoder.Unpack(payload, unpacked);
        Require(unpacked.GetSize() == static_cast<int>(plainChunks[tag].size())
                && (unpacked.GetSize() == 0 || !std::memcmp(unpacked.GetBuffer(), plainChunks[tag].data(), unpacked.GetSize())),
            "a compressed payload did not independently decompress to its native plain contents");
    }
    vector<CObj<NWorld::CCmdReload>> restored;
    packed.Seek(0);
    { CStructureSaver reader(packed, CStructureSaver::READ); reader.Add(2, &restored); }
    Require(restored.size() == commands.size(), "compressed save lost referenced command objects");
    for (std::size_t i = 0; i < restored.size(); ++i)
        Require(IsValid(restored[i]) && restored[i]->nSlot == commands[i]->nSlot,
            "compressed save changed a restored command's slot");
}

static void FreshAmbientGraph()
{
    Require(S2Platform::Window() == nullptr, "fresh ambient fixture unexpectedly owns a display window");
    CObj<NGScene::IGScene> scene = NGScene::CreateScene();
    CStructureNetworkContext references;
    auto saved = BorrowedObjectFields(scene, &references);
    CPtr<CFuncBase<CVec3>> mean, top, bottom, animated;
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      reader.Add(18, &mean); reader.Add(31, &top); reader.Add(32, &bottom); reader.Add(33, &animated); }
    Require(IsValid(animated) && pSSClasses->GetObjectTypeID(animated.GetPtr()) == int(0x01123131),
        "fresh scene omitted its retail animated top-ambient node");
    Require(IsValid(mean) && IsValid(top) && IsValid(bottom), "fresh scene lost its ambient endpoints");
    CStructureNetworkContext meanReferences;
    auto meanFields = BorrowedObjectFields(mean, &meanReferences);
    CPtr<CFuncBase<CVec3>> meanTop, meanBottom;
    { CStructureSaver reader(meanFields, CStructureSaver::READ, &meanReferences);
      reader.Add(2, &meanTop); reader.Add(3, &meanBottom); }
    Require(meanTop == animated && meanBottom == bottom, "fresh ambient mean bypassed its animated top edge");
    const CVec3 topColor(0.8f, 0.6f, 0.4f), bottomColor(0.2f, 0.1f, 0.3f);
    scene->SetAmbient(bottomColor, topColor);
    CDGPtr<CFuncBase<CVec3>> refreshed(mean);
    MarkNewDGFrame(); refreshed.Refresh();
    Require(fabs2(refreshed->GetValue() - (topColor + bottomColor) * 0.5f) < 1e-10f,
        "fresh animated ambient node changed the unanimated scene mean");
    CObj<FixedAnimatedLight> animation = new FixedAnimatedLight;
    animation->Light()->color = CVec3(0.4f, 0.8f, 0.2f); animation->Light()->bEnd = false;
    references.ids[animation.GetPtr()] = 1; references.refs[1] = animation.GetPtr();
    CPtr<CPtrFuncBase<NGScene::CAnimLight>> animationEdge = animation.GetPtr();
    CMemoryStream binding;
    { CStructureSaver writer(binding, CStructureSaver::WRITE, &references); writer.Add(3, &animationEdge); }
    binding.Seek(0);
    { CStructureSaver reader(binding, CStructureSaver::READ, &references); animated->operator&(reader); }
    animated->NetworkFieldsChanged();
    MarkNewDGFrame(); refreshed.Refresh();
    Require(fabs2(refreshed->GetValue() - (animation->Light()->color + bottomColor) * 0.5f) < 1e-10f,
        "live animated ambient did not reach the scene mean");
    animation->Light()->color = CVec3(0.9f, 0.3f, 0.7f); animation->Updated();
    MarkNewDGFrame(); refreshed.Refresh();
    Require(fabs2(refreshed->GetValue() - (animation->Light()->color + bottomColor) * 0.5f) < 1e-10f,
        "animated ambient changes did not invalidate the native dependency graph");
    animation->Light()->bEnd = true; animation->Updated();
    MarkNewDGFrame(); refreshed.Refresh();
    Require(fabs2(refreshed->GetValue() - (topColor + bottomColor) * 0.5f) < 1e-10f,
        "ended ambient animation did not restore the configured top color");
    CObj<NGScene::CCVec3> sunlight = new NGScene::CCVec3(CVec3(1, 1, 1));
    CObj<NGScene::CCVec3> gloss = new NGScene::CCVec3(VNULL3);
    CPtr<CObjectBase> directional = scene->AddDirectionalLight(sunlight, gloss, VNULL3,
        CVec3(0, 0, 1), VNULL3, CVec2(8, 8), 16.f, true, 0);
    Require(IsValid(directional), "fresh ambient fixture did not create a native directional light");
    CStructureNetworkContext directionalReferences;
    auto directionalFields = BorrowedObjectFields(directional, &directionalReferences);
    CPtr<CFuncBase<CVec3>> directionalTop;
    { CStructureSaver reader(directionalFields, CStructureSaver::READ, &directionalReferences);
      reader.Add(15, &directionalTop); }
    Require(directionalTop == animated, "directional lighting bypassed the scene's animated ambient edge");
    Require(S2Platform::Window() == nullptr, "ambient dependency test unexpectedly created a display window");
}

static void AnimatedPointLightGuard(bool inactive)
{
    CObj<FixedAnimatedLight> animator = new FixedAnimatedLight;
    auto* state = animator->Light();
    state->position = VNULL3; state->color = CVec3(1, 1, 1);
    state->fRadius = 4; state->bActive = true; state->bEnd = false;
    CObj<NGScene::CDynamicPointLight> light = new NGScene::CDynamicPointLight(animator);
    CObj<AnimatedLightMaterial> material = new AnimatedLightMaterial;
    NGScene::CSceneFragments scene;
    NGScene::CPartFlags parts; parts.Clear(); parts.Set(0);
    scene.AddElement(0, parts, material, nullptr, nullptr);
    NGScene::SParticleLMRenderTargetInfo particles;
    CTransformStack transform;
    auto reachesMaterials = [&]() {
        // Observe the real render traversal before any graphics calls; the
        // material sentinel also proves that a valid light is not skipped.
        try { light->Render(&transform, &transform, nullptr, NGScene::RP_GF3_CL, nullptr, scene, particles); }
        catch (const AnimatedLightRenderReached&) { return true; }
        return false;
    };
    Require(reachesMaterials(), "animated-light fixture did not enter rendering for a live ordinary light");
    if (inactive) {
        state->bActive = false;
        Require(!reachesMaterials(), "inactive animated point light entered scene rendering before its parameters were ready");
        state->bActive = true;
    } else {
        for (float radius : {0.f, -1.f, 0.0999f}) {
            state->fRadius = radius;
            Require(!reachesMaterials(), "zero/tiny animated point-light radius reached the rendering pipeline");
        }
        state->fRadius = 0.1f;
        Require(reachesMaterials(), "minimum valid animated point-light radius stopped rendering");
    }
    state->bEnd = true;
    Require(!reachesMaterials(), "ended animated point light still rendered");
    state->bEnd = false; state->color = VNULL3;
    Require(!reachesMaterials(), "dark animated point light still rendered");
}

static void TinyPointLightCache()
{
    NGScene::SGlobalIlluminationInfo global;
    global.vUpDifColor = VNULL3;
    const float radii[] = {0.f, 0.25f, 0.5f, 0.5001f, 12.f};
    for (float radius: radii)
        global.points.emplace_back(CVec3(1, 0.5f, 0.25f), VNULL3, radius, false, false);
    NGScene::SLightStateCalcSeed seed;
    NGScene::CLightState cached;
    cached.CreateSimple(&seed, global);
    Require(cached.points.size() == 2,
        "point-light cache retained zero/tiny radii that retail discards before lighting");
    Require(cached.points[0].fRadius == 0.5001f && cached.points[1].fRadius == 12.f
            && cached.points[1].vColor == CVec3(1, 0.5f, 0.25f) && !cached.points[1].bCastShadow,
        "tiny-light cutoff changed surviving radii, colour, ordering or shadow settings");
    cached.skyDirections.clear();
    NGScene::SDynamicAmbientInfo result;
    cached.TraceDynamicLM(&result, SSphere(CVec3(1, 0, 0), 0.1f), nullptr);
    Require(fabs2(result.vXNeg.v - CVec3(4.f/9.f, 2.f/9.f, 1.f/9.f)) < 1e-10f,
        "surviving point light changed its distance attenuation after tiny-light filtering");
}

static void PointLightRadiance()
{
    CObj<NGScene::CPointLight> light = new NGScene::CPointLight(CVec3(4, 2, 1), CVec3(3, -2, 5), 3.f, nullptr, true);
    NGScene::CPointLight::SRadianceInfo info;
    light->GetRadianceInfo(&info, NGScene::RP_GF3_CL);
    Require(info.vColor == CVec3(1, 0.5f, 0.25f) && info.fRadius == 6.f,
        "point-light radiance failed to fold colour strength into its cached radius");
    Require(info.vCenter == CVec3(3, -2, 5) && !info.bIsRendered,
        "radiance conversion changed light position or lightmap-only eligibility");
    CObj<NGScene::CPointLight> dim = new NGScene::CPointLight(CVec3(0.25f, 0.125f, 0), VNULL3, 8.f, nullptr, true);
    dim->GetRadianceInfo(&info, NGScene::RP_GF3_CL);
    Require(info.vColor == CVec3(1, 0.5f, 0) && info.fRadius == 4.f,
        "dim point-light radiance used the wrong radius scaling");
    CObj<NGScene::CPointLight> dark = new NGScene::CPointLight(VNULL3, VNULL3, 3.f, nullptr, true);
    dark->GetRadianceInfo(&info, NGScene::RP_GF3_CL);
    Require(info.vColor == VNULL3 && info.fRadius == 3.f,
        "zero-strength point light generated nonfinite radiance or changed its radius");
}

struct RetailGlobalPointFixture
{
    float color[3], center[3], radius;
    bool rendered, shadow;
    unsigned char padding[2];
};
static_assert(sizeof(RetailGlobalPointFixture) == 32, "retail global point-light record size");

static void GlobalPointLightShadowFlag()
{
    vector<RetailGlobalPointFixture> original(3);
    std::memset(original.data(), 0, original.size() * sizeof(original[0]));
    for (int i = 0; i < 3; ++i) {
        original[i].color[0] = 0.25f;
        original[i].color[1] = 0.5f;
        original[i].color[2] = 0.75f;
        original[i].center[0] = float(i * 4);
        original[i].radius = float(12 + i);
        original[i].shadow = i != 0;
        original[i].rendered = i == 2;
    }
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE); writer.Add(5, &original); }
    saved.Seek(0);
    NGScene::SGlobalIlluminationInfo global;
    global.vUpDifColor = VNULL3;
    { CStructureSaver reader(saved, CStructureSaver::READ); global.operator&(reader); }
    NGScene::SLightStateCalcSeed seed;
    NGScene::CLightState cached;
    cached.CreateSimple(&seed, global);
    Require(cached.points.size() == 2, "global lighting cache lost points or duplicated an already rendered light");
    Require(!cached.points[0].bCastShadow && cached.points[1].bCastShadow,
        "global illumination discarded the saved point-light shadow flags while building its cache");
    Require(cached.points[1].vCenter == CVec3(4, 0, 0) && cached.points[1].fRadius == 13.f
            && cached.points[1].vColor == CVec3(0.25f, 0.5f, 0.75f),
        "global lighting cache changed saved colour, radius or position");
    CMemoryStream resaved;
    { CStructureSaver writer(resaved, CStructureSaver::WRITE); global.operator&(writer); }
    resaved.Seek(0);
    vector<RetailGlobalPointFixture> roundtrip;
    { CStructureSaver reader(resaved, CStructureSaver::READ); reader.Add(5, &roundtrip); }
    Require(roundtrip.size() == 3 && !roundtrip[0].shadow && roundtrip[1].shadow
            && roundtrip[2].shadow && roundtrip[2].rendered,
        "global lighting resave lost retail rendered/shadow flags");
}

class CameraExecutorReached: public std::runtime_error
{
public:
    CameraExecutorReached(): std::runtime_error("automatic action camera escaped cinematic desktop filtering") {}
};

class QueuedCameraWorld: public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(QueuedCameraWorld);
public:
    vector<CObj<NWorld::CUICmd>> commands;
    std::size_t next = 0;
    NWorld::CUICmd* GetUICommand() override {
        return next < commands.size() ? commands[next++].GetPtr() : nullptr;
    }
};

class CinematicDrainMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(CinematicDrainMission);
    CObj<NUI::CDesktopWindow> desktop;
public:
    void Attach(NWorld::CWorld* world, NUI::CDesktopWindow* top) { pWorld = world; desktop = top; }
    NUI::CDesktopWindow* GetDesktop() const override { return desktop; }
    ICamera* GetCamera() const override { throw CameraExecutorReached(); }
    void Drain() { ExecWorldCommands(); }
    bool TutorialEnabled() const { return bTutorialMode; }
};

class ReadinessWorld: public NWorld::CWorld
{
    OBJECT_NOCOPY_METHODS(ReadinessWorld);
public:
    bool sequence = false;
    bool IsSequence() const override { return sequence; }
};

class ClueDispatchMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(ClueDispatchMission);
    CObj<NUI::CDesktopWindow> desktop;
public:
    void Attach(NWorld::CWorld* world,NRPG::CGlobalGame* game) {
        pWorld = world; pGlobalGame = game; desktop = new NUI::CDesktopWindow;
    }
    void Step() override {}
    void OnGetFocus() override {}
    void OnLostFocus() override {}
    NUI::CDesktopWindow* GetDesktop() const override { return desktop; }
    void DoEvent(NWorld::CCommand* command) override { dynamic_cast<NWorld::CWorld*>(pWorld.GetPtr())->ExecuteCommand(command); }
    void Drain() { ExecWorldCommands(); }
};

static CObj<NWorld::CWorld> FixtureWorld(const char* databasePath,const char* resourcePath,
    bool deployParty,CObj<NRPG::CGlobalGame>* retainedGame);

static void FreshClueWaitRelease(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "fresh clue fixture must begin without an active interface or command");
    struct Cleanup { ~Cleanup() { NMainLoop::StepApp(false,false,false); NMainLoop::DoneInterface(); } } cleanup;
    auto* clue = game->pScenarioTracker->GetClueByName("Find_Cipher_key");
    Require(clue != nullptr, "fresh clue fixture has no authored campaign clue");
    auto* script = world->GetOwnScript();
    Require(script != nullptr, "fresh clue fixture has no native mission script");
    CObj<NWorld::CUICmdShowClue> command = new NWorld::CUICmdShowClue(clue);
    const int event = command->GetID();
    Require(script->AddUICommandWithID(command) == 1 && script->IsUIActionIDPresent(event),
        "fresh clue fixture did not register its actual pending script UI action");
    CObj<ClueDispatchMission> mission = new ClueDispatchMission;
    mission->Attach(world,game); mission->Drain();
    Require(NMainLoop::HaveInterfaceCommand() && script->IsUIActionIDPresent(event),
        "dispatching a clue skipped its modal or prematurely released its script wait");
    Require(NMainLoop::StepApp(false,false,false), "the native queued clue modal did not open");
    auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(modal) && pSSClasses->GetObjectTypeID(modal) == int(0xB1001180),
        "fresh clue dispatch opened an unrelated interface");
    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(modal,&references);
    CPtr<NGame::IMission> origin;
    int restoredEvent = -1;
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(2,&restoredEvent); reader.Add(3,&origin); }
    Require(origin.GetPtr() == mission.GetPtr() && restoredEvent == event,
        "fresh clue dispatch dropped the originating mission and its waiting script event ID");
    NInput::SEvent input;
    while (NInput::GetEvent(&input)) {}
    NInput::PostEvent("cancel");
    Require(NInput::GetEvent(&input) && modal->ProcessEvent(input) && NMainLoop::HaveInterfaceCommand(),
        "the fresh clue modal did not consume its native close command");
    Require(!script->IsUIActionIDPresent(event), "closing a fresh clue left its actual script UI action waiting forever");
    NMainLoop::StepApp(false,false,false);
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "the fresh clue fixture retained its modal after closing");
}

class ReadinessMission: public NGame::CMissionBase
{
    OBJECT_NOCOPY_METHODS(ReadinessMission);
    CObj<NUI::CDesktopWindow> desktop;
public:
    bool clientSequence = false;
    void Attach(NWorld::CWorld* world, NUI::CDesktopWindow* top) { pWorld = world; desktop = top; }
    void AttachActive(NGame::IPlayerTracker* player) { pActivePlayer = player; }
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    bool IsSequence() const override { return clientSequence; }
    bool IsRealTime() const override { return true; }
    NUI::CDesktopWindow* GetDesktop() const override { return desktop; }
};

class IconSceneReached: public std::exception {};

class IconGateMission: public ReadinessMission
{
    OBJECT_NOCOPY_METHODS(IconGateMission);
public:
    mutable int sceneQueries = 0;
    NGScene::IGameView* GetScene() const override { ++sceneQueries; throw IconSceneReached(); }
};

class MissionIconProbe: public NUI::CMissionUI
{
    OBJECT_NOCOPY_METHODS(MissionIconProbe);
public:
    void RefreshIcons(bool traps) { if (traps) UpdateTraps(); else UpdateClues(); }
    void RefreshEnemies() { UpdateEnemies(); }
    void RefreshHits(STime time) { UpdateHits(time); }
};

static void OverlayIconVisibility(bool traps)
{
    CObj<IconGateMission> mission = new IconGateMission;
    CObj<MissionIconProbe> desktop = new MissionIconProbe;
    CObj<NUI::CText> pause = new NUI::CText;
    CObj<CObjectBase> icon = pSSClasses->CreateObject(int(traps ? 0xB3618130u : 0xB3131150u));
    Require(IsValid(icon), "native overlay icon class is not registered");
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[mission.GetPtr()] = 1; references.refs[1] = mission.GetPtr();
    references.ids[pause.GetPtr()] = 2; references.refs[2] = pause.GetPtr();
    references.ids[icon.GetPtr()] = 3; references.refs[3] = icon.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CPtr<NGame::IMission> missionRef = mission.GetPtr();
    list<CObj<CObjectBase>> icons; icons.push_back(icon);
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
      writer.Add(2, &missionRef); writer.Add(27, &pause); writer.Add(traps ? 10 : 11, &icons); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(desktop.GetPtr())->operator&(reader); }
    auto countRetained = [&]() {
        int count = 0;
        CStructureNetworkContext graph;
        graph.include = [&](CObjectBase* object) { if (object == icon.GetPtr()) ++count; return false; };
        CMemoryStream traversal;
        { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
          static_cast<CObjectBase*>(desktop.GetPtr())->operator&(writer); }
        return count;
    };
    Require(countRetained() == 1, "overlay fixture did not restore its existing native icon");
    NGlobal::SetVar("ui_showicons", 0);
    bool queriedScene = false;
    try { desktop->RefreshIcons(traps); } catch (const IconSceneReached&) { queriedScene = true; }
    Require(!queriedScene, "disabled overlay icons still queried the scene to rebuild markers");
    Require(countRetained() == 0, "disabling overlay icons retained a stale marker");
    NGlobal::SetVar("ui_showicons", 1);
    try { desktop->RefreshIcons(traps); } catch (const IconSceneReached&) { queriedScene = true; }
    Require(queriedScene && mission->sceneQueries == 1, "enabling overlay icons did not restore native marker updates");
}

static void AIPlayerMissionReadiness()
{
    CObj<ReadinessWorld> world = new ReadinessWorld;
    CObj<ReadinessMission> mission = new ReadinessMission;
    CObj<NUI::CDesktopWindow> gameplay = dynamic_cast<NUI::CDesktopWindow*>(
        pSSClasses->CreateObject(int(0xB0241942)));
    CObj<NGame::IPlayerTracker> player = dynamic_cast<NGame::IPlayerTracker*>(
        pSSClasses->CreateObject(int(0xB2022800))); // Actual CPlayerTracker.
    Require(IsValid(gameplay) && IsValid(player), "AI-player readiness fixture classes are not registered");
    CObj<NRPG::CGlobalPlayer> global = new NRPG::CGlobalPlayer;
    global->bAIPlayer = false;
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[global.GetPtr()] = 1; references.refs[1] = global.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); writer.Add(4, &global); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(player.GetPtr())->operator&(reader); }
    Require(player->GetGlobalPlayer() == global.GetPtr(), "native player tracker lost its restored global player");
    mission->Attach(world, gameplay);
    mission->AttachActive(player);
    Require(mission->IsReady(), "human player was not ready in the ordinary mission desktop");
    global->bAIPlayer = true;
    Require(!mission->IsReady(), "AI-controlled active player remained eligible for manual mission interaction");
    global->bAIPlayer = false;
    Require(mission->IsReady(), "returning to a human active player did not restore mission readiness");
    mission->AttachActive(nullptr);
    Require(mission->IsReady(), "AI-player readiness gate broke the pre-player initialization path");
}

static void MissionOverlayReadiness()
{
    CObj<ReadinessWorld> world = new ReadinessWorld;
    CObj<ReadinessMission> mission = new ReadinessMission;
    mission->Attach(world, nullptr);
    Require(mission->IsReady(), "readiness fixture without an overlay is not interactive");
    CObj<NUI::CDesktopWindow> gameplay = dynamic_cast<NUI::CDesktopWindow*>(
        pSSClasses->CreateObject(int(0xB0241942))); // Actual registered CMissionUI.
    Require(IsValid(gameplay), "native mission desktop class is not registered");
    mission->Attach(world, gameplay);
    Require(mission->IsReady(), "ordinary mission desktop incorrectly blocks interaction");
    vector<CObj<NUI::CDesktopWindow>> overlays;
    overlays.emplace_back(new NUI::CDesktopWindow);
    overlays.emplace_back(new NUI::CMissionDlgUI);
    overlays.emplace_back(new NUI::CMissionMovieUI);
    overlays.emplace_back(new NUI::CMissionFadeUI);
    for (const auto& overlay: overlays) {
        mission->Attach(world, overlay);
        Require(!mission->IsReady(), "mission remained interactive behind a noninteractive desktop overlay");
    }
    mission->Attach(world, gameplay);
    Require(mission->IsReady(), "closing an overlay did not restore ordinary mission interaction");
    mission->clientSequence = true;
    Require(!mission->IsReady(), "desktop readiness bypassed a client-side cinematic sequence");
    mission->clientSequence = false;
    world->sequence = true;
    Require(!mission->IsReady(), "desktop readiness bypassed the remaster's world-sequence guard");
    world->sequence = false;
    Require(mission->IsReady(), "sequence completion did not restore mission readiness");
}

static void CinematicCameraCommands(int kind)
{
    CObj<NUI::CDesktopWindow> desktop;
    if (kind == 0) desktop = new NUI::CMissionDlgUI;
    else if (kind == 1) desktop = new NUI::CMissionMovieUI;
    else desktop = new NUI::CMissionFadeUI;
    CObj<QueuedCameraWorld> world = new QueuedCameraWorld;
    CObj<CinematicDrainMission> mission = new CinematicDrainMission;
    mission->Attach(world, desktop);
    world->commands.emplace_back(new NWorld::CUICmdUnitCamera(nullptr, 1, false, 0.f, nullptr));
    world->commands.emplace_back(new NWorld::CUICmdTutorialMode(true));
    // Exercise the actual mission queue drain: automatic locators must be
    // rejected before they instantiate a camera executor or lock the camera.
    mission->Drain();
    Require(mission->TutorialEnabled(), "cinematic command filter discarded an unrelated script command");
    world->commands.emplace_back(new NWorld::CUICmdScriptMoveCamera({}, 0));
    bool scriptedCameraAccepted = false;
    try { mission->Drain(); }
    catch (const CameraExecutorReached&) { scriptedCameraAccepted = true; }
    Require(scriptedCameraAccepted, "cinematic command filter blocked a scripted camera move");

    CObj<NUI::CDesktopWindow> ordinary = new NUI::CDesktopWindow;
    CObj<QueuedCameraWorld> ordinaryWorld = new QueuedCameraWorld;
    CObj<CinematicDrainMission> ordinaryMission = new CinematicDrainMission;
    ordinaryMission->Attach(ordinaryWorld, ordinary);
    ordinaryWorld->commands.emplace_back(new NWorld::CUICmdUnitCamera(nullptr, 1, false, 0.f, nullptr));
    bool ordinaryCameraAccepted = false;
    try { ordinaryMission->Drain(); }
    catch (const CameraExecutorReached&) { ordinaryCameraAccepted = true; }
    Require(ordinaryCameraAccepted, "ordinary mission desktop lost automatic camera commands");
}

static void CompletedZoneRecommendation()
{
    CObj<NRPG::CGlobalPlayer> player = new NRPG::CGlobalPlayer;
    CObj<NRPG::CUnit> mercenary = new NRPG::CUnit;
    mercenary->skills[NDb::ST_VP] = new NRPG::CDynamicSkill(100);
    mercenary->skills[NDb::ST_LEVEL] = new NRPG::CDynamicSkill(5);
    player->mercs.push_back(mercenary);
    CObj<NScenario::CScenarioTracker> tracker = new NScenario::CScenarioTracker;
    CObj<NDb::CDBScenarioClue> clueRecord = new NDb::CDBScenarioClue;
    clueRecord->clueType = NDb::CT_PERSON;
    CObj<NScenario::CScenarioClue> firstClue = new NScenario::CScenarioClue(clueRecord, 1);
    CObj<NScenario::CScenarioClue> secondClue = new NScenario::CScenarioClue(clueRecord, 2);
    vector<CObj<NDb::CDBScenarioZone>> zoneRecords;
    auto makeZone = [&](NScenario::CScenarioClue* clue, int difficulty) {
        CObj<NDb::CDBScenarioZone> record = new NDb::CDBScenarioZone;
        zoneRecords.push_back(record);
        CObj<NScenario::CScenarioZone> zone = new NScenario::CScenarioZone(record, difficulty);
        // Populate one person-clue slot through the existing save field. No
        // map resource is needed for the recommendation's scenario graph.
        record->templatesIDs.push_back(1);
        unordered_map<int, NScenario::CScenarioZone::STemplate> slots;
        slots[1].nPersonSlots = slots[1].nEmptyPersonSlots = 1;
        CMemoryStream state;
        { CStructureSaver writer(state, CStructureSaver::WRITE); writer.Add(13, &slots); }
        state.Seek(0);
        { CStructureSaver reader(state, CStructureSaver::READ); static_cast<CObjectBase*>(zone.GetPtr())->operator&(reader); }
        zone->PlaceClue(clue);
        zone->SetDifficulty(difficulty);
        tracker->OpenZone(zone);
        return zone;
    };
    auto first = makeZone(firstClue, 1), second = makeZone(secondClue, 3);
    Require(tracker->GetRecommendedZone(player) == first.GetPtr(), "fixture zone priority is unexpected");
    first->SetPassed();
    Require(tracker->GetRecommendedZone(player) == second.GetPtr(),
        "completed mission with a leftover clue was recommended again");
    second->SetPassed();
    Require(!tracker->GetRecommendedZone(player), "all completed missions still produced a recommendation");
    first->SetPassed(false);
    Require(tracker->GetRecommendedZone(player) == first.GetPtr(), "unfinished mission disappeared from recommendations");
    firstClue->SetDestroyed(true);
    Require(!tracker->GetRecommendedZone(player), "destroyed clue produced a recommendation");
    firstClue->SetDestroyed(false);
    tracker->BlockZone(first);
    Require(!tracker->GetRecommendedZone(player), "blocked mission produced a recommendation");
}

static void PerkTooltipParameters()
{
    CObj<NDb::CDBPerk> perk = new NDb::CDBPerk;
    CObj<NDb::CString> description = new NDb::CString;
    description->szStr = L"<value paramonepoint> <value paramoneperc> <value paramtwopoint> <value paramtwoperc>";
    perk->pToolTip = description;
    perk->params = {1.256f, .346f};
    CObj<NUI::CToolTip> tooltip = NUI::CreatePerkToolTip(nullptr, perk);
    auto* text = dynamic_cast<NUI::CText*>(tooltip->GetChildByID(""));
    Require(text && text->GetText() == description->szStr, "perk tooltip lost its localized description");
    wstring value;
    Require(text->GetVal(L"paramonepoint", &value) && value == L"1",
        "perk tooltip omitted its numeric point parameter");
    Require(text->GetVal(L"paramoneperc", &value) && value == L"26", "perk tooltip percentage did not round its multiplier delta");
    Require(text->GetVal(L"paramtwopoint", &value) && value == L"0", "perk tooltip point parameter did not truncate");
    Require(text->GetVal(L"paramtwoperc", &value) && value == L"35", "perk tooltip fractional percentage is incorrect");
    Require(perk->params == vector<float>({1.256f, .346f}), "tooltip formatting changed perk mechanics");
    perk->params = {2.f, -.4f};
    tooltip = NUI::CreatePerkToolTip(nullptr, perk);
    text = dynamic_cast<NUI::CText*>(tooltip->GetChildByID(""));
    Require(text->GetVal(L"paramoneperc", &value) && value == L"100", "perk tooltip multiplier percentage is incorrect");
    Require(text->GetVal(L"paramtwoperc", &value) && value == L"-40", "perk tooltip negative percentage is incorrect");
    perk->params = {1.f};
    tooltip = NUI::CreatePerkToolTip(nullptr, perk);
    text = dynamic_cast<NUI::CText*>(tooltip->GetChildByID(""));
    Require(text->GetVal(L"paramoneperc", &value) && value == L"100", "perk tooltip changed the exact-one percentage boundary");
    Require(!text->GetVal(L"paramtwopoint", &value), "single-parameter perk acquired a second substitution");
    perk->params.clear();
    tooltip = NUI::CreatePerkToolTip(nullptr, perk);
    text = dynamic_cast<NUI::CText*>(tooltip->GetChildByID(""));
    Require(!text->GetVal(L"paramonepoint", &value), "parameter-free perk inherited a numeric substitution");
    perk->pToolTip = nullptr;
    perk->szUserName = "Fallback perk";
    tooltip = NUI::CreatePerkToolTip(nullptr, perk);
    text = dynamic_cast<NUI::CText*>(tooltip->GetChildByID(""));
    Require(text && text->GetText() == L"Fallback perk", "perk tooltip lost its name fallback");
}

static void MechanicalStepSound()
{
    CObj<NDb::CRPGArmor> armor = new NDb::CRPGArmor;
    armor->nAISoundType = 17;
    CObj<NDb::CPanzerklein> suit = new NDb::CPanzerklein;
    CObj<NDb::CTSound> ordinary = new NDb::CTSound, mechanical = new NDb::CTSound, empty = new NDb::CTSound;
    CObj<NDb::CSoundVariant> ordinaryVariant = new NDb::CSoundVariant, mechanicalVariant = new NDb::CSoundVariant;
    CObj<NDb::CSound> ordinarySound = new NDb::CSound, mechanicalSound = new NDb::CSound;
    ordinaryVariant->pSound = ordinarySound.GetPtr(); mechanicalVariant->pSound = mechanicalSound.GetPtr();
    ordinary->variants.push_back(ordinaryVariant.GetPtr()); ordinary->roulette.AddSector(1.f);
    mechanical->variants.push_back(mechanicalVariant.GetPtr()); mechanical->roulette.AddSector(1.f);
    armor->pSoundStep = ordinary.GetPtr(); suit->pStepSound = mechanical.GetPtr();
    SRand random(SRandomSeed(123));
    Require(NWorld::SelectUnitStepSound(armor, suit, &random) == mechanicalSound.GetPtr(),
        "worn Panzerklein played the pilot armor's ordinary footsteps");
    Require(NWorld::SelectUnitStepSound(nullptr, suit, &random) == mechanicalSound.GetPtr(), "mechanical footsteps depended on pilot armor");
    Require(NWorld::SelectUnitStepSound(armor, nullptr, &random) == ordinarySound.GetPtr(), "ordinary footsteps changed for an unsuited unit");
    suit->pStepSound = nullptr;
    Require(NWorld::SelectUnitStepSound(armor, suit, &random) == ordinarySound.GetPtr(), "missing mechanical footsteps lost the armor fallback");
    suit->pStepSound = empty.GetPtr();
    Require(!NWorld::SelectUnitStepSound(armor, suit, &random), "empty mechanical sound set produced a sound");
    armor->pSoundStep = empty.GetPtr();
    Require(!NWorld::SelectUnitStepSound(armor, nullptr, &random), "empty ordinary sound set produced a sound");
    Require(!NWorld::SelectUnitStepSound(nullptr, nullptr, &random), "missing footstep definitions produced a sound");
    Require(armor->nAISoundType == 17, "mechanical sound selection changed AI noise classification");
}

static void TemporaryGroups()
{
    CObj<NWorld::CWorld> world = new NWorld::CWorld;
    CObj<NWorld::CUnitGroup> temporary = world->CreateUnitGroup(42);
    Require(temporary->GetID() == 42, "temporary group lost its requested ID");
    Require(!world->GetUnitGroup(42), "temporary group leaked into the world registry");
    for (int i = 0; i < 100; ++i) {
        CObj<NWorld::CUnitGroup> query = world->CreateUnitGroup(-1);
        Require(!world->GetUnitGroup(query->GetID()), "query claimed a reinforcement ID");
    }
}

static void ScrollInitialization()
{
    for (unsigned char pattern : {0xff, 0x11}) {
        void* memory = ::operator new(sizeof(NUI::CScrollWindowBase));
        std::memset(memory, pattern, sizeof(NUI::CScrollWindowBase));
        CObj<NUI::CScrollWindowBase> scroll =
            new(memory) NUI::CScrollWindowBase(NUI::SWindowInfo());
        Require(scroll->GetValue().x == 0 && scroll->GetValue().y == 0,
            "new paper window inherits stale scroll offsets");
    }
}

class AttackMission: public NGame::CMissionBase
{
    OBJECT_BASIC_METHODS(AttackMission);
public:
    int probes = 0;
    int selected = 0;
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    int CountSelected() override { return selected; }
    void GetSelectedUnits(vector<CPtr<NGame::IUnitTracker>>* result) const override { result->clear(); }
    bool IsRealTime() const override { return false; }
    bool GetTracePosition(CVec3* position) const override { *position = VNULL3; return true; }
    void CanDoCommand(NWorld::CCmd*, bool, NGame::SActionInfo* info) override {
        ++probes;
        *info = NGame::SActionInfo();
    }
};

class AttackState: public NGame::CStateAttack
{
public:
    NWorld::CCmd* TargetCommand() { return GetTargetCmd(); }
    void Attach(NGame::IMission* mission) { CStateBase::Initialize(mission); }
    void RefreshCursor() { UpdateCursor(); }
    void RefreshCaption() { UpdateCursorInfo(); }
};

class AttackCursorUnit: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(AttackCursorUnit);
public:
    EState cursorState = ST_NORMAL_MELEE;
    EState GetState() override { return cursorState; }
};

class AttackCursorMission: public AttackMission
{
    OBJECT_NOCOPY_METHODS(AttackCursorMission);
public:
    vector<CPtr<NGame::IUnitTracker>> selection;
    bool traceAvailable = true;
    void GetSelectedUnits(vector<CPtr<NGame::IUnitTracker>>* result) const override { *result = selection; }
    int CountSelected() override { return static_cast<int>(selection.size()); }
    bool GetTracePosition(CVec3* result) const override { *result = VNULL3; return traceAvailable; }
};

class ToolTargetPlayer: public NWorld::CPlayer
{
    OBJECT_NOCOPY_METHODS(ToolTargetPlayer);
public:
    list<CPtr<CObjectBase>> traps;
    void GetTrappedObjectsList(list<CPtr<CObjectBase>>* result) const override { *result = traps; }
};

class ToolTargetPlayerTracker: public NGame::CPlayerTracker
{
    OBJECT_NOCOPY_METHODS(ToolTargetPlayerTracker);
public:
    CObj<ToolTargetPlayer> player = new ToolTargetPlayer;
    NWorld::IPlayer* GetPlayer() const override { return player; }
};

class ToolTargetMission: public AttackCursorMission
{
    OBJECT_NOCOPY_METHODS(ToolTargetMission);
public:
    CObj<ToolTargetPlayerTracker> player = new ToolTargetPlayerTracker;
    CPtr<CObjectBase> target;
    NGame::EUnitAction requested = NGame::UA_USE;
    NGame::SActionInfo general, targeted;
    int commands = 0, targetQueries = 0;
    ToolTargetMission() { general.SetValid(); targeted.SetValid(); targeted.nMinAP = targeted.nMaxAP = 7; }
    NGame::IPlayerTracker* GetActivePlayer() const override { return player; }
    CObjectBase* GetStateTarget(bool* fromWorld = nullptr) const override {
        if (fromWorld) *fromWorld = true;
        return target;
    }
    void GetActionInfo(NGame::EUnitAction action,NGame::SActionInfo* result) override {
        requested = action;
        *result = action == NGame::UA_USETOOL ? general : NGame::SActionInfo();
    }
    void CanDoCommand(NWorld::CCmd* command,bool,NGame::SActionInfo* result) override {
        auto* untrap = dynamic_cast<NWorld::CCmdUntrapObject*>(command);
        Require(untrap && untrap->pTarget == target.GetPtr(), "tool cursor probed the wrong command or hovered target");
        ++targetQueries; *result = targeted;
    }
    void Command(NWorld::CUnit*,NWorld::CCmd*,bool) override { ++commands; }
};

static void ForcedToolTargetingEntry(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    CObj<ToolTargetMission> mission = new ToolTargetMission;
    CObj<AttackCursorUnit> unit = new AttackCursorUnit;
    unit->cursorState = NWorld::CUnit::ST_NORMAL_TOOL;
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission,unit);
    mission->selection.push_back(tracker.GetPtr());
    CObj<NGame::CStateUntrap> state = new NGame::CStateUntrap(true);
    const bool entered = state->Initialize(mission);
    Require(mission->requested == NGame::UA_USETOOL,
        "forced disarm targeting queried generic Use eligibility instead of UseTool");
    Require(entered && state->GetType() == NGame::IState::FORCED,
        "forced disarm targeting could not begin before a trapped object was hovered");
    Require(state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK) && mission->targetQueries == 0,
        "tool targeting without an object did not begin with a blocked cursor");
    Require(state->OnLButtonUp(0,0) && mission->commands == 0,
        "clicking empty tool-targeting space queued a command");
    CObj<NGame::CStateUntrap> hover = new NGame::CStateUntrap(false);
    Require(!hover->Initialize(mission), "automatic disarm hover mode entered without a known trap");
    mission->general = NGame::SActionInfo();
    CObj<NGame::CStateUntrap> denied = new NGame::CStateUntrap(true);
    Require(!denied->Initialize(mission), "forced tool mode ignored failed general action eligibility");
    mission->general.eResult = NWorld::UCR_PENDING;
    CObj<NGame::CStateUntrap> pending = new NGame::CStateUntrap(true);
    Require(pending->Initialize(mission), "forced tool mode lost the remaster's asynchronous pending eligibility");
    mission->general.SetValid();
    // The general action query owns selection/tool eligibility, as for the other targeting modes.
    unit->cursorState = NWorld::CUnit::ST_NORMAL_RIFLE;
    mission->selection.push_back(tracker.GetPtr());
    CObj<NGame::CStateUntrap> aggregate = new NGame::CStateUntrap(true);
    Require(aggregate->Initialize(mission), "tool targeting overrode the mission's aggregate action eligibility");
    Require(mission->commands == 0, "entering or rejecting tool mode issued gameplay commands");
}

static void ToolTargetingCursorRefresh(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    Require(NDb::GetUICursor(NGame::N_CURSOR_USE_TOOL) && NDb::GetUICursor(NGame::N_CURSOR_BLOCK),
        "tool cursor fixture has no authored tool or blocked cursor");
    CObj<ToolTargetMission> mission = new ToolTargetMission;
    CObj<NWorld::CWindowDoor> trap = new NWorld::CWindowDoor;
    mission->target = trap.GetPtr(); mission->player->player->traps.push_back(trap.GetPtr());
    CObj<NGame::CStateUntrap> state = new NGame::CStateUntrap(true);
    Require(state->Initialize(mission) && mission->targetQueries == 1 &&
        state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_USE_TOOL),
        "tool cursor control did not start with a valid known trap and its real tool cursor");
    const auto caption = [&](int ap) {
        return NUI::GetDBString(20276)+NUI::GetDBString(19808)+std::to_wstring(ap);
    };
    Require(state->GetCursorInfo().wsText == caption(7), "initial native tool cursor lost its action-point caption");
    mission->targeted.bOk = false;
    state->Step();
    Require(mission->targetQueries == 2 && state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK) &&
        state->GetCursorInfo().wsText.empty(),
        "the native disarm cursor retained valid targeting after the hovered action became unavailable");
    mission->targeted.SetValid(); mission->targeted.nMinAP = mission->targeted.nMaxAP = 11;
    state->Step();
    Require(mission->targetQueries == 3 && state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_USE_TOOL) &&
        state->GetCursorInfo().wsText == caption(11), "disarm cursor did not recover with the current target's action-point cost");
    mission->target = nullptr;
    state->Step();
    Require(mission->targetQueries == 3 && state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK) &&
        state->GetCursorInfo().wsText.empty(), "moving off a trap retained its tool cursor or action-point caption");
    mission->target = trap.GetPtr(); mission->player->player->traps.clear();
    state->Step();
    Require(mission->targetQueries == 3 && state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK),
        "tool cursor exposed an unknown trap to its targeting probe");
    CObj<NGame::CStateUntrap> hover = new NGame::CStateUntrap(false);
    Require(!hover->Initialize(mission), "automatic hover state entered for an unknown trap");
    mission->player->player->traps.push_back(trap.GetPtr());
    Require(hover->Initialize(mission) && hover->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_USE_TOOL),
        "automatic disarm hover did not activate for a known valid trap");
    Require(mission->commands == 0, "refreshing a disarm cursor issued a gameplay command");
}

static void SetAttackPreview(AttackState* state, const NGame::SActionInfo& info)
{
    CMemoryStream stream;
    auto value = info;
    auto location = NAI::HL_ANY;
    { CStructureSaver writer(stream, CStructureSaver::WRITE); writer.Add(3, &value); writer.Add(5, &location); }
    stream.Seek(0);
    { CStructureSaver reader(stream, CStructureSaver::READ); static_cast<CObjectBase*>(state)->operator&(reader); }
}

static void MeleeAttackCursor(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    CObj<AttackCursorMission> mission = new AttackCursorMission;
    CObj<AttackCursorUnit> unit = new AttackCursorUnit;
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission, unit);
    mission->selection.push_back(tracker.GetPtr());
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    NGame::SActionInfo info;
    info.SetValid();
    SetAttackPreview(state, info);
    NInput::SEvent event;
    while (NInput::GetEvent(&event)) {}
    for (auto entry : {std::make_pair(NWorld::CUnit::ST_NORMAL_MELEE, 8),
                       std::make_pair(NWorld::CUnit::ST_NORMAL_KNIFE, 9),
                       std::make_pair(NWorld::CUnit::ST_NORMAL_RIFLE, 10)}) {
        Require(NDb::GetUICursor(entry.second) != nullptr, "fixture attack cursor record is absent");
        unit->cursorState = entry.first;
        state->RefreshCursor();
        Require(state->GetCursorInfo().pCursor == NDb::GetUICursor(entry.second),
            "attack cursor did not distinguish fists, knife and firearm");
    }
    info.eResult = NWorld::UCR_OK_RELOAD;
    SetAttackPreview(state, info);
    state->RefreshCursor();
    Require(state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_RELOAD),
        "attack cursor lost replacement-reload priority");
    info.bOk = false;
    info.eResult = NWorld::UCR_UNAVAILABLE;
    SetAttackPreview(state, info);
    state->RefreshCursor();
    Require(state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK),
        "unavailable attack did not retain the blocked cursor");
}

static void AttackCaptionWithoutHitChance(const char* databasePath, bool reload, bool missingTarget = false)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    CObj<AttackCursorMission> mission = new AttackCursorMission;
    mission->traceAvailable = !missingTarget;
    CObj<AttackCursorUnit> unit = new AttackCursorUnit;
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission, unit);
    mission->selection.push_back(tracker.GetPtr());
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    NGame::SActionInfo info;
    info.bValid = info.bAvailable = true;
    info.bOk = info.bEnoughAP = reload || missingTarget;
    info.nMinAP = reload ? 5 : 17;
    info.nMaxAP = reload ? 5 : 21;
    info.eResult = reload ? NWorld::UCR_OK_RELOAD : missingTarget ? NWorld::UCR_OK : NWorld::UCR_TARGET_OUT_OF_RANGE;
    SetAttackPreview(state, info);
    state->RefreshCaption();
    auto caption = state->GetCursorInfo().wsText;
    const auto expected = NUI::GetDBString(info.bEnoughAP ? 20276 : 19807) + NUI::GetDBString(19808)
        + (reload ? L"5" : L"17-21");
    Require(caption == expected, missingTarget
        ? "attack preview displayed a percentage without a valid attack target"
        : reload
        ? "replacement reload preview displayed a shooting percentage instead of only its AP cost"
        : "unavailable attack preview did not preserve its AP cost without a shooting percentage");
}

static void BindInitialization()
{
    for (unsigned char pattern : {0x01, 0x11}) {
        alignas(NInput::SBindCommand) unsigned char storage[sizeof(NInput::SBindCommand)];
        std::memset(storage, pattern, sizeof(storage));
        auto* command = new(storage) NInput::SBindCommand;
        // Inspect the representation instead of reading an indeterminate bool.
        Require(storage[offsetof(NInput::SBindCommand, bActive)] == 0,
            "new input binding inherits an active flag from allocator memory");
        command->~SBindCommand();
    }
    Require(!NInput::CBind("upstream_fresh_binding").IsActive(),
        "new public binding starts active before the first input frame");
}

static void AttackLocationRefresh()
{
    CObj<AttackMission> mission = new AttackMission;
    vector<CPtr<NGame::IUnitTracker>> selection;
    mission->GetSelectedUnits(&selection);
    Require(selection.empty(), "the worldless attack-location fixture unexpectedly selected native units");
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    // Sample an input frame before updating the cursor, as the game does.
    NInput::SEvent event;
    while (NInput::GetEvent(&event)) {}
    for (const char* name : {"hitlocation_head", "hitlocation_body", "hitlocation_larm",
                            "hitlocation_rarm", "hitlocation_lleg", "hitlocation_rleg"})
        Require(!NInput::CBind(name).IsActive(), "fixture retained a targeting key");
    // Restore a state aimed at the head. With no held targeting key, the next
    // cursor update changes it to HL_ANY, just as releasing the targeting key.
    CMemoryStream stream;
    NAI::EHitLocation location = NAI::HL_HEAD;
    { CStructureSaver saver(stream, CStructureSaver::WRITE); saver.Add(5, &location); }
    stream.Seek(0);
    { CStructureSaver saver(stream, CStructureSaver::READ);
      static_cast<CObjectBase*>(state.GetPtr())->operator&(saver); }
    state->RefreshCursor();
    Require(mission->probes == 1, "changed aim location retained stale attack availability");
    state->RefreshCursor();
    Require(mission->probes == 1, "unchanged aim location needlessly refreshed availability");
}

static void FlyingRestoration()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    NAI::SPathPlace flight(10, 10, 200);
    flight.SetFinal(1); // Layer encodes altitude, not a ground-layer index.
    Require(network->IsPassable(flight), "flying pocket restore treated altitude as a ground layer");
    Require(network->GetPassability(flight) == NAI::AIP_YES, "flying passability treated altitude as a ground layer");
    flight.SetIntegral(0);
    Require(network->IsPassable(flight), "flying altitude was treated as a ladder index");
}

template<class T>
static auto SnipingCommand(int, NAI::EHitLocation location)
    -> decltype(new T(nullptr, nullptr, location))
{
    return new T(nullptr, nullptr, location);
}

template<class T>
static auto SnipingCommand(long, NAI::EHitLocation)
    -> decltype(new T(nullptr, nullptr)) { return new T(nullptr, nullptr); }

static void SnipingAim()
{
    for (auto expected : {NAI::HL_HEAD, NAI::HL_LLEG, NAI::HL_ANY}) {
        CObj<NWorld::CExecSnipeAim> command = SnipingCommand<NWorld::CExecSnipeAim>(0, expected);
        CMemoryStream stream;
        { CStructureSaver saver(stream, CStructureSaver::WRITE);
          static_cast<CObjectBase*>(command.GetPtr())->operator&(saver); }
        stream.Seek(0);
        NAI::EHitLocation actual = NAI::HL_BODY;
        { CStructureSaver saver(stream, CStructureSaver::READ); saver.Add(3, &actual); }
        Require(actual == expected, "sniping preparation replaced the requested aim point with the body");
    }
}

class ExplosionDecal: public NGScene::CExplosionDecalGeometry
{
public:
    ExplosionDecal(): CExplosionDecalGeometry(nullptr, {}, VNULL3, 10.f, 0.f) {}
    void Rebuild(NGScene::CObjectInfo::SData* result, const NGScene::CObjectInfo& source) {
        CExplosionDecalGeometry::Recalc(result, source, source.GetPositions());
    }
};

static void DecalRebuild()
{
    NGScene::CObjectInfo::SData data;
    for (auto position : {CVec3(-1,-1,0), CVec3(1,-1,0), CVec3(0,1,0)}) {
        NGScene::SVertex vertex{};
        vertex.pos = position;
        data.verts.push_back(vertex);
    }
    data.geometry.SetTriangles({STriangle(0,1,2)});
    CObj<NGScene::CObjectInfo> source = new NGScene::CObjectInfo;
    source->AssignFast(data);
    CObj<ExplosionDecal> decal = new ExplosionDecal;
    NGScene::CObjectInfo::SData result;
    decal->Rebuild(&result, *source);
    const auto count = result.verts.size();
    Require(count == 3, "invalid explosion decal test fixture");
    for (int reload = 0; reload < 20; ++reload) {
        decal->Rebuild(&result, *source);
        Require(result.verts.size() == count, "decal rebuild appends another copy of saved vertices");
        Require(result.geometry.indices.size() == 3, "decal triangle count changed on rebuild");
    }
}

class MeshNode: public CPtrFuncBase<NGScene::CObjectInfo>
{
    OBJECT_BASIC_METHODS(MeshNode);
protected:
    bool NeedUpdate() override { return false; }
    void Recalc() override {}
public:
    explicit MeshNode(NGScene::CObjectInfo* mesh = nullptr) { pValue = mesh; }
};

class GeometryPart: public NGScene::IPart
{
    OBJECT_BASIC_METHODS(GeometryPart);
public:
    NGScene::ETransformType transform = NGScene::TT_NONE;
    GeometryPart() {}
    explicit GeometryPart(MeshNode* mesh): IPart(mesh, nullptr, false) {}
    NGScene::ETransformType GetTransformType() const override { return transform; }
};

class BoundsProbe: public NGScene::CVBCombiner
{
    OBJECT_NOCOPY_METHODS(BoundsProbe);
public:
    explicit BoundsProbe(CFuncBase<std::vector<CPtr<NGScene::IPart>>>* parts = nullptr):
        CVBCombiner(parts, NGScene::LT_NONE, NGScene::CT_DYNAMIC, nullptr) {}
    void Calculate() { XFormPosition(); }
};

static void NegativeMeshBounds()
{
    for (float shift : {0.f, 20.f, 4.f}) {
        NGScene::CObjectInfo::SData data;
        for (auto position : {CVec3(-4,-6,-8), CVec3(-2,-5,-7), CVec3(-3,-4,-9)}) {
            NGScene::SVertex vertex{};
            vertex.pos = position + CVec3(shift, shift, shift);
            data.verts.push_back(vertex);
        }
        data.geometry.SetTriangles({STriangle(0,1,2)});
        CObj<NGScene::CObjectInfo> mesh = new NGScene::CObjectInfo;
        mesh->AssignFast(data);
        CObj<MeshNode> node = new MeshNode(mesh);
        CObj<GeometryPart> part = new GeometryPart(node);
        CObj<NGScene::CAutomaticCombiner> parts = new NGScene::CAutomaticCombiner;
        parts->AddPart(part);
        CObj<BoundsProbe> combiner = new BoundsProbe(parts);
        combiner->Calculate();
        Require(part->vBVMax == CVec3(shift-2, shift-4, shift-7),
            "negative-coordinate mesh bounds incorrectly include the origin");
        Require(part->vBVMin == CVec3(shift-4, shift-6, shift-9), "mesh bounds lost the minimum vertex");
    }
}

class ProjectionRecorder: public NGScene::CPerPolyDecal
{
    OBJECT_BASIC_METHODS(ProjectionRecorder);
protected:
    void Recalc(NGScene::CObjectInfo::SData*, const NGScene::CObjectInfo&,
                const std::vector<CVec3>& positions) override { captured = positions; }
public:
    std::vector<CVec3> captured;
    ProjectionRecorder() {}
    ProjectionRecorder(GeometryPart* part, const std::vector<CVec3>& saved): CPerPolyDecal(part, saved) {}
    void Rebuild() { CPerPolyDecal::Recalc(); }
};

static void DecalProjection()
{
    NGScene::CObjectInfo::SData data;
    for (auto position : {CVec3(1,2,0), CVec3(3,4,0), CVec3(5,6,0)}) {
        NGScene::SVertex vertex{}; vertex.pos = position; data.verts.push_back(vertex);
    }
    data.geometry.SetTriangles({STriangle(0,1,2)});
    CObj<NGScene::CObjectInfo> mesh = new NGScene::CObjectInfo;
    mesh->AssignFast(data);
    CObj<MeshNode> node = new MeshNode(mesh);
    CObj<GeometryPart> part = new GeometryPart(node);
    auto saved = mesh->GetPositions();
    std::reverse(saved.begin(), saved.end());
    CObj<ProjectionRecorder> terrain = new ProjectionRecorder(part, saved);
    terrain->Rebuild();
    Require(terrain->captured == mesh->GetPositions(), "terrain decal reused a stale saved vertex order");
    part->transform = NGScene::TT_SIMPLE;
    CObj<ProjectionRecorder> transformed = new ProjectionRecorder(part, saved);
    transformed->Rebuild();
    Require(transformed->captured == saved, "transformed decal lost its original projection pose");
}

static CObj<NWorld::CWorld> FixtureWorld(const char* databasePath, const char* resourcePath,
    bool deployParty = false, CObj<NRPG::CGlobalGame>* retainedGame = nullptr)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    if (retainedGame) *retainedGame = game;
    if (deployParty) game->players.push_back(NRPG::CreateGlobalPlayer(std::vector<int>{54}));
    CObj<NWorld::CWorld> world = new NWorld::CWorld(game);
    CObj<NWorld::CPostWorldCreateInfo> post;
    world->CreateRandom(810, {}, true, {}, 0, &post, SRandomSeed(123));
    Require(post && world->GetPathNetwork(), "mission fixture construction failed");
    if (deployParty) {
        CObj<NAI::CSequenceCommander> commander = new NAI::CSequenceCommander(world);
        auto* player = dynamic_cast<NWorld::CPlayer*>(world->AddPlayer(L"Clue fixture", game->players.front(), commander));
        Require(player != nullptr, "human party deployment failed");
        commander->SetPlayer(player);
    }
    // Deliberately exclude mission scripts: this fixture isolates AI scheduling.
    world->RunPostInit(nullptr);
    return world;
}

class ScriptSoundDispatchMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(ScriptSoundDispatchMission);
    CObj<NUI::CDesktopWindow> desktop;
public:
    void Attach(NWorld::CWorld* world,NRPG::CGlobalGame* game,NSound::ISoundScene* scene) {
        pWorld = world; pGlobalGame = game; pSoundScene = scene; desktop = new NUI::CDesktopWindow;
    }
    NUI::CDesktopWindow* GetDesktop() const override { return desktop; }
    void Drain() { ExecWorldCommands(); }
};

static void ScriptSoundStop(const char* databasePath,const char* resourcePath,bool spatial,bool unassigned = false,STime interfaceTime = 0)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr,"script sound fixture requires headless native audio");
    Require(NFMSound::Init(NFMSound::SStartInfo()),"script sound fixture could not initialize native audio");
    struct AudioCleanup { ~AudioCleanup() { ClearHoldQueue(); NFMSound::Done(); } } audioCleanup;
    NFMSound::SetMusicMasterVolume(0);
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    while (auto* pending = world->GetUICommand()) { CObj<NWorld::CUICmd> release = pending; }
    CObj<NSound::ISoundScene> scene = NSound::CreateSoundScene(nullptr,nullptr,world->GetAimTime());
    CObj<ScriptSoundDispatchMission> mission = new ScriptSoundDispatchMission;
    mission->Attach(world,game,scene);
    struct ClockRestore { STime previous; ~ClockRestore() { NMainLoop::SetTimeForHarness(previous); } } clock{NMainLoop::SetTimeForHarness(interfaceTime)};
    CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
    Require(sounds.MoveNext(),"script sound fixture has no native sound record");
    auto* record = sounds.Get(); record->bLoop = true; record->nStartSamples = record->nEndingSamples = 0;
    const auto wave = SilentPCMFixture(44100);
    CObj<NFMSound::CSample2D> sample2D = NFMSound::LoadSample2D(wave.data(),static_cast<int>(wave.size()));
    CObj<NFMSound::CSample3D> sample3D = NFMSound::LoadSample3D(wave.data(),static_cast<int>(wave.size()),1.f,100.f,0);
    Require(IsValid(sample2D) && IsValid(sample3D),"script sound fixture could not decode its native PCM samples");
    CObj<NSound::CFileSample2D> loader2D = NSound::share2DSamples.Get(record->GetRecordID());
    CObj<NSound::CFileSample3D> loader3D = NSound::share3DSamples.Get(record->GetRecordID());
    (loader2D.GetPtr()->*CachedChapterSampleAccess::ValueMember()) = sample2D.GetPtr();
    (loader3D.GetPtr()->*CachedEffectSampleAccess::ValueMember()) = sample3D.GetPtr();
    auto* script = world->GetOwnScript();
    Require(IsValid(script),"script sound fixture has no actual world Lua state");
    CObj<NScript::CLUAObjectPosition> position = new NScript::CLUAObjectPosition(CVec3(0,0,0));
    NScript::luaPushCObj(script->GetState(),position); script->SetGlobal("SoundPosition");
    // A second live userdata exercises commands outside the first root's adjacent GC slot.
    CObj<NScript::CLUAObjectPosition> guard = new NScript::CLUAObjectPosition(CVec3(4,5,6));
    NScript::luaPushCObj(script->GetState(),guard); script->SetGlobal("SoundPositionGuard");
    CObj<ICamera> camera = ::CreateCamera(CAMERA_PC);
    CTransformStack transform; camera->GetTransform(&transform,CVec2(1024,768));
    vector<float> output(512*2);
    auto render = [&]() { Require(S2FmodRenderForTest(output.data(),512) != 0,"script sound fixture could not render native PCM"); };
    auto run = [&](const string& code) {
        Require(script->DoString(code.c_str()) == 0,"script sound fixture could not queue its native Lua command");
        world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        mission->Drain(); scene->Draw(&transform);
    };
    const string play = spatial ? "Play3DSound("+std::to_string(record->GetRecordID())+",SoundPosition)" :
        "PlaySound("+std::to_string(record->GetRecordID())+")";
    auto channelExcept = [&](int except) {
        int found = -1;
        for (int i = 0; i < 128; ++i) if (i != except && FSOUND_IsPlaying(i)) {
            Require(found == -1,"script sound fixture has an unexpected extra channel"); found = i;
        }
        return found;
    };
    auto collect = [&]() {
        Require(script->GetState()->nGCAvoid == 0,"full GC fixture is running inside a collection-avoid scope");
        CObj<NScript::CLUAObjectPosition> orphan = new NScript::CLUAObjectPosition(CVec3(7,8,9));
        CPtr<NScript::CLUAObjectPosition> witness = orphan.GetPtr();
        NScript::luaPushCObj(script->GetState(),orphan); script->Pop(); orphan = nullptr;
        // Drive the VM's full collector, including marking of live globals.
        // luaC_collect alone is the sweep phase and is not a complete GC cycle.
        for (int check = 0; check < 2001; ++check) luaC_checkGC(script->GetState());
        Require(!IsValid(witness),"the GC fixture never collected its unreachable native userdata");
        const int top = script->GetTop(); auto live = script->GetGlobal("SoundPosition");
        Require(live.IsUserData() && live.GetUserData() == position.GetPtr(),"a full Lua collection destroyed a live fixture global");
        script->SetTop(top);
    };
    if (unassigned) {
        Require(script->DoString((play+"; Pause()").c_str()) == 0,"unassigned sound fixture could not queue its Lua producer");
        world->UpdateWorld(world->GetAimTime()->GetValue(),nullptr);
        collect(); // A full collection may occur before the interface consumes the queue.
        mission->Drain(); scene->Draw(&transform);
        const int pending = channelExcept(-1);
        Require(pending >= 0,"an unassigned Lua PlaySound result was collected before its native UI command could start playback");
        Require(mission->IsGamePaused(),"a collected unassigned sound blocked the following native UI command");
        render();
        Require(FSOUND_GetCurrentPosition(pending) > 0,"an unassigned scripted sound did not advance native PCM playback");
        collect(); scene->Draw(&transform); render();
        Require(FSOUND_IsPlaying(pending),"a dispatched unassigned sound did not survive a subsequent full Lua collection");
        mission = nullptr; render();
        Require(!FSOUND_IsPlaying(pending),"mission teardown leaked its unassigned script sound");
        return;
    }
    run("PlayHandle = "+play);
    const int played = channelExcept(-1);
    Require(played >= 0,"native Lua PlaySound dispatch did not start a real PCM channel");
    if (interfaceTime != 0) Require(FSOUND_GetCurrentPosition(played) == 0,
        "a fresh scripted sound inherited absolute interface time as a sample start offset");
    render();
    Require(FSOUND_GetCurrentPosition(played) > 0,"native scripted sound never advanced before StopSound");
    CObj<NFMSound::CSound2D> independent = NFMSound::PlaySound(sample2D);
    const int control = channelExcept(played);
    Require(IsValid(independent) && control >= 0,"script sound fixture did not start its unrelated control");
    run("StopSound(PlayHandle)");
    render();
    Require(!FSOUND_IsPlaying(played),"native Lua StopSound cleared its weak handle but left the actual sound playing");
    Require(FSOUND_IsPlaying(control) && FSOUND_GetCurrentPosition(control) > 0,
        "stopping a scripted sound stopped an unrelated native channel");
    const unsigned beforeControl = FSOUND_GetCurrentPosition(control);
    run("StopSound(PlayHandle)"); render();
    Require(channelExcept(control) == -1 && FSOUND_GetCurrentPosition(control) > beforeControl,
        "repeated StopSound replayed the sample or interrupted unrelated playback");
    run("PlayHandle = "+play);
    const int retained = channelExcept(control);
    Require(retained >= 0,"a new scripted sound failed after stopping the preceding channel");
    run("PlayHandle = nil");
    collect();
    render();
    Require(FSOUND_IsPlaying(retained) && FSOUND_GetCurrentPosition(retained) > 0,
        "collecting a dispatched PlaySound userdata stopped its scene-owned playback");
    mission = nullptr;
    render();
    Require(!FSOUND_IsPlaying(retained) && FSOUND_IsPlaying(control),
        "releasing the mission left its scripted playback alive or stopped unrelated audio");
#else
    Require(false,"script sound test requires the native SFX backend");
#endif
}

class ZoneSnapshotMissionFixture: public NGame::CMission
{
public:
    static auto SaveWorldMethod() { return &ZoneSnapshotMissionFixture::SaveWorld; }
};

static void ZoneSnapshotExcludesHUD(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr, "zone snapshot fixture needs isolated user data");
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    Require(IsValid(world->GetOwnScript()), "zone snapshot fixture has no native world script");
    NMainLoop::CreateProfile("UpstreamZoneSnapshot");
    NMainLoop::SetActiveProfile("UpstreamZoneSnapshot");
    auto* manager = NMainLoop::GetSaveManager();
    struct Cleanup {
        NMainLoop::CSaveManager* manager;
        ~Cleanup() { manager->DeleteSlot(NMainLoop::S_SLOT_ACTIVE); }
    } cleanup{manager};
    CObj<NGame::CMission> mission = new NGame::CMission;
    CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false);
    CObj<NUI::CInterface> interface = new NUI::CInterface(cursor);
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.include = [](CObjectBase*) { return false; };
    const vector<CObjectBase*> objects{game.GetPtr(),world.GetPtr(),cursor.GetPtr(),interface.GetPtr()};
    for (std::size_t i = 0; i < objects.size(); ++i) {
        references.ids[objects[i]] = unsigned(i+1); references.refs[unsigned(i+1)] = objects[i];
    }
    references.nextID = 100;
    CMemoryStream base;
    { CStructureSaver writer(base,CStructureSaver::WRITE,&references);
      writer.Add(2,&game); writer.Add(10,&world); writer.Add(15,&cursor); writer.Add(16,&interface); }
    base.Seek(0);
    { CStructureSaver reader(base,CStructureSaver::READ,&references); mission->NGame::CMissionBase::operator&(reader); }
    CMObj<NUI::CMissionUI> hud = new NUI::CMissionUI(
        NUI::SWindowInfo(interface,NUI::SPoint(0,0),NUI::SPoint(1024,768),"snapshot-hud",NUI::STYLE_ENABLED),mission);
    world->GetOwnScript()->SetScriptInterface(interface);
    Require(world->GetOwnScript()->GetScriptInterface() == interface.GetPtr() &&
        interface->GetChildByID("snapshot-hud") == hud.GetPtr(),
        "zone snapshot fixture did not connect its real world script, HUD and mission");
    {
        // Normal mission saves must keep the UI bridge and the HUD's mission link.
        CObj<NGame::CMission> savedMission = mission.GetPtr(), restoredMission;
        CMemoryStream full;
        { CStructureSaver writer(full,CStructureSaver::WRITE_COMPRESSED); writer.Add(2,&savedMission); }
        full.Seek(0);
        CStructureSaver reader(full,CStructureSaver::READ);
        reader.Add(2,&restoredMission);
        Require(IsValid(restoredMission) && IsValid(restoredMission->GetInterface()),
            "normal native mission save did not restore its registered mission root and interface");
        auto* restoredWorld = dynamic_cast<NWorld::CWorld*>(restoredMission->GetWorld());
        Require(IsValid(restoredWorld) && IsValid(restoredWorld->GetOwnScript()) &&
            restoredWorld->GetOwnScript()->GetScriptInterface() == restoredMission->GetInterface() &&
            IsValid(restoredMission->GetInterface()->GetChildByID("snapshot-hud")),
            "normal native mission save did not retain its script-to-HUD bridge");
    }
    const auto saveWorld = ZoneSnapshotMissionFixture::SaveWorldMethod();
    (mission.GetPtr()->*saveWorld)("upstream-zone-ui.sav");
    Require(world->GetOwnScript()->GetScriptInterface() == interface.GetPtr(),
        "writing a zone snapshot did not restore the live world's HUD bridge");
    CFileStream snapshot;
    snapshot.OpenRead(manager->GetSlotFilePathW(NMainLoop::S_SLOT_ACTIVE,"upstream-zone-ui.sav").c_str());
    CObj<NWorld::IWorld> restored;
    CStructureSaver reader(snapshot,CStructureSaver::READ);
    reader.Add(2,&restored);
    auto* restoredWorld = dynamic_cast<NWorld::CWorld*>(restored.GetPtr());
    Require(IsValid(restoredWorld) && IsValid(restoredWorld->GetOwnScript()),
        "native zone snapshot did not restore its world and script");
    Require(restoredWorld->GetOwnScript()->GetScriptInterface() == nullptr,
        "a native zone snapshot retained the HUD and mission graph through its world-script interface bridge");
}

class PassageGateProbe: public NWorld::CObjectServerBase, public NWorld::IPassageObject
{
    OBJECT_NOCOPY_METHODS(PassageGateProbe);
public:
    bool nearby = false;
    int proximityChecks = 0;
    bool IsBroken() const override { return false; }
    bool CanPass(NWorld::CUnitServer*) override { ++proximityChecks; return nearby; }
    bool UsePassageObject(NWorld::CUnitServer*) override { return false; }
    void GetObjectApproaches(vector<NAI::SPathPlace>*) override {}
    int GetAPRadius() const override { return 4; }
    int GetPassageZoneID() const override { return 0; }
    int GetPassageObjectID() const override { return 123; }
};

static void ScriptedPassageTransition(const char* databasePath,const char* resourcePath, bool emptyPassage = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    CObj<NDb::CDBScenarioZone> zoneRecord = new NDb::CDBScenarioZone;
    CObj<NScenario::CScenarioZone> zone = new NScenario::CScenarioZone(zoneRecord,1);
    game->pCurrentZone = zone;
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (const auto& unit: units)
        if (IsValid(unit) && unit->CanFight() && !unit->IsAIUnit()) { actor = unit; break; }
    Require(actor != nullptr, "scripted passage fixture has no living human party");
    CObj<PassageGateProbe> passage = new PassageGateProbe;
    list<CObj<NWorld::CObjectServerBase>> objects = world->GetObjectsForHarness();
    if (emptyPassage) {
        objects.remove_if([](const CObj<NWorld::CObjectServerBase>& object) {
            auto* exit = dynamic_cast<NWorld::IPassageObject*>(object.GetPtr());
            return exit && exit->GetPassageZoneID() == 0;
        });
    } else objects.push_back(passage.GetPtr());
    CStructureNetworkContext references;
    auto nativeFields = BorrowedObjectFields(world,&references);
    auto id = references.nextID++;
    references.ids[passage.GetPtr()] = id; references.refs[id] = passage.GetPtr();
    CMemoryStream fields;
    { CStructureSaver writer(fields,CStructureSaver::WRITE,&references); writer.Add(20,&objects); }
    fields.Seek(0);
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); static_cast<CObjectBase*>(world.GetPtr())->operator&(reader); }
    const auto& restoredObjects = world->GetObjectsForHarness();
    Require(std::count(restoredObjects.begin(),restoredObjects.end(),passage.GetPtr()) == int(!emptyPassage),
        "scripted passage fixture did not register its passage in the native world");
    auto drainTransitions = [&]() {
        int count = 0;
        while (auto* raw = world->GetUICommand()) {
            CObj<NWorld::CUICmd> command = raw;
            if (dynamic_cast<NWorld::CUICmdContinueChapter*>(raw)) ++count;
        }
        return count;
    };
    drainTransitions();
    if (emptyPassage) {
        actor->MakeUnconscious(VNULL3,true,false);
        Require(actor->IsUnconscious() && !actor->IsDead(), "empty-passage fixture failed to incapacitate its human actor");
        for (const auto& unit: units)
            Require(unit->GetPlayer() != actor->GetPlayer() || unit->IsUnconscious() || unit->GetUnitRPG()->IsDead(),
                "empty-passage fixture still has a conscious party member");
        const int ap = actor->GetAP();
        Require(!world->UsePassageObject(actor,0) && drainTransitions() == 0,
            "an entirely unconscious party left through a nonexistent passage");
        CObj<NScript::CScript> script = NScript::CreateScript(world);
        Require(script->DoString("LeaveToSubZone(0)") == 0, "empty-passage script fixture failed to compile");
        script->ExecuteThreads();
        Require(drainTransitions() == 0, "scripted exit fabricated a missing passage for an unconscious party");
        Require(actor->GetAP() == ap, "rejecting an empty passage spent action points");
        return;
    }
    const int ap = actor->GetAP();
    Require(!world->UsePassageObject(actor,0) && passage->proximityChecks > 0 && drainTransitions() == 0,
        "ordinary passage control accepted a party outside the passage radius");
    const int checks = passage->proximityChecks;
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    Require(script->DoString("LeaveToSubZone(0)") == 0, "scripted passage fixture failed to compile");
    script->ExecuteThreads();
    Require(drainTransitions() == 1,
        "scripted LeaveToSubZone failed to leave after a cutscene while the party was away from the passage");
    Require(passage->proximityChecks == checks,
        "scripted forced passage still performed an ordinary proximity check");
    Require(!world->UsePassageObject(actor,0) && drainTransitions() == 0,
        "scripted forced passage weakened later ordinary passage eligibility");
    passage->nearby = true;
    Require(world->UsePassageObject(actor,0) && drainTransitions() == 1,
        "ordinary passage control rejected a party within the passage radius");
    Require(!world->UsePassageObject(actor,999999) && drainTransitions() == 0,
        "missing passage produced a transition command");
    Require(actor->GetAP() == ap, "scripted or previewed passage transition spent action points");
}

static void FlyingAttackQueue(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (const auto& unit: units) {
        if (!IsValid(unit) || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK()) continue;
        auto* weapon = unit->GetUnitRPG()->GetWeaponItem();
        if (IsValid(weapon) && weapon->GetAmmoQuantity() > 0 && weapon->SetShootMode(NDb::SM_Snap) &&
            NWorld::GetActionType(unit) == NWorld::AT_SHOOT && unit->GetUnitRPG()->CanMove()) {
            actor = unit; break;
        }
    }
    Require(actor != nullptr, "airborne attack fixture has no mobile armed actor");
    const NAI::SUnitPosition original = actor->GetPosition();
    struct PositionRestore {
        NWorld::CUnitServer* unit;
        NAI::SUnitPosition position;
        ~PositionRestore() { unit->SetTemporaryPosition(position); }
    } restore{actor,original};
    NAI::SUnitPosition airborne = original;
    Require(NAI::MakeFlyPos(original.GetCP()+CVec3(0,0,30),world->GetPathNetwork(),&airborne.pos),
        "airborne attack fixture could not encode its flight position");
    airborne.pos.p.SetPose(NAI::CM_STAND);
    actor->SetTemporaryPosition(airborne);
    auto* weapon = actor->GetUnitRPG()->GetWeaponItem();
    const int ap = actor->GetAP(), ammo = weapon->GetAmmoQuantity();
    const CVec3 target = airborne.GetCP()+CVec3(10,0,0);
    CObj<NWorld::CExecShootTile> shot = new NWorld::CExecShootTile(actor,target);
    Require(shot->CanDoIt(airborne) == NWorld::UCR_OK,
        "airborne attack fixture cannot shoot from its unmodified flight position");
    CObj<NWorld::CCmdShootTile> command = new NWorld::CCmdShootTile(target);
    NWorld::EUnitCommandResult error = NWorld::UCR_GENERAL_FAILURE;
    CObj<NWorld::CCommandExecute> executor = NWorld::CreateActionExecutor(actor,command,&error);
    std::printf("Airborne attack queue: final=%d altitude=%d result=%d executor=%d\n",
        int(airborne.pos.p.IsFinal()),airborne.pos.p.GetLayer(),int(error),int(IsValid(executor)));
    Require(IsValid(executor) && error == NWorld::UCR_OK,
        "airborne attack pose candidates lost the flight bit and prevented a valid shot queue");
    auto* mover = dynamic_cast<NWorld::IExecMove*>(executor.GetPtr());
    Require(mover != nullptr, "airborne shot queue has no movement plan");
    NAI::SPathPlace desired;
    NAI::EFindPathParams parameters;
    NWorld::ENeedActiveItem active;
    mover->GetDesiredPlace(&desired,&parameters,&active);
    Require(desired.IsFinal() && desired.GetX() == airborne.pos.p.GetX() &&
        desired.GetY() == airborne.pos.p.GetY() && desired.GetLayer() == airborne.pos.p.GetLayer() &&
        desired.GetPose() == NAI::CM_STAND && desired.GetDirection() ==
            NRPG::GetShootDirection(world->GetPathNetwork(),airborne.pos.p,target),
        "airborne shot queue changed its altitude, place, pose, or target facing");
    Require(actor->GetAP() == ap && weapon->GetAmmoQuantity() == ammo && !actor->HasCommand(),
        "preparing an airborne shot queue spent AP/ammunition or installed a command");
}

static void OwnedAICommanderRosters(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CPlayer>> players;
    world->GetPlayersList(&players);
    list<CPtr<NWorld::CUnitServer>> allUnits;
    world->GetAllUnits(&allUnits);
    Require(players.size() > 1 && !allUnits.empty(), "AI roster fixture has no opposing parties");
    int checked = 0, hostileViews = 0;
    for (const auto& player: players) {
        auto* commander = dynamic_cast<NAI::CAICommander*>(player->GetCommander());
        if (!commander) continue;
        ++checked;
        auto verifyOwnership = [&]() {
            for (const auto& unit: commander->GetUnitsList())
                Require(IsValid(unit) && IsValid(unit->GetUnitServer()) && unit->GetUnitServer()->GetPlayer() == player.GetPtr(),
                    "AI commander retained a foreign unit in its strongly owned roster");
        };
        verifyOwnership();
        auto* state = commander->GetAIState();
        state->Synchronize();
        const auto& allies = *state->GetAllyAIPlayer()->GetUnits();
        const auto& enemies = *state->GetEnemyAIPlayer()->GetUnits();
        int expectedAllies = 0, expectedEnemies = 0;
        for (const auto& server: allUnits) {
            if (!IsValid(server) || !server->CanFight()) continue;
            auto* wrapper = NAI::GetAIUnit(server);
            if (!IsValid(wrapper)) continue;
            if (server->GetPlayer() == player.GetPtr()) {
                ++expectedAllies;
                Require(std::count(allies.begin(), allies.end(), wrapper) == 1 && wrapper->GetAIState() == state,
                    "commander world view lost an owned fighter or attached another commander's state");
            } else if (world->GetDiplomacyState(player.GetPtr(), server->GetPlayer()) == NDb::DS_ENEMY) {
                ++expectedEnemies;
                Require(std::count(enemies.begin(), enemies.end(), wrapper) == 1,
                    "removing foreign ownership also removed a live hostile from the tactical world view");
            }
        }
        Require(allies.size() == expectedAllies && enemies.size() == expectedEnemies,
            "AI tactical views duplicated contacts or mixed ownership with diplomacy");
        if (expectedEnemies) ++hostileViews;
    }
    Require(checked > 1 && hostileViews > 0, "AI roster fixture did not verify multiple commanders and a hostile view");
}

static void UnsupportedNavigationBake(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    auto* network = dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());
    Require(network != nullptr, "navigation bake fixture did not construct the native path network");
    int passable = 0, unsupported = 0;
    for (const auto& layer: network->GetLayers()) {
        for (int y = 0; y < layer->tiles.GetYSize(); ++y) {
            for (int x = 0; x < layer->tiles.GetXSize(); ++x) {
                const auto& tile = layer->tiles[y][x];
                if (tile.nPassable == 0) continue;
                ++passable;
                if (tile.nFloor == 100) ++unsupported;
            }
        }
    }
    std::printf("Navigation bake: %d passable tiles, %d unsupported tiles\n", passable, unsupported);
    Require(passable > 0, "navigation bake fixture did not generate playable surfaces");
    Require(unsupported == 0, "native navigation bake generated passable tiles without supporting surfaces");
}

static void BeginTurnLiveContacts(const char* databasePath, const char* resourcePath, bool staleOwner)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer *watcher = nullptr, *enemy = nullptr, *friendly = nullptr;
    for (const auto& unit: units)
        if (IsValid(unit) && unit->CanFight() && !unit->IsAIUnit() && !unit->IsEmptyPK()) { watcher = unit.GetPtr(); break; }
    Require(watcher, "begin-turn contact fixture has no human watcher");
    for (const auto& unit: units) {
        if (!IsValid(unit) || !unit->CanFight() || unit.GetPtr() == watcher) continue;
        if (!enemy && watcher->GetDiplomacyState(unit) == NDb::DS_ENEMY) enemy = unit.GetPtr();
        if (!friendly && unit->GetPlayer() == watcher->GetPlayer()) friendly = unit.GetPtr();
    }
    if (!friendly) {
        auto* network = world->GetPathNetwork();
        vector<NAI::SPathPlace> places;
        network->GetNearPlaces(SSphere(watcher->GetPosition().GetCP(), 10.f), &places);
        for (auto place: places) {
            place.SetPose(NAI::CM_STAND); place.SetDirection(0);
            if (!network->IsPassable(place) || network->IsLocked(place, true)) continue;
            friendly = world->AddUnitInGame(place, NRPG::CreateUnit(watcher->GetUnitRPG()->GetRPGUnit()->GetPers()),
                dynamic_cast<NWorld::CPlayer*>(watcher->GetPlayer()));
            break;
        }
    }
    Require(enemy && friendly, "begin-turn contact fixture needs an enemy and friendly control");
    auto* owner = NAI::GetAIUnit(watcher);
    auto* contact = NAI::GetAIUnit(enemy);
    Require(IsValid(owner) && IsValid(contact), "begin-turn contact fixture has no native AI wrappers");
    owner->Synchronize(); contact->Synchronize();
    auto* state = owner->GetAIUnitState();
    Require(state && state->pUnit.GetPtr() == owner, "begin-turn fixture lost its native threat owner");
    const auto known = state->GetKnownEnemies();
    for (const auto& unit: known) state->RemoveEnemy(unit);
    const auto possible = state->possibleEnemies.data.units;
    for (const auto& unit: possible) state->RemovePossibleEnemy(unit);
    list<CPtr<NWorld::CUnitServer>> visible;
    if (!staleOwner) { visible.push_back(enemy); visible.push_back(nullptr); visible.push_back(friendly); }
    // Restore the real TBS visibility base through its existing save format.
    // Planning HP is a different snapshot and may already have been spent.
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[enemy] = 1; references.refs[1] = enemy;
    references.ids[friendly] = 2; references.refs[2] = friendly;
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); writer.Add(2, &visible); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<NWorld::CTBSUnitVision<NWorld::CUnitServer,NWorld::CPlayer>*>(watcher)->operator&(reader); }
    Require(watcher->GetTBSVisible().size() == visible.size(), "begin-turn fixture did not restore the native visibility list");
    const int watcherAP = watcher->GetRPG()->GetAP(), enemyAP = enemy->GetRPG()->GetAP();
    if (staleOwner) {
        state->AddEnemy(contact); state->pEnemy = contact;
        owner->SetHP(0, owner->GetMaxHP());
        Require(owner->IsDead() && watcher->CanFight(), "watcher's planning HP did not diverge from the live unit");
    } else {
        state->AddPossibleEnemy(contact); state->pPossibleEnemy = contact;
        contact->SetHP(0, contact->GetMaxHP());
        Require(contact->IsDead() && enemy->CanFight(), "contact's planning HP did not diverge from the live unit");
    }
    state->PrepareEnemies();
    std::printf("BEGIN-TURN staleOwner=%d known=%zu possible=%zu selectedEnemy=%d selectedPossible=%d\n", int(staleOwner),
        state->GetKnownEnemies().size(), state->possibleEnemies.data.units.size(), int(IsValid(state->pEnemy)), int(IsValid(state->pPossibleEnemy)));
    if (staleOwner) {
        Require(state->GetKnownEnemies().empty() && state->possibleEnemies.data.units.size() == 1
                && state->possibleEnemies.data.units.front().GetPtr() == contact,
            "cached zero watcher HP suppressed the live begin-turn contact refresh");
    } else {
        Require(state->GetKnownEnemies().size() == 1 && state->GetKnownEnemies().front().GetPtr() == contact
                && state->possibleEnemies.data.units.empty(),
            "cached zero enemy HP suppressed promotion of a visible fighting contact");
    }
    Require(watcher->CanFight() && enemy->CanFight() && watcher->GetRPG()->GetAP() == watcherAP
            && enemy->GetRPG()->GetAP() == enemyAP, "begin-turn contact refresh changed live health or action points");
}

static void FinishedAILogicRetirement(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NAI::IAIUnit* unit = nullptr;
    for (const auto& candidate: units) {
        auto* wrapper = NAI::GetAIUnit(candidate);
        if (IsValid(candidate) && candidate->CanFight() && !candidate->IsEmptyPK() && !candidate->IsWearingPK()
                && !candidate->HasCommand() && IsValid(wrapper)) { actor = candidate.GetPtr(); unit = wrapper; break; }
    }
    Require(actor && unit, "AI retirement fixture has no idle native actor");
    CObj<NAI::CAILogic> logic = new NAI::CAILogic(unit);
    Require(!logic->IsFinished(), "unfinished AI logic was already retired");
    auto* network = world->GetPathNetwork();
    const auto original = actor->GetPosition();
    vector<NAI::SPathPlace> places;
    network->GetNearPlaces(SSphere(original.GetCP(), 8.f), &places);
    CObj<NWorld::CCmdPath> command;
    for (auto place: places) {
        place.SetPose(NAI::CM_STAND); place.SetDirection(original.GetDir());
        NAI::SPosition destination(place, network);
        if (fabs2(destination.GetCP() - original.GetCP()) < 4.f) continue;
        CObj<NWorld::CCmdPath> candidate = new NWorld::CCmdPath(destination);
        if (actor->CanDo(candidate) == NWorld::UCR_OK) { command = candidate; break; }
    }
    Require(IsValid(command), "AI retirement fixture has no valid movement order");
    actor->Do(new NWorld::CCmdSetCommand(actor, command));
    Require(actor->HasCommand(), "AI retirement fixture did not install its native world command");
    logic->Finish();
    const int busyAP = actor->GetRPG()->GetAP();
    Require(!logic->IsFinished() && actor->GetRPG()->GetAP() == busyAP,
        "finished AI logic was retired while its world command was still installed");
    actor->Do(new NWorld::CCmdCancel(actor));
    Require(!actor->HasCommand() && IsValid(actor->GetRPG()), "AI retirement fixture did not cancel its order while retaining its RPG actor");
    const int idleAP = actor->GetRPG()->GetAP();
    Require(logic->IsFinished(), "finished idle AI logic stayed alive merely because its RPG actor still existed");
    Require(actor->GetRPG()->GetAP() == idleAP && actor->CanFight(), "AI logic retirement changed live AP or health");
}

static void ArrivedRetreatReactionLifetime(const char* databasePath,const char* resourcePath)
{
    for (int variant: {0,1,2}) {
        CObj<NRPG::CGlobalGame> game;
        auto world = FixtureWorld(databasePath,resourcePath,false,&game);
        list<CPtr<NWorld::CUnitServer>> units;
        world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr;
        NAI::IAIUnit* unit = nullptr;
        for (const auto& candidate: units) {
            auto* wrapper = NAI::GetAIUnit(candidate);
            if (IsValid(candidate) && candidate->CanFight() && candidate->IsAIUnit() &&
                !candidate->HasCommand() && IsValid(wrapper)) {
                actor = candidate.GetPtr(); unit = wrapper; break;
            }
        }
        Require(actor && unit, "retreat-reaction fixture has no idle native AI actor");
        unit->SetRouteLogic(nullptr);
        unit->SetLogic(nullptr);
        const auto position = actor->GetPosition();
        auto destination = position.pos.p;
        if (variant == 1) destination = NAI::SPathPlace();
        if (variant == 2) {
            auto* network = world->GetPathNetwork();
            vector<NAI::SPathPlace> places;
            network->GetNearPlaces(SSphere(position.GetCP(),5.f),&places);
            bool found = false;
            for (auto place: places) {
                place.SetPose(NAI::CM_STAND); place.SetDirection(0);
                if (!network->IsPassable(place) || network->IsLocked(place,true) ||
                    fabs2(NAI::SPosition(place,network).GetCP()-position.GetCP()) < 4.f) continue;
                CObj<NAI::CPath> path = NWorld::FindPath(network,actor,position.pos.p,vector<NAI::SPathPlace>{place},nullptr,true);
                if (path && path->points.size() >= 3) { destination = place; found = true; break; }
            }
            Require(found, "retreat-reaction fixture found no genuine reachable fallback place");
        }
        const int ap = actor->GetAP(), vp = actor->GetUnitRPG()->GetTotalVP();
        // As in the commander, retain only the allocation; the unit owns the reaction's contents.
        CPtr<NAI::CAIReaction> retreat = NAI::CreateAIRetreatReaction(unit,destination);
        unit->SetReaction(retreat);
        Require(IsValid(retreat) && unit->GetReaction() == retreat.GetPtr(),
            "retreat-reaction fixture did not install the actual native reaction");
        std::printf("native retreat reaction variant=%d\n",variant); std::fflush(stdout);
        Require(retreat->CanUpdate(), "retreat-reaction fixture is not eligible for the native AI pump");
        NAI::SAIUpdateTracker tracker;
        tracker.Add(retreat);
        Require(!tracker.CanGetCommand(), "the native AI pump did not retain its queued reaction");
        tracker.Update();
        Require(tracker.CanGetCommand(), "the native AI pump failed to consume its one queued reaction");
        if (variant == 2) Require(IsValid(retreat) && unit->GetReaction() == retreat.GetPtr() && IsValid(unit->GetLogic()),
            "a live retreat before arrival lost its reaction or genuine movement logic");
        else Require(!IsValid(retreat) && dynamic_cast<NAI::CAIGuardReaction*>(unit->GetReaction()) && !IsValid(unit->GetLogic()),
            "a completed retreat used its cleared reaction to install another movement logic after switching to guard");
        Require(actor->GetPosition() == position && actor->GetAP() == ap && actor->GetUnitRPG()->GetTotalVP() == vp &&
            !actor->HasCommand(), "updating a retreat reaction moved the actor, spent AP/VP or issued a gameplay order");
    }
}

static void ArrivedRetreatCompletion(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NAI::IAIUnit* unit = nullptr;
    for (const auto& candidate: units) {
        auto* wrapper = NAI::GetAIUnit(candidate);
        if (IsValid(candidate) && candidate->CanFight() && !candidate->HasCommand() && IsValid(wrapper)) {
            actor = candidate.GetPtr(); unit = wrapper; break;
        }
    }
    Require(actor && unit, "retreat completion fixture has no idle native actor");
    const auto position = actor->GetPosition();
    const int ap = actor->GetRPG()->GetAP();
    CObj<NAI::CAIRetreatLogic> retreat = new NAI::CAIRetreatLogic(unit, position.pos.p);
    Require(!retreat->IsFinished(), "new retreat logic completed before checking its destination");
    retreat->MakeDecision();
    Require(retreat->IsFinished(), "arrived retreat finished its chooser job but left the retreat logic unfinished");
    Require(!actor->HasCommand() && actor->GetPosition() == position && actor->GetRPG()->GetAP() == ap,
        "completing an arrived retreat queued an action, moved the actor or spent AP");
}

class CombatContextProbe: public NAI::CAICombatLogic
{
    OBJECT_NOCOPY_METHODS(CombatContextProbe);
public:
    int decisions = 0;
    explicit CombatContextProbe(NAI::IAIUnit* unit = nullptr): CAICombatLogic(unit, nullptr) {}
    void MakeDecision() override { ++decisions; }
    bool CanSkip() const override { return false; }
};

static void CombatChooserContext(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NAI::IAIUnit* acting = nullptr;
    NAI::IAIUnit* other = nullptr;
    for (const auto& actor: units) {
        auto* unit = NAI::GetAIUnit(actor);
        if (!IsValid(actor) || !actor->CanFight() || !IsValid(unit) || actor->HasCommand()) continue;
        if (!acting) acting = unit;
        else { other = unit; break; }
    }
    Require(acting && other, "combat chooser fixture needs two idle native AI wrappers");
    auto* commander = dynamic_cast<NAI::CAICommander*>(acting->GetUnitServer()->GetPlayer()->GetCommander());
    Require(commander, "combat chooser fixture has no owning commander");
    auto* shared = commander->GetAIState();
    shared->Synchronize();
    Require(acting->GetAIState() == shared, "combat chooser fixture did not attach its owning tactical state");
    CObj<CombatContextProbe> logic = new CombatContextProbe(acting);
    logic->Think();
    Require(logic->IsThinking() && !logic->IsJobFinished(), "combat chooser did not start its staged native job");
    for (int i = 0; i < 3; ++i) {
        logic->DoJob();
        Require(logic->decisions == 0 && logic->IsThinking() && !logic->IsJobFinished(),
            "combat chooser skipped a stage or decided before the chooser boundary");
    }
    shared->SetCurrentAIUnit(other); shared->SetCurrentAIEnemy(other);
    logic->DoJob();
    Require(logic->decisions == 1 && !logic->IsThinking() && logic->IsJobFinished(),
        "combat chooser failed to decide once and finish its staged job");
    Require(shared->GetCurrentAIUnit() == other && shared->GetCurrentAIEnemy() == other,
        "finishing an older combat chooser overwrote another unit's shared planning context");
    world->GetAIJobManager()->Remove(static_cast<NAI::CAIJob*>(logic.GetPtr()));
}

static void LiveAIPosition(const char* databasePath, const char* resourcePath, bool fullPosition)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NAI::IAIUnit* unit = nullptr;
    for (const auto& candidate: units) {
        auto* wrapper = NAI::GetAIUnit(candidate);
        if (IsValid(candidate) && candidate->CanFight() && !candidate->IsEmptyPK() && IsValid(wrapper)) {
            actor = candidate.GetPtr(); unit = wrapper; break;
        }
    }
    Require(actor && unit, "live AI position fixture has no native wrapper and server");
    const auto initial = actor->GetPosition();
    const int ap = actor->GetRPG()->GetAP();
    auto plan = initial.pos;
    plan.p.SetDirection((plan.p.GetDirection() + 2) & 7);
    unit->SetPosition(plan);
    auto savedPlan = [&]() {
        CStructureNetworkContext references;
        auto saved = BorrowedObjectFields(unit, &references);
        NAI::SUnitPosition result;
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(4, &result); }
        return result;
    };
    Require(savedPlan().pos == plan && actor->GetPosition() == initial,
        "logging an AI planning position moved the live server or lost the saved planning snapshot");
    if (fullPosition)
        Require(unit->GetUnitPosition() == initial, "AI full-position accessor exposed the speculative planning snapshot");
    else
        Require(unit->GetPosition() == initial.pos, "AI position accessor exposed the speculative planning snapshot");
    unit->SavePrevPosition();
    Require(unit->GetPrevPosition() == plan, "live AI position lookup changed planning rollback semantics");
    auto moved = initial;
    moved.pos.p.SetDirection((moved.pos.p.GetDirection() + 4) & 7);
    moved.bRun = !initial.bRun;
    actor->SetPosition(moved);
    const auto live = actor->GetPosition();
    Require(live == moved && unit->GetPosition() == live.pos && unit->GetUnitPosition() == live,
        "AI position lookup missed a live server update or dropped its run flag");
    Require(savedPlan().pos == plan && actor->GetRPG()->GetAP() == ap,
        "reading live AI positions replaced the planning snapshot or changed action points");
}

static void RemovedPlayerAIRosters(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    CPtr<NWorld::CUnitServer> departing;
    for (const auto& actor: units)
        if (IsValid(actor) && actor->CanFight() && !actor->IsAIUnit() && !actor->IsEmptyPK()) { departing = actor; break; }
    Require(IsValid(departing), "AI teardown fixture has no human actor");
    vector<CPtr<NWorld::CPlayer>> players;
    world->GetPlayersList(&players);
    NAI::CAICommander* surviving = nullptr;
    for (const auto& player: players) {
        auto* commander = dynamic_cast<NAI::CAICommander*>(player->GetCommander());
        if (player.GetPtr() != departing->GetPlayer() && commander && !commander->GetUnitsList().empty()) {
            surviving = commander; break;
        }
    }
    Require(surviving, "AI teardown fixture has no surviving commander");
    const auto original = surviving->GetUnitsList();
    // Reproduce the foreign registration used by earlier worlds and retained
    // by existing saves, without running a new AI decision during teardown.
    surviving->OnUnitAdded(departing);
    CPtr<NAI::IAIUnit> wrapper = surviving->GetAIUnit(departing);
    Require(IsValid(wrapper) && surviving->GetUnitsList().size() == original.size() + 1,
        "legacy AI teardown fixture did not retain the foreign unit");
    world->RemovePlayer(departing->GetPlayer());
    Require(!IsValid(departing), "AI teardown fixture did not destroy the departed player unit");
    Require(surviving->GetUnitsList().size() == original.size(),
        "surviving AI commander retained a wrapper for a destroyed player's unit");
    Require(!IsValid(wrapper) && surviving->GetAIUnit(departing) == nullptr,
        "player teardown retained a stale AI wrapper or server lookup entry");
    for (size_t i = 0; i < original.size(); ++i)
        Require(IsValid(original[i]) && surviving->GetUnitsList()[i] == original[i],
            "AI teardown changed the identity or order of a surviving unit");
}

static void RemovedPlayerFootprints(const char* databasePath, const char* resourcePath, bool checkAcks = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    auto* network = world->GetPathNetwork();
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::IPlayer* departing = nullptr;
    for (const auto& unit: units) {
        auto* actor = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (actor && actor->CanFight() && !actor->IsAIUnit() && !actor->IsEmptyPK()) {
            departing = actor->GetPlayer(); break;
        }
    }
    Require(departing != nullptr, "player teardown fixture has no deployed human squad");
    vector<NAI::SPathPlace> vacated, surviving;
    vector<CPtr<NWorld::CUnitServer>> departingUnits;
    for (const auto& unit: units) {
        auto* actor = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (!actor || !actor->CanFight()) continue;
        const auto place = actor->GetPosition().pos.p;
        if (!network->IsLocked(place, true) || network->GetWhoLocksThisPlace(place) != actor) continue;
        if (actor->GetPlayer() == departing) {
            vacated.push_back(place); departingUnits.push_back(actor);
        } else surviving.push_back(place);
    }
    Require(!vacated.empty() && !surviving.empty(), "player teardown fixture needs departing and surviving footprints");
    vector<CPtr<CObjectBase>> departingAcks, survivingAcks;
    if (checkAcks) {
        CStructureNetworkContext graph;
        graph.include = [&](CObjectBase* object) {
            auto* ack = dynamic_cast<NWorld::CAckBase*>(object);
            if (ack && IsValid(ack->GetUnit())) {
                if (ack->GetUnit()->GetPlayer() == departing) departingAcks.push_back(object);
                else survivingAcks.push_back(object);
            }
            return false;
        };
        CMemoryStream traversal;
        { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
          static_cast<CObjectBase*>(world->GetGlobalAck())->operator&(writer); }
        Require(!departingAcks.empty() && !survivingAcks.empty(), "player teardown fixture needs owned and foreign ack listeners");
    }
    world->RemovePlayer(departing);
    if (checkAcks) {
        for (const auto& ack: departingAcks)
            Require(!IsValid(ack), "removed player retained acknowledgement listeners for its destroyed units");
        for (const auto& ack: survivingAcks)
            Require(IsValid(ack), "removing one player destroyed another player's acknowledgement listeners");
    }
    for (const auto& actor: departingUnits)
        Require(!IsValid(actor), "unregistered player left a master-owned unit alive");
    for (const auto& place: vacated)
        Require(!network->IsLocked(place, true), "removed player left an occupied navigation tile after its unit was destroyed");
    for (const auto& place: surviving)
        Require(network->IsLocked(place, true), "removing one player released another player's footprint");
    std::printf("Player teardown: %zu vacated and %zu surviving footprints\n", vacated.size(), surviving.size());
}

static void EmptyBulletConstruction(const char* databasePath, const char* resourcePath, bool checkRetirement = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    const bool action = world->IsAction();
    vector<NRPG::STrailPoint> trail;
    std::puts("Constructing native bullet with an empty cached hit trail"); std::fflush(stdout);
    CObj<NWorld::IDynamicObject> bullet = NWorld::CreateBulletServer(world, trail,
        world->GetTime()->GetValue(), nullptr, 100.f);
    Require(IsValid(bullet), "empty hit trail did not produce a native bullet object");
    if (checkRetirement) Require(bullet->Segment(), "empty bullet never retired because its endpoint index underflowed");
    bullet = nullptr;
    Require(world->IsAction() == action, "discarding an empty bullet retained the world's action counter");
    NRPG::STrailPoint origin;
    origin.vDir = CVec3(1,0,0); origin.vPosition = VNULL3; origin.vNormal = CVec3(0,0,1);
    origin.nUserID = -1; origin.nFloor = 0;
    trail.push_back(origin);
    CObj<NWorld::IDynamicObject> onePoint = NWorld::CreateBulletServer(world, trail,
        world->GetTime()->GetValue(), nullptr, 100.f);
    Require(IsValid(onePoint), "empty-trail guard rejected a nonempty bullet trail");
    Require(onePoint->Segment(), "one-point bullet did not retire at its existing endpoint");
    onePoint = nullptr;
    Require(world->IsAction() == action, "one-point bullet retained the world's action counter");
}

class HiddenUnitPresentation: public NWorld::IRenderVisitor
{
public:
    vector<CVec4> filters;
    int flares = 0, meshes = 0;
    int alienBegin = 0, alienEnd = 0, alienDepth = 0;
    bool alienMesh = false;
    void StartAlienStyle() override { ++alienBegin; ++alienDepth; }
    void FinishAlienStyle() override { ++alienEnd; --alienDepth; }
    void AddColorPostFilter(const CVec4& value) override { filters.push_back(value); }
    void AddFlare(CFuncBase<CVec3>*, float, NDb::CTexture*, int, float, float) override { ++flares; }
    void AddMesh(NDb::CModel*, CFuncBase<NAnimation::SSkeletonPose>*, CFuncBase<NAnimation::SSkeletonState>*,
        const vector<SBoundMesh>&, NGScene::CLightGroup*, int, NWorld::CUnit*, int) override { ++meshes; alienMesh = alienDepth > 0; }
};

static void HiddenUnitTint(const char* databasePath, const char* resourcePath, bool checkSequence = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (const auto& unit: units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (candidate && candidate->CanFight() && !candidate->IsEmptyPK() && !candidate->IsWearingPK()
                && !candidate->IsClueUnit() && candidate->IsAddedToVisitor()) { actor = candidate; break; }
    }
    Require(actor && !world->IsSequence(), "hidden tint fixture has no ordinary actor outside a sequence");
    auto* rpg = actor->GetUnitRPG();
    const auto position = actor->GetPosition();
    const float ap = rpg->GetRPGUnit()->Skills(NDb::ST_AP);
    rpg->SetHiding(false);
    HiddenUnitPresentation visible;
    actor->Visit(&visible);
    Require(visible.meshes == 1 && visible.filters.empty() && visible.flares == 0,
        "visible human unexpectedly received a hiding overlay");
    rpg->SetHiding(true);
    HiddenUnitPresentation hidden;
    actor->Visit(&hidden);
    Require(hidden.meshes == 1 && hidden.filters.size() == 1 && hidden.flares == 1,
        "hidden human did not receive exactly one overlay and flare");
    const auto& tint = hidden.filters.front();
    std::printf("Hidden human tint: %.8f %.8f %.8f %.8f\n", tint.x, tint.y, tint.z, tint.w);
    Require(tint.x == 0.0703125f && tint.y == 0.11015625f && tint.z == 0.140625f && tint.w == 0.6f,
        "hidden human rendered with an opaque black overlay instead of the translucent blue-gray tint");
    Require(rpg->IsHiding() && actor->GetPosition().pos.p == position.pos.p
            && rpg->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "rendering a hiding tint changed hiding state, position or AP");
    if (checkSequence) {
        world->StartSequence();
        Require(world->IsSequence() && rpg->IsHiding(), "human sequence fixture lost its hidden state");
        HiddenUnitPresentation cinematic;
        actor->Visit(&cinematic);
        Require(cinematic.meshes == 1 && cinematic.filters.empty() && cinematic.flares == 0 && !cinematic.alienMesh,
            "cinematic sequence still rendered the human hiding overlay");
        Require(rpg->IsHiding(), "hiding presentation changed the human's logical hiding state");
        world->EndOfTurn();
        Require(!world->IsSequence(), "human sequence fixture did not leave the sequence");
        HiddenUnitPresentation resumed;
        actor->Visit(&resumed);
        Require(resumed.filters.size() == 1 && resumed.flares == 1,
            "human hiding presentation did not resume after the sequence");
    }

}

class UnitFloorPresentation: public CSyncDst<NWorld::IVisObj>, public NWorld::IRenderVisitor
{
    NWorld::IVisObj* target;
    void VisitObject(int, NWorld::IVisObj* object) override {
        if (object != target) return;
        ++updates;
        if (render) object->Visit(this);
    }
public:
    int updates = 0, meshFloor = 1000;
    bool render = false;
    UnitFloorPresentation(NWorld::CWorld* world, NWorld::IVisObj* actor): CSyncDst(world->GetUnits()), target(actor) {}
    void AddMesh(NDb::CModel*, CFuncBase<NAnimation::SSkeletonPose>*, CFuncBase<NAnimation::SSkeletonState>*,
        const vector<SBoundMesh>&, NGScene::CLightGroup*, int floor, NWorld::CUnit*, int) override { meshFloor = floor; }
};

static void LivingUnitFloorRefresh(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (const auto& unit: units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (candidate && candidate->CanFight() && !candidate->IsEmptyPK() && !candidate->IsClueUnit()
                && candidate->IsAddedToVisitor() && candidate->GetPosition().pos.p.IsIntegral()) {
            actor = candidate; break;
        }
    }
    Require(actor != nullptr, "living-unit floor fixture has no ordinary visible actor");
    const auto position = actor->GetPosition();
    auto* network = dynamic_cast<NAI::CPathNetwork*>(world->GetPathNetwork());
    auto& tile = network->GetLayer(position.pos.p.GetLayer())->tiles[position.pos.p.GetY()][position.pos.p.GetX()];
    const short originalFloor = tile.nFloor;
    const float originalAP = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    UnitFloorPresentation presentation(world, actor);
    presentation.Sync(); presentation.updates = 0;
    tile.nFloor = originalFloor + 1;
    actor->NWorld::CDumbUnitServer::Segment(); presentation.Sync();
    Require(presentation.updates == 0, "never-rendered living unit was unnecessarily requeued for a floor refresh");
    tile.nFloor = originalFloor;
    actor->Visit(static_cast<NWorld::IRenderVisitor*>(&presentation));
    Require(presentation.meshFloor == originalFloor, "native render visitor did not record the unit's initial floor");
    actor->NWorld::CDumbUnitServer::Segment(); presentation.Sync();
    Require(presentation.updates == 0, "unchanged living unit was repeatedly marked dirty for rendering");
    tile.nFloor = originalFloor + 1;
    Require(actor->NWorld::CDumbUnitServer::GetFloor() == originalFloor + 1,
        "navigation floor edit did not reach the living unit's live position");
    actor->NWorld::CDumbUnitServer::Segment(); presentation.Sync();
    Require(presentation.updates == 1, "living unit with a stale rendered floor did not refresh its native visibility binding");
    Require(presentation.meshFloor == originalFloor, "unit floor cache was overwritten before the render visitor rebuilt its mesh");
    actor->NWorld::CDumbUnitServer::Segment(); presentation.Sync();
    Require(presentation.updates == 2, "unrendered stale unit stopped requesting its pending floor refresh");
    presentation.render = true;
    actor->NWorld::CDumbUnitServer::Segment(); presentation.Sync();
    Require(presentation.updates == 3 && presentation.meshFloor == originalFloor + 1,
        "rendered living unit did not rebuild its mesh on the current navigation floor");
    actor->NWorld::CDumbUnitServer::Segment(); presentation.Sync();
    Require(presentation.updates == 3, "render visitor did not acknowledge the completed floor refresh");
    Require(actor->GetPosition().pos.p == position.pos.p && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == originalAP,
        "living unit floor refresh moved its logical position or changed AP");
    actor->MarkNotAddedToVisitors();
    tile.nFloor = originalFloor;
    actor->NWorld::CDumbUnitServer::Segment(); presentation.Sync();
    Require(presentation.updates == 3, "unit excluded from rendering was requeued for a floor refresh");
}

static bool SavedWorldUIRefresh(NWorld::CWorld* world)
{
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE); static_cast<CObjectBase*>(world)->operator&(writer); }
    const auto* bytes = static_cast<const std::uint8_t*>(saved.GetBuffer());
    const auto size = static_cast<std::size_t>(saved.GetSize());
    S2FileIO::StructureChunk root;
    bool foundRoot = false;
    for (std::size_t offset = 0; offset < size;) {
        S2FileIO::StructureChunk chunk;
        Require(S2FileIO::DecodeStructureChunkAt(bytes, size, offset, &chunk), "invalid world save chunk");
        if (chunk.id == 1) { root = chunk; foundRoot = true; break; }
        offset = static_cast<std::size_t>(chunk.payloadOffset + chunk.length);
    }
    Require(foundRoot, "world save has no root payload");
    const auto* fields = bytes + root.payloadOffset;
    for (std::size_t offset = 0; offset < root.length;) {
        S2FileIO::StructureChunk chunk;
        Require(S2FileIO::DecodeStructureChunkAt(fields, root.length, offset, &chunk), "invalid world save field");
        if (chunk.id == 47) {
            Require(chunk.length == 1, "world UI-refresh latch has an unexpected wire size");
            return fields[chunk.payloadOffset] != 0;
        }
        offset = static_cast<std::size_t>(chunk.payloadOffset + chunk.length);
    }
    return false;
}

static void WorldUIRefreshLatch(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    Require(SavedWorldUIRefresh(world), "fresh world did not preserve its pending UI refresh in save tag 47");
    NWorld::IWorld* api = world.GetPtr();
    Require(api->IsUINeedUpdate() && !api->IsUINeedUpdate(), "fresh-world UI refresh was not consumed exactly once");
    Require(!SavedWorldUIRefresh(world), "consumed UI refresh was still armed in an ordinary save");
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& unit: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (server && server->CanFight() && !server->IsEmptyPK()) { actor = server; break; }
    }
    Require(actor != nullptr, "UI-refresh fixture has no living actor");
    actor->Hide(false, false);
    api->IsUINeedUpdate();
    actor->EnableHide();
    Require(api->IsUINeedUpdate() && !api->IsUINeedUpdate(), "re-arming Hide did not request exactly one UI refresh");
    actor->EnableHide();
    Require(!api->IsUINeedUpdate(), "already enabled Hide requested an unnecessary UI refresh");
    auto networkBytes = [&]() {
        CStructureNetworkContext context;
        context.include = [](CObjectBase*) { return false; };
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &context);
          static_cast<CObjectBase*>(world.GetPtr())->operator&(writer); }
        const auto* bytes = static_cast<const std::uint8_t*>(saved.GetBuffer());
        return vector<std::uint8_t>(bytes, bytes + saved.GetSize());
    };
    const auto cleanNetwork = networkBytes();
    api->UINeedUpdate();
    Require(SavedWorldUIRefresh(world), "explicit UI-refresh request was missing from an ordinary save");
    Require(networkBytes() == cleanNetwork, "local UI refresh changed the synchronized world payload");
    Require(api->IsUINeedUpdate() && !api->IsUINeedUpdate(), "explicit UI-refresh request was not consumed exactly once");
    for (bool pending: {true, false}) {
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE); writer.Add(47, &pending); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ);
          static_cast<CObjectBase*>(world.GetPtr())->operator&(reader); }
        Require(api->IsUINeedUpdate() == pending && !api->IsUINeedUpdate(),
            "ordinary save restore lost the pending/consumed UI-refresh state");
    }

}

class PainSoundVisitor: public NWorld::ISoundVisitor
{
public:
    vector<NDb::CSound*> sounds;
    void Add3DSound(STime, NDb::CSound* sound, CFuncBase<CVec3>*) override { sounds.push_back(sound); }
    void AddEffect(STime, NDb::CSoundEffect*, CFuncBase<CVec3>*, const vector<int>&) override {
        throw std::runtime_error("unexpected effect in pain-sound fixture");
    }
    void Add2DSound(STime, NDb::CSound*) override { throw std::runtime_error("unexpected 2D pain sound"); }
};

static vector<NDb::CSound*> UnitAttachedSounds(NWorld::CUnitServer* actor)
{
    vector<CPtr<NWorld::C3DSound>> attached;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* object) {
        if (auto* sound = dynamic_cast<NWorld::C3DSound*>(object)) attached.push_back(sound);
        return false;
    };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
      actor->NWorld::CDumbUnitServer::operator&(writer); }
    PainSoundVisitor visitor;
    for (auto& sound: attached) sound->Visit(&visitor);
    return visitor.sounds;
}

static void PainVoiceOwner(const char* databasePath, const char* resourcePath, bool unconscious = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (server && server->CanFight() && !server->IsEmptyPK() && !server->IsWearingPK()
                && server->IsAIUnit()) { actor = server; break; }
    }
    Require(actor != nullptr, "pain-voice fixture has no ordinary conscious NPC");
    CDBIterator<NDb::CSound> rows(*NDatabase::GetTable<NDb::CSound>());
    Require(rows.MoveNext(), "pain-voice fixture has no first sound");
    NDb::CSound* ownSound = rows.Get();
    Require(rows.MoveNext(), "pain-voice fixture has no donor sound");
    NDb::CSound* donorSound = rows.Get();
    CObj<NDb::CTSound> ownTemplate = new NDb::CTSound, donorTemplate = new NDb::CTSound;
    CObj<NDb::CSoundVariant> ownVariant = new NDb::CSoundVariant, donorVariant = new NDb::CSoundVariant;
    ownVariant->pSound = ownSound; donorVariant->pSound = donorSound;
    ownTemplate->variants.push_back(ownVariant.GetPtr()); ownTemplate->roulette.AddSector(1);
    donorTemplate->variants.push_back(donorVariant.GetPtr()); donorTemplate->roulette.AddSector(1);
    CObj<NDb::CRPGPers> donor = new NDb::CRPGPers;
    donor->pSoundHit = donorTemplate.GetPtr();
    auto* persona = actor->GetUnitRPG()->GetRPGPers();
    persona->pSoundHit = ownTemplate.GetPtr();
    persona->pAcksHolder = donor.GetPtr();
    Require(actor->GetUnitRPG()->GetRPGUnit()->GetAckHolder() == donor.GetPtr(),
        "pain-voice fixture did not install a different acknowledgement owner");
    auto before = UnitAttachedSounds(actor);
    const int vp = actor->GetUnitRPG()->GetTotalVP();
    auto strike = [&]() {
        NRPG::CAttackPortion attack(0, 2, 0, 0, 0, 0, 0, nullptr, actor->GetUnitRPG());
        attack.rTtrajectory.ptOrigin = actor->GetPosition().GetCP();
        attack.rTtrajectory.ptDir = CVec3(0, 1, 0);
        Require(actor->ProcessAttack(world, NAI::HL_BODY, &attack, CVec3(0, 1, 0), nullptr) == 0,
            "pain-voice fixture unexpectedly dealt damage");
    };
    strike();
    auto sounds = UnitAttachedSounds(actor);
    Require(sounds.size() == before.size() + 1 && sounds.back() == donorSound,
        "native hit processing used the persona voice instead of its acknowledgement owner");
    persona->pAcksHolder = nullptr;
    strike();
    sounds = UnitAttachedSounds(actor);
    Require(sounds.size() == before.size() + 2 && sounds.back() == ownSound,
        "missing acknowledgement owner did not fall back to the persona pain voice");
    persona->pSoundHit = nullptr;
    strike();
    Require(UnitAttachedSounds(actor).size() == sounds.size(), "missing pain voice created a spurious sound");
    Require(actor->CanFight() && actor->GetUnitRPG()->GetTotalVP() == vp,
        "pain-voice selection changed combat health or consciousness");
    if (unconscious) {
        persona->pSoundHit = ownTemplate.GetPtr();
        actor->MakeUnconscious(VNULL3, true, false);
        Require(actor->IsUnconscious() && !actor->IsDead() && !actor->CanFight(),
            "pain-eligibility fixture did not enter the native unconscious state");
        const auto quiet = UnitAttachedSounds(actor);
        strike();
        Require(UnitAttachedSounds(actor).size() == quiet.size(),
            "an already unconscious actor emitted a new pain voice on a hit");
        Require(actor->GetUnitRPG()->GetTotalVP() == vp && actor->IsUnconscious(),
            "pain eligibility changed health or the native unconscious state");
    }

}

class VisibleIconPlayer: public NWorld::CPlayer
{
    OBJECT_NOCOPY_METHODS(VisibleIconPlayer);
public:
    vector<NWorld::IVisObj*> sounds;
    void GetSounds(vector<NWorld::IVisObj*>* result) override { *result = sounds; }
    void See(NWorld::CUnitServer* actor) { NWorld::CPlayerBaseVision<NWorld::CUnitServer>::visible.push_back(actor); }
    int GetScenarioPlayerID() const override { return 0; }
};

class EnemyIconMission: public ReadinessMission
{
    OBJECT_NOCOPY_METHODS(EnemyIconMission);
public:
    CObj<NGScene::IGameView> view;
    CTransformStack transform;
    EnemyIconMission() { transform.Init(); }
    NGScene::IGameView* GetScene() const override { return view; }
    const CTransformStack& GetCameraTransform() const override { return transform; }
};

static void EnemyIconVisibility(const char* databasePath, const char* resourcePath,bool heardAnchor = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    CObj<VisibleIconPlayer> visiblePlayer = new VisibleIconPlayer;
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (server && server->CanFight() && !server->IsClueUnit() && !server->IsEmptyPK()
                && world->GetDiplomacyState(visiblePlayer.GetPtr(), server->GetPlayer()) == NDb::DS_ENEMY) {
            actor = server; break;
        }
    }
    Require(actor != nullptr, "enemy-icon fixture has no visible hostile actor");
    if (!heardAnchor) visiblePlayer->See(actor);
    CObj<NGame::IPlayerTracker> tracker = dynamic_cast<NGame::IPlayerTracker*>(pSSClasses->CreateObject(int(0xB2022800)));
    CObj<EnemyIconMission> mission = new EnemyIconMission;
    mission->view = new NGScene::GameViewFixture;
    CObj<MissionIconProbe> desktop = new MissionIconProbe;
    mission->Attach(world, desktop); mission->AttachActive(tracker);
    CObj<NUI::CWindow> client = new NUI::CWindow(NUI::SWindowInfo(nullptr,
        NUI::SPoint(0, 0), NUI::SPoint(1024, 768), "view", NUI::STYLE_ENABLED | NUI::STYLE_VISIBLE));
    CObj<NUI::CText> pause = new NUI::CText;
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.include = [](CObjectBase*) { return false; };
    auto retain = [&](CObjectBase* object, unsigned id) { references.ids[object] = id; references.refs[id] = object; };
    retain(mission.GetPtr(), 1); retain(visiblePlayer.GetPtr(), 2); retain(client.GetPtr(), 3); retain(pause.GetPtr(), 4);
    CPtr<NWorld::IPlayer> worldPlayer = visiblePlayer.GetPtr();
    CMemoryStream playerSaved;
    { CStructureSaver writer(playerSaved, CStructureSaver::WRITE, &references); writer.Add(5, &worldPlayer); }
    playerSaved.Seek(0);
    { CStructureSaver reader(playerSaved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(tracker.GetPtr())->operator&(reader); }
    struct DesktopClientWire {
        CObj<NUI::CWindow> client;
        int operator&(CStructureSaver& saver) { saver.Add(2, &client); return 0; }
    } base;
    base.client = client.GetPtr();
    CPtr<NGame::IMission> missionRef = mission.GetPtr();
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
      writer.Add(1, &base); writer.Add(2, &missionRef); writer.Add(27, &pause); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(desktop.GetPtr())->operator&(reader); }
    auto markerCount = [&](int typeID) {
        int count = 0;
        CStructureNetworkContext graph;
        graph.include = [&](CObjectBase* object) {
            if (pSSClasses->GetObjectTypeID(object) == typeID) ++count;
            return false;
        };
        CMemoryStream traversal;
        { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
          static_cast<CObjectBase*>(desktop.GetPtr())->operator&(writer); }
        return count;
    };
    NGlobal::SetVar("ui_showicons", 1);
    if (heardAnchor) {
        CObj<NWorld::CTimedObject> marker = NWorld::CreateDMesh(actor,VNULL3,QNULL,NDb::GetModel(3899),0);
        visiblePlayer->sounds.push_back(marker.GetPtr());
        SHMatrix projection;
        Identity(&projection);
        projection._23 = .2f;
        mission->transform.Init(projection);
        auto positionAtHeight = [&](float height) {
            CVec2 point;
            Require(TestRayInFrustrum(CVec3(0,0,height),&mission->transform,CVec2(1024,768),&point),
                "heard-icon fixture placed its expected anchor outside the view");
            return NUI::SPoint(int(point.x / S2UI::Scale()),int(point.y / S2UI::Scale()));
        };
        const auto expected = positionAtHeight(1.6f), oldHeight = positionAtHeight(2.2f);
        Require(expected.y != oldHeight.y, "heard-icon fixture cannot distinguish the two anchor heights");
        desktop->RefreshIcons(false);
        Require(markerCount(int(0xB3131150)) == 1, "native heard marker did not produce exactly one ear icon");
        auto* icon = client->GetChildByID("");
        Require(icon && pSSClasses->GetObjectTypeID(icon) == int(0xB3131150),
            "native heard marker did not attach its sound icon to the client view");
        Require(icon->GetPosition().x == expected.x && icon->GetPosition().y == expected.y,
            "the heard-unit ear icon used visible-enemy clearance instead of its sound-marker head height");
        const auto original = actor->GetPosition();
        auto moved = original;
        moved.pos.p = NAI::SPathPlace(original.pos.p.GetX()+3,original.pos.p.GetY(),original.pos.p.GetLayer(),
            original.pos.p.GetDirection(),original.pos.p.GetPose(),original.pos.p.IsMoving());
        actor->SetTemporaryPosition(moved);
        desktop->RefreshIcons(false);
        Require(icon->GetPosition().x == expected.x && icon->GetPosition().y == expected.y,
            "moving an unseen unit leaked its live position through the sound-marker icon");
        actor->SetTemporaryPosition(original);
        visiblePlayer->See(actor);
        desktop->RefreshIcons(false);
        Require(markerCount(int(0xB3131150)) == 0, "a visible unit retained its heard-only ear icon");
        return;
    }
    desktop->RefreshEnemies();
    Require(markerCount(int(0xB0241948)) == 1, "visible hostile actor did not receive its normal enemy icon");
    NGlobal::SetVar("ui_showicons", 0);
    desktop->RefreshEnemies();
    Require(markerCount(int(0xB0241948)) == 0, "disabled enemy icons were rebuilt for a visible hostile actor");
    NGlobal::SetVar("ui_showicons", 1);
    desktop->RefreshEnemies();
    Require(markerCount(int(0xB0241948)) == 1, "re-enabling enemy icons did not restore the visible hostile marker");
    actor->nClueCount = 1;
    NGlobal::SetVar("ui_showicons", 0);
    desktop->RefreshEnemies();
    Require(markerCount(int(0xB0241948)) == 0 && markerCount(int(0xB3123180)) == 1,
        "enemy-icon visibility suppressed the independently visible quest-clue marker");

}

static void MissingUISoundFallback(const char* databasePath,const char* resourcePath)
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS") != nullptr, "missing UI sound fixture requires headless native audio");
    Require(NFMSound::Init(NFMSound::SStartInfo()), "missing UI sound fixture could not initialize native audio");
    struct AudioCleanup { ~AudioCleanup() { NFMSound::Done(); } } audioCleanup;
#endif
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NDb::CSound* sound = nullptr;
    CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());
    while (sounds.MoveNext()) {
        if (!NGScene::CResourceFileOpener::DoesExist("Sounds",sounds.Get()->GetRecordID())) {
            sound = sounds.Get(); break;
        }
    }
    Require(sound != nullptr, "missing UI sound fixture needs an authored sound without an available resource");
    const int id = sound->GetRecordID();
    CObj<NGScene::CFileRequest> missing = new NGScene::CFileRequest("Sounds",id);
    missing->Read();
    Require(missing->IsReady() && missing->GetStream()->GetSize() == 0,
        "missing UI sound fixture unexpectedly resolved actual sound data");
    struct LoaderCleanup { ~LoaderCleanup() { NGScene::StopResourceLoadingThread(); } } cleanup;
    NGScene::RunResourceLoadingThread();
    CObj<NSound::CFileSample2D> loader = new NSound::CFileSample2D;
    loader->SetKey(id);
    CDGPtr<NSound::CFileSample2D> refreshed = loader.GetPtr();
    NFMSound::CSample2D* sample = nullptr;
    for (int retry = 0; retry < 500 && !sample; ++retry) {
        MarkNewDGFrame(); refreshed.Refresh();
        sample = loader->GetValue();
        if (!sample) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    Require(sample != nullptr, "a missing UI sound never resolved to a stable silent sample");
    for (int retry = 0; retry < 4; ++retry) {
        MarkNewDGFrame(); refreshed.Refresh();
        Require(loader->GetValue() == sample, "the missing UI sound fallback was discarded or loaded repeatedly");
    }
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(NFMSound::PlaySound(sample) == nullptr, "a silent missing UI sample started an actual channel");
#endif
    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(loader,&references);
    int savedKey = -1;
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(1,&savedKey); }
    Require(savedKey == id, "changing the UI sound loading path changed its existing resource-key wire field");
#if defined(S2_UPSTREAM_NATIVE_SFX)
    NGScene::AddResourceDir(resourcePath);
    NDb::CSound* playable = nullptr;
    CDBIterator<NDb::CSound> actual(*NDatabase::GetTable<NDb::CSound>());
    while (actual.MoveNext()) {
        if (!NGScene::CResourceFileOpener::DoesExist("Sounds",actual.Get()->GetRecordID())) continue;
        CObj<NGScene::CFileRequest> request = new NGScene::CFileRequest("Sounds",actual.Get()->GetRecordID());
        request->Read();
        auto* stream = request->GetStream();
        if (stream->GetSize() == 0) continue;
        CObj<NFMSound::CSample2D> decoded = NFMSound::LoadSample2D(stream->GetBuffer(),stream->GetSize());
        if (!decoded.GetPtr()) continue;
        CObj<NFMSound::CSound2D> channel = NFMSound::PlaySound(decoded);
        if (channel.GetPtr()) { playable = actual.Get(); break; }
    }
    Require(playable != nullptr, "UI sound fixture has no authored, decodable and playable resource control");
    CObj<NSound::CFileSample2D> available = new NSound::CFileSample2D;
    available->SetKey(playable->GetRecordID());
    CDGPtr<NSound::CFileSample2D> availableRef = available.GetPtr();
    MarkNewDGFrame(); availableRef.Refresh();
    Require(available->GetValue() == nullptr, "a UI sample decoded synchronously before its resource request completed");
    for (int retry = 0; retry < 500 && !available->GetValue(); ++retry) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        MarkNewDGFrame(); availableRef.Refresh();
    }
    Require(available->GetValue() != nullptr, "an available UI sound never completed its native resource load");
    CObj<NFMSound::CSound2D> playing = NFMSound::PlaySound(available->GetValue());
    Require(playing.GetPtr() != nullptr, "a successfully loaded UI sound could no longer start its native channel");
#endif
}

static void FinishedMusicSilence(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    CDBIterator<NDb::CMusic> rows(*NDatabase::GetTable<NDb::CMusic>());
    Require(rows.MoveNext(), "finished-music fixture has no track record");
    auto* track = rows.Get();
    track->szFileName = "__upstream_missing_music_fixture__.ogg";
    track->nFadeIn = 0; track->nRndPlayTime = 0;
    track->nSilence = 10000; track->nRndSilence = 0;
    CObj<NDb::CTMusic> pool = new NDb::CTMusic;
    pool->variants.push_back(track); pool->roulette.AddSector(1);
    CObj<CCTime> clock = new CCTime(0);
    Require(NFMSound::Init(NFMSound::SStartInfo()), "headless native audio initialization failed");
    struct AudioCleanup { ~AudioCleanup() { NFMSound::Done(); } } cleanup;
    CObj<NSound::ISoundScene> scene = NSound::CreateSoundScene(pool, nullptr, clock);
    CTransformStack transform; transform.Init();
    scene->Draw(&transform); // Starts the native stream and leaves the silence state.
    CPtr<NFMSound::CStream> stream = NFMSound::PlayStream(track->szFileName.c_str(), true, 0, true, 0);
    Require(IsValid(stream), "finished-music fixture did not retain a native stream");
    CPtr<CObjectBase> music;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* object) {
        if (pSSClasses->GetObjectTypeID(object) == int(0xa1063160)) music = object;
        return false;
    };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(scene.GetPtr())->operator&(writer); }
    Require(IsValid(music), "native sound scene did not retain its current music record");
    NFMSound::Done();
    Require(stream.GetPtr() != nullptr && !IsValid(stream), "native stream teardown did not leave its retained invalid reference");
    unsigned current = 777;
    CMemoryStream position;
    { CStructureSaver writer(position, CStructureSaver::WRITE); writer.Add(4, &current); }
    position.Seek(0);
    { CStructureSaver reader(position, CStructureSaver::READ); music->operator&(reader); }
    clock->Set(1000);
    scene->Draw(&transform);
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(scene.GetPtr())->operator&(writer); }
    saved.Seek(0);
    __int64 nextTrack = -1;
    { CStructureSaver reader(saved, CStructureSaver::READ, &graph); reader.Add(2, &nextTrack); }
    Require(nextTrack == 11000,
        "finished retained music stream entered restore logic and lost its authored silence interval");
    // A genuinely restored music object has no runtime stream. With audio
    // unavailable it must still attempt restoration and drop the failed track.
    CObj<CObjectBase> restored = pSSClasses->CreateObject(int(0xa1063160));
    Require(IsValid(restored), "native restored-music class is not registered");
    CDBPtr<NDb::CMusic> record = track;
    bool loop = true;
    CMemoryStream trackSave;
    { CStructureSaver writer(trackSave, CStructureSaver::WRITE);
      writer.Add(2, &record); writer.Add(3, &loop); writer.Add(4, &current); }
    trackSave.Seek(0);
    { CStructureSaver reader(trackSave, CStructureSaver::READ); restored->operator&(reader); }
    CObj<NSound::ISoundScene> loaded = NSound::CreateSoundScene(nullptr, nullptr, clock);
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.ids[restored.GetPtr()] = 1; references.refs[1] = restored.GetPtr();
    references.include = [](CObjectBase*) { return false; };
    CMemoryStream sceneSave;
    { CStructureSaver writer(sceneSave, CStructureSaver::WRITE, &references); writer.Add(5, &restored); }
    sceneSave.Seek(0);
    { CStructureSaver reader(sceneSave, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(loaded.GetPtr())->operator&(reader); }
    auto retainsRestored = [&]() {
        bool retained = false;
        CStructureNetworkContext inspect;
        inspect.include = [&](CObjectBase* object) { if (object == restored.GetPtr()) retained = true; return false; };
        CMemoryStream traversal;
        { CStructureSaver writer(traversal, CStructureSaver::WRITE, &inspect);
          static_cast<CObjectBase*>(loaded.GetPtr())->operator&(writer); }
        return retained;
    };
    Require(retainsRestored(), "restored sound scene lost its saved track before drawing");
    loaded->Draw(&transform);
    Require(!retainsRestored(), "genuinely missing stream no longer entered failed-resume cleanup");

}

static void HolsteredHeavyWeaponMesh(const char* databasePath, const char* resourcePath,int visibility = 0)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (server && server->CanFight() && !server->IsEmptyPK() && !server->IsWearingPK()) { actor = server; break; }
    }
    Require(actor != nullptr, "heavy-mesh fixture has no ordinary actor");
    NDb::CRPGWeapon* heavy = nullptr;
    CDBIterator<NDb::CRPGWeapon> rows(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while (rows.MoveNext()) {
        auto* candidate = rows.Get();
        if (IsValid(candidate->pItem) && candidate->pItem->bPlaceInHand && candidate->pItem->subType == NDb::SUBTYPE_HEAVY
                && IsValid(candidate->pItem->pModel) && IsValid(candidate->pAnimWeaponType)) {
            heavy = candidate; break;
        }
    }
    Require(heavy != nullptr, "heavy-mesh fixture has no modelled heavy weapon");
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(heavy);
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    CObj<NRPG::IInventoryItem> oldFirst = inventory->Get(NDb::SLOT_1) ? inventory->TakeOff(NDb::SLOT_1) : nullptr;
    CObj<NRPG::IInventoryItem> oldSecond = inventory->Get(NDb::SLOT_2) ? inventory->TakeOff(NDb::SLOT_2) : nullptr;
    Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1),
        "heavy-mesh fixture could not equip its active weapon");
    const char* bone = NWorld::GetBoneName(NWorld::UIT_WEAPON_HEAVY, weapon);
    Require(bone && bone[0], "heavy-mesh fixture weapon has no heavy bind bone");
    auto heavyCount = [&](bool holstered, bool noHeavy = false) {
        vector<NWorld::IRenderVisitor::SBoundMesh> meshes;
        NWorld::GetItemsBindPlaces(&meshes, actor->GetUnitRPG(), holstered, nullptr, noHeavy);
        return std::count_if(meshes.begin(), meshes.end(), [&](const auto& mesh) {
            return mesh.pszBindBone && !std::strcmp(mesh.pszBindBone, bone);
        });
    };
    Require(heavyCount(false) == 1, "drawn heavy weapon lost its normal mesh");
    Require(heavyCount(false, true) == 0, "no-heavy render mode did not suppress the weapon");
    if (visibility == 1) {
        const int ap = actor->GetAP(), ammo = weapon->GetAmmoQuantity();
        heavy->pItem->bPlaceInHand = false;
        Require(heavyCount(false) == 0,
            "an active item marked invisible in hand still attached its native weapon mesh");
        Require(inventory->GetActive() == weapon.GetPtr() && actor->GetAP() == ap && weapon->GetAmmoQuantity() == ammo,
            "honoring the in-hand visibility flag changed the actual active item, AP or ammunition");
        heavy->pItem->bPlaceInHand = true;
        Require(heavyCount(false) == 1, "reenabling an item's authored visibility failed to restore its native mesh");
        return;
    }
    Require(heavyCount(true) == 0, "holstered active heavy weapon remained in the native render bindings");
    Require(inventory->Activate(NDb::SLOT_2) && heavyCount(true) == 1,
        "active-weapon holster gate incorrectly hid an inactive carried heavy weapon");
    if (visibility == 2) {
        const int ap = actor->GetAP(), ammo = weapon->GetAmmoQuantity();
        heavy->pItem->bPlaceInHand = false;
        Require(heavyCount(true) == 0,
            "an inactive heavy item marked invisible still attached its carried weapon mesh");
        Require(inventory->Get(NDb::SLOT_1) == weapon.GetPtr() && inventory->GetActiveSlot() == NDb::SLOT_2 &&
            actor->GetAP() == ap && weapon->GetAmmoQuantity() == ammo,
            "hiding an inactive heavy mesh changed equipment, the active slot, AP or ammunition");
        heavy->pItem->bPlaceInHand = true;
        Require(heavyCount(true) == 1, "reenabling an inactive heavy item's visibility failed to restore its carried mesh");
    }
}

class AckPriorityDesktop: public NUI::CDesktopWindow
{
    OBJECT_NOCOPY_METHODS(AckPriorityDesktop);
public:
    vector<CPtr<NWorld::CAckEvent>> played;
    AckPriorityDesktop(): CDesktopWindow(NUI::SWindowInfo(nullptr, NUI::SPoint(0, 0),
        NUI::SPoint(100, 100), "ack-priority-probe")) {}
protected:
    NUI::CAckEvent* PlayAckEvent(const STime&, NWorld::CAckEvent* event) override {
        played.push_back(event);
        return new NUI::CAckEvent(event);
    }
};

static void ScriptedDialoguePriority(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    CObj<NWorld::CUnitServer> actor;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight() || unit->IsEmptyPK()) continue;
        int personID = unit->GetUnitRPG()->GetRPGPers()->GetRecordID();
        if (personID != game->GetDialogHeroPersID() && world->GetUnitServerByPersID(personID) == unit) {
            actor = unit; break;
        }
    }
    Require(IsValid(actor), "dialogue-priority fixture has no existing persona speaker");
    CObj<NDb::CDBAckInfo> info = new NDb::CDBAckInfo;
    info->nRPGPersID = actor->GetUnitRPG()->GetRPGPers()->GetRecordID();
    info->voices.resize(6);
    const int dialogID = 900001;
    CDBIterator<NDb::CDBDialogSeq> rows(*NDatabase::GetTable<NDb::CDBDialogSeq>());
    int count = 0;
    while (count < 4 && rows.MoveNext()) {
        rows.Get()->nDialogID = dialogID;
        rows.Get()->pAckInfo = info.GetPtr();
        ++count;
    }
    Require(count == 4, "dialogue-priority fixture has fewer than four sequence rows");
    while (auto* pending = world->GetUICommand()) { CObj<NWorld::CUICmd> release = pending; }
    NWorld::PlayDialogAsAcks(world, dialogID);
    CObj<NWorld::CUICmd> command = world->GetUICommand();
    auto* packet = dynamic_cast<NWorld::CUICmdPlayAck*>(command.GetPtr());
    Require(packet && packet->phrases.size() == NDb::N_ACKINFO_MAX_COUNT,
        "scripted dialogue did not queue its original capped phrase sequence");
    CObj<NWorld::CAckEvent> ordinary = new NWorld::CAckEvent(10, actor, info);
    CObj<AckPriorityDesktop> desktop = new AckPriorityDesktop;
    desktop->PlayAck(ordinary);
    desktop->Draw(0, nullptr);
    Require(desktop->played.size() == 1 && desktop->played.back() == ordinary.GetPtr(),
        "dialogue-priority fixture did not start its ordinary acknowledgement");
    desktop->PlayAck(packet->phrases.front());
    desktop->Draw(1, nullptr);
    Require(desktop->played.size() == 2 && desktop->played.back() == packet->phrases.front().GetPtr(),
        "scripted dialogue was rejected behind an active ordinary acknowledgement");
    desktop->PlayAck(ordinary);
    desktop->Draw(2, nullptr);
    Require(desktop->played.size() == 2 && ordinary->nPriority == 10,
        "ordinary acknowledgement interrupted scripted dialogue or changed its own priority");
}

static void CorpseBackpackDrop(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) { actor = unit; break; }
    }
    Require(actor != nullptr, "backpack-drop fixture has no ordinary living actor");
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    while (!inventory->GetItems().empty()) inventory->Take(inventory->GetItems().front().pItem);
    NDb::CRPGItem* itemTemplate = nullptr;
    CDBIterator<NDb::CRPGItem> items(*NDatabase::GetTable<NDb::CRPGItem>());
    while (items.MoveNext()) {
        auto* item = items.Get();
        if (IsValid(item->pModel) && IsValid(item->pSuccessor) && item->sSize.x > 0
                && item->sSize.y > 0 && item->sSize.x <= 2 && item->sSize.y <= 2) {
            itemTemplate = item; break;
        }
    }
    Require(itemTemplate != nullptr, "backpack-drop fixture has no small modelled inventory item");
    vector<CObj<NRPG::IInventoryItem>> expected;
    for (int i = 0; i < 3; ++i) {
        CObj<NRPG::IInventoryItem> item = NRPG::CreateItem(itemTemplate->pSuccessor);
        CTPoint<int> place;
        Require(IsValid(item) && inventory->FindPlace(item, &place) && inventory->Place(place, item),
            "backpack-drop fixture could not store its three distinct items");
        expected.push_back(item);
    }
    CObj<NDb::CRPGItem> inertTemplate = new NDb::CRPGItem;
    inertTemplate->sSize = CTPoint<int>(1, 1);
    inertTemplate->subType = NDb::SUBTYPE_NONE;
    inertTemplate->nWeight = inertTemplate->nSubTypePriority = 0;
    CObj<NDb::CRPGKey> inertKey = new NDb::CRPGKey;
    inertKey->pItem = inertTemplate.GetPtr();
    inertKey->nKeyID = 0;
    CObj<NRPG::IInventoryItem> inert = NRPG::CreateItem(inertKey);
    CTPoint<int> inertPlace;
    Require(IsValid(inert) && inventory->FindPlace(inert, &inertPlace) && inventory->Place(inertPlace, inert),
        "backpack-drop fixture could not store its unmodelled item");
    Require(inventory->GetItems().size() == 4, "backpack-drop fixture merged distinct inventory entries");
    actor->SetBloodyDeath(false);
    actor->GetUnitRPG()->GetRPGUnit()->Kill();
    actor->SyncConscious();
    Require(actor->IsDead(), "backpack-drop fixture did not take the ordinary death path");
    Require(inventory->GetItems().empty(), "ordinary death skipped a backpack item while removing earlier entries");
    vector<CPtr<NRPG::IInventoryItem>> ground;
    world->GetDebris()->GetGroundItemsForDiagnostics(&ground);
    for (const auto& item: expected)
        Require(std::count_if(ground.begin(), ground.end(), [&](const auto& entry) { return entry.GetPtr() == item.GetPtr(); }) == 1,
            "ordinary death lost or duplicated a backpack item instead of launching it once");
    Require(std::none_of(ground.begin(), ground.end(), [&](const auto& item) { return item.GetPtr() == inert.GetPtr(); }),
        "unmodelled backpack record changed retail's nonvisual cleanup policy");
}

static void RestoreActionState(const char* databasePath, const char* resourcePath, bool zoneReentry)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    Require(!world->GetBuildingsForHarness().empty() && !world->IsAction(), "fixture has no settled buildings");
    vector<int> segments;
    for (const auto& building : world->GetBuildingsForHarness()) segments.push_back(building->nSegemntCnt);
    const bool realtime = world->IsRealTime();
    auto* player = world->GetCurrentPlayer();
    if (zoneReentry) {
        world->CreateRestored(game);
        Require(world->IsAction(), "zone reentry no longer resumed its building simulation");
        for (const auto& building : world->GetBuildingsForHarness())
            Require(building->nSegemntCnt == 0, "zone reentry did not rearm its building updates");
    } else {
        world->RestoreRuntimeCaches(game);
        Require(!world->IsAction(), "restoring save runtime caches started a new building action");
        int index = 0;
        for (const auto& building : world->GetBuildingsForHarness())
            Require(building->nSegemntCnt == segments[index++], "save cache restoration reset building segment timing");
        Require(world->IsRealTime() == realtime && world->GetCurrentPlayer() == player,
            "save cache restoration restarted the active turn");
    }
}

static void RestoreBuildingHulls(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    Require(!world->GetBuildingsForHarness().empty(), "fixture has no building geometry");
    auto* map = world->GetAIMap();
    map->Sync();
    SBound area;
    area.SphereInit(VNULL3, 10000.f);
    vector<CPtr<CObjectBase>> hulls;
    map->SelectHullPointers(&hulls, area, ~0, 0);
    Require(!hulls.empty(), "fixture has no live AI geometry hulls");
    for (const auto& hull : hulls) Require(IsValid(hull), "fixture retained an invalid AI hull");
    // Stability catchers retain weak hull references too. Reconnecting derived
    // save caches must leave unchanged support geometry alive through AI Sync.
    world->RestoreRuntimeCaches(game);
    map->Sync();
    for (const auto& hull : hulls)
        Require(IsValid(hull), "save cache restoration destroyed existing AI support hulls");
    Require(!world->IsAction(), "cache-only building restoration started simulation");
}

class RetiredSupportFixture: public CObjectBase
{
    OBJECT_BASIC_METHODS(RetiredSupportFixture);
};

struct TrackedCorpseFixture
{
    CPtr<NWorld::CUnitServer> unit;
    vector<CPtr<CObjectBase>> catchers;
    SBound bound;
    int operator&(CStructureSaver& saver) {
        saver.Add(2, &unit); saver.Add(3, &catchers); saver.Add(4, &bound); return 0;
    }
};

static void CarriedCorpseStability(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> humans;
    for (auto& actor : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(actor.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) humans.push_back(unit);
        if (humans.size() == 2) break;
    }
    Require(humans.size() == 2, "fixture needs a body and its carrier");
    auto* body = humans[0];
    auto* carrier = humans[1];
    body->GetUnitRPG()->GetRPGUnit()->Kill();
    body->SetState(new NWorld::CUnitStateDeath(body));
    body->animator.BeTaken(carrier, &carrier->animator, body);
    Require(body->animator.bIsCarried, "fixture did not attach the corpse to its carrier");

    // Restore the existing tracker wire with the former resting support absent
    // from the current map. No private production accessor or map copy is needed.
    CObj<RetiredSupportFixture> formerSupport = new RetiredSupportFixture;
    TrackedCorpseFixture record;
    record.unit = body;
    record.catchers.push_back(formerSupport.GetPtr());
    record.bound.SphereInit(body->GetPosition().GetCP(), 1.f);
    vector<TrackedCorpseFixture> corpses{record};
    CPtr<NWorld::IWorld> worldRef = world.GetPtr();
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[world.GetPtr()] = 1; references.refs[1] = world.GetPtr();
    references.ids[body] = 2; references.refs[2] = body;
    references.ids[formerSupport.GetPtr()] = 3; references.refs[3] = formerSupport.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CObj<NAI::IAIMapTracker> tracker = dynamic_cast<NAI::IAIMapTracker*>(pSSClasses->CreateObject(int(0x71812130)));
    Require(tracker != nullptr, "corpse stability tracker class is not registered");
    CMemoryStream state;
    { CStructureSaver writer(state, CStructureSaver::WRITE, &references);
      writer.Add(2, &corpses); writer.Add(5, &worldRef); writer.Add(6, &record.bound); }
    state.Seek(0);
    { CStructureSaver reader(state, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(tracker.GetPtr())->operator&(reader); }
    tracker->OnChange();
    Require(body->animator.bIsCarried, "old ground stability tracking dropped a corpse already attached to its carrier");
    tracker->OnChange();
    Require(body->animator.bIsCarried, "repeated stability notification detached a carried corpse");
}

class TileAttackMission: public AttackCursorMission
{
    OBJECT_NOCOPY_METHODS(TileAttackMission);
public:
    CVec3 target = VNULL3;
    void AttachWorld(NWorld::CWorld* world) { pWorld = world; }
    bool GetTracePosition(CVec3* result) const override { *result = target; return true; }
    bool GetTracePosition(NAI::SPosition*) const override { return false; }
};

static void ExactAttackPreviewTarget(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnit* shooter = nullptr;
    CVec3 target = VNULL3;
    int chance = 0;
    for (auto& actor : units) {
        auto* weapon = dynamic_cast<NRPG::IWeaponItemInfo*>(actor->GetRPG()->GetInventoryInfo()->GetActive());
        if (!weapon || weapon->GetDBWeapon()->bBazookaLogic) continue;
        for (float distance : {1.f, 2.f, 4.f}) {
            target = actor->GetPosition().GetCP() + CVec3(distance, 0.f, 0.97f);
            chance = world->GetGame()->GetTileCompositeToHit(actor, target, NAI::THL_MIDDLE, world->IsFirstTurn());
            if (chance >= 2) { shooter = actor; break; }
        }
        if (shooter) break;
    }
    Require(shooter != nullptr, "fixture has no positive firearm chance at a 3D target");
    CObj<TileAttackMission> mission = new TileAttackMission;
    mission->AttachWorld(world);
    mission->target = target;
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission, shooter);
    mission->selection.push_back(tracker.GetPtr());
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    NGame::SActionInfo info;
    info.SetValid();
    SetAttackPreview(state, info);
    const int ap = shooter->GetRPG()->GetAP();
    state->RefreshCaption();
    const auto expected = NUI::GetDBString(20276) + NUI::GetDBString(19808) + L"0"
        + NUI::GetDBString(20276) + NUI::GetDBString(19809) + std::to_wstring(chance) + L"%";
    Require(state->GetCursorInfo().wsText == expected,
        "attack preview lost the exact 3D target when no movement tile was available");
    Require(shooter->GetRPG()->GetAP() == ap, "attack preview spent action points");
}

static void GroupAttackPreview(const char* databasePath, const char* resourcePath, bool mixedChance)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units, candidates;
    world->GetAllUnits(&units);
    for (auto& actor : units) {
        auto* weapon = dynamic_cast<NRPG::IWeaponItemInfo*>(actor->GetRPG()->GetInventoryInfo()->GetActive());
        if (weapon && !weapon->GetDBWeapon()->bBazookaLogic) candidates.push_back(actor);
    }
    Require(candidates.size() >= 2, "fixture needs two distinct firearm users");
    vector<CPtr<NWorld::CUnit>> shooters{candidates[0], candidates[1]};
    auto chances = [&](const CVec3& target) {
        const int a = world->GetGame()->GetTileCompositeToHit(shooters[0], target, NAI::THL_MIDDLE, world->IsFirstTurn());
        const int b = world->GetGame()->GetTileCompositeToHit(shooters[1], target, NAI::THL_MIDDLE, world->IsFirstTurn());
        return std::make_pair((std::min)(a, b), (std::max)(a, b));
    };
    CVec3 target(1000, 1000, 1);
    auto range = chances(target);
    if (mixedChance) {
        // Exercise the existing native-client preview cache with two server
        // results. This controls UI inputs without changing combat formulas.
        world->bNetworkReplica = true;
        world->networkHitChance = [&](int kind, NWorld::CUnit* actor, NWorld::CUnit*, CVec3, int, bool) {
            Require(kind == 2, "group preview requested the wrong cached attack kind");
            return actor == shooters[0].GetPtr() ? 73 : 0;
        };
        range = chances(target);
    } else {
        Require(range.first == 0 && range.second == 0, "fixture distant target is not out of range for both shooters");
    }
    CObj<TileAttackMission> mission = new TileAttackMission;
    mission->AttachWorld(world);
    mission->target = target;
    vector<CObj<NGame::CUnitTracker>> trackers;
    for (auto& shooter : shooters) {
        trackers.emplace_back(new NGame::CUnitTracker(mission, shooter));
        mission->selection.push_back(trackers.back().GetPtr());
    }
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    NGame::SActionInfo info;
    info.SetValid();
    SetAttackPreview(state, info);
    state->RefreshCaption();
    auto percent = std::to_wstring(range.first);
    if (range.first != range.second) percent += L"-" + std::to_wstring(range.second);
    percent += L"%";
    const auto expected = NUI::GetDBString(20276) + NUI::GetDBString(19808) + L"0"
        + NUI::GetDBString(range.second >= 2 ? 20276 : 19807) + NUI::GetDBString(19809) + percent;
    Require(state->GetCursorInfo().wsText == expected, mixedChance
        ? "group attack caption was coloured by the worst chance despite a valid selected shooter"
        : "equal selected-unit hit chances were displayed as a redundant percentage range");
}

static NWorld::CExecShootUnit* FindQueuedUnitShot(NWorld::CCommandExecute* executor)
{
    if (auto* shot = dynamic_cast<NWorld::CExecShootUnit*>(executor)) return shot;
    NWorld::CExecShootUnit* found = nullptr;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* object) {
        if (auto* shot = dynamic_cast<NWorld::CExecShootUnit*>(object)) {
            found = shot;
            return false;
        }
        return dynamic_cast<NWorld::CSimpleExecQueue*>(object) != nullptr;
    };
    CObj<NWorld::CCommandExecute> root = executor;
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph); writer.Add(1, &root); }
    return found;
}

struct ShotBaseSnapshot
{
    int extraAP = 0;
    bool onlyPrepare = false;
    int operator&(CStructureSaver& saver) {
        saver.Add(2, &extraAP); saver.Add(12, &onlyPrepare); return 0;
    }
};
struct ShotCommandSnapshot
{
    ShotBaseSnapshot base;
    CPtr<NWorld::CUnitServer> target;
    NAI::EHitLocation hitLocation = NAI::HL_ANY;
};

static ShotCommandSnapshot ReadStartedShot(NWorld::CExecShootUnit* shot,
    NWorld::CUnitServer* actor, NWorld::CUnitServer* target)
{
    // Start fills the shot/ray bookkeeping before inspecting a valid in-flight
    // snapshot. No animation labels or world segments fire a bullet here.
    shot->Start();
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[actor] = 1; references.refs[1] = actor;
    references.ids[target] = 2; references.refs[2] = target;
    references.nextID = 100;
    references.include = [](CObjectBase*) { return false; };
    CMemoryStream state;
    { CStructureSaver writer(state, CStructureSaver::WRITE, &references);
      static_cast<CObjectBase*>(shot)->operator&(writer); }
    state.Seek(0);
    ShotCommandSnapshot snapshot;
    { CStructureSaver reader(state, CStructureSaver::READ, &references);
      reader.Add(1, &snapshot.base); reader.Add(2, &snapshot.target); reader.Add(3, &snapshot.hitLocation); }
    return snapshot;
}

static void SniperHeldAim(const char* databasePath, const char* resourcePath, bool targetlessProbe = false)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NDb::CRPGWeapon* rifle = nullptr;
    CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while (weapons.MoveNext()) {
        auto* candidate = weapons.Get();
        if (candidate->shootModes[NDb::SM_Snipe] && !candidate->bBazookaLogic &&
                IsValid(candidate->pItem) && IsValid(candidate->pInnerClip)) {
            rifle = candidate; break;
        }
    }
    Require(rifle != nullptr, "fixture database has no sniper-capable firearm");
    NWorld::CUnitServer* actor = nullptr;
    NWorld::CUnitServer* target = nullptr;
    CObj<NWorld::CUnitStateSniping> state;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (!unit || !unit->CanFight() || unit->IsEmptyPK() || unit->IsWearingPK()) continue;
        CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(rifle);
        auto* inventory = unit->GetUnitRPG()->GetInventory();
        if (!inventory->Equip(NDb::SLOT_1, weapon) || !inventory->Activate(NDb::SLOT_1) ||
                !weapon->SetShootMode(NDb::SM_Snipe)) continue;
        for (auto& other : units) {
            auto* victim = dynamic_cast<NWorld::CUnitServer*>(other.GetPtr());
            if (!victim || victim == unit || !victim->CanFight() || victim->GetPlayer() != unit->GetPlayer()) continue;
            CObj<NWorld::CUnitStateSniping> probe = new NWorld::CUnitStateSniping(unit, victim, 30, NAI::HL_HEAD);
            unit->SetState(probe);
            CObj<NWorld::CCmdShootObject> explicitShot = new NWorld::CCmdShootObject(victim, 0, NAI::HL_HEAD);
            NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
            CObj<NWorld::CCommandExecute> executor = probe->CreateExecutor(explicitShot, &result);
            auto* shot = executor ? FindQueuedUnitShot(executor) : nullptr;
            if (shot && result == NWorld::UCR_OK && shot->CanDoIt(unit->GetPosition()) == NWorld::UCR_OK) {
                actor = unit; target = victim; state = probe; break;
            }
        }
        if (actor) break;
        unit->SetState(new NWorld::CUnitStateNormal(unit));
    }
    Require(actor && target && state, "fixture contains no currently shootable held sniper target");
    if (targetlessProbe) {
        auto check = [&](CObjectBase* requestedTarget, NAI::EHitLocation point, int extraAP,
                         bool onlyPrepare, NAI::EHitLocation expectedPoint) {
            CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(rifle);
            auto* inventory = actor->GetUnitRPG()->GetInventory();
            Require(inventory->Equip(NDb::SLOT_1, weapon) && inventory->Activate(NDb::SLOT_1) &&
                weapon->SetShootMode(NDb::SM_Snipe), "fixture could not reset the sniper magazine");
            CObj<NWorld::CCmdShootObject> command = new NWorld::CCmdShootObject(requestedTarget, extraAP, point);
            command->bOnlyPrepareToShoot = onlyPrepare;
            NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
            CObj<NWorld::CCommandExecute> executor = state->CreateExecutor(command, &result);
            auto* shot = executor ? FindQueuedUnitShot(executor) : nullptr;
            Require(shot && result == NWorld::UCR_OK, "targetless sniper Fire did not resolve the held target");
            CObj<NWorld::CExecShootUnit> reference = new NWorld::CExecShootUnit(actor, target, expectedPoint, extraAP, onlyPrepare);
            Require(shot->GetStartAP() == reference->GetStartAP(), "sniper Fire lost the extra-AP preview cost");
            const int beforeAP = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
            const auto snapshot = ReadStartedShot(shot, actor, target);
            Require(snapshot.target.GetPtr() == target && snapshot.hitLocation == expectedPoint,
                "sniper Fire resolved the wrong target or aim point");
            Require(snapshot.base.onlyPrepare == onlyPrepare, "sniper Fire lost its prepare-only command flag");
            if (onlyPrepare) {
                Require(snapshot.base.extraAP == extraAP, "sniper preparation lost its extra-AP parameter");
                Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == beforeAP,
                    "sniper preparation spent firing action points");
            }
        };
        check(nullptr, NAI::HL_ANY, 7, true, NAI::HL_HEAD);
        check(target, NAI::HL_ANY, 5, true, NAI::HL_HEAD);
        check(nullptr, NAI::HL_BODY, 3, true, NAI::HL_BODY);
        CObj<NWorld::CCmdShootObject> wrongTarget = new NWorld::CCmdShootObject(actor, 0);
        NWorld::EUnitCommandResult result = NWorld::UCR_OK;
        CObj<NWorld::CCommandExecute> rejected = state->CreateExecutor(wrongTarget, &result);
        Require(!rejected && result == NWorld::UCR_UNAVAILABLE, "sniper Fire retargeted an unrelated explicit target");
        state = new NWorld::CUnitStateSniping(actor, target, 30, NAI::HL_ANY);
        actor->SetState(state);
        check(nullptr, NAI::HL_ANY, 0, true, NAI::HL_ANY);
        check(nullptr, NAI::HL_HEAD, 0, false, NAI::HL_HEAD);
        return;
    }
    CObj<NWorld::CCmdSnipeAttack> command = new NWorld::CCmdSnipeAttack;
    NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
    CObj<NWorld::CCommandExecute> executor = state->CreateExecutor(command, &result);
    auto* shot = executor ? FindQueuedUnitShot(executor) : nullptr;
    Require(shot && result == NWorld::UCR_OK, "sniper Fire did not create a unit shot");
    const auto snapshot = ReadStartedShot(shot, actor, target);
    Require(snapshot.hitLocation == NAI::HL_HEAD, "sniper Fire discarded the held called-shot location");
    Require(snapshot.target.GetPtr() == target, "sniper Fire changed its held target");
}

static void PersonClueCreation(const char* databasePath, const char* resourcePath, bool withInventoryClue = false)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    NScenario::CScenarioClue* clue = nullptr;
    CDBIterator<NDb::CDBScenarioClue> clues(*NDatabase::GetTable<NDb::CDBScenarioClue>());
    while (clues.MoveNext()) {
        auto* record = clues.Get();
        if (record->clueType == NDb::CT_PERSON && record->nPersID > 0 && NDb::GetPers(record->nPersID)) {
            clue = game->pScenarioTracker->GetClueByPersID(record->nPersID);
            if (clue) break;
        }
    }
    Require(clue != nullptr, "fixture has no scenario person clue");
    auto* itemClue = game->pScenarioTracker->GetClueByName("Find_Cipher_key");
    auto* item = itemClue ? NDb::GetRPGItem(itemClue->GetDBClue()->nItemID) : nullptr;
    Require(item != nullptr, "fixture has no inventory clue item");
    item->pSuccessor = nullptr;
    item->sSize = CTPoint<int>(1, 1);
    // Force the authored human spawn points to offer person-clue slots. All
    // changes stay in the loaded database, without writing any game assets.
    CDBIterator<NDb::CUnit> records(*NDatabase::GetTable<NDb::CUnit>());
    while (records.MoveNext()) {
        auto* unit = records.Get();
        unit->bClueSlot = IsValid(unit->pMonster) && !IsValid(unit->pMonster->pPanzerklein);
        unit->bClueInventorySlot = withInventoryClue;
    }
    CDBIterator<NDb::CPlacableObject> objects(*NDatabase::GetTable<NDb::CPlacableObject>());
    while (objects.MoveNext()) {
        auto* object = objects.Get();
        if (IsValid(object->pRPGItem) && object->pRPGItem->GetRecordID() == 188)
            object->pRPGItem = nullptr;
    }
    CObj<NWorld::CWorld> world = new NWorld::CWorld(game);
    CObj<NWorld::CPostWorldCreateInfo> post;
    std::list<CPtr<NScenario::CScenarioClue>> requested{clue};
    if (withInventoryClue) requested.push_back(itemClue);
    world->CreateRandom(810, {}, true, requested, 0, &post, SRandomSeed(123));
    vector<CPtr<NWorld::CUnit>> actors;
    world->GetAllUnits(&actors);
    int found = 0;
    for (auto& actor : actors) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(actor.GetPtr());
        if (actor->GetRPG()->GetRPGUnit()->GetPers()->GetRecordID() == clue->GetDBClue()->nPersID) {
            ++found;
            if (withInventoryClue) {
                int carried = 0;
                for (const auto& entry : actor->GetRPG()->GetInventoryInfo()->GetItems())
                    if (dynamic_cast<NRPG::IClueItem*>(entry.pItem.GetPtr()) && entry.pItem->GetDBItem() == item)
                        ++carried;
                Require(carried == 1, "inventory clue was lost when its slot's authored person was replaced");
                Require(server->nClueCount == 2, "replacement person did not retain both scenario clues");
            } else {
                Require(server->IsClueUnit() && server->nClueCount == 1,
                    "spawned scenario person lost its clue marker and corpse-cleanup protection");
            }
        } else {
            Require(!server->IsClueUnit(), "ordinary actor was marked as a scenario person");
        }
    }
    Require(found == 1, "fixture did not spawn exactly one scenario person");
}

static void InventoryClueCreation(const char* databasePath, const char* resourcePath, bool checkCount = false, bool noSpace = false)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    auto* clue = game->pScenarioTracker->GetClueByName("Find_Cipher_key");
    Require(clue && clue->GetDBClue()->clueType == NDb::CT_ITEM, "fixture has no item clue");
    auto* record = NDb::GetRPGItem(clue->GetDBClue()->nItemID);
    Require(record != nullptr, "fixture item clue has no RPG record");
    // A pure quest item has no ordinary-item successor. Keep fixture changes
    // in the loaded database only; no game.db or resource copy is written.
    record->pSuccessor = nullptr;
    record->sSize = noSpace ? CTPoint<int>(100, 100) : CTPoint<int>(1, 1);
    CDBIterator<NDb::CUnit> units(*NDatabase::GetTable<NDb::CUnit>());
    while (units.MoveNext()) {
        auto* unit = units.Get();
        unit->bClueSlot = false;
        unit->bClueInventorySlot = IsValid(unit->pMonster) && !IsValid(unit->pMonster->pPanzerklein);
    }
    // MapBuild's item 188 is the floor clue-slot marker. This fixture routes
    // every candidate slot through a normal human's backpack instead.
    CDBIterator<NDb::CPlacableObject> objects(*NDatabase::GetTable<NDb::CPlacableObject>());
    while (objects.MoveNext()) {
        auto* object = objects.Get();
        if (IsValid(object->pRPGItem) && object->pRPGItem->GetRecordID() == 188)
            object->pRPGItem = nullptr;
    }
    CObj<NWorld::CWorld> world = new NWorld::CWorld(game);
    CObj<NWorld::CPostWorldCreateInfo> post;
    world->CreateRandom(810, {}, true, std::list<CPtr<NScenario::CScenarioClue>>{clue}, 0, &post, SRandomSeed(123));
    vector<CPtr<NWorld::CUnit>> actors;
    world->GetAllUnits(&actors);
    int found = 0;
    for (auto& actor : actors) {
        auto* inventory = actor->GetRPG()->GetInventoryInfo();
        int carried = 0;
        for (const auto& entry : inventory->GetItems())
            if (dynamic_cast<NRPG::IClueItem*>(entry.pItem.GetPtr()) && entry.pItem->GetDBItem() == record)
                ++carried;
        found += carried;
        if (checkCount)
            Require(dynamic_cast<NWorld::CUnitServer*>(actor.GetPtr())->nClueCount == carried,
                "inventory clue placement did not count the clue on its carrier");
    }
    if (noSpace)
        Require(found == 0, "inventory clue placement inserted a quest item without room in the backpack");
    else
        Require(found == 1, "inventory clue placement lost a quest item without an ordinary successor");
}


class EmptyCorpseNetwork: public NAI::CPathNetwork
{
    OBJECT_BASIC_METHODS(EmptyCorpseNetwork);
public:
    int queries = 0;
    void GetNearPlaces(const SSphere&, vector<NAI::SPathPlace>* places, bool) override {
        Require(++queries <= 16, "corpse snap exceeded two bounded navigation searches");
        places->clear();
    }
};

static void FailedCorpseSnap(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) {
            actor = unit; break;
        }
    }
    Require(actor != nullptr, "corpse-snap fixture contains no ordinary actor");
    CObj<EmptyCorpseNetwork> network = new EmptyCorpseNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    NAI::SUnitPosition logical = actor->GetPosition();
    logical.pos = NAI::SPosition(NAI::SPathPlace(3, 3, 0), network);
    logical.pos.p.SetPose(NAI::CM_STAND);
    network->GetLayer(0)->tiles[3][3].nFloor = 0;
    actor->GetUnitRPG()->GetRPGUnit()->Kill();
    actor->SetState(new NWorld::CUnitStateDeath(actor));
    actor->SetTemporaryPosition(logical);
    Require(!actor->CanFight() && actor->GetPosition().IsValid(), "corpse fixture did not retain a valid logical tile");
    // Empty physical-search results are transient: retain the prior valid tile
    // and retry on the next segment rather than caching the failed body snap.
    actor->CDumbUnitServer::Segment();
    Require(actor->GetPosition() == logical, "failed corpse snap replaced the last valid logical position");
    Require(network->queries == 8 && actor->GetCorpseHLs()->size() == 6,
        "corpse segment did not refresh hit locations after its bounded search");
    actor->CDumbUnitServer::Segment();
    Require(actor->GetPosition() == logical && network->queries == 16,
        "failed corpse snap was cached instead of being retried on the next segment");
}


static void MovementCriticalProbe(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK()) { actor = unit; break; }
    }
    Require(actor != nullptr, "movement-critical fixture has no human actor");
    actor->SetState(new NWorld::CUnitStateNormal(actor));
    CObj<NWorld::CCmdPath> command = new NWorld::CCmdPath(actor->GetPosition().pos);
    const auto wishPose = actor->GetWishPose();
    Require(actor->CanDo(command) == NWorld::UCR_OK, "movement-critical fixture has no baseline path");
    for (auto injury : {NDb::C_PATIENT, NDb::C_MOTIONLESS}) {
        actor->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_LEGS, injury, 3, 2.0f));
        Require(actor->GetUnitRPG()->HasCritical(injury), "fixture did not apply the movement-banning critical");
        const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
        int startAP = 0;
        Require(actor->CanDo(command, &startAP) == NWorld::UCR_CRITICALS_BAN,
            "normal movement eligibility did not reject the actual injury");
        Require(actor->CanDo(command) == NWorld::UCR_CRITICALS_BAN,
            "fast movement preview discarded the existing critical-injury rejection");
        Require(actor->GetWishPose() == wishPose && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
            "movement eligibility probes changed stance or action points");
        Require(actor->GetUnitRPG()->RemoveCritical(injury), "fixture injury could not be removed");
        Require(actor->CanDo(command) == NWorld::UCR_OK, "movement preview stayed blocked after the injury was removed");
    }
    Require(actor->GetWishPose() == wishPose, "movement preview changed the requested stance");
}


class RetargetMoveProbe: public NWorld::CCommandExecute, public NWorld::IExecMove
{
    OBJECT_BASIC_METHODS(RetargetMoveProbe);
    bool started = false;
    NAI::EFindPathParams params = NAI::PF_DEFAULT;
    CObj<NAI::CPath> path;
public:
    int updates = 0;
    NAI::SPathPlace destination;
    RetargetMoveProbe() {}
    RetargetMoveProbe(NWorld::CUnitServer* unit, NWorld::CCmdPath* command): CCommandExecute(unit),
        params(command->eParams), destination(command->ptDst.p) {}
    void Run() override { started = true; }
    bool IsExecuting() override { return started; }
    void GetSearchFromPosition(NAI::SPathPlace* result) override { *result = pUS->GetPosition().pos.p; }
    void SetNewPath(NAI::CPath* value, NAI::EFindPathParams flags, NWorld::ENeedActiveItem) override {
        Require(value && !value->points.empty(), "retarget fixture received an empty accepted path");
        path = value; params = flags; destination = value->points.back(); ++updates;
    }
    void GetDesiredPlace(NAI::SPathPlace* result, NAI::EFindPathParams* flags, NWorld::ENeedActiveItem* active) override {
        *result = destination; *flags = params; *active = NWorld::ITEM_NO_MATTER;
    }
    void GetPathPoints(std::list<SPathPoint>* result) override { result->clear(); }
    void FullCancel() override { started = false; Failed(); }
    NAI::CPath* GetCurrentPath() const override { return path; }
};

class RetargetStateProbe: public NWorld::CUnitStateNormal
{
    OBJECT_BASIC_METHODS(RetargetStateProbe);
public:
    int creations = 0;
    CPtr<NWorld::CCmdPath> restoredCommand;
    CObj<RetargetMoveProbe> move;
    explicit RetargetStateProbe(NWorld::CUnitServer* unit = nullptr): CUnitStateNormal(unit) {}
    NWorld::CCommandExecute* CreateExecutor(NWorld::CCmd* command, NWorld::EUnitCommandResult* result) override {
        auto* route = dynamic_cast<NWorld::CCmdPath*>(command);
        if (!route) return CUnitStateNormal::CreateExecutor(command, result);
        restoredCommand = route; ++creations;
        move = new RetargetMoveProbe(pUS, route);
        *result = NWorld::UCR_OK;
        return move;
    }
};

static void LatestRouteCommand(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK()) { actor = unit; break; }
    }
    Require(actor != nullptr, "retarget fixture has no human actor");
    const auto original = actor->GetPosition();
    vector<NAI::SPathPlace> nearby;
    world->GetPathNetwork()->GetNearPlaces(SSphere(original.GetCP(), 5.f), &nearby);
    vector<CObj<NWorld::CCmdPath>> commands;
    for (auto place : nearby) {
        place.SetPose(NAI::CM_STAND); place.SetDirection(original.GetDir());
        auto destination = original.pos; destination.p = place;
        if (fabs2(destination.GetCP() - original.GetCP()) < 1.f) continue;
        if (!commands.empty() && commands.front()->ptDst.p.GetX() == place.GetX() &&
                commands.front()->ptDst.p.GetY() == place.GetY()) continue;
        CObj<NWorld::CCmdPath> command = new NWorld::CCmdPath(destination);
        if (actor->CanDo(command) != NWorld::UCR_OK) continue;
        commands.push_back(command);
        if (commands.size() == 2) break;
    }
    Require(commands.size() == 2, "retarget fixture needs two different reachable destinations");
    CObj<RetargetStateProbe> state = new RetargetStateProbe(actor);
    actor->SetState(state);
    const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    actor->Do(new NWorld::CCmdSetCommand(actor, commands[0]));
    actor->Do(new NWorld::CCmdSetCommand(actor, new NWorld::CCmdContinue));
    Require(state->creations == 1 && actor->IsPerformingAction(), "retarget fixture did not start its original route");
    CObj<RetargetMoveProbe> previous = state->move;
    actor->Do(new NWorld::CCmdSetCommand(actor, commands[1]));
    Require(previous->updates == 1, "retarget fixture did not accept the replacement path through real pathfinding");
    Require(previous->destination.GetX() == commands[1]->ptDst.p.GetX() &&
            previous->destination.GetY() == commands[1]->ptDst.p.GetY(), "accepted live path has the wrong replacement endpoint");
    // The executor is controlled, but command dispatch, pathfinding, the actual
    // interruption event and RefreshExecutor all run through production code.
    actor->OnTBSEvent(NWorld::TBS_RECALC_COMMAND);
    actor->Do(new NWorld::CCmdSetCommand(actor, new NWorld::CCmdContinue));
    Require(state->creations == 2 && state->restoredCommand == commands[1].GetPtr(),
        "movement interruption restored the old route after a new path was accepted");
    actor->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_LEGS, NDb::C_PATIENT, 3));
    actor->Do(new NWorld::CCmdSetCommand(actor, commands[0]));
    Require(state->move->updates == 0, "injury-blocked route replaced the accepted live path");
    actor->GetUnitRPG()->RemoveCritical(NDb::C_PATIENT);
    actor->OnTBSEvent(NWorld::TBS_RECALC_COMMAND);
    actor->Do(new NWorld::CCmdSetCommand(actor, new NWorld::CCmdContinue));
    Require(state->creations == 3 && state->restoredCommand == commands[1].GetPtr(),
        "rejected retarget replaced the route retained for interruption recovery");
    Require(actor->GetPosition() == original && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "retarget bookkeeping moved the actor or changed action points");
}


class DialogueLayoutView: public NGScene::I2DGameView
{
    OBJECT_BASIC_METHODS(DialogueLayoutView);
    CObj<NGScene::CTextLocaleInfo> locale;
    CVec2 viewport = CVec2(1024, 768);
public:
    int queries = 0;
    int maxQueries = 100000;
    DialogueLayoutView(): locale(new NGScene::CTextLocaleInfo) { locale->Setup(viewport); }
    void CreateDynamicRects(CFuncBase<NGScene::SText>*, const CTPoint<int>&, const CTRect<int>&) override {}
    void CreateDynamicRects(NDb::CTexture*, const CRectLayout&, const CTPoint<int>&, const CTRect<int>&) override {}
    void CreateDynamicRects(CPtrFuncBase<NGfx::CTexture>*, const CRectLayout&, const CTPoint<int>&, const CTRect<int>&) override {}
    CFuncBase<NGScene::SText>* CreateText(CFuncBase<wstring>*, CFuncBase<CTPoint<int>>*, bool) override { return nullptr; }
    void CreateDynamicClearRects(const CRectLayout&, const CTPoint<int>&, const CTRect<int>&, float) override {}
    NGScene::CTextLocaleInfo* GetLocaleInfo() const override { return locale; }
    const CVec2& GetViewportSize() override {
        Require(++queries <= maxQueries, "dialogue pagination made no progress with markup-only content");
        return viewport;
    }
    void StartNewFrame() override {}
    void Flush() override {}
};

static void MedalCursorVisible(const char* databasePath,const char* resourcePath)
{
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "medal cursor fixture started with a live interface or command");
    CFileStream database; database.OpenRead(databasePath); NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    struct Cleanup { bool native = S2Platform::NativeCursorEnabled();
        ~Cleanup() { NMainLoop::DoneInterface(); S2Platform::UseNativeCursor(native); } } cleanup;
    S2Platform::UseNativeCursor(false); // Exercise the same cursor's existing software fallback.
    NDb::CSide* side = nullptr; NDb::CMedal* medal = nullptr;
    CDBIterator<NDb::CSide> sides(*NDatabase::GetTable<NDb::CSide>());
    while (sides.MoveNext()) if (IsValid(sides.Get()->pMedalPaperContainer)) { side = sides.Get(); break; }
    CDBIterator<NDb::CMedal> medals(*NDatabase::GetTable<NDb::CMedal>());
    while (medals.MoveNext()) if (IsValid(medals.Get()->pName)) { medal = medals.Get(); break; }
    Require(side && medal,"medal cursor fixture has no authored medal and side paper");
    CObj<NGScene::CScreenshotTexture> texture = new NGScene::CScreenshotTexture;
    CArray2D<NGfx::SPixel8888> pixels; pixels.SetSizes(2,2);
    for (int y=0; y<2; ++y) for (int x=0; x<2; ++x) pixels[y][x] = NGfx::SPixel8888(12,23,34,255);
    texture->Set(pixels);
    CObj<NGame::CICShowMedal> show = new NGame::CICShowMedal(side,L"Cursor fixture",medal,texture); show->Exec();
    CObj<NMainLoop::IInterfaceBase> modal = NMainLoop::GetCurrentInterfaceForHarness();
    Require(IsValid(modal) && NMainLoop::GetInterfaceStackDepth() == 1,"native medal command did not open its real dialog");
    CStructureNetworkContext refs; auto fields = BorrowedObjectFields(modal,&refs);
    CPtr<NUI::ICursor> cursor; CPtr<NUI::CInterface> interface;
    { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(5,&cursor); reader.Add(6,&interface); }
    Require(IsValid(cursor) && IsValid(interface) && IsValid(interface->GetDefaultCursorInfo().pCursor),
        "native medal dialog did not retain its cursor and authored arrow");
    class CursorPaintView: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(CursorPaintView);
    public:
        int images = 0;
        void CreateDynamicRects(NDb::CTexture*,const CRectLayout&,const CTPoint<int>&,const CTRect<int>&) override { ++images; }
        void CreateDynamicRects(CPtrFuncBase<NGfx::CTexture>*,const CRectLayout&,const CTPoint<int>&,const CTRect<int>&) override { ++images; }
    };
    CObj<CursorPaintView> view = new CursorPaintView;
    const auto info = interface->GetDefaultCursorInfo();
    CObj<NUI::ICursor> control = NUI::ICursor::Create(true);
    control->SetCursor(info); control->SetPos(CVec2(60,60)); control->Draw(1000,view); control->Draw(2000,view);
    Require(view->images > 0,"native visible-cursor control submitted no arrow image");
    const int expected = view->images; view->images = 0;
    cursor->SetCursor(info); cursor->SetPos(CVec2(60,60)); cursor->Draw(1000,view); cursor->Draw(2000,view);
    std::printf("native medal cursor image submissions=%d visible control=%d\n",view->images,expected);
    Require(view->images == expected,"native medal dialog hides its cursor instead of submitting its arrow image");
    NInput::SEvent event; while (NInput::GetEvent(&event)) {}
    NInput::PostEvent("cancel");
    Require(NInput::GetEvent(&event) && modal->ProcessEvent(event),"native medal dialog did not consume its close command");
    CObj<NMainLoop::CInterfaceCommand> close = NMainLoop::TakeInterfaceCommandForHarness();
    Require(dynamic_cast<NMainLoop::CICExitModal*>(close.GetPtr()) != nullptr,"native medal close did not use its ordinary modal command");
    close->Exec();
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"native medal dialog did not close cleanly");
}

static void ChapterMedalScreenshot(const char* databasePath,const char* resourcePath)
{
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "chapter medal screenshot fixture started with a live interface or command");
    NativeDisplayContext display;
    display.Initialize(databasePath,resourcePath,"Silent Storm chapter medal screenshot regression");
    struct GammaRestore {
        NGlobal::CValue value = NGlobal::GetVar("gfx_gamma",1);
        ~GammaRestore() { NGlobal::SetVar("gfx_gamma",value); NGfx::SetGamma(true); }
    } gammaRestore;
    NGfx::SetGamma(true);
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CDBIterator<NDb::CChapterMap> maps(*NDatabase::GetTable<NDb::CChapterMap>());
    NDb::CChapterMap* map = nullptr;
    while (maps.MoveNext()) if (IsValid(maps.Get()->pBackground)) { map = maps.Get(); break; }
    Require(map != nullptr,"chapter medal fixture has no authored map");
    CObj<NGame::CICBeginChapter> enter = new NGame::CICBeginChapter(map->GetRecordID(),game); enter->Exec();
    CObj<NGame::CMissionBase> mission = dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    auto* chapter = IsValid(mission) ? mission->GetInterface()->GetChildByID("chaptermapUI") : nullptr;
    Require(IsValid(chapter),"chapter medal fixture did not open its actual native chapter UI");
    list<CPtr<NScenario::CScenarioClue>> clues; game->pScenarioTracker->GetAvailableClues(&clues);
    for (auto clue : clues) clue->SetJustFound(false);
    auto* merc = game->players.front()->mercs.front().GetPtr();
    Require(merc->medalInfos.size() >= 2 && IsValid(game->players.front()->pSide->pMedalPaperContainer),
        "chapter medal fixture needs two authored medals and their paper");
    // Prepare notification state only; scoring and award thresholds are not exercised.
    for (auto& medal : merc->medalInfos) medal.bJustFound = false;
    struct ReachedRender {};
    class StopBeforeRender: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(StopBeforeRender);
        const CVec2& GetViewportSize() override { throw ReachedRender(); }
    };
    CObj<StopBeforeRender> stop = new StopBeforeRender;
    auto drain = [&](STime time) {
        bool reached = false;
        try { chapter->Draw(time,stop); } catch (const ReachedRender&) { reached = true; }
        Require(reached,"native chapter medal drain did not reach its drawing boundary");
    };
    for (float gamma : {1.f,2.f}) {
        NGlobal::SetVar("gfx_gamma",gamma);
        CMemoryStream reset; int frame = 0;
        { CStructureSaver writer(reset,CStructureSaver::WRITE); writer.Add(18,&frame); }
        reset.Seek(0); { CStructureSaver reader(reset,CStructureSaver::READ); static_cast<CObjectBase*>(chapter)->operator&(reader); }
        for (int i=0; i<2; ++i) { merc->medalInfos[i].bIsGained = true; merc->medalInfos[i].bJustFound = true; }
        NGScene::ClearScreen(CVec3(0.125f,0.25f,0.5f));
        mission->GetInterface()->Draw(1000); NGScene::Flip(); MarkNewDGFrame();
        Require(!NMainLoop::HaveInterfaceCommand(),"native chapter queued its medals before its second painted frame");
        CArray2D<NGfx::SPixel8888> original,corrected;
        NGfx::MakeScreenShot(&original,false); NGfx::MakeScreenShot(&corrected,true);
        Require(original.GetXSize() == 320 && original.GetYSize() == 240,"chapter screenshot did not use its native drawable");
        int distinct = 0, gammaChanged = 0;
        const auto first = original[0][0];
        for (int y=0; y<original.GetYSize(); ++y) for (int x=0; x<original.GetXSize(); ++x) {
            const auto a = original[y][x], b = corrected[y][x];
            if (a.r != first.r || a.g != first.g || a.b != first.b) ++distinct;
            if (a.r != b.r || a.g != b.g || a.b != b.b) ++gammaChanged;
        }
        Require(distinct > 100,"the actual painted chapter framebuffer contains no useful image");
        Require(gamma == 1.f ? gammaChanged == 0 : gammaChanged > 100,
            "native gamma control did not distinguish raw scene pixels from presentation correction");
        drain(1000);
        for (int i=0; i<2; ++i) {
            CObj<NMainLoop::CInterfaceCommand> award = NMainLoop::TakeInterfaceCommandForHarness();
            Require(dynamic_cast<NGame::CICShowMedal*>(award.GetPtr()) != nullptr,"native chapter did not queue exactly its medal commands");
            award->Exec();
            auto* modal = NMainLoop::GetCurrentInterfaceForHarness();
            CStructureNetworkContext refs; auto fields = BorrowedObjectFields(modal,&refs);
            CPtr<NUI::CScreenShot> background;
            { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(8,&background); }
            std::printf("native chapter medal screenshot: gamma=%.1f award=%d texture=%d distinct=%d gammaChanged=%d\n",
                gamma,i,int(IsValid(background) && IsValid(background->GetTexture())),distinct,gammaChanged);
            Require(IsValid(background) && IsValid(background->GetTexture()),
                "native chapter medal dialog has no frozen chapter backdrop");
            CArray2D<NGfx::SPixel8888> captured; background->GetTexture()->Get(&captured);
            Require(captured.GetXSize() == original.GetXSize() && captured.GetYSize() == original.GetYSize(),
                "native medal backdrop lost the chapter framebuffer size");
            for (int y=0; y<original.GetYSize(); ++y) for (int x=0; x<original.GetXSize(); ++x) {
                const auto a = original[y][x], b = captured[y][x];
                Require(a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a,
                    "native medal backdrop recaptured an award overlay or applied presentation gamma twice");
            }
            // Paint the first modal before inspecting the second queued snapshot.
            CPtr<NUI::CInterface> root;
            fields.Seek(0);
            { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(6,&root); }
            Require(IsValid(root),"native medal dialog has no drawable root");
            NGScene::ClearScreen(CVec3(0,0,0)); root->Draw(1100); NGScene::Flip(); MarkNewDGFrame();
            CArray2D<NGfx::SPixel8888> overlay; NGfx::MakeScreenShot(&overlay,false);
            Require(overlay.GetXSize() == original.GetXSize() && overlay.GetYSize() == original.GetYSize(),
                "native medal overlay lost its drawable size");
            int changed = 0;
            for (int y=0; y<original.GetYSize(); ++y) for (int x=0; x<original.GetXSize(); ++x) {
                const auto a = original[y][x], b = overlay[y][x];
                if (a.r != b.r || a.g != b.g || a.b != b.b) ++changed;
            }
            Require(changed > 100,"painting the native award modal did not cover its original chapter framebuffer");
            CObj<NMainLoop::CICExitModal> close = new NMainLoop::CICExitModal; close->Exec();
            Require(NMainLoop::GetCurrentInterfaceForHarness() == mission.GetPtr(),"native medal close did not restore its chapter");
        }
        Require(!NMainLoop::HaveInterfaceCommand() && !merc->medalInfos[0].bJustFound && !merc->medalInfos[1].bJustFound,
            "native chapter did not consume its two notification bits exactly once");
        drain(1000);
        Require(!NMainLoop::HaveInterfaceCommand(),"native chapter repeated its medal notifications on the third frame");
    }
}

static void MedalPanelCaption(const char* databasePath,const char* resourcePath,bool camera = false)
{
    Require(std::getenv("S2_USER_DATA_DIR") != nullptr,"medal panel fixture requires isolated user data");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "medal panel fixture started with a live interface or command");
    NativeDisplayContext display;
    display.Initialize(databasePath,resourcePath,"Silent Storm medal panel caption regression");
    NMainLoop::CreateProfile("UpstreamMedalPanelCaption"); NMainLoop::SetActiveProfile("UpstreamMedalPanelCaption");
    NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(-1);
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginMission> enter = new NGame::CICBeginMission(810,-1,vector<string>(),game); enter->Exec();
    CObj<NGame::CMission> mission = dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(mission) && IsValid(mission->GetWorld()),"medal panel fixture did not open its actual tactical mission");
    CObj<NMainLoop::CInterfaceCommand> restart = NMainLoop::TakeInterfaceCommandForHarness();
    Require(dynamic_cast<NMainLoop::CICSaveFile*>(restart.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),
        "medal panel fixture did not isolate its unrelated startup snapshot");
    // The isolated fixture never writes a restart save; only the live panel is tested.
    NWorld::IPlayer::CUnitSet party; mission->GetActivePlayer()->GetPlayer()->GetUnits(&party);
    Require(party.size() == 1,"medal panel fixture needs its single actual roster unit");
    auto* actor = dynamic_cast<NWorld::CUnit*>(party.begin()->GetPtr());
    Require(IsValid(actor),"medal panel fixture has no live world unit");
    auto* merc = actor->GetRPG()->GetRPGUnit();
    Require(!merc->medalInfos.empty(),"medal panel fixture has no authored medal records");
    for (auto& medal : merc->medalInfos) medal.bIsGained = false;
    int awardIndex = 0;
    if (camera) {
        awardIndex = -1;
        const auto& medals = merc->pPers->pSide->medals;
        for (int i=0; i<medals.size() && i<merc->medalInfos.size(); ++i)
            if (IsValid(medals[i]->pModel) && IsValid(medals[i]->pModel->pModel)) { awardIndex = i; break; }
        Require(awardIndex >= 0,"medal camera fixture has no authored award model");
    }
    merc->medalInfos[awardIndex].bIsGained = true; // Prepare an existing award, without scoring it.
    vector<CDBPtr<NDb::CMedal>> gained; merc->GetGainedMedals(&gained);
    Require(gained.size() == 1 && IsValid(gained.front()->pName),"medal panel fixture did not expose its actual award");
    const auto name = NUI::GetDBString(gained.front()->pName);
    const auto normal = NUI::GetDBString(0x1d85), highlighted = NUI::GetDBString(0x1d86);
    Require(!name.empty() && !normal.empty() && !highlighted.empty() && normal.find(L"<") != wstring::npos,
        "medal panel fixture has no authored name and formatting prefixes");
    mission->GetActivePlayer()->Update(true);
    mission->Select(actor);
    Require(mission->CountSelected() == 1,"medal panel fixture did not synchronize its native selected tracker");
    auto* panel = dynamic_cast<NUI::CMedalsPanel*>(mission->GetDesktop()->GetChildByID("medalspanel"));
    Require(IsValid(panel),"actual tactical UI did not construct its medal panel");
    // Loading may already have painted this selection before the fixture award.
    // Reset only the existing cached selection, so Draw regenerates through its native path.
    CMemoryStream selection; CPtr<NWorld::CUnit> previous;
    { CStructureSaver writer(selection,CStructureSaver::WRITE); writer.Add(3,&previous); }
    selection.Seek(0); { CStructureSaver reader(selection,CStructureSaver::READ); static_cast<CObjectBase*>(panel)->operator&(reader); }
    panel->SetStyle(NUI::STYLE_VISIBLE,true);
    struct ReachedRender {};
    class StopBeforeRender: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(StopBeforeRender);
        const CVec2& GetViewportSize() override { throw ReachedRender(); }
    };
    CObj<StopBeforeRender> stop = new StopBeforeRender;
    bool reached = false;
    try { panel->Draw(1000,stop); } catch (const ReachedRender&) { reached = true; }
    Require(reached,"native medal panel did not generate its award before the drawing boundary");
    CStructureNetworkContext refs; auto fields = BorrowedObjectFields(panel,&refs);
    CPtr<NUI::CListView> list;
    { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(8,&list); }
    auto* row = IsValid(list) ? dynamic_cast<NUI::CButton*>(list->GetItem(0)) : nullptr;
    Require(IsValid(row),"native medal panel did not construct its actual award row");
    for (int state : {0,1,255}) {
        auto* window = row->GetState(state);
        auto* text = IsValid(window) ? dynamic_cast<NUI::CText*>(window->GetChildByID("iml-text")) : nullptr;
        Require(IsValid(text) && text->GetText().find(name) != wstring::npos,"native medal caption lost its authored name");
        const auto& prefix = state == 0 ? normal : highlighted;
        std::printf("native medal caption: state=%d prefix-first=%d name-first=%d prefix-chars=%zu name-chars=%zu\n",
            state,int(text->GetText() == prefix+name),int(text->GetText() == name+prefix),prefix.size(),name.size());
        Require(text->GetText() == prefix+name,"native medal caption appends its formatting after the name");
        NUI::SPoint size; text->GetRealSize(&size);
        Require(size.x > 0 && size.y > 0,"native medal caption has empty text layout");
    }
    if (camera) {
        fields.Seek(0); CPtr<NUI::CModel> preview;
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(9,&preview); }
        Require(IsValid(preview) && IsValid(preview->GetModel()),"native medal panel did not assign its actual preview model");
        CMemoryStream base; CStructureNetworkContext modelRefs;
        { CStructureSaver writer(base,CStructureSaver::WRITE,&modelRefs); preview->NUI::CModel::operator&(writer); }
        base.Seek(0); CPtr<NUI::CModelDraw> model;
        { CStructureSaver reader(base,CStructureSaver::READ,&modelRefs); reader.Add(2,&model); }
        Require(IsValid(model),"native medal preview did not retain its real renderer");
        auto modelFields = BorrowedObjectFields(model,&modelRefs); SHMatrix actual;
        { CStructureSaver reader(modelFields,CStructureSaver::READ,&modelRefs); reader.Add(6,&actual); }
        auto matrix = [](const NDb::SCameraParams& parameters) {
            CVec3 forward; (CQuat(parameters.fYaw,V3_AXIS_Z)*CQuat(parameters.fPitch,V3_AXIS_X)).GetYAxis(&forward);
            SHMatrix result; MakeMatrix(&result,parameters.fPitch,parameters.fYaw,parameters.fRoll,
                parameters.vAnchor-forward*parameters.fDistance); return result;
        };
        const auto& parameters = gained.front()->pModel->sCameras[NDb::CAMERA_RELOADBUTTON];
        const auto expected = matrix(parameters), normal = matrix(gained.front()->pModel->sCameras[NDb::CAMERA_NORMAL]);
        auto distance = [](const SHMatrix& a,const SHMatrix& b) {
            const float left[] = {a._11,a._12,a._13,a._14,a._21,a._22,a._23,a._24,a._31,a._32,a._33,a._34,a._41,a._42,a._43,a._44};
            const float right[] = {b._11,b._12,b._13,b._14,b._21,b._22,b._23,b._24,b._31,b._32,b._33,b._34,b._41,b._42,b._43,b._44};
            float result = 0;
            for (int i=0; i<16; ++i) result = Max(result,float(fabs(left[i]-right[i])));
            return result;
        };
        const float authoredDifference = distance(expected,normal);
        Require(authoredDifference > 0.01f,"medal camera fixture cannot distinguish its two authored camera slots");
        std::printf("native medal preview camera: award=%d item=%d authored-slot-difference=%.6f reload-error=%.6f normal-error=%.6f\n",
            gained.front()->GetRecordID(),gained.front()->pModel->GetRecordID(),authoredDifference,distance(actual,expected),distance(actual,normal));
        Require(distance(actual,expected) < 0.0001f,"native medal preview uses the normal inventory camera instead of its authored award camera");
    }
    // Existing remaster snapshots serialize these raw bits. Do not reinterpret
    // an old inventory panel as medals when importing unrelated UI fixes.
    auto loadPanelFields = [&](int bits,bool firstMission) {
        CMemoryStream saved;
        { CStructureSaver writer(saved,CStructureSaver::WRITE); writer.Add(22,&bits); writer.Add(34,&firstMission); }
        saved.Seek(0); { CStructureSaver reader(saved,CStructureSaver::READ); static_cast<CObjectBase*>(mission.GetPtr())->operator&(reader); }
    };
    loadPanelFields(0x4,true);
    Require(mission->GetPanelState(NGame::PANEL_INVENTORY) == 0x4 &&
        !mission->GetPanelState(NGame::PANEL_MEDALS|NGame::PANEL_BIOGRAPHY),
        "existing remaster inventory snapshot bits were reinterpreted as another panel");
    mission->SetPanelState(NGame::PANEL_MEDALS|NGame::PANEL_BIOGRAPHY,true);
    Require(mission->GetPanelState(NGame::PANEL_INVENTORY) == 0x4 &&
        !mission->GetPanelState(NGame::PANEL_MEDALS|NGame::PANEL_BIOGRAPHY),
        "native first-mission restriction blocked inventory or allowed a restricted character tab");
    loadPanelFields(0x10,false);
    Require(mission->GetPanelState(NGame::PANEL_MEDALS) == 0x10 &&
        !mission->GetPanelState(NGame::PANEL_INVENTORY|NGame::PANEL_BIOGRAPHY),
        "existing remaster medal snapshot bits were reinterpreted as another panel");
    loadPanelFields(0x20,false);
    Require(mission->GetPanelState(NGame::PANEL_BIOGRAPHY) == 0x20 &&
        !mission->GetPanelState(NGame::PANEL_INVENTORY|NGame::PANEL_MEDALS),
        "existing remaster biography snapshot bits were reinterpreted as another panel");
    std::puts("native medal panel compatibility: saved bits 4/16/32 and named first-mission restrictions preserved");
    Require(!NMainLoop::HaveInterfaceCommand(),"measuring native medal captions queued an unrelated game command");
}

static void CheckChapterClueHeading(NGame::CMissionBase* mission,NUI::CWindow* chapter)
{
    auto* game = mission->GetRPGGame(); auto* tracker = game->pScenarioTracker.GetPtr();
    auto* map = chapter->GetChildByID("view");
    list<CPtr<NUI::CWindow>> children; map->GetChildrenList(&children);
    std::printf("chapter heading fixture map=%d scenario=%d available=%d children=%zu\n",
        mission->GetChapterMap()->GetRecordID(),tracker->GetScenarioID(),int(tracker->IsScenarioAvailable()),children.size());
    CPtr<NUI::CWindow> sector; CPtr<NScenario::CScenarioZone> zone; CPtr<NScenario::CScenarioClue> clue;
    for (auto child : children) {
        if (pSSClasses->GetObjectTypeID(child) != int(0xB2606153)) continue;
        CStructureNetworkContext refs; auto fields = BorrowedObjectFields(child,&refs);
        CPtr<NScenario::CScenarioZone> candidate;
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(3,&candidate); }
        std::printf("chapter zone=%d clues=%zu\n",IsValid(candidate) ? candidate->GetDBZone()->GetRecordID() : -1,
            IsValid(candidate) ? candidate->GetClues().size() : 0);
        if (!IsValid(candidate)) continue;
        for (auto entry : candidate->GetClues()) {
            if (!entry->IsCompound() && entry->IsPlaced() && !entry->IsDestroyed() &&
                !entry->GetParentZones().empty() && entry->GetParentZones().front() == candidate &&
                !NUI::GetDBString(entry->GetDBClue()->pDescription).empty()) {
                sector = child; zone = candidate; clue = entry; break;
            }
        }
        if (IsValid(clue)) break;
    }
    Require(IsValid(clue),"chapter clue-heading fixture found no native map sector with a placed clue");
    tracker->CheatOpenZone(zone); tracker->CheatTakeClue(clue,true);
    list<CPtr<NScenario::CScenarioClue>> found; tracker->GetAvailableClues(&found);
    Require(std::find(found.begin(),found.end(),clue) != found.end(),"native chapter fixture did not acquire its actual clue");
    for (auto entry : found) entry->SetJustFound(false);
    const auto clueText = NUI::GetDBString(clue->GetDBClue()->pDescription);
    auto* header = NDb::GetString(0x4f35);
    Require(IsValid(header) && !header->szStr.empty(),"native chapter clue heading has no authored localized record");
    std::printf("native chapter heading record: chars=%zu font-tag=%d count-placeholder=%d\n",header->szStr.size(),
        int(header->szStr.find(L"<font") != wstring::npos),int(header->szStr.find(L"%d") != wstring::npos));
    struct Restore { NDb::CString* record; wstring original; ~Restore() { record->szStr = original; } } restore{header,header->szStr};
    struct ReachedRender {};
    class StopBeforeRender: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(StopBeforeRender);
        const CVec2& GetViewportSize() override { throw ReachedRender(); }
    };
    CObj<StopBeforeRender> view = new StopBeforeRender;
    CStructureNetworkContext chapterRefs; auto chapterFields = BorrowedObjectFields(chapter,&chapterRefs);
    CVec2 position; { CStructureSaver reader(chapterFields,CStructureSaver::READ,&chapterRefs); reader.Add(8,&position); }
    CMemoryStream settled; STime time = 1000; int specialFrame = 3;
    { CStructureSaver writer(settled,CStructureSaver::WRITE); writer.Add(8,&position); writer.Add(9,&position); writer.Add(10,&time); writer.Add(18,&specialFrame); }
    settled.Seek(0); { CStructureSaver reader(settled,CStructureSaver::READ); static_cast<CObjectBase*>(chapter)->operator&(reader); }
    auto draw = [&]() {
        bool reached = false;
        try { chapter->Draw(time,view); } catch (const ReachedRender&) { reached = true; }
        Require(reached && !NMainLoop::HaveInterfaceCommand(),"chapter heading fixture did not reach its drawing boundary cleanly");
    };
    draw(); // Native UpdateSector establishes scenario visibility before the hover pass.
    NUI::SPoint origin; NUI::SRect clip;
    Require(sector->ClientToScreen(&origin,&clip),"native clue sector has no visible map rectangle");
    NUI::SPoint cursor((clip.x1+clip.x2)/2,(clip.y1+clip.y2)/2);
    CMemoryStream cursorFields;
    { CStructureSaver writer(cursorFields,CStructureSaver::WRITE); writer.Add(5,&cursor); }
    cursorFields.Seek(0);
    { CStructureSaver reader(cursorFields,CStructureSaver::READ); static_cast<CObjectBase*>(mission->GetInterface())->operator&(reader); }
    const char* panelID = cursor.x > S2UI::MapOffsetX()+512 ? "text_left" : "text_right";
    auto* panel = chapter->GetChildByID(panelID);
    auto* text = panel ? dynamic_cast<NUI::CText*>(panel->GetChildByID("text")) : nullptr;
    Require(IsValid(text),"native chapter map did not build its description text");
    for (const auto& localized : {restore.original,restore.original+L"\u0423\u043B\u0438\u043A\u0438 regression"}) {
        header->szStr = localized; draw();
        Require(text->GetText().find(clueText) != wstring::npos,"hovering the actual chapter sector did not display its acquired clue");
        std::printf("native chapter clue text: localized heading=%d hardcoded English=%d\n",
            int(text->GetText().find(localized) != wstring::npos),int(text->GetText().find(L"Clues found:") != wstring::npos));
        Require(text->GetText().find(localized) != wstring::npos,"native chapter clue description ignored its localized heading record");
    }
}

static void RejectedHitLabelText(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    auto* rejected = NDb::GetString(21060);
    Require(IsValid(rejected) && !rejected->szStr.empty(), "hit-label fixture has no authored localized rejected-hit text");
    class HitLayoutReached: public std::exception {};
    class HitView: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(HitView);
    public:
        const CVec2& GetViewportSize() override { throw HitLayoutReached(); }
    };
    CObj<HitView> view = new HitView;
    for (bool pk: {false,true}) for (int value: {-1,0,42}) {
        CObj<EnemyIconMission> mission = new EnemyIconMission;
        mission->view = new NGScene::GameViewFixture;
        CObj<MissionIconProbe> desktop = new MissionIconProbe;
        mission->Attach(world,desktop);
        CObj<NUI::CWindow> client = new NUI::CWindow(NUI::SWindowInfo(nullptr,
            NUI::SPoint(0,0),NUI::SPoint(1024,768),"hit-view",NUI::STYLE_ENABLED|NUI::STYLE_VISIBLE));
        CObj<NUI::CText> pause = new NUI::CText;
        CStructureNetworkContext refs;
        refs.externalReferences = true; refs.nextID = 100; refs.include = [](CObjectBase*) { return false; };
        for (CObjectBase* object: {static_cast<CObjectBase*>(mission.GetPtr()),static_cast<CObjectBase*>(client.GetPtr()),static_cast<CObjectBase*>(pause.GetPtr())}) {
            const auto id = refs.nextID++; refs.ids[object] = id; refs.refs[id] = object;
        }
        struct ClientWire {
            CObj<NUI::CWindow> client;
            int operator&(CStructureSaver& saver) { saver.Add(2,&client); return 0; }
        } base;
        base.client = client;
        CPtr<NGame::IMission> missionRef = mission.GetPtr();
        CMemoryStream saved;
        { CStructureSaver writer(saved,CStructureSaver::WRITE,&refs); writer.Add(1,&base); writer.Add(2,&missionRef); writer.Add(27,&pause); }
        saved.Seek(0);
        { CStructureSaver reader(saved,CStructureSaver::READ,&refs); static_cast<CObjectBase*>(desktop.GetPtr())->operator&(reader); }
        world->AddHitLocator(new NWorld::CHitLocator(value,pk,CVec3(0,0,0.5f),nullptr));
        desktop->RefreshHits(0);
        auto* label = dynamic_cast<NUI::CText*>(client->GetChildByID("hit"));
        Require(label && pSSClasses->GetObjectTypeID(label) == int(0xB0241949),
            "native HUD did not create its actual hit-text tracker from a world hit event");
        for (STime time: {STime(0),STime(1000)}) {
            bool reached = false;
            try { label->Draw(time,view); } catch (const HitLayoutReached&) { reached = true; }
            Require(reached, "native hit text did not reach layout after updating its position and source");
            const auto& text = label->GetText();
            const auto prefixEnd = text.find(L'>');
            Require(prefixEnd != wstring::npos && text.substr(0,7) == L"<color=",
                "native hit text lost its existing colour markup");
            const wstring expected = value < 0 ? rejected->szStr : std::to_wstring(value);
            Require(text.substr(prefixEnd+1) == expected,
                "a rejected native HUD hit displayed the negative damage sentinel instead of its localized label");
            const wstring oldColour = pk ? (time == 0 ? L"<color=fff3bf00>" : L"<color=7f795f00>") :
                (time == 0 ? L"<color=ffff0000>" : L"<color=7fff0000>");
            Require(text.substr(0,prefixEnd+1) == oldColour,
                "localizing a rejected hit changed the independent existing colour or fade rules");
        }
        const auto lastText = label->GetText();
        label->Draw(2001,view);
        Require(label->GetText() == lastText, "an expired native hit label regenerated or changed its text");
    }
}

static void ScaledEditTextLayout()
{
    S2Platform::SetErrorDialogs(false);
    struct Cleanup { ~Cleanup() { S2Platform::Done(); } } cleanup;
    Require(S2Platform::Init("scaled native edit text",2048,1536,true), "edit layout fixture could not initialize dummy SDL");
    class EditLayoutView: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(EditLayoutView);
        CObj<NGScene::CCVec2> screen = new NGScene::CCVec2(CVec2(2048,1536));
    public:
        CDGPtr<CFuncBase<CTPoint<int>>> dimensions;
        NGScene::SText rendered;
        vector<CPtrFuncBase<NGfx::CTexture>*> textures;
        CFuncBase<NGScene::SText>* CreateText(CFuncBase<wstring>* text,CFuncBase<CTPoint<int>>* size,bool tags) override {
            dimensions = size;
            return NGScene::CreateTextFormater(GetLocaleInfo(),screen,text,size,tags);
        }
        void CreateDynamicRects(CFuncBase<NGScene::SText>* node,const CTPoint<int>&,const CTRect<int>&) override {
            CDGPtr<CFuncBase<NGScene::SText>> text = node;
            text.Refresh(); rendered = text->GetValue();
            for (const auto& layout: rendered.rectLayouts) textures.push_back(layout.pFontInfo->GetTexture());
        }
        void CreateDynamicRects(CPtrFuncBase<NGfx::CTexture>* texture,const CRectLayout& layout,
            const CTPoint<int>&,const CTRect<int>&) override {
            if (!layout.rects.empty()) textures.push_back(texture);
        }
    };
    CObj<EditLayoutView> view = new EditLayoutView;
    CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false);
    CObj<NUI::CInterface> interface = new NUI::CInterface(cursor);
    NUI::CEdit* edit = new NUI::CEdit(NUI::SWindowInfo(interface,NUI::SPoint(5,5),NUI::SPoint(48,100),"edit",
        NUI::STYLE_VISIBLE | NUI::STYLE_ENABLED));
    const wchar_t* formats[] = {L"",L"<font face=Courier size=16pt><left>",L"<font face=Times size=18pt><left>",L"<font face=Arial size=20pt><left>"};
    const char* families[] = {"System","Courier","Times","Arial"};
    const int points[] = {22,16,18,20}; // Preserve the existing formatter's 22pt unformatted default.
    const wstring caption = L"a b c \u0416 \u042f \u0451 d e f g h i j k";
    for (int family = 0; family < 4; ++family) for (float scale: {1.f,1.5f,2.f}) {
        S2Platform::UpdateDisplay(scale*100); MarkNewDGFrame();
        Require(std::fabs(S2Platform::Display().uiScale-scale) < 0.001f, "edit font fixture could not establish its actual UI scale");
        edit->SetTextFormat(formats[family]); edit->SetText(caption); edit->SetCursorPosition(2);
        view->textures.clear();
        auto* font = view->GetLocaleInfo()->GetFont(NGScene::SFont(Float2Int(points[family]*scale),families[family]));
        Require(IsValid(font) && font->IsVector(), "the test application is missing its packaged high-resolution fonts");
        edit->Draw(1000,view);
        Require(!view->textures.empty() && std::all_of(view->textures.begin(),view->textures.end(),
            [&](auto* texture) { return texture == font->GetTexture(); }),
            "the edit renderer changed the remaster's original high-resolution font or its default point size");
        Require(view->dimensions.GetPtr() && !view->rendered.charsSet.empty() && !view->rendered.rectLayouts.empty(),
            "edit font fixture did not evaluate the real native formatter and packaged glyphs");
        view->dimensions.Refresh();
        const CTPoint<int> expectedSize(Float2Int(48*scale),Float2Int(100*scale));
        Require(view->dimensions->GetValue() == expectedSize,
            "the native edit renderer wrapped packaged screen-sized glyphs at the unscaled logical control width");
        for (const auto& layout: view->rendered.rectLayouts)
            Require(layout.pFontInfo.GetPtr() == font, "edit layout changed the packaged font family or its original point size");
        CObj<NGScene::CCWString> source = new NGScene::CCWString(wstring(formats[family])+caption);
        CObj<NGScene::CCTPoint> size = new NGScene::CCTPoint(expectedSize);
        CObj<NGScene::CCVec2> screen = new NGScene::CCVec2(CVec2(2048,1536));
        CDGPtr<CFuncBase<NGScene::SText>> reference = NGScene::CreateTextFormater(view->GetLocaleInfo(),screen,source,size,true);
        reference.Refresh();
        const auto& expected = reference->GetValue();
        Require(view->rendered.sSize == expected.sSize && view->rendered.charsSet.size() == expected.charsSet.size(),
            "edit text wraps differently from the unchanged native font renderer at the correct pixel width");
        for (std::size_t i = 0; i < expected.charsSet.size(); ++i) {
            const auto& actualRect = view->rendered.charsSet[i].sRect;
            const auto& expectedRect = expected.charsSet[i].sRect;
            Require(view->rendered.charsSet[i].nIndex == expected.charsSet[i].nIndex &&
                actualRect.x1 == expectedRect.x1 && actualRect.y1 == expectedRect.y1 &&
                actualRect.x2 == expectedRect.x2 && actualRect.y2 == expectedRect.y2,
                "edit glyph geometry differs from the remaster's original font renderer");
        }
        Require(edit->GetText() == caption && edit->GetCursorPosition() == 2, "font layout modified the editable source or cursor position");
        std::printf("FONT-COMPAT: %s %dpt scale=%.1f vector=1 glyphs=%zu width=%d height=%d\n",
            families[family],points[family],scale,expected.charsSet.size(),expectedSize.x,expected.sSize.y);
    }
}

static void CheckChapterMarkerToolTipTransitions(NUI::CWindow* marker,NUI::CText* text)
{
    CObj<DialogueLayoutView> view = new DialogueLayoutView;
    const int modes[] = {0,1,2}; // Existing native Move, Normal and Zone marker states.
    const int hints[] = {11123,11124,11126};
    for (int i = 0; i < 3; ++i) {
        int from = modes[i] == 0 ? 1 : 0, target = modes[i];
        STime morph = 1000, flash = 1000;
        CMemoryStream state;
        {
            CStructureSaver writer(state,CStructureSaver::WRITE);
            writer.Add(2,&from); writer.Add(3,&target); writer.Add(4,&morph); writer.Add(5,&flash);
        }
        state.Seek(0);
        { CStructureSaver reader(state,CStructureSaver::READ); static_cast<CObjectBase*>(marker)->operator&(reader); }
        marker->Draw(1001,view);
        Require(!NUI::GetDBString(hints[i]).empty() && text->GetText() == NUI::GetDBString(hints[i]),
            "the chapter marker retained its previous hint when its real draw transitioned to another action");
        // Finish this transition and ensure the hint survives steady-state drawing.
        marker->Draw(10000,view);
        Require(text->GetText() == NUI::GetDBString(hints[i]), "the chapter action hint disappeared after its transition");
    }
}

static void CheckChapterExitMarkerHint(NGame::CMissionBase* mission,NUI::CWindow* chapter,NUI::CWindow* marker,NUI::CText* text)
{
    CDGPtr<CPtrFuncBase<CChapterInfo>> info = mission->GetChapterInfo(); info.Refresh();
    CVec2 position;
    bool found = false;
    for (const auto& sector: info->GetValue()->sectorsSet) {
        if (sector.eType != EXITZONE || sector.pointsSet.empty()) continue;
        position = sector.pointsSet.front()+CVec2(10,10); found = true; break;
    }
    Require(found, "chapter exit-hint fixture has no authored exit sector");
    CMemoryStream state;
    STime last = 1000;
    int specialFrame = 3;
    {
        CStructureSaver writer(state,CStructureSaver::WRITE);
        writer.Add(8,&position); writer.Add(9,&position); writer.Add(10,&last); writer.Add(18,&specialFrame);
    }
    state.Seek(0);
    { CStructureSaver reader(state,CStructureSaver::READ); static_cast<CObjectBase*>(chapter)->operator&(reader); }
    struct ReachedChapterRender {};
    class ChapterSelectionView: public DialogueLayoutView {
        OBJECT_NOCOPY_METHODS(ChapterSelectionView);
    public:
        const CVec2& GetViewportSize() override { throw ReachedChapterRender(); }
    };
    CObj<ChapterSelectionView> selectionView = new ChapterSelectionView;
    CObj<DialogueLayoutView> view = new DialogueLayoutView;
    marker->Draw(1000,view); // A real preceding frame establishes the marker's transition clock.
    bool reachedRender = false;
    try { chapter->Draw(1001,selectionView); }
    catch (const ReachedChapterRender&) { reachedRender = true; }
    Require(reachedRender, "chapter exit-hint fixture never reached its rendering boundary");
    Require(!NMainLoop::HaveInterfaceCommand(), "chapter exit-hint selection unexpectedly queued a campaign command");
    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(marker,&references);
    int target = -1;
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(3,&target); }
    Require(target == 2 || target == 3, "chapter exit-hint fixture did not place the party inside an action sector");
    Require(target == 3, "the native chapter map selected a mission-entry marker for its campaign exit sector");
    marker->Draw(1001,view);
    Require(!NUI::GetDBString(20253).empty() && text->GetText() == NUI::GetDBString(20253),
        "the chapter exit marker displayed the mission-entry hint instead of its authored exit hint");
}

class DialogueMissionProbe: public NGame::CMissionBase
{
    OBJECT_BASIC_METHODS(DialogueMissionProbe);
public:
    int stageEvents = 0;
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    void DoEvent(NWorld::CCommand* command) override { CObj<NWorld::CCommand> hold = command; ++stageEvents; }
};

class DialoguePagesProbe: public NUI::CMissionDlgUI
{
    OBJECT_NOCOPY_METHODS(DialoguePagesProbe);
public:
    using CMissionDlgUI::CMissionDlgUI;
    using CMissionDlgUI::UpdatePhrases;
    using CMissionDlgUI::SetStage;
};

static void OversizedDialogueLine(const char* databasePath, const char* resourcePath, int scenario = 0)
{
    const bool markupOnly = scenario == 1;
    const bool normalPages = scenario == 2;
    auto world = FixtureWorld(databasePath, resourcePath, true);
    vector<CPtr<NWorld::CUnit>> actors;
    world->GetAllUnits(&actors);
    Require(!actors.empty(), "dialogue fixture has no speaker");
    CObj<DialogueMissionProbe> mission = new DialogueMissionProbe;
    vector<CObj<NDb::CString>> texts;
    vector<CObj<NDb::CDBAckInfo>> records;
    vector<CObj<NWorld::CAckEvent>> owners;
    vector<CPtr<NWorld::CAckEvent>> phrases;
    wstring first = normalPages
        ? L"alpha beta gamma delta epsilon zeta eta theta iota kappa lambda mu nu xi omicron pi rho sigma tau upsilon phi chi psi omega"
        : L"Oversized first phrase";
    if (markupOnly) {
        int textureID = 0;
        CDBIterator<NDb::CUITexture> textures(*NDatabase::GetTable<NDb::CUITexture>());
        while (textures.MoveNext()) {
            auto* record = textures.Get();
            for (int mode = 0; mode < NDb::UIM_MAX; ++mode)
                if (IsValid(record->pTextures[mode]) && record->pTextures[mode]->nWidth > 0 && record->pTextures[mode]->nHeight > 0)
                    textureID = record->GetRecordID();
            if (textureID) break;
        }
        Require(textureID != 0, "markup dialogue fixture has no usable image");
        const auto image = L"<image id=" + std::to_wstring(textureID) + L" width=32 height=8>";
        first = image + L"<br>" + image;
    }
    for (const wstring& value : {first, wstring(L"Following second phrase")}) {
        CObj<NDb::CString> text = new NDb::CString; text->szStr = value; texts.push_back(text);
        CObj<NDb::CDBAckInfo> info = new NDb::CDBAckInfo;
        info->pText = text; info->voices.resize(6); records.push_back(info);
        owners.emplace_back(new NWorld::CAckEvent(1, actors.front(), info));
        phrases.push_back(owners.back().GetPtr());
    }
    CObj<DialoguePagesProbe> dialog = new DialoguePagesProbe(
        NUI::SWindowInfo(nullptr, NUI::SPoint(0, 0), NUI::SPoint(500, 100), "dialogue-test"),
        mission, nullptr, "diagnostic", vector<CObj<NWorld::CUnit>>{}, phrases);
    CObj<NUI::CText> text = new NUI::CText(
        NUI::SWindowInfo(nullptr, NUI::SPoint(0, 0), NUI::SPoint(normalPages ? 120 : 500, normalPages ? 50 : markupOnly ? 10 : 1), "dialogue-text"));
    CObj<NUI::CHoverButton> back = new NUI::CHoverButton, next = new NUI::CHoverButton, exit = new NUI::CHoverButton;
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[text.GetPtr()] = 1; references.refs[1] = text.GetPtr();
    references.ids[back.GetPtr()] = 2; references.refs[2] = back.GetPtr();
    references.ids[next.GetPtr()] = 3; references.refs[3] = next.GetPtr();
    references.ids[exit.GetPtr()] = 4; references.refs[4] = exit.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CMemoryStream controls;
    { CStructureSaver writer(controls, CStructureSaver::WRITE, &references);
      writer.Add(14, &text); writer.Add(15, &back); writer.Add(16, &next); writer.Add(17, &exit); }
    controls.Seek(0);
    { CStructureSaver reader(controls, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(dialog.GetPtr())->operator&(reader); }
    CObj<DialogueLayoutView> view = new DialogueLayoutView;
    CObj<NUI::IML> layout = NUI::CreateML();
    layout->SetText(texts.front()->szStr, 0); layout->Generate(view, text->GetSize().x);
    Require(layout->GetSize().y > text->GetSize().y, "dialogue fixture did not overflow its text box");
    if (markupOnly) {
        list<NUI::SRect> bounds;
        layout->Render(&bounds, NUI::SPoint(0, 0), NUI::SRect(0, 0, 0, 0));
        Require(bounds.size() == 2, "markup dialogue fixture did not lay out both image bounds");
        for (const auto& bound : bounds) Require(bound.y2 <= 10, "markup fixture did not expose fitting bounds with overflowing line spacing");
        view->queries = 0; view->maxQueries = 512;
    }
    dialog->UpdatePhrases(view);
    if (normalPages) {
        auto plainText = [](const wstring& input) {
            wstring output;
            for (size_t i = 0; i < input.size(); ++i) {
                if (input[i] == L'<') {
                    const auto end = input.find(L'>', i);
                    if (end == wstring::npos) break;
                    i = end;
                } else output += input[i];
            }
            return output;
        };
        wstring recovered;
        int pages = 0, previousPhrase = -1, previousOffset = -1;
        for (; pages < 32; ) {
            dialog->SetStage(pages);
            int page, phrase, offset, phase;
            dialog->GetDisplayProgressForDiagnostics(&page, &phrase, &offset, &phase);
            Require(page == pages && phrase >= 0 && phrase <= 1 && offset >= 0,
                "normal dialogue page lost its logical progress anchor");
            if (phrase == previousPhrase) Require(offset > previousOffset, "normal dialogue continuation did not advance its source offset");
            else Require(phrase == previousPhrase + 1 && offset == 0, "new dialogue phrase did not restart its source offset");
            previousPhrase = phrase; previousOffset = offset;
            recovered += plainText(text->GetText());
            ++pages;
            if (!next->GetStyle(NUI::STYLE_VISIBLE)) break;
        }
        Require(pages > 2 && pages < 32 && previousPhrase == 1,
            "normal dialogue pagination did not reach its final phrase through multiple pages");
        Require(recovered == texts[0]->szStr + texts[1]->szStr,
            "normal dialogue pagination lost or duplicated visible text");
        Require(mission->stageEvents == pages && exit->GetStyle(NUI::STYLE_VISIBLE),
            "normal dialogue pagination did not finish with one stage event per page");
        return;
    }

    dialog->SetStage(0);
    Require(text->GetText().find(texts[0]->szStr) != wstring::npos,
        "oversized dialogue line aborted subtitle pagination");
    dialog->SetStage(1);
    Require(text->GetText().find(texts[1]->szStr) != wstring::npos && mission->stageEvents == 2,
        "oversized dialogue line discarded later phrases");
    int page, phrase, offset, phase;
    dialog->GetDisplayProgressForDiagnostics(&page, &phrase, &offset, &phase);
    Require(page == 1 && phrase == 1 && offset == 0, "oversized dialogue fallback lost logical save progress");
    Require(back->GetStyle(NUI::STYLE_VISIBLE) && exit->GetStyle(NUI::STYLE_VISIBLE) && !next->GetStyle(NUI::STYLE_VISIBLE),
        "dialogue fallback did not preserve final-page navigation");
}


class CancellingMoveActor: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(CancellingMoveActor);
public:
    using CUnitServer::CUnitServer;
    mutable bool armed = false;
    mutable int steps = 0, callbacks = 0;
    mutable NAI::SPathPlace observed, cancelledAt;
    int cancelOnStep = 1;
    bool gracefulCancel = false;
    CPtr<NWorld::CCommandExecute> mover;
    CObj<NWorld::CCommandExecute>* releasingOwner = nullptr;
    bool CanFight() const override {
        if (armed) {
            const auto current = CUnitServer::GetPosition().pos.p;
            if (current.GetX() != observed.GetX() || current.GetY() != observed.GetY() || current.GetLayer() != observed.GetLayer()) {
                observed = current;
                if (++steps == cancelOnStep) {
                    armed = false; ++callbacks; cancelledAt = current;
                    auto* moving = dynamic_cast<NWorld::IExecMove*>(releasingOwner ? releasingOwner->GetPtr() : mover.GetPtr());
                    Require(moving != nullptr, "movement-callback fixture lost its mover");
                    if (gracefulCancel) (releasingOwner ? releasingOwner->GetPtr() : mover.GetPtr())->Cancel();
                    else moving->FullCancel();
                    if (releasingOwner) *releasingOwner = nullptr;
                }
            }
        }
        return CUnitServer::CanFight();
    }
};
REGISTER_SAVELOAD_CLASS(0x7f550106, CancellingMoveActor);

struct NativeMovementFixture
{
    CObj<NRPG::CGlobalGame> game;
    CObj<NWorld::CWorld> world;
    CObj<NRPG::IUnitMission> rpg;
    CObj<CancellingMoveActor> actor;
    CObj<NAI::CPath> path;
};

static NativeMovementFixture MakeNativeMovementFixture(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* source = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK()) { source = unit; break; }
    }
    Require(source != nullptr, "movement-callback fixture has no ordinary actor");
    auto* network = world->GetPathNetwork();
    vector<NAI::SPathPlace> places;
    network->GetNearPlaces(SSphere(source->GetPosition().GetCP(), 8.f), &places);
    auto position = source->GetPosition();
    bool found = false;
    for (auto place : places) {
        place.SetPose(NAI::CM_STAND); place.SetDirection(0);
        if (network->IsPassable(place) && !network->IsLocked(place, true)) {
            position.pos.p = place; found = true; break;
        }
    }
    Require(found, "movement-callback fixture has no free spawn tile");
    CObj<NRPG::IUnitMission> rpg = NRPG::CreateUnit(source->GetUnitRPG()->GetRPGUnit()->GetPers());
    CObj<CancellingMoveActor> actor = new CancellingMoveActor(world, rpg, rpg->GetModel(),
        dynamic_cast<NWorld::CPlayer*>(source->GetPlayer()), position);
    world->AddUnit(actor);
    actor->SetPosition(position);
    CObj<NAI::CPath> path;
    network->GetNearPlaces(SSphere(position.GetCP(), 5.f), &places);
    for (auto destination : places) {
        destination.SetPose(NAI::CM_STAND); destination.SetDirection(0);
        NAI::SPosition target(destination, network);
        if (fabs2(target.GetCP() - position.GetCP()) < 9.f) continue;
        path = NWorld::FindPath(network, actor, position.pos.p, vector<NAI::SPathPlace>{destination}, nullptr, true);
        if (path && path->points.size() >= 5) break;
        path = nullptr;
    }
    Require(path && path->points.size() >= 5, "movement-callback fixture needs a multi-step reachable path");
    return {game, world, rpg, actor, path};
}

static void MovementCallbackCancellation(const char* databasePath, const char* resourcePath, int cancelOnStep, bool releaseOwner = false)
{
    auto fixture = MakeNativeMovementFixture(databasePath, resourcePath);
    auto world = fixture.world;
    auto actor = fixture.actor;
    auto path = fixture.path;
    CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(actor, path, NAI::PF_DEFAULT);
    Require(move != nullptr, "movement-callback fixture could not create a real move executor");
    if (releaseOwner) actor->releasingOwner = &move;
    else actor->mover = move;
    actor->observed = actor->GetPosition().pos.p;
    actor->cancelOnStep = cancelOnStep;
    actor->armed = true;
    move->Run();
    for (int animation = 0; actor->callbacks == 0 && animation < 32 && move->GetState() == NWorld::CCommandExecute::RUNNING; ++animation)
        move->AnimationFinished();
    Require(actor->callbacks == 1 && actor->steps == cancelOnStep,
        "movement callback did not cancel at the requested physical step");
    Require((releaseOwner ? !move : move->GetState() != NWorld::CCommandExecute::RUNNING) &&
            actor->GetPosition().pos.p == actor->cancelledAt,
        "cancelled movement continued to a later queued tile or retained its released owner");
    actor->releasingOwner = nullptr;
    actor->mover = nullptr;
    move = nullptr;
    Require(!world->IsAction(), "cancelled movement left an action counter running");
}

struct MovementWaitWire
{
    int timer = -1;
    int operator&(CStructureSaver& saver) { saver.Add(4, &timer); return 0; }
};

static int MovementWaitTimer(NWorld::CCommandExecute* move)
{
    CStructureNetworkContext graph;
    graph.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(move)->operator&(writer); }
    saved.Seek(0);
    MovementWaitWire state;
    { CStructureSaver reader(saved, CStructureSaver::READ, &graph); reader.Add(1, &state); }
    return state.timer;
}

static void MovementFirstStepWaitTimer(const char* databasePath, const char* resourcePath)
{
    auto fixture = MakeNativeMovementFixture(databasePath, resourcePath);
    CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(fixture.actor, fixture.path, NAI::PF_DEFAULT);
    Require(IsValid(move), "first-step wait fixture could not create its native mover");
    MovementWaitWire previous; previous.timer = 123;
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE); writer.Add(1, &previous); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ);
      static_cast<CObjectBase*>(move.GetPtr())->operator&(reader); }
    Require(MovementWaitTimer(move) == 123, "first-step wait fixture did not restore its previous wait timer");
    const auto initial = fixture.actor->GetPosition().pos.p;
    auto stepped = [&]() {
        const auto current = fixture.actor->GetPosition().pos.p;
        return current.GetX() != initial.GetX() || current.GetY() != initial.GetY() || current.GetLayer() != initial.GetLayer();
    };
    move->Run();
    for (int frame = 0; frame < 32 && !stepped() && move->GetState() == NWorld::CCommandExecute::RUNNING; ++frame)
        move->AnimationFinished();
    const bool moved = stepped();
    const int timer = MovementWaitTimer(move);
    dynamic_cast<NWorld::IExecMove*>(move.GetPtr())->FullCancel();
    move = nullptr;
    Require(moved, "first-step wait fixture did not begin a physical native move");
    Require(timer == 0, "starting a new physical movement step retained the previous wait timer");
    Require(!fixture.world->IsAction(), "first-step wait fixture retained a cancelled movement action");
}

struct MovementCommandSnapshot
{
    list<CObj<CObjectBase>> queued;
    CObj<CObjectBase> current;
};

static MovementCommandSnapshot ReadMovementCommands(NWorld::CCommandExecute* move)
{
    vector<CObjectBase*> objects;
    CStructureNetworkContext gather;
    gather.include = [&](CObjectBase* object) {
        if (std::find(objects.begin(), objects.end(), object) == objects.end()) objects.push_back(object);
        return false;
    };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &gather);
      static_cast<CObjectBase*>(move)->operator&(writer); }
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.include = [](CObjectBase*) { return false; };
    unsigned id = 1;
    for (auto* object: objects) { references.ids[object] = id; references.refs[id++] = object; }
    references.nextID = id + 100;
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
      static_cast<CObjectBase*>(move)->operator&(writer); }
    saved.Seek(0);
    MovementCommandSnapshot snapshot;
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      reader.Add(2, &snapshot.queued); reader.Add(3, &snapshot.current); }
    return snapshot;
}

static void MovementTerminalCompletion(const char* databasePath, const char* resourcePath, bool cancelled)
{
    auto fixture = MakeNativeMovementFixture(databasePath, resourcePath);
    fixture.path->points.resize(2); // Retail completion latch is in the first-step/last-ender branch.
    CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(fixture.actor, fixture.path, NAI::PF_DEFAULT);
    Require(IsValid(move), "terminal-movement fixture could not create its native mover");
    if (cancelled) {
        fixture.actor->mover = move;
        fixture.actor->observed = fixture.actor->GetPosition().pos.p;
        fixture.actor->gracefulCancel = true;
        fixture.actor->cancelOnStep = 1;
        fixture.actor->armed = true;
    }
    move->Run();
    if (cancelled) Require(fixture.actor->callbacks == 1,
        "single-step terminal fixture did not cancel through the physical first-step callback");
    for (int frame = 0; frame < 64; ++frame) {
        auto snapshot = ReadMovementCommands(move);
        if (IsValid(snapshot.current) && snapshot.queued.empty()
                && pSSClasses->GetObjectTypeID(snapshot.current.GetPtr()) == int(0x12541120)) {
            const auto state = move->GetState();
            fixture.actor->mover = nullptr;
            move = nullptr;
            Require(state == (cancelled ? NWorld::CCommandExecute::FAILED : NWorld::CCommandExecute::FINISHED),
                "single-step end-move command did not latch the mover completion result");
            Require(!fixture.world->IsAction(), "released terminal movement retained its action counter");
            return;
        }
        Require(move->GetState() == NWorld::CCommandExecute::RUNNING,
            "terminal-movement fixture completed before exposing its final end command");
        move->AnimationFinished();
    }
    throw std::runtime_error("terminal-movement fixture never reached the last end command");
}

static void MovementActionCamera(const char* databasePath, const char* resourcePath)
{
    auto fixture = MakeNativeMovementFixture(databasePath, resourcePath);
    while (auto* pending = fixture.world->GetUICommand()) { CObj<NWorld::CUICmd> release = pending; }
    CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(fixture.actor, fixture.path, NAI::PF_DEFAULT);
    Require(IsValid(move), "movement camera fixture could not create its native mover");
    const auto initial = fixture.actor->GetPosition().pos.p;
    auto stepped = [&]() {
        const auto current = fixture.actor->GetPosition().pos.p;
        return current.GetX() != initial.GetX() || current.GetY() != initial.GetY() || current.GetLayer() != initial.GetLayer();
    };
    move->Run();
    for (int frame = 0; frame < 32 && !stepped() && move->GetState() == NWorld::CCommandExecute::RUNNING; ++frame)
        move->AnimationFinished();
    Require(stepped(), "movement camera fixture did not begin its physical move");
    auto snapshot = ReadMovementCommands(move);
    Require(IsValid(snapshot.current) && !snapshot.queued.empty()
            && pSSClasses->GetObjectTypeID(snapshot.current.GetPtr()) == int(0x02731142),
        "movement camera fixture did not prepare its next moving step");
    int cameras = 0;
    while (auto* pending = fixture.world->GetUICommand()) {
        CObj<NWorld::CUICmd> command = pending;
        auto* camera = dynamic_cast<NWorld::CUICmdUnitCamera*>(command.GetPtr());
        if (!camera || camera->pUnit != fixture.actor.GetPtr() || camera->GetPriority() != NWorld::PR_UNIT_ACTION) continue;
        ++cameras;
        Require(!camera->bUseSloMo && camera->fSloMoIncrProbability == 1.f && camera->pUnitTarget == nullptr,
            "movement camera notification changed its original framing parameters");
    }
    dynamic_cast<NWorld::IExecMove*>(move.GetPtr())->FullCancel(); move = nullptr;
    Require(cameras > 0, "native movement did not notify the automatic camera when preparing its next step");
    Require(!fixture.world->IsAction(), "movement camera fixture retained a cancelled action");
}

static void HideCooldown(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight()) { actor = unit; break; }
    }
    Require(actor != nullptr, "hide fixture has no human actor");
    CObj<NWorld::CCmdHide> command = new NWorld::CCmdHide;
    NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
    actor->EnableHide();
    CObj<NWorld::CCommandExecute> initial = NWorld::CreateExecutor(actor, command, &result);
    Require(initial && result == NWorld::UCR_OK, "fixture actor cannot hide before the cooldown");
    const int beforeAP = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    actor->Hide(false, false);
    CObj<NWorld::CCommandExecute> blocked = NWorld::CreateExecutor(actor, command, &result);
    Require(!blocked && result == NWorld::UCR_GENERAL_FAILURE,
        "Hide order was accepted during the existing hide cooldown");
    actor->EnableHide();
    CObj<NWorld::CCommandExecute> resumed = NWorld::CreateExecutor(actor, command, &result);
    Require(resumed && result == NWorld::UCR_OK, "Hide order did not resume after cooldown rearming");
    Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == beforeAP,
        "hide eligibility probes changed the unit's action points");
}

class AICollectionCallbackException: public std::runtime_error
{
public:
    AICollectionCallbackException(): std::runtime_error("expected AI collection callback exception") {}
};

class ThreatOwnerFixture: public NAI::AIUnitFixture
{
    OBJECT_NOCOPY_METHODS(ThreatOwnerFixture);
public:
    NAI::SAIUnitState threat;
    NAI::SAIState* shared = nullptr;
    bool exposeThreat = true;
    int sharedReads = 0;
    NAI::SAIState* GetAIState() override { ++sharedReads; return shared; }
    NAI::SAIUnitState* GetAIUnitState() override { return exposeThreat ? &threat : nullptr; }
};

class AIOwnerEventProbe: public NAI::IAIEvent
{
    OBJECT_NOCOPY_METHODS(AIOwnerEventProbe);
public:
    int calls = 0;
    NAI::SAIUnitState* received = nullptr;
    void Modify(NAI::SAIUnitState* state) override { ++calls; received = state; }
};

struct InlineAITrackerFields
{
    CPtr<NAI::CAIEventTrackerImpl> impl;
    int operator&(CStructureSaver& saver) { saver.Add(2, &impl); return 0; }
};

static CPtr<NAI::CAIEventTrackerImpl> ReadNativeAITracker(NAI::IAIUnit* owner)
{
    CStructureNetworkContext references;
    auto saved = BorrowedObjectFields(owner, &references);
    InlineAITrackerFields tracker;
    { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(2, &tracker); }
    Require(IsValid(tracker.impl), "native AI wrapper did not retain its inline event tracker");
    return tracker.impl;
}

static void AIEventOwnerBinding(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NAI::IAIUnit* owner = nullptr;
    for (const auto& unit: units) {
        auto* wrapper = NAI::GetAIUnit(unit);
        if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && IsValid(wrapper)) {
            actor = unit.GetPtr(); owner = wrapper; break;
        }
    }
    Require(actor && owner && owner->GetAIUnitState(), "event-owner fixture has no native AI actor");
    CObj<NAI::CAIEventTrackerImpl> restored;
    CObj<NAI::CAIEventTracker> legacyAdapter;
    CPtr<NAI::CAIEventTrackerImpl> impl = ReadNativeAITracker(owner);
    CObj<ThreatOwnerFixture> boundOwner;
    if (mode == 0) {
        // The serialized owner must win over a fresh global lookup for the same
        // server, including retained callbacks across wrapper replacement.
        boundOwner = new ThreatOwnerFixture;
        boundOwner->server = actor;
        boundOwner->threat.SetUnit(boundOwner);
        owner = boundOwner.GetPtr();
    }

    if (mode != 1) {
        restored = new NAI::CAIEventTrackerImpl;
        CStructureNetworkContext references;
        references.externalReferences = true;
        references.ids[actor] = 1; references.refs[1] = actor;
        references.ids[owner] = 2; references.refs[2] = owner;
        references.ids[restored.GetPtr()] = 3; references.refs[3] = restored.GetPtr();
        references.nextID = 100; references.include = [](CObjectBase*) { return false; };
        CPtr<NWorld::CUnitServer> server = actor;
        CPtr<NAI::IAIUnit> ownerRef = owner;
        if (mode == 2) {
            legacyAdapter = new NAI::CAIEventTracker(actor);
            references.ids[legacyAdapter.GetPtr()] = 4; references.refs[4] = legacyAdapter.GetPtr();
        }
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
          writer.Add(2, &server);
          if (mode == 2) writer.Add(3, &legacyAdapter); else writer.Add(3, &ownerRef); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); restored->operator&(reader); }
        Require(restored->GetUnit() == actor, "restored AI event tracker lost its server reference");
        impl = restored.GetPtr();
        if (mode == 2) {
            InlineAITrackerFields tracker; tracker.impl = restored.GetPtr();
            CMemoryStream legacyUnit;
            { CStructureSaver writer(legacyUnit, CStructureSaver::WRITE, &references); writer.Add(2, &tracker); }
            legacyUnit.Seek(0);
            { CStructureSaver reader(legacyUnit, CStructureSaver::READ, &references);
              static_cast<CObjectBase*>(owner)->operator&(reader); }
            impl = ReadNativeAITracker(owner);
        }
    }
    if (mode == 1) {
        CStructureNetworkContext references;
        auto saved = BorrowedObjectFields(impl, &references);
        CPtr<NAI::IAIUnit> serializedOwner;
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(3, &serializedOwner); }
        Require(serializedOwner.GetPtr() == owner,
            "fresh AI event tracker serialized its adapter instead of the owning AI unit identity");
    }
    const int ap = actor->GetRPG()->GetAP();
    CObj<AIOwnerEventProbe> event = new AIOwnerEventProbe;
    impl->ThrowAIEvent(event);
    Require(event->calls == 1 && event->received == owner->GetAIUnitState(),
        "restored owner-identity AI event tracker did not deliver to its bound unit threat state");
    impl->ThrowAIEvent(nullptr);
    Require(event->calls == 1 && actor->GetRPG()->GetAP() == ap,
        "AI event owner binding duplicated delivery or changed live action points");
    Require(event->received->enemies.nLock == 0 && event->received->possibleEnemies.nLock == 0
            && event->received->allies.nLock == 0, "AI owner dispatch retained contact locks after returning");
    if (mode == 0) {
        boundOwner->Invalidate();
        impl->ThrowAIEvent(event);
        Require(event->calls == 1, "invalid bound AI owner was silently replaced by a global lookup");
    }

}

class AICollectionEventProbe: public NAI::IAIEvent
{
    OBJECT_NOCOPY_METHODS(AICollectionEventProbe);
public:
    CPtr<NAI::CAIEventTracker> tracker;
    CObj<NAI::IAIEvent> nested;
    int expectedDepth = 1, calls = 0;
    bool throwAfterUpdate = false;
    void Modify(NAI::SAIUnitState* state) override {
        ++calls;
        Require(state->enemies.nLock == expectedDepth && state->possibleEnemies.nLock == expectedDepth
                && state->allies.nLock == expectedDepth,
            "AI event callback ran without holding all three contact collection locks");
        state->enemies.bModified = state->possibleEnemies.bModified = state->allies.bModified = true;
        state->Update();
        Require(state->enemies.bModified && state->possibleEnemies.bModified && state->allies.bModified,
            "AI event callback consumed contact dirty flags before notification completed");
        if (IsValid(nested)) tracker->Notify(nested);
        Require(state->enemies.nLock == expectedDepth && state->possibleEnemies.nLock == expectedDepth
                && state->allies.nLock == expectedDepth,
            "nested AI notification corrupted its caller's collection lock depth");
        if (throwAfterUpdate) throw AICollectionCallbackException();
    }
};

static void AIEventCollectionLocks(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NAI::SAIUnitState* state = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight()) continue;
        auto* ai = NAI::GetAIUnit(unit);
        if (ai && ai->IsUnderAIControl() && ai->GetAIUnitState()) {
            actor = unit; state = ai->GetAIUnitState(); break;
        }
    }
    Require(actor && state, "event-lock fixture has no live AI threat state");
    CObj<NAI::CAIEventTracker> tracker = new NAI::CAIEventTracker(actor);
    CObj<AICollectionEventProbe> outer = new AICollectionEventProbe;
    CObj<AICollectionEventProbe> inner = new AICollectionEventProbe;
    outer->tracker = inner->tracker = tracker.GetPtr();
    inner->expectedDepth = 2;
    outer->nested = inner.GetPtr();
    tracker->Notify(outer);
    Require(outer->calls == 1 && inner->calls == 1, "nested AI notification was lost or duplicated");
    Require(state->enemies.nLock == 0 && state->possibleEnemies.nLock == 0 && state->allies.nLock == 0,
        "completed AI notification retained contact collection locks");
    state->Update();
    Require(!state->enemies.bModified && !state->possibleEnemies.bModified && !state->allies.bModified,
        "contact collection dirty flags were not consumed after leaving notification");
    outer->nested = nullptr;
    outer->throwAfterUpdate = true;
    bool caught = false;
    try { tracker->Notify(outer); } catch (const AICollectionCallbackException&) { caught = true; }
    Require(caught && state->enemies.nLock == 0 && state->possibleEnemies.nLock == 0 && state->allies.nLock == 0,
        "throwing AI callback leaked contact collection locks");
}

static void AIReactionResetContacts()
{
    CObj<NAI::AIUnitFixture> contact = new NAI::AIUnitFixture;
    NAI::SAIUnitState state;
    state.enemies.data.units.push_back(contact.GetPtr());
    state.possibleEnemies.data.units.push_back(contact.GetPtr());
    state.allies.data.units.push_back(contact.GetPtr());
    state.knownCorpses.push_back(contact.GetPtr());
    state.pEnemy = state.pPossibleEnemy = state.pAlly = contact.GetPtr();
    state.bHelpCalled = state.bScared = true;
    CObj<NAI::IAIEvent> event = NAI::CreateAIUpdateEvent();
    event->Modify(&state);
    Require(state.IsModified(), "AI update event did not arm a reaction before reset");
    state.Reset();
    Require(state.enemies.data.units.size() == 1 && state.possibleEnemies.data.units.size() == 1
            && state.allies.data.units.size() == 1 && state.pEnemy == contact.GetPtr()
            && state.pPossibleEnemy == contact.GetPtr() && state.pAlly == contact.GetPtr()
            && state.knownCorpses.size() == 1 && state.bHelpCalled && state.bScared,
        "consuming an AI reaction notification erased contacts or morale state");
    Require(!state.IsModified(), "AI notification reset failed to consume its boolean value");
}

static void AIReactionValueLatch(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NAI::IAIUnit* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight()) continue;
        auto* ai = NAI::GetAIUnit(unit);
        if (ai && ai->IsUnderAIControl() && IsValid(ai->GetReaction())) { actor = ai; break; }
    }
    Require(actor != nullptr, "reaction fixture has no AI-controlled actor with a reaction");
    auto* state = actor->GetAIUnitState();
    auto* reaction = actor->GetReaction();
    auto restoreLatch = [&](bool value, bool dirty, int locks) {
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE);
          writer.Add(2, &locks); writer.Add(3, &dirty); writer.Add(4, &value); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ); state->selfModified.operator&(reader); }
    };
    restoreLatch(true, false, 1);
    Require(actor->GetReactionForUpdate() == reaction,
        "AI reaction pump ignored a restored notification value when its tracking flag was clear");
    Require(actor->GetReactionForUpdate() == nullptr && !state->selfModified.bModified,
        "AI reaction pump repeated a consumed value or dirtied a locked tracking flag");
    CObj<NAI::IAIEvent> event = NAI::CreateAIUpdateEvent();
    event->Modify(state);
    Require(actor->GetReactionForUpdate() == reaction && actor->GetReactionForUpdate() == nullptr,
        "AI update event did not deliver exactly one notification under a tracking lock");
    restoreLatch(false, true, 0);
    Require(actor->GetReactionForUpdate() == nullptr,
        "AI reaction pump treated a dirty tracking flag as a new notification");
    actor->SetReaction(reaction);
    Require(actor->GetReactionForUpdate() == reaction && actor->GetReactionForUpdate() == nullptr,
        "installing an AI reaction did not arm and consume exactly one notification");
}

static void IdleAI(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NAI::CAICommander* commander = nullptr;
    for (auto& entry : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight()) continue;
        unit->GetUnitRPG()->GetRPGUnit()->SetCheat(NRPG::CHEAT_NOAI, true);
        if (!actor) {
            commander = dynamic_cast<NAI::CAICommander*>(unit->GetPlayer()->GetCommander());
            if (commander) actor = unit;
        }
    }
    Require(actor && commander, "mission fixture contains no AI actor");
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    Require(!world->IsRealTime() && !world->IsSequence() && !world->IsAction(), "AI fixture is not idle in its turn");
    actor->GetUnitRPG()->GetRPGUnit()->SetCheat(NRPG::CHEAT_NOAI, false);
    auto* ai = commander->GetAIUnit(actor);
    Require(ai != nullptr, "AI actor wrapper missing");
    auto original = actor->GetPosition();
    auto inactive = original;
    inactive.pos.p.SetPose(NAI::CM_INACTIVE);
    for (int routine = 0; routine < 3; ++routine) {
        CObj<NAI::CAIRouteLogic> route = new NAI::CAIRouteLogic(ai);
        route->SetCircled(true);
        if (routine == 1) route->AddCommand(new NAI::CTaskCommandChangePose(NAI::WALK));
        if (routine == 2) route->AddCommand(new NAI::CTaskCommandGoto(original.pos));
        route->AddCommand(new NAI::CTaskCommandWait(100));
        route->AddCommand(new NAI::CTaskCommandChangeDirection(0));
        ai->SetLogic(route);
        actor->SetTemporaryPosition(original);
        Require(!commander->IsEndOfTurn(), "normal AI route was incorrectly skipped");
        actor->SetTemporaryPosition(inactive);
        Require(commander->IsEndOfTurn() == (routine != 2),
            routine == 2 ? "a route with movement was incorrectly skipped" : "inactive stationary AI route blocks the end of turn");
    }
    actor->SetTemporaryPosition(original);
}

class WeaponIconMission: public NGame::CMissionBase
{
    OBJECT_BASIC_METHODS(WeaponIconMission);
public:
    vector<CPtr<NGame::IUnitTracker>> selected;
    CObj<NGame::CStateWait> state = new NGame::CStateWait;
    NGame::EActionIconsSet icons = NGame::AIS_MAIN;
    bool attackAvailable = true;
    bool mixedActionFixture = false;
    bool togglesAvailable = true;
    void Step() override {}
    bool ProcessEvent(const NInput::SEvent&) override { return false; }
    void GetSelectedUnits(vector<CPtr<NGame::IUnitTracker>>* units) const override { *units = selected; }
    NGame::IState* GetState() const override { return state; }
    int GetUnitsState() override { return mixedActionFixture ? NGame::N_UNITSTATE_DEFAULT : NGame::N_UNITSTATE_CANNON; }
    NWorld::CUnit::EState GetUnitsWorldState() override { return mixedActionFixture ? NWorld::CUnit::ST_NORMAL_MELEE : NWorld::CUnit::ST_MACHINE_GUN; }
    NGame::EActionIconsSet GetActionIconsSet() const override { return icons; }
    void SetActionIconsSet(NGame::EActionIconsSet value) override { icons = value; }
    void GetActionInfo(NGame::EUnitAction action, NGame::SActionInfo* info) override {
        *info = NGame::SActionInfo();
        info->bValid = info->bOk = true;
        info->bAvailable = mixedActionFixture
            ? togglesAvailable && (action == NGame::UA_HIDE || action == NGame::UA_STRAFE)
            : action != NGame::UA_ATTACK || attackAvailable;
        info->eResult = NWorld::UCR_OK;
    }
};

class IconBarProbe: public NUI::CUnitIconsBar
{
    vector<CMObj<NUI::CWindow>> detachedControls;
public:
    using CUnitIconsBar::CUnitIconsBar;
    void DetachRendering() {
        // Transfer the main owning references; RemoveChild would invalidate
        // a control even if the fixture retained an ordinary CObj reference.
        detachedControls.swap(listChildren);
        for (auto& control : detachedControls) control->SetParent(nullptr);
    }
};

class IconBarFixture
{
public:
    CObj<IconBarProbe> bar;
    CObj<NDb::CUIContainer> layout;
    vector<CObj<NDb::CUIControl>> controls;
    vector<CObj<NUI::CComplexButton>> buttons;
    explicit IconBarFixture(NGame::IMission* mission) {
        bar = new IconBarProbe(
            NUI::SWindowInfo(nullptr, NUI::SPoint(0, 0), NUI::SPoint(400, 80), "mounted-mode-test"), mission);
        layout = new NDb::CUIContainer();
        layout->nWidth = 400; layout->nHeight = 80;
        for (int i = 0; i < 8; ++i) {
            CObj<NDb::CUIControl> control = new NDb::CUIControl();
            control->szID = "icon_" + std::to_string(i + 1);
            control->type = NDb::UI_WINDOW;
            control->nDepth = i; control->bVisible = true;
            control->rect = CTRect<int>(i % 4 * 100, i / 4 * 40, i % 4 * 100 + 100, i / 4 * 40 + 40);
            controls.push_back(control); layout->controls.push_back(control.GetPtr());
        }
        NUI::LoadTemplate(bar, layout);
        for (int i = 0; i < 8; ++i) {
            auto* button = dynamic_cast<NUI::CComplexButton*>(bar->GetChildByID("icon_" + std::to_string(i + 1)));
            Require(button != nullptr, "fixture icon template did not build all action buttons");
            buttons.emplace_back(button);
        }
        bar->DetachRendering();
        CMemoryStream state;
        int worldState = mission->GetUnitsWorldState();
        { CStructureSaver writer(state, CStructureSaver::WRITE); writer.Add(6, &worldState); }
        state.Seek(0);
        { CStructureSaver reader(state, CStructureSaver::READ);
          static_cast<CObjectBase*>(bar.GetPtr())->operator&(reader); }
    }
};

static void MountedModeButton(WeaponIconMission* mission)
{
    IconBarFixture fixture(mission);
    auto& bar = fixture.bar;
    auto& buttons = fixture.buttons;
    bar->Draw(0, nullptr);
    Require(buttons[4]->GetStyle(NUI::STYLE_VISIBLE) && buttons[4]->GetStyle(NUI::STYLE_ENABLED),
        "mounted-gun action bar omitted the fire-mode submenu button");
    Require(buttons[0]->GetStyle(NUI::STYLE_VISIBLE) && buttons[7]->GetStyle(NUI::STYLE_VISIBLE),
        "mounted-gun mode button replaced attack or stop");
    mission->attackAvailable = false;
    bar->Draw(0, nullptr);
    Require(!buttons[4]->GetStyle(NUI::STYLE_VISIBLE), "unavailable mounted attack still exposed fire modes");
    mission->attackAvailable = true;
    bar->Draw(0, nullptr);
    Require(buttons[4]->GetStyle(NUI::STYLE_VISIBLE), "mounted fire-mode button did not return with attack eligibility");
    mission->icons = NGame::AIS_WEAPONMODES;
    bar->Draw(0, nullptr);
    Require(buttons[0]->GetStyle(NUI::STYLE_VISIBLE) && !buttons[0]->IsChecked() &&
            buttons[1]->GetStyle(NUI::STYLE_VISIBLE) && buttons[1]->IsChecked(),
        "actual mounted-gun mode controls did not check the mounted aimed mode");
    mission->icons = NGame::AIS_MAIN;
}


static void MixedActionButton(const char* databasePath, const char* resourcePath, bool hide)
{
    auto world = FixtureWorld(databasePath, resourcePath, true);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> actors;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) actors.push_back(unit);
        if (actors.size() == 2) break;
    }
    Require(actors.size() == 2, "mixed-button fixture needs two ordinary actors");
    CObj<WeaponIconMission> mission = new WeaponIconMission;
    mission->mixedActionFixture = true;
    vector<CObj<NGame::CUnitTracker>> trackers;
    for (auto* actor : actors) {
        trackers.emplace_back(new NGame::CUnitTracker(mission, actor));
        mission->selected.push_back(trackers.back().GetPtr());
    }
    IconBarFixture fixture(mission);
    auto& button = fixture.buttons[hide ? 7 : 6];
    auto setGroup = [&](bool first, bool second) {
        for (int i = 0; i < 2; ++i)
            if (hide) actors[i]->GetUnitRPG()->SetHiding(i == 0 ? first : second);
            else actors[i]->SetStrafe(i == 0 ? first : second);
        fixture.bar->Draw(0, nullptr);
    };
    setGroup(true, false);
    Require(button->GetStyle(NUI::STYLE_VISIBLE) && button->GetStyle(NUI::STYLE_ENABLED) && !button->IsChecked(),
        hide ? "Hide button disappeared for a selection with mixed hiding states"
             : "Strafe button disappeared for a selection with mixed strafe states");
    setGroup(false, true);
    Require(button->GetStyle(NUI::STYLE_VISIBLE) && !button->IsChecked(), "mixed toggle depends on selection order");
    setGroup(true, true);
    Require(button->GetStyle(NUI::STYLE_VISIBLE) && button->IsChecked(), "all-active selection did not check its toggle");
    setGroup(false, false);
    Require(button->GetStyle(NUI::STYLE_VISIBLE) && !button->IsChecked(), "all-inactive selection did not uncheck its toggle");
    mission->togglesAvailable = false;
    fixture.bar->Draw(0, nullptr);
    Require(!button->GetStyle(NUI::STYLE_VISIBLE), "unavailable group toggle remained visible");
    mission->togglesAvailable = true;
    setGroup(true, false);
    Require(button->GetStyle(NUI::STYLE_VISIBLE), "mixed toggle did not return after becoming available");
}

static NWorld::CCommandExecute* FindNativeReload(NWorld::CCommandExecute* executor)
{
    if (pSSClasses->GetObjectTypeID(executor) == int(0x00122170)) return executor;
    CPtr<NWorld::CCommandExecute> reload;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* object) {
        if (pSSClasses->GetObjectTypeID(object) == int(0x00122170)) {
            reload = dynamic_cast<NWorld::CCommandExecute*>(object); return false;
        }
        return dynamic_cast<NWorld::CCommandExecute*>(object) != nullptr;
    };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(executor)->operator&(writer); }
    Require(IsValid(reload), "native executor queue did not contain its reload action");
    return reload;
}

static void MountedReloadWeapon(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    CPtr<NRPG::CWeaponItem> personal;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!server || !server->CanFight() || server->IsEmptyPK() || server->IsWearingPK()) continue;
        auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(server->GetUnitRPG()->GetInventory()->GetActive());
        if (!firearm || firearm->GetAmmoQuantity() <= 0) continue;
        firearm->DiscardAmmo();
        if (firearm->CanReload(server->GetUnitRPG()->GetInventory())) { actor = server; personal = firearm; break; }
    }
    Require(actor && IsValid(personal), "mounted-reload fixture has no firearm with spare ammunition");
    CObj<NRPG::CWeaponItem> mounted = new NRPG::CWeaponItem(personal->GetDBWeapon());
    mounted->DiscardAmmo();
    Require(mounted->CanReload(actor->GetUnitRPG()->GetInventory()), "mounted-reload fixture has no compatible spare for its cannon");
    actor->GetUnitRPG()->SetCannonItem(mounted);
    Require(actor->GetUnitRPG()->GetWeaponItem() == mounted.GetPtr(), "mounted-reload fixture did not install its effective weapon");
    const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    auto selectedWeapon = [&](int slot) {
        CObj<NWorld::CCmdReload> command = new NWorld::CCmdReload(slot);
        NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
        CObj<NWorld::CCommandExecute> queue = NWorld::CreateExecutor(actor, command, &result);
        Require(IsValid(queue) && result == NWorld::UCR_OK, "mounted-reload fixture could not create a reload executor");
        auto* reload = FindNativeReload(queue);
        CStructureNetworkContext references;
        references.externalReferences = true;
        references.ids[actor] = 1; references.refs[1] = actor;
        references.ids[personal.GetPtr()] = 2; references.refs[2] = personal.GetPtr();
        references.ids[mounted.GetPtr()] = 3; references.refs[3] = mounted.GetPtr();
        references.nextID = 100; references.include = [](CObjectBase*) { return false; };
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
          static_cast<CObjectBase*>(reload)->operator&(writer); }
        saved.Seek(0);
        struct LoadState {
            CPtr<NRPG::IWeaponItemInfo> weapon;
            int operator&(CStructureSaver& saver) { saver.Add(3, &weapon); return 0; }
        } load;
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(1, &load); }
        return load.weapon;
    };
    Require(selectedWeapon(-1) == mounted.GetPtr(), "default reload selected the personal firearm instead of the effective mounted weapon");
    Require(selectedWeapon(actor->GetUnitRPG()->GetInventory()->GetActiveSlot()) == personal.GetPtr(),
        "explicit inventory-slot reload stopped selecting the requested personal item");
    actor->GetUnitRPG()->SetCannonItem(nullptr);
    Require(selectedWeapon(-1) == personal.GetPtr(), "leaving the cannon did not restore personal weapon reload selection");
    Require(personal->GetAmmoQuantity() == 0 && mounted->GetAmmoQuantity() == 0
            && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "constructing reload executors spent ammunition or action points");
}

static void ReloadActionWire(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    CPtr<NRPG::CWeaponItem> weapon;
    for (auto& entry : units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!server || !server->CanFight() || server->IsEmptyPK() || server->IsWearingPK()) continue;
        auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(server->GetUnitRPG()->GetInventory()->GetActive());
        if (!firearm || firearm->GetAmmoQuantity() <= 0) continue;
        firearm->DiscardAmmo();
        if (firearm->CanReload(server->GetUnitRPG()->GetInventory())
                && server->CanSpendAP(server->GetActionAP(NRPG::AC_RELOAD))) { actor = server; weapon = firearm; break; }
    }
    Require(actor && IsValid(weapon), "reload-wire fixture needs a firearm user with spare ammunition and AP");
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[actor] = 1; references.refs[1] = actor;
    references.ids[weapon.GetPtr()] = 2; references.refs[2] = weapon.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    auto registerItem = [&](NRPG::IInventoryItem* item) {
        if (item && !references.ids.count(item)) {
            const auto id = references.nextID++;
            references.ids[item] = id; references.refs[id] = item;
        }
    };
    auto* sourceInventory = actor->GetUnitRPG()->GetInventory();
    for (int i = 0; i < NDb::N_SLOTS; ++i) registerItem(sourceInventory->Get(NDb::ESlot(i)));
    for (const auto& item : sourceInventory->GetItems()) registerItem(item.pItem);
    auto writeReload = [&](int slot) {
        CObj<NWorld::CExecReload> action = new NWorld::CExecReload(actor, slot);
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
          static_cast<CObjectBase*>(action.GetPtr())->operator&(writer); }
        saved.Seek(0);
        return saved;
    };
    auto saved = writeReload(-1);
    int slot = -7;
    { CStructureSaver reader(saved, CStructureSaver::READ, &references); reader.Add(2, &slot); }
    Require(slot == -1, "reload action wrote a weapon reference in the retail slot-number field");
    struct CommandState {
        CPtr<NWorld::CUnitServer> actor;
        int operator&(CStructureSaver& saver) { saver.Add(4, &actor); return 0; }
    };
    struct LoadState {
        CommandState command;
        NWorld::SItem clip;
        CPtr<NRPG::IWeaponItemInfo> weapon;
        int operator&(CStructureSaver& saver) {
            saver.Add(1, &command); saver.Add(2, &clip); saver.Add(3, &weapon); return 0;
        }
    } load;
    bool sameColor = false;
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      reader.Add(1, &load); reader.Add(3, &sameColor); }
    Require(load.command.actor == actor && load.weapon == weapon.GetPtr(),
        "reload action omitted its nested load-weapon/command ownership state");
    auto* spare = dynamic_cast<NRPG::CClipItem*>(load.clip.pItem.GetPtr());
    auto* inner = dynamic_cast<NRPG::CClipItem*>(weapon->GetInnerClip());
    Require(load.clip.pUnit == actor, "reload action lost its magazine's unit owner");
    Require(spare && inner, "reload-wire fixture could not resolve its external magazine references");
    Require(load.clip.sPosition == CTPoint<int>(0, 0)
            && (load.clip.eType == NWorld::SItem::SLOT || load.clip.eType == NWorld::SItem::BACKPACK)
            && inner->IsCompatible(spare, sameColor),
        "reload action did not save its selected native spare magazine and inventory source");
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    if (load.clip.eType == NWorld::SItem::SLOT)
        Require(inventory->Get(NDb::ESlot(load.clip.nSlot)) == spare, "reload saved the wrong spare slot");
    else {
        const auto& items = inventory->GetItems();
        Require(std::any_of(items.begin(), items.end(), [&](const auto& item) { return item.pItem == spare; }),
            "reload saved a spare that is absent from its backpack");
    }
    auto explicitSaved = writeReload(inventory->GetActiveSlot());
    { CStructureSaver reader(explicitSaved, CStructureSaver::READ, &references); reader.Add(2, &slot); }
    Require(slot == inventory->GetActiveSlot(), "explicit reload lost its slot number on save");
    const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    const int rounds = spare->GetQuantity();
    CObj<NWorld::CExecReload> restored = new NWorld::CExecReload;
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
    Require(restored->CanDoIt() == NWorld::UCR_OK && weapon->GetAmmoQuantity() == 0
            && spare->GetQuantity() == rounds && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "restoring reload mutated ammunition/AP or lost execution eligibility");
    const int cost = actor->GetActionAP(NRPG::AC_RELOAD);
    Require(restored->GetStartAP() == cost && restored->GetActionAP() == cost,
        "restoring reload changed the native action cost");
    restored->Run(); restored->TimeLabelReached(); restored->AnimationFinished();
    Require(weapon->GetAmmoQuantity() > 0 && restored->GetState() == NWorld::CCommandExecute::FINISHED,
        "saved reload could not finish its actual ammunition-loading action");
    restored = nullptr;
    Require(!world->IsAction(), "restored reload retained an action counter after completion");
}

class SharedShortcutMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(SharedShortcutMission);
public:
    CObj<NGame::IState> selectedState;
    void Prepare() {
        pCursor = NUI::ICursor::Create(false);
        pInterface = new NUI::CInterface(pCursor);
        desktopWindowsList.push_back(new NUI::CDesktopWindow(NUI::SWindowInfo(
            pInterface,NUI::SPoint(0,0),NUI::SPoint(100,100),"shortcut-probe")));
    }
    bool IsReady() const override { return false; }
    bool IsSequence() const override { return true; }
    bool IsPlayerTurn() const override { return true; }
    bool IsRealTime() const override { return true; }
    bool CommandState(NGame::IState* state) override { selectedState = state; return true; }
    void CacheActions(const vector<NGame::SActionInfo>& actions) {
        auto copy = actions;
        CMemoryStream fields;
        { CStructureSaver writer(fields,CStructureSaver::WRITE); writer.Add(16,&copy); }
        fields.Seek(0);
        { CStructureSaver reader(fields,CStructureSaver::READ); CMission::operator&(reader); }
    }
};

static NInput::SEvent SharedActionEvent(std::initializer_list<const char*> commands)
{
    NInput::SEvent event, part;
    while (NInput::GetEvent(&part)) {}
    for (const char* command: commands) {
        NInput::PostEvent(command);
        Require(NInput::GetEvent(&part) && !part.commands.empty(),
            "shared-key fixture could not obtain its native binding command");
        event.mMessage = part.mMessage;
        event.commands.insert(event.commands.end(),part.commands.begin(),part.commands.end());
    }
    return event;
}

static void UnavailableAttackShortcut()
{
    CObj<SharedShortcutMission> mission = new SharedShortcutMission;
    mission->Prepare();
    vector<NGame::SActionInfo> actions(NGame::UA_MAXVALUE);
    actions[NGame::UA_ATTACK].bValid = true;
    actions[NGame::UA_ATTACK].bAvailable = true;
    // Mode entry remains available even when a target or AP is still needed.
    Require(!actions[NGame::UA_ATTACK].bOk, "shortcut control unexpectedly permits immediate execution");
    mission->CacheActions(actions);
    auto event = SharedActionEvent({"attack","setmine"});
    mission->ProcessEvent(event);
    Require(dynamic_cast<NGame::CStateAttack*>(mission->selectedState.GetPtr()),
        "an available attack with no immediate execution target lost its shared-key priority");
    actions[NGame::UA_ATTACK].bAvailable = false;
    mission->CacheActions(actions);
    mission->selectedState = nullptr;
    mission->ProcessEvent(event);
    Require(dynamic_cast<NGame::CStateSetMine*>(mission->selectedState.GetPtr()),
        "an unavailable attack swallowed the shared shortcut instead of falling through to set-mine");
    Require(!NMainLoop::HaveInterfaceCommand(), "selecting an action mode unexpectedly queued a game command");
}

static void SharedActionShortcutPriority()
{
    CObj<SharedShortcutMission> mission = new SharedShortcutMission;
    mission->Prepare();
    vector<NGame::SActionInfo> actions(NGame::UA_MAXVALUE);
    for (auto& action: actions) action.bValid = true;
    const NGame::EUnitAction priority[] = {NGame::UA_ATTACK,NGame::UA_HEAL,NGame::UA_USETOOL,
        NGame::UA_DROPCORPSE,NGame::UA_MINE};
    for (auto action: priority) actions[action].bAvailable = true;
    auto event = SharedActionEvent({"attack","firstaid","usetool","dropcorpse","setmine"});
    auto send = [&]() {
        mission->CacheActions(actions);
        mission->selectedState = nullptr;
        mission->ProcessEvent(event);
    };
    send();
    Require(dynamic_cast<NGame::CStateAttack*>(mission->selectedState.GetPtr()),
        "shared action key lost its available attack priority");
    actions[NGame::UA_ATTACK].bAvailable = false;
    send();
    Require(dynamic_cast<NGame::CStateFirstAid*>(mission->selectedState.GetPtr()),
        "shared action key selected set-mine ahead of available first aid");
    actions[NGame::UA_HEAL].bAvailable = false;
    send();
    Require(dynamic_cast<NGame::CStateUntrap*>(mission->selectedState.GetPtr()) &&
        mission->selectedState->GetType() == NGame::IState::FORCED,
        "shared action key failed to choose the available forced use-tool mode");
    actions[NGame::UA_USETOOL].bAvailable = false;
    send();
    Require(dynamic_cast<NGame::CStateDropCorpse*>(mission->selectedState.GetPtr()),
        "shared action key failed to choose available drop-corpse after unavailable aid and tools");
    actions[NGame::UA_DROPCORPSE].bAvailable = false;
    send();
    Require(dynamic_cast<NGame::CStateSetMine*>(mission->selectedState.GetPtr()),
        "shared action key failed to reach the available set-mine fallback");
    for (const char* command: {"firstaid","usetool","dropcorpse"}) {
        mission->selectedState = nullptr;
        mission->ProcessEvent(SharedActionEvent({command}));
        Require(!IsValid(mission->selectedState),
            "an individually unavailable aid, tool or body shortcut still selected its mode");
    }
    Require(!NMainLoop::HaveInterfaceCommand(), "shared-key mode selection queued a game command");
}

class NativeActionInfoMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(NativeActionInfoMission);
public:
    vector<CPtr<NGame::IUnitTracker>> selection;
    void Attach(NWorld::CWorld* world,NRPG::CGlobalGame* game) { pWorld = world; pGlobalGame = game; }
    void RefreshActions() { UpdateActionsInfo(); }
    void GetSelectedUnits(vector<CPtr<NGame::IUnitTracker>>* result) const override { *result = selection; }
};

class SelectedAttackTargetMission: public NativeActionInfoMission
{
    OBJECT_NOCOPY_METHODS(SelectedAttackTargetMission);
public:
    CPtr<CObjectBase> target;
    CObjectBase* GetStateTarget(bool* fromWorld = nullptr) const override {
        if (fromWorld) *fromWorld = true;
        return target;
    }
    bool GetTracePosition(CVec3* position) const override { *position = CVec3(1,2,3); return true; }
};

class MeleeReachCaptionMission: public SelectedAttackTargetMission
{
    OBJECT_NOCOPY_METHODS(MeleeReachCaptionMission);
public:
    int CountSelected() override { return int(selection.size()); }
    bool IsRealTime() const override { return false; }
};

static void MeleeReachCaption(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) { actor = unit; break; }
    }
    Require(actor != nullptr, "melee reach fixture has no ordinary native actor");
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    auto slot = NDb::ESlot(inventory->GetActiveSlot());
    CObj<NRPG::IInventoryItem> held = inventory->Get(slot) ? inventory->TakeOff(slot) : nullptr;
    Require(NRPG::GetToHitType(actor) == NRPG::TH_MELEE, "melee reach fixture did not select its native empty-hand attack");
    NWorld::CUnit* target = nullptr;
    CVec3 center;
    for (auto& entry: units) {
        if (entry.GetPtr() == actor || !entry->CanFight() || entry->IsEmptyPK()) continue;
        world->GetAIMap()->GetUnitHLPos(&center,world->GetAIMap()->GetHull(entry),NAI::HL_ANY);
        if (!NWorld::CanMeleeAttack(actor,actor->GetPosition(),center)) { target = entry; break; }
    }
    Require(target != nullptr, "melee reach fixture has no target outside the current-pose reach");
    const int sharedChance = world->GetGame()->GetCompositeToHit(actor,target,NAI::HL_ANY,world->IsFirstTurn());
    Require(sharedChance >= 0, "melee reach fixture needs the unchanged shared numeric-chance contract");
    CObj<MeleeReachCaptionMission> mission = new MeleeReachCaptionMission;
    mission->Attach(world,game); mission->target = target;
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission,actor);
    mission->selection.push_back(tracker.GetPtr());
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    NGame::SActionInfo info;
    info.SetValid(); info.nMinAP = info.nMaxAP = 7;
    SetAttackPreview(state,info);
    const int ap = actor->GetAP();
    state->RefreshCaption();
    const auto expected = NUI::GetDBString(20276) + NUI::GetDBString(19808) + L"7";
    Require(state->GetCursorInfo().wsText == expected,
        "a melee target outside current-pose reach still displayed a hit percentage instead of only its AP preview");
    Require(world->GetGame()->GetCompositeToHit(actor,target,NAI::HL_ANY,world->IsFirstTurn()) == sharedChance,
        "formatting melee reach changed the numeric chance contract used by scripts and LAN preview");
    const auto original = actor->GetPosition();
    actor->SetTemporaryPosition(target->GetPosition());
    Require(NWorld::CanMeleeAttack(actor,actor->GetPosition(),center),
        "melee reach fixture failed to place its control within current-pose reach");
    state->RefreshCaption();
    Require(state->GetCursorInfo().wsText.find(L'%') != wstring::npos,
        "an in-reach native melee target lost its ordinary percentage preview");
    actor->SetTemporaryPosition(original);
    Require(actor->GetAP() == ap && !actor->HasCommand(), "formatting melee reach spent AP or installed an action");
}

static void SelectedAttackTargetRejection(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> actors;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) actors.push_back(unit);
        if (actors.size() == 2) break;
    }
    Require(actors.size() == 2, "self-target fixture has fewer than two ordinary native actors");
    CObj<SelectedAttackTargetMission> mission = new SelectedAttackTargetMission;
    mission->Attach(world,game);
    CObj<NGame::CUnitTracker> first = new NGame::CUnitTracker(mission,actors[0]);
    CObj<NGame::CUnitTracker> second = new NGame::CUnitTracker(mission,actors[1]);
    mission->selection.push_back(first.GetPtr());
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    auto objectTarget = [&](NWorld::CUnit* target) {
        mission->target = target;
        CObj<NWorld::CCmd> command = state->TargetCommand();
        auto* attack = dynamic_cast<NWorld::CCmdShootObject*>(command.GetPtr());
        Require(attack && attack->pTarget == target && attack->bCanBeReplacedByReload,
            "an unselected target lost its native object-attack command or replacement-reload policy");
    };
    objectTarget(actors[1]);
    mission->target = actors[0];
    CObj<NWorld::CCmd> command = state->TargetCommand();
    Require(!command.GetPtr(), "the attack cursor created a shot at its own selected attacker");
    mission->selection.push_back(second.GetPtr());
    mission->target = actors[1];
    command = state->TargetCommand();
    Require(!command.GetPtr(), "the attack cursor created a shot at another member of the selected group");
    mission->selection.erase(mission->selection.begin());
    objectTarget(actors[0]);
    mission->target = nullptr;
    command = state->TargetCommand();
    auto* tile = dynamic_cast<NWorld::CCmdShootTile*>(command.GetPtr());
    Require(tile && tile->ptTarget == CVec3(1,2,3) && tile->bCanBeReplacedByReload,
        "rejecting a selected object changed ordinary traced-tile attacks");
    Require(!actors[0]->HasCommand() && !actors[1]->HasCommand(),
        "building an attack cursor preview installed an actual unit command");
}

static void CharacterPanelFullName(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) { actor = unit; break; }
    }
    Require(actor != nullptr, "character-name fixture has no native actor");
    auto* rpg = actor->GetRPG()->GetRPGUnit();
    rpg->wsFullName = L"Character panel full display name";
    Require(rpg->GetName() != rpg->wsFullName, "character-name fixture needs distinct full and nick names");
    CObj<NativeActionInfoMission> mission = new NativeActionInfoMission;
    mission->Attach(world,game);
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission,actor);
    mission->selection.push_back(tracker.GetPtr());
    NDb::CUIContainer* layout = nullptr;
    CDBIterator<NDb::CUIContainer> layouts(*NDatabase::GetTable<NDb::CUIContainer>());
    while (layouts.MoveNext()) {
        bool fullName = false, strength = false, biography = false;
        for (auto& control: layouts.Get()->controls) {
            if (!IsValid(control)) continue;
            fullName |= control->szID == "fullname";
            strength |= control->szID == "strength";
            biography |= control->szID == "biography";
        }
        if (fullName && strength && biography) { layout = layouts.Get(); break; }
    }
    Require(layout != nullptr, "character-name fixture could not find the authored character-panel layout");
    CObj<NUI::ICursor> cursor = NUI::ICursor::Create(false);
    CObj<NUI::CInterface> interface = new NUI::CInterface(cursor);
    CObj<NUI::CCharacterPanel> panel = new NUI::CCharacterPanel(NUI::SWindowInfo(
        interface,NUI::SPoint(0,0),NUI::SPoint(layout->nWidth,layout->nHeight),"character-name-probe"),mission);
    NUI::LoadTemplate(panel,layout);
    auto* fullName = dynamic_cast<NUI::CText*>(panel->GetChildByID("fullname"));
    Require(fullName != nullptr, "native character panel did not create its authored full-name label");
    const int ap = actor->GetAP();
    panel->SetStyle(NUI::STYLE_VISIBLE,true);
    Require(panel->Update(0,nullptr), "native character panel failed to update its visible controls");
    Require(fullName->GetText() == L"<font face=Courier size=16pt><center>" + rpg->wsFullName,
        "the character panel showed the nickname instead of the full display name");
    rpg->wsFullName = wstring(320,L'X');
    panel->Update(1,nullptr);
    Require(fullName->GetText() == L"<font face=Courier size=16pt><center>" + rpg->wsFullName,
        "the character panel truncated the stored full name to its legacy formatting buffer");
    rpg->wsFullName.clear();
    panel->Update(2,nullptr);
    Require(fullName->GetText() == L"<font face=Courier size=16pt><center>" + rpg->GetName(),
        "an older save without a full name lost its existing nickname heading");
    Require(actor->GetAP() == ap && !actor->HasCommand(), "updating the character name changed native gameplay state");
}

class InitialHeldItemActor: public NWorld::CDumbUnitServer
{
    OBJECT_NOCOPY_METHODS(InitialHeldItemActor);
public:
    InitialHeldItemActor() {}
    InitialHeldItemActor(NWorld::CWorld* world,NRPG::IUnitMission* rpg,NDb::CModel* model,const NAI::SUnitPosition& position):
        CDumbUnitServer(world,rpg,model,position) {}
    bool IsDead() const override { return false; }
    bool IsUnconscious() const override { return false; }
    bool CanFight() const override { return true; }
};

static void CancelledInventoryHiddenItemPose(const char* databasePath,const char* resourcePath,int operation = 0)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight() || unit->IsEmptyPK() || unit->IsWearingPK()) continue;
        auto* active = unit->GetUnitRPG()->GetInventory()->GetActive();
        if (active && active->GetDBItem()->bPlaceInHand) { actor = unit; break; }
    }
    Require(actor != nullptr, "inventory-cancel fixture has no actor with a visible active item");
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    auto* item = inventory->GetActive();
    const auto slot = NDb::ESlot(inventory->GetActiveSlot());
    const int ap = actor->GetAP(), quantity = item->GetQuantity();
    if (operation == 1) {
        auto create = [&]() {
            return CObj<InitialHeldItemActor>(new InitialHeldItemActor(world,actor->GetUnitRPG(),actor->GetModel(),actor->GetPosition()));
        };
        {
            auto shown = create();
            Require(shown->animator.IsActiveItem(), "new actor with a visible active item lost its initial held pose");
        }
        item->GetDBItem()->bPlaceInHand = false;
        {
            auto hidden = create();
            Require(!hidden->animator.IsActiveItem(), "a newly created actor treated an invisible active item as a held weapon");
            Require(hidden->GetUnitRPG() == actor->GetUnitRPG() && hidden->GetUnitPosition().pos.p == actor->GetPosition().pos.p,
                "initial held-pose gating changed the constructor's RPG owner or position");
        }
        item->GetDBItem()->bPlaceInHand = true;
        auto restored = create();
        Require(restored->animator.IsActiveItem(), "new actor did not recover its visible-item held pose");
        Require(inventory->GetActive() == item && inventory->GetActiveSlot() == slot &&
            actor->GetAP() == ap && item->GetQuantity() == quantity,
            "initial held-pose gating changed equipment, quantity or AP");
        return;
    }
    auto animationWeapon = [&]() {
        CMemoryStream fields;
        CStructureNetworkContext references;
        references.include = [](CObjectBase*) { return false; };
        { CStructureSaver writer(fields,CStructureSaver::WRITE,&references); actor->animator.operator&(writer); }
        fields.Seek(0);
        auto type = NDb::WT_DEFAULT;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(29,&type); }
        return type;
    };
    auto cancel = [&]() {
        CObj<NWorld::CCommandExecute> move;
        if (operation == 2) {
            move = dynamic_cast<NWorld::CCommandExecute*>(pSSClasses->CreateObject(int(0x01122133)));
            Require(IsValid(move), "equipment-cancel fixture could not create the native activation executor");
            struct OwnerWire {
                CPtr<NWorld::CUnitServer> actor;
                int operator&(CStructureSaver& saver) { saver.Add(4,&actor); return 0; }
            } owner;
            owner.actor = actor;
            CStructureNetworkContext references;
            references.externalReferences = true; references.nextID = 100;
            references.ids[actor] = 1; references.refs[1] = actor;
            references.include = [](CObjectBase*) { return false; };
            CMemoryStream fields;
            { CStructureSaver writer(fields,CStructureSaver::WRITE,&references); writer.Add(1,&owner); }
            fields.Seek(0);
            { CStructureSaver reader(fields,CStructureSaver::READ,&references); move->operator&(reader); }
        } else {
            CObj<NWorld::CCmdMoveInventoryItem> command = new NWorld::CCmdMoveInventoryItem(
                NWorld::SItem(actor,NWorld::SItem::SLOT,slot,item),NWorld::SItem(actor,NWorld::SItem::SLOT,slot,item));
            move = new NWorld::CExecMoveInventoryItem(actor,command);
        }
        move->Cancel();
        Require(move->GetState() == NWorld::CCommandExecute::FINISHED,
            "cancelling an inventory animation failed to finish its native executor");
    };
    cancel();
    Require(actor->animator.IsActiveItem() && !actor->GetUndrawItem() &&
        animationWeapon() == actor->GetUnitRPG()->GetWeaponType(), "visible-item cancellation control lost its held pose");
    item->GetDBItem()->bPlaceInHand = false;
    cancel();
    Require(!actor->animator.IsActiveItem() && actor->GetUndrawItem() && animationWeapon() == NDb::WT_DEFAULT,
        operation == 2 ? "cancelling equipment activation restored a held-weapon pose for an item marked invisible in hand" :
        "cancelling inventory movement restored a held-weapon pose for an item marked invisible in hand");
    item->GetDBItem()->bPlaceInHand = true;
    cancel();
    Require(actor->animator.IsActiveItem() && !actor->GetUndrawItem(),
        "reenabling item visibility failed to restore its held pose after cancellation");
    Require(inventory->GetActive() == item && inventory->GetActiveSlot() == slot &&
        actor->GetAP() == ap && item->GetQuantity() == quantity && !actor->HasCommand(),
        "cancelling the visual inventory transition changed equipment, AP, quantity or gameplay orders");
}

static void SpentMedkitPoseCleanup(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> actors;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) actors.push_back(unit);
        if (actors.size() == 2) break;
    }
    Require(actors.size() == 2, "spent-medkit fixture has fewer than two ordinary native actors");
    auto* actor = actors[0]; auto* target = actors[1];
    NDb::CRPGFirstAid* record = nullptr;
    CDBIterator<NDb::CRPGFirstAid> kits(*NDatabase::GetTable<NDb::CRPGFirstAid>());
    while (kits.MoveNext()) {
        auto* kit = kits.Get();
        if (IsValid(kit->pItem) && kit->effect == NDb::FAE_NORMAL && kit->nQuantity >= 2) { record = kit; break; }
    }
    Require(record != nullptr, "spent-medkit fixture has no authored ordinary kit with at least two charges");
    CObj<NRPG::CFirstAidItem> kit = new NRPG::CFirstAidItem(record);
    kit->GetItem()->nQuantity = 2;
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    const auto previousWeaponType = actor->GetUnitRPG()->GetWeaponType();
    Require(previousWeaponType != NDb::WT_DEFAULT, "spent-medkit fixture needs a previous held weapon animation for its restore control");
    CObj<NRPG::IInventoryItem> previous = inventory->Get(NDb::SLOT_1) ? inventory->TakeOff(NDb::SLOT_1) : nullptr;
    Require(inventory->Equip(NDb::SLOT_1,kit) && inventory->Activate(NDb::SLOT_1),
        "spent-medkit fixture could not equip its native first-aid item");
    const auto weaponType = actor->GetUnitRPG()->GetWeaponType();
    actor->animator.SetActiveItem(true);
    actor->animator.SetWeaponAnimation(weaponType);
    const int ap = actor->GetAP(), vp = target->GetUnitRPG()->GetTotalVP();
    auto finish = [&]() {
        CObj<NWorld::CUnitStateHealer> state = new NWorld::CUnitStateHealer(actor,target);
        state->OnStateFinished();
    };
    finish();
    Require(kit->GetIncQuantity() == 1 && inventory->GetActive() == kit.GetPtr() && actor->animator.IsActiveItem(),
        "finishing with a nonempty kit changed its remaining charge or held-state control");
    // A restored legacy pose can still carry the preceding item's animation set.
    actor->animator.SetWeaponAnimation(previousWeaponType);
    finish();
    Require(kit->GetIncQuantity() == 0 && !inventory->GetActive(),
        "finishing the final medical charge failed to remove exactly one exhausted kit");
    CMemoryStream fields;
    CStructureNetworkContext references;
    references.include = [](CObjectBase*) { return false; };
    { CStructureSaver writer(fields,CStructureSaver::WRITE,&references); actor->animator.operator&(writer); }
    fields.Seek(0);
    auto animation = weaponType;
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(29,&animation); }
    Require(!actor->animator.IsActiveItem() && animation == NDb::WT_DEFAULT,
        "finishing the last medkit charge retained the held-item or healing weapon animation state");
    Require(actor->GetAP() == ap && target->GetUnitRPG()->GetTotalVP() == vp &&
        !actor->HasCommand() && !target->HasCommand(),
        "cleaning up the spent medkit changed AP, healed VP or installed an action");
}

static void SpentPicklockPoseCleanup(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) { actor = unit; break; }
    }
    NWorld::CWindowDoor* door = nullptr;
    for (const auto& entry: world->GetObjectsForHarness()) {
        auto* candidate = dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr());
        if (candidate && !candidate->IsBroken()) { door = candidate; break; }
    }
    Require(actor && door, "spent-picklock fixture lacks a native actor or authored door");
    NDb::CRPGPicklock* record = nullptr;
    CDBIterator<NDb::CRPGPicklock> picks(*NDatabase::GetTable<NDb::CRPGPicklock>());
    while (picks.MoveNext()) {
        auto* pick = picks.Get();
        if (IsValid(pick->pItem) && pick->nQuantity >= 2) { record = pick; break; }
    }
    Require(record != nullptr, "spent-picklock fixture has no authored picklock with two charges");
    CObj<NRPG::CPicklockItem> pick = new NRPG::CPicklockItem(record);
    pick->GetItem()->nQuantity = 2;
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    const auto previousWeaponType = actor->GetUnitRPG()->GetWeaponType();
    Require(previousWeaponType != NDb::WT_DEFAULT, "spent-picklock fixture needs a preceding held weapon pose");
    CObj<NRPG::IInventoryItem> previous = inventory->Get(NDb::SLOT_1) ? inventory->TakeOff(NDb::SLOT_1) : nullptr;
    Require(inventory->Equip(NDb::SLOT_1,pick) && inventory->Activate(NDb::SLOT_1),
        "spent-picklock fixture could not equip its native picklock");
    door->LockDoor(true,0x7fffffff,0x7fffffff); // Guaranteed failed attempts, independently of the native roll.
    const bool open = door->IsOpen();
    const int ap = actor->GetAP();
    actor->animator.SetActiveItem(true);
    actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    CObj<NWorld::CExecOpenClose> executor = new NWorld::CExecOpenClose(actor,new NWorld::CCmdOpenClose(door,!open));
    executor->TimeLabelReached();
    Require(pick->GetIncQuantity() == 1 && inventory->GetActive() == pick.GetPtr() && actor->animator.IsActiveItem(),
        "a failed pick attempt did not retain the nonempty picklock and its held pose");
    // Include the preceding weapon set retained in a restored legacy animation state.
    actor->animator.SetWeaponAnimation(previousWeaponType);
    executor->TimeLabelReached();
    Require(pick->GetIncQuantity() == 0 && !inventory->GetActive(),
        "the final failed pick attempt did not remove exactly one exhausted picklock");
    CMemoryStream fields;
    CStructureNetworkContext references;
    references.include = [](CObjectBase*) { return false; };
    { CStructureSaver writer(fields,CStructureSaver::WRITE,&references); actor->animator.operator&(writer); }
    fields.Seek(0);
    auto animation = previousWeaponType;
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(29,&animation); }
    Require(!actor->animator.IsActiveItem() && animation == NDb::WT_DEFAULT,
        "consuming the last picklock charge retained the held-item or weapon animation state");
    Require(door->IsLockedDoor() && door->IsOpen() == open && actor->GetAP() == ap && !actor->HasCommand(),
        "picklock pose cleanup changed the failed lock result, AP or gameplay orders");
}

static void MinePlacementSuccessorHandoff(const char* databasePath,const char* resourcePath)
{
    for (int variant: {1,0,2,3}) {
        CObj<NRPG::CGlobalGame> game;
        auto world = FixtureWorld(databasePath,resourcePath,false,&game);
        vector<CPtr<NWorld::CUnit>> units;
        world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr;
        for (auto& entry: units) {
            auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
            if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) { actor = unit; break; }
        }
        Require(actor != nullptr, "mine-handoff fixture lacks an ordinary native actor");
        NDb::CRPGMine* record = nullptr;
        CDBIterator<NDb::CRPGMine> mines(*NDatabase::GetTable<NDb::CRPGMine>());
        while (mines.MoveNext()) {
            auto* mine = mines.Get();
            if (IsValid(mine->pItem) && IsValid(mine->pItem->pModel) && IsValid(mine->pExplosion) &&
                mine->pItem->bPlaceInHand) { record = mine; break; }
        }
        Require(record != nullptr, "mine-handoff fixture has no authored visible mine");
        const bool hidden = variant == 2;
        const bool replacement = variant != 0;
        record->pItem->bPlaceInHand = !hidden;
        CObj<NRPG::CMineItem> spent = new NRPG::CMineItem(record);
        CObj<NRPG::CMineItem> next = replacement ? new NRPG::CMineItem(record) : nullptr;
        spent->nQuantity = 1;
        if (next) next->nQuantity = 1;
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        while (!inventory->GetItems().empty()) inventory->Take(inventory->GetItems().front().pItem);
        CObj<NRPG::IInventoryItem> previous = inventory->Get(NDb::SLOT_1) ? inventory->TakeOff(NDb::SLOT_1) : nullptr;
        Require(inventory->Equip(NDb::SLOT_1,spent) && inventory->Activate(NDb::SLOT_1),
            "mine-handoff fixture could not equip the first mine");
        if (replacement) {
            CTPoint<int> place;
            Require(inventory->FindPlace(next,&place) && inventory->Place(place,next),
                "mine-handoff fixture could not store the second mine");
        }
        const auto target = actor->GetPosition().pos;
        vector<CVec3> places(1,target.GetCP());
        vector<CPtr<NWorld::CMine>> nearby;
        Require(!world->GetMineTracker()->GetMines(places,&nearby), "mine-handoff fixture target already has a mine");
        CObj<NWorld::CCmdSetMineOnTile> command = new NWorld::CCmdSetMineOnTile(target);
        CObj<NWorld::CExecSetMine> executor = new NWorld::CExecSetMine(actor,command);
        Require(executor->CanDoIt(actor->GetPosition()) == NWorld::UCR_OK,
            "mine-handoff fixture cannot perform the ordinary placement command");
        actor->animator.SetActiveItem(!hidden);
        actor->animator.SetMine(actor->GetPosition(),true);
        const auto animationEnd = actor->animator.GetTimeEnd();
        const auto animationLabel = actor->animator.GetTimeLabel1();
        const int ap = actor->GetAP();
        executor->TimeLabelReached();
        nearby.clear();
        Require(world->GetMineTracker()->GetMines(places,&nearby) && nearby.size() == 1,
            "mine placement did not create exactly one native mine at the label");
        Require(nearby.front()->GetMineDC() == actor->GetUnitRPG()->GetMineDC(record) &&
            nearby.front()->GetMinePos() == target.GetCP(), "mine animation handoff changed placement or disarm difficulty");
        Require(!inventory->GetActive(), "mine placement equipped the successor before its animation finished");
        Require(actor->animator.GetTimeEnd() == animationEnd && actor->animator.GetTimeLabel1() == animationLabel,
            "consuming a mine interrupted its native placement animation");
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(executor,&references);
        CObj<NRPG::IInventoryItem> savedNext;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(3,&savedNext); }
        Require(savedNext.GetPtr() == next.GetPtr(), "the pending mine successor was not retained in executor saved field three");
        fields.Seek(0);
        CObj<NWorld::CExecSetMine> restored = new NWorld::CExecSetMine;
        if (variant == 3) {
            // Old saves had no pending field: the next item was already equipped at the label.
            inventory->Take(next);
            Require(inventory->Equip(NDb::SLOT_1,next), "legacy mine fixture could not install its already-active successor");
            CMemoryStream legacy;
            { CStructureSaver writer(legacy,CStructureSaver::WRITE,&references); writer.Add(1,static_cast<NWorld::CCommandExecute*>(executor.GetPtr())); writer.Add(2,&command); }
            legacy.Seek(0);
            { CStructureSaver reader(legacy,CStructureSaver::READ,&references); static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
        } else {
            CStructureSaver reader(fields,CStructureSaver::READ,&references);
            static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);
        }
        Require(restored->GetUnitServer() == actor, "restoring the pending mine callback lost its native actor");
        executor = nullptr;
        restored->AnimationFinished();
        Require(restored->GetState() == NWorld::CCommandExecute::FINISHED && inventory->GetActive() == next.GetPtr(),
            "completing the restored mine callback failed to equip its pending successor");
        Require(actor->animator.IsActiveItem() == (replacement && !hidden),
            "mine placement completion retained an empty or authored-hidden held pose");
        Require(inventory->GetItems().empty() && actor->GetAP() == ap && !actor->HasCommand() &&
            spent->GetQuantity() == 1 && (!replacement || next->GetQuantity() == 1),
            "mine animation handoff duplicated an item, changed quantity or AP, or installed an order");
    }
}

static void TrapPlacementSuccessorHandoff(const char* databasePath,const char* resourcePath,bool engineer = false)
{
    for (int variant: {1,0,2,3}) {
        CObj<NRPG::CGlobalGame> game;
        auto world = FixtureWorld(databasePath,resourcePath,false,&game);
        vector<CPtr<NWorld::CUnit>> units;
        world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr;
        for (auto& entry: units) {
            auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
            if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) { actor = unit; break; }
        }
        CPtr<NWorld::CWindowDoor> door;
        for (const auto& entry: world->GetObjectsForHarness()) {
            auto* candidate = dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr());
            if (candidate && !candidate->IsBroken() && !candidate->IsMineSet() && world->GetAIMap()->GetHull(entry.GetPtr())) {
                door = candidate; break;
            }
        }
        Require(actor && door, "trap-handoff fixture lacks a native actor or untrapped authored door with an AI hull");
        NDb::CRPGGrenade* grenade = nullptr;
        NDb::CRPGEngGrenade* engGrenade = nullptr;
        NDb::CRPGItem* item = nullptr;
        if (engineer) {
            CDBIterator<NDb::CRPGEngGrenade> rows(*NDatabase::GetTable<NDb::CRPGEngGrenade>());
            while (rows.MoveNext()) if (IsValid(rows.Get()->pItem) && IsValid(rows.Get()->pItem->pModel)) {
                engGrenade = rows.Get(); item = engGrenade->pItem; break;
            }
        } else {
            CDBIterator<NDb::CRPGGrenade> rows(*NDatabase::GetTable<NDb::CRPGGrenade>());
            while (rows.MoveNext()) if (IsValid(rows.Get()->pItem) && IsValid(rows.Get()->pItem->pModel)) {
                grenade = rows.Get(); item = grenade->pItem; break;
            }
        }
        Require(item != nullptr, "trap-handoff fixture has no modelled authored grenade of the required kind");
        const bool hidden = variant == 2, replacement = variant != 0;
        item->bPlaceInHand = !hidden;
        CObj<NRPG::CGrenadeItem> spent = engineer ? new NRPG::CGrenadeItem(engGrenade) : new NRPG::CGrenadeItem(grenade);
        CObj<NRPG::CGrenadeItem> next = replacement ?
            (engineer ? new NRPG::CGrenadeItem(engGrenade) : new NRPG::CGrenadeItem(grenade)) : nullptr;
        spent->nQuantity = 1; spent->SetMode(NRPG::GM_SETTRAP);
        if (next) next->nQuantity = 1; // Leave its initial THROW mode for the native handoff to copy.
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        while (!inventory->GetItems().empty()) inventory->Take(inventory->GetItems().front().pItem);
        CObj<NRPG::IInventoryItem> previous = inventory->Get(NDb::SLOT_1) ? inventory->TakeOff(NDb::SLOT_1) : nullptr;
        Require(inventory->Equip(NDb::SLOT_1,spent) && inventory->Activate(NDb::SLOT_1),
            "trap-handoff fixture could not equip its native grenade");
        if (replacement) {
            CTPoint<int> place;
            Require(inventory->FindPlace(next,&place) && inventory->Place(place,next), "trap-handoff fixture could not store a successor");
        }
        CObj<NWorld::CExecSetTrap> executor = new NWorld::CExecSetTrap(actor,door);
        Require(executor->CanDoIt(actor->GetPosition()) == NWorld::UCR_OK, "trap-handoff fixture cannot perform its native command");
        actor->animator.SetActiveItem(!hidden);
        actor->animator.SetMine(actor->GetPosition(),false);
        const auto animationEnd = actor->animator.GetTimeEnd(), animationLabel = actor->animator.GetTimeLabel1();
        const int ap = actor->GetAP();
        const bool open = door->IsOpen(), locked = door->IsLockedDoor();
        const int dc = actor->GetUnitRPG()->GetGrenadeTrapDC(grenade);
        executor->TimeLabelReached();
        Require(door->IsMineSet() && door->GetMineDC() == dc, "trap handoff changed the placed grenade or disarm difficulty");
        Require(!inventory->GetActive(), "door trap placement equipped the successor before its animation finished");
        Require(actor->animator.GetTimeEnd() == animationEnd && actor->animator.GetTimeLabel1() == animationLabel,
            "consuming a trap grenade interrupted its placement animation");
        Require(!replacement || next->GetMode() == NRPG::GM_SETTRAP, "pending trap successor lost the grenade's set-trap mode");
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(executor,&references);
        CObj<NRPG::IInventoryItem> savedNext;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(3,&savedNext); }
        Require(savedNext.GetPtr() == next.GetPtr(), "the pending trap grenade was not retained in saved field three");
        fields.Seek(0);
        CObj<NWorld::CExecSetTrap> restored = new NWorld::CExecSetTrap;
        if (variant == 3) {
            inventory->Take(next);
            Require(inventory->Equip(NDb::SLOT_1,next), "legacy trap fixture could not install its already-active successor");
            CMemoryStream legacy;
            { CStructureSaver writer(legacy,CStructureSaver::WRITE,&references); writer.Add(1,static_cast<NWorld::CCommandExecute*>(executor.GetPtr())); writer.Add(2,&door); }
            legacy.Seek(0);
            { CStructureSaver reader(legacy,CStructureSaver::READ,&references); static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
        } else {
            CStructureSaver reader(fields,CStructureSaver::READ,&references);
            static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);
        }
        Require(restored->GetUnitServer() == actor, "restoring the trap callback lost its native actor");
        executor = nullptr;
        restored->AnimationFinished();
        Require(restored->GetState() == NWorld::CCommandExecute::FINISHED && inventory->GetActive() == next.GetPtr(),
            "the restored trap callback failed to finish with its pending grenade");
        Require(actor->animator.IsActiveItem() == (replacement && !hidden), "trap completion retained an empty or hidden held pose");
        Require(door->IsMineSet() && door->GetMineDC() == dc && door->IsOpen() == open && door->IsLockedDoor() == locked &&
            actor->GetAP() == ap && !actor->HasCommand() && inventory->GetItems().empty() && spent->GetQuantity() == 1 &&
            (!replacement || next->GetQuantity() == 1), "trap animation handoff changed door state, difficulty, AP, quantity or orders");
    }
}

static void ThrownKnifeSuccessorHandoff(const char* databasePath,const char* resourcePath,bool feedback = false)
{
    const vector<int> variants = feedback ? vector<int>{1,3} : vector<int>{1,0,2};
    for (int variant: variants) {
        CObj<NRPG::CGlobalGame> game;
        auto world = FixtureWorld(databasePath,resourcePath,false,&game);
        vector<CPtr<NWorld::CUnit>> units;
        world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr;
        for (auto& entry: units) {
            auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
            if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()) { actor = unit; break; }
        }
        Require(actor != nullptr, "knife-handoff fixture lacks an ordinary native actor");
        CObj<NWorld::CUnitServer> shell;
        if (variant == 3) {
            NDb::CRPGPers* suit = nullptr;
            CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
            while (people.MoveNext()) if (IsValid(people.Get()->pPanzerklein) &&
                !people.Get()->pPanzerklein->bHasNoHead && IsValid(people.Get()->pModel)) { suit = people.Get(); break; }
            Require(suit != nullptr, "knife feedback fixture has no ordinary authored PK shell");
            vector<NAI::SPathPlace> places;
            world->GetPathNetwork()->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),20.f),&places);
            NAI::SPathPlace anchor;
            bool found = false;
            for (auto place: places) {
                place.SetPose(NAI::CM_STAND); place.SetDirection(0);
                if (NAI::IsBigLockerPlacePassable(world->GetPathNetwork(),place)) { anchor = place; found = true; break; }
            }
            Require(found, "knife feedback fixture has no free native PK footprint");
            shell = world->AddUnitInGame(anchor,NRPG::CreateUnit(suit),nullptr);
            Require(shell && shell->IsEmptyPK() && shell->WearAsPK(true), "knife feedback fixture could not prepare its native shell");
            actor->SetPosition(shell->GetPosition());
            actor->FlipPanzerklein(shell,false,false);
            Require(actor->IsWearingPK(), "knife feedback fixture did not put on its actual PK shell");
        }
        NDb::CRPGMeleeWeapon* record = nullptr;
        CDBIterator<NDb::CRPGMeleeWeapon> rows(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
        while (rows.MoveNext()) {
            auto* blade = rows.Get();
            if (blade->bThrowing && IsValid(blade->pItem) && IsValid(blade->pItem->pModel)) { record = blade; break; }
        }
        Require(record != nullptr, "knife-handoff fixture has no authored modelled throwing knife");
        const bool replacement = variant != 0;
        record->pItem->bPlaceInHand = variant != 2;
        CObj<NRPG::CMeleeWeaponItem> spent = new NRPG::CMeleeWeaponItem(record);
        CObj<NRPG::CMeleeWeaponItem> next = replacement ? new NRPG::CMeleeWeaponItem(record) : nullptr;
        spent->nQuantity = 1;
        if (next) next->nQuantity = 1;
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        while (!inventory->GetItems().empty()) inventory->Take(inventory->GetItems().front().pItem);
        CObj<NRPG::IInventoryItem> previous = inventory->Get(NDb::SLOT_1) ? inventory->TakeOff(NDb::SLOT_1) : nullptr;
        Require(inventory->Equip(NDb::SLOT_1,spent) && inventory->Activate(NDb::SLOT_1) &&
            NWorld::GetActionType(actor) == NWorld::AT_THROW, "knife-handoff fixture did not equip a native throwing knife");
        if (replacement) {
            CTPoint<int> place;
            Require(inventory->FindPlace(next,&place) && inventory->Place(place,next), "knife-handoff fixture could not store its successor");
        }
        CObj<NWorld::CExecThrowKnife> executor;
        CVec3 target;
        for (const auto& offset: {CVec3(1,0,0.5f),CVec3(-1,0,0.5f),CVec3(0,1,0.5f),CVec3(0,-1,0.5f)}) {
            target = actor->GetPosition().GetCP()+offset;
            CObj<NWorld::CExecThrowKnife> candidate = new NWorld::CExecThrowKnife(actor,target);
            if (candidate->CanDoIt(actor->GetPosition()) == NWorld::UCR_OK) { executor = candidate; break; }
        }
        Require(executor != nullptr, "knife-handoff fixture has no nearby valid native throw target");
        actor->animator.SetActiveItem(variant != 2);
        actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
        actor->animator.ThrowKnife(actor->GetPosition(),target);
        const auto animationEnd = actor->animator.GetTimeEnd(), animationLabel = actor->animator.GetTimeLabel1();
        Require(!actor->animator.IsActiveItem() && (shell || (animationLabel > 0 && animationEnd >= animationLabel)),
            "knife-handoff fixture did not start a real native throwing animation");
        const int ap = actor->GetAP(), vp = actor->GetUnitRPG()->GetTotalVP();
        vector<NWorld::IDynamicObject*> previousObjects;
        for (auto& object: *world->GetMiscObjects()) previousObjects.push_back(object.GetPtr());
        Require(!executor->TimeLabelReached() && executor->GetState() == NWorld::CCommandExecute::FINISHED,
            "the native throwing-knife label did not finish its one-shot command");
        int projectiles = 0;
        for (auto& object: *world->GetMiscObjects()) {
            if (std::find(previousObjects.begin(),previousObjects.end(),object.GetPtr()) != previousObjects.end() ||
                pSSClasses->GetObjectTypeID(object.GetPtr()) != int(0x10662182)) continue;
            ++projectiles;
            CStructureNetworkContext projectileReferences;
            auto projectileFields = BorrowedObjectFields(object,&projectileReferences);
            CObj<NRPG::IInventoryItem> flyingItem;
            NRPG::SAttackRayInfo ray;
            { CStructureSaver reader(projectileFields,CStructureSaver::READ,&projectileReferences); reader.Add(8,&flyingItem); reader.Add(11,&ray); }
            Require(flyingItem.GetPtr() == spent.GetPtr() && ray.pUS.GetPtr() == actor &&
                dynamic_cast<NWorld::CUnitServer*>(ray.pIgnore.GetPtr()) == actor,
                "the native knife projectile lost the consumed item or throwing actor");
        }
        Require(projectiles == 1 && inventory->GetActive() == next.GetPtr(),
            "the native knife callback failed to launch one blade and equip only its successor");
        if (feedback) {
            struct Feedback: NWorld::ISoundVisitor {
                vector<NDb::CSound*> sounds;
                vector<CVec3> positions;
                void Add3DSound(STime,NDb::CSound* sound,CFuncBase<CVec3>* source) override {
                    sounds.push_back(sound);
                    CDGPtr<CFuncBase<CVec3>> position(source); position.Refresh();
                    positions.push_back(position->GetValue());
                }
                void AddEffect(STime,NDb::CSoundEffect*,CFuncBase<CVec3>*,const vector<int>&) override {
                    throw std::runtime_error("knife feedback unexpectedly created a sound effect");
                }
                void Add2DSound(STime,NDb::CSound*) override { throw std::runtime_error("knife feedback unexpectedly became non-positional"); }
            } sounds;
            for (auto& object: *world->GetMiscObjects()) {
                if (std::find(previousObjects.begin(),previousObjects.end(),object.GetPtr()) != previousObjects.end()) continue;
                if (auto* sound = dynamic_cast<NWorld::C3DSound*>(object.GetPtr())) sound->Visit(&sounds);
            }
            auto* expected = NDb::GetSound(0x3285);
            Require(IsValid(expected), "knife feedback fixture has no authored ordinary throw sound");
            if (!shell) Require(sounds.sounds == vector<NDb::CSound*>{expected} &&
                sounds.positions == vector<CVec3>{actor->GetPosition().GetCP()},
                "throwing a native knife omitted its ordinary positional feedback sound");
            else Require(sounds.sounds.empty(), "a knife thrown from a worn PK played the ordinary human feedback sound");
        }
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(executor,&references);
        CObj<NRPG::IInventoryItem> pending;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(4,&pending); }
        Require(pending.GetPtr() == next.GetPtr(), "the thrown-knife executor did not retain its successor in saved field four");
        Require(actor->animator.GetTimeEnd() == animationEnd && actor->animator.GetTimeLabel1() == animationLabel,
            "equipping the next throwing knife interrupted the native throw animation");
        Require(!actor->animator.IsActiveItem(), "the successor knife became visible in the hand during the throwing animation");
        fields.Seek(0);
        CObj<NWorld::CExecThrowKnife> restored = new NWorld::CExecThrowKnife;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
        Require(restored->GetUnitServer() == actor && restored->GetState() == NWorld::CCommandExecute::FINISHED,
            "restoring the completed knife callback lost its actor or completion state");
        Require(inventory->GetItems().empty() && actor->GetAP() == ap && actor->GetUnitRPG()->GetTotalVP() == vp &&
            !actor->HasCommand() && spent->GetQuantity() == 1 && (!replacement || next->GetQuantity() == 1),
            "knife successor handoff changed quantities, AP, VP or installed a gameplay order");
    }
}

struct NativeGrenadeThrowFixture
{
    CObj<NRPG::CGlobalGame> game;
    CObj<NWorld::CWorld> world;
    NWorld::CUnitServer* actor = nullptr;
    NDb::CRPGGrenade* regular = nullptr;
    NDb::CRPGEngGrenade* special = nullptr;
    CObj<NRPG::CGrenadeItem> grenade;
    CObj<NRPG::IInventoryItem> previous;
    CObj<NWorld::CExecThrowGrenade> executor;
    NativeGrenadeThrowFixture(const char* databasePath,const char* resourcePath,bool engineer,bool human = false,bool aiOnly = false)
    {
        world = FixtureWorld(databasePath,resourcePath,human,&game);
        vector<CPtr<NWorld::CUnit>> units;
        world->GetAllUnits(&units);
        for (auto& entry: units) {
            auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
            if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK() &&
                (!human || !unit->IsAIUnit()) && (!aiOnly || unit->IsAIUnit())) { actor = unit; break; }
        }
        Require(actor != nullptr, "restored-grenade fixture lacks an ordinary native actor");
        if (engineer) {
            CDBIterator<NDb::CRPGEngGrenade> rows(*NDatabase::GetTable<NDb::CRPGEngGrenade>());
            while (rows.MoveNext()) if (IsValid(rows.Get()->pItem) && IsValid(rows.Get()->pItem->pModel) &&
                rows.Get()->pItem->nWeight > 0 && (rows.Get()->nRequiredPerkID <= 0 ||
                actor->GetUnitRPG()->HasPerk(rows.Get()->nRequiredPerkID))) { special = rows.Get(); break; }
        } else {
            CDBIterator<NDb::CRPGGrenade> rows(*NDatabase::GetTable<NDb::CRPGGrenade>());
            while (rows.MoveNext()) if (IsValid(rows.Get()->pItem) && IsValid(rows.Get()->pItem->pModel) &&
                rows.Get()->pItem->nWeight > 0) { regular = rows.Get(); break; }
        }
        Require(regular || special, "restored-grenade fixture lacks a modelled authored grenade of positive weight");
        if (special) {
            auto& engineering = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_ENGINEERING);
            if (int(engineering) < special->nSkillReq) engineering.SetConst(special->nSkillReq);
        }
        grenade = engineer ? new NRPG::CGrenadeItem(special) : new NRPG::CGrenadeItem(regular);
        grenade->nQuantity = 1;
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        while (!inventory->GetItems().empty()) inventory->Take(inventory->GetItems().front().pItem);
        previous = inventory->Get(NDb::SLOT_1) ? inventory->TakeOff(NDb::SLOT_1) : nullptr;
        Require(inventory->Equip(NDb::SLOT_1,grenade) && inventory->Activate(NDb::SLOT_1),
            "restored-grenade fixture could not equip its authored grenade");
        actor->animator.SetActiveItem(true);
        actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
        for (float radius: {0.5f,1.f,2.f,3.f,5.f}) {
            for (const auto& offset: {CVec3(radius,0,0.1f),CVec3(-radius,0,0.1f),CVec3(0,radius,0.1f),CVec3(0,-radius,0.1f)}) {
                CObj<NWorld::CExecThrowGrenade> candidate = new NWorld::CExecThrowGrenade(actor,actor->GetPosition().GetCP()+offset);
                const auto result = candidate->CanDoIt(actor->GetPosition());
                if (result == NWorld::UCR_OK && actor->CanSpendAP(candidate->GetStartAP())) {
                    executor = candidate; break;
                }
            }
            if (executor) break;
        }
        Require(executor != nullptr, "restored-grenade fixture has no nearby valid native throw with sufficient AP");
    }
};

static void RestoredGrenadeThrow(const char* databasePath,const char* resourcePath,bool engineer = false)
{
    for (int variant: {0,1,2}) {
        const bool reload = variant != 0;
        NativeGrenadeThrowFixture fixture(databasePath,resourcePath,engineer);
        auto& world = fixture.world;
        auto* actor = fixture.actor;
        auto* regular = fixture.regular;
        auto* special = fixture.special;
        auto& grenade = fixture.grenade;
        auto& executor = fixture.executor;
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        const int actionCost = executor->GetStartAP();
        executor->Run(); // Initializes the real trajectory and enters the native action-start path once.
        Require(executor->GetState() == NWorld::CCommandExecute::RUNNING && executor->IsExecuting() &&
            actor->animator.GetTimeLabel1() > 0, "restored-grenade fixture did not start a real pending native throw");
        const int ap = actor->GetAP(), vp = actor->GetUnitRPG()->GetTotalVP();
        CPtr<NRPG::IGrenadeItem> remembered;
        if (reload) {
            CStructureNetworkContext references;
            auto fields = BorrowedObjectFields(executor,&references);
            { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(4,&remembered); }
            fields.Seek(0);
            CObj<NWorld::CExecThrowGrenade> restored = new NWorld::CExecThrowGrenade;
            if (variant == 2) {
                CVec3 target;
                NWorld::SGrenadeParams params;
                { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(2,&target); reader.Add(3,&params); }
                CPtr<NRPG::IGrenadeItem> missingReference;
                CObj<NRPG::IInventoryItem> noSuccessor;
                bool updateVision = false;
                CMemoryStream legacy;
                { CStructureSaver writer(legacy,CStructureSaver::WRITE,&references);
                  writer.Add(1,static_cast<NWorld::CCommandExecute*>(executor.GetPtr()));
                  writer.Add(2,&target); writer.Add(3,&params); writer.Add(4,&missingReference);
                  writer.Add(5,&noSuccessor); writer.Add(6,&updateVision); }
                legacy.Seek(0);
                { CStructureSaver reader(legacy,CStructureSaver::READ,&references); static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
            } else {
                CStructureSaver reader(fields,CStructureSaver::READ,&references);
                static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);
            }
            executor = restored;
            Require(executor->GetUnitServer() == actor && executor->IsExecuting(),
                "restoring the grenade executor lost its native actor or action counter");
            Require(executor->GetStartAP() == actionCost, "restoring the grenade executor changed its native action-cost query");
        }
        vector<NWorld::IDynamicObject*> before;
        for (auto& object: *world->GetMiscObjects()) before.push_back(object.GetPtr());
        std::printf("grenade continuation: engineer=%d restore_variant=%d AP=%d\n",int(engineer),variant,ap);
        std::fflush(stdout);
        Require(!executor->TimeLabelReached(), "a single grenade throw unexpectedly requested another time label");
        int projectiles = 0;
        for (auto& object: *world->GetMiscObjects()) {
            if (std::find(before.begin(),before.end(),object.GetPtr()) != before.end() ||
                pSSClasses->GetObjectTypeID(object.GetPtr()) != int(0x118B1160)) continue;
            ++projectiles;
            CStructureNetworkContext references;
            auto fields = BorrowedObjectFields(object,&references);
            CDBPtr<NDb::CRPGGrenade> actualRegular;
            CDBPtr<NDb::CRPGEngGrenade> actualSpecial;
            CPtr<NWorld::CUnitServer> thrower;
            int skill = 0;
            { CStructureSaver reader(fields,CStructureSaver::READ,&references);
              reader.Add(7,&actualRegular); reader.Add(9,&thrower); reader.Add(13,&actualSpecial); reader.Add(14,&skill); }
            Require(actualRegular.GetPtr() == regular && actualSpecial.GetPtr() == special && thrower.GetPtr() == actor &&
                (!engineer || skill == actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_ENGINEERING)),
                "the continued grenade throw changed its authored type, thrower or engineering skill");
        }
        Require(projectiles == 1 && !inventory->GetActive(), "the continued grenade throw did not consume and launch exactly one grenade");
        if (variant == 1) Require(remembered.GetPtr() == grenade.GetPtr(), "the saved grenade executor discarded its actual grenade reference");
        if (variant == 2) {
            CStructureNetworkContext references;
            auto fields = BorrowedObjectFields(executor,&references);
            CPtr<NRPG::IGrenadeItem> rebound;
            { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(4,&rebound); }
            Require(rebound.GetPtr() == grenade.GetPtr(), "continuing an old local save did not restore its missing grenade reference");
        }
        executor->AnimationFinished();
        Require(executor->GetState() == NWorld::CCommandExecute::FINISHED && actor->GetAP() == ap &&
            actor->GetUnitRPG()->GetTotalVP() == vp && grenade->GetQuantity() == 1 && !actor->HasCommand(),
            "continuing a saved grenade action charged AP again, changed VP/quantity or installed an order");
        executor = nullptr;
        Require(world->IsAction(), "the launched grenade lost its in-flight action counter");
    }
}

static void GrenadeSuccessorHandoff(const char* databasePath,const char* resourcePath,bool engineer = false)
{
    for (int variant: {0,1,2,3,4,5}) {
        const bool ai = variant == 3, spare = variant != 1;
        NativeGrenadeThrowFixture fixture(databasePath,resourcePath,engineer,!ai,ai);
        auto& world = fixture.world;
        auto* actor = fixture.actor;
        auto& executor = fixture.executor;
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        CObj<NRPG::CGrenadeItem> next = spare ? (engineer ?
            new NRPG::CGrenadeItem(fixture.special) : new NRPG::CGrenadeItem(fixture.regular)) : nullptr;
        if (next) {
            next->nQuantity = 1;
            CTPoint<int> place;
            Require(inventory->FindPlace(next,&place) && inventory->Place(place,next),
                "grenade-handoff fixture could not store its matching successor");
        }
        Require(actor->IsAIUnit() == ai, "grenade-handoff fixture selected the wrong human/AI control");
        executor->Run();
        Require(executor->IsExecuting() && actor->animator.GetTimeLabel1() > 0 && !actor->animator.IsActiveItem(),
            "grenade-handoff fixture did not enter a real throwing animation");
        const auto end = actor->animator.GetTimeEnd(), label = actor->animator.GetTimeLabel1();
        const int ap = actor->GetAP(), vp = actor->GetUnitRPG()->GetTotalVP();
        const bool hadCommand = actor->HasCommand();
        vector<NWorld::IDynamicObject*> before;
        for (auto& object: *world->GetMiscObjects()) before.push_back(object.GetPtr());
        Require(!executor->TimeLabelReached(), "a grenade handoff rearmed a single-shot time label");
        Require(!inventory->GetActive(), "throwing a grenade equipped the successor before the animation completed");
        Require(actor->animator.GetTimeEnd() == end && actor->animator.GetTimeLabel1() == label &&
            !actor->animator.IsActiveItem(), "consuming a grenade interrupted its throwing animation");
        int projectiles = 0;
        for (auto& object: *world->GetMiscObjects()) if (
            std::find(before.begin(),before.end(),object.GetPtr()) == before.end() &&
            pSSClasses->GetObjectTypeID(object.GetPtr()) == int(0x118B1160)) ++projectiles;
        Require(projectiles == 1, "grenade handoff failed to launch exactly one native projectile");
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(executor,&references);
        CObj<NRPG::IInventoryItem> pending;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(5,&pending); }
        Require(pending.GetPtr() == (!ai && spare ? next.GetPtr() : nullptr),
            "the grenade executor did not retain the correct human-only successor in saved field five");
        if (variant == 2) fixture.grenade->GetDBItem()->bPlaceInHand = false;
        if (variant == 4 || variant == 5) {
            fields.Seek(0);
            CObj<NWorld::CExecThrowGrenade> restored = new NWorld::CExecThrowGrenade;
            if (variant == 5) {
                inventory->Take(next);
                Require(inventory->Equip(NDb::SLOT_1,next), "legacy grenade fixture could not install its already-active successor");
                actor->animator.SetActiveItem(true);
                CVec3 target;
                NWorld::SGrenadeParams params;
                { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(2,&target); reader.Add(3,&params); }
                CPtr<NRPG::IGrenadeItem> missingGrenade;
                CObj<NRPG::IInventoryItem> noPending;
                bool updateVision = false;
                CMemoryStream legacy;
                { CStructureSaver writer(legacy,CStructureSaver::WRITE,&references);
                  writer.Add(1,static_cast<NWorld::CCommandExecute*>(executor.GetPtr())); writer.Add(2,&target);
                  writer.Add(3,&params); writer.Add(4,&missingGrenade); writer.Add(5,&noPending); writer.Add(6,&updateVision); }
                legacy.Seek(0);
                { CStructureSaver reader(legacy,CStructureSaver::READ,&references); static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
            } else {
                CStructureSaver reader(fields,CStructureSaver::READ,&references);
                static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);
            }
            executor = restored;
            Require(executor->GetUnitServer() == actor && executor->IsExecuting(),
                "restoring the post-label grenade handoff lost its native actor or action counter");
        }
        executor->AnimationFinished();
        Require(executor->GetState() == NWorld::CCommandExecute::FINISHED &&
            inventory->GetActive() == (!ai && spare ? next.GetPtr() : nullptr),
            "finishing the grenade animation failed to install only its human-controlled successor");
        Require(inventory->GetItems().size() == (ai && spare ? 1u : 0u) &&
            (!ai || inventory->GetItems().front().pItem == next.GetPtr()),
            "grenade completion duplicated a successor or consumed the AI's spare grenade");
        Require(actor->animator.IsActiveItem() == (variant == 5) && actor->GetAP() == ap &&
            actor->GetUnitRPG()->GetTotalVP() == vp && actor->HasCommand() == hadCommand &&
            fixture.grenade->GetQuantity() == 1 && (!next || next->GetQuantity() == 1),
            "grenade completion changed the animation flag, AP, VP, quantities or gameplay orders");
        executor = nullptr;
        Require(world->IsAction(), "the handoff discarded the live grenade's in-flight action counter");
    }
}

static void DefaultItemDetachment(const char* databasePath,const char* resourcePath)
{
    for (int operation: {1,0,2}) {
        NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false);
        auto* actor = fixture.actor;
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        CObj<NRPG::IInventoryItem> offhand = inventory->Get(NDb::SLOT_2) ? inventory->TakeOff(NDb::SLOT_2) : nullptr;
        if (operation == 1) {
            CObj<NRPG::IInventoryItem> item = inventory->TakeOff(NDb::SLOT_1);
            Require(inventory->Equip(NDb::SLOT_2,item) && inventory->Activate(NDb::SLOT_2),
                "default-detachment fixture could not select its second-slot item");
        }
        CObj<NRPG::CGrenadeItem> spare = new NRPG::CGrenadeItem(fixture.regular);
        spare->nQuantity = 1;
        CTPoint<int> place;
        Require(inventory->FindPlace(spare,&place) && inventory->Place(place,spare),
            "default-detachment fixture could not store its untouched spare");
        actor->animator.SetActiveItem(true);
        actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
        actor->animator.ThrowGrenade(actor->GetPosition(),actor->GetPosition().GetCP()+CVec3(1,0,0),0);
        const auto end = actor->animator.GetTimeEnd(), label = actor->animator.GetTimeLabel1();
        Require(label > 0 && !actor->animator.IsActiveItem(),
            "default-detachment fixture did not establish its native pending throw animation");
        const int ap = actor->GetAP(), vp = actor->GetUnitRPG()->GetTotalVP();
        if (operation == 0) {
            NWorld::CUnitServer::SResItem removed;
            Require(!actor->TearOffItem(&removed,NDb::SLOT_2) && !IsValid(removed.pItem),
                "detaching an empty native slot reported a modelled item");
            Require(inventory->GetActive() == fixture.grenade.GetPtr(),
                "detaching an empty slot changed the active inventory item");
        } else if (operation == 1) {
            actor->MakeUnconscious(VNULL3,true,false); // Public scripted fall, with its explicit death-clip suppression.
            Require(actor->IsUnconscious(), "default-detachment fixture did not enter native unconsciousness");
            Require(!inventory->Get(NDb::SLOT_1) && !inventory->Get(NDb::SLOT_2),
                "native hand-item dropping did not empty both equipment slots");
        } else {
            CObj<NWorld::CCommandExecute> lost = NWorld::CreateLostWeapon(actor,false);
            lost->Run();
            Require(lost->GetState() == NWorld::CCommandExecute::FINISHED && !inventory->GetActive() &&
                !actor->animator.IsActiveItem(), "native critical weapon loss failed to finish with an empty hand");
        }
        if (operation != 2) Require(actor->animator.GetTimeEnd() == end && actor->animator.GetTimeLabel1() == label &&
            !actor->animator.IsActiveItem(),
            "native default item detachment started an unrelated activation animation while processing equipment slots");
        vector<CPtr<NRPG::IInventoryItem>> ground;
        fixture.world->GetDebris()->GetGroundItemsForDiagnostics(&ground);
        const int dropped = int(std::count_if(ground.begin(),ground.end(),[&](const auto& item) {
            return item.GetPtr() == fixture.grenade.GetPtr();
        }));
        Require(dropped == (operation == 0 ? 0 : 1),
            "default detachment lost or duplicated its native falling inventory item");
        Require(inventory->GetItems().size() == 1 && inventory->GetItems().front().pItem == spare.GetPtr() &&
            spare->GetQuantity() == 1 && fixture.grenade->GetQuantity() == 1 && actor->GetAP() == ap &&
            actor->GetUnitRPG()->GetTotalVP() == vp && !actor->HasCommand(),
            "default detachment consumed a backpack spare, changed AP/VP/quantities or installed a gameplay order");
    }
}

static void HitRollEqualityLog(const char* databasePath,const char* resourcePath,bool tile)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NWorld::CUnitServer* target = nullptr;
    NRPG::IWeaponItem* weapon = nullptr;
    CObj<NRPG::CCoverInfo> cover;
    NRPG::STargetHLInfo hitLocation(NAI::HL_ANY);
    CVec3 tilePosition;
    auto check = [&](int* chance) {
        return tile ? NRPG::CheckTileToHit(actor,tilePosition,0,NAI::THL_LOWER,cover,world->IsFirstTurn(),chance) :
            NRPG::CheckToHit(actor,target,0,hitLocation.eHL,hitLocation.accessibleHLs,cover,world->IsFirstTurn(),chance);
    };
    for (auto& entry: units) {
        auto* candidate = entry.GetPtr();
        if (!candidate || !candidate->CanFight() || candidate->IsEmptyPK() || candidate->IsWearingPK() || candidate->HasCommand()) continue;
        auto* firearm = dynamic_cast<NRPG::IWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
        if (!firearm || firearm->GetAmmoQuantity() < 1 || !firearm->SetShootMode(NDb::SM_Snap)) continue;
        actor = candidate; weapon = firearm; cover = nullptr;
        vector<NRPG::CAttackPortion> attack;
        actor->GetUnitRPG()->CreateAttack(&attack,false);
        if (attack.empty()) continue;
        if (tile) {
            for (float distance: {2.f,4.f,8.f,12.f,20.f}) {
                tilePosition = actor->GetPosition().GetCP()+CVec3(distance,0,0.1f);
                cover = world->GetGame()->CalcCoversForTile(actor->GetAttackOrigin(actor->GetPosition()),attack.front(),actor,
                    tilePosition,actor->GetMinClearDistance());
                if (!cover || !NRPG::CanShoot(cover)) { cover = nullptr; continue; }
                int chance = 0; check(&chance);
                if (chance > 0 && chance < 100) break;
                cover = nullptr;
            }
        } else {
            for (auto& other: units) {
                target = other.GetPtr();
                if (!target || target == actor || !target->CanFight() || target->IsEmptyPK()) continue;
                if (!NRPG::SelectTargetHLs(actor,actor->GetPosition(),&hitLocation,target,NAI::HL_ANY)) continue;
                cover = world->GetGame()->CalcCovers(actor->GetAttackOrigin(actor->GetPosition()),attack.front(),actor,target,
                    hitLocation.eHL,actor->GetMinClearDistance());
                if (!cover || !NRPG::CanShoot(cover)) { cover = nullptr; continue; }
                int chance = 0; check(&chance);
                if (chance > 0 && chance < 100) break;
                cover = nullptr;
            }
        }
        if (cover) break;
    }
    Require(actor && weapon && cover, "hit-roll log fixture found no actual shootable cover with a nondegenerate native hit chance");
    class Capture: public ILogNotify {
        OBJECT_NOCOPY_METHODS(Capture);
    public:
        wstring last;
        void OnAddConsoleLine(const SConsoleLine& line) override {
            if (line.eType == STREAM_RPG && line.szText.find(L"\tCheck = ") != wstring::npos) last = line.szText;
        }
    };
    CObj<Capture> log = new Capture;
    SetLogNotify(log);
    struct RestoreNotify { ~RestoreNotify() { SetLogNotify(nullptr); } } restore;
    const int ap = actor->GetAP(), ammo = weapon->GetAmmoQuantity(), vp = actor->GetUnitRPG()->GetTotalVP();
    bool below = false, equal = false, above = false;
    for (int draw = 0; draw < 4096 && !(below && equal && above); ++draw) {
        log->last.clear();
        int chance = 0;
        const float ratio = check(&chance);
        const auto& line = log->last;
        const auto checkStart = line.find(L"\tCheck = ");
        Require(checkStart != wstring::npos && line.find(tile ? L"TileToHit = " : L"ToHit = ") != wstring::npos,
            "native hit check did not publish its actual roll and chance diagnostic");
        const int roll = int(std::wcstol(line.c_str()+checkStart+9,nullptr,10));
        const bool loggedHit = line.find(L"Hit: <green>true<white>") != wstring::npos;
        Require(loggedHit || line.find(L"Hit: <red>false<white>") != wstring::npos,
            "native hit check diagnostic has no boolean outcome");
        Require(chance > 0 && chance < 100 && roll >= 0 && roll <= 100,
            "native hit-roll fixture lost its nondegenerate authored chance or legal random draw");
        if (roll < chance) below = true;
        else if (roll > chance) above = true;
        else {
            equal = true;
            std::printf("native hit equality: tile=%d chance=%d roll=%d ratio=%.2f\n",int(tile),chance,roll,double(ratio));
            Require(ratio == 1.f && loggedHit,
                "a native hit-roll equality returned the hit ratio but was logged as a miss");
            CRay chosen;
            bool missed = true;
            Require(NRPG::PeekRay(cover,&chosen,ratio,&missed) && !missed,
                "the actual native equality ray gate did not accept its hit-ratio control");
        }
        Require(loggedHit == (ratio >= 1.f), "a native hit log disagreed with its actual ratio outside the equality boundary");
    }
    Require(below && equal && above, "fixed-seed native hit checks did not cover both sides and equality of the boundary");
    Require(actor->GetAP() == ap && actor->GetUnitRPG()->GetTotalVP() == vp && weapon->GetAmmoQuantity() == ammo &&
        !actor->HasCommand(), "checking hit-roll diagnostics changed AP/VP/ammunition or issued a gameplay order");
}


// Native world/render/head-controller code runs unchanged; this sink records CPU
// head animators and retains their inputs without creating GPU resources.
class HeadStateView: public NGScene::GameViewFixture
{
    OBJECT_NOCOPY_METHODS(HeadStateView);
    vector<CObj<CObjectBase>> inputs;
    template<class T> void Keep(T* object) { if (object) inputs.push_back(object); }
public:
    std::map<const NLSHead::CHeadInfo*,NLSHead::CHeadAnimator*> heads;
    NGScene::CLightGroup* CreateLightGroup() override { return nullptr; }
    CObjectBase* CreateSkin(NDb::CModel*,CFuncBase<NAnimation::SSkeletonPose>* pose,const NGScene::SFullRoomInfo&) override { Keep(pose); return nullptr; }
    CObjectBase* CreateMesh(NDb::CModel*,CFuncBase<SFBTransform>* pose,const NGScene::SFullRoomInfo&) override { Keep(pose); return nullptr; }
    CObjectBase* CreateMesh(NDb::CModel*,const SFBTransform&,const NGScene::SFullRoomInfo&) override { return nullptr; }
    CObjectBase* CreateOccluder(NDb::CAIGeometry*,const SFBTransform&,const NGScene::SRoomInfo&) override { return nullptr; }
    CObjectBase* CreateOccluder(NDb::CAIGeometry*,NDb::CSkeleton*,CFuncBase<NAnimation::SSkeletonPose>* pose,const NGScene::SRoomInfo&) override { Keep(pose); return nullptr; }
    CObjectBase* CreateTerrainRegion(CFuncBase<STerrainInfo>* info,CVersioningBase*,const SRandomSeed&,const CTRect<int>&,const list<CObj<CPtrFuncBase<CTerrainPart>>>&,NGScene::CGrassTracker*,const NGScene::SFullRoomInfo&) override { Keep(info); return nullptr; }
    CObjectBase* CreateTerrainWall(CPtrFuncBase<CTerrainPart>* part,NDb::CTexture*,const NGScene::SFullRoomInfo&) override { Keep(part); return nullptr; }
    CObjectBase* CreateGrassSector(NGScene::CGrassAnimator*,NDb::CTexture*,CFuncBase<SFBTransform>* pose,const SBound&,const NGScene::SRoomInfo&) override { Keep(pose); return nullptr; }
    NGScene::CBuilding* CreateBuildingPart(int,const SMapBuilding&,NBuilding::CBuildingInfoHold*) override { return nullptr; }
    CObjectBase* AddDirectionalLight(const CVec3&,const CVec3&,const CVec3&,const CVec2&,float,bool) override { return nullptr; }
    CObjectBase* AddPointLight(const CVec3&,const CVec3&,float,bool) override { return nullptr; }
    CObjectBase* AddSpotLight(const CVec3&,const CVec3&,const CVec3&,float,float,NDb::CTexture*,bool) override { return nullptr; }
    CObjectBase* AddFlare(CFuncBase<CVec3>* position,CFuncBase<STime>* time,int,float,NDb::CTexture*,float,float) override { Keep(position); Keep(time); return nullptr; }
    CObjectBase* CreateParticles(NDb::CEffect*,STime,CFuncBase<STime>* time,CFuncBase<SFBTransform>* pose,const NGScene::SRoomInfo&,NAnimation::CSkeletonAnimator* skeleton,NGScene::IParticleFilter* filter) override { Keep(time); Keep(pose); Keep(skeleton); Keep(filter); return nullptr; }
    CObjectBase* CreateParticles(NDb::CEffect*,STime,CFuncBase<STime>* time,const SFBTransform&,const NGScene::SRoomInfo&,NGScene::IParticleFilter* filter) override { Keep(time); Keep(filter); return nullptr; }
    CObjectBase* Precache(NDb::CModel*) override { return nullptr; }
    void SetFogBaseHeight(float) override {}
    CObjectBase* CreateLSHead(NDb::CComplexHead*,CPtrFuncBase<NGScene::CObjectInfo>* animator,CFuncBase<STime>* time,
        CFuncBase<SFBTransform>* pose,bool,const SRandomSeed&,bool,const NGScene::SRoomInfo&,CPtrFuncBase<NGfx::CTexture>*,const NLSHead::CHeadInfo* head) override {
        auto* native = dynamic_cast<NLSHead::CHeadAnimator*>(animator);
        Require(native && head,"downed-head fixture did not receive the native shared head animator");
        heads[head] = native; Keep(animator); Keep(time); Keep(pose);
        return nullptr;
    }
};

static void DownedHeadIdle(const char* databasePath,const char* resourcePath,bool dead)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && unit->IsAddedToVisitor()
                && unit->GetDBHead() && unit->GetDBHead()->pHead) { actor = unit; break; }
    }
    Require(actor != nullptr,"downed-head fixture has no ordinary live actor with a facial head");
    CObj<HeadStateView> view = new HeadStateView;
    CObj<NRender::IRenderGame> render = NRender::CreateRenderGame(world,view,nullptr);
    auto update = [&]() { render->UpdateViewWorld(false,world->GetTime()->GetValue(),nullptr,true); };
    update();
    auto* head = actor->GetHeadInfo();
    Require(view->heads.count(head) == 1,"native world render did not visit the actor's head");
    auto* animator = view->heads.at(head);
    Require(animator == render->GetHeadController()->GetAnimator(head),"world render and controller did not share the native head");
    CObj<NRender::IShowUnit> portrait = NRender::CreateShowUnit(view,static_cast<NWorld::CUnit*>(actor),world->GetTime(),render,false,true,true);
    portrait->Update(0);
    Require(view->heads.at(head) == animator && animator->GetIdleType() == NLSHead::IDLE_NORMAL,
        "living native portrait did not arm facial idle on the shared world head");
    const auto position = actor->GetPosition();
    if (dead) actor->KillUnit(VNULL3);
    else actor->MakeUnconscious(VNULL3,true,false);
    Require(!actor->CanFight() && (dead ? actor->IsDead() : actor->IsUnconscious()),
        "downed-head fixture did not enter its actual native incapacitated state");
    const int vp = actor->GetUnitRPG()->GetTotalVP(), ap = actor->GetAP();
    update();
    std::printf("native %s head idle=%d\n",dead ? "dead" : "unconscious",int(animator->GetIdleType()));
    Require(animator->GetIdleType() == NLSHead::IDLE_DEATH,
        "a downed native world unit left its shared portrait head in living facial idle");
    portrait->Update(0);
    Require(animator->GetIdleType() == NLSHead::IDLE_DEATH,"refreshing a downed native portrait restarted living facial idle");
    portrait = nullptr;
    Require(animator->GetIdleType() == NLSHead::IDLE_DEATH,"removing a downed portrait reset the frozen facial idle");
    Require(actor->GetUnitRPG()->GetTotalVP() == vp && actor->GetAP() == ap &&
        actor->GetPosition().pos.p == position.pos.p && !actor->CanFight(),
        "refreshing downed head presentation changed health, AP, position or combat eligibility");
}


static NAI::SSourceInfo NativeHullSource(CObjectBase* hull)
{
    CStructureNetworkContext refs;
    auto fields = BorrowedObjectFields(hull,&refs);
    NAI::SSourceInfo source(nullptr,nullptr,0,0);
    { CStructureSaver reader(fields,CStructureSaver::READ,&refs);
      const int type = pSSClasses->GetObjectTypeID(hull);
      if (type == int(0x02942161)) {
          struct BaseFields { NAI::SSourceInfo* source; int operator&(CStructureSaver& saver) { saver.Add(6,source); return 0; } } base{&source};
          reader.Add(1,&base);
      } else {
          Require(type == int(0x02942160),"active-hull fixture encountered an unsupported native hull save layout");
          reader.Add(6,&source);
      }
    }
    return source;
}

static void ActiveDoorHullLookup(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    auto* map = world->GetAIMap(); map->Sync();
    NWorld::CWindowDoor* door = nullptr;
    for (const auto& entry: world->GetObjectsForHarness()) {
        auto* candidate = dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr());
        if (!candidate || candidate->IsBroken() || candidate->IsMineSet()) continue;
        auto* hull = map->GetHull(candidate);
        if (hull && (NativeHullSource(hull).nTSFlags & (NWorld::TS_STATE_OPEN | NWorld::TS_STATE_CLOSED))) {
            door = candidate; break;
        }
    }
    Require(door != nullptr,"active-hull fixture has no authored flipping door with registered AI geometry");
    CObj<RetiredSupportFixture> unregistered = new RetiredSupportFixture;
    Require(!map->GetHull(unregistered),"unregistered-object control unexpectedly resolved an AI hull");
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& unit: units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (candidate && candidate->CanFight() && map->GetHull(candidate)) { actor = candidate; break; }
    }
    Require(actor != nullptr,"active-hull fixture has no ordinary registered unit control");
    const int ap = actor->GetAP(), vp = actor->GetUnitRPG()->GetTotalVP();
    auto* ordinary = map->GetHull(actor);
    Require(NativeHullSource(ordinary).pUserData == actor &&
        !(NativeHullSource(ordinary).nTSFlags & (NWorld::TS_STATE_OPEN | NWorld::TS_STATE_CLOSED)),
        "ordinary-unit control unexpectedly used a door-state hull");
    door->LockDoor(false,0,0);
    for (bool open: {false,true,false}) {
        door->OpenClose(open,true); map->Sync();
        Require(door->IsOpen() == open,"active-hull fixture did not enter its real native door state");
        const bool action = world->IsAction();
        auto* hull = map->GetHull(door);
        Require(hull != nullptr,"a live native door lost all registered AI hulls");
        const auto source = NativeHullSource(hull);
        Require(source.pUserData == door,"native object lookup returned another object's AI hull");
        const int state = source.nTSFlags & (NWorld::TS_STATE_OPEN | NWorld::TS_STATE_CLOSED);
        std::printf("native door open=%d lookup flags=0x%x\n",int(open),source.nTSFlags);
        Require((source.nTSFlags & NWorld::TS_DOOR_HULL_VALID) && state == (open ? NWorld::TS_STATE_OPEN : NWorld::TS_STATE_CLOSED),
            "native object lookup returned an inactive door-pose hull");
        SBound area; area.SphereInit(VNULL3,10000.f);
        vector<CPtr<CObjectBase>> active; map->SelectHullPointers(&active,area,~0,0);
        Require(std::find(active.begin(),active.end(),hull) != active.end(),
            "the lookup hull disagreed with native active spatial selection");
        CVec3 point;
        Require(map->GetUnitHLPos(&point,hull,-1) && std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z),
            "the active native door hull did not provide a finite hit-location point");
        Require(door->IsOpen() == open && world->IsAction() == action && actor->GetAP() == ap &&
            actor->GetUnitRPG()->GetTotalVP() == vp && map->GetHull(actor) == ordinary && !map->GetHull(unregistered),
            "querying active door geometry changed door state, action bookkeeping, unit health/AP or ordinary lookups");
    }
}

static void ProjectiveRayUnprojection()
{
    float worst = 0;
    for (const auto& origin: {CVec3(0,0,0),CVec3(12.5f,31.25f,1.8f),CVec3(0,80,2),CVec3(-60,90,12),CVec3(175,-125,30)})
        for (const auto& direction: {CVec3(1,0,0.15f),CVec3(-0.7f,0.8f,-0.2f)})
            for (float distance: {3.f,15.f,40.f}) {
                CVec3 forward = direction;
                Normalize(&forward);
                const CVec3 target = origin+forward*distance;
                NAI::CFastRenderer renderer;
                renderer.InitProjective(origin,target,0.625f,16);
                float targetX = 0, targetY = 0;
                renderer.GetCoordsClamped(target,&targetX,&targetY);
                Require(fabs(targetX-15.5f) < 0.5f && fabs(targetY-15.5f) < 0.5f,
                    "projective-ray fixture did not put its actual target at the centre of the native projection");
                for (const auto& pixel: {CVec2(16,16),CVec2(8,13),CVec2(23,20)}) {
                    CVec3 ray;
                    renderer.GetDir(&ray,pixel.x,pixel.y);
                    Require(std::isfinite(ray.x) && std::isfinite(ray.y) && std::isfinite(ray.z) && fabs(fabs2(ray)-1.f) < 1e-5f,
                        "native projective unprojection returned a nonfinite or unnormalised ray");
                    float x = 0, y = 0;
                    renderer.GetCoordsClamped(origin+ray*distance,&x,&y);
                    // GetPoints consumes the public native trace-interval record. A unit-distance
                    // interval describes the same sample point from which GetDir derives its ray.
                    NAI::CFastRenderer::SResult interval{};
                    interval.fEnter = interval.fExit = 1.f;
                    const int ix = int(pixel.x), iy = int(pixel.y);
                    renderer.resGrid[iy][ix] = &interval;
                    vector<CVec3> entries, exits;
                    renderer.GetPoints(&entries,&exits,ix,iy);
                    renderer.resGrid[iy][ix] = nullptr;
                    Require(entries.size() == 1 && exits.size() == 1 && entries.front() == exits.front(),
                        "native projective point conversion did not consume its unit-distance interval");
                    CVec3 expected = entries.front()-origin;
                    Normalize(&expected);
                    const float error = fabs(ray-expected);
                    worst = Max(worst,error);
                    if (error > 1e-6f) std::printf("native projective ray origin=(%.2f,%.2f,%.2f) distance=%.2f pixel=(%.2f,%.2f) projected=(%.5f,%.5f) point_direction_error=%.8f\n",
                        double(origin.x),double(origin.y),double(origin.z),double(distance),double(pixel.x),double(pixel.y),double(x),double(y),double(error));
                    Require(error <= 1e-6f,
                        "a translated native projective ray disagreed with the renderer's homogeneous point conversion");
                }
            }
    std::printf("native projective ray worst unit-direction error=%.7f\n",double(worst));
}

static void ShootingTorsoTurnLimit(const char* databasePath,const char* resourcePath)
{
    for (float relative: {0.5f,2.4f,-2.4f,0.f}) {
        CObj<NRPG::CGlobalGame> game;
        auto world = FixtureWorld(databasePath,resourcePath,false,&game);
        vector<CPtr<NWorld::CUnit>> units;
        world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr;
        NRPG::IWeaponItem* weapon = nullptr;
        for (auto& entry: units) {
            auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
            if (!unit || !unit->CanFight() || unit->IsEmptyPK() || unit->IsWearingPK() || unit->HasCommand() ||
                NWorld::GetActionType(unit) != NWorld::AT_SHOOT || unit->animator.GetCannon()) continue;
            auto* equipped = dynamic_cast<NRPG::IWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
            if (equipped && equipped->GetAmmoQuantity() > 0) { actor = unit; weapon = equipped; break; }
        }
        Require(actor && weapon, "torso-turn fixture has no idle native firearm actor with ammunition");
        const auto position = actor->GetPosition();
        const int ap = actor->GetAP(), vp = actor->GetUnitRPG()->GetTotalVP(), ammo = weapon->GetAmmoQuantity();
        actor->animator.SetActiveItem(true);
        actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
        actor->animator.PlaceUnit(position);
        CRay ray;
        ray.ptOrigin = position.GetEyePosition();
        ray.ptDir = CVec3(cos(position.GetDirection()+relative),sin(position.GetDirection()+relative),0);
        const auto requested = ray;
        actor->animator.Attack(position,ray);
        auto* animator = actor->animator.GetSkeletonAnimator();
        Require(animator && actor->animator.GetTimeLabel1() > 0,
            "torso-turn fixture did not create its native attack animation");
        vector<NAnimation::CATurnBody*> turns;
        vector<CObjectBase*> pending{animator}, visited;
        while (!pending.empty()) {
            auto* object = pending.back(); pending.pop_back();
            if (std::find(visited.begin(),visited.end(),object) != visited.end()) continue;
            visited.push_back(object);
            if (auto* turn = dynamic_cast<NAnimation::CATurnBody*>(object)) turns.push_back(turn);
            CStructureNetworkContext refs;
            auto fields = BorrowedObjectFields(object,&refs);
            for (const auto& ref: refs.refs)
                if (dynamic_cast<NAnimation::CAnimator*>(ref.second.GetPtr())) pending.push_back(ref.second.GetPtr());
        }
        Require(turns.size() == 1 && turns.front()->nBoneSpine >= 0 && turns.front()->nBoneChest >= 0,
            "torso-turn fixture has no single native spine/chest turn in its actual animation graph");
        CStructureNetworkContext refs;
        auto fields = BorrowedObjectFields(turns.front(),&refs);
        float angle = 100;
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(4,&angle); }
        int bones = 0;
        CStructureNetworkContext skeletonRefs;
        auto skeletonFields = BorrowedObjectFields(animator,&skeletonRefs);
        { CStructureSaver reader(skeletonFields,CStructureSaver::READ,&skeletonRefs); reader.Add(3,&bones); }
        Require(bones > turns.front()->nBoneSpine && bones > turns.front()->nBoneChest,
            "torso-turn fixture has invalid native skeleton bone indices");
        NAnimation::SSkeletonPose frame(bones);
        turns.front()->GetFrame(actor->animator.GetTimeLabel1(),&frame);
        Require(frame.size() == size_t(bones) && std::isfinite(frame[turns.front()->nBoneSpine].pos.x) &&
            std::isfinite(frame[turns.front()->nBoneChest].pos.x),
            "the native shooting torso graph did not evaluate a finite skeleton frame");
        std::printf("native shooting torso requested=%.2f actual=%.2f\n",double(relative),double(angle));
        const float expected = relative > 1.2f ? 1.2f : (relative < -1.2f ? -1.2f : relative);
        Require(fabs(angle-expected) < 1e-5f,
            "the native shooting animation twisted the torso past its allowed horizontal angle");
        CMemoryStream animatorFields;
        CStructureNetworkContext animatorRefs;
        animatorRefs.include = [](CObjectBase*) { return false; };
        { CStructureSaver writer(animatorFields,CStructureSaver::WRITE,&animatorRefs); actor->animator.operator&(writer); }
        animatorFields.Seek(0);
        CVec3 aim;
        { CStructureSaver reader(animatorFields,CStructureSaver::READ,&animatorRefs); reader.Add(21,&aim); }
        Require(fabs2(aim-requested.ptDir) < 1e-10f && ray.ptOrigin == requested.ptOrigin && ray.ptDir == requested.ptDir &&
            actor->GetPosition() == position && actor->GetAP() == ap && actor->GetUnitRPG()->GetTotalVP() == vp &&
            weapon->GetAmmoQuantity() == ammo && !actor->HasCommand(),
            "clamping the shooting torso changed the actual aim, shot ray, unit facing, AP/VP/ammunition or orders");
    }
}

static void GrenadeThrowAnimationRefresh(const char* databasePath,const char* resourcePath)
{
    NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false);
    auto* actor = fixture.actor;
    CDBPtr<NDb::CSkeleton> skeleton;
    CObj<NAnimation::CSkeletonAnimator> animator = actor->animator.GetSkeletonAnimator();
    CDBPtr<NDb::CSide> side;
    int poseFlags = 0, classFlags = 0;
    actor->animator.PlaceUnit(actor->GetPosition());
    {
        CMemoryStream fields;
        CStructureNetworkContext refs;
        refs.include = [](CObjectBase*) { return false; };
        { CStructureSaver writer(fields,CStructureSaver::WRITE,&refs); actor->animator.operator&(writer); }
        fields.Seek(0);
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs);
          reader.Add(4,&poseFlags); reader.Add(5,&classFlags); reader.Add(31,&skeleton);
          reader.Add(40,&side); }
    }
    Require(IsValid(skeleton) && IsValid(animator), "grenade animation fixture lost its native skeleton or animator");
    const int itemFlags = NDb::WeaponTypeToAnimFlags(actor->GetUnitRPG()->GetWeaponType(),true,false);
    const int expectedFlags = (poseFlags & 7) | itemFlags;
    auto* expected = skeleton->GetAnimation(NDb::CAnimation::USE,expectedFlags,"F1",classFlags,side);
    Require(IsValid(expected) && (itemFlags & NDb::CAnimation::WEAPON_ITEM),
        "grenade animation fixture has no authored item-specific forward throw clip");
    auto clips = [&]() {
        vector<NAnimation::CAnimation*> result;
        vector<CObjectBase*> pending{animator.GetPtr()}, visited;
        while (!pending.empty()) {
            auto* object = pending.back(); pending.pop_back();
            if (std::find(visited.begin(),visited.end(),object) != visited.end()) continue;
            visited.push_back(object);
            if (auto* clip = dynamic_cast<NAnimation::CAnimation*>(object)) result.push_back(clip);
            CStructureNetworkContext refs;
            auto fields = BorrowedObjectFields(object,&refs);
            for (const auto& ref: refs.refs)
                if (dynamic_cast<NAnimation::CAnimator*>(ref.second.GetPtr())) pending.push_back(ref.second.GetPtr());
        }
        return result;
    };
    // Preserve the native unarmed fallback but remove only the held-item POSE eligibility.
    struct RestorePoses {
        vector<std::pair<NDb::CAnimation*,int>> saved;
        ~RestorePoses() { for (auto& entry: saved) entry.first->nPoseWeaponFlags = entry.second; }
    } poses;
    for (auto& entry: skeleton->pAnimations[NDb::CAnimation::POSE].anims)
        if (entry->nPoseWeaponFlags & itemFlags) {
            poses.saved.emplace_back(entry.GetPtr(),entry->nPoseWeaponFlags);
            entry->nPoseWeaponFlags &= ~itemFlags;
        }
    Require(!poses.saved.empty() && !skeleton->GetAnimation(NDb::CAnimation::POSE,expectedFlags,0,classFlags,side) &&
        IsValid(skeleton->GetAnimation(NDb::CAnimation::POSE,(poseFlags & 7) | NDb::CAnimation::WEAPON_NONE,0,classFlags,side)),
        "grenade animation fixture did not preserve an unarmed POSE fallback without a held-item POSE");
    const int ap = actor->GetAP();
    auto* equipped = actor->GetUnitRPG()->GetInventory()->GetActive();
    auto isThrowClip = [&](NAnimation::CAnimation* clip) {
        CStructureNetworkContext refs;
        auto fields = BorrowedObjectFields(clip,&refs);
        CDBPtr<NDb::CAnimation> record;
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(4,&record); }
        return IsValid(record) && record->nType == NDb::CAnimation::USE &&
            record->szParams == "F1" && (record->nPoseWeaponFlags & itemFlags) == itemFlags;
    };
    const auto before = clips();
    Require(std::none_of(before.begin(),before.end(),isThrowClip),
        "grenade animation fixture already contained a throw clip before its first throw");
    actor->animator.ThrowGrenade(actor->GetPosition(),actor->GetPosition().GetCP()+CVec3(1,0,0),0);
    const auto after = clips();
    const bool usedThrow = std::any_of(after.begin(),after.end(),isThrowClip);
    Require(usedThrow, "a grenade throw after the native unarmed POSE fallback omitted its authored held-item throw animation");
    Require(!actor->animator.IsActiveItem() && actor->GetAP() == ap &&
        actor->GetUnitRPG()->GetInventory()->GetActive() == equipped,
        "refreshing the grenade animation changed item ownership, AP or the post-throw active flag");
}

static void InvalidatedGrenadeTrajectory(const char* databasePath,const char* resourcePath)
{
    for (bool engineer: {false,true}) {
        NativeGrenadeThrowFixture fixture(databasePath,resourcePath,engineer);
        auto& world = fixture.world;
        auto* actor = fixture.actor;
        auto& executor = fixture.executor;
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        executor->Run();
        Require(executor->IsExecuting() && executor->GetState() == NWorld::CCommandExecute::RUNNING,
            "invalidated-trajectory fixture did not start its native throw");
        const auto initialPosition = actor->GetPosition();
        auto* network = world->GetPathNetwork();
        vector<NAI::SPathPlace> places;
        network->GetNearPlaces(SSphere(initialPosition.GetCP(),100.f),&places);
        std::sort(places.begin(),places.end(),[&](const NAI::SPathPlace& a,const NAI::SPathPlace& b) {
            return fabs2(NAI::SPosition(a,network).GetCP()-initialPosition.GetCP()) >
                fabs2(NAI::SPosition(b,network).GetCP()-initialPosition.GetCP());
        });
        bool moved = false;
        for (auto place: places) {
            place.SetPose(NAI::CM_STAND); place.SetDirection(0);
            if (!network->IsPassable(place) || network->IsLocked(place,true)) continue;
            auto destination = initialPosition;
            destination.pos.p = place;
            if (executor->CanDoIt(destination) != NWorld::UCR_TARGET_OUT_OF_RANGE) continue;
            actor->SetPosition(destination); // Native relocation to a real free map tile during the pending windup.
            if (executor->CanDoIt(actor->GetPosition()) == NWorld::UCR_TARGET_OUT_OF_RANGE) {
                moved = true; break;
            }
            actor->SetPosition(initialPosition);
        }
        Require(moved, "invalidated-trajectory fixture found no real free map tile blocking the pending throw");
        const int ap = actor->GetAP(), vp = actor->GetUnitRPG()->GetTotalVP();
        vector<NWorld::IDynamicObject*> before;
        for (auto& object: *world->GetMiscObjects()) before.push_back(object.GetPtr());
        std::printf("invalidated native grenade trajectory: engineer=%d\n",int(engineer));
        std::fflush(stdout);
        Require(!executor->TimeLabelReached(), "an unavailable grenade trajectory requested an extra release label");
        int projectiles = 0;
        for (auto& object: *world->GetMiscObjects())
            if (std::find(before.begin(),before.end(),object.GetPtr()) == before.end() &&
                pSSClasses->GetObjectTypeID(object.GetPtr()) == int(0x118B1160)) ++projectiles;
        Require(projectiles == 0 && inventory->GetActive() == fixture.grenade.GetPtr(),
            "releasing a grenade after its native trajectory became unavailable consumed the item and launched stale parameters");
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(executor,&references);
        CObj<NRPG::IInventoryItem> pending;
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(5,&pending); }
        Require(!pending, "an unavailable grenade release reserved a replacement item");
        executor->AnimationFinished();
        Require(executor->GetState() == NWorld::CCommandExecute::FINISHED && actor->GetAP() == ap &&
            actor->GetUnitRPG()->GetTotalVP() == vp && fixture.grenade->GetQuantity() == 1 &&
            inventory->GetActive() == fixture.grenade.GetPtr(),
            "finishing the unavailable grenade release changed AP, VP, quantity or the equipped grenade");
    }
}

static void GrenadeCompletionVisibility(const char* databasePath,const char* resourcePath)
{
    for (bool requested: {false,true}) {
        NativeGrenadeThrowFixture fixture(databasePath,resourcePath,false);
        auto& world = fixture.world;
        auto* actor = fixture.actor;
        auto& executor = fixture.executor;
        executor->Run();
        executor->TimeLabelReached();
        const int ap = actor->GetAP();
        const bool hidden = actor->IsHiding();
        CStructureNetworkContext references;
        CMemoryStream overrideFields;
        { CStructureSaver writer(overrideFields,CStructureSaver::WRITE,&references); writer.Add(6,&requested); }
        overrideFields.Seek(0);
        { CStructureSaver reader(overrideFields,CStructureSaver::READ,&references); static_cast<CObjectBase*>(executor.GetPtr())->operator&(reader); }
        auto saved = BorrowedObjectFields(executor,&references);
        CObj<NWorld::CExecThrowGrenade> restored = new NWorld::CExecThrowGrenade;
        { CStructureSaver reader(saved,CStructureSaver::READ,&references); static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader); }
        executor = restored;
        auto setBatch = [&](bool delay,bool queued) {
            CMemoryStream fields;
            CStructureNetworkContext context;
            { CStructureSaver writer(fields,CStructureSaver::WRITE,&context); writer.Add(53,&delay); writer.Add(54,&queued); }
            fields.Seek(0);
            { CStructureSaver reader(fields,CStructureSaver::READ,&context); static_cast<CObjectBase*>(world.GetPtr())->operator&(reader); }
        };
        auto batch = [&]() {
            CStructureNetworkContext context;
            auto fields = BorrowedObjectFields(world,&context);
            bool delay = false, queued = false;
            { CStructureSaver reader(fields,CStructureSaver::READ,&context); reader.Add(53,&delay); reader.Add(54,&queued); }
            return std::make_pair(delay,queued);
        };
        const auto originalBatch = batch();
        setBatch(true,false); // The native state while a world Segment batches visibility requests.
        Require(batch() == std::make_pair(true,false), "grenade visibility fixture did not establish the native deferred world state");
        executor->AnimationFinished();
        const auto result = batch();
        setBatch(originalBatch.first,originalBatch.second);
        Require(result == std::make_pair(true,requested),
            "finishing a restored grenade action discarded its requested deferred visibility refresh");
        Require(executor->GetState() == NWorld::CCommandExecute::FINISHED && actor->IsHiding() == hidden &&
            actor->GetAP() == ap, "honouring the saved visibility request changed concealment, AP or completion state");
    }
}

static void AssignedLevelExperience(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NDb::CRPGPers* persona = nullptr;
    CDBIterator<NDb::CRPGPers> rows(*NDatabase::GetTable<NDb::CRPGPers>());
    while (rows.MoveNext()) {
        auto* candidate = rows.Get();
        if (IsValid(candidate->pClass) && IsValid(candidate->pBaseValue) &&
            IsValid(candidate->pModel) && IsValid(candidate->pName) && candidate->pClass->skills[NDb::ST_LEVEL] > 0) {
            persona = candidate; break;
        }
    }
    Require(persona != nullptr, "assigned-level fixture has no authored persona with a positive level cap");
    CObj<NRPG::CUnit> unit = new NRPG::CUnit(persona);
    const auto name = unit->GetName();
    auto* inventory = unit->GetInventory();
    for (int requested: {3,5,3}) {
        const float expected = unit->GetXPForSkill(NDb::ST_LEVEL,requested);
        Require(expected > 0, "assigned-level fixture has no positive authored XP threshold");
        unit->SetXPLevel(requested);
        Require(unit->fXP == expected,
            "assigning a native unit level updated skill XP but discarded the corresponding total experience");
        CMemoryStream fields;
        CStructureNetworkContext refs;
        refs.include = [](CObjectBase*) { return false; };
        { CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);
          static_cast<NRPG::CSkilledObject*>(unit.GetPtr())->operator&(writer); }
        fields.Seek(0);
        float saved = -1;
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(2,&saved); }
        Require(saved == expected, "the assigned total experience did not reach the native skilled-object saved field");
        unit->AddXP(0);
        Require(unit->fXP == expected, "refreshing assigned experience discarded the native level threshold");
        unit->AddXP(1);
        Require(unit->fXP > expected, "the first earned experience after level assignment started from an obsolete total");
        Require(unit->GetName() == name && unit->GetInventory() == inventory,
            "persisting assigned-level experience changed persona identity or inventory ownership");
    }
}

static void PersonaFullDisplayName(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NDb::CRPGPers* persona = nullptr;
    CDBIterator<NDb::CRPGPers> rows(*NDatabase::GetTable<NDb::CRPGPers>());
    while (rows.MoveNext()) {
        auto* record = rows.Get();
        if (IsValid(record->pName) && IsValid(record->pLongName) && IsValid(record->pModel) &&
            IsValid(record->pBaseValue) && !record->pLongName->szStr.empty() &&
            record->pLongName->szStr != record->pName->szStr) { persona = record; break; }
    }
    Require(persona != nullptr, "full-name fixture has no authored persona with a distinct full name");
    CObj<NRPG::CUnit> unit = new NRPG::CUnit(persona);
    Require(unit->GetName() == persona->pName->szStr, "creating a full display name changed the persona nickname");
    Require(unit->wsFullName == persona->pLongName->szStr,
        "creating an RPG unit discarded its authored full display name");
    CStructureNetworkContext references;
    auto fields = BorrowedObjectFields(unit,&references);
    wstring saved;
    { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(4,&saved); }
    Require(saved == persona->pLongName->szStr, "the authored full name did not reach the existing saved field");
    CPtr<NDb::CString> longName = persona->pLongName;
    persona->pLongName = nullptr;
    CObj<NRPG::CUnit> unnamed = new NRPG::CUnit(persona);
    persona->pLongName = longName;
    Require(unnamed->wsFullName.empty() && unnamed->GetName() == unit->GetName(),
        "a persona without a long-name record changed the existing nickname or missing-name behavior");
}

static void EquippedMeleeAttackCursor(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK() &&
            !unit->HasCommand() && NWorld::GetActionType(unit) == NWorld::AT_SHOOT) { actor = unit; break; }
    }
    Require(actor != nullptr, "equipped-melee cursor fixture has no idle ordinary firearm user");
    NDb::CRPGMeleeWeapon* record = nullptr;
    CDBIterator<NDb::CRPGMeleeWeapon> weapons(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
    while (weapons.MoveNext()) if (!weapons.Get()->bThrowing && IsValid(weapons.Get()->pItem) &&
        IsValid(weapons.Get()->pItem->pModel)) { record = weapons.Get(); break; }
    Require(record != nullptr, "equipped-melee cursor fixture has no modelled authored nonthrowing melee weapon");
    CObj<NRPG::CMeleeWeaponItem> melee = new NRPG::CMeleeWeaponItem(record);
    melee->nQuantity = 1;
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    CObj<NRPG::IInventoryItem> previous = inventory->Get(NDb::SLOT_1) ? inventory->TakeOff(NDb::SLOT_1) : nullptr;
    Require(inventory->Activate(NDb::SLOT_1), "equipped-melee cursor fixture could not select its empty hand");
    CObj<AttackCursorMission> mission = new AttackCursorMission;
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission,actor);
    mission->selection.push_back(tracker.GetPtr());
    CObj<AttackState> state = new AttackState;
    state->Attach(mission);
    NGame::SActionInfo info; info.SetValid(); SetAttackPreview(state,info);
    NInput::SEvent event;
    while (NInput::GetEvent(&event)) {}
    state->RefreshCursor();
    Require(actor->GetState() == NWorld::CUnit::ST_NORMAL_MELEE && state->GetCursorInfo().pCursor == NDb::GetUICursor(8),
        "empty-hand native control did not advertise the original fist cursor");
    NWorld::CCriticalsBan bans;
    CObj<NWorld::CCmdShootObject> object = new NWorld::CCmdShootObject(nullptr,0);
    CObj<NWorld::CCmdShootTile> tile = new NWorld::CCmdShootTile(actor->GetPosition().GetCP()+CVec3(1,0,0));
    const auto objectBans = bans.GetCommandBans(actor,object), tileBans = bans.GetCommandBans(actor,tile);
    Require(inventory->Equip(NDb::SLOT_1,melee), "equipped-melee cursor fixture could not equip its native weapon");
    const int ap = actor->GetAP(), vp = actor->GetUnitRPG()->GetTotalVP();
    const int meleeCost = actor->GetActionAP(NRPG::AC_MELEE);
    state->RefreshCursor();
    Require(actor->GetState() == NWorld::CUnit::ST_NORMAL_KNIFE && state->GetCursorInfo().pCursor == NDb::GetUICursor(9),
        "an equipped native melee weapon still advertised the unarmed fist state and cursor");
    Require(NWorld::GetActionType(actor) == NWorld::AT_MELEE && bans.GetCommandBans(actor,object) == objectBans &&
        bans.GetCommandBans(actor,tile) == tileBans && actor->GetActionAP(NRPG::AC_MELEE) == meleeCost,
        "correcting the melee UI state changed attack dispatch, critical restrictions or its AP-cost query");
    CObj<NRPG::IInventoryItem> removed = inventory->TakeOff(NDb::SLOT_1);
    state->RefreshCursor();
    Require(actor->GetState() == NWorld::CUnit::ST_NORMAL_MELEE && state->GetCursorInfo().pCursor == NDb::GetUICursor(8),
        "unequipping the melee weapon did not restore the original native fist cursor");
    Require(actor->GetAP() == ap && actor->GetUnitRPG()->GetTotalVP() == vp && melee->GetQuantity() == 1 && !actor->HasCommand(),
        "refreshing native equipped-melee cursors spent AP/VP, consumed the weapon or installed an order");
}

static void EquippedToolActionAvailability(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (unit && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() &&
            unit->GetUnitRPG()->CanMove()) { actor = unit; break; }
    }
    Require(actor != nullptr, "tool-availability fixture has no mobile actor");
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    CObj<NRPG::IInventoryItem> old = inventory->Get(NDb::SLOT_1) ? inventory->TakeOff(NDb::SLOT_1) : nullptr;
    Require(inventory->Activate(NDb::SLOT_1), "tool-availability fixture could not activate its empty hand");
    CObj<NativeActionInfoMission> mission = new NativeActionInfoMission;
    mission->Attach(world,game);
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission,actor);
    mission->selection.push_back(tracker.GetPtr());
    const int ap = actor->GetAP();
    auto check = [&](bool available) {
        NGame::SActionInfo direct, cached;
        mission->CanDoCommand(new NWorld::CCmdUntrapObject(nullptr),true,&direct);
        Require(direct.bAvailable == available, "native tool-probe control has unexpected equipment eligibility");
        mission->RefreshActions();
        mission->GetActionInfo(NGame::UA_USETOOL,&cached);
        Require(cached.bValid && cached.bAvailable == direct.bAvailable && cached.bOk == direct.bOk &&
            cached.eResult == direct.eResult && cached.nMinAP == direct.nMinAP && cached.nMaxAP == direct.nMaxAP,
            "the mission's use-tool cache advertised availability without consulting the equipped tool");
        Require(actor->GetAP() == ap && !actor->HasCommand(),
            "checking use-tool availability spent AP or installed a gameplay command");
    };
    check(false);
    NDb::CRPGTool* record = nullptr;
    CDBIterator<NDb::CRPGTool> tools(*NDatabase::GetTable<NDb::CRPGTool>());
    while (tools.MoveNext()) {
        if (IsValid(tools.Get()->pItem) && tools.Get()->bCanUseForMineCleaning) { record = tools.Get(); break; }
    }
    Require(record != nullptr, "tool-availability fixture has no authored mine-clearing tool");
    CObj<NRPG::CToolItem> tool = new NRPG::CToolItem(record);
    Require(inventory->Equip(NDb::SLOT_1,tool), "tool-availability fixture could not equip its authored tool");
    const int quantity = tool->GetQuantity();
    check(true);
    Require(tool->GetQuantity() == quantity, "checking tool availability consumed a charge");
    CObj<NRPG::IInventoryItem> removed = inventory->TakeOff(NDb::SLOT_1);
    check(false);
}

static void EmptyClipAttackEntry(const char* databasePath,const char* resourcePath,bool invalidCache = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    CPtr<NRPG::CWeaponItem> weapon;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK() ||
            !unit->GetUnitRPG()->CanMove()) continue;
        auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        if (firearm && firearm->GetAmmoQuantity() > 0 && firearm->SetShootMode(NDb::SM_Snap) &&
            firearm->CanReload(unit->GetUnitRPG()->GetInventory()) &&
            unit->CanSpendAP(unit->GetActionAP(NRPG::AC_RELOAD))) {
            actor = unit; weapon = firearm; break;
        }
    }
    Require(actor && IsValid(weapon), "attack-entry fixture has no mobile firearm user with spare ammunition");
    CObj<NativeActionInfoMission> mission = new NativeActionInfoMission;
    mission->Attach(world,game);
    CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission,actor);
    mission->selection.push_back(tracker.GetPtr());
    if (invalidCache) {
        NGame::SActionInfo expected, queried;
        auto* attack = new NWorld::CCmdShootObject(nullptr,0);
        attack->bCanBeReplacedByReload = true;
        mission->CanDoCommand(attack,true,&expected);
        Require(expected.bValid && expected.bAvailable && expected.bOk,
            "invalid-cache control could not probe its loaded native firearm");
        mission->GetActionInfo(NGame::UA_ATTACK,&queried);
        Require(queried.bValid && queried.bAvailable == expected.bAvailable && queried.bOk == expected.bOk &&
            queried.eResult == expected.eResult && queried.nMinAP == expected.nMinAP && queried.nMaxAP == expected.nMaxAP,
            "querying an invalid action cache returned an uncomputed unavailable action");
    }
    mission->RefreshActions();
    NGame::SActionInfo loaded;
    mission->GetActionInfo(NGame::UA_ATTACK,&loaded);
    Require(loaded.bOk && loaded.bAvailable && loaded.eResult != NWorld::UCR_OK_RELOAD,
        "loaded-weapon control could not enter the native attack mode");
    weapon->DiscardAmmo();
    Require(weapon->GetAmmoQuantity() == 0 && weapon->CanReload(actor->GetUnitRPG()->GetInventory()),
        "attack-entry fixture failed to empty its weapon while retaining spare ammunition");
    if (invalidCache) {
        NGame::SActionInfo retained;
        mission->GetActionInfo(NGame::UA_ATTACK,&retained);
        Require(retained.bValid && retained.eResult == loaded.eResult && retained.nMinAP == loaded.nMinAP &&
            retained.nMaxAP == loaded.nMaxAP, "a valid action cache was recomputed before invalidation");
    }
    const int ap = actor->GetAP();
    CObj<NWorld::CCmdShootObject> explicitReload = new NWorld::CCmdShootObject(nullptr,0);
    explicitReload->bCanBeReplacedByReload = true;
    NGame::SActionInfo expected;
    mission->CanDoCommand(explicitReload,true,&expected);
    Require(expected.bOk && expected.bAvailable && expected.eResult == NWorld::UCR_OK_RELOAD,
        "explicit attack-preview control did not offer the existing native replacement reload");
    mission->RefreshActions();
    NGame::SActionInfo cached;
    mission->GetActionInfo(NGame::UA_ATTACK,&cached);
    Require(cached.bOk && cached.bAvailable && cached.eResult == expected.eResult &&
        cached.nMinAP == expected.nMinAP && cached.nMaxAP == expected.nMaxAP,
        "the mission's attack-mode entry rejected an empty clip despite an available replacement reload");
    Require(weapon->GetAmmoQuantity() == 0 && actor->GetAP() == ap && !actor->HasCommand(),
        "probing native attack-mode entry spent ammunition or AP or installed a command");
}

static void HolsteredReloadPreparation(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    CPtr<NRPG::CWeaponItem> weapon;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!server || !server->CanFight() || server->IsEmptyPK() || server->IsWearingPK()
                || !server->GetUnitRPG()->CanMove() || server->GetPosition().pos.p.GetPose() == NAI::CM_INACTIVE) continue;
        auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(server->GetUnitRPG()->GetInventory()->GetActive());
        if (!firearm || firearm->GetAmmoQuantity() <= 0) continue;
        firearm->DiscardAmmo();
        if (firearm->CanReload(server->GetUnitRPG()->GetInventory())
                && server->CanSpendAP(server->GetActionAP(NRPG::AC_RELOAD))) { actor = server; weapon = firearm; break; }
    }
    Require(actor && IsValid(weapon), "holstered reload fixture has no mobile firearm user with spare ammunition");
    actor->animator.SetActiveItem(false); actor->SetUndrawItem(true); actor->SetWalkWithoutWeapon(true);
    const auto original = actor->GetPosition().pos.p;
    const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    CObj<NWorld::CCmdReload> command = new NWorld::CCmdReload(-1);
    NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
    CObj<NWorld::CCommandExecute> executor = NWorld::CreateExecutor(actor, command, &result);
    Require(IsValid(executor) && result == NWorld::UCR_OK, "holstered reload fixture could not create its reload action");
    auto* sequence = dynamic_cast<NWorld::CSimpleExecQueue*>(executor.GetPtr());
    Require(sequence != nullptr, "holstered reload started without scheduling weapon activation preparation");
    auto* preparation = sequence->GetFrontExecutor();
    auto* move = dynamic_cast<NWorld::IExecMove*>(preparation);
    Require(move && preparation != FindNativeReload(executor), "reload action preceded its native weapon preparation");
    NAI::SPathPlace target;
    NAI::EFindPathParams params;
    NWorld::ENeedActiveItem active;
    move->GetDesiredPlace(&target, &params, &active);
    Require(target == original && active == NWorld::ITEM_ACTIVE,
        "reload preparation changed the tile/pose/direction or omitted the active-weapon constraint");
    auto plan = ReadMovementCommands(preparation);
    const auto activations = std::count_if(plan.queued.begin(), plan.queued.end(), [](const auto& item) {
        return pSSClasses->GetObjectTypeID(item.GetPtr()) == int(0x113B1130);
    });
    Require(activations == 1, "holstered reload preparation did not queue its explicit weapon draw");
    Require(actor->GetPosition().pos.p == original && weapon->GetAmmoQuantity() == 0
            && actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "preparing reload moved the actor or spent ammunition/AP before execution");
    CPtr<NWorld::CCommandExecute> reload = FindNativeReload(executor);
    executor->Run();
    Require(preparation->IsExecuting() && !reload->IsExecuting() && !actor->IsWalkingWithoutWeapon(),
        "reload started before its explicit weapon-draw action");
    executor->TimeLabelReached();
    Require(!actor->GetUndrawItem() && weapon->GetAmmoQuantity() == 0,
        "weapon-draw time label did not reveal the weapon before loading ammunition");
    executor->AnimationFinished();
    Require(IsValid(reload) && reload->IsExecuting() && !actor->GetUndrawItem(),
        "weapon-draw completion did not start the native reload action");
    executor->TimeLabelReached();
    Require(weapon->GetAmmoQuantity() > 0 && actor->GetPosition().pos.p == original,
        "prepared native reload did not load ammunition in the original tile and pose");
    executor->AnimationFinished();
    executor = nullptr;
    Require(!world->IsAction(), "completed prepared reload retained its action counter");

}

static void EffectiveWeaponMode(const char* databasePath, const char* resourcePath, int checkIcons = 0)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    CPtr<NRPG::CWeaponItem> personal;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (!unit || !unit->CanFight() || unit->IsEmptyPK() || unit->IsWearingPK()) continue;
        auto* item = dynamic_cast<NRPG::CWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        if (item && item->IsShootModeSupported(NDb::SM_Snap) && item->IsShootModeSupported(NDb::SM_Aimed)) {
            actor = unit; personal = item; break;
        }
    }
    Require(actor && IsValid(personal), "mission fixture has no suitable personal firearm");
    CObj<NRPG::CWeaponItem> mounted = new NRPG::CWeaponItem(personal->GetDBWeapon());
    Require(personal->SetShootMode(NDb::SM_Snap) && mounted->SetShootMode(NDb::SM_Snap), "fixture modes are unsupported");
    actor->GetUnitRPG()->SetCannonItem(mounted);
    CObj<NWorld::CCmdShootMode> command = new NWorld::CCmdShootMode(NDb::SM_Aimed);
    NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
    CObj<NWorld::CCommandExecute> executor = NWorld::CreateExecutor(actor, command, &result);
    Require(executor && result == NWorld::UCR_OK, "could not create a fire-mode executor");
    executor->Run();
    Require(mounted->GetShootMode() == NDb::SM_Aimed, "fire-mode executor changed the personal weapon instead of the mounted gun");
    Require(personal->GetShootMode() == NDb::SM_Snap, "mounted fire-mode change modified the holstered personal weapon");
    if (checkIcons) {
        CObj<WeaponIconMission> mission = new WeaponIconMission;
        CObj<NGame::CUnitTracker> tracker = new NGame::CUnitTracker(mission, actor);
        mission->selected.push_back(tracker.GetPtr());
        if (checkIcons == 3) MountedModeButton(mission);
        if (checkIcons == 2)
            Require(NUI::GetSelectedWeaponForIcons(mission) == mounted.GetPtr(),
                "fire-mode submenu enumerated the personal firearm instead of the mounted gun");
        Require(NUI::GetUnitsGroupWeaponMode(mission) == NDb::SM_Aimed,
            "mounted-gun mode indicator displayed the personal firearm mode");
        Require(NUI::GetSelectedWeaponForIcons(mission) == mounted.GetPtr(),
            "fire-mode submenu enumerated the personal firearm instead of the mounted gun");
        actor->GetUnitRPG()->SetCannonItem(nullptr);
        Require(NUI::GetUnitsGroupWeaponMode(mission) == NDb::SM_Snap &&
                NUI::GetSelectedWeaponForIcons(mission) == personal.GetPtr(),
            "personal fire-mode UI did not resume after leaving the mounted gun");
        mission->selected.push_back(tracker.GetPtr());
        Require(!NUI::GetSelectedWeaponForIcons(mission), "multi-selection enabled the single-weapon submenu");
        mission->selected.clear();
        Require(NUI::GetUnitsGroupWeaponMode(mission) == -2 && !NUI::GetSelectedWeaponForIcons(mission),
            "empty selection exposed a weapon mode");
    }
    actor->GetUnitRPG()->SetCannonItem(nullptr);
}

class RenderCache;
class RenderFixture: public CObjectBase
{
    OBJECT_BASIC_METHODS(RenderFixture);
public:
    void Visit(RenderCache*) {}
};

class RenderCache: public COrdinarySyncDst<RenderFixture, RenderCache>
{
protected:
    void PostVisit(int, RenderFixture* object) override { cached = object; }
public:
    CPtr<RenderFixture> cached;
    explicit RenderCache(CSyncSrc<RenderFixture>* source): COrdinarySyncDst(source) {}
};

static void LongBurstVoiceDonor(const char* databasePath,const char* resourcePath)
{
    for (int scenario = 0; scenario < 3; ++scenario) {
        auto world = FixtureWorld(databasePath,resourcePath,false,nullptr);
        vector<CPtr<NWorld::CUnit>> units;
        world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr;
        CPtr<NRPG::CWeaponItem> weapon;
        for (auto& entry: units) {
            auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
            if (!unit || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK() ||
                !unit->GetUnitRPG()->CanMove() || unit->GetPosition().GetPose() == NAI::CRAWL) continue;
            auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
            if (firearm && firearm->GetAmmoQuantity() > 0 && IsValid(firearm->GetDBWeapon()->pAnimWeaponType) &&
                firearm->SetShootMode(NDb::SM_LongBurst)) { actor = unit; weapon = firearm; break; }
        }
        Require(actor && IsValid(weapon), "long-burst voice fixture has no actual armed mobile burst shooter");
        weapon->GetDBWeapon()->pAnimWeaponType->type = NDb::WT_MACHINE_GUN;
        auto* persona = actor->GetUnitRPG()->GetRPGUnit();
        persona->bHero = false;
        CObj<NDb::CRPGPers> donor = new NDb::CRPGPers;
        CObj<NDb::CSound> donorVoice = new NDb::CSound, ownVoice = new NDb::CSound;
        persona->pPers->pAcksHolder = scenario == 1 ? nullptr : donor.GetPtr();
        persona->pPers->pLongBurstSnd = ownVoice;
        donor->pLongBurstSnd = scenario == 2 ? nullptr : donorVoice.GetPtr();
        Require(persona->GetAckHolder() == (scenario == 1 ? nullptr : donor.GetPtr()),
            "long-burst voice fixture did not select its native acknowledgement donor");
        vector<NWorld::IDynamicObject*> previous;
        for (auto& object: *world->GetMiscObjects()) previous.push_back(object.GetPtr());
        CObj<NWorld::CExecShootTile> attack = new NWorld::CExecShootTile(actor,actor->GetPosition().GetCP()+CVec3(5,0,0));
        attack->Start();
        struct Voices: NWorld::ISoundVisitor {
            vector<NDb::CSound*> sounds;
            void Add3DSound(STime,NDb::CSound* sound,CFuncBase<CVec3>*) override { sounds.push_back(sound); }
            void AddEffect(STime,NDb::CSoundEffect*,CFuncBase<CVec3>*,const vector<int>&) override {
                throw std::runtime_error("long-burst voice unexpectedly created an effect");
            }
            void Add2DSound(STime,NDb::CSound*) override { throw std::runtime_error("long-burst voice unexpectedly became non-positional"); }
        } voices;
        for (auto& object: *world->GetMiscObjects()) {
            if (std::find(previous.begin(),previous.end(),object.GetPtr()) != previous.end()) continue;
            if (auto* sound = dynamic_cast<NWorld::C3DSound*>(object.GetPtr())) sound->Visit(&voices);
        }
        if (scenario == 0) Require(voices.sounds == vector<NDb::CSound*>{donorVoice.GetPtr()},
            "starting a real long burst played the fixed generic voice instead of the shooter's acknowledgement donor");
        if (scenario == 1) Require(voices.sounds == vector<NDb::CSound*>{ownVoice.GetPtr()},
            "a long-burst shooter without a donor did not fall back to its own persona voice");
        if (scenario == 2) Require(voices.sounds.empty(),
            "a donor without a long-burst voice incorrectly fell back to another persona");
    }
}

static void RenderRemoval()
{
    CObj<CSetSyncSrc<RenderFixture>> source = new CSetSyncSrc<RenderFixture>;
    CObj<RenderFixture> object = new RenderFixture;
    RenderCache cache(source);
    source->Set(std::vector<RenderFixture*>{object});
    cache.Sync();
    Require(cache.cached == object, "invalid render cache test fixture");
    source->Set(std::vector<RenderFixture*>{});
    cache.Sync();
    Require(!cache.cached, "removed render slot retained a live object reference");
}

static void RenderPassCompaction()
{
    // Start with two low passes so the old wrong-destination write fails
    // deterministically without needing an out-of-bounds memory access.
    for (const auto& passes : std::vector<std::vector<int>>{
            {1, 2, 8, 9}, {8, 1, 9, 2}, {1, 8}, {8, 9}, {1, 2}, {}, {8}}) {
        NGScene::CRenderCmdList source, lower;
        std::vector<float> expectedLower, expectedRetained;
        for (int i = 0; i < int(passes.size()); ++i) {
            source.ops.emplace_back(nullptr, NGScene::RO_NOP,
                static_cast<unsigned char>(passes[i]), 0, 0,
                NGScene::CRenderCmdList::UParameter(float(i + 1)));
            (passes[i] < 8 ? expectedLower : expectedRetained).push_back(float(i + 1));
        }
        NGScene::SplitOps(&lower, &source, 8);
        Require(lower.ops.size() == expectedLower.size(), "render split changed lower-pass count");
        Require(source.ops.size() == expectedRetained.size(), "render split changed retained-pass count");
        for (int i = 0; i < int(expectedLower.size()); ++i)
            Require(lower.ops[i].p1.f == expectedLower[i] && lower.ops[i].nPass < 8,
                "retained render passes overwrote lower passes");
        for (int i = 0; i < int(expectedRetained.size()); ++i)
            Require(source.ops[i].p1.f == expectedRetained[i] && source.ops[i].nPass >= 8,
                "render split lost or reordered retained passes");
    }
}

class PassiveUnitState: public NWorld::CUnitState
{
    OBJECT_NOCOPY_METHODS(PassiveUnitState);
public:
    NWorld::CCommandExecute* CreateExecutor(NWorld::CCmd*, NWorld::EUnitCommandResult*) override { return nullptr; }
    void FilterCriticals() override {}
};

class ReplacingUnitState: public PassiveUnitState
{
    OBJECT_NOCOPY_METHODS(ReplacingUnitState);
    bool* callbackAlive = nullptr;
    int marker = 0;
public:
    ReplacingUnitState() {}
    ReplacingUnitState(NWorld::CUnitServer* unit, bool* alive): callbackAlive(alive), marker(42) { pUS = unit; }
    void OnStateStarted() override {
        // Keep the allocation to observe destroyed contents without a UAF.
        CPtr<ReplacingUnitState> weak(this);
        bool* result = callbackAlive;
        pUS->SetState(new PassiveUnitState);
        *result = IsValid(this) && marker == 42;
    }
};

static void ReentrantStateLifetime()
{
    CObj<NWorld::CUnitServer> unit = new NWorld::CUnitServer;
    bool callbackAlive = false;
    unit->SetState(new ReplacingUnitState(unit, &callbackAlive));
    Require(callbackAlive, "self-replacing unit state was destroyed before its callback returned");
}

class MountedStateProbe: public NWorld::CUnitStateUsingCannon
{
    OBJECT_NOCOPY_METHODS(MountedStateProbe);
public:
    bool blocked = false;
    explicit MountedStateProbe(NWorld::CUnitServer* unit = nullptr): CUnitStateUsingCannon(unit, nullptr) {}
    bool IsCriticalsFailCommand(NWorld::CCmd*, NWorld::EUnitCommandResult* result) override {
        if (blocked) *result = NWorld::UCR_CRITICALS_BAN;
        return blocked;
    }
};

static void MountedModeCommand()
{
    CObj<NWorld::CUnitServer> unit = new NWorld::CUnitServer;
    CObj<MountedStateProbe> state = new MountedStateProbe(unit);
    for (auto mode : {NDb::SM_Snap, NDb::SM_Aimed, NDb::SM_Careful, NDb::SM_ShortBurst, NDb::SM_LongBurst}) {
        CObj<NWorld::CCmdShootMode> command = new NWorld::CCmdShootMode(mode);
        NWorld::EUnitCommandResult result = NWorld::UCR_UNAVAILABLE;
        CObj<NWorld::CCommandExecute> executor = state->CreateExecutor(command, &result);
        Require(executor && result == NWorld::UCR_OK, "mounted-gun state rejected a fire-mode command");
    }
    state->blocked = true;
    CObj<NWorld::CCmdShootMode> blocked = new NWorld::CCmdShootMode(NDb::SM_Aimed);
    NWorld::EUnitCommandResult result = NWorld::UCR_OK;
    CObj<NWorld::CCommandExecute> executor = state->CreateExecutor(blocked, &result);
    Require(!executor && result == NWorld::UCR_CRITICALS_BAN, "mounted fire-mode command bypassed injury restrictions");
    state->blocked = false;
    CObj<NWorld::CCmdSetActiveItem> inventory = new NWorld::CCmdSetActiveItem(0);
    executor = state->CreateExecutor(inventory, &result);
    Require(!executor && result == NWorld::UCR_UNAVAILABLE, "mounted state accepted an unrelated inventory command");
}

class CallbackQueue: public NWorld::CSimpleExecQueue
{
    OBJECT_NOCOPY_METHODS(CallbackQueue);
public:
    void Replace(NWorld::CCommandExecute* replacement) { execList.clear(); AddExecutor(replacement); }
};

class WaitingExecutor: public NWorld::CCommandExecute
{
    OBJECT_NOCOPY_METHODS(WaitingExecutor);
};

class ReplacingExecutor: public NWorld::CCommandExecute
{
    OBJECT_NOCOPY_METHODS(ReplacingExecutor);
    CPtr<CallbackQueue> queue;
    CPtr<WaitingExecutor> replacement;
    void Callback() {
        CPtr<ReplacingExecutor> weak(this);
        queue->Replace(replacement);
        Require(IsValid(this), "queue destroyed its executor while the executor callback was running");
    }
public:
    ReplacingExecutor() {}
    ReplacingExecutor(CallbackQueue* owner, WaitingExecutor* next): queue(owner), replacement(next) {}
    void Run() override { Callback(); }
    void AnimationFinished() override { Callback(); }
};

static void ReentrantQueueLifetime(bool animationFinished)
{
    CObj<CallbackQueue> queue = new CallbackQueue;
    CObj<WaitingExecutor> replacement = new WaitingExecutor;
    queue->AddExecutor(new ReplacingExecutor(queue, replacement));
    if (animationFinished) queue->AnimationFinished(); else queue->Run();
    Require(queue->GetFrontExecutor() == replacement.GetPtr(),
        "callback replacement was removed from the action queue");
    Require(queue->GetState() == NWorld::CCommandExecute::RUNNING,
        "callback replacement incorrectly finished the action queue");
}

class InventoryFirearm: public NAI::CAIFireArmsWeapon
{
    OBJECT_NOCOPY_METHODS(InventoryFirearm);
public:
    void SetItem(NRPG::CWeaponItem* item) { pWeaponItem = item; }
};

static void InvalidAIWeapon()
{
    CObj<NAI::AIUnitFixture> owner = new NAI::AIUnitFixture;
    CObj<NAI::AIUnitFixture> target = new NAI::AIUnitFixture;
    CObj<NAI::CAIInventory> inventory = new NAI::CAIInventory;
    CObj<InventoryFirearm> firearm = new InventoryFirearm;
    inventory->RestoreItem(firearm);
    // Inject the inventory's owner through its existing serialized field.
    CMemoryStream stream;
    CObj<NAI::IAIUnit> ownerReference = owner.GetPtr();
    { CStructureSaver saver(stream, CStructureSaver::WRITE); saver.Add(2, &ownerReference); }
    stream.Seek(0);
    CObj<NAI::IAIUnit> loadedOwner;
    { CStructureSaver saver(stream, CStructureSaver::READ);
      saver.Add(2, &loadedOwner);
      static_cast<CObjectBase*>(inventory.GetPtr())->operator&(saver); }
    owner = dynamic_cast<NAI::AIUnitFixture*>(loadedOwner.GetPtr());
    Require(IsValid(owner), "fixture inventory has no live owner");
    for (bool stale : {false, true}) {
        CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem;
        firearm->SetItem(stale ? weapon.GetPtr() : nullptr);
        weapon = nullptr;
        Require(!IsValid(firearm->GetItem()), "fixture weapon should be missing or invalid");
        int cover = 123, quality = 123, hit = 123;
        NDb::EShootMode mode = NDb::SM_Snap;
        auto* best = inventory->GetBestFireArms(NAI::SUnitPosition(), target, 30,
            &cover, &quality, &mode, &hit);
        Require(best == nullptr && cover == 0 && hit == 0 && quality == -0xFFFF,
            "invalid-only AI inventory produced a weapon quote");
        Require(owner->coverCalls == 0, "invalid AI weapon entered attack evaluation");
    }
}

class FirearmTypeProbe: public NRPG::CWeaponItem
{
    OBJECT_NOCOPY_METHODS(FirearmTypeProbe);
public:
    NDb::EWeaponType GetWeaponType() const override {
        Require(!IsRefInvalid(), "invalid firearm reached weapon-type lookup");
        return NDb::WT_PISTOL;
    }
};

class MeleeTypeProbe: public NRPG::CMeleeWeaponItem
{
    OBJECT_NOCOPY_METHODS(MeleeTypeProbe);
public:
    NDb::EWeaponType GetWeaponType() const override {
        Require(!IsRefInvalid(), "invalid melee weapon reached weapon-type lookup");
        return NDb::WT_KNIFE;
    }
};

static void StaleWeaponReferences(int kind)
{
    CObj<NRPG::CUnit> unit = new NRPG::CUnit;
    unit->pInventory = NRPG::CreateInventory(unit);
    if (kind < 2) {
        CObj<FirearmTypeProbe> personal = new FirearmTypeProbe;
        Require(unit->pInventory->Equip(NDb::SLOT_1, personal) && unit->pInventory->Activate(NDb::SLOT_1),
            "could not equip the weapon-reference fixture");
        Require(unit->GetWeaponType() == NDb::WT_PISTOL, "live personal weapon was not selected");
        if (kind == 0) {
            CObj<FirearmTypeProbe> cannon = new FirearmTypeProbe;
            unit->pCannonItem = cannon;
            cannon = nullptr;
            Require(!IsValid(unit->pCannonItem), "fixture cannon must be invalid");
            Require(unit->GetWeaponItem() == personal.GetPtr(), "invalid mounted weapon masked the equipped firearm");
        } else {
            personal->Invalidate();
            Require(unit->GetWeaponType() == NDb::WT_DEFAULT, "invalid firearm changed the unit's weapon type");
        }
    } else {
        CObj<MeleeTypeProbe> melee = new MeleeTypeProbe;
        unit->pDefaultWeapon = melee;
        Require(unit->GetWeaponType() == NDb::WT_KNIFE, "live default melee weapon was not selected");
        melee->Invalidate();
        Require(unit->GetWeaponType() == NDb::WT_DEFAULT, "invalid melee weapon changed the unit's weapon type");
    }
}

class LockerActor: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(LockerActor);
public:
    bool active = true;
    bool shell = false;
    bool CanFight() const override { return active; }
    bool IsEmptyPK() const override { return shell; }
};

static void NavigationAliasDestinations(bool arrayOutput, bool isolatedSource = false)
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    for (int i = 0; i < 5; ++i) {
        auto* group = i ? network->GetLayer(0)->pGroup.GetPtr() : nullptr;
        network->CreateLayer(4,4,VNULL2,0.f,i,group);
    }
    network->Freeze(true);
    Require(network->GetNumLayers() == 5, "alias fixture did not create its five shared-group surfaces");
    for (int i = 0; i < 5; ++i)
        Require(network->GetLayer(0)->pGroup->layers[i*NAI::N_MAX_LAYERS_PER_FLOOR] == network->GetLayer(i),
            "alias fixture did not register every candidate in the shared layers group");
    NAI::STile connected{};
    connected.nHeight = 100;
    connected.nDisplacement = 2;
    connected.nPassable = 7;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_HAS_SAME | NAI::TF_STAND_PASSABLE;
    for (int i = 0; i < 5; ++i) network->GetLayer(i)->tiles[1][1] = connected;
    auto& isolated = network->GetLayer(1)->tiles[1][1];
    isolated.nMoveStand = 0;
    isolated.nFlags = NAI::TF_STAND_PASSABLE;
    network->GetLayer(3)->tiles[1][1].nHeight = 101;
    network->GetLayer(4)->tiles[1][1].nPassable = 0;
    NAI::SPathPlace origin(1,1,0);
    auto& source = network->GetLayer(0)->tiles[1][1];
    if (isolatedSource) {
        source.nMoveStand = 0;
        source.nFlags = NAI::TF_STAND_PASSABLE;
    }
    auto aliases = [&]() {
        vector<int> result;
        if (arrayOutput) {
            NAI::SPathPlace places[5];
            const auto guard = places[4].GetBits();
            const int count = network->GetLayer(0)->GetSamePoints(origin,places);
            Require(count >= 0 && count < 5 && places[4].GetBits() == guard,
                "array alias enumeration wrote outside its destination slots");
            for (int i = 0; i < count; ++i) {
                Require(places[i].GetX() == 1 && places[i].GetY() == 1, "array alias changed the tile coordinates");
                result.push_back(places[i].GetLayer());
            }
        } else {
            const NAI::SPathPlace sentinel(0,0,0);
            vector<NAI::SPathPlace> places{sentinel};
            network->GetLayer(0)->GetSamePoints(origin,&places);
            Require(!places.empty() && places.front() == sentinel,
                "vector alias enumeration discarded its caller's existing destinations");
            for (std::size_t i = 1; i < places.size(); ++i) {
                Require(places[i].GetX() == 1 && places[i].GetY() == 1, "vector alias changed the tile coordinates");
                result.push_back(places[i].GetLayer());
            }
        }
        return result;
    };
    const auto initial = aliases();
    std::printf("Navigation aliases (%s, source %s):",arrayOutput ? "array" : "vector",isolatedSource ? "isolated" : "connected");
    for (int layer: initial) std::printf(" %d",layer);
    std::puts("; expected only connected layer 2");
    Require(initial == vector<int>({2}),
        isolatedSource ? "an isolated occupied source without the same-point flag could not enumerate its connected escape copy"
                       : "same-point enumeration included isolated or blocked navigation aliases instead of only the connected copy");
    auto& connectedCopy = network->GetLayer(2)->tiles[1][1];
    connectedCopy.nLocks = 1; connectedCopy.nDynLocks = 1;
    Require(aliases() == vector<int>({2}) && connectedCopy.nLocks == 1 && connectedCopy.nDynLocks == 1,
        "temporary occupancy removed a structural alias or enumeration changed its locks");
    connectedCopy.nLocks = 0; connectedCopy.nDynLocks = 0;
    if (isolatedSource) {
        source.nMoveStand = 1;
        Require(aliases().empty(), "a connected source without the same-point flag invented undeclared aliases");
        source.nMoveStand = 0;
    }
    isolated.nMoveCrouch = 1;
    Require(aliases() == vector<int>({1,2}), "a crouch-connected alias was rejected or changed destination order");
    isolated.nMoveCrouch = 0;
    isolated.nFlags |= NAI::TF_HAS_INTERGRID;
    Require(aliases() == vector<int>({1,2}), "an intergrid-linked alias was incorrectly rejected");
    isolated.nFlags = NAI::TF_STAND_PASSABLE | NAI::TF_IS_LADDER_UP;
    Require(aliases() == vector<int>({1,2}), "a ladder-connected alias was incorrectly rejected");
    isolated.nFlags = NAI::TF_STAND_PASSABLE;
    ++network->GetLayer(2)->tiles[1][1].nDisplacement;
    Require(aliases().empty(), "unequal displacement or isolated aliases were still reported as connected copies");
}

static CObj<NAI::CPathNetwork> SimplePathNetwork(int layerCount = 1)
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    for (int layer = 0; layer < layerCount; ++layer)
        network->CreateLayer(8,8,VNULL2,0.f,layer,layer ? network->GetLayer(0)->pGroup.GetPtr() : nullptr);
    network->Freeze(true);
    for (int layer = 0; layer < layerCount; ++layer)
        for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x) {
            auto& tile = network->GetLayer(layer)->tiles[y][x];
            tile = NAI::STile{};
            tile.nPassable = 0xff;
            tile.nMoveStand = tile.nMoveCrouch = tile.nMoveLay = 0xff;
            tile.nFlags = NAI::TF_STAND_PASSABLE;
        }
    return network;
}

class PlacementCursorMission: public AttackMission
{
    OBJECT_NOCOPY_METHODS(PlacementCursorMission);
public:
    NAI::SPosition position;
    CObj<NWorld::CWindowDoor> target = new NWorld::CWindowDoor;
    NGame::SActionInfo info;
    bool traceAvailable = true, mine = false;
    int queries = 0, commands = 0;
    PlacementCursorMission() { info.SetValid(); info.nMinAP = info.nMaxAP = 7; }
    bool GetTracePosition(NAI::SPosition* result) const override { *result = position; return traceAvailable; }
    CObjectBase* GetStateTarget(bool* fromWorld = nullptr) const override {
        if (fromWorld) *fromWorld = true;
        return target;
    }
    void GetActionInfo(NGame::EUnitAction,NGame::SActionInfo* result) override { result->SetValid(); }
    void CanDoCommand(NWorld::CCmd* command,bool,NGame::SActionInfo* result) override {
        if (mine) {
            auto* placement = dynamic_cast<NWorld::CCmdSetMineOnTile*>(command);
            Require(placement && placement->ptDst == position, "mine cursor probed a different command or traced position");
        } else {
            auto* placement = dynamic_cast<NWorld::CCmdSetGrenadeOnObject*>(command);
            Require(placement && placement->pTarget == target.GetPtr(), "trap cursor probed a different command or object");
        }
        ++queries; *result = info;
    }
    void Command(NWorld::CUnit*,NWorld::CCmd*,bool) override { ++commands; }
};

static void PlacementCursor(const char* databasePath,bool mine = false)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    Require(NDb::GetUICursor(19) && NDb::GetUICursor(NGame::N_CURSOR_BLOCK),
        "placement-cursor fixture has no authored placement or blocked cursor");
    auto network = SimplePathNetwork();
    CObj<PlacementCursorMission> mission = new PlacementCursorMission;
    mission->mine = mine;
    mission->position = NAI::SPosition(NAI::SPathPlace(2,3,0),network);
    Require(mission->position.IsValid(), "placement-cursor fixture has no valid native navigation position");
    CObj<NGame::IState> state = mine ? static_cast<NGame::IState*>(new NGame::CStateSetMine) :
        static_cast<NGame::IState*>(new NGame::CStateSetTrap);
    Require(state->Initialize(mission) && mission->queries == 1,
        "placement-cursor control did not initialize and probe its native targeting state");
    const auto expectedCaption = NUI::GetDBString(20276)+NUI::GetDBString(19808)+L"7";
    Require(state->GetCursorInfo().wsText == expectedCaption, "placement cursor lost its original action-point caption");
    Require(state->GetCursorInfo().pCursor == NDb::GetUICursor(19), mine ?
        "mine placement used the healing cursor instead of the authored trap-placement cursor" :
        "object trap placement used the healing cursor instead of the authored trap-placement cursor");
    mission->traceAvailable = false;
    state->Step();
    Require(mission->queries == 1 && state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK) &&
        state->GetCursorInfo().wsText.empty(), "losing the placement trace retained its action cursor or AP caption");
    mission->traceAvailable = true; mission->position = NAI::SPosition(NAI::SPathPlace(3,3,0),network);
    state->Step();
    Require(mission->queries == 2 && state->GetCursorInfo().pCursor == NDb::GetUICursor(19),
        "restoring a placement trace did not recover its authored action cursor");
    mission->info = NGame::SActionInfo(); mission->position = NAI::SPosition(NAI::SPathPlace(4,3,0),network);
    state->Step();
    Require(mission->queries == 3 && state->GetCursorInfo().pCursor == NDb::GetUICursor(NGame::N_CURSOR_BLOCK) &&
        mission->commands == 0, "unavailable placement retained an action cursor or cursor updates issued gameplay commands");
}

static void SimpleCrossLayerPoseEntry()
{
    auto network = SimplePathNetwork(2);
    auto find = [&](const NAI::SPathPlace& source,const NAI::SPathPlace& goal) {
        return CObj<NAI::CPath>(NAI::FindSimplePath(source,{goal},NAI::PF_USE_POSEDIR,network,false,false,false));
    };
    const NAI::SPathPlace standing(4,4,0,3,NAI::CM_STAND,0), prone(4,4,0,3,NAI::CM_LAY,0);
    Require(IsValid(find(standing,prone)), "unlocked prone footprint control rejected a direct pose path");
    vector<NAI::SPathPlace> footprint;
    network->GetLockArea(&footprint,prone,true);
    auto other = std::find_if(footprint.begin(),footprint.end(),[&](const NAI::SPathPlace& place) {
        return place.GetX() != prone.GetX() || place.GetY() != prone.GetY();
    });
    Require(other != footprint.end(), "prone footprint control has no neighbouring occupied cell");
    auto& blocked = network->GetLayer(other->GetLayer())->tiles[other->GetY()][other->GetX()];
    blocked.nLocks = 1;
    Require(network->GetPassability(prone) == NAI::AIP_LOCKED && !network->IsPassable(prone) &&
        NAI::BigLockerPassableState(network,prone) == NAI::AIP_LOCKED && !IsValid(find(standing,prone)),
        "existing native passability did not enforce the full prone footprint on a simple pose path");
    blocked.nLocks = 0;
    Require(IsValid(find(standing,prone)), "unlocking the prone footprint did not restore the simple pose path");
    for (int startPose: {NAI::CM_STAND,NAI::CM_LAY}) {
        const int endPose = startPose == NAI::CM_STAND ? NAI::CM_CROUCH : NAI::CM_STAND;
        const NAI::SPathPlace source(4,4,0,3,startPose,0), destination(4,4,1,3,endPose,0);
        auto sameLayer = destination;
        sameLayer.SetLayer(0);
        auto direct = find(source,sameLayer);
        Require(IsValid(direct) && direct->points.size() == 2,
            "same-layer pose control acquired an unnecessary intermediate point");
        auto path = find(source,destination);
        Require(IsValid(path) && path->points.front() == source && path->points.back() == destination,
            "cross-layer pose fixture did not find the requested native direct route");
        Require(path->points.size() == 3,
            "a simple cross-layer pose path omitted its layer-only entry point and combined the layer and pose change");
        auto entry = source;
        entry.SetLayer(destination.GetLayer());
        Require(path->points[1] == entry && path->actions.empty(),
            "a cross-layer entry changed source pose, heading, coordinates or unrelated path actions");
        for (std::size_t i = 1; i < path->points.size(); ++i)
            Require(NAI::GetTransitionType(network,path->points[i-1],path->points[i]) != NAI::TT_NO_WAY,
                "an emitted layer/pose path segment was not traversable by the native transition classifier");
    }
}

static void SimplePoseFacingPreference()
{
    auto network = SimplePathNetwork();
    auto find = [&](const NAI::SPathPlace& source,const vector<NAI::SPathPlace>& goals,
                    NAI::EFindPathParams params,bool pk = false) {
        return CObj<NAI::CPath>(NAI::FindSimplePath(source,goals,params,network,pk,pk,false));
    };
    const NAI::SPathPlace crouched(4,4,0,3,NAI::CM_CROUCH,0);
    const NAI::SPathPlace stand(4,4,0,3,NAI::CM_STAND,0), turn(4,4,0,4,NAI::CM_CROUCH,0);
    auto ranked = find(crouched,{stand,turn},NAI::PF_USE_POSEDIR);
    Require(IsValid(ranked) && ranked->points.back() == turn,
        "simple-path ranking no longer prefers an explicit turn over a pose change");
    auto pkPath = find(crouched,{stand,turn},NAI::PF_USE_POSEDIR,true);
    Require(IsValid(pkPath) && pkPath->points.back() == stand,
        "simple-path ranking bypassed the crouched Panzerklein turn restriction");
    const NAI::SPathPlace prone(4,4,0,3,NAI::CM_LAY,0);
    for (int pose: {NAI::CM_STAND,NAI::CM_CROUCH}) {
        const NAI::SPathPlace unspecified(4,4,0,0,pose,0);
        auto path = find(prone,{unspecified},NAI::PF_USE_POSE);
        Require(IsValid(path), "simple facing fixture did not find its direct pose path");
        Require(path->points.back().GetDirection() == prone.GetDirection(),
            "simple pose-path selection chose direction zero over an equally valid unchanged facing");
        auto explicitGoal = unspecified;
        explicitGoal.SetDirection(6);
        auto explicitPath = find(prone,{explicitGoal},NAI::PF_USE_POSEDIR);
        Require(IsValid(explicitPath) && explicitPath->points.back() == explicitGoal,
            "simple facing preference overrode the caller's explicit direction request");
    }
}

static void SimplePronePosePath()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8,8,VNULL2,0.f,0);
    network->Freeze(true);
    for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x) {
        auto& tile = network->GetLayer(0)->tiles[y][x];
        tile = NAI::STile{};
        tile.nPassable = 0xff;
        tile.nMoveStand = tile.nMoveCrouch = tile.nMoveLay = 0xff;
        tile.nFlags = NAI::TF_STAND_PASSABLE;
    }
    NAI::SPathPlace prone(4,4,0,0,NAI::CM_LAY,0), standing(4,4,0,0,NAI::CM_STAND,0);
    auto find = [&](const NAI::SPathPlace& source,const vector<NAI::SPathPlace>& goals,bool corpse = false) {
        return CObj<NAI::CPath>(NAI::FindSimplePath(source,goals,NAI::PF_USE_POSE,network,false,false,corpse));
    };
    auto crouched = standing;
    crouched.SetPose(NAI::CM_CROUCH);
    Require(IsValid(find(standing,{crouched})), "ordinary direct pose-change control failed");
    auto path = find(prone,{standing});
    Require(IsValid(path), "simple native pathfinding rejected a direct pose change from prone");
    Require(path->points.size() == 2 && path->points.front() == prone && path->points.back() == standing && path->actions.empty(),
        "direct prone pose path changed its coordinates, heading or action list");
    Require(IsValid(find(prone,{crouched})), "simple native pathfinding rejected a direct crouch transition from prone");
    Require(!IsValid(find(prone,{})) && !IsValid(find(prone,{prone})),
        "simple prone pathfinding changed empty-goal or prone-only rejection");
    auto inactive = prone;
    inactive.SetPose(NAI::CM_INACTIVE);
    Require(!IsValid(find(inactive,{standing})) && !IsValid(find(prone,{standing},true)),
        "simple pose paths bypassed inactive-source or corpse-carrying restrictions");
    auto displaced = standing;
    displaced.SetXY(5,4);
    Require(!IsValid(find(prone,{displaced})), "simple prone pose handling incorrectly replaced a movement route");
}

static void PronePoseChangeFacing()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8,8,VNULL2,0.f,0);
    network->Freeze(true);
    for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x) {
        auto& tile = network->GetLayer(0)->tiles[y][x];
        tile = NAI::STile{};
        tile.nPassable = 0xff;
        tile.nMoveStand = tile.nMoveCrouch = tile.nMoveLay = 0xff;
        tile.nFlags = NAI::TF_STAND_PASSABLE;
    }
    NAI::SPathPlace prone(4,4,0,3,NAI::CM_LAY,0);
    CObj<NAI::CPath> turn = new NAI::CPath;
    turn->pNet = network.GetPtr();
    auto turned = prone;
    turned.SetDirection(4);
    Require(turn->AddNewPoint(prone) && turn->AddNewPoint(turned) && turn->points.back() == turned,
        "explicit same-pose prone turn control lost its requested heading");
    for (int pose: {NAI::CM_STAND,NAI::CM_CROUCH}) {
        CObj<NAI::CPath> path = new NAI::CPath;
        path->pNet = network.GetPtr();
        NAI::SPathPlace canonical(4,4,0,0,pose,0);
        Require(path->AddNewPoint(prone) && path->AddNewPoint(canonical),
            "native path rejected a valid prone-to-upright pose transition");
        Require(path->points.size() == 2 && path->points.back().GetPose() == pose &&
            path->points.back().GetDirection() == prone.GetDirection(),
            "leaving prone adopted the wave's canonical direction instead of retaining the actual facing");
        Require(path->points.front() == prone && path->points.back().GetX() == 4 &&
            path->points.back().GetY() == 4 && path->points.back().GetLayer() == 0 && path->actions.empty(),
            "preserving pose-change facing moved the unit or introduced an unrelated path action");
        auto finalTurn = path->points.back();
        finalTurn.SetDirection(6);
        Require(path->AddNewPoint(finalTurn) && path->points.back().GetDirection() == 6,
            "the following explicit upright turn lost its requested heading");
    }
}

static void IsolatedSourcePath()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8,8,VNULL2,0.f,0);
    network->CreateLayer(8,8,VNULL2,0.f,1,network->GetLayer(0)->pGroup);
    // Keep the real pathfinder and colour caches active; only geometry refresh
    // is frozen because these native tiles are the complete fixture map.
    network->GetLayer(0)->pGroup->bFreeze = true;
    NAI::SPathPlace source(2,2,0), goal(5,2,1);
    source.SetPose(NAI::CM_STAND); goal.SetPose(NAI::CM_STAND);
    const int right = network->GetDir(NAI::SPathPlace(2,2,1),NAI::SPathPlace(3,2,1));
    const int left = (right + 4) & 7;
    for (int x = 1; x <= 6; ++x) {
        auto& tile = network->GetLayer(1)->tiles[2][x];
        tile = NAI::STile{};
        tile.nHeight = 100;
        tile.nPassable = 0xff;
        tile.nFlags = NAI::TF_STAND_PASSABLE;
        tile.nMoveStand = (x > 1 ? 1 << left : 0) | (x < 6 ? 1 << right : 0);
        tile.nMoveCrouch = tile.nMoveLay = tile.nMoveStand;
    }
    auto& isolated = network->GetLayer(0)->tiles[2][2];
    isolated = network->GetLayer(1)->tiles[2][2];
    isolated.nMoveStand = isolated.nMoveCrouch = isolated.nMoveLay = 0;
    int costs[NAI::N_MOVE_TYPES];
    std::fill(std::begin(costs),std::end(costs),1);
    costs[NAI::MT_ZERO] = 0;
    costs[NAI::MT_MOVE_STAND] = 2;
    costs[NAI::MT_MOVE_CROUCH] = 3;
    costs[NAI::MT_MOVE_CRAWL] = 4;
    const list<CObjectBase*> occupants;
    auto find = [&](int limit, NAI::EFindPathParams params) {
        return CObj<NAI::CPath>(NAI::FindPath(network,source,vector<NAI::SPathPlace>{goal},
            costs,limit,false,occupants,nullptr,true,false,params,false));
    };
    auto reachesGoal = [&](NAI::CPath* path) {
        if (!IsValid(path) || path->points.empty()) return false;
        const auto& end = path->points.back();
        return end.GetX() == goal.GetX() && end.GetY() == goal.GetY() && end.GetLayer() == goal.GetLayer();
    };
    const auto shortSearch = find(39,NAI::PF_USE_POSE);
    Require(reachesGoal(shortSearch), "local-wave control could not leave the isolated source and reach its exact standing goal");
    vector<unsigned char> xs,ys;
    vector<int> neighbours;
    auto* colourer = network->GetColourer(0);
    colourer->GetColorNeighbourCenters(network,colourer->GetPointColor(2,2),&xs,&ys,&neighbours,NAI::PM_ANY_MOVE);
    Require(neighbours.empty(), "path fixture unexpectedly had a cached upper-net escape route");
    const auto route = find(100,NAI::PF_DEFAULT);
    if (IsValid(route) && !route->points.empty()) {
        const auto& end = route->points.back();
        std::printf("Full isolated-source route ends at (%u,%u,layer %u,pose %u); expected (%u,%u,layer %u)\n",
            unsigned(end.GetX()),unsigned(end.GetY()),unsigned(end.GetLayer()),unsigned(end.GetPose()),
            unsigned(goal.GetX()),unsigned(goal.GetY()),unsigned(goal.GetLayer()));
    } else std::puts("Full isolated-source route was absent despite the successful local-wave control");
    Require(reachesGoal(route), "upper-net zone pruning rejected a reachable route from an isolated source or stopped before its goal");
    Require(route->points.front().GetLayer() == source.GetLayer() &&
        route->points.front().GetX() == source.GetX() && route->points.front().GetY() == source.GetY(),
        "isolated-source path changed its starting position");
    auto& escape = network->GetLayer(1)->tiles[2][2];
    escape.nMoveStand = escape.nMoveCrouch = escape.nMoveLay = 0;
    Require(!IsValid(find(100,NAI::PF_DEFAULT)), "isolated-source zone bypass invented a route without a connected escape");
}

static void ConnectedAliasMoves(bool isolatedSource = false, bool lockedDestination = false)
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    for (int i = 0; i < 3; ++i)
        network->CreateLayer(4,4,VNULL2,0.f,i,i ? network->GetLayer(0)->pGroup.GetPtr() : nullptr);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 100;
    connected.nPassable = NAI::CP_STAND | NAI::CP_CROUCH;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE | NAI::TF_HAS_SAME;
    for (int i = 0; i < 3; ++i) network->GetLayer(i)->tiles[1][1] = connected;
    auto& isolated = network->GetLayer(1)->tiles[1][1];
    isolated.nMoveStand = 0;
    isolated.nFlags = NAI::TF_STAND_PASSABLE;
    auto& destination = network->GetLayer(2)->tiles[1][1];
    if (lockedDestination) destination.nLocks = 1;
    NAI::SPathPlace source(1,1,0);
    source.SetPose(NAI::CM_STAND);
    auto& sourceTile = network->GetLayer(0)->tiles[1][1];
    if (isolatedSource) {
        sourceTile.nMoveStand = 0;
        sourceTile.nFlags = NAI::TF_STAND_PASSABLE;
    }
    auto aliases = [&]() {
        vector<NAI::SMove> moves(512);
        vector<char> locks(512,-1);
        int count = 0;
        NAI::GetNonStandartMoves(network,source,true,true,true,&moves,&count,&locks);
        Require(count >= 0 && count < 512, "alias move enumeration exceeded its fixture capacity");
        vector<int> result;
        for (int i = 0; i < count; ++i) {
            if (moves[i].type != NAI::MT_ZERO) continue;
            const auto& place = moves[i].dest;
            Require(place.GetX() == source.GetX() && place.GetY() == source.GetY() &&
                place.GetPose() == source.GetPose() && locks[i] == 0,
                "a zero-distance alias changed coordinates, pose or dynamic-lock accounting");
            result.push_back(place.GetLayer());
        }
        return result;
    };
    Require(aliases() == (lockedDestination ? vector<int>{} : vector<int>{2}), lockedDestination
        ? "native move enumeration offered a zero-distance step into a locked connected alias"
        : isolatedSource ? "native move enumeration could not escape an isolated source lacking the same-point flag"
                         : "native move enumeration offered a zero-distance step into an isolated alias");
    if (lockedDestination) {
        destination.nLocks = 0;
        Require(aliases() == vector<int>({2}), "unlocking a connected alias did not restore its zero-distance step");
        destination.nDynLocks = 1;
        Require(aliases() == vector<int>({2}) && destination.nDynLocks == 1,
            "a dynamic counter alone was treated as a hard lock or was changed by enumeration");
        destination.nLocks = 1;
        Require(aliases().empty() && destination.nLocks == 1 && destination.nDynLocks == 1,
            "an occupied alias was accepted or its lock counters were modified");
        destination.nLocks = 0; destination.nDynLocks = 0;
    }
    if (isolatedSource) {
        sourceTile.nMoveStand = 1;
        Require(aliases().empty(), "a connected source invented a same-point step without its declared flag");
        sourceTile.nMoveStand = 0;
    }
    isolated.nMoveCrouch = 1;
    Require(aliases() == vector<int>({1,2}), "move enumeration rejected a linked crouch alias");
    isolated.nPassable = NAI::CP_CROUCH;
    Require(aliases() == vector<int>({2}), "move enumeration ignored the current pose on a linked alias");
    ++destination.nHeight;
    Require(aliases().empty(), "move enumeration offered an alias at a different height");
}

static void IsolatedTransitionLinkMatch()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    for (int i = 0; i < 3; ++i)
        network->CreateLayer(4,4,VNULL2,0.f,i,i ? network->GetLayer(0)->pGroup.GetPtr() : nullptr);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 200;
    connected.nPassable = NAI::CP_STAND | NAI::CP_CROUCH;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE | NAI::TF_HAS_INTERGRID;
    const NAI::SPathPlace source(1,1,0), target(2,1,1), linkTarget(2,1,2);
    network->GetLayer(0)->tiles[1][1] = connected;
    network->GetLayer(0)->tiles[1][1].nHeight = 100;
    network->GetLayer(1)->tiles[1][2] = connected;
    auto& alias = network->GetLayer(2)->tiles[1][2];
    alias = connected;
    alias.nFlags = NAI::TF_STAND_PASSABLE;
    network->GetLayer(0)->transitions[source].links.emplace_back(linkTarget,10);
    network->GetLayer(1)->transitions[target];
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_INTERGRID,
        "connected transition link matching control failed");
    alias.nMoveStand = 0;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a recorded transition ending at an isolated copy matched a connected destination on another layer");
    alias.nMoveStand = 1;
    alias.nPassable = 0;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a recorded transition ending at an impassable copy matched a connected destination");
    alias = connected;
    ++alias.nHeight;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a recorded transition ending at a different height matched the destination");
}

static void IsolatedTransitionShortcut()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(4,4,VNULL2,0.f,0);
    network->CreateLayer(4,4,VNULL2,0.f,1,network->GetLayer(0)->pGroup);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 200;
    connected.nPassable = NAI::CP_STAND | NAI::CP_CROUCH;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE;
    NAI::SPathPlace source(1,1,0), target(2,1,1), intermediate(2,1,0);
    source.SetPose(NAI::CM_STAND); target.SetPose(NAI::CM_STAND);
    auto& start = network->GetLayer(0)->tiles[1][1];
    auto& alias = network->GetLayer(0)->tiles[1][2];
    start = alias = network->GetLayer(1)->tiles[1][2] = connected;
    start.nHeight = 100;
    start.nMoveStand = 1 << network->GetDir(source,intermediate);
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_MOVE,
        "connected cross-layer shortcut control failed");
    alias.nMoveStand = 0;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a cross-layer shortcut borrowed movement through an isolated intermediate copy");
    alias.nMoveCrouch = 1;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_MOVE,
        "a crouch-linked intermediate copy no longer supported its structural shortcut");
    ++alias.nHeight;
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a cross-layer shortcut accepted an intermediate copy at a different height");
}

static void IsolatedTransitionEntry()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(4,4,VNULL2,0.f,0);
    network->CreateLayer(4,4,VNULL2,0.f,1,network->GetLayer(0)->pGroup);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 100;
    connected.nPassable = NAI::CP_STAND | NAI::CP_CROUCH;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE | NAI::TF_HAS_SAME;
    auto& sourceTile = network->GetLayer(0)->tiles[1][1];
    auto& targetTile = network->GetLayer(1)->tiles[1][1];
    sourceTile = targetTile = connected;
    NAI::SPathPlace source(1,1,0), target(1,1,1);
    source.SetPose(NAI::CM_STAND); target.SetPose(NAI::CM_STAND);
    Require(NAI::GetTransitionType(network,source,target) != NAI::TT_NO_WAY,
        "connected same-point transition control failed");
    targetTile.nMoveStand = 0;
    Require(NAI::GetTransitionType(network,target,source) != NAI::TT_NO_WAY,
        "an occupied isolated source could not escape to a connected same-point copy");
    Require(NAI::GetTransitionType(network,source,target) == NAI::TT_NO_WAY,
        "a reverse same-point shortcut allowed entry into an isolated destination");
    targetTile.nMoveCrouch = 1;
    Require(NAI::GetTransitionType(network,source,target) != NAI::TT_NO_WAY,
        "a structurally connected crouch alias was rejected by transition classification");
    targetTile.nMoveCrouch = 0;
    targetTile.nFlags |= NAI::TF_HAS_INTERGRID;
    Require(NAI::GetTransitionType(network,source,target) != NAI::TT_NO_WAY,
        "an intergrid-connected alias was rejected by transition classification");
}

static void ConnectedDirectTransitions()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    for (int i = 0; i < 3; ++i)
        network->CreateLayer(4,4,VNULL2,0.f,i,i ? network->GetLayer(0)->pGroup.GetPtr() : nullptr);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 100;
    connected.nPassable = 7;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE;
    const NAI::SPathPlace source(1,1,1), destination(2,1,2);
    auto& lower = network->GetLayer(0)->tiles[1][1];
    lower = network->GetLayer(1)->tiles[1][1] = connected;
    network->GetLayer(1)->tiles[1][1].nFlags |= NAI::TF_HAS_SAME | NAI::TF_HAS_INTERGRID;
    auto& target = network->GetLayer(2)->tiles[1][2];
    target = connected;
    target.nHeight = 110;
    NAI::CNodesLayer::CTransitionsHash incoming;
    incoming[source].links.push_back(NAI::CNodesLayer::SLink(destination,10));
    auto install = [&]() {
        for (int i = 0; i < 3; ++i) network->GetLayer(i)->transitions.clear();
        network->AddDirectTransitions(incoming);
    };
    auto requireTransition = [&]() {
        const auto& forward = network->GetLayer(1)->transitions;
        const auto& reverse = network->GetLayer(2)->transitions;
        const auto f = forward.find(source), r = reverse.find(destination);
        Require(f != forward.end() && r != reverse.end() && f->second.links.size() == 1 &&
            r->second.links.size() == 1 && f->second.links.front().dst == destination &&
            r->second.links.front().dst == source && f->second.links.front().nHeight == 10 &&
            r->second.links.front().nHeight == 10,
            "an isolated lower copy suppressed a connected bidirectional navigation transition");
        Require(NAI::GetTransitionType(network,source,destination) == NAI::TT_INTERGRID &&
            NAI::GetTransitionType(network,destination,source) == NAI::TT_INTERGRID,
            "the retained direct transition could not be traversed in both directions");
    };
    lower.nMoveStand = 0;
    install();
    requireTransition();
    lower.nMoveStand = 1;
    install();
    Require(network->GetLayer(1)->transitions.empty() && network->GetLayer(2)->transitions.empty(),
        "a connected canonical lower copy no longer suppressed its duplicate transition");
    lower.nPassable = 0;
    install();
    requireTransition();
    lower = connected;
    ++lower.nDisplacement;
    install();
    requireTransition();
}

static void NearbyConnectedDestination()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(4,4,VNULL2,0.f,0);
    network->CreateLayer(4,4,VNULL2,0.f,1,network->GetLayer(0)->pGroup);
    network->Freeze(true);
    NAI::STile connected{};
    connected.nHeight = 100;
    connected.nPassable = 7;
    connected.nMoveStand = 1;
    connected.nFlags = NAI::TF_STAND_PASSABLE;
    auto* upper = network->GetLayer(1);
    auto& lowerTile = network->GetLayer(0)->tiles[1][1];
    auto& upperTile = upper->tiles[1][1];
    upperTile = lowerTile = connected;
    lowerTile.nMoveStand = 0;
    const NAI::SPathPlace destination(1,1,1);
    const SSphere sphere(network->GetCP(destination),0.05f);
    auto nearby = [&](bool all) {
        vector<NAI::SPathPlace> result;
        upper->AddNearPoints(sphere,&result,all);
        vector<int> layers;
        for (const auto& place: result) {
            Require(place.IsIntegral() && place.GetX() == 1 && place.GetY() == 1,
                "nearby fixture returned a point outside the requested sphere");
            layers.push_back(place.GetLayer());
        }
        return layers;
    };
    Require(nearby(true) == vector<int>({1}),
        "nearby fixture did not locate exactly its connected upper destination");
    Require(nearby(false) == vector<int>({1}),
        "nearby destination deduplication discarded a connected tile in favor of an isolated lower copy");
    lowerTile.nMoveStand = 1;
    Require(nearby(false).empty(), "nearby search did not retain canonical lower connected copies");
    lowerTile.nPassable = 1;
    Require(nearby(false) == vector<int>({1}),
        "a lower copy with fewer supported poses suppressed the better destination");
    lowerTile = connected;
    ++lowerTile.nHeight;
    Require(nearby(false) == vector<int>({1}),
        "a lower tile at a different height suppressed the destination");
    lowerTile = connected;
    lowerTile.nLocks = 1;
    lowerTile.nDynLocks = 1;
    Require(nearby(false).empty() && lowerTile.nLocks == 1 && lowerTile.nDynLocks == 1,
        "temporary occupancy changed structural deduplication or its lock counters");
}

static void FlyingShootDirection()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8,8,VNULL2,0.f,0);
    network->CreateLayer(8,8,VNULL2,FP_PI2,1);
    network->Freeze(true);
    for (int n = 0; n < 2; ++n)
        for (int y = 0; y < 8; ++y)
            for (int x = 0; x < 8; ++x) {
                network->GetLayer(n)->tiles[y][x] = NAI::STile{};
                network->GetLayer(n)->tiles[y][x].nPassable = 7;
                network->GetLayer(n)->tiles[y][x].nFlags = NAI::TF_STAND_PASSABLE;
            }
    NAI::SPathPlace ground(2,2,0), rotated(2,2,1);
    ground.SetPose(NAI::CM_STAND); rotated.SetPose(NAI::CM_CROUCH);
    Require(NRPG::GetShootDirection(network,ground,network->GetCP(ground)+CVec3(10,0,0)) == NAI::RIGHT,
        "shoot-direction control did not face east on the ordinary layer");
    Require(NRPG::GetShootDirection(network,rotated,network->GetCP(rotated)+CVec3(10,0,0)) == NAI::DOWN,
        "shoot-direction control ignored the rotated ground layer");
    NAI::SPosition flying;
    Require(NAI::MakeFlyPos(CVec3(2,2,16),network,&flying), "shoot-direction fixture could not encode an airborne position");
    Require(flying.p.IsFinal() && flying.p.GetLayer() >= network->GetNumLayers(),
        "shoot-direction fixture altitude must differ from a valid navigation layer");
    const auto before = flying.p.GetBits();
    std::printf("Shoot direction: final=%d altitude-bits=%d navigation-layers=%d\n",
        int(flying.p.IsFinal()),flying.p.GetLayer(),network->GetNumLayers());
    std::fflush(stdout);
    const CVec3 origin = flying.GetCP();
    Require(NRPG::GetShootDirection(network,flying.p,origin+CVec3(10,0,0)) == NAI::RIGHT,
        "an airborne shot did not face east using the base-layer axes");
    Require(NRPG::GetShootDirection(network,flying.p,origin+CVec3(0,10,0)) == NAI::UP,
        "an airborne shot did not face north using the base-layer axes");
    Require(NRPG::GetShootDirection(network,flying.p,origin+CVec3(-10,0,0)) == NAI::LEFT,
        "an airborne shot did not face west using the base-layer axes");
    Require(flying.p.GetBits() == before, "shoot-direction lookup changed the airborne position or pose");
}

static void FlyingDynamicLocks()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    auto* layer = network->GetLayer(0);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            layer->tiles[y][x] = NAI::STile{};
            layer->tiles[y][x].nPassable = 7;
            layer->tiles[y][x].nFlags = NAI::TF_STAND_PASSABLE;
        }
    NAI::SPosition flying;
    Require(NAI::MakeFlyPos(CVec3(2,2,16), network, &flying), "flying-lock fixture could not encode its airborne position");
    Require(flying.p.IsFinal() && flying.p.IsIntegral() && flying.p.GetLayer() >= network->GetNumLayers(),
        "flying-lock fixture did not encode altitude in the layer bits");
    NAI::SPathPlace first(1,1,0), second(2,1,0);
    first.SetPose(NAI::CM_STAND); second.SetPose(NAI::CM_STAND);
    CObj<LockerActor> actor = new LockerActor;
    std::printf("Dynamic locks: airborne final=%d integral=%d altitude-bits=%d navigation-layers=%d\n",
        int(flying.p.IsFinal()), int(flying.p.IsIntegral()), flying.p.GetLayer(), network->GetNumLayers());
    std::fflush(stdout);
    network->ChangeDynamicLocks(actor, vector<NAI::SPathPlace>{first, flying.p, second, second});
    Require(layer->tiles[1][1].nDynLocks == 1 && layer->tiles[1][2].nDynLocks == 1,
        "skipping an airborne point changed the ground route's dynamic locks or duplicate filtering");
    network->ClearDynamicLocks(actor);
    Require(layer->tiles[1][1].nDynLocks == 0 && layer->tiles[1][2].nDynLocks == 0,
        "clearing a mixed airborne route retained ground dynamic locks");
    network->ChangeDynamicLocks(actor, vector<NAI::SPathPlace>{flying.p});
    network->ClearDynamicLocks(actor);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            Require(layer->tiles[y][x].nDynLocks == 0 && layer->tiles[y][x].nLocks == 0,
                "an airborne-only route locked a ground cell");
}

static void RemovedSelectedAIContact(int kind)
{
    CObj<ThreatOwnerFixture> owner = new ThreatOwnerFixture;
    CObj<NAI::AIUnitFixture> first = new NAI::AIUnitFixture, second = new NAI::AIUnitFixture;
    auto& state = owner->threat;
    state.SetUnit(owner);
    auto* selected = kind == 0 ? &state.pEnemy : kind == 1 ? &state.pPossibleEnemy : &state.pAlly;
    auto* collection = kind == 0 ? &state.enemies : kind == 1 ? &state.possibleEnemies : &state.allies;
    auto add = [&](NAI::IAIUnit* unit) {
        if (kind == 0) state.AddEnemy(unit);
        else if (kind == 1) state.AddPossibleEnemy(unit);
        else state.AddAlly(unit);
    };
    auto remove = [&](NAI::IAIUnit* unit) {
        if (kind == 0) state.RemoveEnemy(unit);
        else if (kind == 1) state.RemovePossibleEnemy(unit);
        else state.RemoveAlly(unit);
    };
    add(first); add(second); *selected = first.GetPtr();
    state.selfModified.data = false;
    collection->bModified = false;
    remove(second);
    Require(selected->GetPtr() == first.GetPtr() && collection->data.units.size() == 1,
        "removing an unrelated AI contact cleared the selected contact");
    Require(collection->bModified && !state.selfModified.data,
        "removing an unrelated contact lost collection invalidation or reported a new threat");
    collection->bModified = false;
    remove(first);
    Require(!IsValid(*selected), "removing the selected AI contact left a stale target for subsequent reactions");
    Require(collection->data.units.empty() && collection->bModified && !state.selfModified.data,
        "removing a selected contact lost collection invalidation or synthesized a threat change");
    // Removal must also clear a selected reference absent from the member list,
    // as can happen while an event consumes a contact with updates deferred.
    *selected = second.GetPtr();
    remove(second);
    Require(!IsValid(*selected) && !state.selfModified.data,
        "removing an already-consumed AI contact retained its selected reference");
}

class ThreatActionProbe: public NAI::CAIAction
{
    OBJECT_NOCOPY_METHODS(ThreatActionProbe);
public:
    explicit ThreatActionProbe(NAI::IAIUnit* unit = nullptr): CAIAction(unit) {}
    using NAI::CAIAction::GetEnemy;
    void Do(NAI::CAILog*) const override {}
    bool ComparePlaces(const NAI::SPlaceWithAP&, const NAI::SPlaceWithAP&) const override { return false; }
    bool CanDo(const NAI::SPlaceWithAP&) override { return false; }
    void ResetInfoHash() override {}
};

class ThreatPlaceSourceProbe: public NAI::CAIActionPlaceSource
{
    OBJECT_NOCOPY_METHODS(ThreatPlaceSourceProbe);
public:
    explicit ThreatPlaceSourceProbe(NAI::IAIUnit* unit = nullptr): CAIActionPlaceSource(unit) {}
    using NAI::CAIActionPlaceSource::GetEnemy;
    void Prepare() override {}
};

static void OwnedAIEnemySelection(bool placeSource)
{
    CObj<ThreatOwnerFixture> owner = new ThreatOwnerFixture;
    CObj<NAI::AIUnitFixture> first = new NAI::AIUnitFixture, second = new NAI::AIUnitFixture;
    NAI::SAIState shared;
    owner->shared = &shared;
    owner->threat.pEnemy = first.GetPtr();
    shared.SetCurrentAIEnemy(second);
    CObj<ThreatActionProbe> action = new ThreatActionProbe(owner);
    CObj<ThreatPlaceSourceProbe> source = new ThreatPlaceSourceProbe(owner);
    auto selected = [&]() { return placeSource ? source->GetEnemy() : action->GetEnemy(); };
    Require(selected() == first.GetPtr(), "AI action selected another unit's shared target instead of its own contact");
    shared.SetCurrentAIEnemy(nullptr);
    Require(selected() == first.GetPtr(), "clearing the shared chooser context erased the acting unit's target");
    owner->threat.pEnemy = second.GetPtr();
    shared.SetCurrentAIEnemy(first);
    Require(selected() == second.GetPtr(), "AI action did not follow a change to its own threat target");
    owner->shared = nullptr;
    Require(selected() == second.GetPtr(), "own target lookup required a shared commander context");
    owner->threat.pEnemy = nullptr;
    Require(selected() == nullptr, "cleared own contact fell back to a shared target");
    owner->exposeThreat = false;
    Require(selected() == nullptr, "AI unit without a threat-state implementation was not handled safely");
    Require(owner->sharedReads == 0, "own-target lookup queried shared commander state");
    owner->Invalidate();
    Require(selected() == nullptr, "own-target lookup queried an invalid AI unit");
}

static void MountedAILogCommands()
{
    CObj<LockerActor> actor = new LockerActor;
    CObj<NAI::AIUnitFixture> unit = new NAI::AIUnitFixture;
    unit->server = actor.GetPtr();
    CObj<NWorld::CCannon> cannon = new NWorld::CCannon;
    CObj<NAI::CAILogUseCannon> enter = new NAI::CAILogUseCannon(unit, cannon);
    CObj<NAI::CAILogExitCannon> leave = new NAI::CAILogExitCannon(unit, cannon);
    list<CPtr<NWorld::CCommand>> commands;
    enter->GetCommands(&commands);
    Require(commands.size() == 2, "AI cannon entry lost its pose and docking commands during an incomplete-type cast");
    auto entry = commands.begin();
    auto* poseOrder = dynamic_cast<NWorld::CCmdSetCommand*>(entry++->GetPtr());
    auto* cannonOrder = dynamic_cast<NWorld::CCmdSetCommand*>(entry->GetPtr());
    Require(poseOrder && cannonOrder, "AI cannon entry emitted an unexpected world command");
    auto* pose = dynamic_cast<NWorld::CCmdWishPose*>(poseOrder->GetCmd());
    auto* dock = dynamic_cast<NWorld::CCmdCannon*>(cannonOrder->GetCmd());
    Require(pose && pose->pose == NAI::RUN && dock && dock->pObject.GetPtr() == static_cast<NWorld::IObject*>(cannon.GetPtr()),
        "AI cannon entry did not preserve its pose or exact cannon target");
    commands.clear();
    leave->GetCommands(&commands);
    Require(commands.size() == 1, "AI cannon exit lost its command during an incomplete-type cast");
    auto* exitOrder = dynamic_cast<NWorld::CCmdSetCommand*>(commands.front().GetPtr());
    auto* exit = exitOrder ? dynamic_cast<NWorld::CCmdExitCannon*>(exitOrder->GetCmd()) : nullptr;
    Require(exit && exit->pCannon.GetPtr() == static_cast<NWorld::IObject*>(cannon.GetPtr()),
        "AI cannon exit targeted another object");
    commands.clear();
    cannon->Invalidate();
    enter->GetCommands(&commands); leave->GetCommands(&commands);
    Require(commands.empty(), "invalid cannon generated an AI entry or exit command");
}

static void NavigationColouringOccupancy(int unavailable = 0)
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    auto* layer = network->GetLayer(0);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            auto& tile = layer->tiles[y][x];
            tile = NAI::STile{};
            tile.nMoveStand = static_cast<char>(0xff);
            tile.nPassable = NAI::CP_STAND;
            tile.nFlags = NAI::TF_STAND_PASSABLE;
        }
    CObj<NAI::CMapColourer> control = new NAI::CMapColourer;
    control->SetLayer(layer, nullptr, 0);
    control->ConstructColouring(network);
    Require(control->GetPointColor(3, 3) == control->GetPointColor(4, 3),
        "occupancy-colouring fixture has no connected open floor");
    CObj<LockerActor> actor = new LockerActor;
    NAI::SPathPlace occupied(3, 3, 0);
    occupied.SetPose(NAI::CM_STAND);
    network->Lock(actor, occupied);
    Require(network->IsLocked(occupied, true) && network->GetWhoLocksThisPlace(occupied) == actor.GetPtr(),
        "occupancy-colouring fixture did not register a live actor footprint");
    CObj<NAI::IAIJob> job = dynamic_cast<NAI::IAIJob*>(pSSClasses->CreateObject(0x00513131));
    Require(IsValid(job), "native recolouring job class is not registered");
    CObj<NAI::CLayersGroup> group = layer->pGroup;
    CStructureNetworkContext references;
    references.externalReferences = true;
    references.ids[group.GetPtr()] = 1; references.refs[1] = group.GetPtr();
    references.nextID = 100; references.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); writer.Add(3, &group); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &references);
      static_cast<CObjectBase*>(job.GetPtr())->operator&(reader); }
    if (unavailable) {
        if (unavailable == 1) layer->pColourer = nullptr;
        else network->GetColourer(0)->Invalidate();
        Require(!IsValid(layer->pColourer), "unavailable colouring fixture retained a valid cache");
    }
    job->DoJob();
    auto* actual = network->GetColourer(0);
    Require(actual->IsReady(), "native recolouring job did not build its layer cache");
    Require(actual->GetPointColor(3, 3) == actual->GetPointColor(4, 3),
        "navigation rebuild cached a standing actor's temporary footprint as disconnected scenery");
    Require(network->IsLocked(occupied, true) && network->GetWhoLocksThisPlace(occupied) == actor.GetPtr()
            && layer->tiles[3][3].nLocks == 1,
        "navigation rebuild failed to restore the actor's original occupancy lock");
    network->Unlock(actor);
    Require(!network->IsLocked(occupied, true), "recolouring changed occupancy ownership or duplicated its lock");
}

class ReactionProbe: public NAI::CAIReaction
{
    OBJECT_NOCOPY_METHODS(ReactionProbe);
public:
    int updates = 0;
    ReactionProbe() {}
    explicit ReactionProbe(NAI::IAIUnit* unit): CAIReaction(unit) {}
    void Update() override { ++updates; }
};

static void StaleAIReaction()
{
    for (int staleReason = 0; staleReason < 4; ++staleReason) {
        CObj<LockerActor> actor = new LockerActor, liveActor = new LockerActor;
        CObj<NAI::AIUnitFixture> unit = new NAI::AIUnitFixture, liveUnit = new NAI::AIUnitFixture;
        unit->server = actor; liveUnit->server = liveActor;
        CObj<ReactionProbe> stale = new ReactionProbe(unit), live = new ReactionProbe(liveUnit);
        NAI::SAIUpdateTracker tracker;
        tracker.Add(stale); tracker.Add(live);
        // State changes after the reaction has already been enqueued.
        if (staleReason == 0) actor->active = false;
        if (staleReason == 1) unit->underAIControl = false;
        if (staleReason == 2) unit->server = nullptr;
        if (staleReason == 3) unit->Invalidate();
        tracker.Update();
        Require(stale->updates == 0, "queued AI reaction updated an actor that can no longer operate");
        Require(live->updates == 0 && !tracker.CanGetCommand(), "AI update drained more than one queued reaction");
        tracker.Update();
        Require(live->updates == 1 && tracker.CanGetCommand(), "live AI reaction did not resume on the next segment");
    }
}

static void OverlappingUnitLocks()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    NAI::SPathPlace shared(3, 3, 0), other(4, 3, 0);
    shared.SetPose(NAI::CM_STAND); other.SetPose(NAI::CM_STAND);
    auto* layer = network->GetLayer(0);
    layer->tiles[3][3].nPassable = layer->tiles[3][4].nPassable = NAI::CP_STAND;
    CObj<LockerActor> first = new LockerActor, second = new LockerActor;
    network->LockMovingObject(first, shared, other);
    network->Lock(second, shared);
    Require(network->IsLocked(shared, true) && network->IsLocked(other, true), "invalid lock fixture");
    network->Unlock(first);
    Require(network->IsLocked(shared, true), "unlocking one unit cleared another unit's overlapping footprint");
    Require(!network->IsLocked(other, true), "exclusive footprint remained locked after its owner left");
    network->Unlock(second);
    Require(!network->IsLocked(shared, true), "last footprint owner did not release the tile");

    // Selected-only queries deliberately disable everyone except the selection.
    network->Lock(first, shared);
    network->Lock(second, shared);
    network->LockSelected(std::list<CObjectBase*>{first.GetPtr()});
    network->Unlock(first);
    Require(!network->IsLocked(shared, true), "disabled unselected owner was incorrectly relocked");
    network->UnlockSelected();
    Require(network->IsLocked(shared, true), "selection query lost the surviving unit's lock");
    second->Invalidate();
    network->Lock(first, shared);
    network->Unlock(first);
    Require(!network->IsLocked(shared, true), "overlap restoration revived an invalid owner's footprint");
    network->Unlock(second);
}

static void CheckStaleUnitLocks(NWorld::CUnitServer* actor, int path, bool dead)
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    NAI::SPathPlace shared(3, 3, 0), other(4, 3, 0);
    shared.SetPose(NAI::CM_STAND); other.SetPose(NAI::CM_STAND);
    auto* layer = network->GetLayer(0);
    layer->tiles[3][3].nPassable = layer->tiles[3][4].nPassable = NAI::CP_STAND;
    network->Lock(actor, shared);
    Require(network->IsLocked(shared, true), "live actor did not lock its footprint");
    if (path == 2) network->LockSelected(std::list<CObjectBase*>{actor});
    if (dead) {
        actor->GetUnitRPG()->GetRPGUnit()->Kill();
        actor->SetState(new NWorld::CUnitStateDeath(actor));
        Require(actor->IsDead() && !actor->IsWearingPK(), "fixture actor is not an ordinary corpse");
    } else actor->Invalidate();
    Require(!NAI::IsLockerUnit(actor), "stale actor unexpectedly remains an eligible locker");
    if (path == 0) network->Lock(actor, shared);
    if (path == 1) network->LockMovingObject(actor, shared, other);
    if (path == 2) network->UnlockSelected();
    Require(!network->IsLocked(shared, true) && !network->IsLocked(other, true),
        "path query restored an ineligible unit's movement footprint");
    network->Unlock(actor);
}

static void InvalidUnitLocks()
{
    for (int path = 0; path < 3; ++path) {
        CObj<LockerActor> actor = new LockerActor;
        CheckStaleUnitLocks(actor, path, false);
    }
}

static void DeadUnitLocks(const char* databasePath, const char* resourcePath, int path)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    for (auto& candidate : units) {
        auto* actor = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (actor && actor->CanFight() && !actor->IsEmptyPK() && !actor->IsWearingPK()) {
            CheckStaleUnitLocks(actor, path, true);
            return;
        }
    }
    throw std::runtime_error("dead-lock mission fixture contains no ordinary live actor");
}

class ScriptActor: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(ScriptActor);
public:
    bool shell = false;
    int orders = 0;
    CPtr<NWorld::CPlayer> owner;
    bool IsEmptyPK() const override { return shell; }
    bool CanFight() const override { return !shell; }
    NWorld::IPlayer* GetPlayer() const override { return owner; }
    void Do(NWorld::CCommand* command) override { CObj<NWorld::CCommand> hold = command; ++orders; }
};

static void RunScript(NScript::CScript* script, const char* code)
{
    Require(script->DoString(code) == 0, "script compilation failed");
    pScript = script;
    script->ExecuteThreads();
    pScript = nullptr;
}

enum class DeferredScriptProducer { Animation, Direction, Pose, WishPose, ShootMode, Reload, ActivateWeapon, Hide };

static void DeferredUnitScriptCommand(const char* databasePath,const char* resourcePath,
    DeferredScriptProducer producer = DeferredScriptProducer::Animation,bool mixedOnly = false)
{
    const bool direction = producer == DeferredScriptProducer::Direction;
    const bool pose = producer == DeferredScriptProducer::Pose;
    const bool wishPose = producer == DeferredScriptProducer::WishPose;
    const bool shootMode = producer == DeferredScriptProducer::ShootMode;
    const bool reload = producer == DeferredScriptProducer::Reload;
    const bool activateWeapon = producer == DeferredScriptProducer::ActivateWeapon;
    const bool hide = producer == DeferredScriptProducer::Hide;
    const bool animationMode = producer == DeferredScriptProducer::Animation;
    auto world = FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto unit : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (!candidate || !candidate->CanFight()) continue;
        if (shootMode || reload || activateWeapon) {
            auto* weapon = candidate->GetUnitRPG()->GetWeaponItem();
            if (candidate->IsEmptyPK() || candidate->IsWearingPK() || candidate->animator.GetCannon() || !weapon) continue;
            if (shootMode && (!weapon->IsShootModeSupported(NDb::SM_Snap) || !weapon->IsShootModeSupported(NDb::SM_Aimed))) continue;
            if (reload) {
                auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(weapon);
                if (!firearm || firearm->GetAmmoQuantity() <= 0) continue;
                firearm->DiscardAmmo();
                if (!weapon->CanReload(candidate->GetUnitRPG()->GetInventory())) continue;
            }
        } else if (candidate->IsAIUnit()) continue;
        actor = candidate; break;
    }
    Require(actor != nullptr,"deferred animation fixture has no actual roster actor");
    if (hide) {
        Require(!actor->IsHiding(),"deferred hide fixture started with an already hidden actor");
        actor->EnableHide();
    }
    auto* commander = actor->GetPlayer()->GetCommander();
    Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
        "deferred animation fixture started with a pending native order");
    auto* animation = NDb::GetAnimation(4437);
    if (animationMode) Require(IsValid(animation),"deferred animation fixture has no authored script-animation record");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
    const auto position = actor->GetPosition(); const int ap = actor->GetAP();
    const auto originalWishPose = actor->GetWishPose();
    const bool originalActiveItem = actor->animator.IsActiveItem();
    const bool originalHiding = actor->IsHiding();
    auto* weapon = (shootMode || reload || activateWeapon) ? actor->GetUnitRPG()->GetWeaponItem() : nullptr;
    auto* clip = weapon ? weapon->GetInnerClip() : nullptr;
    const int ammo = weapon ? weapon->GetAmmoQuantity() : 0;
    vector<std::pair<CPtr<NRPG::IInventoryItem>,int>> backpack;
    if (reload) for (const auto& entry : actor->GetUnitRPG()->GetInventory()->GetItems()) {
        auto* item = dynamic_cast<NRPG::CClipItem*>(entry.pItem.GetPtr());
        backpack.emplace_back(entry.pItem,item ? item->GetIncQuantity() : -1);
    }
    if (shootMode) Require(weapon && weapon->SetShootMode(NDb::SM_Snap),"deferred fire-mode fixture cannot select its native weapon's snap mode");
    const vector<bool> options = mixedOnly ? vector<bool>{} : (reload || hide) ? vector<bool>{false} : vector<bool>{false,true};
    for (bool freeze : options) {
        const int requestedDirection = (position.pos.p.GetDirection()+(freeze ? 7 : 1))&7;
        const auto requestedPose = freeze ? NAI::WALK : NAI::CRAWL;
        const auto requestedMode = freeze ? NDb::SM_Snap : NDb::SM_Aimed;
        string call;
        if (direction) call = "UnitSetDirection(upstreamActor,"+std::to_string(requestedDirection)+")";
        else if (pose) call = "UnitSetPose(upstreamActor,"+std::to_string(int(requestedPose))+")";
        else if (wishPose) call = "UnitSetWishPose(upstreamActor,"+std::to_string(int(requestedPose))+")";
        else if (shootMode) call = "UnitSetShootMode(upstreamActor,"+std::to_string(int(requestedMode))+")";
        else if (reload) call = "UnitReload(upstreamActor)";
        else if (activateWeapon) call = "UnitActivateWeapon(upstreamActor,"+string(freeze ? "1" : "nil")+")";
        else if (hide) call = "UnitHide(upstreamActor)";
        else call = "UnitPlayAnimation(upstreamActor,4437,nil,"+string(freeze ? "1" : "nil")+")";
        const string code = call+"; UpstreamAnimationReturned = 19";
        RunScript(script,code.c_str());
        auto returned = script->GetGlobal("UpstreamAnimationReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 19,"native animation script did not finish its call");
        if (direction)
            std::printf("native script direction dispatch: requested=%d commander-pending=%d unit-command=%d\n",
                requestedDirection,int(commander->HasCommands()),int(actor->HasCommand()));
        else if (pose)
            std::printf("native script pose dispatch: requested=%d commander-pending=%d unit-command=%d\n",
                int(requestedPose),int(commander->HasCommands()),int(actor->HasCommand()));
        else if (wishPose)
            std::printf("native script preferred-pose dispatch: requested=%d commander-pending=%d unit-command=%d before=%d after=%d\n",
                int(requestedPose),int(commander->HasCommands()),int(actor->HasCommand()),int(originalWishPose),int(actor->GetWishPose()));
        else if (shootMode)
            std::printf("native script fire-mode dispatch: requested=%d commander-pending=%d unit-command=%d live-mode=%d\n",
                int(requestedMode),int(commander->HasCommands()),int(actor->HasCommand()),int(weapon->GetShootMode()));
        else if (reload)
            std::printf("native script reload dispatch: commander-pending=%d unit-command=%d ammo-before=%d ammo-after=%d\n",
                int(commander->HasCommands()),int(actor->HasCommand()),ammo,weapon->GetAmmoQuantity());
        else if (activateWeapon)
            std::printf("native script weapon activation: requested=%d commander-pending=%d unit-command=%d walk-without-weapon=%d\n",
                int(freeze),int(commander->HasCommands()),int(actor->HasCommand()),int(actor->IsWalkingWithoutWeapon()));
        else if (hide)
            std::printf("native script hide dispatch: commander-pending=%d unit-command=%d before=%d after=%d\n",
                int(commander->HasCommands()),int(actor->HasCommand()),int(originalHiding),int(actor->IsHiding()));
        else
            std::printf("native script animation dispatch: freeze=%d commander-pending=%d unit-command=%d\n",
                int(freeze),int(commander->HasCommands()),int(actor->HasCommand()));
        Require(commander->HasCommands() && !actor->HasCommand(),
            direction ? "native script direction executes immediately instead of entering the owning commander's queue" :
            pose ? "native script pose bypasses the owning commander's queue" :
            wishPose ? "native script preferred pose bypasses the owning commander's queue" :
            shootMode ? "native script fire mode bypasses the owning commander's queue" :
            reload ? "native script reload bypasses the owning commander's queue" :
            activateWeapon ? "native script weapon activation bypasses the owning commander's queue" :
            hide ? "native script hide bypasses the owning commander's queue" :
            "native script animation executes immediately instead of entering the owning commander's queue");
        CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
        auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
        auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
        Require(set && set->pUnit == actor,"queued native script command lost its target actor");
        if (direction) {
            auto* look = dynamic_cast<NWorld::CCmdLook*>(set->GetCmd());
            auto expected = position.pos; expected.p.SetDirection(requestedDirection);
            Require(look && look->ptDst.p == expected.p && look->ptDst.pNet == expected.pNet,
                "queued native script turn lost its LOOK type, direction, tile, pose or network");
        } else if (pose) {
            auto* path = dynamic_cast<NWorld::CCmdPath*>(set->GetCmd());
            auto expected = position; expected.SetPose(requestedPose);
            Require(path && path->ptDst.p == expected.pos.p && path->ptDst.pNet == expected.pos.pNet &&
                path->eParams == NAI::PF_USE_POSEDIR,
                "queued native script pose lost its PATH type, pose/direction option, tile, heading or network");
        } else if (wishPose) {
            auto* preference = dynamic_cast<NWorld::CCmdWishPose*>(set->GetCmd());
            Require(preference && preference->pose == requestedPose,
                "queued native script preferred pose lost its WISHPOSE type or requested value");
        } else if (shootMode) {
            auto* mode = dynamic_cast<NWorld::CCmdShootMode*>(set->GetCmd());
            Require(mode && mode->eMode == requestedMode,"queued native script fire mode lost its type or requested value");
        } else if (reload) {
            auto* request = dynamic_cast<NWorld::CCmdReload*>(set->GetCmd());
            Require(request && request->nSlot == -1,"queued native script reload lost its type or active-weapon slot");
        } else if (activateWeapon) {
            auto* path = dynamic_cast<NWorld::CCmdPath*>(set->GetCmd());
            Require(path && path->ptDst.p == position.pos.p && path->ptDst.pNet == position.pos.pNet &&
                path->eParams == NAI::PF_DEFAULT && !path->bLeaveZone && !path->bCanFindNotExact &&
                path->needActiveItem == (freeze ? NWorld::ITEM_ACTIVE : NWorld::ITEM_INACTIVE),
                "queued native weapon activation lost its PATH destination, flags or requested hand state");
        } else if (hide) {
            Require(dynamic_cast<NWorld::CCmdHide*>(set->GetCmd()) != nullptr,
                "queued native script hide lost its original command type");
        } else {
            auto* play = dynamic_cast<NWorld::CCmdPlayAnimation*>(set->GetCmd());
            Require(play && play->nDBAnimationID == 4437 && play->bFreezeAfterLastFrame == freeze,
                "queued native animation lost its clip or freeze parameter");
        }
        Require(resume && resume->pUnit == actor && dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) &&
            !commander->HasCommands(),"native animation did not queue exactly one ordered Continue");
        Require(!actor->HasCommand() && actor->GetAP() == ap &&
            actor->GetPosition().pos.p == position.pos.p,"queueing a script animation executed an actor action");
        if (wishPose) Require(actor->GetWishPose() == originalWishPose,
            "native preferred pose changed before its queued command was consumed");
        if (shootMode) Require(actor->GetUnitRPG()->GetWeaponItem() == weapon && weapon->GetShootMode() == NDb::SM_Snap,
            "native weapon mode changed before its queued script command was consumed");
        if (activateWeapon) Require(actor->IsWalkingWithoutWeapon() == !freeze &&
            actor->animator.IsActiveItem() == originalActiveItem &&
            actor->GetUnitRPG()->GetWeaponItem() == weapon && weapon->GetAmmoQuantity() == ammo,
            "queued weapon activation lost the immediate persistent holster preference or changed the live weapon");
        if (hide) Require(actor->IsHiding() == originalHiding,"native script changed hiding state before queued command consumption");
        if (reload) {
            const auto& items = actor->GetUnitRPG()->GetInventory()->GetItems();
            Require(actor->GetUnitRPG()->GetWeaponItem() == weapon && weapon->GetInnerClip() == clip &&
                weapon->GetAmmoQuantity() == ammo && items.size() == backpack.size(),
                "native script reload changed the weapon, inner clip or backpack before command consumption");
            for (std::size_t i=0; i<items.size(); ++i) {
                auto* item = dynamic_cast<NRPG::CClipItem*>(items[i].pItem.GetPtr());
                Require(items[i].pItem == backpack[i].first && (item ? item->GetIncQuantity() : -1) == backpack[i].second,
                    "native script reload consumed or replaced backpack ammunition before queued command consumption");
            }
        }
    }
    if (animationMode) {
        RunScript(script,"UnitPlayAnimation(upstreamActor,4437,1); UpstreamAnimationReturned = 23");
        auto returned = script->GetGlobal("UpstreamAnimationReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 23 && !commander->HasCommands() && !actor->HasCommand(),
            "native custom-idle animation was incorrectly converted into a queued action");
    }
    if (mixedOnly) {
        Require(shootMode && IsValid(animation),"mixed script fixture has no native weapon or authored clip");
        RunScript(script,"UnitSetDirection(upstreamActor,1); UnitSetShootMode(upstreamActor,1); "
            "UnitSetWishPose(upstreamActor,0); UnitPlayAnimation(upstreamActor,4437,nil,nil); UpstreamMixedReturned = 31");
        auto returned = script->GetGlobal("UpstreamMixedReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 31,"mixed native script did not finish its four calls");
        vector<CObj<NWorld::CCommand>> pending;
        while (commander->HasCommands()) pending.push_back(commander->GetCommand());
        std::printf("native mixed script order: commands=%zu expected=8 live-mode=%d live-wish=%d\n",
            pending.size(),int(weapon->GetShootMode()),int(actor->GetWishPose()));
        Require(pending.size() == 8 && weapon->GetShootMode() == NDb::SM_Snap &&
            actor->GetWishPose() == originalWishPose && !actor->HasCommand(),
            "native mixed script loses a queued request or changes live state ahead of its command order");
        for (int i=0; i<8; ++i) {
            auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(pending[i].GetPtr());
            Require(set && set->pUnit == actor,"mixed native queue lost its command wrapper or target actor");
            auto* payload = set->GetCmd();
            bool matches = false;
            if (i%2) matches = dynamic_cast<NWorld::CCmdContinue*>(payload) != nullptr;
            else if (i == 0) { auto* look = dynamic_cast<NWorld::CCmdLook*>(payload); matches = look && look->ptDst.p.GetDirection() == 1; }
            else if (i == 2) { auto* mode = dynamic_cast<NWorld::CCmdShootMode*>(payload); matches = mode && mode->eMode == NDb::SM_Aimed; }
            else if (i == 4) { auto* wish = dynamic_cast<NWorld::CCmdWishPose*>(payload); matches = wish && wish->pose == NAI::CRAWL; }
            else if (i == 6) { auto* play = dynamic_cast<NWorld::CCmdPlayAnimation*>(payload); matches = play && play->nDBAnimationID == 4437 && !play->bFreezeAfterLastFrame; }
            Require(matches,"native mixed script commands arrived in a different order or lost their parameters");
        }
        Require(actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p,
            "queueing the mixed native script moved its actor or spent action points");
    }
}

static void DeferredScriptShot(const char* databasePath,const char* resourcePath,bool prepare = false)
{
    auto world = FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NWorld::CUnitServer* target = nullptr;
    for (auto entry : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!candidate || !candidate->CanFight() || candidate->IsEmptyPK() || candidate->IsWearingPK() ||
            candidate->animator.GetCannon() || !dynamic_cast<NRPG::CWeaponItem*>(candidate->GetUnitRPG()->GetWeaponItem())) continue;
        for (auto other : units) {
            auto* enemy = dynamic_cast<NWorld::CUnitServer*>(other.GetPtr());
            if (enemy && enemy != candidate && enemy->CanFight() && !enemy->IsEmptyPK() && !enemy->IsWearingPK() &&
                world->GetDiplomacyState(candidate->GetPlayer(),enemy->GetPlayer()) == NDb::DS_ENEMY) {
                actor = candidate; target = enemy; break;
            }
        }
        if (actor) break;
    }
    Require(actor && target,"deferred shot fixture has no real armed actor and distinct living enemy");
    auto* weapon = actor->GetUnitRPG()->GetWeaponItem();
    const int ammo = weapon->GetAmmoQuantity(), ap = actor->GetAP();
    const auto position = actor->GetPosition();
    NRPG::SUnitInfo before; target->GetInfo(&before);
    auto* commander = actor->GetPlayer()->GetCommander();
    Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
        "deferred shot fixture started with an existing native order");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
    NScript::luaPushCPtr(*script,target); script->SetGlobal("upstreamTarget");
    const string function = prepare ? "UnitShootPrepare" : "UnitShoot";
    const string nilControl = function+"(upstreamActor,nil,0); "+function+"(nil,upstreamTarget,0); UpstreamShotReturned = 13";
    RunScript(script,nilControl.c_str());
    auto returned = script->GetGlobal("UpstreamShotReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 13 && !commander->HasCommands() && !actor->HasCommand(),
        "deferred shot lost the fork's nil hardening or issued an order for a null actor/target");
    for (auto location : {NAI::HL_BODY,NAI::HL_HEAD}) {
        const string call = function+"(upstreamActor,upstreamTarget,"+std::to_string(int(location))+"); UpstreamShotReturned = 17";
        RunScript(script,call.c_str());
        returned = script->GetGlobal("UpstreamShotReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 17,"deferred shot Lua callback did not finish");
        std::printf("native script shot dispatch: prepare=%d location=%d commander-pending=%d unit-command=%d\n",
            int(prepare),int(location),int(commander->HasCommands()),int(actor->HasCommand()));
        Require(commander->HasCommands() && !actor->HasCommand(),
            "native script shot bypasses the owning commander's queue");
        CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
        auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
        auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
        auto* shot = set ? dynamic_cast<NWorld::CCmdShootObject*>(set->GetCmd()) : nullptr;
        Require(set && set->pUnit == actor && shot && shot->pTarget == target && shot->eHL == location &&
            shot->nExtraAttackAP == 0 && shot->bOnlyPrepareToShoot == prepare && !shot->bCanBeReplacedByReload &&
            resume && resume->pUnit == actor && dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) &&
            !commander->HasCommands(),"queued native shot lost its target, hit location, flags, actor or ordered Continue");
        NRPG::SUnitInfo after; target->GetInfo(&after);
        Require(!actor->HasCommand() && actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p &&
            actor->GetUnitRPG()->GetWeaponItem() == weapon && weapon->GetAmmoQuantity() == ammo &&
            target->CanFight() && after.nHP == before.nHP,
            "native script shot changed actor/AP/ammunition or damaged the target before queued command consumption");
    }
}

static void DeferredWaypointCommand(const char* databasePath,const char* resourcePath,bool fly = true)
{
    auto world = FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr; NRPG::CWeaponItem* weapon = nullptr;
    for (auto entry : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!candidate || !candidate->CanFight() || candidate->IsEmptyPK() || candidate->IsWearingPK() ||
            candidate->animator.GetCannon() || candidate->GetPosition().pos.GetFloor() != 0) continue;
        auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
        if (firearm) { actor = candidate; weapon = firearm; break; }
    }
    Require(actor && weapon,"deferred waypoint fixture has no ordinary armed actor on the ground floor");
    const auto position = actor->GetPosition(); const int ap = actor->GetAP(), ammo = weapon->GetAmmoQuantity();
    auto* commander = actor->GetPlayer()->GetCommander();
    Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
        "deferred waypoint fixture started with an existing native order");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
    const string function = fly ? "UnitFlyToWaypoint" : "UnitAttackWaypoint";
    for (bool threeD : {false,true}) {
        CObj<NDb::CWaypointName> name = new NDb::CWaypointName;
        name->szName = threeD ? "upstream_dispatch_probe_fly" : "upstream_dispatch_probe_ground";
        CObj<CMapWaypoint> authored = new CMapWaypoint;
        authored->pName = name; authored->b3DWaypoint = threeD;
        authored->pos.ptPos = position.GetCP()+CVec3(2,0,threeD ? 2.f : 0.f);
        world->AddWaypoint(authored);
        auto* waypoint = world->GetWaypoint(name->szName);
        Require(waypoint && waypoint->pos.p.IsFinal() == threeD,
            "deferred waypoint fixture did not load its actual ground/fly point through the world's registry");
        const auto target = waypoint->pos;
        const string controls = function+"(nil,'"+name->szName+"'); "+function+
            "(upstreamActor,'upstream_dispatch_probe_missing'); UpstreamWaypointReturned = 23";
        RunScript(script,controls.c_str());
        auto returned = script->GetGlobal("UpstreamWaypointReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 23 && !commander->HasCommands() && !actor->HasCommand(),
            "native waypoint nil-actor/missing-point controls issued an order or stopped the script");
        const string call = function+"(upstreamActor,'"+name->szName+"'); UpstreamWaypointReturned = 29";
        RunScript(script,call.c_str());
        returned = script->GetGlobal("UpstreamWaypointReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 29,"native waypoint Lua callback did not finish");
        std::printf("native script waypoint dispatch: fly=%d three-dimensional=%d queued=%d unit-command=%d\n",
            int(fly),int(threeD),int(commander->HasCommands()),int(actor->HasCommand()));
        Require(commander->HasCommands() && !actor->HasCommand(),
            "native script waypoint command bypasses the owning commander's queue");
        CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
        auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
        auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
        Require(set && set->pUnit == actor && resume && resume->pUnit == actor &&
            dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) && !commander->HasCommands(),
            "queued waypoint command lost its actor or exact ordered Continue");
        if (fly) {
            auto* request = dynamic_cast<NWorld::CCmdFly*>(set->GetCmd());
            Require(request && request->dst.pos.p.IsFinal() && !request->dst.bRun,
                "queued fly request lost its command type, fly-place encoding or walk/run flag");
            const auto actual = request->dst.GetCP(), expected = target.GetCP();
            Require(std::fabs(actual.x-expected.x)<0.001f && std::fabs(actual.y-expected.y)<0.001f &&
                std::fabs(actual.z-expected.z)<=NAI::F_3D_STEP+0.001f,
                "queued fly destination differs from its native waypoint beyond fly-height quantization");
            if (threeD) Require(request->dst.pos.p == target.p && request->dst.pos.pNet == target.pNet,
                "queued fly request reinterpreted an already loaded 3D waypoint's position, altitude or heading bits");
        } else {
            auto* request = dynamic_cast<NWorld::CCmdShootTile*>(set->GetCmd());
            Require(request && fabs2(request->ptTarget-target.GetCP())<0.000001f && !request->bCanBeReplacedByReload,
                "queued waypoint attack lost its native tile destination, type or reload replacement flag");
        }
        Require(!actor->HasCommand() && actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p &&
            actor->GetUnitRPG()->GetInventory()->GetActive() == weapon && weapon->GetAmmoQuantity() == ammo &&
            waypoint->pos.p == target.p && waypoint->pos.pNet == target.pNet,
            "native waypoint producer moved the actor, changed AP/ammo or rewrote its loaded point before consumption");
    }
}

static void DeferredCorpseCommand(const char* databasePath,const char* resourcePath,bool drop = false)
{
    auto world = FixtureWorld(databasePath,resourcePath,true);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    vector<NWorld::CUnitServer*> ordinary;
    for (auto entry : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (candidate && candidate->CanFight() && !candidate->IsEmptyPK() && !candidate->IsWearingPK() &&
            !candidate->animator.GetCannon()) ordinary.push_back(candidate);
        if (ordinary.size() == 2) break;
    }
    Require(ordinary.size() == 2,"deferred corpse fixture has no two living ordinary actors");
    auto* actor = ordinary[0]; auto* body = ordinary[1];
    auto* commander = actor->GetPlayer()->GetCommander();
    Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
        "deferred corpse fixture started with an existing order");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
    NScript::luaPushCPtr(*script,body); script->SetGlobal("upstreamBody");
    RunScript(script,"UnitTakeCorpse(upstreamActor,upstreamBody); UpstreamCorpseReturned = 7");
    auto returned = script->GetGlobal("UpstreamCorpseReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 7 && !commander->HasCommands() && !actor->HasCommand() && body->CanFight(),
        "native script corpse pickup accepted a living target or stopped the callback");
    body->GetUnitRPG()->GetRPGUnit()->Kill();
    body->SetState(new NWorld::CUnitStateDeath(body));
    Require(actor->CanFight() && !actor->IsDead() && body->IsDead() && !body->CanFight() && !body->GetCorpseCarrier(),
        "deferred corpse fixture did not prepare an actual uncarried dead actor");
    CObj<NWorld::CExecCorpse> take;
    if (drop) {
        take = new NWorld::CExecCorpse(actor,body,true);
        take->Run();
        take->TimeLabelReached();
        Require(actor->GetCorpse() == body && actor->IsCarryingCorpse() && body->GetCorpseCarrier() == actor,
            "deferred drop fixture did not enter the native corpse-carry state and body attachment");
    }
    Require(!commander->HasCommands() && !actor->HasCommand(),"corpse fixture preparation issued an actor order");
    const auto position = actor->GetPosition(), bodyPosition = body->GetPosition(); const int ap = actor->GetAP();
    auto* previousCorpse = actor->GetCorpse(); auto* previousCarrier = body->GetCorpseCarrier();
    const bool carrying = actor->IsCarryingCorpse();
    const string controls = drop ? "UnitDropCorpse(nil); UpstreamCorpseReturned = 11" :
        "UnitTakeCorpse(nil,upstreamBody); UnitTakeCorpse(upstreamActor,nil); UpstreamCorpseReturned = 11";
    RunScript(script,controls.c_str());
    returned = script->GetGlobal("UpstreamCorpseReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 11 && !commander->HasCommands() && !actor->HasCommand(),
        "native corpse nil controls issued an order or stopped the callback");
    RunScript(script,drop ? "UnitDropCorpse(upstreamActor); UpstreamCorpseReturned = 13" :
        "UnitTakeCorpse(upstreamActor,upstreamBody); UpstreamCorpseReturned = 13");
    returned = script->GetGlobal("UpstreamCorpseReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 13,"native corpse Lua callback did not finish");
    std::printf("native script corpse dispatch: drop=%d queued=%d unit-command=%d carrying=%d\n",
        int(drop),int(commander->HasCommands()),int(actor->HasCommand()),int(actor->IsCarryingCorpse()));
    Require(commander->HasCommands() && !actor->HasCommand(),
        "native script corpse action bypasses the owning commander's queue");
    CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
    auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
    auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
    Require(set && set->pUnit == actor && resume && resume->pUnit == actor &&
        dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) && !commander->HasCommands(),
        "queued corpse command lost its actor or exact ordered Continue");
    if (drop) {
        auto* request = dynamic_cast<NWorld::CCmdDropCorpse*>(set->GetCmd());
        Require(request && !IsValid(request->pCorpse),
            "queued corpse drop lost its type or resolved the carrier-state body before consumption");
    } else {
        auto* request = dynamic_cast<NWorld::CCmdTakeCorpse*>(set->GetCmd());
        Require(request && request->pCorpse == body,"queued corpse pickup lost its actual target body");
    }
    Require(!actor->HasCommand() && actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p &&
        body->GetPosition().pos.p == bodyPosition.pos.p && actor->GetCorpse() == previousCorpse &&
        body->GetCorpseCarrier() == previousCarrier && actor->IsCarryingCorpse() == carrying && body->IsDead(),
        "native corpse producer changed position/AP/carry state or body attachment before queued consumption");
}

static void ForcedActionRecalc(const char* databasePath,const char* resourcePath)
{
    for (bool forced : {false,true}) {
        auto world = FixtureWorld(databasePath,resourcePath,true);
        vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr;
        for (auto unit : units) {
            auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
            if (candidate && !candidate->IsAIUnit() && candidate->CanFight()) { actor = candidate; break; }
        }
        Require(actor != nullptr && IsValid(NDb::GetAnimation(4437)),"forced recalc fixture has no native actor or authored animation");
        auto read = [&]() {
            CStructureNetworkContext refs; auto fields = BorrowedObjectFields(actor,&refs);
            CObj<NWorld::CCommandExecute> executor; bool flag = false;
            { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(7,&executor); reader.Add(12,&flag); }
            return std::make_pair(executor,flag);
        };
        Require(!read().first && !read().second && !actor->IsPerformingAction(),
            "forced recalc fixture started with a running action");
        const int ap = actor->GetAP(); const auto position = actor->GetPosition();
        if (forced) {
            CObj<NWorld::CExecPlayAnimation> animation = new NWorld::CExecPlayAnimation(actor,4437,false);
            actor->RunCriticalExecutor(animation);
        } else {
            actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdPlayAnimation(4437,false)));
            actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));
        }
        auto before = read();
        Require(IsValid(before.first) && dynamic_cast<NWorld::CExecPlayAnimation*>(before.first.GetPtr()) &&
            before.first->GetState() == NWorld::CCommandExecute::RUNNING && before.second == forced && actor->IsPerformingAction(),
            "native animation entry did not establish the expected running/forced action");
        actor->OnTBSEvent(NWorld::TBS_RECALC_COMMAND);
        auto after = read();
        std::printf("native action recalc: forced=%d retained=%d forced-flag=%d executing=%d\n",
            int(forced),int(after.first.GetPtr() == before.first.GetPtr()),int(after.second),int(before.first->IsExecuting()));
        if (forced) {
            Require(after.first.GetPtr() == before.first.GetPtr() && after.second && actor->IsPerformingAction(),
                "native command recalc drops a running forced action while its forced flag remains latched");
            world->GetTime()->Set(actor->animator.GetTimeEnd());
            actor->Segment(); // Reach the existing animation edge and native retirement path.
            auto finished = read();
            Require(!IsValid(finished.first) && !finished.second && !actor->IsPerformingAction(),
                "native forced animation did not retire and release its flag at the ordinary animation edge");
        } else {
            Require(!IsValid(after.first) && !after.second,"native recalc stopped retiring an ordinary cancelable action");
            before.first->Cancel(); // The retained witness owns the detached control until cleanup.
        }
        Require(actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p,
            "native recalc lifecycle changed logical position or action points");
    }
}

static void ScriptDrawWeaponSlot(const char* databasePath,const char* resourcePath,bool requireQueued = false,bool grenade = false)
{
    for (auto active : {NDb::SLOT_1,NDb::SLOT_2}) {
        auto world = FixtureWorld(databasePath,resourcePath);
        vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr; NDb::CRPGWeapon* record = nullptr;
        for (auto unit : units) {
            auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
            if (!candidate || !candidate->CanFight() || candidate->IsEmptyPK() || candidate->IsWearingPK() ||
                candidate->animator.GetCannon()) continue;
            auto* weapon = dynamic_cast<NRPG::CWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
            if (weapon) { actor = candidate; record = weapon->GetDBWeapon(); break; }
        }
        Require(actor && IsValid(record),"script draw fixture has no native armed actor");
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        vector<CObj<NRPG::IInventoryItem>> removed;
        while (!inventory->GetItems().empty()) {
            CObj<NRPG::IInventoryItem> item = inventory->GetItems().back().pItem;
            removed.push_back(item); inventory->Take(item);
        }
        for (auto slot : {NDb::SLOT_1,NDb::SLOT_2})
            if (inventory->Get(slot)) removed.push_back(inventory->TakeOff(slot));
        CObj<NRPG::CWeaponItem> first = new NRPG::CWeaponItem(record), second = new NRPG::CWeaponItem(record);
        CObj<NRPG::IInventoryItem> spare;
        if (grenade) {
            NDb::CRPGGrenade* grenadeRecord = nullptr;
            CDBIterator<NDb::CRPGGrenade> rows(*NDatabase::GetTable<NDb::CRPGGrenade>());
            while (rows.MoveNext()) if (IsValid(rows.Get()->pItem) && IsValid(rows.Get()->pItem->pModel) &&
                rows.Get()->pItem->nWeight > 0) { grenadeRecord = rows.Get(); break; }
            Require(grenadeRecord != nullptr,"script draw fixture has no authored grenade");
            CObj<NRPG::CGrenadeItem> item = new NRPG::CGrenadeItem(grenadeRecord);
            item->nQuantity = 1; spare = item;
        } else spare = new NRPG::CWeaponItem(record);
        Require(inventory->Equip(NDb::SLOT_1,first) && inventory->Equip(NDb::SLOT_2,second) && inventory->Activate(active),
            "script draw fixture could not equip and select its two native hand slots");
        auto* commander = actor->GetPlayer()->GetCommander();
        Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
            "script draw fixture started with a pending order");
        CObj<NScript::CScript> script = NScript::CreateScript(world);
        NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
        const string call = grenade ? "UnitSwitchToGrenade(upstreamActor)" : "UnitDrawWeapon(upstreamActor)";
        RunScript(script,(call+"; UpstreamDrawReturned = 1").c_str());
        Require(!commander->HasCommands() && !actor->HasCommand() && inventory->GetActiveSlot() == active,
            "native script draw without a backpack weapon changed a hand or issued a command");
        CTPoint<int> place;
        Require(inventory->FindPlace(spare,&place) && inventory->Place(place,spare),
            "script draw fixture could not store its real backpack weapon");
        const auto targetSlot = active == NDb::SLOT_1 ? NDb::SLOT_2 : NDb::SLOT_1;
        auto* targetItem = inventory->Get(targetSlot);
        const auto ap = actor->GetAP();
        RunScript(script,(call+"; UpstreamDrawReturned = 7").c_str());
        auto returned = script->GetGlobal("UpstreamDrawReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 7,"native draw script did not finish its callback");
        const bool queued = commander->HasCommands();
        std::printf("native script inventory dispatch: grenade=%d active=%d queued=%d unit-command=%d\n",
            int(grenade),int(active),int(queued),int(actor->HasCommand()));
        if (requireQueued) Require(queued && !actor->HasCommand(),
            "native script inventory draw executes immediately instead of entering its owning commander's queue");
        CObj<NWorld::CCmd> requested;
        if (queued) {
            CObj<NWorld::CCommand> firstOrder = commander->GetCommand(), resume = commander->GetCommand();
            auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(firstOrder.GetPtr());
            auto* next = dynamic_cast<NWorld::CCmdSetCommand*>(resume.GetPtr());
            Require(set && set->pUnit == actor && next && next->pUnit == actor &&
                dynamic_cast<NWorld::CCmdContinue*>(next->GetCmd()) && !commander->HasCommands(),
                "native draw did not issue its exact actor command and ordered Continue");
            requested = set->GetCmd();
        } else {
            CStructureNetworkContext refs; auto fields = BorrowedObjectFields(actor,&refs);
            { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(9,&requested); }
        }
        auto* exchange = dynamic_cast<NWorld::CCmdExchangeInventoryItems*>(requested.GetPtr());
        Require(exchange != nullptr,"native script draw did not retain its actual inventory exchange request");
        std::printf("native script draw slot: active=%d expected=%d actual=%d target=%d queued=%d\n",
            int(active),int(targetSlot),exchange->GetSlot(),exchange->GetTarget().nSlot,int(queued));
        Require(exchange->GetSlot() == targetSlot && exchange->GetTarget().nSlot == targetSlot,
            "native script draw chooses the active hand instead of the opposite active-slot index");
        Require(exchange->GetActivate() && exchange->GetSource().pUnit == actor && exchange->GetTarget().pUnit == actor &&
            exchange->GetSource().eType == NWorld::SItem::BACKPACK && exchange->GetSource().pItem == spare.GetPtr() &&
            exchange->GetTarget().eType == NWorld::SItem::SLOT && exchange->GetTarget().pItem == targetItem,
            "native script draw lost its real source item, target item, owner or activation intent");
        if (requireQueued) Require(actor->GetAP() == ap && inventory->GetActiveSlot() == active &&
            inventory->Get(NDb::SLOT_1) == first.GetPtr() && inventory->Get(NDb::SLOT_2) == second.GetPtr() &&
            inventory->GetItems().size() == 1 && inventory->GetItems().front().pItem == spare.GetPtr(),
            "native draw changed inventory or AP before queued command consumption");
        CStructureNetworkContext refs; auto fields = BorrowedObjectFields(actor,&refs);
        CObj<NWorld::CCommandExecute> executor;
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(7,&executor); }
        if (IsValid(executor)) executor->Cancel();
    }
}

static void DeferredCreateAndActivateItem(const char* databasePath,const char* resourcePath)
{
    for (int scenario=0; scenario<3; ++scenario) {
        const bool occupied = scenario != 0, full = scenario == 2;
        auto world = FixtureWorld(databasePath,resourcePath);
        vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
        NWorld::CUnitServer* actor = nullptr; NDb::CRPGWeapon* record = nullptr;
        for (auto unit : units) {
            auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
            if (!candidate || !candidate->CanFight() || candidate->IsEmptyPK() || candidate->IsWearingPK() ||
                candidate->animator.GetCannon()) continue;
            auto* weapon = dynamic_cast<NRPG::CWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
            if (weapon) { actor = candidate; record = weapon->GetDBWeapon(); break; }
        }
        Require(actor && IsValid(record) && IsValid(record->pItem),"script create fixture has no native armed actor/item record");
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        vector<CObj<NRPG::IInventoryItem>> removed;
        while (!inventory->GetItems().empty()) {
            CObj<NRPG::IInventoryItem> item = inventory->GetItems().back().pItem;
            removed.push_back(item); inventory->Take(item);
        }
        for (auto slot : {NDb::SLOT_1,NDb::SLOT_2})
            if (inventory->Get(slot)) removed.push_back(inventory->TakeOff(slot));
        CObj<NRPG::CWeaponItem> old = new NRPG::CWeaponItem(record), active = new NRPG::CWeaponItem(record);
        Require(inventory->Equip(NDb::SLOT_2,active) && inventory->Activate(NDb::SLOT_2) &&
            (!occupied || inventory->Equip(NDb::SLOT_1,old)),"script create fixture could not prepare its actual hand slots");
        if (full) for (int i=0; ; ++i) {
            CObj<NRPG::CWeaponItem> filler = new NRPG::CWeaponItem(record);
            CTPoint<int> place;
            if (!inventory->FindPlace(filler,&place)) break;
            Require(i<100 && inventory->Place(place,filler),"script create fixture could not fill its native backpack");
        }
        CTPoint<int> freePlace;
        Require(inventory->FindPlace(old,&freePlace) == !full,"script create fixture has the wrong real backpack capacity");
        const auto backpack = inventory->GetItems();
        const auto position = actor->GetPosition(); const int ap = actor->GetAP();
        auto* commander = actor->GetPlayer()->GetCommander();
        Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
            "script create fixture started with an existing order");
        CObj<NScript::CScript> script = NScript::CreateScript(world);
        NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
        const string id = std::to_string(record->pItem->GetRecordID());
        RunScript(script,("CreateAndActivateItem(nil,"+id+"); UpstreamCreateReturned = 5").c_str());
        auto returned = script->GetGlobal("UpstreamCreateReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 5 && !commander->HasCommands() && !actor->HasCommand(),
            "script create nil-actor control failed to finish or issued an order");
        RunScript(script,("CreateAndActivateItem(upstreamActor,"+id+"); UpstreamCreateReturned = 9").c_str());
        returned = script->GetGlobal("UpstreamCreateReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 9,"native create Lua callback did not finish");
        std::printf("native script create dispatch: occupied=%d full=%d queued=%d unit-command=%d\n",
            int(occupied),int(full),int(commander->HasCommands()),int(actor->HasCommand()));
        Require(commander->HasCommands() && !actor->HasCommand(),
            "native script create-and-activate bypasses the owning commander's queue");
        CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
        auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
        auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
        auto* create = set ? dynamic_cast<NWorld::CCmdCreateAndActivateInventoryItem*>(set->GetCmd()) : nullptr;
        Require(set && set->pUnit == actor && create && create->GetItem() == record->pItem &&
            create->GetSlot() == NDb::SLOT_1 && create->GetNeedMove() == occupied &&
            resume && resume->pUnit == actor && dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) &&
            !commander->HasCommands(),"queued script create lost its record, fixed hand slot, move flag, actor or Continue");
        if (occupied) {
            const auto& source = create->GetMoveSource(); const auto& target = create->GetMoveTarget();
            Require(source.pUnit == actor && source.eType == NWorld::SItem::SLOT && source.nSlot == NDb::SLOT_1 &&
                source.pItem == old.GetPtr() && target.pUnit == actor &&
                target.eType == (full ? NWorld::SItem::GROUND : NWorld::SItem::BACKPACK) &&
                target.sPosition.x == -1 && target.sPosition.y == -1,
                "queued create lost the old held item or changed backpack/ground stash selection");
        }
        Require(!actor->HasCommand() && actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p &&
            inventory->GetActiveSlot() == NDb::SLOT_2 && inventory->Get(NDb::SLOT_2) == active.GetPtr() &&
            inventory->Get(NDb::SLOT_1) == (occupied ? old.GetPtr() : nullptr) &&
            inventory->GetItems().size() == backpack.size(),"script create mutated actor/hands/backpack/AP before command consumption");
        for (std::size_t i=0; i<backpack.size(); ++i)
            Require(inventory->GetItems()[i].pItem == backpack[i].pItem,"script create replaced backpack items before command consumption");
    }
}

static void DeferredTakeObject(const char* databasePath,const char* resourcePath)
{
    auto world = FixtureWorld(databasePath,resourcePath);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr; NRPG::CWeaponItem* weapon = nullptr;
    for (auto entry : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!candidate || !candidate->CanFight() || candidate->IsEmptyPK() || candidate->IsWearingPK() ||
            candidate->animator.GetCannon()) continue;
        auto* firearm = dynamic_cast<NRPG::CWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
        if (firearm) { actor = candidate; weapon = firearm; break; }
    }
    Require(actor && weapon,"script pickup fixture has no native armed actor");
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    vector<CObj<NRPG::IInventoryItem>> removed;
    while (!inventory->GetItems().empty()) {
        CObj<NRPG::IInventoryItem> item = inventory->GetItems().back().pItem;
        removed.push_back(item); inventory->Take(item);
    }
    CObj<NRPG::CWeaponItem> item = new NRPG::CWeaponItem(weapon->GetDBWeapon());
    CTPoint<int> place;
    Require(inventory->FindPlace(item,&place),"script pickup fixture has no real backpack space");
    auto* body = world->AddFrozenItem(world->GetAIMap(),actor->GetPosition().GetCP()+CVec3(0.25f,0,0.25f),QNULL,item);
    Require(IsValid(body) && body->GetInvItem() == item.GetPtr() && world->GetFrozenItem(item) == body,
        "script pickup fixture did not register its actual world-body/item pair");
    const auto position = actor->GetPosition(); const int ap = actor->GetAP();
    const auto activeSlot = inventory->GetActiveSlot(); const int ammo = weapon->GetAmmoQuantity();
    auto* commander = actor->GetPlayer()->GetCommander();
    Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
        "script pickup fixture started with an existing order");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
    NScript::luaPushCPtr(*script,body); script->SetGlobal("upstreamGroundItem");
    NScript::luaPushCPtr(*script,item.GetPtr()); script->SetGlobal("upstreamInventoryItem");
    RunScript(script,"UnitTakeObject(nil,upstreamGroundItem); UnitTakeObject(upstreamActor,nil); "
        "UnitTakeObject(upstreamActor,upstreamInventoryItem); UpstreamPickupReturned = 13");
    auto returned = script->GetGlobal("UpstreamPickupReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 13 && !commander->HasCommands() && !actor->HasCommand(),
        "pickup nil controls or the fork's world-body argument contract issued an invalid order");
    RunScript(script,"UnitTakeObject(upstreamActor,upstreamGroundItem); UpstreamPickupReturned = 19");
    returned = script->GetGlobal("UpstreamPickupReturned");
    Require(returned.IsNumber() && returned.GetNumber() == 19,"native pickup Lua callback did not finish");
    std::printf("native script pickup dispatch: queued=%d unit-command=%d ground-item-retained=%d\n",
        int(commander->HasCommands()),int(actor->HasCommand()),int(world->GetFrozenItem(item) == body));
    Require(commander->HasCommands() && !actor->HasCommand(),
        "native script pickup bypasses the owning commander's queue");
    CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
    auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
    auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
    auto* move = set ? dynamic_cast<NWorld::CCmdMoveInventoryItem*>(set->GetCmd()) : nullptr;
    Require(set && set->pUnit == actor && move && resume && resume->pUnit == actor &&
        dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) && !commander->HasCommands(),
        "queued pickup lost its actor, move type or exact ordered Continue");
    const auto& source = move->GetSource(); const auto& target = move->GetTarget();
    Require(!IsValid(source.pUnit) && source.eType == NWorld::SItem::GROUND && source.nSlot == -1 &&
        source.pItem == item.GetPtr() && source.pWorldItem == body && target.pUnit == actor &&
        target.eType == NWorld::SItem::BACKPACK && target.sPosition.x == -1 && target.sPosition.y == -1,
        "queued pickup lost the fork's ground-body/item adapter or original ground-to-backpack payload");
    Require(!actor->HasCommand() && actor->GetAP() == ap && actor->GetPosition().pos.p == position.pos.p &&
        inventory->GetItems().empty() && inventory->GetActiveSlot() == activeSlot && inventory->GetActive() == weapon &&
        weapon->GetAmmoQuantity() == ammo && world->GetFrozenItem(item) == body && body->GetInvItem() == item.GetPtr(),
        "native script pickup changed actor/AP/inventory or removed the world body before queued command consumption");
}

static void ScriptHeldGrenadeActivation(const char* databasePath,const char* resourcePath)
{
    for (bool engineer : {true,false}) for (auto heldSlot : {NDb::SLOT_1,NDb::SLOT_2}) for (bool alreadyActive : {true,false}) {
        NativeGrenadeThrowFixture fixture(databasePath,resourcePath,engineer);
        auto* actor = fixture.actor;
        auto* inventory = actor->GetUnitRPG()->GetInventory();
        NDb::CRPGWeapon* weaponRecord = nullptr;
        vector<CPtr<NWorld::CUnit>> units; fixture.world->GetAllUnits(&units);
        for (auto unit : units) {
            auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
            auto* weapon = candidate ? candidate->GetUnitRPG()->GetWeaponItem() : nullptr;
            if (weapon && IsValid(weapon->GetDBWeapon())) { weaponRecord = weapon->GetDBWeapon(); break; }
        }
        Require(weaponRecord != nullptr,"held-grenade fixture has no authored firearm donor");
        vector<CObj<NRPG::IInventoryItem>> removed;
        for (auto slot : {NDb::SLOT_1,NDb::SLOT_2})
            if (inventory->Get(slot)) removed.push_back(inventory->TakeOff(slot));
        const auto otherSlot = heldSlot == NDb::SLOT_1 ? NDb::SLOT_2 : NDb::SLOT_1;
        CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem(weaponRecord);
        NDb::CRPGGrenade* spareRecord = fixture.regular;
        if (!spareRecord) {
            CDBIterator<NDb::CRPGGrenade> rows(*NDatabase::GetTable<NDb::CRPGGrenade>());
            while (rows.MoveNext()) if (IsValid(rows.Get()->pItem) && IsValid(rows.Get()->pItem->pModel) &&
                rows.Get()->pItem->nWeight > 0) { spareRecord = rows.Get(); break; }
        }
        Require(spareRecord != nullptr,"held-grenade fixture has no regular backpack grenade");
        CObj<NRPG::CGrenadeItem> spare = new NRPG::CGrenadeItem(spareRecord);
        spare->nQuantity = 1;
        const auto active = alreadyActive ? heldSlot : otherSlot;
        CTPoint<int> place;
        Require(inventory->Equip(heldSlot,fixture.grenade) && inventory->Equip(otherSlot,weapon) &&
            inventory->Activate(active) && inventory->FindPlace(spare,&place) && inventory->Place(place,spare),
            "held-grenade fixture could not equip its actual hands and spare backpack grenade");
        auto* commander = actor->GetPlayer()->GetCommander();
        Require(IsValid(commander) && !commander->HasCommands() && !actor->HasCommand(),
            "held-grenade fixture started with an existing order");
        const auto ap = actor->GetAP();
        CObj<NScript::CScript> script = NScript::CreateScript(fixture.world);
        NScript::luaPushCPtr(*script,actor); script->SetGlobal("upstreamActor");
        RunScript(script,"UnitSwitchToGrenade(upstreamActor); UpstreamHeldReturned = 11");
        auto returned = script->GetGlobal("UpstreamHeldReturned");
        Require(returned.IsNumber() && returned.GetNumber() == 11,"held-grenade Lua callback did not finish");
        std::printf("native held grenade: engineer=%d held=%d active=%d already-active=%d queued=%d unit-command=%d\n",
            int(engineer),int(heldSlot),int(active),int(alreadyActive),int(commander->HasCommands()),int(actor->HasCommand()));
        if (alreadyActive || engineer) Require(!commander->HasCommands(),
            "active grenade or existing engineer-grenade selection policy issued a new command");
        else {
            Require(commander->HasCommands(),"native grenade switch leaves the held grenade inactive and queues no activation");
            CObj<NWorld::CCommand> first = commander->GetCommand(), second = commander->GetCommand();
            auto* set = dynamic_cast<NWorld::CCmdSetCommand*>(first.GetPtr());
            auto* resume = dynamic_cast<NWorld::CCmdSetCommand*>(second.GetPtr());
            auto* select = set ? dynamic_cast<NWorld::CCmdSetActiveItem*>(set->GetCmd()) : nullptr;
            Require(set && set->pUnit == actor && select && select->nSlot == heldSlot &&
                resume && resume->pUnit == actor && dynamic_cast<NWorld::CCmdContinue*>(resume->GetCmd()) &&
                !commander->HasCommands(),"held-grenade activation lost its slot/actor/Continue or drew the spare instead");
        }
        Require(!actor->HasCommand() && actor->GetAP() == ap && inventory->GetActiveSlot() == active &&
            inventory->Get(heldSlot) == fixture.grenade.GetPtr() && inventory->Get(otherSlot) == weapon.GetPtr() &&
            inventory->GetItems().size() == 1 && inventory->GetItems().front().pItem == spare.GetPtr(),
            "held-grenade script changed inventory or AP before queued activation consumption");
    }
}

static void ScriptedEnemyDeath(const char* databasePath,const char* resourcePath, bool spareUnconscious = false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    auto* human = world->GetPlayerByID(0);
    Require(IsValid(human) && !NAI::IsAIPlayer(human), "scripted death fixture has no scenario-player-zero human");
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    vector<CPtr<NWorld::CUnitServer>> enemies;
    vector<std::pair<CPtr<NWorld::CUnitServer>,int>> protectedUnits;
    for (const auto& unit: units) {
        if (!IsValid(unit) || !unit->CanFight()) continue;
        if (world->GetDiplomacyState(human,unit->GetPlayer()) == NDb::DS_ENEMY)
            enemies.push_back(unit);
        else protectedUnits.push_back({unit,unit->GetAP()});
    }
    Require(!enemies.empty() && !protectedUnits.empty(), "scripted death fixture needs hostile and protected units");
    CPtr<NWorld::CUnitServer> unconscious;
    if (spareUnconscious) {
        Require(enemies.size() > 1, "scripted unconscious gate fixture needs two hostile actors");
        unconscious = enemies.front();
        enemies.erase(enemies.begin());
        unconscious->MakeUnconscious(VNULL3,true,false);
        Require(unconscious->IsUnconscious() && !unconscious->IsDead() && !unconscious->CanFight() &&
            !unconscious->GetUnitRPG()->IsDead(), "scripted unconscious gate fixture did not preserve a living incapacitated hostile");
    }
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    RunScript(script,"KillEmAll()");
    if (spareUnconscious)
        Require(unconscious->IsUnconscious() && !unconscious->IsDead() && !unconscious->GetUnitRPG()->IsDead(),
            "KillEmAll killed an incapacitated hostile outside its fight-capable target set");
    for (const auto& enemy: enemies) {
        Require(enemy->GetUnitRPG()->IsDead(),
            "KillEmAll left a world-dead hostile's RPG persona alive");
        Require(enemy->IsDead() && !enemy->CanFight(), "KillEmAll did not complete the hostile's world death");
    }
    for (const auto& item: protectedUnits)
        Require(item.first->CanFight() && !item.first->GetUnitRPG()->IsDead() && item.first->GetAP() == item.second,
            "KillEmAll harmed a friendly or neutral unit or spent its AP");
}

static bool SavedWalkWithoutWeapon(NWorld::CUnitServer* actor)
{
    CStructureNetworkContext graph;
    graph.include = [](CObjectBase*) { return false; };
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(actor)->operator&(writer); }
    saved.Seek(0);
    bool holstered = false;
    { CStructureSaver reader(saved, CStructureSaver::READ, &graph); reader.Add(26, &holstered); }
    return holstered;
}

static void ScriptHolsterPersistence(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (server && server->CanFight() && !server->IsEmptyPK() && !server->IsWearingPK()) { actor = server; break; }
    }
    Require(actor != nullptr, "scripted holster fixture has no living actor");
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script, actor); script->SetGlobal("upstreamActor");
    const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    Require(!SavedWalkWithoutWeapon(actor), "new actor unexpectedly requested persistent holstering");
    RunScript(script, "UnitActivateWeapon(upstreamActor, nil)");
    Require(SavedWalkWithoutWeapon(actor), "scripted holstering did not persist the walk-without-weapon flag");
    RunScript(script, "UnitActivateWeapon(upstreamActor, 1)");
    Require(!SavedWalkWithoutWeapon(actor), "scripted weapon activation did not clear persistent holstering");
    RunScript(script, "UnitActivateWeapon(upstreamActor, nil)");
    RunScript(script, "UnitActivateWeapon(upstreamActor)");
    Require(!SavedWalkWithoutWeapon(actor), "default scripted activation retained the holster flag");
    Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "recording scripted holster intent spent action points before queued execution");
}

enum class MovementHolsterCheck { Persistent, ShortInactive, HiddenActive };

static void MovementHolsterPolicy(const char* databasePath, const char* resourcePath,
    MovementHolsterCheck check = MovementHolsterCheck::Persistent)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (server && server->CanFight() && !server->IsEmptyPK() && !server->IsWearingPK()
                && server->GetUnitRPG()->CanMove() && server->GetPosition().pos.p.GetPose() != NAI::CM_INACTIVE
                && IsValid(server->GetUnitRPG()->GetInventoryInfo()->GetActive())) { actor = server; break; }
    }
    Require(actor != nullptr, "movement holster fixture has no mobile armed actor");
    CObj<NAI::CPath> path = new NAI::CPath;
    path->pNet = world->GetPathNetwork();
    auto place = actor->GetPosition().pos.p;
    const int initialDirection = place.GetDirection();
    for (int turn = 0; turn < 4; ++turn) {
        place.SetDirection((initialDirection + turn) & 7);
        path->points.push_back(place);
    }
    const int ap = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    int deactivations = 0;
    auto activationCount = [&](bool persistent, NWorld::ENeedActiveItem constraint, int pointCount = 4,
        bool animationActive = false, bool modelHidden = true) {
        actor->SetWalkWithoutWeapon(persistent);
        actor->animator.SetActiveItem(animationActive);
        actor->SetUndrawItem(modelHidden);
        CObj<NAI::CPath> planned = new NAI::CPath;
        planned->pNet = path->pNet.GetPtr();
        planned->points.assign(path->points.begin(), path->points.begin() + pointCount);
        CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(actor, planned, NAI::PF_DEFAULT, constraint);
        Require(IsValid(move), "movement holster fixture could not create its native path executor");
        int activations = 0, rotations = 0;
        deactivations = 0;
        CStructureNetworkContext graph;
        graph.include = [&](CObjectBase* object) {
            const int type = pSSClasses->GetObjectTypeID(object);
            if (type == int(0x113B1130)) ++activations;
            if (type == int(0x114B1130)) ++deactivations;
            if (type == int(0x02731143)) ++rotations;
            return false;
        };
        CMemoryStream traversal;
        { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
          static_cast<CObjectBase*>(move.GetPtr())->operator&(writer); }
        Require(rotations >= pointCount - 1, "movement holster fixture lost its native turn commands");
        return activations;
    };
    Require(activationCount(false, NWorld::ITEM_NO_MATTER) == 1,
        "ordinary movement did not retain its normal weapon activation");
    Require(activationCount(true, NWorld::ITEM_NO_MATTER) == 0,
        "native movement queued automatic weapon activation despite persistent scripted holstering");
    Require(activationCount(true, NWorld::ITEM_ACTIVE) == 1,
        "persistent holstering suppressed an explicit active-weapon path constraint");
    if (check == MovementHolsterCheck::ShortInactive) {
        for (int count: {2, 3}) {
            Require(activationCount(false, NWorld::ITEM_INACTIVE, count) == 0 && deactivations == 0,
                "short inactive path queued an unnecessary weapon draw and holster cycle");
        }
        Require(activationCount(false, NWorld::ITEM_INACTIVE, 4) == 1 && deactivations == 1,
            "short inactive-path exception changed activation policy for a longer path");
    }
    if (check == MovementHolsterCheck::HiddenActive) {
        Require(activationCount(false, NWorld::ITEM_NO_MATTER, 4, true, true) == 1,
            "hidden weapon was treated as drawn solely because its animator flag was active");
        Require(activationCount(false, NWorld::ITEM_NO_MATTER, 4, true, false) == 0,
            "a genuinely drawn weapon received a redundant activation command");
        Require(activationCount(true, NWorld::ITEM_NO_MATTER, 4, true, true) == 0,
            "drawn-state reconciliation bypassed persistent scripted holstering");
    }
    Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
        "constructing the movement activation policy spent action points");
}

static void ExplicitWeaponActivation(const char* databasePath, const char* resourcePath, bool movement)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& entry: units) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (server && server->CanFight() && !server->IsEmptyPK() && !server->IsWearingPK()
                && server->GetUnitRPG()->CanMove() && IsValid(server->GetUnitRPG()->GetInventoryInfo()->GetActive())) {
            actor = server; break;
        }
    }
    Require(actor != nullptr, "explicit activation fixture has no mobile armed actor");
    actor->SetWalkWithoutWeapon(true);
    actor->animator.SetActiveItem(false);
    actor->SetUndrawItem(true);
    if (movement) {
        CObj<NAI::CPath> path = new NAI::CPath;
        path->pNet = world->GetPathNetwork(); path->points.push_back(actor->GetPosition().pos.p);
        CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(actor, path, NAI::PF_DEFAULT, NWorld::ITEM_ACTIVE);
        Require(IsValid(move), "explicit activation fixture could not create its movement executor");
        move->Run();
        const bool reset = !actor->IsWalkingWithoutWeapon();
        move->TimeLabelReached(); move->AnimationFinished(); move = nullptr;
        Require(reset, "explicit movement weapon activation retained the scripted holster flag");
        Require(!world->IsAction(), "completed movement activation retained its action counter");
        return;
    }
    const auto slot = static_cast<NDb::ESlot>(actor->GetUnitRPG()->GetInventoryInfo()->GetActiveSlot());
    auto equipment = [&](int stage) {
        CObj<NWorld::CCommandExecute> executor = dynamic_cast<NWorld::CCommandExecute*>(
            pSSClasses->CreateObject(int(0x01122133))); // Native CExecSetActiveItem.
        Require(IsValid(executor), "native equipment activation class is not registered");
        struct OwnerWire {
            CPtr<NWorld::CUnitServer> actor;
            int operator&(CStructureSaver& saver) { saver.Add(4, &actor); return 0; }
        } owner;
        owner.actor = actor;
        auto savedSlot = slot;
        bool twoHeavy = false;
        CStructureNetworkContext references;
        references.externalReferences = true;
        references.ids[actor] = 1; references.refs[1] = actor;
        references.nextID = 100; references.include = [](CObjectBase*) { return false; };
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references);
          writer.Add(1, &owner); writer.Add(2, &savedSlot); writer.Add(3, &stage); writer.Add(4, &twoHeavy); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ, &references);
          static_cast<CObjectBase*>(executor.GetPtr())->operator&(reader); }
        Require(executor->GetUnitServer() == actor, "restored equipment callback lost its native actor");
        return executor;
    };
    auto transientFlags = [&]() {
        struct BaseFlags {
            bool undraw = false, noHeavy = false;
            int operator&(CStructureSaver& saver) { saver.Add(10,&undraw); saver.Add(11,&noHeavy); return 0; }
        } flags;
        CStructureNetworkContext references;
        auto fields = BorrowedObjectFields(actor,&references);
        { CStructureSaver reader(fields,CStructureSaver::READ,&references); reader.Add(2,&flags); }
        return std::make_pair(flags.undraw,flags.noHeavy);
    };
    const int ap = actor->GetAP();
    auto executor = equipment(1);
    executor->AnimationFinished();
    const bool reset = !actor->IsWalkingWithoutWeapon();
    actor->SetWalkWithoutWeapon(true); // A newer script request while the animation completes.
    actor->SetUndrawItem(true,true);
    Require(transientFlags() == std::make_pair(true,true), "equipment fixture did not seed both native temporary holster flags");
    executor->AnimationFinished();
    Require(reset, "explicit equipment activation retained the scripted holster flag");
    Require(actor->IsWalkingWithoutWeapon() && executor->GetState() == NWorld::CCommandExecute::FINISHED,
        "equipment completion overwrote a later scripted holster request");
    Require(!actor->GetUndrawItem() && transientFlags() == std::make_pair(false,false),
        "completed equipment activation retained temporary item or heavy-weapon hiding flags");
    CObj<NRPG::IInventoryItem> removed = actor->GetUnitRPG()->GetInventory()->TakeOff(slot);
    actor->SetUndrawItem(true,true);
    auto empty = equipment(1);
    empty->AnimationFinished();
    Require(actor->IsWalkingWithoutWeapon() && empty->GetState() == NWorld::CCommandExecute::FINISHED,
        "empty equipment slot cancelled scripted holstering without activating an item");
    Require(!actor->GetUndrawItem() && transientFlags() == std::make_pair(false,false),
        "completing activation of an empty slot retained temporary item hiding flags");
    Require(actor->GetAP() == ap && !actor->HasCommand(), "clearing equipment visibility flags spent AP or installed an action");
}

static void ScriptAIActivation(const char* databasePath, const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, false, &game);
    vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    NAI::IAIUnit* ai = nullptr;
    for (auto& entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight()) continue;
        auto* candidate = NAI::GetAIUnit(unit);
        if (candidate && candidate->IsUnderAIControl() && IsValid(candidate->GetReaction())) {
            actor = unit; ai = candidate; break;
        }
    }
    Require(actor && ai, "script activation fixture has no controlled AI actor");
    CObj<NAI::CAILogic> route = new NAI::CAILogic(ai);
    ai->SetRouteLogic(route);
    Require(ai->GetRouteLogic() == route.GetPtr() && route->IsActive(), "script activation fixture route is not active");
    CObj<NAI::CAIReaction> original = ai->GetReaction();
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    NScript::luaPushCPtr(*script, actor); script->SetGlobal("upstreamActor");
    const int beforeAP = actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
    RunScript(script, "UnitCheat(upstreamActor, 32, 1)");
    Require(actor->GetUnitRPG()->GetRPGUnit()->IsCheatEnabled(NRPG::CHEAT_NOAI), "script failed to set the no-AI flag");
    Require(ai->GetRouteLogic() == route.GetPtr() && !route->IsActive(),
        "scripted AI deactivation left the current native route running");
    CObj<NAI::CAIReaction> disabled = ai->GetReaction();
    Require(IsValid(disabled) && disabled != original.GetPtr(), "scripted AI deactivation did not replace the active reaction");
    RunScript(script, "UnitCheat(upstreamActor, 32, nil)");
    Require(!actor->GetUnitRPG()->GetRPGUnit()->IsCheatEnabled(NRPG::CHEAT_NOAI)
            && ai->GetRouteLogic() == route.GetPtr() && route->IsActive(),
        "scripted AI activation did not resume the preserved route");
    Require(IsValid(ai->GetReaction()) && ai->GetReaction() != disabled.GetPtr(),
        "scripted AI activation retained the disabled reaction");
    Require(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == beforeAP,
        "scripted AI activation changed the actor's action points");
}

static void ScriptPK(bool leave)
{
    CObj<NWorld::CWorld> world = new NWorld::CWorld;
    CObj<NScript::CScript> script = NScript::CreateScript(world);
    CObj<ScriptActor> actor = new ScriptActor;
    CObj<ScriptActor> shell = new ScriptActor;
    shell->shell = true;
    NScript::luaPushCPtr(*script, actor); script->SetGlobal("upstreamActor");
    NScript::luaPushCPtr(*script, shell); script->SetGlobal("upstreamShell");
    Script::AutoBlock stack(*script);
    if (leave) {
        RunScript(script, "upstreamResult = UnitLeavePK(upstreamActor)");
        Require(script->GetGlobal("upstreamResult").IsNil(), "UnitLeavePK returned a pilot instead of its missing shell");
    } else {
        RunScript(script, "UnitWearPK(upstreamActor, upstreamShell)");
        Require(actor->orders > 0, "UnitWearPK rejected an empty shell because it cannot fight");
        const int before = actor->orders;
        RunScript(script, "UnitWearPK(upstreamActor, upstreamActor)");
        Require(actor->orders == before, "UnitWearPK accepted a living soldier as a shell");
        CObj<NWorld::CPlayer> owner = new NWorld::CPlayer;
        CObj<NAI::CAICommander> commander = new NAI::CAICommander;
        owner->SetCommander(commander);
        actor->owner = owner; shell->owner = owner;
        Require(NAI::GetAICommander(actor) == commander, "live pilot lost its AI commander");
        Require(!NAI::GetAICommander(shell), "empty PK shell acquired an AI commander");
    }
}

static void PKEquipmentTransfer()
{
    CObj<NRPG::IInventory> shell = NRPG::CreateInventory(nullptr);
    CObj<NRPG::IInventory> pilot = NRPG::CreateInventory(nullptr);
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem;
    Require(shell->Equip(NDb::SLOT_1, weapon), "invalid PK weapon test fixture");
    CObj<NDb::CPanzerklein> fixedSuit = new NDb::CPanzerklein;
    fixedSuit->bSingleSlot = true;
    pilot->SetPanzerklein(fixedSuit, shell);
    Require(pilot->Get(NDb::SLOT_1) == weapon, "putting on a fixed-weapon PK lost its weapon");
    Require(!shell->Get(NDb::SLOT_1), "PK weapon exists in both inventories");
    Require(!pilot->CanEquip(NDb::SLOT_1, weapon), "manual fixed-weapon replacement became allowed");
    pilot->SetPanzerklein(nullptr, shell);
    Require(shell->Get(NDb::SLOT_1) == weapon && !pilot->Get(NDb::SLOT_1),
        "removing the PK failed to return its weapon to the shell");
}

template<class T>
static auto CanRemovePKWeapon(T* inventory, int) -> decltype(inventory->CanTakeOff(NDb::SLOT_1))
{ return inventory->CanTakeOff(NDb::SLOT_1); }
template<class T>
static bool CanRemovePKWeapon(T*, long) { return true; }

static void PKFixedWeaponPolicy()
{
    CObj<NRPG::IInventory> inventory = NRPG::CreateInventory(nullptr);
    Require(CanRemovePKWeapon(inventory.GetPtr(), 0), "normal equipment removal was restricted");
    CObj<NDb::CPanzerklein> suit = new NDb::CPanzerklein;
    suit->bSingleSlot = false;
    inventory->SetPanzerklein(suit, nullptr);
    Require(CanRemovePKWeapon(inventory.GetPtr(), 0), "removable PK equipment was restricted");
    suit->bSingleSlot = true;
    Require(!CanRemovePKWeapon(inventory.GetPtr(), 0), "manual removal of a fixed PK weapon is allowed");
}

static void DoorSwingDirection(const char* databasePath,const char* resourcePath)
{
    auto world = FixtureWorld(databasePath,resourcePath);
    auto* map = world->GetAIMap(); map->Sync();
    int doors = 0;
    for (const auto& entry : world->GetObjectsForHarness()) {
        auto* door = dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr());
        if (!door || door->IsBroken()) continue;
        const bool wasOpen = door->IsOpen();
        struct Restore { NAI::IAIMap* map; NWorld::CWindowDoor* door; bool open;
            ~Restore() { map->FlipDoorWindow(door,open); } } restore{map,door,wasOpen};
        // Existing native state filtering exposes each real collision hull in turn.
        // No grid cells or synthetic hull bounds are used as the expected direction.
        map->FlipDoorWindow(door,false); CObj<CObjectBase> closedHull = map->GetHull(door);
        CVec3 closed;
        if (!IsValid(closedHull) || !map->GetUnitHLPos(&closed,closedHull,-1)) continue;
        map->FlipDoorWindow(door,true); CObj<CObjectBase> openHull = map->GetHull(door);
        CVec3 opened;
        if (!IsValid(openHull) || openHull == closedHull || !map->GetUnitHLPos(&opened,openHull,-1)) continue;
        CVec3 expected = opened-closed;
        if (fabs2(expected) < 0.0001f) continue;
        Normalize(&expected);
        for (bool visibleOpen : {false,true}) {
            map->FlipDoorWindow(door,visibleOpen);
            const auto forward = door->GetChangeStateDirection(true), backward = door->GetChangeStateDirection(false);
            std::printf("native door swing: door=%d visible-open=%d length=%.6f hull-error=%.6f reverse-error=%.6f\n",
                doors,int(visibleOpen),float(fabs(forward)),float(fabs(forward-expected)),float(fabs(backward+expected)));
            Require(fabs2(forward-expected) < 0.00001f && fabs2(backward+expected) < 0.00001f,
                "native door swing uses arbitrary grid locks instead of its normalized collision-hull direction");
            Require(door->IsOpen() == wasOpen,"measuring a native door swing changed its logical open state");
        }
        ++doors;
    }
    Require(doors > 0,"door swing fixture has no authored door with distinct closed and open hulls");
    std::printf("native door swing: checked %d authored doors in both collision states\n",doors);
}

static void DoorApproachHeading(const char* databasePath,const char* resourcePath)
{
    auto world = FixtureWorld(databasePath,resourcePath,true);
    auto* network = world->GetPathNetwork(); world->GetAIMap()->Sync();
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto unit : units) {
        auto* candidate = dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if (candidate && !candidate->IsAIUnit() && candidate->CanFight() && candidate->GetUnitRPG()->CanMove()) { actor = candidate; break; }
    }
    Require(actor != nullptr,"door approach fixture has no movable native roster actor");
    const auto original = actor->GetPosition();
    const auto ap = actor->GetAP();
    struct Restore { NWorld::CUnitServer* actor; NAI::SUnitPosition position;
        ~Restore() { actor->SetTemporaryPosition(position); } } restore{actor,original};
    int cases = 0;
    for (const auto& entry : world->GetObjectsForHarness()) {
        auto* door = dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr());
        if (!door || door->IsBroken() || door->IsOpen()) continue;
        vector<NAI::SPathPlace> approaches; door->GetApproaches(&approaches,network);
        const auto swing = door->GetChangeStateDirection(true);
        for (int i=0; i<approaches.size(); ++i) {
            auto from = approaches[i];
            if (!network->IsPassable(from)) continue;
            // These real authored Unit bones face 4 or 0. Check concrete nearby
            // headings and the circular 6 -> 7 approach to 0, without reproducing
            // the production candidate-selection algorithm in the expectation.
            struct Case { int authored, facing, expected; };
            const Case casesForBone[] = {{4,4,4},{4,5,4},{4,6,5},{0,4,4},{0,6,7}};
            for (const auto& test : casesForBone) {
                if (approaches[i].GetDirection() != test.authored) continue;
                const int facing = test.facing;
                from.SetDirection(facing);
                NAI::SPosition at(from,network); const float angle = at.GetDirection();
                Require(swing.x*cos(angle)+swing.y*sin(angle) <= 0,"door approach fixture no longer needs a separate opening action");
                auto placed = original; placed.pos.p = from; actor->SetTemporaryPosition(placed);
                if (test.expected != facing) {
                    CObj<NAI::CPath> controlPath = new NAI::CPath;
                    controlPath->pNet = network; controlPath->points.push_back(from);
                    auto turn = from; turn.SetDirection(test.expected); controlPath->points.push_back(turn);
                    CObj<NWorld::CExecQueue> control = new NWorld::CExecQueue(actor);
                    control->AddPath(controlPath,NAI::PF_DEFAULT,NWorld::ITEM_NO_MATTER);
                    auto* head = control->GetFrontExecutor();
                    CObj<NAI::CPath> rotated = head ? head->GetCurrentPath() : nullptr;
                    Require(IsValid(rotated) && rotated->points.back().GetDirection() == test.expected,
                        "native rotation control cannot prepare the same actor's expected turn");
                }
                CObj<NAI::CPath> path = new NAI::CPath;
                path->pNet = network; path->points.push_back(from);
                NAI::CPath::SPathAction action; action.where = from; action.action = NAI::PA_OPEN; action.pObject = door;
                path->actions.push_back(action);
                CObj<NWorld::CExecQueue> queue = new NWorld::CExecQueue(actor);
                queue->AddPath(path,NAI::PF_DEFAULT,NWorld::ITEM_NO_MATTER);
                auto* front = queue->GetFrontExecutor(); CObj<NAI::CPath> prepared = front ? front->GetCurrentPath() : nullptr;
                Require(IsValid(front),"native door queue did not create its first movement executor");
                std::printf("native door approach: approach=%d authored-dir=%d from=%d expected=%d prepared=%d last=%d pose=%d\n",
                    i,approaches[i].GetDirection(),facing,test.expected,int(IsValid(prepared)),
                    IsValid(prepared) ? prepared->points.back().GetDirection() : -1,from.GetPose());
                if (test.expected != facing)
                    Require(IsValid(prepared) && prepared->points.back().GetDirection() == test.expected,
                        "native door queue omits the authored near-door approach turn");
                else
                    Require(!IsValid(prepared),"native door queue adds an unwanted turn for an aligned or opposite heading");
                ++cases;
                Require(actor->GetAP() == ap && door->IsOpen() == false,"preparing the native door queue executed a world action");
            }
        }
    }
    Require(cases >= 5,"door approach fixture did not cover its authored aligned, near-door and wrap headings");
}

static void DoorCollision()
{
    CObj<RenderFixture> first = new RenderFixture, second = new RenderFixture;
    NAI::SSourceInfo closedA(first, nullptr, 0, NWorld::TS_STATE_CLOSED);
    NAI::SSourceInfo openA(first, nullptr, 0, NWorld::TS_STATE_OPEN);
    NAI::SSourceInfo openB(second, nullptr, 0, NWorld::TS_STATE_OPEN);
    NAI::SDoorColliderAnalyzer analyzer;
    Require(!analyzer(NAI::SColliderUserInfo(&closedA, 0)), "one door hull blocked by itself");
    Require(!analyzer(NAI::SColliderUserInfo(&openB, 0)), "inactive hulls from different doors were combined");
    Require(analyzer(NAI::SColliderUserInfo(&openA, 0)), "both hulls of the same door did not block");
    analyzer.Clear();
    Require(!analyzer.IsCollided(), "door collision state survived Clear");
}

static void OverlappingDoors()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    auto* layer = network->GetLayer(0);
    for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x)
        layer->tiles[y][x].nPassable = NAI::CP_STAND;
    CObj<RenderFixture> first = new RenderFixture, second = new RenderFixture;
    NAI::SPathPlace key(3, 3, 0), position = key;
    position.SetPose(NAI::CM_STAND);
    NAI::CNodesLayer::STile blocked{}, open{};
    open.nPassable = NAI::CP_STAND;
    auto* door = network->GetFlipper(first);
    door->bOpen = false; door->nFixedFlags = NAI::CPathNetwork::F_LOCKED_DOOR;
    door->locksClosed[key] = blocked; door->locksOpen[key] = open;
    door = network->GetFlipper(second);
    door->bOpen = false; door->nFixedFlags = NAI::CPathNetwork::F_LOCKED_DOOR;
    door->locksClosed[key] = open; door->locksOpen[key] = blocked;
    layer->tiles[3][3].nFlipper = 2; // The legacy single-door index hides door A.
    Require(network->IsLocked(position, false), "overlapping door A lost its blocking restriction");
    Require(!network->IsLocked(position, true), "ignoring doors also blocked an unlocked tile");
    network->GetFlipper(first)->bOpen = true;
    Require(!network->IsLocked(position, false), "inactive overlapping door hulls blocked the passage");
    // Replication restores fields into a live grid. A derived door lookup
    // must not retain indices into the old serialized flipper arrays.
    CMemoryStream update;
    { CStructureSaver saver(update, CStructureSaver::WRITE);
      std::vector<NAI::CPathNetwork::SFlipper> empty;
      std::unordered_map<CPtr<CObjectBase>, int, SPtrHash> noDoors;
      saver.Add(9, &empty); saver.Add(10, &noDoors); }
    update.Seek(0);
    { CStructureSaver saver(update, CStructureSaver::READ);
      static_cast<CObjectBase*>(network.GetPtr())->operator&(saver); }
    Require(network->GetFlippersAt(position).empty(), "restored grid retained a stale door lookup");
}

static void WeaponFamiliarity()
{
    CObj<NRPG::CUnit> unit = new NRPG::CUnit;
    CObj<NRPG::CWeaponItem> weapon = new NRPG::CWeaponItem;
    CObj<NRPG::CWeaponItem> other = new NRPG::CWeaponItem;
    unit->UseWeapon(weapon, 1.f, 100, .1f);
    unit->UseWeapon(weapon, 2.f, 100, .1f);
    Require(fabs(unit->GetWeaponAdaptation(weapon) - .2f) < .0001f,
        "familiarity bonus uses training rate instead of the DB multiplier");
    unit->UseWeapon(weapon, .5f, 100, .1f);
    Require(fabs(unit->GetWeaponAdaptation(weapon) - .25f) < .0001f,
        "changing training rate changed familiarity scaling");
    unit->UseWeapon(weapon, 200.f, 100, .1f);
    Require(fabs(unit->GetWeaponAdaptation(weapon) - 10.f) < .0001f,
        "retail familiarity cap changed");
    Require(unit->GetWeaponAdaptation(other) == 0.f, "familiarity leaked to another weapon");
}

static void ClueSlotSelection()
{
    for (auto type : {NDb::CT_ITEM, NDb::CT_PERSON}) {
        CObj<NDb::CDBScenarioZone> dbZone = new NDb::CDBScenarioZone;
        CObj<NScenario::CScenarioZone> zone = new NScenario::CScenarioZone(dbZone, 1);
        dbZone->templatesIDs = {0, 101, 102};
        std::unordered_map<int, NScenario::CScenarioZone::STemplate> slots;
        slots[102].nEmptyItemSlots = 1;
        slots[102].nEmptyPersonSlots = 1;
        CMemoryStream state;
        { CStructureSaver saver(state, CStructureSaver::WRITE); saver.Add(13, &slots); }
        state.Seek(0);
        { CStructureSaver saver(state, CStructureSaver::READ);
          static_cast<CObjectBase*>(zone.GetPtr())->operator&(saver); }
        CObj<NDb::CDBScenarioClue> record = new NDb::CDBScenarioClue;
        record->clueType = type;
        CObj<NScenario::CScenarioClue> clue = new NScenario::CScenarioClue(record, 1);
        zone->PlaceClue(clue);
        Require(clue->IsPlaced() && clue->GetTemplateID() == 102,
            "zero-capacity map selected for a clue instead of the available slot");
        zone->RemoveClue(clue);
        zone->PlaceClue(clue);
        Require(clue->GetTemplateID() == 102, "returned clue slot was not available for reuse");
    }
}

class FullScenarioGraph: public NScenario::CScenarioFlowChartBase
{
public:
    FullScenarioGraph(): CScenarioFlowChartBase(1, true) { Generate(); }
};

static void ConclusionRouting(const char* databasePath, const char* resourcePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<FullScenarioGraph> graph = new FullScenarioGraph;
    std::vector<CPtr<NScenario::CScenarioClue>> clues;
    graph->GetClues(&clues);
    int checked = 0;
    for (auto& clue : clues) {
        if (clue->GetDBClue()->clueType != NDb::CT_CONCLUSION) continue;
        ++checked;
        Require(clue->IsPlaced(), "abstract conclusion was dropped from the campaign graph");
        Require(clue->GetTemplateID() == 0 && clue->GetParentZones().empty(),
            "abstract conclusion consumed a physical map slot");
    }
    Require(checked > 0, "campaign fixture has no conclusion clues");
}

template<class Network>
static auto CanPlacePK(Network* network, const NAI::SPathPlace& place, int)
    -> decltype(IsBigLockerPlacePassable(network, place))
{ return IsBigLockerPlacePassable(network, place); }
template<class Network>
static bool CanPlacePK(Network* network, const NAI::SPathPlace& place, long)
{ return NAI::IsBigLockerPassable(network, place); }

static void PKPlacement()
{
    CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
    network->CreateLayer(8, 8, VNULL2, 0.f, 0);
    network->Freeze(true);
    auto* layer = network->GetLayer(0);
    NAI::SPathPlace position(3, 3, 0);
    position.SetPose(NAI::CRAWL);
    position.SetDirection(0);
    Require(!CanPlacePK(network.GetPtr(), position, 0), "PK placement accepted an unlocked layer without a floor");
    layer->tiles[3][3].nPassable = NAI::CP_STAND;
    Require(!CanPlacePK(network.GetPtr(), position, 0), "PK placement ignored blocked footprint neighbors");
    for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x)
        layer->tiles[y][x].nPassable = NAI::CP_STAND;
    for (int direction = 0; direction < 8; ++direction) {
        position.SetDirection(direction);
        Require(CanPlacePK(network.GetPtr(), position, 0), "open standing footprint was rejected");
        NAI::SPathPlace edge(position);
        edge.SetXY(0, 0);
        Require(!CanPlacePK(network.GetPtr(), edge, 0), "PK footprint was clipped at the grid border");
    }
    position.SetLayer(200);
    position.SetFinal(1);
    position.SetPose(NAI::CM_STAND);
    Require(position.IsFinal() && CanPlacePK(network.GetPtr(), position, 0), "flying PK restoration was rejected");
}

static void CriticalIDs(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    CObj<NRPG::CUnit> unit = new NRPG::CUnit;
    CObj<NRPG::IUnitMission> mission = NRPG::CreateUnit(unit);
    for (bool enable : {false, true}) {
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE);
          std::vector<int> empty; writer.Add(20, &empty); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ);
          static_cast<CObjectBase*>(mission.GetPtr())->operator&(reader); }
        if (enable) mission->EnableCriticals(); else mission->DisableCriticals();
        CMemoryStream result;
        { CStructureSaver writer(result, CStructureSaver::WRITE);
          static_cast<CObjectBase*>(mission.GetPtr())->operator&(writer); }
        result.Seek(0);
        std::vector<int> states;
        { CStructureSaver reader(result, CStructureSaver::READ); reader.Add(20, &states); }
        Require(states.size() == NDb::N_CRIT_TYPES, "critical toggle did not repair a restored short state vector");
        for (int state : states)
            Require(state == (enable ? NRPG::CS_ENABLED : NRPG::CS_DISABLED), "critical toggle left a stale status");
    }
    Require(int(NDb::C_PANZERKLEIN_AXIS) == 16 && int(NDb::C_PANZERKLEIN_BROKEN) == 19,
        "PK critical identifiers do not match the retail database and save format");
}

class PathProbeExecutor: public NWorld::CCommandExecute
{
    OBJECT_BASIC_METHODS(PathProbeExecutor);
public:
    static int probes;
    bool running = false;
    int cost = 0;
    int operator&(CStructureSaver& saver) override {
        saver.Add(2, &running); saver.Add(3, &cost); return 0;
    }
    int GetStartAP() const override { return cost; }
    bool IsExecuting() override { return running; }
    NWorld::CPathConflictsRemover* GetPathConflictsRemover() override { ++probes; return nullptr; }
};
int PathProbeExecutor::probes = 0;
REGISTER_SAVELOAD_CLASS(0x7f550101, PathProbeExecutor);

static void IdlePathBlocker(const char* databasePath, const char* resourcePath)
{
    auto world = FixtureWorld(databasePath, resourcePath);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && unit->CanFight()) { actor = unit; break; }
    }
    Require(actor != nullptr, "path blocker fixture has no actor");
    world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
    Require(!world->IsRealTime() && actor->CanSpendAP(0), "path blocker fixture is not in its turn");
    for (int state = 0; state < 3; ++state) {
        CObj<PathProbeExecutor> executor = new PathProbeExecutor;
        executor->running = state != 0;
        executor->cost = state == 2 ? 100000 : 0;
        CMemoryStream saved;
        { CStructureSaver saver(saved, CStructureSaver::WRITE); saver.Add(7, &executor); }
        saved.Seek(0);
        { CStructureSaver saver(saved, CStructureSaver::READ);
          static_cast<CObjectBase*>(actor)->operator&(saver); }
        PathProbeExecutor::probes = 0;
        actor->GetPathConflictsRemover();
        Require(PathProbeExecutor::probes == (state == 1 ? 1 : 0),
            state == 0 ? "idle unit promises to free its movement tile" :
            state == 2 ? "unaffordable move promises to free its movement tile" : "running affordable mover was ignored");
    }
}

static void HiddenPortraitAck()
{
    CObj<AttackMission> mission = new AttackMission;
    CObj<NUI::CUnitPanel> panel = new NUI::CUnitPanel(NUI::SWindowInfo(), mission);
    for (int selected : {0, 2}) {
        mission->selected = selected;
        CObj<NUI::CAckEvent> event = new NUI::CAckEvent;
        panel->PlayAckEvent(STime(100), event);
        Require(event->IsReady(), "voice acknowledgement waits forever for a hidden portrait");
        const STime end = event->GetEndTime();
        panel->PlayAckEvent(STime(200), event);
        Require(event->GetEndTime() == end, "releasing a hidden portrait extended acknowledgement lifetime");
        event->Cancel();
        panel->PlayAckEvent(STime(300), event);
        Require(event->IsComplete(STime(300)), "hidden portrait revived a cancelled acknowledgement");
    }
}

static void ClueInitialization()
{
    CObj<NDb::CDBScenarioClue> record = new NDb::CDBScenarioClue;
    for (unsigned char pattern : {0xff, 0x11}) {
        void* memory = ::operator new(sizeof(NScenario::CScenarioClue));
        std::memset(memory, pattern, sizeof(NScenario::CScenarioClue));
        CObj<NScenario::CScenarioClue> clue = new(memory) NScenario::CScenarioClue(record, 1);
        Require(clue->GetTemplateID() == 0, "new clue inherits a stale map template ID");
    }
}

template<class Player>
static auto DropInStore(Player* player, const CTPoint<int>& point, NRPG::IInventoryItem* item, int)
    -> decltype(player->PlaceStoreItem(point, item), void())
{ player->PlaceStoreItem(point, item); }
template<class Player>
static void DropInStore(Player* player, const CTPoint<int>&, NRPG::IInventoryItem* item, long)
{ player->PlaceStoreItem(item); }

static void StoreDropPosition()
{
    CObj<NRPG::CGlobalPlayer> global = new NRPG::CGlobalPlayer;
    CObj<NWorld::CPlayer> player = new NWorld::CPlayer(L"Store fixture", nullptr, global, 0);
    CObj<NDb::CRPGItem> record = new NDb::CRPGItem;
    record->sSize = CTPoint<int>(1, 1);
    record->nWeight = 0;
    CObj<NRPG::IInventoryItem> first = NRPG::CreateDummyItem(record);
    DropInStore(player.GetPtr(), CTPoint<int>(5, 5), first, 0);
    auto* items = player->GetStoreItems();
    Require(items->size() == 1 && items->front().sPos == CTPoint<int>(5, 5),
        "store discarded a valid manual drop position");
    CObj<NRPG::IInventoryItem> second = NRPG::CreateDummyItem(record);
    DropInStore(player.GetPtr(), CTPoint<int>(5, 5), second, 0);
    Require(items->size() == 2 && items->back().sPos != CTPoint<int>(5, 5),
        "occupied store cell did not use a free fallback");
    CObj<NRPG::IInventoryItem> third = NRPG::CreateDummyItem(record);
    DropInStore(player.GetPtr(), CTPoint<int>(-1, -1), third, 0);
    Require(items->size() == 3 && items->back().sPos.x >= 0 && items->back().sPos.y >= 0,
        "automatic store placement lost the returned item");
}

class TransferActor: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(TransferActor);
public:
    int operator&(CStructureSaver&) override { return 0; }
};
REGISTER_SAVELOAD_CLASS(0x7f550102, TransferActor);

class TransferMission: public AttackMission
{
    OBJECT_BASIC_METHODS(TransferMission);
public:
    static CPtr<NWorld::CUnit> actor;
    static CObj<NWorld::CCmd> command;
    int operator&(CStructureSaver&) override { return 0; }
    void Command(NWorld::CUnit* unit, NWorld::CCmd* order, bool) override {
        actor = unit; command = order;
    }
};
CPtr<NWorld::CUnit> TransferMission::actor;
CObj<NWorld::CCmd> TransferMission::command;
REGISTER_SAVELOAD_CLASS(0x7f550103, TransferMission);

class TransferTracker: public NGame::CUnitTracker
{
    OBJECT_BASIC_METHODS(TransferTracker);
public:
    CObj<TransferActor> unit;
    int operator&(CStructureSaver& saver) override { saver.Add(2, &unit); return 0; }
    NWorld::CUnit* GetUnit() const override { return unit; }
};
REGISTER_SAVELOAD_CLASS(0x7f550104, TransferTracker);

static void InventoryTransferActor(bool backpack)
{
    CObj<TransferMission> mission = new TransferMission;
    CObj<TransferActor> source = new TransferActor, destination = new TransferActor;
    CObj<NUI::CSlot> slot = dynamic_cast<NUI::CSlot*>(pSSClasses->CreateObject(
        backpack ? int(0xB0521142) : int(0xB0521155)));
    Require(slot != nullptr, "inventory slot class is not registered");
    CObj<TransferTracker> tracker = new TransferTracker;
    tracker->unit = destination;
    CMemoryStream state;
    { CStructureSaver saver(state, CStructureSaver::WRITE);
      saver.Add(2, &mission);
      if (backpack) {
          saver.Add(3, &tracker);
      } else {
          NDb::ESlot hand = NDb::SLOT_1;
          saver.Add(3, &hand); saver.Add(4, &destination);
      } }
    state.Seek(0);
    { CStructureSaver saver(state, CStructureSaver::READ);
      static_cast<CObjectBase*>(slot.GetPtr())->operator&(saver); }
    if (backpack) {
        CMemoryStream base;
        CObj<NUI::CWindow> view = new NUI::CWindow(NUI::SWindowInfo());
        view->SetSize(NUI::SPoint(100, 100));
        { CStructureSaver saver(base, CStructureSaver::WRITE);
          int width = 10, height = 10;
          saver.Add(3, &width); saver.Add(4, &height); saver.Add(8, &view); }
        base.Seek(0);
        CStructureSaver saver(base, CStructureSaver::READ);
        slot->NUI::CSlot::operator&(saver);
    }
    slot->SetPosition(NUI::SPoint(0, 0));
    slot->SetSize(NUI::SPoint(100, 100));
    CObj<NDb::CRPGItem> record = new NDb::CRPGItem;
    record->sSize = CTPoint<int>(1, 1);
    CObj<NRPG::IInventoryItem> item = NRPG::CreateDummyItem(record);
    NWorld::SItem drag(source, NWorld::SItem::HAND, item);
    slot->Place(10, 10, drag);
    Require(TransferMission::actor == source, "inventory transfer executes on the recipient instead of the hand owner");
    auto* order = dynamic_cast<NWorld::CCmdMoveInventoryItem*>(TransferMission::command.GetPtr());
    Require(order && order->GetSource().pUnit == source && order->GetTarget().pUnit == slot->GetUnit(),
        "inventory transfer lost its source or destination");
    drag.pUnit = nullptr;
    slot->Place(10, 10, drag);
    Require(TransferMission::actor == slot->GetUnit(), "ownerless hand did not use the destination actor");
    TransferMission::actor = nullptr; TransferMission::command = nullptr;
}

class DragActor: public NWorld::CUnitServer
{
    OBJECT_NOCOPY_METHODS(DragActor);
public:
    CPtr<NWorld::IPlayer> owner;
    NWorld::IPlayer* GetPlayer() const override { return owner; }
};

class OwnerlessDragMission: public ReadinessMission
{
    OBJECT_NOCOPY_METHODS(OwnerlessDragMission);
public:
    vector<CPtr<NGame::IUnitTracker>> selected, available;
    CPtr<CObjectBase> target;
    CPtr<NWorld::CUnit> issuedActor;
    CObj<NWorld::CCmd> issuedCommand;
    void GetSelectedUnits(vector<CPtr<NGame::IUnitTracker>>* result) const override { *result = selected; }
    void GetUnits(vector<CPtr<NGame::IUnitTracker>>* result) const override { *result = available; }
    CObjectBase* GetStateTarget(bool* fromWorld = nullptr) const override { if (fromWorld) *fromWorld = false; return target; }
    void Command(NWorld::CUnit* actor, NWorld::CCmd* command, bool) override { issuedActor = actor; issuedCommand = command; }
};

class PortraitSelectionTracker: public NGame::CUnitTracker
{
    OBJECT_NOCOPY_METHODS(PortraitSelectionTracker);
public:
    bool selected = false;
    bool IsSelected() const override { return selected; }
};

class PortraitPanelView: public DialogueLayoutView
{
    OBJECT_NOCOPY_METHODS(PortraitPanelView);
public:
    vector<float> clears;
    void CreateDynamicClearRects(const CRectLayout&,const CTPoint<int>&,const CTRect<int>&,float depth) override
    { clears.push_back(depth); }
};

static void RestoredPortraitPanel(bool emptyRoster)
{
    CObj<OwnerlessDragMission> mission = new OwnerlessDragMission;
    CObj<PortraitSelectionTracker> first = new PortraitSelectionTracker, second = new PortraitSelectionTracker;
    first->selected = true;
    if (!emptyRoster) mission->available = {first.GetPtr(),second.GetPtr()};
    CObj<NUI::CWindow> panel = dynamic_cast<NUI::CWindow*>(pSSClasses->CreateObject(int(0xB0521157)));
    Require(IsValid(panel), "portrait panel fixture could not create the native saved panel class");
    vector<CObj<NUI::CUnitView>> faces;
    for (int i = 0; i < 3; ++i) {
        CObj<NUI::CUnitView> face = dynamic_cast<NUI::CUnitView*>(pSSClasses->CreateObject(int(0xB0521153)));
        Require(IsValid(face), "portrait panel fixture could not create a native saved unit face");
        face->SetStyle(NUI::STYLE_VISIBLE,true);
        float angle = 1.f;
        CMemoryStream fields;
        { CStructureSaver writer(fields,CStructureSaver::WRITE); writer.Add(4,&angle); }
        fields.Seek(0);
        { CStructureSaver reader(fields,CStructureSaver::READ); face->NUI::CUnitView::operator&(reader); }
        faces.push_back(face);
    }
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.include = [](CObjectBase*) { return false; };
    references.ids[mission.GetPtr()] = 1; references.refs[1] = mission.GetPtr();
    for (int i = 0; i < int(faces.size()); ++i) {
        references.ids[faces[i].GetPtr()] = i+2;
        references.refs[i+2] = faces[i].GetPtr();
    }
    CPtr<NGame::IMission> missionReference = mission.GetPtr();
    CMemoryStream saved;
    { CStructureSaver writer(saved,CStructureSaver::WRITE,&references);
      writer.Add(2,&missionReference); writer.Add(4,&faces); }
    saved.Seek(0);
    { CStructureSaver reader(saved,CStructureSaver::READ,&references);
      static_cast<CObjectBase*>(panel.GetPtr())->operator&(reader); }
    panel->SetSize(NUI::SPoint(240,120));
    panel->SetPosition(NUI::SPoint(0,0));
    panel->SetStyle(NUI::STYLE_VISIBLE,true);
    CObj<PortraitPanelView> view = new PortraitPanelView;
    auto angle = [](NUI::CUnitView* face) {
        CMemoryStream fields;
        { CStructureSaver writer(fields,CStructureSaver::WRITE); face->NUI::CUnitView::operator&(writer); }
        fields.Seek(0);
        float result = 0;
        { CStructureSaver reader(fields,CStructureSaver::READ); reader.Add(4,&result); }
        return result;
    };
    // Face references retain their native types. They are deliberately not children:
    // exercise the panel's real state update without invoking unrelated 3D rendering.
    panel->Draw(0,view);
    Require(view->clears == vector<float>({1.f,0.f}),
        "native portrait panel did not bracket its drawing with depth clears");
    if (emptyRoster) {
        for (const auto& face: faces)
            Require(!face->GetStyle(NUI::STYLE_VISIBLE), "restored empty roster retained a visible portrait");
        return;
    }
    Require(faces[0]->GetStyle(NUI::STYLE_VISIBLE) && faces[1]->GetStyle(NUI::STYLE_VISIBLE) &&
        !faces[2]->GetStyle(NUI::STYLE_VISIBLE), "restored portrait panel displayed an unused face or hid a living roster slot");
    Require(fabs(angle(faces[0])-(-0.3f)) < 1e-6f && angle(faces[1]) == 1.f,
        "portrait selection did not turn the selected face toward the viewer");
    first->selected = false; second->selected = true;
    view->clears.clear();
    panel->Draw(1,view);
    Require(angle(faces[0]) == 1.f && fabs(angle(faces[1])-(-0.3f)) < 1e-6f,
        "changing selection did not restore the old portrait angle and turn the newly selected face");
    Require(view->clears == vector<float>({1.f,0.f}), "selection refresh left portrait depth state uncleared");
    mission->available.clear();
    view->clears.clear();
    panel->Draw(2,view);
    for (const auto& face: faces)
        Require(!face->GetStyle(NUI::STYLE_VISIBLE), "clearing the restored roster left a visible portrait");
}

class DragState: public NGame::CStateDragItem
{
public:
    void Attach(NGame::IMission* mission) { CStateBase::Initialize(mission); }
    using CStateDragItem::GetTargetCmd;
};

static void OwnerlessInventoryDrag()
{
    CObj<OwnerlessDragMission> mission = new OwnerlessDragMission;
    CObj<NWorld::CPlayer> player = new NWorld::CPlayer;
    CObj<NGame::IPlayerTracker> playerTracker = dynamic_cast<NGame::IPlayerTracker*>(pSSClasses->CreateObject(int(0xB2022800)));
    Require(IsValid(playerTracker), "ownerless drag fixture could not create the native player tracker");
    CStructureNetworkContext references;
    references.externalReferences = true; references.nextID = 100;
    references.include = [](CObjectBase*) { return false; };
    references.ids[player.GetPtr()] = 1; references.refs[1] = player.GetPtr();
    CMemoryStream savedPlayer;
    { CStructureSaver writer(savedPlayer, CStructureSaver::WRITE, &references); CPtr<NWorld::IPlayer> ref = player.GetPtr(); writer.Add(5, &ref); }
    savedPlayer.Seek(0);
    { CStructureSaver reader(savedPlayer, CStructureSaver::READ, &references); static_cast<CObjectBase*>(playerTracker.GetPtr())->operator&(reader); }
    mission->AttachActive(playerTracker);
    CObj<DragActor> selected = new DragActor, available = new DragActor, recipient = new DragActor;
    selected->owner = available->owner = recipient->owner = player.GetPtr();
    CObj<NGame::CUnitTracker> selectedTracker = new NGame::CUnitTracker(mission, selected);
    CObj<NGame::CUnitTracker> availableTracker = new NGame::CUnitTracker(mission, available);
    mission->selected.push_back(selectedTracker.GetPtr()); mission->available.push_back(availableTracker.GetPtr());
    CObj<NDb::CRPGItem> record = new NDb::CRPGItem;
    record->sSize = CTPoint<int>(1, 1);
    CObj<NRPG::IInventoryItem> item = NRPG::CreateDummyItem(record);
    player->SetInHandItem(NWorld::SItem(nullptr, NWorld::SItem::HAND, item));
    CObj<DragState> state = new DragState;
    state->Attach(mission);
    CObj<NWorld::CCmd> command = state->GetTargetCmd(false);
    auto* drop = dynamic_cast<NWorld::CCmdMoveInventoryItem*>(command.GetPtr());
    Require(drop && drop->GetSource().pUnit == selected.GetPtr(), "ownerless inventory drag did not use the first selected unit");
    Require(drop->GetSource().eType == NWorld::SItem::HAND && drop->GetTarget().eType == NWorld::SItem::GROUND
            && drop->GetTarget().pUnit == selected.GetPtr(), "ownerless ground drop lost its selected actor or placement");
    Require(state->OnLButtonDown(0, 0) && mission->issuedActor == selected.GetPtr(),
        "ownerless inventory click dispatched the move without its selected source actor");
    mission->target = recipient.GetPtr();
    command = state->GetTargetCmd(false); drop = dynamic_cast<NWorld::CCmdMoveInventoryItem*>(command.GetPtr());
    Require(drop && drop->GetSource().pUnit == selected.GetPtr() && drop->GetTarget().pUnit == recipient.GetPtr()
            && drop->GetTarget().eType == NWorld::SItem::UNIT_ANYPLACE, "ownerless drag confused the selected actor with its recipient");
    CObj<NUI::CSlotInfo> storage = new NUI::CSlotInfo(NUI::CSlotInfo::STORAGE);
    mission->target = storage.GetPtr();
    command = state->GetTargetCmd(true); drop = dynamic_cast<NWorld::CCmdMoveInventoryItem*>(command.GetPtr());
    Require(drop && drop->GetTarget().eType == NWorld::SItem::STORAGE && drop->GetTarget().pPlayer == player.GetPtr(),
        "ownerless storage drop did not resolve the selected unit's player");
    command = state->GetTargetCmd(false);
    Require(!IsValid(command) && !state->OnLButtonDown(0, 0), "inventory drag bypassed the inventory slot's own click handler");
    mission->target = nullptr; mission->selected.clear();
    command = state->GetTargetCmd(false); drop = dynamic_cast<NWorld::CCmdMoveInventoryItem*>(command.GetPtr());
    Require(drop && drop->GetSource().pUnit == available.GetPtr(), "ownerless inventory drag did not fall back to the mission roster");
    Require(state->OnLButtonDown(0, 0) && mission->issuedActor == available.GetPtr(), "roster fallback was not used for click dispatch");
    mission->available.clear();
    command = state->GetTargetCmd(false);
    Require(!IsValid(command) && !state->OnLButtonDown(0, 0), "ownerless drag invented an actor for an empty party");
    player->SetInHandItem(NWorld::SItem());
    mission->selected.push_back(selectedTracker.GetPtr());
    command = state->GetTargetCmd(false);
    Require(!IsValid(command), "inventory drag produced a command without an in-hand item");
}

static void GeometryHashBuckets()
{
    int buckets[4096] = {};
    GeometryTestHash hash;
    for (int i = 0; i < 4096; ++i)
        ++buckets[static_cast<unsigned>(hash(CVec3(i * .25f, 0.f, 0.f))) & 4095];
    int occupied = 0, largest = 0;
    for (int count : buckets) { occupied += count != 0; largest = Max(largest, count); }
    Require(occupied > 2000 && largest < 32,
        "grid-aligned geometry collapses into masked hash buckets");
    Require(hash(CVec3(0.f, 0.f, 0.f)) == hash(CVec3(-0.f, 0.f, 0.f)),
        "equal signed-zero positions have different merge hashes");
    std::vector<CVec3> positions = {VNULL3, CVec3(-0.f, 0.f, 0.f), CVec3(1,2,3), CVec3(1,2,3), CVec3(2,1,3)};
    std::vector<WORD> matches;
    NGScene::MergePositions(&matches, &positions);
    Require(positions.size() == 3 && matches == std::vector<WORD>({0,0,1,1,2}),
        "geometry hash changed vertex merge identity or ordering");
}

static void ImmediateClueNotification(const char* databasePath, const char* resourcePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NScenario::CScenarioTracker> tracker = NScenario::CreateScenarioTracker(1);
    CPtr<NScenario::CScenarioClue> selected, other;
    auto* table = NDatabase::GetTable<NDb::CDBScenarioClue>();
    CDBIterator<NDb::CDBScenarioClue> it(*table);
    while (it.MoveNext()) {
        auto* record = it.Get();
        auto* clue = tracker->GetClueByName(record->sSmallDescription);
        if (!clue) continue;
        if (!selected && record->bGiveImmediately && clue->GetObjectiveByType(NDb::OT_CAPTURE) && !tracker->IsClueFound(clue))
            selected = clue;
        else if (!other) other = clue;
    }
    Require(selected && other && selected != other, "campaign fixture has no immediate capturable clue");
    other->SetJustFound(true);
    tracker->CheatTakeClue(selected, true);
    Require(tracker->IsClueFound(selected), "immediate clue capture did not advance the scenario");
    Require(!selected->IsJustFound(), "immediate clue is announced again on the chapter map");
    Require(other->IsJustFound(), "clearing an immediate clue discarded another pending notification");
}

static void ImmediateCluePickup(const char* databasePath, const char* resourcePath, bool occupiedHand)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer* actor = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (unit && !unit->IsAIUnit() && unit->CanFight()) { actor = unit; break; }
    }
    Require(actor != nullptr, "clue fixture has no human actor");
    auto* tracker = world->GetGlobalGame()->pScenarioTracker.GetPtr();
    auto* clue = tracker->GetClueByName("Find_Cipher_key");
    Require(clue && clue->GetDBClue()->bGiveImmediately && !tracker->IsClueFound(clue), "immediate clue fixture is unavailable");
    CObj<NRPG::IInventoryItem> item = NRPG::CreateClueItem(NDb::GetRPGItem(clue->GetDBClue()->nItemID));
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    CTPoint<int> position;
    Require(inventory->FindPlace(item, &position) && inventory->Place(position, item), "clue source does not fit the fixture backpack");
    actor->nClueCount = 1;
    CObj<NRPG::IInventoryItem> held;
    if (occupiedHand) {
        held = NRPG::CreateDummyItem(item->GetDBItem());
        actor->GetTBSPlayer()->SetInHandItem(NWorld::SItem(actor, NWorld::SItem::HAND, held));
        CObj<NWorld::CCmdMoveInventoryItem> ordinaryCommand = new NWorld::CCmdMoveInventoryItem(
            NWorld::SItem(actor, NWorld::SItem::BACKPACK, held), NWorld::SItem(actor, NWorld::SItem::HAND));
        CObj<NWorld::CExecMoveInventoryItem> ordinary = new NWorld::CExecMoveInventoryItem(actor, ordinaryCommand);
        Require(ordinary->CanDoIt(actor->GetPosition()) != NWorld::UCR_OK,
            "ordinary item with a clue DB identity bypassed occupied-hand checks");
    }
    CObj<NWorld::CCmdMoveInventoryItem> command = new NWorld::CCmdMoveInventoryItem(
        NWorld::SItem(actor, NWorld::SItem::BACKPACK, item), NWorld::SItem(actor, NWorld::SItem::HAND));
    CObj<NWorld::CExecMoveInventoryItem> executor = new NWorld::CExecMoveInventoryItem(actor, command);
    Require(executor->CanDoIt(actor->GetPosition()) == NWorld::UCR_OK, "an immediate clue requires an empty drag hand");
    executor->Run();
    Require(tracker->IsClueFound(clue), "immediate clue transfer did not advance the campaign on pickup");
    NWorld::SItem after;
    bool hasHand = actor->GetTBSPlayer()->GetInHandItem(&after);
    Require(occupiedHand ? hasHand && after.pItem == held : !hasHand,
        "immediate clue entered the drag hand or replaced its previous item");
    Require(actor->nClueCount == 0, "consumed immediate clue retained a carrier count");
    for (const auto& entry : inventory->GetItems())
        Require(entry.pItem != item, "consumed clue remained in the backpack");
    int modals = 0;
    while (auto* raw = world->GetUICommand()) {
        CObj<NWorld::CUICmd> event = raw;
        auto* show = dynamic_cast<NWorld::CUICmdShowClue*>(event.GetPtr());
        if (show && show->pClue == clue) ++modals;
    }
    Require(modals == 1, "immediate clue pickup did not open exactly one clue modal");
    if (auto* goal = clue->GetGoal()) for (auto& task : goal->GetTasks()) {
        auto tag = task->GetDBTask()->eTag;
        if (tag == NDb::TT_FIND_ITEM || tag == NDb::TT_TAKE_ITEM || tag == NDb::TT_CARRY_OUT_ITEM)
            Require(task->GetState() == NScenario::TS_COMPLETED, "immediate clue task did not complete");
    }
}

static CPtr<NAnimation::CASequence> ReadAnimationSequence(NAnimation::CSkeletonAnimator* animator)
{
    CPtr<NAnimation::CASequence> sequence;
    CStructureNetworkContext graph;
    graph.include = [&](CObjectBase* object) {
        if (auto* found = dynamic_cast<NAnimation::CASequence*>(object)) sequence = found;
        return false;
    };
    CMemoryStream traversal;
    { CStructureSaver writer(traversal, CStructureSaver::WRITE, &graph);
      static_cast<CObjectBase*>(animator)->operator&(writer); }
    Require(IsValid(sequence), "fixture has no current animation sequence");
    return sequence;
}

static void PKShellWorld(const char* databasePath, const char* resourcePath, bool extractPilot, bool checkModel = false, int checkFeedback = 0)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    std::vector<CPtr<NWorld::CUnit>> units;
    world->GetAllUnits(&units);
    NWorld::CUnitServer *pilot = nullptr, *rescuer = nullptr;
    for (auto& candidate : units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(candidate.GetPtr());
        if (!unit || !unit->CanFight()) continue;
        if (!unit->IsAIUnit()) pilot = unit; else if (!rescuer) rescuer = unit;
    }
    Require(pilot && rescuer, "dead-pilot fixture has no pilot or rescuer");
    NDb::CRPGPers* suitRecord = nullptr;
    auto* table = NDatabase::GetTable<NDb::CRPGPers>();
    CDBIterator<NDb::CRPGPers> it(*table);
    while (it.MoveNext()) {
        auto* record = it.Get();
        if (record->pPanzerklein && !record->pPanzerklein->bHasNoHead && record->pModel) { suitRecord = record; break; }
    }
    Require(suitRecord != nullptr, "fixture database has no regular PK shell");
    auto* network = world->GetPathNetwork();
    std::vector<NAI::SPathPlace> places;
    network->GetNearPlaces(SSphere(pilot->GetPosition().GetCP(), 20.f), &places);
    NAI::SPathPlace anchor;
    bool found = false;
    for (auto place : places) {
        place.SetPose(NAI::CM_STAND); place.SetDirection(0);
        if (NAI::IsBigLockerPlacePassable(network, place)) { anchor = place; found = true; break; }
    }
    Require(found, "fixture has no unobstructed PK footprint");
    CObj<NWorld::CUnitServer> shell = world->AddUnitInGame(anchor, NRPG::CreateUnit(suitRecord), nullptr);
    Require(shell && shell->IsEmptyPK(), "could not create a PK shell");
    if (checkFeedback == 4 || checkFeedback == 5) {
        Require(shell->WearAsPK(true), "hidden PK fixture could not prepare its shell");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell, false, false);
        Require(pilot->IsWearingPK(), "hidden PK fixture did not equip its pilot");
        const int ap = pilot->GetRPG()->GetAP();
        for (bool hidden: {false, true}) {
            pilot->GetUnitRPG()->SetHiding(hidden);
            HiddenUnitPresentation shown;
            pilot->Visit(&shown);
            Require(shown.meshes == 1 && shown.alienMesh == hidden && shown.alienBegin == int(hidden)
                    && shown.alienEnd == int(hidden) && shown.alienDepth == 0,
                "hidden Panzerklein mesh did not use its bracketed alien material");
            Require(shown.filters.empty() && shown.flares == 0,
                "Panzerklein incorrectly received the human hiding tint or flare");
            Require(pilot->GetUnitRPG()->IsHiding() == hidden && pilot->GetRPG()->GetAP() == ap,
                "PK hiding presentation changed hiding state or action points");
        }
        if (checkFeedback == 5) {
            world->StartSequence();
            Require(world->IsSequence() && pilot->GetUnitRPG()->IsHiding(), "PK sequence fixture lost its hidden state");
            HiddenUnitPresentation cinematic;
            pilot->Visit(&cinematic);
            Require(cinematic.meshes == 1 && !cinematic.alienMesh && cinematic.alienBegin == 0
                    && cinematic.alienEnd == 0 && cinematic.filters.empty() && cinematic.flares == 0,
                "cinematic sequence still rendered the Panzerklein hiding material");
            Require(pilot->GetUnitRPG()->IsHiding(), "cinematic rendering cleared the PK's logical hiding state");
            world->EndOfTurn();
            Require(!world->IsSequence(), "PK sequence fixture did not leave the sequence");
            HiddenUnitPresentation resumed;
            pilot->Visit(&resumed);
            Require(resumed.alienMesh && resumed.alienBegin == 1 && resumed.alienEnd == 1,
                "PK hiding presentation did not resume after the sequence");
        }
        // The existing global material override remains independent of hiding.
        pilot->GetUnitRPG()->SetHiding(false);
        NGlobal::SetVar("i_am_an_alien", 1);
        HiddenUnitPresentation forced;
        pilot->Visit(&forced);
        NGlobal::SetVar("i_am_an_alien", 0);
        Require(forced.alienMesh && forced.alienBegin == 1 && forced.alienEnd == 1 && forced.alienDepth == 0,
            "PK hiding material changed the existing global alien-style override");
        return;
    }
    if (checkFeedback == 3) {
        Require(shell->WearAsPK(true), "turn-animation fixture could not prepare its shell");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell, false, false);
        Require(pilot->IsWearingPK(), "turn-animation fixture did not put on the suit");
        const int ap = pilot->GetRPG()->GetAP();
        auto sequence = ReadAnimationSequence(pilot->animator.GetSkeletonAnimator());
        const auto time = pilot->animator.GetTimeEnd();
        CPtr<NAnimation::CAnimator> before = sequence->GetAnimator(time);
        Require(IsValid(before), "fixture PK has no active animation track");
        pilot->animator.StartNewTurn(pilot->GetPosition());
        Require(sequence->GetAnimator(time) == before.GetPtr(),
            "new turn replaced the Panzerklein animation with a normal standing track");
        Require(pilot->GetRPG()->GetAP() == ap, "PK turn animation changed action points");
        pilot->FlipPanzerklein(nullptr, false, false);
        pilot->animator.PlaceUnit(pilot->GetPosition());
        sequence = ReadAnimationSequence(pilot->animator.GetSkeletonAnimator());
        const auto ordinaryTime = pilot->animator.GetTimeEnd();
        before = sequence->GetAnimator(ordinaryTime);
        pilot->animator.StartNewTurn(pilot->GetPosition());
        Require(sequence->GetAnimator(ordinaryTime) != before.GetPtr(),
            "ordinary human lost the original turn-start standing refresh");
        return;
    }
    if (checkFeedback) {
        CObj<NWorld::CUnitStateNormal> state = new NWorld::CUnitStateNormal(pilot);
        if (checkFeedback == 1) {
            CObj<NWorld::CCmdHeal> command = new NWorld::CCmdHeal(nullptr);
            for (int value = NDb::C_PANZERKLEIN_AXIS; value <= NDb::C_PANZERKLEIN_TERRORS_HWG; ++value) {
                const auto status = static_cast<NDb::ECritical>(value);
                pilot->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_ANY, status));
                Require(pilot->GetUnitRPG()->HasCritical(status), "PK feedback fixture did not apply a status");
                auto result = NWorld::UCR_NO_TARGET;
                Require(state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_PK_BAN,
                    "PK command ban reported an ordinary critical-injury rejection");
                Require(pilot->GetUnitRPG()->RemoveCritical(status), "PK fixture status could not be removed");
            }
            pilot->GetUnitRPG()->ApplyCritical(NRPG::SCritical(NDb::CL_HEAD, NDb::C_BLIND, 3));
            auto result = NWorld::UCR_NO_TARGET;
            Require(state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_CRITICALS_BAN,
                "ordinary injury incorrectly reported a PK restriction");
            pilot->GetUnitRPG()->RemoveCritical(NDb::C_BLIND);
            result = NWorld::UCR_NO_TARGET;
            Require(!state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_NO_TARGET,
                "successful command probe did not preserve its prior result");
            return;
        }
        Require(shell->WearAsPK(true), "PK feedback fixture could not prepare its shell");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell, false, false);
        Require(pilot->IsWearingPK(), "PK feedback fixture did not put on the suit");
        const int ap = pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
        for (auto pose : {NAI::CRAWL, NAI::RUN}) {
            CObj<NWorld::CCmdWishPose> command = new NWorld::CCmdWishPose(pose);
            auto result = NWorld::UCR_NO_TARGET;
            Require(state->IsCriticalsFailCommand(command, &result) && result == NWorld::UCR_PK_BAN,
                "worn-PK pose ban reported an ordinary critical-injury rejection");
        }
        CObj<NWorld::CCmdWishPose> allowed = new NWorld::CCmdWishPose(NAI::WALK);
        auto result = NWorld::UCR_NO_TARGET;
        Require(!state->IsCriticalsFailCommand(allowed, &result) && result == NWorld::UCR_NO_TARGET,
            "allowed PK pose did not preserve its prior eligibility result");
        Require(pilot->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP) == ap,
            "PK eligibility feedback changed action points");
        return;
    }

    if (checkModel) {
        auto verifyPrivateModel = [](NWorld::CUnitServer* unit, NDb::CModel* source) {
            auto* live = unit->GetModel();
            Require(live && live != source, "PK mode change reused the shared RPG model");
            Require(live->pGeometry.GetPtr() == source->pGeometry.GetPtr() &&
                live->pSkeleton.GetPtr() == source->pSkeleton.GetPtr(), "private PK model lost its geometry or skeleton");
            auto geometry = source->pGeometry;
            Require(IsValid(geometry), "PK model fixture has no geometry");
            live->pGeometry = nullptr;
            Require(source->pGeometry.GetPtr() == geometry.GetPtr(), "live model mutation changed the RPG source model");
            live->pGeometry = geometry;
        };
        auto* humanModel = pilot->GetUnitRPG()->GetRPGUnit()->pModel.GetPtr();
        auto* suitModel = shell->GetUnitRPG()->GetRPGUnit()->pModel.GetPtr();
        Require(shell->WearAsPK(true), "could not prepare the model ownership fixture");
        pilot->SetPosition(shell->GetPosition());
        pilot->FlipPanzerklein(shell, false, false);
        verifyPrivateModel(pilot, suitModel);
        pilot->FlipPanzerklein(nullptr, false, false);
        verifyPrivateModel(pilot, humanModel);
        return;
    }
    if (!extractPilot) {
        NAI::SPosition target = shell->GetPosition().pos;
        bool targetFound = false;
        for (auto place : places) {
            place.SetPose(NAI::CM_STAND); place.SetDirection(0);
            if (!NAI::IsBigLockerPlacePassable(network, place)) continue;
            target.p = place;
            if (fabs2(target.GetCP() - shell->GetPosition().GetCP()) > 9.f) { targetFound = true; break; }
        }
        Require(targetFound, "fixture has no distant PK waypoint");
        CObj<NDb::CWaypointName> name = new NDb::CWaypointName;
        name->szName = "upstream_pk";
        CObj<CMapWaypoint> authored = new CMapWaypoint;
        authored->pName = name;
        authored->pos.ptPos = target.GetCP();
        world->AddWaypoint(authored);
        auto* waypoint = world->GetWaypoint(name->szName);
        Require(waypoint != nullptr, "PK waypoint was not registered");
        waypoint->pos = target;
        CObj<NScript::CScript> script = NScript::CreateScript(world);
        NScript::luaPushCPtr(*script, shell); script->SetGlobal("upstreamShell");
        RunScript(script, "UnitSetToWaypoint(upstreamShell, 'upstream_pk')");
        Require(shell->GetPosition().GetPose() == NAI::CRAWL, "scripted PK relocation lost the parked pose");
        Require(fabs2(shell->GetPosition().GetCP() - target.GetCP()) < .0025f, "scripted empty PK did not reach its waypoint");
        return;
    }
    Require(shell->WearAsPK(true), "could not prepare a worn PK shell");
    pilot->SetPosition(shell->GetPosition());
    pilot->FlipPanzerklein(shell, false, false);
    pilot->GetUnitRPG()->GetRPGUnit()->Kill();
    pilot->SetState(new NWorld::CUnitStateDeath(pilot));
    pilot->SetPosition(pilot->GetPosition());
    Require(pilot->IsDead() && pilot->IsWearingPK() && network->IsLocked(anchor, true),
        "dead pilot did not retain the worn footprint");
    CObj<NWorld::CExecCorpse> spill = new NWorld::CExecCorpse(rescuer, pilot, true);
    spill->Run();
    Require(!pilot->IsWearingPK() && shell->IsEmptyPK(), "corpse extraction left the suit occupied");
    Require(shell->GetPosition().pos.p.GetX() == anchor.GetX() && shell->GetPosition().pos.p.GetY() == anchor.GetY(),
        "restoring the shell treated its dead pilot as an obstacle and relocated it");
}

static bool RunRegressionMode(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "heard-icon-anchor-height")) { Require(argc == 4, "heard icon requires db and res"); EnemyIconVisibility(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "missing-ui-sound-fallback")) { Require(argc == 4, "missing UI sound requires db and res"); MissingUISoundFallback(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "selected-attack-target-rejection")) { Require(argc == 4, "selected target requires db and res"); SelectedAttackTargetRejection(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "melee-current-pose-caption")) { Require(argc == 4, "melee caption requires db and res"); MeleeReachCaption(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "cancelled-inventory-hidden-item-pose")) { Require(argc == 4, "inventory cancellation requires db and res"); CancelledInventoryHiddenItemPose(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "initial-hidden-item-pose")) { Require(argc == 4, "initial item pose requires db and res"); CancelledInventoryHiddenItemPose(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "cancelled-equipment-hidden-item-pose")) { Require(argc == 4, "equipment cancellation requires db and res"); CancelledInventoryHiddenItemPose(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "spent-medkit-pose-cleanup")) { Require(argc == 4, "spent medkit requires db and res"); SpentMedkitPoseCleanup(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "spent-picklock-pose-cleanup")) { Require(argc == 4, "spent picklock requires db and res"); SpentPicklockPoseCleanup(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mine-placement-successor-handoff")) { Require(argc == 4, "mine handoff requires db and res"); MinePlacementSuccessorHandoff(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "trap-placement-successor-handoff")) { Require(argc == 4, "trap handoff requires db and res"); TrapPlacementSuccessorHandoff(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "engineer-trap-placement-successor-handoff")) { Require(argc == 4, "engineer trap handoff requires db and res"); TrapPlacementSuccessorHandoff(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "thrown-knife-successor-handoff")) { Require(argc == 4, "knife handoff requires db and res"); ThrownKnifeSuccessorHandoff(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "thrown-knife-feedback")) { Require(argc == 4, "knife feedback requires db and res"); ThrownKnifeSuccessorHandoff(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "restored-grenade-throw")) { Require(argc == 4, "saved grenade requires db and res"); RestoredGrenadeThrow(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "restored-engineer-grenade-throw")) { Require(argc == 4, "saved engineer grenade requires db and res"); RestoredGrenadeThrow(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "grenade-successor-handoff")) { Require(argc == 4, "grenade handoff requires db and res"); GrenadeSuccessorHandoff(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "engineer-grenade-successor-handoff")) { Require(argc == 4, "engineer grenade handoff requires db and res"); GrenadeSuccessorHandoff(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "default-item-detachment")) { Require(argc == 4, "item detachment requires db and res"); DefaultItemDetachment(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "unit-hit-roll-equality-log")) { Require(argc == 4, "unit hit log requires db and res"); HitRollEqualityLog(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "tile-hit-roll-equality-log")) { Require(argc == 4, "tile hit log requires db and res"); HitRollEqualityLog(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "dead-head-idle") && argc == 4) { DownedHeadIdle(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "unconscious-head-idle") && argc == 4) { DownedHeadIdle(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "active-door-hull-lookup") && argc == 4) { ActiveDoorHullLookup(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "projective-ray-unprojection")) { ProjectiveRayUnprojection(); return true; }
    if (!std::strcmp(mode, "shooting-torso-turn-limit")) { Require(argc == 4, "shooting torso limit requires db and res"); ShootingTorsoTurnLimit(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "grenade-throw-animation-refresh")) { Require(argc == 4, "grenade animation requires db and res"); GrenadeThrowAnimationRefresh(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "invalidated-grenade-trajectory")) { Require(argc == 4, "grenade trajectory requires db and res"); InvalidatedGrenadeTrajectory(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "grenade-completion-visibility")) { Require(argc == 4, "grenade visibility requires db and res"); GrenadeCompletionVisibility(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "unavailable-attack-shortcut")) { UnavailableAttackShortcut(); return true; }
    if (!std::strcmp(mode, "shared-action-shortcut-priority")) { SharedActionShortcutPriority(); return true; }
    if (!std::strcmp(mode, "equipped-tool-action-availability")) { Require(argc == 4, "tool availability requires db and res"); EquippedToolActionAvailability(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "assigned-level-experience")) { Require(argc == 3, "assigned level experience requires db"); AssignedLevelExperience(argv[2]); return true; }
    if (!std::strcmp(mode, "persona-full-display-name")) { Require(argc == 3, "full display name requires db"); PersonaFullDisplayName(argv[2]); return true; }
    if (!std::strcmp(mode, "character-panel-full-name")) { Require(argc == 4, "character panel name requires db and res"); CharacterPanelFullName(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "temporary-groups")) { TemporaryGroups(); return true; }
    if (!std::strcmp(mode, "scroll-initialization")) { ScrollInitialization(); return true; }
    if (!std::strcmp(mode, "attack-location")) { AttackLocationRefresh(); return true; }
    if (!std::strcmp(mode, "bind-initialization")) { BindInitialization(); return true; }
    if (!std::strcmp(mode, "flying-restoration")) { FlyingRestoration(); return true; }
    if (!std::strcmp(mode, "sniping-aim")) { SnipingAim(); return true; }
    if (!std::strcmp(mode, "decal-rebuild")) { DecalRebuild(); return true; }
    if (!std::strcmp(mode, "decal-projection")) { DecalProjection(); return true; }
    if (!std::strcmp(mode, "partial-reference-import") && argc == 3) { PartialReferenceImport(argv[2]); return true; }
    if (!std::strcmp(mode, "permanent-clue-single-map")) { PermanentClueSingleMap(); return true; }
    if (!std::strcmp(mode, "deferred-click-world")) { DeferredClickWorld(); return true; }
    if (!std::strcmp(mode, "point-light-without-scene")) { PointLightWithoutScene(); return true; }
    if (!std::strcmp(mode, "dynamic-ambient-fallback")) { DynamicAmbientFallback(); return true; }
    if (!std::strcmp(mode, "inventory-without-uniform")) { InventoryWithoutUniform(); return true; }
    if (!std::strcmp(mode, "corpse-backpack-drop") && argc == 4) { CorpseBackpackDrop(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "scripted-dialogue-priority") && argc == 4) { ScriptedDialoguePriority(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "point-light-save-layout")) { PointLightSaveLayout(); return true; }
    if (!std::strcmp(mode, "legacy-point-light-save-layout")) { PointLightSaveLayout(true); return true; }
    if (!std::strcmp(mode, "point-light-shadow-flag")) { PointLightShadowFlag(); return true; }
    if (!std::strcmp(mode, "global-point-light-shadow-flag")) { GlobalPointLightShadowFlag(); return true; }
    if (!std::strcmp(mode, "point-light-radiance")) { PointLightRadiance(); return true; }
    if (!std::strcmp(mode, "tiny-point-light-cache")) { TinyPointLightCache(); return true; }
    if (!std::strcmp(mode, "inactive-animated-light")) { AnimatedPointLightGuard(true); return true; }
    if (!std::strcmp(mode, "tiny-animated-light")) { AnimatedPointLightGuard(false); return true; }
    if (!std::strcmp(mode, "fresh-ambient-graph")) { FreshAmbientGraph(); return true; }
    if (!std::strcmp(mode, "compressed-save-writing")) { CompressedSaveWriting(); return true; }
    if (!std::strcmp(mode, "save-success-feedback") && argc == 3) { SaveSuccessFeedback(argv[2]); return true; }
    if (!std::strcmp(mode, "encoded-save-command") && argc == 3) { SaveSuccessFeedback(argv[2],true); return true; }
    if (!std::strcmp(mode, "load-success-feedback") && argc == 4) { LoadSuccessFeedback(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "encoded-load-command") && argc == 4) { LoadSuccessFeedback(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "zone-snapshot-excludes-hud") && argc == 4) { ZoneSnapshotExcludesHUD(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "window-interface-propagation")) { WindowInterfacePropagation(); return true; }
    if (!std::strcmp(mode, "window-child-registration")) { WindowChildRegistration(); return true; }
    if (!std::strcmp(mode, "lua-argument-state")) { LuaArgumentState(); return true; }
    if (!std::strcmp(mode, "worldless-ui-script")) { WorldlessUIScript(); return true; }
    if (!std::strcmp(mode, "lua-window-userdata")) { LuaWindowUserdata(); return true; }
    if (!std::strcmp(mode, "lua-window-setter-stack")) { LuaWindowSetterStack(); return true; }
    if (!std::strcmp(mode, "lua-window-state-context")) { LuaWindowStateContext(); return true; }
    if (!std::strcmp(mode, "window-message-callbacks")) { WindowMessageCallbacks(); return true; }
    if (!std::strcmp(mode, "window-callback-lifetime")) { WindowCallbackLifetime(); return true; }
    if (!std::strcmp(mode, "window-lazy-update")) { WindowLazyUpdate(); return true; }
    if (!std::strcmp(mode, "combo-popup-clipping")) { ComboPopupClipping(); return true; }
    if (!std::strcmp(mode, "combo-popup-item-width")) { ComboPopupClipping(true); return true; }
    if (!std::strcmp(mode, "chapter-clue-heading") && argc == 4) { ChapterTeamMarkerToolTip(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "medal-cursor-visible") && argc == 4) { MedalCursorVisible(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-medal-screenshot") && argc == 4) { ChapterMedalScreenshot(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "medal-panel-caption") && argc == 4) { MedalPanelCaption(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "medal-preview-camera") && argc == 4) { MedalPanelCaption(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "window-nested-wakeup")) { WindowNestedWakeup(); return true; }
    if (!std::strcmp(mode, "window-template-script")) { Require(argc == 3, "window template script requires db"); WindowTemplateScript(argv[2]); return true; }
    if (!std::strcmp(mode, "console-visibility-toggle")) { ConsoleVisibilityToggle(); return true; }
    if (!std::strcmp(mode, "ai-reaction-reset-contacts")) { AIReactionResetContacts(); return true; }
    if (!std::strcmp(mode, "ai-reaction-value-latch") && argc == 4) { AIReactionValueLatch(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "script-ai-activation") && argc == 4) { ScriptAIActivation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-event-collection-locks") && argc == 4) { AIEventCollectionLocks(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "mission-overlay-readiness")) { MissionOverlayReadiness(); return true; }
    if (!std::strcmp(mode, "ai-player-mission-readiness")) { AIPlayerMissionReadiness(); return true; }
    if (!std::strcmp(mode, "sound-icon-visibility")) { OverlayIconVisibility(false); return true; }
    if (!std::strcmp(mode, "trap-icon-visibility")) { OverlayIconVisibility(true); return true; }
    if (!std::strcmp(mode, "enemy-icon-visibility")) { Require(argc == 4, "enemy icons require db and res"); EnemyIconVisibility(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "finished-music-silence")) { Require(argc == 3, "finished music requires db"); FinishedMusicSilence(argv[2]); return true; }
    if (!std::strcmp(mode, "holstered-heavy-weapon-mesh")) { Require(argc == 4, "heavy mesh requires db and res"); HolsteredHeavyWeaponMesh(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "hidden-active-item-mesh")) { Require(argc == 4, "hidden active mesh requires db and res"); HolsteredHeavyWeaponMesh(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "hidden-carried-heavy-mesh")) { Require(argc == 4, "hidden carried mesh requires db and res"); HolsteredHeavyWeaponMesh(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "script-holster-persistence")) { Require(argc == 4, "script holster requires db and res"); ScriptHolsterPersistence(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "movement-holster-policy")) { Require(argc == 4, "movement holster requires db and res"); MovementHolsterPolicy(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "short-inactive-movement")) { Require(argc == 4, "inactive movement requires db and res"); MovementHolsterPolicy(argv[2], argv[3], MovementHolsterCheck::ShortInactive); return true; }
    if (!std::strcmp(mode, "hidden-active-movement")) { Require(argc == 4, "hidden active movement requires db and res"); MovementHolsterPolicy(argv[2], argv[3], MovementHolsterCheck::HiddenActive); return true; }
    if (!std::strcmp(mode, "equipment-holster-reset")) { Require(argc == 4, "equipment activation requires db and res"); ExplicitWeaponActivation(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "movement-holster-reset")) { Require(argc == 4, "movement activation requires db and res"); ExplicitWeaponActivation(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "movement-first-step-wait")) { Require(argc == 4, "movement wait requires db and res"); MovementFirstStepWaitTimer(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "movement-terminal-finished")) { Require(argc == 4, "terminal movement requires db and res"); MovementTerminalCompletion(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "movement-terminal-failed")) { Require(argc == 4, "terminal movement requires db and res"); MovementTerminalCompletion(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "movement-action-camera")) { Require(argc == 4, "movement camera requires db and res"); MovementActionCamera(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "mounted-reload-weapon")) { Require(argc == 4, "mounted reload requires db and res"); MountedReloadWeapon(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "holstered-reload-preparation")) { Require(argc == 4, "reload preparation requires db and res"); HolsteredReloadPreparation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "empty-clip-attack-entry") && argc == 4) { EmptyClipAttackEntry(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "invalid-action-cache-query") && argc == 4) { EmptyClipAttackEntry(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "object-trap-placement-cursor") && argc == 3) { PlacementCursor(argv[2]); return true; }
    if (!std::strcmp(mode, "mine-placement-cursor") && argc == 3) { PlacementCursor(argv[2],true); return true; }
    if (!std::strcmp(mode, "reload-action-wire")) { Require(argc == 4, "reload wire requires db and res"); ReloadActionWire(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-reload-log-wire")) { AIReloadLogRecord(0); return true; }
    if (!std::strcmp(mode, "ai-reload-log-rollback")) { AIReloadLogRecord(1); return true; }
    if (!std::strcmp(mode, "ai-reload-log-ownership")) { AIReloadLogRecord(2); return true; }
    if (!std::strcmp(mode, "pain-voice-owner")) { Require(argc == 4, "pain voice requires db and res"); PainVoiceOwner(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "unconscious-pain-voice")) { Require(argc == 4, "pain eligibility requires db and res"); PainVoiceOwner(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "world-ui-refresh-latch")) { Require(argc == 4, "world UI refresh requires db and res"); WorldUIRefreshLatch(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "dialogue-camera-filter")) { CinematicCameraCommands(0); return true; }
    if (!std::strcmp(mode, "movie-camera-filter")) { CinematicCameraCommands(1); return true; }
    if (!std::strcmp(mode, "fade-camera-filter")) { CinematicCameraCommands(2); return true; }
    if (!std::strcmp(mode, "completed-zone-recommendation")) { CompletedZoneRecommendation(); return true; }
    if (!std::strcmp(mode, "perk-tooltip-parameters")) { PerkTooltipParameters(); return true; }
    if (!std::strcmp(mode, "mechanical-step-sound")) { MechanicalStepSound(); return true; }
    if (!std::strcmp(mode, "nearest-invalid")) { InvalidNearestPosition(); return true; }
    if (!std::strcmp(mode, "lua-empty-default")) { EmptyLuaDefault(); return true; }
    if (!std::strcmp(mode, "missing-lua-global-warning")) { MissingLuaGlobalWarning(); return true; }
    if (!std::strcmp(mode, "precalc-sweep-bounds")) { PrecalcSweepBounds(); return true; }
    if (!std::strcmp(mode, "smart-aimer-boundary")) { SmartAimerBoundary(); return true; }
    if (!std::strcmp(mode, "nearest-empty")) { EmptyNearestPosition(); return true; }
    if (!std::strcmp(mode, "fresh-navigation-colouring")) { FreshNavigationColouring(); return true; }
    if (!std::strcmp(mode, "fresh-navigation-transitions")) { FreshNavigationTransitions(); return true; }
    if (!std::strcmp(mode, "lazy-navigation-colourer")) { LazyNavigationColourer(); return true; }
    if (!std::strcmp(mode, "restored-portrait-selection")) { RestoredPortraitPanel(false); return true; }
    if (!std::strcmp(mode, "restored-empty-portraits")) { RestoredPortraitPanel(true); return true; }
    if (!std::strcmp(mode, "edit-escape-cancel")) { EditKeyboardRouting(true); return true; }
    if (!std::strcmp(mode, "edit-keyboard-edges")) { EditKeyboardRouting(false); return true; }
    if (!std::strcmp(mode, "native-cursor-position")) { NativeCursorPositioning(); return true; }
    if (!std::strcmp(mode, "disabled-cursor-input")) { DisabledCursorInput(); return true; }
    if (!std::strcmp(mode, "exit-menu-save-visibility") && argc == 4) { ExitMenuSavePermission(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "exit-menu-save-shortcut") && argc == 4) { ExitMenuSavePermission(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "main-menu-load-save-tab") && argc == 4) { MainMenuLoadSaveTab(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-legacy-fixture-write") && argc == 5) { WriteLegacyChapterFixture(argv[2],argv[3],argv[4]); return true; }
    if (!std::strcmp(mode, "script-sound-start-2d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],false,false,500); return true; }
    if (!std::strcmp(mode, "script-sound-start-3d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],true,false,500); return true; }
    if (!std::strcmp(mode, "script-sound-unassigned-2d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],false,true); return true; }
    if (!std::strcmp(mode, "script-sound-unassigned-3d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],true,true); return true; }
    if (!std::strcmp(mode, "script-sound-stop-2d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "script-sound-stop-3d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "backdrop-script-audio-2d") && argc == 4) { BackdropScriptAudio(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "main-menu-map-script") && argc == 4) { MainMenuMapScript(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "main-menu-authored-script") && argc == 4) { MainMenuMapScript(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "main-menu-script-cadence") && argc == 4) { MainMenuMapScript(argv[2],argv[3],false,true); return true; }
    if (!std::strcmp(mode, "mission-loading-input") && argc == 4) { MissionLoadingInput(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mission-save-menu") && argc == 4) { MissionSaveMenu(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mission-quick-save") && argc == 4) { MissionSaveMenu(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "audio-option-checkboxes") && argc == 4) { AudioOptionCheckboxes(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "audio-option-checkbox-notify") && argc == 4) { AudioOptionCheckboxes(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "audio-option-defaults") && argc == 4) { AudioOptionCheckboxes(argv[2],argv[3],false,true); return true; }
    if (!std::strcmp(mode, "lua-unused-stack-init")) { LuaUnusedStackInitialization(); return true; }
    if (!std::strcmp(mode, "in-game-save-button") && argc == 4) { InGameSaveButton(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "in-game-save-shortcut") && argc == 4) { InGameSaveButton(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "backdrop-script-audio-3d") && argc == 4) { BackdropScriptAudio(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "chapter-active-script") && argc == 4) { ChapterActiveScript(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-active-command") && argc == 5) { ChapterActiveScript(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "chapter-clock-step") && argc == 5) { ChapterClockStep(argv[2],argv[3],argv[4]); return true; }
    if (!std::strcmp(mode, "legacy-chapter-world-migration") && argc == 5) { LegacyChapterWorldMigration(argv[2],argv[3],argv[4]); return true; }
    if (!std::strcmp(mode, "chapter-focus-audio") && argc == 4) { ChapterFocusAudio(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-script-initialization") && argc == 4) { ChapterScriptInitialization(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-world-graph") && argc == 4) { ChapterWorldGraph(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "global-sector-description") && argc == 4) { GlobalSectorDescription(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "global-map-return-destination") && argc == 4) { GlobalMapReturnDestination(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-team-marker-tooltip") && argc == 4) { ChapterTeamMarkerToolTip(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-marker-tooltip-transitions") && argc == 4) { ChapterTeamMarkerToolTip(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "chapter-exit-marker-hint") && argc == 4) { ChapterTeamMarkerToolTip(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "forced-tool-targeting-entry") && argc == 3) { ForcedToolTargetingEntry(argv[2]); return true; }
    if (!std::strcmp(mode, "tool-targeting-cursor-refresh") && argc == 3) { ToolTargetingCursorRefresh(argv[2]); return true; }
    if (!std::strcmp(mode, "sound-mixer-clock-catchup")) { SoundMixerClockCatchUp(); return true; }
    if (!std::strcmp(mode, "hotseat-opponent-visibility")) { HotseatOpponentVisibility(); return true; }
    if (!std::strcmp(mode, "restored-clue-close-notification")) { RestoredClueCloseNotification(); return true; }
    if (!std::strcmp(mode, "rejected-hit-label-text")) { Require(argc == 4, "hit label text requires db and res"); RejectedHitLabelText(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "scaled-edit-text-layout")) { ScaledEditTextLayout(); return true; }
    if (!std::strcmp(mode, "long-burst-voice-donor") && argc == 4) { LongBurstVoiceDonor(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "fresh-clue-wait-release") && argc == 4) { FreshClueWaitRelease(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mission-sound-game-clock")) { MissionSoundGameClock(); return true; }
    if (!std::strcmp(mode, "delayed-sound-effect-age") && argc == 3) { DelayedSoundEffectAge(argv[2]); return true; }
    if (!std::strcmp(mode, "infinite-sound-effect-loop") && argc == 3) { DelayedSoundEffectAge(argv[2],true); return true; }
    if (!std::strcmp(mode, "late-looping-sound-offset")) { LateLoopingSoundOffset(); return true; }
    if (!std::strcmp(mode, "failed-sound-channel-lifetime")) { FailedSoundChannelLifetime(); return true; }
    if (!std::strcmp(mode, "headless-music-stream")) { HeadlessMusicStream(); return true; }
    if (!std::strcmp(mode, "trackless-scene-music")) { TracklessSceneMusicContinuity(); return true; }
    if (!std::strcmp(mode, "reloaded-music-record")) { ReloadedMusicRecordContinuity(); return true; }
    if (!std::strcmp(mode, "exit-menu-confirmation") && argc == 4) { ExitMenuConfirmation(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "supplied-defeat-screenshot") && argc == 4) { SuppliedMenuScreenshot(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "supplied-options-screenshot") && argc == 4) { SuppliedMenuScreenshot(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "new-layer-compact-save")) { NewLayerCompactSave(); return true; }
    if (!std::strcmp(mode, "unsupported-navigation-floor")) { UnsupportedNavigationFloor(); return true; }
    if (!std::strcmp(mode, "unsupported-navigation-bake") && argc == 4) { UnsupportedNavigationBake(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "restored-ai-event-owner") && argc == 4) { AIEventOwnerBinding(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "fresh-ai-event-owner") && argc == 4) { AIEventOwnerBinding(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "legacy-ai-event-owner") && argc == 4) { AIEventOwnerBinding(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "scripted-unconscious-enemy") && argc == 4) { ScriptedEnemyDeath(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "scripted-enemy-death") && argc == 4) { ScriptedEnemyDeath(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "empty-passage-exit") && argc == 4) { ScriptedPassageTransition(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "scripted-passage-transition") && argc == 4) { ScriptedPassageTransition(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "flying-attack-queue") && argc == 4) { FlyingAttackQueue(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "connected-alias-moves")) { ConnectedAliasMoves(); return true; }
    if (!std::strcmp(mode, "isolated-source-moves")) { ConnectedAliasMoves(true); return true; }
    if (!std::strcmp(mode, "locked-alias-moves")) { ConnectedAliasMoves(false,true); return true; }
    if (!std::strcmp(mode, "isolated-source-path")) { IsolatedSourcePath(); return true; }
    if (!std::strcmp(mode, "prone-pose-change-facing")) { PronePoseChangeFacing(); return true; }
    if (!std::strcmp(mode, "simple-prone-pose-path")) { SimplePronePosePath(); return true; }
    if (!std::strcmp(mode, "simple-pose-facing-preference")) { SimplePoseFacingPreference(); return true; }
    if (!std::strcmp(mode, "simple-cross-layer-pose-entry")) { SimpleCrossLayerPoseEntry(); return true; }
    if (!std::strcmp(mode, "isolated-transition-link-match")) { IsolatedTransitionLinkMatch(); return true; }
    if (!std::strcmp(mode, "isolated-transition-shortcut")) { IsolatedTransitionShortcut(); return true; }
    if (!std::strcmp(mode, "isolated-transition-entry")) { IsolatedTransitionEntry(); return true; }
    if (!std::strcmp(mode, "connected-direct-transitions")) { ConnectedDirectTransitions(); return true; }
    if (!std::strcmp(mode, "nearby-connected-destination")) { NearbyConnectedDestination(); return true; }
    if (!std::strcmp(mode, "isolated-source-alias-vector")) { NavigationAliasDestinations(false,true); return true; }
    if (!std::strcmp(mode, "isolated-source-alias-array")) { NavigationAliasDestinations(true,true); return true; }
    if (!std::strcmp(mode, "navigation-alias-vector")) { NavigationAliasDestinations(false); return true; }
    if (!std::strcmp(mode, "navigation-alias-array")) { NavigationAliasDestinations(true); return true; }
    if (!std::strcmp(mode, "flying-shoot-direction")) { FlyingShootDirection(); return true; }
    if (!std::strcmp(mode, "flying-dynamic-locks")) { FlyingDynamicLocks(); return true; }
    if (!std::strcmp(mode, "arrived-retreat-reaction-lifetime")) { Require(argc == 4, "retreat reaction lifetime requires db and res"); ArrivedRetreatReactionLifetime(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "arrived-retreat-completion") && argc == 4) { ArrivedRetreatCompletion(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "finished-ai-logic-retirement") && argc == 4) { FinishedAILogicRetirement(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "removed-selected-enemy")) { RemovedSelectedAIContact(0); return true; }
    if (!std::strcmp(mode, "removed-selected-possible")) { RemovedSelectedAIContact(1); return true; }
    if (!std::strcmp(mode, "removed-selected-ally")) { RemovedSelectedAIContact(2); return true; }
    if (!std::strcmp(mode, "begin-turn-live-watcher") && argc == 4) { BeginTurnLiveContacts(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "begin-turn-live-enemy") && argc == 4) { BeginTurnLiveContacts(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "combat-chooser-context") && argc == 4) { CombatChooserContext(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "live-ai-position") && argc == 4) { LiveAIPosition(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "live-ai-unit-position") && argc == 4) { LiveAIPosition(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "owned-ai-action-enemy")) { OwnedAIEnemySelection(false); return true; }
    if (!std::strcmp(mode, "owned-ai-place-enemy")) { OwnedAIEnemySelection(true); return true; }
    if (!std::strcmp(mode, "mounted-ai-log-commands")) { MountedAILogCommands(); return true; }
    if (!std::strcmp(mode, "removed-player-ai-rosters") && argc == 4) { RemovedPlayerAIRosters(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "removed-player-acks") && argc == 4) { RemovedPlayerFootprints(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "removed-player-footprints") && argc == 4) { RemovedPlayerFootprints(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "empty-bullet-retirement") && argc == 4) { EmptyBulletConstruction(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "empty-bullet-construction") && argc == 4) { EmptyBulletConstruction(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "hidden-unit-tint") && argc == 4) { HiddenUnitTint(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "living-unit-floor-refresh") && argc == 4) { LivingUnitFloorRefresh(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "owned-ai-commander-rosters") && argc == 4) { OwnedAICommanderRosters(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "navigation-colouring-occupancy")) { NavigationColouringOccupancy(); return true; }
    if (!std::strcmp(mode, "missing-navigation-colourer")) { NavigationColouringOccupancy(1); return true; }
    if (!std::strcmp(mode, "stale-navigation-colourer")) { NavigationColouringOccupancy(2); return true; }
    if (!std::strcmp(mode, "ragdoll-restoration")) { RagdollRestoration(); return true; }
    if (!std::strcmp(mode, "negative-mesh-bounds")) { NegativeMeshBounds(); return true; }
    if (!std::strcmp(mode, "sniper-targetless-probe") && argc == 4) { SniperHeldAim(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "sniper-held-aim") && argc == 4) { SniperHeldAim(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "person-clue-creation") && argc == 4) { PersonClueCreation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "equipped-melee-attack-cursor")) { Require(argc == 4, "equipped melee cursor requires db and res"); EquippedMeleeAttackCursor(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "melee-attack-cursor") && argc == 4) { MeleeAttackCursor(argv[2]); return true; }
    if (!std::strcmp(mode, "blocked-attack-caption") && argc == 4) { AttackCaptionWithoutHitChance(argv[2], false); return true; }
    if (!std::strcmp(mode, "reload-attack-caption") && argc == 4) { AttackCaptionWithoutHitChance(argv[2], true); return true; }
    if (!std::strcmp(mode, "exact-attack-preview-target") && argc == 4) { ExactAttackPreviewTarget(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "equal-group-attack-caption") && argc == 4) { GroupAttackPreview(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "mixed-group-attack-caption") && argc == 4) { GroupAttackPreview(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "missing-attack-target-caption") && argc == 4) { AttackCaptionWithoutHitChance(argv[2], false, true); return true; }
    if (!std::strcmp(mode, "restore-action-state") && argc == 4) { RestoreActionState(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "zone-reentry-action-state") && argc == 4) { RestoreActionState(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "restore-building-hulls") && argc == 4) { RestoreBuildingHulls(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "carried-corpse-stability") && argc == 4) { CarriedCorpseStability(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "replacement-person-inventory-clue") && argc == 4) { PersonClueCreation(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "inventory-clue-creation") && argc == 4) { InventoryClueCreation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "inventory-clue-count") && argc == 4) { InventoryClueCreation(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "inventory-clue-capacity") && argc == 4) { InventoryClueCreation(argv[2], argv[3], true, true); return true; }
    if (!std::strcmp(mode, "corpse-snap-failure") && argc == 4) { FailedCorpseSnap(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "movement-critical-probe") && argc == 4) { MovementCriticalProbe(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "latest-route-command") && argc == 4) { LatestRouteCommand(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "oversized-dialogue-line") && argc == 4) { OversizedDialogueLine(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "markup-dialogue-progress") && argc == 4) { OversizedDialogueLine(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "dialogue-normal-pagination") && argc == 4) { OversizedDialogueLine(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "movement-first-step-cancel") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "movement-next-step-cancel") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "movement-first-step-release") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 1, true); return true; }
    if (!std::strcmp(mode, "movement-next-step-release") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 2, true); return true; }
    if (!std::strcmp(mode, "hide-cooldown") && argc == 4) { HideCooldown(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-idle") && argc == 4) { IdleAI(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "mixed-hide-button") && argc == 4) { MixedActionButton(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "mixed-strafe-button") && argc == 4) { MixedActionButton(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "mounted-mode-button") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "mounted-weapon-submenu") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "mounted-weapon-mode-ui") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "effective-weapon-mode") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "render-removal")) { RenderRemoval(); return true; }
    if (!std::strcmp(mode, "render-pass-compaction")) { RenderPassCompaction(); return true; }
    if (!std::strcmp(mode, "state-lifetime")) { ReentrantStateLifetime(); return true; }
    if (!std::strcmp(mode, "mounted-mode-command")) { MountedModeCommand(); return true; }
    if (!std::strcmp(mode, "invalid-ai-weapon")) { InvalidAIWeapon(); return true; }
    if (!std::strcmp(mode, "stale-cannon-selection")) { StaleWeaponReferences(0); return true; }
    if (!std::strcmp(mode, "stale-firearm-type")) { StaleWeaponReferences(1); return true; }
    if (!std::strcmp(mode, "stale-melee-type")) { StaleWeaponReferences(2); return true; }
    if (!std::strcmp(mode, "stale-ai-reaction")) { StaleAIReaction(); return true; }
    if (!std::strcmp(mode, "overlapping-unit-locks")) { OverlappingUnitLocks(); return true; }
    if (!std::strcmp(mode, "invalid-unit-locks")) { InvalidUnitLocks(); return true; }
    if (!std::strcmp(mode, "dead-unit-locks-single") && argc == 4) { DeadUnitLocks(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "dead-unit-locks-moving") && argc == 4) { DeadUnitLocks(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "dead-unit-locks-selection") && argc == 4) { DeadUnitLocks(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "queue-lifetime")) { ReentrantQueueLifetime(false); return true; }
    if (!std::strcmp(mode, "queue-animation-lifetime")) { ReentrantQueueLifetime(true); return true; }
    if (!std::strcmp(mode, "pk-leave")) { ScriptPK(true); return true; }
    if (!std::strcmp(mode, "pk-wear")) { ScriptPK(false); return true; }
    if (!std::strcmp(mode, "pk-equipment")) { PKEquipmentTransfer(); return true; }
    if (!std::strcmp(mode, "pk-fixed-weapon")) { PKFixedWeaponPolicy(); return true; }
    if (!std::strcmp(mode, "door-collision")) { DoorCollision(); return true; }
    if (!std::strcmp(mode, "door-swing-direction") && argc == 4) { DoorSwingDirection(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "door-approach-heading") && argc == 4) { DoorApproachHeading(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-animation-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-direction-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::Direction); return true; }
    if (!std::strcmp(mode, "deferred-pose-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::Pose); return true; }
    if (!std::strcmp(mode, "deferred-wish-pose-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::WishPose); return true; }
    if (!std::strcmp(mode, "deferred-shoot-mode-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::ShootMode); return true; }
    if (!std::strcmp(mode, "deferred-reload-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::Reload); return true; }
    if (!std::strcmp(mode, "deferred-weapon-activation-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::ActivateWeapon); return true; }
    if (!std::strcmp(mode, "deferred-hide-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::Hide); return true; }
    if (!std::strcmp(mode, "deferred-create-activate-command") && argc == 4) { DeferredCreateAndActivateItem(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-take-object-command") && argc == 4) { DeferredTakeObject(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-shoot-command") && argc == 4) { DeferredScriptShot(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-shoot-prepare-command") && argc == 4) { DeferredScriptShot(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "deferred-fly-waypoint-command") && argc == 4) { DeferredWaypointCommand(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-attack-waypoint-command") && argc == 4) { DeferredWaypointCommand(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "deferred-take-corpse-command") && argc == 4) { DeferredCorpseCommand(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-script-command-order") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::ShootMode,true); return true; }
    if (!std::strcmp(mode, "script-draw-weapon-slot") && argc == 4) { ScriptDrawWeaponSlot(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-draw-weapon-command") && argc == 4) { ScriptDrawWeaponSlot(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "deferred-draw-grenade-command") && argc == 4) { ScriptDrawWeaponSlot(argv[2],argv[3],true,true); return true; }
    if (!std::strcmp(mode, "script-held-grenade-activation") && argc == 4) { ScriptHeldGrenadeActivation(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "forced-action-recalc") && argc == 4) { ForcedActionRecalc(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "overlapping-doors")) { OverlappingDoors(); return true; }
    if (!std::strcmp(mode, "weapon-familiarity")) { WeaponFamiliarity(); return true; }
    if (!std::strcmp(mode, "clue-slots")) { ClueSlotSelection(); return true; }
    if (!std::strcmp(mode, "conclusions") && argc == 4) { ConclusionRouting(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "pk-placement")) { PKPlacement(); return true; }
    if (!std::strcmp(mode, "critical-ids") && argc == 3) { CriticalIDs(argv[2]); return true; }
    if (!std::strcmp(mode, "idle-blocker") && argc == 4) { IdlePathBlocker(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "hidden-portrait")) { HiddenPortraitAck(); return true; }
    if (!std::strcmp(mode, "clue-initialization")) { ClueInitialization(); return true; }
    if (!std::strcmp(mode, "store-drop")) { StoreDropPosition(); return true; }
    if (!std::strcmp(mode, "inventory-actor")) { InventoryTransferActor(true); return true; }
    if (!std::strcmp(mode, "slot-actor")) { InventoryTransferActor(false); return true; }
    if (!std::strcmp(mode, "ownerless-inventory-drag")) { OwnerlessInventoryDrag(); return true; }
    if (!std::strcmp(mode, "geometry-hash")) { GeometryHashBuckets(); return true; }
    if (!std::strcmp(mode, "clue-notification") && argc == 4) { ImmediateClueNotification(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "clue-pickup") && argc == 4) { ImmediateCluePickup(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "clue-capacity") && argc == 4) { ImmediateCluePickup(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "dead-pilot") && argc == 4) { PKShellWorld(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "pk-waypoint") && argc == 4) { PKShellWorld(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "pk-critical-feedback") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 1); return true; }
    if (!std::strcmp(mode, "pk-pose-feedback") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 2); return true; }
    if (!std::strcmp(mode, "human-sequence-hiding") && argc == 4) { HiddenUnitTint(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "pk-sequence-hiding") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 5); return true; }
    if (!std::strcmp(mode, "hidden-pk-material") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 4); return true; }
    if (!std::strcmp(mode, "pk-turn-animation") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 3); return true; }
    if (!std::strcmp(mode, "pk-model-ownership") && argc == 4) { PKShellWorld(argv[2], argv[3], false, true); return true; }
    return false;
}

int main(int argc, char** argv)
{
    try {
        const char* mode = argc > 1 ? argv[1] : "temporary-groups";
        if (!RunRegressionMode(mode, argc, argv)) throw std::runtime_error("unknown upstream regression mode");
        std::printf("PASS: %s\n", mode);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
