#ifndef __WMAINTRACE_H_
#define __WMAINTRACE_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "wInterface.h"
#include "aiPosition.h"
namespace NAI
{
	class IAIMap;
}
namespace NWorld
{
	// CMission stores this cursor result in save tag 21.
	struct STraceResult
	{
		bool bPointSet;
		CVec3 vPoint;
		bool bTileSet;
		NAI::SPosition sTile;
		bool bObjectSet;
		CPtr<CObjectBase> pObject;

		STraceResult(): bPointSet( false ), vPoint( 0, 0, 0 ), bTileSet( false ), bObjectSet( false ) {}
		int operator&( CStructureSaver &f ) { f.Add(2,&bPointSet); f.Add(3,&vPoint); f.Add(4,&bTileSet); f.Add(5,&sTile); f.Add(6,&bObjectSet); f.Add(7,&pObject); return 0; }
	};

	void Trace( IWorld *pWorld, IPlayer *pPlayer, const CRay &ray, int nMaxFloor, STraceResult *pRes, bool bModifier );
	bool TraceRay( IWorld *pWorld, const CRay &ray, int nMaxFloor, CObjectBase **ppUserData, int *pUserID, CVec3 *pPoint );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
