static void LooseRayIncidental(const char* databasePath,const char* resourcePath,int kind)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> all;world->GetAllUnits(&all);NWorld::CUnitServer* shooter=nullptr;
    for(const auto& unit:all) {
        if(!IsValid(unit) || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK() || unit->GetUnitRPG()->HasPerk(0x15))continue;
        auto* weapon=unit->GetUnitRPG()->GetWeaponItem();
        if(weapon && weapon->SetShootMode(NDb::SM_Snap) && NRPG::GetToHitType(unit)==NRPG::TH_SHOOT){shooter=unit.GetPtr();break;}
    }
    Require(shooter,"incidental ray fixture has no native firearm shooter without night vision");
    NWorld::CUnitServer* target=nullptr;NWorld::CUnitServer* intended=nullptr;
    for(const auto& unit:all)if(IsValid(unit) && unit.GetPtr()!=shooter && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK()) {
        if(!target)target=unit.GetPtr();else {intended=unit.GetPtr();break;}
    }
    Require(target && intended,"incidental ray fixture lacks two native non-PK targets");
    auto* original=NDb::GetArmor(NDb::N_DEFAULT_ARMOR);Require(original && original->pMaterial,"incidental ray fixture lacks authored armor material");
    CObj<NDb::CRPGMaterial> material=new NDb::CRPGMaterial;material->nDR=original->pMaterial->nDR;material->fDensity=2;
    CObj<NDb::CRPGArmor> armor=new NDb::CRPGArmor;armor->pMaterial=material;armor->fRicochetMaxCos=0;armor->fRicochetProbability=0;
    CObj<NDb::CRPGArmor> mirror=new NDb::CRPGArmor;mirror->pMaterial=material;mirror->fRicochetMaxCos=2;mirror->fRicochetProbability=1;
    CObj<LooseRayWallToken> token=new LooseRayWallToken;CObj<LooseRayTraceMap> map=new LooseRayTraceMap;
    NRPG::CAttackPortion attack(100000,1,0.f,3,3,0,0,shooter->GetUnitRPG(),intended->GetUnitRPG(),1.f,-1,false);
    NRPG::SAttackRayInfo ray(attack,shooter->GetPosition().GetCP(),CVec3(1,0,0),shooter,shooter->GetPosition(),0,0,false,0,12,intended);
    ray.pIgnore=shooter;
    const int forced=kind==2 || kind==3?0:kind==4 || kind==5?37:100;shooter->SetScriptToHit(forced);
    if(kind==0)ray.pUS=nullptr;
    int desiredRoll=kind==3?0:kind==4?37:kind==5?38:50;
    int chance=kind==0?100:forced,dayChance=-1,nightChance=-1;
    if(kind>=13) {
        shooter->SetScriptToHit(-1);ray.bFirstTurn=true;
        auto nativeChance=[&](bool night) {
            const int distance=int(fabs(ray.vOrigin-target->GetPosition().GetCP())*FP_INV_GRID_STEP);
            CObj<NRPG::CUnitToHitCalcer> calc=new NRPG::CUnitToHitCalcer(shooter,ray.from.GetPose(),distance,ray.vOrigin,target->GetPosition().pos,
                ray.nExtraAP,0,75.f,ray.bFirstTurn,night,CVec3(1,1,1),NAI::HL_BODY,target,ray.nBullet,!target->IsUnitVisible(shooter));
            return calc->GetToHit();
        };
        for(int distance=3;distance<=80;++distance) {
            ray.vOrigin=target->GetPosition().GetCP()-CVec3(float(distance),0,0);
            dayChance=nativeChance(false);nightChance=nativeChance(true);
            if(dayChance>nightChance && nightChance>=0 && dayChance<100)break;
        }
        Require(dayChance>nightChance && nightChance>=0 && dayChance<100,"native incidental night fixture has no observable day/night accuracy boundary");
        chance=kind==14?nightChance:dayChance;desiredRoll=dayChance;
    }
    world->GetGame()->SetNight(kind==14);
    const float x=ray.vOrigin.x;
    CObjectBase* surface=kind==7?static_cast<CObjectBase*>(shooter):kind==10 || kind==11?static_cast<CObjectBase*>(token.GetPtr()):static_cast<CObjectBase*>(target);
    if(kind==6)ray.pTarget=target;
    if(kind==11)ray.pTarget=token.GetPtr();
    NAI::SSourceInfo source(surface,armor,3,NWorld::TS_FRAGMENTED);map->slabs={{x+2,x+3,source,NAI::HL_BODY}};
    if(kind==8)map->slabs.push_back({x+4,x+5,source,NAI::HL_BODY});
    if(kind==9)map->slabs.push_back({x+6,x+6.02f,NAI::SSourceInfo(token,mirror,3,NWorld::TS_FRAGMENTED),77});
    CRandomGenerator reference;unsigned seed=0;
    for(unsigned candidate=1;candidate<100000;++candidate){reference.SeedForHarness(candidate);if(int(reference.Get(100))==desiredRoll){seed=candidate;break;}}
    Require(seed,"incidental ray fixture could not select its exact native roll");reference.SeedForHarness(seed);
    int draws=kind==6 || kind==7 || kind==10 || kind==11?0:1;
    if(draws)reference.Get(100);
    if(kind==9){reference.GetFloat(0,1);reference.Get(100);draws=3;}
    const unsigned expectedNext=reference.Get();GlobalGameRandom().SeedForHarness(seed);
    vector<NRPG::STrailPoint> trail;NRPG::TraceLooseRay(map,ray,&trail);const unsigned actualNext=GlobalGameRandom().Get();
    int records=0,receivers=0;
    for(const auto& point:trail)if(point.pObject.GetPtr()==surface) {
        ++records;if(IsValid(point.pAttackTarget)){Require(point.pAttackTarget.GetPtr()==surface,"incidental ray selected a different damage receiver");++receivers;}
    }
    const bool hit=desiredRoll<=chance;const int expectedReceivers=kind==6 || kind==7 || kind==11?0:kind==10?1:hit?(kind==9?2:1):0;
    const int expectedRecords=kind==7?0:kind==10 || kind==11?2:kind==6 || !hit?0:kind==9?4:2;
    std::printf("native incidental ray: kind=%d chance=%d day=%d night=%d roll=%d records=%d/%d receivers=%d/%d rays=%zu draws=%d next=%u/%u kinetic=%d\n",
        kind,chance,dayChance,nightChance,desiredRoll,records,expectedRecords,receivers,expectedReceivers,map->rays.size(),draws,actualNext,expectedNext,trail.back().sAttack.nK);std::fflush(stdout);
    Require(records==expectedRecords && receivers==expectedReceivers,"native loose ray omitted incidental damage routing or emitted a missed unit's entry/exit");
    Require(actualNext==expectedNext,"native incidental ray used the wrong original ISAAC draw count or inclusive hit boundary");
    Require(map->rays.size()==std::size_t(kind==9?2:1) && trail.back().sAttack.nK==(kind==7?100000:kind==9?99996:99998),
        "incidental hit/miss changed original geometry traversal or carried armor energy");
    if(kind>=13) {
        const int actualChance=NRPG::GetBulletToHit(ray,target,NAI::HL_BODY);
        std::printf("native incidental exact chance: kind=%d actual=%d expected=%d\n",kind,actualChance,chance);std::fflush(stdout);
        Require(actualChance==chance,"incidental helper changed the exact existing native day/night accuracy inputs");
    }
    if(kind==12) {
        const int expectedDamage=attack.CalcStructDmg(world,NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR),0);
        Require(expectedDamage>0,"incidental damage fixture has no positive native damage control");
        const int before=target->GetUnitRPG()->GetTotalVP();for(;;){CObj<NWorld::CHitLocator> hit=world->GetHitEvent();if(!hit)break;}
        GlobalGameRandom().SeedForHarness(2); // Isolate the existing world dodge control after the traced-hit RNG assertion.
        CObj<NWorld::IDynamicObject> bullet=NWorld::CreateBulletServer(world,trail,world->GetTime()->GetValue(),nullptr,100.f);
        Require(bullet && bullet->Segment(),"incidental damage fixture did not process its actual native bullet");bullet=nullptr;
        CObj<NWorld::CHitLocator> event=world->GetHitEvent();
        std::printf("native incidental world damage: VP=%d->%d hit=%d expected=%d\n",before,target->GetUnitRPG()->GetTotalVP(),event?event->nHitValue:-999,expectedDamage);std::fflush(stdout);
        Require(event && event->pUnit.GetPtr()==target && event->nHitValue==expectedDamage && target->GetUnitRPG()->GetTotalVP()==before-expectedDamage && !world->GetHitEvent(),
            "incidental traced entry did not deliver its actual native damage exactly once");
    }
}

static void LooseRayBounds(const char* databasePath,const char* resourcePath)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    auto* original=NDb::GetArmor(NDb::N_DEFAULT_ARMOR);Require(IsValid(original) && IsValid(original->pMaterial),"loose-ray fixture lacks native default armor");
    CObj<NDb::CRPGMaterial> material=new NDb::CRPGMaterial;material->nDR=original->pMaterial->nDR;material->fDensity=2;
    CObj<NDb::CRPGArmor> armor=new NDb::CRPGArmor;armor->pMaterial=material;armor->fRicochetMaxCos=0;armor->fRicochetProbability=0;
    CObj<LooseRayWallToken> token=new LooseRayWallToken;
    CObj<LooseRayTraceMap> map=new LooseRayTraceMap;
    NAI::SSourceInfo source(token,armor,3,NWorld::TS_FRAGMENTED);
    NRPG::SAttackRayInfo ray;ray.vOrigin=VNULL3;ray.vDir=CVec3(1,0,0);ray.atk.nDmgType=1;ray.atk.nK=100000;
    ray.fMinClearDistance=5;ray.fMaxRange=8;
    map->slabs={{2,3,source,1},{4,6,source,2},{10,11,source,3}};
    vector<NRPG::STrailPoint> trail;NRPG::TraceLooseRay(map,ray,&trail);
    Require(!trail.empty(),"native bounded loose ray produced no trajectory");
    std::printf("native loose-ray bounds: start=%g expected=5 points=%zu expected=4 end=%g\n",trail.front().vPosition.x,trail.size(),trail.back().vPosition.x);std::fflush(stdout);
    Require(fabs(trail.front().vPosition.x-5)<1e-5f,"native loose ray started before its minimum clear distance");
    Require(trail.size()==4 && trail[1].nUserID==2 && trail[2].nUserID==2 && fabs(trail[1].vPosition.x-4)<1e-5f && fabs(trail[2].vPosition.x-6)<1e-5f && fabs(trail.back().vPosition.x-8)<1e-5f,
        "loose ray did not skip cleared surfaces, retain a straddling interval or stop at range");
    ray.fMinClearDistance=0;map->slabs={{8,9,source,4}};NRPG::TraceLooseRay(map,ray,&trail);
    Require(trail.size()==3 && trail[1].nUserID==4 && fabs(trail[1].vPosition.x-8)<1e-5f,"loose ray discarded an entry exactly at its range boundary");
    map->slabs.clear();ray.fMaxRange=0;NRPG::TraceLooseRay(map,ray,&trail);
    Require(trail.size()==2 && trail.front().vPosition==VNULL3 && trail.back().vPosition==VNULL3,"zero-range loose ray invented a thirty-unit trajectory");
    ray.fMinClearDistance=3;ray.fMaxRange=8;ray.pIgnore=token.GetPtr();map->slabs={{4,6,source,5}};NRPG::TraceLooseRay(map,ray,&trail);
    Require(trail.size()==2 && fabs(trail.front().vPosition.x-3)<1e-5f && fabs(trail.back().vPosition.x-8)<1e-5f,"loose-ray clipping lost its ignored-object control");
}

static void LooseRayTermination(const char* databasePath,const char* resourcePath,bool stopped)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    auto* original=NDb::GetArmor(NDb::N_DEFAULT_ARMOR);Require(IsValid(original) && IsValid(original->pMaterial),"loose-ray terminal fixture lacks default armor");
    CObj<NDb::CRPGMaterial> material=new NDb::CRPGMaterial;material->nDR=original->pMaterial->nDR;material->fDensity=2;
    CObj<NDb::CRPGArmor> armor=new NDb::CRPGArmor;armor->pMaterial=material;armor->fRicochetMaxCos=0;armor->fRicochetProbability=0;
    CObj<LooseRayWallToken> token=new LooseRayWallToken;CObj<LooseRayTraceMap> map=new LooseRayTraceMap;
    NAI::SSourceInfo source(token,armor,3,NWorld::TS_FRAGMENTED);map->slabs={{2,3,source,7}};
    NRPG::SAttackRayInfo ray;ray.vOrigin=VNULL3;ray.vDir=CVec3(1,0,0);ray.fMaxRange=8;ray.atk.nDmgType=1;ray.atk.nK=stopped?1:10;
    vector<NRPG::STrailPoint> trail;NRPG::TraceLooseRay(map,ray,&trail);
    Require(trail.size()>=2 && trail[1].nUserID==7 && trail[1].pObject.GetPtr()==token.GetPtr() && trail[1].pAttackTarget.GetPtr()==token.GetPtr(),
        "terminal fixture did not traverse its native nonunit armor entry");
    const auto& end=trail.back();
    std::printf("native loose-ray terminal: stopped=%d points=%zu end=%g kinetic=%d expectedEnd=%d expectedKinetic=%d\n",
        int(stopped),trail.size(),end.vPosition.x,end.sAttack.nK,stopped?2:8,stopped?1:8);std::fflush(stdout);
    if(stopped)Require(trail.size()==2 && fabs(end.vPosition.x-2)<1e-5f && end.pObject.GetPtr()==token.GetPtr(),
        "stopped native loose bullet appended an endpoint beyond its blocking surface");
    else Require(trail.size()==4 && fabs(end.vPosition.x-8)<1e-5f && end.sAttack.nK==8 && !IsValid(end.pObject),
        "penetrating native loose bullet restored its initial energy at the terminal point");
    if(!stopped)Require(trail[2].pObject.GetPtr()==token.GetPtr() && !IsValid(trail[2].pAttackTarget) && !IsValid(end.pAttackTarget),
        "nonunit exit or terminal endpoint acquired an entry-only damage receiver");
    map->slabs.clear();NRPG::TraceLooseRay(map,ray,&trail);
    Require(trail.size()==2 && fabs(trail.back().vPosition.x-8)<1e-5f && trail.back().sAttack.nK==ray.atk.nK,
        "terminal ownership truncated or weakened an unobstructed native ray");
}

static void LooseRayRicochet(const char* databasePath,const char* resourcePath,int kind)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    auto* original=NDb::GetArmor(NDb::N_DEFAULT_ARMOR);Require(IsValid(original) && IsValid(original->pMaterial),"ricochet fixture lacks native default armor");
    CObj<NDb::CRPGMaterial> material=new NDb::CRPGMaterial;material->nDR=original->pMaterial->nDR;material->fDensity=2;
    CObj<NDb::CRPGArmor> passing=new NDb::CRPGArmor;passing->pMaterial=material;passing->fRicochetMaxCos=0;passing->fRicochetProbability=0;
    CObj<NDb::CRPGArmor> reflecting=new NDb::CRPGArmor;reflecting->pMaterial=material;reflecting->fRicochetMaxCos=kind==2?1.f:.5f;
    const unsigned int seed=0x52a1;
    CRandomGenerator reference;reference.SeedForHarness(seed);const float roll=reference.GetFloat(0,1);
    Require(roll>0 && roll<1,"ricochet fixture seed produced a probability endpoint");
    reflecting->fRicochetProbability=kind==1?0.f:kind==3?roll:1.f;
    const bool bounce=kind==0 || kind==4;
    const int draws=kind==2?0:1;
    reference.SeedForHarness(seed);if(draws)reference.GetFloat(0,1);const unsigned int expectedNext=reference.Get();
    CObj<LooseRayWallToken> token=new LooseRayWallToken;CObj<LooseRayTraceMap> map=new LooseRayTraceMap;
    NAI::SSourceInfo passSource(token,passing,3,NWorld::TS_FRAGMENTED),bounceSource(token,reflecting,3,NWorld::TS_FRAGMENTED);
    map->slabs={{.2f,.3f,passSource,11},{1,1.02f,bounceSource,12}};
    NRPG::SAttackRayInfo ray;ray.vOrigin=VNULL3;ray.vDir=kind==2?CVec3(1,0,0):CVec3(.2f,1,0);Normalize(&ray.vDir);
    ray.fMaxRange=8;ray.atk.nDmgType=1;ray.atk.nK=10;
    if(kind==4) {
        // The original keeps a straddling interval even when its entry precedes
        // clear distance. After reflection, one surface is fully clear and the
        // other straddles the correctly shifted remaining clear distance.
        ray.fMinClearDistance=6;
        map->slabs={{.2f,.3f,passSource,11},{.8f,.9f,passSource,13},{.92f,.96f,passSource,14},{1,1.5f,bounceSource,12}};
    }
    GlobalGameRandom().SeedForHarness(seed);
    vector<NRPG::STrailPoint> trail;NRPG::TraceLooseRay(map,ray,&trail);
    Require(!trail.empty(),"ricochet fixture produced no native trajectory");
    const float impactDistance=1.f/ray.vDir.x;
    const CVec3 impact=ray.vDir*impactDistance,reflected(-ray.vDir.x,ray.vDir.y,ray.vDir.z);
    const CVec3 expectedEnd=bounce?impact+reflected*(ray.fMaxRange-impactDistance):ray.vDir*ray.fMaxRange;
    const int expectedKinetic=bounce?8:kind==2?8:7;
    std::printf("native loose-ray ricochet: kind=%d rays=%zu expectedRays=%d end=%g,%g expectedEnd=%g,%g kinetic=%d expectedKinetic=%d expectedDraws=%d\n",
        kind,map->rays.size(),bounce?2:1,trail.back().vPosition.x,trail.back().vPosition.y,expectedEnd.x,expectedEnd.y,trail.back().sAttack.nK,expectedKinetic,draws);std::fflush(stdout);
    Require(map->rays.size()==std::size_t(bounce?2:1) && fabs(trail.back().vPosition-expectedEnd)<1e-4f,
        "native loose ray did not follow the original reflected direction and remaining range");
    Require(trail.back().sAttack.nK==expectedKinetic,"native reflected ray lost its carried energy or shifted clear distance");
    if(bounce) {
        Require(fabs(map->rays[1].ptOrigin-impact)<1e-5f && fabs(map->rays[1].ptDir-reflected)<1e-5f,
            "native recursive trace did not start at the actual impact with the reflected direction");
        int reflectedRecords=0,clearedRecords=0;
        for(const auto& point:trail) {
            if(point.nUserID==12) {
                ++reflectedRecords;
                Require(fabs(point.vPosition-impact)<1e-5f && fabs(point.vDir-reflected)<1e-5f && point.sAttack.nK==(kind==4?10:8) &&
                    point.pObject.GetPtr()==token.GetPtr() && !IsValid(point.pAttackTarget),"native reflection record lost pre-impact energy or object identity");
            }
            if(point.nUserID==14)++clearedRecords;
        }
        Require(reflectedRecords==1 && clearedRecords==0,"reflected ray re-hit its departing surface or a fully cleared interval");
    }
    const unsigned int actualNext=GlobalGameRandom().Get();
    std::printf("native loose-ray RNG: kind=%d expectedDraws=%d actualNext=%u expectedNext=%u\n",kind,draws,actualNext,expectedNext);std::fflush(stdout);
    Require(actualNext==expectedNext,"native loose ray consumed a different number of original ISAAC draws");
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

