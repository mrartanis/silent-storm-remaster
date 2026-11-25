#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace NRetail
{
// Integer-key traversal of the original 32-bit STLport hash table.
// Lookups and values stay in the caller's container.
class CIntHashOrder
{
	static constexpr std::size_t NONE = (std::numeric_limits<std::size_t>::max)();
	struct SEntry
	{
		int key;
		std::size_t next;
	};
	std::vector<std::size_t> buckets;
	std::vector<SEntry> entries;

	static std::size_t GetBucket( int key, std::size_t count )
	{
		return static_cast<std::uint32_t>( key ) % count;
	}

	void Grow()
	{
		static constexpr std::size_t primes[] = {
			29u, 53u, 97u, 193u, 389u, 769u, 1543u, 3079u, 6151u, 12289u, 24593u,
			49157u, 98317u, 196613u, 393241u, 786433u, 1572869u, 3145739u, 6291469u,
			12582917u, 25165843u, 50331653u, 100663319u, 201326611u, 402653189u,
			805306457u, 1610612741u, 3221225473u, 4294967291u
		};
		for ( std::size_t count : primes )
		{
			if ( count <= buckets.size() )
				continue;
			std::vector<std::size_t> next( count, NONE );
			for ( std::size_t entry : buckets )
			{
				while ( entry != NONE )
				{
					auto &value = entries[entry];
					const std::size_t oldNext = value.next;
					auto &head = next[GetBucket( value.key, count )];
					value.next = head;
					head = entry;
					entry = oldNext;
				}
			}
			buckets.swap( next );
			return;
		}
	}

public:
	class CIterator
	{
		const CIntHashOrder *pOrder;
		std::size_t bucket, entry;

		void MoveToBucket()
		{
			while ( bucket < pOrder->buckets.size() && pOrder->buckets[bucket] == NONE )
				++bucket;
			entry = bucket == pOrder->buckets.size() ? NONE : pOrder->buckets[bucket];
		}

	public:
		CIterator( const CIntHashOrder *pOrder, std::size_t bucket ) : pOrder(pOrder), bucket(bucket)
		{
			MoveToBucket();
		}
		int operator*() const { return pOrder->entries[entry].key; }
		bool operator==( const CIterator &other ) const { return pOrder == other.pOrder && entry == other.entry; }
		bool operator!=( const CIterator &other ) const { return !( *this == other ); }
		CIterator& operator++()
		{
			entry = pOrder->entries[entry].next;
			if ( entry == NONE )
			{
				++bucket;
				MoveToBucket();
			}
			return *this;
		}
	};

	CIntHashOrder() : buckets(13, NONE) {}
	void Clear()
	{
		std::fill( buckets.begin(), buckets.end(), NONE );
		entries.clear();
	}
	void Insert( int key )
	{
		if ( entries.size() == buckets.size() )
			Grow();
		auto &head = buckets[GetBucket( key, buckets.size() )];
		entries.push_back( { key, head } );
		head = entries.size() - 1;
	}
	CIterator begin() const { return CIterator( this, 0 ); }
	CIterator end() const { return CIterator( this, buckets.size() ); }
	int GetInsertedKey( std::size_t index ) const { return entries[index].key; }
};
}
