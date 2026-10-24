#pragma once
namespace NDb { class CDBPerk; }
namespace NUI {
class CWindow;
class CToolTip;
CToolTip* CreatePerkToolTip(CWindow* parent, NDb::CDBPerk* perk);
}