static void ActionSkillAcknowledgment(const char* databasePath,bool interrupt = false)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NDb::CRPGPers* persona = nullptr;
    CDBIterator<NDb::CRPGPers> rows(*NDatabase::GetTable<NDb::CRPGPers>());
    while (rows.MoveNext()) {
        auto* candidate = rows.Get();
        if (IsValid(candidate->pClass) && IsValid(candidate->pBaseValue) && IsValid(candidate->pModel) &&
            IsValid(candidate->pName) && !IsValid(candidate->pPanzerklein) &&
            !IsValid(candidate->pDefaultWearsPanzerklein) && candidate->pClass->skills[NDb::ST_LEVEL] > 0) {
            persona = candidate; break;
        }
    }
    Require(persona != nullptr,"skill acknowledgment fixture has no authored ordinary classed persona");
    struct Case { NRPG::EAction action; NDb::ESkillType skill; int flags = 0; };
    vector<Case> cases = {
        {NRPG::AC_MELEE,NDb::ST_MELEE}, {NRPG::AC_THROW_GRENADE,NDb::ST_THROWING},
        {NRPG::AC_THROW_KNIFE,NDb::ST_THROWING}, {NRPG::AC_SHOOT,NDb::ST_SHOOTING},
        {NRPG::AC_PREPARE_AND_SHOOT,NDb::ST_SHOOTING}, {NRPG::AC_BURST,NDb::ST_BURST},
        {NRPG::AC_MOVE_SIDE,NDb::ST_AP}, {NRPG::AC_MOVE_DIAGONAL,NDb::ST_AP}
    };
    if (interrupt) cases = {
        {NRPG::AC_NONE,NDb::ST_INTERRUPT,0}, {NRPG::AC_NONE,NDb::ST_INTERRUPT,1},
        {NRPG::AC_NONE,NDb::ST_INTERRUPT,2}, {NRPG::AC_NONE,NDb::ST_INTERRUPT,3},
        {NRPG::AC_NONE,NDb::ST_INTERRUPT,4}
    };
    for (const auto& test: cases) {
        CObj<NRPG::CUnit> unit = new NRPG::CUnit(persona), reference = new NRPG::CUnit(persona);
        CObj<NRPG::IUnitMission> mission = NRPG::CreateUnit(unit);
        const float extraXP = unit->GetXPForSkill(NDb::ST_LEVEL,int(unit->Skills(NDb::ST_LEVEL))+3)-unit->fXP+1;
        Require(extraXP > 0,"skill acknowledgment fixture has no higher native experience threshold");
        unit->AddXP(extraXP); reference->AddXP(extraXP);
        CObj<NRPG::IUnitMission> opponent;
        if (interrupt) {
            CObj<NRPG::CUnit> enemy = new NRPG::CUnit(persona);
            opponent = NRPG::CreateUnit(enemy);
            if (test.flags == 4) {
                mission->SpendAP(unit->Skills(NDb::ST_AP));
                CObj<NRPG::IUnitMission> referenceMission = NRPG::CreateUnit(reference);
                referenceMission->SpendAP(reference->Skills(NDb::ST_AP));
                Require(unit->Skills(NDb::ST_AP) == 0,"interrupt fixture failed to spend its available AP");
            }
        }
        auto unchangedTraining = [&]() {
            Require(unit->fXP == reference->fXP,"acknowledging training changed total experience");
            for (int skill=0;skill<NDb::SKILL_TYPE_NUMBERS;++skill) {
                const auto& actual = unit->Skills(skill); const auto& expected = reference->Skills(skill);
                Require(int(actual) == int(expected) && actual.GetXPPart() == expected.GetXPPart() &&
                    actual.GetProgress() == expected.GetProgress() && actual.GetMaxValue() == expected.GetMaxValue(),
                    "action training diverged from the unchanged native unit training formula");
            }
        };
        unchangedTraining();
        int ack = -1; NRPG::IUnitMissionInfo* attacker = nullptr;
        mission->RegisterAction(NRPG::AC_NONE);
        Require(!mission->GetAck(&ack,&attacker),"an action without skill training emitted an acknowledgment");
        int gained = 0, fractional = 0;
        for (int use=0;use<500 && gained<2;++use) {
            const bool advanced = reference->UseSkill(test.skill,0); // Ordinary human has no PK skill offset.
            if (interrupt) {
                auto expectedRandom = GlobalGameRandom();
                const int roll = expectedRandom.Get(100);
                NRPG::SUnitInfo info;
                mission->GetInfo(NAI::WALK, &info);
                const auto& constants = *mission->GetInterruptsConstants();
                const int base = (test.flags & 2) ? constants.nMissedShotInterruptsBase :
                    (test.flags & 1) ? constants.nInterruptsBase : constants.nBackInterruptsBase;
                const int skills = int(unit->Skills(NDb::ST_INTERRUPT)) -
                    int(opponent->GetRPGUnit()->Skills(NDb::ST_INTERRUPT));
                const int penalty = int(constants.fAPInterruptReduction * (info.nMaxAP - info.nAP));
                const int probability = info.nAP <= constants.nMinInterruptAP ? 0 : Clamp(base + skills - penalty, 5, 95);
                Require(test.flags != 4 || probability == 0, "exhausted-AP fixture did not reach its zero-probability gate");
                const int outcome = mission->CheckInterrupt(opponent,test.flags & 1,test.flags & 2);
                auto afterRandom = GlobalGameRandom();
                for (int next=0;next<4;++next)
                    Require(afterRandom.Get() == expectedRandom.Get(),
                        "interrupt skill acknowledgment changed the native single-roll random stream");
                const int expected = roll <= probability ? int(reference->Skills(NDb::ST_INTERRUPT)) : -1;
                Require(outcome == expected, "native interrupt did not rank a success by skill or return minus one on failure");
            }
            else mission->RegisterAction(test.action);
            unchangedTraining();
            const bool acknowledged = mission->GetAck(&ack,&attacker);
            if (!advanced) ++fractional;
            else {
                ++gained;
                std::printf("native skill acknowledgment: action=%d skill=%d use=%d gained=%d ack=%d fractional=%d flags=%d\n",
                    int(test.action),int(test.skill),use,gained,int(acknowledged),fractional,test.flags);
            }
            Require(acknowledged == advanced && (!advanced || (ack == NWorld::N_ACK_SKILL && attacker == nullptr)),
                "a native action improved its skill without the corresponding skill acknowledgment");
            Require(!mission->GetAck(&ack,&attacker),"one skill improvement emitted duplicate acknowledgments");
        }
        Require(gained == 2 && fractional > 0,"skill acknowledgment fixture never crossed two actual native skill boundaries");
    }
}

static void SnipeSkillAcknowledgment(const char* databasePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NDb::CRPGPers* persona = nullptr;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while (people.MoveNext()) {
        auto* candidate = people.Get();
        if (IsValid(candidate->pClass) && IsValid(candidate->pBaseValue) && IsValid(candidate->pModel) &&
            IsValid(candidate->pName) && !IsValid(candidate->pPanzerklein) &&
            !IsValid(candidate->pDefaultWearsPanzerklein) && candidate->pClass->skills[NDb::ST_LEVEL] > 0) {
            persona = candidate; break;
        }
    }
    NDb::CRPGWeapon* record = nullptr;
    CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while (weapons.MoveNext()) {
        auto* candidate = weapons.Get();
        if (IsValid(candidate->pItem) && IsValid(candidate->pInnerClip) && IsValid(candidate->pWeaponType) &&
            candidate->pInnerClip->nQuantity > 0 && candidate->eWeaponType == NDb::WT_RIFLE &&
            candidate->shootModes[NDb::SM_Snipe]) { record = candidate; break; }
    }
    Require(persona && record,"snipe acknowledgment fixture has no authored ordinary persona and sniper rifle");
    for (bool anonymous: {false,true}) {
        CObj<NRPG::CUnit> unit = new NRPG::CUnit(persona), reference = new NRPG::CUnit(persona);
        CObj<NRPG::IUnitMission> mission = NRPG::CreateUnit(unit), target = NRPG::CreateUnit(persona);
        const float extraXP = unit->GetXPForSkill(NDb::ST_LEVEL,int(unit->Skills(NDb::ST_LEVEL))+3)-unit->fXP+1;
        Require(extraXP > 0,"snipe acknowledgment fixture has no higher native XP threshold");
        unit->AddXP(extraXP); reference->AddXP(extraXP);
        auto unchangedTraining = [&]() {
            Require(unit->fXP == reference->fXP,"snipe acknowledgment changed total experience");
            for (int skill=0;skill<NDb::SKILL_TYPE_NUMBERS;++skill) {
                const auto& actual = unit->Skills(skill); const auto& expected = reference->Skills(skill);
                Require(int(actual) == int(expected) && actual.GetXPPart() == expected.GetXPPart() &&
                    actual.GetProgress() == expected.GetProgress() && actual.GetMaxValue() == expected.GetMaxValue(),
                    "snipe training diverged from the unchanged native training formula");
            }
        };
        auto* inventory = mission->GetInventory();
        const auto slot = NDb::ESlot(inventory->GetActiveSlot());
        CObj<NRPG::CWeaponItem> weapon;
        auto loadedRifle = [&]() {
            if (weapon && weapon->HasAmmo()) return;
            CObj<NRPG::IInventoryItem> previous = inventory->Get(slot) ? inventory->TakeOff(slot) : nullptr;
            weapon = new NRPG::CWeaponItem(record);
            Require(weapon->SetShootMode(NDb::SM_Snipe) && weapon->HasAmmo() && inventory->Equip(slot,weapon),
                "snipe acknowledgment fixture failed to equip a loaded native sniper rifle");
        };
        loadedRifle();
        int ack = -1; NRPG::IUnitMissionInfo* attacker = nullptr;
        vector<NRPG::CAttackPortion> ordinary;
        const int initialAmmo = weapon->GetAmmoQuantity();
        Require(mission->CreateAttack(&ordinary,true,anonymous,target,false,false) && ordinary.size() == 1 &&
            weapon->GetAmmoQuantity() == initialAmmo-1,"ordinary-shot control did not consume exactly one round");
        unchangedTraining();
        Require(!mission->GetAck(&ack,&attacker),"an ordinary shot trained or acknowledged the sniper skill");
        mission->SaveAP(NRPG::SSnipeAP(4,target));
        auto attackFields = [&](NRPG::CAttackPortion& attack) {
            CStructureNetworkContext refs;
            refs.externalReferences = true; refs.include = [](CObjectBase*) { return false; };
            refs.ids[mission.GetPtr()] = 1; refs.refs[1] = mission.GetPtr();
            refs.ids[target.GetPtr()] = 2; refs.refs[2] = target.GetPtr(); refs.nextID = 10;
            CMemoryStream saved;
            { CStructureSaver writer(saved,CStructureSaver::WRITE,&refs); writer.Add(2,&attack); }
            const auto* bytes = static_cast<const unsigned char*>(saved.GetBuffer());
            return vector<unsigned char>(bytes,bytes+saved.GetSize());
        };
        int gained = 0, fractional = 0;
        for (int use=0;use<500 && gained<2;++use) {
            loadedRifle();
            const int ammo = weapon->GetAmmoQuantity();
            vector<NRPG::CAttackPortion> preview, actual;
            Require(mission->CreateAttack(&preview,false,anonymous,target,false,false) && preview.size() == 1 &&
                weapon->GetAmmoQuantity() == ammo,"sniper preview consumed ammunition or lost its attack portion");
            unchangedTraining();
            Require(!mission->GetAck(&ack,&attacker),"a sniper preview trained or acknowledged the skill");
            const bool advanced = reference->UseSkill(NDb::ST_SNIPE,0);
            Require(mission->CreateAttack(&actual,true,anonymous,target,false,false) && actual.size() == 1 &&
                weapon->GetAmmoQuantity() == ammo-1,"native sniper shot failed to consume exactly one round");
            Require(attackFields(actual.front()) == attackFields(preview.front()) &&
                mission->GetSavedAP().pTarget == target.GetPtr() && mission->GetSavedAP().nAP == 4,
                "sniper acknowledgment changed the produced attack or saved aiming AP");
            unchangedTraining();
            const bool acknowledged = mission->GetAck(&ack,&attacker);
            if (advanced) {
                ++gained;
                std::printf("native snipe acknowledgment: anonymous=%d use=%d gained=%d ack=%d fractional=%d\n",
                    int(anonymous),use,gained,int(acknowledged),fractional);
            } else ++fractional;
            Require(acknowledged == advanced && (!advanced || (ack == NWorld::N_ACK_SKILL && attacker == nullptr)),
                "a native sniper shot improved its skill without the corresponding acknowledgment");
            Require(!mission->GetAck(&ack,&attacker),"one sniper skill improvement emitted duplicate acknowledgments");
        }
        Require(gained == 2 && fractional > 0,"sniper acknowledgment fixture did not cross two native skill boundaries");
    }
}

static void VitalitySkillAcknowledgment(const char* databasePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    NDb::CRPGPers* persona = nullptr;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while (people.MoveNext()) {
        auto* candidate = people.Get();
        if (IsValid(candidate->pClass) && IsValid(candidate->pBaseValue) && IsValid(candidate->pModel) &&
            IsValid(candidate->pName) && !IsValid(candidate->pPanzerklein) &&
            !IsValid(candidate->pDefaultWearsPanzerklein) && candidate->pClass->skills[NDb::ST_LEVEL] > 0) {
            persona = candidate; break;
        }
    }
    auto* armor = NDb::GetArmor(NDb::N_HUMAN_BODY_ARMOR);
    Require(persona && armor && IsValid(armor->pMaterial),"vitality acknowledgment fixture lacks its authored persona/body armor");
    int damageType = -1;
    CDBIterator<NDb::CRPGDmgToArmor> damageTypes(*NDatabase::GetTable<NDb::CRPGDmgToArmor>());
    while (damageTypes.MoveNext()) {
        auto* type = damageTypes.Get();
        if (type->armors[armor->pMaterial->nDR] > 0) { damageType = type->GetRecordID(); break; }
    }
    Require(damageType >= 0,"vitality acknowledgment fixture has no damage type that reaches human body armor");
    for (bool anonymous: {false,true}) {
        CObj<NRPG::CUnit> unit = new NRPG::CUnit(persona), reference = new NRPG::CUnit(persona);
        CObj<NRPG::IUnitMission> mission = NRPG::CreateUnit(unit), attackerMission = NRPG::CreateUnit(persona);
        const float extraXP = unit->GetXPForSkill(NDb::ST_LEVEL,int(unit->Skills(NDb::ST_LEVEL))+3)-unit->fXP+1;
        Require(extraXP > 0,"vitality acknowledgment fixture has no higher native XP threshold");
        unit->AddXP(extraXP); reference->AddXP(extraXP);
        auto unchangedTraining = [&]() {
            Require(unit->fXP == reference->fXP,"vitality acknowledgment changed total experience");
            for (int skill=0;skill<NDb::SKILL_TYPE_NUMBERS;++skill) {
                const auto& actual = unit->Skills(skill); const auto& expected = reference->Skills(skill);
                Require(int(actual) == int(expected) && actual.GetXPPart() == expected.GetXPPart() &&
                    actual.GetProgress() == expected.GetProgress() && actual.GetMaxValue() == expected.GetMaxValue(),
                    "damage/vitality training diverged from the unchanged native training formula");
            }
        };
        NRPG::CAttackPortion portion(100000,damageType,0.f,1,1,0,0,
            anonymous ? nullptr : attackerMission.GetPtr(),mission,1.f,-1);
        int ack = -1; NRPG::IUnitMissionInfo* ackAttacker = nullptr;
        auto blocked = portion; blocked.nK = 0;
        Require(mission->ProcessAttack(nullptr,NAI::HL_BODY,&blocked,nullptr).nDmg == 0,
            "vitality control did not reject an attack with zero penetration");
        unchangedTraining();
        Require(!mission->GetAck(&ack,&ackAttacker),"blocked damage trained or acknowledged vitality");
        int gained = 0, fractional = 0;
        for (int use=0;use<500 && gained<2;++use) {
            Require(int(unit->Skills(NDb::ST_VP)) > 8 && !mission->HasCritical(NDb::C_VP),
                "vitality fixture is no longer healthy enough to isolate skill acknowledgment");
            const int beforeVP = unit->Skills(NDb::ST_VP);
            reference->Skills(NDb::ST_VP) -= 1;
            const bool advanced = reference->UseSkill(NDb::ST_VP,0);
            auto expectedRandom = GlobalGameRandom();
            if (!anonymous)
                for (int level=0;level<attackerMission->GetSkillValue(NDb::ST_LEVEL);++level)
                    expectedRandom.Get(1,20);
            expectedRandom.Get(100);
            const int damage = mission->ProcessAttack(nullptr,NAI::HL_BODY,&portion,nullptr).nDmg;
            auto afterRandom = GlobalGameRandom();
            for (int next=0;next<4;++next)
                Require(afterRandom.Get() == expectedRandom.Get(),
                    "vitality acknowledgment changed the native noncritical damage random stream");
            Require(damage == 1 && !mission->IsDead() && !mission->IsUnconscious() &&
                !mission->HasCritical(NDb::C_VP) && unit->Skills(NDb::ST_VP) == beforeVP-1,
                "vitality fixture did not process exactly one noncritical, nonfatal point of body damage");
            unchangedTraining();
            const bool acknowledged = mission->GetAck(&ack,&ackAttacker);
            if (advanced) {
                ++gained;
                std::printf("native vitality acknowledgment: anonymous=%d use=%d damage=%d gained=%d ack=%d fractional=%d\n",
                    int(anonymous),use,damage,gained,int(acknowledged),fractional);
            } else ++fractional;
            Require(acknowledged == advanced && (!advanced || (ack == NWorld::N_ACK_SKILL && ackAttacker == nullptr)),
                "native body damage improved vitality without the corresponding skill acknowledgment");
            Require(!mission->GetAck(&ack,&ackAttacker),"one vitality improvement emitted duplicate acknowledgments");
            // Keep this controlled training fixture healthy between hits; medical gameplay is separate.
            unit->Skills(NDb::ST_VP) += damage; reference->Skills(NDb::ST_VP) += damage;
            unchangedTraining();
        }
        Require(gained == 2 && fractional > 0,"vitality fixture did not cross two native skill boundaries");
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

static void NativeMeleeKnifeCritical(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=7,"unknown native knife critical classification mode");CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;for(auto u:units)if(u && u->CanFight() && !u->IsEmptyPK() && !u->IsWearingPK() && !u->HasCommand() && NWorld::GetActionType(u)==NWorld::AT_SHOOT){actor=u;break;}Require(actor,"knife critical fixture lacks actual ordinary armed actor");auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous=inventory->Get(NDb::SLOT_1)?inventory->TakeOff(NDb::SLOT_1):nullptr;Require(inventory->Activate(NDb::SLOT_1),"knife critical fixture cannot select empty hand");CObj<NRPG::CMeleeWeaponItem> blade;int recordID=-1;
    if(mode<2 || mode==4 || mode==5){const bool throwing=mode>=4;NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> records(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());while(records.MoveNext()){auto* candidate=records.Get();if(candidate->bThrowing==throwing && candidate->pItem && candidate->pItem->pModel && (!record || candidate->GetRecordID()<record->GetRecordID()))record=candidate;}Require(record,"knife critical fixture lacks authored modelled blade");blade=new NRPG::CMeleeWeaponItem(record);blade->nQuantity=1;recordID=record->GetRecordID();Require(inventory->Equip(NDb::SLOT_1,blade),"knife critical fixture cannot equip actual blade");}
    if(mode>=6){Require(previous && dynamic_cast<NRPG::IWeaponItem*>(previous.GetPtr()) && inventory->Equip(NDb::SLOT_1,previous),"knife critical firearm control lacks actual native original gun");}
    const bool tile=mode&1,fists=mode==2 || mode==3;CObj<NWorld::CCmd> command;if(tile)command=new NWorld::CCmdShootTile(actor->GetPosition().GetCP()+CVec3(1,0,0));else command=new NWorld::CCmdShootObject(nullptr,0);NWorld::CCriticalsBan bans;const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),cost=actor->GetActionAP(NRPG::AC_MELEE);const auto position=actor->GetPosition();GlobalGameRandom().SeedForHarness(666);auto random=GlobalGameRandom();const auto actual=bans.GetCommandBans(actor,command);list<NDb::ECritical> expected;if(!tile)expected.push_back(NDb::C_BLIND);if(fists)expected.push_back(NDb::C_MOTIONLESS);expected.push_back(NDb::C_IDLE_HAND);if(fists)expected.push_back(NDb::C_PATIENT);expected.push_back(NDb::C_PANZERKLEIN_BROKEN);
    std::printf("native knife critical mode=%d blade=%d tile=%d fists=%d action=%d bans=%zu expected=%zu motionless=%d patient=%d\n",mode,recordID,int(tile),int(fists),int(NWorld::GetActionType(actor)),actual.size(),expected.size(),int(std::find(actual.begin(),actual.end(),NDb::C_MOTIONLESS)!=actual.end()),int(std::find(actual.begin(),actual.end(),NDb::C_PATIENT)!=actual.end()));std::fflush(stdout);Require(actual==expected,"native equipped knife inherited fist-only motionless/patient command restrictions");
    if(mode<2)Require(NWorld::GetActionType(actor)!=NWorld::AT_MELEE && NWorld::GetActionType(actor)!=NWorld::AT_THROW && actor->GetState()==NWorld::CUnit::ST_NORMAL_KNIFE,"nonthrowing native blade lost distinct action/cursor state");else Require(NWorld::GetActionType(actor)==(fists?NWorld::AT_MELEE:mode<6?NWorld::AT_THROW:NWorld::AT_SHOOT),"native knife classification changed fist/throw/firearm action identity");Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && actor->GetActionAP(NRPG::AC_MELEE)==cost && !actor->HasCommand() && GlobalGameRandom().Get()==random.Get() && (!blade || blade->GetQuantity()==1),"critical classification query changed native AP/HP/position/cost/order/RNG/item");
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
    // Action classification/critical restrictions have their own native original-matrix regression.
    Require(actor->GetActionAP(NRPG::AC_MELEE) == meleeCost,
        "refreshing the melee UI state changed its AP-cost query");
    CObj<NRPG::IInventoryItem> removed = inventory->TakeOff(NDb::SLOT_1);
    state->RefreshCursor();
    Require(actor->GetState() == NWorld::CUnit::ST_NORMAL_MELEE && state->GetCursorInfo().pCursor == NDb::GetUICursor(8),
        "unequipping the melee weapon did not restore the original native fist cursor");
    Require(actor->GetAP() == ap && actor->GetUnitRPG()->GetTotalVP() == vp && melee->GetQuantity() == 1 && !actor->HasCommand(),
        "refreshing native equipped-melee cursors spent AP/VP, consumed the weapon or installed an order");
}

