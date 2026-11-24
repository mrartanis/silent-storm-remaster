static void NativeFragmentSpeed(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=4,"unknown fragment speed mode");CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,false,&campaign);
    NDb::CRPGGrenade* grenade=nullptr;CDBIterator<NDb::CRPGGrenade> grenades(*NDatabase::GetTable<NDb::CRPGGrenade>());while(grenades.MoveNext()){auto* g=grenades.Get();if(g->nFragmentNumber>20 && g->fFragmentRange>0 && (!grenade || g->nFragmentNumber<grenade->nFragmentNumber || (g->nFragmentNumber==grenade->nFragmentNumber && g->GetRecordID()<grenade->GetRecordID())))grenade=g;}NDb::CRPGEngGrenade* eng=nullptr;CDBIterator<NDb::CRPGEngGrenade> engineers(*NDatabase::GetTable<NDb::CRPGEngGrenade>());while(engineers.MoveNext()){auto* g=engineers.Get();if(g->fFragmentRange>0 && (!eng || g->GetRecordID()<eng->GetRecordID()))eng=g;}Require(grenade && eng,"fragment speed fixture lacks authored fragment records");
    list<CPtr<NWorld::CUnitServer>> all;world->GetAllUnits(&all);NWorld::CUnitServer* anchor=nullptr;for(auto u:all)if(u && u->CanFight() && !u->IsEmptyPK() && !u->IsWearingPK()){anchor=u;break;}Require(anchor,"fragment speed fixture lacks native anchor");const bool engineering=mode==2 || mode==3,remote=mode==1 || mode==3;const int skill=160,count=mode==4?0:engineering?skill/5:grenade->nFragmentNumber;const float range=(engineering?eng->fFragmentRange:grenade->fFragmentRange)*FP_GRID_STEP;CVec3 center=anchor->GetPosition().GetCP();if(remote)center.z+=1000;list<CPtr<NWorld::CUnitServer>> nearby;world->GetUnitsNear(center,&nearby,range);const int targeted=mode==4?0:(std::min)(int(nearby.size()),count-20);Require(mode==4 || (remote?targeted==0:targeted>0),"fragment speed fixture lacks the declared targeted/random branch inputs");
    std::set<NWorld::IDynamicObject*> old;for(const auto& object:*world->GetMiscObjects())old.insert(object.GetPtr());NWorld::SPerkMineModifiers modifiers;modifiers.fAEDmgModifier=0;CObj<NWorld::CExplosionSpace> space=NWorld::CreateExplosionSpace(world->GetAIMap());CObj<NWorld::CVoxelExplTracker> tracker=new NWorld::CVoxelExplTracker(space,center,nullptr,mode==4 || engineering?nullptr:grenade,nullptr,modifiers,world,engineering?eng:nullptr,skill);
    int created=0,correct=0,nonempty=0;for(const auto& object:*world->GetMiscObjects())if(!old.count(object.GetPtr())){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(object,&refs);float speed=-999;CDBPtr<NDb::CModel> model;vector<NRPG::STrailPoint> trail;CPtr<NWorld::CUnitServer> shooter;int effect=999;CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(3,&speed);reader.Add(8,&model);reader.Add(11,&trail);reader.Add(13,&shooter);reader.Add(15,&effect);Require(!model && !shooter && effect==-1,"fragment speed changed own shooterless model/particle inputs");++created;correct+=speed==3.f;nonempty+=!trail.empty();std::printf("native fragment speed mode=%d bullet=%d speed=%.9g expected=3 points=%zu\n",mode,created,speed,trail.size());}
    std::printf("native fragment speed summary mode=%d grenade=%d engineer=%d count=%d targeted=%d random=%d created=%d correct=%d nonempty=%d\n",mode,grenade->GetRecordID(),eng->GetRecordID(),count,targeted,count-targeted,created,correct,nonempty);std::fflush(stdout);Require(created==count && correct==count && nonempty==count,"native fragment constructor did not retain original speed3 in each real bullet");
}

static void NativePlainRayRoute(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=5,"unknown plain ray mode");CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);auto* map=world->GetAIMap();map->Sync();list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* target=nullptr;CRay ray;NAI::SSourceInfo firstSource;int location=-1;float range=4.f;bool ricochetDraw=false;int missing=0,insideCount=0,empty=0,wrong=0,armorCount=0,angleCount=0;
    const CVec3 directions[]={CVec3(1,0,0),CVec3(0,1,0),CVec3(-1,0,0),CVec3(0,-1,0)};
    for(auto unit:units){if(!unit || !unit->CanFight() || unit->IsEmptyPK() || unit->IsWearingPK() || unit->GetUnitRPG()->GetRPGUnit()->IsCheatEnabled(NRPG::CHEAT_GODMODE))continue;CVec3 body;if(!map->GetUnitHLPos(&body,map->GetHull(unit),NAI::HL_BODY)){++missing;continue;}for(const auto& direction:directions){CRay candidate;candidate.ptDir=direction;candidate.ptOrigin=body-direction*2.f;vector<NAI::SInterval> intervals;map->Trace(candidate,&intervals,NWorld::TS_FRAGMENTED);const NAI::SInterval* first=nullptr;bool inside=false;for(const auto& interval:intervals){if(interval.exit.fT<.01f || interval.enter.fT>range)continue;if(interval.enter.fT<=.01f){inside=true;break;}if(!first || interval.enter.fT<first->enter.fT)first=&interval;}if(inside){++insideCount;continue;}if(!first){++empty;continue;}if(first->pSrc->pUserData!=unit.GetPtr()){++wrong;continue;}if(!first->pSrc->pArmor || !first->pSrc->pArmor->pMaterial){++armorCount;continue;}const bool angle=-direction*first->enter.ptNormal<first->pSrc->pArmor->fRicochetMaxCos;if(angle && first->pSrc->pArmor->fRicochetProbability!=0){++angleCount;continue;}ricochetDraw=angle;target=unit;ray=candidate;firstSource=*first->pSrc;location=first->nUserID;break;}if(target)break;}
    std::printf("native plain ray geometry selection mode=%d found=%d missing=%d inside=%d empty=%d wrong=%d armor=%d angle=%d\n",mode,int(target!=nullptr),missing,insideCount,empty,wrong,armorCount,angleCount);std::fflush(stdout);Require(target && location>=0,"plain ray fixture lacks an actual isolated positive-enter human interval without ricochet");int damageType=-1;CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());while(types.MoveNext())if(types.Get()->armors[firstSource.pArmor->pMaterial->nDR]>0 && (damageType<0 || types.Get()->GetRecordID()<damageType))damageType=types.Get()->GetRecordID();Require(damageType>=0,"plain ray fixture lacks actual flesh damage type");if(mode==3)ray.ptOrigin.z+=1000;NRPG::CAttackPortion attack(0,damageType,0.f,0,0,0,0,nullptr,nullptr,1.f,-1);attack.atkType=mode==2?NRPG::AT_FRAGMENT:NRPG::AT_NORMAL;vector<NRPG::IAttackable*> ignores;if(mode==1)ignores.push_back(target);const int hp=target->GetUnitRPG()->GetTotalVP(),ap=target->GetAP();const bool action=world->IsAction();std::set<NWorld::IDynamicObject*> before;for(const auto& item:*world->GetMiscObjects())before.insert(item.GetPtr());const int draws=mode==0 || mode==2?1+int(ricochetDraw):0;CRandomGenerator reference;reference.SeedForHarness(777);if(draws){if(ricochetDraw)reference.Get();reference.Get(100);}GlobalGameRandom().SeedForHarness(777);
    if(mode>=4){NRPG::SAttackRayInfo cached(attack,ray.ptOrigin,ray.ptDir,true,0,range,nullptr);if(mode==5){world->GetGame()->ProcessRangedAttackPortion(attack,ray,ignores,&cached.trailPoints,range);Require(!cached.trailPoints.empty(),"cached hit control lacks a real native populated trail");cached.vOrigin.z+=1000;}else Require(cached.trailPoints.empty(),"native empty cached hit unexpectedly has points");world->PerformRangedAttack(cached,world->GetTime()->GetValue(),nullptr,3.f);}else world->PerformRangedAttack(attack,ray,ignores,world->GetTime()->GetValue(),nullptr,3.f,range);auto actual=GlobalGameRandom();bool same=true;unsigned actualFirst=0,expectedFirst=0;for(int i=0;i<4;++i){unsigned a=actual.Get(),b=reference.Get();if(i==0){actualFirst=a;expectedFirst=b;}same&=a==b;}
    CObj<NWorld::IDynamicObject> bullet;int created=0;for(const auto& item:*world->GetMiscObjects())if(!before.count(item.GetPtr())){bullet=item;++created;}Require(created==1 && bullet,"plain ray did not create exactly one actual native bullet");CStructureNetworkContext refs;auto fields=BorrowedObjectFields(bullet,&refs);vector<NRPG::STrailPoint> trail;CDBPtr<NDb::CModel> model;CPtr<NWorld::CUnitServer> shooter;float speed=0;int effect=0;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(3,&speed);reader.Add(8,&model);reader.Add(11,&trail);reader.Add(13,&shooter);reader.Add(15,&effect);}int entries=0;for(const auto& point:trail)if(point.pAttackTarget.GetPtr()==target)++entries;const bool excluded=mode==1 || mode==3 || mode==4;std::printf("native plain ray mode=%d persona=%d location=%d attack=%d expectedDraws=%d RNG=%d next=%u expected=%u points=%zu entries=%d expectedEntries=%d speed=%.9g effect=%d\n",mode,target->GetUnitRPG()->GetRPGPersID(),location,int(attack.atkType),draws,int(same),actualFirst,expectedFirst,trail.size(),entries,int(!excluded),speed,effect);std::fflush(stdout);Require(same,"plain ray used legacy cached-hit fallback instead of original loose trace incidental draw");Require((mode==4?trail.empty():!trail.empty()) && entries==int(!excluded) && speed==3 && !model && !shooter && effect==-1,"native cached-hit trail or loose-ray hit/ignore/range/speed/shooterless metadata was lost");
    Require(bullet->Segment(),"zero-model plain ray bullet did not retire through its actual Segment");world->GetMiscObjects()->remove(bullet);bullet=nullptr;Require(target->GetUnitRPG()->GetTotalVP()==hp && target->GetAP()==ap && target->CanFight() && world->IsAction()==action,"zero-kinetic plain ray changed victim health/AP/life or retained its action");
}

// Both branches call the real world factory. The pre-port branch exercises its
// existing scalar API; the carrier API becomes available with the actual port.
template<class T>
static auto NativeSpawnKnifeCarrier(T* world,const NRPG::SAttackRayInfo& info,float speed,STime time,float range,NDb::CModel* model,NRPG::IInventoryItem* item,bool* full,int)
    -> decltype(world->ThrowKnife(info,speed,time,range,model,item))
{
    *full=true;return world->ThrowKnife(info,speed,time,range,model,item);
}
template<class T>
static CObjectBase* NativeSpawnKnifeCarrier(T* world,const NRPG::SAttackRayInfo& info,float speed,STime time,float range,NDb::CModel* model,NRPG::IInventoryItem* item,bool* full,long)
{
    *full=false;auto portion=info.atk;return world->ThrowKnife(info.vOrigin,info.vDir*speed,time,range,model,portion,item,info.pUS);
}

static void NativeKnifeFlightIntent(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=4,"unknown native knife flight-intent mode");const CVec3 direction(0,0,-1),stale=direction;const int baseDamage=10;CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* owner=nullptr;for(auto unit:units)if(unit && unit->CanFight() && unit->GetTBSPlayer() && !unit->IsEmptyPK() && !unit->IsWearingPK()){owner=unit;break;}Require(owner,"knife direction fixture lacks actual native owner");auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(owner->GetPosition().GetCP(),20.f),&places);NAI::SPathPlace from,to;bool found=false;for(auto a:places){a.SetPose(NAI::CM_STAND);a.SetMoving(0);if(!network->IsPassable(a) || network->IsLocked(a))continue;for(auto b:places){b.SetPose(NAI::CM_CROUCH);b.SetMoving(0);const auto separation=fabs(NAI::SPosition(a,network).GetCP()-NAI::SPosition(b,network).GetCP());if(network->IsPassable(b) && !network->IsLocked(b) && separation>5.f && separation<8.f){from=a;to=b;found=true;break;}}if(found)break;}Require(found,"knife direction fixture lacks separate clear native standing/crouch cells");
    CObj<NRPG::IUnitMission> attacker=NRPG::CreateUnit(NDb::GetPers(103)),victim=NRPG::CreateUnit(NDb::GetPers(103));attacker->GetRPGUnit()->Skills(NDb::ST_LEVEL).SetConst(0);for(auto mission:{attacker.GetPtr(),victim.GetPtr()}){mission->GetRPGUnit()->Skills(NDb::ST_VP).SetConst(500);mission->GetRPGUnit()->nHealedVP=0;mission->GetRPGUnit()->Skills(NDb::ST_IC).SetConst(0);mission->GetRPGUnit()->Skills(NDb::ST_IC).SetProgress(0);}CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(from,attacker,owner->GetTBSPlayer(),"NativeKnifeDirectionActor"),target=world->AddUnitInGame(to,victim,owner->GetTBSPlayer(),"NativeKnifeDirectionTarget");Require(actor && target && target->CanFight() && target->GetPose()==NAI::CROUCH,"knife direction fixture cannot deploy actual crouching receiver");auto* difficulty=campaign->pDifficulty.GetPtr();Require(difficulty,"knife direction fixture lacks actual native difficulty");struct RestoreDifficulty{NDb::CDBDifficulty* value;float ours,enemy;~RestoreDifficulty(){value->fOurDamageMult=ours;value->fEnemyDamageMult=enemy;}}restoreDifficulty{difficulty,difficulty->fOurDamageMult,difficulty->fEnemyDamageMult};difficulty->fOurDamageMult=difficulty->fEnemyDamageMult=1.f;world->GetAIMap()->Sync();CVec3 head;Require(world->GetAIMap()->GetUnitHLPos(&head,world->GetAIMap()->GetHull(target),NAI::HL_HEAD),"knife direction fixture lacks actual native head locator");const float height=head.z-target->GetPosition().GetCP().z;std::printf("native knife direction setup mode=%d pose=%d headHeight=%.9g\n",mode,int(target->GetPose()),height);std::fflush(stdout);Require(height>.65f && height<1.25f,"knife direction fixture head is outside controlled native floor-collision height");CRay ray;ray.ptOrigin=head+CVec3(0,0,.6f);ray.ptDir=direction;vector<NAI::SInterval> intervals;world->GetAIMap()->Trace(ray,&intervals,NWorld::TS_FRAGMENTED|NWorld::TS_ITEM_BLOCKER);const NAI::SInterval* first=nullptr;for(const auto& interval:intervals){if(interval.exit.fT<=.001f)continue;Require(interval.enter.fT>.001f,"knife direction launch starts inside an actual native blocker");if(!first || interval.enter.fT<first->enter.fT)first=&interval;}auto* flesh=NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);Require(first && first->pSrc->pUserData.GetPtr()==target.GetPtr() && first->nUserID==NAI::HL_HEAD && first->pSrc->pArmor==flesh && first->enter.fT<1.f && flesh->pShotMaterial,"knife direction fixture lacks a real first head/flesh hit before floor");int type=-1;CDBIterator<NDb::CRPGDmgToArmor> damageTypes(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());while(damageTypes.MoveNext())if(damageTypes.Get()->armors[flesh->pMaterial->nDR]>0 && (type<0 || damageTypes.Get()->GetRecordID()<type))type=damageTypes.Get()->GetRecordID();Require(type>=0,"knife direction fixture lacks actual flesh damage type");NRPG::CAttackPortion portion(100000,type,0.f,baseDamage,baseDamage,0,0,attacker,victim,1.f,-1);portion.rTtrajectory.ptOrigin=ray.ptOrigin;portion.rTtrajectory.ptDir=stale;Require(portion.CalcStructDmg(world,flesh,0)==baseDamage,"knife direction fixture lost constant native damage control");

    const bool intended=mode<2,expectedHit=mode==1 || mode>=3;
    actor->SetScriptToHit(mode==2?0:100);
    NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> blades(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());
    while(blades.MoveNext())if(blades.Get()->bThrowing && blades.Get()->pItem && blades.Get()->pItem->pModel){record=blades.Get();break;}
    Require(record,"flight intent lacks authored modelled blade");SRand modelRandom;CObj<NDb::CModel> model=record->pItem->pModel->CreateModel(&modelRandom);CObj<NRPG::CMeleeWeaponItem> item=new NRPG::CMeleeWeaponItem(record);item->nQuantity=1;
    NRPG::SAttackRayInfo info(portion,ray.ptOrigin,direction,mode==4?nullptr:actor.GetPtr(),actor->GetPosition(),0,0,mode==1,0,10.f,intended?target.GetPtr():actor.GetPtr());info.pIgnore=actor.GetPtr();
    const STime now=world->GetTime()->GetValue();struct RestoreTime{CCTime* clock;STime value;~RestoreTime(){clock->Set(value);MarkNewDGFrame();}}restoreTime{world->GetTime(),now};
    const int hp=victim->GetTotalVP(),actorHP=attacker->GetTotalVP(),ap=actor->GetAP();const auto position=actor->GetPosition();
    const int chance=NRPG::GetBulletToHit(info,target,NAI::HL_HEAD);Require(chance==(mode==2?0:100),"flight intent lacks native forced incidental chance control");
    GlobalGameRandom().SeedForHarness(2026);auto untouchedRandom=GlobalGameRandom();auto expectedRandom=GlobalGameRandom();const auto roll=expectedRandom.Get(100);Require(roll>0,"incidental-miss seed drew inclusive zero");
    bool full=false;CObj<CObjectBase> spawned=NativeSpawnKnifeCarrier(world.GetPtr(),info,10.f,now,10.f,model,item,&full,0);CObj<NWorld::IDynamicObject> knife=dynamic_cast<NWorld::IDynamicObject*>(spawned.GetPtr());Require(knife && !knife->Segment() && victim->GetTotalVP()==hp,"flight intent applied damage before actual flight");
    world->GetTime()->Set(now+100);MarkNewDGFrame();const bool retired=knife->Segment();vector<CPtr<CObjectBase>> ignored;{CStructureNetworkContext refs;auto data=BorrowedObjectFields(knife,&refs);CStructureSaver reader(data,CStructureSaver::READ,&refs);reader.Add(12,&ignored);}
    std::printf("KNIFE-FLIGHT-INTENT mode=%d api=%d intended=%d rolledHit=%d chance=%d roll=%u damage=%d retired=%d ignored=%zu expectedHit=%d\n",mode,int(full),int(intended),int(info.bTargetIsHit),chance,unsigned(roll),hp-victim->GetTotalVP(),int(retired),ignored.size(),int(expectedHit));std::fflush(stdout);
    Require(expectedHit?(retired && hp-victim->GetTotalVP()==15):(!retired && victim->GetTotalVP()==hp),"real knife flight lost intended miss or incidental unit roll");
    if(!expectedHit){auto actual=GlobalGameRandom();if(intended){for(int i=0;i<4;++i)Require(actual.Get()==untouchedRandom.Get(),"missed intended target consumed an incidental roll");}else{for(int i=0;i<4;++i)Require(actual.Get()==expectedRandom.Get(),"incidental miss did not use exactly one native roll before effects");Require(ignored.size()==1 && ignored.front().GetPtr()==target.GetPtr(),"missed incidental unit was not remembered once");}auto before=GlobalGameRandom();world->GetTime()->Set(now+110);MarkNewDGFrame();Require(!knife->Segment() && victim->GetTotalVP()==hp,"second body interval damaged the missed native unit");auto after=GlobalGameRandom();for(int i=0;i<4;++i)Require(before.Get()==after.Get(),"second native body interval rerolled an ignored unit");}
    Require(actor->GetAP()==ap && attacker->GetTotalVP()==actorHP && actor->GetPosition()==position && target->CanFight() && item->GetQuantity()==1,"flight intent changed actor/AP/HP/pose/item controls");
}

static void NativeKnifeDoorFrame(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode==0 || mode==1,"unknown knife frame mode");auto world=FixtureWorld(databasePath,resourcePath,false,nullptr,4517);auto* map=world->GetAIMap();map->Sync();CObj<NWorld::CWindowDoor> door;CRay chosen;float travel=0;int flags=0,dr=-1,candidates=0,located=0,firstDoors=0;
    for(auto object:world->GetObjectsForHarness()){
        auto* candidate=dynamic_cast<NWorld::CWindowDoor*>(object.GetPtr());CVec3 closed,opened;if(!candidate || candidate->IsBroken() || candidate->IsMineSet() || !map->GetWindowPos(candidate,&closed,&opened))continue;
        ++candidates;candidate->OpenClose(mode!=0,true);map->Sync();CVec3 center;if(!map->GetUnitHLPos(&center,map->GetHull(candidate),NAI::HL_ANY))continue;++located;
        for(float height:{0.f,-.3f,.3f})for(int angle=0;angle<16 && !door;++angle){const float a=angle*FP_2PI/16;CRay ray;ray.ptDir=CVec3(cos(a),sin(a),0);ray.ptOrigin=center+CVec3(0,0,height)-ray.ptDir*.8f;vector<NAI::SInterval> hits;map->Trace(ray,&hits,NWorld::TS_FRAGMENTED|NWorld::TS_ITEM_BLOCKER);const NAI::SInterval* first=nullptr;for(const auto& hit:hits)if(hit.exit.fT>0 && (!first || hit.enter.fT<first->enter.fT))first=&hit;
            if(first && first->pSrc->pUserData.GetPtr()==candidate){++firstDoors;if(firstDoors<=12)std::printf("KNIFE-FRAME-CANDIDATE mode=%d record=%d t=%.9g flags=%x DR=%d\n",mode,candidate->GetDBObjectID(),first->enter.fT,first->pSrc->nTSFlags,first->pSrc->pArmor && first->pSrc->pArmor->pMaterial?first->pSrc->pArmor->pMaterial->nDR:-1);}
            if(!first || first->enter.fT<=.01f || first->enter.fT>=1.5f || first->pSrc->pUserData.GetPtr()!=candidate || !(first->pSrc->nTSFlags&(mode?NWorld::TS_STATE_OPEN:NWorld::TS_STATE_CLOSED)) || !first->pSrc->pArmor || !first->pSrc->pArmor->pMaterial)continue;const int material=first->pSrc->pArmor->pMaterial->nDR;if(material!=0 && material!=1 && material!=2 && material!=10)continue;door=candidate;chosen=ray;travel=first->enter.fT+.05f;flags=first->pSrc->nTSFlags;dr=material;
        }if(door)break;
    }
    std::printf("KNIFE-FRAME-SETUP mode=%d found=%d flags=%x DR=%d travel=%.9g candidates=%d located=%d firstDoor=%d\n",mode,int(bool(door)),flags,dr,travel,candidates,located,firstDoors);std::fflush(stdout);Require(door,"knife frame lacks authored active stickable door intersection");
    NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> blades(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());while(blades.MoveNext())if(blades.Get()->bThrowing && blades.Get()->pItem && blades.Get()->pItem->pModel){record=blades.Get();break;}Require(record,"knife frame lacks authored modelled blade");SRand random;CObj<NDb::CModel> model=record->pItem->pModel->CreateModel(&random);CObj<NRPG::CMeleeWeaponItem> item=new NRPG::CMeleeWeaponItem(record);item->nQuantity=1;NRPG::CAttackPortion portion;portion.nK=100000;portion.nDmgMin=portion.nDmgMax=0;NRPG::SAttackRayInfo info(portion,chosen.ptOrigin,chosen.ptDir,false,0,10.f,nullptr);
    const STime now=world->GetTime()->GetValue();struct RestoreTime{CCTime* clock;STime value;~RestoreTime(){clock->Set(value);MarkNewDGFrame();}}restore{world->GetTime(),now};bool full=false;CObj<CObjectBase> spawned=NativeSpawnKnifeCarrier(world.GetPtr(),info,10.f,now,10.f,model,item,&full,0);CObj<NWorld::IDynamicObject> knife=dynamic_cast<NWorld::IDynamicObject*>(spawned.GetPtr());Require(knife && !knife->Segment() && !world->GetFrozenItem(item),"knife frame changed state before its physical advance");world->GetTime()->Set(now+STime(ceil(travel*100)));MarkNewDGFrame();const bool retired=knife->Segment();const bool frozen=world->GetFrozenItem(item)!=nullptr;
    std::printf("KNIFE-FRAME mode=%d api=%d retired=%d frozen=%d broken=%d quantity=%d\n",mode,int(full),int(retired),int(frozen),int(door->IsBroken()),item->GetQuantity());std::fflush(stdout);Require(retired && !frozen && !door->IsBroken() && item->GetQuantity()==1,"knife embedded in a door frame instead of bouncing");
}

