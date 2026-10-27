#include "StdAfx.h"
#include "Interface.h"
#include "iCommonUI.h"
#include "iMission.h"
#include "iPlayerSwitch.h"
#include "../DBFormat/DataFormat.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// Physical key edges use the same event contract as the SDL-backed interface.
const int EVENT_PS_KEYPRESS = EVENT_CHAR;
////////////////////////////////////////////////////////////////////////////////////////////////////
CPlayerSwitchUI::CPlayerSwitchUI( const SWindowInfo &sInfo, NGame::IMission *pMission ):
	CWindow( sInfo ), bPause( false ), pMission( pMission )
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPlayerSwitchUI::OnOK()
{
	// Restore the pause state that was in effect before the banner appeared, then hide the banner.
	pMission->PauseGame( bPause );
	SetStyle( STYLE_VISIBLE, false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CPlayerSwitchUI::Show()
{
	CInterface *pRoot = GetInterface();
	if ( IsValid( pRoot ) && ( GetParent() != pRoot || GetSize().x <= 0 || GetSize().y <= 0 ||
		!IsValid( pText ) || !IsValid( pOk ) ) )
	{
		// Older fork saves retained a zero-sized HUD child. Repair it after loading,
		// when all graph references exist; retain mission ownership while reparenting.
		CMObj<CPlayerSwitchUI> keepAlive( this );
		if ( IsValid( GetParent() ) )
			GetParent()->RemoveChild( this );
		pRoot->AddChild( this );
		LoadTemplate( this, NDb::GetUIContainer( 453 ) );
		SetStyle( STYLE_ENABLED | STYLE_TOPMOST, true );
	}
	ShowWindow( SWTYPE_SHOW );
	// Title = localized prefix (DB string 0x51F3) + the active player's name.
	pText->SetText( GetDBString( 0x51F3 ) + pMission->GetActivePlayer()->GetPlayer()->GetPlayerName() );
	// Remember the current pause state, then freeze the game while the banner is up.
	bPause = pMission->IsGamePaused();
	pMission->PauseGame( true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlayerSwitchUI::ProcessMessage( const SEvent &sEvent )
{
	switch( sEvent.nEvent )
	{
	case EVENT_TEMPLATELOAD:
		{
			pText = new CText( sEvent.pLoader->GetControl( "text" ) );

			pOk = new CHoverButton( sEvent.pLoader->GetControl( "ok" ) );
			pOk->AddTextState( CHoverButton::STATE_HOVER,  GetDBString( 0x41C7 ) + GetDBString( 0x2B88 ) );
			pOk->AddTextState( CHoverButton::STATE_NORMAL, GetDBString( 0x41C7 ) + GetDBString( 0x2B87 ) );
			break;
		}
	case EVENT_PS_KEYPRESS:
		// ORIGINAL BUG (confirmed @0x22e5b0): returns true for EVERY key, swallowing all input while up;
		// only Enter actually accepts.
		if ( sEvent.nVal == VK_RETURN )
			OnOK();
		return true;
	case EVENT_NOTIFY:
		// ORIGINAL BUG (confirmed @0x22e5b0): ANY notification is treated as the OK click (no szID test).
		OnOK();
		return true;
	}

	return CWindow::ProcessMessage( sEvent );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace NUI
////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace NUI;
REGISTER_SAVELOAD_CLASS( 0xB3723130, CPlayerSwitchUI );
