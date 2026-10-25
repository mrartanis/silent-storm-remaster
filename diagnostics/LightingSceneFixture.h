#pragma once
#include "../Main/GScene.h"
#include <stdexcept>

namespace NGScene {
// Only the visibility query is expected; GPU and scene mutations fail explicitly.
class LightingSceneFixture: public IGScene
{
    OBJECT_NOCOPY_METHODS(LightingSceneFixture);
public:
    int traceCalls = 0;
    bool blocked = true;
    CLightGroup* CreateLightGroup() override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, const SFullGroupInfo &_ginfo ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, const SFBTransform &trans, const SFullGroupInfo &_ginfo ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, const SDiscretePos &trans, const SFullGroupInfo &_ginfo ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, CFuncBase<SFBTransform> *pPlacement, const SFullGroupInfo &_ginfo ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* CreateGeometry( CPtrFuncBase<CObjectInfo> *pInfo, IMaterial *pMat, CFuncBase<vector<SHMatrix> > *pPlacement, CFuncBase<vector<NGfx::SCompactTransformer> > *_pMMXAnim, const SFullGroupInfo &_ginfo ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* CreateDynamicGeometry( CPtrFuncBase<CObjectInfo> *pInfo, CFuncBase<SFBTransform> *pPlacement, IMaterial *pMat, CFuncBase<SBound> *pBound, const SFullGroupInfo &_ginfo ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* CreateParticles( CPtrFuncBase<CParticleEffect> *pInfo, CFuncBase<SFBTransform> *pPlacement, const SBound &bound, const SGroupInfo &_ginfo, int nPFlags ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CPolyline* CreatePolyline( CPtrFuncBase<NGfx::CGeometry> *pGeometry, const CVec3 &color ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* CreateSelection( CObjectBase *pRenderNode, const CVec4 &vColor ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* CreatePostProcessor( CObjectBase *pRenderNode, IPostProcess *pProcessor ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    void SetAmbient( const CVec3 &vBottomAmbientColor, const CVec3 &vTopAmbientColor ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* AddDirectionalLight( CFuncBase<CVec3> *pColor, CFuncBase<CVec3> *pGlossColor, const CVec3 &vShadowColor, const CVec3 &ptLight, const CVec3 &ptOrigin, const CVec2 &ptSize, float fMaxHeight, bool bLightmapOnly, float fBlurShift ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* AddPointLight( const CVec3 &_vColor, const CVec3 &ptOrigin, float fR, bool bLightmapOnly ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* AddPointLight( CPtrFuncBase<CAnimLight> *pLight ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* AddSpotLight( CFuncBase<CVec3> *pColor, const CVec3 &ptOrigin, const CVec3 &ptDir, float fFOV, float fRadius, CPtrFuncBase<NGfx::CTexture> *pMask, bool bLightmapOnly ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CDecalTarget* CreateDecalTarget( const vector<CObjectBase*> &targets, const SDecalMappingInfo &_info ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CObjectBase* AddDecal( NGScene::CDecalTarget *pTarget, IMaterial *pMaterial ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    void Draw( CTransformStack *pTS, CTransformStack *pClipTS, NGfx::CRenderContext *pRC, const SGroupSelect &mask, ERenderPath rp, EFogMode fogMode, const SFogParams &fog, EHSRMode hsrMode, ETransparentMode trMode, NGfx::CCubeTexture *pSky ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CFuncBase<CVec3>* GetCamera() override { throw std::runtime_error("unexpected lighting fixture operation"); }
    CVec2 GetScreenRect() override { throw std::runtime_error("unexpected lighting fixture operation"); }
    bool TraceScene(const SGroupSelect&, const CRay&, float* time, CVec3* normal,
        CVec3* color, EScenePartsSet = SPS_STATIC, CObjectBase** = nullptr) override {
        ++traceCalls;
        *time = 0.5f; *normal = CVec3(0, 0, 1); *color = VNULL3;
        return blocked;
    }
    SGroupSelect GetLastMask() override { return MakeSelectAll(); }
    void PrecacheMaterials() override { throw std::runtime_error("unexpected lighting fixture operation"); }
    void SetLightingOptions( int nOptions ) override { throw std::runtime_error("unexpected lighting fixture operation"); }
};
} // namespace NGScene