static void NativeKnifeImpactDirection(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown native knife impact-direction mode");const CVec3 direction(0,0,-1),stale=mode==0?CVec3(0,0,1):mode==1?CVec3(1,0,0):direction;const int baseDamage=mode==3?2:10;CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* owner=nullptr;for(auto unit:units)if(unit && unit->CanFight() && unit->GetTBSPlayer() && !unit->IsEmptyPK() && !unit->IsWearingPK()){owner=unit;break;}Require(owner,"knife direction fixture lacks actual native owner");auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(owner->GetPosition().GetCP(),20.f),&places);NAI::SPathPlace from,to;bool found=false;for(auto a:places){a.SetPose(NAI::CM_STAND);a.SetMoving(0);if(!network->IsPassable(a) || network->IsLocked(a))continue;for(auto b:places){b.SetPose(NAI::CM_CROUCH);b.SetMoving(0);const auto separation=fabs(NAI::SPosition(a,network).GetCP()-NAI::SPosition(b,network).GetCP());if(network->IsPassable(b) && !network->IsLocked(b) && separation>5.f && separation<8.f){from=a;to=b;found=true;break;}}if(found)break;}Require(found,"knife direction fixture lacks separate clear native standing/crouch cells");
    CObj<NRPG::IUnitMission> attacker=NRPG::CreateUnit(NDb::GetPers(103)),victim=NRPG::CreateUnit(NDb::GetPers(103));attacker->GetRPGUnit()->Skills(NDb::ST_LEVEL).SetConst(0);for(auto mission:{attacker.GetPtr(),victim.GetPtr()}){mission->GetRPGUnit()->Skills(NDb::ST_VP).SetConst(500);mission->GetRPGUnit()->nHealedVP=0;mission->GetRPGUnit()->Skills(NDb::ST_IC).SetConst(0);mission->GetRPGUnit()->Skills(NDb::ST_IC).SetProgress(0);}CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(from,attacker,owner->GetTBSPlayer(),"NativeKnifeDirectionActor"),target=world->AddUnitInGame(to,victim,owner->GetTBSPlayer(),"NativeKnifeDirectionTarget");Require(actor && target && target->CanFight() && target->GetPose()==NAI::CROUCH,"knife direction fixture cannot deploy actual crouching receiver");auto* difficulty=campaign->pDifficulty.GetPtr();Require(difficulty,"knife direction fixture lacks actual native difficulty");struct RestoreDifficulty{NDb::CDBDifficulty* value;float ours,enemy;~RestoreDifficulty(){value->fOurDamageMult=ours;value->fEnemyDamageMult=enemy;}}restoreDifficulty{difficulty,difficulty->fOurDamageMult,difficulty->fEnemyDamageMult};difficulty->fOurDamageMult=difficulty->fEnemyDamageMult=1.f;world->GetAIMap()->Sync();CVec3 head;Require(world->GetAIMap()->GetUnitHLPos(&head,world->GetAIMap()->GetHull(target),NAI::HL_HEAD),"knife direction fixture lacks actual native head locator");const float height=head.z-target->GetPosition().GetCP().z;std::printf("native knife direction setup mode=%d pose=%d headHeight=%.9g\n",mode,int(target->GetPose()),height);std::fflush(stdout);Require(height>.65f && height<1.25f,"knife direction fixture head is outside controlled native floor-collision height");CRay ray;ray.ptOrigin=head+CVec3(0,0,.6f);ray.ptDir=direction;vector<NAI::SInterval> intervals;world->GetAIMap()->Trace(ray,&intervals,NWorld::TS_FRAGMENTED|NWorld::TS_ITEM_BLOCKER);const NAI::SInterval* first=nullptr;for(const auto& interval:intervals){if(interval.exit.fT<=.001f)continue;Require(interval.enter.fT>.001f,"knife direction launch starts inside an actual native blocker");if(!first || interval.enter.fT<first->enter.fT)first=&interval;}auto* flesh=NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);Require(first && first->pSrc->pUserData.GetPtr()==target.GetPtr() && first->nUserID==NAI::HL_HEAD && first->pSrc->pArmor==flesh && first->enter.fT<1.f && flesh->pShotMaterial,"knife direction fixture lacks a real first head/flesh hit before floor");int type=-1;CDBIterator<NDb::CRPGDmgToArmor> damageTypes(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());while(damageTypes.MoveNext())if(damageTypes.Get()->armors[flesh->pMaterial->nDR]>0 && (type<0 || damageTypes.Get()->GetRecordID()<type))type=damageTypes.Get()->GetRecordID();Require(type>=0,"knife direction fixture lacks actual flesh damage type");NRPG::CAttackPortion portion(100000,type,0.f,baseDamage,baseDamage,0,0,attacker,victim,1.f,-1);portion.rTtrajectory.ptOrigin=ray.ptOrigin;portion.rTtrajectory.ptDir=stale;Require(portion.CalcStructDmg(world,flesh,0)==baseDamage,"knife direction fixture lost constant native damage control");
    auto decals=[&](){vector<CObj<NWorld::CDecal>> result;CStructureNetworkContext refs;auto fields=BorrowedObjectFields(world->GetActive(),&refs);for(const auto& entry:refs.refs)if(auto* decal=dynamic_cast<NWorld::CDecal*>(entry.second.GetPtr()))result.push_back(decal);return result;};const auto previous=decals();std::set<NWorld::CDecal*> old;for(const auto& decal:previous)old.insert(decal.GetPtr());NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> blades(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());while(blades.MoveNext()){auto* value=blades.Get();if(value->bThrowing && value->pItem && value->pItem->pModel && (!record || value->GetRecordID()<record->GetRecordID()))record=value;}Require(record,"knife direction fixture lacks authored modelled blade");SRand modelRandom;CObj<NDb::CModel> model=record->pItem->pModel->CreateModel(&modelRandom);CObj<NRPG::CMeleeWeaponItem> item=new NRPG::CMeleeWeaponItem(record);item->nQuantity=1;const STime now=world->GetTime()->GetValue();struct RestoreTime{CCTime* clock;STime value;~RestoreTime(){clock->Set(value);MarkNewDGFrame();}}restoreTime{world->GetTime(),now};std::set<NWorld::IDynamicObject*> existing;for(const auto& object:*world->GetMiscObjects())existing.insert(object.GetPtr());const int hp=victim->GetTotalVP(),actorHP=attacker->GetTotalVP(),ap=actor->GetAP();for(;;){CObj<NWorld::CHitLocator> event=world->GetHitEvent();if(!event)break;}GlobalGameRandom().SeedForHarness(2026);NRPG::SAttackRayInfo intendedFlight(portion,ray.ptOrigin,direction,actor,actor->GetPosition(),0,0,true,0,10.f,target);intendedFlight.pIgnore=actor;bool fullCarrier=false;CObj<CObjectBase> spawned=NativeSpawnKnifeCarrier(world.GetPtr(),intendedFlight,10.f,now,10.f,model,item,&fullCarrier,0);CObj<NWorld::IDynamicObject> knife=dynamic_cast<NWorld::IDynamicObject*>(spawned.GetPtr());Require(knife && !knife->Segment() && victim->GetTotalVP()==hp,"knife direction fixture applied damage before flight advanced");world->GetTime()->Set(now+100);MarkNewDGFrame();const bool retired=knife->Segment();CStructureNetworkContext refs;auto fields=BorrowedObjectFields(knife,&refs);CVec3 position;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(9,&position);}int hits=0,damage=-1;for(;;){CObj<NWorld::CHitLocator> event=world->GetHitEvent();if(!event)break;if(event->pUnit.GetPtr()==target.GetPtr()){++hits;damage=event->nHitValue;}}Require(retired && hits==1 && damage==(mode==3?3:15) && hp-victim->GetTotalVP()==damage && fabs(position-ray.ptOrigin)>.99f,"knife direction fixture did not isolate a real nonlethal native head impact");int added=0,ground=0;float minZ=10000;auto after=decals();for(const auto& decal:after)if(!old.count(decal.GetPtr())){CStructureNetworkContext context;auto data=BorrowedObjectFields(decal,&context);NGScene::SDecalMappingInfo info;{CStructureSaver reader(data,CStructureSaver::READ,&context);reader.Add(5,&info);}++added;minZ=(std::min)(minZ,info.vCenter.z);if(info.vCenter.z<head.z-.2f)++ground;}const int expected=mode==3?0:1;std::printf("native knife direction mode=%d staleDot=%.9g damage=%d expectedDamage=%d added=%d ground=%d expectedGround=%d minZ=%.9g headZ=%.9g height=%.9g hits=%d retired=%d distance=%.9g\n",mode,stale*direction,damage,mode==3?3:15,added,ground,expected,minZ,head.z,height,hits,int(retired),fabs(position-ray.ptOrigin));std::fflush(stdout);Require(ground==expected,"native knife impact used stale attack direction for actual world blood decal");Require(actor->GetAP()==ap && attacker->GetTotalVP()==actorHP && target->CanFight() && target->GetPose()==NAI::CROUCH && item->GetQuantity()==1,"knife direction impact changed native actor/AP/HP/receiver/item controls");knife=nullptr;auto* objects=world->GetMiscObjects();for(auto i=objects->begin();i!=objects->end();)if(!existing.count(i->GetPtr()))i=objects->erase(i);else ++i;
}

static void NativeKnifeWorldContext(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown native knife world-context mode");CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);auto* map=world->GetAIMap();map->Sync();list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* target=nullptr;CRay ray;NDb::CRPGArmor* armor=nullptr;int location=-1;const CVec3 directions[]={CVec3(1,0,0),CVec3(0,1,0),CVec3(-1,0,0),CVec3(0,-1,0)};
    for(auto unit:units){if(!unit || !unit->CanFight() || unit->IsEmptyPK() || unit->IsWearingPK() || unit->GetUnitRPG()->GetTotalVP()<20 || unit->GetUnitRPG()->GetRPGUnit()->nHealedVP || unit->GetUnitRPG()->GetRPGUnit()->IsCheatEnabled(NRPG::CHEAT_GODMODE))continue;CVec3 body;if(!map->GetUnitHLPos(&body,map->GetHull(unit),NAI::HL_BODY))continue;for(const auto& direction:directions){CRay candidate;candidate.ptDir=direction;candidate.ptOrigin=body-direction*2.f;vector<NAI::SInterval> intervals;map->Trace(candidate,&intervals,NWorld::TS_FRAGMENTED | NWorld::TS_ITEM_BLOCKER);const NAI::SInterval* first=nullptr;bool inside=false;for(const auto& interval:intervals){if(interval.exit.fT<.01f)continue;if(interval.enter.fT<=.01f){inside=true;break;}if(!first || interval.enter.fT<first->enter.fT)first=&interval;}if(inside || !first || first->enter.fT>4 || first->nUserID!=NAI::HL_BODY || first->pSrc->pUserData!=unit.GetPtr() || !first->pSrc->pArmor || !first->pSrc->pArmor->pMaterial || first->pSrc->pArmor->pMaterial->GetRecordID()!=NDb::CRPGMaterial::HUMAN_BODY)continue;target=unit;ray=candidate;armor=first->pSrc->pArmor;location=first->nUserID;break;}if(target)break;}Require(target && armor,"knife world fixture lacks actual isolated native flesh receiver");NWorld::CUnitServer* actor=nullptr;for(auto unit:units)if(unit && unit.GetPtr()!=target && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK() && unit->IsAIUnit()==(mode==1)){actor=unit;break;}Require(actor,"knife world fixture lacks actual ordinary attacker owner");
    auto* difficulty=campaign->pDifficulty.GetPtr();Require(difficulty,"knife world fixture lacks native difficulty");struct Restore{NDb::CDBDifficulty* value;float ours,enemy;~Restore(){value->fOurDamageMult=ours;value->fEnemyDamageMult=enemy;}}restore{difficulty,difficulty->fOurDamageMult,difficulty->fEnemyDamageMult};difficulty->fOurDamageMult=difficulty->fEnemyDamageMult=mode==3?1.f:2.f;
    int damageType=-1;CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());while(types.MoveNext())if(types.Get()->armors[armor->pMaterial->nDR]>0 && (damageType<0 || types.Get()->GetRecordID()<damageType))damageType=types.Get()->GetRecordID();Require(damageType>=0,"knife world fixture lacks native flesh damage type");const bool anonymous=mode==2;NRPG::CAttackPortion portion(100000,damageType,0.f,2,2,0,0,anonymous?nullptr:actor->GetUnitRPG(),target->GetUnitRPG(),1.f,-1);const int expected=anonymous || mode==3?2:4;Require(portion.CalcStructDmg(nullptr,armor,0)==2 && portion.CalcStructDmg(world,armor,0)==expected,"knife world fixture lacks literal null/world damage controls");portion.rTtrajectory=ray;NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> blades(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());while(blades.MoveNext()){auto* value=blades.Get();if(value->bThrowing && value->pItem && value->pItem->pModel && (!record || value->GetRecordID()<record->GetRecordID()))record=value;}Require(record,"knife world fixture lacks an actual authored throwing knife model");SRand modelRandom;CObj<NDb::CModel> model=record->pItem->pModel->CreateModel(&modelRandom);Require(model,"knife world fixture cannot resolve actual native model pool");CObj<NRPG::CMeleeWeaponItem> item=new NRPG::CMeleeWeaponItem(record);item->nQuantity=1;const int hp=target->GetUnitRPG()->GetTotalVP(),actorHP=actor->GetUnitRPG()->GetTotalVP(),ap=actor->GetAP();const STime now=world->GetTime()->GetValue();struct RestoreTime{CCTime* clock;STime value;~RestoreTime(){clock->Set(value);MarkNewDGFrame();}}restoreTime{world->GetTime(),now};std::set<NWorld::IDynamicObject*> existing;for(const auto& object:*world->GetMiscObjects())existing.insert(object.GetPtr());for(;;){CObj<NWorld::CHitLocator> event=world->GetHitEvent();if(!event)break;}GlobalGameRandom().SeedForHarness(2026);NRPG::SAttackRayInfo intendedFlight(portion,ray.ptOrigin,ray.ptDir,actor,actor->GetPosition(),0,0,true,0,10.f,target);intendedFlight.pIgnore=actor;bool fullCarrier=false;CObj<CObjectBase> spawned=NativeSpawnKnifeCarrier(world.GetPtr(),intendedFlight,10.f,now,10.f,model,item,&fullCarrier,0);CObj<NWorld::IDynamicObject> knife=dynamic_cast<NWorld::IDynamicObject*>(spawned.GetPtr());Require(knife,"knife world fixture cannot construct actual native projectile");Require(!knife->Segment() && target->GetUnitRPG()->GetTotalVP()==hp,"knife world fixture applied damage before native flight advanced");world->GetTime()->Set(now+500);MarkNewDGFrame();const bool retired=knife->Segment();CStructureNetworkContext refs;auto fields=BorrowedObjectFields(knife,&refs);CVec3 actualPosition;NRPG::SAttackRayInfo carried;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(9,&actualPosition);reader.Add(11,&carried);}int count=0,damage=-999;for(;;){CObj<NWorld::CHitLocator> event=world->GetHitEvent();if(!event)break;if(event->pUnit.GetPtr()==target){++count;damage=event->nHitValue;}}std::printf("native knife world mode=%d actor=%d receiver=%d location=%d AI=%d anonymous=%d coefficient=%.9g received=%d expected=%d HPdrop=%d hits=%d retired=%d distance=%.9g AP=%d->%d\n",mode,actor->GetUnitRPG()->GetRPGPersID(),target->GetUnitRPG()->GetRPGPersID(),location,int(actor->GetUnitRPG()->IsAIPlayer()),int(anonymous),difficulty->fOurDamageMult,damage,expected,hp-target->GetUnitRPG()->GetTotalVP(),count,int(retired),fabs(actualPosition-ray.ptOrigin),ap,actor->GetAP());std::fflush(stdout);Require(retired && fabs(actualPosition-ray.ptOrigin)>4.9f && count==1,"knife world fixture did not reach a real native flight/body collision");Require(damage==expected && hp-target->GetUnitRPG()->GetTotalVP()==expected,"native flying knife discarded owning world before actual collision damage");Require(carried.pUS.GetPtr()==actor && carried.pIgnore.GetPtr()==actor && carried.atk.pAttacker.GetPtr()==portion.pAttacker.GetPtr() && carried.atk.pTarget.GetPtr()==portion.pTarget.GetPtr() && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==actorHP && target->CanFight() && item->GetQuantity()==1,"native knife world dispatch changed actor/AP/HP/item/carrier controls");knife=nullptr;auto* objects=world->GetMiscObjects();for(auto i=objects->begin();i!=objects->end();)if(!existing.count(i->GetPtr()))i=objects->erase(i);else ++i;
}

static void NativeMeleeWorldContext(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown native melee world-context mode");CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);auto* map=world->GetAIMap();map->Sync();list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* target=nullptr;CRay ray;NDb::CRPGArmor* armor=nullptr;int location=-1;const CVec3 directions[]={CVec3(1,0,0),CVec3(0,1,0),CVec3(-1,0,0),CVec3(0,-1,0)};
    for(auto unit:units){if(!unit || !unit->CanFight() || unit->IsEmptyPK() || unit->IsWearingPK() || unit->GetUnitRPG()->GetTotalVP()<20 || unit->GetUnitRPG()->GetRPGUnit()->nHealedVP || unit->GetUnitRPG()->GetRPGUnit()->IsCheatEnabled(NRPG::CHEAT_GODMODE))continue;CVec3 body;if(!map->GetUnitHLPos(&body,map->GetHull(unit),NAI::HL_BODY))continue;for(const auto& direction:directions){CRay candidate;candidate.ptDir=direction;candidate.ptOrigin=body-direction*2.f;vector<NAI::SInterval> intervals;map->Trace(candidate,&intervals,NWorld::TS_FRAGMENTED);const NAI::SInterval* first=nullptr;bool inside=false;for(const auto& interval:intervals){if(interval.exit.fT<.01f)continue;if(interval.enter.fT<=.01f){inside=true;break;}if(!first || interval.enter.fT<first->enter.fT)first=&interval;}if(inside || !first || first->enter.fT>4 || first->pSrc->pUserData!=unit.GetPtr() || !first->pSrc->pArmor || !first->pSrc->pArmor->pMaterial || first->pSrc->pArmor->pMaterial->GetRecordID()!=NDb::CRPGMaterial::HUMAN_BODY)continue;target=unit;ray=candidate;armor=first->pSrc->pArmor;location=first->nUserID;break;}if(target)break;}Require(target && armor,"melee world fixture lacks actual isolated native flesh receiver");NWorld::CUnitServer* actor=nullptr;for(auto unit:units)if(unit && unit.GetPtr()!=target && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK() && unit->IsAIUnit()==(mode==1)){actor=unit;break;}Require(actor,"melee world fixture lacks actual ordinary attacker owner");
    auto* difficulty=campaign->pDifficulty.GetPtr();Require(difficulty,"melee world fixture lacks native difficulty");struct Restore{NDb::CDBDifficulty* value;float ours,enemy;~Restore(){value->fOurDamageMult=ours;value->fEnemyDamageMult=enemy;}}restore{difficulty,difficulty->fOurDamageMult,difficulty->fEnemyDamageMult};difficulty->fOurDamageMult=difficulty->fEnemyDamageMult=mode==3?1.f:2.f;
    int damageType=-1;CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());while(types.MoveNext())if(types.Get()->armors[armor->pMaterial->nDR]>0 && (damageType<0 || types.Get()->GetRecordID()<damageType))damageType=types.Get()->GetRecordID();Require(damageType>=0,"melee world fixture lacks native flesh damage type");const bool anonymous=mode==2;NRPG::CAttackPortion portion(100000,damageType,0.f,2,2,0,0,anonymous?nullptr:actor->GetUnitRPG(),target->GetUnitRPG(),1.f,-1);const int expected=anonymous || mode==3?2:4;Require(portion.CalcStructDmg(nullptr,armor,0)==2 && portion.CalcStructDmg(world,armor,0)==expected,"melee world fixture lacks literal null/world damage controls");vector<NRPG::CAttackPortion> attacks(1,portion);CObj<NWorld::CExecMelee> exec;if(mode==0 || mode==2)exec=new NWorld::CExecMeleeUnit(actor,target,NAI::HL_BODY,0);else exec=new NWorld::CExecMeleeTile(actor,target->GetPosition().GetCP());const int hp=target->GetUnitRPG()->GetTotalVP(),actorHP=actor->GetUnitRPG()->GetTotalVP(),ap=actor->GetAP();for(;;){CObj<NWorld::CHitLocator> event=world->GetHitEvent();if(!event)break;}GlobalGameRandom().SeedForHarness(2026);exec->PerformAttack(attacks,ray,1.f);int count=0,damage=-999;for(;;){CObj<NWorld::CHitLocator> event=world->GetHitEvent();if(!event)break;if(event->pUnit.GetPtr()==target){++count;damage=event->nHitValue;}}
    std::printf("native melee world mode=%d actor=%d receiver=%d location=%d AI=%d anonymous=%d coefficient=%.9g received=%d expected=%d HPdrop=%d hits=%d AP=%d->%d\n",mode,actor->GetUnitRPG()->GetRPGPersID(),target->GetUnitRPG()->GetRPGPersID(),location,int(actor->GetUnitRPG()->IsAIPlayer()),int(anonymous),difficulty->fOurDamageMult,damage,expected,hp-target->GetUnitRPG()->GetTotalVP(),count,ap,actor->GetAP());std::fflush(stdout);Require(count==1 && damage==expected && hp-target->GetUnitRPG()->GetTotalVP()==expected,"native executor lost real world context before melee damage callback");Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==actorHP && target->CanFight(),"direct native melee dispatch spent actor AP/health or incapacitated its small-damage receiver");
}

// Decode an independent legacy executor. The live world is only borrowed by reference.
static CObj<NWorld::CExecMelee> RestoreNativeObjectMeleeSave(NWorld::CUnitServer* actor,
    CObjectBase* target,const CVec3& point,int extra)
{
    CObj<NWorld::CExecMeleeTile> seed=new NWorld::CExecMeleeTile(actor,point);
    struct MeleeFields {
        NWorld::CExecAttack* attack;CVec3 point;int extra;
        int operator&(CStructureSaver& f){f.Add(1,attack);f.Add(2,&point);f.Add(3,&extra);return 0;}
    };
    struct ObjectFields {
        MeleeFields melee;CPtr<CObjectBase> target;
        int operator&(CStructureSaver& f){f.Add(1,&melee);f.Add(2,&target);return 0;}
    } saved{{seed.GetPtr(),point,extra},target};
    CStructureNetworkContext refs;auto ignored=BorrowedObjectFields(seed,&refs);
    if(target && !refs.ids.count(target)){const auto id=refs.nextID++;refs.ids[target]=id;refs.refs[id]=target;}
    CMemoryStream fields;{CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);saved.operator&(writer);}fields.Seek(0);
    CObj<NWorld::CExecMelee> restored=dynamic_cast<NWorld::CExecMelee*>(pSSClasses->CreateObject(0x01353130));
    std::printf("OBJECT-MELEE-FACTORY available=%d target=%d extra=%d\n",int(bool(restored)),int(target!=nullptr),extra);std::fflush(stdout);
    Require(restored,"native save loader cannot construct original object-melee executor 01353130");
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(restored.GetPtr())->operator&(reader);}
    Require(pSSClasses->GetObjectTypeID(restored.GetPtr())==0x01353130,"object-melee legacy factory changed its native type identity");
    return restored;
}

