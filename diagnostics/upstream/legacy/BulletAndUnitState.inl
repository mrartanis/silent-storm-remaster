class BulletDecalPresentation: public NWorld::IRenderVisitor
{
public:
    vector<CObjectBase*> targets;
    int created=0,submitted=0;
    NGScene::CDecalTarget* CreateDecalTarget(const vector<CObjectBase*>& values,const NGScene::SDecalMappingInfo& info) override {
        targets=values;++created;return new NGScene::CDecalTarget(info);
    }
    void AddDecal(NGScene::CDecalTarget* target,NDb::CMaterial* material) override {Require(target && material,"native bullet decal lost its target or authored material");++submitted;}
};

class BulletImpactSurface: public CObjectBase
{
    OBJECT_NOCOPY_METHODS(BulletImpactSurface);
};

static void BulletImpactEffects(const char* databasePath,const char* resourcePath,int kind)
{
    struct BloodRestore {NGlobal::CValue saved;~BloodRestore(){NGlobal::SetVar("cheat_blood",saved);}} blood{NGlobal::GetVar("cheat_blood",1)};
    NGlobal::SetVar("cheat_blood",kind==5 || kind==6?0:1);
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;NWorld::CUnitServer* other=nullptr;
    for(const auto& unit:units)if(IsValid(unit) && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK()) {
        if(!actor && !unit->IsAIUnit())actor=unit.GetPtr();else if(!other)other=unit.GetPtr();
    }
    Require(actor && other && actor!=other,"impact fixture lacks two real native human objects");
    NDb::CEffect* effect=nullptr;CDBIterator<NDb::CEffect> effects(*NDatabase::GetTable<NDb::CEffect>());if(effects.MoveNext())effect=effects.Get();
    NDb::CSound* sound=nullptr;CDBIterator<NDb::CSound> sounds(*NDatabase::GetTable<NDb::CSound>());if(sounds.MoveNext())sound=sounds.Get();
    NDb::CMaterial* decalMaterial=nullptr;CDBIterator<NDb::CMaterial> materials(*NDatabase::GetTable<NDb::CMaterial>());
    while(materials.MoveNext())if(materials.Get()->GetRecordID()!=3411){decalMaterial=materials.Get();break;}
    NDb::CRPGMaterial* stone=nullptr;CDBIterator<NDb::CRPGMaterial> surfaces(*NDatabase::GetTable<NDb::CRPGMaterial>());
    while(surfaces.MoveNext())if(surfaces.Get()->GetRecordID()!=NDb::CRPGMaterial::HUMAN_BODY){stone=surfaces.Get();break;}
    auto* flesh=NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);
    Require(effect && sound && decalMaterial && stone && flesh && flesh->pMaterial,"impact fixture lacks its authored effect/sound/material records");
    CObj<NDb::CTEffect> effectTemplate=new NDb::CTEffect;effectTemplate->variants.push_back(effect);effectTemplate->roulette.AddSector(1.f);
    CObj<NDb::CSoundVariant> soundVariant=new NDb::CSoundVariant;soundVariant->pSound=sound;
    CObj<NDb::CTSound> soundTemplate=new NDb::CTSound;soundTemplate->variants.push_back(soundVariant.GetPtr());soundTemplate->roulette.AddSector(1.f);
    CObj<NDb::CTMaterial> materialTemplate=new NDb::CTMaterial;materialTemplate->variants.push_back(decalMaterial);materialTemplate->roulette.AddSector(1.f);
    CObj<NDb::CRPGArmor> armor=new NDb::CRPGArmor;armor->pMaterial=kind==6 || kind==7 || kind==9 || kind==11?stone:flesh->pMaterial.GetPtr();
    armor->pShotEffect=effectTemplate;armor->pSoundShot=soundTemplate;armor->pShotMaterial=materialTemplate;armor->fShotRadius=.2f;
    CObj<BulletImpactSurface> surfaceToken=new BulletImpactSurface;
    auto counts=[&] {
        std::pair<int,int> result{};
        for(const auto& object:*world->GetMiscObjects())if(IsValid(object)) {
            const int id=pSSClasses->GetObjectTypeID(object);
            if(id!=int(0x01512120) && id!=int(0x01512121))continue;
            CStructureNetworkContext refs;auto fields=BorrowedObjectFields(object,&refs);
            if(id==int(0x01512120)) {CDBPtr<NDb::CEffect> record;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&record);}if(record.GetPtr()==effect)++result.first;}
            else {CDBPtr<NDb::CSound> record;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(3,&record);}if(record.GetPtr()==sound)++result.second;}
        }
        return result;
    };
    auto decals=[&] {
        vector<CObj<NWorld::CDecal>> result;CStructureNetworkContext refs;auto fields=BorrowedObjectFields(world->GetActive(),&refs);
        for(auto& ref:refs.refs)if(auto* decal=dynamic_cast<NWorld::CDecal*>(ref.second.GetPtr()))result.push_back(decal);
        return result;
    };
    const auto beforeEffects=counts();const auto beforeDecals=decals();
    const int beforeVP=actor->GetUnitRPG()->GetTotalVP(),beforeOtherVP=other->GetUnitRPG()->GetTotalVP(),ap=actor->GetAP();
    const auto position=actor->GetPosition();const bool action=world->IsAction();
    const bool nonunit=kind==6 || kind==7 || kind==9 || kind==11;
    CObjectBase* object=nonunit?static_cast<CObjectBase*>(surfaceToken.GetPtr()):kind==4?static_cast<CObjectBase*>(other):static_cast<CObjectBase*>(actor);
    CObjectBase* receiver=nonunit || kind==3?nullptr:static_cast<CObjectBase*>(actor);
    // Original incidental evasion reduces converted damage; it does not reject the whole hit.
    // Base4/IC50 ->3 (stored-float .01), while a failed roll leaves4.
    if(kind==12){auto* unit=actor->GetUnitRPG()->GetRPGUnit();unit->Skills(NDb::ST_IC).SetConst(50);unit->Skills(NDb::ST_IC).SetProgress(0);float hitAura,evadeAura;NRPG::GetAuraAdd(&hitAura,&evadeAura,actor);Require(actor->GetUnitRPG()->GetIC()==50 && evadeAura==0,"impact order fixture lacks isolated native IC50");}
    const int baseDamage=kind==12?4:3;
    NRPG::CAttackPortion attack(kind==1?0:100000,2,0.f,baseDamage,baseDamage,0,0,nullptr,kind==2 || kind==12?nullptr:actor->GetUnitRPG(),1.f,-1,false);
    const int damage=!receiver || kind==1?0:kind==2?-1:kind==12?2:attack.CalcStructDmg(world,flesh,0);
    const CVec3 point=position.GetCP()+CVec3(0,0,1),direction(1,0,0),normal(-1,0,0);
    attack.rTtrajectory.ptOrigin=point-direction;attack.rTtrajectory.ptDir=direction;
    vector<NRPG::STrailPoint> trail;trail.emplace_back(-1,direction,point-direction,attack,nullptr,nullptr,nullptr,normal,100);
    trail.emplace_back(NAI::HL_BODY,direction,point,attack,receiver,object,kind==11?nullptr:armor.GetPtr(),normal,kind==8?100:0);
    for(;;){CObj<NWorld::CHitLocator> event=world->GetHitEvent();if(!event)break;}
    unsigned seed=2;
    if(kind==12) {
        const int ic=actor->GetUnitRPG()->GetIC();Require(ic>0 && ic<90,"impact order fixture has no bounded native dodge chance");
        CRandomGenerator reference;
        for(seed=1;seed<200000;++seed){reference.SeedForHarness(seed);if(reference.Get(100)==0 && int(reference.Get(100))>=90)break;}
        Require(seed<200000,"impact order fixture could not select distinct original dodge/visual rolls");
    }
    if(kind==2)actor->GetUnitRPG()->GetRPGUnit()->SetCheat(NRPG::CHEAT_GODMODE,true);
    GlobalGameRandom().SeedForHarness(seed);
    CObj<NWorld::IDynamicObject> bullet=NWorld::CreateBulletServer(world,trail,world->GetTime()->GetValue(),nullptr,100.f,nullptr,kind==9 || kind==10?-1:0);
    Require(bullet && bullet->Segment(),"impact effects fixture did not process and retire its actual native bullet");bullet=nullptr;
    CObj<NWorld::CHitLocator> event=world->GetHitEvent();
    std::printf("native impact damage control: kind=%d actual=%d expected=%d VP=%d->%d seed=%u\n",kind,event?event->nHitValue:-999,damage,beforeVP,actor->GetUnitRPG()->GetTotalVP(),seed);std::fflush(stdout);
    Require((receiver?event && event->pUnit.GetPtr()==actor && event->nHitValue==damage:!event) && !world->GetHitEvent() &&
        actor->GetUnitRPG()->GetTotalVP()==beforeVP-Max(0,damage) && other->GetUnitRPG()->GetTotalVP()==beforeOtherVP &&
        actor->GetAP()==ap && actor->GetPosition()==position && world->IsAction()==action,"impact effects fixture changed its native damage/HP/AP/action control");
    const auto afterEffects=counts();const auto afterDecals=decals();vector<CObj<NWorld::CDecal>> added;
    for(const auto& decal:afterDecals)if(std::none_of(beforeDecals.begin(),beforeDecals.end(),[&](const CObj<NWorld::CDecal>& old){return old.GetPtr()==decal.GetPtr();}))added.push_back(decal);
    const bool show=kind!=1 && kind!=2 && kind!=3 && kind!=4 && kind!=8 && kind!=9 && kind!=11;
    const int expectedParticles=show && kind!=5 && kind!=10?1:0,expectedSounds=show && kind!=5?1:0,expectedDecals=show?1:0;
    std::printf("native impact effects: kind=%d damage=%d particles=%d/%d sounds=%d/%d decals=%zu/%d blood=%g\n",kind,damage,
        afterEffects.first-beforeEffects.first,expectedParticles,afterEffects.second-beforeEffects.second,expectedSounds,added.size(),expectedDecals,NGlobal::GetVar("cheat_blood",1).GetFloat());std::fflush(stdout);
    Require(afterEffects.first-beforeEffects.first==expectedParticles && afterEffects.second-beforeEffects.second==expectedSounds && int(added.size())==expectedDecals,
        "native bullet emitted phantom/disabled flesh effects or lost its actual surface decal");
    for(const auto& decal:added) {
        BulletDecalPresentation presentation;decal->Visit(&presentation);
        Require(presentation.created==1 && presentation.submitted==1 && presentation.targets.size()==1 && presentation.targets.front()==object,
            "native impact decal follows the damage receiver instead of the actual surface");
    }
}

