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
    std::printf("MELEE-REACH-CAPTION shared=%d farAPOnly=1 inReachPercent=1 APsame=1 noCommand=1\n",sharedChance);std::fflush(stdout);
    Require(sharedChance==-1,"unreachable melee caption must share the unavailable sentinel with scripts and network queries");
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
        if (IsValid(kit->pItem) && kit->effect == NDb::FAE_NORMAL && kit->GetRecordID()==1 && kit->nQuantity >= 2) { record = kit; break; }
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

static NWorld::CExecShoot* FindNativeObjectOrPointShot(NWorld::CCommandExecute* executor)
{
    if(auto* shot=dynamic_cast<NWorld::CExecShoot*>(executor))return shot;
    auto* queue=dynamic_cast<NWorld::CSimpleExecQueue*>(executor);if(!queue)return nullptr;
    vector<CObjectBase*> objects;CStructureNetworkContext graph;graph.include=[&](CObjectBase* child){objects.push_back(child);return false;};
    CMemoryStream traversal;{CStructureSaver writer(traversal,CStructureSaver::WRITE,&graph);queue->NWorld::CSimpleExecQueue::operator&(writer);}
    CStructureNetworkContext references;references.externalReferences=true;references.nextID=100;references.include=[](CObjectBase*){return false;};
    for(auto* child:objects)if(!references.ids.count(child)){const auto id=references.nextID++;references.ids[child]=id;references.refs[id]=child;}
    CMemoryStream fields;{CStructureSaver writer(fields,CStructureSaver::WRITE,&references);queue->NWorld::CSimpleExecQueue::operator&(writer);}fields.Seek(0);
    list<CObj<NWorld::CCommandExecute>> children;{CStructureSaver reader(fields,CStructureSaver::READ,&references);reader.Add(2,&children);}
    for(const auto& child:children)if(auto* shot=FindNativeObjectOrPointShot(child))return shot;
    return nullptr;
}

static NWorld::CExecMelee* FindNativeQueuedMelee(NWorld::CCommandExecute* executor)
{
    if(auto* melee=dynamic_cast<NWorld::CExecMelee*>(executor))return melee;
    auto* queue=dynamic_cast<NWorld::CSimpleExecQueue*>(executor);if(!queue)return nullptr;
    vector<CObjectBase*> objects;CStructureNetworkContext graph;
    graph.include=[&](CObjectBase* object){objects.push_back(object);return false;};
    CMemoryStream traversal;{CStructureSaver writer(traversal,CStructureSaver::WRITE,&graph);queue->NWorld::CSimpleExecQueue::operator&(writer);}
    CStructureNetworkContext refs;refs.externalReferences=true;refs.nextID=100;refs.include=[](CObjectBase*){return false;};
    for(auto* object:objects)if(!refs.ids.count(object)){const auto id=refs.nextID++;refs.ids[object]=id;refs.refs[id]=object;}
    CMemoryStream fields;{CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);queue->NWorld::CSimpleExecQueue::operator&(writer);}fields.Seek(0);
    list<CObj<NWorld::CCommandExecute>> children;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&children);}
    for(const auto& child:children)if(auto* melee=FindNativeQueuedMelee(child))return melee;
    return nullptr;
}

static void NativeObjectMeleeRoute(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<7,"unknown native object-melee routing mode");
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsEmptyPK() && !unit->IsWearingPK() && !unit->HasCommand() && !unit->IsPerformingAction() && unit->GetUnitRPG()->CanMove()){actor=unit;break;}
    Require(actor,"object-melee route fixture lacks an ordinary available native actor");
    auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous;
    const int slot=inventory->GetActiveSlot();if(slot>=0)previous=inventory->TakeOff(static_cast<NDb::ESlot>(slot));
    CObj<NRPG::CMeleeWeaponItem> blade;
    if(mode==6) {
        NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> rows(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
        while(rows.MoveNext()){auto* w=rows.Get();if(!w->bThrowing && w->pWeaponType && w->pItem && w->pItem->pModel && (!record || w->GetRecordID()<record->GetRecordID()))record=w;}
        Require(record,"object-melee route fixture lacks an authored nonthrowing knife");blade=new NRPG::CMeleeWeaponItem(record);blade->nQuantity=1;
        if(inventory->Get(NDb::SLOT_1))previous=inventory->TakeOff(NDb::SLOT_1);
        Require(inventory->Equip(NDb::SLOT_1,blade) && inventory->Activate(NDb::SLOT_1),"object-melee route fixture cannot activate its native knife");
    }
    Require(NWorld::GetActionType(actor)==(mode==6?NWorld::AT_KNIFE:NWorld::AT_MELEE),"object-melee route fixture selected a different native action type");
    auto* map=world->GetAIMap();auto* network=world->GetPathNetwork();NWorld::CWindowDoor* door=nullptr;CVec3 targetPoint;
    CObj<NWorld::CCommandExecute> root;NWorld::EUnitCommandResult result=NWorld::UCR_OK;int attempts=0;
    for(const auto& entry:world->GetObjectsForHarness()) {
        auto* candidate=dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr());CVec3 point;
        if(!candidate || candidate->IsBroken() || !map->GetUnitHLPos(&point,map->GetHull(candidate),-1))continue;
        vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(point,2.f),&places);
        for(auto place:places) {
            place.SetPose(NAI::CM_STAND);place.SetDirection(NRPG::GetShootDirection(network,place,point));
            if(!network->IsPassable(place) || network->IsLocked(place,false))continue;
            auto at=actor->GetPosition();at.pos=NAI::SPosition(place,network);const float distance=fabs(at.GetCP()-point);
            if(distance<.3f || distance>1.4f)continue;
            world->GetTime()->Set(world->GetTime()->GetValue()+1);actor->SetPosition(at);actor->animator.AlignTime();actor->animator.PlaceUnit(at);
            actor->animator.SetActiveItem(false);actor->SetUndrawItem(true);actor->SetWalkWithoutWeapon(true);MarkNewDGFrame();map->Sync();
            root=NWorld::CreateActionExecutor(actor,new NWorld::CCmdShootObject(candidate,0),&result);++attempts;
            if(root && result==NWorld::UCR_OK && FindNativeQueuedMelee(root)){door=candidate;targetPoint=point;break;}
            if(attempts>=100)break;
        }
        if(door || attempts>=100)break;
    }
    Require(door && root,"object-melee route fixture lacks a native authored door with a valid close approach");
    const auto position=actor->GetPosition();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),extra=mode==4?0:7;
    const bool active=actor->animator.IsActiveItem();auto random=GlobalGameRandom();
    CObj<NWorld::CCmd> command;
    if(mode==4)command=new NWorld::CCmdShootTile(targetPoint);
    else command=new NWorld::CCmdShootObject(mode==5?nullptr:static_cast<CObjectBase*>(door),extra);
    root=NWorld::CreateActionExecutor(actor,command,&result);auto* melee=FindNativeQueuedMelee(root);
    Require(root && melee && result==(mode==5?NWorld::UCR_NO_TARGET:NWorld::UCR_OK),"native object-melee routing changed its existing availability control");
    const int type=pSSClasses->GetObjectTypeID(melee),expectedType=mode==4?0x00422171:mode==5?0x00422172:0x01353130;
    const int base=actor->GetActionAP(NRPG::AC_MELEE),start=melee->GetStartAP();
    std::printf("OBJECT-MELEE-ROUTE mode=%d type=%08x expectedType=%08x action=%d startAP=%d expectedStart=%d status=%d attempts=%d\n",mode,type,expectedType,int(NWorld::GetActionType(actor)),start,base+extra,result,attempts);std::fflush(stdout);
    if(mode==1)Require(start==base+extra,"native object-melee command discarded requested extra AP in a point conversion");
    else if(mode==3){melee->Start();Require(actor->GetAP()==ap-base-extra,"native object-melee Start did not spend its requested extra AP");}
    else Require(type==expectedType,"native object-melee command still uses a point executor instead of its original saved type");
    if(mode==2) {
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(melee,&refs);CPtr<CObjectBase> target;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&target);}
        Require(!target,"fresh SS object-melee retained a Sentinels target filter");
        struct BaseState {CVec3 point;int extra=0;int operator&(CStructureSaver& f){f.Add(2,&point);f.Add(3,&extra);return 0;}} baseState;
        CMemoryStream baseFields;{CStructureSaver writer(baseFields,CStructureSaver::WRITE,&refs);melee->NWorld::CExecMelee::operator&(writer);}baseFields.Seek(0);
        {CStructureSaver reader(baseFields,CStructureSaver::READ,&refs);baseState.operator&(reader);}
        Require(std::isfinite(baseState.point.x) && std::isfinite(baseState.point.y) && std::isfinite(baseState.point.z) && fabs2(baseState.point-targetPoint)<.00000001f && baseState.extra==extra,
            "fresh SS object-melee lost the existing native hull point or saved extra AP");
        CObj<NWorld::CExecMelee> restored=dynamic_cast<NWorld::CExecMelee*>(pSSClasses->CreateObject(type));Require(restored,"native fresh object-melee cannot roundtrip");fields.Seek(0);
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);}
        Require(restored->GetStartAP()==base+extra && restored->CanDoIt(position)==NWorld::UCR_OK,"native object-melee roundtrip lost its finite remembered point or extra AP");
    }
    Require(actor->GetAP()==ap-(mode==3?base+extra:0) && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && !actor->HasCommand(),"object-melee construction changed unexpected AP, health, position or orders");
    if(mode!=3)Require(actor->animator.IsActiveItem()==active && random.Get()==GlobalGameRandom().Get(),"object-melee availability query executed item preparation or changed RNG");
    Require(!blade || blade->nQuantity==1,"native knife melee routing spent the held knife early");
}