static void NativeObjectMeleeSave(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<6,"unknown native object-melee save mode");
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsAIUnit() && !unit->IsEmptyPK() && !unit->IsWearingPK() && !unit->HasCommand()){actor=unit;break;}
    Require(actor,"object-melee save fixture lacks an ordinary native human actor");
    NWorld::CWindowDoor* door=nullptr;auto* map=world->GetAIMap();CVec3 center;
    for(const auto& entry:world->GetObjectsForHarness()) {
        auto* candidate=dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr());
        if(candidate && !candidate->IsBroken() && map->GetUnitHLPos(&center,map->GetHull(candidate),-1)){door=candidate;break;}
    }
    Require(door,"object-melee save fixture lacks a real authored door and its native hull");
    auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous;
    const auto slot=inventory->GetActiveSlot();if(slot>=0)previous=inventory->TakeOff(static_cast<NDb::ESlot>(slot));
    Require(NWorld::GetActionType(actor)==NWorld::AT_MELEE,"object-melee save fixture did not select native unarmed melee");
    const CVec3 remembered=actor->GetPosition().GetCP()+CVec3(.25f,0,.5f);
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();
    const int cost=actor->GetActionAP(NRPG::AC_MELEE),extra=7;
    CObj<NWorld::CExecMelee> executor;
    if(mode==0)executor=new NWorld::CExecMeleeTile(actor,remembered);
    else executor=RestoreNativeObjectMeleeSave(actor,mode==2 || mode==5?nullptr:static_cast<CObjectBase*>(door),remembered,extra);
    const int ignored=executor->CanDoIt(position,true),available=executor->CanDoIt(position);
    Require(ignored==NWorld::UCR_NO_TARGET && available==NWorld::UCR_OK && executor->GetStartAP()==cost+(mode?extra:0),
        "loaded native melee lost inherited remembered-point eligibility or requested AP");
    if(mode==1 || mode==2) {
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(executor,&refs);CPtr<CObjectBase> target;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&target);}
        Require(target.GetPtr()==(mode==2?nullptr:static_cast<CObjectBase*>(door)),"native object-melee save lost its retained target");
        CObj<NWorld::CExecMelee> second=dynamic_cast<NWorld::CExecMelee*>(pSSClasses->CreateObject(0x01353130));
        Require(second && second.GetPtr()!=executor.GetPtr(),"object-melee roundtrip reused its source executor");fields.Seek(0);
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(second.GetPtr())->operator&(reader);}
        CStructureNetworkContext roundRefs;auto round=BorrowedObjectFields(second,&roundRefs);CPtr<CObjectBase> roundTarget;
        {CStructureSaver reader(round,CStructureSaver::READ,&roundRefs);reader.Add(2,&roundTarget);}
        Require(roundTarget==target && second->GetStartAP()==cost+extra && second->CanDoIt(position)==NWorld::UCR_OK,
            "native object-melee roundtrip lost target, remembered point or extra AP");
    }
    if(mode==3) {
        executor->Start();
        Require(actor->GetAP()==ap-cost-extra && actor->GetPosition()==position && !door->IsBroken(),
            "restored native object-melee Start lost base action/extra AP or changed its target early");
    }
    if(mode==4 || mode==5) {
        if(mode==4){world->PlaceObjectInPocket(door);MarkNewDGFrame();map->Sync();Require(!map->GetHull(door),"native pocket did not remove the saved door hull");}
        GlobalGameRandom().SeedForHarness(2026);executor->OnLabel();
        struct Attack {bool canceled=false;int operator&(CStructureSaver& f){f.Add(2,&canceled);return 0;}};
        struct Melee {Attack attack;int operator&(CStructureSaver& f){f.Add(1,&attack);return 0;}} melee;
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(executor,&refs);
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,&melee);}
        Require(melee.attack.canceled,"native loaded object-melee label failed to consume its one-shot attack");
    }
    Require(actor->GetAP()==ap-(mode==3?cost+extra:0) && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && !actor->HasCommand(),
        "native object-melee restore/query/label changed actor health, position, orders or unexpected AP");
    std::printf("OBJECT-MELEE-SAVE mode=%d native=1 query=%d ignored=%d startAP=%d expectedStart=%d AP=%d->%d graph=1\n",mode,available,ignored,executor->GetStartAP(),cost+(mode?extra:0),ap,actor->GetAP());std::fflush(stdout);
}

static void NativeMeleeTargetFilter(const char* databasePath,const char* resourcePath,int mode,bool objectSave=false)
{
    Require(mode>=0 && mode<=6,"unknown native melee filter mode");CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto u:units)if(u && u->CanFight() && !u->IsAIUnit() && u->GetTBSPlayer() && !u->IsEmptyPK() && !u->IsWearingPK()){actor=u;break;}Require(actor,"melee filter fixture lacks actual human owner/attacker");auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),30.f),&places);NAI::SPathPlace firstPlace,secondPlace;bool found=false;for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetMoving(0);if(!network->IsPassable(p) || network->IsLocked(p))continue;for(int d=0;d<4;++d){auto q=p;const int x=int(p.GetX())+(d==0?1:d==1?-1:0),y=int(p.GetY())+(d==2?1:d==3?-1:0);if(x<0 || y<0 || x>254 || y>254)continue;q.SetXY(x,y);if(network->IsPassable(q) && !network->IsLocked(q)){firstPlace=p;secondPlace=q;found=true;break;}}if(found)break;}Require(found,"melee filter fixture lacks real adjacent free standing navigation places");
    CObj<NRPG::IUnitMission> firstRPG=NRPG::CreateUnit(NDb::GetPers(103)),secondRPG=NRPG::CreateUnit(NDb::GetPers(103));for(auto* mission:{firstRPG.GetPtr(),secondRPG.GetPtr()}){auto* unit=mission->GetRPGUnit();unit->nHealedVP=0;unit->Skills(NDb::ST_IC).SetConst(0);unit->Skills(NDb::ST_IC).SetProgress(0);Require(mission->GetTotalVP()>20 && mission->GetIC()==0,"melee filter fixture lacks isolated healthy no-evasion humans");}CObj<NWorld::CUnitServer> first=world->AddUnitInGame(firstPlace,firstRPG,actor->GetTBSPlayer(),"NativeMeleeFilterFront"),second=world->AddUnitInGame(secondPlace,secondRPG,actor->GetTBSPlayer(),"NativeMeleeFilterBack");Require(first && second && first->CanFight() && second->CanFight(),"melee filter fixture cannot deploy actual distinct receivers");auto* map=world->GetAIMap();map->Sync();CVec3 a,b;Require(map->GetUnitHLPos(&a,map->GetHull(first),NAI::HL_BODY) && map->GetUnitHLPos(&b,map->GetHull(second),NAI::HL_BODY),"melee filter fixture lacks native body locators");CRay ray;ray.ptDir=b-a;Normalize(&ray.ptDir);ray.ptOrigin=a-ray.ptDir*2.f;vector<NAI::SInterval> intervals;map->Trace(ray,&intervals,NWorld::TS_FRAGMENTED);int firstPieces=0,secondPieces=0;NDb::CRPGArmor* armor=nullptr;for(const auto& interval:intervals)if(interval.enter.fT>0){if(interval.pSrc->pUserData==first.GetPtr()){++firstPieces;if(!armor)armor=interval.pSrc->pArmor;}if(interval.pSrc->pUserData==second.GetPtr())++secondPieces;}Require(firstPieces>0 && secondPieces>0 && armor && armor->pMaterial,"melee filter fixture ray does not cross both actual native humans");
    Require(mode!=6 || firstPieces>1,"native godmode dedup input needs multiple actual body pieces");const bool god=mode==4 || mode==5 || mode==6;if(god)firstRPG->GetRPGUnit()->SetCheat(NRPG::CHEAT_GODMODE,true);NWorld::CUnitServer* selected=mode==1?first.GetPtr():second.GetPtr();int damageType=-1;CDBIterator<NDb::CRPGDmgToArmor> types(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());while(types.MoveNext())if(types.Get()->armors[armor->pMaterial->nDR]>0 && (damageType<0 || types.Get()->GetRecordID()<damageType))damageType=types.Get()->GetRecordID();Require(damageType>=0,"melee filter fixture lacks native flesh damage type");NRPG::CAttackPortion portion(100000,damageType,0.f,2,2,0,0,actor->GetUnitRPG(),selected->GetUnitRPG(),1.f,-1);const int base=portion.CalcStructDmg(world,armor,0);Require(base>0 && base<6,"melee filter fixture lacks positive small literal native damage control");vector<NRPG::CAttackPortion> portions(1,portion);CObj<NWorld::CExecMelee> exec;if(mode==2 || mode==4 || mode==6)exec=new NWorld::CExecMeleeTile(actor,b);else if(objectSave)exec=RestoreNativeObjectMeleeSave(actor,mode==3?nullptr:selected,b,0);else exec=new NWorld::CExecMeleeUnit(actor,mode==3?nullptr:selected,NAI::HL_BODY,0);for(;;){CObj<NWorld::CHitLocator> event=world->GetHitEvent();if(!event)break;}const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),firstHP=firstRPG->GetTotalVP(),secondHP=secondRPG->GetTotalVP();Require(firstHP>base && secondHP>base,"melee filter fixture lacks healthy post-spawn native receivers");std::printf("MELEE-RECEIVER-BASELINE mode=%d first=%d second=%d\n",mode,firstHP,secondHP);GlobalGameRandom().SeedForHarness(2026);exec->PerformAttack(portions,ray,1.f);int frontHits=0,backHits=0,frontDamage=0,backDamage=0;for(;;){CObj<NWorld::CHitLocator> event=world->GetHitEvent();if(!event)break;if(event->pUnit.GetPtr()==first.GetPtr()){++frontHits;frontDamage+=event->nHitValue;}if(event->pUnit.GetPtr()==second.GetPtr()){++backHits;backDamage+=event->nHitValue;}}
    const bool hitFront=mode==1 || mode==2 || mode==3,hitBack=mode!=1;const int expectedFront=hitFront?base:0,expectedBack=hitBack?base:0;std::printf("native melee filter mode=%d base=%d frontPieces=%d backPieces=%d god=%d frontDrop=%d expectedFront=%d backDrop=%d expectedBack=%d frontHits=%d backHits=%d frontDamage=%d backDamage=%d\n",mode,base,firstPieces,secondPieces,int(god),firstHP-firstRPG->GetTotalVP(),expectedFront,secondHP-secondRPG->GetTotalVP(),expectedBack,frontHits,backHits,frontDamage,backDamage);std::fflush(stdout);Require(firstHP-firstRPG->GetTotalVP()==expectedFront && secondHP-secondRPG->GetTotalVP()==expectedBack,"native unit melee dispatch damaged an unselected real receiver on its ray");const int expectedFrontHits=mode==4?-1:mode==6?1:hitFront?1:0;Require((expectedFrontHits<0?frontHits>0 && frontDamage==-frontHits:frontHits==expectedFrontHits && frontDamage==(mode==6?-1:expectedFront)) && backHits==int(hitBack) && backDamage==expectedBack,mode==6?"native godmode unit was not deduplicated to one original damage callback":"native melee target filter lost callback identity, default/null behavior or godmode protection");Require(first->CanFight() && second->CanFight() && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp,"small direct melee filter dispatch changed actor AP/health or incapacitated a receiver");
}

static void AICoverRange(const char* databasePath,const char* resourcePath,int mode)
{
    const bool human=mode==2 || mode==4 || mode==6 || mode==7;
    const bool distant=mode==0 || mode==2 || mode==3 || mode==4;
    const bool elevated=mode==0 || mode==1 || mode==2 || mode==7;
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> all;world->GetAllUnits(&all);
    NWorld::CUnitServer *actor=nullptr,*target=nullptr;NRPG::IWeaponItem* firearm=nullptr;
    for(const auto& candidate:all) {
        if(!IsValid(candidate) || !candidate->CanFight() || !candidate->IsAIUnit() || candidate->HasCommand()
            || candidate->IsEmptyPK() || candidate->IsWearingPK() || candidate->animator.GetCannon())continue;
        auto* weapon=dynamic_cast<NRPG::IWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
        if(weapon && weapon->GetAmmoQuantity()>0 && weapon->SetShootMode(NDb::SM_Snap)){actor=candidate.GetPtr();firearm=weapon;break;}
    }
    Require(actor && firearm,"cover range fixture has no actual armed AI actor");
    for(const auto& candidate:all)if(IsValid(candidate) && candidate.GetPtr()!=actor && candidate->CanFight()
        && !candidate->IsEmptyPK() && !candidate->IsWearingPK() && actor->GetDiplomacyState(candidate)==NDb::DS_ENEMY){target=candidate.GetPtr();break;}
    Require(target,"cover range fixture has no opposing native target");
    NRPG::IGame* game=world->GetGame();CPtr<NAI::IAIMap> original;
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(game,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&original);}
    CVec3 aim;Require(original.GetPtr()==world->GetAIMap() && original->GetUnitHLPos(&aim,original->GetHull(target),NAI::HL_ANY),
        "cover range fixture has no actual native target locator");
    Require(fabs(aim.x-target->GetPosition().GetCP().x,aim.y-target->GetPosition().GetCP().y)<1.f,
        "cover range fixture did not place actual target geometry at the native server position");
    vector<NRPG::CAttackPortion> attacks;actor->GetUnitRPG()->CreateAttack(&attacks,false);
    Require(!attacks.empty() && attacks.front().nK>0,"cover range fixture has no actual native weapon portion");
    CObj<CoverRangeMapProbe> probe=new CoverRangeMapProbe(original);probe->target=target;
    struct Restore{NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}}restore{game,original};SetCoverProbeMap(game,probe);
    // Select the direction using a separate actual29-unit non-AI query.
    // Both native producers now cap at30, so an elevated33-unit query can no
    // longer serve as an uncapped positive reference. Keep real near blocked
    // controls as well, independently of the distant result under test.
    const float distance=distant?33.f:29.f;CVec3 source;bool clear=false;int directionIndex=-1,referenceScore=0;
    const CVec2 directions[]={CVec2(1,0),CVec2(0,1),CVec2(-1,0),CVec2(0,-1),CVec2(1,1),CVec2(-1,1),CVec2(-1,-1),CVec2(1,-1)};
    for(const auto& xy:directions) {
        ++directionIndex;CVec3 offset(xy.x,xy.y,0);Normalize(&offset);
        if(elevated)offset=offset*.6f+CVec3(0,0,.8f);
        const CVec3 candidate=aim+offset*distance;
        const CVec3 referenceSource=aim+offset*29.f;
        probe->source=referenceSource;probe->traces.clear();probe->targetIntervals=probe->insideCap=probe->outsideCap=probe->insideNaturalRange=0;
        CObj<NRPG::CCoverInfo> reference=game->CalcCovers(referenceSource,attacks.front(),actor,target,NAI::HL_ANY,1.f,false);
        referenceScore=NRPG::GetHitCover(actor,reference);clear=elevated ? referenceScore>0 : referenceScore==-1;
        if(clear){source=candidate;break;}
    }
    Require(clear,"cover range fixture has no native non-AI reference of the required positive/blocked type");
    probe->source=source;probe->traces.clear();probe->targetIntervals=probe->insideCap=probe->outsideCap=probe->insideNaturalRange=0;
    const auto actorPosition=actor->GetPosition(),targetPosition=target->GetPosition();
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),targetHP=target->GetUnitRPG()->GetTotalVP(),ammo=firearm->GetAmmoQuantity();
    const bool command=actor->HasCommand();GlobalGameRandom().SeedForHarness(0x739d);auto beforeRandom=GlobalGameRandom();
    int score;
    if(human) {
        CObj<NRPG::CCoverInfo> cover=game->CalcCovers(source,attacks.front(),actor,target,NAI::HL_ANY,1.f,false);
        score=NRPG::GetHitCover(actor,cover);
    } else score=game->GetCoverForAIUnit(source,actor,target,attacks.front(),NAI::HL_ANY);
    const float realDistance=fabs(aim-source),horizontal=fabs(aim.x-source.x,aim.y-source.y);
    std::printf("native AI cover range: mode=%d human=%d distant=%d elevated=%d direction=%d distance=%.2f horizontal=%.2f score=%d reference=%d referenceDistance=29 target=%d inside=%d outside=%d natural=%d grids=%zu\n",
        mode,int(human),int(distant),int(elevated),directionIndex,realDistance,horizontal,score,referenceScore,probe->targetIntervals,probe->insideCap,probe->outsideCap,probe->insideNaturalRange,probe->traces.size());std::fflush(stdout);
    Require(probe->traces.size()==(human?2u:1u) && probe->targetIntervals>0 && probe->insideNaturalRange==probe->targetIntervals,
        "cover range fixture did not preserve genuine target intersections within the natural projection distance");
    for(const auto& trace:probe->traces)Require(fabs(trace.origin-source)<2e-4f,"cover range query changed its supplied hypothetical source point");
    Require(fabs(realDistance-distance)<2e-4f && (!elevated || horizontal<30.f),"cover range fixture did not separate native 3D range from horizontal range");
    Require(distant ? probe->insideCap==0 && probe->outsideCap==probe->targetIntervals : probe->insideCap==probe->targetIntervals && probe->outsideCap==0,
        "cover range fixture did not separate the real target intervals on the two sides of the original cap");
    Require(!human || distant || score==referenceScore,"near non-AI range control changed its actual native29-unit reference score");
    if(elevated)Require(distant ? (human?score==-1:score==0) : score>0,
        "cover exceeded the original30-unit radial limit or changed its real near control");
    else Require(human ? score==-1 : score==0,"AI range policy changed the real map's fully blocked level-ray control");
    Require(GlobalGameRandom().Get()==beforeRandom.Get(),"cover range queries consumed native RNG");
    Require(actor->GetPosition()==actorPosition && target->GetPosition()==targetPosition && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp
        && target->GetUnitRPG()->GetTotalVP()==targetHP && firearm->GetAmmoQuantity()==ammo && actor->HasCommand()==command,
        "cover range queries changed native AP/HP/ammo/poses or issued an order");
}

static void AICoverTargetReach(const char* databasePath,const char* resourcePath,int mode)
{
    const bool human=mode==1 || mode==3,pk=mode==5 || mode==6;
    const bool shortened=mode==0 || mode==1 || mode==5;
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> all;world->GetAllUnits(&all);
    NWorld::CUnitServer *actor=nullptr,*target=nullptr;NRPG::IWeaponItem* firearm=nullptr;
    for(const auto& candidate:all) {
        if(!IsValid(candidate) || !candidate->CanFight() || candidate->HasCommand() || candidate->IsEmptyPK() || candidate->IsWearingPK())continue;
        auto* weapon=dynamic_cast<NRPG::IWeaponItem*>(candidate->GetUnitRPG()->GetInventory()->GetActive());
        if(weapon && weapon->GetAmmoQuantity()>0 && weapon->SetShootMode(NDb::SM_Snap)){actor=candidate.GetPtr();firearm=weapon;break;}
    }
    Require(actor && firearm,"cover reach fixture has no actual armed actor");
    for(const auto& candidate:all)if(IsValid(candidate) && candidate.GetPtr()!=actor && candidate->CanFight()
        && !candidate->IsEmptyPK() && !candidate->IsWearingPK() && actor->GetDiplomacyState(candidate)==NDb::DS_ENEMY){target=candidate.GetPtr();break;}
    Require(target,"cover reach fixture has no actual opposing target");
    CObj<NWorld::CUnitServer> shell;
    if(pk) {
        NDb::CRPGPers* persona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
        while(people.MoveNext())if(people.Get()->pPanzerklein && !people.Get()->pPanzerklein->bHasNoHead && people.Get()->pModel){persona=people.Get();break;}
        Require(persona,"cover reach fixture has no authored regular PK shell");
        auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;
        network->GetNearPlaces(SSphere(target->GetPosition().GetCP(),20.f),&places);NAI::SPathPlace anchor;bool found=false;
        for(auto place:places){place.SetPose(NAI::CM_STAND);place.SetDirection(0);if(NAI::IsBigLockerPlacePassable(network,place)){anchor=place;found=true;break;}}
        Require(found,"cover reach fixture has no free native PK footprint");
        shell=world->AddUnitInGame(anchor,NRPG::CreateUnit(persona),nullptr);
        Require(shell && shell->IsEmptyPK() && shell->WearAsPK(true),"cover reach fixture could not prepare its real PK shell");
        target->SetPosition(shell->GetPosition());target->FlipPanzerklein(shell,false,false);
        Require(target->IsWearingPK() && target->GetDecalsRef()==shell.GetPtr(),"cover reach fixture lost the fork's real pilot/surface-owner relation");
        world->GetAIMap()->Sync();
    }
    NRPG::IGame* game=world->GetGame();CPtr<NAI::IAIMap> original;
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(game,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&original);}
    Require(original.GetPtr()==world->GetAIMap() && original->GetHull(target),"cover reach fixture has no native gameplay target hull");
    CObj<CoverReachMapProbe> probe=new CoverReachMapProbe(original);probe->target=target;
    struct Restore{NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}}restore{game,original};SetCoverProbeMap(game,probe);
    vector<NRPG::CAttackPortion> attacks;actor->GetUnitRPG()->CreateAttack(&attacks,false);
    Require(!attacks.empty() && attacks.front().nK>0,"cover reach fixture has no native penetrating attack portion");
    CVec3 source=actor->GetAttackOrigin(actor->GetPosition(),false);
    if(pk)source=target->GetPosition().GetCP()+CVec3(6.f,0,1.f);
    probe->source=source;
    bool ignoreTarget=false;
    auto query=[&]() {
        if(!human)return game->GetCoverForAIUnit(source,ignoreTarget?target:actor,target,attacks.front(),NAI::HL_ANY);
        CObj<NRPG::CCoverInfo> cover=game->CalcCovers(source,attacks.front(),actor,target,NAI::HL_ANY,1.f,false);
        return NRPG::GetHitCover(actor,cover);
    };
    const auto actorPosition=actor->GetPosition(),targetPosition=target->GetPosition();
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),targetHP=target->GetUnitRPG()->GetTotalVP(),ammo=firearm->GetAmmoQuantity();
    const int shellHP=shell?shell->GetUnitRPG()->GetTotalVP():0;const bool command=actor->HasCommand();
    GlobalGameRandom().SeedForHarness(0x72b9);auto beforeRandom=GlobalGameRandom();
    const int clear=query();
    std::printf("native cover reach control: mode=%d score=%d target=%d reachable=%d grids=%zu pk=%d\n",mode,clear,probe->targetIntervals,probe->reachableIntervals,probe->traces.size(),int(pk));std::fflush(stdout);
    Require(clear>0 && probe->targetIntervals>0 && probe->reachableIntervals>0,"cover reach fixture lacks its actually reached positive native target control");
    probe->Reset();
    if(mode==4) {
        // An actual open point above the same native world keeps tile traversal default-open.
        probe->inspectTarget=false;
        CVec3 tileDirection=probe->aim-source;Normalize(&tileDirection);
        const CVec3 tile=source+tileDirection*2.f;
        CObj<NRPG::CCoverInfo> cover=game->CalcCoversForTile(source,attacks.front(),actor,tile,0.f);
        const int score=NRPG::GetHitCover(actor,cover);
        std::printf("native cover reach tile: score=%d grids=%zu\n",score,probe->traces.size());std::fflush(stdout);
        Require(score>0 && probe->traces.size()==1,"requiring a unit target also blocked the native open tile control");
    } else {
        // A controlled shorter aim distance exercises the native finite walk;
        // the delegating map retains the full native target projection/geometry.
        probe->shortenAim=shortened;ignoreTarget=mode==7;const int actual=query();
        std::printf("native cover reach result: mode=%d score=%d target=%d reachable=%d beyond=%d grids=%zu shortened=%d\n",mode,actual,
            probe->targetIntervals,probe->reachableIntervals,probe->beyondIntervals,probe->traces.size(),int(shortened));std::fflush(stdout);
        Require(probe->traces.size()==(human?2u:1u) && probe->targetIntervals>0,"cover reach query lost its native target intersections/projection grids");
        if(shortened) {
            Require(probe->reachableIntervals==0 && probe->beyondIntervals==probe->targetIntervals,"cover reach fixture did not put all actual target intersections beyond the finite traversal");
            Require(actual<=0,"a native unit cover ray succeeded although traversal never reached the target");
        } else if(mode==7) {
            Require(probe->reachableIntervals>0 && probe->beyondIntervals==0,"ignored-target control lost its genuinely in-range native geometry");
            Require(actual==0,"a native cover query counted an ignored target as reached");
        } else Require(actual==clear && actual>0 && probe->reachableIntervals>0,"requiring the unit target changed an actually reached native target");
    }
    Require(GlobalGameRandom().Get()==beforeRandom.Get(),"cover reach queries consumed native RNG");
    Require(actor->GetPosition()==actorPosition && target->GetPosition()==targetPosition && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp
        && target->GetUnitRPG()->GetTotalVP()==targetHP && firearm->GetAmmoQuantity()==ammo && actor->HasCommand()==command
        && (!pk || (target->IsWearingPK() && target->GetDecalsRef()==shell.GetPtr() && shell->GetUnitRPG()->GetTotalVP()==shellHP)),
        "cover reach queries changed AP/HP/ammo/commands/positions or the fork's PK pilot/surface-owner relation");
}