static void InventoryNoPlaceUI(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);list<CPtr<NWorld::CUnitServer>> actors;world->GetAllUnits(&actors);NWorld::CUnitServer* actor=nullptr;
    for(auto p:actors)if(p && !p->IsAIUnit() && p->CanFight() && !p->IsWearingPK() && !p->IsEmptyPK() && !p->HasCommand()){actor=p;break;}Require(actor,"inventory UI fixture lacks actual human actor");auto* owner=dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer());Require(owner,"inventory UI fixture lacks native owner");world->GivePlayerTurn(owner);
    auto* inventory=actor->GetUnitRPG()->GetInventory();vector<NDb::CRPGClip*> clips;CDBIterator<NDb::CRPGClip> records(*NDatabase::GetTable<NDb::CRPGClip>());
    while(records.MoveNext()){auto* p=records.Get();if(p->pItem && p->pItem->sSize.x>0 && p->pItem->sSize.y>0 && p->nQuantity>0)clips.push_back(p);}
    std::sort(clips.begin(),clips.end(),[](const NDb::CRPGClip* a,const NDb::CRPGClip* b){const int aa=a->pItem->sSize.x*a->pItem->sSize.y,bb=b->pItem->sSize.x*b->pItem->sSize.y;return aa<bb || (aa==bb && a->GetRecordID()<b->GetRecordID());});
    CObj<NRPG::IInventoryItem> item;NDb::CRPGClip* clip=nullptr;CTPoint<int> sourceCell;
    for(auto p:clips){CObj<NRPG::IInventoryItem> candidate=NRPG::CreateClipItem(p);if(candidate && inventory->FindPlace(candidate,&sourceCell)){clip=p;item=candidate;break;}}
    Require(item && clip && dynamic_cast<NRPG::IClipItem*>(item.GetPtr()) && dynamic_cast<NRPG::IClipItem*>(item.GetPtr())->GetIncQuantity()>0,"inventory UI fixture lacks a real authored clip fitting its backpack");
    // Establish genuine inventory -> player drag-hand provenance through native APIs.
    Require(inventory->Place(sourceCell,item),"inventory UI fixture cannot place original drag item");inventory->Take(item);actor->GetTBSPlayer()->SetInHandItem(NWorld::SItem(actor,NWorld::SItem::HAND,item));
    vector<CObj<NRPG::IInventoryItem>> fillers;const bool fill=mode==0 || mode==1 || mode==3;
    if(fill){for(int n=0;n<300;++n){CObj<NRPG::IInventoryItem> next=NRPG::CreateClipItem(clip);CTPoint<int> cell;if(!inventory->FindPlace(next,&cell))break;Require(inventory->Place(cell,next),"inventory UI fixture cannot fill native free backpack cell");fillers.push_back(next);}CTPoint<int> cell;Require(!fillers.empty() && !inventory->FindPlace(item,&cell),"inventory UI fixture did not exhaust actual item placement space");}
    if(mode==3){inventory->Take(fillers.back());CTPoint<int> cell;Require(inventory->FindPlace(item,&cell),"inventory UI fixture did not recover actual space after removing one filler");}
    if(mode==1)for(int n=0;n<NDb::N_SLOTS;++n)Require(!inventory->CanEquip(NDb::ESlot(n),item),"inventory UI clip unexpectedly fits a hand/equipment slot");
    if(mode==5){actor->GetTBSPlayer()->SetInHandItem(NWorld::SItem(actor,NWorld::SItem::VACUUM));NWorld::SItem empty;Require(!actor->GetTBSPlayer()->GetInHandItem(&empty),"inventory UI fixture did not clear actual unit-owned drag hand");}
    const NWorld::SItem target=mode==1?NWorld::SItem(actor,NWorld::SItem::UNIT_ANYPLACE):mode==4?NWorld::SItem(actor,NWorld::SItem::SLOT,int(NDb::SLOT_1)):NWorld::SItem(actor,NWorld::SItem::BACKPACK,CTPoint<int>(-1,-1));
    CObj<NWorld::CCmdMoveInventoryItem> command=new NWorld::CCmdMoveInventoryItem(NWorld::SItem(actor,NWorld::SItem::HAND,item),target);
    CObj<NWorld::CExecMoveInventoryItem> executor=new NWorld::CExecMoveInventoryItem(actor,command);
    const auto expected=mode==0 || mode==1?NWorld::UCR_INVENTORY_NO_PLACE:mode==4?NWorld::UCR_GENERAL_FAILURE:mode==5?NWorld::UCR_INVALID_COMMAND:NWorld::UCR_OK;
    Require(executor->CanDoIt(actor->GetPosition())==expected,"inventory UI fixture native real-item executor has unexpected eligibility");
    CObj<NativeActionInfoMission> mission=new NativeActionInfoMission;mission->Attach(world,game);CObj<NGame::CUnitTracker> tracker=new NGame::CUnitTracker(mission,actor);mission->selection.push_back(tracker.GetPtr());
    const int ammunition=dynamic_cast<NRPG::IClipItem*>(item.GetPtr())->GetIncQuantity();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),quantity=item->GetQuantity(),count=int(inventory->GetItems().size());const auto active=inventory->GetActive();const auto activeSlot=inventory->GetActiveSlot();const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();NWorld::SItem handBefore;const bool hasHand=actor->GetTBSPlayer()->GetInHandItem(&handBefore);auto random=GlobalGameRandom();
    int directMin=-1,directMax=-1;const auto direct=actor->CanDo(command,&directMin,&directMax);Require(direct==expected,"inventory UI fixture public native unit probe differs from actual executor result");
    NGame::SActionInfo info;mission->CanDoCommand(command,false,&info);NGame::SActionInfo again;mission->CanDoCommand(command,false,&again);
    std::uint8_t wire[20]={};NGame::SActionInfo restored;const bool encoded=S2FileIO::StructureFieldCodec<NGame::SActionInfo>::Encode(info,wire,sizeof(wire)) && S2FileIO::StructureFieldCodec<NGame::SActionInfo>::Decode(wire,sizeof(wire),&restored);
    const bool roundtrip=encoded && restored.bValid==info.bValid && restored.bOk==info.bOk && restored.bAvailable==info.bAvailable && restored.bEnoughAP==info.bEnoughAP && restored.bEnoughAPToStart==info.bEnoughAPToStart && restored.nMinAP==info.nMinAP && restored.nMaxAP==info.nMaxAP && restored.eResult==info.eResult && wire[16]==unsigned(expected) && wire[17]==0 && wire[18]==0 && wire[19]==0;
    NWorld::SItem handAfter;const bool hasHandAfter=actor->GetTBSPlayer()->GetInHandItem(&handAfter);auto afterRandom=GlobalGameRandom();const bool unchanged=dynamic_cast<NRPG::IClipItem*>(item.GetPtr())->GetIncQuantity()==ammunition && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && item->GetQuantity()==quantity && int(inventory->GetItems().size())==count && inventory->GetActive()==active && inventory->GetActiveSlot()==activeSlot && actor->GetPosition()==position && !actor->HasCommand() && hasHandAfter==hasHand && (!hasHand || handAfter.pItem==handBefore.pItem) && world->GetTime()->GetValue()==time && random.Get()==afterRandom.Get();
    const bool repeat=again.bValid==info.bValid && again.bOk==info.bOk && again.bAvailable==info.bAvailable && again.bEnoughAP==info.bEnoughAP && again.bEnoughAPToStart==info.bEnoughAPToStart && again.nMinAP==info.nMinAP && again.nMaxAP==info.nMaxAP && again.eResult==info.eResult;
    std::printf("native inventory UI: mode=%d clip=%d fill=%u count=%d target=%d native=%d expected=%d ui=%d valid=%d ok=%d enough=%d startEnough=%d available=%d min=%d max=%d repeat=%d wire=%d rawResult=%u AP=%d quantity=%d ammo=%d hand=%d unchanged=%d\n",mode,clip->GetRecordID(),unsigned(fillers.size()),count,int(target.eType),int(direct),int(expected),int(info.eResult),int(info.bValid),int(info.bOk),int(info.bEnoughAP),int(info.bEnoughAPToStart),int(info.bAvailable),info.nMinAP,info.nMaxAP,int(repeat),int(roundtrip),unsigned(wire[16]),ap,quantity,ammunition,int(hasHand),int(unchanged));std::fflush(stdout);
    Require(unchanged && repeat && roundtrip && info.bValid && info.eResult==expected && info.bEnoughAP,"native inventory UI query changes gameplay/drag ownership/legacy wire or loses native result");
    Require(info.bAvailable==(mode!=5) && info.bOk==(expected==NWorld::UCR_OK),"native full-inventory rejection hides the action instead of remaining blocked but available");
}

static void EquippedToolActionAvailability(const char* databasePath,const char* resourcePath,bool tileAttack = false)
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
    const int vp = actor->GetUnitRPG()->GetTotalVP();
    const auto position = actor->GetPosition();
    CObj<NRPG::IInventoryItem> key;
    if(tileAttack) {
        Require(world->IsAttackAllowed(),"non-attack tile fixture is masked by the world's no-attack gate");
        CDBIterator<NDb::CRPGItem> records(*NDatabase::GetTable<NDb::CRPGItem>());
        while(records.MoveNext()) {
            auto* descriptor=dynamic_cast<NDb::CRPGKey*>(records.Get()->pSuccessor.GetPtr());
            if(IsValid(descriptor) && IsValid(descriptor->pItem)) { key=NRPG::CreateItem(descriptor);break; }
        }
        Require(IsValid(key) && inventory->Equip(NDb::SLOT_1,key) && NWorld::GetActionType(actor)==NWorld::AT_KEY,
            "non-attack tile fixture has no equippable authored native key");
        CObj<NRPG::IInventoryItem> removedKey=inventory->TakeOff(NDb::SLOT_1);
        Require(removedKey==key,"non-attack tile fixture removed a different key");
    }
    auto checkTileAttack = [&](NWorld::EActionType type) {
        Require(NWorld::GetActionType(actor)==type,"non-attack tile fixture has the wrong native action type");
        CObj<NWorld::CCmdShootTile> request=new NWorld::CCmdShootTile(actor->GetPosition().GetCP()+CVec3(1,0,0));
        NWorld::EUnitCommandResult result=NWorld::UCR_OK;
        CObj<NWorld::CCommandExecute> executor=NWorld::CreateActionExecutor(actor,request,&result);
        NGame::SActionInfo info;mission->CanDoCommand(request,true,&info);
        std::printf("native non-attack tile: type=%d item=%d result=%d executor=%d available=%d uiResult=%d\n",int(type),
            inventory->GetActive()->GetDBItem()->GetRecordID(),int(result),int(executor!=nullptr),int(info.bAvailable),int(info.eResult));
        Require(!executor && result==NWorld::UCR_UNAVAILABLE,"a native non-attack item reported an available tile attack failure");
        Require(info.bValid && !info.bAvailable && !info.bOk && info.eResult==NWorld::UCR_UNAVAILABLE,
            "the mission still advertised a tile attack for an equipped non-attack item");
        Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==vp && !actor->HasCommand() &&
            actor->GetPosition().pos.p.GetBits()==position.pos.p.GetBits(),"non-attack tile previews changed native AP, health, position or orders");
    };
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
    if(tileAttack)checkTileAttack(NWorld::AT_TOOL);
    Require(tool->GetQuantity() == quantity, "checking tool availability consumed a charge");
    CObj<NRPG::IInventoryItem> removed = inventory->TakeOff(NDb::SLOT_1);
    if(tileAttack) {
        Require(inventory->Equip(NDb::SLOT_1,key),"non-attack tile fixture could not restore its native key");
        const int keyQuantity=key->GetQuantity();
        checkTileAttack(NWorld::AT_KEY);
        Require(key->GetQuantity()==keyQuantity,"non-attack tile preview consumed a key");
        removed=inventory->TakeOff(NDb::SLOT_1);
    }
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







static bool NativeSavedTemporaryAim(NWorld::CDumbUnitServer* actor)
{
    // Observe the existing native save field; never inject an aim flag/provider.
    CStructureNetworkContext refs;refs.include=[](CObjectBase*){return false;};CMemoryStream fields;
    {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);actor->NWorld::CDumbUnitServer::operator&(writer);}
    fields.Seek(0);bool value=false;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(23,&value);}return value;
}

static void QueuedAim(const char* databasePath,const char* resourcePath,int mode)
{
    const bool aimed=mode!=6 && mode!=7 && mode!=8 && mode!=17 && mode!=18 && mode!=19;
    const bool shortBurst=mode>=3 && mode<=5,bounce=mode>=13 && mode<=15,empty=mode==10 || mode==18;
    const int kind=mode==1 || mode==4 || mode==7 || mode==12 || mode==14?1:mode==2 || mode==5 || mode==8 || mode==15?2:0;
    const bool hasPrefix=mode<=8 || mode==11 || mode==12 || bounce || mode==19;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto p:units)if(p && !p->IsAIUnit() && p->CanFight() && !p->IsWearingPK() && !p->IsEmptyPK() && p->GetUnitRPG()->CanMove() && !p->HasCommand()){actor=p;break;}
    Require(actor,"queued aim fixture lacks actual native human actor");auto* owner=dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer());Require(owner,"queued aim fixture lacks native owner");world->GivePlayerTurn(owner);Require(world->IsTurnBased(),"queued aim fixture lacks real tactical turn");
    auto* record=NDatabase::GetTable<NDb::CRPGWeapon>()->GetRecord(26);Require(record && record->pWeaponType && record->pWeaponType->nPrepareCost>0 && record->shootModes[NDb::SM_Snap] && record->shootModes[NDb::SM_ShortBurst],"queued aim fixture lacks its actual authored firearm26");
    auto* rpg=actor->GetUnitRPG();auto* inventory=rpg->GetInventory();const auto slot=NDb::ESlot(inventory->GetActiveSlot());CObj<NRPG::IInventoryItem> old=inventory->Get(slot)?inventory->TakeOff(slot):nullptr;CObj<NRPG::CWeaponItem> gun=new NRPG::CWeaponItem(record);
    Require(inventory->Equip(slot,gun) && inventory->Activate(slot) && gun->SetShootMode(shortBurst?NDb::SM_ShortBurst:NDb::SM_Snap) && rpg->GetWeaponItem()==gun.GetPtr(),"queued aim fixture cannot equip its actual supported firearm");
    auto* network=world->GetPathNetwork();auto from=actor->GetPosition();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(from.GetCP(),5.f),&places);bool placed=false;
    for(auto place:places){place.SetPose(NAI::CM_STAND);place.SetDirection(from.pos.p.GetDirection());if(!place.IsIntegral() || place.IsFinal() || !network->IsPassable(place) || network->IsLocked(place,true))continue;from.pos.p=place;actor->SetPosition(from);actor->animator.PlaceUnit(from);actor->Update();placed=true;break;}
    Require(placed,"queued aim fixture lacks genuine free integral standing cell");from=actor->GetPosition();actor->animator.SetActiveItem(true);actor->SetUndrawItem(false);actor->SetWalkWithoutWeapon(false);actor->animator.SetWeaponAnimation(rpg->GetWeaponType());actor->animator.SetWeaponName(record->szAnimName.c_str());
    CObj<NWorld::CCommandExecute> prefix;CObj<NAI::CPath> path;auto future=from;int prefixCost=0;
    if(hasPrefix){
        vector<NAI::SPathPlace> destinations;
        if(kind){auto end=from.pos.p;if(kind==1)end.SetDirection((end.GetDirection()+1)%8);else end.SetPose(NAI::CM_CROUCH);destinations.push_back(end);}
        else {network->GetNearPlaces(SSphere(from.GetCP(),4.f),&destinations);}
        for(auto end:destinations){if(!end.IsIntegral() || end.IsFinal())continue;if(!kind){end.SetPose(from.pos.p.GetPose());end.SetDirection(from.pos.p.GetDirection());auto to=from;to.pos.p=end;if(fabs(to.GetCP()-from.GetCP())<.6f)continue;}if(kind){std::printf("native queued aim path probe: mode=%d kind=%d passable=%d locked=%d from=%u end=%u\n",mode,kind,int(network->IsPassable(end)),int(network->IsLocked(end,true)),from.pos.p.GetBits(),end.GetBits());std::fflush(stdout);}if(!kind && (!network->IsPassable(end) || network->IsLocked(end,true)))continue;
            CObj<NAI::CPath> candidate=NWorld::FindPath(network,actor,from.pos.p,vector<NAI::SPathPlace>{end},nullptr,false,NAI::PF_USE_POSEDIR);if(kind){std::printf("native queued aim path result: mode=%d points=%u end=%u\n",mode,candidate?unsigned(candidate->points.size()):0,candidate && !candidate->points.empty()?candidate->points.back().GetBits():0);std::fflush(stdout);}if(!candidate || candidate->points.size()<2 || candidate->points.back()==from.pos.p)continue;
            NWorld::EUnitCommandResult result=NWorld::UCR_OK;prefix=NWorld::CreateMoveExecutor(actor,candidate,NAI::PF_USE_POSEDIR,NWorld::ITEM_NO_MATTER,&result);if(!prefix || result!=NWorld::UCR_OK)continue;path=prefix->GetCurrentPath();if(path && path->points.size()>1 && !(path->points.back()==from.pos.p)){future.pos.p=path->points.back();break;}prefix=nullptr;path=nullptr;
        }
        Require(prefix && path && path->points.size()>1 && !(future.pos.p==from.pos.p),"queued aim fixture cannot build actual native movement/turn/pose preparation");prefixCost=prefix->GetActionAP();Require(prefixCost>=0 && (kind==1 || prefixCost>0),"queued aim fixture native movement/pose price is invalid");
        if(kind==1)Require(future.pos.p.GetX()==from.pos.p.GetX() && future.pos.p.GetY()==from.pos.p.GetY() && future.pos.p.GetPose()==from.pos.p.GetPose() && future.pos.p.GetDirection()!=from.pos.p.GetDirection(),"queued aim turn fixture did not retain actual turn-only endpoint");
        if(kind==2)Require(future.pos.p.GetX()==from.pos.p.GetX() && future.pos.p.GetY()==from.pos.p.GetY() && future.pos.p.GetPose()!=from.pos.p.GetPose(),"queued aim pose fixture did not retain actual pose-only endpoint");
    }
    const float angle=from.GetDirection();const CVec3 point=from.GetCP()+CVec3(20*cos(angle),20*sin(angle),1);Require(NRPG::GetShootDirection(network,from.pos.p,point)==from.pos.p.GetDirection(),"queued aim fixture lacks actual aligned tile target");
    if(aimed){CRay ray;ray.ptOrigin=from.GetEyePosition();ray.ptDir=point-ray.ptOrigin;Normalize(&ray.ptDir);actor->animator.Snipe(from,ray);Require(actor->animator.IsAiming(),"queued aim fixture cannot enter its real native aiming graph");}else Require(!actor->animator.IsAiming(),"queued aim cold fixture unexpectedly aims");
    Require(NativeSavedTemporaryAim(actor),"queued aim fixture starts with a stale temporary aim field");CObj<NWorld::CExecShootTile> shot=new NWorld::CExecShootTile(actor,point);Require(shot->CanDoIt(from)==NWorld::UCR_OK,"queued aim fixture actual native shot validator rejects its loaded actor");
    float bonus=0;rpg->HasPerk(16,&bonus);const int length=int(record->nRoF/6.f+bonus),extra=shortBurst?Max(0,length-1)*rpg->GetActionAP(future.GetPose(),NRPG::AC_BURST):0;
    const int direct=rpg->GetActionAP(from.GetPose(),aimed?NRPG::AC_SHOOT:NRPG::AC_PREPARE_AND_SHOOT)+extra;
    const int first=rpg->GetActionAP(future.GetPose(),hasPrefix?NRPG::AC_PREPARE_AND_SHOOT:aimed?NRPG::AC_SHOOT:NRPG::AC_PREPARE_AND_SHOOT),tail=first+extra;
    const int ap=actor->GetAP(),hp=rpg->GetTotalVP(),ammo=gun->GetAmmoQuantity();const auto count=inventory->GetItems().size();const auto time=world->GetTime()->GetValue();auto rng=GlobalGameRandom();
    if(bounce || mode==16){
        actor->SetTemporaryPosition(bounce?future:from);const bool at=NativeSavedTemporaryAim(actor);actor->SetTemporaryPosition(from);const bool back=NativeSavedTemporaryAim(actor);const int price=shot->GetStartAP();CObj<NWorld::CSimpleExecQueue> reset=new NWorld::CSimpleExecQueue(actor);const int resetPrice=reset->GetActionAP();const bool restored=NativeSavedTemporaryAim(actor);const int afterPrice=shot->GetStartAP();auto after=GlobalGameRandom();
        const int expected=bounce?rpg->GetActionAP(from.GetPose(),NRPG::AC_PREPARE_AND_SHOOT):direct;const bool unchanged=actor->GetAP()==ap && rpg->GetTotalVP()==hp && gun->GetAmmoQuantity()==ammo && actor->GetPosition()==from && actor->animator.IsAiming()==aimed && inventory->GetActive()==gun.GetPtr() && inventory->GetItems().size()==count && world->GetTime()->GetValue()==time && rng.Get()==after.Get() && !actor->HasCommand();
        std::printf("native queued aim transition: mode=%d kind=%d at=%d back=%d price=%d expected=%d reset=%d restored=%d after=%d direct=%d unchanged=%d\n",mode,kind,int(at),int(back),price,expected,resetPrice,int(restored),afterPrice,direct,int(unchanged));std::fflush(stdout);
        Require(unchanged && restored && resetPrice==0 && afterPrice==direct,"temporary aim reset changes physical state or leaves subsequent direct shots unprepared");
        Require(at==!bounce && back==!bounce && price==expected,"native temporary position does not latch aim loss until the original queue reset");return;
    }
    CObj<NWorld::CSimpleExecQueue> queue=new NWorld::CSimpleExecQueue(actor);if(prefix)queue->AddExecutor(prefix);
    if(!empty){CObj<NWorld::CCommandExecute> tailExec=shot.GetPtr();if(mode==11 || mode==12 || mode==19)for(int depth=0;depth<2;++depth){CObj<NWorld::CSimpleExecQueue> nested=new NWorld::CSimpleExecQueue(actor);nested->AddExecutor(tailExec);tailExec=nested.GetPtr();}queue->AddExecutor(tailExec);}
    const int expected=empty?0:prefixCost+tail;const int total=queue->GetActionAP(),repeat=queue->GetActionAP();NWorld::CCommandExecute* base=queue.GetPtr();const int viaBase=base->GetActionAP();const bool restored=NativeSavedTemporaryAim(actor);const int afterPrice=shot->GetStartAP();auto after=GlobalGameRandom();
    const bool unchanged=actor->GetAP()==ap && rpg->GetTotalVP()==hp && gun->GetAmmoQuantity()==ammo && actor->GetPosition()==from && actor->animator.IsAiming()==aimed && inventory->GetActive()==gun.GetPtr() && inventory->GetActiveSlot()==slot && inventory->GetItems().size()==count && world->GetTime()->GetValue()==time && rng.Get()==after.Get() && !actor->HasCommand();
    std::printf("native queued aim: mode=%d kind=%d aimed=%d short=%d prefix=%d path=%d changed=%d first=%d extra=%d direct=%d actual=%d repeat=%d base=%d expected=%d restored=%d after=%d AP=%d ammo=%d unchanged=%d\n",mode,kind,int(aimed),int(shortBurst),prefixCost,path?int(path->points.size()):0,int(!(future.pos.p==from.pos.p)),first,extra,direct,total,repeat,viaBase,expected,int(restored),afterPrice,ap,ammo,int(unchanged));std::fflush(stdout);
    Require(unchanged && restored && afterPrice==direct,"forecasting queued aim changes physical state or fails to restore original position/temporary aim");
    Require(total==expected && repeat==expected && viaBase==expected,"native queued shot keeps the old aimed discount after actual planned movement/turn/pose");
}

