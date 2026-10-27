#ifndef __A5_INGAMEMENU_H_
#define __A5_INGAMEMENU_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "iMain.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGame
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class CICInGameMenu: public NMainLoop::CInterfaceCommand
{
	OBJECT_BASIC_METHODS(CICInGameMenu);
private:
	CPtr<NRPG::CGlobalPlayer> pGlobalPlayer;
	// retail threads bAllowRestart into CInGameMenuInterface::Initialize @0x1e9940: the "Restart
	// mission" button follows the saved mission restart permission. Local initialization arms it;
	// chapter/global-map callers and network preparation do not have a local restart snapshot.
	bool bAllowRestart = false;
	bool bAllowSave = true;   // retail: the mission's bCanSave, forwarded to every save/load screen this menu opens

public:
	CICInGameMenu() {}
	CICInGameMenu( NRPG::CGlobalPlayer *pGlobalPlayer, bool bAllowRestart = false, bool bAllowSave = true );   // retail threads the mission's bCanSave down to the save/load screen

	virtual void Exec();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