static void NativeObjectShot(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);auto* map=world->GetAIMap();auto* network=world->GetPathNetwork();
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;NRPG::CWeaponItem* weapon=nullptr;
    for(const auto& unit:units) {
        if(!IsValid(unit) || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK() || unit->HasCommand() || unit->IsPerformingAction() || unit->GetState()==NWorld::CUnit::ST_MACHINE_GUN || !unit->GetUnitRPG()->CanMove())continue;
        auto* candidate=dynamic_cast<NRPG::CWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        if(candidate && candidate->GetAmmoQuantity()>1 && candidate->CanReload(unit->GetUnitRPG()->GetInventory()) && candidate->SetShootMode(NDb::SM_Snap)){actor=unit.GetPtr();weapon=candidate;break;}
    }
    Require(actor && weapon,"object shot fixture has no ordinary authored loaded firearm with native reload supply");
    CObj<NWorld::CCommandExecute> executor;NWorld::CWindowDoor* door=nullptr;CVec3 targetPoint;NWorld::EUnitCommandResult result=NWorld::UCR_OK;
    int attempts=0;
    for(const auto& object:world->GetObjectsForHarness()) {
        auto* candidate=dynamic_cast<NWorld::CWindowDoor*>(object.GetPtr());CVec3 point;
        if(!candidate || candidate->IsBroken() || !map->GetUnitHLPos(&point,map->GetHull(candidate),-1))continue;
        vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(point,4.f),&places);
        for(auto place:places) {
            place.SetPose(NAI::CM_STAND);place.SetDirection(NRPG::GetShootDirection(network,place,point));
            if(!network->IsPassable(place) || network->IsLocked(place,false))continue;
            auto at=actor->GetPosition();at.pos=NAI::SPosition(place,network);const float distance=fabs(at.GetCP()-point);
            if(distance<1.5f || distance>4.f)continue;
            world->GetTime()->Set(world->GetTime()->GetValue()+1);actor->SetPosition(at);actor->animator.AlignTime();actor->animator.PlaceUnit(at);
            actor->animator.SetActiveItem(true);actor->SetUndrawItem(false);actor->SetWalkWithoutWeapon(false);MarkNewDGFrame();map->Sync();
            ++attempts;executor=NWorld::CreateActionExecutor(actor,new NWorld::CCmdShootObject(candidate,0),&result);
            if(executor && (result==NWorld::UCR_OK || result==NWorld::UCR_NO_TARGET) && FindNativeObjectOrPointShot(executor)){door=candidate;targetPoint=point;break;}
            if(attempts>=80)break;
        }
        if(door || attempts>=80)break;
    }
    Require(door && executor,"object shot fixture has no real authored door with an available native firearm approach");
    if(mode==8) {
        auto away=actor->GetPosition();away.pos.p.SetDirection((away.pos.p.GetDirection()+4)&7);
        world->GetTime()->Set(world->GetTime()->GetValue()+1);actor->SetPosition(away);actor->animator.AlignTime();actor->animator.PlaceUnit(away);
        actor->animator.SetActiveItem(false);actor->SetUndrawItem(true);actor->SetWalkWithoutWeapon(true);MarkNewDGFrame();map->Sync();
    }
    const int ap=actor->GetAP(),vp=actor->GetUnitRPG()->GetTotalVP(),ammo=weapon->GetAmmoQuantity();const auto position=actor->GetPosition();auto random=GlobalGameRandom();
    const int extra=mode==1?5:0;CObj<NWorld::CCmd> command;
    if(mode==3)command=new NWorld::CCmdShootTile(targetPoint);
    else command=new NWorld::CCmdShootObject(mode==4?nullptr:static_cast<CObjectBase*>(door),extra);
    if(mode==6){dynamic_cast<NWorld::CCmdShootObject*>(command.GetPtr())->bCanBeReplacedByReload=true;weapon->DiscardAmmo();}
    const int expectedAmmo=mode==6?0:ammo;
    executor=NWorld::CreateActionExecutor(actor,command,&result);auto* shot=FindNativeObjectOrPointShot(executor);
    const int type=shot?pSSClasses->GetObjectTypeID(shot):0;
    std::printf("native object shot factory: mode=%d attempts=%d type=%08x result=%d extra=%d AP=%d ammo=%d\n",mode,attempts,type,int(result),extra,actor->GetAP(),weapon->GetAmmoQuantity());std::fflush(stdout);
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==vp && actor->GetPosition()==position && weapon->GetAmmoQuantity()==expectedAmmo && !actor->HasCommand(),"object shot preview changed native AP/health/position/ammo/orders");
    if(mode!=6)Require(random.Get()==GlobalGameRandom().Get(),"object shot preview consumed native gameplay RNG");
    if(mode==7 || mode==8) {
        std::printf("native object preparation status: mode=%d result=%d expected=%d\n",mode,int(result),int(NWorld::UCR_OK));std::fflush(stdout);
        Require(executor && shot && result==NWorld::UCR_OK,"native object approach preparation retained a missing-target status for a real object");
        if(mode==8) {
            auto* move=dynamic_cast<NWorld::IExecMove*>(executor.GetPtr());Require(move,"holstered object attack has no native preparation queue");
            NAI::SPathPlace destination;NAI::EFindPathParams params;NWorld::ENeedActiveItem active;move->GetDesiredPlace(&destination,&params,&active);
            const auto aimed=NRPG::GetShootDirection(network,position.pos.p,targetPoint);
            std::printf("native object preparation destination: beforeDir=%d desiredDir=%d targetDir=%d active=%d sameCell=%d\n",int(position.pos.p.GetDirection()),int(destination.GetDirection()),int(aimed),int(active),int(destination.GetX()==position.pos.p.GetX() && destination.GetY()==position.pos.p.GetY() && destination.GetLayer()==position.pos.p.GetLayer()));std::fflush(stdout);
            Require(destination.GetX()==position.pos.p.GetX() && destination.GetY()==position.pos.p.GetY() && destination.GetLayer()==position.pos.p.GetLayer() && destination.GetDirection()!=position.pos.p.GetDirection() && active==NWorld::ITEM_ACTIVE,"native object preparation lost its facing/draw constraint or walked away from the firing cell");
            Require(!actor->animator.IsActiveItem() && actor->GetUndrawItem() && actor->IsWalkingWithoutWeapon(),"object preparation query executed its weapon activation early");
        }
        return;
    }
    if(mode==9 || mode==10) {
        Require(executor && shot && result==NWorld::UCR_OK,"object lifetime fixture lost its ordinary native attack");
        struct State {
            NRPG::SAttackRayInfo ray;CVec3 remembered;
            int operator&(CStructureSaver& saver){saver.Add(8,&ray);saver.Add(9,&remembered);return 0;}
        };
        auto read=[](NWorld::CExecShoot* action) {
            CStructureNetworkContext refs;auto fields=BorrowedObjectFields(action,&refs);State result;
            {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,&result);}return result;
        };
        shot->Start();const auto started=read(shot);const int startedAP=actor->GetAP(),startedAmmo=weapon->GetAmmoQuantity();
        Require(fabs2(started.remembered-targetPoint)<.0001f,"native object shot did not remember its original real hull center");
        CObj<NWorld::CWindowDoor> retained=door;world->PlaceObjectInPocket(door);MarkNewDGFrame();map->Sync();CVec3 absent(7,8,9);
        Require(IsValid(retained) && world->GetPocket()->IsObjectInPocket(door) && !map->GetUnitHLPos(&absent,map->GetHull(door),-1) && fabs2(absent)==0,"native pocket lifecycle did not leave a valid actual object without a physical hull");
        auto queryRandom=GlobalGameRandom();const int missing=int(shot->CanDoIt(actor->GetPosition()));
        Require(actor->GetAP()==startedAP && weapon->GetAmmoQuantity()==startedAmmo && actor->GetPosition()==position && queryRandom.Get()==GlobalGameRandom().Get(),"missing-hull native eligibility query changed gameplay or RNG");
        CObj<NWorld::CExecShoot> restored;NWorld::CExecShoot* selected=shot;
        if(mode==10) {
            CStructureNetworkContext refs;auto fields=BorrowedObjectFields(shot,&refs);
            restored=dynamic_cast<NWorld::CExecShoot*>(pSSClasses->CreateObject(type));Require(restored,"native missing-hull shot factory failed");
            {CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);}selected=restored.GetPtr();
        }
        selected->SelectRay();const auto fallback=read(selected);CVec3 aim=started.remembered-fallback.ray.vOrigin;Normalize(&aim);
        const float alignment=aim*fallback.ray.vDir;
        Require(fabs2(fallback.remembered-started.remembered)<.0001f && std::isfinite(alignment) && alignment>.8f && fallback.ray.pIgnore.GetPtr()==actor,"missing-hull object shot did not fall back toward its remembered native point");
        Require(actor->GetAP()==startedAP && weapon->GetAmmoQuantity()==startedAmmo-1 && actor->GetUnitRPG()->GetTotalVP()==vp && actor->GetPosition()==position && !actor->HasCommand(),"native remembered-point fallback changed AP/health/position/orders or spent an unexpected number of rounds");
        world->RestoreObjectFromPocket(door);MarkNewDGFrame();map->Sync();CVec3 returned;const bool hasHull=map->GetUnitHLPos(&returned,map->GetHull(door),-1);
        const int available=int(selected->CanDoIt(actor->GetPosition()));
        std::printf("native object shot lifetime: mode=%d missingQuery=%d rememberedDistance=%g alignment=%g restoredHull=%d restoredQuery=%d AP=%d ammo=%d\n",mode,missing,double(std::sqrt(fabs2(started.remembered-targetPoint))),double(alignment),int(hasHull),available,actor->GetAP(),weapon->GetAmmoQuantity());std::fflush(stdout);
        Require(!world->GetPocket()->IsObjectInPocket(door) && hasHull && fabs2(returned-targetPoint)<.0001f && available==NWorld::UCR_OK,"native object restore did not recover its original hull and shot eligibility");return;
    }
    if(mode==6){Require(executor && result==NWorld::UCR_OK_RELOAD && !shot,"object shot lost the native empty-clip reload fallback");return;}
    Require(executor && shot && result==(mode==4?NWorld::UCR_NO_TARGET:NWorld::UCR_OK),"object shot factory changed a native availability control");
    if(mode==3 || mode==4){Require(type==int(mode==3?0x00122173:0x00422170),"point/missing-target attack lost its existing executor kind");return;}
    if(mode==0){Require(type==int(0x01142170),"world object attack discarded its native object identity by becoming a point shot");return;}
    if(mode==1) {
        const int base=actor->GetActionAP(actor->animator.IsAiming()?NRPG::AC_SHOOT:NRPG::AC_PREPARE_AND_SHOOT);
        std::printf("native object shot extra AP: start=%d base=%d expectedExtra=%d\n",shot->GetStartAP(),base,extra);std::fflush(stdout);
        Require(shot->GetStartAP()==base+extra,"world object attack discarded requested extra AP in its point conversion");return;
    }
    if(mode==5){const int ignored=int(shot->CanDoIt(actor->GetPosition(),true));std::printf("native object shot ignore: result=%d expected=%d\n",ignored,int(NWorld::UCR_NO_TARGET));std::fflush(stdout);Require(ignored==NWorld::UCR_NO_TARGET,"world object attack ignored the native target-free validation request");return;}
    shot->Start();
    CStructureNetworkContext references;auto fields=BorrowedObjectFields(shot,&references);CPtr<CObjectBase> retained;
    {CStructureSaver reader(fields,CStructureSaver::READ,&references);reader.Add(2,&retained);}
    std::printf("native object shot retained target: matches=%d type=%08x AP=%d ammo=%d\n",int(retained.GetPtr()==door),type,actor->GetAP(),weapon->GetAmmoQuantity());std::fflush(stdout);
    Require(retained.GetPtr()==door,"started object shot failed to retain its real door target in the native save fields");
    CObj<NWorld::CExecShoot> restored=dynamic_cast<NWorld::CExecShoot*>(pSSClasses->CreateObject(type));Require(restored,"object shot native factory could not restore its executor");fields.Seek(0);
    {CStructureSaver reader(fields,CStructureSaver::READ,&references);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);}
    CStructureNetworkContext roundRefs;auto round=BorrowedObjectFields(restored,&roundRefs);CPtr<CObjectBase> roundTarget;
    {CStructureSaver reader(round,CStructureSaver::READ,&roundRefs);reader.Add(2,&roundTarget);}
    Require(roundTarget==retained && restored->CanDoIt(actor->GetPosition())==NWorld::UCR_OK && restored->CanDoIt(actor->GetPosition(),true)==NWorld::UCR_NO_TARGET,"native object executor roundtrip lost target or validation semantics");
}

static void MineLateReach(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);auto* network=world->GetPathNetwork();
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;CObj<NRPG::CToolItem> tool;
    for(const auto& unit:units) {
        if(!IsValid(unit) || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK() || unit->HasCommand() || unit->IsPerformingAction() || unit->GetState()==NWorld::CUnit::ST_MACHINE_GUN || !unit->GetPosition().pos.p.IsIntegral())continue;
        CDBIterator<NDb::CRPGTool> tools(*NDatabase::GetTable<NDb::CRPGTool>());
        while(tools.MoveNext()) {
            if(!IsValid(tools.Get()->pItem) || !tools.Get()->bCanUseForMineCleaning)continue;
            CObj<NRPG::CToolItem> candidate=new NRPG::CToolItem(tools.Get());
            if(candidate->CanBeUsed(unit->GetUnitRPG()->GetRPGUnit())){actor=unit.GetPtr();tool=candidate;break;}
        }
        if(actor)break;
    }
    CObj<NWorld::CUnitServer> spawned;
    if(!actor) {
        NWorld::CUnitServer* anchor=nullptr;
        for(const auto& unit:units)if(IsValid(unit) && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->IsAIUnit()){anchor=unit.GetPtr();break;}
        Require(anchor,"late mine fixture has no human player for its authored sapper");
        CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
        while(people.MoveNext() && !actor) {
            auto* persona=people.Get();if(!IsValid(persona->pModel) || !IsValid(persona->pClass) || !IsValid(persona->pBaseValue) || !IsValid(persona->pName) || persona->pClass->skills[NDb::ST_LEVEL]<=0 || IsValid(persona->pPanzerklein) || IsValid(persona->pDefaultWearsPanzerklein))continue;
            CObj<NRPG::IUnitMission> rpg=NRPG::CreateUnit(persona);
            CDBIterator<NDb::CRPGTool> tools(*NDatabase::GetTable<NDb::CRPGTool>());
            while(tools.MoveNext()) {
                if(!IsValid(tools.Get()->pItem) || !tools.Get()->bCanUseForMineCleaning)continue;
                CObj<NRPG::CToolItem> candidate=new NRPG::CToolItem(tools.Get());
                if(!candidate->CanBeUsed(rpg->GetRPGUnit()))continue;
                spawned=world->AddUnitInGame(anchor->GetPosition().pos.p,rpg,anchor->GetTBSPlayer());
                Require(spawned && spawned->CanFight() && !spawned->IsWearingPK() && !spawned->IsEmptyPK(),"authored sapper native placement failed");
                actor=spawned.GetPtr();tool=candidate;break;
            }
        }
    }
    Require(actor && IsValid(tool),"late mine fixture has no authored actor/tool pair satisfying native skill, perk and charge gates");
    std::printf("native late mine actor: persona=%d engineering=%d tool=%d needed=%d charges=%d spawned=%d\n",actor->GetUnitRPG()->GetRPGUnit()->GetPers()->GetRecordID(),int(actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_ENGINEERING)),tool->GetDBItemInfo()->GetRecordID(),tool->GetDBItemInfo()->nNeededEngSkill,tool->GetIncQuantity(),int(IsValid(spawned)));std::fflush(stdout);
    auto* inventory=actor->GetUnitRPG()->GetInventory();
    CObj<NRPG::IInventoryItem> original=inventory->Get(NDb::SLOT_1)?inventory->TakeOff(NDb::SLOT_1):nullptr;
    Require(inventory->Equip(NDb::SLOT_1,tool) && inventory->Activate(NDb::SLOT_1),"late mine fixture cannot equip/activate its usable native clearing tool");
    NDb::CRPGMine* record=nullptr;CDBIterator<NDb::CRPGMine> mines(*NDatabase::GetTable<NDb::CRPGMine>());
    while(mines.MoveNext())if(IsValid(mines.Get()->pItem) && IsValid(mines.Get()->pItem->pModel) && IsValid(mines.Get()->pExplosion)){record=mines.Get();break;}
    Require(record,"late mine fixture has no authored mine model/explosion");
    const auto nearPosition=actor->GetPosition();const auto center=nearPosition.GetCP()+CVec3(.6f,0,0);
    CObj<NWorld::CMine> mine=new NWorld::CMine(world,center,record,actor->GetUnitRPG()->GetMineDC(record),nearPosition.pos.GetFloor(),actor,0);
    actor->AddToVisibleTraps(mine);world->UpdateVisible(true);
    Require(actor->GetTBSPlayer()->CanSeeObject(mine) && actor->GetTBSPlayer()->CanSeeTrap(mine),"late mine fixture did not discover the real mine through native unit/player visibility");
    CObj<NWorld::CExecDisarmMine> executor=new NWorld::CExecDisarmMine(actor,mine);
    const int nearResult=int(executor->CanDoIt(nearPosition,false));
    Require(nearResult==NWorld::UCR_OK,"late mine nearPosition control is blocked before the reach check");
    vector<NAI::SPathPlace> points;network->GetNearPlaces(SSphere(center,3.f),&points);
    auto outer=nearPosition,farPosition=nearPosition;bool haveOuter=false,haveFar=false;
    for(const auto& point:points) {
        if(!point.IsIntegral() || point.GetLayer()!=nearPosition.pos.p.GetLayer() || !network->IsNativePassable(point))continue;
        auto candidate=nearPosition;candidate.pos=NAI::SPosition(point,network);const auto cp=candidate.GetCP();
        if(std::fabs(cp.z-center.z)>.2f)continue;
        const float dx=cp.x-center.x,dy=cp.y-center.y,dist=std::sqrt(dx*dx+dy*dy);
        if(std::fabs(dx)<=.5f && std::fabs(dy)<=.5f)continue;
        if(!haveOuter && !NWorld::IsWithinHumanReach(cp,center,NWorld::F_HEAL_DISTANCE) && NWorld::IsWithinHumanReach(cp,center,1.f)){outer=candidate;haveOuter=true;}
        if(!haveFar && dist>1.2f && dist<2.5f){farPosition=candidate;haveFar=true;}
    }
    Require(haveOuter && haveFar,"late mine fixture has no native passable original-radius boundary and distant control positions");
    auto from=mode==0?nearPosition:mode==1?outer:farPosition;
    CObj<NRPG::IInventoryItem> removed;
    CObj<NWorld::CMine> unknown;
    if(mode==3)removed=inventory->TakeOff(NDb::SLOT_1);
    if(mode==4) {
        unknown=new NWorld::CMine(world,center,record,actor->GetUnitRPG()->GetMineDC(record),nearPosition.pos.GetFloor(),actor,0);
        Require(!actor->GetTBSPlayer()->CanSeeTrap(unknown),"late mine unknown-target control was already discovered");
        executor=new NWorld::CExecDisarmMine(actor,unknown);
    }
    if(mode==5)executor=new NWorld::CExecDisarmMine(actor,nullptr);
    if(mode==7){actor->SetPosition(farPosition);from=actor->GetPosition();}
    const auto position=actor->GetPosition();const int ap=actor->GetAP(),vp=actor->GetUnitRPG()->GetTotalVP(),charges=tool->GetIncQuantity();
    const auto active=inventory->GetActive();const bool command=actor->HasCommand(),action=world->IsAction();auto random=GlobalGameRandom();
    const int expected=mode==0?NWorld::UCR_OK:(mode==3 || mode==4)?NWorld::UCR_GENERAL_FAILURE:(mode==5 || mode==6)?NWorld::UCR_NO_TARGET:NWorld::UCR_TARGET_OUT_OF_RANGE;
    const int result=int(executor->CanDoIt(from,mode==6));const auto cp=from.GetCP();const float dx=cp.x-center.x,dy=cp.y-center.y;
    vector<CVec3> locations(1,center);vector<CPtr<NWorld::CMine>> tracked;
    Require(world->GetMineTracker()->GetMines(locations,&tracked) && std::find(tracked.begin(),tracked.end(),CPtr<NWorld::CMine>(mine.GetPtr()))!=tracked.end(),"late mine eligibility query removed or disarmed the real mine");
    std::printf("native late mine reach: mode=%d nearPosition=%d distance=%g originalReach=%g actorDistance=%g result=%d expected=%d AP=%d charges=%d\n",mode,nearResult,double(std::sqrt(dx*dx+dy*dy)),double(NWorld::F_HEAL_DISTANCE),double(std::sqrt(std::pow(position.GetCP().x-center.x,2.f)+std::pow(position.GetCP().y-center.y,2.f))),result,expected,actor->GetAP(),tool->GetIncQuantity());std::fflush(stdout);
    Require(actor->GetPosition()==position && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==vp && tool->GetIncQuantity()==charges && inventory->GetActive()==active && actor->HasCommand()==command && world->IsAction()==action && random.Get()==GlobalGameRandom().Get(),"late mine eligibility query changed native gameplay, equipment, orders or RNG");
    Require(result==expected,"late disarm validation accepted an out-of-original-range mine or changed an earlier gate");
}