class NativeBurstExecutionAccess: public NWorld::CExecShootTile
{
public:
    NativeBurstExecutionAccess(NWorld::CUnitServer* actor,const CVec3& point):CExecShootTile(actor,point){}
    void PayFirstShot() { SpendAP(); } // Expose the actual protected charge; no price/provider overrides.
    int NativeLength() const { return GetShortBurstLength(); }
};

static void BurstPreview(const char* databasePath,const char* resourcePath,int mode)
{
    const bool aimed=mode==1 || mode==7 || mode==12,perkCase=mode==6 || mode==7 || mode==13,execute=mode>=11 && mode<=14;
    const auto shootMode=mode==8?NDb::SM_Snap:mode==9?NDb::SM_Aimed:mode==10?NDb::SM_LongBurst:NDb::SM_ShortBurst;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    NDb::CRPGPers* persona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext()){auto* p=people.Get();if(!p->pClass || !p->pBaseValue || !p->pModel || p->pPanzerklein || p->pDefaultWearsPanzerklein)continue;CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(p->pClass->nPerkTreeID);vector<CPtr<NRPG::CPerk>> entries;tree->GetAllPerks(&entries);for(auto entry:entries)if(entry->GetDBPerk()->GetRecordID()==16 && (!persona || p->GetRecordID()<persona->GetRecordID()))persona=p;}
    Require(persona,"burst fixture lacks a real persona with perk16 in its tree");
    CObj<NRPG::CUnit> unit=new NRPG::CUnit(persona);CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(unit);auto* inventory=unit->GetInventory();
    for(auto slot:{NDb::SLOT_1,NDb::SLOT_2})if(inventory->Get(slot)){CObj<NRPG::IInventoryItem> removed=inventory->TakeOff(slot);}inventory->Activate(NDb::SLOT_1);unit->SetCannonItem(nullptr);
    Require(!mission->HasPerk(16),"burst fixture unexpectedly starts with longer-burst perk");
    if(perkCase){auto* definition=NDb::GetDBPerk(16);Require(definition && !definition->params.empty(),"burst fixture lacks authored perk16");unit->GetPerksTree()->AddPerkPoints(1);unit->GetPerksTree()->TakePerk(16);float actual=-999;Require(mission->HasPerk(16,&actual) && actual==definition->params[0],"burst fixture did not acquire its actual authored perk16");}
    float bonus=0;mission->HasPerk(16,&bonus);vector<NDb::CRPGWeapon*> records;CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while(weapons.MoveNext()){auto* p=weapons.Get();if(p->pItem && p->pWeaponType && p->pWeaponType->nPrepareCost>0 && p->pInnerClip && p->pInnerClip->pItem && !p->bBazookaLogic && p->nShotsInOne<=1 && p->nRoF>=12 && p->shootModes[NDb::SM_ShortBurst] && p->shootModes[NDb::SM_Snap] && p->shootModes[NDb::SM_Aimed] && p->shootModes[NDb::SM_LongBurst])records.push_back(p);}
    std::sort(records.begin(),records.end(),[](const NDb::CRPGWeapon* a,const NDb::CRPGWeapon* b){return a->GetRecordID()<b->GetRecordID();});
    CObj<NRPG::CWeaponItem> gun;NDb::CRPGWeapon* record=nullptr;
    for(auto p:records){CObj<NRPG::CWeaponItem> candidate=new NRPG::CWeaponItem(p);inventory->Equip(NDb::SLOT_1,candidate);candidate->SetShootMode(NDb::SM_ShortBurst);const int length=int(p->nRoF/6.f+bonus),extra=mission->GetActionAP(NAI::WALK,NRPG::AC_BURST),price=mission->GetActionAP(NAI::WALK,NRPG::AC_PREPARE_AND_SHOOT)+Max(0,length-1)*extra;if(length>=2 && extra>0 && price+8<mission->GetAP() && candidate->GetAmmoQuantity()>length){gun=candidate;record=p;break;}}
    Require(gun && record,"burst fixture lacks an affordable actual multi-round authored firearm");
    CObj<NDb::CRPGWeapon> boundary;
    if(mode==16 || mode==17){boundary=new NDb::CRPGWeapon;CMemoryStream fields;{CStructureSaver w(fields,CStructureSaver::WRITE);record->operator&(w);}fields.Seek(0);{CStructureSaver r(fields,CStructureSaver::READ);boundary->operator&(r);}boundary->nRoF=mode==16?3:6;gun=new NRPG::CWeaponItem(boundary);Require(inventory->Equip(NDb::SLOT_1,gun),"burst fixture cannot equip its native copied boundary record");record=boundary;}
    Require(gun->SetShootMode(shootMode) && unit->GetWeaponItem()==gun.GetPtr(),"burst fixture lost its real effective firearm/mode");
    list<CPtr<NWorld::CUnitServer>> actors;world->GetAllUnits(&actors);NWorld::CUnitServer* anchor=nullptr;for(auto actor:actors)if(actor && !actor->IsAIUnit() && actor->CanFight()){anchor=actor;break;}Require(anchor,"burst fixture lacks a real native human owner");auto* owner=dynamic_cast<NWorld::CPlayer*>(anchor->GetPlayer());Require(owner,"burst fixture lacks concrete native player");
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(anchor->GetPosition().GetCP(),5.f),&places);NAI::SPathPlace spawn;bool found=false;for(auto p:places){p.SetPose(NAI::CM_STAND);if(p.IsIntegral() && !p.IsFinal() && network->IsPassable(p) && !network->IsLocked(p,true)){spawn=p;found=true;break;}}Require(found,"burst fixture lacks native free standing spawn");
    CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(spawn,mission,owner);Require(actor && actor->GetUnitRPG()==mission.GetPtr(),"burst fixture cannot spawn its actual authored actor");world->GivePlayerTurn(owner);Require(world->IsTurnBased() && !actor->IsAIUnit() && !actor->HasCommand(),"burst fixture has no real native tactical turn");
    actor->animator.SetActiveItem(true);actor->SetUndrawItem(false);actor->SetWalkWithoutWeapon(false);actor->animator.SetWeaponAnimation(mission->GetWeaponType());actor->animator.SetWeaponName(gun->GetDBWeapon()->szAnimName.c_str());
    if(mode==5 || mode==15){do{vector<NRPG::CAttackPortion> portion;gun->CreateNewAttackPortion(&portion,true);Require(!portion.empty(),"burst fixture cannot spend its actual loaded round");}while(mode==15 && gun->GetAmmoQuantity()>1);Require(gun->GetAmmoQuantity()>0,"burst fixture unexpectedly emptied its authored weapon");}
    if(mode==5 && !gun->CanReload(inventory)){CObj<NRPG::IInventoryItem> spare=NRPG::CreateClipItem(record->pInnerClip,gun->GetInnerClip()->GetDBAmmo());CTPoint<int> cell;Require(spare && inventory->FindPlace(spare,&cell) && inventory->Place(cell,spare),"burst fixture cannot place real compatible reload ammunition");}
    const auto position=actor->GetPosition();const float angle=position.GetDirection();CVec3 point=position.GetCP()+CVec3(20*cos(angle),20*sin(angle),1);Require(NRPG::GetShootDirection(network,position.pos.p,point)==position.pos.p.GetDirection(),"burst fixture cannot select native aligned target");
    if(aimed){CRay ray;ray.ptOrigin=position.GetEyePosition();ray.ptDir=point-ray.ptOrigin;Normalize(&ray.ptDir);actor->animator.Snipe(position,ray);Require(actor->animator.IsAiming(),"burst fixture did not enter real native aiming graph");}else Require(!actor->animator.IsAiming(),"cold burst fixture unexpectedly starts aimed");
    CObj<NativeBurstExecutionAccess> shot=new NativeBurstExecutionAccess(actor,point);Require(shot->CanDoIt(position)==NWorld::UCR_OK,"burst fixture real native shot validation failed");
    const int length=int(record->nRoF/6.f+bonus),nativeLength=shot->NativeLength(),first=actor->GetActionAP(aimed?NRPG::AC_SHOOT:NRPG::AC_PREPARE_AND_SHOOT),burst=actor->GetActionAP(NRPG::AC_BURST),full=first+Max(0,length-1)*burst;
    Require(length==nativeLength && first>0 && burst>0 && (mode<16?length>=2:length==(mode==16?0:1)),"burst fixture native authored length/first/per-round cost is invalid");
    if(execute){
        const int target=mode==14?first+burst-1:full;Require(target<=actor->GetAP(),"burst charge fixture lacks actual AP capacity");actor->SpendAP(actor->GetAP()-target);const int hp=mission->GetTotalVP(),ammo=gun->GetAmmoQuantity();shot->PayFirstShot();const int afterFirst=actor->GetAP();bool valid=afterFirst==target-first;int paid=0;
        if(mode==14){const int before=actor->GetAP();valid=valid && !shot->CheckBurst(1,false) && !shot->CheckBurst(1,true) && actor->GetAP()==before;}
        else{const int before=actor->GetAP();valid=valid && shot->CheckBurst(0,false) && shot->CheckBurst(0,true) && actor->GetAP()==before;for(int fired=1;fired<length;++fired){const int beforeRound=actor->GetAP();valid=valid && shot->CheckBurst(fired,false) && actor->GetAP()==beforeRound && shot->CheckBurst(fired,true) && actor->GetAP()==beforeRound-burst;++paid;}const int end=actor->GetAP();valid=valid && !shot->CheckBurst(length,true) && actor->GetAP()==end && end==0;}
        const bool unchanged=gun->GetAmmoQuantity()==ammo && mission->GetTotalVP()==hp && actor->GetPosition()==position && inventory->GetActive()==gun.GetPtr() && actor->animator.IsAiming()==aimed && !actor->HasCommand();
        std::printf("native burst payment: mode=%d weapon=%d aimed=%d perk=%d length=%d first=%d burst=%d full=%d before=%d afterFirst=%d paid=%d after=%d valid=%d unchanged=%d\n",mode,record->GetRecordID(),int(aimed),int(perkCase),length,first,burst,full,target,afterFirst,paid,actor->GetAP(),int(valid),int(unchanged));std::fflush(stdout);
        Require(valid && unchanged,"native burst first-shot/per-round charging changed or exceeded its actual AP budget");return;
    }
    CObj<NWorld::CCommandExecute> rootQueue=shot.GetPtr();int prefix=0;
    if(mode==4){for(int depth=0;depth<3;++depth){CObj<NWorld::CSimpleExecQueue> queue=new NWorld::CSimpleExecQueue(actor);queue->AddExecutor(rootQueue);rootQueue=queue.GetPtr();}}
    if(mode==5){NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCmdReload> reload=new NWorld::CCmdReload;CObj<NWorld::CCommandExecute> prepare=NWorld::CreateActionExecutor(actor,reload,&result);Require(prepare && result==NWorld::UCR_OK && FindNativeReload(prepare),"burst fixture lacks real native reload prefix");prefix=prepare->GetActionAP();Require(prefix>0,"burst fixture reload prefix is not a genuine priced action");CObj<NWorld::CSimpleExecQueue> queue=new NWorld::CSimpleExecQueue(actor);queue->AddExecutor(prepare);queue->AddExecutor(shot);rootQueue=queue.GetPtr();}
    const int expectedStart=shootMode==NDb::SM_ShortBurst?full:first;
    if(mode==2 || mode==3){const int target=expectedStart-(mode==3?1:0);Require(target<actor->GetAP(),"burst fixture cannot spend to native AP boundary");actor->SpendAP(actor->GetAP()-target);}
    const int ap=actor->GetAP(),hp=mission->GetTotalVP(),ammo=gun->GetAmmoQuantity(),points=unit->GetPerksTree()->GetPerkPoints();const auto count=inventory->GetItems().size();const auto slot=inventory->GetActiveSlot();const auto time=world->GetTime()->GetValue();const float xp=unit->fXP;auto rng=GlobalGameRandom();
    const int expected=prefix+(shootMode==NDb::SM_LongBurst?Max(ap-prefix,first):expectedStart);const int start=shot->GetStartAP(),total=rootQueue->GetActionAP(),repeat=rootQueue->GetActionAP();NWorld::CCommandExecute* base=rootQueue;const int viaBase=base->GetActionAP();
    int previewStart=-1,previewTotal=-1;NWorld::EUnitCommandResult preview=NWorld::UCR_OK;if(mode!=5){CObj<NWorld::CCmdShootTile> command=new NWorld::CCmdShootTile(point);preview=actor->CanDo(command,&previewStart,&previewTotal);}
    auto after=GlobalGameRandom();const bool unchanged=actor->GetAP()==ap && mission->GetTotalVP()==hp && gun->GetAmmoQuantity()==ammo && actor->GetPosition()==position && actor->animator.IsAiming()==aimed && inventory->GetActive()==gun.GetPtr() && inventory->GetActiveSlot()==slot && inventory->GetItems().size()==count && world->GetTime()->GetValue()==time && unit->GetPerksTree()->GetPerkPoints()==points && unit->fXP==xp && rng.Get()==after.Get() && !actor->HasCommand();
    std::printf("native burst preview: mode=%d weapon=%d shootMode=%d aimed=%d perk=%d bonus=%g length=%d ammo=%d first=%d burst=%d prefix=%d AP=%d start=%d expectedStart=%d actual=%d repeat=%d base=%d expected=%d preview=%d previewStart=%d previewTotal=%d unchanged=%d\n",mode,record->GetRecordID(),int(shootMode),int(aimed),int(perkCase),bonus,length,ammo,first,burst,prefix,ap,start,expectedStart,total,repeat,viaBase,expected,int(preview),previewStart,previewTotal,int(unchanged));std::fflush(stdout);
    Require(unchanged,"native burst preview spends AP/ammo or changes health/position/aim/equipment/XP/perks/time/next RNG word");
    Require(start==expectedStart && total==expected && repeat==expected && viaBase==expected,"native short-burst forecast reserves only the first shot instead of the complete original burst");
    if(mode!=5)Require(previewTotal==expected && preview==(actor->CanSpendAP(expected)?NWorld::UCR_OK:NWorld::UCR_NOT_ENOUGH_AP),"public native short-burst preview admits or misprices an incomplete AP budget");
}

static NDb::CRPGPers* PreparationPersona()
{
    NDb::CRPGPers* result=nullptr;
    CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());
    while(people.MoveNext()) {
        auto* p=people.Get();
        if(!p->pClass || !p->pBaseValue || !p->pModel || p->pPanzerklein || p->pDefaultWearsPanzerklein)continue;
        CObj<NRPG::CPerksTree> tree=NRPG::CreatePerksTree(p->pClass->nPerkTreeID);
        vector<CPtr<NRPG::CPerk>> entries;tree->GetAllPerks(&entries);
        for(auto entry:entries)if(entry->GetDBPerk()->GetRecordID()==22 && (!result || p->GetRecordID()<result->GetRecordID()))result=p;
    }
    return result;
}

