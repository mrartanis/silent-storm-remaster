#pragma once
#include "iMain.h"

namespace NGame
{
class CICAutoPlay: public NMainLoop::CInterfaceCommand
{
	OBJECT_BASIC_METHODS(CICAutoPlay)
public:
	CICAutoPlay() {}

	virtual void Exec();
};
}