static void AICoverWeaponOrigin(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> campaign; auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> all; world->GetAllUnits(&all);
    NWorld::CUnitServer *actor=nullptr,*target=nullptr; NRPG::CWeaponItem* weapon=nullptr; NRPG::IWeaponItem* firearm=nullptr;
    for (const auto& unit: all) {
        if (!IsValid(unit) || !unit->CanFight() || unit->HasCommand() || unit->IsEmptyPK() || unit->IsWearingPK()
                || unit->animator.GetCannon() || !unit->IsAIUnit() || !IsValid(NAI::GetAIUnit(unit))) continue;
        auto* candidate=dynamic_cast<NRPG::IWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        auto* item=dynamic_cast<NRPG::CWeaponItem*>(candidate);
        if (!candidate || !item || candidate->GetAmmoQuantity()<1 || !candidate->SetShootMode(NDb::SM_Snap)
                || !unit->GetUnitRPG()->GetDBAnimWeapon()) continue;
        actor=unit.GetPtr(); firearm=candidate; weapon=item; break;
    }
    Require(actor && weapon && firearm,"AI cover origin fixture has no native armed actor of its requested control type");
    if (mode==7) {
        NWorld::CPlayer* human=nullptr;
        for (const auto& other: all) if (IsValid(other) && !other->IsAIUnit() && IsValid(other->GetPlayer())) {
            human=dynamic_cast<NWorld::CPlayer*>(other->GetPlayer()); break;
        }
        Require(human,"AI cover origin fixture has no actual human player");
        world->ChangeUnitPlayer(actor,human);
        Require(actor->GetPlayer()==human && !actor->IsAIUnit() && IsValid(NAI::GetAIUnit(actor)),
            "AI cover origin fixture failed to transfer the real armed actor to human control");
    }
    for (const auto& unit: all) if (IsValid(unit) && unit->CanFight() && unit.GetPtr()!=actor
            && actor->GetDiplomacyState(unit)==NDb::DS_ENEMY) {target=unit.GetPtr();break;}
    Require(target,"AI cover origin fixture has no native opposing target");
    auto* unit=NAI::GetAIUnit(actor); auto input=actor->GetPosition();
    if (mode==1) input.pos.p.SetPose(NAI::CM_CROUCH);
    if (mode==2) input.pos.p.SetPose(NAI::CM_LAY);
    if (mode==3) input.pos.p.SetDirection((input.pos.p.GetDirection()+3)&7);
    if (mode==4) {
        auto* network=world->GetPathNetwork(); vector<NAI::SPathPlace> points;
        network->GetNearPlaces(SSphere(input.GetCP(),6.f),&points); bool found=false;
        for (auto point: points) {
            point.SetPose(NAI::CM_STAND); point.SetDirection((input.pos.p.GetDirection()+2)&7);
            if (network->IsPassable(point) && !network->IsLocked(point,true)
                    && fabs(NAI::SPosition(point,network).GetCP()-actor->GetPosition().GetCP())>2.f) {
                input.pos=NAI::SPosition(point,network); found=true;break;
            }
        }
        Require(found,"AI cover origin fixture has no native hypothetical firing position");
    }
    const auto requested=input; const auto position=actor->GetPosition(),targetPosition=target->GetPosition();
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),ammo=firearm->GetAmmoQuantity();
    const bool command=actor->HasCommand(); auto* shared=unit->GetAIState();
    auto planned=input.pos; planned.p.SetDirection((planned.p.GetDirection()+1)&7); unit->SetPosition(planned); unit->SetHP(-11,99); unit->SetAP(-21,101);
    auto plannedPose=[&]() { CStructureNetworkContext refs;auto fields=BorrowedObjectFields(unit,&refs);NAI::SUnitPosition result;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(4,&result);}return result; };
    const auto planBefore=plannedPose(); const CVec3 muzzle=actor->GetAttackOrigin(input,false),eye=input.GetEyePosition();
    Require(fabs(muzzle-eye)>.01f,"AI cover origin fixture did not separate native weapon and eye geometry");
    NRPG::IGame* game=world->GetGame(); CPtr<NAI::IAIMap> original;
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(game,&refs);
        CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&original);}
    Require(original.GetPtr()==world->GetAIMap(),"AI cover origin fixture does not own the expected native geometry map");
    CObj<CoverOriginMapProbe> probe=new CoverOriginMapProbe(original);
    struct Restore {NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}} restore{game,original};
    SetCoverProbeMap(game,probe);
    auto checkTrace=[&](const CVec3& expected) {
        Require(probe->traces.size()==1 && fabs(probe->traces[0].origin-expected)<2e-4f,
            "AI cover projection was not built from the requested native source point");
    };
    GlobalGameRandom().SeedForHarness(0x713b); auto randomBefore=GlobalGameRandom();
    auto* argument=mode==5 ? nullptr : weapon; int expected=0;
    if (mode!=6) {
        vector<NRPG::CAttackPortion> attacks; if (argument) argument->CreateNewAttackPortion(&attacks,false);
        if (attacks.empty()) attacks.push_back(NRPG::CAttackPortion(1,0,0.f,0,0,0));
        expected=game->GetCoverForAIUnit(muzzle,actor,target,attacks.front(),NAI::HL_ANY); checkTrace(muzzle);
        probe->traces.clear();
        game->GetCoverForAIUnit(eye,actor,target,attacks.front(),NAI::HL_ANY); checkTrace(eye);
        probe->traces.clear();
    }
    const int actual=unit->GetCoverForFixedUnit(input,mode==6 ? nullptr : target,argument,NAI::HL_ANY);
    const float error=probe->traces.empty() ? 0.f : fabs(probe->traces[0].origin-muzzle);
    std::printf("AI cover origin mode=%d calls=%zu error=%g eyeDelta=%g score=%d/%d\n",mode,probe->traces.size(),error,fabs(muzzle-eye),actual,expected);
    Require(mode==6 ? probe->traces.empty() && actual==0 : probe->traces.size()==1 && error<2e-4f && actual==expected,
        "fixed-unit AI cover used the eyes instead of the native weapon origin for the supplied position");
    Require(GlobalGameRandom().Get()==randomBefore.Get(),"cover-origin queries consumed native RNG");
    Require(actor->GetPosition()==position && target->GetPosition()==targetPosition && actor->GetAP()==ap
            && actor->GetUnitRPG()->GetTotalVP()==hp && firearm->GetAmmoQuantity()==ammo && actor->HasCommand()==command
            && unit->GetAIState()==shared && unit->GetAP()==-21 && unit->GetHP()==-11 && plannedPose()==planBefore && input==requested,
        "AI cover-origin queries changed native/planning state, ammunition, commands or the hypothetical input");
}

static void AINearestContact(const char* databasePath, const char* resourcePath, int kind, int profile)
{
    CObj<NRPG::CGlobalGame> game; auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> all; world->GetAllUnits(&all);
    NWorld::CUnitServer *actor=nullptr,*nearServer=nullptr;
    for (const auto& unit: all)
        if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && !unit->HasCommand() && IsValid(NAI::GetAIUnit(unit))) { actor=unit.GetPtr(); break; }
    Require(actor,"nearest contact fixture has no native AI owner");
    for (const auto& unit: all)
        if (IsValid(unit) && unit.GetPtr()!=actor && unit->CanFight() && IsValid(NAI::GetAIUnit(unit))
                && ((actor->GetDiplomacyState(unit)==NDb::DS_ENEMY)==(kind==1))) { nearServer=unit.GetPtr(); break; }
    Require(nearServer,"nearest contact fixture has no eligible native ally/suspect");
    auto* network=world->GetPathNetwork(); vector<NAI::SPathPlace> points;
    network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),12.f),&points);
    NAI::SPosition nearPoint,farPoint; bool found=false;
    for (auto n: points) {
        n.SetPose(NAI::CM_STAND); n.SetDirection(0); NAI::SPosition np(n,network);
        const float distance=fabs(np.GetCP()-actor->GetPosition().GetCP());
        if (distance<2.f || distance>3.5f || !network->IsPassable(n) || network->IsLocked(n,true)) continue;
        for (auto f: points) {
            f.SetPose(NAI::CM_STAND); f.SetDirection(0); NAI::SPosition fp(f,network);
            if (fabs(fp.GetCP()-actor->GetPosition().GetCP())>6.f && network->IsPassable(f) && !network->IsLocked(f,true)) {
                nearPoint=np; farPoint=fp; found=true; break;
            }
        }
        if (found) break;
    }
    Require(found,"nearest contact fixture has no genuine separated native positions");
    auto pose=nearServer->GetPosition(); pose.pos=nearPoint; nearServer->SetPosition(pose);
    const bool tie=profile==3 || profile==7;
    auto* farServer=world->AddUnitInGame((tie ? nearPoint : farPoint).p,
        NRPG::CreateUnit(nearServer->GetUnitRPG()->GetRPGUnit()->GetPers()),nearServer->GetTBSPlayer());
    // Native deployment can relocate a new unit; establish the explicit exact-tie pose afterwards.
    if (tie && IsValid(farServer)) { auto tied=farServer->GetPosition(); tied.pos=nearServer->GetPosition().pos; farServer->SetPosition(tied); }
    auto* owner=NAI::GetAIUnit(actor); auto* nearUnit=NAI::GetAIUnit(nearServer); auto* farUnit=NAI::GetAIUnit(farServer);
    Require(IsValid(farServer) && farServer->CanFight() && owner && nearUnit && farUnit,"nearest contact fixture could not create its native second candidate");
    owner->Synchronize(false); nearUnit->Synchronize(false); farUnit->Synchronize(false);
    NAI::SAIUnitState state; state.SetUnit(owner);
    auto add=[&](NAI::IAIUnit* unit) { if (kind==1) state.AddPossibleEnemy(unit); else state.AddAlly(unit); };
    if (tie) { add(farUnit); add(nearUnit); } else { add(nearUnit); add(farUnit); }
    auto& collection=kind==1 ? state.possibleEnemies : state.allies;
    Require(collection.data.units.size()==2 && collection.data.positions.size()==2,
        "nearest contact fixture failed to add genuine contacts through the native diplomacy/fightability protocol");
    auto* expected=tie ? farUnit : nearUnit; auto* remaining=tie ? nearUnit : farUnit;
    if (profile==4) {
        nearServer->GetUnitRPG()->Kill(); nearServer->KillUnit(VNULL3);
        Require(IsValid(nearUnit) && nearServer->IsDead() && !nearServer->CanFight(),
            "nearest contact corpse fixture did not retain a valid native corpse wrapper");
    }
    if (profile==6) {
        nearServer->MakeUnconscious(VNULL3,true,false);
        Require(!nearServer->CanFight() && !nearServer->IsDead(),"nearest contact KO control has the wrong native life state");
    }
    for (auto* unit: {owner,nearUnit,farUnit}) { unit->SetHP(37,99); unit->SetAP(-43,101); }
    if (profile==1 || profile==3 || profile==4) expected->SetHP(0,99);
    if (profile==2) expected->SetHP(-37,99);
    if (profile==5) {
        owner->SetPosition(farPoint); nearUnit->SetPosition(farPoint); farUnit->SetPosition(actor->GetPosition().pos);
        Require(nearUnit->GetPosition()==nearServer->GetPosition().pos && farUnit->GetPosition()==farServer->GetPosition().pos,
            "nearest contact control replaced the existing live getter with its planning copy");
    }
    const float chosenDistance=fabs(expected->GetPosition().GetCP()-owner->GetPosition().GetCP());
    const float otherDistance=fabs(remaining->GetPosition().GetCP()-owner->GetPosition().GetCP());
    Require(tie ? chosenDistance==otherDistance : chosenDistance<otherDistance,
        "nearest contact fixture did not establish its actual expected closest/tied contact");
    auto plannedPose=[](NAI::IAIUnit* unit) {
        CStructureNetworkContext refs; auto fields=BorrowedObjectFields(unit,&refs); NAI::SUnitPosition result;
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(4,&result); } return result;
    };
    struct Snapshot { NAI::IAIUnit* unit; NAI::SUnitPosition live,plan; int ap,hp,planAP,planHP; bool command; NAI::SAIState* state; };
    vector<Snapshot> snapshots;
    for (auto* unit: {owner,nearUnit,farUnit}) {
        auto* server=unit->GetUnitServer();
        snapshots.push_back({unit,server->GetPosition(),plannedPose(unit),server->GetAP(),server->GetUnitRPG()->GetTotalVP(),
            unit->GetAP(),unit->GetHP(),server->HasCommand(),unit->GetAIState()});
    }
    auto* commander=dynamic_cast<NAI::CAICommander*>(actor->GetPlayer()->GetCommander());
    Require(commander,"nearest contact fixture lost its native owner commander");
    const auto roster=commander->GetUnitsList(); const auto contacts=collection.data.units; const auto positions=collection.data.positions;
    GlobalGameRandom().SeedForHarness(0x61aa); auto reference=GlobalGameRandom();
    auto selected=[&]() -> NAI::IAIUnit* {
        if (kind==2) return NAI::FindNearestUnit(owner,collection.data.units);
        state.Update(); return kind==1 ? state.pPossibleEnemy.GetPtr() : state.pAlly.GetPtr();
    };
    auto* actual=selected();
    std::printf("nearest contact kind=%d profile=%d planHP=%d liveDead=%d fightable=%d selected=%d distance=%g/%g\n",
        kind,profile,expected->GetHP(),int(expected->GetUnitServer()->IsDead()),int(expected->GetUnitServer()->CanFight()),int(actual==expected),chosenDistance,otherDistance);
    Require(actual==expected,"nearest contact selection filtered a valid world contact using planning HP or chose another list-order tie");
    Require(collection.data.units==contacts && collection.data.positions==positions,
        "nearest selection changed actual cached contact membership or positions");
    if (kind!=2) {
        Require(state.IsModified() && state.selfModified.bModified && !collection.bModified,
            "changed nearest contact did not preserve the existing threat-notification/collection-consumption protocol");
        state.selfModified.data=state.selfModified.bModified=false; collection.SetModified();
        Require(selected()==expected && !state.IsModified() && !state.selfModified.bModified && !collection.bModified,
            "a stable nearest contact synthesized another threat notification");
    } else Require(!state.IsModified() && !state.selfModified.bModified && collection.bModified,
        "shared nearest query mutated its caller's native notification/collection state");
    CObj<NAI::IAIEvent> removal=kind==1 ? NAI::CreateAILostPossibleEnemyEvent(expected) : NAI::CreateAILostAllyEvent(expected);
    state.OnAIEvent(removal);
    Require(std::find(collection.data.units.begin(),collection.data.units.end(),expected)==collection.data.units.end()
            && collection.data.positions.find(expected)==collection.data.positions.end(),"actual removal event retained the old nearest contact");
    Require(selected()==remaining && collection.data.units.size()==1 && collection.data.positions.size()==1,
        "nearest selection did not honor an actual contact-removal event");
    const unsigned next=GlobalGameRandom().Get();
    Require(next==reference.Get(),"nearest contact queries or removal consumed native RNG");
    Require(commander->GetUnitsList()==roster,"nearest contact selection changed commander ownership");
    for (const auto& saved: snapshots) {
        auto* server=saved.unit->GetUnitServer();
        Require(server->GetPosition()==saved.live && plannedPose(saved.unit)==saved.plan && server->GetAP()==saved.ap
                && server->GetUnitRPG()->GetTotalVP()==saved.hp && saved.unit->GetAP()==saved.planAP && saved.unit->GetHP()==saved.planHP
                && server->HasCommand()==saved.command && saved.unit->GetAIState()==saved.state,
            "nearest contact queries changed world/planning AP/HP/positions/commands or tactical ownership");
    }
    std::printf("nearest contact completed kind=%d profile=%d rng=%u\n",kind,profile,next);
}