static void MineApproachStatus(const char* databasePath,const char* resourcePath,int mode)
{
    auto world=FixtureWorld(databasePath,resourcePath,true);auto* network=world->GetPathNetwork();
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;NRPG::CWeaponItem* weapon=nullptr;
    for(const auto& unit:units) {
        if(!IsValid(unit) || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK() || unit->HasCommand() || unit->IsPerformingAction() || unit->GetState()==NWorld::CUnit::ST_MACHINE_GUN)continue;
        auto* active=dynamic_cast<NRPG::CWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        if(active && unit->GetPosition().pos.p.IsIntegral()){actor=unit.GetPtr();weapon=active;break;}
    }
    Require(actor && weapon,"mine approach fixture has no ordinary armed native actor without an active clearing tool");
    NDb::CRPGMine* record=nullptr;CDBIterator<NDb::CRPGMine> records(*NDatabase::GetTable<NDb::CRPGMine>());
    while(records.MoveNext())if(IsValid(records.Get()->pItem) && IsValid(records.Get()->pItem->pModel) && IsValid(records.Get()->pExplosion)){record=records.Get();break;}
    Require(record,"mine approach fixture has no authored mine model/explosion record");
    vector<NAI::SPathPlace> nearby;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),8.f),&nearby);
    std::map<unsigned,NAI::SPathPlace> cells;
    for(auto point:nearby)if(point.IsIntegral())cells[(unsigned(point.GetLayer())<<16)|(unsigned(point.GetY())<<8)|point.GetX()]=point;
    NAI::SPathPlace anchor;CVec3 center;float minimumOutside=0;int extendedCandidates=0;bool found=false;
    auto measure=[&](const CVec3& target,int* extended) {
        vector<NAI::SPathPlace> points;network->GetNearPlaces(SSphere(target,5.f),&points);float minimum=1e9f;*extended=0;
        for(auto point:points) {
            const auto cp=NAI::SPosition(point,network).GetCP();const float dx=cp.x-target.x,dy=cp.y-target.y;
            if(std::fabs(dx)<=.5f && std::fabs(dy)<=.5f)continue;
            minimum=(std::min)(minimum,std::sqrt(dx*dx+dy*dy));
            if(network->IsNativePassable(point) && NWorld::IsWithinHumanReach(cp,target,1.f))++*extended;
        }
        return minimum;
    };
    for(const auto& entry:cells) {
        const auto point=entry.second;if(point.GetX()>=254 || point.GetY()>=254)continue;
        auto x=cells.find(entry.first+1),y=cells.find(entry.first+256),xy=cells.find(entry.first+257);
        if(x==cells.end() || y==cells.end() || xy==cells.end() || !network->IsNativePassable(point) || !network->IsNativePassable(x->second) || !network->IsNativePassable(y->second) || !network->IsNativePassable(xy->second))continue;
        const auto p0=NAI::SPosition(point,network).GetCP(),p1=NAI::SPosition(x->second,network).GetCP(),p2=NAI::SPosition(y->second,network).GetCP(),p3=NAI::SPosition(xy->second,network).GetCP();
        const float minZ=(std::min)((std::min)(p0.z,p1.z),(std::min)(p2.z,p3.z)),maxZ=(std::max)((std::max)(p0.z,p1.z),(std::max)(p2.z,p3.z));
        if(maxZ-minZ>.2f)continue;
        const auto midpoint=(p0+p1+p2+p3)*.25f;int extended=0;const float minimum=measure(midpoint,&extended);
        if(minimum>.8001f && minimum<.9999f && extended>0){anchor=point;center=midpoint;minimumOutside=minimum;extendedCandidates=extended;found=true;break;}
    }
    Require(found,"native mine fixture has no real off-grid cell center whose safe approach distance lies strictly between original0.8 and upstream1.0");
    if(mode==1){center=NAI::SPosition(anchor,network).GetCP()+CVec3(.6f,0,0);minimumOutside=measure(center,&extendedCandidates);Require(minimumOutside<.8f,"reachable control has no original-radius grid approach");}
    CObj<NWorld::CMine> mine=new NWorld::CMine(world,center,record,actor->GetUnitRPG()->GetMineDC(record),NAI::SPosition(anchor,network).GetFloor(),actor,0);
    CObj<NWorld::CCmdUntrapObject> command=new NWorld::CCmdUntrapObject(mode==2?nullptr:mode==3?static_cast<CObjectBase*>(actor):static_cast<CObjectBase*>(mine.GetPtr()));
    const auto position=actor->GetPosition();const int ap=actor->GetAP(),vp=actor->GetUnitRPG()->GetTotalVP(),ammo=weapon->GetAmmoQuantity();
    const bool hadCommand=actor->HasCommand(),hadAction=world->IsAction();auto random=GlobalGameRandom();NWorld::EUnitCommandResult direct=NWorld::UCR_OK;
    CObj<NWorld::CCommandExecute> executor=NWorld::CreateActionExecutor(actor,command,&direct);const int publicResult=int(actor->CanDo(command));
    const int expected=int(mode==0?NWorld::UCR_PATH_NOT_FOUND:mode==1?NWorld::UCR_GENERAL_FAILURE:mode==2?NWorld::UCR_UNAVAILABLE:NWorld::UCR_INVALID_COMMAND);
    vector<CVec3> positions(1,center);vector<CPtr<NWorld::CMine>> tracked;
    Require(world->GetMineTracker()->GetMines(positions,&tracked) && std::find(tracked.begin(),tracked.end(),CPtr<NWorld::CMine>(mine.GetPtr()))!=tracked.end(),"mine approach query disarmed or removed the native mine");
    std::printf("native mine approach status: mode=%d minimumOutside=%g extendedCandidates=%d originalReach=%g executor=%d direct=%d public=%d expected=%d AP=%d\n",mode,double(minimumOutside),extendedCandidates,double(NWorld::F_HEAL_DISTANCE),int(IsValid(executor)),int(direct),publicResult,expected,actor->GetAP());std::fflush(stdout);
    Require(actor->GetPosition()==position && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==vp && weapon->GetAmmoQuantity()==ammo && actor->GetUnitRPG()->GetInventory()->GetActive()==weapon && actor->HasCommand()==hadCommand && world->IsAction()==hadAction && random.Get()==GlobalGameRandom().Get(),"mine approach query changed native gameplay, equipment, orders or RNG");
    Require(!executor && int(direct)==expected && publicResult==expected,"native mine approach returned success/general failure instead of the actual no-path reason, or changed a control result");
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
        Require(!executor->TimeLabelReached() && executor->GetState() == NWorld::CCommandExecute::RUNNING,
            "the native throwing-knife label did not retain the executor until animation completion");
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
        executor->AnimationFinished();
        Require(executor->GetState() == NWorld::CCommandExecute::FINISHED,
            "the actual knife animation callback did not complete the executor");
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
    CObjectBase* AddPointLight(const CVec3&,const CVec3&,float,bool,bool) override { return nullptr; }
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

// The world, terrain/grass trackers and CSetRender dispatch run unchanged;
// this scene sink records native room metadata without creating GPU resources.
class GrassFloorView: public HeadStateView
{
    OBJECT_NOCOPY_METHODS(GrassFloorView);
public:
    vector<NGScene::SRoomInfo> grassRooms;vector<int> terrainFloors;vector<CObj<NGScene::CGrassAnimator>> animators;vector<int> textures;
    CObjectBase* CreateGrassSector(NGScene::CGrassAnimator* animator,NDb::CTexture* texture,CFuncBase<SFBTransform>* pose,const SBound& bound,const NGScene::SRoomInfo& room) override {Require(animator && texture && pose,"native grass dispatch omitted its actual animator/texture/pose");grassRooms.push_back(room);animators.push_back(animator);textures.push_back(texture->GetRecordID());return HeadStateView::CreateGrassSector(animator,texture,pose,bound,room);}
    CObjectBase* CreateTerrainRegion(CFuncBase<STerrainInfo>* info,CVersioningBase* version,const SRandomSeed& seed,const CTRect<int>& region,const list<CObj<CPtrFuncBase<CTerrainPart>>>& parts,NGScene::CGrassTracker* tracker,const NGScene::SFullRoomInfo& room) override {terrainFloors.push_back(room.room.nFloor);return HeadStateView::CreateTerrainRegion(info,version,seed,region,parts,tracker,room);}
};
static void TerrainGrassFloor(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=1,"unknown native terrain grass floor mode");const int variant=mode==0?810:4517;CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game,variant);CDGPtr<CFuncBase<STerrainInfo>> terrain=world->GetTerrainInfo();terrain.Refresh();const int layers=int(terrain->GetValue().grass.size());std::printf("native grass input mode=%d variant=%d terrainLayers=%d\n",mode,variant,layers);std::fflush(stdout);Require(layers>0,"native grass fixture has no authored terrain grass layers; not a floor RED");CObj<GrassFloorView> view=new GrassFloorView;CObj<NRender::IRenderGame> render=NRender::CreateRenderGame(world,view,nullptr);render->UpdateViewWorld(false,world->GetTime()->GetValue(),nullptr,true);
    Require(!view->grassRooms.empty() && !view->terrainFloors.empty(),"native grass fixture never reached actual grass/terrain render dispatch; not a floor RED");int wrong=0;for(const auto& room:view->grassRooms){Require(!room.pGroup,"native ungrouped terrain grass acquired a room light group");wrong+=room.nFloor!=0;}for(int floor:view->terrainFloors)Require(floor==0,"native terrain control unexpectedly lost its existing floor0");Require(NGScene::SRoomInfo().nFloor==-100,"grass fix incorrectly changed the shared default room for unrelated render nodes");std::printf("native terrain grass mode=%d variant=%d layers=%d sectors=%zu wrongFloor=%d firstFloor=%d expected=0 terrainControls=%zu defaultRoom=%d texture=%d\n",mode,variant,layers,view->grassRooms.size(),wrong,int(view->grassRooms.front().nFloor),view->terrainFloors.size(),int(NGScene::SRoomInfo().nFloor),view->textures.front());std::fflush(stdout);Require(wrong==0,"native terrain grass dispatch used always-visible default floor-100 instead of original floor0");
}