static void BulletDecalEquipmentTarget(const char* databasePath,const char* resourcePath,bool equipped,bool sceneMode = false)
{
    NativeDisplayContext display;
    if(sceneMode)display.Initialize(databasePath,resourcePath,"Silent Storm native PK decal surface regression");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units) {
        auto* candidate=dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());
        if(candidate && !candidate->IsAIUnit() && candidate->CanFight() && !candidate->IsWearingPK() && !candidate->IsEmptyPK()) {actor=candidate;break;}
    }
    Require(actor,"bullet decal fixture has no ordinary native human actor");
    CObj<NWorld::CUnitServer> shell;
    if(equipped) {
        NDb::CRPGPers* record=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
        while(people.MoveNext())if(people.Get()->pPanzerklein && !people.Get()->pPanzerklein->bHasNoHead && people.Get()->pModel) {record=people.Get();break;}
        Require(record,"bullet decal fixture lacks an authored ordinary PK persona");
        vector<NAI::SPathPlace> places;auto* network=world->GetPathNetwork();network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),20.f),&places);
        NAI::SPathPlace anchor;bool found=false;
        for(auto place:places) {place.SetPose(NAI::CM_STAND);place.SetDirection(0);if(NAI::IsBigLockerPlacePassable(network,place)) {anchor=place;found=true;break;}}
        Require(found,"bullet decal fixture lacks an actual free PK footprint");
        shell=world->AddUnitInGame(anchor,NRPG::CreateUnit(record),nullptr);
        Require(shell && shell->IsEmptyPK() && shell->WearAsPK(true),"bullet decal fixture could not prepare its native PK shell");
        actor->SetPosition(shell->GetPosition());actor->FlipPanzerklein(shell,false,false);
        Require(actor->IsWearingPK(),"bullet decal fixture did not equip its pilot through the native API");
    }
    auto* armor=(equipped?shell.GetPtr():actor)->GetUnitRPG()->GetRPGArmor();
    Require(armor && armor->pShotMaterial,"bullet decal fixture's authored armor has no impact material");
    CObj<NGScene::IGameView> view;CObj<NRender::IRenderGame> renderer;NGScene::SDecalMappingInfo sceneProbe;unsigned liveParts=0;
    if(sceneMode) {
        view=NGScene::CreateNewView();renderer=NRender::CreateRenderGame(world,view,nullptr);
        Require(view && renderer,"native decal scene could not create its actual game renderer");
        renderer->UpdateViewWorld(false,world->GetTime()->GetValue(),nullptr,true);
        sceneProbe.vCenter=actor->GetPosition().GetCP()+CVec3(0,0,1.f);sceneProbe.vNormal=VNULL3;sceneProbe.fRadius=3.5f;sceneProbe.fRotation=0;
        for(int poll=0;poll<2000 && !liveParts;++poll) {
            MarkNewDGFrame();CObj<NGScene::CDecalTarget> control=view->CreateDecalTarget(vector<CObjectBase*>{actor,shell.GetPtr()},sceneProbe);
            liveParts=unsigned(control->targetParts.size());if(!liveParts)Sleep(1);
        }
        std::printf("native decal scene control: live actor/shell mesh parts=%u\n",liveParts);std::fflush(stdout);
        Require(liveParts>0,"native decal scene has no loaded rendered body for its positive ownership control");
    }
    auto decals=[&]() {
        vector<CObj<NWorld::CDecal>> result;CStructureNetworkContext refs;auto fields=BorrowedObjectFields(world->GetActive(),&refs);
        for(auto& ref:refs.refs)if(auto* decal=dynamic_cast<NWorld::CDecal*>(ref.second.GetPtr()))result.push_back(decal);
        return result;
    };
    const auto before=decals();const int ap=actor->GetAP(),vp=actor->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();const bool action=world->IsAction();
    NRPG::CAttackPortion attack(100000,0,0.f,3,3,0,0,nullptr,actor->GetUnitRPG(),1.f,-1,false);
    auto* humanArmor=NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);int damageType=-1;
    CDBIterator<NDb::CRPGDmgToArmor> damageTypes(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());
    while(damageTypes.MoveNext()) {
        attack.nDmgType=damageTypes.Get()->GetRecordID();
        if(!attack.CanRicochet() && attack.CanDealDmg(armor) && attack.CanDealDmg(humanArmor) && attack.CalcStructDmg(world,armor,0)>0){damageType=attack.nDmgType;break;}
    }
    Require(damageType>=0,"bullet decal fixture lacks a native non-ricochet positive impact control");
    const int expectedDamage=attack.CalcStructDmg(world,humanArmor,0),suitBefore=equipped?shell->GetUnitRPG()->GetTotalVP():0;
    const int suitDamage=equipped?attack.CalcStructDmg(world,armor,0):0;
    Require(expectedDamage>0 && expectedDamage<=5 && (!equipped || suitBefore>suitDamage),"positive decal fixture would destroy its controlled body surface");
    CVec3 direction(1,0,0),normal(-1,0,0),impact=position.GetCP()+CVec3(0,0,1.f);
    if(sceneMode) {
        CObj<NGScene::CDecalTarget> surface=view->CreateDecalTarget(vector<CObjectBase*>{actor,shell.GetPtr()},sceneProbe);bool found=false;
        for(const auto& part:surface->parts) {
            vector<CVec3> positions;vector<STriangle> triangles;NGScene::TransformPart(part,&positions,&triangles);
            for(const auto& triangle:triangles) {
                const CVec3& a=positions[triangle.i1];const CVec3& b=positions[triangle.i2];const CVec3& c=positions[triangle.i3];
                normal=(b-a)^(c-a);if(fabs2(normal)<0.000001f)continue;
                Normalize(&normal);impact=(a+b+c)/3.f;direction=-normal;found=true;break;
            }
            if(found)break;
        }
        Require(found,"native bullet decal fixture lacks a real nondegenerate rendered surface triangle");
    }
    attack.rTtrajectory.ptOrigin=impact-direction;attack.rTtrajectory.ptDir=direction;
    vector<NRPG::STrailPoint> trail;
    trail.emplace_back(-1,direction,impact-direction,attack,nullptr,nullptr,nullptr,normal,0);
    trail.emplace_back(NAI::HL_BODY,direction,impact,attack,actor,actor,armor,normal,0);
    CObj<NWorld::IDynamicObject> bullet=NWorld::CreateBulletServer(world,trail,world->GetTime()->GetValue(),nullptr,100.f);
    GlobalGameRandom().SeedForHarness(2);
    Require(bullet && bullet->Segment(),"native cached positive impact did not process and retire its bullet");bullet=nullptr;
    Require(!equipped || shell->GetUnitRPG()->GetTotalVP()==suitBefore-suitDamage,"positive decal fixture lost its native suit damage control");
    const auto after=decals();CObj<NWorld::CDecal> added;unsigned count=0;
    for(const auto& decal:after)if(std::none_of(before.begin(),before.end(),[&](const CObj<NWorld::CDecal>& old){return old.GetPtr()==decal.GetPtr();})) {added=decal;++count;}
    Require(count==1 && added,"native bullet impact did not create exactly one new world decal");
    auto* expected=equipped?static_cast<CObjectBase*>(shell.GetPtr()):static_cast<CObjectBase*>(actor);
    if(sceneMode) {
        renderer->UpdateViewWorld(false,world->GetTime()->GetValue(),nullptr,true);
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(added,&refs);CObj<NGScene::CDecalTarget> attached;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(4,&attached);}
        std::printf("native actual bullet decal scene: liveParts=%u attachedParts=%u\n",liveParts,attached?unsigned(attached->targetParts.size()):0);std::fflush(stdout);
        Require(attached && !attached->targetParts.empty(),"equipment-targeted bullet decal did not bind to the actual rendered Panzerklein mesh");
        for(const auto& part:attached->targetParts)Require(part.pUser==expected,"actual bullet decal bound its persistent surface to the pilot instead of the PK");
        auto query=[&](CObjectBase* owner) {
            CObj<NGScene::CDecalTarget> result;
            for(int poll=0;poll<2000;++poll) {
                MarkNewDGFrame();result=view->CreateDecalTarget(vector<CObjectBase*>{owner},sceneProbe);
                if(!result->parts.empty())break;Sleep(1);
            }
            Require(result && !result->parts.empty(),"native decal lifecycle lost its loaded body surface");return result;
        };
        auto bindings=[](NGScene::CDecalTarget* target,CObjectBase* selectable) {
            unsigned count=0,triangles=0;
            for(const auto& part:target->parts) {
                Require(part && part->GetFullGroupInfo().pUser==selectable,"decal ownership changed the mesh's selectable gameplay owner");
                for(const auto& decal:part->decals)if(decal) {
                    ++count;auto* geometry=dynamic_cast<NGScene::CNonePart*>(decal.GetPtr());
                    Require(geometry && geometry->HasLoadedObjectInfo(),"native bullet decal has no real loaded geometry node");
                    triangles+=unsigned(geometry->GetObjectInfo()->GetTrisCount());
                }
            }
            return std::make_pair(count,triangles);
        };
        auto worn=query(shell.GetPtr());const auto wornBindings=bindings(worn,actor);
        std::printf("native worn decal geometry: nodes=%u triangles=%u\n",wornBindings.first,wornBindings.second);std::fflush(stdout);
        Require(wornBindings.first>0 && wornBindings.second>0,"native worn PK surface has no actual attached decal geometry node");
        Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==vp-expectedDamage && actor->GetPosition()==position && actor->IsWearingPK() && world->IsAction()==action,
            "native rendered positive impact changed AP/position/equipment or lost its exact damage/action control");
        // Actual pre-extension scene fields have no optional tag 5. Loading
        // them must fall back to their original selectable owner, even if a
        // reused destination still held an equipment alias before READ.
        CStructureNetworkContext oldRefs;oldRefs.externalReferences=true;oldRefs.include=[](CObjectBase*){return false;};
        oldRefs.ids[actor]=1;oldRefs.refs[1]=actor;oldRefs.nextID=100;
        auto oldGroup=worn->parts.front()->GetFullGroupInfo();CMemoryStream legacy;
        {CStructureSaver writer(legacy,CStructureSaver::WRITE,&oldRefs);writer.Add(2,&oldGroup.groupInfo);writer.Add(3,&oldGroup.pUser);writer.Add(4,&oldGroup.nUserID);}
        legacy.Seek(0);{CStructureSaver reader(legacy,CStructureSaver::READ,&oldRefs);oldGroup.operator&(reader);}
        Require(oldGroup.pUser==actor && !oldGroup.pDecalUser && oldGroup.GetDecalUser()==actor,"old scene-group fields retained a stale equipment alias instead of their original owner");
        NGScene::SFullRoomInfo oldRoom;oldRoom.pUser=actor;oldRoom.pDecalUser=shell.GetPtr();oldRoom.nUserID=7;CMemoryStream legacyRoom;
        {CStructureSaver writer(legacyRoom,CStructureSaver::WRITE,&oldRefs);writer.Add(2,&oldRoom.room);writer.Add(3,&oldRoom.pUser);writer.Add(4,&oldRoom.nUserID);}
        legacyRoom.Seek(0);{CStructureSaver reader(legacyRoom,CStructureSaver::READ,&oldRefs);oldRoom.operator&(reader);}
        Require(oldRoom.pUser==actor && !oldRoom.pDecalUser && oldRoom.nUserID==7,"old scene-room fields lost their owner/ID or retained an equipment alias");
        actor->FlipPanzerklein(nullptr,false,false);Require(shell->WearAsPK(false),"native decal lifecycle could not detach its empty PK shell");
        renderer->UpdateViewWorld(false,world->GetTime()->GetValue(),nullptr,true);
        auto detached=query(shell.GetPtr());auto human=query(actor);
        const auto shellBindings=bindings(detached,shell.GetPtr()),humanBindings=bindings(human,actor);
        std::printf("native decal ownership lifecycle: worn=%u/%u detachedShell=%u/%u human=%u/%u nodes/triangles oldGroup/room=compatible\n",wornBindings.first,wornBindings.second,shellBindings.first,shellBindings.second,humanBindings.first,humanBindings.second);std::fflush(stdout);
        Require(shellBindings.first>0 && shellBindings.second>0 && humanBindings.first==0 && !actor->IsWearingPK(),"PK bullet decal followed the unmounted pilot instead of remaining on the actual suit mesh");
        return;
    }
    BulletDecalPresentation presentation;added->Visit(&presentation);
    std::printf("native bullet decal: equipped=%d newDecals=%u targets=%u pilot=%d shell=%d AP=%d->%d VP=%d->%d\n",int(equipped),count,unsigned(presentation.targets.size()),
        int(presentation.targets.size()==1 && presentation.targets[0]==actor),int(presentation.targets.size()==1 && presentation.targets[0]==shell.GetPtr()),ap,actor->GetAP(),vp,actor->GetUnitRPG()->GetTotalVP());std::fflush(stdout);
    Require(presentation.created==1 && presentation.submitted==1 && presentation.targets.size()==1 && presentation.targets[0]==expected,
        "native bullet decal was sent to the hidden pilot instead of the worn Panzerklein surface");
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==vp-expectedDamage && actor->GetPosition()==position && actor->IsWearingPK()==equipped && world->IsAction()==action,
        "positive impact changed AP/position/equipment or lost its exact native damage/action control");
}