static void WeaponPreparation(const char* databasePath,const char* resourcePath,int mode)
{
    const bool perkCase=mode==2 || mode==4 || mode==5 || mode==7 || mode==9 || mode==11;
    const bool mounted=mode==3 || mode==4,bounds=mode==6 || mode==7,empty=mode==0 || mode==5,melee=mode==8 || mode==9,spend=mode>=10;
    CObj<NRPG::CGlobalGame> game;CObj<NWorld::CWorld> world;
    if(spend)world=FixtureWorld(databasePath,resourcePath,true,&game);
    else {CFileStream db;db.OpenRead(databasePath);NDatabase::Serialize(db,CStructureSaver::READ);}
    auto* persona=PreparationPersona();auto* definition=NDb::GetDBPerk(22);
    Require(persona && definition && !definition->params.empty() && std::isfinite(definition->params[0]),"preparation fixture lacks real persona/perk22 data");
    CObj<NRPG::CUnit> unit=new NRPG::CUnit(persona);CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(unit);
    auto* inventory=unit->GetInventory();
    for(auto slot:{NDb::SLOT_1,NDb::SLOT_2})if(inventory->Get(slot)){CObj<NRPG::IInventoryItem> removed=inventory->TakeOff(slot);}
    inventory->Activate(NDb::SLOT_1);unit->SetCannonItem(nullptr);
    Require(!unit->GetWeaponItem() && !mission->HasPerk(22),"preparation fixture starts armed or already owns perk22");
    if(perkCase){unit->GetPerksTree()->AddPerkPoints(1);unit->GetPerksTree()->TakePerk(22);float actual=-999;Require(mission->HasPerk(22,&actual) && actual==definition->params[0],"preparation fixture did not acquire its actual authored perk22");}
    const float parameter=perkCase?definition->params[0]:0.f;
    vector<NDb::CRPGWeapon*> records;CDBIterator<NDb::CRPGWeapon> weapons(*NDatabase::GetTable<NDb::CRPGWeapon>());
    while(weapons.MoveNext()){auto* p=weapons.Get();if(p->pItem && p->pWeaponType && p->pInnerClip && p->pInnerClip->pItem)records.push_back(p);}
    Require(!records.empty(),"preparation fixture lacks actual authored firearms");
    std::sort(records.begin(),records.end(),[](const NDb::CRPGWeapon* a,const NDb::CRPGWeapon* b){return a->GetRecordID()<b->GetRecordID();});
    int checked=0,mismatches=0,unchangedRows=0,zeroRows=0,capRows=0,matchingRows=0;
    auto check=[&](NDb::CRPGWeapon* record,int raw,int ordinal){
        auto* effective=unit->GetWeaponItem();auto* active=inventory->GetActive();const int ap=mission->GetAP(),hp=mission->GetTotalVP(),ammo=effective?effective->GetAmmoQuantity():-1,points=unit->GetPerksTree()->GetPerkPoints();
        const auto slot=inventory->GetActiveSlot();const auto count=inventory->GetItems().size();const float xp=unit->fXP;auto rng=GlobalGameRandom();
        const int expected=Min(8,Max(0,int(float(raw)-parameter)));
        for(auto pose:{NAI::WALK,NAI::CROUCH,NAI::CRAWL,NAI::RUN}) {
            const int actual=mission->GetActionAP(pose,NRPG::AC_PREPARE),repeat=mission->GetActionAP(pose,NRPG::AC_PREPARE);
            const int shoot=mission->GetActionAP(pose,NRPG::AC_SHOOT),combined=mission->GetActionAP(pose,NRPG::AC_PREPARE_AND_SHOOT);
            auto after=GlobalGameRandom();auto before=rng;
            const bool unchanged=mission->GetAP()==ap && mission->GetTotalVP()==hp && (!effective || effective->GetAmmoQuantity()==ammo) && unit->GetWeaponItem()==effective && inventory->GetActive()==active && inventory->GetActiveSlot()==slot && inventory->GetItems().size()==count && unit->GetPerksTree()->GetPerkPoints()==points && unit->fXP==xp && before.Get()==after.Get();
            std::printf("native preparation AP: mode=%d row=%d weapon=%d pose=%d mounted=%d perk=%d param=%g raw=%d actual=%d repeat=%d expected=%d shoot=%d combined=%d AP=%d ammo=%d unchanged=%d\n",mode,ordinal,record?record->GetRecordID():-1,int(pose),int(mounted),int(perkCase),parameter,raw,actual,repeat,expected,shoot,combined,ap,ammo,int(unchanged));
            ++checked;unchangedRows+=unchanged;zeroRows+=expected==0;capRows+=expected==8;matchingRows+=actual==expected;
            mismatches+=actual!=expected || repeat!=expected || combined!=expected+shoot;
            Require(unchanged,"preparation price query changes AP/HP/ammo/equipment/XP/perks/inventory or next RNG word");
        }
    };
    if(empty)check(nullptr,0,0);
    else if(melee){NDb::CRPGMeleeWeapon* record=nullptr;CDBIterator<NDb::CRPGMeleeWeapon> all(*NDatabase::GetTable<NDb::CRPGMeleeWeapon>());while(all.MoveNext())if(all.Get()->pItem){record=all.Get();break;}Require(record,"preparation fixture lacks authored melee item");CObj<NRPG::IInventoryItem> item=NRPG::CreateMeleeWeaponItem(record);Require(item && inventory->Equip(NDb::SLOT_1,item) && !unit->GetWeaponItem(),"preparation fixture cannot equip an actual non-firearm");check(nullptr,0,0);}
    else if(bounds){
        CObj<NDb::CRPGWeapon> copy=new NDb::CRPGWeapon;CObj<NDb::CRPGWeaponType> type=new NDb::CRPGWeaponType;
        CMemoryStream weaponFields,typeFields;{CStructureSaver w(weaponFields,CStructureSaver::WRITE);records.front()->operator&(w);}{CStructureSaver w(typeFields,CStructureSaver::WRITE);records.front()->pWeaponType->operator&(w);}
        weaponFields.Seek(0);{CStructureSaver r(weaponFields,CStructureSaver::READ);copy->operator&(r);}typeFields.Seek(0);{CStructureSaver r(typeFields,CStructureSaver::READ);type->operator&(r);}copy->pWeaponType=type;
        int i=0;for(int raw:{-1,0,1,2,7,8,9,20}){type->nPrepareCost=raw;CObj<NRPG::CWeaponItem> gun=new NRPG::CWeaponItem(copy);unit->SetCannonItem(gun);Require(unit->GetWeaponItem()==gun.GetPtr(),"boundary fixture lost its native effective firearm");check(copy,raw,i++);}
    }
    else if(spend){
        NDb::CRPGWeapon* record=nullptr;for(auto p:records)if(p->pWeaponType->nPrepareCost>0 && p->pWeaponType->nPrepareCost!=2){record=p;break;}
        Require(record,"preparation spend fixture lacks an authored firearm distinguishing the old2AP stub");
        CObj<NRPG::CWeaponItem> gun=new NRPG::CWeaponItem(record);Require(inventory->Equip(NDb::SLOT_1,gun),"preparation spend fixture cannot equip its actual firearm");
        list<CPtr<NWorld::CUnitServer>> actors;world->GetAllUnits(&actors);NWorld::CUnitServer* anchor=nullptr;for(auto actor:actors)if(actor && !actor->IsAIUnit() && actor->CanFight()){anchor=actor;break;}Require(anchor,"preparation spend fixture has no native human map anchor");
        auto* owner=dynamic_cast<NWorld::CPlayer*>(anchor->GetPlayer());Require(owner,"preparation spend fixture lacks its actual owner");vector<NAI::SPathPlace> nearby;auto* network=world->GetPathNetwork();network->GetNearPlaces(SSphere(anchor->GetPosition().GetCP(),5.f),&nearby);NAI::SPathPlace spawn;bool found=false;for(auto place:nearby){place.SetPose(NAI::CM_STAND);if(place.IsIntegral() && !place.IsFinal() && network->IsPassable(place) && !network->IsLocked(place,true)){spawn=place;found=true;break;}}Require(found,"preparation spend fixture lacks a native free spawn cell");CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(spawn,mission,owner);Require(actor && actor->GetUnitRPG()==mission.GetPtr(),"preparation spend fixture cannot add its real actor to the world");
        world->GivePlayerTurn(owner);const int raw=record->pWeaponType->nPrepareCost,expected=Min(8,Max(0,int(float(raw)-parameter))),before=actor->GetAP(),ammo=gun->GetAmmoQuantity(),hp=mission->GetTotalVP();const auto position=actor->GetPosition();
        Require(world->IsTurnBased() && before>=8 && !actor->HasCommand(),"preparation spend fixture has no genuine tactical AP budget");
        actor->DoAction(NRPG::AC_PREPARE);const int after=actor->GetAP();
        std::printf("native preparation spend: mode=%d weapon=%d raw=%d param=%g before=%d after=%d expected=%d ammo=%d unchanged=%d\n",mode,record->GetRecordID(),raw,parameter,before,after,before-expected,ammo,int(gun->GetAmmoQuantity()==ammo && mission->GetTotalVP()==hp && actor->GetPosition()==position && !actor->HasCommand()));std::fflush(stdout);
        Require(gun->GetAmmoQuantity()==ammo && mission->GetTotalVP()==hp && actor->GetPosition()==position && !actor->HasCommand(),"preparation spend changes ammo/health/position or installs a command");
        Require(after==before-expected,"native preparation action spends the2AP stub or perk parameter instead of the original weapon cost");return;
    }
    else {
        CObj<NRPG::CWeaponItem> held=new NRPG::CWeaponItem(records.front());if(mounted)Require(inventory->Equip(NDb::SLOT_1,held),"mounted preparation fixture cannot retain its real personal firearm");
        int row=0;for(auto record:records){CObj<NRPG::CWeaponItem> gun=new NRPG::CWeaponItem(record);if(mounted)unit->SetCannonItem(gun);else {if(inventory->Get(NDb::SLOT_1)){CObj<NRPG::IInventoryItem> previous=inventory->TakeOff(NDb::SLOT_1);}Require(inventory->Equip(NDb::SLOT_1,gun),"preparation fixture cannot equip an authored firearm");}Require(unit->GetWeaponItem()==gun.GetPtr() && (!mounted || inventory->GetActive()==held.GetPtr()),"preparation fixture lost effective/held weapon precedence");check(record,record->pWeaponType->nPrepareCost,row++);}
    }
    std::printf("native preparation summary: mode=%d checked=%d mismatches=%d matching=%d zero=%d cap=%d unchanged=%d\n",mode,checked,mismatches,matchingRows,zeroRows,capRows,unchangedRows);std::fflush(stdout);
    Require(checked>0 && checked==unchangedRows,"preparation fixture did not measure native unchanged price queries");
    Require(mismatches==0,"native preparation AP ignores authored weapon type or subtractive perk22/clamp boundaries");
}

static void ReservedAP(const char* databasePath,const char* resourcePath,int mode)
{
    const bool longBurst=mode==2 || mode==3 || mode==5 || mode==10 || mode==13 || mode==15;
    const bool careful=mode==0 || mode==1 || mode==4 || mode==9 || mode==12 || mode==14 || mode==16;
    const bool holstered=mode==12 || mode==13,moving=mode==16 || mode==17,reloadOnly=mode==11;
    const bool reloadPrefix=mode<=8 || reloadOnly,directOnly=mode==9 || mode==10 || mode==14 || mode==15,low=mode==4 || mode==5 || mode==14 || mode==15;
    const auto shootMode=longBurst?NDb::SM_LongBurst:careful?NDb::SM_Careful:mode==7?NDb::SM_Aimed:mode==8?NDb::SM_ShortBurst:NDb::SM_Snap;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;CPtr<NRPG::CWeaponItem> weapon;NWorld::CPlayer* owner=nullptr;for(auto unit:units)if(unit && !unit->IsAIUnit() && unit->CanFight()){owner=dynamic_cast<NWorld::CPlayer*>(unit->GetPlayer());break;}Require(owner,"reserved AP fixture has no actual human owner");
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && unit->GetUnitRPG()->CanMove() && !unit->HasCommand() && !unit->IsPerformingAction()) {auto* gun=dynamic_cast<NRPG::CWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());if(gun && gun->GetAmmoQuantity()>1 && gun->IsShootModeSupported(shootMode)){actor=unit;weapon=gun;break;}}
    if(!actor){for(auto unit:units)if(unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK()){actor=unit;break;}Require(actor,"reserved AP fixture has no native human equipment owner");NDb::CRPGWeapon* selected=nullptr;int supported=0;CDBIterator<NDb::CRPGWeapon> records(*NDatabase::GetTable<NDb::CRPGWeapon>());while(records.MoveNext()){auto* record=records.Get();if(!record->shootModes[shootMode] || record->bBazookaLogic || !record->pItem || !record->pInnerClip || !record->pInnerClip->pItem || record->nReloadAP<=0 || (record->nInnerClipAmmoQuantity>0?record->nInnerClipAmmoQuantity:record->pInnerClip->nQuantity)<2)continue;++supported;const int score=record->nReloadAP+record->nShotAP+record->nTargetingAP;const int prior=selected?selected->nReloadAP+selected->nShotAP+selected->nTargetingAP:0;if(!selected || score<prior || (score==prior && record->GetRecordID()<selected->GetRecordID()))selected=record;}std::printf("native reserved AP weapon search: mode=%d supported=%d selected=%d\n",mode,supported,selected?selected->GetRecordID():-1);std::fflush(stdout);Require(selected,"reserved AP fixture has no authored database weapon for this shooting mode");auto* inventory=actor->GetUnitRPG()->GetInventory();const auto slot=NDb::ESlot(inventory->GetActiveSlot());CObj<NRPG::IInventoryItem> old=inventory->Get(slot)?inventory->TakeOff(slot):nullptr;CObj<NRPG::IWeaponItem> created=NRPG::CreateWeaponItem(selected);weapon=dynamic_cast<NRPG::CWeaponItem*>(created.GetPtr());Require(weapon && inventory->Equip(slot,weapon) && inventory->Activate(slot),"reserved AP fixture cannot equip its genuine authored weapon");}
    Require(actor && weapon,"reserved AP fixture lacks a real loaded human firearm user supporting this authored mode");
    if(actor->GetPlayer()!=owner)world->ChangeUnitPlayer(actor,owner);Require(!actor->IsAIUnit() && actor->GetPlayer()==owner,"reserved AP fixture did not retain real native human ownership");world->GivePlayerTurn(owner);Require(world->IsTurnBased() && world->GetCurrentPlayer()==owner,"reserved AP fixture did not enter its native tactical turn");
    if(reloadPrefix){const int loaded=weapon->GetAmmoQuantity();vector<NRPG::CAttackPortion> portion;weapon->CreateNewAttackPortion(&portion,true);if(!weapon->CanReload(actor->GetUnitRPG()->GetInventory())){CObj<NRPG::IInventoryItem> spare=NRPG::CreateClipItem(weapon->GetDBWeapon()->pInnerClip,weapon->GetInnerClip()->GetDBAmmo());CTPoint<int> cell;Require(spare && actor->GetUnitRPG()->GetInventory()->FindPlace(spare,&cell) && actor->GetUnitRPG()->GetInventory()->Place(cell,spare),"reserved AP fixture cannot place an authored compatible spare clip");}Require(!portion.empty() && weapon->GetAmmoQuantity()>0 && weapon->GetAmmoQuantity()<loaded && weapon->CanReload(actor->GetUnitRPG()->GetInventory()),"reserved AP fixture needs a genuinely partially spent weapon with a compatible native reload");}
    Require(weapon->SetShootMode(shootMode),"reserved AP fixture cannot select its authored shooting mode");actor->animator.SetActiveItem(!holstered);actor->SetUndrawItem(false);actor->SetWalkWithoutWeapon(false);
    {auto at=actor->GetPosition();vector<NAI::SPathPlace> places;auto* net=world->GetPathNetwork();net->GetNearPlaces(SSphere(at.GetCP(),5.f),&places);bool placed=false;for(auto place:places){if(!place.IsIntegral() || place.IsFinal())continue;place.SetPose(NAI::CM_STAND);place.SetDirection(at.pos.p.GetDirection());if(!net->IsPassable(place) || net->IsLocked(place,true))continue;at.pos.p=place;actor->SetPosition(at);actor->animator.PlaceUnit(at);actor->Update();placed=true;break;}Require(placed,"reserved AP fixture has no actual unlocked integral standing cell");}
    const auto from=actor->GetPosition();CVec3 point=VNULL3;bool aligned=false;for(int dir=0;dir<8;++dir){const float angle=dir*FP_PI*.25f;const auto to=from.GetCP()+CVec3(20*cos(angle),20*sin(angle),1);if(NRPG::GetShootDirection(world->GetPathNetwork(),from.pos.p,to)==from.pos.p.GetDirection()){point=to;aligned=true;break;}}Require(aligned,"reserved AP fixture cannot select a native aligned tile shot");
    CObj<NWorld::CExecShootTile> shot=new NWorld::CExecShootTile(actor,point);Require(shot->CanDoIt(from)==NWorld::UCR_OK,"reserved AP fixture's actual shot validator rejects its loaded native actor");
    CObj<NWorld::CCommandExecute> prefix,rootQueue;CObj<NAI::CPath> prefixPath;NWorld::EUnitCommandResult preparation=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCmdShootTile> command=new NWorld::CCmdShootTile(point);
    if(reloadPrefix){CObj<NWorld::CCmdReload> reload=new NWorld::CCmdReload;prefix=NWorld::CreateActionExecutor(actor,reload,&preparation);Require(prefix && preparation==NWorld::UCR_OK && FindNativeReload(prefix),"reserved AP fixture did not build its real native reload preparation queue");}
    else if(moving){auto* net=world->GetPathNetwork();vector<NAI::SPathPlace> destinations;net->GetNearPlaces(SSphere(from.GetCP(),4.f),&destinations);for(auto end:destinations){if(!end.IsIntegral() || end.IsFinal())continue;end.SetPose(from.pos.p.GetPose());end.SetDirection(from.pos.p.GetDirection());auto at=from;at.pos.p=end;if(fabs(at.GetCP()-from.GetCP())<.6f || !net->IsPassable(end) || net->IsLocked(end,true))continue;CObj<NAI::CPath> candidate=NWorld::FindPath(net,actor,from.pos.p,vector<NAI::SPathPlace>{end},nullptr,false,NAI::PF_USE_POSEDIR);if(!candidate || candidate->points.size()<2 || !candidate->actions.empty())continue;preparation=NWorld::UCR_OK;prefix=NWorld::CreateMoveExecutor(actor,candidate,NAI::PF_USE_POSEDIR,NWorld::ITEM_NO_MATTER,&preparation);if(prefix && preparation==NWorld::UCR_OK){prefixPath=prefix->GetCurrentPath();if(prefixPath && prefixPath->points.size()>1)break;std::printf("native reserved AP move probe: mode=%d candidate=%u from=%u,%u,%u end=%u,%u,%u cost=%d\n",mode,unsigned(candidate->points.size()),unsigned(from.pos.p.GetX()),unsigned(from.pos.p.GetY()),unsigned(from.pos.p.GetLayer()),unsigned(end.GetX()),unsigned(end.GetY()),unsigned(end.GetLayer()),prefix->GetActionAP());std::fflush(stdout);prefix=nullptr;}}Require(prefix && prefixPath && !prefixPath->points.empty(),"reserved AP fixture did not build a genuine native movement preparation");}
    else if(holstered){rootQueue=NWorld::CreateActionExecutor(actor,command,&preparation);auto* queue=dynamic_cast<NWorld::CSimpleExecQueue*>(rootQueue.GetPtr());auto* front=queue?queue->GetFrontExecutor():nullptr;Require(rootQueue && preparation==NWorld::UCR_OK && front && dynamic_cast<NWorld::IExecMove*>(front),"reserved AP fixture did not get its real holstered-shot movement preparation");prefix=front;}
    const int prefixCost=prefix?prefix->GetActionAP():0;Require(!reloadPrefix && !moving || prefixCost>0,"reserved AP fixture has no positive native preceding-action cost");
    auto future=from;if(prefixPath)future.pos.p=prefixPath->points.back();actor->SetTemporaryPosition(future);const int start=shot->GetStartAP();actor->SetTemporaryPosition(from);
    if(low){const int desired=Max(0,(directOnly?start:prefixCost+start)-1);Require(desired<actor->GetAP(),"reserved AP fixture cannot reach its native start-cost floor with remaining AP");actor->SpendAP(actor->GetAP()-desired);Require(actor->GetAP()==desired,"reserved AP fixture could not spend to its actual tactical AP boundary");}
    if(!holstered){CObj<NWorld::CSimpleExecQueue> queue=new NWorld::CSimpleExecQueue(actor);if(prefix)queue->AddExecutor(prefix);if(!reloadOnly){CObj<NWorld::CCommandExecute> tail=shot.GetPtr();for(int depth=0;depth<(mode==1 || mode==3?2:moving?1:0);++depth){CObj<NWorld::CSimpleExecQueue> nested=new NWorld::CSimpleExecQueue(actor);nested->AddExecutor(tail);tail=nested.GetPtr();}queue->AddExecutor(tail);}rootQueue=queue.GetPtr();}
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),ammo=weapon->GetAmmoQuantity();const auto count=actor->GetUnitRPG()->GetInventory()->GetItems().size();const auto activeSlot=actor->GetUnitRPG()->GetInventory()->GetActiveSlot();const auto time=world->GetTime()->GetValue();const bool aiming=actor->animator.IsAiming();auto random=GlobalGameRandom();
    const int expected=reloadOnly?prefixCost:prefixCost+(careful || longBurst?Max(ap-prefixCost,start):start);const int total=rootQueue->GetActionAP(),again=rootQueue->GetActionAP();NWorld::CCommandExecute* polymorphic=rootQueue.GetPtr();const int viaBase=polymorphic->GetActionAP();
    int previewStart=-1,previewTotal=-1;auto preview=NWorld::UCR_OK;if(holstered)preview=actor->CanDo(command,&previewStart,&previewTotal);
    auto afterRandom=GlobalGameRandom();const bool unchanged=actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && weapon->GetAmmoQuantity()==ammo && weapon->GetShootMode()==shootMode && actor->GetPosition()==from && actor->animator.IsActiveItem()==!holstered && actor->animator.IsAiming()==aiming && !actor->GetUndrawItem() && !actor->IsWalkingWithoutWeapon() && actor->GetUnitRPG()->CanMove() && !actor->HasCommand() && actor->GetUnitRPG()->GetInventory()->GetActive()==weapon.GetPtr() && actor->GetUnitRPG()->GetInventory()->GetActiveSlot()==activeSlot && actor->GetUnitRPG()->GetInventory()->GetItems().size()==count && world->GetTime()->GetValue()==time && random.Get()==afterRandom.Get();
    std::printf("native reserved AP: mode=%d shootMode=%d reload=%d moving=%d holstered=%d direct=%d low=%d prefix=%d start=%d AP=%d actual=%d repeat=%d base=%d expected=%d affordable=%d preview=%d previewStart=%d previewTotal=%d ammo=%d unchanged=%d\n",mode,int(shootMode),int(reloadPrefix),int(moving),int(holstered),int(directOnly),int(low),prefixCost,start,ap,total,again,viaBase,expected,int(actor->CanSpendAP(expected)),int(preview),previewStart,previewTotal,ammo,int(unchanged));std::fflush(stdout);
    Require(unchanged,"querying native queued AP changed actor ammunition/HP/AP/pose/wishes/inventory/time or next RNG word");
    Require(total==expected && again==expected && viaBase==expected,"native action queue counts AP already reserved by earlier actions again");
    if(holstered)Require(previewTotal==expected && preview==(actor->CanSpendAP(expected)?NWorld::UCR_OK:NWorld::UCR_NOT_ENOUGH_AP),"native command preview rejects or misprices its real holstered shot despite the remaining budget");
}