static CMemoryStream NativePortraitBaseFields(NUI::CUnitView* view,CStructureNetworkContext* references)
{
    vector<CObjectBase*> referenced;CStructureNetworkContext graph;graph.include=[&](CObjectBase* child){referenced.push_back(child);return false;};
    CMemoryStream traversal;{CStructureSaver writer(traversal,CStructureSaver::WRITE,&graph);view->NUI::CUnitView::operator&(writer);}
    references->externalReferences=true;references->nextID=100;references->include=[](CObjectBase*){return false;};
    for(auto* child:referenced)if(!references->ids.count(child)){const auto id=references->nextID++;references->ids[child]=id;references->refs[id]=child;}
    CMemoryStream fields;{CStructureSaver writer(fields,CStructureSaver::WRITE,references);view->NUI::CUnitView::operator&(writer);}fields.Seek(0);return fields;
}
static vector<NLSHead::CHeadAnimator*> NativePortraitHeadGraph(CObjectBase* root,int* count)
{
    vector<CObjectBase*> pending(1,root);std::set<CObjectBase*> visited;vector<NLSHead::CHeadAnimator*> result;
    for(unsigned i=0;i<pending.size();++i){auto* object=pending[i];if(!object || !visited.insert(object).second)continue;Require(visited.size()<10000,"portrait graph exceeded its bounded read-only observer");
        if(auto* animator=dynamic_cast<NLSHead::CHeadAnimator*>(object))result.push_back(animator);
        CStructureNetworkContext graph;graph.include=[&](CObjectBase* child){if(child && !visited.count(child))pending.push_back(child);return false;};
        CMemoryStream fields;{CStructureSaver writer(fields,CStructureSaver::WRITE,&graph);object->operator&(writer);}
    }
    *count=int(visited.size());return result;
}
static void NativeRecruitmentIdle(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown native recruitment idle mode");const bool recruitment=mode<2;const int person=mode%2?103:54;
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm recruitment facial idle regression");
    CObj<NRPG::CGlobalPlayer> party=NRPG::CreateGlobalPlayer(vector<int>{person});Require(party && party->mercs.size()==1 && party->totalMercs.size()==1,"recruitment idle fixture lacks its actual authored mercenary");
    CObj<NRPG::CUnit> merc=party->mercs.front();Require(merc->GetHead() && merc->GetHead()->pHead,"recruitment idle fixture lacks its actual complex facial head");
    CObj<NUI::CInterface> interface;CPtr<NUI::CUnitView> portrait;
    if(recruitment){CObj<NGame::CICTeamMngMenu> enter=new NGame::CICTeamMngMenu(party,nullptr);enter->Exec();auto* current=NMainLoop::GetCurrentInterfaceForHarness();Require(current,"recruitment idle did not open its actual native menu");
        CStructureNetworkContext references;auto fields=BorrowedObjectFields(current,&references);CObj<NUI::CWindow> menu;
        {CStructureSaver reader(fields,CStructureSaver::READ,&references);reader.Add(6,&interface);reader.Add(7,&menu);}Require(interface && menu,"recruitment idle menu omitted its real interface/window");
        list<CPtr<NUI::CWindow>> pending;menu->GetChildrenList(&pending);while(!pending.empty()){auto child=pending.front();pending.pop_front();if(child->GetWindowID()=="unit_view_0")if(auto* native=dynamic_cast<NUI::CUnitView*>(child.GetPtr()))portrait=native;list<CPtr<NUI::CWindow>> nested;child->GetChildrenList(&nested);pending.insert(pending.end(),nested.begin(),nested.end());}
    }else{interface=new NUI::CInterface(NUI::ICursor::Create());portrait=new NUI::CUnitView(NUI::SWindowInfo(interface,NUI::SPoint(0,0),NUI::SPoint(160,160),"native_facegen_idle",NUI::STYLE_ENABLED|NUI::STYLE_VISIBLE));portrait->SetUnit(merc,NUI::CUnitView::CAMERA_FACEGEN);}
    Require(portrait,"recruitment idle did not find an actual native portrait view");CStructureNetworkContext references;auto fields=NativePortraitBaseFields(portrait,&references);CObj<NRender::IShowUnit> show;
    {CStructureSaver reader(fields,CStructureSaver::READ,&references);reader.Add(8,&show);}Require(show,"recruitment idle native view did not create its actual shown RPG unit");show->Update(0);
    int nodes=0;auto heads=NativePortraitHeadGraph(show,&nodes);Require(heads.size()==1,"recruitment idle observer did not reach exactly one real native head animator");
    const auto expected=recruitment?NLSHead::IDLE_NONE:NLSHead::IDLE_NORMAL;
    std::printf("RECRUITMENT-IDLE mode=%d person=%d recruitment=%d head=%d observed=%d expected=%d animators=%zu graphNodes=%d\n",mode,person,int(recruitment),merc->GetHead()->GetRecordID(),int(heads.front()->GetIdleType()),int(expected),heads.size(),nodes);std::fflush(stdout);
    Require(heads.front()->GetIdleType()==expected,"native recruitment portrait did not suppress facial idle or default FaceGen lost it");
}

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


class DoorFlagsColliderProbe: public NAI::IPrepareCollider
{
    CObjectBase* owner;
public:
    unsigned hulls=0,points=0;int flags=0;
    explicit DoorFlagsColliderProbe(CObjectBase* object):owner(object) {}
    void SetBoundAndResolution(const SBound&,float) override {}
    void AddConvexHull(const NAI::SConvexHull& hull) override {
        if(hull.src.pUserData!=owner)return;
        ++hulls;points+=unsigned(hull.points.size());flags|=hull.src.nTSFlags;
    }
};
static std::pair<bool,bool> SavedDoorFlags(NWorld::CWindowDoor* door)
{
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(door,&refs);bool chest=false,transparent=false;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(8,&chest);reader.Add(9,&transparent);}
    return {chest,transparent};
}
static bool MissingDoorFlagsDefault()
{
    // Only the absent optional-field class default is under test here. A full
    // native world graph is independently round-tripped in the main fixture.
    // Do not clone a live world's intrusive visitor links via borrowed refs.
    CMemoryStream legacy;
    {CStructureSaver writer(legacy,CStructureSaver::WRITE);bool open=false;writer.Add(2,&open);}
    legacy.Seek(0);CObj<NWorld::CWindowDoor> loaded=new NWorld::CWindowDoor;
    {CStructureSaver reader(legacy,CStructureSaver::READ);static_cast<CObjectBase*>(loaded.GetPtr())->operator&(reader);}
    return SavedDoorFlags(loaded)==std::make_pair(false,false);
}
static void DoorCreationFlags(const char* databasePath,const char* resourcePath,int mode)
{
    const bool wantChest=mode==2;const int variant=mode==0?810:4517;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game,variant);
    // Read the actual native authoring pipeline with the SAME seed/relative level
    // into an independent network, then inspect objects born through CreateRandom.
    // No private AddObject access, new test API, fabricated flags or DB edits.
    CObj<NAI::CPathNetwork> reference=new NAI::CPathNetwork;SMapInfo info;
    Require(BuildMap(variant,{},reference,&info,-1,SRandomSeed(123),0),"door flags could not rebuild the native authored input map");
    auto* map=world->GetAIMap();map->Sync();CObj<NWorld::CWindowDoor> door;
    std::pair<bool,bool> expected;bool initialOpen=false;
    unsigned authoredChest=0,authoredOrdinary=0,sameRecord=0,samePosition=0,hasHull=0;
    for(const auto& element:info.items) {
        if(!IsValid(element.pObject) || !IsValid(element.pObject->pDoor))continue;
        if(element.bIsChest)++authoredChest;else ++authoredOrdinary;
        if(element.bIsChest!=wantChest || door)continue;
        for(auto object:world->GetObjectsForHarness()) {
            auto* candidate=dynamic_cast<NWorld::CWindowDoor*>(object.GetPtr());
            if(!candidate || candidate->IsBroken() || candidate->IsMineSet() || candidate->GetDBObjectID()!=element.pObject->nParentID)continue;
            ++sameRecord;const float distance=fabs(candidate->GetPosition().ptPos-element.pos.ptPos);
            if(distance>.001f)continue;++samePosition;
            auto* hull=map->GetHull(candidate);if(!hull)continue;++hasHull;
            if(hasHull<7)std::printf("native door matching: record=%d distance=%g flags=%x expected=%d,%d\n",candidate->GetDBObjectID(),double(distance),NativeHullSource(hull).nTSFlags,int(element.bIsChest),int(element.bIsTransparentIfOpen));
            const int mask=NWorld::TS_VISION|NWorld::TS_PICK|NWorld::TS_PASS_BLOCKER;
            if((NativeHullSource(hull).nTSFlags&mask)!=mask)continue;
            door=candidate;expected={element.bIsChest,element.bIsTransparentIfOpen};initialOpen=element.bOpen;break;
        }
    }
    std::printf("native authored door inventory: variant=%d chest=%u ordinary=%u selected=%d records=%u positions=%u hulls=%u\n",variant,authoredChest,authoredOrdinary,int(bool(door)),sameRecord,samePosition,hasHull);std::fflush(stdout);
    Require(door && expected.first==wantChest && expected.second==wantChest && door->IsOpen()==initialOpen,"native authored map has no matching real ordinary/chest door and creation pose control");
    const int recordID=door->GetDBObjectID();const bool chest=expected.first,transparent=expected.second;
    const auto created=SavedDoorFlags(door);bool valid=created==expected;
    // Pose/vision checks need an unlocked native fixture; preserve lock rules.
    door->LockDoor(false,0,0);
    const bool legacy=MissingDoorFlagsDefault();Require(legacy,"missing old-save door tags no longer default to false");
    auto unitState=[](NWorld::CWorld* w){list<CPtr<NWorld::CUnitServer>> units;w->GetAllUnits(&units);vector<std::tuple<int,int,unsigned>> result;for(auto unit:units)result.emplace_back(unit->GetUnitRPG()->GetTotalVP(),unit->GetAP(),unit->GetPosition().pos.p.GetBits());std::sort(result.begin(),result.end());return result;};
    const auto stats=unitState(world);const auto time=world->GetTime()->GetValue();
    auto check=[&](NWorld::CWorld* w,NWorld::CWindowDoor* d,bool restored){
        auto* ai=w->GetAIMap();ai->Sync();bool ok=SavedDoorFlags(d)==expected;
        for(bool open:{initialOpen,!initialOpen,initialOpen}) {
            d->OpenClose(open,true);ai->Sync();SBound bound;
            Require(d->IsOpen()==open && ai->GetObjectBound(&bound,d),"native door fixture lost its actual pose or geometry bound");
            auto* hull=ai->GetHull(d);Require(hull,"native door fixture has no active-pose hull");const auto source=NativeHullSource(hull);
            DoorFlagsColliderProbe vision(d),pick(d),movement(d);
            ai->PrepareCollider(&vision,bound,.5f,NWorld::TS_VISION,false);
            ai->PrepareCollider(&pick,bound,.5f,NWorld::TS_PICK,false);
            ai->PrepareCollider(&movement,bound,.5f,NWorld::TS_PASS_BLOCKER,false);
            const bool wantVision=!(open && transparent);
            const int state=open?NWorld::TS_STATE_OPEN:NWorld::TS_STATE_CLOSED;
            const bool masks=source.pUserData==d && (source.nTSFlags&(NWorld::TS_STATE_OPEN|NWorld::TS_STATE_CLOSED))==state && (source.nTSFlags&NWorld::TS_DOOR_HULL_VALID) && bool(source.nTSFlags&NWorld::TS_VISION)==wantVision;
            const bool queries=(vision.hulls>0)==wantVision && pick.hulls>0 && pick.points>0 && movement.hulls>0 && movement.points>0;
            std::printf("native door creation: mode=%d restored=%d open=%d chest=%d transparent=%d flags=%x vision=%u pick=%u move=%u matched=%d\n",mode,int(restored),int(open),int(SavedDoorFlags(d).first),int(SavedDoorFlags(d).second),source.nTSFlags,vision.hulls,pick.hulls,movement.hulls,int(masks && queries));
            ok&=masks && queries;
        }
        Require(unitState(w)==stats && w->GetTime()->GetValue()==time,"door pose/query/save checks changed native actor HP/AP/positions or time");return ok;
    };
    valid=check(world,door,false) && valid;
    CMemoryStream snapshot;
    {CStructureSaver writer(snapshot,CStructureSaver::WRITE);writer.Add(1,&game);writer.Add(2,&world);writer.Add(3,&door);}
    snapshot.Seek(0);CObj<NRPG::CGlobalGame> loadedGame;CObj<NWorld::CWorld> loadedWorld;CObj<NWorld::CWindowDoor> loadedDoor;
    {CStructureSaver reader(snapshot,CStructureSaver::READ);reader.Add(1,&loadedGame);reader.Add(2,&loadedWorld);reader.Add(3,&loadedDoor);}
    Require(loadedGame && loadedWorld && loadedDoor && loadedWorld.GetPtr()!=world.GetPtr() && loadedDoor.GetPtr()!=door.GetPtr(),"door flags did not restore an independent complete native world/object graph");
    loadedWorld->RestoreRuntimeCaches(loadedGame);const auto restored=SavedDoorFlags(loadedDoor);
    valid=check(loadedWorld,loadedDoor,true) && valid;
    std::printf("native door flags: mode=%d record=%d expected=%d,%d created=%d,%d restored=%d,%d initialOpen=%d legacy=%d bytes=%d valid=%d\n",mode,recordID,int(chest),int(transparent),int(created.first),int(created.second),int(restored.first),int(restored.second),int(initialOpen),int(legacy),snapshot.GetSize(),int(valid));std::fflush(stdout);
    Require(valid,"native world door creation lost map flags or open-door vision filtering, including a complete world save round trip");
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

class LooseRayWallToken: public CObjectBase
{
    OBJECT_NOCOPY_METHODS(LooseRayWallToken);
};