class BulletPerceptionProbe: public CObjectBase
{
    OBJECT_NOCOPY_METHODS(BulletPerceptionProbe);
    NGlobal::CEventRegister<BulletPerceptionProbe,NWorld::CEventOnBullet> registration;
public:
    CPtr<NWorld::CUnitServer> shooter;
    vector<NWorld::CEventOnBullet> events;
    BulletPerceptionProbe():registration(this,&BulletPerceptionProbe::OnBullet) {}
    void OnBullet(const NWorld::CEventOnBullet& event){if(event.pShooter==shooter)events.push_back(event);}
};

static void BulletNearestTarget(const char* databasePath,const char* resourcePath,int kind)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> all;world->GetAllUnits(&all);vector<CPtr<NWorld::CUnitServer>> units;
    for(const auto& unit:all)if(IsValid(unit) && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK())units.push_back(unit);
    Require(units.size()>=3,"bullet nearest-target fixture requires three live native humans");
    auto* shooter=units[0].GetPtr();auto* first=units[1].GetPtr();auto* second=units[2].GetPtr();
    CObj<BulletPerceptionProbe> listener=new BulletPerceptionProbe;listener->shooter=shooter;
    const int firstVP=first->GetUnitRPG()->GetTotalVP(),secondVP=second->GetUnitRPG()->GetTotalVP();
    const int shooterAP=shooter->GetAP();const auto shooterPosition=shooter->GetPosition();const bool action=world->IsAction();
    NRPG::CAttackPortion attack(0,2,0.f,0,0,0,0,shooter->GetUnitRPG(),first->GetUnitRPG(),1.f,-1,false);
    const CVec3 origin=shooterPosition.GetCP()+CVec3(0,0,1),direction(1,0,0),normal(-1,0,0);
    attack.rTtrajectory.ptOrigin=origin;attack.rTtrajectory.ptDir=direction;
    vector<NRPG::STrailPoint> trail;trail.emplace_back(-1,direction,origin,attack,nullptr,nullptr,nullptr,normal,100);
    if(kind==2 || kind==4)trail.emplace_back(NAI::HL_BODY,direction,origin+direction,attack,nullptr,first,first->GetUnitRPG()->GetRPGArmor(),normal,100);
    auto* firstHit=kind==2?second:first;
    if(kind!=4)trail.emplace_back(NAI::HL_BODY,direction,origin+direction*2,attack,firstHit,firstHit,firstHit->GetUnitRPG()->GetRPGArmor(),normal,100);
    if(kind==1)trail.emplace_back(NAI::HL_BODY,direction,origin+direction*3,attack,second,second,second->GetUnitRPG()->GetRPGArmor(),normal,100);
    CObj<NWorld::IDynamicObject> bullet=NWorld::CreateBulletServer(world,trail,world->GetTime()->GetValue(),nullptr,100.f);
    Require(bullet,"nearest-target fixture could not create its real native bullet");
    if(kind==3) {
        CPtr<NWorld::CUnitServer> savedTarget=second;CMemoryStream saved;
        CStructureNetworkContext refs;refs.externalReferences=true;refs.include=[](CObjectBase*){return false;};refs.ids[second]=1;refs.refs[1]=second;refs.nextID=100;
        {CStructureSaver writer(saved,CStructureSaver::WRITE,&refs);writer.Add(16,&savedTarget);}saved.Seek(0);
        {CStructureSaver reader(saved,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(bullet.GetPtr())->operator&(reader);}
    }
    for(;;){CObj<NWorld::CHitLocator> hit=world->GetHitEvent();if(!hit)break;}
    GlobalGameRandom().SeedForHarness(2);
    Require(bullet->Segment(),"nearest-target native bullet did not process its trail and retire");
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(bullet,&refs);CPtr<NWorld::CUnitServer> cached;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(16,&cached);}
    bullet=nullptr;
    const int expectedHits=kind==4?0:kind==1?2:1;
    int hits=0;for(;;){CObj<NWorld::CHitLocator> hit=world->GetHitEvent();if(!hit)break;
        Require(hit->pUnit.GetPtr()==(hits==0?firstHit:second) && hit->nHitValue<=0,"nearest-target fixture lost its actual stopped/rejected native impact control");++hits;}
    Require(hits==expectedHits && listener->events.size()==1 && listener->events.front().pShooter.GetPtr()==shooter,
        "nearest-target fixture did not execute real impacts and exactly one native bullet event");
    auto* expected=kind==4?nullptr:kind==3?second:firstHit;const auto& event=listener->events.front();
    std::printf("native bullet nearest target: kind=%d impacts=%d cached=%p event=%p expected=%p distance=%g\n",kind,hits,static_cast<void*>(cached.GetPtr()),static_cast<void*>(event.pTarget.GetPtr()),static_cast<void*>(expected),event.fDistance);std::fflush(stdout);
    Require(first->GetUnitRPG()->GetTotalVP()==firstVP && second->GetUnitRPG()->GetTotalVP()==secondVP && shooter->GetAP()==shooterAP && shooter->GetPosition()==shooterPosition && world->IsAction()==action,
        "zero-damage bullet metadata fixture changed HP, shooter AP/position or retained an action");
    Require(cached.GetPtr()==expected && event.pTarget.GetPtr()==expected,"native bullet perception omitted its first impacted unit or replaced a retained nearest target");
    const auto attackOrigin = shooter->GetAttackOrigin(shooterPosition);
    Require(fabs2(attackOrigin - origin) > 0.00001f, "bullet perception fixture needs distinct attack and trail origins");
    auto expectedDirection = trail.back().vPosition - attackOrigin;
    const float expectedDistance = fabs(expectedDirection);
    Normalize(&expectedDirection);
    Require(fabs2(event.ray.ptOrigin - attackOrigin) < 0.00001f &&
        fabs2(event.ray.ptDir - expectedDirection) < 0.00001f && fabs(event.fDistance - expectedDistance) < 0.00001f,
        "bullet perception used the stored trail origin instead of the shooter's attack origin");
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

class KnifePosePresentation: public NWorld::IRenderVisitor
{
public:
    CPtr<NDb::CModel> model;
    CPtr<CFuncBase<NAnimation::SSkeletonPose>> animation;
    int meshes = 0;
    void AddItemMesh(NDb::CModel* value,CFuncBase<NAnimation::SSkeletonPose>* pose,int) override {
        model = value; animation = pose; ++meshes;
    }
};

