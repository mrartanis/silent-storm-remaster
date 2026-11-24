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

static void LuaImplicitEndYield(int mode)
{
    Require(mode>=0 && mode<4,"implicit end mode outside fixture range");
    Script script;
    const bool explicitReturn=mode==1;
    const char* source=explicitReturn
        ? "trace = 0; function child() trace = trace * 10 + 1; return end; child(); trace = trace * 10 + 2"
        : mode==2
        ? "trace = 0; function leaf() trace = trace * 10 + 1 end; function child() leaf(); trace = trace * 10 + 2 end; child(); trace = trace * 10 + 3"
        : "trace = 0; function child() trace = trace * 10 + 1 end; child(); trace = trace * 10 + 2";
    Require(script.DoString(source)==LUA_NOERR,"implicit end fixture did not compile in the native VM");
    script.ExecuteThreads();
    const int first=script.GetGlobal("trace").GetNumber();
    std::printf("LUA-IMPLICIT-END mode=%d update=1 trace=%d expected=%d\n",mode,first,explicitReturn?12:1);
    Require(first==(explicitReturn?12:1),"native implicit function end continued its caller in the same thread update");
    if(mode==3) {
        CMemoryStream bytes;
        { CStructureSaver save(bytes,CStructureSaver::WRITE);save.Add(1,&script); }
        bytes.Seek(0);
        Script restored;
        { CStructureSaver load(bytes,CStructureSaver::READ);load.Add(1,&restored); }
        Require(restored.GetGlobal("trace").GetNumber()==first,"saving a suspended native Lua caller changed its progress");
        restored.ExecuteThreads();script.ExecuteThreads();
        const int live=script.GetGlobal("trace").GetNumber();
        const int loaded=restored.GetGlobal("trace").GetNumber();
        std::printf("LUA-IMPLICIT-END mode=%d update=2 live=%d loaded=%d expected=12\n",mode,live,loaded);
        Require(live==12 && loaded==12,"fresh native Lua-state reload did not resume the suspended caller on the next update");
        restored.ExecuteThreads();script.ExecuteThreads();
        Require(restored.GetGlobal("trace").GetNumber()==12 && script.GetGlobal("trace").GetNumber()==12,"completed native Lua caller repeated after save/load");
        return;
    }
    script.ExecuteThreads();
    const int second=script.GetGlobal("trace").GetNumber();
    std::printf("LUA-IMPLICIT-END mode=%d update=2 trace=%d expected=12\n",mode,second);
    Require(second==12,"native Lua caller did not resume exactly once on its next thread update");
    if(mode==2) {
        script.ExecuteThreads();
        const int third=script.GetGlobal("trace").GetNumber();
        std::printf("LUA-IMPLICIT-END mode=%d update=3 trace=%d expected=123\n",mode,third);
        Require(third==123,"nested native implicit ends did not preserve one caller per thread update");
    }
    script.ExecuteThreads();
    Require(script.GetGlobal("trace").GetNumber()==(mode==2?123:12),"completed native Lua script repeated or lost final progress");
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

// Keep the native call boundary out of MSVC's test-loop inliner.
#if defined(_MSC_VER)
__declspec(noinline)
#else
__attribute__((noinline))
#endif
static unsigned char ColouringMoveCost(const NAI::CNodesLayerProxy* proxy,
    unsigned char x,unsigned char y,NAI::EDirection direction)
{
    return proxy->GetOneMoveCost(x,y,direction);
}

static void InvalidRegionColouring()
{
    auto layer = []() {
        CObj<NAI::CNodesLayer> result = new NAI::CNodesLayer;
        result->nLayer = result->nFloor = 0; result->tiles.SetSizes(3,3);
        for (int y=0;y<3;++y) for (int x=0;x<3;++x) result->tiles[y][x] = NAI::STile{};
        return result;
    };
    // Preserve existing valid-edge weights: cardinal/diagonal and all four
    // supported movement types, with the stand-only restriction unchanged.
    const int poses[] = {2,1,0,3}, weights[] = {1,2,3,5};
    for (auto direction: {NAI::RIGHT,NAI::UPRIGHT})
        for (int kind=0;kind<4;++kind) {
            auto grid = layer(); const int opposite = (int(direction)+4)&7;
            auto& source = grid->tiles[1][1];
            auto& target = grid->tiles[1+NAI::cTranslations[direction][1]][1+NAI::cTranslations[direction][0]];
            source.nPassable = target.nPassable = 7;
            source.nFlags = target.nFlags = NAI::TF_STAND_PASSABLE;
            source.nMove[poses[kind]] = static_cast<char>(1<<direction);
            target.nMove[poses[kind]] = static_cast<char>(1<<opposite);
            CObj<NAI::CNodesLayerProxy> proxy = new NAI::CNodesLayerProxy(grid);
            const int step = direction == NAI::RIGHT ? 2 : 3;
            proxy->SetPathfinderMode(NAI::PM_ANY_MOVE);
            Require(ColouringMoveCost(proxy.GetPtr(),1,1,direction) == step*weights[kind],
                "valid navigation edge changed its existing movement cost");
            proxy->SetPathfinderMode(NAI::PM_STAND_ONLY);
            Require(ColouringMoveCost(proxy.GetPtr(),1,1,direction) == (kind == 0 ? step : 0),
                "stand-only navigation changed its original pose restriction");
            proxy->SetPathfinderMode(NAI::PM_ANY_MOVE); target.nLocks = 1;
            Require(ColouringMoveCost(proxy.GetPtr(),1,1,direction) == 0, "valid source entered a locked target");
        }
    auto grid = layer();
    auto& blocked = grid->tiles[1][1]; auto& walkable = grid->tiles[1][0];
    blocked.nMoveStand = static_cast<char>(1<<NAI::LEFT);
    walkable.nMoveStand = static_cast<char>(1<<NAI::RIGHT);
    walkable.nPassable = 7; walkable.nFlags = NAI::TF_STAND_PASSABLE;
    CObj<NAI::CNodesLayerProxy> proxy = new NAI::CNodesLayerProxy(grid);
    Require(!proxy->IsValidPoint(1,1) && proxy->IsValidPoint(0,1),
        "navigation region fixture did not retain one invalid source and one valid neighbour");
    CObj<NAI::CMapColourer> colourer = new NAI::CMapColourer;
    colourer->SetLayer(grid,nullptr,0); colourer->ConstructColouring(nullptr);
    std::printf("native recolouring: invalid=%u valid=%u internal-invalid=%u edge-cost=%d\n",
        unsigned(colourer->GetPointColor(1,1)),unsigned(colourer->GetPointColor(0,1)),
        unsigned(colourer->GetPointColor(0,0)),int(ColouringMoveCost(proxy.GetPtr(),1,1,NAI::LEFT)));
    Require(colourer->GetPointColor(1,1) != colourer->GetPointColor(0,1),
        "native colouring joined an invalid tile to a walkable region through a stale movement flag");
    Require(colourer->GetPointColor(1,1) == colourer->GetPointColor(0,0),
        "native colouring failed to connect the internal invalid region");
    for (auto mode: {NAI::PM_STAND_ONLY,NAI::PM_ANY_MOVE}) {
        proxy->SetPathfinderMode(mode);
        Require(ColouringMoveCost(proxy.GetPtr(),1,1,NAI::LEFT) == 0,
            "invalid source still connects to a valid neighbour");
        Require(ColouringMoveCost(proxy.GetPtr(),1,1,NAI::UP) == 2 && ColouringMoveCost(proxy.GetPtr(),1,1,NAI::UPRIGHT) == 3,
            "internal invalid-region colouring changed cardinal/diagonal weights");
    }
}

static void FreshNavigationColouring()
{
    CObj<NAI::CNodesLayer> layer = new NAI::CNodesLayer;
    layer->nLayer = layer->nFloor = 0;
    layer->tiles.SetSizes(8, 8);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) layer->tiles[y][x] = NAI::STile{};
    // Two genuine walkable islands; blocked-cell label identities are not
    // part of this cache-initialization regression's contract.
    layer->tiles[0][0].nFlags = layer->tiles[7][7].nFlags = NAI::TF_STAND_PASSABLE;
    layer->tiles[0][0].nPassable = layer->tiles[7][7].nPassable = 7;
    CObj<NAI::CMapColourer> full = new NAI::CMapColourer;
    full->SetLayer(layer, nullptr, 0);
    full->ConstructColouring(nullptr);
    Require(full->IsReady() && full->GetPointColor(0, 0) != full->GetPointColor(7, 7),
        "fresh-colouring fixture did not build separate walkable islands");
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

static void EmptyModelFallback(const char* databasePath)
{
    CFileStream source;source.OpenRead(databasePath);vector<unsigned char> bytes(source.GetSize());source.Read(bytes.data(),unsigned(bytes.size()));
    S2FileIO::PortableGameDatabase decoded;string error;
    Require(S2FileIO::LoadPortableGameDatabaseBytes(bytes.data(),bytes.size(),&decoded,&error),"empty-model fixture could not decode its actual game database");
    const S2FileIO::GameDatabaseTable* models=nullptr;
    for(const auto& table:decoded.tables)if(table.tableId==1) {models=&table;break;}
    Require(models,"empty-model fixture lacks the real Models table");
    auto column=[&](const char* name) {
        auto found=std::find(models->intNames.begin(),models->intNames.end(),name);
        Require(found!=models->intNames.end(),"empty-model fixture lacks an authored model column");return size_t(found-models->intNames.begin());
    };
    const auto id=column("ID"),geometry=column("GeometryID"),parent=column("TemplateID");
    int nullVariant=0,nullTemplate=0,validVariant=0,validGeometry=0,validTemplate=0,nullCount=0;
    for(const auto& row:models->intRows) {
        if(row[id]<=0 || row[parent]<=0)continue;
        if(row[geometry]==0) {++nullCount;if(!nullVariant){nullVariant=row[id];nullTemplate=row[parent];}}
        else if(row[geometry]>0 && row[geometry]!=4299 && !validVariant) {
            validVariant=row[id];validGeometry=row[geometry];validTemplate=row[parent];
        }
    }
    std::printf("native empty-model input: nullVariants=%d selected=%d template=%d validVariant=%d validGeometry=%d\n",
        nullCount,nullVariant,nullTemplate,validVariant,validGeometry);std::fflush(stdout);
    Require(nullVariant && validVariant,"empty-model fixture lacks its actual authored null-geometry variant or valid control");
    CMemoryStream native;native.Write(bytes.data(),unsigned(bytes.size()));native.Seek(0);NDatabase::Serialize(native,CStructureSaver::READ);
    auto* expected=NDb::GetGeometry(4299);auto* valid=NDb::GetGeometry(validGeometry);
    Require(expected && valid && expected!=valid,"empty-model fixture lacks the actual original default geometry and distinct valid control");
    std::printf("native default geometry control: id=%d primaryAI=%d flags=%u secondaryAI=%d flags=%u bound=%g/%g/%g\n",expected->GetRecordID(),
        IsValid(expected->pAIGeometry)?expected->pAIGeometry->GetRecordID():-1,IsValid(expected->pAIGeometry)?unsigned(expected->pAIGeometry->traficability):0,
        IsValid(expected->pAIGeometry2)?expected->pAIGeometry2->GetRecordID():-1,IsValid(expected->pAIGeometry2)?unsigned(expected->pAIGeometry2->traficability):0,
        expected->boundSize.x,expected->boundSize.y,expected->boundSize.z);
    SRand random;CObj<NDb::CModel> control=NDb::GetModelVariant(validVariant,&random);
    Require(control && control->pGeometry==valid && IsValid(control->pSrcRndModel) && control->pSrcRndModel->GetRecordID()==validTemplate,
        "native model import changed a valid authored geometry or source-template identity");
    CObj<NDb::CModel> empty=NDb::GetModelVariant(nullVariant,&random);
    Require(empty && IsValid(empty->pSrcRndModel) && empty->pSrcRndModel->GetRecordID()==nullTemplate,
        "native empty-model variant lost its actual source template");
    std::printf("native empty-model fallback: variant=%d geometry=%d expected=%d primaryFlags=%u\n",nullVariant,
        IsValid(empty->pGeometry)?empty->pGeometry->GetRecordID():-1,expected->GetRecordID(),
        IsValid(empty->pGeometry) && IsValid(empty->pGeometry->pAIGeometry)?unsigned(empty->pGeometry->pAIGeometry->traficability):0);std::fflush(stdout);
    Require(empty->pGeometry==expected,"native database import substituted the sphere instead of the original empty-model default geometry4299");
    Require(IsValid(expected->pAIGeometry) && unsigned(expected->pAIGeometry->traficability)==0 && !IsValid(expected->pAIGeometry2),
        "original empty-model default unexpectedly injects collision/cover/vision/item traffic");
    const int pass=NWorld::TS_PASS_BLOCKER|NWorld::TS_WEAPON_BLOCKER|NWorld::TS_ITEM_BLOCKER;
    const int blocked=pass|NWorld::TS_COVER|NWorld::TS_VISION;
    const int defaultMask=NWorld::GetMask(expected->pAIGeometry,nullptr);
    auto* sphere=NDb::GetGeometry(6);Require(sphere && sphere->pAIGeometry,"fallback mask control lacks its explicit original sphere resource");
    const int sphereMask=NWorld::GetMask(sphere->pAIGeometry,nullptr);
    Require(!(defaultMask&blocked) && (sphereMask&pass)==pass,"native object mask did not distinguish the nonblocking default from the real sphere");
    std::printf("native empty-model masks: default4299=0x%X sphere6=0x%X blockerBits=0x%X\n",unsigned(defaultMask),unsigned(sphereMask),unsigned(blocked));
    int verified=0;
    for(const auto& row:models->intRows) {
        if(row[id]<=0 || row[parent]<=0 || row[geometry]!=0)continue;
        CObj<NDb::CModel> model=NDb::GetModelVariant(row[id],&random);
        Require(model && model->pGeometry==expected && model->pSrcRndModel && model->pSrcRndModel->GetRecordID()==row[parent],
            "an authored empty variant retained a blocking fallback or lost its source-template identity");
        Require(!(NWorld::GetMask(model->pGeometry->pAIGeometry,model->pRPGArmor)&blocked),
            "native object mask turns an authored empty variant into a movement/weapon/item/cover/vision blocker");++verified;
    }
    Require(verified==nullCount,"empty-model fixture did not verify every authored null-geometry variant");
    CObj<NDb::CModel> retained=NDb::GetModelVariant(validVariant,&random);
    Require(retained && retained->pGeometry==valid && retained->pSrcRndModel==control->pSrcRndModel,
        "repairing empty variants changed an explicitly authored model geometry/template");
    std::printf("native empty-model verified: %d/%d original default4299 AItraffic0, explicit geometry%d retained\n",verified,nullCount,validGeometry);
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

static void WindowActivationFlags(int mode)
{
    Require(mode>=0 && mode<=5,"unknown native window activation case");
    CObj<NUI::CWindow> root=new NUI::CWindow(NUI::SWindowInfo());
    if(mode<=1) {
        if(mode==1)root->ProcessMessage(NUI::SEvent(NUI::EVENT_ACTIVATE,NUI::EAF_ACTIVATE));
        Require(root->IsActive()==(mode==1),"notification control did not prepare its native initial activity");
        const int flags=mode==0?NUI::EAF_ACTIVATENOTIFY:NUI::EAF_DEACTIVATENOTIFY;
        const bool handled=root->ProcessMessage(NUI::SEvent(NUI::EVENT_ACTIVATE,flags));
        std::printf("native window activation mode=%d flags=%x handled=%d active=%d\n",mode,flags,int(handled),int(root->IsActive()));std::fflush(stdout);
        Require(handled && root->IsActive()==(mode==0),"native activation notification flags did not update window activity");
        return;
    }
    if(mode==4) {
        root->SetStyle(NUI::STYLE_NOACTIVATE,true);
        Require(root->ProcessMessage(NUI::SEvent(NUI::EVENT_ACTIVATE,NUI::EAF_ACTIVATENOTIFY)) && !root->IsActive(),
            "NOACTIVATE window accepted a native activation notification");
        return;
    }
    if(mode==5) {
        for(int flags:{NUI::EAF_ACTIVATE,NUI::EAF_DEACTIVATE}) {
            Require(root->ProcessMessage(NUI::SEvent(NUI::EVENT_ACTIVATE,flags)) && root->IsActive()==(flags==NUI::EAF_ACTIVATE),
                "plain native activation/deactivation stopped updating window activity");
        }
        return;
    }
    CObj<NUI::CWindow> child=new NUI::CWindow(NUI::SWindowInfo(root,NUI::SPoint(0,0),NUI::SPoint(20,20),"activation-child"));
    CObj<NUI::CWindow> grandchild=new NUI::CWindow(NUI::SWindowInfo(child,NUI::SPoint(0,0),NUI::SPoint(10,10),"activation-grandchild"));
    const bool activating=mode==2;
    const int initial=activating?NUI::EAF_DEACTIVATE:NUI::EAF_ACTIVATE;
    for(auto* window:{root.GetPtr(),child.GetPtr(),grandchild.GetPtr()})window->ProcessMessage(NUI::SEvent(NUI::EVENT_ACTIVATE,initial));
    Require(root->IsActive()==!activating && child->IsActive()==!activating && grandchild->IsActive()==!activating,
        "native activation tree did not prepare three ordinary initial window states");
    const int flags=activating?NUI::EAF_ACTIVATE:NUI::EAF_DEACTIVATE;
    const bool handled=root->ProcessMessage(NUI::SEvent(NUI::EVENT_ACTIVATE,flags));
    std::printf("native window activation mode=%d root=%d child=%d grandchild=%d\n",mode,int(root->IsActive()),int(child->IsActive()),int(grandchild->IsActive()));std::fflush(stdout);
    Require(handled && root->IsActive()==activating && child->IsActive()==activating && grandchild->IsActive()==activating,
        "native activation notification did not propagate through the real child window tree");
    Require(child->GetParent()==root.GetPtr() && grandchild->GetParent()==child.GetPtr(),"activation changed the real window parent links");
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

namespace NFMSound {
class CChannel;
extern std::unordered_map<int,CMObj<CChannel>> hashChannel;
}

static void Sound2DEnding()
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS")!=nullptr,"2D ending fixture requires headless native audio");
    NFMSound::SStartInfo info;info.nMaxChannels=2;
    Require(NFMSound::Init(info),"2D ending fixture could not initialize our native mixer");
    struct Cleanup { ~Cleanup() { NFMSound::Done(); } } cleanup;
    auto wave=SilentPCMFixture(4096);
    for(int frame=0;frame<4096;++frame) {
        const int amplitude=frame<1536 ? 4000 : 16000;
        wave[44+frame*2]=amplitude&255;wave[45+frame*2]=amplitude>>8;
    }
    auto sample=[&]() {
        CObj<NFMSound::CSample2D> result=NFMSound::LoadSample2D(wave.data(),int(wave.size()));
        Require(IsValid(result),"2D ending fixture could not decode its marked PCM sample");return result;
    };
    std::vector<float> pcm(8192);
    auto mix=[&](int frames) {
        Require(S2FmodRenderForTest(pcm.data(),frames)!=0,"2D ending fixture could not render actual PCM");
        float peak=0;for(int i=0;i<frames*2;++i)peak=Max(peak,std::fabs(pcm[i]));return peak;
    };
    auto channel=[&](int except=-1) {
        int found=-1;
        for(int id=0;id<128;++id)if(id!=except && FSOUND_IsPlaying(id)) {
            Require(found==-1,"2D ending fixture has an unexpected extra native voice");found=id;
        }
        Require(found>=0,"2D ending fixture could not identify its real voice");return found;
    };
    CTransformStack transform;transform.Init();
    NFMSound::SListener listener;listener.vPosition=CVec3(0,0,0);listener.toProjective=transform.Get().forward;
    CObj<NFMSound::CSample2D> shared=sample();
    // Backend control: switching an OWNED loop off reaches the complete sample
    // tail without changing shared sample loop points or adding an FMOD API.
    CObj<NFMSound::CSound2D> backend=NFMSound::PlaySound(shared,0,512,1536);
    const int backendChannel=channel();const float loopPeak=mix(2048);
    Require(loopPeak>0 && FSOUND_GetLoopMode(backendChannel)==FSOUND_LOOP_NORMAL,
        "2D ending backend control did not play the loop region");
    Require(FSOUND_SetLoopMode(backendChannel,FSOUND_LOOP_OFF)!=0,
        "2D ending backend control could not release its loop");
    float tailPeak=0;for(int i=0;i<10;++i)tailPeak=Max(tailPeak,mix(512));
    Require(tailPeak>loopPeak*2 && !FSOUND_IsPlaying(backendChannel),
        "our native backend did not play the marked tail to completion");
    NFMSound::Update(listener);
    Require(NFMSound::hashChannel.empty(),"2D ending backend control did not clean up");
    std::printf("native 2D backend control: loop peak=%g tail=%g, complete sample reached\n",loopPeak,tailPeak);
    std::fflush(stdout);
    CObj<NFMSound::CSound2D> ending=NFMSound::PlaySound(shared,0,512,1536);
    const int endingChannel=channel();
    CObj<NFMSound::CSound2D> independent=NFMSound::PlaySound(shared,0,0,0,true);
    const int independentChannel=channel(endingChannel);
    const float mixedLoopPeak=mix(2048);
    CObj<NFMSound::CSound2D> otherOwner=ending.GetPtr();ending=nullptr;
    Require(NFMSound::IsPlaying(otherOwner) && FSOUND_GetLoopMode(endingChannel)==FSOUND_LOOP_NORMAL,
        "2D ending started before the last scene owner released the loop");
    std::puts("native 2D last-owner control: shared sample and independent loop remain active");std::fflush(stdout);
    CPtr<NFMSound::CSound2D> retired=otherOwner.GetPtr();otherOwner=nullptr;
    std::printf("native 2D release: retired=%d ending=%d independent=%d\n",
        int(!IsValid(retired)),int(FSOUND_IsPlaying(endingChannel)),int(NFMSound::IsPlaying(independent)));
    std::fflush(stdout);
    Require(!IsValid(retired) && FSOUND_IsPlaying(endingChannel),
        "releasing the 2D scene owner cuts off its sample ending");
    Require(FSOUND_GetLoopMode(endingChannel)==FSOUND_LOOP_OFF,
        "released 2D ending continues looping instead of finishing");
    tailPeak=0;for(int i=0;i<10;++i)tailPeak=Max(tailPeak,mix(512));
    NFMSound::Update(listener);
    Require(tailPeak>mixedLoopPeak*1.8f && !FSOUND_IsPlaying(endingChannel) &&
        NFMSound::IsPlaying(independent) && FSOUND_GetLoopMode(independentChannel)==FSOUND_LOOP_NORMAL &&
        FSOUND_GetCurrentPosition(independentChannel)>=512 && FSOUND_GetCurrentPosition(independentChannel)<1536 &&
        NFMSound::hashChannel.size()==1,
        "2D ending failed to finish or changed the independent shared-sample loop");
    independent=nullptr;NFMSound::Update(listener);
    Require(!FSOUND_IsPlaying(independentChannel) && NFMSound::hashChannel.empty(),
        "non-ending loop release changed its immediate-stop behavior");
    shared=nullptr;

    CObj<NFMSound::CSample2D> owned=sample();
    CPtr<NFMSound::CSample2D> observedSample=owned.GetPtr();
    ending=NFMSound::PlaySound(owned,0,512,1536);
    const int ownedChannel=channel();const float ownedLoopPeak=mix(2048);
    owned=nullptr;ending=nullptr;
    Require(IsValid(observedSample) && FSOUND_IsPlaying(ownedChannel),
        "2D ending holder failed to retain the decoded sample after all scene references expired");
    tailPeak=0;for(int i=0;i<10;++i)tailPeak=Max(tailPeak,mix(512));
    NFMSound::Update(listener);
    Require(tailPeak>ownedLoopPeak*2 && !FSOUND_IsPlaying(ownedChannel) &&
        !IsValid(observedSample) && NFMSound::hashChannel.empty(),
        "completed 2D ending retained its sample/channel or never played its marked tail");

    // Shutdown must not insert a new tail holder while clearing the owner map.
    for(bool releaseBeforeShutdown:{false,true}) {
        Require(NFMSound::IsInitialized() || NFMSound::Init(info),"2D shutdown fixture could not restart audio");
        owned=sample();observedSample=owned.GetPtr();ending=NFMSound::PlaySound(owned,0,512,1536);
        const int shutdownChannel=channel();mix(512);owned=nullptr;
        if(releaseBeforeShutdown)ending=nullptr;
        NFMSound::Done();
        Require(!NFMSound::IsInitialized() && NFMSound::hashChannel.empty() &&
            !FSOUND_IsPlaying(shutdownChannel) && !IsValid(observedSample),
            "audio shutdown retained or recreated an owned/ending 2D channel");
        ending=nullptr;
    }
    std::puts("native 2D ending: audible tail, shared loop isolation, sample lifetime, immediate-stop control and owned/tail shutdown passed");
#else
    throw std::runtime_error("2D ending regression requires the native audio backend");
#endif
}

static void ReleasedSoundChannelCleanup()
{
#if defined(S2_UPSTREAM_NATIVE_SFX)
    Require(std::getenv("S2_AUDIO_HEADLESS")!=nullptr,"released-channel fixture requires headless native audio");
    NFMSound::SStartInfo info;info.nMaxChannels=2;
    Require(NFMSound::Init(info),"released-channel fixture could not initialize our native mixer");
    struct Cleanup { ~Cleanup() { NFMSound::Done(); } } cleanup;
    Require(NFMSound::hashChannel.empty(),"released-channel fixture did not start with an empty owner table");
    const auto wave=SilentPCMFixture();
    CObj<NFMSound::CSample2D> sample=NFMSound::LoadSample2D(wave.data(),int(wave.size()));
    Require(IsValid(sample),"released-channel fixture could not decode its actual PCM sample");
    CTransformStack transform;transform.Init();
    NFMSound::SListener listener;listener.vPosition=CVec3(0,0,0);listener.toProjective=transform.Get().forward;
    std::vector<float> pcm(4096);
    CObj<NFMSound::CSound2D> finished=NFMSound::PlaySound(sample);
    Require(IsValid(finished) && NFMSound::IsPlaying(finished),"completed-channel control did not start");
    Require(S2FmodRenderForTest(pcm.data(),2048)!=0 && !NFMSound::IsPlaying(finished),
        "completed-channel control did not finish through the actual mixer");
    NFMSound::Update(listener);
    Require(NFMSound::hashChannel.empty(),"normal completed-channel cleanup control failed");
    CObj<NFMSound::CSound2D> live=NFMSound::PlaySound(sample,0,0,0,true);
    Require(IsValid(live) && NFMSound::IsPlaying(live) && NFMSound::hashChannel.size()==1,
        "released-channel fixture did not retain its unrelated looping control");
    std::puts("native channel cleanup controls: completed voice removed, unrelated live loop retained");
    std::fflush(stdout);
    for(int cycle=0;cycle<16;++cycle) {
        CObj<NFMSound::CSound2D> released=NFMSound::PlaySound(sample,0,0,0,true);
        Require(IsValid(released) && NFMSound::IsPlaying(released),"released-channel fixture could not start its temporary voice");
        CPtr<NFMSound::CSound2D> observed=released.GetPtr();released=nullptr;
        Require(!IsValid(observed),"temporary sound retained a scene owner after its handle was released");
        NFMSound::Update(listener);
        std::printf("native released-channel cleanup cycle=%d table=%zu expected=1 live=%d\n",
            cycle,NFMSound::hashChannel.size(),int(NFMSound::IsPlaying(live)));std::fflush(stdout);
        Require(NFMSound::hashChannel.size()==1,"released sound entries accumulate in the native channel owner table");
        Require(NFMSound::IsPlaying(live) && IsValid(sample),
            "released-channel cleanup stopped the unrelated loop or freed its decoded sample");
    }
    live=nullptr;NFMSound::Update(listener);
    Require(NFMSound::hashChannel.empty(),"final released voice remains in the native owner table");
#else
    throw std::runtime_error("released-channel regression requires the native audio backend");
#endif
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