// Analytic slab geometry supplies only map intersections. The native bullet code
// owns clipping, armor traversal, trajectory records, reflection and RNG decisions.
class LooseRayTraceMap: public NAI::IAIMap
{
    OBJECT_NOCOPY_METHODS(LooseRayTraceMap);
    static void Unused(){Require(false,"loose-ray fixture unexpectedly used an unrelated map operation");}
public:
    struct Slab {float lo,hi;NAI::SSourceInfo source;int id;};
    vector<Slab> slabs;
    vector<CRay> rays;
    void Trace(const CRay& ray,vector<NAI::SInterval>* result,int mask,const NAI::CFloorsSet&,ESplitTerrainHGroups) override {
        Require(mask==NWorld::TS_FRAGMENTED,"loose ray changed its native trace mask");
        rays.push_back(ray);result->clear();
        if(fabs(ray.ptDir.x)<1e-6f)return;
        for(const auto& slab:slabs) {
            float a=(slab.lo-ray.ptOrigin.x)/ray.ptDir.x,b=(slab.hi-ray.ptOrigin.x)/ray.ptDir.x;
            CVec3 na(-1,0,0),nb(1,0,0);
            if(a>b){std::swap(a,b);std::swap(na,nb);}
            if(b<0)continue;
            result->push_back(NAI::SInterval(slab.source,slab.id,NAI::SInterval::SCrossPoint(a,na),NAI::SInterval::SCrossPoint(b,nb)));
        }
        std::sort(result->begin(),result->end(),[](const NAI::SInterval& a,const NAI::SInterval& b){return a.enter.fT<b.enter.fT;});
    }
    void Sync(ESyncType) override {Unused();}
    void GetEntities(list<NAI::SObjectInfo>*,int,const NAI::CFloorsSet&) override {Unused();}
    void TraceGrid(NAI::CFastRenderer*,int,ESort,const NAI::CFloorsSet&,ESplitTerrainHGroups,bool) override {Unused();}
    void TraceVoxelGrid(NAI::CExplVoxelRenderer*,int,const NAI::CFloorsSet&,bool) override {Unused();}
    void TraceVisionGrid(NAI::CVisionVoxelRenderer*,int,const NAI::CFloorsSet&,bool) override {Unused();}
    bool GetUnitHLPos(CVec3*,CObjectBase*,int) override {Unused();return false;}
    void GetAccessibleUnitHL(vector<int>*,const CVec3&,CObjectBase*,float) override {Unused();}
    CObjectBase* GetHull(CObjectBase*) override {Unused();return nullptr;}
    bool GetObjectBound(SBound*,CObjectBase*) override {Unused();return false;}
    bool CalcIntersection(const CVec3&,float,int,CObjectBase*) override {Unused();return false;}
    void PrepareCollider(NAI::IPrepareCollider*,const SBound&,float,int,bool) override {Unused();}
    void AddTracker(NAI::IAIMapTracker*,const SBound&,int,bool) override {Unused();}
    void FlipDoorWindow(CObjectBase*,bool) override {Unused();}
    NAI::IStabilityTrackers* GetStabilityTrackers() override {Unused();return nullptr;}
    void SelectHullPointers(vector<CPtr<CObjectBase>>*,const SBound&,int,int) override {Unused();}
    bool GetWindowPos(CObjectBase*,CVec3*,CVec3*) override {Unused();return false;}
};

class PeriodicClockStateProbe: public NWorld::CUnitStateNormal
{
    OBJECT_NOCOPY_METHODS(PeriodicClockStateProbe);
public:
    vector<STime> callbacks;
    explicit PeriodicClockStateProbe(NWorld::CUnitServer* unit=nullptr): CUnitStateNormal(unit) {}
    void OnFinishTimeOrTurn(bool realTime) override {
        Require(realTime,"periodic state probe received a turn callback in its realtime test");
        callbacks.push_back(pUS->GetWorld()->GetTime()->GetValue());
    }
};

static void PeriodicUnitClocks(const char* databasePath,const char* resourcePath,bool staggered)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    Require(world->IsRealTime(),"periodic clock fixture did not start in native realtime");
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(const auto& unit:units)if(IsValid(unit) && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->IsPerformingAction()){actor=unit.GetPtr();break;}
    Require(actor,"periodic clock fixture has no idle healthy non-PK native actor");
    CObj<PeriodicClockStateProbe> observer=new PeriodicClockStateProbe(actor);actor->SetState(observer);
    const STime now=world->GetTime()->GetValue();
    auto clocks=[&] {
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(actor,&refs);std::pair<STime,STime> value{};
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(20,&value.first);reader.Add(21,&value.second);}
        return value;
    };
    auto before=clocks();
    Require(before.first==before.second,"fresh native unit clocks have different construction epochs");
    // Post-init already advances the world by50ms; thresholds use the unit's actual birth epoch.
    const STime base=staggered?now:before.first;
    std::printf("native periodic initial clocks: world=%u base=%u critical=%u state=%u\n",now,base,before.first,before.second);std::fflush(stdout);
    const int ap=actor->GetAP(),vp=actor->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();
    auto step=[&](STime delta,int callbacks,STime stateAt,STime criticalAt) {
        world->GetTime()->Set(base+delta);world->GetAimTime()->Set(base+delta);actor->Segment();
        const auto stamps=clocks();
        std::printf("native periodic clocks: staggered=%d elapsed=%u callbacks=%zu expected=%d state=%u expectedState=%u critical=%u expectedCritical=%u\n",
            int(staggered),delta,observer->callbacks.size(),callbacks,stamps.second,stateAt,stamps.first,criticalAt);std::fflush(stdout);
        Require(observer->callbacks.size()==std::size_t(callbacks) && stamps.second==stateAt && stamps.first==criticalAt,
            "native realtime state/critical schedules fired early or changed each other's clock");
        Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==vp && actor->GetPosition()==position,
            "periodic scheduling changed an idle non-PK actor's AP/VP/position");
    };
    if(staggered) {
        STime critical=base-28000,state=base-1000;CMemoryStream saved;
        {CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(20,&critical);writer.Add(21,&state);}saved.Seek(0);
        {CStructureSaver reader(saved,CStructureSaver::READ);static_cast<CObjectBase*>(actor)->operator&(reader);}
        step(0,0,state,critical);step(1999,0,state,critical);step(2000,0,state,base+2000);
        step(4999,0,state,base+2000);step(5000,1,base+5000,base+2000);
    } else {
        step(5999,0,base,base);step(6000,1,base+6000,base);step(11999,1,base+6000,base);
        step(12000,2,base+12000,base);step(18000,3,base+18000,base);step(24000,4,base+24000,base);
        step(29999,4,base+24000,base);step(30000,5,base+30000,base+30000);
        step(36000,6,base+36000,base+30000);step(59999,7,base+59999,base+30000);
        step(60000,7,base+59999,base+60000);
    }
}

static void PKWorldDamageRouting(const char* databasePath,const char* resourcePath,int kind)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* pilot=nullptr;
    for(const auto& unit:units)if(IsValid(unit) && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK()){pilot=unit.GetPtr();break;}
    Require(pilot,"PK world fixture lacks a healthy native pilot");
    NDb::CRPGPers* shellPersona=nullptr;auto* table=NDatabase::GetTable<NDb::CRPGPers>();CDBIterator<NDb::CRPGPers> it(*table);
    while(it.MoveNext()){auto* p=it.Get();if(p->pPanzerklein && !p->pPanzerklein->bHasNoHead && p->pModel){shellPersona=p;break;}}
    Require(shellPersona,"PK world fixture lacks an authored regular shell");
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(pilot->GetPosition().GetCP(),20.f),&places);
    NAI::SPathPlace anchor;bool found=false;
    for(auto place:places){place.SetPose(NAI::CM_STAND);place.SetDirection(0);if(NAI::IsBigLockerPlacePassable(network,place)){anchor=place;found=true;break;}}
    Require(found,"PK world fixture lacks an unoccupied native suit footprint");
    CObj<NWorld::CUnitServer> shell=world->AddUnitInGame(anchor,NRPG::CreateUnit(shellPersona),nullptr);
    Require(shell && shell->IsEmptyPK() && shell->WearAsPK(true),"PK world fixture could not prepare the real shell");
    pilot->SetPosition(shell->GetPosition());pilot->FlipPanzerklein(shell,false,false);
    Require(pilot->IsWearingPK() && pilot->GetUnitRPG()->GetPanzerklein()==shellPersona->pPanzerklein.GetPtr(),"PK world fixture did not establish its real suit/pilot relation");
    auto& shellVP=shell->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP);shellVP.SetValue(20);Require(int(shellVP)==20,"PK world fixture could not set its native shell VP");
    auto* flesh=NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);Require(IsValid(flesh) && IsValid(flesh->pMaterial) && flesh->pMaterial->nThreshold==0,"PK world flesh control lacks zero armor threshold");
    CObj<NDb::CRPGMaterial> material=new NDb::CRPGMaterial;material->nDR=flesh->pMaterial->nDR;material->nThreshold=0;material->fDensity=1;
    CObj<NDb::CRPGArmor> armor=new NDb::CRPGArmor;armor->pMaterial=material;armor->fRicochetMaxCos=0;armor->fRicochetProbability=0;
    const int kinetic=kind==1?1600:kind==3?1591:1590,base=kind==4?0:8;
    NRPG::CAttackPortion attack(kinetic,kind==2?-1:0,0.f,base,base,0,0,nullptr,pilot->GetUnitRPG(),1.f,-1);attack.bBypassPK=kind==1;
    for(;;){CObj<NWorld::CHitLocator> pending=world->GetHitEvent();if(!pending)break;}
    const int humanBefore=pilot->GetUnitRPG()->GetTotalVP();GlobalGameRandom().SeedForHarness(1);
    const int amount=pilot->ProcessAttack(world,NAI::HL_BODY,&attack,CVec3(0,0,-1),armor);
    CObj<NWorld::CHitLocator> hit=world->GetHitEvent();
    const int expectedAmount=kind==2?-1:kind==4?0:8,expectedHuman=(kind==1 || kind==3)?8:0,expectedShell=(kind==0 || kind==3)?8:0;
    const bool expectedPK=kind==0 || kind==4;
    std::printf("native PK world hit: kind=%d amount=%d hit=%d value=%d bPK=%d human=%d->%d shell=20->%d K=%d expectedValue=%d expectedPK=%d\n",kind,amount,int(IsValid(hit)),hit?hit->nHitValue:0,hit?int(hit->bPK):-1,humanBefore,pilot->GetUnitRPG()->GetTotalVP(),int(shellVP),attack.nK,expectedAmount,int(expectedPK));std::fflush(stdout);
    Require(amount==expectedAmount && humanBefore-pilot->GetUnitRPG()->GetTotalVP()==expectedHuman && 20-int(shellVP)==expectedShell && hit && hit->pUnit.GetPtr()==pilot && hit->nHitValue==expectedAmount && hit->bPK==expectedPK,
        "native world PK hit lost damage or labelled the pilot, suit or rejected impact as the wrong receiver");
    Require(!world->GetHitEvent(),"one native PK hit emitted duplicate hit locators");
}

static void PKDamageRouting(const char* databasePath,int kind)
{
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);
    auto* persona=NDb::GetPers(103);Require(IsValid(persona) && !persona->pPanzerklein,"PK damage fixture lacks an ordinary native persona");
    NDb::CRPGPers* shellPersona=nullptr;auto* table=NDatabase::GetTable<NDb::CRPGPers>();CDBIterator<NDb::CRPGPers> it(*table);
    while(it.MoveNext()){auto* p=it.Get();if(p->pPanzerklein){shellPersona=p;break;}}
    Require(shellPersona,"PK damage fixture lacks an authored native shell persona");
    CObj<NRPG::CUnit> human=new NRPG::CUnit(persona),shell=new NRPG::CUnit(shellPersona);
    CObj<NRPG::IUnitMission> humanMission=NRPG::CreateUnit(human),shellMission=NRPG::CreateUnit(shell);
    auto& shellVP=shell->Skills(NDb::ST_VP);Require(shellVP.GetMaxValue()>=20,"native PK shell lacks twenty expendable VP");
    shellVP.SetValue(kind==5?5:kind==7?0:20);
    auto* flesh=NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);Require(IsValid(flesh) && IsValid(flesh->pMaterial) && flesh->pMaterial->nThreshold==0,"PK damage fixture requires the authored zero-threshold flesh control");
    CObj<NDb::CRPGMaterial> material=new NDb::CRPGMaterial;material->nDR=flesh->pMaterial->nDR;material->nThreshold=0;material->fDensity=1;
    CObj<NDb::CRPGArmor> armor=new NDb::CRPGArmor;armor->pMaterial=material;armor->fRicochetMaxCos=0;armor->fRicochetProbability=0;
    CObj<NDb::CPanzerklein> suit=new NDb::CPanzerklein;
    suit->pPers=shellPersona;suit->nRicochetProb=0;suit->nMaxVP=shellVP.GetMaxValue();suit->nAddMoveAP=0;suit->fAddCoverIgnore=0;
    suit->fSensorRange=0;suit->fRegenerationValue=0;suit->fCriticalResist=(kind==3 || kind==12 || kind==13)?.75f:.5f;suit->pArmor=armor;suit->fEncumbranceKoeff=1;
    suit->nGrenadeStrength=0;suit->bHasNoHead=false;for(auto& allowed:suit->bAllowWeaponType)allowed=true;
    if(kind!=8 && kind!=10 && kind!=11)humanMission->SetPanzerklein(suit,&shellVP,shellMission->GetInventory());
    CObj<NRPG::IUnitMission> target=humanMission;CObj<NRPG::CUnit> targetUnit=human;
    if(kind==8){target=shellMission;targetUnit=shell;}
    Require(target->GetTotalVP()>16 && !target->IsDead() && !target->IsUnconscious(),"PK damage fixture target is not healthy/conscious");
    const int kinetic=(kind==1 || kind==13)?1591:kind==2?1600:1590,base=kind==4?0:8;
    NRPG::CAttackPortion attack(kinetic,kind==9?-1:0,0.f,base,base,kind==3?80:(kind==12 || kind==13)?100:0,0,nullptr,target.GetPtr(),1.f,-1);
    attack.bBypassPK=kind==2;
    if(kind==6 || kind==11)targetUnit->nCheats|=NRPG::CHEAT_GODMODE;
    Require(kind==9 || (attack.CanDealDmg(armor) && attack.CalcStructDmg(nullptr,armor,0)==base),"PK controlled armor did not retain native damage input");
    const int humanBefore=humanMission->GetTotalVP(),shellBefore=shellVP;
    GlobalGameRandom().SeedForHarness((kind==12 || kind==13)?2:1);auto beforeRandom=GlobalGameRandom(),probe=beforeRandom;const auto firstRoll=probe.Get(100);
    if(kind==3)Require(firstRoll<20,"PK pilot-critical fixture seed does not select its native probability branch");
    if(kind==12 || kind==13)Require(firstRoll>=25 && firstRoll<75,"PK asymmetric resistance control does not distinguish the two pilot probability formulas");
    const auto received=target->ProcessAttack(nullptr,NAI::HL_BODY,&attack,armor);
    const int amount=received.nDmg,receiver=int(received.type);
    const int humanLoss=humanBefore-humanMission->GetTotalVP(),shellLoss=shellBefore-int(shellVP);
    std::printf("native PK routing: kind=%d amount=%d receiver=%d human=%d->%d shell=%d->%d K=%d->%d critical=%d min=%d max=%d firstRoll=%u\n",kind,amount,receiver,humanBefore,humanMission->GetTotalVP(),shellBefore,int(shellVP),kinetic,attack.nK,attack.nCrtical,attack.nDmgMin,attack.nDmgMax,firstRoll);std::fflush(stdout);
    int expectedReceiver=1;
    if(kind==0 || kind==4 || kind==5 || kind==8 || kind==12)expectedReceiver=2;
    if(kind==6 || kind==9 || kind==11)expectedReceiver=0;
    if(kind==0 || kind==1 || kind==2 || kind==4 || kind==5 || kind==7 || kind==8 || kind==10) {
        const int expectedAmount=kind==4?0:8;
        const int expectedHuman=(kind==1 || kind==2 || kind==7 || kind==10)?8:0;
        const int expectedShell=(kind==0 || kind==1 || kind==8)?8:kind==5?5:0;
        const int expectedK=(kind==0 || kind==4 || kind==5 || kind==8)?0:kind==1?1:kinetic;
        Require(amount==expectedAmount && humanLoss==expectedHuman && shellLoss==expectedShell && attack.nK==expectedK,
            "native PK routing mixed suit/pilot damage, lost zero damage, or changed the original 1590 penetration boundary");
        if(kind==5)Require(int(shellVP)==0 && target->HasCritical(NDb::C_PANZERKLEIN_BROKEN),"native PK overkill did not clamp VP and mark the broken suit");
    } else if(kind==3) {
        Require(shellLoss==0 && humanLoss>0 && amount==humanLoss && attack.nCrtical==20 && attack.nDmgMin==2 && attack.nDmgMax==2 && attack.nK==kinetic,
            "native pilot critical used resistance rather than its complement, scaled suit damage, or forced critical2000");
    } else if(kind==12 || kind==13) {
        Require(shellLoss==8 && (kind==12?humanLoss==0 && amount==8:humanLoss>0 && amount==humanLoss) && attack.nK==(kind==12?0:1) && attack.nCrtical==100 && attack.nDmgMin==8 && attack.nDmgMax==8,
            "native asymmetric resistance picked the wrong pilot branch or armor passage rewrote the human attack's critical/damage values");
    } else {
        Require(amount==-1 && humanLoss==0 && shellLoss==0 && attack.nK==kinetic,"native rejected/GodMode PK attack damaged its receiver or changed penetration");
    }
    Require(receiver==expectedReceiver,"native received-damage classification does not match suit/pilot/rejection route");
    if(kind!=3 && kind!=13) {
        const int draws=(kind==6 || kind==11)?0:(kind==1 || kind==9)?2:1;
        auto expectedRandom=beforeRandom,afterRandom=GlobalGameRandom();for(int i=0;i<draws;++i)expectedRandom.Get();
        for(int i=0;i<4;++i)Require(afterRandom.Get()==expectedRandom.Get(),"native PK damage changed the original combat RNG draw sequence");
    }
}