static vector<NAI::SPathPlace> NativeRetryCandidates(NWorld::CUnitServer* actor,const CVec3& point,float radius,bool standing)
{
    auto* network=actor->GetWorld()->GetPathNetwork();vector<NAI::SPathPlace> nearby,result;network->GetNearPlaces(SSphere(point,5.f),&nearby);auto from=actor->GetPosition();
    for(const auto& cell:nearby){if(!network->IsNativePassable(cell))continue;from.pos.p=cell;if(!NWorld::IsWithinHumanReach(from.GetCP(),point,radius))continue;const auto direction=NRPG::GetShootDirection(network,cell,point);for(int pose=standing?int(NAI::CM_STAND):int(NAI::CM_LAY);pose<=NAI::CM_STAND;++pose){NAI::SPathPlace destination(cell.GetX(),cell.GetY(),cell.GetLayer(),direction,pose,false);destination.SetFinal(cell.IsFinal());result.push_back(destination);}}
    return result;
}

static void ApproachRetry(const char* databasePath,const char* resourcePath,int mode)
{
    const int kind=mode<6?mode%3:0;const bool talk=kind==1,transfer=kind==2,motionless=mode>=3 && mode<6,expected=mode<3 || mode==7;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer *actor=nullptr,*target=nullptr;CPtr<NRPG::CWeaponItem> held;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && unit->GetUnitRPG()->CanMove() && !unit->HasCommand() && !unit->IsPerformingAction()){auto* gun=dynamic_cast<NRPG::CWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());if(!unit->IsAIUnit() && unit->GetUnitRPG()->GetRPGUnit()->IsHero() && gun && !actor){actor=unit;held=gun;}else if(unit->IsAIUnit() && !target)target=unit;}
    Require(actor && held && target,"retry fixture needs actual healthy hero/ordinary NPC");auto* owner=dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer());Require(owner,"retry fixture has no human owner");world->GivePlayerTurn(owner);
    CObj<NWorld::CUnitServer> suit;
    if(talk || transfer){NDb::CRPGPers* persona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());while(people.MoveNext()){auto* p=people.Get();if(p->pPanzerklein && !p->pPanzerklein->bHasNoHead && p->pModel && (!persona || p->GetRecordID()<persona->GetRecordID()))persona=p;}Require(persona,"retry talk fixture has no authored PK chassis");auto* net=world->GetPathNetwork();vector<NAI::SPathPlace> places;net->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),20.f),&places);bool deployed=false;for(auto place:places){if(place.GetX()<5 || place.GetY()<5 || !place.IsIntegral() || place.IsFinal())continue;place.SetPose(NAI::CM_STAND);place.SetDirection(0);if(!NAI::IsBigLockerPlacePassable(net,place) || net->IsLocked(place,false))continue;suit=world->AddUnitInGame(place,NRPG::CreateUnit(persona),actor->GetTBSPlayer());if(suit && suit->IsEmptyPK() && suit->WearAsPK(true)){actor->SetPosition(suit->GetPosition());actor->FlipPanzerklein(suit,false,false);deployed=true;break;}}Require(deployed && actor->IsWearingPK() && actor->GetWearingPK()==suit.GetPtr() && actor->GetUnitRPG()->GetRPGUnit()->IsHero(),"retry talk fixture did not establish actual hero/PK ownership");}
    int doors=0;for(const auto& entry:world->GetObjectsForHarness())if(auto* door=dynamic_cast<NWorld::CWindowDoor*>(entry.GetPtr())){if(door->IsBroken())continue;door->OpenClose(false,true);door->LockDoor(true,0,1);++doors;}world->GetAIMap()->Sync();
    auto* network=world->GetPathNetwork();const auto from=actor->GetPosition();vector<NAI::SPathPlace> nearby;network->GetNearPlaces(SSphere(from.GetCP(),25.f),&nearby);std::set<unsigned> seen;unsigned maxX=0,maxY=0;for(const auto& cell:nearby){maxX=Max(maxX,unsigned(cell.GetX()));maxY=Max(maxY,unsigned(cell.GetY()));}
    std::printf("native retry fixture doors=%d max=%u,%u\n",doors,maxX,maxY);std::fflush(stdout);
    int inspected=0,innerEmpty=0,innerBlocked=0,outerPaths=0;bool found=mode>=6;CVec3 point=mode==6?from.GetCP()+CVec3(0,0,100):mode==7?from.GetCP()+CVec3(.25f,0,.25f):VNULL3;vector<NAI::SPathPlace> initial,wide;CObj<NAI::CPath> innerPath,outerPath;
    for(auto place:nearby){if(mode>=6)break;place.SetPose(NAI::CM_STAND);place.SetDirection(0);place.SetMoving(false);if(!seen.insert(place.GetBits()).second || place.GetX()<2 || place.GetY()<2 || unsigned(place.GetX())+2>=maxX || unsigned(place.GetY())+2>=maxY)continue;auto to=from;to.pos.p=place;CVec3 proposed=to.GetCP()+(talk?VNULL3:CVec3(.3125f,.3125f,0));if(fabs(proposed-from.GetCP())<1.3f)continue;
        if(talk || transfer){if(!network->IsPassable(place) || network->IsLocked(place,true))continue;to=target->GetPosition();to.pos.p=place;target->SetPosition(to);target->animator.PlaceUnit(to);target->Update();proposed=talk?target->GetPosition().GetEyePosition():target->GetPosition().GetCP();}
        ++inspected;initial=NativeRetryCandidates(actor,proposed,talk?.8f:transfer?.725f:.625f,talk);wide=NativeRetryCandidates(actor,proposed,transfer?1.7f:1.6f,talk);if(wide.empty())continue;
        innerPath=nullptr;if(!initial.empty())innerPath=NWorld::FindPath(network,actor,from.pos.p,initial,nullptr,false,NAI::PF_USE_POSEDIR,actor->IsStrafing());
        if(initial.empty())++innerEmpty;else if(!innerPath)++innerBlocked;else continue;
        outerPath=NWorld::FindPath(network,actor,from.pos.p,wide,nullptr,false,NAI::PF_USE_POSEDIR,actor->IsStrafing());if(!outerPath)continue;++outerPaths;point=proposed;found=true;break;
    }
    if(mode>=6){initial=NativeRetryCandidates(actor,point,.625f,false);wide=NativeRetryCandidates(actor,point,1.6f,false);}
    std::printf("native approach retry scene: mode=%d kind=%d cells=%u inspected=%d empty=%d blocked=%d outer=%d found=%d actor=%u,%u,%u point=%.6f,%.6f,%.6f initial=%u wide=%u\n",mode,kind,unsigned(nearby.size()),inspected,innerEmpty,innerBlocked,outerPaths,int(found),unsigned(from.pos.p.GetX()),unsigned(from.pos.p.GetY()),unsigned(from.pos.p.GetLayer()),double(point.x),double(point.y),double(point.z),unsigned(initial.size()),unsigned(wide.size()));std::fflush(stdout);
    Require(found && (mode>=6 || (!innerPath && outerPath)),"retry fixture did not establish an actual geometry/path failure recoverable by original widened reach");
    CObj<NWorld::CCmd> command;CPtr<NWorld::CDFrozenItem> body;CObj<NRPG::IInventoryItem> item;
    if(talk){target->SetCanTalk(true);command=new NWorld::CCmdTalk(target);}else if(transfer){item=NRPG::CreateClipItem(held->GetDBWeapon()->pInnerClip);CTPoint<int> origin,destination;auto* inventory=actor->GetUnitRPG()->GetInventory();Require(item && inventory->FindPlace(item,&origin) && inventory->Place(origin,item) && target->GetUnitRPG()->GetInventory()->FindPlace(item,&destination),"retry transfer fixture lacks real source/destination backpack cells");command=new NWorld::CCmdMoveInventoryItem(NWorld::SItem(actor,NWorld::SItem::BACKPACK,item),NWorld::SItem(target,NWorld::SItem::BACKPACK,destination));}else{item=NRPG::CreateClipItem(held->GetDBWeapon()->pInnerClip);CTPoint<int> destination;Require(item && actor->GetUnitRPG()->GetInventory()->FindPlace(item,&destination),"retry fixture has no actual clip/backpack place");body=world->AddFrozenItem(world->GetAIMap(),point,QNULL,item);Require(body && body->GetInvItem()==item.GetPtr() && body->GetPos()==point,"retry fixture cannot register its genuine world item at measured point");NWorld::SItem source(nullptr,NWorld::SItem::GROUND,item);source.pWorldItem=body;command=new NWorld::CCmdMoveInventoryItem(source,NWorld::SItem(actor,NWorld::SItem::BACKPACK,destination));}
    actor->animator.SetActiveItem(true);actor->SetUndrawItem(false);actor->SetWalkWithoutWeapon(false);
    auto* rpg=actor->GetUnitRPG();struct ReleaseMotionless{NRPG::IUnitMission* unit=nullptr;~ReleaseMotionless(){if(unit)unit->ReleaseMotionless();}} release;if(motionless){rpg->ApplyMotionless();release.unit=rpg;}Require(rpg->CanMove()!=motionless,"retry fixture has the wrong actual motionless counter");
    auto* inventory=rpg->GetInventory();const auto targetPosition=target->GetPosition();const int ap=actor->GetAP(),hp=rpg->GetTotalVP(),ammo=held->GetAmmoQuantity(),targetAP=target->GetAP(),targetHP=target->GetUnitRPG()->GetTotalVP();const auto count=inventory->GetItems().size();const auto targetCount=target->GetUnitRPG()->GetInventory()->GetItems().size();const auto activeSlot=inventory->GetActiveSlot();CPtr<NRPG::IInventoryItem> activeItem=inventory->GetActive();auto* clip=item?dynamic_cast<NRPG::IClipItem*>(item.GetPtr()):nullptr;const int quantity=item?item->GetQuantity():-1,rounds=clip?clip->GetIncQuantity():-1;const auto time=world->GetTime()->GetValue();const auto hints=game->hintsSet.size();
    CObj<NWorld::CCommandExecute> action;if(talk)action=new NWorld::CExecTalk(actor,target);else action=new NWorld::CExecMoveInventoryItem(actor,dynamic_cast<NWorld::CCmdMoveInventoryItem*>(command.GetPtr()));const auto direct=talk?dynamic_cast<NWorld::CExecTalk*>(action.GetPtr())->CanDoIt(from):dynamic_cast<NWorld::CExecMoveInventoryItem*>(action.GetPtr())->CanDoIt(from);Require(direct==NWorld::UCR_OK,"retry fixture's real action validator rejects its actor/target/item independently of movement");const int price=action->GetStartAP();auto random=GlobalGameRandom();
    NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCommandExecute> executor=NWorld::CreateActionExecutor(actor,command,&result);auto afterRandom=GlobalGameRandom();auto* queue=dynamic_cast<NWorld::CSimpleExecQueue*>(executor.GetPtr());auto* front=queue?queue->GetFrontExecutor():nullptr;auto* mover=dynamic_cast<NWorld::IExecMove*>(front);CObj<NAI::CPath> path=executor?executor->GetCurrentPath():nullptr;int desiredPose=-1;float reach=-1;bool destinationPassable=false;
    if(mover){NAI::SPathPlace desired;NAI::EFindPathParams parameters;NWorld::ENeedActiveItem active;mover->GetDesiredPlace(&desired,&parameters,&active);auto end=from;end.pos.p=desired;desiredPose=desired.GetPose();reach=fabs(end.GetCPNoHeight()-CVec2(point.x,point.y));CObj<NAI::CPath> checked=NWorld::FindPath(network,actor,from.pos.p,vector<NAI::SPathPlace>{desired},nullptr,false,NAI::PF_USE_POSEDIR,actor->IsStrafing());destinationPassable=network->IsNativePassable(desired) && checked && !checked->points.empty() && NAI::IsSamePlace(checked->points.back(),desired,0xf9feffff);}
    std::set<CObjectBase*> actions;if(executor){CStructureNetworkContext graph;graph.include=[&](CObjectBase* object){if(talk?dynamic_cast<NWorld::CExecTalk*>(object)!=nullptr:dynamic_cast<NWorld::CExecMoveInventoryItem*>(object)!=nullptr){actions.insert(object);return false;}return dynamic_cast<NWorld::CCommandExecute*>(object)!=nullptr;};CMemoryStream traversal;CStructureSaver writer(traversal,CStructureSaver::WRITE,&graph);static_cast<CObjectBase*>(executor.GetPtr())->operator&(writer);}
    const bool unchanged=actor->GetPosition()==from && target->GetPosition()==targetPosition && actor->GetAP()==ap && rpg->GetTotalVP()==hp && target->GetAP()==targetAP && target->GetUnitRPG()->GetTotalVP()==targetHP && held->GetAmmoQuantity()==ammo && inventory->GetItems().size()==count && target->GetUnitRPG()->GetInventory()->GetItems().size()==targetCount && inventory->GetActive()==activeItem.GetPtr() && inventory->GetActiveSlot()==activeSlot && actor->animator.IsActiveItem() && !actor->GetUndrawItem() && !actor->IsWalkingWithoutWeapon() && rpg->CanMove()!=motionless && !actor->HasCommand() && !target->HasCommand() && world->GetTime()->GetValue()==time && game->hintsSet.size()==hints && (!item || item->GetQuantity()==quantity) && (!clip || clip->GetIncQuantity()==rounds) && (!body || (body->GetPos()==point && body->GetInvItem()==item.GetPtr() && world->GetFrozenItem(item)==body.GetPtr())) && (!(talk || transfer) || (actor->IsWearingPK() && actor->GetWearingPK()==suit.GetPtr() && suit->IsEmptyPK())) && random.Get()==afterRandom.Get();
    std::printf("native approach retry: mode=%d kind=%d motionless=%d PK=%d initial=%u wide=%u direct=%d result=%d executor=%d move=%d path=%d desiredPose=%d reach=%.6f passable=%d actions=%u expected=%d price=%d AP=%d ammo=%d rounds=%d unchanged=%d\n",mode,kind,int(motionless),int(actor->IsWearingPK()),unsigned(initial.size()),unsigned(wide.size()),int(direct),int(result),int(bool(executor)),int(bool(mover)),int(bool(path)),desiredPose,double(reach),int(destinationPassable),unsigned(actions.size()),int(expected),price,ap,ammo,rounds,int(unchanged));std::fflush(stdout);
    Require(unchanged,"retry planning changed actual actors/items/PK ownership/AP/HP/inventory/hints/time or next RNG word");
    Require(bool(executor)==expected && (!expected || result==NWorld::UCR_OK),"actual movement preparation failed without the original wider retry, or bypassed mobility/geometry controls");
    if(!expected)return;
    Require(actions.size()==1 && (mode==7?(!mover && !path):(mover && path && destinationPassable && reach<=(transfer?1.7f:1.6f)+.00001f && (!talk || desiredPose==NAI::CM_STAND))),"wider retry omitted/duplicated its real queued action, used an invalid endpoint or changed a ready action control");
}