static void ThrownKnifeOrientation(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,false,&game);
    NDb::CRPGMeleeWeapon* record = nullptr;
    CDBIterator<NDb::CRPGMeleeWeapon> rows(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
    while (rows.MoveNext()) if (rows.Get()->bThrowing && IsValid(rows.Get()->pItem) &&
        IsValid(rows.Get()->pItem->pModel)) { record = rows.Get(); break; }
    Require(record != nullptr, "knife orientation fixture has no authored modelled throwing knife");
    SRand modelRandom; CObj<NDb::CModel> model = record->pItem->pModel->CreateModel(&modelRandom);
    Require(IsValid(model), "knife orientation fixture did not resolve its authored model pool");
    CObj<NRPG::CMeleeWeaponItem> item = new NRPG::CMeleeWeaponItem(record);
    const bool action = world->IsAction();
    const CVec3 start(11,12,13);
    const STime now = world->GetTime()->GetValue();
    // The model's forward axis is -Z. Keep the supported vertical-down identity
    // control first, then cover four horizontal headings and sloped flights.
    for (const auto& velocity: {CVec3(0,0,-10),CVec3(10,0,0),CVec3(0,10,0),CVec3(-10,0,0),
        CVec3(0,-10,0),CVec3(3,4,5),CVec3(3,-4,-5),CVec3(-7,2,-1)}) {
        NRPG::CAttackPortion attack;
        CObj<NWorld::IDynamicObject> knife = NWorld::CreateKnifeServer(world,start,velocity,
            now,100.f,model,attack,item,nullptr);
        Require(IsValid(knife), "knife orientation fixture could not construct the native projectile");
        KnifePosePresentation presentation;
        auto* visual = dynamic_cast<NWorld::IVisObj*>(knife.GetPtr());
        Require(visual != nullptr, "native knife has no render-visitor interface");
        visual->Visit(&presentation);
        Require(presentation.meshes == 1 && presentation.model == model.GetPtr() &&
            IsValid(presentation.animation), "native knife did not publish its actual model and animation");
        CDGPtr<CFuncBase<NAnimation::SSkeletonPose>> evaluated(presentation.animation);
        evaluated.Refresh();
        const auto& pose = evaluated->GetValue();
        Require(pose.size() == 1 && fabs2(pose[0].pos-start) < 1e-8f,
            "native knife orientation changed its initial position or root-pose shape");
        const CVec3 forward = pose[0].rot.Rotate(CVec3(0,0,-1));
        CVec3 expected = velocity; Normalize(&expected);
        const float error = fabs2(forward-expected);
        std::printf("native knife velocity=(%.1f,%.1f,%.1f) forward=(%.4f,%.4f,%.4f) error=%.6f\n",
            velocity.x,velocity.y,velocity.z,forward.x,forward.y,forward.z,error);
        Require(error < 1e-6f, "thrown knife model faces against its flight direction");
        knife = nullptr;
        Require(world->IsAction() == action, "inspecting knife orientation retained a projectile action");
    }
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
    ICamera* GetCamera() const override { return nullptr; }
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

static void ExplicitItemActivationVisibility(const char* databasePath,const char* resourcePath,bool movement = false)
{
    for (bool visible: {true,false})
        for (bool heavy: {false,true}) {
            CObj<NRPG::CGlobalGame> game;
            auto world = FixtureWorld(databasePath,resourcePath,true,&game);
            vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
            NWorld::CUnitServer* actor = nullptr;
            for (auto& entry: units) {
                auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
                if (unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK() &&
                    unit->animator.CanActivateItem() && unit->GetUnitRPG()->GetInventory()->GetActive()) { actor = unit; break; }
            }
            Require(actor != nullptr, "item activation fixture has no ordinary armed human");
            auto* inventory = actor->GetUnitRPG()->GetInventory();
            CObj<NRPG::IInventoryItem> source = inventory->GetActive();
            const auto oldSlot = NDb::ESlot(inventory->GetActiveSlot());
            const auto targetSlot = oldSlot == NDb::SLOT_1 ? NDb::SLOT_2 : NDb::SLOT_1;
            NDb::CRPGWeapon* record = nullptr;
            CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
            while (weapons.MoveNext()) {
                auto* weapon = weapons.Get();
                if (!IsValid(weapon->pItem) || weapon->pItem.GetPtr() == source->GetDBItem() ||
                    !IsValid(weapon->pItem->pModel) || weapon->pItem->subType == NDb::SUBTYPE_MINE_DETECTOR) continue;
                if ((weapon->pItem->subType == NDb::SUBTYPE_HEAVY) == heavy) { record = weapon; break; }
            }
            Require(record != nullptr, "item activation fixture has no distinct authored weapon for this branch");
            record->pItem->bPlaceInHand = visible;
            CObj<NRPG::CWeaponItem> item = new NRPG::CWeaponItem(record);
            CObj<NRPG::IInventoryItem> previous = inventory->Get(targetSlot) ? inventory->TakeOff(targetSlot) : nullptr;
            Require(inventory->Equip(targetSlot,item) && inventory->GetActiveSlot() == oldSlot,
                "item activation fixture changed its active slot during preparation");
            world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer()));
            Require(!world->IsRealTime() && !world->IsSequence(), "item activation fixture did not enter a native player turn");
            actor->SetWalkWithoutWeapon(true);
            const auto position = actor->GetPosition(); const int ap = actor->GetAP();
            const int ammo = item->GetAmmoQuantity();
            CObj<NWorld::CCmdSetActiveItem> request = new NWorld::CCmdSetActiveItem(targetSlot);
            NWorld::EUnitCommandResult result;
            CObj<NWorld::CCommandExecute> executor = NWorld::CreateExecutor(actor,request,&result);
            Require(executor && result == NWorld::UCR_OK, "item activation fixture could not create its native executor");
            executor->Run();
            if (movement) {
                // Reconcile the new hand through the real equipment cancellation path.
                executor->Cancel(); executor = nullptr;
                Require(inventory->GetActive() == item.GetPtr() && !world->IsAction(),
                    "movement visibility preparation lost its active hand or retained an action");
                auto activationPath = [&](NWorld::ENeedActiveItem constraint) {
                    CObj<NAI::CPath> path = new NAI::CPath;
                    path->pNet = world->GetPathNetwork();
                    path->points.push_back(actor->GetPosition().pos.p);
                    CObj<NWorld::CCommandExecute> move = NWorld::CreateSimpleMoveExecutor(actor,path,NAI::PF_DEFAULT,constraint);
                    Require(move != nullptr,"movement visibility fixture could not create its native activation path");
                    return move;
                };
                // Holster visible controls through the mover too; do not inject animator flags.
                executor = activationPath(NWorld::ITEM_INACTIVE);
                executor->Run();
                for (int callback=0;callback<3 && executor->GetState() != NWorld::CCommandExecute::FINISHED;++callback) {
                    executor->TimeLabelReached(); executor->AnimationFinished();
                }
                Require(executor->GetState() == NWorld::CCommandExecute::FINISHED && !actor->animator.IsActiveItem(),
                    "native inactive path failed to prepare a holstered item");
                executor = nullptr;
                actor->SetWalkWithoutWeapon(true);
                executor = activationPath(NWorld::ITEM_ACTIVE);
                executor->Run();
            }
            for (int callback=0;callback<3 && executor->GetState() != NWorld::CCommandExecute::FINISHED;++callback) {
                executor->TimeLabelReached(); executor->AnimationFinished();
            }
            Require(executor->GetState() == NWorld::CCommandExecute::FINISHED &&
                inventory->GetActiveSlot() == targetSlot && inventory->GetActive() == item.GetPtr(),
                "item activation did not finish its real slot-switch state machine");
            std::printf("native active-item visibility: heavy=%d authored-visible=%d animator-active=%d\n",
                int(heavy),int(visible),int(actor->animator.IsActiveItem()));
            Require(actor->animator.IsActiveItem() == visible,
                "explicit activation drew an item whose authored flag forbids showing it in hand");
            Require(!actor->IsWalkingWithoutWeapon() && inventory->Get(oldSlot) == source.GetPtr() &&
                actor->GetAP() == ap && item->GetAmmoQuantity() == ammo &&
                actor->GetPosition().pos.p == position.pos.p && actor->GetPosition().pos.pNet == position.pos.pNet,
                "visibility handling changed explicit holster cancellation, inventory, turn-based AP, ammo or position");
            executor = nullptr;
            Require(!world->IsAction(), "item activation retained its native action counter");
        }
}

static vector<int> BindingModelSignature(NDb::CModel* model)
{
    vector<int> key;
    if(!IsValid(model) || !IsValid(model->pGeometry))return key;
    key.push_back(model->pGeometry->GetRecordID());
    for(const auto& material:model->pMaterials)key.push_back(IsValid(material)?material->GetRecordID():-1);
    return key;
}

static void BackpackBoneBinding(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=4,"unknown backpack binding case");
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    if(mode==4) {
        int count=0;std::set<int> skeletonIDs;
        CDBIterator<NDb::CRPGPers> catalogue(*NDatabase::GetTable<NDb::CRPGPers>());
        while(catalogue.MoveNext()) {
            auto* p=catalogue.Get();auto* pk=p->pPanzerklein.GetPtr();
            if(!pk || !p->pClass || !p->pBaseValue || !p->pModel || p->pDefaultWearsPanzerklein ||
                !pk->pPers || !pk->pPers->pUniform || !pk->pPers->pUniform->pBackpackModel)continue;
            CObj<NRPG::IUnitMission> unit=NRPG::CreateUnit(p);auto* model=unit->GetModel();
            Require(model && model->pSkeleton,"authored PK backpack owner has no native skeletal model");
            CObj<NAnimation::CSkeletonAnimator> skeleton=new NAnimation::CSkeletonAnimator(model->pSkeleton);
            const int index=skeleton->GetBoneIndex("Slot1");
            std::printf("native PK backpack catalogue persona=%d skeleton=%d Slot1=%d\n",p->GetRecordID(),model->pSkeleton->GetRecordID(),index);
            Require(index>0,"authored PK backpack catalogue contains a skeleton without its non-root Slot1 node");
            skeletonIDs.insert(model->pSkeleton->GetRecordID());++count;
        }
        std::printf("native PK backpack catalogue checked=%d distinct-skeletons=%zu\n",count,skeletonIDs.size());std::fflush(stdout);
        Require(count>0 && !skeletonIDs.empty(),"PK backpack catalogue control found no applicable authored actors");
        return;
    }
    const bool pk=mode!=1,bodyOnly=mode==2,leftHand=mode==3;
    NDb::CPanzerklein* suit=nullptr;NDb::CRPGPers* persona=nullptr;
    CObj<NRPG::IUnitMission> mission;CObj<NAnimation::CSkeletonAnimator> animator;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext() && !persona) {
        auto* p=people.Get();auto* candidateSuit=p->pPanzerklein.GetPtr();
        if(!p->pClass || !p->pBaseValue || !p->pModel || bool(candidateSuit)!=pk || p->pDefaultWearsPanzerklein)continue;
        auto* uniform=pk && candidateSuit->pPers?candidateSuit->pPers->pUniform.GetPtr():p->pUniform.GetPtr();
        if(!uniform || !uniform->pBackpackModel || (pk && (!candidateSuit->pLeftHandItem || !candidateSuit->pLeftHandItem->pModel)))continue;
        CObj<NRPG::IUnitMission> current=NRPG::CreateUnit(p);auto* model=current->GetModel();
        if(!model || !model->pSkeleton)continue;
        CObj<NAnimation::CSkeletonAnimator> skeleton=new NAnimation::CSkeletonAnimator(model->pSkeleton);
        if(skeleton->GetBoneIndex(pk?"Slot1":"BackPack")<=0 || (leftHand && skeleton->GetBoneIndex("L_Weapon")<=0))continue;
        persona=p;suit=candidateSuit;mission=current;animator=skeleton;
    }
    Require(persona && mission && animator,"authored unit catalogue lacks a model/uniform with the required backpack skeleton node");
    auto* unit=mission->GetRPGUnit();const int hp=mission->GetTotalVP(),seed=unit->bindSeed.nSeed;
    auto* active=mission->GetInventoryInfo()->GetActive();auto* uniform=mission->GetInventoryInfo()->GetUniform();const auto itemCount=mission->GetInventoryInfo()->GetItems().size();
    auto random=GlobalGameRandom();vector<NWorld::IRenderVisitor::SBoundMesh> meshes;
    NWorld::GetItemsBindPlaces(&meshes,mission,false,suit,false,true,bodyOnly);auto afterRandom=GlobalGameRandom();
    Require(hp==mission->GetTotalVP() && unit->bindSeed.nSeed==seed && active==mission->GetInventoryInfo()->GetActive() &&
        uniform==mission->GetInventoryInfo()->GetUniform() && itemCount==mission->GetInventoryInfo()->GetItems().size() && random.Get()==afterRandom.Get(),
        "backpack binding changed native inventory, health, visual seed or gameplay ISAAC");
    if(bodyOnly){Require(meshes.empty(),"body-only no-cap pose emitted a backpack or weapon");return;}
    const char* expected=leftHand?"L_Weapon":pk?"Slot1":"BackPack";const int expectedIndex=animator->GetBoneIndex(expected);
    Require(expectedIndex>0,"authored backpack/weapon reference resolves only to the root or a missing node");
    const NWorld::IRenderVisitor::SBoundMesh* bound=nullptr;
    if(leftHand){for(const auto& mesh:meshes)if(mesh.pszBindBone && std::string(mesh.pszBindBone)==expected){bound=&mesh;break;}}
    else if(!meshes.empty())bound=&meshes.front();
    Require(bound && bound->pModel && bound->pszBindBone,"actual item renderer did not emit the authored backpack/weapon model");
    auto* templateModel=leftHand?suit->pLeftHandItem->pModel.GetPtr():(pk?suit->pPers->pUniform->pBackpackModel.GetPtr():uniform->pBackpackModel.GetPtr());
    SRand reference{mission->GetBindPlacesSeed()};
    if(leftHand){CObj<NDb::CModel> first=suit->pPers->pUniform->pBackpackModel->CreateModel(&reference);}
    CObj<NDb::CModel> expectedModel=templateModel->CreateModel(&reference);
    Require(BindingModelSignature(bound->pModel)==BindingModelSignature(expectedModel),"backpack fixture selected another native model rather than its authored uniform attachment");
    const int actualIndex=animator->GetBoneIndex(bound->pszBindBone);
    NAnimation::SBonePose point;animator->GetDefaultBonePos(expectedIndex,&point);
    std::printf("native backpack binding mode=%d PK=%d persona=%d skeleton=%d bone=%s index=%d expected=%s index=%d point=%.6g,%.6g,%.6g\n",
        mode,int(pk),persona->GetRecordID(),mission->GetModel()->pSkeleton->GetRecordID(),bound->pszBindBone,actualIndex,expected,expectedIndex,point.pos.x,point.pos.y,point.pos.z);std::fflush(stdout);
    Require(actualIndex==expectedIndex && std::string(bound->pszBindBone)==expected,
        "native PK backpack uses a missing or wrong skeleton node, so the render visitor drops the mesh");
}