static void RayShooterFirstTurn(const char* databasePath,const char* resourcePath,int kind)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(const auto& unit:units)if(IsValid(unit) && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK()){actor=unit.GetPtr();break;}
    Require(actor,"ray first-turn fixture has no native non-PK shooter");
    auto* rpg=actor->GetUnitRPG();int counter=kind==3?1:kind;
    CMemoryStream restored;
    {CStructureSaver writer(restored,CStructureSaver::WRITE);writer.Add(39,&counter);}restored.Seek(0);
    {CStructureSaver reader(restored,CStructureSaver::READ);static_cast<CObjectBase*>(rpg)->operator&(reader);}
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(rpg,&refs);int actualCounter=-999;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(39,&actualCounter);}
    Require(actualCounter==counter,"ray fixture did not restore the actual mission turn counter");
    auto* shooter=kind==3?nullptr:actor;const bool expected=kind<2;
    const int ap=actor->GetAP(),vp=rpg->GetTotalVP();const auto position=actor->GetPosition();
    NRPG::CAttackPortion attack;const CVec3 origin=position.GetCP(),direction(1,0,0);
    NRPG::SAttackRayInfo direct(attack,origin,direction,shooter,position,0,0,false,0,30,nullptr);
    NRPG::SAttackRayInfo accidental;NRPG::MakeAccidentalShot(&accidental,shooter,position,attack,origin,direction,30);
    std::printf("native ray first-turn: counter=%d shooter=%d worldFirst=%d direct=%d accidental=%d expected=%d\n",actualCounter,int(shooter!=nullptr),int(world->IsFirstTurn()),int(direct.bFirstTurn),int(accidental.bFirstTurn),int(expected));std::fflush(stdout);
    Require(direct.bFirstTurn==expected && accidental.bFirstTurn==expected,"native ray constructor/accidental shot lost the shooter's own first-turn flag");
    Require(direct.pUS.GetPtr()==shooter && accidental.pUS.GetPtr()==shooter && !accidental.bTargetIsHit,
        "first-turn propagation changed the shooter or accidental-shot miss metadata");
    Require(actor->GetAP()==ap && rpg->GetTotalVP()==vp && actor->GetPosition()==position,"ray metadata construction changed native AP/VP/position");
}

static void PeriodicCriticalLifetime(const char* databasePath,const char* resourcePath,int kind)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(const auto& unit:units)if(IsValid(unit) && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && !unit->IsPerformingAction()){actor=unit.GetPtr();break;}
    Require(actor,"critical lifetime fixture has no idle healthy non-PK native actor");
    auto* rpg=actor->GetUnitRPG();rpg->RemoveCritical(NDb::C_BLEEDING);
    const int duration=kind==3?-1:kind==4?3:1;
    rpg->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_BLEEDING,duration,0));
    auto check=[&](bool present,int remaining,const char* phase) {
        NRPG::CCritical* critical=nullptr;const bool actual=rpg->HasCritical(NDb::C_BLEEDING,&critical);
        const int actualRemaining=critical?critical->GetRemainingTime():-999;
        std::printf("native critical lifetime: kind=%d phase=%s present=%d expected=%d remaining=%d expected=%d\n",kind,phase,int(actual),int(present),actualRemaining,remaining);std::fflush(stdout);
        Require(actual==present && (!present || actualRemaining==remaining),"native critical lifetime advanced at AP refresh or failed to advance at its periodic end tick");
    };
    check(true,duration,"installed");
    if(kind==0) {
        rpg->StartNewTurn(actor->GetPosition().GetCP());check(true,1,"first AP refresh");
        rpg->StartNewTurn(actor->GetPosition().GetCP());check(true,1,"second AP refresh");
    } else if(kind==1) {
        actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);check(true,0,"first own-turn finish");
        rpg->StartNewTurn(actor->GetPosition().GetCP());check(true,0,"intervening AP refresh");
        actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);check(false,0,"second own-turn finish");
    } else if(kind==2) {
        Require(world->IsRealTime(),"critical lifetime fixture is not in real time");
        const STime base=world->GetTime()->GetValue();
        auto step=[&](STime elapsed) {world->GetTime()->Set(base+elapsed);world->GetAimTime()->Set(base+elapsed);actor->Segment();};
        step(6000);check(true,1,"state-only interval");step(29900);check(true,1,"before regeneration");
        step(30000);check(true,0,"first regeneration tick");step(60000);check(false,0,"second regeneration tick");
    } else if(kind==3) {
        for(int i=0;i<3;++i){rpg->StartNewTurn(actor->GetPosition().GetCP());actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);check(true,-1,"permanent control");}
    } else {
        rpg->SuspendCriticals(1);check(false,0,"suspended");
        actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);check(true,3,"suspension expired");
        rpg->StartNewTurn(actor->GetPosition().GetCP());check(true,3,"resumed AP refresh");
        actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);check(true,2,"resumed periodic tick");
    }
}

static void SeedPeriodicBleedNoDodge(NRPG::IUnitMission* rpg)
{
    // Select a deterministic native RNG input for the existing dodge rule, not a replacement damage path.
    const int ic=rpg->GetIC();Require(ic<100,"periodic fixture requires a native shooter with a non-certain dodge chance");
    CRandomGenerator reference;unsigned int seed=1,roll=0;
    for(;seed<256;++seed){reference.SeedForHarness(seed);roll=reference.Get(100);if(int(roll)>=ic)break;}
    Require(seed<256,"periodic fixture could not select a native no-dodge RNG input");
    GlobalGameRandom().SeedForHarness(seed);
    std::printf("native periodic RNG: seed=%u firstDodgeRoll=%u IC=%d\n",seed,roll,ic);std::fflush(stdout);
}

static void NoManufacturedBleeding(const char* databasePath,const char* resourcePath,bool blocked)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(const auto& unit:units)if(IsValid(unit) && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && unit->GetUnitRPG()->GetHealedVP()==0 && unit->GetUnitRPG()->GetTotalVP()>30){actor=unit.GetPtr();break;}
    Require(actor,"implicit bleeding fixture lacks a healthy native human actor");
    auto* rpg=actor->GetUnitRPG();rpg->RemoveCritical(NDb::C_BLEEDING);auto& vp=rpg->GetRPGUnit()->Skills(NDb::ST_VP);
    const int desired=Float2Int(vp.GetTheoreticalMax()*.5f)-9;Require(desired>8,"implicit bleeding fixture lacks enough low-health VP");vp.SetValue(desired);
    Require(!rpg->HasCritical(NDb::C_BLEEDING),"implicit bleeding fixture already has an explicit critical");
    for(;;){CObj<NWorld::CHitLocator> pending=world->GetHitEvent();if(!pending)break;}
    NRPG::CAttackPortion attack(blocked?0:100000,0,0.f,4,4,0,0,nullptr,rpg,1.f,-1);GlobalGameRandom().SeedForHarness(2);
    const int before=rpg->GetTotalVP(),amount=actor->ProcessAttack(world,NAI::HL_BODY,&attack,CVec3(0,0,-1),rpg->GetRPGArmor());
    CObj<NWorld::CHitLocator> hit=world->GetHitEvent();NRPG::CCritical* critical=nullptr;const bool manufactured=rpg->HasCritical(NDb::C_BLEEDING,&critical);
    std::printf("native manufactured bleeding: blocked=%d amount=%d VP=%d->%d hit=%d critical=%d strength=%g\n",int(blocked),amount,before,rpg->GetTotalVP(),hit?hit->nHitValue:-999,int(manufactured),critical?critical->GetValue():0.f);std::fflush(stdout);
    const int expected=blocked?0:4;
    Require(amount==expected && rpg->GetTotalVP()==before-expected && hit && hit->nHitValue==expected && !world->GetHitEvent(),"implicit bleeding fixture did not execute its real blocked/landed attack control");
    Require(!manufactured,"ordinary low-health damage manufactured a permanent bleeding critical, including on a blocked hit");
}

static void BleedingReadout(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(const auto& unit:units)if(IsValid(unit) && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK()){actor=unit.GetPtr();break;}
    Require(actor,"bleeding readout fixture has no native human actor");
    const NWorld::CUnit* unit=actor;
    Require(unit->GetBleeding()==0,"fresh native unit has an unexpected bleeding readout");
    for(int value:{7,-7,0}) {
        CMemoryStream saved;{CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(32,&value);}saved.Seek(0);
        {CStructureSaver reader(saved,CStructureSaver::READ);static_cast<CObjectBase*>(actor)->operator&(reader);}
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(actor,&refs);int restored=-999;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(32,&restored);}
        const int actual=unit->GetBleeding();
        std::printf("native bleeding readout: saved=%d restored=%d virtual=%d\n",value,restored,actual);std::fflush(stdout);
        Require(restored==value,"native bleeding readout fixture did not restore its real saved field");
        Require(actual==restored,"native CUnit bleeding readout ignored the server's saved periodic amount");
    }
}