static void AITacticalWorldViews(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game; auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> all; world->GetAllUnits(&all);
    NWorld::CUnitServer *actor=nullptr,*buddy=nullptr,*human=nullptr;
    for (const auto& unit: all) if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && IsValid(NAI::GetAIUnit(unit))) { actor=unit.GetPtr(); break; }
    Require(actor,"tactical view fixture has no native AI owner");
    for (const auto& unit: all) {
        if (!IsValid(unit) || !unit->CanFight() || !IsValid(NAI::GetAIUnit(unit))) continue;
        if (!buddy && unit.GetPtr()!=actor && unit->GetPlayer()==actor->GetPlayer()) buddy=unit.GetPtr();
        if (!human && !unit->IsAIUnit() && actor->GetDiplomacyState(unit)==NDb::DS_ENEMY) human=unit.GetPtr();
    }
    auto* commander=dynamic_cast<NAI::CAICommander*>(actor->GetPlayer()->GetCommander());
    Require(commander && buddy && human,"tactical view fixture lacks native commander/own/human controls");
    auto* state=commander->GetAIState(); state->pPlayer=actor->GetPlayer(); state->Synchronize();
    const vector<CPtr<NRPG::IUnitMission>> noUnits; const int callerSide=actor->GetPlayer()->GetScenarioPlayerID();
    CObj<NAI::CAICommander> foreignCommander; NWorld::CPlayer* foreignPlayer=nullptr;
    if (mode==1 || mode==2 || mode==4 || mode==10 || mode==11) {
        game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54})); foreignCommander=new NAI::CAICommander(world,nullptr);
        foreignPlayer=dynamic_cast<NWorld::CPlayer*>(world->AddPlayer(L"Native tactical view party",game->players.back(),foreignCommander));
        Require(foreignPlayer,"tactical view fixture failed to create an actual foreign AI party"); foreignCommander->SetPlayer(foreignPlayer);
        world->GetDiplomacy()->SetDiplomacyState(callerSide,noUnits,foreignPlayer->GetScenarioPlayerID(),
            mode==2 ? NDb::DS_ALLY : (mode==4 || mode==10) ? NDb::DS_ENEMY : NDb::DS_NEUTRAL);
    }
    if (mode==3) {
        world->GetDiplomacy()->SetDiplomacyState(callerSide,noUnits,human->GetPlayer()->GetScenarioPlayerID(),NDb::DS_NEUTRAL);
        Require(actor->GetDiplomacyState(human)==NDb::DS_ENEMY,"tactical view fixture erased its distinct individual hostility");
    }
    if (mode==5) state->pPlayer=human->GetPlayer();
    if (mode==6) state->pPlayer=nullptr;
    if (mode==7) state->pAICommander=nullptr;
    if (mode==8) state->pWorld=nullptr;
    if (mode==9) { buddy->MakeUnconscious(VNULL3,true,false); Require(!buddy->CanFight(),"tactical own-KO control remained fightable"); }
    if (mode==11) {
        world->ChangeUnitPlayer(buddy,foreignPlayer);
        Require(buddy->GetPlayer()==foreignPlayer && IsValid(NAI::GetAIUnit(buddy)),"tactical view reownership failed to replace the native wrapper");
    }
    world->GetAllUnits(&all);
    if (mode==12) for (const auto& unit: all) if (IsValid(unit) && unit->CanFight()) unit->MakeUnconscious(VNULL3,true,false);
    vector<CPtr<NWorld::CPlayer>> players; world->GetPlayersList(&players);
    struct Roster { NAI::CAICommander* commander; vector<CObj<NAI::IAIUnit>> units; };
    vector<Roster> rosters;
    for (const auto& player: players) if (auto* current=dynamic_cast<NAI::CAICommander*>(player->GetCommander())) rosters.push_back({current,current->GetUnitsList()});
    struct Snapshot { NAI::IAIUnit* unit; NAI::SAIState* expectedState; NAI::SUnitPosition pose; int ap,hp,planAP,planHP; bool command; };
    vector<Snapshot> before;
    for (const auto& server: all) {
        auto* unit=NAI::GetAIUnit(server); if (!IsValid(server) || !IsValid(unit)) continue;
        const bool owned=server->GetPlayer()==actor->GetPlayer();
        if (owned) unit->SetAIState(nullptr); // Real restore seam: raw state backref is not serialized.
        unit->SetHP(37,99); auto projected=server->GetPosition().pos; projected.p.SetDirection((projected.p.GetDirection()+3)&7); unit->SetPosition(projected);
        before.push_back({unit,owned && mode!=7 ? state : unit->GetAIState(),server->GetPosition(),server->GetAP(),server->GetUnitRPG()->GetTotalVP(),unit->GetAP(),unit->GetHP(),server->HasCommand()});
    }
    const int passes=mode==4 ? 2 : 1;
    for (int pass=0; pass<passes; ++pass) {
        if (pass) world->GetDiplomacy()->SetDiplomacyState(callerSide,noUnits,foreignPlayer->GetScenarioPlayerID(),NDb::DS_NEUTRAL);
        vector<CPtr<NAI::IAIUnit>> expectedAllies,expectedEnemies;
        state->GetUnits(&expectedAllies,true,true); state->GetUnits(&expectedEnemies,false,true);
        // Separately check every expected identity against native global diplomacy and fightability.
        for (const auto& saved: before) {
            auto* server=saved.unit->GetUnitServer(); const bool fighter=IsValid(state->GetWorld()) && server->CanFight();
            const bool nonEnemy=world->GetDiplomacyState(state->pPlayer,server->GetPlayer())!=NDb::DS_ENEMY;
            Require(std::count(expectedAllies.begin(),expectedAllies.end(),saved.unit)==int(fighter && nonEnemy)
                    && std::count(expectedEnemies.begin(),expectedEnemies.end(),saved.unit)==int(fighter && !nonEnemy),
                "tactical expected views disagree with native global diplomacy or live fightability");
        }
        auto* allies=state->GetAllyAIPlayer()->GetUnits(); auto* enemies=state->GetEnemyAIPlayer()->GetUnits();
        allies->assign(2,NAI::GetAIUnit(human)); enemies->assign(2,NAI::GetAIUnit(actor));
        GlobalGameRandom().SeedForHarness(0x51aa); auto randomBefore=GlobalGameRandom();
        state->Synchronize(); const unsigned next=GlobalGameRandom().Get();
        std::printf("tactical views mode=%d pass=%d allies=%zu/%zu enemies=%zu/%zu owned=%zu rng=%u\n",mode,pass,
            allies->size(),expectedAllies.size(),enemies->size(),expectedEnemies.size(),commander->GetUnitsList().size(),next);
        Require(*allies==expectedAllies && *enemies==expectedEnemies,
            "legacy tactical views used commander ownership instead of saved-player non-enemy/enemy world sets");
        Require(next==randomBefore.Get(),"tactical view rebuild consumed native RNG");
        for (const auto& saved: before) {
            auto* server=saved.unit->GetUnitServer();
            Require(saved.unit->GetAIState()==saved.expectedState,
                "tactical view synchronization skipped an owned nonfighter backref or rebound a foreign unit");
            Require(server->GetPosition()==saved.pose && server->GetAP()==saved.ap && server->GetUnitRPG()->GetTotalVP()==saved.hp
                    && server->HasCommand()==saved.command && saved.unit->GetAP()==saved.planAP && saved.unit->GetHP()==saved.planHP,
                "tactical view rebuild synchronized planning data or changed native AP/HP/positions/commands");
        }
        for (const auto& roster: rosters) Require(roster.commander->GetUnitsList()==roster.units,
            "building tactical world views changed a commander's strongly owned roster");
        if (mode!=7) {
            const auto& groups=state->GetEnemyGroups(); Require(groups.size()==expectedEnemies.size(),"legacy groups were not refreshed from the current enemy view");
            for (const auto& group: groups) {
                for (const auto& enemy: group.enemies) Require(std::find(expectedEnemies.begin(),expectedEnemies.end(),enemy)!=expectedEnemies.end(),"legacy group retained an ineligible enemy");
                for (const auto& ally: group.allies) Require(std::find(expectedAllies.begin(),expectedAllies.end(),ally)!=expectedAllies.end(),"legacy group retained an ineligible non-enemy");
            }
        }
    }
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
        state->enemies.data.units.size(), state->possibleEnemies.data.units.size(), int(IsValid(state->pEnemy)), int(IsValid(state->pPossibleEnemy)));
    if (staleOwner) {
        Require(state->enemies.data.units.empty() && state->possibleEnemies.data.units.size() == 1
                && state->possibleEnemies.data.units.front().GetPtr() == contact,
            "cached zero watcher HP suppressed the live begin-turn contact refresh");
    } else {
        Require(state->enemies.data.units.size() == 1 && state->enemies.data.units.front().GetPtr() == contact
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

static void AIReactionContactDelivery(const char* databasePath, const char* resourcePath, int mode)
{
    // Exercise pre-existing dirty marks before the failing clean-collection case.
    for (bool dirty: {true, false}) {
        CObj<NRPG::CGlobalGame> game;
        auto world = FixtureWorld(databasePath, resourcePath, true, &game);
        list<CPtr<NWorld::CUnitServer>> units; world->GetAllUnits(&units);
        NWorld::CUnitServer *actor = nullptr, *enemy = nullptr, *ally = nullptr;
        NAI::IAIUnit* owner = nullptr;
        for (const auto& unit: units) {
            auto* wrapper = NAI::GetAIUnit(unit);
            if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && !unit->HasCommand()
                    && unit->GetUnitRPG()->CanMove() && IsValid(wrapper)) {
                actor = unit.GetPtr(); owner = wrapper; break;
            }
        }
        Require(actor && owner, "reaction contact fixture has no mobile idle native AI actor");
        for (const auto& unit: units) {
            if (!IsValid(unit) || unit.GetPtr() == actor || !unit->CanFight() || !IsValid(NAI::GetAIUnit(unit))) continue;
            if (actor->GetDiplomacyState(unit) == NDb::DS_ENEMY) enemy = unit.GetPtr();
            else ally = unit.GetPtr();
        }
        Require(enemy && ally, "reaction contact fixture needs native hostile and allied contacts");
        auto* commander = dynamic_cast<NAI::CAICommander*>(actor->GetPlayer()->GetCommander());
        Require(commander != nullptr, "reaction contact fixture has no native commander");
        commander->GetAIState()->Synchronize();
        owner->SetLogic(nullptr); owner->SetRouteLogic(nullptr);
        owner->Synchronize();
        auto* state = owner->GetAIUnitState();
        Require(state != nullptr, "reaction contact fixture has no native threat state");
        state->enemies.data.units.clear(); state->enemies.data.positions.clear();
        state->possibleEnemies.data.units.clear(); state->possibleEnemies.data.positions.clear();
        state->allies.data.units.clear(); state->allies.data.positions.clear();
        state->pEnemy = state->pPossibleEnemy = state->pAlly = nullptr;
        state->bScared = mode == 5;
        auto* target = (mode == 1 || mode == 4) ? ally : enemy;
        auto* contact = NAI::GetAIUnit(target);
        const auto targetPosition = target->GetPosition();
        auto plan = targetPosition.pos; plan.p.SetDirection((plan.p.GetDirection()+3)&7);
        contact->SetPosition(plan);
        Require(contact->GetUnitPosition() == targetPosition && target->GetPosition() == targetPosition,
            "reaction contact control replaced live position with the separate planning copy");
        if (mode == 1 || mode == 4) { state->AddAlly(contact); state->pAlly = contact; }
        else if (mode == 5) { state->AddEnemy(contact); state->pEnemy = contact; }
        else { state->AddPossibleEnemy(contact); state->pPossibleEnemy = contact; }
        state->enemies.nLock = state->possibleEnemies.nLock = state->allies.nLock = 0;
        state->enemies.bModified = state->possibleEnemies.bModified = state->allies.bModified = dirty;
        state->selfModified.data = state->selfModified.bModified = false;
        CObj<NAI::CAIReaction> reaction;
        NAI::SPathPlace fallback = ally->GetPosition().pos.p;
        if (mode == 0 || mode == 1) reaction = NAI::CreateAIGuardReaction(owner, nullptr, 12);
        else if (mode == 2) {
            Require(fabs(ally->GetPosition().GetCP()-actor->GetPosition().GetCP()) >= .5f,
                "retreat contact fixture already reached its fallback");
            reaction = NAI::CreateAIRetreatReaction(owner, fallback);
        } else if (mode == 3) reaction = NAI::CreateAIFearReaction(owner, false, false, 0);
        else reaction = NAI::CreateAINormalReaction(owner);
        Require(IsValid(reaction), "reaction contact fixture could not construct its native reaction");
        owner->SetReaction(reaction);
        Require(owner->GetReactionForUpdate() == reaction.GetPtr(),
            "native reaction selection did not consume its initial installation notification");
        unsigned seed = 2, expectedNext = 0;
        CObj<NAI::IAILogic> referencePlan;
        if (mode == 5) {
            auto* allies = owner->GetAIState()->GetAllyAIPlayer();
            Require(allies && allies->GetUnits()->size() > 1, "alarm contact fixture lacks native garrison support");
            for (seed = 1; seed != 513; ++seed) {
                GlobalGameRandom().SeedForHarness(seed);
                if (GlobalGameRandom().Get(100) >= 40) continue;
                referencePlan = NAI::CreateAIAlarmLogic(owner, contact);
                if (IsValid(referencePlan)) break;
            }
            Require(seed != 513 && IsValid(referencePlan), "alarm contact fixture found no actual reachable seeded alarm plan");
            expectedNext = GlobalGameRandom().Get();
        } else {
            GlobalGameRandom().SeedForHarness(seed);
            if (mode <= 1) {
                CObj<NAI::CUnitArea> area = new NAI::CUnitArea(actor, actor->GetPosition().pos.p, 12, NAI::WALK);
                Require(area->Prepare(), "guard contact fixture failed to prepare its native area");
                referencePlan = NAI::CreateAICheckPositionLogic(owner, area, targetPosition.pos, NAI::CROUCH);
            } else if (mode == 4)
                referencePlan = NAI::CreateAICheckForEnemyLogic(owner, targetPosition, NAI::RUN, nullptr);
            else referencePlan = NAI::CreateAILookToPositionLogic(owner, targetPosition.pos.p);
            Require(IsValid(referencePlan), "reaction contact fixture could not build the native reference plan");
            expectedNext = GlobalGameRandom().Get();
        }
        const int ap = actor->GetAP(), hp = actor->GetUnitRPG()->GetTotalVP(); const auto position = actor->GetPosition();
        GlobalGameRandom().SeedForHarness(seed);
        NAI::SAIUpdateTracker tracker; tracker.Add(reaction);
        Require(!tracker.CanGetCommand() && reaction->CanUpdate(), "native reaction pump did not queue an eligible contact reaction");
        tracker.Update();
        const unsigned next = GlobalGameRandom().Get();
        Require(tracker.CanGetCommand() && owner->GetReaction() == reaction.GetPtr() && IsValid(owner->GetLogic()),
            "native reaction did not handle its controlled contact through the reaction pump");
        auto commands = [](NAI::IAILogic* logic) {
            Require(dynamic_cast<NAI::CAIRouteLogic*>(logic) != nullptr, "reaction installed a non-route plan");
            CStructureNetworkContext refs; auto saved = BorrowedObjectFields(logic, &refs);
            vector<CObj<NAI::CTaskCommand>> result;
            { CStructureSaver reader(saved, CStructureSaver::READ, &refs); reader.Add(4, &result); }
            return result;
        };
        const auto actualCommands = commands(owner->GetLogic()), expectedCommands = commands(referencePlan);
        Require(!actualCommands.empty() && actualCommands.size() == expectedCommands.size(), "contact delivery changed its native plan command count");
        int looks = 0, alarms = 0;
        for (std::size_t i = 0; i != actualCommands.size(); ++i) {
            Require(typeid(*actualCommands[i]) == typeid(*expectedCommands[i]), "contact delivery changed its native plan command sequence");
            if (dynamic_cast<NAI::CTaskCommandLookToPosition*>(actualCommands[i].GetPtr())) {
                CStructureNetworkContext refs; auto saved = BorrowedObjectFields(actualCommands[i], &refs);
                NAI::SPathPlace looked;
                { CStructureSaver reader(saved, CStructureSaver::READ, &refs); reader.Add(3, &looked); }
                if (mode != 5) Require(looked == targetPosition.pos.p, "reaction looked at a speculative or unrelated target place");
                ++looks;
            }
            if (dynamic_cast<NAI::CTaskCommandAlarm*>(actualCommands[i].GetPtr())) ++alarms;
        }
        Require(looks > 0 && alarms == (mode == 5 ? 1 : 0) && next == expectedNext,
            "contact delivery changed the native plan role or consumed different random words");
        Require(!IsValid(state->pEnemy) && !IsValid(state->pPossibleEnemy) && !IsValid(state->pAlly)
                && state->enemies.data.units.empty() && state->possibleEnemies.data.units.empty() && state->allies.data.units.empty()
                && state->enemies.data.positions.empty() && state->possibleEnemies.data.positions.empty() && state->allies.data.positions.empty(),
            "reaction failed to consume its actual contact membership, pose and selected reference");
        std::printf("reaction mode=%d oldDirty=%d flags=%d,%d,%d commands=%zu looks=%d alarms=%d seed=%u rng=%u\n",
            mode, int(dirty), int(state->enemies.bModified), int(state->possibleEnemies.bModified), int(state->allies.bModified),
            actualCommands.size(), looks, alarms, seed, next);
        Require(state->enemies.bModified == dirty && state->possibleEnemies.bModified == dirty && state->allies.bModified == dirty,
            "reaction contact consumption armed new dirty flags or discarded existing marks");
        Require(state->enemies.nLock == 0 && state->possibleEnemies.nLock == 0 && state->allies.nLock == 0 && !state->IsModified(),
            "reaction contact consumption retained collection locks or synthesized another threat notification");
        const int expectedAP = mode == 3 ? actor->GetUnitRPG()->GetActionAP(owner->GetUnitPosition().GetPose(), NRPG::AC_ROTATE) : ap;
        Require(actor->GetAP() == expectedAP && actor->GetUnitRPG()->GetTotalVP() == hp && actor->GetPosition() == position
                && target->GetPosition() == targetPosition && !actor->HasCommand(),
            "reaction contact delivery violated its original AP rule, changed HP/position or issued a gameplay order");
    }
}

static void AIAlarmDelivery(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer *actor = nullptr, *enemy = nullptr, *recipient = nullptr;
    const bool humanOwner = mode == 8;
    for (const auto& unit: units)
        if (IsValid(unit) && unit->CanFight() && !unit->HasCommand() && unit->IsAIUnit() != humanOwner
                && IsValid(NAI::GetAIUnit(unit))) { actor = unit.GetPtr(); break; }
    Require(actor != nullptr, "alarm delivery fixture has no native idle caller");
    for (const auto& unit: units) {
        if (!IsValid(unit) || !unit->CanFight() || !IsValid(NAI::GetAIUnit(unit))) continue;
        if (!enemy && actor->GetDiplomacyState(unit) == NDb::DS_ENEMY) enemy = unit.GetPtr();
        if (!recipient && unit.GetPtr() != actor && unit->GetPlayer() == actor->GetPlayer()) recipient = unit.GetPtr();
    }
    if (mode == 7) {
        recipient = enemy;
        // Qualify the human for the original global non-enemy query, while
        // retaining the caller's authored individual hostility to the contact.
        const vector<CPtr<NRPG::IUnitMission>> noUnits;
        world->GetDiplomacy()->SetDiplomacyState(actor->GetPlayer()->GetScenarioPlayerID(),noUnits,
            enemy->GetPlayer()->GetScenarioPlayerID(),NDb::DS_NEUTRAL);
        Require(world->GetDiplomacyState(actor->GetPlayer(),enemy->GetPlayer()) == NDb::DS_NEUTRAL
                && actor->GetDiplomacyState(enemy) == NDb::DS_ENEMY,
            "alarm human control lost the native global/individual diplomacy distinction");
    }
    if (humanOwner) recipient = actor;
    Require(enemy && recipient, "alarm delivery fixture lacks native hostile/recipient controls");
    auto* owner = NAI::GetAIUnit(actor); auto* contact = NAI::GetAIUnit(enemy); auto* receiver = NAI::GetAIUnit(recipient);
    auto* commander = dynamic_cast<NAI::CAICommander*>(actor->GetPlayer()->GetCommander());
    Require(owner && contact && receiver && commander, "alarm delivery fixture has no native wrappers/commander");
    auto* shared = commander->GetAIState(); shared->Synchronize(); owner->SetAIState(shared);
    if (recipient != actor) recipient->SetPosition(actor->GetPosition());
    if (mode == 11) {
        vector<NAI::SPathPlace> places;
        world->GetPathNetwork()->GetNearPlaces(SSphere(actor->GetPosition().GetCP(), 12.f), &places);
        bool placed = false;
        for (auto place: places) {
            place.SetPose(NAI::CM_STAND); place.SetDirection(0);
            NAI::SPosition candidate(place,world->GetPathNetwork());
            if (world->GetPathNetwork()->IsPassable(place) && fabs(candidate.GetCP()-actor->GetPosition().GetCP()) > 6.f) {
                auto position = recipient->GetPosition(); position.pos = candidate; recipient->SetPosition(position); placed = true; break;
            }
        }
        Require(placed, "alarm delivery fixture found no native far-recipient control");
    }
    if (mode == 4) actor->MakeUnconscious(VNULL3,true,false);
    if (mode == 5) enemy->MakeUnconscious(VNULL3,true,false);
    if (mode == 6) recipient->MakeUnconscious(VNULL3,true,false);
    Require(actor->CanFight() == (mode != 4) && enemy->CanFight() == (mode != 5)
            && recipient->CanFight() == (mode != 6), "alarm fixture has incorrect native unconsciousness controls");
    owner->SetHP(mode == 1 ? 0 : 37, 99); contact->SetHP(mode == 2 ? 0 : 37, 99);
    if (receiver != owner && receiver != contact) receiver->SetHP(mode == 3 ? 0 : 37, 99);
    vector<NAI::IAIUnit*> wrappers{owner};
    if (contact != owner) wrappers.push_back(contact);
    if (receiver != owner && receiver != contact) wrappers.push_back(receiver);
    for (size_t i=0; i<wrappers.size(); ++i) {
        wrappers[i]->SetAP(17+int(i),71+int(i));
        auto* state = wrappers[i]->GetAIUnitState();
        Require(state != nullptr, "alarm delivery fixture lost its genuine threat owner");
        state->enemies.data.units.clear(); state->enemies.data.positions.clear();
        state->possibleEnemies.data.units.clear(); state->possibleEnemies.data.positions.clear();
        state->allies.data.units.clear(); state->allies.data.positions.clear();
        state->pEnemy = state->pPossibleEnemy = state->pAlly = nullptr;
        state->enemies.nLock = state->possibleEnemies.nLock = state->allies.nLock = 0;
        state->enemies.bModified = state->possibleEnemies.bModified = state->allies.bModified = false;
        state->selfModified.data = state->selfModified.bModified = false;
        state->bHelpCalled = false; state->bScared = wrappers[i] == owner;
    }
    auto* mine = owner->GetAIUnitState(); auto* theirs = receiver->GetAIUnitState();
    const int lock = mode == 9 ? 2 : 0; const bool dirty = mode == 10;
    theirs->possibleEnemies.nLock = lock;
    theirs->enemies.bModified = theirs->possibleEnemies.bModified = theirs->allies.bModified = dirty;
    // Isolate delivery from the separate world-query restoration: feed the
    // existing adapter actual wrappers, including its possible stale human entry.
    auto* roster = shared->GetAllyAIPlayer()->GetUnits(); roster->clear(); roster->push_back(owner);
    if (receiver != owner) roster->push_back(receiver);
    auto plan = enemy->GetPosition().pos; plan.p.SetDirection((plan.p.GetDirection()+3)&7); contact->SetPosition(plan);
    const auto liveEnemyPose = enemy->GetPosition();
    struct Live { NWorld::CUnitServer* server; NAI::SUnitPosition pose; int ap,hp,planAP,planHP; bool command; };
    vector<Live> before;
    for (auto* wrapper: wrappers) {
        auto* server = wrapper->GetUnitServer(); before.push_back({server,server->GetPosition(),server->GetAP(),
            server->GetUnitRPG()->GetTotalVP(),wrapper->GetAP(),wrapper->GetHP(),server->HasCommand()});
    }
    const auto ownedRoster = commander->GetUnitsList();
    GlobalGameRandom().SeedForHarness(43); auto reference = GlobalGameRandom(); const unsigned next = reference.Get();
    CObj<NAI::CTaskCommandAlarm> command = new NAI::CTaskCommandAlarm(mode == 12 ? nullptr : enemy);
    command->SetUnitServer(mode == 13 ? nullptr : actor);
    command->Do();
    const bool validCall = mode != 4 && mode != 5 && mode != 12 && mode != 13;
    const bool selfDelivered = validCall && owner->IsUnderAIControl();
    const bool delivered = validCall && receiver->IsUnderAIControl() && recipient->CanFight()
        && fabs(recipient->GetPosition().GetCP()-actor->GetPosition().GetCP()) < 5.f;
    std::printf("alarm delivery mode=%d live=%d,%d,%d planHP=%d,%d,%d help=%d/%d recipient=%d/%d lock=%d dirty=%d\n", mode,
        int(actor->CanFight()),int(enemy->CanFight()),int(recipient->CanFight()),owner->GetHP(),contact->GetHP(),receiver->GetHP(),
        int(mine->bHelpCalled),int(selfDelivered),int(theirs->selfModified.data),int(delivered),theirs->possibleEnemies.nLock,int(theirs->possibleEnemies.bModified));
    Require(mine->bHelpCalled == selfDelivered && mine->bScared == !selfDelivered,
        "alarm caller/target eligibility used speculative HP or bypassed the native AI-owner event gate");
    auto verify = [&](NAI::SAIUnitState* state, bool received, int inheritedLock, bool inheritedDirty) {
        Require(state->selfModified.data == received && state->selfModified.bModified == received,
            "alarm delivered an AI event to a human/non-fighting recipient or skipped a live recipient with speculative zero HP");
        Require(state->possibleEnemies.data.units.size() == size_t(received) && state->possibleEnemies.data.positions.size() == size_t(received),
            "alarm delivery lost its native suspected-contact membership/pose");
        if (received) {
            const auto found = state->possibleEnemies.data.positions.find(contact);
            Require(state->possibleEnemies.data.units.front().GetPtr() == contact && found != state->possibleEnemies.data.positions.end()
                    && found->second == liveEnemyPose, "alarm used the planning pose or another native contact");
        }
        Require(state->possibleEnemies.nLock == inheritedLock
                && state->possibleEnemies.bModified == (inheritedDirty || (received && inheritedLock == 0))
                && state->enemies.bModified == inheritedDirty && state->allies.bModified == inheritedDirty,
            "alarm changed inherited locks/marks or incorrectly used the reaction Notify bracket");
    };
    verify(mine,selfDelivered,receiver == owner ? lock : 0,receiver == owner ? dirty : false);
    if (receiver != owner) verify(theirs,delivered,lock,dirty);
    Require(!IsValid(mine->pPossibleEnemy) && !IsValid(theirs->pPossibleEnemy)
            && commander->GetUnitsList() == ownedRoster && GlobalGameRandom().Get() == next,
        "alarm prematurely selected a contact, changed commander ownership or consumed native RNG");
    for (size_t i=0; i<wrappers.size(); ++i) {
        auto* server = before[i].server;
        Require(server->GetPosition() == before[i].pose && server->GetAP() == before[i].ap
                && server->GetUnitRPG()->GetTotalVP() == before[i].hp && server->HasCommand() == before[i].command
                && wrappers[i]->GetAP() == before[i].planAP && wrappers[i]->GetHP() == before[i].planHP,
            "alarm event delivery changed world/planning AP/HP, position or issued a world command");
    }
}

static void AIAlarmWorldQuery(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> initial; world->GetAllUnits(&initial);
    NWorld::CUnitServer *actor=nullptr,*enemy=nullptr,*buddy=nullptr;
    for (const auto& unit: initial)
        if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && !unit->HasCommand()
                && IsValid(NAI::GetAIUnit(unit))) { actor=unit.GetPtr(); break; }
    Require(actor,"alarm world fixture has no native AI caller");
    for (const auto& unit: initial) {
        if (!IsValid(unit) || !unit->CanFight() || !IsValid(NAI::GetAIUnit(unit))) continue;
        if (!enemy && actor->GetDiplomacyState(unit)==NDb::DS_ENEMY) enemy=unit.GetPtr();
        if (!buddy && unit.GetPtr()!=actor && unit->GetPlayer()==actor->GetPlayer()) buddy=unit.GetPtr();
    }
    Require(enemy && buddy && !enemy->IsAIUnit(),"alarm world fixture lacks a native human contact and owned AI control");
    auto* owner=NAI::GetAIUnit(actor); auto* contact=NAI::GetAIUnit(enemy);
    auto* commander=dynamic_cast<NAI::CAICommander*>(actor->GetPlayer()->GetCommander());
    Require(owner && contact && commander,"alarm world fixture lacks its genuine commander/wrappers");
    auto* shared=commander->GetAIState(); shared->Synchronize(); owner->SetAIState(shared);
    Require(shared->pPlayer.GetPtr()==actor->GetPlayer(),"alarm world fixture did not retain its constructor's saved player");
    buddy->SetPosition(actor->GetPosition());
    const vector<CPtr<NRPG::IUnitMission>> noUnits;
    const int callerSide=actor->GetPlayer()->GetScenarioPlayerID(), enemySide=enemy->GetPlayer()->GetScenarioPlayerID();
    NWorld::CUnitServer* recipient=buddy;
    NWorld::CPlayer* foreign=nullptr;
    CObj<NAI::CAICommander> foreignCommander;
    if ((mode>=2 && mode<=6) || (mode>=8 && mode<=10) || mode==13) {
        game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        foreignCommander=new NAI::CAICommander(world,nullptr);
        foreign=dynamic_cast<NWorld::CPlayer*>(world->AddPlayer(L"Native alarm query party",game->players.back(),foreignCommander));
        Require(foreign && foreign!=actor->GetPlayer(),"alarm world fixture did not create a distinct native AI party");
        foreignCommander->SetPlayer(foreign);
        list<CPtr<NWorld::CUnitServer>> deployed; world->GetAllUnits(&deployed);
        recipient=nullptr;
        for (const auto& unit: deployed)
            if (IsValid(unit) && unit->GetPlayer()==foreign && unit->CanFight()) { recipient=unit.GetPtr(); break; }
        Require(recipient && recipient->IsAIUnit() && IsValid(NAI::GetAIUnit(recipient))
                && NAI::GetAIUnit(recipient)->IsUnderAIControl(),"alarm world fixture did not deploy a dynamically AI-controlled native recipient");
        Require(foreign->GetScenarioPlayerID()==enemySide && callerSide!=enemySide,
            "alarm world fixture lost its distinct-player/same-scenario identity control");
        const auto relation=(mode==4 || mode==9 || mode==10) ? NDb::DS_ENEMY : mode==3 ? NDb::DS_ALLY : NDb::DS_NEUTRAL;
        world->GetDiplomacy()->SetDiplomacyState(callerSide,noUnits,foreign->GetScenarioPlayerID(),relation);
        Require(world->GetDiplomacyState(actor->GetPlayer(),foreign)==relation
                && actor->GetDiplomacyState(enemy)==NDb::DS_ENEMY && recipient->GetDiplomacyState(enemy)==NDb::DS_ENEMY,
            "alarm query fixture confused global diplomacy with individual hostility/player identity");
        recipient->SetPosition(actor->GetPosition());
    }
    if (mode==7) {
        recipient=enemy; recipient->SetPosition(actor->GetPosition());
        world->GetDiplomacy()->SetDiplomacyState(callerSide,noUnits,enemySide,NDb::DS_NEUTRAL);
        Require(world->GetDiplomacyState(actor->GetPlayer(),enemy->GetPlayer())==NDb::DS_NEUTRAL
                && actor->GetDiplomacyState(enemy)==NDb::DS_ENEMY,"alarm world human fixture changed the authored individual enemy override");
    }
    CPtr<NAI::IAIUnit> oldBuddy;
    if (mode==13) {
        oldBuddy=NAI::GetAIUnit(buddy);
        world->ChangeUnitPlayer(buddy,foreign); recipient=buddy;
        Require(buddy->GetPlayer()==foreign && IsValid(NAI::GetAIUnit(buddy)) && NAI::GetAIUnit(buddy)!=oldBuddy.GetPtr(),
            "alarm reownership fixture did not replace the old native wrapper through the real transfer API");
        buddy->SetPosition(actor->GetPosition());
    }
    if (mode==5) {
        vector<NAI::SPathPlace> places; world->GetPathNetwork()->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),12.f),&places);
        bool placed=false;
        for (auto place: places) {
            place.SetPose(NAI::CM_STAND); place.SetDirection(0); NAI::SPosition point(place,world->GetPathNetwork());
            if (world->GetPathNetwork()->IsPassable(place) && fabs(point.GetCP()-actor->GetPosition().GetCP())>6.f) {
                auto pose=recipient->GetPosition(); pose.pos=point; recipient->SetPosition(pose); placed=true; break;
            }
        }
        Require(placed,"alarm world fixture lacks its actual distant native control");
    }
    if (mode==6) { recipient->MakeUnconscious(VNULL3,true,false); Require(!recipient->CanFight(),"alarm world KO control remained fightable"); }
    if (mode==9) shared->pPlayer=nullptr;
    if (mode==10) {
        shared->pPlayer=foreign;
        world->GetDiplomacy()->SetDiplomacyState(foreign->GetScenarioPlayerID(),noUnits,callerSide,NDb::DS_ENEMY);
    }
    if (mode==11) shared->pAICommander=nullptr;
    if (mode==12) shared->pWorld=nullptr;
    list<CPtr<NWorld::CUnitServer>> all; world->GetAllUnits(&all);
    vector<NAI::IAIUnit*> wrappers;
    for (const auto& unit: all) if (IsValid(unit) && IsValid(NAI::GetAIUnit(unit))) wrappers.push_back(NAI::GetAIUnit(unit));
    auto* cached=shared->GetAllyAIPlayer()->GetUnits(); cached->clear();
    if (mode==2 || mode==3 || mode==9 || mode==10) cached->push_back(owner);
    else if (mode==13) { cached->push_back(owner); cached->push_back(oldBuddy); }
    else if (mode!=0 && mode!=11) {
        for (auto* wrapper: wrappers) {
            if (mode==1 || mode==8) {
                if (world->GetDiplomacyState(shared->pPlayer,wrapper->GetUnitServer()->GetPlayer())==NDb::DS_ENEMY) continue;
            }
            cached->push_back(wrapper);
        }
    }
    const auto cachedBefore=*cached; const auto roster=commander->GetUnitsList();
    const auto savedPlayer=shared->pPlayer; auto* savedWorld=shared->GetWorld(); const auto backRef=shared->pAICommander;
    auto planner=enemy->GetPosition().pos; planner.p.SetDirection((planner.p.GetDirection()+3)&7); contact->SetPosition(planner);
    struct Live { NAI::SUnitPosition pose; int ap,hp; bool command; };
    vector<Live> before;
    for (size_t i=0; i<wrappers.size(); ++i) {
        auto* server=wrappers[i]->GetUnitServer(); wrappers[i]->SetAP(17+int(i),71+int(i)); wrappers[i]->SetHP(37,99);
        before.push_back({server->GetPosition(),server->GetAP(),server->GetUnitRPG()->GetTotalVP(),server->HasCommand()});
    }
    const int passes=mode==8 ? 2 : 1;
    for (int pass=0; pass<passes; ++pass) {
        if (pass) world->GetDiplomacy()->SetDiplomacyState(callerSide,noUnits,foreign->GetScenarioPlayerID(),NDb::DS_ENEMY);
        for (auto* wrapper: wrappers) {
            auto* state=wrapper->GetAIUnitState(); Require(state,"alarm world fixture lost a native threat owner");
            state->enemies.data.units.clear(); state->enemies.data.positions.clear();
            state->possibleEnemies.data.units.clear(); state->possibleEnemies.data.positions.clear();
            state->allies.data.units.clear(); state->allies.data.positions.clear();
            state->pEnemy=state->pPossibleEnemy=state->pAlly=nullptr;
            state->enemies.nLock=state->possibleEnemies.nLock=state->allies.nLock=0;
            state->enemies.bModified=state->possibleEnemies.bModified=state->allies.bModified=false;
            state->selfModified.data=state->selfModified.bModified=false;
            state->bHelpCalled=false; state->bScared=wrapper==owner;
        }
        // Consumer runtime RED was established before adding this new API.
        // Exercise the complete original flag contract, output clearing and world order.
        for (bool allies: {false,true}) for (bool humans: {false,true}) {
            vector<CPtr<NAI::IAIUnit>> queried{contact}, expected;
            for (auto* wrapper: wrappers) {
                auto* server=wrapper->GetUnitServer();
                if (IsValid(savedWorld) && server->CanFight()
                        && ((world->GetDiplomacyState(savedPlayer,server->GetPlayer())!=NDb::DS_ENEMY)==allies)
                        && (humans || wrapper->IsUnderAIControl())) expected.push_back(wrapper);
            }
            shared->GetUnits(&queried,allies,humans);
            Require(queried==expected,"native world query lost output clearing, world order or its diplomacy/human/fightability flags");
        }
        vector<bool> received,membership;
        int expectedMembers=0;
        for (auto* wrapper: wrappers) {
            auto* server=wrapper->GetUnitServer();
            const bool eligible=IsValid(savedWorld) && server->CanFight() && wrapper->IsUnderAIControl()
                && world->GetDiplomacyState(savedPlayer,server->GetPlayer())!=NDb::DS_ENEMY
                && fabs(server->GetPosition().GetCP()-actor->GetPosition().GetCP())<5.f;
            received.push_back(eligible); membership.push_back(eligible && server->GetDiplomacyState(enemy)==NDb::DS_ENEMY);
            expectedMembers+=membership.back();
        }
        GlobalGameRandom().SeedForHarness(47); auto reference=GlobalGameRandom(); const unsigned next=reference.Get();
        CObj<NAI::CTaskCommandAlarm> command=new NAI::CTaskCommandAlarm(enemy); command->SetUnitServer(actor); command->Do();
        int actualMembers=0;
        for (auto* wrapper: wrappers) actualMembers+=int(wrapper->GetAIUnitState()->possibleEnemies.data.units.size());
        std::printf("alarm world mode=%d pass=%d units=%zu cached=%zu memberships=%d/%d savedOwner=%d\n",mode,pass,wrappers.size(),cachedBefore.size(),
            actualMembers,expectedMembers,savedPlayer ? savedPlayer->GetScenarioPlayerID() : -1);
        for (size_t i=0; i<wrappers.size(); ++i) {
            auto* wrapper=wrappers[i]; auto* state=wrapper->GetAIUnitState(); const bool help=wrapper==owner;
            Require(state->possibleEnemies.data.units.size()==size_t(membership[i]) && state->possibleEnemies.data.positions.size()==size_t(membership[i])
                    && state->selfModified.data==(received[i] || help) && state->selfModified.bModified==(received[i] || help),
                "alarm used a cached/own-player roster instead of the current non-enemy world recipients and saved tactical player");
            if (membership[i]) {
                const auto found=state->possibleEnemies.data.positions.find(contact);
                Require(state->possibleEnemies.data.units.front().GetPtr()==contact && found!=state->possibleEnemies.data.positions.end()
                        && found->second==enemy->GetPosition(),"alarm world query delivered another contact or a speculative pose");
            }
            Require(state->possibleEnemies.bModified==membership[i] && !state->enemies.bModified && !state->allies.bModified
                    && state->bHelpCalled==help && !state->bScared && !IsValid(state->pPossibleEnemy)
                    && state->enemies.nLock==0 && state->possibleEnemies.nLock==0 && state->allies.nLock==0,
                "alarm world query lost immediate marks/help eligibility or added reaction locks/selected contacts");
            auto* server=wrapper->GetUnitServer();
            Require(server->GetPosition()==before[i].pose && server->GetAP()==before[i].ap && server->GetUnitRPG()->GetTotalVP()==before[i].hp
                    && server->HasCommand()==before[i].command && wrapper->GetAP()==17+int(i) && wrapper->GetHP()==37,
                "alarm world query synchronized planning skills, spent AP/HP, moved a unit or issued a world command");
        }
        Require(*cached==cachedBefore && commander->GetUnitsList()==roster && shared->pPlayer==savedPlayer
                && shared->GetWorld()==savedWorld && shared->pAICommander==backRef && GlobalGameRandom().Get()==next,
            "alarm world query rebuilt the legacy adapter, changed ownership/backrefs or consumed native RNG");
    }
}

