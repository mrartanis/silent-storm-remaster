#pragma once

#include "TestSupport.h"

namespace NRPG { class CCoverInfo; }

namespace UpstreamTests
{
// Read the existing saved cover fields through their native serializer. Compare
// named ray values, not padding bytes or a new production cover representation.
struct CoverParitySnapshot
{
    struct Ray {CVec3 direction;bool penetrate;float deviation;};
    vector<Ray> hits,loose,obstacles;
    CVec3 source;
    float piercing=0;
};
inline CoverParitySnapshot CoverSnapshot(const CObj<NRPG::CCoverInfo>& cover)
{
    CMemoryStream fields;{CStructureSaver writer(fields,CStructureSaver::WRITE);cover.GetBarePtr()->operator&(writer);}
    fields.Seek(0);CoverParitySnapshot snapshot;
    {CStructureSaver reader(fields,CStructureSaver::READ);reader.Add(2,&snapshot.hits);reader.Add(3,&snapshot.source);
     reader.Add(4,&snapshot.loose);reader.Add(5,&snapshot.obstacles);reader.Add(6,&snapshot.piercing);}
    return snapshot;
}
}