static void NativeDirectAttackXP(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<19,"unknown direct attack XP mode");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    auto deploy=[&](const vector<int>& people,const wchar_t* name) {
        game->players.push_back(NRPG::CreateGlobalPlayer(people));CObj<NAI::CSequenceCommander> commander=new NAI::CSequenceCommander(world);
        auto* p=dynamic_cast<NWorld::CPlayer*>(world->AddPlayer(name,game->players.back(),commander));Require(p,"direct XP party deployment failed");commander->SetPlayer(p);return p;
    };
    CObj<NWorld::CPlayer> party=deploy(vector<int>{54,2,3},L"Direct XP party"),second;
    if(mode==5 || mode==16)second=deploy(vector<int>{54},L"Direct XP finisher");
    list<CPtr<NWorld::CUnitServer>> all;world->GetAllUnits(&all);CPtr<NWorld::CUnitServer> victim,attacker,finisher;
    for(auto u:all) {
        if(u->GetPlayer()==party.GetPtr() && u->CanFight() && !attacker)attacker=u;
        if(second && u->GetPlayer()==second.GetPtr())finisher=u;
        if(!victim && u->GetPlayer() && u->GetPlayer()->GetScenarioPlayerID()>0 && u->CanFight() && !u->IsWearingPK() && !u->IsEmptyPK() && !u->IsPerformingAction() && u->GetUnitRPG()->GetTotalVP()>16 && std::isfinite(u->GetUnitRPG()->GetXP(1)) && u->GetUnitRPG()->GetXP(1)>0)victim=u;
    }
    if(mode==6)for(auto u:all)if(u->GetPlayer()==party.GetPtr() && u!=attacker && u->CanFight()){victim=u;break;}
    Require(victim && attacker && (!second || finisher),"direct XP fixture lacks live native victim/attacker");victim->nClueCount=0;
    CObj<NRPG::IUnitMission> victimMission=victim->GetUnitRPG(),attackerMission=attacker->GetUnitRPG(),finisherMission=finisher?finisher->GetUnitRPG():nullptr;
    auto relation=[&](NWorld::CUnitServer* source,NDb::EDiplomacyState value){auto d=source->GetUnitRPG()->GetDiplomacy();d.SetDiplomacyState(victim->GetPlayer()->GetScenarioPlayerID(),value);source->GetUnitRPG()->SetDiplomacy(d);};
    relation(attacker,mode==2?NDb::DS_ALLY:mode==3?NDb::DS_NEUTRAL:NDb::DS_ENEMY);if(finisher)relation(finisher,NDb::DS_ENEMY);
    const bool retired=mode==14 || mode==15,removed=mode==12 || mode==13 || mode==18;
    const CVec3 origin=attacker->GetPosition().GetCP();
    struct RestoreOptions {NGlobal::CValue golden,blood;~RestoreOptions(){NGlobal::SetVar("cheat_golden_shot",golden);NGlobal::SetVar("cheat_blood",blood);}}options{NGlobal::GetVar("cheat_golden_shot",0),NGlobal::GetVar("cheat_blood",1)};
    NGlobal::SetVar("cheat_golden_shot",0);if(mode==13)NGlobal::SetVar("cheat_blood",1);
    auto hit=[&](NRPG::IUnitMission* source,bool ko,bool lethal,bool blocked,bool gib) {
        const int damage=lethal?100000:ko?8:4;
        NRPG::CAttackPortion attack(blocked?0:100000,0,0.f,damage,damage,0,0,source,victimMission,1.f,ko?100:-1);
        attack.bNoBlowUp=!gib;attack.rTtrajectory.ptOrigin=origin;attack.rTtrajectory.ptDir=CVec3(0,1,0);GlobalGameRandom().SeedForHarness(0x7bda);
        return victim->ProcessAttack(world,NAI::HL_BODY,&attack,CVec3(0,1,0),victimMission->GetRPGArmor());
    };
    if(mode==0 || mode==4 || mode==5 || mode==18) {
        // This XP fixture needs a real ordinary-hit knockout. Original health scaling
        // suppresses ordinary knockout above half health. Keep the real difficulty
        // multiplier: fixed input8 deals11 with the party attacker,8 anonymously.
        // Prepare exactly that native body damage plus1, so the real hit leaves HP1.
        auto* source=(mode==0 || mode==5)?attackerMission.GetPtr():nullptr;
        NRPG::CAttackPortion probe(100000,0,0.f,8,8,0,0,source,victimMission,1.f,100);
        const int expectedDamage=probe.CalcStructDmg(world,NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR),0);
        Require(expectedDamage==((mode==0 || mode==5)?11:8),"direct XP native difficulty-adjusted knockout damage changed");
        auto& health=victimMission->GetRPGUnit()->Skills(NDb::ST_VP);const int before=health;
        health.Modify(expectedDamage+1-int(health));
        Require(victimMission->GetTotalVP()==expectedDamage+1 && health.GetMaxValue()==80 && !victim->IsDead() && !victim->IsUnconscious(),"direct XP lacks its native wounded, living pre-knockout target");
        std::printf("DIRECT_XP_KO_HEALTH mode=%d before=%d prepared=%d cap=%d expectedDamage=%d expectedRemaining=1\n",mode,before,victimMission->GetTotalVP(),health.GetMaxValue(),expectedDamage);std::fflush(stdout);
    }
    if(mode==4 || mode==17 || mode==18) {
        world->GetTime()->Set(1000);victim->animator.AlignTime();hit(nullptr,mode!=17,mode==17,false,false);
        Require(mode!=17?victim->IsUnconscious() && !victim->IsDead():victim->IsDead(),"direct XP pre-incapacitated setup failed");
        if(mode!=17)Require(victimMission->GetTotalVP()==1,"direct XP native pre-knockout hit did not leave exactly HP1");
    }
    int excluded=0;
    if(mode==10 || mode==11) {
        list<CPtr<NWorld::CUnitServer>> roster;party->GetUnitsThatCanFight(&roster);
        for(auto u:roster)if(mode==11 || (u!=attacker && excluded==0)){u->SetState(new NWorld::CUnitStateUnconscious(u));Require(!u->CanFight(),"direct XP excluded combatant still can fight");++excluded;}
        Require(excluded>0,"direct XP exclusion control removed no fighter");
    }
    if(retired){world->RemoveUnit(attacker);Require(!world->GetUnitServer(attackerMission),"direct XP retired attacker is still registered in the world");}
    struct Check {CObj<NRPG::CUnit> actual,reference;CPtr<NWorld::CUnitServer> server;int side,persona;};std::vector<Check> checks;
    for(auto u:all) {
        // Logical RPG data is held independently; removal tests never read invalid server contents.
        auto* actual=u==attacker?attackerMission->GetRPGUnit():u==victim?victimMission->GetRPGUnit():u->GetUnitRPG()->GetRPGUnit();
        CObj<NRPG::CUnit> saved=actual;CMemoryStream bytes;{CStructureSaver writer(bytes,CStructureSaver::WRITE);writer.Add(1,&saved);}bytes.Seek(0);CObj<NRPG::CUnit> reference;{CStructureSaver reader(bytes,CStructureSaver::READ);reader.Add(1,&reference);}
        Require(reference && reference.GetPtr()!=actual && &reference->Skills(0)!=&actual->Skills(0),"direct XP reference is not a deep native unit clone");
        checks.push_back({actual,reference,u,u==attacker?party->GetScenarioPlayerID():u->GetPlayer()->GetScenarioPlayerID(),actual->pPers->GetRecordID()});
    }
    auto points=[](NRPG::CUnit* u){return u->GetPerksTree()?u->GetPerksTree()->GetPerkPoints():0;};int totalMismatch=0;
    const int phases=(mode==5 || mode==16)?2:1;
    for(int phase=0;phase<phases;++phase) {
        const bool ko=(mode==0 || mode==5) && phase==0,blocked=mode==8,nonlethal=mode==7 || mode==8 || mode==14 || mode==17 || (mode==16 && phase==1);
        const bool anonymous=mode==9;
        auto* source=phase==1?finisher.GetPtr():retired?nullptr:attacker.GetPtr();NRPG::IUnitMission* sourceMission=anonymous?nullptr:phase==1?finisherMission.GetPtr():attackerMission.GetPtr();
        const bool beforeDead=victim->IsDead(),beforeKO=victim->IsUnconscious();
        const bool hostile=source && source->GetDiplomacyState(victim)==NDb::DS_ENEMY;
        list<CPtr<NWorld::CUnitServer>> roster;if(source && source->GetTBSPlayer())source->GetTBSPlayer()->GetUnitsThatCanFight(&roster);
        const float share=victimMission->GetXP(int(roster.size()));const bool shouldPay=!anonymous && hostile && (removed || (!beforeDead && !beforeKO && !nonlethal));
        std::vector<float> xpBefore;for(auto& x:checks){xpBefore.push_back(x.actual->fXP);if(shouldPay && std::find(roster.begin(),roster.end(),x.server)!=roster.end())x.reference->AddXP(share);}
        const int beforeVP=victimMission->GetTotalVP();world->GetTime()->Set(phase?2345:1234);victim->animator.AlignTime();if(mode==12 || mode==18)NGlobal::SetVar("cheat_golden_shot",1);
        std::printf("DIRECT_XP_INPUT mode=%d phase=%d persona=%d cap=%d level=%d party=%d share=%.9g beforeDead=%d beforeKO=%d sourceRegistered=%d anonymous=%d hostile=%d shouldPay=%d\n",mode,phase,victimMission->GetRPGUnit()->pPers->GetRecordID(),victimMission->GetRPGUnit()->cap[NDb::ST_LEVEL],int(victimMission->GetRPGUnit()->Skills(NDb::ST_LEVEL)),int(roster.size()),share,int(beforeDead),int(beforeKO),int(source!=nullptr),int(anonymous),int(hostile),int(shouldPay));std::fflush(stdout);
        const int damage=hit(sourceMission,ko,!nonlethal && !ko,blocked,mode==13);const bool valid=IsValid(victim);
        if(removed)Require(!valid,"direct XP removal control retained a valid native server");
        else if(ko)Require(valid && victim->IsUnconscious() && !victim->IsDead() && damage==11 && victimMission->GetTotalVP()==1,"direct XP attack did not produce its real first knockout");
        else if(!nonlethal)Require(valid && victim->IsDead() && damage>0,"direct XP attack did not produce real death");
        else if(mode!=17 && !(mode==16 && phase==1))Require(valid && victim->CanFight() && (blocked?damage==0:damage>0),"direct XP non-incapacitating control downed its target or missed");
        int mismatches=0,awarded=0;
        for(std::size_t i=0;i<checks.size();++i) {
            auto& x=checks[i];auto* a=x.actual.GetPtr();auto* ref=x.reference.GetPtr();const bool equal=a->fXP==ref->fXP && int(a->Skills(NDb::ST_LEVEL))==int(ref->Skills(NDb::ST_LEVEL)) && a->Skills(NDb::ST_LEVEL).GetProgress()==ref->Skills(NDb::ST_LEVEL).GetProgress() && points(a)==points(ref);
            if(!equal)++mismatches;if(a->fXP!=xpBefore[i])++awarded;
            std::printf("DIRECT_XP_RECIPIENT mode=%d phase=%d index=%d persona=%d side=%d eligible=%d before=%.9g share=%.9g actual=%.9g expected=%.9g level=%d expectedLevel=%d progress=%.9g expectedProgress=%.9g points=%d expectedPoints=%d equal=%d\n",mode,phase,int(i),x.persona,x.side,int(std::find(roster.begin(),roster.end(),x.server)!=roster.end()),xpBefore[i],share,a->fXP,ref->fXP,int(a->Skills(NDb::ST_LEVEL)),int(ref->Skills(NDb::ST_LEVEL)),a->Skills(NDb::ST_LEVEL).GetProgress(),ref->Skills(NDb::ST_LEVEL).GetProgress(),points(a),points(ref),int(equal));
        }
        auto random=GlobalGameRandom();unsigned words[4];for(auto& w:words)w=random.Get();
        std::printf("native direct attack XP mode=%d phase=%d sourceRegistered=%d anonymous=%d hostile=%d beforeDead=%d beforeKO=%d valid=%d dead=%d knockout=%d HP=%d finalHP=%d damage=%d roster=%d share=%.9g shouldPay=%d awarded=%d mismatches=%d killerFirst=%d killerFinisher=%d deathTime=%u rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,phase,int(source!=nullptr),int(anonymous),int(hostile),int(beforeDead),int(beforeKO),int(valid),int(victimMission->IsDead()),valid?int(victim->IsUnconscious()):-1,beforeVP,victimMission->GetTotalVP(),damage,int(roster.size()),share,int(shouldPay),awarded,mismatches,valid?int(victim->pKiller.GetPtr()==attacker.GetPtr()):-1,valid && finisher?int(victim->pKiller.GetPtr()==finisher.GetPtr()):-1,valid?unsigned(victim->GetDeathTime()):0u,words[0],words[1],words[2],words[3]);std::fflush(stdout);totalMismatch+=mismatches;
    }
    Require(totalMismatch==0,"direct attack XP paid a non-hostile or later corpse hit, missed first KO/removal, or lost the prepared roster/reward");
}