static bool FindBindingModelVariants(NDb::CTRndModel* source,int* seeds,vector<int>* expected)
{
    vector<int> first;
    for(int sample=0;sample<64;++sample) {
        const int seed=sample*1234567;
        SRand random{SRandomSeed(seed)};CObj<NDb::CModel> model=source->CreateModel(&random);
        auto key=BindingModelSignature(model);if(key.empty())continue;
        if(first.empty()){first=key;seeds[0]=seed;}
        else if(first!=key){seeds[1]=seed;expected[0]=first;expected[1]=key;return true;}
    }
    return false;
}

static void BindPlacesSeed(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=2,"unknown bind-place seed case");
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    NDb::CRPGPers* persona=nullptr;int storedSeeds[2]={0,0};vector<int> expected[2];bool cap=false;
    std::set<int> fixedBackpacks,fixedCaps;int candidates=0;CObj<NRPG::IUnitMission> source;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext() && !persona) {
        auto* p=people.Get();
        if(!p->pClass || !p->pBaseValue || !p->pModel || p->pClass->skills[NDb::ST_LEVEL]<=0 ||
            p->pPanzerklein || p->pDefaultWearsPanzerklein || !p->pUniform)continue;
        if(auto* backpack=p->pUniform->pBackpackModel.GetPtr())if(!fixedBackpacks.count(backpack->GetRecordID())) {
            ++candidates;
            if(FindBindingModelVariants(backpack,storedSeeds,expected)){persona=p;source=NRPG::CreateUnit(p);break;}
            fixedBackpacks.insert(backpack->GetRecordID());
        }
        if(auto* hat=p->pUniform->pCapModel.GetPtr())if(!fixedCaps.count(hat->GetRecordID())) {
            ++candidates;
            if(!FindBindingModelVariants(hat,storedSeeds,expected)){fixedCaps.insert(hat->GetRecordID());continue;}
            CObj<NRPG::IUnitMission> candidate=NRPG::CreateUnit(p);auto* head=candidate->GetRPGUnit()->GetHeadInfo();
            if(head && IsValid(head->GetHair()))continue;
            persona=p;source=candidate;cap=true;
        }
    }
    std::printf("native bind seed mode=%d authored-templates=%d persona=%d cap=%d seeds=%d/%d\n",
        mode,candidates,persona?persona->GetRecordID():-1,int(cap),storedSeeds[0],storedSeeds[1]);std::fflush(stdout);
    Require(persona && source,"authored equipment catalogue has no visible distinct native material/model variants for the seed fixture");
    Require(source->GetInventoryInfo()->GetUniform()==persona->pUniform.GetPtr(),"bind seed fixture lost its actual authored uniform");
    const bool bodyOnly=mode==2;int mismatches=0,queries=0;
    for(int i=0;i<2;++i) {
        CObj<NRPG::IUnitMission> mission=source;
        // Configure the existing public persisted seed for each independent unit/save input.
        mission->GetRPGUnit()->bindSeed=SRandomSeed(storedSeeds[i]);
        if(mode==1) {
            CObj<NRPG::CUnit> unit=mission->GetRPGUnit();CMemoryStream saved;
            {CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(1,&unit);}
            saved.Seek(0);CObj<NRPG::CUnit> restored;
            {CStructureSaver reader(saved,CStructureSaver::READ);reader.Add(1,&restored);}
            Require(restored && restored.GetPtr()!=unit.GetPtr() && restored->bindSeed.nSeed==storedSeeds[i],
                "ordinary native save/load lost the existing per-unit bind seed");
            mission=NRPG::CreateUnit(restored);
        }
        for(unsigned clockSeed:{11u,47u,180u,902u}) {
            auto* unit=mission->GetRPGUnit();const int hp=mission->GetTotalVP();
            auto* active=mission->GetInventoryInfo()->GetActive();const auto count=mission->GetInventoryInfo()->GetItems().size();auto* head=unit->GetHeadInfo();
            GlobalGameRandom().SeedForHarness(clockSeed);auto before=GlobalGameRandom();
            vector<NWorld::IRenderVisitor::SBoundMesh> meshes;
            NWorld::GetItemsBindPlaces(&meshes,mission,false,nullptr,false,bodyOnly || !cap,bodyOnly);
            auto after=GlobalGameRandom();
            Require(before.Get()==after.Get() && unit->bindSeed.nSeed==storedSeeds[i] && unit->GetHeadInfo()==head &&
                mission->GetTotalVP()==hp && mission->GetInventoryInfo()->GetActive()==active &&
                mission->GetInventoryInfo()->GetUniform()==persona->pUniform.GetPtr() && mission->GetInventoryInfo()->GetItems().size()==count,
                "render attachment selection changed gameplay ISAAC, saved seed, head, health or inventory");
            if(bodyOnly)Require(meshes.empty(),"body-only no-cap pose emitted an equipment mesh");
            else {
                Require(!meshes.empty() && meshes.front().pszBindBone && std::string(meshes.front().pszBindBone)==(cap?"Cap":"BackPack"),
                    "native bind fixture did not emit the actual authored equipment as its first attachment");
                auto actual=BindingModelSignature(meshes.front().pModel);const bool matches=actual==expected[i];mismatches+=!matches;
                std::printf("native bind seed mode=%d unit=%d stored=%d clock=%u geometry=%d expected=%d matches=%d\n",
                    mode,i,storedSeeds[i],clockSeed,actual.empty()?-1:actual[0],expected[i][0],int(matches));
            }
            ++queries;
        }
    }
    std::printf("native bind seed result mode=%d queries=%d mismatches=%d saved=%d\n",mode,queries,mismatches,int(mode==1));std::fflush(stdout);
    Require(mismatches==0,"native rendered equipment ignores the unit's persisted bind seed and follows the frame clock seed");
}

static void NonUniformAttachment(const char* databasePath,const char* resourcePath,int mode)
{
    const bool uniform=mode>=7 && mode<=10 || mode==12 || mode==15,heavy=mode==2 || mode==3 || mode==9,gun=mode==1 || mode==8 || mode==13;
    const bool inactive=mode==3 || mode==5 || mode==9,hidden=mode==4 || mode==10,empty=mode==6 || mode==14 || mode==15,detach=mode>=11;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* anchor=nullptr;for(auto entry:units){auto* unit=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());if(unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()){anchor=unit;break;}}Require(anchor,"nonuniform attachment has no real native owner anchor");
    NDb::CRPGPers* persona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext()){auto* p=people.Get();if(!p->pClass || !p->pBaseValue || !p->pModel || p->pClass->skills[NDb::ST_LEVEL]<=0 || p->pPanzerklein || p->pDefaultWearsPanzerklein || bool(p->pUniform)!=uniform)continue;if(!persona || p->GetRecordID()<persona->GetRecordID())persona=p;}
    Require(persona,"nonuniform attachment lacks an actual authored persona with the required uniform state");CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(persona);CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(anchor->GetPosition().pos.p,mission,anchor->GetTBSPlayer());Require(actor && actor->CanFight() && !actor->IsWearingPK() && !actor->IsEmptyPK(),"nonuniform attachment cannot deploy its real ordinary authored actor");
    auto* inventory=mission->GetInventory();vector<CObj<NRPG::IInventoryItem>> previous;for(auto slot:{NDb::SLOT_1,NDb::SLOT_2})if(auto* item=inventory->TakeOff(slot))previous.push_back(item);const auto bag=inventory->GetItems();for(const auto& item:bag){previous.push_back(item.pItem.GetPtr());inventory->Take(item.pItem);}Require(bool(inventory->GetUniform())==uniform,"nonuniform attachment changed the authored saved uniform");
    NDb::CRPGWeapon* weaponRecord=nullptr;CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());while(weapons.MoveNext()){auto* w=weapons.Get();if(!w->pItem || !w->pItem->pModel || !w->pItem->bPlaceInHand || !w->pAnimWeaponType || bool(w->pItem->subType==NDb::SUBTYPE_HEAVY)!=heavy)continue;const auto type=w->pAnimWeaponType->type;if(heavy && type!=NDb::WT_RIFLE && type!=NDb::WT_SUB_MACHINE_GUN && type!=NDb::WT_MACHINE_GUN && type!=NDb::WT_RLAUNCHER && type!=NDb::WT_PLAZMAGUN)continue;if(!weaponRecord || w->GetRecordID()<weaponRecord->GetRecordID())weaponRecord=w;}
    NDb::CRPGGrenade* grenadeRecord=nullptr;CDBIterator<NDb::CRPGGrenade> grenades(*NDatabase::GetTable<NDb::CRPGGrenade>());while(grenades.MoveNext()){auto* g=grenades.Get();if(g->pItem && g->pItem->pModel && g->pItem->bPlaceInHand && (g->pItem->subType==NDb::SUBTYPE_GRENADE_SMALL || g->pItem->subType==NDb::SUBTYPE_GRENADE_LARGE) && (!grenadeRecord || g->GetRecordID()<grenadeRecord->GetRecordID()))grenadeRecord=g;}
    Require(weaponRecord && grenadeRecord,"nonuniform attachment lacks modelled authored ordinary/heavy weapons and grenade");CObj<NRPG::CWeaponItem> weapon;CObj<NRPG::CGrenadeItem> grenade;CObj<NRPG::IInventoryItem> item;
    if(!empty){if(heavy || gun){weapon=new NRPG::CWeaponItem(weaponRecord);item=weapon;}else{grenade=new NRPG::CGrenadeItem(grenadeRecord);grenade->nQuantity=1;item=grenade;}Require(inventory->Equip(NDb::SLOT_1,item),"nonuniform attachment cannot equip its real item");}Require(inventory->Activate(inactive?NDb::SLOT_2:NDb::SLOT_1),"nonuniform attachment cannot set its actual selected hand");
    actor->SetUndrawItem(false);actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(mission->GetWeaponType());MarkNewDGFrame();world->GetAIMap()->Sync();
    std::string expected;
    if(!empty){if(heavy)expected=NWorld::GetBoneName(NWorld::UIT_WEAPON_HEAVY,item);else if(!inactive && !hidden)expected="Item";else if(uniform){const int place=inventory->GetPlaceBySubType(item->GetDBItem()->subType);if(place>=0)expected=NWorld::GetBoneName(static_cast<NWorld::EUnitItemType>(NWorld::UIT_BELT_L1+place),item);}}
    const auto position=actor->GetPosition();const int ap=actor->GetAP(),hp=mission->GetTotalVP(),ammo=weapon?weapon->GetAmmoQuantity():0,quantity=grenade?grenade->nQuantity:0;auto* actualUniform=inventory->GetUniform();auto random=GlobalGameRandom();const std::string bone=NWorld::GetBoneName(NDb::SLOT_1,mission,hidden),repeat=NWorld::GetBoneName(NDb::SLOT_1,mission,hidden);
    vector<NWorld::IRenderVisitor::SBoundMesh> meshes;NWorld::GetItemsBindPlaces(&meshes,mission,hidden,nullptr);int matching=0;for(const auto& mesh:meshes)if(mesh.pszBindBone && !expected.empty() && expected==mesh.pszBindBone)++matching;
    std::printf("native nonuniform attachment: mode=%d uniform=%d persona=%d item=%d heavy=%d inactive=%d hidden=%d empty=%d detach=%d bone=%s repeat=%s expected=%s matchingMeshes=%d AP=%d HP=%d ammo=%d quantity=%d\n",mode,int(uniform),persona->GetRecordID(),item?item->GetDBItem()->GetRecordID():0,int(heavy),int(inactive),int(hidden),int(empty),int(detach),bone.empty()?"<empty>":bone.c_str(),repeat.empty()?"<empty>":repeat.c_str(),expected.empty()?"<empty>":expected.c_str(),matching,ap,hp,ammo,quantity);std::fflush(stdout);
    if(!detach){auto afterRandom=GlobalGameRandom();Require(actor->GetAP()==ap && mission->GetTotalVP()==hp && actor->GetPosition()==position && inventory->GetUniform()==actualUniform && inventory->Get(NDb::SLOT_1)==item.GetPtr() && inventory->GetActiveSlot()==(inactive?NDb::SLOT_2:NDb::SLOT_1) && (weapon?weapon->GetAmmoQuantity():0)==ammo && (grenade?grenade->nQuantity:0)==quantity && random.Get()==afterRandom.Get(),"nonuniform attachment query changed actual unit/item/uniform/AP/HP/RNG");if(!expected.empty() && !hidden)Require(matching==1,"native body binding control does not contain the authored selected item bone");Require(bone==expected && repeat==expected,"slot attachment wrongly requires a uniform for a held or heavy item");return;}
    CVec3 expectedPosition=VNULL3;CQuat expectedRotation;int index=-1;if(!empty){auto* skeleton=actor->animator.GetSkeletonAnimator();Require(skeleton,"nonuniform attachment has no actual body animator");index=skeleton->GetBoneIndex("Item");Require(index>0,"nonuniform attachment actor lacks the real non-root hand bone");actor->GetBonePos(&expectedPosition,&expectedRotation,"Item");}
    NWorld::CUnitServer::SResItem removed;const bool detached=actor->TearOffItem(&removed,NDb::SLOT_1);auto afterRandom=GlobalGameRandom();const bool positioned=detached && std::isfinite(removed.ptCenter.x) && std::isfinite(removed.ptCenter.y) && std::isfinite(removed.ptCenter.z) && fabs(removed.ptCenter-expectedPosition)<.001f;
    std::printf("native nonuniform detachment: mode=%d success=%d expected=%d model=%d itemIdentity=%d emptySlot=%d handBone=%d positioned=%d AP=%d HP=%d\n",mode,int(detached),int(!empty),int(IsValid(removed.pModel)),int(removed.pItem.GetPtr()==item.GetPtr()),int(inventory->Get(NDb::SLOT_1)==nullptr),index,int(positioned),actor->GetAP(),mission->GetTotalVP());std::fflush(stdout);
    Require(!inventory->Get(NDb::SLOT_1) && removed.pItem.GetPtr()==item.GetPtr() && actor->GetAP()==ap && mission->GetTotalVP()==hp && actor->GetPosition()==position && inventory->GetUniform()==actualUniform && (weapon?weapon->GetAmmoQuantity():0)==ammo && (grenade?grenade->nQuantity:0)==quantity && random.Get()==afterRandom.Get(),"native detachment changed more than the intended equipment slot or lost the real item");
    Require(detached==!empty && IsValid(removed.pModel)==!empty && (empty || positioned),"uniformless held item detaches without its real model and hand-bone transform");
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

