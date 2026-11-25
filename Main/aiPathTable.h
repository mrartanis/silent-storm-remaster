#ifndef __aiPathTable_H_
#define __aiPathTable_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../Misc/RetailHashOrder.h"

namespace NAI 
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPathPlaceTable
{
public:
	typedef SMoveInfo<SPathPlace, WORD> SMove;
	typedef unordered_map<SPathPlace, SMove, SPathPlaceHash> CMovesHash;
	ZDATA
	CMovesHash data;
	ZEND int operator&( CStructureSaver &f ) { f.Add(2,&data); return 0; }
	SMove& operator[]( const SPathPlace &_pt ) { return data[ _pt ]; }
	WORD GetCost( const SPathPlace &pt ) 
	{ 
		unordered_map<SPathPlace, SMove, SPathPlaceHash>::const_iterator i = data.find( pt );
		if ( i == data.end() ) 
			return 65535;
		return (i->second).cost;
	}
	void MakeInfinity(void) { data.clear(); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
// The route wave reuses this workspace. Saved reachable-area tables keep CPathPlaceTable's layout.
class CPathSearchTable : public CPathPlaceTable
{
	NRetail::CIntHashOrder order;
public:
	SMove& operator[]( const SPathPlace &pt )
	{
		auto entry = data.try_emplace( pt );
		if ( entry.second )
			order.Insert( pt.GetData() );
		return entry.first->second;
	}
	void MakeInfinity()
	{
		CPathPlaceTable::MakeInfinity();
		order.Clear();
	}
	const NRetail::CIntHashOrder& GetSearchOrder() const { return order; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif
