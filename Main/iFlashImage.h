#pragma once
#include "Interface.h"

namespace NUI
{
// Shared publisher-logo window; save fields and class ID stay unchanged.
class CFlashImage: public CWindow
{
	OBJECT_BASIC_METHODS(CFlashImage);
private:
	ZDATA_(CWindow)
	float fCoeff;
	STime sMorphTime;
	CPtr<CImage> pActive;
	CPtr<CImage> pBackground;
	ZEND int operator&( CStructureSaver &f ) { f.Add(1,(CWindow*)this); f.Add(2,&fCoeff); f.Add(3,&sMorphTime); f.Add(4,&pActive); f.Add(5,&pBackground); return 0; }

public:
	CFlashImage() {}
	CFlashImage( const SWindowInfo &sInfo );

	bool ProcessMessage( const SEvent &sEvent );
	void Draw( const STime &sTime, NGScene::I2DGameView *pView );
};
}
