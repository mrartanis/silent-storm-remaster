#pragma once

#include "../Misc/Geom.h"
#include "../Misc/Tools.h"

namespace NWorld
{
inline constexpr float F_MINE_GRID_STEP = 0.25f;

inline CVec3 QuantizeMinePosition( const CVec3 &position )
{
	return CVec3(
		Float2Int( position.x / F_MINE_GRID_STEP ) * F_MINE_GRID_STEP,
		Float2Int( position.y / F_MINE_GRID_STEP ) * F_MINE_GRID_STEP,
		Float2Int( position.z / F_MINE_GRID_STEP ) * F_MINE_GRID_STEP );
}

inline bool IsMineTriggerPoint( const CVec3 &minePosition, const CVec3 &position )
{
	const CVec3 delta = QuantizeMinePosition( position ) - QuantizeMinePosition( minePosition );
	return fabs( delta.x ) <= F_MINE_GRID_STEP &&
		fabs( delta.y ) <= F_MINE_GRID_STEP && fabs( delta.z ) <= F_MINE_GRID_STEP;
}
}