static void PickupRadii(const char* databasePath,const char* resourcePath,int mode)
{
    const bool transfer=mode>=8,hint=mode==0 || mode==1 || mode==5 || mode==6 || mode==7 || mode==8 || mode==10,dummy=mode==2 || mode==9;
    const bool motionless=mode!=1 && mode!=3 && mode!=4,expected=mode!=2 && mode!=5 && mode!=9 && mode!=10,preparation=mode==3;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer *actor=nullptr,*recipient=nullptr;CPtr<NRPG::CWeaponItem> held;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && unit->GetUnitRPG()->CanMove() && !unit->HasCommand() && !unit->IsPerformingAction()) {
        auto* gun=dynamic_cast<NRPG::CWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        if(!unit->IsAIUnit() && gun && IsValid(gun->GetDBWeapon()->pInnerClip) && !actor){actor=unit;held=gun;}
        else if(unit->IsAIUnit() && !recipient)recipient=unit;
    }
    Require(actor && held && (!transfer || recipient),"pickup radii has no real human gun user/ordinary recipient");
    auto* owner=dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer());Require(owner,"pickup radii has no human owner");world->GivePlayerTurn(owner);Require(world->IsTurnBased() && world->GetCurrentPlayer()==owner,"pickup radii did not enter its native tactical turn");
    auto* network=world->GetPathNetwork();auto pose=actor->GetPosition();pose.pos.p.SetPose(mode==6?NAI::CM_CROUCH:mode==7?NAI::CM_LAY:NAI::CM_STAND);Require(network->IsNativePassable(pose.pos.p),"pickup radii has no native passable actor pose");actor->SetPosition(pose);actor->animator.PlaceUnit(pose);actor->Update();
    CObj<NRPG::IInventoryItem> marker=NRPG::CreateHintItem();Require(marker && dynamic_cast<NRPG::IHintItem*>(marker.GetPtr()),"pickup radii did not create the real authored hint marker");
    CObj<NRPG::IInventoryItem> item=hint?marker.GetPtr():dummy?NRPG::CreateDummyItem(marker->GetDBItem()):NRPG::CreateClipItem(held->GetDBWeapon()->pInnerClip);
    auto* clip=dynamic_cast<NRPG::IClipItem*>(item.GetPtr());Require(item && bool(dynamic_cast<NRPG::IHintItem*>(item.GetPtr()))==hint && (!dummy || item->GetDBItem()==marker->GetDBItem()),"pickup radii lost its real type-vs-same-DB marker control");
    NWorld::SItem source,target;CPtr<NWorld::CDFrozenItem> body;CVec3 point;bool placed=false;
    auto* inventory=actor->GetUnitRPG()->GetInventory();auto* destinationInventory=transfer?recipient->GetUnitRPG()->GetInventory():inventory;CTPoint<int> destination;
    Require(destinationInventory->FindPlace(item,&destination),"pickup radii has no actual destination backpack space");
    if(transfer) {
        const int dx=mode==11?1:2,dy=mode==10?2:mode==11?0:1;const int offsets[4][2]={{dx,dy},{-dy,dx},{-dx,-dy},{dy,-dx}};
        for(const auto& offset:offsets){auto to=recipient->GetPosition();to.pos.p=pose.pos.p;to.pos.p.SetPose(NAI::CM_STAND);to.pos.p.SetXY(pose.pos.p.GetX()+offset[0],pose.pos.p.GetY()+offset[1]);if(!network->IsPassable(to.pos.p) || network->IsLocked(to.pos.p,true))continue;recipient->SetPosition(to);recipient->animator.PlaceUnit(to);recipient->Update();placed=true;break;}
        Require(placed,"pickup radii cannot place its real recipient on an unlocked native cell");point=recipient->GetPosition().GetCP();CTPoint<int> origin;Require(inventory->FindPlace(item,&origin) && inventory->Place(origin,item),"pickup radii cannot place its source item in the actor backpack");source=NWorld::SItem(actor,NWorld::SItem::BACKPACK,item);target=NWorld::SItem(recipient,NWorld::SItem::BACKPACK,destination);
    } else {
        const float distance=mode==3?1.35f:mode==4?1.f:mode==5?1.32f:1.28f;
        for(int dir:{0,2,4,6}){auto one=pose,two=pose;const int dx=NAI::nMoveShift[dir][0],dy=NAI::nMoveShift[dir][1];one.pos.p.SetXY(pose.pos.p.GetX()+dx,pose.pos.p.GetY()+dy);two.pos.p.SetXY(pose.pos.p.GetX()+2*dx,pose.pos.p.GetY()+2*dy);one.pos.p.SetPose(NAI::CM_STAND);two.pos.p.SetPose(NAI::CM_STAND);if(!network->IsPassable(one.pos.p) || network->IsLocked(one.pos.p,true) || !network->IsPassable(two.pos.p) || network->IsLocked(two.pos.p,true))continue;CVec3 axis=two.GetCP()-pose.GetCP();axis.z=0;Normalize(&axis);point=pose.GetCP()+axis*distance;placed=true;break;}
        Require(placed,"pickup radii has no genuine clear native two-cell approach control");body=world->AddFrozenItem(world->GetAIMap(),point,QNULL,item);Require(body && body->GetInvItem()==item.GetPtr() && world->GetFrozenItem(item)==body.GetPtr(),"pickup radii cannot register its actual world body");point=body->GetPos();source=NWorld::SItem(nullptr,NWorld::SItem::GROUND,item);source.pWorldItem=body.GetPtr();target=NWorld::SItem(actor,NWorld::SItem::BACKPACK,destination);
    }
    pose=actor->GetPosition();pose.pos.p.SetDirection(NRPG::GetShootDirection(network,pose.pos.p,point));actor->SetPosition(pose);actor->animator.PlaceUnit(pose);actor->Update();actor->animator.SetActiveItem(true);actor->SetUndrawItem(false);actor->SetWalkWithoutWeapon(false);
    auto* rpg=actor->GetUnitRPG();struct ReleaseMotionless{NRPG::IUnitMission* unit=nullptr;~ReleaseMotionless(){if(unit)unit->ReleaseMotionless();}} release;if(motionless){rpg->ApplyMotionless();release.unit=rpg;}Require(rpg->CanMove()!=motionless,"pickup radii has the wrong actual motionless counter");
    const auto from=actor->GetPosition();const auto recipientPosition=transfer?recipient->GetPosition():from;const int ap=actor->GetAP(),hp=rpg->GetTotalVP(),ammo=held->GetAmmoQuantity(),quantity=item->GetQuantity(),rounds=clip?clip->GetIncQuantity():-1;const int targetAP=transfer?recipient->GetAP():ap,targetHP=transfer?recipient->GetUnitRPG()->GetTotalVP():hp;const auto count=inventory->GetItems().size();const auto destinationCount=destinationInventory->GetItems().size();const auto slot=inventory->GetActiveSlot();const auto hintCount=game->hintsSet.size();const bool exhausted=game->bNoMoreHints;const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    CObj<NWorld::CCmdMoveInventoryItem> command=new NWorld::CCmdMoveInventoryItem(source,target);CObj<NWorld::CExecMoveInventoryItem> action=new NWorld::CExecMoveInventoryItem(actor,command);const auto direct=action->CanDoIt(from);Require(direct==NWorld::UCR_OK,"pickup radii native source/destination validation failed before planning");const int price=action->GetStartAP();
    NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCommandExecute> executor=NWorld::CreateActionExecutor(actor,command,&result);auto* queue=dynamic_cast<NWorld::CSimpleExecQueue*>(executor.GetPtr());auto* front=queue?queue->GetFrontExecutor():nullptr;auto* mover=dynamic_cast<NWorld::IExecMove*>(front);CObj<NAI::CPath> path=executor?executor->GetCurrentPath():nullptr;
    float reach=-1;int desiredPose=-1;if(mover){NAI::SPathPlace desired;NAI::EFindPathParams parameters;NWorld::ENeedActiveItem active;mover->GetDesiredPlace(&desired,&parameters,&active);auto end=from;end.pos.p=desired;reach=fabs(end.GetCPNoHeight()-CVec2(point.x,point.y));desiredPose=desired.GetPose();}
    auto afterRandom=GlobalGameRandom();const bool unchanged=actor->GetPosition()==from && actor->GetAP()==ap && rpg->GetTotalVP()==hp && held->GetAmmoQuantity()==ammo && inventory->GetActive()==held.GetPtr() && inventory->GetActiveSlot()==slot && inventory->GetItems().size()==count && destinationInventory->GetItems().size()==destinationCount && rpg->CanMove()!=motionless && actor->animator.IsActiveItem() && !actor->GetUndrawItem() && !actor->IsWalkingWithoutWeapon() && !actor->HasCommand() && (!transfer?(body && body->GetInvItem()==item.GetPtr() && body->GetPos()==point):(recipient->GetPosition()==recipientPosition && recipient->GetAP()==targetAP && recipient->GetUnitRPG()->GetTotalVP()==targetHP && !recipient->HasCommand())) && item->GetQuantity()==quantity && (!clip || clip->GetIncQuantity()==rounds) && game->hintsSet.size()==hintCount && game->bNoMoreHints==exhausted && world->GetTime()->GetValue()==time && random.Get()==afterRandom.Get();
    std::printf("native pickup radii: mode=%d hint=%d dummy=%d transfer=%d motionless=%d pose=%d distance=%.6f direct=%d result=%d executor=%d move=%d path=%d reach=%.6f desiredPose=%d expected=%d preparation=%d price=%d AP=%d quantity=%d rounds=%d unchanged=%d\n",mode,int(hint),int(dummy),int(transfer),int(motionless),int(from.pos.p.GetPose()),double(fabs(from.GetCPNoHeight()-CVec2(point.x,point.y))),int(direct),int(result),int(bool(executor)),int(bool(mover)),int(bool(path)),double(reach),desiredPose,int(expected),int(preparation),price,ap,quantity,rounds,int(unchanged));std::fflush(stdout);
    Require(unchanged,"pickup radii planning changed native item/inventory/actor/recipient/hint/AP/time or next RNG word");
    Require(bool(executor)==expected && result==(expected?NWorld::UCR_OK:NWorld::UCR_GENERAL_FAILURE),"initial pickup radius ignores real hint type or original other-unit margin");
    if(!expected)return;
    Require(bool(mover)==preparation && (preparation?(path && reach<=.625f):!path),"pickup planning does not use original ordinary/hint radius or changed an existing direct pickup");
    if(preparation)return;
    Require(front && dynamic_cast<NWorld::CExecMoveInventoryItem*>(front),"pickup radii did not queue its actual native inventory executor first");executor->Run();unsigned inSource=0,inDestination=0;for(const auto& entry:inventory->GetItems())if(entry.pItem==item)++inSource;for(const auto& entry:destinationInventory->GetItems())if(entry.pItem==item)++inDestination;
    Require(hint?(inSource==0 && inDestination==0 && game->hintsSet.size()==hintCount+1):(inDestination==1 && (!transfer || inSource==0) && (!clip || clip->GetIncQuantity()==rounds)),"native pickup did not consume exactly one hint or transfer exactly one intact ordinary item");
    Require(transfer || !IsValid(world->GetFrozenItem(item)),"native pickup retained its old ground body");Require(actor->GetAP()==ap-price && rpg->GetTotalVP()==hp && held->GetAmmoQuantity()==ammo && actor->GetPosition()==from && rpg->CanMove()!=motionless && (!transfer || (recipient->GetPosition()==recipientPosition && recipient->GetAP()==targetAP && recipient->GetUnitRPG()->GetTotalVP()==targetHP)),"native pickup changed its price, actors, held ammunition or mobility");
    std::printf("native pickup radii performed: mode=%d hint=%d transfer=%d source=%u destination=%u hints=%u->%u price=%d AP=%d->%d rounds=%d\n",mode,int(hint),int(transfer),inSource,inDestination,unsigned(hintCount),unsigned(game->hintsSet.size()),price,ap,actor->GetAP(),clip?clip->GetIncQuantity():-1);std::fflush(stdout);
}

static void StandingTalk(const char* databasePath,const char* resourcePath,int mode)
{
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer *hero=nullptr,*npc=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && unit->GetUnitRPG()->CanMove() && !unit->HasCommand() && !unit->IsPerformingAction()) {
        if(!unit->IsAIUnit() && unit->GetUnitRPG()->GetRPGUnit()->IsHero())hero=unit;
        else if(!unit->GetUnitRPG()->GetRPGUnit()->IsHero() && !npc)npc=unit;
    }
    Require(hero && npc,"standing talk lacks an actual hero and nonhero human target");
    auto* owner=dynamic_cast<NWorld::CPlayer*>(hero->GetPlayer());Require(owner,"standing talk has no human owner");world->GivePlayerTurn(owner);
    Require(world->IsTurnBased() && world->GetCurrentPlayer()==owner,"standing talk did not enter its native tactical turn");
    auto* actor=mode==7?npc:hero;auto* target=mode==7?hero:npc;auto* network=world->GetPathNetwork();
    vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),2.f),&places);bool placed=false;
    for(auto place:places) {
        place.SetPose(mode==9?NAI::CM_CROUCH:NAI::CM_STAND);place.SetMoving(false);
        if(!network->IsPassable(place) || network->IsLocked(place,true))continue;
        auto to=target->GetPosition();to.pos.p=place;
        const auto delta=to.GetCPNoHeight()-actor->GetPosition().GetCPNoHeight();const float distance=fabs(delta);
        if(distance<.2f || distance>.75f || !NWorld::IsWithinHumanReach(actor->GetPosition().GetCP(),to.GetEyePosition(),.8f))continue;
        target->SetPosition(to);target->animator.PlaceUnit(to);target->Update();placed=true;break;
    }
    Require(placed,"standing talk cannot place its real target in an unlocked native nearby cell");target->SetCanTalk(mode!=6);
    auto pose=actor->GetPosition();pose.pos.p.SetPose(mode==1 || mode==3?NAI::CM_CROUCH:mode==2 || mode==4?NAI::CM_LAY:NAI::CM_STAND);
    const auto direction=NRPG::GetShootDirection(network,pose.pos.p,target->GetPosition().GetEyePosition());pose.pos.p.SetDirection(mode==8?NAI::EDirection((int(direction)+4)%8):direction);
    Require(network->IsNativePassable(pose.pos.p),"standing talk has no native passable actor posture");actor->SetPosition(pose);actor->animator.PlaceUnit(pose);actor->Update();
    actor->animator.SetActiveItem(true);actor->SetUndrawItem(false);actor->SetWalkWithoutWeapon(false);
    auto* rpg=actor->GetUnitRPG();const bool motionless=mode==3 || mode==4 || mode==5;
    struct ReleaseMotionless {NRPG::IUnitMission* unit=nullptr;~ReleaseMotionless(){if(unit)unit->ReleaseMotionless();}} release;
    if(motionless){rpg->ApplyMotionless();release.unit=rpg;}Require(rpg->CanMove()!=motionless,"standing talk has the wrong real native motionless counter");
    const auto from=actor->GetPosition(),targetPosition=target->GetPosition();const int ap=actor->GetAP(),hp=rpg->GetTotalVP(),targetAP=target->GetAP(),targetHP=target->GetUnitRPG()->GetTotalVP();
    auto* inventory=rpg->GetInventory();CPtr<NRPG::IInventoryItem> held=inventory->GetActive();auto* gun=dynamic_cast<NRPG::IWeaponItem*>(held.GetPtr());const int ammo=gun?gun->GetAmmoQuantity():-1;const auto count=inventory->GetItems().size();const auto slot=inventory->GetActiveSlot();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    CObj<NWorld::CExecTalk> action=new NWorld::CExecTalk(actor,target);const auto direct=action->CanDoIt(from);
    Require(direct==(mode==6?NWorld::UCR_GENERAL_FAILURE:mode==7?NWorld::UCR_NOT_HERO:NWorld::UCR_OK),"standing talk direct validator violates its existing target/hero contract");
    CObj<NWorld::CCmdTalk> command=new NWorld::CCmdTalk(target);NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;
    CObj<NWorld::CCommandExecute> executor=NWorld::CreateActionExecutor(actor,command,&result);auto* queue=dynamic_cast<NWorld::CSimpleExecQueue*>(executor.GetPtr());auto* front=queue?queue->GetFrontExecutor():nullptr;auto* mover=dynamic_cast<NWorld::IExecMove*>(front);
    CObj<NAI::CPath> path=executor?executor->GetCurrentPath():nullptr;int desiredPose=-1,desiredDirection=-1;bool desiredReach=false;
    if(mover){NAI::SPathPlace desired;NAI::EFindPathParams parameters;NWorld::ENeedActiveItem active;mover->GetDesiredPlace(&desired,&parameters,&active);desiredPose=desired.GetPose();desiredDirection=desired.GetDirection();auto destination=from;destination.pos.p=desired;desiredReach=NWorld::IsWithinHumanReach(destination.GetCP(),targetPosition.GetEyePosition(),.8f);}
    const bool expected=mode!=3 && mode!=4 && mode!=6 && mode!=7;const bool preparation=mode==1 || mode==2 || mode==8;
    auto afterRandom=GlobalGameRandom();const bool unchanged=actor->GetPosition()==from && target->GetPosition()==targetPosition && actor->GetAP()==ap && rpg->GetTotalVP()==hp && target->GetAP()==targetAP && target->GetUnitRPG()->GetTotalVP()==targetHP && inventory->GetActive()==held.GetPtr() && inventory->GetActiveSlot()==slot && inventory->GetItems().size()==count && (!gun || gun->GetAmmoQuantity()==ammo) && actor->animator.IsActiveItem() && !actor->GetUndrawItem() && !actor->IsWalkingWithoutWeapon() && rpg->CanMove()!=motionless && target->CanTalk()==(mode!=6) && !actor->HasCommand() && !target->HasCommand() && world->GetTime()->GetValue()==time && random.Get()==afterRandom.Get();
    std::printf("native standing talk: mode=%d hero=%d canTalk=%d motionless=%d pose=%d targetPose=%d direction=%d aligned=%d direct=%d result=%d executor=%d move=%d path=%d desiredPose=%d desiredDirection=%d desiredReach=%d expected=%d preparation=%d AP=%d ammo=%d unchanged=%d\n",mode,int(rpg->GetRPGUnit()->IsHero()),int(target->CanTalk()),int(motionless),int(from.pos.p.GetPose()),int(targetPosition.pos.p.GetPose()),int(from.pos.p.GetDirection()),int(from.pos.p.GetDirection()==direction),int(direct),int(result),int(bool(executor)),int(bool(mover)),int(bool(path)),desiredPose,desiredDirection,int(desiredReach),int(expected),int(preparation),ap,ammo,int(unchanged));std::fflush(stdout);
    Require(unchanged,"standing talk planning changed native actors/inventory/health/AP/mobility/time or next RNG word");
    Require(bool(executor)==expected && result==(mode==7?NWorld::UCR_NOT_HERO:expected?NWorld::UCR_OK:NWorld::UCR_GENERAL_FAILURE),"talk must require a standing actor and preserve existing hero/target/mobility gates");
    if(!expected)return;
    Require(bool(mover)==preparation && (!preparation?(dynamic_cast<NWorld::CExecTalk*>(front) && !path):(path && desiredPose==NAI::CM_STAND && desiredReach)),"talk from crouched/prone pose skipped native standing preparation or changed a direct/turning control");
}

static void NearbyPickup(const char* databasePath,const char* resourcePath,int mode)
{
    const bool mobile=mode==7 || mode==8,within=mode!=4 && mode!=5 && mode!=6;
    float distance=mode==3?1.25f:mode==4?1.2505f:mobile?.25f:1.f;
    float height=mode==1?1.f:mode==2?-.25f:mode==5?-.5f:mode==6?2.f:mode==7?.25f:mode==9?.5f:mode==11?-.499f:mode==12?1.999f:0.f;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;CPtr<NRPG::CWeaponItem> held;
    for(auto unit:units)if(unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK() && unit->GetUnitRPG()->CanMove() && !unit->HasCommand() && !unit->IsPerformingAction()) {
        auto* gun=dynamic_cast<NRPG::CWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        if(gun && IsValid(gun->GetDBWeapon()->pInnerClip) && IsValid(gun->GetDBWeapon()->pInnerClip->pItem)){actor=unit;held=gun;break;}
    }
    Require(actor && held,"nearby pickup fixture lacks its real human firearm user and authored clip");
    auto* owner=dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer());Require(owner,"nearby pickup has no actual owning player");world->GivePlayerTurn(owner);Require(world->IsTurnBased() && world->GetCurrentPlayer()==owner,"nearby pickup did not enter a native tactical turn");
    auto* network=world->GetPathNetwork();
    if(mode==9 || mode==10){auto pose=actor->GetPosition();pose.pos.p.SetPose(mode==9?NAI::CM_CROUCH:NAI::CM_LAY);Require(network->IsNativePassable(pose.pos.p),"nearby pickup has no native passable posture control");actor->SetPosition(pose);actor->animator.PlaceUnit(pose);actor->Update();}
    actor->animator.SetActiveItem(true);actor->SetUndrawItem(false);actor->SetWalkWithoutWeapon(false);
    auto* rpg=actor->GetUnitRPG();auto* inventory=rpg->GetInventory();
    struct ReleaseMotionless {NRPG::IUnitMission* unit=nullptr;~ReleaseMotionless(){if(unit)unit->ReleaseMotionless();}} release;
    if(!mobile){rpg->ApplyMotionless();release.unit=rpg;}Require(rpg->CanMove()==mobile,"nearby pickup has the wrong real native motionless counter");
    CObj<NRPG::IInventoryItem> item=NRPG::CreateClipItem(held->GetDBWeapon()->pInnerClip);auto* clip=dynamic_cast<NRPG::IClipItem*>(item.GetPtr());CTPoint<int> destination;
    Require(clip && inventory->FindPlace(item,&destination),"nearby pickup has no authored clip or genuine backpack space");
    const auto from=actor->GetPosition();CVec3 requested;bool found=false;
    for(int i=0;i<8;++i){const float angle=i*FP_PI*.25f;const CVec3 p=from.GetCP()+CVec3(distance*cos(angle),distance*sin(angle),height);const auto dir=NRPG::GetShootDirection(network,from.pos.p,p);if((dir==from.pos.p.GetDirection())==(mode==8)){requested=p;found=true;break;}}
    Require(found,"nearby pickup cannot choose a native aligned/other-heading item point");
    CPtr<NWorld::CDFrozenItem> body=world->AddFrozenItem(world->GetAIMap(),requested,QNULL,item);
    Require(IsValid(body) && body->GetInvItem()==item.GetPtr() && world->GetFrozenItem(item)==body,"nearby pickup failed to register its actual native world body and item identity");
    const auto point=body->GetPos();const auto planar=from.GetCPNoHeight()-CVec2(point.x,point.y);const float actualDistance=fabs(planar),actualHeight=point.z-from.GetCP().z;
    Require(fabs(actualDistance-distance)<.0001f && fabs(actualHeight-height)<.0001f,"nearby pickup native body position does not match its boundary input");
    NWorld::SItem source(nullptr,NWorld::SItem::GROUND,item);source.pWorldItem=body.GetPtr();NWorld::SItem target(actor,NWorld::SItem::BACKPACK,destination);
    CObj<NWorld::CCmdMoveInventoryItem> command=new NWorld::CCmdMoveInventoryItem(source,target);CObj<NWorld::CExecMoveInventoryItem> action=new NWorld::CExecMoveInventoryItem(actor,command);
    const auto direct=action->CanDoIt(from);Require(direct==NWorld::UCR_OK,"nearby pickup inventory validation rejects its actual free destination");
    const int price=action->GetStartAP(),ap=actor->GetAP(),hp=rpg->GetTotalVP(),ammo=held->GetAmmoQuantity(),quantity=item->GetQuantity(),rounds=clip->GetIncQuantity();const auto count=inventory->GetItems().size();const auto time=world->GetTime()->GetValue();const auto slot=inventory->GetActiveSlot();auto random=GlobalGameRandom();
    NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCommandExecute> executor=NWorld::CreateActionExecutor(actor,command,&result);
    auto* queue=dynamic_cast<NWorld::CSimpleExecQueue*>(executor.GetPtr());auto* front=queue?queue->GetFrontExecutor():nullptr;const bool move=front && dynamic_cast<NWorld::IExecMove*>(front);CObj<NAI::CPath> path=executor?executor->GetCurrentPath():nullptr;
    auto afterRandom=GlobalGameRandom();const bool unchanged=actor->GetPosition()==from && actor->GetAP()==ap && rpg->GetTotalVP()==hp && held->GetAmmoQuantity()==ammo && inventory->GetActive()==held.GetPtr() && inventory->GetActiveSlot()==slot && inventory->GetItems().size()==count && rpg->CanMove()==mobile && actor->animator.IsActiveItem() && !actor->GetUndrawItem() && !actor->IsWalkingWithoutWeapon() && !actor->HasCommand() && IsValid(body) && body->GetInvItem()==item.GetPtr() && body->GetPos()==point && item->GetQuantity()==quantity && clip->GetIncQuantity()==rounds && world->GetTime()->GetValue()==time && random.Get()==afterRandom.Get();
    std::printf("native nearby pickup: mode=%d mobile=%d pose=%d direction=%d distance=%.6f height=%.6f direct=%d result=%d executor=%d move=%d path=%d near=%d price=%d AP=%d quantity=%d rounds=%d unchanged=%d\n",mode,int(mobile),int(from.pos.p.GetPose()),int(from.pos.p.GetDirection()),double(actualDistance),double(actualHeight),int(direct),int(result),int(bool(executor)),int(move),int(bool(path)),int(within),price,ap,quantity,rounds,int(unchanged));std::fflush(stdout);
    Require(unchanged,"nearby pickup planning changed native item/owner/inventory/actor state or next RNG word");
    Require(within?(executor && result==NWorld::UCR_OK && !move && !path):!executor,"nearby ground item requires a turn/move despite the original reach bounds or escaped the original near boundary");
    if(!within)return;
    Require(front && dynamic_cast<NWorld::CExecMoveInventoryItem*>(front),"nearby pickup did not queue its real inventory action first");executor->Run();unsigned occurrences=0;for(const auto& entry:inventory->GetItems())if(entry.pItem==item)++occurrences;
    Require(occurrences==1 && !IsValid(world->GetFrozenItem(item)) && (!IsValid(body) || body->GetInvItem()!=item.GetPtr()) && item->GetQuantity()==quantity && clip->GetIncQuantity()==rounds,"actual nearby pickup did not transfer exactly one intact native item out of its world body");
    Require(actor->GetAP()==ap-price && rpg->GetTotalVP()==hp && actor->GetPosition()==from && held->GetAmmoQuantity()==ammo && inventory->GetActive()==held.GetPtr() && rpg->CanMove()==mobile,"actual nearby pickup changed its existing AP price, held weapon, health, position or mobility");
    std::printf("native nearby pickup performed: mode=%d AP=%d->%d price=%d quantity=%d rounds=%d occurrences=%u motionless=%d\n",mode,ap,actor->GetAP(),price,item->GetQuantity(),clip->GetIncQuantity(),occurrences,int(!rpg->CanMove()));std::fflush(stdout);
}