static void AIAlarmNearestTarget(const char* databasePath, const char* resourcePath, int mode, bool supportCount=false)
{
    CObj<NRPG::CGlobalGame> game;
    auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> all; world->GetAllUnits(&all);
    NWorld::CUnitServer *actor=nullptr,*enemy=nullptr,*buddy=nullptr;
    for (const auto& unit: all)
        if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && unit->GetUnitRPG()->CanMove()
                && !unit->HasCommand() && IsValid(NAI::GetAIUnit(unit))) { actor=unit.GetPtr(); break; }
    Require(actor,"alarm target fixture has no mobile native AI caller");
    for (const auto& unit: all) {
        if (!IsValid(unit) || !unit->CanFight() || !IsValid(NAI::GetAIUnit(unit))) continue;
        if (!enemy && actor->GetDiplomacyState(unit)==NDb::DS_ENEMY) enemy=unit.GetPtr();
        if (!buddy && unit.GetPtr()!=actor && unit->GetPlayer()==actor->GetPlayer()) buddy=unit.GetPtr();
    }
    Require(enemy && buddy && !enemy->IsAIUnit(),"alarm target fixture lacks native human/enemy and own AI controls");
    auto* owner=NAI::GetAIUnit(actor); auto* contact=NAI::GetAIUnit(enemy);
    auto* commander=dynamic_cast<NAI::CAICommander*>(actor->GetPlayer()->GetCommander());
    Require(owner && contact && commander,"alarm target fixture lost its native commander/wrappers");
    auto* shared=commander->GetAIState(); shared->Synchronize(); owner->SetAIState(shared);
    auto* network=world->GetPathNetwork();
    vector<NAI::SPathPlace> points; network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),12.f),&points);
    NAI::SPosition nearPoint,farPoint; bool found=false;
    for (auto n: points) {
        n.SetPose(NAI::CM_STAND); n.SetDirection(0); NAI::SPosition np(n,network);
        const float distance=fabs(np.GetCP()-actor->GetPosition().GetCP());
        if (distance<2.f || distance>3.5f || !network->IsPassable(n) || network->IsLocked(n,true)) continue;
        for (auto f: points) {
            f.SetPose(NAI::CM_STAND); f.SetDirection(0); NAI::SPosition fp(f,network);
            if (fabs(fp.GetCP()-np.GetCP())>5.f && fabs(fp.GetCP()-actor->GetPosition().GetCP())>6.f
                    && network->IsPassable(f) && !network->IsLocked(f,true)) {
                nearPoint=np; farPoint=fp; found=true; break;
            }
        }
        if (found) break;
    }
    Require(found,"alarm target fixture lacks disjoint native destination neighborhoods");
    auto place=[&](NWorld::CUnitServer* server,const NAI::SPosition& point) {
        auto pose=server->GetPosition(); pose.pos=point; server->SetPosition(pose);
    };
    const bool foreignCase=mode==2 || mode==4 || mode==5 || mode==7 || mode==10 || mode==11 || mode==12;
    CObj<NAI::CAICommander> foreignCommander; NWorld::CPlayer* foreignPlayer=nullptr; NWorld::CUnitServer* foreign=nullptr;
    const vector<CPtr<NRPG::IUnitMission>> noUnits;
    const int callerSide=actor->GetPlayer()->GetScenarioPlayerID(),enemySide=enemy->GetPlayer()->GetScenarioPlayerID();
    if (foreignCase) {
        game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
        foreignCommander=new NAI::CAICommander(world,nullptr);
        foreignPlayer=dynamic_cast<NWorld::CPlayer*>(world->AddPlayer(L"Native alarm target party",game->players.back(),foreignCommander));
        Require(foreignPlayer,"alarm target fixture failed to create a native foreign party"); foreignCommander->SetPlayer(foreignPlayer);
        world->GetAllUnits(&all);
        for (const auto& unit: all) if (IsValid(unit) && unit->GetPlayer()==foreignPlayer && unit->CanFight()) { foreign=unit.GetPtr(); break; }
        Require(foreign && foreign->IsAIUnit() && IsValid(NAI::GetAIUnit(foreign)),"alarm target foreign recipient lacks native AI control");
        const auto relation=(mode==5 || mode==7) ? NDb::DS_ENEMY : NDb::DS_NEUTRAL;
        world->GetDiplomacy()->SetDiplomacyState(callerSide,noUnits,foreignPlayer->GetScenarioPlayerID(),relation);
    }
    const bool farBuddy=mode==2 || mode==3 || mode==4 || mode==5 || mode==7 || mode==12;
    place(buddy,farBuddy ? farPoint : nearPoint);
    if (foreign) place(foreign,mode==11 ? farPoint : nearPoint);
    if (mode==3 || mode==6) {
        place(enemy,nearPoint);
        world->GetDiplomacy()->SetDiplomacyState(callerSide,noUnits,enemySide,NDb::DS_NEUTRAL);
        Require(actor->GetDiplomacyState(enemy)==NDb::DS_ENEMY,"alarm target human fixture erased individual hostility");
    }
    if (mode==4) { foreign->MakeUnconscious(VNULL3,true,false); Require(!foreign->CanFight(),"alarm target KO control remained fightable"); }
    if (mode==6) {
        world->GetAllUnits(&all);
        for (const auto& unit: all)
            if (IsValid(unit) && unit.GetPtr()!=actor && unit->IsAIUnit() && unit->CanFight()) unit->MakeUnconscious(VNULL3,true,false);
    }
    if (supportCount && (mode==2 || mode==3 || mode==4 || mode==5 || mode==12)) {
        buddy->MakeUnconscious(VNULL3,true,false);
        Require(!buddy->CanFight(),"alarm support fixture retained its own fightable second AI");
    }
    if (mode==11) {
        world->ChangeUnitPlayer(buddy,foreignPlayer);
        Require(buddy->GetPlayer()==foreignPlayer && IsValid(NAI::GetAIUnit(buddy)),"alarm target fixture did not re-register a transferred unit");
    }
    if (mode==7) shared->pPlayer=nullptr;
    if (mode==8) shared->pWorld=nullptr;
    if (mode==9) shared->pAICommander=nullptr;
    world->GetAIMap()->Sync(); actor->SetWishPose(NAI::WALK); owner->Synchronize(false);
    owner->SetLogic(nullptr);
    CObj<NAI::CAIRouteLogic> incumbent=new NAI::CAIRouteLogic(owner); incumbent->AddCommand(new NAI::CTaskCommandWait(3)); incumbent->SetCircled(true);
    owner->SetRouteLogic(incumbent);
    Require(owner->GetLogic()==incumbent.GetPtr(),"alarm target fixture failed to retain its actual native route");
    auto* cache=shared->GetAllyAIPlayer()->GetUnits(); cache->clear(); cache->push_back(owner);
    if (mode==0 || mode==9 || mode==11) cache->clear();
    else if (mode==3 || mode==6) cache->push_back(contact);
    else if (mode==4 || mode==5) cache->push_back(NAI::GetAIUnit(foreign));
    else if (mode==10) { cache->push_back(NAI::GetAIUnit(foreign)); cache->push_back(NAI::GetAIUnit(buddy)); }
    else { cache->push_back(NAI::GetAIUnit(buddy)); if (mode==12) cache->push_back(NAI::GetAIUnit(foreign)); }
    if (supportCount && (mode==2 || mode==7)) { cache->clear(); cache->push_back(owner); }
    if (supportCount && mode==12) { cache->clear(); cache->push_back(owner); cache->push_back(NAI::GetAIUnit(foreign)); }
    const auto staleCache=*cache; const auto ownedRoster=commander->GetUnitsList();
    world->GetAllUnits(&all);
    struct Snapshot { NAI::IAIUnit* wrapper; NAI::SUnitPosition pose; int ap,hp,planAP,planHP; bool command; NAI::SAIState* state; };
    vector<Snapshot> before;
    for (const auto& unit: all) {
        auto* wrapper=NAI::GetAIUnit(unit); if (!IsValid(unit) || !IsValid(wrapper)) continue;
        wrapper->SetHP(37,99); // KO control deliberately retains positive planning HP.
        auto projected=unit->GetPosition().pos; projected.p.SetDirection((projected.p.GetDirection()+3)&7); wrapper->SetPosition(projected);
        before.push_back({wrapper,unit->GetPosition(),unit->GetAP(),unit->GetUnitRPG()->GetTotalVP(),wrapper->GetAP(),wrapper->GetHP(),unit->HasCommand(),wrapper->GetAIState()});
    }
    struct Plan { NAI::SPosition go; NAI::SPathPlace look; int pose=-1; STime wait=0; bool strafe=true; };
    auto inspect=[&](NAI::IAILogic* logic) {
        auto* route=dynamic_cast<NAI::CAIRouteLogic*>(logic); Require(route && !route->IsCircled(),"alarm target factory changed the native route type/circular flag");
        CStructureNetworkContext refs; auto fields=BorrowedObjectFields(route,&refs); vector<CObj<NAI::CTaskCommand>> commands;
        { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(4,&commands); }
        Require(commands.size()==5 && dynamic_cast<NAI::CTaskCommandChangePose*>(commands[0].GetPtr())
                && dynamic_cast<NAI::CTaskCommandGoto*>(commands[1].GetPtr()) && dynamic_cast<NAI::CTaskCommandLookToPosition*>(commands[2].GetPtr())
                && dynamic_cast<NAI::CTaskCommandWait*>(commands[3].GetPtr()) && dynamic_cast<NAI::CTaskCommandAlarm*>(commands[4].GetPtr()),
            "alarm target factory changed its actual five-command sequence");
        Plan result;
        for (const auto& command: commands) Require(command->pUnitServer.GetPtr()==actor && command->Commands.empty(),"alarm target factory rewired or executed a native route step");
        auto poseFields=BorrowedObjectFields(commands[0],&refs);
        { CStructureSaver reader(poseFields,CStructureSaver::READ,&refs); reader.Add(3,&result.pose); }
        auto goFields=BorrowedObjectFields(commands[1],&refs);
        { CStructureSaver reader(goFields,CStructureSaver::READ,&refs); reader.Add(3,&result.go); reader.Add(4,&result.strafe); }
        auto lookFields=BorrowedObjectFields(commands[2],&refs);
        { CStructureSaver reader(lookFields,CStructureSaver::READ,&refs); reader.Add(3,&result.look); }
        auto waitFields=BorrowedObjectFields(commands[3],&refs);
        { CStructureSaver reader(waitFields,CStructureSaver::READ,&refs); reader.Add(3,&result.wait); }
        auto alarmFields=BorrowedObjectFields(commands[4],&refs); CPtr<NWorld::CUnitServer> target;
        { CStructureSaver reader(alarmFields,CStructureSaver::READ,&refs); reader.Add(3,&target); }
        Require(result.pose==NAI::RUN && !result.strafe && result.wait==STime(1) && target.GetPtr()==enemy && result.look==result.go.p,
            "alarm target selection changed pose, strafe, wait duration, look point or the actual enemy");
        return result;
    };
    if (supportCount) {
        struct Outcome { bool alarm=false; unsigned next=0; vector<string> types; vector<NAI::SPathPlace> points; string reaction; };
        auto* state=owner->GetAIUnitState(); Require(state,"alarm support fixture lacks native unit threat state");
        const int passes=mode==12 ? 2 : 1;
        for (int pass=0; pass<passes; ++pass) {
            if (pass) world->GetDiplomacy()->SetDiplomacyState(callerSide,noUnits,foreignPlayer->GetScenarioPlayerID(),NDb::DS_ENEMY);
            vector<CPtr<NAI::IAIUnit>> eligible; shared->GetUnits(&eligible,true,false);
            const int expectedCount=mode==8 ? 0 : (mode==3 || mode==4 || mode==5 || (mode==12 && pass)) ? 1 : (mode==7 || mode==11) ? 3 : 2;
            Require(int(eligible.size())==expectedCount,"alarm support fixture did not straddle its intended native support threshold");
            const bool shouldAlarm=expectedCount>1 && mode!=13;
            unsigned seed=0,expectedNext=0; CObj<NAI::IAILogic> referencePlan; Plan expected;
            for (seed=1; seed!=513; ++seed) {
                GlobalGameRandom().SeedForHarness(seed); auto untouched=GlobalGameRandom();
                const unsigned roll=GlobalGameRandom().Get(100);
                if (mode==13 ? roll<40 : roll>=40) continue;
                if (shouldAlarm) {
                    referencePlan=NAI::CreateAIAlarmLogic(owner,contact);
                    if (!IsValid(referencePlan)) continue;
                    expected=inspect(referencePlan); expectedNext=GlobalGameRandom().Get();
                } else {
                    // A declined alarm continues through the original fear wave.
                    // With <=1 supporter Normal does not draw the alarm roll.
                    if(mode!=13)GlobalGameRandom().SeedForHarness(seed);
                    const vector<CPtr<NAI::IAIUnit>> contacts{contact};NAI::SPathPlace fallback;
                    NAI::GetFearPosition(actor,contacts,&fallback,5.f,30);
                    expectedNext=GlobalGameRandom().Get();
                }
                break;
            }
            Require(seed!=513 && (shouldAlarm==IsValid(referencePlan)),"alarm support fixture could not realize its native seeded branch");
            auto run=[&](const vector<CPtr<NAI::IAIUnit>>& roster) {
                *cache=roster; owner->SetReaction(nullptr); owner->SetLogic(nullptr); owner->SetRouteLogic(incumbent);
                *state=NAI::SAIUnitState(); state->SetUnit(owner); state->AddEnemy(contact); state->pEnemy=contact; state->bScared=true;
                state->enemies.bModified=state->possibleEnemies.bModified=state->allies.bModified=false;
                CObj<NAI::CAIReaction> reaction=NAI::CreateAINormalReaction(owner); owner->SetReaction(reaction);
                Require(owner->GetReactionForUpdate()==reaction.GetPtr(),"alarm support fixture could not consume native reaction installation");
                GlobalGameRandom().SeedForHarness(seed); NAI::SAIUpdateTracker tracker; tracker.Add(reaction); tracker.Update();
                Outcome result; result.next=GlobalGameRandom().Get();
                Require(tracker.CanGetCommand() && IsValid(owner->GetReaction()),"alarm support native pump failed to finish its reaction");
                result.reaction=typeid(*owner->GetReaction()).name();
                auto* route=dynamic_cast<NAI::CAIRouteLogic*>(owner->GetLogic());
                if (route) {
                    CStructureNetworkContext refs; auto fields=BorrowedObjectFields(route,&refs); vector<CObj<NAI::CTaskCommand>> commands;
                    { CStructureSaver reader(fields,CStructureSaver::READ,&refs); reader.Add(4,&commands); }
                    for (const auto& command: commands) {
                        result.types.push_back(typeid(*command).name());
                        if (dynamic_cast<NAI::CTaskCommandAlarm*>(command.GetPtr())) result.alarm=true;
                        if (dynamic_cast<NAI::CTaskCommandGoto*>(command.GetPtr())) {
                            auto saved=BorrowedObjectFields(command,&refs); NAI::SPosition point;
                            { CStructureSaver reader(saved,CStructureSaver::READ,&refs); reader.Add(3,&point); }
                            result.points.push_back(point.p);
                        } else if (dynamic_cast<NAI::CTaskCommandLookToPosition*>(command.GetPtr())) {
                            auto saved=BorrowedObjectFields(command,&refs); NAI::SPathPlace point;
                            { CStructureSaver reader(saved,CStructureSaver::READ,&refs); reader.Add(3,&point); }
                            result.points.push_back(point);
                        }
                    }
                }
                if (result.alarm) {
                    const auto actual=inspect(route);
                    Require(actual.go==expected.go && actual.look==expected.look,
                        "native support branch changed the separately verified alarm destination");
                    Require(owner->GetReaction()==reaction.GetPtr() && !IsValid(state->pEnemy) && state->enemies.data.units.empty()
                            && state->enemies.data.positions.empty() && !state->enemies.bModified && !state->IsModified(),
                        "native alarm branch failed to consume its contact through the existing locked event path");
                } else Require(state->pEnemy.GetPtr()==contact && state->enemies.data.IsContain(contact,false),
                    "native fallback unexpectedly dropped its live enemy contact");
                return result;
            };
            const auto reference=run(eligible);
            Require(reference.alarm==shouldAlarm && reference.next==expectedNext,
                "native support reference changed the original threshold/40-percent roll or fallback RNG");
            const auto actual=run(staleCache);
            std::printf("alarm support mode=%d pass=%d world=%zu cached=%zu alarm=%d/%d seed=%u rng=%u/%u\n",
                mode,pass,eligible.size(),staleCache.size(),int(actual.alarm),int(shouldAlarm),seed,actual.next,expectedNext);
            Require(actual.alarm==reference.alarm && actual.next==reference.next && actual.types==reference.types
                    && actual.points==reference.points && actual.reaction==reference.reaction,
                "normal reaction counted stale/human/unfightable support instead of the current non-enemy AI world set");
            Require(*cache==staleCache && commander->GetUnitsList()==ownedRoster,
                "support counting changed cached or owned commander rosters");
            for (const auto& saved: before) {
                auto* server=saved.wrapper->GetUnitServer();
                Require(server->GetPosition()==saved.pose && server->GetAP()==saved.ap && server->GetUnitRPG()->GetTotalVP()==saved.hp
                        && server->HasCommand()==saved.command && saved.wrapper->GetAP()==saved.planAP && saved.wrapper->GetHP()==saved.planHP
                        && saved.wrapper->GetAIState()==saved.state,
                    "support counting changed native/planning AP/HP, world positions/commands or tactical ownership");
            }
        }
        return;
    }
    const int passes=mode==12 ? 2 : 1;
    for (int pass=0; pass<passes; ++pass) {
        if (pass) world->GetDiplomacy()->SetDiplomacyState(callerSide,noUnits,foreignPlayer->GetScenarioPlayerID(),NDb::DS_ENEMY);
        vector<CPtr<NAI::IAIUnit>> eligible; shared->GetUnits(&eligible,true,false);
        eligible.erase(std::remove(eligible.begin(),eligible.end(),owner),eligible.end());
        const CVec3 origin=actor->GetPosition().GetCP();
        auto nearest=std::min_element(eligible.begin(),eligible.end(),[&](const auto& a,const auto& b) {
            return fabs2(a->GetPosition().GetCP()-origin)<fabs2(b->GetPosition().GetCP()-origin);
        });
        auto* desired=nearest==eligible.end() ? nullptr : nearest->GetPtr();
        if (mode==2 || mode==7 || (mode==12 && pass==0))
            Require(desired==NAI::GetAIUnit(foreign),"alarm fixture did not make its foreign world recipient genuinely closest");
        if ((mode==12 && pass==1) || mode==11)
            Require(desired==NAI::GetAIUnit(buddy),"alarm fixture failed to select its current owned/transferred recipient");
        unsigned seed=47,expectedNext=0; CObj<NAI::IAILogic> referencePlan; Plan expected;
        cache->clear(); if (desired) cache->push_back(desired);
        if (desired) {
            vector<NAI::SPathPlace> neighborhood; network->GetNearPlaces(SSphere(desired->GetPosition().GetCP(),2.f),&neighborhood);
            for (seed=1; seed!=513; ++seed) {
                GlobalGameRandom().SeedForHarness(seed); auto reference=GlobalGameRandom(); reference.Get(int(neighborhood.size())); expectedNext=reference.Get();
                referencePlan=NAI::CreateAIAlarmLogic(owner,contact);
                const unsigned actualNext=GlobalGameRandom().Get();
                if (!IsValid(referencePlan)) continue;
                Require(actualNext==expectedNext,"native alarm factory consumed more than its original one destination draw");
                expected=inspect(referencePlan);
                Require(std::find(neighborhood.begin(),neighborhood.end(),expected.go.p)!=neighborhood.end(),
                    "alarm reference route was not beside the independently selected native world recipient");
                break;
            }
            Require(seed!=513 && IsValid(referencePlan),"alarm target fixture found no reachable actual reference route");
        } else {
            GlobalGameRandom().SeedForHarness(seed); auto reference=GlobalGameRandom(); expectedNext=reference.Get();
            referencePlan=NAI::CreateAIAlarmLogic(owner,contact);
            Require(!IsValid(referencePlan) && GlobalGameRandom().Get()==expectedNext,"empty eligible-world reference unexpectedly planned or consumed RNG");
        }
        *cache=staleCache; GlobalGameRandom().SeedForHarness(seed);
        CObj<NAI::IAILogic> actual=NAI::CreateAIAlarmLogic(owner,contact); const unsigned next=GlobalGameRandom().Get();
        std::printf("alarm target mode=%d pass=%d candidates=%zu cached=%zu selected=%d actual=%d seed=%u rng=%u/%u\n",mode,pass,eligible.size(),staleCache.size(),
            int(desired!=nullptr),int(IsValid(actual)),seed,next,expectedNext);
        Require(IsValid(actual)==bool(desired) && next==expectedNext,
            "alarm destination used the cached garrison or an ineligible recipient instead of the nearest current AI ally");
        if (desired) {
            const auto result=inspect(actual);
            Require(result.go==expected.go && result.look==expected.look,
                "alarm route approached the cached/human/unfightable target instead of the nearest eligible world recipient");
        }
        // After factory runtime RED, also check the complete new query's contract.
        auto untouched=GlobalGameRandom();
        Require(shared->GetNearestUnit(origin,true,false,owner)==desired,
            "native nearest query disagreed with the actual current world recipient");
        if (IsValid(shared->GetWorld())) {
            Require(shared->GetNearestUnit(origin,true,false,nullptr)==owner,
                "nearest query always excluded the caller instead of honoring the explicit exclusion argument");
        }
        if (mode==3) {
            Require(shared->GetNearestUnit(enemy->GetPosition().GetCP(),true,true,owner)==contact
                    && shared->GetNearestUnit(enemy->GetPosition().GetCP(),true,false,owner)==NAI::GetAIUnit(buddy),
                "nearest query lost the human inclusion flag or used individual hostility for global eligibility");
        }
        if (mode==5) {
            Require(shared->GetNearestUnit(foreign->GetPosition().GetCP(),false,false,owner)==NAI::GetAIUnit(foreign)
                    && shared->GetNearestUnit(foreign->GetPosition().GetCP(),false,true,NAI::GetAIUnit(foreign))==contact,
                "nearest enemy query lost its diplomacy flag or arbitrary exclusion pointer");
        }
        if (mode==10) {
            Require(shared->GetNearestUnit(nearPoint.GetCP(),true,false,owner)==NAI::GetAIUnit(buddy)
                    && shared->GetNearestUnit(nearPoint.GetCP(),true,false,NAI::GetAIUnit(buddy))==NAI::GetAIUnit(foreign),
                "nearest exact zero-distance tie lost world order or exclusion handling");
        }
        if (mode==1) {
            Require(shared->GetNearestUnit(origin+CVec3(500,0,0),true,false,owner)==NAI::GetAIUnit(buddy)
                    && shared->GetNearestUnit(origin+CVec3(100000,0,0),true,false,owner)==nullptr,
                "nearest query reused the cached squared-distance cap instead of the native linear65535 limit");
        }
        Require(GlobalGameRandom().Get()==untouched.Get(),"nearest world query consumed native RNG");
        Require(*cache==staleCache && commander->GetUnitsList()==ownedRoster && owner->GetLogic()==incumbent.GetPtr() && incumbent->IsCircled(),
            "alarm nearest query changed the cached/owned rosters or installed its speculative route");
        for (const auto& saved: before) {
            auto* server=saved.wrapper->GetUnitServer();
            Require(server->GetPosition()==saved.pose && server->GetAP()==saved.ap && server->GetUnitRPG()->GetTotalVP()==saved.hp
                    && server->HasCommand()==saved.command && saved.wrapper->GetAP()==saved.planAP && saved.wrapper->GetHP()==saved.planHP
                    && saved.wrapper->GetAIState()==saved.state,
                "alarm target search changed native/planning AP/HP, world positions/commands or tactical ownership");
        }
    }
}

