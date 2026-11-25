class NativeAtlasTextureInput:public CPtrFuncBase<NGfx::CTexture>
{
    OBJECT_NOCOPY_METHODS(NativeAtlasTextureInput);
    void Recalc() override {}
public:
    explicit NativeAtlasTextureInput(NGfx::CTexture* texture=nullptr){pValue=texture;}
};
class NativeAtlasParticleOutput:public NGScene::IParticleOutput
{
public:
    struct Quad{CVec3 positions[4];DWORD color;NGScene::STransparentTexturePlace texture;float depth;};
    NGScene::SParticleOrientationInfo orientation;
    vector<Quad> quads;
    NativeAtlasParticleOutput(){orientation.vBasic[0]=CVec3(1,0,0);orientation.vBasic[1]=CVec3(0,1,0);orientation.vBasic[2]=CVec3(0,0,1);orientation.vBasic[3]=VNULL3;orientation.vDepth=CVec3(0,0,1);}
    const NGScene::SParticleOrientationInfo& GetOrientationInfo() const override{return orientation;}
    void AddParticle(const CVec3* positions,DWORD color,const NGScene::STransparentTexturePlace& texture,float depth) override
    {
        Quad q;for(int i=0;i<4;++i){q.positions[i]=positions[i];Require(std::isfinite(positions[i].x) && std::isfinite(positions[i].y) && std::isfinite(positions[i].z),"atlas native particle has nonfinite geometry");}q.color=color;q.texture=texture;q.depth=depth;quads.push_back(q);
    }
    unsigned GeometryHash() const
    {
        unsigned h=2166136261u;auto add=[&](const void* value){unsigned word;std::memcpy(&word,value,4);h=(h^word)*16777619u;};
        for(const auto& q:quads){for(const auto& p:q.positions){add(&p.x);add(&p.y);add(&p.z);}add(&q.color);add(&q.depth);}return h;
    }
};
static bool NativeAtlasSameRegion(const CTRect<int>& a,const CTRect<int>& b)
{
    return a.x1==b.x1 && a.y1==b.y1 && a.x2==b.x2 && a.y2==b.y2;
}
static bool NativeAtlasQuadMatches(const NGScene::STransparentTexturePlace& q,int l,int t,int r,int b)
{
    const int expected[4][2]={{l,t},{r,t},{r,b},{l,b}};
    for(int i=0;i<4;++i)if(q.vUVs[i].nU!=expected[i][0] || q.vUVs[i].nV!=expected[i][1])return false;return true;
}

static void NativeParticleAtlasMargin(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<6 && std::getenv("S2_USER_DATA_DIR"),"atlas particle requires actual mode and isolated profile");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm native particle atlas margin");
    const int oldHD=NGlobal::GetVar("gfx_hd_textures",1).GetInt();struct Restore{int hd;~Restore(){NGlobal::SetVar("gfx_hd_textures",hd);}}restore{oldHD};
    const bool hd=mode>=3,offset=mode==2 || mode==5;NGlobal::SetVar("gfx_hd_textures",int(hd));
    CObj<NGfx::CTexture> padding;if(offset)padding=NGfx::MakeTexture(64,64,1,NGfx::SPixel8888::ID,NGfx::TRANSPARENT_TEXTURE,NGfx::CLAMP);
    const int width=mode==0 || mode==3?16:19,height=mode==0 || mode==3?16:mode==4?19:17;
    CObj<NGfx::CTexture> texture=NGfx::MakeTexture(width,height,1,NGfx::SPixel8888::ID,NGfx::TRANSPARENT_TEXTURE,NGfx::CLAMP);Require(texture,"atlas particle cannot allocate actual bgfx RGBA atlas texture");
    NGfx::STexturePlaceInfo place;CObj<NGfx::CTexture> holder=NGfx::GetTextureContainer(texture,&place);
    Require(holder && NGfx::HasSameContainer(holder,NGfx::GetTransparentTextureCache()) && place.size.x==(hd?4096:1024) && place.size.y==place.size.x && place.place.Width()==width && place.place.Height()==height && (place.place.x1%4)==0 && (place.place.y1%4)==0 && (!offset || place.place.x1>0 || place.place.y1>0),"atlas particle lacks real aligned/current cache region");
    auto* buffer=texture->pTB.GetPtr();const auto region=texture->region;CObj<NativeAtlasTextureInput> input=new NativeAtlasTextureInput(texture);
    CObj<NGScene::CRainParticleEffect> effect=new NGScene::CRainParticleEffect;effect->bEnd=false;effect->nGrassSize=0;effect->textures.push_back(CObj<CPtrFuncBase<NGfx::CTexture>>(input.GetPtr()));effect->positions.push_back(CVec3(0,1,-4));effect->directions.push_back(CVec3(0,0,-1));effect->faces.push_back(0);
    NativeAtlasParticleOutput output;auto random=GlobalGameRandom();effect->AddParticles(&output);Require(output.quads.size()==1,"actual rain consumer did not emit exactly one quad");
    // Literal atlas contracts:1024 maps integer texels to two UV units;4096 maps pairs of texels to one. Half-texel margin rounds to these bounds with current2048 packing.
    const int left=hd?place.place.x1/2:place.place.x1*2+1,top=hd?place.place.y1/2:place.place.y1*2+1,right=hd?place.place.x2/2:place.place.x2*2-1,bottom=hd?place.place.y2/2:place.place.y2*2-1;
    const auto& q=output.quads.front();const bool matches=NativeAtlasQuadMatches(q.texture,left,top,right,bottom);const CVec3 vertices[]={CVec3(0,1,-4),CVec3(.033f,1,-4),CVec3(.033f,1,-5.75f),CVec3(0,1,-5.75f)};bool geometry=q.color==0xffffffffu && q.depth==-4.f;for(int i=0;i<4;++i)geometry&=fabs(q.positions[i]-vertices[i])<.000001f;
    auto after=GlobalGameRandom();bool sameRNG=true;for(int i=0;i<4;++i)sameRNG&=random.Get()==after.Get();
    std::printf("native particle atlas margin mode=%d HD=%d atlas=%d width=%d height=%d region=%d,%d,%d,%d uv=%d,%d,%d,%d expected=%d,%d,%d,%d matches=%d geometry=%d hash=%08x RNG=%d owner=%d regionSame=%d\n",mode,int(hd),place.size.x,width,height,place.place.x1,place.place.y1,place.place.x2,place.place.y2,q.texture.vUVs[0].nU,q.texture.vUVs[0].nV,q.texture.vUVs[2].nU,q.texture.vUVs[2].nV,left,top,right,bottom,int(matches),int(geometry),output.GeometryHash(),int(sameRNG),int(texture->pTB.GetPtr()==buffer && NGfx::HasSameContainer(texture,holder)),int(NativeAtlasSameRegion(texture->region,region)));std::fflush(stdout);
    Require(geometry && sameRNG && texture->pTB.GetPtr()==buffer && NativeAtlasSameRegion(texture->region,region) && effect->textures.front().GetPtr()==input.GetPtr() && !effect->bEnd,"particle UV query changed native geometry/color/depth/RNG/texture ownership/input");
    Require(matches,"actual native rain output retains unpadded particle atlas edges");
}

static void NativeGrassAtlasMargin(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<8 && std::getenv("S2_USER_DATA_DIR"),"atlas grass requires actual mode and isolated profile");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm native grass atlas margin");
    const int oldHD=NGlobal::GetVar("gfx_hd_textures",1).GetInt();struct Restore{int hd;~Restore(){NGlobal::SetVar("gfx_hd_textures",hd);}}restore{oldHD};
    const bool hd=mode>=4;const int grid=mode%4+1;NGlobal::SetVar("gfx_hd_textures",int(hd));
    CObj<NGfx::CTexture> texture=NGfx::MakeTexture(48,48,1,NGfx::SPixel8888::ID,NGfx::TRANSPARENT_TEXTURE,NGfx::CLAMP);Require(texture,"atlas grass cannot allocate actual bgfx atlas texture");
    NGfx::STexturePlaceInfo place;CObj<NGfx::CTexture> holder=NGfx::GetTextureContainer(texture,&place);Require(holder && NGfx::HasSameContainer(holder,NGfx::GetTransparentTextureCache()) && place.size.x==(hd?4096:1024) && place.size.y==place.size.x && place.place.Width()==48 && place.place.Height()==48 && (place.place.x1%4)==0 && (place.place.y1%4)==0,"atlas grass lacks actual current aligned cache region");
    auto* buffer=texture->pTB.GetPtr();const auto region=texture->region;CObj<NativeAtlasTextureInput> input=new NativeAtlasTextureInput(texture);
    CObj<NGScene::CGrassParticleEffect> effect=new NGScene::CGrassParticleEffect;effect->bEnd=false;effect->nGrassSize=grid;effect->textures.push_back(CObj<CPtrFuncBase<NGfx::CTexture>>(input.GetPtr()));effect->fTEffect=0;effect->fXPivot=effect->fYPivot=effect->fScaleRange=0;effect->scale=CVec2(2,2);
    for(int i=0;i<grid*grid;++i){effect->positions.push_back(CVec3(0,0,4.f+i));effect->colors.push_back(NGfx::SPixel8888(24,36,48,255));effect->waveAmps.push_back(0.f);}
    NativeAtlasParticleOutput output;auto random=GlobalGameRandom();effect->AddParticles(&output);Require(output.quads.size()==size_t(grid*grid),"actual native grass consumer lost blades");
    // Fixed cell-boundary tables for48-pixel input sheets in the two actual atlas sizes.
    const int stockLeft[4][4]={{1,0,0,0},{1,49,0,0},{1,33,65,0},{1,25,49,73}};
    const int stockRight[4][4]={{95,0,0,0},{47,95,0,0},{31,63,95,0},{23,47,71,95}};
    const int hdLeft[4][4]={{0,0,0,0},{0,12,0,0},{0,8,16,0},{0,6,12,18}};
    const int hdRight[4][4]={{24,0,0,0},{12,24,0,0},{8,16,24,0},{6,12,18,24}};
    const int sprite1[]={0},sprite2[]={0,1,2,3},sprite3[]={0,2,4,6,8,1,3,5,7},sprite4[]={0,9,2,11,4,13,6,15,8,1,10,3,12,5,14,7};const int* sprites=grid==1?sprite1:grid==2?sprite2:grid==3?sprite3:sprite4;
    const auto& l=hd?hdLeft:stockLeft;const auto& rr=hd?hdRight:stockRight;const int baseU=hd?place.place.x1/2:place.place.x1*2,baseV=hd?place.place.y1/2:place.place.y1*2;int matches=0;bool geometry=true;
    for(int i=0;i<grid*grid;++i){const auto& q=output.quads[i];const int cell=sprites[i],col=cell%grid,row=grid-1-cell/grid;matches+=int(NativeAtlasQuadMatches(q.texture,baseU+l[grid-1][col],baseV+l[grid-1][row],baseU+rr[grid-1][col],baseV+rr[grid-1][row]));geometry&=q.color==NGfx::SPixel8888(24,36,48,255).color && q.depth==4.f+i;}
    auto after=GlobalGameRandom();bool sameRNG=true;for(int i=0;i<4;++i)sameRNG&=random.Get()==after.Get();const auto& first=output.quads.front().texture;
    std::printf("native grass atlas margin mode=%d HD=%d atlas=%d grid=%d region=%d,%d,%d,%d quads=%zu matches=%d expected=%d firstUV=%d,%d,%d,%d hash=%08x geometry=%d RNG=%d owner=%d regionSame=%d\n",mode,int(hd),place.size.x,grid,place.place.x1,place.place.y1,place.place.x2,place.place.y2,output.quads.size(),matches,grid*grid,first.vUVs[0].nU,first.vUVs[0].nV,first.vUVs[2].nU,first.vUVs[2].nV,output.GeometryHash(),int(geometry),int(sameRNG),int(texture->pTB.GetPtr()==buffer && NGfx::HasSameContainer(texture,holder)),int(NativeAtlasSameRegion(texture->region,region)));std::fflush(stdout);
    Require(geometry && sameRNG && texture->pTB.GetPtr()==buffer && NativeAtlasSameRegion(texture->region,region) && effect->positions.size()==size_t(grid*grid) && effect->textures.front().GetPtr()==input.GetPtr() && !effect->bEnd,"grass UV update changed actual geometry/color/depth/RNG/cache/input identity");
    Require(matches==grid*grid,"actual native grass output retains unpadded cell edges");
}


static void NativeAICoverDepth(const char* db,const char* resources,int mode)
{
    Require(mode>=0 && mode<8,"unknown native AI cover depth case");NativeHearingOwners f(db,resources);
    auto* actor=f.humanUnit;auto* target=f.aiUnit;auto* map=f.world->GetAIMap();auto* game=f.world->GetGame();map->Sync();
    CVec3 aim;Require(map->GetUnitHLPos(&aim,map->GetHull(target),NAI::HL_ANY),"AI cover depth lacks actual target hull");
    const bool human=mode==3 || mode==6;const float distance=mode>=4 && mode<=6?40.f:8.f;
    const CVec3 origin=aim-CVec3(0,distance,0);float testDistance=mode==2?9.f:mode==4 || mode==6?41.f:mode==5?43.f:mode==7?29.f:20.f;
    const int expected=mode==2 || mode==4?1:human?2:0;
    CObj<NativeCoverFramingMap> probe=new NativeCoverFramingMap(map);probe->expectedDirection=CVec3(0,1,0);probe->farPoint=origin+CVec3(0,testDistance,0);
    struct Restore{NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}}restore{game,map};SetCoverProbeMap(game,probe);
    const auto position=actor->GetPosition();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();auto random=GlobalGameRandom();
    auto attack=NativeCoverFixedAttack(actor,target);CObj<NRPG::CCoverInfo> cover;float score=0;
    if(mode==1)score=game->GetCoverForAIUnitPrecise(origin,actor,target,attack,NAI::HL_ANY);
    else cover=game->CalcCovers(origin,attack,actor,target,NAI::HL_ANY,1.f,!human);
    auto after=GlobalGameRandom();bool rng=true;for(int i=0;i<4;++i)rng&=random.Get()==after.Get();
    std::printf("native AI cover depth mode=%d human=%d distance=%.9g point=%.9g grids=%d farInside=%d expected=%d helper=%d score=%.9g AP=%d afterAP=%d HP=%d afterHP=%d position=%d RNG=%d\n",mode,int(human),distance,testDistance,probe->grids,probe->farInside,expected,int(mode==1),score,ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP(),int(position==actor->GetPosition()),int(rng));std::fflush(stdout);
    Require(probe->grids==(human?2:1) && rng && ap==actor->GetAP() && hp==actor->GetUnitRPG()->GetTotalVP() && position==actor->GetPosition(),"AI projection query changed native grid/actor/RNG state");
    Require(probe->farInside==expected,"actual AI cover raster still extends to the unrelated30 bullet range");
}
static void NativePathWaveFlags(const char* db,const char* resources,int mode)
{
    Require(mode>=0 && mode<4,"unknown native path-wave flag case");NativeHearingOwners f(db,resources);auto* actor=f.humanUnit;auto* net=f.world->GetPathNetwork();
    const bool locked=mode==0 || mode==2;const int budget=mode>=2?0:16;actor->SetForceNoChangePose(locked);
    const auto position=actor->GetPosition();const int pose=position.pos.p.GetPose(),ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();auto random=GlobalGameRandom();
    NAI::CMultiMovesTable table;list<NAI::SPathPlace> paths;NWorld::PrepareAllPaths(net,&table,&paths,actor,position.pos.p,budget,actor,false,true);
    int same=0,other=0;for(auto p:paths){same+=p.GetPose()==pose;other+=p.GetPose()!=pose;}
    auto after=GlobalGameRandom();bool rng=true;for(int i=0;i<4;++i)rng&=random.Get()==after.Get();
    std::printf("native path wave flags mode=%d locked=%d big=%d budget=%d pose=%d same=%d other=%d paths=%zu AP=%d afterAP=%d HP=%d afterHP=%d position=%d RNG=%d\n",mode,int(locked),int(NAI::IsBigLocker(actor)),budget,pose,same,other,paths.size(),ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP(),int(position==actor->GetPosition()),int(rng));std::fflush(stdout);
    Require(same>0 && ap==actor->GetAP() && hp==actor->GetUnitRPG()->GetTotalVP() && position==actor->GetPosition() && rng,"native wave lost current pose or changed physical actor/RNG");
    // Actual Steam77df57..77df80 also forwards checkSuicide as big-unit,
    // pose-lock as suicide-check and default false as move-only. Preserve this
    // original gameplay defect: the donor wrapper correction is excluded.
    Require(budget==0?other==0:other>0,"native world path wrapper differs from the independently decoded original flag-shift control");
}
static void NativeAIFlankWave(const char* db,const char* resources,int mode)
{
    Require(mode>=0 && mode<4,"unknown native AI flank wave case");NativeHearingOwners f(db,resources);auto* actor=f.humanUnit;auto* target=f.aiUnit;auto* net=f.world->GetPathNetwork();
    vector<NAI::SPathPlace> nearby;net->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),60.f),&nearby);NAI::SPathPlace center,enemyPlace;bool found=false;int bestClear=0;
    for(auto p:nearby){const int x=p.GetX(),y=p.GetY();if(x<4 || x>251 || y<4 || y>245)continue;bool clear=true;
        int good=0;for(int dy=-1;dy<=1;++dy)for(int dx=-2;dx<=2;++dx){NAI::SPathPlace cell(x+dx,y+dy,p.GetLayer(),0,NAI::CM_STAND,0);if(!net->IsPassable(cell) || net->IsLocked(cell))clear=false;else ++good;}bestClear=Max(bestClear,good);
        if(!clear)continue;NAI::SPathPlace enemy(x,y+3,p.GetLayer(),0,NAI::CM_STAND,0);if(!net->IsPassable(enemy) || net->IsLocked(enemy))continue;
        center=NAI::SPathPlace(x,y,p.GetLayer(),0,NAI::CM_STAND,0);enemyPlace=enemy;found=true;break;}
    std::printf("native flank clearing nearby=%zu bestClear=%d found=%d\n",nearby.size(),bestClear,int(found));std::fflush(stdout);Require(found,"native flank fixture lacks an actual open5x3 navigation clearing and enemy cell");
    auto place=[&](NWorld::CUnitServer* unit,NAI::SPathPlace p){auto next=NAI::GetUnitPos(p,net);unit->animator.PlaceUnit(next);unit->SetPosition(next);unit->Update();};place(actor,center);place(target,enemyPlace);actor->SetForceNoChangePose(false);actor->SetWishPose(NAI::RUN);
    vector<CObj<NWorld::CUnitServer>> ring;
    if(mode!=1)for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){if(dx==0 && dy==0)continue;NAI::SPathPlace cell(center.GetX()+dx,center.GetY()+dy,center.GetLayer(),0,NAI::CM_STAND,0);
        CObj<NWorld::CUnitServer> ally=f.world->AddUnitInGame(cell,NRPG::CreateUnit(NDb::GetPers(54)),f.human,"NativeFlankTransientAlly");Require(ally && actor->GetDiplomacyState(ally)==NDb::DS_ALLY,"native flank ring lost actual live allies");ring.push_back(ally);}
    CObj<NAI::IAIUnit> unit=NAI::CreateAIUnit(actor,true),enemy=NAI::CreateAIUnit(target,false);unit->Synchronize(false);enemy->Synchronize(false);
    Require(unit->GetAIUnitState() && unit->GetUnitServer()==actor && enemy->GetUnitServer()==target,"flank fixture did not construct real native AI wrappers");unit->GetAIUnitState()->pEnemy=mode==2?nullptr:enemy.GetPtr();
    NAI::CMultiMovesTable respected,ignored;list<NAI::SPathPlace> withLocks,withoutLocks;
    NWorld::PrepareAllPaths(net,&respected,&withLocks,actor,center,8,actor,false,false);
    NWorld::PrepareAllPaths(net,&ignored,&withoutLocks,actor,center,8,actor,false,true);
    std::set<unsigned> blockedKeys;for(auto p:withLocks)blockedKeys.insert(NAI::UnitAreaPlaceKey(p));NAI::SPathPlace goal;bool haveGoal=false;
    for(auto p:withoutLocks){if(p.GetPose()!=NAI::CM_STAND || p.GetLayer()!=center.GetLayer() || p.GetY()!=center.GetY() || std::abs(int(p.GetX())-int(center.GetX()))<2)continue;
        if(mode!=1 && blockedKeys.count(NAI::UnitAreaPlaceKey(p)))continue;goal=p;haveGoal=true;break;}
    std::printf("native flank wave inputs wish=%d runCost=%d walkCost=%d respect=%zu ignore=%zu center=%u,%u,%u goal=%d\n",int(actor->GetWishPose()),actor->GetRPG()->GetActionAP(NAI::RUN,NRPG::AC_MOVE_SIDE),actor->GetRPG()->GetActionAP(NAI::WALK,NRPG::AC_MOVE_SIDE),withLocks.size(),withoutLocks.size(),unsigned(center.GetX()),unsigned(center.GetY()),unsigned(center.GetLayer()),int(haveGoal));std::fflush(stdout);Require(haveGoal,"native flank fixture has no unoccupied perpendicular place reachable past transient ally locks within8 wave AP");
    const auto original=actor->GetPosition();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();auto random=GlobalGameRandom();
    CObj<NAI::CAIAttackPlaceSource> source=new NAI::CAIAttackPlaceSource(unit,mode==3?0:14,nullptr,false);source->Prepare();vector<NAI::SPlaceWithAP> places;source->GetPlaces(&places);bool hasGoal=false;
    for(auto p:places)if(NAI::UnitAreaPlaceKey(p.place.pos.p)==NAI::UnitAreaPlaceKey(goal))hasGoal=true;
    auto after=GlobalGameRandom();bool rng=true;for(int i=0;i<4;++i)rng&=random.Get()==after.Get();const bool expected=mode==0 || mode==1;
    std::printf("native AI flank wave mode=%d ring=%zu respect=%zu ignore=%zu current=%08x goal=%08x goalRespect=%d plans=%zu goalPresent=%d expected=%d AP=%d afterAP=%d HP=%d afterHP=%d position=%d RNG=%d\n",mode,ring.size(),withLocks.size(),withoutLocks.size(),unsigned(center.GetBits()),unsigned(goal.GetBits()),int(blockedKeys.count(NAI::UnitAreaPlaceKey(goal))!=0),places.size(),int(hasGoal),int(expected),ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP(),int(original==actor->GetPosition()),int(rng));std::fflush(stdout);
    Require(ap==actor->GetAP() && hp==actor->GetUnitRPG()->GetTotalVP() && original==actor->GetPosition() && rng,"native flank planning changed physical actor or RNG");
    Require(hasGoal==expected,"actual native attack-place flank wave still applies suicide/dynamic unit-lock checks");
}

// Authored closed geometry enters the real renderer and cover classifier. The
// fixture supplies no depth intervals, hit/miss decisions, or cover scores.
class NativeAllyRayMap:public CoverOriginMapProbe
{
    OBJECT_NOCOPY_METHODS(NativeAllyRayMap);
    struct Volume
    {
        vector<CVec3> points;NAI::CEdgesInfo edges;SFBTransform transform;NAI::SSourceInfo surface;
        Volume(NWorld::CUnitServer* unit,const CVec3& lo,const CVec3& hi,int flags):surface(unit,unit->GetUnitRPG()->GetRPGArmor(),0,flags)
        {
            points={CVec3(lo.x,lo.y,lo.z),CVec3(hi.x,lo.y,lo.z),CVec3(hi.x,hi.y,lo.z),CVec3(lo.x,hi.y,lo.z),CVec3(lo.x,lo.y,hi.z),CVec3(hi.x,lo.y,hi.z),CVec3(hi.x,hi.y,hi.z),CVec3(lo.x,hi.y,hi.z)};
            vector<STriangle> faces={STriangle(0,2,1),STriangle(0,3,2),STriangle(4,5,6),STriangle(4,6,7),STriangle(0,1,5),STriangle(0,5,4),STriangle(3,7,6),STriangle(3,6,2),STriangle(0,4,7),STriangle(0,7,3),STriangle(1,2,6),STriangle(1,6,5)};
            edges.GenerateEdgeList(faces,points);Require(edges.IsClosed(),"ally-ray authored input is not a closed native hull");Identity(&transform.forward);Identity(&transform.backward);
        }
    };
    vector<std::unique_ptr<Volume>> volumes;
public:
    explicit NativeAllyRayMap(NAI::IAIMap* map=nullptr):CoverOriginMapProbe(map){}
    void Box(NWorld::CUnitServer* unit,const CVec3& lo,const CVec3& hi,int flags=NWorld::TS_COVER){volumes.emplace_back(new Volume(unit,lo,hi,flags));}
    void TraceGrid(NAI::CFastRenderer* grid,int,ESort,const NAI::CFloorsSet&,ESplitTerrainHGroups,bool) override
    {
        for(auto& v:volumes){NAI::SConvexHull hull(v->points,v->edges,v->transform,v->surface,NAI::HL_BODY,{});grid->TraceEntity(hull,false);}grid->SortIntervals();
    }
};
static void NativeAllyRayClassification(const char* db,const char* resources,int mode)
{
    Require(mode>=0 && mode<6,"unknown native ally-ray classification mode");NativeHearingOwners f(db,resources);auto* actor=f.humanUnit;auto* target=f.aiUnit;
    CObj<NWorld::CUnitServer> other=f.world->AddUnitInGame(f.FreePlace(actor->GetPosition().GetCP(),10.f),NRPG::CreateUnit(NDb::GetPers(54)),mode==2?f.ai:f.human,"NativeRayOther");Require(other,"ally-ray cannot create its real ownership input");
    auto* game=f.world->GetGame();auto* map=f.world->GetAIMap();map->Sync();CVec3 aim;Require(map->GetUnitHLPos(&aim,map->GetHull(target),NAI::HL_ANY),"ally-ray target lacks native hull locator");const CVec3 origin=aim-CVec3(0,8,0);
    CObj<NativeAllyRayMap> probe=new NativeAllyRayMap(map);probe->Box(target,origin+CVec3(-2,7.7f,-2),origin+CVec3(2,8.2f,2));
    if(mode!=1){const float depth=mode==3?9.f:mode==4?.25f:3.f;probe->Box(other,origin+CVec3(-2,depth,-2),origin+CVec3(2,depth+.1f,2));}
    struct Restore{NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}}restore{game,map};SetCoverProbeMap(game,probe);
    const auto position=actor->GetPosition();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),otherHP=other->GetUnitRPG()->GetTotalVP();auto random=GlobalGameRandom();
    auto attack=NativeCoverFixedAttack(actor,target);CObj<NRPG::CCoverInfo> cover=game->CalcCovers(origin,attack,actor,target,NAI::HL_BODY,1.f,mode!=5);const auto snapshot=CoverSnapshot(cover);int penetrated=0;for(auto ray:snapshot.hits)penetrated+=int(ray.penetrate);
    auto after=GlobalGameRandom();bool rng=true;for(int i=0;i<4;++i)rng&=random.Get()==after.Get();const bool expectedHit=mode!=0 && mode!=5;
    std::printf("native ally-ray classification mode=%d diplomacy=%d hits=%zu loose=%zu obstacles=%zu penetrate=%d expectedHit=%d AP=%d afterAP=%d HP=%d afterHP=%d otherHP=%d afterOtherHP=%d position=%d RNG=%d\n",mode,int(actor->GetDiplomacyState(other)),snapshot.hits.size(),snapshot.loose.size(),snapshot.obstacles.size(),penetrated,int(expectedHit),ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP(),otherHP,other->GetUnitRPG()->GetTotalVP(),int(position==actor->GetPosition()),int(rng));std::fflush(stdout);
    Require(ap==actor->GetAP() && hp==actor->GetUnitRPG()->GetTotalVP() && otherHP==other->GetUnitRPG()->GetTotalVP() && position==actor->GetPosition() && rng,"ally-ray classification mutated native actor/target/RNG");
    Require(snapshot.obstacles.empty() && (expectedHit?(!snapshot.hits.empty() && penetrated>0):(snapshot.hits.empty() && !snapshot.loose.empty())),"actual native ally blocker is still classified as a blocked hit rather than a loose ray");
}


class NativeAllySafetyAccess:public NAI::CAIActionPlaceSource
{
    OBJECT_NOCOPY_METHODS(NativeAllySafetyAccess);
public:
    explicit NativeAllySafetyAccess(NAI::IAIUnit* unit=nullptr):NAI::CAIActionPlaceSource(unit){}
    void Prepare() override {}
    bool Dangerous(const NAI::SUnitPosition& pos,NAI::CAIFireArmsWeapon* weapon){return IsPosDangerousForAllies(pos,weapon);}
};
static void NativeAllyRasterSafety(const char* db,const char* resources,int mode)
{
    Require(mode>=0 && mode<9,"unknown native ally raster safety mode");NativeHearingOwners f(db,resources);auto* actor=f.aiUnit;auto* target=f.humanUnit;auto* net=f.world->GetPathNetwork();
    auto at=actor->GetPosition();at.pos.p.SetPose(NAI::CM_STAND);actor->animator.PlaceUnit(at);actor->SetPosition(at);actor->Update();
    auto* commander=dynamic_cast<NAI::CAICommander*>(f.ai->GetCommander());Require(commander,"ally raster safety lacks actual AI commander");commander->OnUnitAdded(actor);CObj<NAI::IAIUnit> unit=commander->GetAIUnit(actor),enemy=NAI::CreateAIUnit(target,false);Require(unit && enemy,"ally raster safety lacks native factory wrappers");unit->Synchronize(false);enemy->Synchronize(false);unit->SetAIState(commander->GetAIState());unit->GetAIUnitState()->pEnemy=mode==4?nullptr:enemy.GetPtr();if(mode==5 || mode==6)unit->SetAIState(nullptr);
    auto* weapon=unit->GetAIInventory()->GetFirstFireArms();Require(weapon && weapon->GetItem(),"ally raster safety lacks actual authored firearm");
    const CVec3 from=actor->GetPosition().GetCP(),to=target->GetPosition().GetCP(),shot=to-from;const float lengthSq=shot*shot;Require(lengthSq>16.f,"ally raster safety needs separated actual actors");
    vector<NAI::SPathPlace> nearby;net->GetNearPlaces(SSphere(from,40.f),&nearby);NAI::SPathPlace place;bool found=false;
    for(auto p:nearby){p.SetPose(NAI::CM_STAND);p.SetMoving(0);p.SetDirection(0);if(!net->IsPassable(p) || net->IsLocked(p))continue;const auto pos=NAI::GetUnitPos(p,net);const CVec3 rel=pos.GetCP()-from;const float t=(rel*shot)/lengthSq;
        if(fabs(rel)>3.f && (t<=0 || t>=1 || fabs(rel-shot*t)>2.f)){place=p;found=true;break;}}
    Require(found,"ally raster safety cannot put actual unit outside the old centreline corridor");CObj<NWorld::CUnitServer> other=f.world->AddUnitInGame(place,NRPG::CreateUnit(NDb::GetPers(54)),mode==2?f.human:f.ai,"NativeAllyRasterSafety");Require(other,"ally raster safety cannot add real ownership input");
    if(mode!=2){commander->OnUnitAdded(other);Require(NAI::GetAIUnit(other),"ally raster safety's actual state query cannot resolve live ally");}
    auto* map=f.world->GetAIMap();auto* game=f.world->GetGame();map->Sync();CVec3 aim;Require(map->GetUnitHLPos(&aim,map->GetHull(target),NAI::HL_ANY),"ally raster safety has no actual whole-hull aim");const CVec3 origin=actor->GetAttackOrigin(actor->GetPosition(),false);CVec3 axis=aim-origin;const float distance=fabs(axis);Require(distance>4.f,"ally raster safety has no usable native weapon-origin axis");Normalize(&axis);
    CObj<NativeAllyRayMap> probe=new NativeAllyRayMap(map);probe->Box(target,aim-CVec3(2,2,2),aim+CVec3(2,2,2));
    if(mode!=1){CVec3 center=origin+axis*(mode==8?distance+1.f:distance*.5f);if(mode==3){CVec3 side(axis.y,-axis.x,0);Normalize(&side);center+=side*20.f;}probe->Box(other,center-CVec3(.6f,.6f,.6f),center+CVec3(.6f,.6f,.6f));}
    struct Restore{NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}}restore{game,map};SetCoverProbeMap(game,probe);
    const auto position=actor->GetPosition();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),targetHP=target->GetUnitRPG()->GetTotalVP(),otherHP=other->GetUnitRPG()->GetTotalVP(),ammo=weapon->GetItem()->GetAmmoQuantity();auto random=GlobalGameRandom();
    CObj<NativeAllySafetyAccess> access=new NativeAllySafetyAccess(unit);const bool danger=access->Dangerous(position,mode==6 || mode==7?nullptr:weapon);// Actual Steam48f769..48faaa: missing enemy returns false; invalid state
    // returns true through48fac8..48fae6 before checking a null weapon.
    const bool expected=mode==0 || mode==5 || mode==6 || mode==8;
    bool consumer=true;int plans=-1,current=-1;if(mode<4 || mode==8){CObj<NAI::CAIAttackPlaceSource> source=new NAI::CAIAttackPlaceSource(unit,0,nullptr,true);source->Prepare();vector<NAI::SPlaceWithAP> places;source->GetPlaces(&places);plans=int(places.size());current=0;for(auto p:places)current+=NAI::UnitAreaPlaceKey(p.place.pos.p)==NAI::UnitAreaPlaceKey(position.pos.p);consumer=bool(current)==!expected;}
    auto after=GlobalGameRandom();bool rng=true;for(int i=0;i<4;++i)rng&=random.Get()==after.Get();
    std::printf("native ally raster safety mode=%d distance=%.9g diplomacy=%d danger=%d expected=%d plans=%d current=%d consumer=%d AP=%d afterAP=%d HP=%d afterHP=%d targetHP=%d afterTargetHP=%d otherHP=%d afterOtherHP=%d ammo=%d afterAmmo=%d position=%d RNG=%d\n",mode,distance,int(actor->GetDiplomacyState(other)),int(danger),int(expected),plans,current,int(consumer),ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP(),targetHP,target->GetUnitRPG()->GetTotalVP(),otherHP,other->GetUnitRPG()->GetTotalVP(),ammo,weapon->GetItem()->GetAmmoQuantity(),int(position==actor->GetPosition()),int(rng));std::fflush(stdout);
    Require(ap==actor->GetAP() && hp==actor->GetUnitRPG()->GetTotalVP() && targetHP==target->GetUnitRPG()->GetTotalVP() && otherHP==other->GetUnitRPG()->GetTotalVP() && ammo==weapon->GetItem()->GetAmmoQuantity() && position==actor->GetPosition() && rng,"native ally safety query changed physical unit/ammo/RNG state");
    Require(danger==expected && consumer,"actual native AI safety still uses a centreline corridor or checks weapon validity before its actor/enemy/state");
}


static void NativeAllyCoverCache(const char* db,const char* resources,int mode)
{
    Require(mode==0 || mode==1,"unknown native ally cover-cache case");NativeHearingOwners f(db,resources);auto* actor=f.humanUnit;auto* target=f.aiUnit;
    CObj<NWorld::CUnitServer> other=f.world->AddUnitInGame(f.FreePlace(actor->GetPosition().GetCP(),10.f),NRPG::CreateUnit(NDb::GetPers(54)),f.human,"NativeCoverCacheOther");Require(other,"cover cache cannot create actual ally");auto* game=f.world->GetGame();auto* map=f.world->GetAIMap();map->Sync();CVec3 aim;Require(map->GetUnitHLPos(&aim,map->GetHull(target),NAI::HL_ANY),"cover cache lacks actual target locator");const CVec3 origin=aim-CVec3(0,8,0);
    CObj<NativeAllyRayMap> probe=new NativeAllyRayMap(map);probe->Box(target,origin+CVec3(-2,7.7f,-2),origin+CVec3(2,8.2f,2));const float depth=mode==0?3.f:.2f;probe->Box(other,origin+CVec3(-2,depth,-2),origin+CVec3(2,depth+.1f,2),mode==0?NWorld::TS_COVER:NWorld::TS_WEAPON_BLOCKER);
    struct Restore{NRPG::IGame* game;CPtr<NAI::IAIMap> map;~Restore(){SetCoverProbeMap(game,map);}}restore{game,map};SetCoverProbeMap(game,probe);auto random=GlobalGameRandom();const auto position=actor->GetPosition();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();auto attack=NativeCoverFixedAttack(actor,target);
    CObj<NRPG::CCoverInfo> cover=game->CalcCovers(origin,attack,actor,target,NAI::HL_BODY,1.f,true);const auto before=CoverSnapshot(cover);unordered_map<CPtr<CObjectBase>,int,SPtrHash> objects;objects[CastToObjectBase(actor)]=99;NRPG::GetObjectsThatMayBeDamaged(cover,&objects);
    const bool correct=mode==0?objects.size()==2 && objects.count(CastToObjectBase(target)) && objects.count(CastToObjectBase(other.GetPtr())):objects.empty();const auto initialObjects=objects.size();Require(correct && !objects.count(CastToObjectBase(actor)),"live collector lost full hit/loose-cell objects, failed to clear prior keys, or included obstacle-only rays");
    Require(mode==0?before.hits.empty() && !before.loose.empty() && before.obstacles.empty():before.hits.empty() && before.loose.empty() && !before.obstacles.empty(),"collector fixture did not separate loose and obstacle-only native cells");
    CMemoryStream saved;{CStructureSaver writer(saved,CStructureSaver::WRITE);cover.GetBarePtr()->operator&(writer);}saved.Seek(0);{CStructureSaver reader(saved,CStructureSaver::READ);cover.GetBarePtr()->operator&(reader);}const auto afterLoad=CoverSnapshot(cover);objects[CastToObjectBase(actor)]=99;NRPG::GetObjectsThatMayBeDamaged(cover,&objects);const auto restoredObjects=objects.size();Require(objects.empty(),"cover load retained stale transient object identities");
    CMemoryStream again;{CStructureSaver writer(again,CStructureSaver::WRITE);cover.GetBarePtr()->operator&(writer);}const bool sameBytes=saved.GetSize()==again.GetSize() && std::memcmp(saved.GetBuffer(),again.GetBuffer(),saved.GetSize())==0;
    const bool sameFields=before.source==afterLoad.source && before.piercing==afterLoad.piercing && SameCoverRays(before.hits,afterLoad.hits) && SameCoverRays(before.loose,afterLoad.loose) && SameCoverRays(before.obstacles,afterLoad.obstacles);
    CObj<NRPG::CCoverInfo> fresh=game->CalcCovers(origin,attack,actor,target,NAI::HL_BODY,1.f,true);NRPG::GetObjectsThatMayBeDamaged(fresh,&objects);Require(objects.size()==initialObjects,"fresh native recalculation did not rebuild the transient union");auto after=GlobalGameRandom();bool rng=true;for(int i=0;i<4;++i)rng&=random.Get()==after.Get();
    std::printf("native ally cover cache mode=%d initialObjects=%zu restoredObjects=%zu rebuiltObjects=%zu hits=%zu loose=%zu obstacles=%zu bytes=%d sameBytes=%d sameFields=%d AP=%d afterAP=%d HP=%d afterHP=%d position=%d RNG=%d\n",mode,initialObjects,restoredObjects,objects.size(),before.hits.size(),before.loose.size(),before.obstacles.size(),int(saved.GetSize()),int(sameBytes),int(sameFields),ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP(),int(position==actor->GetPosition()),int(rng));std::fflush(stdout);
    Require(sameBytes && sameFields && ap==actor->GetAP() && hp==actor->GetUnitRPG()->GetTotalVP() && position==actor->GetPosition() && rng,"transient collector changed legacy saved cover fields/bytes or native actor/RNG state");
}


namespace NativeShadowDepthInput
{
using namespace NGScene;
class MeshInput:public CPtrFuncBase<CObjectInfo>{OBJECT_NOCOPY_METHODS(MeshInput);protected:bool NeedUpdate()override{return false;}void Recalc()override{}public:MeshInput(){}explicit MeshInput(CObjectInfo* m){pValue=m;}};
class MeshPart:public IPart{OBJECT_NOCOPY_METHODS(MeshPart);public:MeshPart(){}explicit MeshPart(MeshInput* m):IPart(m,nullptr,false){auto* mesh=m->GetValue();fAverageTriArea=mesh->GetAverageTriArea();const auto& p=mesh->GetPositions();vBVMin=vBVMax=p.front();for(const auto& v:p){vBVMin.x=Min(vBVMin.x,v.x);vBVMin.y=Min(vBVMin.y,v.y);vBVMin.z=Min(vBVMin.z,v.z);vBVMax.x=Max(vBVMax.x,v.x);vBVMax.y=Max(vBVMax.y,v.y);vBVMax.z=Max(vBVMax.z,v.z);}}};
class BoundsInput:public IVBCombiner{OBJECT_NOCOPY_METHODS(BoundsInput);protected:bool NeedUpdate()override{return false;}void Recalc()override{}public:void Add(float depth,float halfX=-1,float halfY=-1){partBVs.push_back(SSphere(CVec3(0,0,depth),depth*3));bound.BoxInit(CVec3(-depth*3,-depth*3,-depth*2),CVec3(depth*3,depth*3,depth*4));}void AddBound(const SSphere& sphere){partBVs.push_back(sphere);}};
struct PartsInput:IRender{
 vector<CObj<CObjectInfo>> meshes;vector<CObj<MeshInput>> meshInputs;vector<CObj<MeshPart>> owned;vector<CPtr<IPart>> parts;CObj<BoundsInput> bounds=new BoundsInput;SRenderGeometryInfo geometry;bool shadows=true;int calls=0;
 void Add(float depth,float halfX=-1,float halfY=-1){CObjectInfo::SData data;data.verts.resize(4);const float hx=halfX<0?depth*2:halfX,hy=halfY<0?depth*2:halfY;CVec3 p[]={CVec3(-hx,-hy,depth),CVec3(hx,-hy,depth),CVec3(hx,hy,depth),CVec3(-hx,hy,depth)};for(int i=0;i<4;++i){Zero(data.verts[i]);data.verts[i].pos=p[i];}vector<STriangle> tris={STriangle(0,1,2),STriangle(0,2,3),STriangle(0,2,1),STriangle(0,3,2)};data.geometry.SetTriangles(tris);CObj<CObjectInfo> mesh=new CObjectInfo;mesh->Assign(data,false);CObj<MeshInput> input=new MeshInput(mesh);CObj<MeshPart> part=new MeshPart(input);meshes.push_back(mesh);meshInputs.push_back(input);owned.push_back(part);parts.push_back(part.GetPtr());bounds->Add(depth);}
 void AddSloped(){CObjectInfo::SData data;data.verts.resize(4);CVec3 p[4];const float u[]={-2,2,2,-2},v[]={-2,-2,2,2};for(int i=0;i<4;++i){const float z=1.f/(.25f+.0103f*u[i]+.0057f*v[i]);p[i]=CVec3(u[i]*z,v[i]*z,z);}for(int i=0;i<4;++i){Zero(data.verts[i]);data.verts[i].pos=p[i];}vector<STriangle> tris={STriangle(0,1,2),STriangle(0,2,3),STriangle(0,2,1),STriangle(0,3,2)};data.geometry.SetTriangles(tris);CObj<CObjectInfo> mesh=new CObjectInfo;mesh->Assign(data,false);CObj<MeshInput> input=new MeshInput(mesh);CObj<MeshPart> part=new MeshPart(input);meshes.push_back(mesh);meshInputs.push_back(input);owned.push_back(part);parts.push_back(part.GetPtr());bounds->AddBound(SSphere(CVec3(0,0,4),20));}
 void FormPartList(CTransformStack*,list<SRenderPartSet>* out,EDepthType,const SGroupSelect&)override{++calls;geometry.pVertices=bounds.GetPtr();geometry.pVertices.Refresh();SRenderPartSet set(meshInputs.front().GetPtr(),&parts,&geometry,0);set.parts.Clear();set.castShadow.Clear();for(int i=0;i<int(parts.size());++i){set.parts.Set(i);if(shadows)set.castShadow.Set(i);}out->push_back(set);}
 void FormDepthList(CTransformStack*,const CVec3&,CSceneFragments*,EDepthType)override{}
 void FormDirOccludersList(CTransformStack*,const CVec3&,CSceneFragments*,const SGroupSelect&,bool)override{}
};
}

static void NativeFloorCulling(int mode)
{
 using namespace NGScene;using namespace NativeShadowDepthInput;
 Require(mode>=0 && mode<5,"unknown floor-culling mode");
 CTransformStack transform;transform.MakeProjective(CVec2(128,128),90,.01f,100);SHMatrix camera;MakeMatrix(&camera,VNULL3,CVec3(0,0,1));transform.SetCamera(camera);
 CIgnorePartsHash ignored;CObj<IHZBuffer> hz;
 if(mode<2){
  struct TwoFloors:IRender {
   PartsInput parts[2];int floor[2]={0,0};
   void FormPartList(CTransformStack* ts,list<SRenderPartSet>* out,EDepthType type,const SGroupSelect& mask)override{for(int i=0;i<2;++i){list<SRenderPartSet> one;parts[i].FormPartList(ts,&one,type,mask);one.front().nFloorMask=floor[i];out->splice(out->end(),one);}}
   void FormDepthList(CTransformStack*,const CVec3&,CSceneFragments*,EDepthType)override{}
   void FormDirOccludersList(CTransformStack*,const CVec3&,CSceneFragments*,const SGroupSelect&,bool)override{}
  }input;
  input.parts[0].Add(4.00004f);input.parts[1].Add(4.00002f);input.floor[0]=mode==1?2:1;input.floor[1]=1;
  ++nDGCurrentFrame;MakeInvisibleElementsList(&input,&transform,MakeSelectAll(),CVec2(128,128),&ignored,&hz);
  auto hidden=[&](int i){auto f=ignored.find(input.parts[i].meshInputs.front().GetPtr());return f!=ignored.end() && f->second.IsSet(0);};
  std::printf("NATIVE-FLOOR-CULL mode=%d farHidden=%d nearHidden=%d farFloor=%d nearFloor=%d\n",mode,int(hidden(0)),int(hidden(1)),input.floor[0],input.floor[1]);std::fflush(stdout);
  Require(mode==0?hidden(0) && !hidden(1):!hidden(0) && hidden(1),"actual native equal-word-depth raster lost front-to-back within floor or descending floor priority");
 }else{
  PartsInput input;input.Add(4.f,mode==2?.1f:.4f,mode==3?.1f:mode==2?.1f:.4f);
  const float area=input.meshes.front()->GetAverageTriArea();Require(fabs(area-(mode==2?.02f:mode==3?.08f:.32f))<.0001f,"object average triangle area not calculated from actual geometry");
  ++nDGCurrentFrame;if(mode==2)MakeInvisibleElementsList(&input,&transform,MakeSelectAll(),CVec2(128,128),&ignored,&hz);else MakeInvisibleElementsListFast(&input,&transform,MakeSelectAll(),CVec2(128,128),&ignored,&hz);
  const auto found=ignored.find(input.meshInputs.front().GetPtr());bool selfHidden=found!=ignored.end() && found->second.IsSet(0);const bool behind=hz->IsVisible(SSphere(CVec3(0,0,8),.0001f),&transform);
  std::printf("NATIVE-FLOOR-CULL mode=%d area=%.6f selfHidden=%d behindVisible=%d\n",mode,area,int(selfHidden),int(behind));std::fflush(stdout);
  Require(!selfHidden && behind==(mode!=4),"small native geometry must stay visible without occluding; large control must actually occlude");
 }
}

static void NativeIntegerShadowDepth(int mode)
{
 using namespace NGScene;using namespace NativeShadowDepthInput;Require(mode>=0 && mode<19,"unknown native integer shadow depth case");
 auto random=GlobalGameRandom();PartsInput input;float a=4.00004f,b=4.00002f;
 if(mode==2 || mode==12){a=4.00002f;b=4.00004f;}if(mode==3){a=4.1f;b=4.f;}if(mode==4 || mode==5){a=.4f;b=.5f;}if(mode==6)a=.2f;
 if(mode==8){a=40.0002f;b=40.0001f;}if(mode==9){a=4.0009f;b=4.0005f;}if(mode==13){a=5;b=4;}
 if(mode>=17){a=4.f;b=0.f;input.AddSloped();}else{input.Add(a);if(mode!=6 && mode!=10)input.Add(b);}input.shadows=mode!=7;
 vector<vector<CVec3>> originalPositions;for(auto mesh:input.meshes)originalPositions.push_back(mesh->GetPositions());
 const CVec2 screen=mode==14?CVec2(31,27):CVec2(64,64);CTransformStack transform;transform.MakeProjective(screen,90,.01f,1000);SHMatrix camera;MakeMatrix(&camera,CVec3(0,0,0),CVec3(0,0,1));transform.SetCamera(camera);
 CIgnorePartsHash ignored;CObj<IHZBuffer> hz;++nDGCurrentFrame;const bool generate=(mode>=11 && mode<=13)||mode==16;
 int mask=0,second=-1,persistent=-1;bool visible=false,cover=false;
 auto hidden=[&](){int result=0;auto found=ignored.find(input.meshInputs.front().GetPtr());if(found!=ignored.end())for(int i=0;i<int(input.parts.size());++i)if(found->second.IsSet(i))result|=1<<i;return result;};
 if(generate){list<SRenderPartSet> result;GeneratePartList(&input,CVec3(0,0,0),mode==16?3.f:8.f,&result,IRender::DT_STATIC,MakeSelectAll());int shown=0;for(auto set:result)if(set.pNode==input.meshInputs.front().GetPtr())for(int i=0;i<int(input.parts.size());++i)if(set.parts.IsSet(i))shown|=1<<i;mask=((1<<input.parts.size())-1)&~shown;}
 else{if(mode==1 || mode==5 || mode==15)MakeInvisibleElementsListFast(&input,&transform,MakeSelectAll(),screen,&ignored,&hz);else MakeInvisibleElementsList(&input,&transform,MakeSelectAll(),screen,&ignored,&hz);mask=hidden();SSphere query(CVec3(0,0,mode==17?2.f:mode==18?10.f:mode==10?4.00002f:.5f),mode==10?.0000001f:.01f);CTRect<float> rect;cover=transform.GetCoverRect(&rect,query.ptCenter,query.fRadius);visible=hz->IsVisible(query,&transform);
 if(mode==15){CObj<IHZBuffer> before=hz;std::swap(input.parts[0],input.parts[1]);ignored.clear();MakeInvisibleElementsListFast(&input,&transform,MakeSelectAll(),screen,&ignored,&hz);second=hidden();persistent=before.GetPtr()==hz.GetPtr();}}

 const int expectedHidden[]={2,2,2,1,1,1,0,0,2,1,0,2,2,1,2,2,3,0,0};
 const int expectedQuery[]={1,1,1,1,1,1,1,1,1,1,0,-1,-1,-1,1,1,-1,1,0};
 bool geometry=true,identity=true;for(size_t i=0;i<input.meshes.size();++i){const auto& positions=input.meshes[i]->GetPositions();geometry&=positions.size()==originalPositions[i].size();for(size_t j=0;j<positions.size();++j)geometry&=positions[j]==originalPositions[i][j];identity&=input.owned[i]->GetObjectInfoNode()==input.meshInputs[i].GetPtr();}
 auto after=GlobalGameRandom();bool rng=true;for(int i=0;i<4;++i)rng&=random.Get()==after.Get();const int query=generate?-1:int(visible),calls=mode==15?2:generate?6:1;
 std::printf("native integer shadow depth mode=%d sloped=%d z=%.9g/%.9g hidden=%d expectedHidden=%d second=%d persistent=%d calls=%d expectedCalls=%d generate=%d hz=%d cover=%d query=%d expectedQuery=%d geometry=%d identity=%d RNG=%d\n",mode,int(mode>=17),a,b,mask,expectedHidden[mode],second,persistent,input.calls,calls,int(generate),int(hz!=nullptr),int(cover),query,expectedQuery[mode],int(geometry),int(identity),int(rng));std::fflush(stdout);
 Require(geometry && identity && rng && input.calls==calls && (generate || (hz && cover)) && (mode!=15 || (persistent==1 && second==2)),"native shadow input geometry/source/RNG/frustum/reuse control failed");
 Require(mask==expectedHidden[mode] && query==expectedQuery[mode],"actual native occlusion still uses float depth instead of original word-depth/iterator semantics");
}


static void NativeWoundedAIScared(const char* db,const char* resources,int mode)
{
    Require(mode>=0 && mode<18,"unknown native wounded AI mode");NativeHearingOwners f(db,resources);
    auto* actor=f.aiUnit;auto* commander=dynamic_cast<NAI::CAICommander*>(f.ai->GetCommander());Require(commander,"wounded AI lacks real commander");commander->OnUnitAdded(actor);
    CObj<NAI::IAIUnit> owner=commander->GetAIUnit(actor);Require(owner,"wounded AI lacks real owner");owner->SetAIState(commander->GetAIState());
    list<CPtr<NWorld::CUnitServer>> all;f.world->GetAllUnits(&all);
    for(auto p:all)if(IsValid(p) && p.GetPtr()!=actor && p.GetPtr()!=f.humanUnit && p->CanFight())p->MakeUnconscious(VNULL3,true,false);
    vector<CObj<NWorld::CUnitServer>> extra;vector<CObj<NAI::IAIUnit>> contacts;
    contacts.push_back(NAI::CreateAIUnit(f.humanUnit,false));Require(contacts.front(),"wounded AI lacks live enemy wrapper");
    const int count=mode==3?3:mode==4?2:(mode>=6 && mode<=13) || mode==17?9:1;
    for(int i=1;i<count;++i){auto* p=f.world->AddUnitInGame(f.FreePlace(f.humanUnit->GetPosition().GetCP(),30.f),NRPG::CreateUnit(NDb::GetPers(54)),f.human,"NativeFearContact");Require(p,"wounded AI cannot deploy native enemy");extra.push_back(p);contacts.push_back(NAI::CreateAIUnit(p,false));Require(contacts.back(),"wounded AI cannot wrap native enemy");}
    CObj<NAI::CAICommander> foreignCommander;NWorld::CPlayer* supportPlayer=f.ai;CObj<NWorld::CUnitServer> support;CObj<NWorld::CUnitServer> shell;
    const bool neutral=mode==6 || mode==7;
    if(neutral){f.game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));foreignCommander=new NAI::CAICommander(f.world,nullptr);supportPlayer=dynamic_cast<NWorld::CPlayer*>(f.world->AddPlayer(L"Native fear neutral support",f.game->players.back(),foreignCommander));Require(supportPlayer,"wounded AI cannot deploy neutral owner");foreignCommander->SetPlayer(supportPlayer);const vector<CPtr<NRPG::IUnitMission>> noUnits;f.world->GetDiplomacy()->SetDiplomacyState(f.ai->GetScenarioPlayerID(),noUnits,supportPlayer->GetScenarioPlayerID(),NDb::DS_NEUTRAL);f.world->GetAllUnits(&all);for(auto p:all)if(IsValid(p) && p->GetPlayer()==supportPlayer && p->CanFight()){support=p;break;}}
    if(mode>=5 && mode<=12){if(mode==9)supportPlayer=f.human;if(!support)support=f.world->AddUnitInGame(f.FreePlace(actor->GetPosition().GetCP(),20.f),NRPG::CreateUnit(NDb::GetPers(54)),supportPlayer,"NativeFearSupport");Require(support,"wounded AI cannot deploy actual support");
        auto* net=f.world->GetPathNetwork();vector<NAI::SPathPlace> places;net->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),12.f),&places);bool found=false;const CVec3 center=actor->GetPosition().GetCP();
        for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetDirection(0);p.SetMoving(0);if(!net->IsPassable(p) || net->IsLocked(p))continue;auto pos=support->GetPosition();pos.pos=NAI::SPosition(p,net);const float distance=fabs(pos.GetCP()-center);const bool eligible=mode==11?distance>=9.f && distance<10.f:mode==12?distance>8.f && distance<9.f:distance>2.f && distance<6.f;if(!eligible)continue;support->SetPosition(pos);found=true;break;}
        Require(found,"wounded AI cannot realize its intended actual nine-unit support boundary");if(mode==10)support->MakeUnconscious(VNULL3,true,false);
    }
    if(support){auto* supportCommander=dynamic_cast<NAI::CAICommander*>(supportPlayer->GetCommander());Require(supportCommander,"wounded AI support lacks real commander");supportCommander->OnUnitAdded(support);Require(NAI::GetAIUnit(support),"wounded AI support is absent from actual native registry");}
    if(mode==17){NDb::CRPGPers* persona=nullptr;CDBIterator<NDb::CRPGPers> people(*NDatabase::GetTable<NDb::CRPGPers>());while(people.MoveNext()){auto* p=people.Get();if(p->pPanzerklein && !p->pPanzerklein->bHasNoHead && p->pModel){persona=p;break;}}Require(persona,"wounded AI lacks authored suit");auto* net=f.world->GetPathNetwork();vector<NAI::SPathPlace> places;net->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),20.f),&places);for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetDirection(0);if(!NAI::IsBigLockerPlacePassable(net,p))continue;shell=f.world->AddUnitInGame(p,NRPG::CreateUnit(persona),nullptr);if(shell && shell->IsEmptyPK() && shell->WearAsPK(true)){actor->SetPosition(shell->GetPosition());actor->FlipPanzerklein(shell,false,false);break;}}Require(actor->IsWearingPK(),"wounded AI did not establish real pilot/suit relation");}
    auto& vp=actor->GetUnitRPG()->GetRPGUnit()->Skills(NDb::ST_VP);vp.SetConst(100);vp.SetValue(mode==1?50:mode==2?51:mode==0 || mode==5 || mode>=14?49:100);actor->GetUnitRPG()->GetRPGUnit()->nHealedVP=0;
    owner->Synchronize(false);for(auto p:contacts)p->Synchronize(false);if(mode==13)contacts.front()->SetHP(0,contacts.front()->GetMaxHP());
    auto* state=owner->GetAIUnitState();*state=NAI::SAIUnitState();state->SetUnit(owner);for(auto p:contacts)state->AddEnemy(p);state->pEnemy=mode==16?nullptr:contacts.front().GetPtr();state->enemies.bModified=state->possibleEnemies.bModified=state->allies.bModified=false;state->ClearModified();state->bHelpCalled=mode==14;state->bScared=mode==15;
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),planAP=owner->GetAP(),planHP=owner->GetHP();const auto position=actor->GetPosition();const auto contact=state->pEnemy;const auto enemies=state->enemies.data.units;auto random=GlobalGameRandom();
    state->Update();const bool expected=mode==0 || mode==3 || mode==9 || mode==10 || mode==11 || mode==13 || mode==15;const bool expectedModified=expected && mode!=15;auto after=GlobalGameRandom();bool sameRNG=true;for(int i=0;i<4;++i)sameRNG&=random.Get()==after.Get();
    std::printf("native wounded AI mode=%d VP=%d max=%d enemies=%zu support=%d distance=%.9g supportFight=%d PK=%d planEnemyHP=%d scared=%d expected=%d modified=%d expectedModified=%d RNG=%d\n",mode,int(vp),vp.GetMaxValue(),enemies.size(),int(bool(support)),support?fabs(support->GetPosition().GetCP()-position.GetCP()):-1.f,support?int(support->CanFight()):-1,int(actor->IsWearingPK()),contacts.front()->GetHP(),int(state->bScared),int(expected),int(state->IsModified()),int(expectedModified),int(sameRNG));std::fflush(stdout);
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && owner->GetAP()==planAP && owner->GetHP()==planHP && actor->GetPosition()==position && state->pEnemy==contact && state->enemies.data.units==enemies && sameRNG,"native fear check changed actual/planned health, AP, position, contacts or RNG");
    Require(state->bScared==expected && state->IsModified()==expectedModified,"native threat update has wrong wounded/enemy-count/support/PK fear policy");
}

static void NativeWoundedAIContacts(const char* db,const char* resources,int mode)
{
    Require(mode>=0 && mode<6,"unknown native fear contact mode");NativeHearingOwners f(db,resources);
    CObj<NAI::IAIUnit> first=NAI::CreateAIUnit(f.humanUnit,false),second=NAI::CreateAIUnit(f.aiUnit,true);Require(first && second,"fear contacts lack real native factory identities");auto* state=second->GetAIUnitState();*state=NAI::SAIUnitState();state->SetUnit(second);
    vector<CPtr<NAI::IAIUnit>> expected;
    if(mode==1){state->enemies.data.units={first.GetPtr(),second.GetPtr()};expected={first.GetPtr(),second.GetPtr()};}
    if(mode==2){state->possibleEnemies.data.units={first.GetPtr()};expected={first.GetPtr()};}
    if(mode==3){state->enemies.data.units={first.GetPtr()};state->possibleEnemies.data.units={second.GetPtr()};expected={second.GetPtr(),first.GetPtr()};}
    if(mode==4){state->enemies.data.units={first.GetPtr()};state->possibleEnemies.data.units={first.GetPtr(),nullptr};expected={first.GetPtr(),nullptr,first.GetPtr()};}
    if(mode==5){state->enemies.data.units={first.GetPtr()};state->possibleEnemies.data.units={second.GetPtr()};expected={second.GetPtr(),first.GetPtr()};}
    const auto confirmed=state->enemies.data.units,possible=state->possibleEnemies.data.units;const bool modified=state->IsModified();const auto position=f.aiUnit->GetPosition();const int ap=f.aiUnit->GetAP(),hp=f.aiUnit->GetUnitRPG()->GetTotalVP();auto random=GlobalGameRandom();
    // Keep the same const-reference call shape used by the live mounted-gun consumer.
    const auto& actual=state->GetKnownEnemies();bool unchanged=state->enemies.data.units==confirmed && state->possibleEnemies.data.units==possible && state->IsModified()==modified;
    if(mode==5){state->enemies.data.units.clear();state->possibleEnemies.data.units.clear();}
    auto after=GlobalGameRandom();bool rng=true;for(int i=0;i<4;++i)rng&=random.Get()==after.Get();bool identities=actual==expected;
    std::printf("native fear contacts mode=%d confirmed=%zu suspected=%zu actual=%zu expected=%zu order=%d unchanged=%d detached=%d RNG=%d\n",mode,confirmed.size(),possible.size(),actual.size(),expected.size(),int(identities),int(unchanged),int(mode==5),int(rng));std::fflush(stdout);
    Require(unchanged && rng && position==f.aiUnit->GetPosition() && ap==f.aiUnit->GetAP() && hp==f.aiUnit->GetUnitRPG()->GetTotalVP(),"native fear contact query changed threat or physical/RNG state");
    Require(identities,"native fear contacts omit suspects, lose original order/duplicates/nulls or alias the source vector");
}

static void NativeWoundedAIRetreat(const char* db,const char* resources,int mode)
{
    Require(mode>=0 && mode<8,"unknown native fear retreat mode");NativeHearingOwners f(db,resources);auto* actor=f.aiUnit;auto* target=f.humanUnit;auto* net=f.world->GetPathNetwork();
    auto* commander=dynamic_cast<NAI::CAICommander*>(f.ai->GetCommander());Require(commander,"fear retreat lacks native commander");commander->OnUnitAdded(actor);CObj<NAI::IAIUnit> owner=commander->GetAIUnit(actor);Require(owner,"fear retreat lacks actual owner");owner->SetAIState(commander->GetAIState());
    list<CPtr<NWorld::CUnitServer>> all;f.world->GetAllUnits(&all);for(auto p:all)if(IsValid(p) && p.GetPtr()!=actor && p.GetPtr()!=target && p->CanFight())p->MakeUnconscious(VNULL3,true,false);
    CObj<NAI::IAIUnit> enemy=NAI::CreateAIUnit(target,false);Require(enemy,"fear retreat lacks real target wrapper");CObj<NWorld::CUnitServer> possibleServer;CObj<NAI::IAIUnit> possible;
    if(mode==1 || mode==2){possibleServer=f.world->AddUnitInGame(f.FreePlace(actor->GetPosition().GetCP(),12.f),NRPG::CreateUnit(NDb::GetPers(54)),f.human,"NativeFearSuspect");Require(possibleServer,"fear retreat cannot deploy actual suspected contact");possible=NAI::CreateAIUnit(possibleServer,false);Require(possible,"fear retreat cannot wrap suspect");possible->Synchronize(false);}
    auto move=[&](NWorld::CUnitServer* server,NAI::SPathPlace p){auto pos=server->GetPosition();pos.pos=NAI::SPosition(p,net);server->SetPosition(pos);};
    vector<NAI::SPathPlace> candidates;net->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),40.f),&candidates);bool staged=false;NAI::SPathPlace reference;
    vector<CPtr<NAI::IAIUnit>> expectedContacts;
    for(auto p:candidates){p.SetPose(NAI::CM_STAND);p.SetDirection(0);p.SetMoving(0);if(!net->IsPassable(p) || net->IsLocked(p))continue;const float distance=fabs(NAI::SPosition(p,net).GetCP()-actor->GetPosition().GetCP());if(distance<=2.f || distance>=10.f)continue;
        move(target,p);enemy->Synchronize(false);expectedContacts.clear();if(mode==1 || mode==2)expectedContacts.push_back(possible.GetPtr());if(mode!=2 && mode!=3)expectedContacts.push_back(enemy.GetPtr());
        if(mode==4)target->MakeUnconscious(VNULL3,true,false);GlobalGameRandom().SeedForHarness(1337);const bool found=NAI::GetFearPosition(actor,expectedContacts,&reference,5.f,30);
        if(mode==1 && found){NAI::SPathPlace confirmedOnly;GlobalGameRandom().SeedForHarness(1337);const vector<CPtr<NAI::IAIUnit>> single{enemy.GetPtr()};const bool singleFound=NAI::GetFearPosition(actor,single,&confirmedOnly,5.f,30);if(singleFound && confirmedOnly==reference)continue;}
        if((mode==3 || mode==4)?!found:found){staged=true;break;}}
    Require(staged,"fear retreat could not realize its actual reachable or empty native fear-wave input");owner->Synchronize(false);enemy->Synchronize(false);if(possible)possible->Synchronize(false);
    if(mode==5){enemy->SetHP(0,enemy->GetMaxHP());Require(target->CanFight(),"fear retreat stale planning HP lost physical target");}
    if(mode==6)actor->GetUnitRPG()->SpendAP(actor->GetAP());
    auto* state=owner->GetAIUnitState();*state=NAI::SAIUnitState();state->SetUnit(owner);if(mode!=2 && mode!=3)state->AddEnemy(enemy);if(mode==4)state->enemies.data.units.push_back(enemy.GetPtr());if(possible)state->AddPossibleEnemy(possible);state->pEnemy=enemy;state->bScared=true;state->enemies.bModified=state->possibleEnemies.bModified=state->allies.bModified=false;
    CObj<NAI::CAIRouteLogic> incumbent;if(mode!=7){incumbent=new NAI::CAIRouteLogic(owner);incumbent->AddCommand(new NAI::CTaskCommandWait(3));incumbent->SetCircled(true);owner->SetRouteLogic(incumbent);}else{owner->SetRouteLogic(nullptr);owner->SetLogic(nullptr);}
    GlobalGameRandom().SeedForHarness(1337);const bool shouldRetreat=NAI::GetFearPosition(actor,expectedContacts,&reference,5.f,30);auto expectedRandom=GlobalGameRandom();int cost=-1;float distance=-1;
    if(shouldRetreat){NAI::CMultiMovesTable table;vector<NAI::SPathPlace> reachable;NAI::GetNearestPlaces(actor,actor->GetPosition().pos.p,30,NAI::RUN,&reachable,&table);cost=int(table.GetCost(reference));distance=fabs(NAI::SPosition(reference,net).GetCP()-actor->GetPosition().GetCP());Require(cost<=30 && cost>=0 && distance>=5.f && std::find(reachable.begin(),reachable.end(),reference)!=reachable.end(),"reference retreat is not in the actual30AP RUN wave at least5 units away");}
    CObj<NAI::CAIReaction> normal=NAI::CreateAINormalReaction(owner);owner->SetReaction(normal);Require(owner->GetReactionForUpdate()==normal.GetPtr(),"fear retreat did not consume actual normal-reaction installation");const auto position=actor->GetPosition();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP(),plannedAP=owner->GetAP(),plannedHP=owner->GetHP();const auto confirmed=state->enemies.data.units,suspected=state->possibleEnemies.data.units;const auto wish=actor->GetWishPose();
    GlobalGameRandom().SeedForHarness(1337);NAI::SAIUpdateTracker pump;pump.Add(normal);pump.Update();auto* reaction=owner->GetReaction();const bool retreat=dynamic_cast<NAI::CAIRetreatReaction*>(reaction)!=nullptr,guard=dynamic_cast<NAI::CAIGuardReaction*>(reaction)!=nullptr;bool destination=!shouldRetreat;NAI::SPathPlace actual;
    if(retreat){CStructureNetworkContext refs;auto bytes=BorrowedObjectFields(reaction,&refs);CStructureSaver reader(bytes,CStructureSaver::READ,&refs);reader.Add(3,&actual);destination=actual==reference;}
    auto after=GlobalGameRandom();bool rng=true;for(int i=0;i<4;++i)rng&=expectedRandom.Get()==after.Get();const bool deferred=owner->GetRouteLogic()==incumbent.GetPtr() && owner->GetLogic()==incumbent.GetPtr();
    std::printf("native fear retreat mode=%d confirmed=%zu suspected=%zu expectedRetreat=%d retreat=%d guard=%d destination=%d cost=%d distance=%.9g deferred=%d RNG=%d AP=%d afterAP=%d HP=%d afterHP=%d\n",mode,confirmed.size(),suspected.size(),int(shouldRetreat),int(retreat),int(guard),int(destination),cost,distance,int(deferred),int(rng),ap,actor->GetAP(),hp,actor->GetUnitRPG()->GetTotalVP());std::fflush(stdout);
    Require(pump.CanGetCommand() && actor->GetPosition()==position && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && owner->GetAP()==plannedAP && owner->GetHP()==plannedHP && state->enemies.data.units==confirmed && state->possibleEnemies.data.units==suspected && actor->GetWishPose()==wish,"normal fear routing changed physical/planned unit state or contact lists");
    Require(retreat==shouldRetreat && guard==!shouldRetreat && destination && deferred && rng,"normal fear routing uses the approximate unreachable target, omits suspects or inline-updates the new reaction");
}

static void NativeTooltipDelay(const char* db,const char* resources,int mode)
{
    Require(mode>=0 && mode<12,"unknown native tooltip delay mode");S2Platform::SetErrorDialogs(false);struct PlatformCleanup{~PlatformCleanup(){S2Platform::Done();}} platformCleanup;
    Require(S2Platform::Init("native tooltip delay",1024,768,true),"tooltip clock cannot initialize owned dummy display");CFileStream database;database.OpenRead(db);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resources);
    const auto oldShow=NGlobal::GetVar("ui_showtooltips"),oldDelay=NGlobal::GetVar("ui_tooltipdelay");struct Options{NGlobal::CValue show,delay;~Options(){NGlobal::SetVar("ui_showtooltips",show);NGlobal::SetVar("ui_tooltipdelay",delay);}} options{oldShow,oldDelay};NGlobal::SetVar("ui_showtooltips",NGlobal::CValue(mode==2?0.f:1.f));NGlobal::SetVar("ui_tooltipdelay",NGlobal::CValue(mode==1?0.f:mode==10?.7f:.1f));
    struct FrameBoundary{};class ClockFrameView:public DialogueLayoutView{
        OBJECT_NOCOPY_METHODS(ClockFrameView);
        CObj<NGScene::CCVec2> screen=new NGScene::CCVec2(CVec2(1024,768));
    public:
        CFuncBase<NGScene::SText>* CreateText(CFuncBase<wstring>* text,CFuncBase<CTPoint<int>>* size,bool tags) override{return NGScene::CreateTextFormater(GetLocaleInfo(),screen,text,size,tags);}
        void StartNewFrame() override{throw FrameBoundary();}
    };
    CObj<CCTime> clock=new CCTime(0);CObj<NSound::ISoundScene> sound=NSound::CreateSoundScene(nullptr,nullptr,clock);CObj<NUI::ICursor> cursor=NUI::ICursor::Create(false);CObj<NUI::CInterface> root=new NUI::CInterface(cursor,sound);CObj<NGScene::I2DGameView> view=new ClockFrameView;
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(root,&refs);auto* original=root->GetView();Require(refs.ids.count(original)!=0,"tooltip clock view not captured by actual native fields");refs.refs[refs.ids.at(original)]=view.GetPtr();CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(root.GetPtr())->operator&(reader);}
    Require(root->GetView()==view.GetPtr() && root->GetSound()==sound.GetPtr(),"tooltip clock observer changed shared sound/view ownership");
    CObj<NUI::CWindow> first=new NUI::CWindow(NUI::SWindowInfo(root,NUI::SPoint(0,0),NUI::SPoint(200,80),"clock-owner",NUI::STYLE_VISIBLE|NUI::STYLE_ENABLED));CObj<NUI::CWindow> second=new NUI::CWindow(NUI::SWindowInfo(root,NUI::SPoint(250,0),NUI::SPoint(200,80),"clock-other",NUI::STYLE_VISIBLE|NUI::STYLE_ENABLED));
    CObj<NUI::CToolTip> tip=new NUI::CToolTip(NUI::SWindowInfo(root,NUI::SPoint(0,0),NUI::SPoint(0,0),"clock-tooltip",NUI::STYLE_VISIBLE|NUI::STYLE_ENABLED)),other=new NUI::CToolTip(NUI::SWindowInfo(root,NUI::SPoint(0,0),NUI::SPoint(0,0),"clock-other-tooltip",NUI::STYLE_VISIBLE|NUI::STYLE_ENABLED));tip->SetText(L"<font face=Courier size=16pt>Native clock");other->SetText(L"<font face=Courier size=16pt>Native other clock");first->SetToolTip(tip);other->SetStyle(NUI::STYLE_VISIBLE,false);if(mode!=6)second->SetToolTip(other);
    auto draw=[&](STime time){MarkNewDGFrame();bool reached=false;try{root->Draw(time);}catch(const FrameBoundary&){reached=true;}Require(reached,"actual interface Draw did not reach frame boundary");};
    string actual,expected;auto sample=[&](bool a,bool b,const char* label,STime time){const bool firstVisible=tip->GetStyle(NUI::STYLE_VISIBLE),secondVisible=other->GetStyle(NUI::STYLE_VISIBLE);actual+=firstVisible?'1':'0';actual+=secondVisible?'1':'0';expected+=a?'1':'0';expected+=b?'1':'0';std::printf("native tooltip delay mode=%d step=%s time=%u visible=%d/%d expected=%d/%d\n",mode,label,unsigned(time),int(firstVisible),int(secondVisible),int(a),int(b));};
    auto at=[&](STime time,bool a,bool b){draw(time);sample(a,b,"draw",time);};
    const auto firstPosition=first->GetPosition(),secondPosition=second->GetPosition(),firstSize=first->GetSize(),secondSize=second->GetSize();auto random=GlobalGameRandom();
    if(mode==6){root->SetToolTipOwner(second);at(100,false,false);at(2001,false,false);}
    else{
        root->SetToolTipOwner(first);
        if(mode==5){draw(100);at(2001,true,false);root->SetToolTipOwner(nullptr);sample(false,false,"clear",2001);at(3000,false,false);}
        else if(mode==9){draw(100);at(2001,true,false);NGlobal::SetVar("ui_showtooltips",NGlobal::CValue(0.f));root->SetToolTipOwner(first);sample(true,false,"disabled-before-step",2001);root->Step(2002);sample(false,false,"disabled-step",2002);at(2003,false,false);}
        else{
            sample(false,false,"claim",0);
            if(mode==0){at(0,false,false);at(100,false,false);at(1099,false,false);at(1100,false,false);at(1101,true,false);}
            if(mode==1){at(100,false,false);at(101,true,false);}
            if(mode==2){at(100,false,false);at(2001,false,false);}
            if(mode==3){at(100,false,false);draw(700);root->SetToolTipOwner(first);sample(false,false,"same-owner",700);at(1100,false,false);at(1101,true,false);}
            if(mode==4){draw(100);at(1101,true,false);root->SetToolTipOwner(second);sample(false,false,"switch",1101);at(1200,false,false);at(2200,false,false);at(2201,false,true);}
            if(mode==7){at(100,false,false);NGlobal::SetVar("ui_tooltipdelay",NGlobal::CValue(.6f));at(800,false,false);at(1101,true,false);}
            if(mode==8){at(100,false,false);root->SetToolTipOwner(nullptr);draw(500);root->SetToolTipOwner(first);at(700,false,false);at(1101,false,false);at(1700,false,false);at(1701,true,false);}
            if(mode==10){at(100,false,false);at(7099,false,false);at(7100,true,false);}
            if(mode==11){at(0x80000000u,false,false);at(0x80000001u,true,false);}
        }
    }
    auto after=GlobalGameRandom();bool rng=true;for(int i=0;i<4;++i)rng&=random.Get()==after.Get();const bool geometry=first->GetPosition()==firstPosition && first->GetSize()==firstSize && second->GetPosition()==secondPosition && second->GetSize()==secondSize;
    std::printf("native tooltip delay result mode=%d trace=%s expected=%s geometry=%d RNG=%d sharedSound=%d\n",mode,actual.c_str(),expected.c_str(),int(geometry),int(rng),int(root->GetSound()==sound.GetPtr()));std::fflush(stdout);
    Require(geometry && rng && root->GetSound()==sound.GetPtr(),"tooltip clock changed owner geometry/RNG/shared sound");Require(actual==expected,"actual interface tooltip claim/Draw/Step does not implement original options/deadline/owner lifetime");
}

static void NativeTooltipPlacement(const char* db,const char* resources,int mode)
{
    Require(mode>=0 && mode<20,"unknown native tooltip placement mode");S2Platform::SetErrorDialogs(false);struct Cleanup{~Cleanup(){S2Platform::Done();}} cleanup;const int pixelWidth=mode==16?1792:mode==17?2048:1024,pixelHeight=mode==17?1536:768;
    Require(S2Platform::Init("native tooltip placement",pixelWidth,pixelHeight,true),"tooltip placement cannot initialize owned dummy display");S2Platform::UpdateDisplay(mode==17?200:100);CFileStream database;database.OpenRead(db);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resources);
    const auto oldShow=NGlobal::GetVar("ui_showtooltips"),oldDelay=NGlobal::GetVar("ui_tooltipdelay");struct Options{NGlobal::CValue show,delay;~Options(){NGlobal::SetVar("ui_showtooltips",show);NGlobal::SetVar("ui_tooltipdelay",delay);}} options{oldShow,oldDelay};NGlobal::SetVar("ui_showtooltips",NGlobal::CValue(1.f));NGlobal::SetVar("ui_tooltipdelay",NGlobal::CValue(0.f));
    struct FrameBoundary{};class PlacementView:public NGScene::I2DGameView{
        OBJECT_NOCOPY_METHODS(PlacementView);
        CObj<NGScene::CTextLocaleInfo> locale;CObj<NGScene::CCVec2> screen;CVec2 viewport;
    public:
        int glyphs=0;
        explicit PlacementView(CVec2 size=CVec2(1024,768)):locale(new NGScene::CTextLocaleInfo),screen(new NGScene::CCVec2(size)),viewport(size){locale->Setup(viewport);}
        CFuncBase<NGScene::SText>* CreateText(CFuncBase<wstring>* text,CFuncBase<CTPoint<int>>* size,bool tags) override{return NGScene::CreateTextFormater(locale,screen,text,size,tags);}
        void CreateDynamicRects(CFuncBase<NGScene::SText>* value,const NativeUIDrawPoint&,const NativeUIDrawRect&) override{CDGPtr<CFuncBase<NGScene::SText>> node=value;node.Refresh();glyphs+=int(node->GetValue().charsSet.size());}
        void CreateDynamicRects(NDb::CTexture*,const CRectLayout&,const NativeUIDrawPoint&,const NativeUIDrawRect&) override{}
        void CreateDynamicRects(CPtrFuncBase<NGfx::CTexture>*,const CRectLayout& layout,const NativeUIDrawPoint&,const NativeUIDrawRect&) override{glyphs+=int(layout.rects.size());}
        void CreateDynamicClearRects(const CRectLayout&,const NativeUIDrawPoint&,const NativeUIDrawRect&,float) override{}
        NGScene::CTextLocaleInfo* GetLocaleInfo() const override{return locale;}
        const CVec2& GetViewportSize() override{return viewport;}
        void StartNewFrame() override{throw FrameBoundary();}
        void Flush() override{}
    };
    // Literal mouse input only; all hover/claim/placement decisions stay native.
    class PlacementCursor:public NUI::ICursor{
        OBJECT_NOCOPY_METHODS(PlacementCursor);
        CVec2 point=CVec2(0,0);NUI::SCursorInfo info;
    public:
        const CVec2& GetPos() const override{return point;}void SetPos(const CVec2& value) override{point=value;}
        const NUI::SCursorInfo& GetCursor() const override{return info;}void SetCursor(const NUI::SCursorInfo& value) override{info=value;}
        void Update() override{}void ProcessEvent(const NInput::SEvent&) override{}void Draw(const STime&,NGScene::I2DGameView*) override{}
    };
    CObj<CCTime> clock=new CCTime(0);CObj<NSound::ISoundScene> sound=NSound::CreateSoundScene(nullptr,nullptr,clock);CObj<NUI::ICursor> cursor=new PlacementCursor;CObj<NUI::CInterface> root=new NUI::CInterface(cursor,sound);CObj<PlacementView> view=new PlacementView(CVec2(float(pixelWidth),float(pixelHeight)));
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(root,&refs);auto* original=root->GetView();Require(refs.ids.count(original)!=0,"tooltip placement has no saved native view identity");refs.refs[refs.ids.at(original)]=view.GetPtr();CStructureSaver reader(fields,CStructureSaver::READ,&refs);static_cast<CObjectBase*>(root.GetPtr())->operator&(reader);}
    const auto canvas=root->GetSize();Require(canvas.x==S2UI::Width() && canvas.y==S2UI::Height(),"tooltip placement root lost owned logical canvas");if(mode==16)Require(canvas.x>1024,"wide tooltip fixture did not establish native wider canvas");if(mode==17)Require(std::fabs(S2UI::Scale()-2.f)<.001f && canvas.x==1024 && canvas.y==768,"tooltip fixture did not establish owned native HiDPI canvas");
    CObj<NUI::CWindow> parent=root.GetPtr();if(mode==4 || mode==5)parent=new NUI::CWindow(NUI::SWindowInfo(root,NUI::SPoint(50,70),NUI::SPoint(800,600),"tooltip-nested",NUI::STYLE_VISIBLE|NUI::STYLE_ENABLED));
    NUI::SPoint ownerPosition(mode==7?100:300,mode==7?100:300),ownerInputSize(mode==7?650:80,mode==7?450:40);if(mode==6)ownerPosition.y=2;if(mode==13)ownerPosition.y=canvas.y-10;
    CObj<NUI::CWindow> owner=new NUI::CWindow(NUI::SWindowInfo(parent,ownerPosition,ownerInputSize,"placement-owner",NUI::STYLE_VISIBLE|NUI::STYLE_ENABLED));CObj<NUI::CToolTip> tip=new NUI::CToolTip(NUI::SWindowInfo(root,NUI::SPoint(0,0),NUI::SPoint(0,0),"placement-tooltip",NUI::STYLE_VISIBLE|NUI::STYLE_ENABLED));owner->SetToolTip(tip);
    CObj<NUI::CText> reference=new NUI::CText(NUI::SWindowInfo(root,NUI::SPoint(0,0),NUI::SPoint(512,0),"placement-reference",NUI::STYLE_ENABLED|NUI::STYLE_TRANSPARENT));
    const wstring format=L"<font face=Courier size=16pt><left>",shortText=format+L"Anchor",longText=format+L"WWWW WWWW WWWW WWWW WWWW WWWW WWWW WWWW WWWW";wstring bigText=format+L"Line";for(int i=0;i<100;++i)bigText+=L"<br>Line";
    auto* font=view->GetLocaleInfo()->GetFont(NGScene::SFont(Float2Int(16*S2UI::Scale()),"Courier"));Require(font && font->IsVector(),"tooltip placement lost packaged high-resolution vector Courier");
    NUI::SPoint cursorInput(mode==13?30:200,200),anchor(600,400);NDb::EUIAnchor type=mode>=1 && mode<=4?static_cast<NDb::EUIAnchor>(mode):NDb::UIA_NONE;
    if(mode==11){type=NDb::UIA_RIGHTBOTTOM;anchor=NUI::SPoint(-20,-30);}if(mode==12){type=NDb::UIA_RIGHTBOTTOM;anchor=NUI::SPoint(canvas.x+100,canvas.y+100);}if(mode==13){type=NDb::UIA_LEFTTOP;anchor=NUI::SPoint(30,canvas.y-10);}if(mode==16){type=NDb::UIA_RIGHTTOP;anchor=NUI::SPoint(canvas.x-10,300);}if(mode==17){type=NDb::UIA_LEFTTOP;anchor=NUI::SPoint(400,200);}if(mode==18){type=NDb::UIA_LEFTTOP;anchor=NUI::SPoint(600,400);}
    auto applyInputs=[&](CObjectBase* object,const std::map<unsigned,vector<unsigned char>>& inputs){
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(object,&refs);auto* bytes=fields.GetBufferForWrite();S2FileIO::StructureChunk body{};
        for(std::size_t at=0;at<std::size_t(fields.GetSize());){S2FileIO::StructureChunk field;Require(S2FileIO::DecodeStructureChunkAt(bytes,fields.GetSize(),at,&field) && field.id!=3,"authored input snapshot is not plain native chunks");if(field.id==1)body=field;at=field.payloadOffset+field.length;}
        Require(body.length!=0,"authored input snapshot has no native root fields");unsigned patched=0;
        for(std::size_t at=0;at<body.length;){S2FileIO::StructureChunk field;auto* data=bytes+body.payloadOffset;Require(S2FileIO::DecodeStructureChunkAt(data,body.length,at,&field),"authored input native field invalid");auto input=inputs.find(field.id);if(input!=inputs.end()){Require(field.length==input->second.size(),"authored input field wire size differs");std::memcpy(data+field.payloadOffset,input->second.data(),field.length);++patched;}at=field.payloadOffset+field.length;}
        Require(patched==inputs.size(),"authored input did not patch each existing native field");fields.Seek(0);CStructureSaver reader(fields,CStructureSaver::READ,&refs);object->operator&(reader);
    };
    unsigned inputStep=0;auto setCursor=[&](NUI::SPoint point){cursor->SetPos(CVec2(S2UI::ToPixel(point.x),S2UI::ToPixel(point.y)));MarkNewDGFrame();root->Step(++inputStep);Require(root->GetCursorPos()==point && root->GetView()==view.GetPtr() && root->GetSound()==sound.GetPtr(),"literal native mouse input changed view/sound or failed coordinate mapping");};
    auto setAnchor=[&](){const auto position=owner->GetPosition(),size=owner->GetSize();vector<unsigned char> point(8),kind(4);Require(S2FileIO::EncodeStructureScalar(anchor.x,point.data(),4) && S2FileIO::EncodeStructureScalar(anchor.y,point.data()+4,4) && S2FileIO::EncodeStructureScalar(int(type),kind.data(),4),"authored anchor encoding failed");applyInputs(owner,{{14,point},{15,kind}});owner->DisableAdaptiveLayout();Require(owner->GetParent()==parent.GetPtr() && owner->GetToolTip()==tip.GetPtr() && owner->GetPosition()==position && owner->GetSize()==size,"complete borrowed authored anchor input changed actual owner");};
    setCursor(cursorInput);setAnchor();wstring text=mode==0 || mode==14?longText:mode==10 || mode==13?bigText:mode==15?L"":shortText;tip->SetText(text);auto random=GlobalGameRandom();const auto ownerSize=owner->GetSize();
    auto measure=[&](){reference->SetSize(NUI::SPoint(512,0));reference->SetText(text);NUI::SPoint size;reference->GetRealSize(&size);return size+NUI::SPoint(8,8);};
    auto draw=[&](STime time){MarkNewDGFrame();bool reached=false;try{root->Draw(time);}catch(const FrameBoundary&){reached=true;}Require(reached,"native tooltip placement did not reach frame boundary");};
    root->SetToolTipOwner(owner);const bool prime=mode==7 || mode==8 || mode==9 || mode==10 || mode==13 || mode==14 || mode==19;if(prime){MarkNewDGFrame();tip->Draw(1,view);}
    if(mode==7){setCursor(NUI::SPoint(700,500));root->SetToolTipOwner(owner);}if(mode==8){ownerPosition.y=420;owner->SetPosition(ownerPosition);}if(mode==9){text=bigText;tip->SetText(text);}if(mode==10){text=shortText;tip->SetText(text);}if(mode==18){type=NDb::UIA_RIGHTBOTTOM;setAnchor();}
    draw(100);draw(101);const auto expectedSize=measure();if(mode==13)Require(expectedSize.y>canvas.y,"oversized placement fixture is not taller than the native canvas");NUI::SPoint point;NUI::SRect rect;owner->ClientToScreen(&point,&rect);NUI::SPoint expected;
    switch(type){case NDb::UIA_NONE:expected=NUI::SPoint(cursorInput.x,point.y-expectedSize.y);if(expected.y<0)expected.y=point.y+owner->GetSize().y+32;break;case NDb::UIA_LEFTTOP:expected=anchor;break;case NDb::UIA_RIGHTTOP:expected=NUI::SPoint(anchor.x-expectedSize.x,anchor.y);break;case NDb::UIA_LEFTBOTTOM:expected=NUI::SPoint(anchor.x,anchor.y-expectedSize.y);break;case NDb::UIA_RIGHTBOTTOM:expected=anchor-expectedSize;break;default:Require(false,"invalid native anchor input");}
    expected.x=Max(0,Min(expected.x,canvas.x-expectedSize.x));expected.y=Max(0,Min(expected.y,canvas.y-expectedSize.y));
    const auto actualSize=tip->GetSize(),actualPosition=tip->GetPosition();MarkNewDGFrame();tip->Draw(102,view);if(mode!=15)Require(view->glyphs>0,"native placement widget did not submit packaged vector glyphs after the measured frame boundary");bool stable=true;if(mode==19){for(STime time:{200u,300u,400u}){draw(time);MarkNewDGFrame();tip->Draw(time,view);stable&=tip->GetSize()==actualSize && tip->GetPosition()==actualPosition;}}
    auto after=GlobalGameRandom();bool rng=true;for(int i=0;i<4;++i)rng&=random.Get()==after.Get();const bool ownerIntact=owner->GetPosition()==ownerPosition && owner->GetSize()==ownerSize && owner->GetParent()==parent.GetPtr() && owner->GetToolTip()==tip.GetPtr() && root->GetSound()==sound.GetPtr();
    std::printf("native tooltip placement mode=%d canvas=%d,%d scale=%.1f anchor=%d size=%d,%d expectedSize=%d,%d position=%d,%d expectedPosition=%d,%d prime=%d glyphs=%d stable=%d owner=%d RNG=%d\n",mode,canvas.x,canvas.y,S2UI::Scale(),int(type),actualSize.x,actualSize.y,expectedSize.x,expectedSize.y,actualPosition.x,actualPosition.y,expected.x,expected.y,int(prime),view->glyphs,int(stable),int(ownerIntact),int(rng));std::fflush(stdout);
    Require(ownerIntact && rng,"tooltip measurement/placement changed actual owner/input/sound/RNG state");Require(actualSize==expectedSize && actualPosition==expected && stable,"native tooltip is not measured512 before placement or ignores anchors/moving owner/logical canvas");
}

static void NativeAutoPlaySquads(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode==0 || mode==1,"unknown autoplay squad fixture");
    Require(std::getenv("S2_USER_DATA_DIR") && !NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),
        "autoplay squad fixture needs isolated empty interface state");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm demonstration squad regression");
    struct Restore {
        NGlobal::CValue units=NGlobal::GetVar("autoplay_units"), maps=NGlobal::GetVar("autoplay_templates");
        ~Restore(){NGlobal::SetVar("autoplay_units",units);NGlobal::SetVar("autoplay_templates",maps);}
    } restore;
    const int person=mode==0?968:970;
    NGlobal::SetVar("autoplay_units",NGlobal::CValue(mode==0?L"968":L"970"));
    NGlobal::SetVar("autoplay_templates",NGlobal::CValue(L"4412"));
    CObj<NRPG::CGlobalPlayer> human=NRPG::CreateGlobalPlayer(vector<int>{person});
    Require(IsValid(human) && !human->bAIPlayer && human->mercs.size()==1,"ordinary player control was not created");
    const int humanLevel=int(human->mercs.front()->Skills(NDb::ST_LEVEL));
    Require(humanLevel!=30,"ordinary player control already has the demonstration level");
    NGlobal::ProcessCommand(L"autoplay");
    CObj<NMainLoop::CInterfaceCommand> enter=NMainLoop::TakeInterfaceCommandForHarness();
    Require(IsValid(enter) && !NMainLoop::HaveInterfaceCommand(),"real autoplay command did not queue exactly one entry");
    enter->Exec();
    CObj<NGame::CMissionBase> demo=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(demo) && pSSClasses->GetObjectTypeID(demo)==int(0xB3723140),"real autoplay command did not initialize its registered mission");
    auto* world=demo->GetWorld();auto* global=world?world->GetGlobalGame():nullptr;
    Require(IsValid(global) && global->players.size()==4,"actual autoplay mission did not create four squads");
    vector<CPtr<NGame::IPlayerTracker>> players;demo->GetPlayers(&players);
    Require(players.size()==4,"actual autoplay world lost a squad tracker");
    int count=0,ai=0,leveled=0;bool links=true,cardinality=true;
    for(std::size_t i=0;i<global->players.size();++i) {
        auto* player=global->players[i].GetPtr();ai+=player->bAIPlayer?1:0;
        cardinality=cardinality && player->mercs.size()>=4 && player->mercs.size()<=7;
        bool linked=false;for(const auto& tracker:players)if(tracker->GetGlobalPlayer()==player)linked=true;
        links=links && linked;
        for(const auto& unit:player->mercs) {++count;if(int(unit->Skills(NDb::ST_LEVEL))==30)++leveled;}
    }
    std::printf("AUTOPLAY-SQUADS mode=%d person=%d players=4 ai=%d units=%d level30=%d cardinality=%d links=%d humanAI=%d humanLevel=%d\n",
        mode,person,ai,count,leveled,int(cardinality),int(links),int(human->bAIPlayer),int(human->mercs.front()->Skills(NDb::ST_LEVEL)));
    Require(cardinality && links && count>=16 && count<=28,"demonstration initialization changed the original squad cardinality or native world links");
    Require(!human->bAIPlayer && int(human->mercs.front()->Skills(NDb::ST_LEVEL))==humanLevel,"autoplay preparation changed a separately created ordinary player");
    Require(ai==4 && leveled==count,"actual demonstration squads lack AI control or the original level-30 preparation");
    NMainLoop::DoneInterface();
}


struct AutoPlayTurnMemberTag {
    using type=int NWorld::CWorld::*;
    friend type AutoPlayTurnField(AutoPlayTurnMemberTag);
};
template<auto Member> struct AutoPlayTurnMemberAccess {
    friend AutoPlayTurnMemberTag::type AutoPlayTurnField(AutoPlayTurnMemberTag){return Member;}
};
template struct AutoPlayTurnMemberAccess<&NWorld::CWorld::nTurnID>;

// Observe the real virtual demo consumer before the common pump can select a new
// active player. Keep a genuinely non-active winner as a distinct regression control.
struct AutoPlayStepMemberTag {using type=void(NGame::CMission::*)();friend type AutoPlayGameStepMember(AutoPlayStepMemberTag);};
template<auto Member> struct AutoPlayStepMemberAccess {friend AutoPlayStepMemberTag::type AutoPlayGameStepMember(AutoPlayStepMemberTag){return Member;}};
template struct AutoPlayStepMemberAccess<&NGame::CMission::GameStep>;

static void NativeAutoPlayLifecycle(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<10,"unknown autoplay lifecycle fixture");
    Require(std::getenv("S2_USER_DATA_DIR") && !NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"autoplay lifecycle needs isolated empty interface state");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm demonstration lifecycle regression");
    struct Restore {
        NGlobal::CValue units=NGlobal::GetVar("autoplay_units"),maps=NGlobal::GetVar("autoplay_templates"),freeze=NGlobal::GetVar("diag_freeze_world_clock",0);
        STime time=NMainLoop::SetTimeForHarness(0);
        ~Restore(){NGlobal::SetVar("autoplay_units",units);NGlobal::SetVar("autoplay_templates",maps);NGlobal::SetVar("diag_freeze_world_clock",freeze);NMainLoop::SetTimeForHarness(time);}
    } restore;
    NGlobal::SetVar("autoplay_units",NGlobal::CValue(L"968"));NGlobal::SetVar("autoplay_templates",NGlobal::CValue(L"4412"));NGlobal::SetVar("diag_freeze_world_clock",NGlobal::CValue(1));
    NGlobal::ProcessCommand(L"autoplay");CObj<NMainLoop::CInterfaceCommand> enter=NMainLoop::TakeInterfaceCommandForHarness();Require(IsValid(enter),"autoplay lifecycle has no real entry command");enter->Exec();
    CObj<NGame::CMissionBase> demo=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(IsValid(demo) && pSSClasses->GetObjectTypeID(demo)==int(0xB3723140),"autoplay lifecycle did not open the actual registered mission");
    CObj<NMainLoop::CInterfaceCommand> snapshot=NMainLoop::TakeInterfaceCommandForHarness();
    Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"autoplay lifecycle did not isolate its existing mission-start snapshot command");
    auto* world=dynamic_cast<NWorld::CWorld*>(demo->GetWorld());Require(world && world->GetGlobalGame(),"autoplay lifecycle has no native world/global game");
    auto* global=world->GetGlobalGame();auto* scene=demo->GetSoundScene();auto* interface=demo->GetInterface();
    vector<CPtr<NWorld::CPlayer>> players;world->GetPlayersList(&players);Require(players.size()>=2,"autoplay lifecycle has no competing native players");
    vector<CPtr<NGame::IPlayerTracker>> trackers;demo->GetPlayers(&trackers);Require(trackers.size()==4,"autoplay lifecycle lost its four actual trackers");
    auto* winnerTarget=mode==8?demo->GetActivePlayer()->GetPlayer():trackers.back()->GetPlayer();
    if(mode==9){winnerTarget=nullptr;for(auto it=trackers.rbegin();it!=trackers.rend();++it){auto* candidate=(*it)->GetPlayer();if(candidate!=demo->GetActivePlayer()->GetPlayer() && candidate!=world->GetCurrentPlayer()){winnerTarget=candidate;break;}}Require(winnerTarget,"non-active victory control has no independent real player");}
    const bool winnerInput=mode==5 || mode==8 || mode==9;
    if(winnerInput){
        vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
        for(const auto& unit:units)if(unit->GetPlayer()!=winnerTarget){
            auto* server=dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());Require(server,"autoplay victory input has a non-server combat unit");
            server->MakeUnconscious(VNULL3,true,false);Require(!server->CanFight(),"autoplay victory input did not knock out a real opponent");
        }
    }
    int winners=0;for(const auto& tracker:trackers)if(tracker->IsPlayerWinner())++winners;
    Require(winners==(winnerInput?1:0),"autoplay lifecycle native knockouts did not establish real winner/nonwinner conditions");
    auto setWorldTurn=[&](int turn){
        // A literal input to the existing native int; do not deserialize the live world.
        (world->*AutoPlayTurnField(AutoPlayTurnMemberTag{}))=turn;
        Require(world->GetTurnID()==turn && world->GetGlobalGame()==global && demo->GetWorld()==world,"autoplay turn input changed the actual world graph");
    };
    const int turn=mode==1?100:mode==2?101:0;setWorldTurn(turn);
    {int current=0;for(const auto& tracker:trackers)if(tracker->IsPlayerWinner())++current;Require(current==winners,"autoplay scalar input changed actual winner state");}
    CTimeCounter watchdog;
    auto signal=[&](){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(demo,&refs);bool value=false;CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&value);reader.Add(3,&watchdog);return value;};
    Require(!signal(),"autoplay lifecycle started with a dismissed signal");
    const STime watchdogInput=mode==3?1200000:mode==4?1200001:0;watchdog.SetCurrent(watchdogInput);
    if(mode==8)scene->SetMusic(NDb::MT_COMBAT);
    auto nextMusic=[&](){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(scene,&refs);int value=-1;CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(9,&value);return value;};
    if(mode==8)Require(nextMusic()==int(NDb::MT_COMBAT),"autoplay sound input did not establish native requested combat music");
    if(mode==6 || mode==7){NInput::SEvent event;event.mMessage.cType=mode==6?NInput::CT_KEY:NInput::CT_TIME;demo->ProcessEvent(event);}
    CObj<NMainLoop::CInterfaceCommand> dismissed=NMainLoop::TakeInterfaceCommandForHarness();
    Require((mode==6 && dynamic_cast<NMainLoop::CICExitModal*>(dismissed.GetPtr())) || (mode!=6 && !IsValid(dismissed)),"native dismiss/time input queued the wrong command");
    if(mode==9){
        Require(!demo->GetActivePlayer()->IsPlayerWinner() && winnerTarget!=demo->GetActivePlayer()->GetPlayer(),"virtual demo consumer does not have a genuinely non-active native winner");
        auto* native=dynamic_cast<NGame::CMission*>(demo.GetPtr());Require(native,"virtual demo consumer lost its native mission type");
        std::printf("AUTOPLAY-NONACTIVE-WINNER mode=9 activeWinner=0 actualVirtualConsumer=1\n");std::fflush(stdout);
        (native->*AutoPlayGameStepMember(AutoPlayStepMemberTag{}))();
    }else{MarkNewDGFrame();Require(NMainLoop::StepApp(true,false,false),"autoplay lifecycle frame requested application exit");}
    CObj<NMainLoop::CInterfaceCommand> next=NMainLoop::TakeInterfaceCommandForHarness();const bool isContainer=dynamic_cast<NMainLoop::CICContainer*>(next.GetPtr())!=nullptr;
    const bool latched=signal(),expected=mode==2 || mode==4 || winnerInput;
    const bool graph=demo->GetWorld()==world && world->GetGlobalGame()==global && demo->GetSoundScene()==scene && demo->GetInterface()==interface && NMainLoop::GetCurrentInterfaceForHarness()==demo.GetPtr();
    int winnersNow=0;for(const auto& tracker:trackers)if(tracker->IsPlayerWinner())++winnersNow;
    std::printf("AUTOPLAY-LIFECYCLE mode=%d turn=%d watchdog=%u winners=%d container=%d signal=%d expected=%d graph=%d music=%d winnersNow=%d\n",mode,world->GetTurnID(),unsigned(watchdog.GetTime()->GetValue()),winners,int(isContainer),int(latched),int(expected),int(graph),nextMusic(),winnersNow);std::fflush(stdout);
    Require(winnersNow==winners,"autoplay frozen native frame changed the fixture winner condition");
    Require(graph && !NMainLoop::HaveInterfaceCommand(),"autoplay frame changed its graph before executing transition or queued duplicates");
    Require(isContainer==expected && latched==(expected || mode==6),"actual autoplay frame does not honor winner/strict turn100/strict20minute watchdog or dismiss latch");
    if(mode==8)Require(nextMusic()==int(NDb::MT_COMBAT),"autoplay ran the interactive mission's combat/ambient music policy");
    MarkNewDGFrame();demo->Step();Require(!NMainLoop::HaveInterfaceCommand(),"latched autoplay lifecycle queued the same transition more than once");
    if(expected){next->Exec();auto* replacement=NMainLoop::GetCurrentInterfaceForHarness();Require(replacement && replacement!=demo.GetPtr() && pSSClasses->GetObjectTypeID(replacement)==int(0xB3723140),"actual map transition did not replace its modal with a fresh registered demonstration mission");}
    NMainLoop::DoneInterface();
}


static CTimeCounter NativeFrontendUIClock(CObjectBase* frontend)
{
    struct Base {CTimeCounter clock;int operator&(CStructureSaver& f){f.Add(33,&clock);return 0;}};
    struct Parent {Base base;int operator&(CStructureSaver& f){f.Add(1,&base);return 0;}};
    struct Root {Parent parent;int operator&(CStructureSaver& f){f.Add(1,&parent);return 0;}} state;
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(frontend,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);state.operator&(reader);return state.parent.base.clock;
}

class AutoPlayClockObserver:public NUI::CWindow {
    OBJECT_NOCOPY_METHODS(AutoPlayClockObserver);
public:
    int draws=0;STime observed=0;
    AutoPlayClockObserver() {}
    explicit AutoPlayClockObserver(NUI::CWindow* root):CWindow(NUI::SWindowInfo(root,NUI::SPoint(0,0),NUI::SPoint(8,8),"demo-clock-observer",NUI::STYLE_VISIBLE|NUI::STYLE_ENABLED)){}
    void Draw(const STime& time,NGScene::I2DGameView* view) override {++draws;observed=time;CWindow::Draw(time,view);}
};

static void NativeAutoPlayPresentation(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<6,"unknown demonstration presentation fixture");
    Require(std::getenv("S2_USER_DATA_DIR") && !NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"demo presentation needs isolated empty interface state");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm demonstration presentation regression");
    struct Restore {NGlobal::CValue units=NGlobal::GetVar("autoplay_units"),maps=NGlobal::GetVar("autoplay_templates"),scale=NGlobal::GetVar("ui_scale",0);STime time=NMainLoop::SetTimeForHarness(0);~Restore(){NGlobal::SetVar("autoplay_units",units);NGlobal::SetVar("autoplay_templates",maps);NGlobal::SetVar("ui_scale",scale);NMainLoop::SetTimeForHarness(time);}} restore;
    if(mode==3 || mode==4){NGlobal::SetVar("ui_scale",NGlobal::CValue(mode==4?200:100));NGfx::SRenderTargetsInfo targets;Require(NGfx::SetMode(NGfx::SVideoMode(mode==3?1792:2048,mode==3?768:1536,32,NGfx::WINDOWED),targets),"demo presentation could not resize its owned render surface");S2Platform::UpdateDisplay(mode==4?200:100);}
    NGlobal::SetVar("autoplay_units",NGlobal::CValue(L"968"));NGlobal::SetVar("autoplay_templates",NGlobal::CValue(L"4412"));
    NGlobal::ProcessCommand(L"autoplay");CObj<NMainLoop::CInterfaceCommand> enter=NMainLoop::TakeInterfaceCommandForHarness();Require(IsValid(enter),"demo presentation has no native entry command");enter->Exec();
    CObj<NGame::CMissionBase> demo=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());Require(IsValid(demo) && pSSClasses->GetObjectTypeID(demo)==int(0xB3723140),"presentation fixture did not create the actual registered demo");
    CObj<NMainLoop::CInterfaceCommand> snapshot=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"presentation fixture did not isolate its existing mission-start snapshot");
    demo->PauseGame(true);Require(demo->IsGamePaused(),"presentation fixture did not pause native world simulation");
    struct Base {bool hide=false;int operator&(CStructureSaver& f){f.Add(13,&hide);return 0;}};
    struct Mission {Base base;int sequence=-1;bool lose=false;int operator&(CStructureSaver& f){f.Add(1,&base);f.Add(17,&sequence);f.Add(31,&lose);return 0;}};
    struct State {Mission mission;CObj<NUI::ICursor> cursor;CObj<NUI::CInterface> root;CObj<NUI::CWindow> flash;int operator&(CStructureSaver& f){f.Add(1,&mission);f.Add(4,&cursor);f.Add(5,&root);f.Add(6,&flash);return 0;}} state;
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(demo,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);state.operator&(reader);}
    Require(IsValid(state.root) && IsValid(state.cursor),"actual demo lacks its existing logo root/cursor");
    bool scriptTB=false,showCursor=true;
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(demo->GetWorld(),&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(56,&scriptTB);}
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(state.cursor,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(3,&showCursor);}
    const bool shared=state.root->GetSound()==demo->GetSoundScene();
    CTimeCounter ui=NativeFrontendUIClock(demo);ui.SetCurrent(123);Require(demo->GetUITime()==123,"presentation clock reader does not reference the actual frontend UI node");
    CObj<AutoPlayClockObserver> observer;
    if(mode!=0){observer=new AutoPlayClockObserver(state.root);NMainLoop::SetTimeForHarness(4000);MarkNewDGFrame();Require(NMainLoop::StepApp(true,false,false) && !NMainLoop::HaveInterfaceCommand(),"paused presentation frame unexpectedly changed interfaces");}
    const auto rect=demo->GetCamera()->GetScreenRect();const auto canvas=state.root->GetSize();const bool full=std::fabs(rect.x1)<1e-6f && std::fabs(rect.y1)<1e-6f && std::fabs(rect.x2-1)<1e-6f && std::fabs(rect.y2-1)<1e-6f;
    const bool flash=IsValid(state.flash) && pSSClasses->GetObjectTypeID(state.flash)==int(0xB2540911);STime morph=0;float coefficient=-1;
    if(flash){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(state.flash,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&coefficient);reader.Add(3,&morph);}
    std::printf("AUTOPLAY-PRESENTATION mode=%d sequence=%d hide=%d lose=%d scriptTB=%d cursor=%d flash=%d shared=%d ui=%u rawInput=%u observed=%u draws=%d full=%d canvas=%d,%d scale=%.1f morph=%u coefficient=%.6f\n",mode,state.mission.sequence,int(state.mission.base.hide),int(state.mission.lose),int(scriptTB),int(showCursor),int(flash),int(shared),unsigned(demo->GetUITime()),mode==0?0u:4000u,observer?unsigned(observer->observed):0,observer?observer->draws:0,int(full),canvas.x,canvas.y,S2UI::Scale(),unsigned(morph),coefficient);std::fflush(stdout);
    Require(shared,"demonstration overlay stopped sharing the mission sound scene");
    if(mode==0)Require(state.mission.sequence==1 && state.mission.base.hide && state.mission.lose && scriptTB && !showCursor && flash && state.flash->GetParent()==state.root.GetPtr() && state.flash->GetStyle(NUI::STYLE_TOPMOST) && state.flash->GetStyle(NUI::STYLE_VISIBLE),"actual demonstration initialization lacks original sequence/HUD/lose/TB/cursor/flash state");
    if(mode==1)Require(observer->draws>0 && observer->observed==demo->GetUITime() && observer->observed!=4000,"actual demonstration overlay uses raw time instead of the always-running UI clock");
    if(mode>=2 && mode<=4){Require(full && canvas.x==S2UI::Width() && canvas.y==S2UI::Height(),"actual demonstration retains mission client-view margins or lost logical display bounds");if(mode==3)Require(canvas.x==1792 && canvas.y==768,"wide demo fixture did not establish its real larger canvas");if(mode==4)Require(canvas.x==1024 && canvas.y==768 && std::fabs(S2UI::Scale()-2.f)<.001f,"HiDPI demo fixture lost native two-pixel logical scaling");}
    if(mode==5)Require(flash && state.flash->GetChildByID("active") && state.flash->GetChildByID("background") && morph==demo->GetUITime() && coefficient>=0 && coefficient<=1,"actual publisher-logo widget was not loaded/drawn on the demo UI clock");
    NMainLoop::DoneInterface();
}


static bool NativeMenuIdleStamp(CObjectBase* menu,STime* stamp,std::size_t* size)
{
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(menu,&refs);const auto* bytes=fields.GetBufferForWrite();S2FileIO::StructureChunk body{};
    for(std::size_t at=0;at<std::size_t(fields.GetSize());){S2FileIO::StructureChunk field;Require(S2FileIO::DecodeStructureChunkAt(bytes,fields.GetSize(),at,&field) && field.id!=3,"menu timestamp has invalid native wire");if(field.id==1)body=field;at=field.payloadOffset+field.length;}
    Require(body.length,"menu timestamp has no actual root fields");
    for(std::size_t at=0;at<body.length;){S2FileIO::StructureChunk field;const auto* data=bytes+body.payloadOffset;Require(S2FileIO::DecodeStructureChunkAt(data,body.length,at,&field),"menu timestamp native field is malformed");if(field.id==2){*size=field.length;return field.length==4 && S2FileIO::DecodeStructureScalar(data+field.payloadOffset,field.length,stamp);}at=field.payloadOffset+field.length;}
    *size=0;return false;
}

static void NativeMenuIdleDemo(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<8,"unknown idle-menu demonstration fixture");
    Require(std::getenv("S2_USER_DATA_DIR") && !NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"idle-menu fixture needs isolated empty interface state");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm idle-menu demonstration regression");
    struct Restore {NGlobal::CValue units=NGlobal::GetVar("autoplay_units"),maps=NGlobal::GetVar("autoplay_templates");STime time=NMainLoop::SetTimeForHarness(0);~Restore(){NGlobal::SetVar("autoplay_units",units);NGlobal::SetVar("autoplay_templates",maps);NMainLoop::SetTimeForHarness(time);}} restore;
    NGlobal::SetVar("autoplay_units",NGlobal::CValue(L"968"));NGlobal::SetVar("autoplay_templates",NGlobal::CValue(L"4412"));
    CObj<NGame::CICMainMenu> open=new NGame::CICMainMenu;open->Exec();
    CObj<NGame::CMissionBase> menu=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());Require(IsValid(menu) && pSSClasses->GetObjectTypeID(menu)==int(0xB2540912) && !NMainLoop::HaveInterfaceCommand(),"idle-menu fixture did not initialize the actual registered main menu");
    auto* root=menu->GetInterface();auto* world=menu->GetWorld();auto* sound=menu->GetSoundScene();CTimeCounter ui=NativeFrontendUIClock(menu);ui.SetCurrent(7);Require(menu->GetUITime()==7,"idle-menu clock reader does not reference its actual UI node");ui.SetCurrent(0);
    int handled=0;bool minimized=false;STime expectedStamp=0;
    if(mode==0)ui.SetCurrent(120000);
    if(mode==1)ui.SetCurrent(120001);
    if(mode==2){ui.SetCurrent(12345);menu->OnGetFocus();expectedStamp=12345;}
    if(mode==3){ui.SetCurrent(12345);NInput::SEvent event{};event.mMessage.cType=NInput::CT_KEY;handled=menu->ProcessEvent(event)?1:0;expectedStamp=12345;}
    if(mode==4){ui.SetCurrent(111);menu->OnGetFocus();expectedStamp=111;ui.SetCurrent(120112);NInput::SEvent event{};event.mMessage.cType=NInput::CT_TIME;handled=menu->ProcessEvent(event)?1:0;}
    if(mode==5){NMainLoop::SetTimeForHarness(1000);menu->Step();Require(!NMainLoop::HaveInterfaceCommand(),"idle-menu focus-gap baseline unexpectedly queued a transition");ui.SetCurrent(1234);NMainLoop::SetTimeForHarness(6000);menu->OnGetFocus();expectedStamp=1234;}
    struct NativeInputScope { bool active=false; ~NativeInputScope(){if(active)NInput::DoneInput();} } nativeInput;
    if(mode==6){
        Require(nativeInput.active=NInput::InitInput(true),"idle-menu fixture could not initialize native SDL input");
        ui.SetCurrent(12345);expectedStamp=12345;NInput::SetSection("menu");NGlobal::ProcessCommand(L"bind 'console' 'F12'");NInput::UpdateBinds();
        SDL_Event focus{};focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED;focus.window.windowID=SDL_GetWindowID(S2Platform::Window());Require(SDL_PushEvent(&focus),"idle-menu UI input could not queue owned focus");
        SDL_Event key{};key.type=SDL_EVENT_KEY_DOWN;key.key.windowID=SDL_GetWindowID(S2Platform::Window());key.key.scancode=SDL_SCANCODE_F12;key.key.down=true;Require(SDL_PushEvent(&key),"idle-menu UI input could not queue native console key");S2Platform::PumpEvents();NInput::PumpMessages(true,true);NInput::SEvent event;while(NInput::GetEvent(&event))if(menu->ProcessEvent(event))++handled;
        key.type=SDL_EVENT_KEY_UP;key.key.down=false;Require(SDL_PushEvent(&key),"idle-menu UI input could not release owned console key");S2Platform::PumpEvents();NInput::PumpMessages(true,true);while(NInput::GetEvent(&event))menu->ProcessEvent(event);
        Require(handled>0,"idle-menu fixture did not reach a real UI-consumed SDL key before checking the timestamp");
    }
    if(mode==7){ui.SetCurrent(120001);Require(SDL_MinimizeWindow(S2Platform::Window()),"idle-menu fixture could not minimize its owned window");SDL_SyncWindow(S2Platform::Window());S2Platform::PumpEvents();S2Platform::UpdateDisplay();minimized=!S2Platform::Display().drawable;Require(minimized,"idle-menu fixture did not establish the native nondrawable state");}
    if(mode!=2 && mode!=3 && mode!=6){MarkNewDGFrame();menu->Step();}
    STime stamp=0;std::size_t stampSize=0;const bool scalar=NativeMenuIdleStamp(menu,&stamp,&stampSize);CObj<NMainLoop::CInterfaceCommand> queued=NMainLoop::TakeInterfaceCommandForHarness();const bool expectedQueue=mode==1 || mode==4;
    const bool graph=NMainLoop::GetCurrentInterfaceForHarness()==menu.GetPtr() && menu->GetInterface()==root && menu->GetWorld()==world && menu->GetSoundScene()==sound;
    std::printf("MENU-IDLE-DEMO mode=%d ui=%u scalar=%d size=%u stamp=%u expectedStamp=%u queued=%d expectedQueue=%d handled=%d minimized=%d graph=%d\n",mode,unsigned(menu->GetUITime()),int(scalar),unsigned(stampSize),unsigned(stamp),unsigned(expectedStamp),int(IsValid(queued)),int(expectedQueue),handled,int(minimized),int(graph));std::fflush(stdout);
    Require(graph && !NMainLoop::HaveInterfaceCommand(),"idle-menu frame changed its world/UI/audio graph before transition execution or queued duplicate commands");
    Require(IsValid(queued)==expectedQueue,"actual main menu does not honor the strict two-minute idle threshold, time-only input or drawable gate");
    if(mode>=2 && mode<=6)Require(scalar && stamp==expectedStamp,"native main-menu save field2 does not hold the reset UI timestamp before/after focus or input");
    if(mode==5)Require(menu->GetUITime()==1384,"returning menu focus changed the original 150ms UI-counter catch-up behavior");
    if(mode==6)Require(handled>0,"idle-menu activity fixture did not reach an actual UI-consumed native console event");
    if(expectedQueue){queued->Exec();auto* demo=dynamic_cast<NGame::CMissionBase*>(NMainLoop::GetCurrentInterfaceForHarness());Require(demo && pSSClasses->GetObjectTypeID(demo)==int(0xB3723140) && demo!=menu.GetPtr(),"native menu idle command did not push a real demonstration modal");CObj<NMainLoop::CInterfaceCommand> snapshot=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"idle-demo transition did not isolate its existing mission-start snapshot");CObj<NMainLoop::CICExitModal> exit=new NMainLoop::CICExitModal;exit->Exec();Require(NMainLoop::GetCurrentInterfaceForHarness()==menu.GetPtr(),"demo dismissal did not focus its original menu");MarkNewDGFrame();menu->Step();Require(!NMainLoop::HaveInterfaceCommand(),"returning from an idle demonstration immediately launched it again");}
    if(minimized){Require(SDL_RestoreWindow(S2Platform::Window()),"idle-menu fixture could not restore its owned window");SDL_SyncWindow(S2Platform::Window());S2Platform::PumpEvents();}
    NMainLoop::DoneInterface();
}


class CachedPointReceiverReached: public std::exception {};
class CachedPointReceiverTriangles: public CFuncBase<vector<NGfx::STriangleList>>
{
    OBJECT_NOCOPY_METHODS(CachedPointReceiverTriangles);
protected:
    bool NeedUpdate() override {return true;}
    void Recalc() override {throw CachedPointReceiverReached();}
};

// Access existing cache inputs/state without changing production interfaces or layout.
struct CachedPointStateTag { friend auto CachedPointMember(CachedPointStateTag); };
template<auto Member> struct CachedPointStateAccess { friend auto CachedPointMember(CachedPointStateTag){return Member;} };
template struct CachedPointStateAccess<&NGScene::CLightmapTracker::rs>;
struct CachedPointRendererTag { friend auto CachedPointMember(CachedPointRendererTag); };
template<auto Member> struct CachedPointRendererAccess { friend auto CachedPointMember(CachedPointRendererTag){return Member;} };
template struct CachedPointRendererAccess<&NGScene::CLightmapTracker::pRender>;
struct CachedPointStepTag { using type=void(NGScene::CLightmapTracker::*)(NGfx::CRenderContext*,NGScene::CSceneFragments*,CTransformStack*,bool,int);friend type CachedPointMember(CachedPointStepTag); };
template<auto Member> struct CachedPointStepAccess { friend CachedPointStepTag::type CachedPointMember(CachedPointStepTag){return Member;} };
template struct CachedPointStepAccess<&NGScene::CLightmapTracker::RecalcStep>;

// Count actual occluder-list requests, forwarding every request into the real scene.
class ObservedCachedPointRenderer: public NGScene::CRenderWrapper
{
public:
    int shadowViews=0;
    explicit ObservedCachedPointRenderer(NGScene::CGScene* scene):CRenderWrapper(scene){}
    void FormDepthList(CTransformStack* ts,const CVec3& direction,NGScene::CSceneFragments* result,EDepthType type) override {
        ++shadowViews;CRenderWrapper::FormDepthList(ts,direction,result,type);
    }
};

static void NativeCachedPointShadowFlag(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=7,"unknown cached point shadow-flag case");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm cached point shadow flag regression");
    NGlobal::SetVar("gfx_resolution",wstring(L"320x240"));NGlobal::SetVar("gfx_fullscreen",0);Require(NGScene::SetModeFromConfig(false) && NGScene::UsePrecisePointShadows(),"cached point fixture lacks actual configured refinement targets");
    CObj<NGScene::CGScene> scene=new NGScene::CGScene(0);ObservedCachedPointRenderer renderer(scene);
    CObj<NGScene::CLightmapTracker> tracker=new NGScene::CLightmapTracker;
    NGScene::CLightState input;input.vAmbientColor=VNULL3;input.vUpDifColor=VNULL3;
    const bool firstShadow=mode==0 || mode==4;
    if(mode!=6)input.points.push_back(NGScene::CLightState::SPointLight(mode==3?VNULL3:CVec3(1,0.5f,0.25f),CVec3(0,0,2),8.f,firstShadow));
    if(mode==2 || mode==5)input.points.push_back(NGScene::CLightState::SPointLight(CVec3(0.25f,1,0.5f),CVec3(4,0,2),9.f,mode==5));
    CMemoryStream fields;{CStructureSaver writer(fields,CStructureSaver::WRITE);writer.Add(4,&input);}fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ);tracker->operator&(reader);}
    Require(tracker->GetLightState().points.size()==input.points.size(),"cached point fixture lost its actual saved light inputs");
    for(int i=0;i<input.points.size();++i)Require(tracker->GetLightState().points[i].bCastShadow==input.points[i].bCastShadow,"cached point fixture changed shadow flag through the actual light-state codec");
    auto& state=tracker.GetPtr()->*CachedPointMember(CachedPointStateTag{});state.nState=static_cast<decltype(state.nState)>(2);state.nStep=mode==4?1:0;state.bCalcSky=false;state.bCalcColor=true;
    (tracker.GetPtr()->*CachedPointMember(CachedPointRendererTag{}))=&renderer;
    CTransformStack transform;NGScene::CSceneFragments receivers;NGfx::CRenderContext context;
    if(mode==7){
        CObj<CachedPointReceiverTriangles> triangles=new CachedPointReceiverTriangles;CObj<AnimatedLightMaterial> material=new AnimatedLightMaterial;
        NGScene::SRenderGeometryInfo geometry;geometry.pTriLists[NGScene::TLT_GEOM]=triangles.GetPtr();geometry.pTriLists[NGScene::TLT_POSITION]=triangles.GetPtr();
        SBound bound;bound.SphereInit(CVec3(0,0,2),1.f);const int index=receivers.AddGeometry(nullptr,&geometry,bound);NGScene::CPartFlags parts;parts.Clear();parts.Set(0);receivers.AddElement(index,parts,material,nullptr,nullptr);
        bool reached=false;try{(tracker.GetPtr()->*CachedPointMember(CachedPointStepTag{}))(&context,&receivers,&transform,false,1);}catch(const CachedPointReceiverReached&){reached=true;}catch(const AnimatedLightRenderReached&){reached=true;}
        std::printf("CACHED-POINT-FLAG mode=7 receiverReached=%d expectedReceiver=1 shadowViews=%d expectedViews=0 state=%d step=%d\n",int(reached),renderer.shadowViews,int(state.nState),state.nStep);std::fflush(stdout);
        Require(reached && renderer.shadowViews==0,"nonshadowed cached point skipped its positive native receiver color traversal or still generated occluder views");return;
    }
    const int calls=mode==2 || mode==5?2:1;
    for(int i=0;i<calls;++i)(tracker.GetPtr()->*CachedPointMember(CachedPointStepTag{}))(&context,&receivers,&transform,false,1);
    const int expectedViews=mode==0 || mode==4 || mode==5?2:0;
    const int expectedStep=mode==0?1:mode==4?2:mode==5?5:0;
    const int expectedState=mode==0 || mode==4 || mode==5?2:5;
    std::printf("CACHED-POINT-FLAG mode=%d lights=%u firstShadow=%d calls=%d shadowViews=%d expectedViews=%d step=%d expectedStep=%d state=%d expectedState=%d\n",mode,unsigned(input.points.size()),int(firstShadow),calls,renderer.shadowViews,expectedViews,state.nStep,expectedStep,int(state.nState),expectedState);std::fflush(stdout);
    Require(renderer.shadowViews==expectedViews,"cached point light with shadows disabled still generated real cubemap occluder views or disturbed a shadow-casting neighbor");
    Require(state.nStep==expectedStep && int(state.nState)==expectedState,"cached point light did not complete its color work and advance to the next source without shadow passes");
}


class NativeSubzoneMissionAccess: public NGame::CMission
{
public:
    static auto Drain() { return &NativeSubzoneMissionAccess::ExecWorldCommands; }
};

static void NativeSubzoneTransition(const char* databasePath,const char* resourcePath,int mode,bool withTemporaryCompanion=false)
{
    Require(mode>=0 && mode<3,"unknown native subzone transition mode");
    Require(std::getenv("S2_USER_DATA_DIR") && !NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"subzone transition requires its isolated native profile and empty interface stack");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm native subzone termination ordering");
    NMainLoop::CreateProfile("UpstreamSubzoneTransition");NMainLoop::SetActiveProfile("UpstreamSubzoneTransition");
    auto* manager=NMainLoop::GetSaveManager();
    struct Slots {NMainLoop::CSaveManager* manager;~Slots(){manager->DeleteSlot(NMainLoop::S_SLOT_ACTIVE);}} slots{manager};
    NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(1);
    auto* zone=game->pScenarioTracker->GetZone(2291);
    Require(zone && zone->GetDBZone() && zone->GetDBZone()->sSmallDescription=="ETStore","subzone transition has no genuine ETStore campaign zone");
    game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    const int from=mode==1?2292:2291;
    CObj<NGame::CICBeginMission> enter=new NGame::CICBeginMission(zone,from,{},game);enter->Exec();
    CObj<NGame::CMission> mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(mission && mission->GetRPGGame()==game.GetPtr() && game->nCurrentTemplateID==from,"native subzone entry lost its actual mission/campaign/template");
    while(NMainLoop::HaveInterfaceCommand()){CObj<NMainLoop::CInterfaceCommand> start=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(start.GetPtr()),"native subzone entry queued an unrelated transition");}
    CObj<NWorld::CWorld> oldWorld=dynamic_cast<NWorld::CWorld*>(mission->GetWorld());Require(oldWorld && oldWorld->IsLinkedZone(),"native ETStore entry did not retain its linked-zone lifecycle");
    auto* player=dynamic_cast<NWorld::CPlayer*>(mission->GetActivePlayer()->GetPlayer());Require(player,"native subzone entry has no active human owner");
    vector<CPtr<NWorld::CUnitServer>> units;player->GetUnits(&units);Require(units.size()==1 && units[0]->CanFight(),"native subzone entry did not deploy its genuine mercenary");
    CObj<NRPG::CUnit> temporary;
    if(withTemporaryCompanion){
        list<CPtr<NWorld::CUnitServer>> all;oldWorld->GetAllUnits(&all);NWorld::CPlayer* previousOwner=nullptr;
        for(auto unit:all)if(unit && unit->GetPlayer()!=player && unit->GetTBSPlayer()){previousOwner=unit->GetTBSPlayer();break;}
        Require(previousOwner,"temporary subzone has no genuine non-party owner");
        vector<NAI::SPathPlace> places;oldWorld->GetPathNetwork()->GetNearPlaces(SSphere(units[0]->GetPosition().GetCP(),20),&places);NAI::SPathPlace place;bool free=false;
        for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetMoving(0);if(oldWorld->GetPathNetwork()->IsPassable(p) && !oldWorld->GetPathNetwork()->IsLocked(p)){place=p;free=true;break;}}
        Require(free,"temporary subzone has no actual free place");
        CObj<NRPG::IUnitMission> recruit=NRPG::CreateUnit(NDb::GetPers(103));temporary=recruit->GetRPGUnit();
        CObj<NWorld::CUnitServer> actor=oldWorld->AddUnitInGame(place,recruit,previousOwner,"NativeTemporaryCompanion");Require(actor,"temporary subzone could not deploy its native recruit");oldWorld->ChangeUnitPlayer(actor,player);
        Require(std::count(game->players[0]->zoneMercs.begin(),game->players[0]->zoneMercs.end(),temporary)==1,"native recruited companion was not retained in its global owner before passage");
        player->GetUnits(&units);Require(units.size()==2,"native recruitment did not create exactly two actual party actors");
    }
    const size_t expectedParty=withTemporaryCompanion?2:1;
    const string saveName=std::to_string(zone->GetDBZone()->GetRecordID())+"_"+std::to_string(from)+".sav";
    const wstring savedPath=manager->GetSlotFilePathW(NMainLoop::S_SLOT_ACTIVE,saveName);
    auto savedExists=[&](){CFileStream file;try{file.OpenRead(savedPath.c_str());return file.GetSize()>0;}catch(...){return false;}};
    Require(!savedExists(),"native subzone test inherited an outgoing world snapshot");
    if(mode==2){std::printf("SUBZONE-TRANSITION mode=2 from=%d current=%d party=%u saved=%d control=first-entry\n",from,game->nCurrentTemplateID,unsigned(units.size()),int(savedExists()));return;}
    std::set<int> groups;for(auto object:oldWorld->GetObjectsForHarness())if(auto* passage=dynamic_cast<NWorld::IPassageObject*>(object.GetPtr()))if(passage->GetPassageZoneID()>0)groups.insert(passage->GetPassageZoneID());
    int passageGroup=0;for(int group:groups)if(oldWorld->UsePassageObject(units[0],group,true)){passageGroup=group;break;}
    Require(passageGroup>0 && game->players[0]->deployData.bPassage,"real native ETStore passage did not queue its actual template transition");
    (mission.GetPtr()->*NativeSubzoneMissionAccess::Drain())();
    CObj<NMainLoop::CInterfaceCommand> pending=NMainLoop::TakeInterfaceCommandForHarness();
    auto* transition=dynamic_cast<NGame::CICBeginMission*>(pending.GetPtr());Require(transition && !NMainLoop::HaveInterfaceCommand(),"real mission did not defer its single native passage transition");
    const int to=transition->*BeginMissionCommandAccess::Template();Require(to>0 && to!=from,"real passage transition did not select a different native template");
    vector<CPtr<NWorld::CUnitServer>> before;player->GetUnits(&before);
    Require(before.size()==expectedParty && !savedExists() && mission->GetWorld()==oldWorld.GetPtr() && NMainLoop::GetCurrentInterfaceForHarness()==mission.GetPtr(),"outgoing mission was terminated inside its UI update instead of its deferred main-loop command");
    pending->Exec();
    auto* next=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(next && next!=mission.GetPtr() && next->GetWorld()!=oldWorld.GetPtr() && next->GetRPGGame()==game.GetPtr() && game->nCurrentTemplateID==to,"native subzone transition did not install its real replacement mission");
    list<CPtr<NWorld::CUnitServer>> remaining;oldWorld->GetAllUnits(&remaining);int oldParty=0;for(auto unit:remaining)if(unit->GetPlayer()==player)++oldParty;
    auto* nextPlayer=dynamic_cast<NWorld::CPlayer*>(next->GetActivePlayer()->GetPlayer());Require(nextPlayer,"replacement subzone has no native concrete player");
    vector<CPtr<NWorld::CUnitServer>> newParty;nextPlayer->GetUnits(&newParty);
    const bool saved=savedExists();
    std::printf("SUBZONE-TRANSITION mode=%d from=%d to=%d group=%d deferredParty=%u outgoingParty=%d newParty=%u saved=%d passageConsumed=%d\n",mode,from,to,passageGroup,unsigned(before.size()),oldParty,unsigned(newParty.size()),int(saved),int(!game->players[0]->deployData.bPassage));std::fflush(stdout);
    Require(oldParty==0 && saved,"deferred native subzone transition did not terminate/save the outgoing linked world before creating its replacement");
    Require(newParty.size()==expectedParty && newParty[0]->CanFight() && newParty[0]->GetUnitRPG()->GetRPGUnit()==game->players[0]->mercs[0].GetPtr() && !game->players[0]->deployData.bPassage,"native subzone transition duplicated/lost the actual mercenary or failed to consume its passage arrival");
    if(withTemporaryCompanion){int found=0;for(auto unit:newParty)found+=unit->GetUnitRPG()->GetRPGUnit()==temporary.GetPtr();Require(found==1 && std::count(game->players[0]->zoneMercs.begin(),game->players[0]->zoneMercs.end(),temporary)==1,"linked subzone transition lost/duplicated/released its temporary recruit");std::puts("TEMPORARY-COMPANION actual linked passage/save/redeployment passed");}
}

static void NativeTemporaryCompanions(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<6,"unknown temporary companion case");
    if(mode==4){NativeSubzoneTransition(databasePath,resourcePath,0,true);return;}
    NativeHearingOwners f(databasePath,resourcePath);
    auto* global=f.game->players.front().GetPtr();list<CPtr<NWorld::CUnitServer>> actual;f.world->GetAllUnits(&actual);f.human=nullptr;f.humanUnit=nullptr;
    for(auto unit:actual)if(unit && unit->CanFight() && unit->GetTBSPlayer() && unit->GetTBSPlayer()->GetGlobalPlayer()==global){f.human=unit->GetTBSPlayer();f.humanUnit=unit;break;}
    Require(f.human && f.humanUnit,"temporary companion fixture lacks its genuine global human owner");
    CObj<NRPG::CGlobalPlayer> retainedOld;NRPG::CGlobalPlayer* oldGlobal=nullptr;
    if(mode==0 || mode==5){retainedOld=NRPG::CreateGlobalPlayer(vector<int>{});retainedOld->bAIPlayer=true;f.game->players.push_back(retainedOld);f.ai=dynamic_cast<NWorld::CPlayer*>(f.world->AddPlayer(L"Temporary previous global owner",retainedOld,nullptr));Require(f.ai && f.ai->GetGlobalPlayer()==retainedOld.GetPtr(),"temporary previous owner not installed");f.world->ChangeUnitPlayer(f.aiUnit,f.ai);oldGlobal=retainedOld;}
    CObj<NRPG::CUnit> companion=f.aiUnit->GetUnitRPG()->GetRPGUnit();
    if(mode==0){
        oldGlobal->zoneMercs.push_back(companion);
        const auto position=f.aiUnit->GetPosition();const int ap=f.aiUnit->GetAP(),hp=f.aiUnit->GetUnitRPG()->GetTotalVP();const auto time=f.world->GetTime()->GetValue();
        f.world->ChangeUnitPlayer(f.aiUnit,f.human);
        Require(f.aiUnit->GetPlayer()==f.human && companion.GetPtr()==f.aiUnit->GetUnitRPG()->GetRPGUnit(),"native ownership transfer lost its actual actor/RPG identity");
        std::printf("TEMPORARY-COMPANION transfer new=%zu old=%zu\n",global->zoneMercs.size(),oldGlobal->zoneMercs.size());std::fflush(stdout);
        Require(std::count(global->zoneMercs.begin(),global->zoneMercs.end(),companion)==1 && std::count(oldGlobal->zoneMercs.begin(),oldGlobal->zoneMercs.end(),companion)==0,"actual owner transfer did not retain/release its temporary companion");
        f.world->ChangeUnitPlayer(f.aiUnit,f.ai);f.world->ChangeUnitPlayer(f.aiUnit,f.human);
        Require(std::count(global->zoneMercs.begin(),global->zoneMercs.end(),companion)==1 && std::count(oldGlobal->zoneMercs.begin(),oldGlobal->zoneMercs.end(),companion)==0,"repeated native ownership transfers duplicate or strand a companion");
        Require(f.aiUnit->GetPosition()==position && f.aiUnit->GetAP()==ap && f.aiUnit->GetUnitRPG()->GetTotalVP()==hp && f.world->GetTime()->GetValue()==time,"temporary ownership retention changed AP/HP/position/time");
    }else if(mode==1){
        CObj<NRPG::IUnitMission> deadMission=NRPG::CreateUnit(NDb::GetPers(103));CObj<NRPG::CUnit> dead=deadMission->GetRPGUnit();dead->Kill();
        global->zoneMercs={companion,dead};vector<CPtr<NRPG::CUnit>> alive;global->GetAliveUnits(&alive);
        std::printf("TEMPORARY-COMPANION alive count=%zu temporary=%d dead=%d\n",alive.size(),int(std::count(alive.begin(),alive.end(),companion.GetPtr())),int(std::count(alive.begin(),alive.end(),dead.GetPtr())));std::fflush(stdout);
        Require(std::count(alive.begin(),alive.end(),companion.GetPtr())==1 && std::count(alive.begin(),alive.end(),dead.GetPtr())==0 && std::count(alive.begin(),alive.end(),global->mercs.front().GetPtr())==1,"mission exit alive set omits a temporary companion or includes a dead actor");
        global->deployData.unitsDeployData[global->mercs.front().GetPtr()].pCorpse=companion;
        global->GetAliveUnits(&alive);Require(std::count(alive.begin(),alive.end(),companion.GetPtr())==1,"carried temporary companion duplicated in alive set");
    }else if(mode==2){
        f.aiUnit->MakeUnconscious(VNULL3,true,false);Require(companion->IsUnconscious() && !companion->IsDead(),"arrival control did not create actual unconscious RPG state");
        CObj<NRPG::CGlobalPlayer> arriving=NRPG::CreateGlobalPlayer(vector<int>{54});
        CObj<NRPG::IUnitMission> healthyMission=NRPG::CreateUnit(NDb::GetPers(103)),deadMission=NRPG::CreateUnit(NDb::GetPers(103));
        CObj<NRPG::CUnit> healthy=healthyMission->GetRPGUnit(),dead=deadMission->GetRPGUnit();dead->Kill();arriving->zoneMercs={healthy,companion,dead};f.game->players.push_back(arriving);
        CObj<NAI::CSequenceCommander> commander=new NAI::CSequenceCommander(f.world);
        auto* player=dynamic_cast<NWorld::CPlayer*>(f.world->AddPlayer(L"Temporary arrivals",arriving,commander));Require(player,"actual temporary arrival owner failed");commander->SetPlayer(player);
        vector<CPtr<NWorld::CUnitServer>> units;player->GetUnits(&units);int regular=0,temporary=0,downed=0;
        for(auto unit:units){auto* identity=unit->GetUnitRPG()->GetRPGUnit();regular+=identity==arriving->mercs.front().GetPtr();temporary+=identity==healthy.GetPtr();downed+=identity==companion.GetPtr() || identity==dead.GetPtr();}
        std::printf("TEMPORARY-COMPANION arrival regular=%d temporary=%d downed=%d total=%zu\n",regular,temporary,downed,units.size());std::fflush(stdout);
        Require(regular==1 && temporary==1 && downed==0 && units.size()==2,"actual AddPlayer dropped a healthy temporary or directly redeployed an unconscious/dead companion");
    }else if(mode==3){
        global->zoneMercs.push_back(companion);const auto regular=global->mercs;
        f.game->UpdateScenarioOnLeaveZone();
        std::printf("TEMPORARY-COMPANION exit temporary=%zu regular=%zu\n",global->zoneMercs.size(),global->mercs.size());std::fflush(stdout);
        Require(global->zoneMercs.empty() && global->mercs==regular,"scenario exit did not release temporary companions after processing, or changed the regular squad");
    }else{
        const auto own=global->zoneMercs,other=oldGlobal->zoneMercs;const int ap=f.aiUnit->GetAP(),hp=f.aiUnit->GetUnitRPG()->GetTotalVP();const auto position=f.aiUnit->GetPosition();auto random=GlobalGameRandom();
        f.world->ChangeUnitPlayer(f.aiUnit,f.ai);f.world->ChangeUnitPlayer(nullptr,f.human);f.world->ChangeUnitPlayer(f.aiUnit,nullptr);
        Require(global->zoneMercs==own && oldGlobal->zoneMercs==other && f.aiUnit->GetPlayer()==f.ai && f.aiUnit->GetAP()==ap && f.aiUnit->GetUnitRPG()->GetTotalVP()==hp && f.aiUnit->GetPosition()==position && random.Get()==GlobalGameRandom().Get(),"same-owner/null native transfer guards changed ownership, AP/HP/position/RNG");
        std::puts("TEMPORARY-COMPANION same-owner/null controls passed");
    }
}

static void NativeHealingPose(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<3,"unknown healing pose mode");
    NativeHearingOwners f(databasePath,resourcePath);NWorld::CUnitServer* actor=nullptr;list<CPtr<NWorld::CUnitServer>> actual;f.world->GetAllUnits(&actual);
    for(auto unit:actual)if(unit && unit->CanFight() && unit->GetTBSPlayer() && unit->GetTBSPlayer()->GetGlobalPlayer()==f.game->players.front().GetPtr()){actor=unit;break;}Require(actor,"healing fixture lacks its actual global human actor");
    auto position=actor->GetPosition();position.SetPose(mode==1?NAI::CROUCH:NAI::WALK);
    actor->SetPosition(position);actor->animator.SetWeaponAnimation(NDb::WT_DEFAULT);actor->animator.SetActiveItem(true);actor->animator.PlaceUnit(position);
    actor->animator.StartHealing(position,NAI::BH_TOP,mode==2,mode==2);
    f.world->GetTime()->Set(actor->animator.GetTimeEnd()+100);actor->animator.AlignTime();
    auto* body=actor->animator.GetSkeletonAnimator();Require(body,"healing has no actual skeleton animator");
    const auto prior=NativeCannonAnimationClips(body);if(mode!=2){bool started=false;for(auto* clip:prior){auto* record=NDb::GetAnimation(clip->GetRecordID());started|=record && record->nType==NDb::CAnimation::START_HEAL;}Require(started,"ordinary healing fixture did not produce an actual authored START_HEAL clip");}vector<CObj<NAnimation::CAnimation>> keep;for(auto* clip:prior)keep.push_back(clip);
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto time=f.world->GetTime()->GetValue();
    actor->animator.FinishHealing(position,mode==2);
    const auto after=NativeCannonAnimationClips(body);int freshPose=0,freshRepair=0;
    for(auto* clip:after)if(std::find(prior.begin(),prior.end(),clip)==prior.end()){
        auto* record=NDb::GetAnimation(clip->GetRecordID());if(record && record->nType==NDb::CAnimation::POSE)++freshPose;if(clip->GetRecordID()==0x1184)++freshRepair;
    }
    actor->animator.IdleBan(NAnimation::E_INTERNAL_IDLE_OFF,true);
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(body,&refs);int bones=0;CPtr<NAnimation::CASequence> sequence;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&sequence);reader.Add(3,&bones);}
    Require(bones>0 && sequence,"healing skeleton has no actual sequence/bones");NAnimation::SSkeletonPose frame(bones);sequence->GetFrame(actor->animator.GetTimeEnd()+100,&frame);
    bool finite=frame.size()==size_t(bones);for(const auto& joint:frame)finite&=std::isfinite(joint.pos.x) && std::isfinite(joint.pos.y) && std::isfinite(joint.pos.z);
    std::printf("HEALING-POSE mode=%d pose=%d freshStanding=%d freshRepair=%d bones=%d finite=%d AP=%d HP=%d\n",mode,int(position.GetPose()),freshPose,freshRepair,bones,int(finite),ap,hp);std::fflush(stdout);
    Require(finite && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && f.world->GetTime()->GetValue()==time,"finishing the visual healing pose changed native AP/HP/position/world time or produced an invalid frame");
    Require(mode==2?freshRepair>0 && freshPose==0:freshPose>0 && freshRepair==0,"healing idle overlay left the underlying healing pose, or repair lost its authored end clip");
}

static void NativeHeroSeven(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<3,"unknown hero seven mode");
    NativeDisplayContext display;display.Initialize(databasePath,resourcePath,"Silent Storm seventh party member input");NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(-1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>(6,54)));
    Require(game->players[0]->mercs.size()==6,"hero seven needs six genuine regular RPG actors");
    CObj<NRPG::IUnitMission> temporary=NRPG::CreateUnit(NDb::GetPers(103));if(mode!=2)game->players[0]->zoneMercs.push_back(temporary->GetRPGUnit());
    CObj<NGame::CICBeginMission> open=new NGame::CICBeginMission(810,-1,vector<string>(),game);open->Exec();
    CObj<NGame::CMission> mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());Require(mission && mission->GetWorld(),"hero seven did not open the actual mission");mission->PauseGame(true);mission->SetCheatVisibility(true);
    while(NMainLoop::HaveInterfaceCommand()){CObj<NMainLoop::CInterfaceCommand> pending=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(pending.GetPtr()),"hero seven has an unexpected startup command");}
    auto* world=dynamic_cast<NWorld::CWorld*>(mission->GetWorld());auto* owner=dynamic_cast<NWorld::CPlayer*>(mission->GetActivePlayer()->GetPlayer());Require(world && owner && owner->GetGlobalPlayer()==game->players[0].GetPtr(),"hero seven lacks its genuine global human owner");
    struct RestoreWorld {NWorld::CWorld* previous=NWorld::pCurrentWorld;~RestoreWorld(){NWorld::pCurrentWorld=previous;}} restore;NWorld::pCurrentWorld=world;world->ProcessTBSEvents();
    mission->GetActivePlayer()->Update(false);vector<CPtr<NGame::IUnitTracker>> units;mission->GetUnits(&units);Require(units.size()==size_t(mode==2?6:7),"hero seven mission tracker set lost its real regular/temporary party");
    vector<int> ap,hp;vector<NAI::SUnitPosition> positions;for(auto unit:units){auto* native=dynamic_cast<NWorld::CUnitServer*>(unit->GetUnit());Require(native,"hero seven tracker has no actual native actor");ap.push_back(native->GetAP());hp.push_back(native->GetUnitRPG()->GetTotalVP());positions.push_back(native->GetPosition());}
    mission->Select(units[0]->GetUnit(),false);vector<CPtr<NGame::IUnitTracker>> before;mission->GetSelectedUnits(&before);Require(before.size()==1 && before[0]==units[0],"hero seven initial native selection failed");
    NInput::SEvent event;while(NInput::GetEvent(&event)){};NInput::PostEvent(mode==1?"hero_6":"hero_7");Require(NInput::GetEvent(&event),"hero seven input queue did not deliver the real named bind");mission->ProcessEvent(event);
    vector<CPtr<NGame::IUnitTracker>> selected;mission->GetSelectedUnits(&selected);const int expected=mode==0?6:mode==1?5:0;
    std::printf("HERO-SEVEN mode=%d party=%zu selected=%zu expectedIndex=%d matched=%d\n",mode,units.size(),selected.size(),expected,int(selected.size()==1 && selected[0]==units[expected]));std::fflush(stdout);
    Require(selected.size()==1 && selected[0]==units[expected],"actual hero_7 command did not select member seven or changed the six-person guard/hero_6 control");
    for(size_t i=0;i<units.size();++i){auto* native=dynamic_cast<NWorld::CUnitServer*>(units[i]->GetUnit());Require(native->GetAP()==ap[i] && native->GetUnitRPG()->GetTotalVP()==hp[i] && native->GetPosition()==positions[i],"party selection changed native AP/HP/position");}
}

static void NativeStorePassage(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=4,"unknown native store passage mode");
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);NGScene::AddResourceDir(resourcePath);
    CObj<NRPG::CGlobalGame> campaign;NScenario::CScenarioZone* zone=nullptr;int scenario=0;
    for(int candidate=1;candidate<=3;++candidate){
        CObj<NRPG::CGlobalGame> current=NRPG::CreateGlobalGame(candidate);
        auto* found=current->pScenarioTracker->GetZone(2291);
        if(found && found->GetDBZone() && found->GetDBZone()->sSmallDescription=="ETStore"){campaign=current;zone=found;scenario=candidate;break;}
    }
    Require(campaign && zone,"store passage fixture has no actual campaign ETStore/template2291 association");
    int templateID=2291;
    if(mode>=2){vector<int> templates;zone->GetTemplatesIDs(&templates);std::sort(templates.begin(),templates.end());templateID=0;for(int id:templates)if(id>0 && id!=2291){templateID=id;break;}Require(templateID>0,"ETStore lacks a real ordinary companion template");}
    const int variantID=zone->GetVariantIDForTemplate(templateID);Require(variantID>0,"actual ETStore zone did not select its authored template variant");
    campaign->pCurrentZone=zone;campaign->nCurrentTemplateID=templateID;
    CObj<NWorld::CWorld> world=new NWorld::CWorld(campaign);CObj<NWorld::CPostWorldCreateInfo> post;
    world->CreateRandom(variantID,{},true,{},0,&post,zone->GetRandomSeedForTemplate(templateID));
    Require(post && world->GetPathNetwork(),"native ETStore world failed to construct");
    int passageZone=mode==0?1:0;list<CPtr<NWorld::IPassageObject>> passages;auto collectPassages=[&](int group){passages.clear();for(auto object:world->GetObjectsForHarness())if(auto* passage=dynamic_cast<NWorld::IPassageObject*>(object.GetPtr()))if(passage->GetPassageZoneID()==group)passages.push_back(passage);};collectPassages(passageZone);
    if(mode!=0 && passages.empty())for(int group=2;group<9 && passages.empty();++group){collectPassages(group);if(!passages.empty())passageZone=group;}
    Require(!passages.empty(),"actual ETStore world lacks its matching incoming passage group");
    auto* passage=passages.front().GetPtr();vector<NAI::SPathPlace> approaches;passage->GetObjectApproaches(&approaches);Require(!approaches.empty(),"actual ETStore passage has no real native approach");
    auto expected=world->GetPathNetwork()->GetDeployPlace(approaches.front(),0);
    string marker=passageZone==1?"unit1_l":"unit1_g";
    if(mode<2){auto* waypoint=world->GetWaypoint(marker);Require(waypoint,"actual ETStore underground variant lacks its authored party marker");expected=waypoint->pos.p;expected.SetPose(NAI::CM_CROUCH);Require(world->GetPathNetwork()->IsPassable(expected),"authored crouch arrival marker is not a real passable native navigation place");}
    std::set<std::uint32_t> formationExpected;int nativeSnapCount=0;
    if(mode==4){
        auto* network=world->GetPathNetwork();
        for(int i=0;i<2;++i){
            auto target=network->GetDeployPlace(approaches.front(),i-1);
            if(!network->IsPassable(target)){
                const NAI::SPosition requested(target,network);auto nearest=NAI::GetNearestPosition(requested.GetCP(),network);
                Require(nearest.IsValid() && nearest.GetFloor()<=requested.GetFloor(),"native nearest-placement control left the real arrival floor");
                nearest.p.SetPose(target.GetPose());nearest.p.SetDirection(target.GetDirection());
                std::printf("STORE-NATIVE-SNAP mode=4 requested=%08x nearest=%08x\n",unsigned(target.GetBits()),unsigned(nearest.p.GetBits()));
                target=nearest.p;++nativeSnapCount;
            }
            Require(network->IsPassable(target),"native formation control has no passable resolved target");formationExpected.insert(target.GetBits());
        }
    }
    const int partySize=mode==4?2:1;CObj<NRPG::CGlobalPlayer> party=NRPG::CreateGlobalPlayer(mode==4?vector<int>{54,103}:vector<int>{54});Require(party && party->mercs.size()==partySize,"native store party creation failed");campaign->players.push_back(party);
    party->deployData.bPassage=mode!=3;party->deployData.nPassageZoneID=passageZone;for(auto merc:party->mercs)party->deployData.unitsDeployData[merc.GetPtr()].nPassageObjectID=passage->GetPassageObjectID();
    CObj<NAI::CSequenceCommander> commander=new NAI::CSequenceCommander(world);
    auto* player=dynamic_cast<NWorld::CPlayer*>(world->AddPlayer(L"Native store arrival",party,commander,false));Require(player,"native passage AddPlayer did not create its owner");commander->SetPlayer(player);
    vector<CPtr<NWorld::CUnitServer>> units;player->GetUnits(&units);Require(units.size()==partySize,"native passage AddPlayer lost its actual mercenary");for(auto unit:units)Require(unit && unit->CanFight(),"native passage AddPlayer lost a live party member");
    if(mode==4){
        std::set<std::uint32_t> actualPlaces;const auto& expectedPlaces=formationExpected;
        for(auto unit:units)actualPlaces.insert(unit->GetPosition().pos.p.GetBits());
        std::printf("STORE-FORMATION mode=4 scenario=%d template=%d variant=%d group=%d nativeSnap=%d actualCells=%zu expectedCells=%zu",scenario,templateID,variantID,passageZone,nativeSnapCount,actualPlaces.size(),expectedPlaces.size());
        for(auto bits:actualPlaces)std::printf(" actual=%08x",unsigned(bits));for(auto bits:expectedPlaces)std::printf(" expected=%08x",unsigned(bits));std::printf(" passageConsumed=%d\n",int(!party->deployData.bPassage));std::fflush(stdout);
        Require(expectedPlaces.size()==partySize,"native ordinary passage formation lacks two distinct valid target cells");
        Require(actualPlaces==expectedPlaces,"native ordinary passage stacked the party or lost final formation/facing");
        Require(!party->deployData.bPassage && campaign->pCurrentZone==zone && campaign->nCurrentTemplateID==templateID,"native formation did not consume passage metadata while preserving zone/template");return;
    }
    const auto actual=units[0]->GetPosition().pos.p;
    if(mode==3){NAI::SPathPlace deploySpot;player->GetDeploySpot(&deploySpot);expected=world->GetPathNetwork()->GetDeployPlace(deploySpot,0);}
    std::printf("STORE-PASSAGE mode=%d scenario=%d template=%d variant=%d group=%d passage=%d marker=%s approach=%08x actual=%08x expected=%08x floor=%d pose=%d passageConsumed=%d\n",mode,scenario,templateID,variantID,passageZone,passage->GetPassageObjectID(),mode<2?marker.c_str():"ordinary",unsigned(approaches.front().GetBits()),unsigned(actual.GetBits()),unsigned(expected.GetBits()),units[0]->GetPosition().pos.GetFloor(),int(actual.GetPose()),int(!party->deployData.bPassage));std::fflush(stdout);
    Require(actual==expected,"native passage deployment ignored the authored crouch marker or final approach facing");
    Require(!party->deployData.bPassage && campaign->pCurrentZone==zone && campaign->nCurrentTemplateID==templateID,"native arrival did not consume passage metadata while preserving its actual zone/template");
}

template<class View>
static auto AddAuthoredPoint(View* view,const CVec3& color,const CVec3& origin,float radius,bool lightmap,bool shadow,int)
    ->decltype(view->AddPointLight(color,origin,radius,lightmap,shadow))
{return view->AddPointLight(color,origin,radius,lightmap,shadow);}
template<class View>
static auto AddAuthoredPoint(View* view,const CVec3& color,const CVec3& origin,float radius,bool lightmap,bool,long)
    ->decltype(view->AddPointLight(color,origin,radius,lightmap))
{return view->AddPointLight(color,origin,radius,lightmap);}
template<class Light>
static auto ConstructAuthoredPoint(bool shadow,int)->decltype(new Light(CVec3(1,.5f,.25f),CVec3(3,2,1),4.f,nullptr,false,shadow))
{return new Light(CVec3(1,.5f,.25f),CVec3(3,2,1),4.f,nullptr,false,shadow);}
template<class Light>
static auto ConstructAuthoredPoint(bool,long)->decltype(new Light(CVec3(1,.5f,.25f),CVec3(3,2,1),4.f,nullptr,false))
{return new Light(CVec3(1,.5f,.25f),CVec3(3,2,1),4.f,nullptr,false);}

static bool NativeAuthoredPointShadow(NGScene::CPointLight* light)
{
    Require(light,"point light route did not create the actual native light");
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(light,&refs);bool shadow=false;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(12,&shadow);}
    NGScene::CPointLight::SRadianceInfo radiance;light->GetRadianceInfo(&radiance,NGScene::RP_GF3_CL);
    Require(radiance.bCastShadow==shadow,"native radiance disagrees with the saved authored shadow flag");
    CObj<CObjectBase> fresh=pSSClasses->CreateObject(0x01961170);fields.Seek(0);
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);fresh->operator&(reader);}
    auto* restored=dynamic_cast<NGScene::CPointLight*>(fresh.GetPtr());Require(restored,"point-light save class has wrong native type");
    restored->GetRadianceInfo(&radiance,NGScene::RP_GF3_CL);Require(radiance.bCastShadow==shadow,"fresh native light reload lost the emitted shadow flag");
    return shadow;
}

class AuthoredLightView:public HeadStateView
{
    OBJECT_NOCOPY_METHODS(AuthoredLightView);
    CObj<NGScene::IGameView> actual;
public:
    vector<CObj<NGScene::CPointLight>> points;
    AuthoredLightView():actual(NGScene::CreateNewView()){}
    CObjectBase* KeepPoint(CObjectBase* object){
        if(!object)return nullptr;auto* light=dynamic_cast<NGScene::CPointLight*>(object);Require(light,"world point-light dispatch did not reach the actual game-view/scene factory");points.push_back(light);return object;
    }
    // Both native API shapes compile on the predecessor; neither synthesizes a light or shadow flag.
    CObjectBase* AddPointLight(const CVec3& color,const CVec3& origin,float radius,bool lightmap){return KeepPoint(AddAuthoredPoint(actual.GetPtr(),color,origin,radius,lightmap,false,0));}
    CObjectBase* AddPointLight(const CVec3& color,const CVec3& origin,float radius,bool lightmap,bool shadow){return KeepPoint(AddAuthoredPoint(actual.GetPtr(),color,origin,radius,lightmap,shadow,0));}
};

static void NativeAuthoredLight(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=6,"unknown authored point-light mode");
    const bool shadow=(mode&1)!=0;
    if(mode<=1){
        CObj<NGScene::CPointLight> point=ConstructAuthoredPoint<NGScene::CPointLight>(shadow,0);
        const bool actual=NativeAuthoredPointShadow(point);
        std::printf("AUTHORED-LIGHT mode=%d route=constructor actual=%d expected=%d\n",mode,int(actual),int(shadow));std::fflush(stdout);
        Require(actual==shadow,"native point-light constructor dropped the shadow input");return;
    }
    NativeDisplayContext display;
    if(mode<=3 || mode==6){
        CObj<NGScene::IGameView> view=NGScene::CreateNewView();
        CObj<CObjectBase> object=AddAuthoredPoint(view.GetPtr(),CVec3(1,.5f,.25f),CVec3(3,2,1),mode==6?0.f:4.f,false,shadow,0);
        if(mode==6){Require(!object,"zero-radius point light became a real light");std::printf("AUTHORED-LIGHT mode=6 route=game-view zeroRadius=1\n");return;}
        auto* point=dynamic_cast<NGScene::CPointLight*>(object.GetPtr());const bool actual=NativeAuthoredPointShadow(point);
        std::printf("AUTHORED-LIGHT mode=%d route=game-view actual=%d expected=%d\n",mode,int(actual),int(shadow));std::fflush(stdout);
        Require(actual==shadow,"native game-view/scene propagation dropped the authored shadow input");return;
    }
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,false,&campaign,4517);world->SetTimeOfDay(NWorld::TOD_ANYTIME);
    vector<std::pair<NDb::CContainerModel*,bool>> changed;
    vector<NDb::CObject*> visited;
    std::function<void(NDb::CObject*)> setFlags=[&](NDb::CObject* data){
        if(!data || std::find(visited.begin(),visited.end(),data)!=visited.end())return;visited.push_back(data);
        for(int stage=0;stage<NDb::N_DESTROY_STAGES;++stage){auto* model=data->pModels[stage].GetPtr();if(!model || model->ptPLightCr==VNULL3 || model->fPLightRadius<=0)continue;
            bool known=false;for(const auto& saved:changed)if(saved.first==model){known=true;break;}if(known)continue;
            changed.emplace_back(model,model->bPLightShadow);model->bPLightShadow=shadow;
        }
        setFlags(data->pChild);
    };
    for(auto object:world->GetObjectsForHarness())setFlags(object->GetDBObjectForHarness());
    struct RestoreFlags{vector<std::pair<NDb::CContainerModel*,bool>>& changed;~RestoreFlags(){for(auto saved:changed)saved.first->bPLightShadow=saved.second;}} restore{changed};
    Require(!changed.empty(),"authored-light fixture has no actual authored lamp model; not a shadow RED");
    CObj<AuthoredLightView> view=new AuthoredLightView;CObj<NRender::IRenderGame> render=NRender::CreateRenderGame(world,view,nullptr);
    render->UpdateViewWorld(false,world->GetTime()->GetValue(),nullptr,true);
    Require(!view->points.empty(),"actual world/render visitor never emitted a lamp; not a shadow RED");
    int wrong=0;for(auto point:view->points)wrong+=NativeAuthoredPointShadow(point)!=shadow;
    std::printf("AUTHORED-LIGHT mode=%d route=world models=%zu points=%zu wrong=%d expected=%d\n",mode,changed.size(),view->points.size(),wrong,int(shadow));std::fflush(stdout);
    Require(wrong==0,"actual world object/render visitor/game-view/scene chain dropped its authored shadow flag");
}

static CPtr<CObjectBase> NativeFootBlock(NWorld::CUnitServer* actor)
{
    vector<CObjectBase*> referenced;CStructureNetworkContext graph;
    graph.include=[&](CObjectBase* child){referenced.push_back(child);return false;};CMemoryStream traversal;
    {CStructureSaver writer(traversal,CStructureSaver::WRITE,&graph);actor->animator.operator&(writer);}
    CStructureNetworkContext refs;refs.externalReferences=true;refs.nextID=100;refs.include=[](CObjectBase*){return false;};
    for(auto* child:referenced)if(!refs.ids.count(child)){const auto id=refs.nextID++;refs.ids[child]=id;refs.refs[id]=child;}
    CMemoryStream fields;{CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);actor->animator.operator&(writer);}
    fields.Seek(0);CPtr<CObjectBase> terrain;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&terrain);}
    Require(terrain,"native animator lost its actual terrain function");
    CStructureNetworkContext terrainRefs;auto terrainFields=BorrowedObjectFields(terrain,&terrainRefs);
    CDGPtr<CPtrFuncBase<NAI::CHeightMapBlockInfo>> block;
    {CStructureSaver reader(terrainFields,CStructureSaver::READ,&terrainRefs);reader.Add(1,&block);}
    Require(IsValid(block),"native terrain lost its registered height DG node");return block.GetPtr();
}

static void NativeFootSupportProducer(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=4 && mode<=6,"unknown native foot producer mode");
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->CanFight() && !unit->IsEmptyPK() && !unit->IsWearingPK()){actor=unit;break;}
    Require(actor,"foot producer lacks a native ordinary actor");
    const auto initial=actor->GetPosition();auto destination=initial;
    CPtr<CObjectBase> block=NativeFootBlock(actor);
    // Populate the real runtime cache before moving, exercising its in-place path too.
    CDGPtr<CPtrFuncBase<NAI::CHeightMapBlockInfo>> cached=dynamic_cast<CPtrFuncBase<NAI::CHeightMapBlockInfo>*>(block.GetPtr());cached.Refresh();
    Require(cached->GetValue(),"native foot producer did not create its initial cache");
    if(mode!=4){
        auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(initial.GetCP(),4.f),&places);bool found=false;
        for(auto place:places){place.SetPose(initial.pos.p.GetPose());place.SetDirection(initial.GetDir());place.SetMoving(0);if(!network->IsPassable(place) || network->IsLocked(place))continue;auto next=initial;next.pos=NAI::SPosition(place,network);if(fabs(next.GetCP()-initial.GetCP())>0.4f){destination=next;found=true;break;}}
        Require(found,"foot producer lacks another real native navigation place");
        if(mode==5)actor->animator.PlaceUnit(destination);
        else {actor->animator.StartMove(false);actor->animator.Move(initial,destination,destination,false);}
    }
    Require(NativeFootBlock(actor)==block,"native movement replaced its terrain node unexpectedly");
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(block,&refs);CPtr<NAI::IAIMap> map;CVec3 center,special;int floor1=-99,floor2=-99;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&map);reader.Add(3,&center);reader.Add(5,&special);reader.Add(6,&floor1);reader.Add(7,&floor2);}
    const CVec3 expectedSpecial=mode==6?initial.GetCP():destination.GetCP();
    std::printf("FOOT-PRODUCER mode=%d center=%.6f,%.6f,%.6f special=%.6f,%.6f,%.6f expectedSpecial=%.6f,%.6f,%.6f floors=%d,%d expectedFloors=%d,%d map=%d stableNode=1\n",mode,center.x,center.y,center.z,special.x,special.y,special.z,expectedSpecial.x,expectedSpecial.y,expectedSpecial.z,floor1,floor2,destination.pos.GetFloor(),mode==6?initial.pos.GetFloor():destination.pos.GetFloor(),map.GetPtr()==world->GetAIMap());std::fflush(stdout);
    Require(center==destination.GetCP() && special==expectedSpecial && floor1==destination.pos.GetFloor() && floor2==(mode==6?initial.pos.GetFloor():destination.pos.GetFloor()) && map.GetPtr()==world->GetAIMap(),"native ctor/movement did not publish its actual foot-support samples");
    CObj<CObjectBase> restored=pSSClasses->CreateObject(0x11461141);fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ,&refs);restored->operator&(reader);}
    CDGPtr<CPtrFuncBase<NAI::CHeightMapBlockInfo>> decoded=dynamic_cast<CPtrFuncBase<NAI::CHeightMapBlockInfo>*>(restored.GetPtr());decoded.Refresh();cached.Refresh();
    auto* live=cached->GetValue();auto* reloaded=decoded->GetValue();Require(live->height.GetXSize()==reloaded->height.GetXSize() && live->height.GetYSize()==reloaded->height.GetYSize() && live->ptShift==reloaded->ptShift,"foot reload changed native cache coordinates");
    for(int y=0;y<live->height.GetYSize();++y)for(int x=0;x<live->height.GetXSize();++x)Require(live->height[y][x]==reloaded->height[y][x],"native movement and fresh reload computed different foot surfaces");
    Require(actor->GetPosition().GetCP()==initial.GetCP(),"animation-only foot sampling changed actual navigation position");
}

static void NativeFootSupport(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown foot support mode");
    CObj<NRPG::CGlobalGame> campaign;auto world=FixtureWorld(databasePath,resourcePath,true,&campaign);
    auto* map=world->GetAIMap();map->Sync();
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;for(auto unit:units)if(unit && unit->CanFight() && !unit->IsEmptyPK()){actor=unit;break;}
    Require(actor,"foot support lacks a real deployed owner");
    const float step=FP_GRID_STEP*0.333333f;
    const auto position=actor->GetPosition();CVec3 center=position.GetCP();
    center.x=Float2Int(center.x/step)*step;center.y=Float2Int(center.y/step)*step;
    const float ground=center.z;
    if(mode!=0)center.z+=0.75f;
    CVec3 special=center;if(mode==2){special.x+=step*4;special.z+=0.25f;}
    int floor1=mode==1?4:position.pos.GetFloor(),floor2=mode==3?position.pos.GetFloor():floor1;
    if(mode==3)floor1=4;
    CTRect<int> rect;NAI::CalcHeightMapSize(&rect,center,2.5f);
    NAI::CFastRenderer all,empty;all.InitParallel(CVec2(0,0),0,step,rect);empty.InitParallel(CVec2(0,0),0,step,rect);
    map->TraceGrid(&all,NWorld::TS_PASS_BLOCKER,NAI::IAIMap::STH_NOSORT,NAI::CFloorsSet());
    map->TraceGrid(&empty,NWorld::TS_PASS_BLOCKER,NAI::IAIMap::STH_NOSORT,NAI::CFloorsSet(4));
    int allCount=0,emptyCount=0;for(int y=0;y<all.resGrid.GetYSize();++y)for(int x=0;x<all.resGrid.GetXSize();++x){for(auto* p=all.resGrid[y][x];p;p=p->pNext)++allCount;for(auto* p=empty.resGrid[y][x];p;p=p->pNext)++emptyCount;}
    Require(allCount>0 && emptyCount==0,"foot support requires actual ground and an empty allowed floor4");
    // Decode only this new registered DG node, borrowing the native map. Never reload the live world.
    CObj<CObjectBase> object=pSSClasses->CreateObject(0x11461141);
    auto* node=dynamic_cast<CPtrFuncBase<NAI::CHeightMapBlockInfo>*>(object.GetPtr());Require(node,"registered height map node has wrong native type");
    CStructureNetworkContext refs;refs.externalReferences=true;refs.include=[](CObjectBase*){return false;};refs.ids[map]=100;refs.refs[100]=map;refs.nextID=101;
    CMemoryStream fields;CPtr<NAI::IAIMap> borrowed=map;
    {CStructureSaver writer(fields,CStructureSaver::WRITE,&refs);writer.Add(2,&borrowed);writer.Add(3,&center);writer.Add(5,&special);writer.Add(6,&floor1);writer.Add(7,&floor2);}
    fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ,&refs);object->operator&(reader);}
    CDGPtr<CPtrFuncBase<NAI::CHeightMapBlockInfo>> refreshed=node;refreshed.Refresh();auto* heights=node->GetValue();Require(heights,"native height map failed to recalculate");
    const CVec3 sample=mode==2?special:mode==1?center+CVec3(step*4,0,0):center;
    const float actual=heights->GetHeight(sample.x,sample.y),expected=mode==0?ground:mode==1?center.z:mode==2?special.z:center.z;
    const CVec3 normal=heights->GetNormal(sample.x,sample.y);
    int unionChangedCells=0;bool unionMatches=true;
    if(mode==3){
        auto surface=[&](int first,int second)->CObj<NAI::CHeightMapBlockInfo>{
            CObj<CObjectBase> fresh=pSSClasses->CreateObject(0x11461141);CMemoryStream payload;
            {CStructureSaver writer(payload,CStructureSaver::WRITE,&refs);writer.Add(2,&borrowed);writer.Add(3,&center);writer.Add(5,&special);writer.Add(6,&first);writer.Add(7,&second);}
            payload.Seek(0);{CStructureSaver reader(payload,CStructureSaver::READ,&refs);fresh->operator&(reader);}
            CDGPtr<CPtrFuncBase<NAI::CHeightMapBlockInfo>> evaluated=dynamic_cast<CPtrFuncBase<NAI::CHeightMapBlockInfo>*>(fresh.GetPtr());evaluated.Refresh();return evaluated->GetValue();
        };
        const int nativeFloor=position.pos.GetFloor();auto included=surface(nativeFloor,nativeFloor),excluded=surface(4,4);
        Require(included->height.GetXSize()==heights->height.GetXSize() && excluded->height.GetXSize()==heights->height.GetXSize() && included->height.GetYSize()==heights->height.GetYSize() && excluded->height.GetYSize()==heights->height.GetYSize(),"native floor controls changed sample dimensions");
        for(int y=0;y<heights->height.GetYSize();++y)for(int x=0;x<heights->height.GetXSize();++x){unionMatches=unionMatches && heights->height[y][x]==included->height[y][x];unionChangedCells+=heights->height[y][x]!=excluded->height[y][x];}
        std::printf("FOOT-FLOOR-UNION mode=3 matchesIncluded=%d differsExcluded=%d\n",int(unionMatches),unionChangedCells);
    }

    std::printf("FOOT-SUPPORT mode=%d floor=%d,%d ground=%.6f center=%.6f special=%.6f actual=%.6f expected=%.6f normal=%.6f,%.6f,%.6f traces=%d,%d\n",mode,floor1,floor2,ground,center.z,special.z,actual,expected,normal.x,normal.y,normal.z,allCount,emptyCount);std::fflush(stdout);
    Require(std::isfinite(actual) && fabs(actual-expected)<0.002f,"native foot support ignored saved center/special point/floor contract");
    Require(std::isfinite(normal.x) && std::isfinite(normal.y) && std::isfinite(normal.z),"native foot normal is not finite");
    if(mode==3)Require(unionMatches && unionChangedCells>0,"native height sampling ignored the second movement floor");
    Require(actor->GetPosition().GetCP()==position.GetCP(),"height sampling moved its native owner");
}

static void NativeCinematicFloorRange(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<3,"unknown native initial cinematic range mode");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"cinematic range fixture starts with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm initial cinematic floor range regression");
    struct RestoreConfig {NGlobal::CValue units=NGlobal::GetVar("autoplay_units"),maps=NGlobal::GetVar("autoplay_templates");STime time=NMainLoop::SetTimeForHarness(0);~RestoreConfig(){NGlobal::SetVar("autoplay_units",units);NGlobal::SetVar("autoplay_templates",maps);NMainLoop::SetTimeForHarness(time);}} restoreConfig;
    struct Bounds {NDb::CTemplVariant* variant;int minimum,maximum;};
    struct RestoreBounds {std::vector<Bounds> values;~RestoreBounds(){for(auto value:values){value.variant->nMinCutFloor=value.minimum;value.variant->nMaxCutFloor=value.maximum;}}} restoreBounds;
    auto* mapTemplate=NDb::GetTemplate(4412);Require(mapTemplate,"cinematic range fixture lacks its authored native template");
    const int rawMinimum=mode==1?-1:0,rawMaximum=mode==0?0:mode==1?2:1;
    CDBIterator<NDb::CTemplVariant> variants(*NDatabase::GetTable<NDb::CTemplVariant>());
    while(variants.MoveNext()) {
        auto* variant=variants.Get();if(variant->pTemplate.GetPtr()!=mapTemplate)continue;
        restoreBounds.values.push_back({variant,variant->nMinCutFloor,variant->nMaxCutFloor});
        variant->nMinCutFloor=rawMinimum;variant->nMaxCutFloor=rawMaximum;
    }
    Require(!restoreBounds.values.empty(),"cinematic range fixture did not configure real variants before native initialization");
    NGlobal::SetVar("autoplay_units",NGlobal::CValue(L"968"));NGlobal::SetVar("autoplay_templates",NGlobal::CValue(L"4412"));
    CObj<NGame::CICAutoPlay> open=new NGame::CICAutoPlay;open->Exec();
    CObj<NGame::CMission> mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(mission && pSSClasses->GetObjectTypeID(mission)==int(0xB3723140),"cinematic range fixture did not initialize an actual native mission");
    CObj<NMainLoop::CInterfaceCommand> snapshot=NMainLoop::TakeInterfaceCommandForHarness();
    Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"cinematic range fixture did not isolate the pending start snapshot");
    auto* camera=mission->GetCamera();auto* player=mission->GetActivePlayer();auto* world=mission->GetWorld();auto* view=mission->GetScene();auto* root=mission->GetInterface();auto* sound=mission->GetSoundScene();
    Require(camera && player && player->GetCamera() && world && view && root && sound && mission->IsSequence(),"cinematic range fixture lacks native selected cinematic/player/world/view/UI/audio");
    struct Base {CObj<ICamera> camera;int operator&(CStructureSaver& f){f.Add(18,&camera);return 0;}} base;
    struct Mission {Base* base;int operator&(CStructureSaver& f){f.Add(1,base);return 0;}} state{&base};
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(mission,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,&state);}
    Require(base.camera.GetPtr()==camera && camera!=player->GetCamera(),"native sequence did not select its separate cinematic camera");
    mission->PauseGame(true);
    const int expectedMinimum=mode==0?-3:rawMinimum,expectedMaximum=mode==0?4:rawMaximum-1;
    int playerMinimum,playerMaximum,minimum,maximum;
    player->GetCamera()->GetCutFloorRange(&playerMinimum,&playerMaximum);camera->GetCutFloorRange(&minimum,&maximum);
    Require(playerMinimum==expectedMinimum && playerMaximum==expectedMaximum,"native player cameras did not consume the configured real variant range");
    if(mode==2) {
        camera->SetCutFloor(4);camera->FreezeCamera(true);
        NMainLoop::SetTimeForHarness(25);Require(NMainLoop::StepApp(true,false,false),"native cinematic range frame failed");
    }
    const bool graph=mission.GetPtr()==NMainLoop::GetCurrentInterfaceForHarness() && mission->GetCamera()==camera && mission->GetWorld()==world && mission->GetScene()==view && mission->GetInterface()==root && mission->GetSoundScene()==sound && !NMainLoop::HaveInterfaceCommand();
    std::printf("CINEMATIC-FLOOR-RANGE mode=%d raw=%d,%d player=%d,%d cinematic=%d,%d expected=%d,%d cameraFloor=%d viewFloor=%d variants=%zu graph=%d\n",mode,rawMinimum,rawMaximum,playerMinimum,playerMaximum,minimum,maximum,expectedMinimum,expectedMaximum,camera->GetCutFloor(),view->GetCutFloor(),restoreBounds.values.size(),int(graph));std::fflush(stdout);
    Require(minimum==expectedMinimum && maximum==expectedMaximum,"initial native cinematic camera lost the variant's inclusive floor range");
    if(mode==2)Require(camera->GetCutFloor()==0 && view->GetCutFloor()==0,"native selected cinematic floor disagrees with its initialized single-floor view");
    Require(graph,"cinematic range initialization or frame replaced its live graph or queued an unexpected interface command");
    NMainLoop::DoneInterface();
}

static void NativeCameraFloorOwner(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=13,"unknown native camera floor-owner case");
    Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"camera floor-owner fixture starts with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm camera floor ownership regression");
    struct Restore { NGlobal::CValue units=NGlobal::GetVar("autoplay_units"),maps=NGlobal::GetVar("autoplay_templates");STime time=NMainLoop::SetTimeForHarness(0);~Restore(){NGlobal::SetVar("autoplay_units",units);NGlobal::SetVar("autoplay_templates",maps);NMainLoop::SetTimeForHarness(time);} } restore;
    NGlobal::SetVar("autoplay_units",NGlobal::CValue(L"968"));NGlobal::SetVar("autoplay_templates",NGlobal::CValue(L"4412"));
    CObj<NGame::CICAutoPlay> open=new NGame::CICAutoPlay;open->Exec();
    CObj<NGame::CMission> mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());Require(IsValid(mission) && pSSClasses->GetObjectTypeID(mission)==int(0xB3723140),"camera floor-owner fixture did not enter a real demo mission");
    CObj<NMainLoop::CInterfaceCommand> snapshot=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(snapshot.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"camera floor-owner fixture did not isolate the native mission-start snapshot");
    auto* camera=mission->GetCamera();auto* view=mission->GetScene();auto* world=mission->GetWorld();auto* root=mission->GetInterface();auto* sound=mission->GetSoundScene();Require(camera && view && world && root && sound && mission->IsSequence(),"camera floor-owner fixture lacks its real cinematic camera/view/world/UI/audio");mission->PauseGame(true);
    struct InitialBaseState {CObj<ICamera> cinematic;CObj<NGame::CUICmdLocatorExec> locator;int operator&(CStructureSaver& f){f.Add(18,&cinematic);f.Add(21,&locator);return 0;}} initial;
    struct InitialMissionState {InitialBaseState* base;int operator&(CStructureSaver& f){f.Add(1,base);return 0;}} initialMission{&initial};
    {CStructureNetworkContext refs;auto fields=BorrowedObjectFields(mission,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,&initialMission);}
    auto* active=mission->GetActivePlayer();Require(active && active->GetCamera() && initial.cinematic==camera,"camera floor initial observer lost the actual selected cinematic/player cameras");
    vector<CPtr<NGame::IPlayerTracker>> trackers;mission->GetPlayers(&trackers);int activeIndex=-1,currentIndex=-1;for(int i=0;i<trackers.size();++i){if(trackers[i]==active)activeIndex=i;if(trackers[i]->GetPlayer()==world->GetCurrentPlayer())currentIndex=i;}
    ICamera::SCameraPos cinematicPose,playerPose;camera->GetPlacement(&cinematicPose);active->GetCamera()->GetPlacement(&playerPose);
    std::printf("CAMERA-FLOOR-INIT mode=%d active=%d current=%d locator=%d cineFloor=%d playerFloor=%d viewFloor=%d cineAnchor=%.6g,%.6g,%.6g playerAnchor=%.6g,%.6g,%.6g poseEqual=%d\n",mode,activeIndex,currentIndex,int(IsValid(initial.locator)),camera->GetCutFloor(),active->GetCamera()->GetCutFloor(),view->GetCutFloor(),cinematicPose.ptAnchor.x,cinematicPose.ptAnchor.y,cinematicPose.ptAnchor.z,playerPose.ptAnchor.x,playerPose.ptAnchor.y,playerPose.ptAnchor.z,int(std::memcmp(&cinematicPose,&playerPose,sizeof(cinematicPose))==0));std::fflush(stdout);
    if(mode==8){const bool selectedCurrent=activeIndex==currentIndex;std::printf("CAMERA-FLOOR-OWNER mode=8 activeCurrent=%d expectedActiveCurrent=1 graph=1\n",int(selectedCurrent));std::fflush(stdout);Require(selectedCurrent,"mission initialization did not select the actual first acting AI before choosing its deploy camera");NMainLoop::DoneInterface();return;}
    if(mode==9){Require(!IsValid(initial.locator),"idle-camera handoff fixture unexpectedly has a native pending locator");const bool copied=std::memcmp(&cinematicPose,&playerPose,sizeof(cinematicPose))==0;const bool floor=camera->GetCutFloor()==active->GetCamera()->GetCutFloor() && view->GetCutFloor()==camera->GetCutFloor();std::printf("CAMERA-FLOOR-OWNER mode=9 poseCopied=%d expectedPoseCopied=1 floorCopied=%d expectedFloorCopied=1 graph=1\n",int(copied),int(floor));std::fflush(stdout);Require(copied && floor,"mission initialization did not hand off its selected deploy camera pose/floor to the idle cinematic camera before demo mode");NMainLoop::DoneInterface();return;}
    camera->SetCutFloorRange(-1,2);camera->SetCutFloor(1);view->SetCutFloorRange(-3,4);view->SetCutFloor(mode==0 || mode==6 || mode==13?1:0);
    Require(camera->GetCutFloor()==1 && view->GetCutFloor()==(mode==0 || mode==6 || mode==13?1:0),"camera floor-owner fixture did not establish independently controlled native camera/render floors");
    int expectedCamera=1,expectedView=mode==0 || mode==6 || mode==13?1:0,expectedMission=1;
    if(mode==1){camera->SetCutFloor(2);expectedCamera=expectedMission=2;}
    if(mode==2){mission->SetCutFloor(-1);expectedCamera=expectedMission=-1;}
    if(mode==3){view->SetCutFloor(2);expectedView=2;}
    if(mode==4){camera->SetCutFloor(2);camera->FreezeCamera(true);NMainLoop::SetTimeForHarness(50);MarkNewDGFrame();Require(NMainLoop::StepApp(true,false,false),"camera floor publication frame exited");expectedCamera=expectedMission=expectedView=2;camera->FreezeCamera(false);}
    if(mode==5){camera->SetWorld(nullptr);struct WorldRestore{ICamera* camera;NWorld::IWorld* world;~WorldRestore(){camera->SetWorld(world);}} restoreWorld{camera,world};camera->SetPlacement(ICamera::SCameraPos(CVec3(0,0,0),20.f,-FP_PI/4,.125f,0,35.f));camera->SetScreenRect(CTRect<float>(0,0,1,1));camera->ShowPlacesFromBestPoint(CVec3(-.125f,0,0),CVec3(-.125f,0,0),2,24.f,1,1.f,true,true);expectedCamera=expectedMission=2;}
    if(mode==6){camera->FreezeCamera(true);mission->SetCutFloor(2);camera->FreezeCamera(false);}
    if(mode==7){camera->SetCutFloorRange(0,1);camera->SetCutFloor(0);mission->SetCutFloor(9);}
    if(mode>=10){auto* hud=root->GetChildByID("missionUI");auto* panel=hud?hud->GetChildByID("unitpanel"):nullptr;auto* bar=panel?panel->GetChildByID("levelswitch"):nullptr;Require(bar && pSSClasses->GetObjectTypeID(bar)==int(0xB0241943),"camera floor-owner widget fixture did not find its actual native level switch bar");const char* command=mode==10 || mode==13?"up":mode==11?"down":"level_4";if(mode==13)camera->FreezeCamera(true);Require(bar->ProcessMessage(NUI::SEvent(NUI::EVENT_NOTIFY,string(command))),"actual native level switch bar did not consume its floor notification");if(mode==13)camera->FreezeCamera(false);expectedCamera=expectedMission=mode==11?0:mode==13?1:2;}
    const bool graph=NMainLoop::GetCurrentInterfaceForHarness()==mission.GetPtr() && mission->GetCamera()==camera && mission->GetScene()==view && mission->GetWorld()==world && mission->GetInterface()==root && mission->GetSoundScene()==sound && !NMainLoop::HaveInterfaceCommand();
    std::printf("CAMERA-FLOOR-OWNER mode=%d camera=%d expectedCamera=%d mission=%d expectedMission=%d view=%d expectedView=%d graph=%d\n",mode,camera->GetCutFloor(),expectedCamera,mission->GetCutFloor(),expectedMission,view->GetCutFloor(),expectedView,int(graph));std::fflush(stdout);
    Require(graph,"camera floor API/publication changed actual camera/world/UI/audio graph or queued a transition");
    Require(camera->GetCutFloor()==expectedCamera && mission->GetCutFloor()==expectedMission && view->GetCutFloor()==expectedView,"mission floor ownership ignores the selected camera, its clamp/freeze or deferred native scene publication");
    NMainLoop::DoneInterface();
}


static void NativeAttackSession(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=4,"attack session case out of range");
    if(mode>=3){
        NativeGrenadeThrowFixture fixture(databasePath,resourcePath,mode==4);CObj<NWorld::CUnitServer> actor=fixture.actor;auto* mission=actor->GetUnitRPG();auto* tracker=dynamic_cast<NRPG::CUnitMissionForMedals*>(mission);
        std::printf("ATTACK-SESSION mode=%d tracker=%d\n",mode,int(tracker!=nullptr));std::fflush(stdout);Require(tracker,"real native unit does not expose its transient attack-session tracker");CObj<NRPG::IUnitMission> heldMission=mission;Require(!tracker->bFinished && tracker->waitForBullets.empty(),"fresh native grenade actor starts with a stale attack session");
        fixture.executor->Run();fixture.executor->TimeLabelReached();Require(tracker->waitForBullets.size()==1 && IsValid(tracker->waitForBullets.front()),"real grenade release did not record its actual live projectile weakly");
        CPtr<CObjectBase> bullet=tracker->waitForBullets.front();const int ap=actor->GetAP(),vp=mission->GetTotalVP();const auto position=actor->GetPosition();fixture.executor->AnimationFinished();Require(tracker->bFinished && fixture.executor->GetState()==NWorld::CCommandExecute::FINISHED,"grenade animation did not finish the executor and its transient session");tracker->Segment();Require(tracker->bFinished && tracker->waitForBullets.size()==1 && tracker->waitForBullets.front()==bullet && IsValid(bullet),"session discarded a live awaited grenade");
        Require(actor->GetAP()==ap && mission->GetTotalVP()==vp && actor->GetPosition()==position,"transient grenade bookkeeping changed AP, health or position");
        fixture.world=nullptr;Require(IsValid(heldMission),"test lost its explicitly retained RPG session owner during world teardown");Require(!IsValid(bullet),"destroying the real owning world did not retire its actual projectile");tracker->Segment();Require(!tracker->bFinished && tracker->waitForBullets.empty(),"session failed to drain an expired actual projectile");
        std::printf("ATTACK-SESSION grenade=%d liveWait=1 finished=1 worldReleased=1 drained=1\n",int(mode==4));return;
    }
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;CObj<NWorld::CExecShootUnit> shot;
    for(auto unit:units)if(unit && !unit->IsAIUnit() && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK()){actor=unit;break;}
    Require(actor,"attack session fixture lacks its real healthy human actor");
    if(mode){auto* record=NDatabase::GetTable<NDb::CRPGWeapon>()->GetRecord(26);Require(record && record->pWeaponType && record->shootModes[NDb::SM_Snap],"attack session lacks authored firearm26");auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous=inventory->Get(NDb::SLOT_1)?inventory->TakeOff(NDb::SLOT_1):nullptr;CObj<NRPG::CWeaponItem> gun=new NRPG::CWeaponItem(record);Require(inventory->Equip(NDb::SLOT_1,gun) && inventory->Activate(NDb::SLOT_1) && gun->SetShootMode(NDb::SM_Snap) && gun->GetAmmoQuantity()>0,"attack session could not equip its real loaded native firearm");actor->animator.SetActiveItem(true);actor->SetUndrawItem(false);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());for(auto target:units){if(!target || target==actor || !target->CanFight())continue;CObj<NWorld::CExecShootUnit> candidate=new NWorld::CExecShootUnit(actor,target,NAI::HL_ANY,0,true);if(candidate->CanDoIt(actor->GetPosition())==NWorld::UCR_OK && actor->CanSpendAP(candidate->GetStartAP())){shot=candidate;break;}}Require(shot,"attack session fixture lacks a legal real native aim-and-hold target");}
    auto* mission=actor->GetUnitRPG();auto* weapon=mission->GetWeaponItem();const int ap=actor->GetAP(),vp=mission->GetTotalVP(),ammo=weapon?weapon->GetAmmoQuantity():0;const auto position=actor->GetPosition();
    if(mode==0){auto* tracker=dynamic_cast<NRPG::CUnitMissionForMedals*>(mission);std::printf("ATTACK-SESSION mode=0 tracker=%d\n",int(tracker!=nullptr));std::fflush(stdout);Require(tracker,"real native unit does not expose its transient attack-session tracker");Require(dynamic_cast<NRPG::IUnitMissionForMedals*>(mission)==static_cast<NRPG::IUnitMissionForMedals*>(tracker) && !tracker->bFinished && tracker->pGame==game && tracker->pMedalsGainer==mission->GetRPGUnit() && tracker->attackedInSession.empty() && tracker->hitInSession.empty() && tracker->criticalHits.empty() && tracker->hitsPK.empty() && tracker->waitForBullets.empty(),"native tracker lost fresh context or initial empty transient state");CStructureNetworkContext refs;auto fields=BorrowedObjectFields(mission,&refs);int scenarioID=-999;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(44,&scenarioID);}Require(scenarioID==actor->GetTBSPlayer()->GetScenarioPlayerID(),"native session relation context lost the actual owning scenario player");tracker->Segment();Require(!tracker->bFinished && tracker->waitForBullets.empty(),"idle native session segment changed its empty state");}
    else {shot->Run();Require(shot->GetState()==NWorld::CCommandExecute::RUNNING && shot->IsExecuting(),"actual aim-and-hold did not enter its pending native action");std::printf("ATTACK-SESSION-AIM mode=%d ap=%d expectedAP=%d health=%d expectedHealth=%d ammo=%d expectedAmmo=%d position=%d\n",mode,actor->GetAP(),ap,mission->GetTotalVP(),vp,weapon->GetAmmoQuantity(),ammo,int(actor->GetPosition()==position));std::fflush(stdout);Require(actor->GetAP()==ap && mission->GetTotalVP()==vp && weapon->GetAmmoQuantity()==ammo && actor->GetPosition()==position,"aim-only setup spent ammunition or changed AP, health, or position");if(mode==1){const bool keepLabels=shot->TimeLabelReached();std::printf("ATTACK-SESSION mode=1 keepLabels=%d running=%d executing=%d expectedRunning=1\n",int(keepLabels),int(shot->GetState()==NWorld::CCommandExecute::RUNNING),int(shot->IsExecuting()));std::fflush(stdout);Require(!keepLabels && shot->GetState()==NWorld::CCommandExecute::RUNNING && shot->IsExecuting(),"attack label prematurely finished the executor instead of closing only its transient session");auto* tracker=dynamic_cast<NRPG::CUnitMissionForMedals*>(mission);Require(tracker && tracker->bFinished && tracker->waitForBullets.empty(),"actual aim-and-hold label did not finish its empty transient session");shot->AnimationFinished();Require(shot->GetState()==NWorld::CCommandExecute::FINISHED,"actual animation end did not finish its executor");tracker->Segment();Require(!tracker->bFinished && tracker->waitForBullets.empty(),"native empty finished session did not settle");}else std::printf("ATTACK-SESSION mode=2 pendingAim=1 labelNotReached=1\n");}
    Require(actor->GetAP()==ap && mission->GetTotalVP()==vp && (!weapon || weapon->GetAmmoQuantity()==ammo) && actor->GetPosition()==position,"attack-session query/aim-only lifecycle changed AP, health, ammo or position");
}

static void CoverLegacyRayWire()
{
    struct LegacyRay{CVec3 ptDir;bool isPenetrate;float fDeviation;};static_assert(sizeof(LegacyRay)==20,"legacy saved cover ray must remain20bytes");
    vector<LegacyRay> hit(2),loose(1),obstacle(1);std::memset(hit.data(),0,hit.size()*sizeof(LegacyRay));std::memset(loose.data(),0,loose.size()*sizeof(LegacyRay));std::memset(obstacle.data(),0,obstacle.size()*sizeof(LegacyRay));hit[0].ptDir=CVec3(1,0,0);hit[0].isPenetrate=true;hit[0].fDeviation=.75f;hit[1].ptDir=CVec3(0,1,0);hit[1].fDeviation=.5f;loose[0].ptDir=CVec3(0,0,1);loose[0].fDeviation=.25f;obstacle[0].ptDir=CVec3(-1,0,0);obstacle[0].fDeviation=.125f;CVec3 origin(2,3,4);float apa=7.25f;
    CMemoryStream legacy;{CStructureSaver writer(legacy,CStructureSaver::WRITE);writer.Add(2,&hit);writer.Add(3,&origin);writer.Add(4,&loose);writer.Add(5,&obstacle);writer.Add(6,&apa);}legacy.Seek(0);CObj<CObjectBase> cover=pSSClasses->CreateObject(0x02841161);Require(cover && pSSClasses->GetObjectTypeID(cover)==int(0x02841161),"legacy cover registry identity changed");{CStructureSaver reader(legacy,CStructureSaver::READ);cover->operator&(reader);}CMemoryStream saved;{CStructureSaver writer(saved,CStructureSaver::WRITE);cover->operator&(writer);}Require(saved.GetSize()==legacy.GetSize() && !std::memcmp(saved.GetBuffer(),legacy.GetBuffer(),legacy.GetSize()),"cover rays no longer round-trip the original three-field20byte saved payload exactly");
    std::printf("COVER-LEGACY-WIRE ray=20 hit=2 loose=1 obstacle=1 bytes=%lld exact=1 registry=0x02841161\n",static_cast<long long>(saved.GetSize()));
}

// LAB draft only. Append after existing native fixture definitions after177 is pushed clean.
template<class T> static auto NativeAddAutomaticFOV(T* camera,const CVec3& anchor,float yaw,STime duration,int divisor,int)
    -> decltype(camera->AddFOVEffect(anchor,yaw,duration,divisor),bool())
{camera->AddFOVEffect(anchor,yaw,duration,divisor);return true;}
template<class T> static bool NativeAddAutomaticFOV(T*,const CVec3&,float,STime,int,long){return false;}
template<class T> static auto NativeAutomaticUnitPOI(T* actor,CVec3* point,int)
    -> decltype(actor->GetPointOfInterest(point),bool()) {return actor->GetPointOfInterest(point);}
template<class T> static bool NativeAutomaticUnitPOI(T*,CVec3*,long){return false;}

static void NativeAutomaticCameraPOI(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=8,"unknown automatic-camera POI mode");
    CObj<NRPG::CGlobalGame> game;CObj<NWorld::CWorld> world;CObj<NWorld::CUnitServer> actor,target;CObj<NWorld::CCannon> cannon;
    if(mode==4){auto fixture=MakeNativeCannonFixture(databasePath,resourcePath);game=fixture.game;world=fixture.world;actor=fixture.actor.GetPtr();cannon=fixture.cannon.GetPtr();}
    else {world=FixtureWorld(databasePath,resourcePath,true,&game);vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);for(auto unit:units){auto* actual=dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());if(actual && actual->CanFight() && !actual->IsWearingPK() && !actual->IsEmptyPK() && actual->IsAIUnit() && NAI::GetAIUnit(actual)){actor=actual;break;}}}
    Require(world && actor,"automatic-camera POI lacks actual actor/world");
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);for(auto unit:units){auto* actual=dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());if(actual && actual!=actor && actual->CanFight()){target=actual;break;}}
    Require(target,"automatic-camera POI lacks actual target");
    CObj<NAI::IAIUnit> brain=mode==4?NAI::CreateAIUnit(actor,false):NAI::GetAIUnit(actor);Require(brain && brain->GetUnitServer()==actor.GetPtr(),"automatic-camera POI lacks native brain binding");
    CObj<NAI::CAILogic> logic=new NAI::CAILogic(brain);const auto destination=target->GetPosition().pos;CVec3 expected=destination.GetCP(),observed(101,102,103);CObj<NWorld::CCmd> command;
    switch(mode){case 0:case 6:case 7:case 8:command=new NWorld::CCmdPath(destination);break;case 1:command=new NWorld::CCmdLook(destination);break;case 2:command=new NWorld::CCmdShootObject(target,0);Require(world->GetAIMap()->GetUnitHLPos(&expected,world->GetAIMap()->GetHull(target),-1),"POI target hull lacks actual center");break;case 3:expected+=CVec3(.125f,.25f,.5f);command=new NWorld::CCmdShootTile(expected);break;case 4:command=new NWorld::CCmdCannon(cannon);Require(cannon && world->GetAIMap()->GetUnitHLPos(&expected,world->GetAIMap()->GetHull(cannon),-1),"POI cannon hull lacks actual center");break;case 5:command=new NWorld::CCmdReload;break;}
    if(mode==7){Require(!world->IsSequence() && brain->IsUnderAIControl(),"combat POI requires actual normal AI context");brain->SetLogic(logic);Require(brain->GetLogic()==logic.GetPtr(),"combat POI did not install actual active logic");}
    if(mode==8){world->StartSequence();Require(world->IsSequence(),"script POI did not enter native sequence");brain->SetRouteLogic(logic);Require(brain->GetLogic()==logic.GetPtr(),"script POI did not install actual sequence route");}
    const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();auto random=GlobalGameRandom();
    logic->DoCommand(new NWorld::CCmdSetCommand(actor,command));if(mode==6)logic->DoCommand(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdReload));
    const bool found=mode>=7?NativeAutomaticUnitPOI(actor.GetPtr(),&observed,0):logic->GetPointOfInterest(&observed),wanted=mode!=5;const float error=wanted?fabs(observed-expected):fabs(observed-CVec3(101,102,103));
    std::printf("AUTOMATIC-CAMERA-POI mode=%d found=%d expected=%d error=%.9g queued=%zu sequence=%d\n",mode,int(found),int(wanted),error,logic->commands.size(),int(world->IsSequence()));std::fflush(stdout);
    Require(logic->commands.size()==(mode==6?2u:1u) && actor->GetPosition()==position && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"POI queuing changed physical actor/time/RNG or queue length");
    Require(found==wanted && error<1e-5f,"actual command/active-logic/commander/unit chain lost point of interest");
}

static void NativeAutomaticCameraFOV(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=5,"unknown automatic-camera FOV mode");CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);Require(game && world->GetGlobalGame()==game.GetPtr(),"FOV fixture lacks actual campaign/world");
    struct RestoreCount{NRPG::CGlobalGame* game;int count;~RestoreCount(){game->nSloMoTimes=count;}} restore{game,game->nSloMoTimes};game->nSloMoTimes=3;
    const int divisor=mode==1?2:mode==2?0:1,window=divisor>1?3/divisor:3;unsigned seed=0;for(unsigned trial=1;trial<10000;++trial){GlobalGameRandom().SeedForHarness(trial);SRand local;const int roll=local.Get(window);if(mode==3?roll>0:roll==0){seed=trial;break;}}Require(seed,"FOV fixture lacks native local zero-roll input");
    CObj<ICamera> camera=::CreateCamera(CAMERA_PC);Require(camera,"FOV fixture lacks native camera");if(mode!=4)camera->SetWorld(world);camera->SetScreenRect(CTRect<float>(0,0,1,1));camera->SetCutFloorRange(0,4);camera->SetCutFloor(0);const CVec3 anchor(0,0,1000);const float yaw=.125f;const STime duration=100;
    camera->SetPlacement(ICamera::SCameraPos(anchor,20.f,-FP_PI/4,yaw,0,mode==5?61.f:35.f));if(mode==5){ICamera::SCameraLimits limits;camera->GetLimits(&limits);limits.bMovie=true;camera->SetLimits(limits);}
    ICamera::SCameraPos liveBefore;camera->GetPlacement(&liveBefore);const auto time=world->GetTime()->GetValue();GlobalGameRandom().SeedForHarness(seed);auto random=GlobalGameRandom();
    const bool api=NativeAddAutomaticFOV(camera.GetPtr(),anchor,yaw,duration,divisor,0);S2CameraWire::SCameraFOVEffect effect;ICamera::SCameraPos desired;
    auto read=[&](){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(camera,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(11,&effect);reader.Add(4,&desired);};read();ICamera::SCameraPos liveAfter;camera->GetPlacement(&liveAfter);
    std::printf("AUTOMATIC-CAMERA-FOV mode=%d api=%d divisor=%d window=%d seed=%u ratio=%d duration=%u anchorError=%.9g\n",mode,int(api),divisor,window,seed,camera->GetSloMoRatio(),unsigned(effect.tMaxLen),fabs(desired.ptAnchor-(anchor+CVec3(0,0,1.5f))));std::fflush(stdout);
    Require(std::memcmp(&liveBefore,&liveAfter,sizeof(liveBefore))==0 && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get() && game->nSloMoTimes==3,"FOV request changed live placement/world clock/gameplay RNG or cheat slow-motion count");
    if(mode==3 || mode==4){Require(camera->GetSloMoRatio()==1 && effect.tMaxLen==0 && std::memcmp(&desired,&liveBefore,sizeof(desired))==0,"rejected FOV request changed camera");camera->FreezeCamera(true);camera->Update(1000);camera->FreezeCamera(false);Require(camera->GetSloMoRatio()==1 && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"inactive FOV control changed native world time/RNG or effect");return;}
    Require(api && camera->GetSloMoRatio()==1000 && effect.tMaxLen==duration && effect.tOn==0 && effect.tOnFOVSpring==0 && desired.fRod==12.f && fabs(desired.ptAnchor-(anchor+CVec3(0,0,1.5f)))<1e-5f,"native FOV request failed to establish its delayed effect");
    const auto originalDesired=desired;const auto originalEffect=effect;NativeAddAutomaticFOV(camera.GetPtr(),anchor+CVec3(10,10,0),yaw+1,999,divisor,0);read();Require(std::memcmp(&effect,&originalEffect,sizeof(effect))==0 && std::memcmp(&desired,&originalDesired,sizeof(desired))==0,"active FOV request was replaced");
    // Public placement/CameraLock simulate arrival without any saved-state injection.
    camera->SetPlacement(desired);camera->FreezeCamera(true);camera->Update(1000);read();Require(effect.nSloMo==3 && effect.tOn==1000 && effect.tOnFOVSpring==1000,"actual FOV update did not settle the delayed spring");camera->Update(1100);read();Require(effect.nSloMo==3,"FOV expired at equality instead of strict elapsed greater-than");camera->Update(1101);read();camera->GetPlacement(&liveAfter);Require(effect.nSloMo==1 && effect.fFOV==35.f && effect.fRoll==0 && camera->GetSloMoRatio()==1,"actual FOV effect failed to expire/reset");
    Require(liveAfter.fFOV==(mode==5?61.f:35.f),"FOV update changed movie FOV policy");camera->FreezeCamera(false);Require(random.Get()==GlobalGameRandom().Get() && world->GetTime()->GetValue()==time,"FOV timer changed native world time or gameplay RNG");
}

static void NativeAutomaticCameraFollowing(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=2,"unknown automatic-camera follow mode");Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"native following starts with interface");NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm native camera following regression");
    struct Restore{NGlobal::CValue units=NGlobal::GetVar("autoplay_units"),maps=NGlobal::GetVar("autoplay_templates");STime time=NMainLoop::SetTimeForHarness(0);~Restore(){NGlobal::SetVar("autoplay_units",units);NGlobal::SetVar("autoplay_templates",maps);NMainLoop::SetTimeForHarness(time);}}restore;
    NGlobal::SetVar("autoplay_units",NGlobal::CValue(L"968"));NGlobal::SetVar("autoplay_templates",NGlobal::CValue(L"4412"));CObj<NGame::CICAutoPlay> open=new NGame::CICAutoPlay;open->Exec();CObj<NGame::CMission> mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());Require(mission && mission->GetScene(),"following lacks actual initialized mission/view");CObj<NMainLoop::CInterfaceCommand> startSave=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(startSave.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"following did not isolate initial save");mission->PauseGame(true);CObj<NWorld::CWorld> world=dynamic_cast<NWorld::CWorld*>(mission->GetWorld());Require(world,"following lacks actual mission world");vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnit* actor=nullptr;for(auto unit:units)if(unit && unit->CanFight() && !unit->IsEmptyPK()){actor=unit;break;}Require(actor,"follow fixture lacks actual hull owner");CVec3 head;Require(world->GetAIMap()->GetUnitHLPos(&head,world->GetAIMap()->GetHull(actor),0),"follow fixture lacks actual native locator");
    CObj<ICamera> camera=mission->CreateCamera(CAMERA_PC);Require(camera,"following factory lacks actual independent tactical camera");ICamera::SCameraLimits limits;camera->GetLimits(&limits);limits.bMovie=false;camera->SetLimits(limits);camera->SetScreenRect(CTRect<float>(0,0,1,1));camera->SetCutFloorRange(-3,4);const auto zone=world->GetMapSafeZone();CVec3 initial=head;float initialDistance=0;for(const auto& candidate:vector<CVec3>{CVec3(zone.x1,zone.y1,head.z),CVec3(zone.x1,zone.y2,head.z),CVec3(zone.x2,zone.y1,head.z),CVec3(zone.x2,zone.y2,head.z)}){const float distance=fabs(CVec2(candidate.x-head.x,candidate.y-head.y));if(distance>initialDistance){initial=candidate;initialDistance=distance;}}Require(initialDistance>3.f,"following lacks an actual distant point inside map limits");camera->SetPlacement(ICamera::SCameraPos(initial,40.f,-FP_PI/4,.125f,0,35.f));camera->Update(1000);camera->FollowUnit(actor);if(mode!=1)camera->SetLock(true);const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();camera->Update(mode==2?11000:1050);ICamera::SCameraPos desired;{CStructureNetworkContext refs;auto fields=BorrowedObjectFields(camera,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(4,&desired);}const float distance=fabs(CVec2(desired.ptAnchor.x-head.x,desired.ptAnchor.y-head.y));
    std::printf("AUTOMATIC-CAMERA-FOLLOW mode=%d locked=%d distance=%.9g expectedFollow=%d\n",mode,int(mode!=1),distance,int(mode==0));std::fflush(stdout);Require(mode==0?distance<2.f:fabs(CVec2(desired.ptAnchor.x-initial.x,desired.ptAnchor.y-initial.y))<1e-4f,"locked native follow lost its active lease or ran outside the locked/live-lease branch");Require(world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"camera following changed physical world time/gameplay RNG");if(mode!=1)camera->SetLock(false);camera=nullptr;Require(NMainLoop::GetCurrentInterfaceForHarness()==mission.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"following replaced actual mission");NMainLoop::DoneInterface();
}

static void NativeAutomaticCameraMission(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=2,"unknown automatic-camera mission mode");Require(!NMainLoop::GetCurrentInterfaceForHarness() && !NMainLoop::HaveInterfaceCommand(),"automatic camera mission starts with an interface");NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm automatic camera regression");
    struct Restore{NGlobal::CValue units=NGlobal::GetVar("autoplay_units"),maps=NGlobal::GetVar("autoplay_templates");STime time=NMainLoop::SetTimeForHarness(0);~Restore(){NGlobal::SetVar("autoplay_units",units);NGlobal::SetVar("autoplay_templates",maps);NMainLoop::SetTimeForHarness(time);}} restore;
    NGlobal::SetVar("autoplay_units",NGlobal::CValue(L"968"));NGlobal::SetVar("autoplay_templates",NGlobal::CValue(L"4412"));CObj<NGame::CICAutoPlay> open=new NGame::CICAutoPlay;open->Exec();CObj<NGame::CMission> mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());Require(mission && mission->IsSequence(),"automatic camera did not initialize actual tactical sequence");CObj<NMainLoop::CInterfaceCommand> startSave=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(startSave.GetPtr()) && !NMainLoop::HaveInterfaceCommand(),"automatic camera did not isolate initial save");mission->PauseGame(true);
    CObj<ICamera> camera=mission->GetEnemyTurnCamera();auto* player=mission->GetActivePlayer();CObj<NWorld::CWorld> world=dynamic_cast<NWorld::CWorld*>(mission->GetWorld());Require(camera && player && player->GetCamera() && world,"automatic camera mission lacks actual camera/player/world");
    struct Base{int locks=-99;int operator&(CStructureSaver& f){f.Add(2,&locks);return 0;}}base;{CStructureNetworkContext refs;auto fields=BorrowedObjectFields(camera,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,&base);}
    if(mode==0){std::printf("AUTOMATIC-CAMERA-MISSION mode=0 initialLocks=%d\n",base.locks);std::fflush(stdout);Require(base.locks>=1,"initialized cinematic camera lost its persistent scroll lock");NMainLoop::DoneInterface();return;}
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnit* actor=nullptr;for(auto unit:units)if(unit && unit->GetPlayer()==player->GetPlayer() && unit->CanFight() && !unit->IsEmptyPK()){actor=unit;break;}Require(actor,"automatic UI framing lacks actual visible party actor");CVec3 primary;actor->GetRealPosition(&primary);const CVec3 expected=primary+CVec3(0,0,.5f);auto* server=dynamic_cast<NWorld::CUnitServer*>(actor);Require(server,"automatic UI actor is not native server");const int ap=server->GetAP(),hp=server->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();
    // Keep actual mission/UI execution; use the supported no-world camera context to isolate framing from terrain/LOS.
    {camera->SetWorld(nullptr);struct RestoreWorld{ICamera* camera;NWorld::IWorld* world;~RestoreWorld(){camera->SetWorld(world);}}restoreWorld{camera,world};camera->SetScreenRect(CTRect<float>(0,0,1,1));camera->SetPlacement(ICamera::SCameraPos(primary+CVec3(1000,1000,0),40.f,-FP_PI/4,.125f,0,35.f));player->GetCamera()->SetPlacement(ICamera::SCameraPos(primary,24.f,-FP_PI/4,.125f,0,35.f));auto random=GlobalGameRandom();
        CObj<NWorld::CUICmdUnitCamera> command=new NWorld::CUICmdUnitCamera(mode==1?nullptr:actor,NWorld::PR_UNIT_IS_DEAD,false,1.f,mode==1?actor:nullptr);CObj<NGame::CUICmdLocatorExec> executor=NGame::CreateCameraExecutor(command,mission);Require(executor,"actual UI camera factory rejected native command");const bool done=executor->Update(1200);ICamera::SCameraPos desired;{CStructureNetworkContext refs;auto fields=BorrowedObjectFields(camera,&refs);CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(4,&desired);}const float error=fabs(desired.ptAnchor-expected);std::printf("AUTOMATIC-CAMERA-MISSION mode=%d realTime=%d activeAI=%d done=%d anchorError=%.9g rod=%.9g initialLocks=%d\n",mode,int(mission->IsRealTime()),int(IsValid(player->GetGlobalPlayer()) && player->GetGlobalPlayer()->bAIPlayer),int(done),error,desired.fRod,base.locks);std::fflush(stdout);executor->Cancel();executor=nullptr;
        Require(!done && error<1e-4f,"actual UI executor lost target-only framing or original half-height interest point");Require(server->GetAP()==ap && server->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"automatic UI framing changed physical actor/world time/gameplay RNG");}
    Require(NMainLoop::GetCurrentInterfaceForHarness()==mission.GetPtr() && !NMainLoop::HaveInterfaceCommand(),"automatic UI framing replaced live mission");NMainLoop::DoneInterface();
}






// LAB draft only. Required native headers: GRects.h,2DScene.h,GfxUtils.h.
// Add after all existing native fixtures, only after178 has been pushed clean.
template<class F,class... Args> static void NativeInvokeFractionalLegacy(F function,Args&&... args)
{function(std::forward<Args>(args)...);}
template<class T> static auto NativeFractionalDirect(NGfx::C2DQuadsRenderer* quads,const CRectLayout& layout,const CTPoint<T>& point,const CTRect<T>& clip,NGScene::ELayoutRenderMode mode,int)
    -> decltype(static_cast<void(*)(NGfx::C2DQuadsRenderer*,NGfx::CTexture*,const CRectLayout&,const CTPoint<T>&,const CTRect<T>&,float,NGScene::ELayoutRenderMode,bool)>(&NGScene::RenderRectLayoutClipped),bool())
{NGScene::RenderRectLayoutClipped(quads,nullptr,layout,point,clip,0.f,mode,false);return true;}
template<class T> static bool NativeFractionalDirect(NGfx::C2DQuadsRenderer* quads,const CRectLayout& layout,const CTPoint<T>& point,const CTRect<T>& clip,NGScene::ELayoutRenderMode mode,long)
{NativeInvokeFractionalLegacy(&NGScene::RenderRectLayoutClipped,quads,static_cast<NGfx::CTexture*>(nullptr),layout,CTPoint<int>(int(point.x),int(point.y)),CTRect<int>(int(clip.x1),int(clip.y1),int(clip.x2),int(clip.y2)),0.f,mode,false);return false;}
template<class C> static auto NativeFractionalDraw(C* owner,const CRectLayout& layout,const CTPoint<float>& point,const CTRect<float>& clip,int)
    -> decltype(static_cast<void(C::*)(CPtrFuncBase<NGfx::CTexture>*,const CRectLayout&,const CTPoint<float>&,const CTRect<float>&)>(&C::CreateDynamicRects),bool())
{owner->CreateDynamicRects(static_cast<CPtrFuncBase<NGfx::CTexture>*>(nullptr),layout,point,clip);return true;}
template<class C> static bool NativeFractionalDraw(C* owner,const CRectLayout& layout,const CTPoint<float>& point,const CTRect<float>& clip,long)
{owner->CreateDynamicRects(static_cast<CPtrFuncBase<NGfx::CTexture>*>(nullptr),layout,CTPoint<int>(int(point.x),int(point.y)),CTRect<int>(int(clip.x1),int(clip.y1),int(clip.x2),int(clip.y2)));return false;}
template<class C> static auto NativeFractionalClear(C* owner,const CRectLayout& layout,const CTPoint<float>& point,const CTRect<float>& clip,int)
    -> decltype(static_cast<void(C::*)(const CRectLayout&,const CTPoint<float>&,const CTRect<float>&,float)>(&C::CreateDynamicClearRects),bool())
{owner->CreateDynamicClearRects(layout,point,clip,0.f);return true;}
template<class C> static bool NativeFractionalClear(C* owner,const CRectLayout& layout,const CTPoint<float>& point,const CTRect<float>& clip,long)
{owner->CreateDynamicClearRects(layout,CTPoint<int>(int(point.x),int(point.y)),CTRect<int>(int(clip.x1),int(clip.y1),int(clip.x2),int(clip.y2)),0.f);return false;}

static void NativeFractionalUIPixels(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<18,"unknown native fractional UI pixel mode");const int layer=mode/6,variant=mode%6;const bool clear=variant==5,empty=variant==4,control=variant==0 || empty;NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm fractional UI rendering regression");
    const auto size=NGfx::GetScreenRect();Require(size.x>=64 && size.y>=64,"fractional UI lacks actual native render surface");CRectLayout layout;const NGfx::SPixel8888 white(255,255,255,255),zero(0,0,0,0);CTPoint<float> point(0,0);CTRect<float> clip(0,0,64,64),expected(0,0,0,0);
    if(variant==0){point=CTPoint<float>(4,4);layout.AddRect(0,0,8,8,CTRect<float>(0,0,1,1),white);expected=CTRect<float>(4,4,12,12);}
    if(variant==1 || clear){point=CTPoint<float>(.75f,.75f);layout.AddRect(0,0,8,8,CTRect<float>(0,0,1,1),white);expected=CTRect<float>(.75f,.75f,8.75f,8.75f);}
    if(variant==2){clip=CTRect<float>(4.75f,4.75f,12.75f,12.75f);layout.AddRect(0,0,16,16,CTRect<float>(0,0,1,1),white);expected=clip;}
    if(variant==3){point=CTPoint<float>(.75f,.75f);layout.AddRect(12,12,-8,-8,CTRect<float>(0,0,1,1),white);expected=CTRect<float>(4.75f,4.75f,12.75f,12.75f);}
    if(empty){clip=CTRect<float>(32.75f,32.75f,40.75f,40.75f);layout.AddRect(0,0,8,8,CTRect<float>(0,0,1,1),white);}
    auto context=[](){NGfx::CRenderContext rc;rc.SetAlphaCombine(NGfx::COMBINE_SMART_ALPHA);rc.SetStencil(NGfx::STENCIL_NONE);return rc;};
    // The reference uses the existing actual float renderer, with simple fixed target rectangles.
    // It does not duplicate the clipping implementation or intercept a production draw call.
    {auto rc=context();rc.ClearBuffers(0xff315579);NGfx::C2DQuadsRenderer quads(rc,size,NGfx::QRM_OVERWRITE);if(!empty)quads.AddRect(expected,nullptr,CTRect<float>(0,0,1,1),clear?zero:white,0.f);quads.Flush();}
    CArray2D<NGfx::SPixel8888> reference;NGfx::MakeScreenShot(&reference,false);Require(reference.GetXSize()==int(size.x) && reference.GetYSize()==int(size.y),"fractional UI reference readback lost native surface dimensions");
    bool api=false;{auto rc=context();rc.ClearBuffers(0xff315579);if(layer==0){NGfx::C2DQuadsRenderer quads(rc,size,NGfx::QRM_OVERWRITE);api=NativeFractionalDirect(&quads,layout,point,clip,clear?NGScene::LRM_CLEAR_RECT:NGScene::LRM_NORMAL,0);quads.Flush();}else if(layer==1){CObj<NGScene::I2DScene> scene=NGScene::Make2DScene();Require(scene,"fractional UI lacks actual registered immediate scene");scene->StartNewFrame(nullptr,size);api=clear?NativeFractionalClear(scene.GetPtr(),layout,point,clip,0):NativeFractionalDraw(scene.GetPtr(),layout,point,clip,0);scene->Flush();}else{CObj<NGScene::I2DGameView> view=NGScene::CreateNew2DView();Require(view,"fractional UI lacks actual native game view");view->StartNewFrame();api=clear?NativeFractionalClear(view.GetPtr(),layout,point,clip,0):NativeFractionalDraw(view.GetPtr(),layout,point,clip,0);view->Flush();}}
    CArray2D<NGfx::SPixel8888> observed;NGfx::MakeScreenShot(&observed,false);Require(observed.GetXSize()==reference.GetXSize() && observed.GetYSize()==reference.GetYSize(),"fractional UI changed native dimensions between reference and production draw");int mismatch=0,referenceInk=0,observedInk=0;
    for(int y=0;y<48;++y)for(int x=0;x<48;++x){const auto& a=reference[y][x];const auto& b=observed[y][x];if(a.r!=b.r || a.g!=b.g || a.b!=b.b || a.a!=b.a)++mismatch;if(a.r==255 && a.g==255 && a.b==255)++referenceInk;if(b.r==255 && b.g==255 && b.b==255)++observedInk;}
    if(!empty && !clear)Require(referenceInk>0,"fractional UI direct float reference drew no real pixels");
    std::printf("FRACTIONAL-UI-PIXELS mode=%d layer=%d variant=%d api=%d mismatch=%d referenceInk=%d observedInk=%d control=%d clear=%d\n",mode,layer,variant,int(api),mismatch,referenceInk,observedInk,int(control),int(clear));std::fflush(stdout);
    Require(mismatch==0 && (control || api),"actual native UI pipeline lost fractional position/clip or its float interface");
}

// Native contextual actions: actual world/mission/mod/modal paths and read-only field observers.
namespace NWorld { class CCmdSwap; }
template<class T> static auto NativeConstructSwapCommand(NWorld::CUnit* target,int)
    -> decltype(sizeof(T), static_cast<NWorld::CCmd*>(static_cast<T*>(nullptr)))
{return new T(target);}
template<class T> static NWorld::CCmd* NativeConstructSwapCommand(NWorld::CUnit*,long){return nullptr;}

static void NativeContextSwap(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=6,"unknown contextual swap case");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* owner=nullptr;
    for(auto unit:units)if(unit && !unit->IsAIUnit() && unit->CanFight() && unit->GetTBSPlayer()){owner=unit;break;}
    Require(owner && !world->IsBase(),"swap fixture lacks its actual nonbase human owner");
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;
    network->GetNearPlaces(SSphere(owner->GetPosition().GetCP(),5.f),&places);
    NAI::SPathPlace from,to;bool found=false;
    for(auto a:places){a.SetPose(NAI::CM_STAND);a.SetMoving(0);if(!network->IsPassable(a) || network->IsLocked(a))continue;
        for(auto b:places){b.SetPose(NAI::CM_STAND);b.SetMoving(0);if(!network->IsPassable(b) || network->IsLocked(b))continue;
            const auto transition=NAI::GetTransitionType(network,a,b);
            const bool wanted=mode==4 ? fabs(NAI::SPosition(a,network).GetCP()-NAI::SPosition(b,network).GetCP())>3.f
                :transition==(mode==1 ? NAI::TT_MOVE_DIAGONAL : NAI::TT_MOVE);
            if(wanted){from=a;to=b;found=true;break;}}
        if(found)break;}
    Require(found,"swap fixture cannot find its genuine unlocked native transition cells");
    from.SetDirection(1);to.SetDirection(5);if(mode==2)from.SetPose(NAI::CM_LAY);if(mode==3)to.SetPose(NAI::CM_LAY);
    CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(from,NRPG::CreateUnit(NDb::GetPers(54)),owner->GetTBSPlayer(),"NativeSwapActor");
    CObj<NWorld::CUnitServer> target=world->AddUnitInGame(to,NRPG::CreateUnit(NDb::GetPers(103)),owner->GetTBSPlayer(),"NativeSwapTarget");
    Require(actor && target && actor->CanFight() && target->CanFight(),"swap fixture cannot deploy two real native units");
    // Real-time actions do not debit AP. Establish the actual owned turn and
    // native world callback context before observing cost or executor behavior.
    struct NativeSwapWorldContext { NWorld::CWorld* previous=NWorld::pCurrentWorld;~NativeSwapWorldContext(){NWorld::pCurrentWorld=previous;} } restoreWorld;
    NWorld::pCurrentWorld=world;world->GivePlayerTurn(actor->GetTBSPlayer());
    Require(world->IsTurnBased() && world->IsUnitActive(actor),"swap fixture did not enter its actual owned native turn");
    std::printf("CONTEXT-SWAP-SETUP mode=%d turnBased=%d active=%d\n",mode,int(world->IsTurnBased()),int(world->IsUnitActive(actor)));std::fflush(stdout);

    const auto a=actor->GetPosition(),b=target->GetPosition();const int actorAP=actor->GetAP(),targetAP=target->GetAP();
    const int actorHP=actor->GetUnitRPG()->GetTotalVP(),targetHP=target->GetUnitRPG()->GetTotalVP();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    CObj<NWorld::CCmd> command=NativeConstructSwapCommand<NWorld::CCmdSwap>(mode==5?nullptr:target.GetPtr(),0);
    std::printf("CONTEXT-SWAP mode=%d api=%d\n",mode,int(IsValid(command)));std::fflush(stdout);
    Require(command,"native contextual swap command foundation is missing");
    if(mode==6){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(command,&refs);CObj<NWorld::CCmd> restored=dynamic_cast<NWorld::CCmd*>(pSSClasses->CreateObject(0x01093130));Require(restored,"native Swap command registered save type is missing");fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ,&refs);restored->operator&(reader);}Require(restored!=command,"Swap command restoration reused its original object");command=restored;}
    NWorld::EUnitCommandResult error=NWorld::UCR_OK;CObj<NWorld::CCommandExecute> executor=NWorld::CreateActionExecutor(actor,command,&error);
    const auto expected=mode==5?NWorld::UCR_NO_TARGET:(mode>=2 && mode<=4)?NWorld::UCR_TARGET_OUT_OF_RANGE:NWorld::UCR_OK;
    Require(error==expected && bool(executor)==(expected==NWorld::UCR_OK),"actual swap dispatch lost native target/range/crawl eligibility");
    if(mode==6){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(executor,&refs);CObj<NWorld::CCommandExecute> restored=dynamic_cast<NWorld::CCommandExecute*>(pSSClasses->CreateObject(0x71983146));Require(restored,"native Swap executor registered save type is missing");fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ,&refs);restored->operator&(reader);}Require(restored!=executor && restored->GetUnitServer()==actor && restored->GetState()==executor->GetState(),"restored Swap executor lost actual actor or execution state");executor=restored;std::printf("CONTEXT-SWAP-SAVE commandClone=1 executorClone=1 actor=1\n");}
    if(executor){auto* cost=NDb::GetRPGAP(10);Require(cost && cost->nAP>0 && actorAP>=cost->nAP && executor->GetStartAP()==cost->nAP,"native swap did not use action41/RPGAP10");
        auto expectedActor=b,expectedTarget=a;expectedActor.pos.p.SetDirection(a.pos.p.GetDirection());expectedTarget.pos.p.SetDirection(b.pos.p.GetDirection());
        executor->Run();Require(executor->GetState()==NWorld::CCommandExecute::FINISHED && actor->GetPosition()==expectedActor && target->GetPosition()==expectedTarget,"actual swap failed to exchange positions while retaining individual facing");
        Require(actor->GetAP()==actorAP-cost->nAP && target->GetAP()==targetAP,"actual swap spent the wrong unit AP or cost");}
    else Require(actor->GetPosition()==a && target->GetPosition()==b && actor->GetAP()==actorAP && target->GetAP()==targetAP,"rejected swap changed real units");
    const bool healthSame=actor->GetUnitRPG()->GetTotalVP()==actorHP && target->GetUnitRPG()->GetTotalVP()==targetHP,timeSame=world->GetTime()->GetValue()==time,rngSame=random.Get()==GlobalGameRandom().Get();
    std::printf("CONTEXT-SWAP-EFFECT mode=%d healthSame=%d timeSame=%d rngSame=%d executed=%d\n",mode,int(healthSame),int(timeSame),int(rngSame),int(IsValid(executor)));std::fflush(stdout);
    // Real movement uses native SetPosition, whose step/visibility notifications
    // can roll hearing probability. Rejecting a command must have no such effects.
    Require(healthSame && timeSame && (executor || rngSame),"contextual swap changed health/time or a rejected query consumed gameplay RNG");
}

static void NativeContextEndTurn(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=4,"unknown contextual end-turn case");
    if(mode==0){CObj<NAI::CAILogic> logic=new NAI::CAILogic;const bool ended=logic->IsEndOfTurn();std::printf("CONTEXT-ENDTURN mode=0 worldless=1 ended=%d expected=1\n",int(ended));std::fflush(stdout);Require(ended,"worldless native AI logic did not end its unavailable turn");return;}
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units){auto* actual=dynamic_cast<NWorld::CUnitServer*>(unit.GetPtr());if(actual && actual->CanFight() && actual->GetAP()>0 && !actual->HasCommand() && world->IsUnitActive(actual) && NAI::GetAIUnit(actual)){actor=actual;break;}}
    Require(actor && !world->IsExecuting(),"AI end-turn fixture lacks a healthy active native idle actor");
    if(mode==4){struct RestoreWorld{NWorld::CWorld* previous=NWorld::pCurrentWorld;~RestoreWorld(){NWorld::pCurrentWorld=previous;}} restoreWorld;NWorld::pCurrentWorld=world;world->GivePlayerTurn(actor->GetTBSPlayer());Require(world->IsTurnBased() && world->IsUnitActive(actor),"busy end-turn fixture failed to enter the actual owned turn");}
    CObj<NAI::IAIUnit> brain=NAI::GetAIUnit(actor);CObj<NAI::CAILogic> logic=new NAI::CAILogic(brain);
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();const auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    CObj<NWorld::CActionCounter> action;if(mode==4){action=world->GetActiveCounter();Require(action && world->IsExecuting(),"busy end-turn fixture lacks its actual native action counter");}
    if(mode==1)logic->Pause();if(mode==3 || mode==4)logic->Finish();const bool ended=logic->IsEndOfTurn(),expected=mode==1 || mode==3;
    std::printf("CONTEXT-ENDTURN mode=%d active=%d ended=%d expected=%d ap=%d executing=%d\n",mode,int(logic->IsActive()),int(ended),int(expected),actor->GetAP(),int(world->IsExecuting()));std::fflush(stdout);
    Require(ended==expected,"actual native AI end-turn ignored paused/active/finished command eligibility");
    if(mode==1){logic->Resume();Require(!logic->IsEndOfTurn(),"resumed actual idle AI logic remained ended");}
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && world->GetTime()->GetValue()==time && random.Get()==GlobalGameRandom().Get(),"AI end-turn query changed actor/time/gameplay RNG");
}

template<class T> static auto NativeConstructContextClue(NRPG::CGlobalGame* game,NRPG::CGlobalPlayer* player,NScenario::CScenarioClue* clue,int)
    -> decltype(new T(nullptr,0,game,player,clue,nullptr))
{return new T(nullptr,0,game,player,clue,nullptr);}
template<class T> static T* NativeConstructContextClue(NRPG::CGlobalGame* game,NRPG::CGlobalPlayer*,NScenario::CScenarioClue* clue,long)
{return new T(nullptr,0,game,clue,nullptr);}

static void NativeContextClue(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=1,"unknown contextual clue case");Require(!NMainLoop::GetCurrentInterfaceForHarness(),"clue fixture starts with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm player clue paper regression");
    vector<NDb::CSide*> sides;CDBIterator<NDb::CSide> records(*NDatabase::GetTable<NDb::CSide>());
    while(records.MoveNext())if(IsValid(records.Get()->pCluePaperBackground))sides.push_back(records.Get());
    std::sort(sides.begin(),sides.end(),[](NDb::CSide* a,NDb::CSide* b){return a->GetRecordID()<b->GetRecordID();});
    Require(sides.size()>=2 && sides[0]->pCluePaperBackground!=sides[1]->pCluePaperBackground,"clue fixture lacks two different authored side papers");
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(1);CObj<NRPG::CGlobalPlayer> first=new NRPG::CGlobalPlayer,second=new NRPG::CGlobalPlayer;
    first->pSide=sides[0];second->pSide=sides[1];game->players.push_back(first);game->players.push_back(second);
    NDb::CDBScenarioClue* record=nullptr;CDBIterator<NDb::CDBScenarioClue> clues(*NDatabase::GetTable<NDb::CDBScenarioClue>());
    while(clues.MoveNext())if(!record || clues.Get()->GetRecordID()<record->GetRecordID())record=clues.Get();Require(record && game->pScenarioTracker,"clue fixture lacks actual authored clue/scenario");
    CObj<NScenario::CScenarioClue> clue=new NScenario::CScenarioClue(record,1);NRPG::CGlobalPlayer* selected=mode?second.GetPtr():first.GetPtr();
    CObj<NMainLoop::CInterfaceCommand> open=NativeConstructContextClue<NGame::CICShowClue>(game,selected,clue,0);open->Exec();
    CObj<NMainLoop::IInterfaceBase> modal=NMainLoop::GetCurrentInterfaceForHarness();Require(modal,"actual clue command did not open its native modal");
    CStructureNetworkContext refs;auto fields=BorrowedObjectFields(modal.GetPtr(),&refs);CObj<NUI::CWindow> root;CPtr<NRPG::CGlobalPlayer> interfacePlayer;
    {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(5,&interfacePlayer);reader.Add(9,&root);}
    Require(root,"clue modal lacks its actual saved UI root");auto* view=root->GetChildByID("view");Require(view,"clue modal lacks its actual view");
    CStructureNetworkContext viewRefs;auto viewFields=BorrowedObjectFields(view,&viewRefs);CPtr<NRPG::CGlobalPlayer> viewPlayer;CPtr<NUI::CImage> background;
    {CStructureSaver reader(viewFields,CStructureSaver::READ,&viewRefs);reader.Add(3,&viewPlayer);reader.Add(7,&background);}
    auto* paper=background?background->GetImage():nullptr;
    std::printf("CONTEXT-CLUE mode=%d player=%d interfacePlayer=%d viewPlayer=%d background=%d paper=%d expected=%d\n",mode,selected->pSide->GetRecordID(),int(interfacePlayer.GetPtr()==selected),int(viewPlayer.GetPtr()==selected),int(IsValid(background)),paper?paper->GetRecordID():-1,selected->pSide->pCluePaperBackground->GetRecordID());std::fflush(stdout);
    Require(interfacePlayer.GetPtr()==selected && viewPlayer.GetPtr()==selected && background && paper==selected->pSide->pCluePaperBackground.GetPtr(),"actual clue modal lost player context, clue template background or authored side paper");
    NMainLoop::DoneInterface();
}

static void NativeWriteContextAnimationOverlay(const std::filesystem::path& destination,
    const S2FileIO::GameDatabaseTable& source,std::size_t row,int id,int skeleton)
{
    vector<vector<int>> ints(1,source.intRows[row]);vector<vector<float>> floats(1,source.floatRows[row]);
    vector<vector<wstring>> strings(1,source.stringRows[row]);vector<NativeContextColumn> columns;
    for(const auto& q:source.columns){NativeContextColumn column;column.name=q.name;column.type=q.type;columns.push_back(column);}
    auto intNames=source.intNames,floatNames=source.floatNames,stringNames=source.stringNames;
    const auto idColumn=std::find(intNames.begin(),intNames.end(),"ID"),skeletonColumn=std::find(intNames.begin(),intNames.end(),"SkeletonID");
    Require(idColumn!=intNames.end() && skeletonColumn!=intNames.end(),"native overlay lacks animation ID/SkeletonID");
    ints[0][idColumn-intNames.begin()]=id;ints[0][skeletonColumn-intNames.begin()]=skeleton;
    CObj<CObjectBase> storage=pSSClasses->CreateObject(int(0xa1843130u));Require(storage,"actual registered columnar storage is missing");
    CMemoryStream fields;{CStructureSaver writer(fields,CStructureSaver::WRITE);writer.Add(2,&ints);writer.Add(3,&floats);writer.Add(4,&strings);writer.Add(5,&columns);writer.Add(6,&intNames);writer.Add(7,&floatNames);writer.Add(8,&stringNames);}
    fields.Seek(0);{CStructureSaver reader(fields,CStructureSaver::READ);storage->operator&(reader);}
    unordered_map<int,CObj<CObjectBase>> tables;tables[2]=storage;list<NativeContextEmptyRelation> relations;
    CMemoryStream database;{CStructureSaver writer(database,CStructureSaver::WRITE);writer.Add(1,&tables);writer.Add(2,&relations);}
    S2FileIO::PortableGameDatabase decoded;std::string error;Require(S2FileIO::LoadPortableGameDatabaseBytes(static_cast<const std::uint8_t*>(database.GetBuffer()),database.GetSize(),&decoded,&error) && decoded.tables.size()==1 && decoded.tables[0].intRows.size()==1,"native small animation overlay is not a valid release columnar database");
    std::ofstream output(destination,std::ios::binary);Require(bool(output),"cannot create private native animation mod input");output.write(reinterpret_cast<const char*>(database.GetBuffer()),database.GetSize());Require(bool(output),"cannot write private native animation mod input");
}

static void NativeContextModLinks(const char* databasePath,int mode)
{
    Require(mode>=0 && mode<=2,"unknown contextual mod links case");
    const auto base=std::filesystem::absolute(databasePath);const auto oldDirectory=std::filesystem::current_path();
    struct Restore{std::filesystem::path directory;~Restore(){std::error_code error;std::filesystem::current_path(directory,error);}} restore{oldDirectory};
    std::filesystem::current_path(base.parent_path());{CFileStream input;input.OpenRead(base.string().c_str());NDatabase::Serialize(input,CStructureSaver::READ);}
    Require(CModManager::Activate(vector<SModInfo>{}),"native mod fixture cannot activate its actual base database");
    S2FileIO::PortableGameDatabase database;std::string error;Require(S2FileIO::LoadPortableGameDatabase(base.string(),&database,&error),"mod fixture cannot decode authoritative animation input");
    const S2FileIO::GameDatabaseTable* source=nullptr;for(const auto& q:database.tables)if(q.tableId==2)source=&q;
    Require(source && source->intRows.size()==source->floatRows.size() && source->intRows.size()==source->stringRows.size(),"native mod fixture lacks aligned real animation rows");
    const auto idColumn=std::find(source->intNames.begin(),source->intNames.end(),"ID");Require(idColumn!=source->intNames.end(),"native animation table lacks IDs");const auto idIndex=idColumn-source->intNames.begin();
    std::map<int,std::pair<int,int>> expectedRecords;CDBIterator<NDb::CAnimation> animation(*NDatabase::GetTable<NDb::CAnimation>());
    while(animation.MoveNext()){auto* a=animation.Get();if(IsValid(a->pSkeleton))expectedRecords[a->GetRecordID()]={a->pSkeleton->GetRecordID(),int(a->nType)};}
    Require(!expectedRecords.empty(),"native mod fixture has no real skeleton animation groups");const int target=expectedRecords.begin()->first,oldSkeleton=expectedRecords.begin()->second.first;
    vector<int> skeletons;CDBIterator<NDb::CSkeleton> skeleton(*NDatabase::GetTable<NDb::CSkeleton>());while(skeleton.MoveNext())if(skeleton.Get()->GetRecordID()!=oldSkeleton)skeletons.push_back(skeleton.Get()->GetRecordID());std::sort(skeletons.begin(),skeletons.end());Require(skeletons.size()>=2,"native mod fixture lacks distinct real skeletons");
    std::size_t row=source->intRows.size();std::vector<std::int32_t> insertion;int maxID=0;
    for(std::size_t i=0;i<source->intRows.size();++i){const int id=source->intRows[i][idIndex];insertion.push_back(id);maxID=Max(maxID,id);if(id==target)row=i;}
    Require(row<source->intRows.size(),"native linked animation has no authoritative source row");
    const char* userData=std::getenv("S2_USER_DATA_DIR");Require(userData && *userData,"mod fixture lacks its isolated user directory");
    const auto root=std::filesystem::absolute(userData)/("context-mod-links-"+std::to_string(mode));std::filesystem::create_directories(root);
    vector<SModInfo> mods;const int editedID=mode==2?maxID+1:target;const int count=mode==1?2:1;
    for(int i=0;i<count;++i){const auto path=root/("layer-"+std::to_string(i));std::filesystem::create_directories(path);{std::ofstream description(path/"description.txt",std::ios::binary);description<<"Native animation link layer";}
        NativeWriteContextAnimationOverlay(path/"game.db",*source,row,editedID,skeletons[i]);SModInfo info;info.szName="Native animation link layer";info.szDirectory=path.string();mods.push_back(info);}
    if(mode==2){expectedRecords[editedID]={skeletons[0],expectedRecords[target].second};insertion.push_back(editedID);}else expectedRecords[target].first=skeletons[count-1];
    std::map<std::pair<int,int>,std::vector<int>> expected;
    for(auto id:S2FileIO::OrderGameDatabaseRecordIdsLikeStlport(insertion)){auto record=expectedRecords.find(id);if(record!=expectedRecords.end())expected[record->second].push_back(id);}
    Require(CModManager::Activate(mods),"actual mod manager rejected valid native animation overlays");
    std::map<std::pair<int,int>,std::vector<int>> actual;CDBIterator<NDb::CSkeleton> after(*NDatabase::GetTable<NDb::CSkeleton>());
    while(after.MoveNext())for(const auto& group:after.Get()->pAnimations)for(auto a:group.second.anims){Require(a,"native skeleton has an expired animation link");actual[{after.Get()->GetRecordID(),group.first}].push_back(a->GetRecordID());}
    auto* edited=NDb::GetDBAnimation(editedID);Require(edited && edited->pSkeleton && edited->pSkeleton->GetRecordID()==skeletons[count-1],"actual native animation overlay did not update its forward skeleton reference");
    std::printf("CONTEXT-MOD-LINKS mode=%d layers=%d animation=%d oldSkeleton=%d newSkeleton=%d groups=%zu expectedGroups=%zu orderedEqual=%d\n",mode,count,editedID,oldSkeleton,skeletons[count-1],actual.size(),expected.size(),int(actual==expected));std::fflush(stdout);
    Require(actual==expected,"native mod activation left stale/duplicate/missing reverse animation links or lost release insertion order");
    Require(CModManager::Activate(vector<SModInfo>{}),"native mod fixture cannot restore the actual base database");
}

static void NativeContextTrap(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=3,"unknown contextual trap case");Require(!NMainLoop::GetCurrentInterfaceForHarness(),"trap fixture starts with an interface");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm contextual trap regression");
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(1);game->players.push_back(NRPG::CreateGlobalPlayer(std::vector<int>{54}));
    CObj<NGame::CICBeginMission> open=new NGame::CICBeginMission(-1,810,vector<string>(),game);open->Exec();
    CObj<NGame::CMission> mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());Require(mission && mission->GetActivePlayer(),"trap fixture did not initialize its actual native human mission");
    while(NMainLoop::HaveInterfaceCommand()){CObj<NMainLoop::CInterfaceCommand> queued=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(queued.GetPtr()),"trap setup posted an unexpected native interface command");}
    mission->PauseGame(true);auto* player=mission->GetActivePlayer();player->Update(true);vector<CPtr<NGame::IUnitTracker>> trackers;player->GetUnits(&trackers);NWorld::CUnitServer* actor=nullptr;
    for(auto tracker:trackers){auto* unit=dynamic_cast<NWorld::CUnitServer*>(tracker->GetUnit());if(unit && unit->CanFight() && !unit->IsWearingPK() && !unit->IsEmptyPK()){actor=unit;break;}}
    std::printf("CONTEXT-TRAP-SETUP mode=%d trackers=%zu actor=%d sequence=%d playerTurn=%d realtime=%d\n",mode,trackers.size(),int(actor!=nullptr),int(mission->IsSequence()),int(mission->IsPlayerTurn()),int(mission->IsRealTime()));std::fflush(stdout);
    Require(actor && !mission->IsSequence() && mission->IsPlayerTurn(),"trap fixture lacks its real human controlled tactical actor");
    NDb::CRPGGrenade* record=nullptr;CDBIterator<NDb::CRPGGrenade> grenades(*NDatabase::GetTable<NDb::CRPGGrenade>());
    while(grenades.MoveNext()){auto* grenade=grenades.Get();if(grenade->pItem && (!record || grenade->GetRecordID()<record->GetRecordID()))record=grenade;}
    Require(record,"trap fixture lacks an authored ordinary grenade");CObj<NRPG::CGrenadeItem> grenade=new NRPG::CGrenadeItem(record);grenade->nQuantity=1;grenade->SetMode(mode==0 || mode==3?NRPG::GM_THROW:NRPG::GM_SETTRAP);
    auto* inventory=actor->GetUnitRPG()->GetInventory();CObj<NRPG::IInventoryItem> previous=inventory->Get(NDb::SLOT_1)?inventory->TakeOff(NDb::SLOT_1):nullptr;
    Require(inventory->Equip(NDb::SLOT_1,grenade) && inventory->Activate(NDb::SLOT_1),"trap fixture cannot equip/activate its actual grenade");
    actor->SetWalkWithoutWeapon(false);actor->SetUndrawItem(false);actor->animator.SetActiveItem(true);actor->animator.SetWeaponAnimation(actor->GetUnitRPG()->GetWeaponType());
    mission->Select(actor,false);mission->RefreshPointerTarget();vector<CPtr<NGame::IUnitTracker>> selected;mission->GetSelectedUnits(&selected);Require(selected.size()==1 && selected.front()->GetUnit()==actor && mission->IsReady(),"trap fixture did not select its actual ready native actor");
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();const auto position=actor->GetPosition();auto random=GlobalGameRandom();
    NGame::SActionInfo expected;mission->CanDoCommand(new NWorld::CCmdSetGrenadeOnObject(nullptr),true,&expected);
    const bool trapMode=mode==1 || mode==2;Require(expected.bAvailable==trapMode,"native grenade command did not expose the intended throw/trap setup");
    if(mode<=1){NGame::SActionInfo actual;mission->GetActionInfo(static_cast<NGame::EUnitAction>(14),&actual);
        std::printf("CONTEXT-TRAP mode=%d grenadeMode=%d available=%d expectedAvailable=%d AP=%d,%d expectedAP=%d,%d result=%d expectedResult=%d\n",mode,int(grenade->GetMode()),int(actual.bAvailable),int(expected.bAvailable),actual.nMinAP,actual.nMaxAP,expected.nMinAP,expected.nMaxAP,int(actual.eResult),int(expected.eResult));std::fflush(stdout);
        Require(actual.bAvailable==expected.bAvailable && actual.nMinAP==expected.nMinAP && actual.nMaxAP==expected.nMaxAP && actual.eResult==expected.eResult,"native action14 cache did not represent the real object-trap command");
        NGame::SActionInfo attack;mission->GetActionInfo(NGame::UA_ATTACK,&attack);Require(attack.bAvailable==!trapMode,"ordinary attack still advertises object-trap mode or lost ordinary grenade throwing");}
    else if(mode==2){auto event=SharedActionEvent({"setmine"});mission->ProcessEvent(event);const bool isTrap=dynamic_cast<NGame::CStateSetTrap*>(mission->GetState())!=nullptr;
        std::printf("CONTEXT-TRAP mode=2 sharedSetMineTrap=%d expected=1\n",int(isTrap));std::fflush(stdout);Require(isTrap,"actual shared setmine input did not enter the available native object-trap state");}
    else {CObj<NGame::CStateSetTrap> state=new NGame::CStateSetTrap;const bool accepted=state->Initialize(mission);std::printf("CONTEXT-TRAP mode=3 throwModeAccepted=%d expected=0\n",int(accepted));std::fflush(stdout);if(accepted)state->Terminate();Require(!accepted,"forced object-trap state used ordinary attack availability for a throw-mode grenade");}
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && actor->GetPosition()==position && random.Get()==GlobalGameRandom().Get(),"contextual trap queries/input changed physical actor or gameplay RNG");
    NMainLoop::DoneInterface();
}

static void NativeInactiveSwap(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=7,"unknown inactive Swap mode");
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);NWorld::CUnitServer* owner=nullptr;
    for(auto unit:units)if(unit && !unit->IsAIUnit() && unit->CanFight() && unit->GetTBSPlayer()){owner=unit;break;}
    Require(owner && !world->IsBase(),"swap fixture lacks its actual nonbase human owner");
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;
    network->GetNearPlaces(SSphere(owner->GetPosition().GetCP(),5.f),&places);
    NAI::SPathPlace from,to;bool found=false;
    for(auto a:places){a.SetPose(NAI::CM_STAND);a.SetMoving(0);if(!network->IsPassable(a) || network->IsLocked(a))continue;
        for(auto b:places){b.SetPose(NAI::CM_STAND);b.SetMoving(0);if(!network->IsPassable(b) || network->IsLocked(b))continue;
            const auto transition=NAI::GetTransitionType(network,a,b);
            const bool wanted=transition==NAI::TT_MOVE;
            if(wanted){from=a;to=b;found=true;break;}}
        if(found)break;}
    Require(found,"swap fixture cannot find its genuine unlocked native transition cells");
    from.SetDirection(1);to.SetDirection(5);
    CObj<NWorld::CUnitServer> actor=world->AddUnitInGame(from,NRPG::CreateUnit(NDb::GetPers(54)),owner->GetTBSPlayer(),"NativeSwapActor");
    CObj<NWorld::CUnitServer> target=world->AddUnitInGame(to,NRPG::CreateUnit(NDb::GetPers(103)),owner->GetTBSPlayer(),"NativeSwapTarget");
    Require(actor && target && actor->CanFight() && target->CanFight(),"swap fixture cannot deploy two real native units");
    // Real-time actions do not debit AP. Establish the actual owned turn and
    // native world callback context before observing cost or executor behavior.
    struct NativeSwapWorldContext { NWorld::CWorld* previous=NWorld::pCurrentWorld;~NativeSwapWorldContext(){NWorld::pCurrentWorld=previous;} } restoreWorld;
    NWorld::pCurrentWorld=world;world->GivePlayerTurn(actor->GetTBSPlayer());
    Require(world->IsTurnBased() && world->IsUnitActive(actor),"swap fixture did not enter its actual owned native turn");
    std::printf("CONTEXT-SWAP-SETUP mode=%d turnBased=%d active=%d\n",mode,int(world->IsTurnBased()),int(world->IsUnitActive(actor)));std::fflush(stdout);


    auto makeInactive=[](NWorld::CUnitServer* unit){auto position=unit->GetPosition();position.pos.p.SetPose(NAI::CM_INACTIVE);unit->SetPosition(position);Require(unit->GetPosition().pos.p.GetPose()==NAI::CM_INACTIVE,"inactive Swap setup did not install the real raw transition pose");};
    if(mode==0 || mode==5)makeInactive(actor);if(mode==1)makeInactive(target);
    CObj<NWorld::CExecSwap> direct=new NWorld::CExecSwap(actor,mode==6?nullptr:target.GetPtr());
    int initialAP=actor->GetAP(),targetAP=target->GetAP(),actorHP=actor->GetUnitRPG()->GetTotalVP(),targetHP=target->GetUnitRPG()->GetTotalVP();auto time=world->GetTime()->GetValue();auto random=GlobalGameRandom();
    if(mode<=1 || mode==5 || mode==6){const auto before=actor->GetPosition(),targetBefore=target->GetPosition();const auto result=direct->CanDoIt(before,mode==5);const auto expected=mode==6?NWorld::UCR_NO_TARGET:NWorld::UCR_UNAVAILABLE;
        std::printf("INACTIVE-SWAP mode=%d admission=%d expected=%d actorRaw=%d targetRaw=%d ignore=%d\n",mode,int(result),int(expected),int(before.pos.p.GetPose()),int(targetBefore.pos.p.GetPose()),int(mode==5));std::fflush(stdout);
        Require(result==expected,"actual Swap admits inactive transition pose or loses missing-target result");Require(actor->GetPosition()==before && target->GetPosition()==targetBefore && actor->GetAP()==initialAP && target->GetAP()==targetAP,"rejected Swap admission changed native units");}
    else {NWorld::EUnitCommandResult error=NWorld::UCR_OK;CObj<NWorld::CCommandExecute> executor=NWorld::CreateActionExecutor(actor,new NWorld::CCmdSwap(target),&error);Require(executor && error==NWorld::UCR_OK,"late Swap fixture lacks actual accepted native executor");
        if(mode==2)makeInactive(target);if(mode==3)makeInactive(actor);
        if(mode==4){bool distant=false;for(auto place:places){place.SetPose(NAI::CM_STAND);place.SetMoving(0);if(network->IsPassable(place) && !network->IsLocked(place) && fabs(NAI::SPosition(place,network).GetCP()-actor->GetPosition().GetCP())>3.f){auto position=target->GetPosition();position.pos=NAI::SPosition(place,network);target->SetPosition(position);distant=true;break;}}Require(distant,"late Swap lacks genuine unlocked distant target cell");}
        // Late pose/position changes are deliberate native setup, including hearing notifications.
        initialAP=actor->GetAP();targetAP=target->GetAP();actorHP=actor->GetUnitRPG()->GetTotalVP();targetHP=target->GetUnitRPG()->GetTotalVP();time=world->GetTime()->GetValue();random=GlobalGameRandom();
        const auto before=actor->GetPosition(),targetBefore=target->GetPosition();executor->Run();const bool rejected=mode!=7;
        std::printf("INACTIVE-SWAP mode=%d state=%d expected=%d actorAP=%d beforeAP=%d targetAP=%d\n",mode,int(executor->GetState()),int(rejected?NWorld::CCommandExecute::FAILED:NWorld::CCommandExecute::FINISHED),actor->GetAP(),initialAP,target->GetAP());std::fflush(stdout);
        if(rejected)Require(executor->GetState()==NWorld::CCommandExecute::FAILED && actor->GetPosition()==before && target->GetPosition()==targetBefore && actor->GetAP()==initialAP && target->GetAP()==targetAP,"late-invalid Swap moved units or spent AP before live validation");
        else {auto expectedActor=targetBefore,expectedTarget=before;expectedActor.pos.p.SetDirection(before.pos.p.GetDirection());expectedTarget.pos.p.SetDirection(targetBefore.pos.p.GetDirection());auto* cost=NDb::GetRPGAP(10);Require(cost && executor->GetState()==NWorld::CCommandExecute::FINISHED && actor->GetPosition()==expectedActor && target->GetPosition()==expectedTarget && actor->GetAP()==initialAP-cost->nAP && target->GetAP()==targetAP,"standing native Swap control lost execution/cost/facing");}}
    Require(actor->GetUnitRPG()->GetTotalVP()==actorHP && target->GetUnitRPG()->GetTotalVP()==targetHP && world->GetTime()->GetValue()==time && (mode==7 || random.Get()==GlobalGameRandom().Get()),"inactive Swap query/run changed health/time or rejected-command gameplay RNG");
}


struct SharedGlobalProbe {
    int calls=0; NGlobal::CCmd* remove=nullptr;
    static void Command(const string&,const vector<wstring>&,void* context) { auto* self=static_cast<SharedGlobalProbe*>(context);++self->calls;if(self->remove){auto* old=self->remove;self->remove=nullptr;delete old;} }
    static void Variable(const string&,const NGlobal::CValue&,void* context) {++static_cast<SharedGlobalProbe*>(context)->calls;}
};
static void NativeSharedGlobals(int mode)
{
    const string key="upstream_shared_registration"; SharedGlobalProbe first,second;
    if(mode<=2){
        NGlobal::CCmd a(key,SharedGlobalProbe::Command,&first);
        auto* b=new NGlobal::CCmd(key,SharedGlobalProbe::Command,&second);
        if(mode==0){NGlobal::ProcessCommand(L"upstream_shared_registration argument");delete b;Require(first.calls==1 && second.calls==1,"command registration overwrote an existing subscriber");}
        if(mode==1){delete b;NGlobal::ProcessCommand(L"upstream_shared_registration");Require(first.calls==1 && second.calls==0,"destroying one command owner removed another owner");}
        if(mode==2){first.remove=b;NGlobal::ProcessCommand(L"upstream_shared_registration");if(first.remove){delete first.remove;first.remove=nullptr;}Require(first.calls==1 && second.calls==0,"dispatch invoked a removed subscriber or skipped the first owner");}
    } else if(mode<=4) {
        NGlobal::CVar a(key,SharedGlobalProbe::Variable,&first,NGlobal::CValue(7.f),true);
        auto* b=new NGlobal::CVar(key,SharedGlobalProbe::Variable,&second,NGlobal::CValue(99.f),false);
        if(mode==3){NGlobal::SetVar(key,NGlobal::CValue(13.f));delete b;Require(first.calls==1 && second.calls==1 && a.Get().GetInt()==13,"variable update did not broadcast to both registrations");}
        if(mode==4){delete b;NGlobal::SetVar(key,NGlobal::CValue(13.f));NGlobal::ResetVar(key);Require(first.calls==2 && second.calls==0 && a.Get().GetInt()==7,"variable destruction lost another observer or the original default");}
    } else if(mode==5) {
        NGlobal::SetVar(key,NGlobal::CValue(23.f));
        {NGlobal::CVar a(key,SharedGlobalProbe::Variable,&first,NGlobal::CValue(7.f),true);Require(a.Get().GetInt()==23,"registration overwrote a preloaded configuration value");a.Set(NGlobal::CValue(17.f));}
        NGlobal::CVar b(key,SharedGlobalProbe::Variable,&second,NGlobal::CValue(99.f),false);Require(b.Get().GetInt()==17,"reopening a variable reset its persistent value");
    } else if(mode==6) {
        NGlobal::CVar var(key,SharedGlobalProbe::Variable,&first,NGlobal::CValue(7.f));
        {NGlobal::CCmd command(key,SharedGlobalProbe::Command,&second);NGlobal::ProcessCommand(L"upstream_shared_registration");}
        NGlobal::SetVar(key,NGlobal::CValue(9.f));Require(first.calls==1 && second.calls==1 && var.Get().GetInt()==9,"command and variable registrations interfered");
    } else throw std::runtime_error("unknown shared registration case");
    std::printf("SHARED-GLOBALS mode=%d first=%d second=%d passed\n",mode,first.calls,second.calls);
}
template<class T> static auto NativeDropClipList(T* item,vector<CPtr<NRPG::IInventoryItem>>* out,int)->decltype(item->GetInventoryItemWithClips(out),bool()) {item->GetInventoryItemWithClips(out);return true;}
template<class T> static bool NativeDropClipList(T*,vector<CPtr<NRPG::IInventoryItem>>*,long){return false;}
static void NativeLootRecords(const char* databasePath,const char* resourcePath,int mode)
{
    auto world=FixtureWorld(databasePath,resourcePath,true);
    struct Restore {NWorld::CWorld* previous=NWorld::pCurrentWorld;~Restore(){NWorld::pCurrentWorld=previous;}} restore;
    NWorld::pCurrentWorld=world;
    list<CPtr<NWorld::CUnitServer>> actors;world->GetAllUnits(&actors);
    NWorld::CUnitServer* actor=nullptr;NAI::IAIUnit* brain=nullptr;
    for(auto candidate:actors)if(candidate && candidate->CanFight() && !candidate->IsWearingPK() && !candidate->IsEmptyPK()) {
        auto* ai=NAI::GetAIUnit(candidate);if(ai){ai->Synchronize(false);if(ai->GetAIInventory()->GetFirstFireArms()){actor=candidate;brain=ai;break;}}
    }
    Require(actor && brain,"native loot fixture has no real armed AI actor");
    auto* mirror=brain->GetAIInventory();CObj<NAI::CAIFireArmsWeapon> gun=mirror->GetFirstFireArms();auto* item=gun->GetItem();
    if(mode==0){CObj<NAI::CAILogDropItem> record=new NAI::CAILogDropItem(brain,item);record->Commit();Require(mirror->GetFirstFireArms()!=gun.GetPtr(),"AI drop record did not update its simulated inventory");return;}
    if(mode==1 || mode==2){
        CObj<NRPG::IInventoryItem> picked=NRPG::CreateWeaponItem(item->GetDBWeapon());
        auto* body=world->AddFrozenItem(world->GetAIMap(),actor->GetPosition().GetCP()+CVec3(.25f,0,.25f),QNULL,picked);
        Require(body && body->GetInvItem()==picked.GetPtr(),"native loot fixture could not create an actual ground weapon");
        CObj<NAI::CAILogPickUpItem> record=new NAI::CAILogPickUpItem(brain,body,NAI::RUN);
        if(mode==1){while(auto* held=mirror->GetFirstFireArms())mirror->RemoveItem(held->GetInventoryItem());record->Commit();Require(mirror->GetFirstFireArms() && mirror->GetFirstFireArms()->GetItem()==picked.GetPtr(),"AI pickup record did not update its simulated inventory");}
        else {list<CPtr<NWorld::CCommand>> commands;record->GetCommands(&commands);Require(commands.size()==3,"AI pickup omitted pose or backpack arrangement");auto* set=dynamic_cast<NWorld::CCmdSetCommand*>(commands.back().GetPtr());auto* move=set?dynamic_cast<NWorld::CCmdMoveInventoryItem*>(set->GetCmd()):nullptr;Require(move && move->GetSource().pWorldItem==body && move->GetSource().pItem==picked.GetPtr() && !move->GetSource().pUnit && move->GetTarget().sPosition==CTPoint<int>(-1,-1),"AI pickup lost actual ground identity or selected backpack cells prematurely");}
        return;
    }
    if(mode==3){CObj<NAI::CAILogDropItem> record=new NAI::CAILogDropItem(brain,item);list<CPtr<NWorld::CCommand>> commands;record->GetCommands(&commands);Require(commands.size()>=2,"AI drop did not produce its actual inventory commands");auto it=commands.begin();auto* a=dynamic_cast<NWorld::CCmdSetCommand*>((it++)->GetPtr());auto* b=dynamic_cast<NWorld::CCmdSetCommand*>(it->GetPtr());auto* first=a?dynamic_cast<NWorld::CCmdMoveInventoryItem*>(a->GetCmd()):nullptr;auto* second=b?dynamic_cast<NWorld::CCmdMoveInventoryItem*>(b->GetCmd()):nullptr;Require(first && second && first->GetSource().pItem==item && first->GetTarget().eType==NWorld::SItem::HAND && second->GetSource().eType==NWorld::SItem::HAND && second->GetTarget().eType==NWorld::SItem::GROUND,"AI drop did not route through the hand to the ground");return;}
    if(mode==4){CObj<NRPG::IInventoryItem> spareItem=NRPG::CreateClipItem(item->GetDBWeapon()->pInnerClip);CPtr<NRPG::CClipItem> spare=CDynamicCast<NRPG::CClipItem>(spareItem.GetPtr()).GetPtr();CObj<NAI::CAIFireArmsWeaponClip> wrapper=NAI::CreateAIFireArmsWeaponClip(spare);gun->AddClip(wrapper);vector<CPtr<NRPG::IInventoryItem>> dropped;bool api=NativeDropClipList(static_cast<NAI::IAIInventoryItem*>(gun.GetPtr()),&dropped,0);std::printf("LOOT-CLIPS api=%d count=%d\n",int(api),int(dropped.size()));std::fflush(stdout);Require(api,"weapon plus spare-clip enumeration API is missing");Require(!dropped.empty() && dropped.front()==item && find(dropped.begin(),dropped.end(),spare.GetPtr())!=dropped.end(),"drop list lost its actual gun or spare clip");auto* loaded=gun->GetCurrentClip();if(loaded)Require(find(dropped.begin(),dropped.end(),loaded->GetItem())==dropped.end(),"loaded magazine was incorrectly separated from its gun");return;}
    if(mode==5){NAI::SPosition stop,flank,attack;auto enemy=actor->GetPosition();enemy.pos.p.SetDirection((enemy.pos.p.GetDirection()+1)&7);bool accepted=NAI::GetRoundUpPlaces(actor,enemy,&stop,&flank,&attack,false);Require(accepted && stop.p==actor->GetPosition().pos.p && flank.p==stop.p && attack.p==enemy.pos.p,"short-range approach did not return the direct native positions");return;}
    if(mode==6 || mode==7){auto position=actor->GetPosition();CObj<NWorld::CUnitServer> ally;for(auto other:actors)if(other && other.GetPtr()!=actor && other->CanFight() && !other->IsEmptyPK() && !other->IsWearingPK() && actor->GetDiplomacyState(other)==NDb::DS_ALLY){ally=other;break;}Require(ally,"native flank fixture lacks an existing allied actor");ally->SetPosition(position);Require(ally && ally->CanFight() && actor->GetDiplomacyState(ally)==NDb::DS_ALLY,"native flank fixture lacks a real ally");if(mode==7){ally->GetUnitRPG()->Kill();ally->Die(false,false);}Require(ally->CanFight()==(mode==6),"native flank fixture did not establish actual ally combat eligibility");std::printf("FLANK-SETUP mode=%d owner=%p allyOwner=%p alive=%d distance=%f\n",mode,(void*)actor->GetTBSPlayer(),(void*)ally->GetTBSPlayer(),int(ally->CanFight()),fabs(ally->GetPosition().GetCP()-position.GetCP()));std::fflush(stdout);NAI::SPosition stop,flank,attack;bool accepted=NAI::GetRoundUpPlaces(actor,position,&stop,&flank,&attack,true);Require(accepted==(mode==6),"direct approach inverted its downed-ally avoidance");return;}
    throw std::runtime_error("unknown native loot records case");
}

static void NativeLootHotseat(const char* databasePath,const char* resourcePath,int mode)
{
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm hotseat turn and selection");NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(-1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{103}));
    CObj<NGame::CICBeginMission> open=new NGame::CICBeginMission(810,-1,vector<string>(),game);open->Exec();
    CObj<NGame::CMission> mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());Require(mission && mission->GetWorld(),"native hotseat did not open an actual two-player mission");
    while(NMainLoop::HaveInterfaceCommand()){CObj<NMainLoop::CInterfaceCommand> queued=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(queued.GetPtr()),"hotseat startup posted an unexpected transition");}
    mission->PauseGame(true);vector<CPtr<NGame::IPlayerTracker>> players;mission->GetPlayers(&players);Require(players.size()==2,"native hotseat lost a player");
    for(auto player:players)player->Update(false);
    auto* world=dynamic_cast<NWorld::CWorld*>(mission->GetWorld());Require(world,"hotseat needs actual native world");std::printf("HOTSEAT-SETUP sequence=%d realtime=%d active=%p owner=%p current=%p\n",int(world->IsSequence()),int(mission->IsRealTime()),(void*)mission->GetActivePlayer()->GetPlayer(),(void*)players.front()->GetPlayer(),(void*)world->GetCurrentPlayer());std::fflush(stdout);world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(mission->GetActivePlayer()->GetPlayer()));MarkNewDGFrame();Require(NMainLoop::StepApp(true,false,false),"native hotseat frame did not execute");
    Require(mission->IsPlayerTurn() && !mission->IsRealTime(),"native hotseat did not establish the active player turn");
    if(mode==0){for(auto player:players)if(player!=mission->GetActivePlayer()){vector<CPtr<NGame::IUnitTracker>> units;player->GetUnits(&units);Require(!units.empty(),"inactive hotseat player has no actual unit tracker");for(auto unit:units)Require(!unit->IsActive() && !unit->IsSelected(),"inactive hotseat tracker retained selection or became selectable");}}
    else {Require(!mission->CanDoCommand(new NWorld::CCmdEndOfTurn),"native hotseat setup did not enter its real turn-change cooldown");auto* before=world->GetTBSCurrentPlayer();auto* commander=mission->GetActivePlayer()->GetCommander();Require(!commander->HasCommands(),"native cooldown fixture starts with pending commands");auto event=SharedActionEvent({"endofturn"});Require(mission->ProcessEvent(event),"native end-of-turn input was not handled");Require(world->GetTBSCurrentPlayer()==before && !commander->HasCommands(),"keyboard end turn bypassed the native turn-change cooldown");}
}


namespace NWorld {struct STBSEvent;}
template<class T> static auto NativeDrainTurnEvents(T* world,int)->decltype(world->ProcessTBSEvents(),bool()){world->ProcessTBSEvents();return true;}
template<class T> static bool NativeDrainTurnEvents(T*,long){return false;}
class NativeDeferredTurnObserver:public CObjectBase
{
    OBJECT_NOCOPY_METHODS(NativeDeferredTurnObserver);
    NGlobal::CEventRegister<NativeDeferredTurnObserver,NWorld::CEventOnPassControl> registration;
public:
    NWorld::CWorld* world;NWorld::CPlayer* reenter=nullptr;bool once=false;int depth=0,maxDepth=0;
    vector<CPtr<NWorld::CPlayer>> owners;
    explicit NativeDeferredTurnObserver(NWorld::CWorld* value=nullptr):registration(this,&NativeDeferredTurnObserver::OnPass),world(value){}
    void OnPass(const NWorld::CEventOnPassControl& event){if(NWorld::pCurrentWorld!=world)return;++depth;maxDepth=Max(maxDepth,depth);owners.push_back(event.pPlayer);if(reenter && !once){once=true;world->GivePlayerTurn(reenter);}--depth;}
};
struct NativeSavedTurnEventObserver {
    int event=-1;CPtr<CObjectBase> param;
    int operator&(CStructureSaver& saver){saver.Add(2,&event);saver.Add(3,&param);return 0;}
};
struct NativeSavedTBSObserver {
    list<NativeSavedTurnEventObserver> events;
    int operator&(CStructureSaver& saver){saver.Add(7,&events);return 0;}
};
template<class T> static auto NativeQueuedTurnWire(NWorld::CPlayer* owner,int)->decltype(sizeof(T),bool()) {
    T event;event.event=static_cast<decltype(event.event)>(9);event.pParam=owner;
    CStructureNetworkContext refs;refs.externalReferences=true;refs.ids[owner]=1;refs.refs[1]=owner;refs.nextID=100;refs.include=[](CObjectBase*){return false;};
    CMemoryStream saved;{CStructureSaver writer(saved,CStructureSaver::WRITE,&refs);writer.Add(2,&event);}saved.Seek(0);
    T restored;{CStructureSaver reader(saved,CStructureSaver::READ,&refs);reader.Add(2,&restored);}
    Require(int(restored.event)==9 && restored.pParam==owner,"native queued-event roundtrip lost its event code or captured owner");return true;
}
template<class T> static bool NativeQueuedTurnWire(NWorld::CPlayer*,long){return false;}
static void NativeDeferredMoveControl(const char* databasePath,const char* resourcePath,bool ai)
{
    CObj<NRPG::CGlobalGame> game;
    auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    struct Restore {NWorld::CWorld* previous=NWorld::pCurrentWorld;~Restore(){NWorld::pCurrentWorld=previous;}} restore;NWorld::pCurrentWorld=world;
    vector<CPtr<NWorld::CUnit>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;
    CObj<NAI::CPath> route;
    for(auto entry: units) {
        auto* unit=dynamic_cast<NWorld::CUnitServer*>(entry.GetPtr());
        if(!unit || unit->IsAIUnit()!=ai || !unit->CanFight() || unit->IsWearingPK() || unit->IsEmptyPK() || !unit->GetUnitRPG()->CanMove())continue;
        world->GivePlayerTurn(dynamic_cast<NWorld::CPlayer*>(unit->GetPlayer()));NativeDrainTurnEvents(world.GetPtr(),0);
        vector<NAI::SPathPlace> places;
        world->GetPathNetwork()->GetNearPlaces(SSphere(unit->GetPosition().GetCP(),6.f),&places);
        for(auto destination: places) {
            destination.SetPose(NAI::CM_STAND);destination.SetDirection(unit->GetPosition().GetDir());
            auto target=unit->GetPosition().pos;target.p=destination;
            if(!world->GetPathNetwork()->IsPassable(destination) || world->GetPathNetwork()->IsLocked(destination))continue;
            const float distance2=fabs2(target.GetCP()-unit->GetPosition().GetCP());
            if(distance2<9.f || distance2>16.f)continue;
            CObj<NWorld::CCmdPath> command=new NWorld::CCmdPath(target);
            if(unit->CanDo(command)!=NWorld::UCR_OK)continue;
            route=NWorld::FindPath(world->GetPathNetwork(),unit,unit->GetPosition().pos.p,vector<NAI::SPathPlace>{destination},nullptr,true);
            if(route && route->points.size()>=5 && route->points.size()<=7 && route->actions.empty() && std::all_of(route->points.begin(),route->points.end(),[&](const NAI::SPathPlace& p) { return p.IsIntegral() && p.GetLayer()==unit->GetPosition().pos.p.GetLayer() && p.GetPose()==NAI::CM_STAND; })) {actor=unit;break;}
            route=nullptr;
        }
        if(actor)break;
    }
    Require(actor && route,"interrupted-route fixture has no native reachable multi-step actor path");
    const auto initial=actor->GetPosition();
    auto target=initial.pos;target.p=route->points.back();
    CObj<NWorld::CCmdPath> command=new NWorld::CCmdPath(target);
    auto readOrder = [&]() {
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(actor,&refs);
        CObj<NWorld::CCommandExecute> executor;CObj<NWorld::CCmd> current;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(7,&executor);reader.Add(9,&current);}
        return std::make_pair(executor,current);
    };
    auto sameCell = [](const NAI::SPathPlace& a,const NAI::SPathPlace& b) {
        return a.GetX()==b.GetX() && a.GetY()==b.GetY() && a.GetLayer()==b.GetLayer();
    };
    actor->Do(new NWorld::CCmdSetCommand(actor,command));
    auto prepared=readOrder();
    CObj<NAI::CPath> preparedPath=actor->GetCurrentPath();
    Require(prepared.first && prepared.second==command.GetPtr() && preparedPath,
        "interrupted-route fixture did not prepare its real native command and path");

    if(!ai){actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));
    for(int edge=0;edge<32 && sameCell(actor->GetPosition().pos.p,initial.pos.p) && actor->IsPerformingAction();++edge){world->GetTime()->Set(actor->animator.GetTimeEnd());actor->Segment();}
    Require(!sameCell(actor->GetPosition().pos.p,initial.pos.p) && !sameCell(actor->GetPosition().pos.p,target.p) && actor->IsPerformingAction() && actor->HasCommand(),"deferred stop fixture did not start a real intermediate movement step");}
    const int ap=actor->GetAP(),hp=actor->GetUnitRPG()->GetTotalVP();
    world->GlobalSituationHasChanged();
    if(!ai)Require(actor->HasCommand(),"global interruption synchronously discarded the real human route");
    NativeDrainTurnEvents(world.GetPtr(),0);
    if(ai){actor->Do(new NWorld::CCmdSetCommand(actor,new NWorld::CCmdContinue));Require(!actor->HasCommand() && !actor->IsPerformingAction(),"queued global interruption retained a stale AI movement order");}
    else {
        Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp,"queued movement cancellation itself spent extra AP or health");
        auto stopped=readOrder();Require(stopped.first && stopped.second==command.GetPtr() && actor->HasCommand(),"queued global interruption lost the human destination");
        // Cancel queues native EndMove; the current animation must retire before Continue.
        for(int edge=0;edge<32 && readOrder().first;++edge){world->GetTime()->Set(actor->animator.GetTimeEnd());actor->Segment();}
        auto retired=readOrder();Require(!retired.first && retired.second==command.GetPtr() && stopped.first->GetState()==NWorld::CCommandExecute::FAILED && !actor->IsPerformingAction(),"queued human stop did not retire its genuine failed mover and retain its route");
        Require(!sameCell(actor->GetPosition().pos.p,target.p) && actor->GetUnitRPG()->GetTotalVP()==hp && actor->CanDo(new NWorld::CCmdContinue)==NWorld::UCR_OK,"retired human route lost its destination or native Continue eligibility");
        std::printf("DEFERRED-MOVE ai=%d retained=%d AP=%d\n",int(ai),int(actor->HasCommand()),actor->GetAP());
        return;
    }
    Require(actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp,"global movement interruption spent extra AP or health");
    std::printf("DEFERRED-MOVE ai=%d retained=%d AP=%d\n",int(ai),int(actor->HasCommand()),actor->GetAP());
}

static void NativeDeferredTurns(const char* databasePath,const char* resourcePath,int mode)
{
    if(mode>=7){NativeDeferredMoveControl(databasePath,resourcePath,mode==8);return;}
    CObj<NRPG::CGlobalGame> game;auto world=FixtureWorld(databasePath,resourcePath,true,&game);
    struct Restore {NWorld::CWorld* previous=NWorld::pCurrentWorld;~Restore(){NWorld::pCurrentWorld=previous;}} restore;NWorld::pCurrentWorld=world;
    vector<CPtr<NWorld::CPlayer>> players;world->GetPlayersList(&players);NWorld::CPlayer* first=nullptr;NWorld::CPlayer* second=nullptr;
    for(auto player:players){if(!first && player && player->GetGlobalPlayer() && !player->GetGlobalPlayer()->bAIPlayer)first=player;}
    Require(first,"deferred turn fixture lacks a real human owner");
    for(auto player:players)if(player && player.GetPtr()!=first && !player->GetPlayerUnits().empty()){second=player;break;}
    Require(second,"deferred turn fixture lacks a real second owner");
    NativeDrainTurnEvents(world.GetPtr(),0);
    CObj<NativeDeferredTurnObserver> observer=new NativeDeferredTurnObserver(world);
    if(mode==3){const bool api=NativeQueuedTurnWire<NWorld::STBSEvent>(first,0);std::printf("DEFERRED-WIRE api=%d\n",int(api));std::fflush(stdout);Require(api,"native STBSEvent wire foundation is missing");return;}
    if(mode==4){world->GivePlayerTurn(first);NativeDrainTurnEvents(world.GetPtr(),0);observer->owners.clear();auto* commander=first->GetCommander();commander->Do(new NWorld::CCmdEndOfTurn);Require(commander->HasCommands(),"deferred global-change setup did not queue its actual command");world->GlobalSituationHasChanged();Require(commander->HasCommands(),"global situation change synchronously cleared queued commands");Require(NativeDrainTurnEvents(world.GetPtr(),0),"native turn-event pump is missing");Require(!commander->HasCommands(),"queued global situation change did not clear the real command queue");return;}
    if(mode==6){world->GivePlayerTurn(first);NativeDrainTurnEvents(world.GetPtr(),0);observer->owners.clear();world->StartSequence();world->EndOfTurn();Require(observer->owners.empty(),"sequence enter/exit notified owners before the event drain");Require(NativeDrainTurnEvents(world.GetPtr(),0),"native turn-event pump is missing");Require(observer->owners.size()==2 && !observer->owners[0] && observer->owners[1]==first,"queued sequence lost its ownerless entry or resumed owner");return;}
    if(mode==2)observer->reenter=second;
    world->GivePlayerTurn(first);
    if(mode==5){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(world,&refs);NativeSavedTBSObserver saved;{CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(1,&saved);}std::printf("DEFERRED-SAVED count=%d\n",int(saved.events.size()));std::fflush(stdout);Require(saved.events.size()==3,"native world save omits its pending turn event queue");auto it=saved.events.begin();Require(it->event==7 && !it->param,"saved global change event is malformed");++it;Require(it->event==1 && it->param==first,"saved new-turn event lost its owner");++it;Require(it->event==9 && it->param==first,"saved control-pass event lost its captured owner");return;}
    if(mode!=2)Require(observer->owners.empty(),"control-pass callback ran synchronously before the event drain");
    if(mode==1)world->GivePlayerTurn(second);
    if(mode==2 && observer->maxDepth>1)Require(false,"turn callback recursively reentered another turn instead of appending events");
    Require(NativeDrainTurnEvents(world.GetPtr(),0),"native turn-event pump is missing");
    const int expected=mode==0?1:2;Require(int(observer->owners.size())==expected && observer->owners.front()==first,"event drain lost or duplicated a captured owner");
    if(expected==2)Require(observer->owners.back()==second,"event drain used current stack owner in place of its captured owner");
    Require(observer->maxDepth==1,"turn callbacks recursively reentered dispatch");
    std::printf("DEFERRED-TURN mode=%d callbacks=%d maxDepth=%d\n",mode,int(observer->owners.size()),observer->maxDepth);
}

static void NativeRuntimeInputSettings(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<8,"unknown runtime input setting case");
    NativeDisplayContext display;display.Initialize(databasePath,resourcePath,"Silent Storm actual input settings");NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(-1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginMission> open=new NGame::CICBeginMission(810,-1,vector<string>(),game);open->Exec();
    CObj<NGame::CMission> mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
    Require(mission && mission->GetWorld(),"input setting did not open the actual mission");mission->PauseGame(true);
    while(NMainLoop::HaveInterfaceCommand()){CObj<NMainLoop::CInterfaceCommand> pending=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(pending.GetPtr()),"input startup has an unexpected transition");}
    auto* world=dynamic_cast<NWorld::CWorld*>(mission->GetWorld());Require(world,"input setting lacks native world");
    struct RestoreWorld {NWorld::CWorld* previous=NWorld::pCurrentWorld;~RestoreWorld(){NWorld::pCurrentWorld=previous;}} restoreWorld;NWorld::pCurrentWorld=world;world->ProcessTBSEvents();
    mission->GetActivePlayer()->Update(false);vector<CPtr<NGame::IUnitTracker>> units;mission->GetActivePlayer()->GetUnits(&units);
    Require(units.size()==1,"input setting needs its single actual party tracker");auto* tracker=dynamic_cast<NGame::CUnitTracker*>(units.front().GetPtr());auto* actor=dynamic_cast<NWorld::CUnitServer*>(tracker->GetUnit());
    Require(actor && actor->CanFight() && !actor->IsWearingPK() && !actor->IsCarryingCorpse(),"input setting lacks an ordinary healthy actor");
    if(mode<3){
        const auto saved=NGlobal::GetVar("game_selectionsensivity",1);struct Restore {NGlobal::CValue value;~Restore(){NGlobal::SetVar("game_selectionsensivity",value);}} restore{saved};
        const float sensitivity=mode==0?3.f:mode==1?.3f:1.f;NGlobal::SetVar("game_selectionsensivity",sensitivity);
        CObj<NGame::CStateMove> move=new NGame::CStateMove(false);Require(mission->CommandState(move),"input setting did not install native move state");
        mission->GetCursor()->SetPos(CVec2(100,100));Require(move->OnLButtonDown(100,100),"native mouse down was not handled");
        mission->GetCursor()->SetPos(CVec2(mode==0?125:110,100));move->Step();
        const bool selected=dynamic_cast<NGame::CStateSelection*>(mission->GetState())!=nullptr;
        std::printf("ACTUAL-SELECTION sensitivity=%g delta=%d selected=%d\n",double(sensitivity),mode==0?25:10,int(selected));
        Require(selected==(mode==1),"native mouse drag ignores the selection sensitivity slider");return;
    }
    Require(mission->IsRealTime() && !mission->IsSequence(),"input fixture must start in actual realtime without a sequence");
    auto position=actor->GetPosition();position.SetPose(NAI::WALK);actor->SetPosition(position);
    if(mode==6){world->GivePlayerTurn(actor->GetTBSPlayer());world->ProcessTBSEvents();Require(!mission->IsRealTime(),"turn-based run guard did not establish a native turn");}
    auto* commander=mission->GetActivePlayer()->GetCommander();Require(!commander->HasCommands(),"input setup left queued commands");
    auto* network=world->GetPathNetwork();vector<NAI::SPathPlace> places;network->GetNearPlaces(SSphere(actor->GetPosition().GetCP(),4.f),&places);
    NAI::SPosition destination;bool found=false;
    for(auto p:places){p.SetPose(NAI::CM_STAND);p.SetMoving(0);if(fabs2(NAI::SPosition(p,network).GetCP()-actor->GetPosition().GetCP())<1.f || !network->IsPassable(p) || network->IsLocked(p))continue;
        CObj<NAI::CPath> path=NWorld::FindPath(network,actor,actor->GetPosition().pos.p,vector<NAI::SPathPlace>{p},nullptr,true);if(path && path->points.size()>1){destination=NAI::SPosition(p,network);found=true;break;}}
    Require(found,"input setting lacks a genuinely reachable native destination");
    const bool instant=mode==4 || mode==5 || mode==6,run=mode==5 || mode==6;
    Require(tracker->SetTargetPosition(destination,instant,run)==NWorld::UCR_OK,"actual unit tracker rejected reachable input");
    vector<CObj<NWorld::CCommand>> commands;while(commander->HasCommands()){CObj<NWorld::CCommand> command=commander->GetCommand();Require(command,"commander queue has an empty command");commands.push_back(command);}
    if(mode==7){
        Require(commands.size()==1,"path overlay setup unexpectedly executed movement");world->ExecuteCommand(commands.front().GetPtr());
        Require(actor->GetCurrentPath(),"path overlay lacks actual prepared pending movement");tracker->SetSelected(true);
        const auto saved=NGlobal::GetVar("game_pathinrealtime",1);struct Restore {NGlobal::CValue value;~Restore(){NGlobal::SetVar("game_pathinrealtime",value);}} restore{saved};
        auto pathVisible=[&](){CStructureNetworkContext refs;auto fields=BorrowedObjectFields(tracker,&refs);bool visible=false;CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(9,&visible);return visible;};
        const auto before=actor->GetPosition();const auto ap=actor->GetAP();const auto hp=actor->GetUnitRPG()->GetTotalVP();
        for(int enabled:{1,0,1}){NGlobal::SetVar("game_pathinrealtime",enabled);MarkNewDGFrame();tracker->Update();Require(pathVisible()==bool(enabled),"native realtime path visibility ignores gameplay option");}
        Require(actor->GetPosition()==before && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp,"path option changed simulation instead of presentation");return;
    }
    const bool running=mode==5;const size_t count=running?5:instant?2:1;
    std::printf("ACTUAL-MOVE realtime=%d instant=%d run=%d commands=%zu expected=%zu\n",int(mission->IsRealTime()),int(instant),int(run),commands.size(),count);
    Require(commands.size()==count,"tracker queued an unexpected implicit Continue or run command");size_t offset=0;
    if(running){Require(dynamic_cast<NWorld::CCmdCancel*>(commands[offset++].GetPtr()),"run did not cancel the old path first");auto* wish=dynamic_cast<NWorld::CCmdSetCommand*>(commands[offset++].GetPtr());auto* pose=wish?dynamic_cast<NWorld::CCmdWishPose*>(wish->GetCmd()):nullptr;Require(wish && wish->pUnit==actor && pose && pose->pose==NAI::RUN,"realtime run did not queue actual owned WishPose");auto* startPose=dynamic_cast<NWorld::CCmdSetCommand*>(commands[offset++].GetPtr());Require(startPose && startPose->pUnit==actor && dynamic_cast<NWorld::CCmdContinue*>(startPose->GetCmd()),"native WishPose lost its default instant Continue");}
    auto* path=dynamic_cast<NWorld::CCmdSetCommand*>(commands[offset++].GetPtr());Require(path && path->pUnit==actor && dynamic_cast<NWorld::CCmdPath*>(path->GetCmd()),"tracker lost native path/actor identity");
    if(instant){auto* continuation=dynamic_cast<NWorld::CCmdSetCommand*>(commands[offset++].GetPtr());Require(continuation && continuation->pUnit==actor && dynamic_cast<NWorld::CCmdContinue*>(continuation->GetCmd()),"explicit instant input did not queue native Continue");}
    Require(offset==commands.size(),"input setting left unexamined native commands");
}


static void NativeCachedLightingPipeline(const char* databasePath,const char* resourcePath,int mode)
{
    Require(mode>=0 && mode<=2,"unknown cached-lighting pipeline mode");
    NativeDisplayContext display;display.Initialize(databasePath,resourcePath,"Silent Storm native cached-lighting pipeline");NGlobal::SetVar("gfx_resolution",wstring(L"320x240"));NGlobal::SetVar("gfx_fullscreen",0);Require(NGScene::SetModeFromConfig(false) && NGScene::CanCacheLighting() && NGScene::CanRenderShadows(),"cached lighting must enable the actual full shadow/cache pipeline");NGame::InitLoadingScreen();
    CObj<NRPG::CGlobalGame> game=NRPG::CreateGlobalGame(-1);game->players.push_back(NRPG::CreateGlobalPlayer(vector<int>{54}));
    CObj<NGame::CICBeginMission> open=new NGame::CICBeginMission(810,-1,vector<string>(),game);open->Exec();
    CObj<NGame::CMission> mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());Require(mission && mission->GetWorld(),"cached lighting did not open its actual mission");mission->PauseGame(true);mission->SetCheatVisibility(true);mission->GetDesktop()->SetStyle(NUI::STYLE_VISIBLE,false);
    while(NMainLoop::HaveInterfaceCommand()){CObj<NMainLoop::CInterfaceCommand> pending=NMainLoop::TakeInterfaceCommandForHarness();Require(dynamic_cast<NMainLoop::CICSaveFile*>(pending.GetPtr()),"cached lighting has an unexpected startup transition");}
    vector<CPtr<NGame::IUnitTracker>> units;mission->GetActivePlayer()->Update(false);mission->GetActivePlayer()->GetUnits(&units);Require(units.size()==1,"cached lighting lacks its real party actor");auto* actor=dynamic_cast<NWorld::CUnitServer*>(units.front()->GetUnit());Require(actor,"cached lighting actor is not native");auto* world=dynamic_cast<NWorld::CWorld*>(mission->GetWorld());Require(world,"cached lighting requires native world");
    const CVec3 origin=actor->GetPosition().GetCP();
    CObj<CObjectBase> shadow=mission->GetScene()->AddPointLight(CVec3(2,.6f,.2f),origin+CVec3(0,0,3),12,true,true);
    CObj<CObjectBase> clear=mission->GetScene()->AddPointLight(CVec3(.2f,.4f,1),origin+CVec3(2,0,2),8,true,false);
    Require(shadow && clear && dynamic_cast<NGScene::CPointLight*>(shadow.GetPtr()) && NativeAuthoredPointShadow(dynamic_cast<NGScene::CPointLight*>(shadow.GetPtr())) && !NativeAuthoredPointShadow(dynamic_cast<NGScene::CPointLight*>(clear.GetPtr())),"actual game-view light factory lost shadowed/unshadowed cache inputs");
    auto* camera=mission->GetCamera();camera->FreezeCamera(true);ICamera::SCameraPos base;camera->GetPlacement(&base);base.ptAnchor=origin+CVec3(0,0,1);base.fRod=18;camera->SetPlacement(base);
    struct RestoreClock {STime previous=NMainLoop::SetTimeForHarness(100000);~RestoreClock(){NMainLoop::SetTimeForHarness(previous);}} restoreClock;
    NGlobal::SetVar("gfx_hw_hsr",0);
    int frame=0;
    auto render=[&](const ICamera::SCameraPos& position,int count){for(int i=0;i<count;++i){camera->SetPlacement(position);NMainLoop::SetTimeForHarness(100000+(++frame)*40);MarkNewDGFrame();Require(NMainLoop::StepApp(true,false,false) && NMainLoop::GetCurrentInterfaceForHarness()==mission.GetPtr() && NGfx::Is3DActive(),"native cached-lighting frame failed or lost its real renderer");}CArray2D<NGfx::SPixel8888> pixels;NGfx::MakeScreenShot(&pixels,false);Require(pixels.GetXSize()>=128 && pixels.GetYSize()>=128,"cached lighting has no actual GPU image");int lit=0;for(int y=8;y<pixels.GetYSize()-8;++y)for(int x=8;x<pixels.GetXSize()-8;++x)lit+=pixels[y][x].r>8 || pixels[y][x].g>8 || pixels[y][x].b>8;Require(lit>(pixels.GetXSize()-16)*(pixels.GetYSize()-16)/10,"cached-lighting frame is blank");return pixels;};
    auto first=render(base,36);const auto position=actor->GetPosition();const auto ap=actor->GetAP();const auto hp=actor->GetUnitRPG()->GetTotalVP();const auto time=world->GetTime()->GetValue();
    auto moved=base;moved.fYaw+=.35f;moved.fRod=22;moved.ptAnchor+=CVec3(1,0,0);auto alternate=render(moved,18);auto returned=render(base,36);
    Require(first.GetXSize()==returned.GetXSize() && first.GetYSize()==returned.GetYSize(),"camera-only cache reuse changed drawable dimensions");int changedView=0,changedReturn=0;const int total=(first.GetXSize()-16)*(first.GetYSize()-16);
    for(int y=8;y<first.GetYSize()-8;++y)for(int x=8;x<first.GetXSize()-8;++x){const auto a=first[y][x],b=alternate[y][x],c=returned[y][x];changedView+=abs(int(a.r)-int(b.r))+abs(int(a.g)-int(b.g))+abs(int(a.b)-int(b.b))>24;changedReturn+=abs(int(a.r)-int(c.r))+abs(int(a.g)-int(c.g))+abs(int(a.b)-int(c.b))>24;}
    std::printf("ACTUAL-CACHED-LIGHTING mode=%d frames=%d changedCamera=%d changedReturn=%d pixels=%d\n",mode,frame,changedView,changedReturn,total);std::fflush(stdout);
    Require(changedView>total/20,"cached-lighting fixture did not actually change its rendered camera");Require(changedReturn<total/20,"returning the native camera left widespread cached-lighting artifacts");
    if(mode==2){Require(NGfx::DoesSupportOcclusionQueries(),"actual mission hardware query support missing");NGlobal::SetVar("gfx_hw_hsr",1);auto hwMoved=render(moved,18);auto hwReturned=render(base,36);int changed=0;for(int y=8;y<first.GetYSize()-8;++y)for(int x=8;x<first.GetXSize()-8;++x){const auto a=returned[y][x],b=hwReturned[y][x];changed+=abs(int(a.r)-int(b.r))+abs(int(a.g)-int(b.g))+abs(int(a.b)-int(b.b))>24;}std::printf("ACTUAL-HW-HSR frames=%d off-on-return-difference=%d pixels=%d\n",frame,changed,total);std::fflush(stdout);Require(changed<total/20,"hardware queries removed visible native mission surfaces");NGlobal::SetVar("gfx_hw_hsr",0);}
    if(mode==1){const int generation=NGfx::GetDeviceCreationID();NGlobal::SetVar("gfx_resolution",wstring(L"384x288"));Require(NGScene::SetModeFromConfig(false) && NGfx::GetDeviceCreationID()!=generation,"actual native graphics-object recreation did not invalidate cache generation");auto reset=render(base,36);Require(reset.GetXSize()==384 && reset.GetYSize()==288,"native graphics reset did not use its requested real dimensions");}
    Require(actor->GetPosition()==position && actor->GetAP()==ap && actor->GetUnitRPG()->GetTotalVP()==hp && world->GetTime()->GetValue()==time,"camera/lighting cache changed paused simulation position/AP/HP/time");
}


static void DefaultMeleeLifetime(const char* databasePath)
{
    CFileStream database; database.OpenRead(databasePath);
    NDatabase::Serialize(database,CStructureSaver::READ);
    CObj<NRPG::IUnitMission> mission=NRPG::CreateUnit(NDb::GetPers(103));
    auto* unit=mission->GetRPGUnit();
    CObj<NRPG::IInventoryItem> first=unit->pInventory->TakeOff(NDb::SLOT_1);
    CObj<NRPG::IInventoryItem> second=unit->pInventory->TakeOff(NDb::SLOT_2);
    CPtr<NRPG::CMeleeWeaponItem> fists=unit->GetMeleeWeaponItem();
    Require(IsValid(fists),"unarmed unit has no default melee weapon");
    for(int i=0;i<3;++i) {
        { CObj<NRPG::IMeleeWeaponItem> attack=unit->GetMeleeWeaponItem(); }
        Require(IsValid(fists) && unit->GetMeleeWeaponItem()==fists.GetPtr(),
            "ending a melee attack destroyed the unit's default fists");
    }
    CMemoryStream saved; {CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(1,&mission);}
    saved.Seek(0);CObj<NRPG::IUnitMission> restored;
    {CStructureSaver reader(saved,CStructureSaver::READ);reader.Add(1,&restored);}
    CPtr<NRPG::CMeleeWeaponItem> loaded=restored->GetRPGUnit()->GetMeleeWeaponItem();
    {CObj<NRPG::IMeleeWeaponItem> attack=loaded.GetPtr();}
    Require(IsValid(loaded),"loaded fists have no lifetime owner");
}

static void ObjectivesValueRoundTrip(const char* databasePath)
{
    CFileStream database;database.OpenRead(databasePath);NDatabase::Serialize(database,CStructureSaver::READ);
    NScenario::SGoalDescription goal;goal.pString=NDb::GetString(4382);goal.state=NScenario::STS_COMPLETED;
    NScenario::STaskDescription task;task.pString=NDb::GetString(4382);task.state=NScenario::STS_FAILED;
    goal.tasks.push_back(task);
    CMemoryStream saved;{CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(1,&goal);}
    saved.Seek(0);
    // The old raw-vector reader aliases the source storage. Keep that invalid
    // object alive on failure so the assertion reports the defect before a double free.
    auto* restored=new NScenario::SGoalDescription;
    {CStructureSaver reader(saved,CStructureSaver::READ);reader.Add(1,restored);}
    Require(restored->tasks.size()==1 && restored->tasks.data()!=goal.tasks.data(),
        "saved objectives restored raw vector storage instead of their values");
    Require(restored->pString==goal.pString && restored->state==goal.state &&
        restored->tasks[0].pString==task.pString && restored->tasks[0].state==task.state,
        "objectives lost localized text or completion state");
    delete restored;
}

static void ObjectivesRowCompatibility()
{
    for(int type:{int(0xB3225131),int(0xB3225132)}) {
        CObj<CObjectBase> row=pSSClasses->CreateObject(type);
        Require(IsValid(row),"objective row class is unregistered");
        unsigned char invalid[40];memset(invalid,0xff,sizeof(invalid));
        CMemoryStream legacy;
        {CStructureSaver writer(legacy,CStructureSaver::WRITE);writer.AddRawData(type==int(0xB3225131)?3:4,invalid,sizeof(invalid));}
        legacy.Seek(0);{CStructureSaver reader(legacy,CStructureSaver::READ);row->operator&(reader);}
        CMemoryStream saved;{CStructureSaver writer(saved,CStructureSaver::WRITE);writer.Add(1,&row);}
        saved.Seek(0);CObj<CObjectBase> restored;
        {CStructureSaver reader(saved,CStructureSaver::READ);reader.Add(1,&restored);}
        Require(restored && typeid(*restored.GetPtr())==typeid(*row.GetPtr()),"legacy row did not survive ordinary save/reload");
    }
}

static void ConvenienceDefaultsAndInput()
{
    Require(NGlobal::GetVar("ui_weapon_comparison").GetInt()==1 && NGlobal::GetVar("ui_keyboard_help").GetInt()==1 &&
        NGlobal::GetVar("input_mac_numpad").GetInt()==1 && NGlobal::GetVar("game_other_turn_speed").GetInt()==1,
        "convenience defaults are incorrect");
    for(int speed=1;speed<=4;++speed) {
        Require(NGame::OtherTurnSpeed(speed,false,false,false,false,false)==speed,"speed selection was lost");
        for(int gate=0;gate<5;++gate)Require(NGame::OtherTurnSpeed(speed,gate==0,gate==1,gate==2,gate==3,gate==4)==1,
            "speed escaped the network/realtime/player/sequence/pause guard");
    }
    Require(NGame::OtherTurnSpeed(99,false,false,false,false,false)==1,"invalid speed is not normal");
    for(int digit=0;digit<10;++digit) {
        auto source=digit?SDL_Scancode(SDL_SCANCODE_1+digit-1):SDL_SCANCODE_0;
        auto target=digit?SDL_Scancode(SDL_SCANCODE_KP_1+digit-1):SDL_SCANCODE_KP_0;
        Require(NInput::MacNumpadAlias(source,true,true)==target,"Mac digit does not reach numpad");
        Require(NInput::MacNumpadAlias(source,false,true)==source && NInput::MacNumpadAlias(source,true,false)==source,
            "ordinary digits or disabled aliases were changed");
    }
    Require(NInput::MacNumpadAlias(SDL_SCANCODE_S,true,true)==SDL_SCANCODE_S,"Mac aliases added an unrelated save shortcut");
    NInput::MacNumpadState aliases;SDL_Scancode mapped=SDL_SCANCODE_UNKNOWN;
    Require(aliases.Route(SDL_SCANCODE_8,true,true,true,false,&mapped) && mapped==SDL_SCANCODE_KP_8,"Mac alias press failed");
    Require(aliases.Route(SDL_SCANCODE_8,true,false,false,true,&mapped) && mapped==SDL_SCANCODE_KP_8,"Mac repeat lost its original key route");
    Require(!aliases.Route(SDL_SCANCODE_KP_8,true,false,true,false,&mapped),"physical numpad duplicated an alias press");
    Require(!aliases.Route(SDL_SCANCODE_8,false,false,false,false,&mapped),"releasing Command or disabling aliases released a held physical numpad");
    Require(aliases.Route(SDL_SCANCODE_KP_8,false,false,false,false,&mapped) && mapped==SDL_SCANCODE_KP_8,"last numpad release was lost");
    aliases.Route(SDL_SCANCODE_5,true,true,true,false,&mapped);aliases.Reset();
    Require(!aliases.Route(SDL_SCANCODE_5,false,false,true,false,&mapped),"focus loss retained a stale alias");
    NGlobal::ProcessCommand(L"bind test_custom_reference 'F12'");
    vector<string> commands;NInput::GetBoundCommands(&commands);
    Require(std::find(commands.begin(),commands.end(),string("test_custom_reference"))!=commands.end(),"help omits custom bindings");
    NInput::Unbind("test_custom_reference");NInput::GetBoundCommands(&commands);
    Require(std::find(commands.begin(),commands.end(),string("test_custom_reference"))==commands.end(),"help retains removed bindings");
}

static void WeaponComparisonState(const char* databasePath,const char* resourcePath)
{
    auto world=FixtureWorld(databasePath,resourcePath);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->GetUnitRPG() && unit->GetUnitRPG()->GetRPGUnit()->GetWeaponItem()) {actor=unit;break;}
    Require(actor,"comparison fixture has no equipped native actor");
    auto* rpg=actor->GetUnitRPG()->GetRPGUnit();auto* held=rpg->GetWeaponItem();
    CObj<NRPG::CWeaponItem> candidate=held->CreateComparisonPreview(held->GetShootMode());
    Require(candidate,"comparison fixture cannot create a detached candidate");
    const auto mode=held->GetShootMode();const int ammo=held->GetAmmoQuantity();
    const auto* active=rpg->pInventory->GetActive();const int ap=actor->GetAP();
    CPtr<NRPG::CUnit> rpgReference=rpg;
    CMemoryStream before;{CStructureSaver writer(before,CStructureSaver::WRITE);writer.Add(1,&rpgReference);}
    NGlobal::SetVar("ui_weapon_comparison",1);
    for(int i=0;i<20;++i)Require(!NUI::WeaponComparisonText(actor,candidate,i%2).empty(),"weapon comparison is empty");
    CMemoryStream after;{CStructureSaver writer(after,CStructureSaver::WRITE);writer.Add(1,&rpgReference);}
    Require(held->GetShootMode()==mode && held->GetAmmoQuantity()==ammo && rpg->pInventory->GetActive()==active &&
        actor->GetAP()==ap,"weapon comparison changed gameplay state");
    Require(before.GetSize()==after.GetSize() && !memcmp(before.GetBuffer(),after.GetBuffer(),before.GetSize()),
        "weapon comparison changed serialized soldier state");
    NGlobal::SetVar("ui_weapon_comparison",0);
    Require(NUI::WeaponComparisonText(actor,candidate,true).empty(),"comparison cannot be disabled");
}

class ConvenienceClockMission: public NGame::CMission
{
    OBJECT_NOCOPY_METHODS(ConvenienceClockMission);
protected:
    const STime GetTime() override {return 1050;}
public:
    bool player=false,realtime=false,sequence=false;
    bool IsPlayerTurn() const override {return player;}
    bool IsRealTime() const override {return realtime;}
    bool IsSequence() const override {return sequence;}
    void Prepare(NWorld::CWorld* world,bool pause) {
        pWorld=world;pRender=new MissionSoundClockRenderer;bPause=pause;
        sMinFrameTime=0;sTimeCounter.SetCurrent(10000);sTimeCounter.Advance(false,1000);
        nDeltaTime=0;
    }
};
static void ConvenienceMissionClock(const char* databasePath,const char* resourcePath)
{
    auto world=FixtureWorld(databasePath,resourcePath);
    Require(!NGfx::Is3DActive(),"clock fixture requires the non-rendering mission path");
    for(int speed=1;speed<=4;++speed)for(int gate=0;gate<5;++gate) {
        CObj<ConvenienceClockMission> mission=new ConvenienceClockMission;
        mission->player=gate==1;mission->realtime=gate==2;mission->sequence=gate==3;
        mission->Prepare(world,gate==4);NGlobal::SetVar("game_other_turn_speed",speed);
        bool reached=false;
        try {mission->Step();}catch(const std::runtime_error& error) {reached=string(error.what())=="unexpected sound-clock reset";if(!reached)throw;}
        Require(reached,"actual mission Step did not reach its timing reset");
        const int elapsed=mission->GetGameTime()-10000;
        Require(elapsed==(gate==4?0:gate?50:50*speed),"actual mission clock ignored speed or accelerated a protected phase");
    }
}

static void ConvenienceSettingsUI(const char* databasePath,const char* resourcePath)
{
    Require(std::getenv("S2_USER_DATA_DIR"),"settings test requires an isolated user directory");
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm convenience settings");
    Require(NInput::InitInput(true),"convenience flow requires native input");
    struct InputCleanup { ~InputCleanup() { NMainLoop::DoneInterface(); NInput::DoneInput(); } } inputCleanup;
    NGlobal::LoadConfig((std::filesystem::path(databasePath).parent_path()/"cfg/input.cfg").string());
    NGfx::SRenderTargetsInfo targets;
    Require(NGfx::SetMode(NGfx::SVideoMode(1024,768,32,NGfx::WINDOWED),targets),"convenience UI has no full-size renderer");
    CObj<NGame::CICOptions> open=new NGame::CICOptions(NGame::OS_GAMEPLAY);open->Exec();
    CStructureNetworkContext references;
    auto fields=BorrowedObjectFields(NMainLoop::GetCurrentInterfaceForHarness(),&references);
    CPtr<NUI::CInterface> ui;
    {CStructureSaver reader(fields,CStructureSaver::READ,&references);reader.Add(4,&ui);}
    Require(ui,"options has no active native UI");
    auto capture=[&](const char* name) {
        for(int frame=0;frame<4;++frame) {MarkNewDGFrame();NMainLoop::GetCurrentInterfaceForHarness()->Step();}
        if(const char* directory=std::getenv("S2_CONVENIENCE_EVIDENCE_DIR")) {
            CArray2D<NGfx::SPixel8888> pixels;NGfx::MakeScreenShot(&pixels,false);
            std::filesystem::create_directories(directory);
            std::ofstream file(std::filesystem::path(directory)/(string(name)+".ppm"),std::ios::binary);
            file<<"P6\n"<<pixels.GetXSize()<<" "<<pixels.GetYSize()<<"\n255\n";
            for(int y=0;y<pixels.GetYSize();++y)for(int x=0;x<pixels.GetXSize();++x) {
                const char rgb[]={char(pixels[y][x].r),char(pixels[y][x].g),char(pixels[y][x].b)};file.write(rgb,3);
            }
        }
    };
    Require(ui->GetChildByID("options") && ui->GetChildByID("options")->GetChildByID("convenience_options"),"gameplay settings have no convenience button");
    capture("gameplay-settings");
    auto click=[&](NUI::CWindow* button) {
        NUI::SPoint position;NUI::SRect rect;
        Require(button && button->ClientToScreen(&position,&rect) && rect.Width()>0 && rect.Height()>0,"convenience control has no hit rectangle");
        const int x=(rect.x1+rect.x2)/2,y=(rect.y1+rect.y2)/2;
        ui->ProcessMessage(NUI::SEvent(NUI::EVENT_MOUSEMOVE,x,y));
        ui->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONDOWN,x,y));
        ui->ProcessMessage(NUI::SEvent(NUI::EVENT_LBUTTONUP,x,y));
    };
    auto key=[&](SDL_Scancode scan) {
        for(bool down:{true,false}) {
            SDL_Event event{};event.type=down?SDL_EVENT_KEY_DOWN:SDL_EVENT_KEY_UP;
            event.key.windowID=SDL_GetWindowID(S2Platform::Window());event.key.scancode=scan;event.key.down=down;
            Require(SDL_PushEvent(&event),"convenience flow could not queue native key");
            S2Platform::PumpEvents();NInput::PumpMessages(true,true);
            NInput::SEvent input;while(NInput::GetEvent(&input))NMainLoop::GetCurrentInterfaceForHarness()->ProcessEvent(input);
        }
    };
    click(ui->GetChildByID("options")->GetChildByID("convenience_options"));
    auto* settings=ui->GetChildByID("convenience_settings");
    Require(settings && settings->GetStyle(NUI::STYLE_VISIBLE),"gameplay button did not open convenience settings");
    for(const auto& id:{"convenience_compare","convenience_help"}) {
        const char* variable=!strcmp(id,"convenience_compare")?"ui_weapon_comparison":"ui_keyboard_help";
        click(settings->GetChildByID(id));
        Require(NGlobal::GetVar(variable).GetInt()==0,"convenience toggle did not switch off");
        click(settings->GetChildByID(id));
        Require(NGlobal::GetVar(variable).GetInt()==1,"convenience toggle did not switch on");
    }
    for(int speed:{2,3,4,1}) {click(settings->GetChildByID("convenience_speed"));
        Require(NGlobal::GetVar("game_other_turn_speed").GetInt()==speed,"speed menu lost its selection");
        auto* caption=dynamic_cast<NUI::CText*>(settings->GetChildByID("convenience_speed")->GetChildByID("caption"));
        Require(caption && (speed==1 || caption->GetText().find(wstring(L"\u00d7")+wchar_t(L'0'+speed))!=wstring::npos),
            "speed menu does not display its selected multiplier");}
    capture("convenience-settings");
    click(settings->GetChildByID("convenience_keys"));
    Require(ui->GetChildByID("keyboard_help")->GetStyle(NUI::STYLE_VISIBLE),"keyboard reference did not open");
    capture("keyboard-reference");
    click(ui->GetChildByID("keyboard_help")->GetChildByID("keys_next"));
    Require(ui->keyboardHelpPage==1,"next page did not change the keyboard reference");
    click(ui->GetChildByID("keyboard_help")->GetChildByID("keys_previous"));
    Require(ui->keyboardHelpPage==0,"previous page did not restore the reference");
    click(ui->GetChildByID("keyboard_help")->GetChildByID("keys_previous"));
    Require(ui->keyboardHelpPage==0,"keyboard reference moved before its first page");
    key(SDL_SCANCODE_ESCAPE);
    Require(!ui->GetChildByID("keyboard_help")->GetStyle(NUI::STYLE_VISIBLE),"native Escape did not close the keyboard reference");
    key(SDL_SCANCODE_F1);
    Require(ui->GetChildByID("keyboard_help")->GetStyle(NUI::STYLE_VISIBLE),"native F1 did not open the keyboard reference");
    key(SDL_SCANCODE_F1);
    Require(!ui->GetChildByID("keyboard_help")->GetStyle(NUI::STYLE_VISIBLE),"native F1 did not close the keyboard reference");
    click(settings->GetChildByID("convenience_help"));
    key(SDL_SCANCODE_F1);
    Require(!ui->GetChildByID("keyboard_help")->GetStyle(NUI::STYLE_VISIBLE),"disabled F1 reference still opened");
    click(settings->GetChildByID("convenience_keys"));
    Require(ui->GetChildByID("keyboard_help")->GetStyle(NUI::STYLE_VISIBLE),"disabled hotkey removed menu access to the reference");
    key(SDL_SCANCODE_ESCAPE);
    click(settings->GetChildByID("convenience_compare"));
    click(settings->GetChildByID("convenience_speed"));
    // Read the configuration actually written by the buttons, after replacing all live values.
    NGlobal::SetVar("ui_keyboard_help",1);NGlobal::SetVar("ui_weapon_comparison",1);NGlobal::SetVar("game_other_turn_speed",4);
    NGlobal::LoadConfig(".\\cfg\\config.cfg");
    Require(NGlobal::GetVar("ui_keyboard_help").GetInt()==0 && NGlobal::GetVar("ui_weapon_comparison").GetInt()==0 &&
        NGlobal::GetVar("game_other_turn_speed").GetInt()==2,"menu choices were not persisted to the user's config");
    click(settings->GetChildByID("convenience_help"));
    NGlobal::ProcessCommand(L"bind convenience_custom_f1 'F1'");
    key(SDL_SCANCODE_F1);
    Require(!ui->GetChildByID("keyboard_help")->GetStyle(NUI::STYLE_VISIBLE),"reference took over an existing F1 binding");
    NInput::Unbind("convenience_custom_f1");
    key(SDL_SCANCODE_F1);
    Require(ui->GetChildByID("keyboard_help")->GetStyle(NUI::STYLE_VISIBLE),"reference did not recover after removing an F1 binding");
    key(SDL_SCANCODE_ESCAPE);
    key(SDL_SCANCODE_ESCAPE);
    Require(!settings->GetStyle(NUI::STYLE_VISIBLE),"Escape did not close convenience settings");
    Require(!NMainLoop::HaveInterfaceCommand(),"closing a convenience panel also closed its underlying game menu");
    auto referenceContains=[&](const wstring& caption) {
        ui->ShowKeyboardHelp();auto* panel=ui->GetChildByID("keyboard_help");bool found=false;
        for(int page=0;page<32;++page) {
            for(int row=0;row<12;++row) {
                char id[32];sprintf(id,"key_row_%d",row);
                auto* text=dynamic_cast<NUI::CText*>(panel->GetChildByID(id));
                found|=text && text->GetText().find(caption)!=wstring::npos;
            }
            const int current=ui->keyboardHelpPage;click(panel->GetChildByID("keys_next"));
            if(ui->keyboardHelpPage==current) {key(SDL_SCANCODE_ESCAPE);return found;}
        }
        throw std::runtime_error("keyboard reference pagination never reached its last page");
    };
    NGlobal::ProcessCommand(L"bind convenience_dynamic_reference 'F12'");
    Require(referenceContains(L"Convenience dynamic reference"),"keyboard reference omitted a newly assigned shortcut");
    NInput::Unbind("convenience_dynamic_reference");
    Require(!referenceContains(L"Convenience dynamic reference"),"keyboard reference retained a removed shortcut");
}

static void ConvenienceTooltipUI(const char* databasePath,const char* resourcePath)
{
    NativeDisplayContext cleanup;cleanup.Initialize(databasePath,resourcePath,"Silent Storm weapon tooltip flow");
    auto world=FixtureWorld(databasePath,resourcePath);
    list<CPtr<NWorld::CUnitServer>> units;world->GetAllUnits(&units);
    NWorld::CUnitServer* actor=nullptr;
    for(auto unit:units)if(unit && unit->GetUnitRPG() && unit->GetUnitRPG()->GetWeaponItem()) {actor=unit;break;}
    Require(actor,"tooltip flow has no equipped soldier");
    auto* rpg=actor->GetUnitRPG()->GetRPGUnit();auto* held=rpg->GetWeaponItem();
    CObj<NRPG::CWeaponItem> candidate=held->CreateComparisonPreview(held->GetShootMode());
    CObj<NUI::CInterface> ui=new NUI::CInterface(NUI::ICursor::Create());
    CObj<NUI::CShowItemModel> item=new NUI::CShowItemModel(NUI::SWindowInfo(ui,NUI::SPoint(20,20),NUI::SPoint(80,80),"comparison_item"));
    item->Set(nullptr,actor,candidate,NDb::CAMERA_NORMAL);
    auto* tip=item->GetToolTip();Require(tip,"weapon slot did not create its real tooltip");
    auto value=[&]() {
        CStructureNetworkContext refs;auto fields=BorrowedObjectFields(tip,&refs);CPtr<NUI::CText> text;
        {CStructureSaver reader(fields,CStructureSaver::READ,&refs);reader.Add(2,&text);}
        Require(text,"weapon tooltip lost its text control");wstring result;text->GetVal(L"weaponcompare",&result);return result;
    };
    CPtr<NRPG::CUnit> reference=rpg;CMemoryStream before;
    {CStructureSaver writer(before,CStructureSaver::WRITE);writer.Add(1,&reference);}
    auto random=GlobalGameRandom();const auto modifiers=SDL_GetModState();
    struct Modifiers {SDL_Keymod original;~Modifiers(){SDL_SetModState(original);}} restore{modifiers};
    NGlobal::SetVar("ui_weapon_comparison",1);SDL_SetModState(SDL_KMOD_NONE);tip->ShowWindow(NUI::SWTYPE_SHOW);
    item->Draw(1000,ui->GetView());
    const auto shortText=value();Require(!shortText.empty() && shortText==NUI::WeaponComparisonText(actor,candidate,false),"visible inventory tooltip did not refresh the comparison");
    tip->UpdateToolTip();const int comparedHeight=tip->GetSize().y;
    // An old save can restore the inventory and its pre-comparison tooltip text.
    tip->SetText(L"Legacy weapon <value id=name>");
    CStructureNetworkContext legacyRefs;auto legacyFields=BorrowedObjectFields(item,&legacyRefs);
    CObj<NUI::CShowItemModel> restored=new NUI::CShowItemModel;
    {CStructureSaver reader(legacyFields,CStructureSaver::READ,&legacyRefs);restored->operator&(reader);}
    restored->Draw(1000,ui->GetView());
    CStructureNetworkContext textRefs;auto tipFields=BorrowedObjectFields(tip,&textRefs);CPtr<NUI::CText> restoredText;
    {CStructureSaver reader(tipFields,CStructureSaver::READ,&textRefs);reader.Add(2,&restoredText);}
    Require(restoredText && restoredText->GetText().find(L"<value id=weaponcompare>")!=wstring::npos,
        "legacy saved inventory tooltip did not acquire its comparison placeholder");
    SDL_SetModState(SDL_KMOD_SHIFT);item->Draw(1001,ui->GetView());
    Require(value()!=shortText && value()==NUI::WeaponComparisonText(actor,candidate,true),"Shift did not refresh tooltip details immediately");
    NGlobal::SetVar("ui_weapon_comparison",0);item->Draw(1002,ui->GetView());
    Require(value().empty(),"disabled comparison remained visible in the inventory tooltip");
    tip->UpdateToolTip();
    Require(tip->GetSize().y<comparedHeight,"comparison values did not expand the rendered tooltip");
    NGlobal::SetVar("ui_weapon_comparison",1);SDL_SetModState(SDL_KMOD_NONE);item->Draw(1003,ui->GetView());
    Require(value()==shortText,"re-enabled comparison did not return immediately");
    item->Set(nullptr,actor,held,NDb::CAMERA_NORMAL);tip=item->GetToolTip();tip->ShowWindow(NUI::SWTYPE_SHOW);
    item->Draw(2000,ui->GetView());Require(value().empty(),"equipped weapon compared with itself");
    CMemoryStream after;{CStructureSaver writer(after,CStructureSaver::WRITE);writer.Add(1,&reference);}
    auto endRandom=GlobalGameRandom();bool sameRandom=true;for(int i=0;i<8;++i)sameRandom&=random.Get()==endRandom.Get();
    Require(sameRandom && before.GetSize()==after.GetSize() && !memcmp(before.GetBuffer(),after.GetBuffer(),before.GetSize()),
        "inventory tooltip changed soldier state or the game random sequence");
}

static void LegacyDirectionalAmbient()
{
    CObj<NGScene::CCVec3> color=new NGScene::CCVec3(CVec3(1,1,1));
    CObj<NGScene::CCVec3> gloss=new NGScene::CCVec3(VNULL3);
    CObj<NGScene::CCVec3> ambient=new NGScene::CCVec3(CVec3(.2f,.3f,.4f));
    // Older saves retain the mean ambient but may have no upper/lower ambient edges.
    CObj<NGScene::CDirectionalLight> light=new NGScene::CDirectionalLight(color,gloss,VNULL3,
        CVec3(0,0,1),VNULL3,CVec2(8,8),16,nullptr,ambient,true,0,nullptr,nullptr,nullptr);
    NGScene::CDirectionalLight::SRadianceInfo info;MarkNewDGFrame();light->GetRadianceInfo(&info,NGScene::RP_GF2);
    Require(info.bIsRendered && info.vAmbientColor==CVec3(.2f,.3f,.4f) && info.vUpDifColor==VNULL3,
        "legacy directional lighting lost its saved mean ambient");
    CObj<NGScene::CCVec3> top=new NGScene::CCVec3(CVec3(.4f,.6f,.8f));
    CObj<NGScene::CDirectionalLight> complete=new NGScene::CDirectionalLight(color,gloss,VNULL3,
        CVec3(0,0,1),VNULL3,CVec2(8,8),16,nullptr,ambient,true,0,nullptr,top,gloss);
    complete->GetRadianceInfo(&info,NGScene::RP_GF2);
    Require(info.vAmbientColor==CVec3(.2f,.3f,.4f) && info.vUpDifColor==CVec3(.2f,.3f,.4f),
        "legacy lighting fallback changed a complete directional light");
}
