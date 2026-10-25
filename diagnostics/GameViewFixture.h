#pragma once
#include "../Main/GView.h"
#include <stdexcept>

namespace NGScene {
// UI geometry queries are allowed; rendering and resource creation fail explicitly.
class GameViewFixture: public IGameView
{
    OBJECT_NOCOPY_METHODS(GameViewFixture);
public:
    IGScene* GetGScene() override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CLightGroup* CreateLightGroup() override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateMesh( NDb::CModel *pModel, CFuncBase<SFBTransform> *pPlacement, const SFullRoomInfo &_g = SFullRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateMesh( CMemObject *pModel, const CVec4 &color, CFuncBase<SFBTransform> *pPlacement, const SFullRoomInfo &_g = SFullRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateMesh( NDb::CModel *pModel, const SFBTransform &placement, const SFullRoomInfo &_g = SFullRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateMesh( CMemObject *pModel, const CVec4 &color, const SFBTransform &placement, const SFullRoomInfo &_g = SFullRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateSkin( NDb::CModel *pModel, CFuncBase<NAnimation::SSkeletonPose> *pAnimation, const SFullRoomInfo &_g = SFullRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateOccluder( NDb::CAIGeometry *pGeom, const SFBTransform &placement, const SRoomInfo &_g = SRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateOccluder( NDb::CAIGeometry *pGeom, NDb::CSkeleton *pSkeleton, CFuncBase<NAnimation::SSkeletonPose> *pAnimation, const SRoomInfo &_g = SRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateTerrainRegion( CFuncBase<STerrainInfo> *pInfo, CVersioningBase *pUpdateRegion, const SRandomSeed &sSeed, const CTRect<int> &sRegion, const list<CObj<CPtrFuncBase<CTerrainPart> > > &partsList, NGScene::CGrassTracker *pGrass, const SFullRoomInfo &_g = SFullRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateTerrainWall( CPtrFuncBase<CTerrainPart> *pPart, NDb::CTexture *pTexture, const SFullRoomInfo &_g = SFullRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateGrassSector( CGrassAnimator *pEffect, NDb::CTexture *pTexture, CFuncBase<SFBTransform> *pPlacement, const SBound &bound, const SRoomInfo &_g = SRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateSelection( const vector<CObjectBase*> &target, const CVec4 &vColor ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateParticles( NDb::CEffect *pEffect, STime stBeginTime, CFuncBase<STime> *pTime, CFuncBase<SFBTransform> *pPlacement, const SRoomInfo &_g = SRoomInfo(), NAnimation::CSkeletonAnimator *pScAnim = 0, IParticleFilter *pFilter = 0 ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateParticles( NDb::CEffect *pEffect, STime stBeginTime, CFuncBase<STime> *pTime, const SFBTransform &place, const SRoomInfo &_g = SRoomInfo(), IParticleFilter *pFilter = 0 ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateRain( NDb::CParticleInstance *pInstance, CFuncBase<STime> *pTime, IParticleFilter *pFilter, const SRoomInfo &_g = SRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CBuilding* CreateBuildingPart( int nPartID, const SMapBuilding &info, NBuilding::CBuildingInfoHold *pBI ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CPolyline* CreatePolyline( const vector<CVec3> &points, const CVec3 &color ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateExplosion( CFuncBase<STime> *pTime, NDb::CEffect *pEffect, CFuncBase<CExplosionInfo> *pExplosion, const CVec3 &pos, const SRoomInfo &_g = SRoomInfo() ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* CreateLSHead( NDb::CComplexHead *pHead, CPtrFuncBase<NGScene::CObjectInfo> *pAnimator, CFuncBase<STime> *pTime, CFuncBase<SFBTransform> *pPlacement, bool bInterface, const SRandomSeed &headSeed, bool bHasCap = false, const SRoomInfo &_g = SRoomInfo(), CPtrFuncBase<NGfx::CTexture> *pFaceTexture = 0, const NLSHead::CHeadInfo *pHeadInfo = 0 ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* Precache( NDb::CModel *pModel ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void StartAlienStyle() override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void FinishAlienStyle() override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void Draw( const SDrawInfo &drawInfo ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CVec2 GetScreenRect() override { return CVec2(1024, 768); }
    int  GetCutFloor() override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void SetCutFloor( int nFloor ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void SetCutFloorRange( int nMinFloor, int nMaxFloor ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void GetCutFloorRange( int *pMinFloor, int *pMaxFloor ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void SetCutFloorLock( bool bLock ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    bool GetParticleShow() override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void SetParticleShow( bool bNewState ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void SetAmbient( const CVec3 &vBottomAmbientColor, const CVec3 &vTopAmbientColor ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* AddDirectionalLight( const CVec3 &ptColor, const CVec3 &ptLight, const CVec3 &ptOrigin, const CVec2 &ptSize, float fMaxHeight, bool bLightmapOnly = false ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* AddPointLight( const CVec3 &ptColor, const CVec3 &ptOrigin, float fR, bool bLightmapOnly ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* AddFlare( CFuncBase<CVec3> *pOrigin, CFuncBase<STime> *pTime, int nFloor, float fFlareRadius, NDb::CTexture *pFlareTexture, float fOnTime, float fOffTime ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* AddPostFilter( const vector<CObjectBase*> &target, IPostProcess *pEffect ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* AddSpotLight( const CVec3 &ptColor, const CVec3 &ptOrigin, const CVec3 &ptDir, float fFOV, float fRadius, NDb::CTexture *pMask, bool bLightmapOnly = false ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CDecalTarget* CreateDecalTarget( const vector<CObjectBase*> &targets, const SDecalMappingInfo &_info ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    CObjectBase* AddDecal( NGScene::CDecalTarget *pTarget, NDb::CMaterial *pMaterial ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void SetAmbient( NDb::CAmbientLightReal *pLight, ELightMode lm = LT_ZONE ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    ESceneRenderMode GetRenderMode() const override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void SetRenderMode( ESceneRenderMode mode ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    EFogMode GetFogMode() const override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void SetFogMode( EFogMode mode ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void SetHSRMode( EHSRMode m ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    EHSRMode GetHSRMode() const override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void SetTransparentMode( ETransparentMode m ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    ETransparentMode GetTransparentMode() const override { throw std::runtime_error("unexpected icon fixture render operation"); }
    bool TraceScene( const CRay &r, float *pfT, CVec3 *pNormal, CVec3 *pColor, EScenePartsSet ps = SPS_STATIC, CObjectBase **ppObject = 0 ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
    void SetFogBaseHeight( float f ) override { throw std::runtime_error("unexpected icon fixture render operation"); }
};
} // namespace NGScene
