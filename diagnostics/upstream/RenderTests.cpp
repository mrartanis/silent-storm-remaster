#include "TestSupport.h"
#include "../../Main/GParticleFormat.h"
#include "../../Main/Render.h"
#include "../../Main/aiRender.h"
#include "../../Main/aiObject.h"
#include "../../Main/SWTexture.h"
#include "../../Main/2DSceneSW.h"
#include "../../Main/MapBuildTerrain.h"
#include "../../Main/TerrainInfo.h"
#include "../../Main/Grid.h"
#include "../../DBFormat/DataFormat.h"
#include "../../DBFormat/DataMap.h"
#include "../../DBFormat/DataRPG.h"
#include "../../DBFormat/DataTerrain.h"
#include "../../Misc/RandomGen.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace UpstreamTests
{
namespace
{
void TerrainSpotTieOrder(const char* databasePath)
{
    CFileStream database;
    database.OpenRead(databasePath);
    NDatabase::Serialize(database, CStructureSaver::READ);
    auto* variant = NDb::GetTemplVariant(810);
    Require(IsValid(variant), "terrain spot fixture lacks its authored variant");

    CObj<NDb::CMaterial> material = new NDb::CMaterial;
    CObj<NDb::CTMaterial> materials = new NDb::CTMaterial;
    materials->variants.push_back(material.GetPtr());
    materials->roulette.AddSector(1.f);
    vector<CObj<NDb::CRPGArmor>> armors;
    vector<CObj<NDb::CSpot>> spots;
    vector<CObj<NDb::CRndTerrainSpot>> authored;
    variant->terrainSpots.clear();
    for (int priority : {7, 3, 7, 3}) {
        CObj<NDb::CRPGArmor> armor = new NDb::CRPGArmor;
        CObj<NDb::CSpot> spot = new NDb::CSpot;
        CObj<NDb::CRndTerrainSpot> spec = new NDb::CRndTerrainSpot;
        spot->pMaterial = materials;
        spot->pArmor = armor;
        spec->pSpot = spot;
        spec->ptPos = CVec2(8.f * FP_GRID_STEP, 8.f * FP_GRID_STEP);
        spec->ptSize = CVec2(4.f * FP_GRID_STEP, 4.f * FP_GRID_STEP);
        spec->nRotation = 0;
        spec->nPriority = priority;
        armors.push_back(armor);
        spots.push_back(spot);
        authored.push_back(spec);
        variant->terrainSpots.push_back(spec.GetPtr());
    }
    STerrainInfo terrain;
    terrain.typeMap.SetSizes(16, 16);
    terrain.typeMap.FillZero();
    SRand random(SRandomSeed(801));
    LoadTerrSpots(810, &terrain, &random, {});
    Require(terrain.spots.size() == 4, "native map build did not resolve all four terrain spots");
    MakeSoundMap(&terrain);
    const auto listener = terrain.GetStepSound(9, 9);
    const size_t expected[] = {3, 1, 2, 0};
    for (size_t index = 0; index < 4; ++index)
        Require(terrain.spots[index].pMaterial.GetPtr() == material.GetPtr() &&
            terrain.spots[index].stepSound.pArmor.GetPtr() == armors[expected[index]].GetPtr(),
            "native terrain spots did not follow retail priority and equal-priority order");
    Require(listener.pArmor.GetPtr() == armors.front().GetPtr(),
        "overlapping terrain spots produced a different footstep surface");
}

static void PackedParticleColor()
{
    NGScene::TParticleKey<DWORD> keys[] = {{0, 0xfe012ba7u}, {8, 0x1fa2f4dbu}};
    const NGScene::TKeyTrack<DWORD> track{2, keys};
    const std::pair<float, DWORD> expected[] = {
        {0.f, 0xfe012ba7u}, {1.f, 0xe21544aeu}, {2.f, 0xc6295db4u},
        {4.f, 0x8e5190c1u}, {6.f, 0x577ac2ceu}, {7.f, 0x3b8edbd5u}, {8.f, 0x1fa2f4dbu},
    };
    for (const auto& sample : expected) {
        DWORD actual = 0;
        track.GetValue(sample.first, &actual);
        Require(actual == sample.second, "particle color sampling did not match the packed channel interpolation");
    }
}

void FixedRasterEdgeReciprocal()
{
    struct RowSink : CRasterizer<RowSink> {
        std::array<std::pair<int, int>, 3> spans = {};
        void ClipVertical(int*, int*, int*) const {}
        void ClipHorizontal(int*, int*) const {}
        bool DoRenderBackface() const { return true; }
        void RasterSpan(int y, int first, int end, float, float, int) {
            if (y >= 51 && y <= 53)
                spans[y - 51] = {first, end};
        }
    } raster;
    // The diagonal crosses the pixel-60 boundary on row 52. Rounding the reciprocal
    // before multiplying changes the 16.16 step from 64270 to 64269.
    raster.RasterNoClip(CVec3(10.262948f, 1.27278805f, 1.f), CVec3(98.4306564f, 91.1779404f, 1.f),
        CVec3(10.262948f, 91.1779404f, 1.f));
    Require(raster.spans[0] == std::make_pair(10, 60) && raster.spans[2] == std::make_pair(10, 61),
        "neighboring control spans do not match the native fixed edge");
    Require(raster.spans[1] == std::make_pair(10, 60),
        "direct division added a pixel to the boundary span");
}

class NativeGenericRasterSource:public CObjectBase
{
    OBJECT_NOCOPY_METHODS(NativeGenericRasterSource);
};
static void NativeGenericRaster(int mode,bool fog)
{
    Require(mode>=0 && mode<8,"unknown native generic raster case");
    const float left[]={2.5f,1.5f,2.f,-1.5f,2.499f,2.501f,.5f,5.5f};
    const float right[]={4.5f,3.5f,4.f,.5f,4.499f,4.501f,2.5f,7.5f};
    const int first[]={3,2,2,0,2,3,1,6},last[]={5,4,4,1,4,5,3,8};
    auto rng=GlobalGameRandom();unsigned mask[8]={};int identity=1,depth=1;unsigned count=0;
    if(fog){
        CArray2D<NGfx::SPixel8888> pixels;pixels.SetSizes(4,4);pixels.FillEvery(NGfx::SPixel8888(255,255,255,255));
        // Use the existing native texture resampler as an input node. The
        // software scene builds/rasterizes its own two actual spot triangles.
        CObj<NGScene::CBilinearTexture> texture=new NGScene::CBilinearTexture(pixels,4,4);
        CObj<NGScene::ISW2DScene> scene=NGScene::Make2DSWScene();
        auto* spot=scene->CreateSpot(texture,nullptr,CVec2(left[mode],2.f),CVec2(right[mode]-left[mode],3.f),0.f);
        Require(spot,"native software scene did not create a real spot");
        CArray2D<int> result;result.SetSizes(8,8);result.FillEvery(255);scene->DrawFog(&result,CTPoint<int>(8,8));
        for(int y=0;y<8;++y)for(int x=0;x<8;++x){Require(result[y][x]==0 || result[y][x]==255,"native fog changed its existing min/intensity rule");if(result[y][x]==0){mask[y]|=1u<<x;++count;}}
    }else{
        CObj<NativeGenericRasterSource> object=new NativeGenericRasterSource;
        // InitParallel's existing +0.499 shift is input framing, not part of
        // this port. Supply projected half-pixel boundaries through world hulls.
        const CVec3 a(left[mode]-.499f,2.f-.499f,1.f),b(right[mode]-.499f,5.f-.499f,3.f);
        vector<CVec3> points={CVec3(a.x,a.y,a.z),CVec3(b.x,a.y,a.z),CVec3(b.x,b.y,a.z),CVec3(a.x,b.y,a.z),
            CVec3(a.x,a.y,b.z),CVec3(b.x,a.y,b.z),CVec3(b.x,b.y,b.z),CVec3(a.x,b.y,b.z)};
        vector<STriangle> faces={STriangle(0,2,1),STriangle(0,3,2),STriangle(4,5,6),STriangle(4,6,7),
            STriangle(0,1,5),STriangle(0,5,4),STriangle(3,7,6),STriangle(3,6,2),STriangle(0,4,7),STriangle(0,7,3),STriangle(1,2,6),STriangle(1,6,5)};
        NAI::CEdgesInfo edges;edges.GenerateEdgeList(faces,points);Require(edges.IsClosed(),"native raster hull is not closed");
        SFBTransform transform;Identity(&transform.forward);Identity(&transform.backward);NAI::SSourceInfo source(object,nullptr,0,0);
        NAI::SConvexHull hull(points,edges,transform,source,17,{});NAI::CFastRenderer renderer;
        renderer.InitParallel(CVec2(0,0),0.f,1.f,CTRect<int>(0,0,7,7));renderer.TraceEntity(hull,false);renderer.SortIntervals();
        Require(renderer.resGrid.GetXSize()==8 && renderer.resGrid.GetYSize()==8,"native interval grid lost inclusive region framing");
        for(int y=0;y<8;++y)for(int x=0;x<8;++x)if(auto* hit=renderer.resGrid[y][x]){
            mask[y]|=1u<<x;++count;identity&=hit->pSrc && hit->pSrc->pSrc==&source && hit->pSrc->nUserID==17 && hit->pNext==nullptr;
            depth&=std::fabs(hit->fEnter-1.f)<.00001f && std::fabs(hit->fExit-3.f)<.00001f;}
    }
    bool coverage=true;unsigned expectedCount=0;for(int y=0;y<8;++y){unsigned expected=0;if(y>=2 && y<5)for(int x=first[mode];x<last[mode];++x){expected|=1u<<x;++expectedCount;}coverage&=mask[y]==expected;}
    auto after=GlobalGameRandom();bool sameRNG=true;for(int i=0;i<4;++i)sameRNG&=rng.Get()==after.Get();
    std::printf("native generic raster mode=%d fog=%d left=%.9g right=%.9g rows=%02x/%02x/%02x/%02x/%02x/%02x/%02x/%02x count=%u expected=%u coverage=%d identity=%d depth=%d RNG=%d\n",mode,int(fog),left[mode],right[mode],mask[0],mask[1],mask[2],mask[3],mask[4],mask[5],mask[6],mask[7],count,expectedCount,int(coverage),identity,depth,int(sameRNG));std::fflush(stdout);
    Require(identity && depth && sameRNG,"generic edge coverage changed native source/depth/RNG state");
    Require(coverage && count==expectedCount,"actual native generic consumer retains float/ties-even edge coverage");
}


static void NativeRasterGuardPlane(int mode)
{
    Require(mode>=0 && mode<20,"unknown native raster guard-plane case");
    const float radii[]={5,100,500,1000,1100,2000,4096,5000,8192,10000,20000,50000};
    const float radius=mode<12?radii[mode]:2000.f;CVec3 a(-radius,-radius,1.f),b(radius,radius,3.f);
    if(mode==12){a.x=1000.f;b.x=2000.f;}if(mode==13){a.x=-2000.f;b.x=-1000.f;}
    if(mode==14){a.y=1000.f;b.y=2000.f;}if(mode==15){a.y=-2000.f;b.y=-1000.f;}
    if(mode==16)a.x=0.f;if(mode==17)b.x=0.f;if(mode==18)a.y=0.f;if(mode==19)b.y=0.f;
    CObj<NativeGenericRasterSource> object=new NativeGenericRasterSource;
    vector<CVec3> points={CVec3(a.x,a.y,a.z),CVec3(b.x,a.y,a.z),CVec3(b.x,b.y,a.z),CVec3(a.x,b.y,a.z),
        CVec3(a.x,a.y,b.z),CVec3(b.x,a.y,b.z),CVec3(b.x,b.y,b.z),CVec3(a.x,b.y,b.z)};
    vector<STriangle> faces={STriangle(0,2,1),STriangle(0,3,2),STriangle(4,5,6),STriangle(4,6,7),
        STriangle(0,1,5),STriangle(0,5,4),STriangle(3,7,6),STriangle(3,6,2),STriangle(0,4,7),STriangle(0,7,3),STriangle(1,2,6),STriangle(1,6,5)};
    NAI::CEdgesInfo edges;edges.GenerateEdgeList(faces,points);Require(edges.IsClosed(),"guard-plane input is not closed native geometry");
    SFBTransform transform;Identity(&transform.forward);Identity(&transform.backward);NAI::SSourceInfo source(object,nullptr,0,0);
    NAI::SConvexHull hull(points,edges,transform,source,17,{});NAI::CFastRenderer renderer;
    auto rng=GlobalGameRandom();renderer.InitProjective(CVec3(0,0,0),CVec3(0,0,4),CVec2(2,2),16);renderer.TraceEntity(hull,false);renderer.SortIntervals();
    Require(renderer.resGrid.GetXSize()==32 && renderer.resGrid.GetYSize()==32,"guard-plane fixture lost native grid framing");
    int expectedCount=0,actualCount=0,missing=0,spurious=0,wrongDepth=0,wrongSource=0;double worst=0;
    // Use actual unchanged native projection directions, then an independent
    // double slab intersection of the authored world box. No interval/raster
    // implementation supplies expected coverage or depths.
    for(int y=0;y<32;++y)for(int x=0;x<32;++x){
        CVec3 direction;renderer.GetDir(&direction,float(x),float(y));Require(direction.z>0,"native projective ray lost forward direction");
        double enterDistance=0,exitDistance=1e30;const double d[]={direction.x,direction.y,direction.z},lo[]={a.x,a.y,a.z},hi[]={b.x,b.y,b.z};
        for(int axis=0;axis<3;++axis){if(std::fabs(d[axis])<1e-20){if(lo[axis]>0 || hi[axis]<0)exitDistance=-1;continue;}
            double first=lo[axis]/d[axis],last=hi[axis]/d[axis];if(first>last)std::swap(first,last);enterDistance=(std::max)(enterDistance,first);exitDistance=(std::min)(exitDistance,last);}
        const bool expected=exitDistance>enterDistance;expectedCount+=int(expected);auto* hit=renderer.resGrid[y][x];actualCount+=int(hit!=nullptr);
        if(expected && !hit)++missing;if(!expected && hit)++spurious;
        if(hit){wrongSource+=int(!hit->pSrc || hit->pSrc->pSrc!=&source || hit->pSrc->nUserID!=17 || hit->pNext!=nullptr);
            if(expected){double error=(std::max)(std::fabs(hit->fEnter-enterDistance),std::fabs(hit->fExit-exitDistance));worst=(std::max)(worst,error);
                wrongDepth+=int(!std::isfinite(hit->fEnter) || !std::isfinite(hit->fExit) || error>0.0002*(std::max)(1.,exitDistance));}}
    }
    auto after=GlobalGameRandom();bool sameRNG=true;for(int i=0;i<4;++i)sameRNG&=rng.Get()==after.Get();
    const int literalExpected=mode<12?1024:mode<16?0:512;
    auto* center=renderer.resGrid[16][16];
    std::printf("native raster guard mode=%d radius=%.9g expected=%d literal=%d actual=%d missing=%d spurious=%d wrongDepth=%d worst=%.9g wrongSource=%d RNG=%d centerEnter=%.9g centerExit=%.9g\n",mode,radius,expectedCount,literalExpected,actualCount,missing,spurious,wrongDepth,worst,wrongSource,int(sameRNG),center?center->fEnter:0.f,center?center->fExit:0.f);std::fflush(stdout);
    Require(expectedCount==literalExpected && sameRNG,"guard-plane fixture lost its independent world-ray/count/RNG premise");
    Require(missing==0 && spurious==0 && wrongDepth==0 && wrongSource==0,"actual native projective intervals overflow without original guard planes");
}
}

bool RunRenderTests(const char* mode, int argc, char** argv)
{
    if (!std::strcmp(mode, "terrain-spot-tie-order") && argc == 3) { TerrainSpotTieOrder(argv[2]); return true; }
    if (!std::strcmp(mode, "packed-particle-color")) { PackedParticleColor(); return true; }
    if (!std::strcmp(mode, "fixed-raster-edge-reciprocal")) { FixedRasterEdgeReciprocal(); return true; }
    if (!std::strcmp(mode, "native-raster-guard-plane")) { Require(argc==3,"guard plane requires mode"); NativeRasterGuardPlane(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "native-generic-raster")) { Require(argc==4,"generic raster requires mode and fog flag"); NativeGenericRaster(std::atoi(argv[2]),std::atoi(argv[3])!=0); return true; }
    return false;
}
}
