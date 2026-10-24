#pragma once
struct SRand;
namespace NDb { class CRPGArmor; class CPanzerklein; class CSound; }
namespace NWorld {
NDb::CSound* SelectUnitStepSound(NDb::CRPGArmor* armor, NDb::CPanzerklein* panzerklein, SRand* random);
}