using PointVisionCache = unordered_map<NRPG::SVisionQuery, bool, NRPG::SVisionQueryHash>;
static PointVisionCache PointVisionEntries(NRPG::IVisionTracker* vision)
{
    CStructureNetworkContext refs;
    auto fields = BorrowedObjectFields(vision, &refs);
    PointVisionCache entries;
    { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(6, &entries); }
    return entries;
}

static void PointVisionCaching(int mode, const char* databasePath, const char* resourcePath)
{
    Require(mode >= 0 && mode <= 2, "unknown point vision cache mode");
    if (mode < 2) {
        CObj<NRPG::IVisionTracker> vision = dynamic_cast<NRPG::IVisionTracker*>(pSSClasses->CreateObject(0x00413190));
        Require(vision, "native vision tracker class is not registered");
        const CVec3 from(1,1,100), to(2,1,100), forward(1,0,0);
        if (mode == 0) {
            Require(vision->IsPointVisible(from,to,forward,10,.5f), "empty-grid point ray unexpectedly blocked");
            auto entries = PointVisionEntries(vision);
            std::printf("native point cache accepted ray: entries=%zu expected=1\n",entries.size()); std::fflush(stdout);
            NRPG::SVisionQuery query; query.vFrom=from; query.vWhat=to; query.fRange=10; query.fCosFOV=.5f;
            Require(entries.size()==1 && entries.count(query)==1 && entries.at(query), "accepted point ray was not retained with its full query key");
            Require(vision->IsPointVisible(from,to,CVec3(2,0,0),10,.5f) && PointVisionEntries(vision).size()==1,
                "an equivalent accepted direction changed the cached result or query identity");
            Require(!vision->IsPointVisible(from,to,CVec3(-1,0,0),10,.5f) && PointVisionEntries(vision).size()==1,
                "a cached visible point bypassed the caller's current FOV gate");
            Require(!vision->IsPointVisible(from,to,forward,1,.5f) && PointVisionEntries(vision).size()==1,
                "an equal-range rejected point changed the cache");
            Require(vision->IsPointVisible(from,CVec3(3,1,100),forward,10,.5f), "second empty-grid point ray unexpectedly blocked");
            Require(vision->IsPointVisible(from,to,forward,20,.5f), "changed-range point ray unexpectedly blocked");
            Require(vision->IsPointVisible(from,to,forward,10,.75f), "changed-FOV point ray unexpectedly blocked");
            Require(PointVisionEntries(vision).size()==4, "target, effective range or FOV was omitted from cache identity");
            vision->SetVisionMultiplier(1.5f);
            Require(PointVisionEntries(vision).size()==4, "unchanged capped vision multiplier discarded cached points");
            vision->SetVisionMultiplier(.5f);
            Require(PointVisionEntries(vision).empty(), "changed vision multiplier left stale point queries");
            Require(vision->IsPointVisible(from,to,forward,10,.5f), "scaled empty-grid point ray unexpectedly blocked");
            query.fRange=5;
            entries=PointVisionEntries(vision);
            Require(entries.size()==1 && entries.count(query)==1 && entries.at(query), "scaled query did not use the effective sight distance");
            // Full ordinary graph -> a NEW tracker; never restore borrowed fields into a live object.
            CMemoryStream snapshot;
            { CStructureSaver writer(snapshot,CStructureSaver::WRITE); writer.Add(1,&vision); }
            snapshot.Seek(0);
            CObj<NRPG::IVisionTracker> loaded;
            { CStructureSaver reader(snapshot,CStructureSaver::READ); reader.Add(1,&loaded); }
            Require(loaded && loaded.GetPtr()!=vision.GetPtr(), "point cache save did not create an independent native tracker");
            const auto restored=PointVisionEntries(loaded);
            Require(restored.size()==1 && restored.count(query)==1 && restored.at(query), "ordinary save lost the populated point cache or effective range");
            Require(loaded->IsPointVisible(from,to,forward,10,.5f) && PointVisionEntries(loaded).size()==1,
                "restored point cache changed its visible result or query identity");
            loaded->SetVisionMultiplier(.25f);
            Require(PointVisionEntries(loaded).empty() && PointVisionEntries(vision).size()==1,
                "restored tracker did not invalidate its own independent cache");
        } else {
            auto query=[&](int index) {return vision->IsPointVisible(from,CVec3(2.f+index*.00001f,1,100),forward,10,.5f);};
            for (int i=0;i<15000;++i) Require(query(i), "bounded-cache empty-grid ray unexpectedly blocked");
            auto entries=PointVisionEntries(vision);
            std::printf("native point cache 15000 distinct rays: entries=%zu expected=15000\n",entries.size()); std::fflush(stdout);
            Require(entries.size()==15000, "distinct accepted point queries were not retained");
            Require(query(15000) && PointVisionEntries(vision).size()==15001, "point cache cleared before the original strict threshold");
            Require(query(15001) && PointVisionEntries(vision).size()==1, "point cache did not clear and retain the new result after its size exceeded 15000");
            Require(query(0) && PointVisionEntries(vision).size()==2, "a query discarded by the cache limit was not recomputed");
        }
        return;
    }
    Require(databasePath && resourcePath, "point cache geometry test requires assets");
    auto world=FixtureWorld(databasePath,resourcePath,false,nullptr,4517);
    auto* map=world->GetAIMap(); map->Sync();
    auto* vision=world->GetGame()->GetVisionTracker();
    struct Ray {CVec3 from,to,forward;};
    NWorld::CWindowDoor* selected=nullptr; Ray selectedRay; bool originalOpen=false;
    bool originalLocked=false; int originalKey=0,originalHardness=0,candidates=0;
    for (const auto& object:world->GetObjectsForHarness()) {
        auto* door=dynamic_cast<NWorld::CWindowDoor*>(object.GetPtr());
        CVec3 closed,opened;
        if (!door || door->IsBroken() || door->IsMineSet() || !map->GetWindowPos(door,&closed,&opened)) continue;
        const bool initialOpen=door->IsOpen(),initialLocked=door->IsLockedDoor();
        const int initialKey=door->GetKeyID(),initialHardness=door->GetLockHardness();
        door->LockDoor(false,0,0); ++candidates;
        vector<Ray> rays;
        for (float height:{-.5f,0.f,.5f}) for (int angle=0;angle<16;++angle) {
            const float radians=angle*6.28318530718f/16;
            const CVec3 delta(std::cos(radians),std::sin(radians),0), center=closed+CVec3(0,0,height);
            rays.push_back({center-delta*.8f,center+delta*.8f,delta});
        }
        door->OpenClose(false,true); map->Sync();
        vector<bool> blocked;
        for (const auto& ray:rays) blocked.push_back(!vision->IsPointVisible(ray.from,ray.to,ray.forward,10,-1));
        door->OpenClose(true,true); map->Sync();
        if(candidates<=4){std::printf("point cache door candidate=%d record=%d closedPos=%g,%g,%g blocked=%d/48\n",candidates,door->GetDBObjectID(),double(closed.x),double(closed.y),double(closed.z),int(std::count(blocked.begin(),blocked.end(),true)));std::fflush(stdout);}
        for (std::size_t i=0;i<rays.size();++i) {
            const auto& ray=rays[i];
            if (blocked[i] && vision->IsPointVisible(ray.from,ray.to,ray.forward,10,-1)) {
                selected=door;selectedRay=ray;originalOpen=initialOpen;originalLocked=initialLocked;originalKey=initialKey;originalHardness=initialHardness;break;
            }
        }
        if (selected) break;
        door->OpenClose(initialOpen,true); map->Sync();
        door->LockDoor(initialLocked,initialKey,initialHardness);
    }
    std::printf("point cache door searched candidates=%d selected=%d\n",candidates,int(selected!=nullptr));std::fflush(stdout);
    Require(selected, "native door fixture has no real point ray blocked closed and visible open");
    selected->OpenClose(false,true); map->Sync();
    Require(PointVisionEntries(vision).empty(), "closing the sampled door did not invalidate point queries");
    auto query=[&](){return vision->IsPointVisible(selectedRay.from,selectedRay.to,selectedRay.forward,10,-1);};
    Require(!query(), "selected native closed door did not block its point ray");
    auto entries=PointVisionEntries(vision);
    std::printf("native point cache door record=%d closed=0 open=1 cachedEntries=%zu expected=1\n",selected->GetDBObjectID(),entries.size());std::fflush(stdout);
    Require(entries.size()==1 && !entries.begin()->second, "blocked point ray was not cached");
    Require(!query() && PointVisionEntries(vision).size()==1, "repeated closed-door point query changed its result or identity");
    selected->OpenClose(true,true); map->Sync();
    Require(PointVisionEntries(vision).empty(), "opening the native door retained stale blocked point results");
    Require(query(), "opening the native door did not recompute point visibility");
    entries=PointVisionEntries(vision);
    Require(entries.size()==1 && entries.begin()->second, "recomputed open-door point ray was not cached as visible");
    selected->OpenClose(false,true); map->Sync();
    Require(PointVisionEntries(vision).empty() && !query(), "closing the door retained a stale visible point result");
    selected->OpenClose(originalOpen,true); map->Sync();
    selected->LockDoor(originalLocked,originalKey,originalHardness);
}