static void NativeFatalBleedingXP(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<11,"unknown fatal bleeding XP mode");
    CObj<NRPG::CGlobalGame> game;
    auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    auto deploy=[&](const vector<int>& people,const wchar_t* name) {
        game->players.push_back(NRPG::CreateGlobalPlayer(people));
        CObj<NAI::CSequenceCommander> commander=new NAI::CSequenceCommander(world);
        auto* added=dynamic_cast<NWorld::CPlayer*>(world->AddPlayer(name,game->players.back(),commander));
        Require(added,"fatal bleed fixture could not deploy real party");commander->SetPlayer(added);
    };
    deploy(vector<int>{54,2,3},L"Bleeding XP party");
    if(mode==8)deploy(vector<int>{54},L"Second bleeding XP party");
    vector<CPtr<NWorld::CPlayer>> players;world->GetPlayersList(&players);
    list<CPtr<NWorld::CUnitServer>> all;world->GetAllUnits(&all);
    NWorld::CUnitServer* victim=nullptr;
    for(auto entry:all) {
        auto* u=entry.GetPtr();
        if(u && u->GetPlayer() && u->GetPlayer()->GetScenarioPlayerID()>0 && u->CanFight() && !u->IsWearingPK() && !u->IsEmptyPK() && !u->IsPerformingAction() && u->GetUnitRPG()->GetIC()<100 && u->GetUnitRPG()->GetTotalVP()>20 && u->GetUnitRPG()->GetHealedVP()==0 && std::isfinite(u->GetUnitRPG()->GetXP(1)) && u->GetUnitRPG()->GetXP(1)>0) {victim=u;break;}
    }
    Require(victim,"fatal bleed fixture lacks ordinary live authored NPC victim");
    auto* mission=victim->GetUnitRPG();auto* deadUnit=mission->GetRPGUnit();victim->nClueCount=mode==10?1:0;
    auto diplomacy=mission->GetDiplomacy();
    for(auto p:players)if(p.GetPtr()!=victim->GetPlayer() && p->GetScenarioPlayerID()>=0)
        diplomacy.SetDiplomacyState(p->GetScenarioPlayerID(),mode==2?NDb::DS_NEUTRAL:mode==3?NDb::DS_ALLY:NDb::DS_ENEMY);
    mission->SetDiplomacy(diplomacy);
    int removed=0;
    if(mode==4 || mode==9)for(auto p:players) {
        if(world->GetDiplomacyState(static_cast<NWorld::CUnit*>(victim),p)!=NDb::DS_ENEMY)continue;
        list<CPtr<NWorld::CUnitServer>> roster;p->GetUnitsThatCanFight(&roster);
        for(auto u:roster)if(u.GetPtr()!=victim && (mode==9 || removed==0)) {
            u->SetState(new NWorld::CUnitStateUnconscious(u));Require(!u->CanFight(),"fatal bleed KO recipient still can fight");++removed;
        }
    }
    if(mode==4 || mode==9)Require(removed>0,"fatal bleed exclusion fixture did not remove a real combatant");
    mission->RemoveCritical(NDb::C_BLEEDING);deadUnit->Skills(NDb::ST_VP).SetValue(mode==1?40:10);
    mission->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_BLEEDING,3,mode==1?1.f:float(110-deadUnit->GetDeathVP())));
    if(mode==10) {victim->SetState(new NWorld::CUnitStateUnconscious(victim));Require(victim->IsClueUnit() && victim->IsUnconscious(),"fatal bleed clue control was not established");}
    if(mode==6) {deadUnit->Kill();victim->SyncConscious();Require(victim->IsDead(),"fatal bleed already-dead control was not established");}
    if(mode==7)world->bNetworkReplica=true;
    const int victimLevel=deadUnit->Skills(NDb::ST_LEVEL);
    std::printf("FATAL_BLEED_VICTIM mode=%d persona=%d level=%d cap=%d currentThreshold=%.9g nextThreshold=%.9g deathReserve=%d\n",mode,deadUnit->pPers->GetRecordID(),victimLevel,deadUnit->cap[NDb::ST_LEVEL],deadUnit->GetXPForSkill(NDb::ST_LEVEL,victimLevel),deadUnit->GetXPForSkill(NDb::ST_LEVEL,victimLevel+1),deadUnit->GetDeathVP());
    const bool expectDeath=mode!=1 && mode!=7 && mode!=10;
    const bool expectAward=expectDeath && mode!=6;
    struct Recipient {CPtr<NWorld::CUnitServer> server;CObj<NRPG::CUnit> reference;float before,share;int side;bool eligible;};
    std::vector<Recipient> checks;int hostileParties=0,eligible=0;
    for(auto p:players) {
        const bool hostile=world->GetDiplomacyState(static_cast<NWorld::CUnit*>(victim),p)==NDb::DS_ENEMY;
        list<CPtr<NWorld::CUnitServer>> roster;p->GetUnitsThatCanFight(&roster);
        if(hostile)++hostileParties;
        const float share=hostile?mission->GetXP(int(roster.size())):0.f;
        for(auto u:all)if(u.GetPtr()!=victim && u->GetPlayer()==p.GetPtr()) {
            auto* actual=u->GetUnitRPG()->GetRPGUnit();CObj<NRPG::CUnit> saved=actual;CMemoryStream bytes;
            {CStructureSaver writer(bytes,CStructureSaver::WRITE);writer.Add(1,&saved);}bytes.Seek(0);
            CObj<NRPG::CUnit> reference;{CStructureSaver reader(bytes,CStructureSaver::READ);reader.Add(1,&reference);}
            Require(reference && reference.GetPtr()!=actual && &reference->Skills(0)!=&actual->Skills(0) && (!actual->GetPerksTree() || reference->GetPerksTree()!=actual->GetPerksTree()),"fatal bleed XP reference did not deep-clone real native unit");
            const bool receives=hostile && std::find(roster.begin(),roster.end(),u)!=roster.end();
            if(receives)++eligible;
            const float before=actual->fXP;
            if(expectAward && receives)reference->AddXP(share);
            checks.push_back({u,reference,before,share,p->GetScenarioPlayerID(),receives});
        }
        std::printf("FATAL_BLEED_ROSTER mode=%d side=%d hostile=%d size=%d share=%.9g\n",mode,p->GetScenarioPlayerID(),int(hostile),int(roster.size()),share);
    }
    Require(!checks.empty() && (mode==2 || mode==3 || hostileParties>0) && (mode==2 || mode==3 || mode==9 || eligible>0),"fatal bleed fixture has no recipient/hostility controls");
    if(mode==8)Require(hostileParties>=2,"fatal bleed two-party fixture did not establish separate hostile rosters");
    for(;;){CObj<NWorld::CHitLocator> hit=world->GetHitEvent();if(!hit)break;}
    SeedPeriodicBleedNoDodge(mission);const int initialVP=mission->GetTotalVP();
    victim->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);
    Require(victim->IsDead()==expectDeath,"fatal bleed stimulus did not establish expected death transition/control");
    if(mode==5)victim->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);
    auto points=[](NRPG::CUnit* u){return u->GetPerksTree()?u->GetPerksTree()->GetPerkPoints():0;};
    int mismatches=0,awarded=0;
    for(std::size_t i=0;i<checks.size();++i) {
        const auto& x=checks[i];auto* actual=x.server->GetUnitRPG()->GetRPGUnit();auto* reference=x.reference.GetPtr();
        const bool equal=actual->fXP==reference->fXP && int(actual->Skills(NDb::ST_LEVEL))==int(reference->Skills(NDb::ST_LEVEL)) && actual->Skills(NDb::ST_LEVEL).GetProgress()==reference->Skills(NDb::ST_LEVEL).GetProgress() && points(actual)==points(reference);
        if(!equal)++mismatches;if(actual->fXP!=x.before)++awarded;
        std::printf("FATAL_BLEED_RECIPIENT mode=%d index=%d persona=%d side=%d eligible=%d before=%.9g share=%.9g actual=%.9g expected=%.9g level=%d expectedLevel=%d progress=%.9g expectedProgress=%.9g points=%d expectedPoints=%d equal=%d\n",mode,int(i),actual->pPers->GetRecordID(),x.side,int(x.eligible),x.before,x.share,actual->fXP,reference->fXP,int(actual->Skills(NDb::ST_LEVEL)),int(reference->Skills(NDb::ST_LEVEL)),actual->Skills(NDb::ST_LEVEL).GetProgress(),reference->Skills(NDb::ST_LEVEL).GetProgress(),points(actual),points(reference),int(equal));
    }
    auto random=GlobalGameRandom();unsigned words[4];for(auto& word:words)word=random.Get();
    std::printf("native fatal bleeding XP mode=%d victim=%d dead=%d initialVP=%d finalVP=%d parties=%d eligible=%d removed=%d recipients=%d awarded=%d mismatches=%d rng0=%u rng1=%u rng2=%u rng3=%u\n",mode,deadUnit->pPers->GetRecordID(),int(victim->IsDead()),initialVP,mission->GetTotalVP(),hostileParties,eligible,removed,int(checks.size()),awarded,mismatches,words[0],words[1],words[2],words[3]);std::fflush(stdout);
    Require(mismatches==0,"fatal periodic death omitted hostile combat-capable roster XP, paid excluded recipients or paid twice");
}

static void PeriodicBleedingPipeline(const char* databasePath,const char* resourcePath,int kind)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(const auto& unit:units) {
        if(!IsValid(unit) || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK() || unit->IsPerformingAction())continue;
        auto* rpg=unit->GetUnitRPG();const auto& vp=rpg->GetRPGUnit()->Skills(NDb::ST_VP);
        if(vp.GetTheoreticalMax()>=40 && int(vp)>=Float2Int(vp.GetTheoreticalMax()*.5f) && rpg->GetHealedVP()==0){actor=unit.GetPtr();break;}
    }
    Require(actor,"periodic pipeline fixture has no healthy idle native non-PK actor");
    auto* rpg=actor->GetUnitRPG();rpg->RemoveCritical(NDb::C_BLEEDING);
    float strength=kind==0?.5f:(kind==2 || kind==3)?0.f:(kind==4 || kind==5)?3.9f:3.5f;
    if(kind==2 || kind==3 || kind==4) {
        auto& vp=rpg->GetRPGUnit()->Skills(NDb::ST_VP);
        const int desired=Float2Int(vp.GetTheoreticalMax()*.5f)-(kind==3?10:9);vp-=int(vp)-desired;
        Require(int(vp)==desired && desired>0,"periodic low-health fixture did not install its native VP");
    }
    if(strength>0)rpg->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_BLEEDING,3,strength));
    if(kind==5) {
        int stopped=2;CMemoryStream saved;{CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(35,&stopped);}saved.Seek(0);
        {CStructureSaver reader(saved,CStructureSaver::READ);static_cast<CObjectBase*>(rpg)->operator&(reader);}
    }
    if(kind==9 || kind==10) {
        actor->nClueCount=kind==9?1:0;actor->SetState(new NWorld::CUnitStateUnconscious(actor));
        Require(actor->IsUnconscious() && !actor->IsDead() && actor->IsClueUnit()==(kind==9),"periodic unconscious/clue fixture did not enter its actual native state");
    }
    if(kind==7)world->bNetworkReplica=true;
    for(;;){CObj<NWorld::CHitLocator> pending=world->GetHitEvent();if(!pending)break;}
    const int before=rpg->GetTotalVP();
    auto inspect=[&](int expected,const char* phase) {
        CObj<NWorld::CHitLocator> hit=world->GetHitEvent();
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(actor,&refs);int amount=-999;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(32,&amount);}
        std::printf("native periodic pipeline: kind=%d phase=%s strength=%g VP=%d->%d hit=%d value=%d savedBleed=%d expected=%d\n",kind,phase,strength,before,rpg->GetTotalVP(),int(IsValid(hit)),hit?hit->nHitValue:0,amount,expected);std::fflush(stdout);
        Require(rpg->GetTotalVP()==before-expected && (expected?hit && hit->pUnit.GetPtr()==actor && hit->nHitValue==expected:!hit) && amount==expected,
            "native periodic bleeding used the old event, omitted whole damage or changed the original health/stop/zero rule");
        const int readout=static_cast<const NWorld::CUnit*>(actor)->GetBleeding();
        std::printf("native periodic readout: kind=%d phase=%s saved=%d virtual=%d\n",kind,phase,amount,readout);std::fflush(stdout);
        Require(readout==amount,"native periodic damage was recorded but its CUnit readout returned a different amount");
        Require(!world->GetHitEvent(),"one periodic pass emitted duplicate native hit events");
    };
    SeedPeriodicBleedNoDodge(rpg);
    const int expected=(kind==0 || kind==2 || kind==6 || kind==7 || kind==9)?0:(kind==3 || kind==5)?1:3;
    if(kind==6) {
        auto* player=dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer());Require(player,"periodic old-event fixture lacks native player");
        actor->OnNewPlayerTurnOrTime(NWorld::CEventOnNewPlayerTurnOrTime(player));inspect(0,"old player event");
    } else if(kind==8) {
        Require(world->IsRealTime(),"periodic bleeding fixture is not in real time");const STime base=world->GetTime()->GetValue();
        auto step=[&](STime delta){world->GetTime()->Set(base+delta);world->GetAimTime()->Set(base+delta);actor->Segment();};
        step(6000);inspect(0,"state-only tick");step(29900);inspect(0,"before regeneration");step(30000);inspect(expected,"regeneration tick");
    } else {actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);inspect(expected,"own-turn finish");}
    if(kind==3)Require(!rpg->HasCritical(NDb::C_BLEEDING),"implicit low-health periodic damage manufactured an explicit permanent bleeding critical");
    if(kind==4) {
        NRPG::CCritical* critical=nullptr;Require(rpg->HasCritical(NDb::C_BLEEDING,&critical) && critical && critical->IsTemporarily() && critical->GetRemainingTime()==2,
            "periodic damage turned its finite bleeding critical into a permanent effect");
    }
}

static void PeriodicBleedingAttack(const char* databasePath,const char* resourcePath,bool fractional)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    Require(!world->bNetworkReplica,"periodic bleeding fixture unexpectedly uses a network replica");
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(const auto& unit:units) {
        if(!IsValid(unit) || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK())continue;
        auto* rpg=unit->GetUnitRPG();const auto& vp=rpg->GetRPGUnit()->Skills(NDb::ST_VP);
        if(vp.GetTheoreticalMax()>=40 && rpg->GetTotalVP()>20 && rpg->GetTotalVP()>=Float2Int(vp.GetTheoreticalMax()*.5f)){actor=unit.GetPtr();break;}
    }
    Require(actor,"periodic bleeding fixture has no healthy non-PK native actor");
    auto* rpg=actor->GetUnitRPG();rpg->RemoveCritical(NDb::C_BLEEDING);
    const float strength=fractional?.5f:3.5f;
    rpg->ApplyCritical(NRPG::SCritical(NDb::CL_ANY,NDb::C_BLEEDING,3,strength));
    NRPG::CCritical* critical=nullptr;
    Require(rpg->HasCritical(NDb::C_BLEEDING,&critical) && critical && fabs(critical->GetValue()-strength)<1e-5f,
        "periodic fixture did not install its actual fractional/integer bleeding critical");
    for(;;){CObj<NWorld::CHitLocator> pending=world->GetHitEvent();if(!pending)break;}
    const int vp=rpg->GetTotalVP(),ap=actor->GetAP();const STime time=world->GetAimTime()->GetValue();const auto position=actor->GetPosition();
    auto* player=dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer());Require(player,"bleeding fixture has no concrete native player");
    // Reproduce the positive attack path deterministically; the original dodge rule still runs.
    CRandomGenerator reference;reference.SeedForHarness(2);const auto dodgeRoll=reference.Get(100);
    Require(int(dodgeRoll)>=rpg->GetIC(),"periodic bleeding fixture seed no longer selects the native no-dodge control");
    GlobalGameRandom().SeedForHarness(2);
    std::printf("native bleeding fixture RNG: seed=2 firstRoll=%u IC=%d\n",dodgeRoll,rpg->GetIC());std::fflush(stdout);
    actor->OnTBSEvent(NWorld::TBS_FINISH_OWN_TURN);
    CObj<NWorld::CHitLocator> hit=world->GetHitEvent();
    std::printf("native periodic bleeding: strength=%g whole=%d hit=%d value=%d VP=%d->%d\n",strength,int(strength),int(IsValid(hit)),hit?hit->nHitValue:0,vp,rpg->GetTotalVP());std::fflush(stdout);
    if(fractional)Require(!hit && rpg->GetTotalVP()==vp && actor->GetAP()==ap && actor->GetPosition()==position && world->GetAimTime()->GetValue()==time,
        "zero-strength native bleeding dispatched an attack/hit event or changed the unit");
    else Require(hit && hit->pUnit.GetPtr()==actor && hit->nHitValue>0 && rpg->GetTotalVP()<vp,
        "positive whole-number bleeding control lost its native damage or hit event");
    Require(!world->GetHitEvent(),"one periodic bleeding callback emitted duplicate hit events");
}