static void AIFearRouteRetention(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer *actor = nullptr, *enemy = nullptr;
    NAI::IAIUnit* owner = nullptr;
    for (const auto& unit: units) {
        auto* wrapper = NAI::GetAIUnit(unit);
        if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && !unit->HasCommand()
                && unit->GetUnitRPG()->CanMove() && IsValid(wrapper)) {
            actor = unit.GetPtr(); owner = wrapper; break;
        }
    }
    Require(actor && owner, "fear route fixture has no mobile idle native AI actor");
    for (const auto& unit: units)
        if (IsValid(unit) && unit->CanFight() && actor->GetDiplomacyState(unit) == NDb::DS_ENEMY
                && IsValid(NAI::GetAIUnit(unit))) { enemy = unit.GetPtr(); break; }
    Require(enemy != nullptr, "fear route fixture has no native hostile contact");
    owner->SetLogic(nullptr); owner->SetRouteLogic(nullptr); owner->Synchronize();
    world->GetAIMap()->Sync();
    auto* state = owner->GetAIUnitState();
    Require(state != nullptr, "fear route fixture has no native threat state");
    state->enemies.data.units.clear(); state->enemies.data.positions.clear();
    state->possibleEnemies.data.units.clear(); state->possibleEnemies.data.positions.clear();
    state->allies.data.units.clear(); state->allies.data.positions.clear();
    state->pEnemy = state->pPossibleEnemy = state->pAlly = nullptr; state->bScared = false;
    if (mode == 1) { state->AddPossibleEnemy(NAI::GetAIUnit(enemy)); state->pPossibleEnemy = NAI::GetAIUnit(enemy); }
    if (mode == 2) { state->AddEnemy(NAI::GetAIUnit(enemy)); state->pEnemy = NAI::GetAIUnit(enemy); }
    state->enemies.bModified = state->possibleEnemies.bModified = state->allies.bModified = false;
    CObj<NAI::CTaskCommandWait> wait = new NAI::CTaskCommandWait(3);
    CObj<NAI::CAIRouteLogic> normal = NAI::CreateAIRouteLogic(owner, vector<CPtr<NAI::CTaskCommand>>{wait.GetPtr()}, true);
    CObj<NAI::CAIRouteLogic> sequence = new NAI::CAIRouteLogic(owner);
    Require(IsValid(normal) && normal->IsActive(), "fear route fixture could not build its actual normal route");
    if (mode != 3) owner->SetRouteLogic(normal);
    const int priorPauses = mode == 1 ? 2 : 0;
    for (int i = 0; i != priorPauses; ++i) normal->Pause();
    // Keep the fork's independent sequence slot populated while in normal mode.
    // Fear must not remove or repurpose either saved route identity.
    CStructureNetworkContext refs; auto initial = BorrowedObjectFields(owner, &refs);
    const auto id = refs.nextID++; refs.ids[sequence.GetPtr()] = id; refs.refs[id] = sequence.GetPtr();
    vector<CObj<NAI::IAILogic>> slots = {mode == 3 ? nullptr : normal.GetPtr(), sequence.GetPtr()};
    CMemoryStream saved;
    { CStructureSaver writer(saved, CStructureSaver::WRITE, &refs); writer.Add(7, &slots); }
    saved.Seek(0);
    { CStructureSaver reader(saved, CStructureSaver::READ, &refs); static_cast<CObjectBase*>(owner)->operator&(reader); }
    auto readSlots = [&]() {
        CStructureNetworkContext references; auto fields = BorrowedObjectFields(owner, &references);
        vector<CObj<NAI::IAILogic>> result;
        { CStructureSaver reader(fields, CStructureSaver::READ, &references); reader.Add(7, &result); }
        return result;
    };
    const auto before = readSlots();
    Require(before.size() == 2 && before[0].GetPtr() == (mode == 3 ? nullptr : normal.GetPtr())
            && before[1].GetPtr() == sequence.GetPtr() && owner->GetRouteLogic() == before[0].GetPtr(),
        "fear route fixture did not retain its two serialized route slots in normal mode");
    CObj<NAI::CAIReaction> reaction = NAI::CreateAIFearReaction(owner, false, false, 0);
    owner->SetReaction(reaction);
    Require(owner->GetReactionForUpdate() == reaction.GetPtr(), "fear route fixture did not consume reaction installation");
    const int ap = actor->GetAP(), hp = actor->GetUnitRPG()->GetTotalVP(); const auto position = actor->GetPosition();
    GlobalGameRandom().SeedForHarness(2);
    NAI::SAIUpdateTracker tracker; tracker.Add(reaction); tracker.Update();
    Require(tracker.CanGetCommand() && owner->GetReaction() == reaction.GetPtr() && IsValid(owner->GetLogic())
            && owner->GetLogic() != normal.GetPtr(), "fear route fixture failed to install its separate native fear logic");
    const auto after = readSlots();
    std::printf("fear route mode=%d normal=%d sequence=%d pauses=%d expected=%d fearLogic=%d\n", mode,
        int(after.size() == 2 && after[0].GetPtr() == normal.GetPtr()),
        int(after.size() == 2 && after[1].GetPtr() == sequence.GetPtr()), normal->nPause,
        priorPauses + (mode == 3 ? 0 : 1), int(IsValid(owner->GetLogic())));
    Require(after.size() == 2 && after[1].GetPtr() == sequence.GetPtr()
            && after[0].GetPtr() == (mode == 3 ? nullptr : normal.GetPtr()),
        "fear reaction deleted or replaced a saved route instead of retaining it");
    Require(normal->nPause == priorPauses + (mode == 3 ? 0 : 1),
        "fear logic did not add exactly one suspension to the retained normal route");
    owner->SetLogic(nullptr);
    const auto resumed = readSlots();
    Require(owner->GetLogic() == before[0].GetPtr() && resumed.size() == 2 && resumed[0].GetPtr() == before[0].GetPtr()
            && resumed[1].GetPtr() == sequence.GetPtr() && normal->nPause == priorPauses
            && normal->IsActive() == (priorPauses == 0),
        "finishing fear logic failed to resume the original route or corrupted an inherited pause");
    CStructureNetworkContext routeRefs; auto routeFields = BorrowedObjectFields(normal, &routeRefs);
    vector<CObj<NAI::CTaskCommand>> commands;
    { CStructureSaver reader(routeFields, CStructureSaver::READ, &routeRefs); reader.Add(4, &commands); }
    Require(commands.size() == 1 && commands.front().GetPtr() == wait.GetPtr() && normal->IsCircled(),
        "fear discarded the original patrol's actual command or looping state");
    const int expectedAP = mode == 1 ? actor->GetUnitRPG()->GetActionAP(owner->GetUnitPosition().GetPose(), NRPG::AC_ROTATE) : ap;
    Require(actor->GetAP() == expectedAP && actor->GetUnitRPG()->GetTotalVP() == hp && actor->GetPosition() == position
            && !actor->HasCommand(), "fear route suspension violated its original AP rule, changed HP/position or issued a gameplay order");
}

static void AIFearAPReservation(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer *actor = nullptr, *enemy = nullptr;
    NAI::IAIUnit* owner = nullptr;
    for (const auto& unit: units) {
        auto* wrapper = NAI::GetAIUnit(unit);
        if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && !unit->HasCommand()
                && unit->GetUnitRPG()->CanMove() && IsValid(wrapper)) {
            actor = unit.GetPtr(); owner = wrapper; break;
        }
    }
    Require(actor && owner, "fear AP fixture has no mobile idle native AI actor");
    for (const auto& unit: units)
        if (IsValid(unit) && unit->CanFight() && actor->GetDiplomacyState(unit) == NDb::DS_ENEMY
                && IsValid(NAI::GetAIUnit(unit))) { enemy = unit.GetPtr(); break; }
    Require(enemy != nullptr, "fear AP fixture has no native hostile contact");
    auto* mission = actor->GetUnitRPG(); auto* rpg = mission->GetRPGUnit();
    rpg->SetCheat(NRPG::CHEAT_AP, false); rpg->SetCheat(NRPG::CHEAT_SCRIPTSEQUENCE, false);
    const NAI::EPose pose = mode == 1 ? NAI::CROUCH : mode == 2 ? NAI::CRAWL : mode == 12 ? NAI::RUN : NAI::WALK;
    auto placed = actor->GetPosition(); placed.SetPose(pose); actor->SetPosition(placed);
    owner->SetLogic(nullptr); owner->SetRouteLogic(nullptr); owner->Synchronize();
    world->GetAIMap()->Sync();
    const bool perkCase = mode == 10 || mode == 11;
    constexpr int rotatePerk = 4; // The authored cheap-rotation perk used by the original AC_ROTATE branch.
    Require(!mission->HasPerk(rotatePerk), "default fear AP actor unexpectedly already owns cheap rotation");
    if (perkCase) {
        auto* definition = NDb::GetDBPerk(rotatePerk);
        auto* tree = rpg->GetPerksTree();
        Require(IsValid(definition) && !definition->params.empty() && IsValid(tree), "fear AP fixture lacks its authored rotation perk/tree");
        CObj<NRPG::CPerk> perk = new NRPG::CPerk(definition); perk->Take();
        // Add a genuine authored perk to this owned test actor through the existing
        // tree save fields; do not override HasPerk or the native action-cost provider.
        CStructureNetworkContext refs; auto fields = BorrowedObjectFields(tree, &refs);
        unordered_map<int, CObj<NRPG::CPerk>> perks;
        { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(2, &perks); }
        const auto id = refs.nextID++; refs.ids[perk.GetPtr()] = id; refs.refs[id] = perk.GetPtr();
        perks[rotatePerk] = perk.GetPtr();
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &refs); writer.Add(2, &perks); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ, &refs); static_cast<CObjectBase*>(tree)->operator&(reader); }
        float parameter = -1;
        Require(mission->HasPerk(rotatePerk, &parameter) && parameter == definition->params[0],
            "fear AP fixture did not install the actual authored rotation perk");
    }
    const int cost = mission->GetActionAP(owner->GetUnitPosition().GetPose(), NRPG::AC_ROTATE);
    const int maxAP = rpg->Skills(NDb::ST_AP).GetCurrentMaxValue();
    Require(cost >= 0 && maxAP > cost + 20 && owner->GetUnitPosition().GetPose() == pose,
        "fear AP fixture has no valid native rotation cost/pose/AP capacity");
    auto* authoredRotation = NDb::GetRPGAP(pose == NAI::CRAWL ? 24 : 25);
    Require(IsValid(authoredRotation) && authoredRotation->nAP == (pose == NAI::CRAWL ? 4 : 2),
        "native fixture rotation data differs from the original rotation cost");
    if (!perkCase) Require(cost == authoredRotation->nAP, "native baseline rotation cost changed before the caller fix");
    int initialAP = cost + 7;
    if (mode == 3) initialAP = cost;
    if (mode == 4) initialAP = (std::max)(0, cost-1);
    if (mode == 5 || mode == 11) initialAP = 0;
    Require(mission->GetAP() >= initialAP, "fear AP actor lacks the initial AP control budget");
    mission->SpendAP(mission->GetAP()-initialAP);
    const bool unlimitedAP = mode == 6, scriptAP = mode == 7;
    rpg->SetCheat(NRPG::CHEAT_AP, unlimitedAP); rpg->SetCheat(NRPG::CHEAT_SCRIPTSEQUENCE, scriptAP);
    const int plannerAP = initialAP == 17 ? 16 : 17;
    owner->SetAP(plannerAP, maxAP);
    Require(actor->GetAP() == initialAP && owner->GetAP() == plannerAP && plannerAP != initialAP,
        "fear AP fixture failed to separate native live AP from the planning snapshot");
    auto* state = owner->GetAIUnitState(); Require(state != nullptr, "fear AP fixture has no native threat state");
    state->enemies.data.units.clear(); state->enemies.data.positions.clear();
    state->possibleEnemies.data.units.clear(); state->possibleEnemies.data.positions.clear();
    state->allies.data.units.clear(); state->allies.data.positions.clear();
    state->pEnemy = state->pPossibleEnemy = state->pAlly = nullptr; state->bScared = false;
    const bool suspect = mode != 8 && mode != 9;
    if (suspect) { state->AddPossibleEnemy(NAI::GetAIUnit(enemy)); state->pPossibleEnemy = NAI::GetAIUnit(enemy); }
    if (mode == 9) { state->AddEnemy(NAI::GetAIUnit(enemy)); state->pEnemy = NAI::GetAIUnit(enemy); }
    state->enemies.bModified = state->possibleEnemies.bModified = state->allies.bModified = false;
    CObj<NAI::CAIReaction> fear = NAI::CreateAIFearReaction(owner, false, false, 0);
    owner->SetReaction(fear);
    Require(owner->GetReactionForUpdate() == fear.GetPtr(), "fear AP fixture failed to select its native reaction");
    const auto position = actor->GetPosition(), enemyPosition = enemy->GetPosition();
    const int hp = mission->GetTotalVP(); auto* inventory = mission->GetInventory();
    GlobalGameRandom().SeedForHarness(2); auto reference = GlobalGameRandom(); const unsigned expectedNext = reference.Get();
    NAI::SAIUpdateTracker tracker; tracker.Add(fear);
    Require(fear->CanUpdate(), "fear AP fixture was excluded by native eligibility at its controlled AP");
    tracker.Update(); const unsigned next = GlobalGameRandom().Get();
    Require(tracker.CanGetCommand() && owner->GetReaction() == fear.GetPtr() && IsValid(owner->GetLogic()),
        "fear AP fixture did not run its native reaction and install a genuine plan");
    const int expected = suspect && !unlimitedAP && !scriptAP ? (std::min)(cost, maxAP) : initialAP;
    std::printf("fear AP mode=%d pose=%d perk=%d liveBefore=%d planAP=%d rotate=%d liveAfter=%d expected=%d rng=%u\n",
        mode, int(pose), int(perkCase), initialAP, plannerAP, cost, actor->GetAP(), expected, next);
    Require(actor->GetAP() == expected && int(rpg->Skills(NDb::ST_AP)) == expected && owner->GetAP() == plannerAP,
        "fear glance did not reserve the original native rotation cost while preserving planning AP");
    if (suspect) {
        Require(!IsValid(state->pPossibleEnemy) && state->possibleEnemies.data.units.empty() && state->possibleEnemies.data.positions.empty()
                && !state->possibleEnemies.bModified && next == expectedNext,
            "fear AP reservation lost locked contact consumption or changed the glance RNG sequence");
        auto* route = dynamic_cast<NAI::CAIRouteLogic*>(owner->GetLogic());
        Require(route != nullptr, "fear AP reservation replaced the glance route type");
        CStructureNetworkContext refs; auto fields = BorrowedObjectFields(route, &refs);
        vector<CObj<NAI::CTaskCommand>> commands;
        { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(4, &commands); }
        Require(commands.size() == 2 && dynamic_cast<NAI::CTaskCommandLookToPosition*>(commands.front().GetPtr())
                && dynamic_cast<NAI::CTaskCommandWait*>(commands.back().GetPtr()), "fear AP reservation changed its look/wait command sequence");
    }
    Require(mission->GetTotalVP() == hp && actor->GetPosition() == position && enemy->GetPosition() == enemyPosition
            && mission->GetInventory() == inventory && !actor->HasCommand()
            && rpg->IsCheatEnabled(NRPG::CHEAT_AP) == unlimitedAP
            && rpg->IsCheatEnabled(NRPG::CHEAT_SCRIPTSEQUENCE) == scriptAP,
        "fear AP reservation changed HP, positions, inventory, cheat policy or issued a world command");
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

class AISyncLogicProbe: public NAI::CAILogic
{
    OBJECT_NOCOPY_METHODS(AISyncLogicProbe);
public:
    int stops = 0;
    AISyncLogicProbe() = default;
    explicit AISyncLogicProbe(NAI::IAIUnit* owner): CAILogic(owner) {}
    void StopThinking() override { ++stops; CAILogic::StopThinking(); }
};

static void AICombatSnapshot(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units;
    world->GetAllUnits(&units);
    vector<NAI::IAIUnit*> ai;
    NAI::IAIUnit* human = nullptr;
    for (const auto& server: units) {
        auto* wrapper = NAI::GetAIUnit(server);
        if (!IsValid(server) || !server->CanFight() || !IsValid(wrapper)) continue;
        if (server->IsAIUnit()) ai.push_back(wrapper); else human = wrapper;
    }
    Require(!ai.empty() && human, "combat snapshot fixture needs native AI and human players");
    while (ai.size() < 4) {
        auto* anchor = ai.front()->GetUnitServer();
        auto* added = world->AddUnitInGame(anchor->GetPosition().pos.p,
            NRPG::CreateUnit(anchor->GetUnitRPG()->GetRPGUnit()->GetPers()), anchor->GetTBSPlayer());
        auto* wrapper = NAI::GetAIUnit(added);
        Require(IsValid(added) && added->CanFight() && added->IsAIUnit() && IsValid(wrapper),
            "combat snapshot fixture failed to deploy its additional native AI actor");
        ai.push_back(wrapper);
    }
    auto* owner = mode == 5 ? human : ai[0];
    auto* server = owner->GetUnitServer();
    auto readFields = [&](vector<CObj<NAI::IAILogic>>* routes, CObj<NAI::IAILogic>* logic,
                          NAI::SUnitPosition* pose, CObj<NAI::CAIInventory>* inventory) {
        CStructureNetworkContext references;
        auto saved = BorrowedObjectFields(owner, &references);
        CStructureSaver reader(saved, CStructureSaver::READ, &references);
        reader.Add(4, pose); reader.Add(6, inventory); reader.Add(7, routes); reader.Add(8, logic);
    };
    auto plannedPose = [&]() {
        vector<CObj<NAI::IAILogic>> routes; CObj<NAI::IAILogic> logic;
        NAI::SUnitPosition pose; CObj<NAI::CAIInventory> inventory;
        readFields(&routes, &logic, &pose, &inventory); return pose;
    };
    if (mode <= 2) {
        if (mode == 1) server->MakeUnconscious(VNULL3, true, false);
        if (mode == 2) { server->GetUnitRPG()->Kill(); server->KillUnit(VNULL3); }
        Require(server->CanFight() == (mode == 0), "snapshot retirement control has wrong native fightability");
        CObj<AISyncLogicProbe> combat = new AISyncLogicProbe(owner);
        CObj<NAI::CAIRouteLogic> normal = new NAI::CAIRouteLogic(owner), sequence = new NAI::CAIRouteLogic(owner);
        vector<CObj<NAI::IAILogic>> seeded = {normal.GetPtr(), sequence.GetPtr()};
        CObj<NAI::IAILogic> current = combat.GetPtr();
        CStructureNetworkContext references;
        auto originalFields = BorrowedObjectFields(owner, &references);
        for (CObjectBase* object: {static_cast<CObjectBase*>(combat.GetPtr()),
                                  static_cast<CObjectBase*>(normal.GetPtr()), static_cast<CObjectBase*>(sequence.GetPtr())}) {
            const auto id = references.nextID++; references.ids[object] = id; references.refs[id] = object;
        }
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &references); writer.Add(7, &seeded); writer.Add(8, &current); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ, &references); static_cast<CObjectBase*>(owner)->operator&(reader); }
        const auto position = server->GetPosition(); const int ap = server->GetAP(), hp = server->GetUnitRPG()->GetTotalVP();
        const int churn = owner->GetNonFreezeCounter();
        owner->Synchronize();
        vector<CObj<NAI::IAILogic>> routes; CObj<NAI::IAILogic> logic;
        NAI::SUnitPosition pose; CObj<NAI::CAIInventory> inventory;
        readFields(&routes, &logic, &pose, &inventory);
        Require(routes.size() == 2, "snapshot retirement changed the serialized route slot count");
        std::printf("snapshot mode=%d logic=%d routes=%d,%d stops=%d\n", mode,
            int(IsValid(logic)), int(IsValid(routes[0])), int(IsValid(routes[1])), combat->stops);
        if (mode == 0)
            Require(logic.GetPtr() == combat.GetPtr() && routes[0].GetPtr() == normal.GetPtr()
                    && routes[1].GetPtr() == sequence.GetPtr() && combat->stops == 0,
                "active snapshot unexpectedly retired its current logic or either route");
        else
            Require(!IsValid(logic) && !IsValid(routes[0]) && !IsValid(routes[1]) && combat->stops == 1,
                "inactive snapshot retained combat logic/routes or failed to stop outgoing thinking");
        Require(server->GetPosition() == position && server->GetAP() == ap && server->GetUnitRPG()->GetTotalVP() == hp,
            "snapshot retirement changed world AP, HP or position");
        Require(owner->GetNonFreezeCounter() == churn, "snapshot retirement counted as a new AI planning change");
    } else {
        const auto position = server->GetPosition(); const int ap = server->GetAP(), hp = server->GetUnitRPG()->GetTotalVP();
        auto plan = position.pos; plan.p.SetDirection((plan.p.GetDirection()+3)&7); owner->SetPosition(plan);
        owner->SetAP(1, 2); owner->SetHP(1, 2);
        owner->SetHurtHP(11); owner->SetMaxToHit(37); owner->SetAdditionalExpediency(29);
        const int turnHP = owner->GetTurnStartHP();
        auto* state = owner->GetAIUnitState();
        state->enemies.data.units = {ai[1], nullptr};
        state->possibleEnemies.data.units = {ai[2]}; state->allies.data.units = {ai[3]};
        // A valid existing contact can itself know other units. Snapshot delivery
        // is one level deep, regardless of cycles or old saved diplomacy rosters.
        ai[1]->GetAIUnitState()->enemies.data.units = {owner, ai[2]};
        for (int i = 1; i != 4; ++i) { ai[i]->SetAP(-10-i, 50); ai[i]->SetHP(0, 50); }
        const int knownAP = ai[1]->GetUnitServer()->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP);
        owner->Synchronize();
        if (mode == 3) {
            std::printf("snapshot legacy counters: turnHP=%d hurt=%d maxHit=%d extra=%d\n", owner->GetTurnStartHP(),
                owner->GetHurtHP(), owner->GetMaxToHit(), owner->GetAdditionalExpediency());
            Require(owner->GetTurnStartHP() == turnHP && owner->GetHurtHP() == 11
                    && owner->GetMaxToHit() == 37 && owner->GetAdditionalExpediency() == 29,
                "ordinary snapshot refresh reset unrelated legacy planning counters");
        } else if (mode == 4)
            Require(ai[1]->GetAP() == knownAP && ai[1]->GetHP() > 0 && ai[2]->GetAP() == -12 && ai[3]->GetAP() == -13,
                "default AI snapshot failed to refresh only direct known contacts without recursive/suspect/ally updates");
        else
            Require(ai[1]->GetAP() == -11 && ai[2]->GetAP() == -12 && ai[3]->GetAP() == -13,
                "human snapshot recursively refreshed tactical contacts despite lacking AI control");
        Require(owner->GetAP() == server->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_AP)
                && owner->GetHP() == server->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP)
                && plannedPose() == position && IsValid(owner->GetAIInventory()),
            "live snapshot failed to refresh native skills, full pose or inventory");
        Require(server->GetPosition() == position && server->GetAP() == ap && server->GetUnitRPG()->GetTotalVP() == hp,
            "snapshot refresh spent world AP/HP or replaced the live position");
    }
}