static void VisionDestinationVoxel(const char* databasePath, const char* resourcePath, int mode)
{
    Require(mode>=0 && mode<=6, "unknown vision destination mode");
    const int axis=mode==6?0:mode/2, sign=mode==6?1:(mode%2?1:-1);
    auto world=FixtureWorld(databasePath,resourcePath);
    auto* map=world->GetAIMap();map->Sync();
    auto* vision=world->GetGame()->GetVisionTracker();
    const float step=FP_GRID_STEP/4.f,cubeWidth=step*32.f;
    CTPoint3<int> target,observer;
    bool found=false;
    list<NAI::SObjectInfo> geometry;map->GetEntities(&geometry,NWorld::TS_VISION);
    Require(!geometry.empty(), "destination fixture has no native vision geometry");
    for(const auto& object:geometry) {
        if(object.points.empty())continue;
        CVec3 center(0,0,0);for(const auto& point:object.points)center+=point;center/=float(object.points.size());
        const int gx=Clamp(int(std::floor(center.x/cubeWidth)),0,31),gy=Clamp(int(std::floor(center.y/cubeWidth)),0,31);
        for(int gz=0;gz<4 && !found;++gz) {
            NAI::CVisionVoxelRenderer renderer;
            renderer.Init(CVec3((gx+.5f)*cubeWidth,(gy+.5f)*cubeWidth,(gz+.5f)*cubeWidth),step*34.f,34,NWorld::TS_VISION_SOLID);
            map->TraceVisionGrid(&renderer,NWorld::TS_VISION);
            for(int x=1;x<31 && !found;++x)for(int y=1;y<31 && !found;++y)for(int z=1;z<31;++z) {
                if(!(renderer.voxels[x+1][y+1][z+1]&0x80))continue;
                CTPoint3<int> adjacent(x,y,z);adjacent.m[axis]+=sign;
                if(renderer.voxels[adjacent.x+1][adjacent.y+1][adjacent.z+1]&0xc0)continue;
                const CTPoint3<int> occupied(gx*32+x,gy*32+y,gz*32+z),empty(gx*32+adjacent.x,gy*32+adjacent.y,gz*32+adjacent.z);
                if(vision->GetVision(occupied.x,occupied.y,occupied.z)!=NRPG::VVS_SOLID ||
                   vision->GetVision(empty.x,empty.y,empty.z)!=NRPG::VVS_NONE)continue;
                target=occupied;observer=empty;found=true;break;
            }
        }
        if(found)break;
    }
    Require(found, "native destination fixture has no independently rasterized SOLID/NONE boundary for the requested axis and direction");
    CVec3 from,to,beyond;
    auto through=target;through.m[axis]-=sign;
    vision->GetCenter(observer,&from);vision->GetCenter(target,&to);vision->GetCenter(through,&beyond);
    CTPoint3<int> roundTrip;vision->GetCoord(to,&roundTrip);
    Require(roundTrip.x==target.x && roundTrip.y==target.y && roundTrip.z==target.z,
        "destination fixture did not preserve actual voxel centers");
    const CVec3 forward=to-from;
    Require(!vision->IsPointVisible(from,beyond,forward,20,-1), "solid geometry inside the ray stopped blocking visibility");
    Require(!vision->IsPointVisible(to,from,-forward,20,-1), "solid observer voxel stopped blocking a nonzero ray");
    Require(vision->IsPointVisible(to,to,forward,20,-1), "zero-length point query lost its existing visible result");
    if(mode==6) {
        Require(!vision->IsPointVisible(from,to,-forward,20,.5f), "destination handling bypassed the existing FOV gate");
        Require(!vision->IsPointVisible(from,to,forward,step,-1), "equal-range horizontal point query bypassed its strict range gate");
        std::printf("native destination controls axis=%d sign=%d interior=blocked observer=blocked zeroLength=visible FOV/range=rejected\n",axis,sign);std::fflush(stdout);
        return;
    }
    const bool visible=vision->IsPointVisible(from,to,forward,20,-1);
    std::printf("native vision destination axis=%d sign=%d from=%d,%d,%d NONE -> target=%d,%d,%d SOLID visible=%d expected=1; interior/start controls passed\n",
        axis,sign,observer.x,observer.y,observer.z,target.x,target.y,target.z,int(visible));std::fflush(stdout);
    Require(visible,"a solid destination voxel incorrectly hid its own boundary");
}

static void VisionVoxelQuery(int mode, const char* databasePath, const char* resourcePath)
{
    Require(mode == 0 || mode == 1, "unknown vision voxel query mode");
    if (mode == 0) {
        // The registered default object has an empty grid, as before full save restoration.
        CObj<NRPG::IVisionTracker> vision = dynamic_cast<NRPG::IVisionTracker*>(pSSClasses->CreateObject(0x00413190));
        Require(vision, "native vision tracker class is not registered");
        CTPoint3<int> position(7, 19, 31), roundTrip;
        CVec3 center;
        vision->GetCenter(position, &center);
        vision->GetCoord(center, &roundTrip);
        Require(roundTrip.x == 7 && roundTrip.y == 19 && roundTrip.z == 31,
            "vision tracker coordinate fixture did not initialize");
        const CTPoint3<int> outside[] = {{-1,0,0},{0,-1,0},{0,0,-1},{1024,0,0},{0,1024,0},{0,0,128}};
        for (const auto& point : outside) {
            std::printf("native public GetVision boundary query %d,%d,%d\n", point.x, point.y, point.z); std::fflush(stdout);
            Require(vision->GetVision(point.x, point.y, point.z) == NRPG::VVS_NONE,
                "out-of-grid vision voxel did not return empty");
        }
        return;
    }
    Require(databasePath && resourcePath, "native vision voxel query requires assets");
    auto world = FixtureWorld(databasePath, resourcePath);
    auto* map = world->GetAIMap();
    map->Sync();
    auto* vision = world->GetGame()->GetVisionTracker();
    Require(vision, "world has no native vision tracker");
    const float step = FP_GRID_STEP / 4.f, cubeWidth = step * 32.f;
    CTPoint3<int> solid;
    bool found = false;
    NRPG::EVoxelVisionState expected = NRPG::VVS_NONE;
    list<NAI::SObjectInfo> geometry;
    map->GetEntities(&geometry, NWorld::TS_VISION);
    Require(!geometry.empty(), "native map fixture has no vision geometry");
    for (const auto& object : geometry) {
        if (object.points.empty()) continue;
        CVec3 center(0,0,0);
        for (const auto& point : object.points) center += point;
        center /= float(object.points.size());
        const int gx = Clamp(int(std::floor(center.x / cubeWidth)), 0, 31);
        const int gy = Clamp(int(std::floor(center.y / cubeWidth)), 0, 31);
        for (int gz = 0; gz < 4 && !found; ++gz) {
            // Independent native map rasterization, with the cache's one-voxel border.
            NAI::CVisionVoxelRenderer renderer;
            renderer.Init(CVec3((gx + .5f)*cubeWidth, (gy + .5f)*cubeWidth, (gz + .5f)*cubeWidth),
                step * 34.f, 34, NWorld::TS_VISION_SOLID);
            map->TraceVisionGrid(&renderer, NWorld::TS_VISION);
            for (int x = 0; x < 32 && !found; ++x)
                for (int y = 0; y < 32 && !found; ++y)
                    for (int z = 0; z < 32; ++z)
                        if (renderer.voxels[x+1][y+1][z+1] & 0xc0) {
                            expected = (renderer.voxels[x+1][y+1][z+1] & 0x80) ? NRPG::VVS_SOLID : NRPG::VVS_TRANSPARENT;
                            solid = CTPoint3<int>(gx*32+x, gy*32+y, gz*32+z);
                            found = true; break;
                        }
        }
        if (found) break;
    }
    Require(found, "native map fixture has no independently rasterized occupied voxel");
    std::printf("native public GetVision occupied query %d,%d,%d; independent map rasterizer expects %d\n",
        solid.x, solid.y, solid.z, int(expected)); std::fflush(stdout);
    Require(vision->GetVision(solid.x, solid.y, solid.z) == expected,
        "public vision query lost a solid voxel from actual map geometry");
    Require(vision->GetVision(-1,0,0) == NRPG::VVS_NONE, "out-of-grid query did not clear the previous cube selection");
    Require(vision->GetVision(solid.x, solid.y, solid.z) == expected,
        "returning to a previously selected cube lost its solid voxel");
}

class RetiredSupportFixture: public CObjectBase
{
    OBJECT_BASIC_METHODS(RetiredSupportFixture);
};

// Observe the existing serialized weak list; never deserialize into the live script.
static vector<CObjectBase*> ScriptMiscEntries(NScript::CScript* script)
{
    CStructureNetworkContext refs;
    auto fields = BorrowedObjectFields(script, &refs);
    list<CPtr<CObjectBase>> entries;
    { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(4, &entries); }
    vector<CObjectBase*> result;
    for (const auto& entry : entries) result.push_back(IsValid(entry) ? entry.GetPtr() : nullptr);
    return result;
}

