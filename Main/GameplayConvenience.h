#pragma once
namespace NWorld { class CUnit; }
namespace NRPG { class IWeaponItemInfo; }
namespace NUI {
bool ConvenienceRussian();
wstring WeaponComparisonText(NWorld::CUnit* unit, NRPG::IWeaponItemInfo* candidate, bool details);
}
namespace NGame {
// Local presentation preference: network sessions always retain their own clock.
inline int OtherTurnSpeed(int configured, bool network, bool realtime, bool playerTurn, bool sequence, bool paused) {
    return network || realtime || playerTurn || sequence || paused ? 1 : (configured>=2 && configured<=4 ? configured : 1);
}
}
