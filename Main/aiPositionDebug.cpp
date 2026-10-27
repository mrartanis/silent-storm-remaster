#include "StdAfx.h"
#include "wUnitServer.h"   // NWorld::CUnitServer, CDynamicCast, IsValid
#include "aiPosition.h"    // NAI::IsUnitNear, NAI::SUnitPosition, CVec3
#include "aiLocker.h"      // NAI::IsBigLocker

namespace NAI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// aiPositionDebug -- locker-validity probes consumed by CPathNetwork::DebugCheck (retail @0x3f270,
// absent from the dev tree). Reconstructed faithfully from retail NAI::IsLockerUnit @0x4917d0 and
// NAI::IsUnitNear @0x491810. IsLockerUnit also gates runtime path-lock restoration.
// IsUnitNear remains a debug probe; neither helper changes the save format.
////////////////////////////////////////////////////////////////////////////////////////////////////
// Runtime locker eligibility is implemented through CUnit in aiLocker.cpp.
////////////////////////////////////////////////////////////////////////////////////////////////////
// The unit's center (SUnitPosition::GetCP) must lie within 2 grid steps -- 4 for a big locker -- of
// pt, full-3D distance, inclusive, with the retail 0.00625 epsilon: 1.25625 (@0x3fa0cccd) /
// 2.50625 (@0x40206667). IsBigLocker is keyed off the unit (the retail EDI/point aliasing is a
// decompiler artefact), so it re-probes the same unit object.
bool IsUnitNear( CObjectBase *pUnit, const CVec3 &pt )
{
	CDynamicCast<NWorld::CUnitServer> pUS( pUnit );
	if ( !IsValid( pUS ) )
		return false;
	CVec3 cp = pUS->GetPosition().GetCP();
	bool bBigLocker = IsBigLocker( pUnit );
	return fabs( pt - cp ) <= ( bBigLocker ? 2.50625f : 1.25625f );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}