static void ScriptMiscWeakCleanup(int mode)
{
    Require(mode >= 0 && mode <= 3, "unknown script miscellaneous-reference mode");
    CObj<NScript::CScript> script = NScript::CreateScript(nullptr);
    Require(ScriptMiscEntries(script).empty(), "new script has miscellaneous entries");
    if (mode == 0) {
        CObj<RetiredSupportFixture> expired = new RetiredSupportFixture;
        CPtr<CObjectBase> former = expired.GetPtr();
        script->AddMiscObject(expired);
        Require(ScriptMiscEntries(script) == vector<CObjectBase*>{expired.GetPtr()}, "live tracking failed before expiration");
        expired = nullptr;
        Require(!IsValid(former), "miscellaneous tracking gained strong ownership");
        CObj<RetiredSupportFixture> live = new RetiredSupportFixture;
        script->AddMiscObject(live);
        const auto entries = ScriptMiscEntries(script);
        std::printf("native script misc expired-then-live: entries=%zu expected=1\n", entries.size()); std::fflush(stdout);
        Require(entries == vector<CObjectBase*>{live.GetPtr()}, "new script insertion retained an expired weak entry");
    } else if (mode == 1) {
        script->AddMiscObject(nullptr);
        const auto empty = ScriptMiscEntries(script);
        std::printf("native script misc null insertion: entries=%zu expected=0\n", empty.size()); std::fflush(stdout);
        Require(empty.empty(), "script tracked a null miscellaneous handle");
        CObj<RetiredSupportFixture> expired = new RetiredSupportFixture;
        script->AddMiscObject(expired);
        expired = nullptr;
        script->AddMiscObject(nullptr);
        Require(ScriptMiscEntries(script).empty(), "null insertion did not sweep earlier expired handles");
    } else if (mode == 2) {
        CObj<NWorld::CUICmdPlaySound> sound = new NWorld::CUICmdPlaySound;
        CObj<NWorld::CUICmdPlayEffect> effect = new NWorld::CUICmdPlayEffect;
        CObj<RetiredSupportFixture> particle = new RetiredSupportFixture;
        CPtr<CObjectBase> observedParticle = particle.GetPtr();
        effect->pHandle = particle.GetPtr();
        particle = nullptr;
        script->AddMiscObject(sound);
        script->AddMiscObject(effect);
        script->AddMiscObject(sound);
        Require(ScriptMiscEntries(script) == vector<CObjectBase*>{sound.GetPtr(), effect.GetPtr(), sound.GetPtr()},
            "sweeping changed live command identity, order, or duplicate entries");
        Require(IsValid(observedParticle) && effect->pHandle.GetPtr() == observedParticle.GetPtr(),
            "tracking released the effect command's own particle handle");
        CPtr<CObjectBase> observedSound = sound.GetPtr(), observedEffect = effect.GetPtr();
        sound = nullptr;
        effect = nullptr;
        Require(!IsValid(observedSound) && !IsValid(observedEffect) && !IsValid(observedParticle),
            "miscellaneous tracking extended command or particle ownership");
    } else {
        for (int i = 0; i < 1024; ++i) {
            CObj<RetiredSupportFixture> command = new RetiredSupportFixture;
            script->AddMiscObject(command);
        }
        CObj<RetiredSupportFixture> current = new RetiredSupportFixture;
        script->AddMiscObject(current);
        const auto entries = ScriptMiscEntries(script);
        std::printf("native script misc 1024 retired handles: entries=%zu expected=1\n", entries.size()); std::fflush(stdout);
        Require(entries == vector<CObjectBase*>{current.GetPtr()}, "retired script handles accumulated over repeated insertion");
        script = nullptr;
        Require(IsValid(current), "destroying the script invalidated an independently owned object");
    }
}

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
        world->networkHitChance = [&](int kind, NWorld::CUnit* actor, CObjectBase*, CVec3, int, bool) {
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

static void SnipeTileRestriction(const char* databasePath,const char* resourcePath)
{
    auto world = FixtureWorld(databasePath,resourcePath);
    vector<CPtr<NWorld::CUnit>> units; world->GetAllUnits(&units);
    NDb::CRPGWeapon* rifle = nullptr;
    CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while (weapons.MoveNext()) {
        auto* record = weapons.Get();
        if (record->shootModes[NDb::SM_Snipe] && record->shootModes[NDb::SM_Snap] && !record->bBazookaLogic &&
            IsValid(record->pItem) && IsValid(record->pInnerClip) && IsValid(record->pWeaponType)) { rifle = record; break; }
    }
    Require(rifle != nullptr,"sniper tile fixture has no authored rifle supporting snap and snipe");
    NWorld::CUnitServer* actor = nullptr;
    NWorld::CUnitServer* target = nullptr;
    CObj<NRPG::CWeaponItem> weapon;
    CVec3 point;
    for (auto entry: units) {
        auto* unit = dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if (!unit || !unit->CanFight() || unit->IsEmptyPK() || unit->IsWearingPK() || unit->IsSniping()) continue;
        weapon = new NRPG::CWeaponItem(rifle);
        auto* inventory = unit->GetUnitRPG()->GetInventory();
        Require(inventory->Equip(NDb::SLOT_1,weapon) && inventory->Activate(NDb::SLOT_1) &&
            weapon->SetShootMode(NDb::SM_Snap) && weapon->HasAmmo(),"sniper tile fixture failed to equip its native rifle");
        bool tileAccepted = false;
        for (const auto offset: {CVec3(2,0,0.5f),CVec3(0,2,0.5f),CVec3(-2,0,0.5f),CVec3(0,-2,0.5f)}) {
            point = unit->GetPosition().GetCP()+offset;
            CObj<NWorld::CCmdShootTile> command = new NWorld::CCmdShootTile(point);
            NWorld::EUnitCommandResult result;
            CObj<NWorld::CCommandExecute> executor = NWorld::CreateExecutor(unit,command,&result);
            if (executor && result == NWorld::UCR_OK) { tileAccepted = true; break; }
        }
        if (!tileAccepted) continue;
        Require(weapon->SetShootMode(NDb::SM_Snipe),"sniper tile fixture could not select snipe mode");
        for (auto other: units) {
            auto* candidate = dynamic_cast<NWorld::CUnitServer*>(other.GetPtr());
            if (!candidate || candidate == unit || !candidate->CanFight() || candidate->GetPlayer() != unit->GetPlayer()) continue;
            CObj<NWorld::CCmdShootObject> command = new NWorld::CCmdShootObject(candidate,0);
            NWorld::EUnitCommandResult result;
            CObj<NWorld::CCommandExecute> executor = NWorld::CreateExecutor(unit,command,&result);
            if (executor && result == NWorld::UCR_OK) { actor = unit; target = candidate; break; }
        }
        if (actor) break;
    }
    Require(actor && target,"sniper tile fixture lacks its accepted snap-tile and snipe-unit controls");
    Require(!actor->IsSniping() && NWorld::GetActionType(actor) == NWorld::AT_SHOOT,
        "sniper tile fixture already entered the held-aim state");
    const int ap = actor->GetAP(), ammo = weapon->GetAmmoQuantity();
    const auto position = actor->GetPosition();
    const auto saved = actor->GetUnitRPG()->GetSavedAP();
    CObj<NWorld::CCmdShootTile> tile = new NWorld::CCmdShootTile(point);
    CObj<NWorld::CUnitStateSniping> held = new NWorld::CUnitStateSniping(actor,target,30,NAI::HL_BODY);
    NWorld::EUnitCommandResult heldResult;
    CObj<NWorld::CCommandExecute> heldTile = held->CreateExecutor(tile,&heldResult);
    Require(!heldTile && heldResult == NWorld::UCR_UNAVAILABLE,
        "existing held-sniper state no longer rejects tile attacks");
    // The real held-state constructor seeds saved aim even without installation.
    actor->GetUnitRPG()->SaveAP(saved);
    std::puts("native sniper tile controls: snap-tile accepted, snipe-unit accepted, held-state tile unavailable");
    for (bool replaceReload: {false,true}) {
        tile->bCanBeReplacedByReload = replaceReload;
        NWorld::EUnitCommandResult result;
        CObj<NWorld::CCommandExecute> executor = NWorld::CreateExecutor(actor,tile,&result);
        std::printf("native pre-aim sniper tile: replacement-reload=%d result=%d executor=%d\n",
            int(replaceReload),int(result),int(executor != nullptr));
        Require(!executor && result == NWorld::UCR_UNAVAILABLE,
            "sniper fire mode accepted a tile attack before entering the held-aim state");
    }
    Require(actor->GetAP() == ap && weapon->GetAmmoQuantity() == ammo && !actor->IsSniping() &&
        actor->GetPosition().pos.p == position.pos.p && actor->GetPosition().pos.pNet == position.pos.pNet &&
        actor->GetUnitRPG()->GetSavedAP().pTarget == saved.pTarget &&
        actor->GetUnitRPG()->GetSavedAP().nAP == saved.nAP && !world->IsAction(),
        "checking sniper target eligibility changed AP, ammo, position, aim state or the native action counter");
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

static void PersonClueCreation(const char* databasePath, const char* resourcePath, bool withInventoryClue = false, int reserveMode = -1)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> game = NRPG::CreateGlobalGame(1);
    if(reserveMode>=0) {
        Require(reserveMode<=3 && game->pDifficulty && game->pDifficulty->fClueDeathCoeff>0,"clue reserve fixture lacks its positive authored difficulty coefficient");
        if(reserveMode==1)game->pDifficulty->fClueDeathCoeff=0;
    }
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
        unit->bClueSlot = reserveMode!=3 && IsValid(unit->pMonster) && !IsValid(unit->pMonster->pPanzerklein);
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
    if(reserveMode==3)requested.clear();
    world->CreateRandom(810, {}, true, requested, reserveMode==2?10:0, &post, SRandomSeed(123));
    vector<CPtr<NWorld::CUnit>> actors;
    world->GetAllUnits(&actors);
    int found = 0, ordinary = 0;
    for (auto& actor : actors) {
        auto* server = dynamic_cast<NWorld::CUnitServer*>(actor.GetPtr());
        if (reserveMode!=3 && actor->GetRPG()->GetRPGUnit()->GetPers()->GetRecordID() == clue->GetDBClue()->nPersID) {
            ++found;
            if(reserveMode>=0) {
                auto* unit=actor->GetRPG()->GetRPGUnit();const int maximum=unit->Skills(NDb::ST_VP).GetMaxValue();
                const float coefficient=game->pDifficulty->fClueDeathCoeff;const int expected=int(-coefficient*maximum);
                std::printf("native clue death reserve mode=%d pers=%d level=%d VP=%d coefficient=%.6g reserve=%d/%d\n",reserveMode,unit->GetPers()->GetRecordID(),int(unit->Skills(NDb::ST_LEVEL)),maximum,coefficient,unit->GetDeathVP(),expected);std::fflush(stdout);
                Require(expected==(reserveMode==1?0:expected) && (reserveMode==1 || expected<0),"native clue reserve fixture has no nonzero death threshold control");
                Require(unit->GetDeathVP()==expected,"spawned person clue did not receive the authored death reserve after final level/VP initialization");
                CObj<NRPG::CUnit> original=unit;CMemoryStream saved;
                {CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(2,&original);}saved.Seek(0);
                CObj<NRPG::CUnit> restored;{CStructureSaver reader(saved,CStructureSaver::READ);reader.Add(2,&restored);}
                Require(restored && restored.GetPtr()!=unit && restored->GetDeathVP()==expected && restored->Skills(NDb::ST_VP).GetMaxValue()==maximum,"fresh full RPG-unit save/load lost the clue death reserve or final VP cap");
                restored->Skills(NDb::ST_VP).SetValue(expected+1);Require(!restored->IsDead(),"clue reserve ended above its native death boundary");
                restored->Skills(NDb::ST_VP).SetValue(expected);Require(restored->IsDead(),"clue reserve survived its inclusive native death boundary");
                Require(unit->GetDeathVP()==expected && !unit->IsDead() && unit->Skills(NDb::ST_VP)>0 && server->IsClueUnit() && server->nClueCount==1,"boundary/save checks changed the real clue's health or marker");
            }
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
            ++ordinary;
            Require(!server->IsClueUnit(), "ordinary actor was marked as a scenario person");
            if(reserveMode>=0)Require(actor->GetRPG()->GetRPGUnit()->GetDeathVP()==0,"ordinary non-slot actor acquired a person-clue death reserve");
        }
    }
    if(reserveMode>=0){std::printf("native clue reserve population mode=%d clues=%d ordinary=%d\n",reserveMode,found,ordinary);std::fflush(stdout);}
    Require(found == (reserveMode==3?0:1), "fixture did not spawn the requested number of scenario people");
    if(reserveMode==3)Require(ordinary>0,"normal non-slot control has no actual native ordinary actors");
}