static void ReadyAction(const char* databasePath,const char* resourcePath,int mode)
{
    const bool shot=mode>=8,motionless=mode==1 || mode==3 || mode==5 || mode==10;
    const bool holstered=mode==2 || mode==3,undraw=mode==4 || mode==5,invalid=mode==6;
    const bool preparation=holstered || undraw || mode==9;
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,false,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;CPtr<NRPG::CWeaponItem> weapon;
    for(auto unit:units) {
        if(!unit || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK() || !unit->GetUnitRPG()->CanMove() || unit->HasCommand() || unit->IsPerformingAction() || unit->GetPosition().pos.p.GetPose()==NAI::CM_INACTIVE)continue;
        auto* item=dynamic_cast<NRPG::CWeaponItem*>(unit->GetUnitRPG()->GetInventory()->GetActive());
        if(!item || item->GetAmmoQuantity()<=0 || !unit->CanSpendAP(unit->GetActionAP(NRPG::AC_RELOAD)))continue;
        if(shot){if(!item->SetShootMode(NDb::SM_Snap) || NWorld::GetActionType(unit)!=NWorld::AT_SHOOT)continue;}
        else {item->DiscardAmmo();if(!item->CanReload(unit->GetUnitRPG()->GetInventory()))continue;}
        actor=unit;weapon=item;break;
    }
    Require(actor && weapon,"ready-action fixture lacks an actual healthy firearm user with authored ammunition");
    const bool wasRealTime=world->IsRealTime();auto* owner=dynamic_cast<NWorld::CPlayer*>(actor->GetPlayer());Require(owner,"ready action has no native owning player");
    world->GivePlayerTurn(owner);Require(world->IsTurnBased() && world->GetCurrentPlayer()==owner,"ready action could not enter its actual native tactical turn");
    std::printf("native ready action turn: mode=%d realtime=%d->%d owner=%d\n",mode,int(wasRealTime),int(world->IsRealTime()),owner->GetScenarioPlayerID());std::fflush(stdout);
    if(mode==7){auto crouch=actor->GetPosition();crouch.pos.p.SetPose(NAI::CM_CROUCH);actor->SetPosition(crouch);actor->animator.PlaceUnit(crouch);actor->Update();}
    actor->animator.SetActiveItem(!holstered);actor->SetUndrawItem(undraw);actor->SetWalkWithoutWeapon(undraw);
    auto* rpg=actor->GetUnitRPG();auto* inventory=rpg->GetInventory();
    struct ReleaseMotionless {NRPG::IUnitMission* unit=nullptr;~ReleaseMotionless(){if(unit)unit->ReleaseMotionless();}} release;
    if(motionless){rpg->ApplyMotionless();release.unit=rpg;}Require(rpg->CanMove()!=motionless,"ready-action fixture did not set the actual native motionless counter");
    const auto from=actor->GetPosition();CObj<NRPG::IInventoryItem> removed;int slot=-1;
    if(invalid){slot=inventory->GetActiveSlot()==NDb::SLOT_1?int(NDb::SLOT_2):int(NDb::SLOT_1);if(inventory->Get(NDb::ESlot(slot)))removed=inventory->TakeOff(NDb::ESlot(slot));}
    CVec3 target;int desired=from.pos.p.GetDirection();CObj<NWorld::CCmd> command;NWorld::EUnitCommandResult direct;
    if(shot) {
        bool found=false;
        for(int i=0;i<8;++i){const float angle=i*FP_PI*.25f;const CVec3 p=from.GetCP()+CVec3(20*cos(angle),20*sin(angle),1);const auto dir=NRPG::GetShootDirection(world->GetPathNetwork(),from.pos.p,p);if((dir==from.pos.p.GetDirection())!=(mode==9)){target=p;desired=dir;found=true;break;}}
        Require(found,"ready-action fixture could not select an actual native aligned/turning shot");
        command=new NWorld::CCmdShootTile(target);CObj<NWorld::CExecShootTile> action=new NWorld::CExecShootTile(actor,target);direct=action->CanDoIt(from);
    } else {command=new NWorld::CCmdReload(slot);CObj<NWorld::CExecReload> action=new NWorld::CExecReload(actor,slot);direct=action->CanDoIt(from);}
    Require(invalid?direct==NWorld::UCR_NO_EQUIPMENT:direct==NWorld::UCR_OK,"ready-action direct native action has unexpected availability before its queue is constructed");
    const int ap=actor->GetAP(),hp=rpg->GetTotalVP(),ammo=weapon->GetAmmoQuantity(),price=actor->GetActionAP(NRPG::AC_RELOAD);const bool active=actor->animator.IsActiveItem(),walk=actor->IsWalkingWithoutWeapon();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    NWorld::EUnitCommandResult result=NWorld::UCR_GENERAL_FAILURE;CObj<NWorld::CCommandExecute> executor=NWorld::CreateActionExecutor(actor,command,&result);
    auto* queue=dynamic_cast<NWorld::CSimpleExecQueue*>(executor.GetPtr());auto* front=queue?queue->GetFrontExecutor():nullptr;
    const bool move=front && dynamic_cast<NWorld::IExecMove*>(front);CObj<NAI::CPath> path=executor?executor->GetCurrentPath():nullptr;
    const bool expectedAvailable=!invalid && !(motionless && preparation);
    auto afterRandom=GlobalGameRandom();const bool unchanged=actor->GetPosition()==from && actor->GetAP()==ap && rpg->GetTotalVP()==hp && weapon->GetAmmoQuantity()==ammo && actor->animator.IsActiveItem()==active && actor->GetUndrawItem()==undraw && actor->IsWalkingWithoutWeapon()==walk && rpg->CanMove()!=motionless && world->GetTime()->GetValue()==time && !actor->HasCommand() && random.Get()==afterRandom.Get();
    std::printf("native ready action: mode=%d shot=%d motionless=%d active=%d undraw=%d preparation=%d pose=%d direction=%d desired=%d direct=%d result=%d executor=%d move=%d path=%d expected=%d AP=%d ammo=%d unchanged=%d\n",mode,int(shot),int(motionless),int(active),int(undraw),int(preparation),int(from.pos.p.GetPose()),int(from.pos.p.GetDirection()),int(desired),int(direct),int(result),int(bool(executor)),int(move),int(bool(path)),int(expectedAvailable),ap,ammo,int(unchanged));std::fflush(stdout);
    Require(unchanged,"constructing a ready native action changed AP/ammo/health/pose/item wishes/time/RNG or mobility");
    Require(bool(executor)==expectedAvailable && result==(invalid?direct:expectedAvailable?NWorld::UCR_OK:NWorld::UCR_GENERAL_FAILURE) && (!executor || (move==preparation && (!path || preparation) && (mode!=9 || bool(path)))),"already-ready native action unnecessarily requires movement or ignores required item/facing preparation");
    if(!expectedAvailable || preparation || shot)return;
    Require(front && pSSClasses->GetObjectTypeID(front)==int(0x00122170) && front==FindNativeReload(executor),"ready reload is not the next actual native action");
    // Execute the actual native action, including the ammunition label and cost;
    // this is not a fake queue or a planner-only substitute for reloading.
    executor->Run();Require(front->IsExecuting() && actor->GetAP()==ap-price && weapon->GetAmmoQuantity()==0,"ready native reload did not start immediately and spend exactly its existing AP cost");
    executor->TimeLabelReached();Require(weapon->GetAmmoQuantity()>0 && actor->GetAP()==ap-price && rpg->GetTotalVP()==hp && actor->GetPosition()==from && rpg->CanMove()!=motionless,"ready native reload failed at its real ammunition label or changed position/health/mobility");
    executor->AnimationFinished();
    std::printf("native ready reload performed: mode=%d price=%d AP=%d->%d ammo=%d->%d motionless=%d\n",mode,price,ap,actor->GetAP(),ammo,weapon->GetAmmoQuantity(),int(!rpg->CanMove()));std::fflush(stdout);
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
        float cover = 123.f;
        int quality = 123, hit = 123;
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

class LayerMergeMapProbe: public CoverOriginMapProbe
{
    OBJECT_NOCOPY_METHODS(LayerMergeMapProbe);
public:
    void PrepareCollider(NAI::IPrepareCollider*,const SBound&,float,int,bool) override { Require(false,"layer merge unexpectedly requested geometry instead of its supplied native collider"); }
};

static void MovementLayerMerge(int mode)
{
    CObj<NAI::CPathNetwork> network=new NAI::CPathNetwork;network->CreateLayer(8,8,VNULL2,0.f,0);auto* first=network->GetLayer(0);network->CreateLayer(8,8,VNULL2,0.f,1,first->pGroup);network->Freeze(true);auto* current=network->GetLayer(1);
    Require(first->pGroup==current->pGroup && first->pGroup->layers[0]==first && first->pGroup->layers[NAI::N_MAX_LAYERS_PER_FLOOR]==current,"native layer merge has no two ordered surfaces in its actual group");
    // A single boundary cell retains previously calculated connections outside
    // this buffer. Public Calc still executes its real private merge stage;
    // no collision traces or access to private members are needed.
    const CTRect<int> region(3,3,4,4);NAI::STempArrayGroup<NAI::STile> buffers;buffers.region=region;auto* lower=buffers.GetArray(0,0);auto* upper=buffers.GetArray(1,0);auto& a=(*upper)[0][0];auto& b=(*lower)[0][0];
    a=NAI::STile{};b=NAI::STile{};a.nPassable=b.nPassable=NAI::CP_STAND|NAI::CP_CROUCH;a.nMoveCrouch=b.nMoveCrouch=1;a.nMoveStand=b.nMoveStand=1;a.nHeight=1;b.nHeight=0;a.nLocks=2;a.nDynLocks=3;b.nLocks=4;b.nDynLocks=5;
    if(mode==0){b.nPassable=0;b.nMoveCrouch=0;}
    if(mode==1)a.nMoveCrouch=0;
    if(mode==2)b.nMoveCrouch=0;
    if(mode==4){a.nMoveCrouch=b.nMoveCrouch=0;a.nMoveHC=1;}
    if(mode==5){b.nMoveCrouch=0;b.nMoveHC=1;}
    if(mode==6)b.nHeight=1+NAI::GetIHeight(0.05f)+1;
    if(mode==7)b.nDisplacement=1;
    if(mode==8)a.nPassable=0;
    if (a.nPassable && a.nMoveCrouch) {
        a.nPassable |= NAI::CP_LAY;
        a.nMoveLay = 1 << NAI::CD_L;
    }
    const auto beforeA=a,beforeB=b;auto random=GlobalGameRandom();CObj<LayerMergeMapProbe> map=new LayerMergeMapProbe;CObj<NAI::CCollider> collider=new NAI::CCollider;SBound bound;bound.BoxInit(CVec3(-8,-8,-8),CVec3(8,8,8));collider->SetBoundAndResolution(bound,2.f);
    NAI::CMovesCalcer calcer(current,map,region,buffers,0,1,collider);calcer.Calc();auto afterRandom=GlobalGameRandom();
    const bool mergedA=(a.nFlags&NAI::TF_HAS_SAME)!=0,mergedB=(b.nFlags&NAI::TF_HAS_SAME)!=0,expected=mode==3 || mode==4;
    auto expectedA=beforeA,expectedB=beforeB;if(expected){expectedA.nFlags|=NAI::TF_HAS_SAME|NAI::TF_STAND_PASSABLE;expectedB.nFlags|=NAI::TF_HAS_SAME|NAI::TF_STAND_PASSABLE;expectedB.nHeight=expectedA.nHeight;}
    if (mode == 1) expectedA.nPassable = 0;
    bool unchanged=std::memcmp(&a,&expectedA,sizeof(a))==0 && std::memcmp(&b,&expectedB,sizeof(b))==0 && random.Get()==afterRandom.Get() && first->transitions.empty() && current->transitions.empty() && map->traces.empty();
    std::printf("native layer merge: mode=%d currentPass=%d otherPass=%d currentCrouch=%d otherCrouch=%d currentHC=%d otherHC=%d currentHeight=%d otherHeight=%d->%d flags=%d/%d merged=%d/%d expected=%d exact=%d locks=%d,%d/%d,%d\n",mode,int(beforeA.nPassable),int(beforeB.nPassable),int(beforeA.nMoveCrouch),int(beforeB.nMoveCrouch),int(beforeA.nMoveHC),int(beforeB.nMoveHC),int(a.nHeight),int(beforeB.nHeight),int(b.nHeight),int(a.nFlags),int(b.nFlags),int(mergedA),int(mergedB),int(expected),int(unchanged),int(a.nLocks),int(a.nDynLocks),int(b.nLocks),int(b.nDynLocks));std::fflush(stdout);
    Require(mergedA==expected && mergedB==expected && unchanged,"native movement calculation merged an ineligible surface or changed the original merge eligibility/metadata");
}

static void IsolatedMovementCell()
{
    for (int exit : {0, 1, 2}) {
        CObj<NAI::CPathNetwork> network = new NAI::CPathNetwork;
        network->CreateLayer(8, 8, VNULL2, 0.f, 0);
        network->Freeze(true);
        auto* layer = network->GetLayer(0);
        const CTRect<int> region(3, 3, 4, 4);
        NAI::STempArrayGroup<NAI::STile> buffers;
        buffers.region = region;
        auto& cell = (*buffers.GetArray(0, 0))[0][0];
        cell = NAI::STile{};
        cell.nPassable = NAI::CP_STAND | NAI::CP_CROUCH | NAI::CP_LAY;
        cell.nMoveLay = exit == 1 ? 1 << NAI::CD_L : 0;
        cell.nMoveHC = exit == 2 ? 1 << NAI::CD_L : 0;
        cell.nHeight = 1;

        CObj<LayerMergeMapProbe> map = new LayerMergeMapProbe;
        CObj<NAI::CCollider> collider = new NAI::CCollider;
        SBound bound;
        bound.BoxInit(CVec3(-8, -8, -8), CVec3(8, 8, 8));
        collider->SetBoundAndResolution(bound, 2.f);
        NAI::CMovesCalcer calcer(layer, map, region, buffers, 0, 0, collider);
        calcer.Calc();
        Require((cell.nPassable != 0) == (exit != 0),
            exit ? "a crawling or high-climb exit lost passability" : "a movement cell with no exit remained passable");
    }
}

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

static void GroundPassabilityPrecedence()
{
    auto network=SimplePathNetwork();
    NAI::SPathPlace point(4,4,0,0,NAI::CM_STAND,0), key(4,4,0);
    auto& tile=network->GetLayer(0)->tiles[4][4];
    CObj<NWorld::CWindowDoor> source=new NWorld::CWindowDoor;
    // Initialize the actual native door's open field through its public serializer.
    CMemoryStream state;
    {CStructureSaver save(state,CStructureSaver::WRITE);bool open=false;save.Add(2,&open);}
    state.Seek(0);
    {CStructureSaver load(state,CStructureSaver::READ);static_cast<CObjectBase*>(source.GetPtr())->operator&(load);}
    Require(!source->IsOpen(),"ground verdict fixture did not initialize its native door");
    auto* door=network->GetFlipper(source);
    NAI::STile blocked{},open{};open.nPassable=0xff;
    door->locksClosed[key]=blocked;door->locksOpen[key]=open;
    network->InvalidateFlipperIndex();
    Require(network->GetFlippersAt(point).size()==1 && network->GetPassability(point)==NAI::AIP_DOOR,
        "ground verdict fixture did not register an actual blocking door restriction");
    for(int pose: {NAI::CM_STAND,NAI::CM_CROUCH,NAI::CM_INACTIVE,NAI::CM_LAY}) {
        point.SetPose(pose);tile.nPassable=0xff;
        for(bool doorBlocked: {false,true}) for(bool locked: {false,true}) {
            door->bOpen=!doorBlocked;tile.nLocks=locked ? 1 : 0;
            const auto expected=locked ? NAI::AIP_LOCKED : doorBlocked ? NAI::AIP_DOOR : NAI::AIP_YES;
            Require(network->GetPassability(point)==expected,
                "ground verdict controls lost their distinct allowed, locked and door results");
        }
    }
    struct BlockedPose {int pose;unsigned char mask;NAI::EPassable expected;};
    const BlockedPose cases[]={{NAI::CM_STAND,0,NAI::AIP_NOT_PASSABLE},
        {NAI::CM_CROUCH,0,NAI::AIP_NOT_PASSABLE},{NAI::CM_INACTIVE,0,NAI::AIP_NOT_PASSABLE},
        {NAI::CM_LAY,0,NAI::AIP_NOT_PASSABLE},{NAI::CM_LAY,NAI::CP_LAY,NAI::AIP_CANNOT_LAY}};
    for(const auto& test: cases) for(int facing=0;facing<8;++facing)
        for(bool doorBlocked: {false,true}) for(bool locked: {false,true}) {
            point.SetPose(test.pose);point.SetDirection(facing);tile.nPassable=test.mask;tile.nLocks=locked ? 1 : 0;door->bOpen=!doorBlocked;
            const auto actual=network->GetPassability(point);
            if(actual!=test.expected)std::printf("native ground verdict: pose=%d facing=%d mask=%u lock=%d door=%d actual=%d expected=%d\n",
                test.pose,facing,unsigned(test.mask),locked,doorBlocked,int(actual),int(test.expected));
            Require(actual==test.expected,"ground pose failure was misclassified as an occupancy or door failure");
            Require(!network->IsPassable(point),"correcting a granular failure reason allowed a forbidden pose");
        }
    tile.nPassable=0xff;tile.nLocks=0;door->bOpen=true;
    point.SetPose(NAI::CM_STAND);
    Require(network->GetPassability(point)==NAI::AIP_YES && network->IsPassable(point),
        "ground query failed to recover after all native restrictions were removed");
}