static void AIPassControlSnapshots(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer *actor = nullptr, *enemy = nullptr;
    const bool human = mode == 1;
    for (const auto& unit: units)
        if (IsValid(unit) && unit->CanFight() && !unit->HasCommand() && unit->IsAIUnit() != human
                && IsValid(NAI::GetAIUnit(unit))) { actor = unit.GetPtr(); break; }
    Require(actor != nullptr, "pass-control fixture has no idle native actor of its requested control type");
    for (const auto& unit: units)
        if (IsValid(unit) && unit->CanFight() && actor->GetDiplomacyState(unit) == NDb::DS_ENEMY
                && IsValid(NAI::GetAIUnit(unit))) { enemy = unit.GetPtr(); break; }
    auto* player = dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer());
    auto* foreign = enemy ? dynamic_cast<NWorld::CPlayer*>(enemy->GetPlayer()) : nullptr;
    auto* commander = player ? dynamic_cast<NAI::CAICommander*>(player->GetCommander()) : nullptr;
    auto* owner = NAI::GetAIUnit(actor); auto* contact = NAI::GetAIUnit(enemy);
    Require(commander && foreign && owner && contact, "pass-control fixture lacks opposing native commanders/wrappers");
    auto* shared = commander->GetAIState();
    shared->pPlayer = player; // Explicit saved tactical player: the human commander may be constructed unbound.
    shared->Synchronize();
    const auto expectedGroups = shared->GetEnemyGroups();
    Require(!expectedGroups.empty(), "pass-control fixture has no native hostile groups to refresh");
    auto* script = world->GetOwnScript();
    Require(IsValid(script) && script->DoString("UpstreamPassCalls=0; UpstreamPassPlayer=-1; function OnStartTurn(player) UpstreamPassCalls=UpstreamPassCalls+1; UpstreamPassPlayer=player end") == 0,
        "pass-control fixture could not define its real Lua turn hook");
    world->ExecuteOwnScript();
    struct ScriptContext { ~ScriptContext() { pScript = nullptr; } } scriptContext;
    owner->SetLogic(nullptr);
    CObj<NAI::CAIRouteLogic> route = new NAI::CAIRouteLogic(owner);
    route->AddCommand(new NAI::CTaskCommandWait(3)); route->SetCircled(true);
    owner->SetRouteLogic(route);
    CObj<NAI::IAILogic> behavior = owner->GetLogic(); // Human normal-route installation is intentionally refused.
    auto planningPosition = actor->GetPosition();
    planningPosition.pos.p.SetDirection((planningPosition.pos.p.GetDirection()+3)&7);
    owner->SetPosition(planningPosition.pos);
    auto savedPose = [&]() {
        CStructureNetworkContext refs; auto fields = BorrowedObjectFields(owner, &refs);
        NAI::SUnitPosition pose;
        { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(4, &pose); }
        return pose;
    };
    planningPosition = savedPose();
    Require(!(planningPosition == actor->GetPosition()) && (human ? behavior.GetPtr() != route.GetPtr() : behavior.GetPtr() == route.GetPtr()),
        "pass-control fixture failed to separate the actual planner pose/route from the live server");
    owner->GetAIUnitState()->AddEnemy(contact); owner->GetAIUnitState()->pEnemy = contact;
    const auto ownedRoster = commander->GetUnitsList();
    vector<NAI::IAIUnit*> snapshots;
    for (const auto& unit: ownedRoster)
        if (IsValid(unit) && IsValid(unit->GetUnitServer()) && unit->GetUnitServer()->CanFight()) snapshots.push_back(unit.GetPtr());
    snapshots.push_back(contact);
    for (size_t i=0; i<snapshots.size(); ++i) {
        snapshots[i]->SetAP(-100-int(i), 300+int(i)); snapshots[i]->SetHP(-200-int(i), 400+int(i));
    }
    shared->pCurrentUnit = contact; shared->pCurrentEnemy = owner;
    shared->nCurrentAction = 73; shared->nTurnStartAllyHP = 311; shared->nTurnStartEnemyHP = 411;
    shared->pAlly->GetUnits()->clear(); shared->pEnemy->GetUnits()->clear(); shared->enemyGroups.clear();
    CObj<NWorld::CCommand> pending = new NWorld::CCmdSetCommand(actor, new NWorld::CCmdContinue);
    auto commanderFields = [&](CStructureNetworkContext* refs) {
        auto traversal = BorrowedObjectFields(commander, refs);
        // The human CSequenceCommander nests this base in tag2. Inspect the
        // actual CAICommander fields explicitly instead of flattening that save.
        CMemoryStream fields;
        { CStructureSaver writer(fields, CStructureSaver::WRITE, refs); commander->NAI::CAICommander::operator&(writer); }
        fields.Seek(0); return fields;
    };
    auto readTracker = [&]() {
        CStructureNetworkContext refs; auto fields = commanderFields(&refs);
        NAI::SAICommandTracker tracker;
        { CStructureSaver reader(fields, CStructureSaver::READ, &refs); reader.Add(9, &tracker); }
        return tracker;
    };
    auto seedTracker = [&]() {
        CStructureNetworkContext refs; auto fields = commanderFields(&refs);
        const auto id = refs.nextID++; refs.ids[pending.GetPtr()] = id; refs.refs[id] = pending.GetPtr();
        NAI::SAICommandTracker tracker; tracker.commands.push_back(pending); tracker.bCommandGiven = true; tracker.nNonFreezeCounter = 11;
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &refs); writer.Add(9, &tracker); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ, &refs); commander->NAI::CAICommander::operator&(reader); }
        Require(readTracker().commands.size() == 1, "pass-control fixture did not seed its native outgoing command queue");
    };
    const auto position = actor->GetPosition(), enemyPosition = enemy->GetPosition();
    const int ap = actor->GetAP(), enemyAP = enemy->GetAP(), hp = actor->GetUnitRPG()->GetTotalVP();
    auto* inventory = actor->GetUnitRPG()->GetInventory();
    const bool ownPass = mode != 2; const int repeats = mode == 3 ? 2 : 1;
    GlobalGameRandom().SeedForHarness(37); auto reference = GlobalGameRandom(); const unsigned expectedNext = reference.Get();
    for (int pass=0; pass<repeats; ++pass) {
        seedTracker(); pScript = script;
        commander->OnPassControl(ownPass ? player : foreign);
        const auto tracker = readTracker();
        Require(commander->IsAITurn() == ownPass && tracker.commands.empty()
                && tracker.bCommandGiven && tracker.nNonFreezeCounter == 11,
            "pass-control lost turn ownership, failed to clear queued commands or changed unrelated tracker state");
        std::printf("pass-control mode=%d pass=%d nativeAI=%d ownerAP=%d contactAP=%d groups=%zu\n",
            mode, pass, int(actor->IsAIUnit()), owner->GetAP(), contact->GetAP(), shared->enemyGroups.size());
        for (size_t i=0; i<snapshots.size(); ++i)
            Require(snapshots[i]->GetAP() == -100-int(i) && snapshots[i]->GetMaxAP() == 300+int(i)
                    && snapshots[i]->GetHP() == -200-int(i) && snapshots[i]->GetMaxHP() == 400+int(i),
                "passing control refreshed a unit's speculative AP/HP snapshot before its own combat logic requested synchronization");
        Require(savedPose() == planningPosition && owner->GetLogic() == behavior.GetPtr() && route->IsCircled(),
            "passing control overwrote the speculative pose or replaced the patrol route");
        if (ownPass) {
            Require(shared->enemyGroups.size() == expectedGroups.size() && !shared->pAlly->GetUnits()->empty()
                    && !shared->pEnemy->GetUnits()->empty(), "own pass failed to refresh actual tactical rosters/groups");
            for (size_t i=0; i<expectedGroups.size(); ++i)
                Require(fabs2(shared->enemyGroups[i].ptCenter-expectedGroups[i].ptCenter) < 1e-8f
                        && shared->enemyGroups[i].enemies == expectedGroups[i].enemies
                        && shared->enemyGroups[i].allies == expectedGroups[i].allies,
                    "own pass changed the native group membership or live-position center");
        } else Require(shared->enemyGroups.empty() && shared->pAlly->GetUnits()->empty() && shared->pEnemy->GetUnits()->empty(),
            "foreign pass rebuilt another player's tactical groups");
        world->ExecuteOwnScript();
        Require(script->GetGlobal("UpstreamPassCalls").GetNumber() == (ownPass ? pass+1 : 0)
                && script->GetGlobal("UpstreamPassPlayer").GetNumber() == (ownPass ? player->GetScenarioPlayerID() : -1),
            "pass-control did not deliver exactly one real Lua OnStartTurn hook to the owning scenario player");
    }
    Require(commander->GetUnitsList() == ownedRoster && shared->pCurrentUnit.GetPtr() == contact && shared->pCurrentEnemy.GetPtr() == owner
            && shared->nCurrentAction == 73 && shared->nTurnStartAllyHP == 311 && shared->nTurnStartEnemyHP == 411,
        "pass-control changed owned identities, planning context or start-of-turn HP counters");
    Require(actor->GetPosition() == position && enemy->GetPosition() == enemyPosition && actor->GetAP() == ap && enemy->GetAP() == enemyAP
            && actor->GetUnitRPG()->GetTotalVP() == hp && actor->GetUnitRPG()->GetInventory() == inventory && !actor->HasCommand()
            && GlobalGameRandom().Get() == expectedNext,
        "pass-control changed live AP/HP, position, inventory, commands or the native random sequence");
}

static void PlayerTBSTurnOwnership(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game; auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> all; world->GetAllUnits(&all);
    NWorld::CUnitServer *actor=nullptr,*enemy=nullptr;
    for (const auto& unit: all)
        if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && !unit->HasCommand() && IsValid(NAI::GetAIUnit(unit))) { actor=unit.GetPtr(); break; }
    Require(actor,"player TBS fixture has no idle native AI actor");
    for (const auto& unit: all)
        if (IsValid(unit) && unit->CanFight() && actor->GetDiplomacyState(unit)==NDb::DS_ENEMY) { enemy=unit.GetPtr(); break; }
    auto* player=dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer());
    auto* foreign=enemy ? dynamic_cast<NWorld::CPlayer*>(enemy->GetPlayer()) : nullptr;
    auto* commander=player ? dynamic_cast<NAI::CAICommander*>(player->GetCommander()) : nullptr;
    auto* owner=NAI::GetAIUnit(actor); Require(player && foreign && commander && owner,"player TBS fixture lacks native parties/commander");
    auto* script=world->GetOwnScript();
    Require(IsValid(script) && script->DoString("function OnStartTurn(player) end")==0,"player TBS fixture could not define its native Lua hook");
    world->ExecuteOwnScript(); struct ScriptContext { ~ScriptContext(){pScript=nullptr;} } context;
    commander->GetAIState()->pPlayer=player;
    const bool expectedTurn=mode!=0; commander->OnPassControl(expectedTurn ? player : foreign);
    Require(commander->IsAITurn()==expectedTurn,"player TBS fixture failed to establish actual pass-control ownership");
    CObj<NAI::CAIRouteLogic> logic=new NAI::CAIRouteLogic(owner); logic->AddCommand(new NAI::CTaskCommandWait(3));
    using Memo=unordered_map<CPtr<NAI::IAILogic>,int,SPtrHash>; Memo expected; expected[logic.GetPtr()]=23;
    auto fields=[&](CStructureNetworkContext* refs) {
        auto traversal=BorrowedObjectFields(commander,refs); CMemoryStream saved;
        { CStructureSaver writer(saved,CStructureSaver::WRITE,refs); commander->NAI::CAICommander::operator&(writer); }
        saved.Seek(0); return saved;
    };
    {
        CStructureNetworkContext refs; auto original=fields(&refs);
        auto id=refs.nextID++; refs.ids[logic.GetPtr()]=id; refs.refs[id]=logic.GetPtr();
        CMemoryStream saved; { CStructureSaver writer(saved,CStructureSaver::WRITE,&refs); writer.Add(10,&expected); }
        saved.Seek(0); { CStructureSaver reader(saved,CStructureSaver::READ,&refs); commander->NAI::CAICommander::operator&(reader); }
    }
    auto memo=[&]() {
        CStructureNetworkContext refs; auto saved=fields(&refs); Memo result;
        { CStructureSaver reader(saved,CStructureSaver::READ,&refs); reader.Add(10,&result); } return result;
    };
    Require(memo()==expected,"player TBS fixture failed to restore the actual logic memo");
    const auto roster=commander->GetUnitsList(); auto* shared=commander->GetAIState();
    shared->nTurnStartAllyHP=311; shared->nTurnStartEnemyHP=411;
    if (mode==0 || mode>=4) {
        Require(actor->GetAP()>1,"player TBS new-turn fixture lacks native AP");
        actor->GetUnitRPG()->SpendAP(actor->GetAP()-1);
        Require(actor->GetAP()==1,"player TBS new-turn fixture did not spend actual mission AP");
    }
    const NWorld::ETBSEvent event=(mode==0 || mode>=4) ? NWorld::TBS_START_NEW_TURN : mode==1 ? NWorld::TBS_FINISH_OWN_TURN : mode==2 ? NWorld::TBS_START_REAL_TIME : NWorld::TBS_CANCEL_ACTION;
    const auto position=actor->GetPosition();
    const auto originalUnits=player->GetPlayerUnits();
    auto setUnits=[&](const vector<CMObj<NWorld::CUnitServer>>& units) {
        CStructureNetworkContext refs; auto traversal=BorrowedObjectFields(player,&refs);
        auto restored=units; CMemoryStream saved;
        { CStructureSaver writer(saved,CStructureSaver::WRITE,&refs); writer.Add(5,&restored); }
        saved.Seek(0); { CStructureSaver reader(saved,CStructureSaver::READ,&refs);
            player->NWorld::CPlayerBase<NWorld::CUnitServer,NWorld::CCommander>::operator&(reader); }
    };
    if (mode>=4) {
        auto incomplete=originalUnits; Require(!incomplete.empty(),"player TBS null-reference fixture has no valid native units");
        incomplete.insert(incomplete.begin()+(mode==4 ? 0 : 1),nullptr); setUnits(incomplete);
        Require(player->GetPlayerUnits().size()==originalUnits.size()+1,"player TBS fixture did not restore its incomplete native player roster");
        std::printf("player TBS before native event mode=%d nullIndex=%d roster=%zu\n",mode,mode==4 ? 0 : 1,player->GetPlayerUnits().size()); std::fflush(stdout);
    }
    player->OnTBSEvent(event);
    if (mode>=4) setUnits(originalUnits);
    const auto after=memo();
    std::printf("player TBS mode=%d turn=%d/%d memo=%zu/%zu AP=%d\n",mode,int(commander->IsAITurn()),int(expectedTurn),after.size(),expected.size(),actor->GetAP());
    Require(commander->IsAITurn()==expectedTurn && after==expected && shared->nTurnStartAllyHP==311 && shared->nTurnStartEnemyHP==411,
        "player unit event incorrectly ran commander turn-start/finish work instead of preserving pass-control ownership");
    Require(commander->GetUnitsList()==roster && actor->GetPosition()==position,"player TBS delivery changed commander ownership or moved the native actor");
    if (mode==0 || mode>=4) Require(actor->GetAP()>1,"unit-only new-turn dispatch failed to refresh actual mission AP");
    if (mode==1) Require(player->IsTurnDone(),"unit-only finish dispatch lost the native player's completed-turn latch");
    if (mode==2) Require(!player->IsTurnDone(),"unit-only realtime dispatch failed to reopen the native player's turn latch");
}

static void AIPassControlEndMemo(const char* databasePath, const char* resourcePath, int mode)
{
    CObj<NRPG::CGlobalGame> game;
    auto world = FixtureWorld(databasePath, resourcePath, true, &game);
    list<CPtr<NWorld::CUnitServer>> units; world->GetAllUnits(&units);
    NWorld::CUnitServer *actor = nullptr, *enemy = nullptr;
    for (const auto& unit: units)
        if (IsValid(unit) && unit->CanFight() && unit->IsAIUnit() && !unit->HasCommand()
                && IsValid(NAI::GetAIUnit(unit))) { actor = unit.GetPtr(); break; }
    Require(actor != nullptr, "pass-control memo fixture has no native idle AI actor");
    for (const auto& unit: units)
        if (IsValid(unit) && unit->CanFight() && actor->GetDiplomacyState(unit) == NDb::DS_ENEMY) { enemy = unit.GetPtr(); break; }
    auto* player = dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer());
    auto* foreign = enemy ? dynamic_cast<NWorld::CPlayer*>(enemy->GetPlayer()) : nullptr;
    auto* owner = NAI::GetAIUnit(actor);
    CObj<NAI::CAICommander> restoring;
    if (mode == 2) restoring = new NAI::CAICommander;
    if (mode == 3) restoring = new NAI::CAICommander(world, nullptr);
    auto* commander = mode >= 2 ? restoring.GetPtr() : dynamic_cast<NAI::CAICommander*>(player->GetCommander());
    Require(commander && owner && foreign, "pass-control memo fixture lacks real commanders/opposing player");
    Require((mode >= 2) == !IsValid(commander->GetPlayer()), "pass-control memo fixture has the wrong owner binding");
    CObj<NAI::CAIRouteLogic> first = new NAI::CAIRouteLogic(owner), second = new NAI::CAIRouteLogic(owner);
    first->AddCommand(new NAI::CTaskCommandWait(3)); second->AddCommand(new NAI::CTaskCommandWait(7));
    using Memo = unordered_map<CPtr<NAI::IAILogic>, int, SPtrHash>;
    Memo expected; expected[first.GetPtr()] = 1; expected[second.GetPtr()] = 23;
    CObj<NWorld::CCommand> pending = new NWorld::CCmdSetCommand(actor, new NWorld::CCmdContinue);
    auto fields = [&](CStructureNetworkContext* refs) {
        auto traversal = BorrowedObjectFields(commander, refs);
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, refs); commander->NAI::CAICommander::operator&(writer); }
        saved.Seek(0); return saved;
    };
    {
        CStructureNetworkContext refs; auto original = fields(&refs);
        for (CObjectBase* object: {static_cast<CObjectBase*>(first.GetPtr()), static_cast<CObjectBase*>(second.GetPtr()),
                                  static_cast<CObjectBase*>(pending.GetPtr())}) {
            const auto id = refs.nextID++; refs.ids[object] = id; refs.refs[id] = object;
        }
        NAI::SAICommandTracker tracker; tracker.commands.push_back(pending); tracker.bCommandGiven = true; tracker.nNonFreezeCounter = 19;
        CMemoryStream saved;
        { CStructureSaver writer(saved, CStructureSaver::WRITE, &refs); writer.Add(9, &tracker); writer.Add(10, &expected); }
        saved.Seek(0);
        { CStructureSaver reader(saved, CStructureSaver::READ, &refs); commander->NAI::CAICommander::operator&(reader); }
    }
    auto read = [&](Memo* memo, NAI::SAICommandTracker* tracker) {
        CStructureNetworkContext refs; auto saved = fields(&refs);
        CStructureSaver reader(saved, CStructureSaver::READ, &refs); reader.Add(9, tracker); reader.Add(10, memo);
    };
    Memo before; NAI::SAICommandTracker beforeTracker; read(&before, &beforeTracker);
    Require(before == expected && beforeTracker.commands.size() == 1, "pass-control memo fixture failed to restore its genuine native logic memo/command queue");
    auto* shared = commander->GetAIState(); shared->enemyGroups.clear();
    NAI::SAIUnitGroup marker; marker.ptCenter = CVec3(1234,5678,9012); marker.enemies.push_back(owner);
    shared->enemyGroups.push_back(marker);
    auto* script = world->GetOwnScript();
    Require(IsValid(script) && script->DoString("UpstreamMemoCalls=0; function OnStartTurn(player) UpstreamMemoCalls=UpstreamMemoCalls+1 end") == 0,
        "pass-control memo fixture failed to define the real Lua hook");
    world->ExecuteOwnScript();
    struct ScriptContext { ~ScriptContext() { pScript = nullptr; } } scriptContext;
    const auto position = actor->GetPosition(); const int ap = actor->GetAP(), hp = actor->GetUnitRPG()->GetTotalVP();
    const auto roster = commander->GetUnitsList();
    GlobalGameRandom().SeedForHarness(41); auto reference = GlobalGameRandom(); const unsigned next = reference.Get();
    commander->OnPassControl(mode >= 2 ? nullptr : mode == 1 ? foreign : player);
    Memo after; NAI::SAICommandTracker tracker; read(&after, &tracker);
    const bool ownPlayer = mode == 0;
    std::printf("pass-control memo mode=%d validOwner=%d turn=%d entries=%zu expected=%zu\n", mode,
        int(IsValid(commander->GetPlayer())), int(commander->IsAITurn()), after.size(), ownPlayer ? size_t(0) : expected.size());
    Require(ownPlayer ? after.empty() : after == expected,
        "a pass without a valid owning player erased the saved end-of-turn logic memo");
    Require(tracker.commands.empty() && tracker.bCommandGiven && tracker.nNonFreezeCounter == 19
            && commander->IsAITurn() == (mode != 1), "memo guard skipped native queue clearing or changed the existing turn/tracker state");
    if (mode == 3) Require(shared->enemyGroups.empty(), "unbound world-connected commander skipped the group refresh before its owner guard");
    if (mode == 1) Require(shared->enemyGroups.size() == 1 && fabs2(shared->enemyGroups[0].ptCenter-marker.ptCenter) == 0,
        "foreign pass changed the memo owner's tactical groups");
    world->ExecuteOwnScript();
    Require(script->GetGlobal("UpstreamMemoCalls").GetNumber() == (ownPlayer ? 1 : 0),
        "memo guard suppressed the owning Lua hook or invoked it without a valid owner");
    Require(commander->GetUnitsList() == roster && actor->GetPosition() == position && actor->GetAP() == ap
            && actor->GetUnitRPG()->GetTotalVP() == hp && !actor->HasCommand() && GlobalGameRandom().Get() == next,
        "memo guard changed native roster, world AP/HP, position, commands or RNG");
}

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
