#pragma once

#include "TestSupport.h"

namespace UpstreamTests
{
// Native serialized DB input, not a replacement database loader or link builder.
struct NativeContextColumn
{
    string name;int type=0;
    int operator&(CStructureSaver& f){f.Add(2,&name);f.Add(3,&type);return 0;}
};
// An empty relation list has no element-type tag; no private SRelation access.
struct NativeContextEmptyRelation { int operator&(CStructureSaver&) { Require(false,"fixture encodes only an empty relation list"); return 0; } };
}
