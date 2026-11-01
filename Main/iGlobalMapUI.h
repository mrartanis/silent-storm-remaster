#ifndef __A5_GLOBALMAP_UI_H__
#define __A5_GLOBALMAP_UI_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "iDesktopWindow.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NGScene
{
	class ILight;
	class IGameView;
}
namespace NGame
{
	class IMission;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NUI
{
class CFlashButton;
class CGlobalSector;
////////////////////////////////////////////////////////////////////////////////////////////////////
// CGlobalMapUI
////////////////////////////////////////////////////////////////////////////////////////////////////
class CGlobalMapUI: public CDesktopWindow
{
	OBJECT_NOCOPY_METHODS(CGlobalMapUI);
private:
	ZDATA_(CDesktopWindow)
	CPtr<NGame::IMission> pGlobal;
	////
	SCursorInfo sCursor;
	////
	CObj<CGlobalInfo> pInfo;
	////
	CObj<CImage> pBackground;
	CObj<CWindow> pMapView;
	CObj<CFlashButton> pReturn;
	CObj<CFlashButton> pBaseZone;
	vector<CObj<CGlobalSector> > sectorsSet;
	// Read the remaster's historical flat CWindow chunk as well as the native
	// desktop base. Keep every outer field ID and the existing class registration.
	struct SCompatibleBase
	{
		CGlobalMapUI *pWindow;
		int operator&( CStructureSaver &f )
		{
			if ( f.IsReading() && !f.HasChunk( 1 ) )
				return pWindow->CWindow::operator&( f );
			return pWindow->CDesktopWindow::operator&( f );
		}
	};
	ZEND int operator&( CStructureSaver &f ) { SCompatibleBase base = { this }; f.Add(1,&base); f.Add(2,&pGlobal); f.Add(3,&sCursor); f.Add(4,&pInfo); f.Add(5,&pBackground); f.Add(6,&pMapView); f.Add(7,&pReturn); f.Add(8,&pBaseZone); f.Add(9,&sectorsSet); return 0; }

protected:
	void GetGlobalSectorInfo( const SGlobalSector &sSector, bool *pbVisible, bool *pRecommended );

public:
	CGlobalMapUI() {}
	CGlobalMapUI( const SWindowInfo &sInfo, NGame::IMission *pGlobal );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
////////////////////////////////////////////////////////////////////////////////////////////////////
} // Namespace
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
