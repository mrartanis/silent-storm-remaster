#if defined(_WIN32)
#include "StdAfx.h"
#else
#include "../FileIO/StdAfx.h"
#include "../FileIO/BasicChunk1.h"
#include "../Misc/Geom.h"
#endif
//
#include "aiUnit.h"
#include "wUnitServer.h"
#include "aiUnitState.h"
#include "aistate.h"
//
#include "aiActionBase.h"
//
////////////////////////////////////////////////////////////////////////////////////////////////////
// Release CAICombatLogic substrate - CAIAction base helpers.
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NAI
{
IAIUnit* CAIAction::GetUnit() const { return pUnit; }
//
// @0x13ac0 - the acting unit's AI state, guarded on a live unit (pUnit != 0 && alive).
SAIState* CAIAction::GetAIState() const
{
	if ( IsValid( pUnit ) )
		return pUnit->GetAIState();
	return 0;
}
//
IAIUnit* CAIAction::GetEnemy() const
{
	if ( IsValid( pUnit ) )
	{
		SAIUnitState *pState = pUnit->GetAIUnitState();
		if ( pState != 0 )
			return pState->pEnemy;
	}
	return 0;
}
//
SPlaceWithAP CAIAction::GetCurrentPlace() const
{
	if ( pExecutionPlace )
		return *pExecutionPlace;
	SPlaceWithAP p;
	if ( IsValid( pUnit ) )
	{
		p.place   = pUnit->GetUnitPosition();
		p.nUnitAP = pUnit->GetAP();
	}
	return p;
}
//
void CAIAction::DoFromPlace( CAILog *pLog, const SPlaceWithAP &place )
{
	struct SRestorePlace
	{
		const SPlaceWithAP *&slot;
		const SPlaceWithAP *previous;
		~SRestorePlace() { slot = previous; }
	} restore{ pExecutionPlace, pExecutionPlace };
	pExecutionPlace = &place;
	Do( pLog );
}
//
// Retail 0x414180: PK pilots may only consider actions supported by the suit.
bool CAIAction::CanPerform()
{
	return IsValid( pUnit ) && IsValid( pUnit->GetUnitServer() ) &&
		( !pUnit->GetUnitServer()->IsWearingPK() || CanPerformInPK() );
}

}
