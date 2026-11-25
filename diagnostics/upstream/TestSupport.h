#pragma once

#if defined(_WIN32)
#include "../../Main/StdAfx.h"
#else
#include "../../FileIO/StdAfx.h"
#include "../../FileIO/BasicChunk1.h"
#include "../../Misc/Geom.h"
#endif

namespace NRPG { class CGlobalGame; class CUnit; }
namespace NWorld { class CWorld; class CUnitServer; }

namespace UpstreamTests
{
void Require(bool value, const char* message);
CMemoryStream BorrowedObjectFields(CObjectBase* object, CStructureNetworkContext* references);
void RestoreSinglePerk(NRPG::CUnit* unit, int id);
CObj<NWorld::CWorld> FixtureWorld(const char* databasePath, const char* resourcePath,
    bool deployParty = false, CObj<NRPG::CGlobalGame>* retainedGame = nullptr, int variantID = 810,
    NWorld::CWorld* (*factory)(NRPG::CGlobalGame*) = nullptr);
NWorld::CUnitServer* MovementPolicyActor(NWorld::CWorld* world);

bool RunAITests(const char* mode, int argc, char** argv);
bool RunInventoryTests(const char* mode, int argc, char** argv);
bool RunNavigationTests(const char* mode);
bool RunUITests(const char* mode, int argc, char** argv);
bool RunBuildingTests(const char* mode, int argc, char** argv);
bool RunDatabaseTests(const char* mode, int argc, char** argv);
bool RunRenderTests(const char* mode, int argc, char** argv);
bool RunUnitTests(const char* mode, int argc, char** argv);
bool RunProjectileTests(const char* mode, int argc, char** argv);
bool RunLegacyTests(const char* mode, int argc, char** argv);
}
