#pragma once
namespace NGame { class IMission; }
namespace NRPG { class IWeaponItemInfo; }
namespace NUI {
// A mode ordinal, -1 for mixed modes, or -2 when unavailable.
int GetUnitsGroupWeaponMode(NGame::IMission* mission);
// The single selected unit's effective weapon; null for other selections.
NRPG::IWeaponItemInfo* GetSelectedWeaponForIcons(NGame::IMission* mission);
}
