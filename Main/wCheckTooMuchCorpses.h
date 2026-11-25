#pragma once

namespace NAI { class IPathNetwork; }

namespace NWorld
{
class CUnitServer;

void CheckTooMuchCorpses( NAI::IPathNetwork *pNetwork, const list< CObj<CUnitServer> > &units );
}
