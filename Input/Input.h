#ifndef __INPUT_H__
#define __INPUT_H__
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <cstdint>
#if defined(__ANDROID__)
#include "TouchGestures.h"
#endif
class CDataStream;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NInput
{
typedef std::uint32_t STime;
////////////////////////////////////////////////////////////////////////////////////////////////////
	enum EPOVAxis
	{
		PA_UNKNOWN,
		PA_X,
		PA_Y
	};
	enum EControlType
	{
		CT_KEY,
		CT_POV,
		CT_AXIS,
		CT_TIME,
		CT_LIMAXIS,
		CT_UNKNOWN,
		// Historical UI event names retained for compatibility. SDL text supplies
		// CT_WIN_CHAR; SDL key-down/repeat supplies CT_WIN_KEY in the UI's virtual-key
		// numbering. Physical CT_KEY edges remain separate from text and repeat.
		CT_WIN_CHAR,
		CT_WIN_KEY,
		CT_POINTER, // absolute position, including sub-pixel motion without an axis delta
#if defined(__ANDROID__)
		CT_TOUCH_CAMERA,
		CT_TOUCH_CONTEXT,
		CT_TOUCH_SKIP,
#endif
	};
////////////////////////////////////////////////////////////////////////////////////////////////////
	struct SMessage
	{
		int nAction;
		EPOVAxis ePOVAxis;
		EControlType cType;

		int nParam;
		bool bState;
		STime tTime;
		bool hasPointer = false;
		float pointerX = 0, pointerY = 0;
#if defined(__ANDROID__)
		TouchGestures::Gesture touchGesture;
		const void* touchSkipTarget = nullptr;
#endif
	};
////////////////////////////////////////////////////////////////////////////////////////////////////
	bool InitInput( bool bNonExclusiveMode = false, int nSampleBufferSize = -1 );
	bool DoneInput();
#if defined(__ANDROID__)
	void SetTouchMapHitTest(bool (*hitTest)(float x, float y));
	void SetTouchSkipTarget(const void* (*getTarget)());
	std::optional<TouchGestures::SkipProgress> GetTouchSkipProgress();
#endif

	void PumpMessages( bool bFocus );
	// Explicit queue draining must also poll when the normal frame clock has not advanced.
	void PumpMessages( bool bFocus, bool bForce );
	bool GetMessage( SMessage *pMsg );
	// Compatibility hook for synthesized UI events; the game uses SDL events.
	void AddWinMessage( EControlType cType, int nParam );
	bool GetKeyForMessage( const SMessage &mMsg, int *pnVirtualKey );
	// Physical keyboard edges offered to UI controls, excluding Escape.
	bool IsDiscardableKeyboardMessage( const SMessage &message );
		
	int GetControlID( const string &sCommand );
	void GetControlInfo( int nAction, EControlType *pcType, float *pfGranularity );

	void StartSaveInput( CDataStream *pStream );
	void StopSaveInput();
	void StartEmulateInput( CDataStream *pStream );
	void StopEmulateInput();
////////////////////////////////////////////////////////////////////////////////////////////////////
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
