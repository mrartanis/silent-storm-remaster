#ifndef __RPGCOVER_H_
#define __RPGCOVER_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
// Original per-shot cover queries operate on the native hit/loose ray cells.
// CCoverInfo preserves the legacy 20-byte saved ray payload while retaining
// grid ownership and selected-cell coordinates only at runtime.
// CCoverGridInfo remains the earlier unregistered transient helper carrier;
// it is not a substitute saved class and does not add a class-registry entry.
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "RPGBullet.h"   // NRPG::SAttackRayInfo, NRPG::STrailPoint (via RPGGame.h), NAI::CFastRenderer::SResult
                         // + NAI::SSourceInfo (via aiRender.h->aiInterval.h), NAI::IAIMap (fwd-declared there)
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NRPG
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// The shipped grid-bearing cover object (release NRPG::CCoverInfo layout). Field IDENTITY mirrors
// the release class; exact byte offsets are NOT reproduced (purely transient runtime state).
struct CCoverGridInfo
{
	struct SRay
	{
		CVec3 ptDir;			// ray direction
		bool  isPenetrate;
		float fDeviation;		// squared projection onto the target direction
		float fResMaxEnter;		// release-new: nearest blocker enter distance on this ray
		int   nGrid;			// release-new: which grid this ray came from (0=hi-res, 1=lo-res)
		int   x, y;				// release-new: that grid's cell coordinates
	};

	NAI::CFastRenderer grids[2];	// [0]=hi-res, [1]=lo-res rasterizer grids (kept, not discarded)
	vector<SRay> hitRays;
	CVec3 src;
	vector<SRay> looseRays;
	vector<SRay> obstRays;
	float fSummAPA;
	float fMinClearDistance;	// release-new
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// @0x294300 -- collect every distinct source object lying on a hit-or-loose ray into *pRes (an
// unordered_map used as a set: value 1). For each ray, re-walk its source grid cell's SResult
// depth-interval list and add the interval's source object.
void GetObjectsThatMayBeDamaged( CCoverGridInfo *pCover,
	unordered_map<CPtr<CObjectBase>, int, SPtrHash> *pRes );
// Live cover overload: full hit/loose-cell union; saved fields remain unchanged.
void GetObjectsThatMayBeDamaged( CCoverInfo *pCover,
	unordered_map<CPtr<CObjectBase>, int, SPtrHash> *pRes );
// @0x293910 -- index a finished ray back into its grid cell, then hand that cell's SResult list to
// the per-ray GetHitIntersections overload (RPGBullet.obj @0x2915d0). The cell indexing is
// reconstructed faithfully; the per-ray overload builds the cached hit trail.
void GetHitIntersections( NAI::IAIMap *pMap, vector<STrailPoint> *pTrail,
	CCoverGridInfo *pCover, const CCoverGridInfo::SRay *pRay, SAttackRayInfo *pInfo );
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace NRPG
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
