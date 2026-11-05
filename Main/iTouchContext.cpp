#include "StdAfx.h"
#include "Sync.h"
#include "Gfx.h"
#include "RectLayout.h"
#include "../DBFormat/DataMap.h"
#include "../MiscDll/Commands.h"
#include "iMission.h"
#include "iGameStates.h"
#include "iMissionInternal.h"
#include "iMissionUI.h"
#include "UIWrap.h"
#include "DisplayLayout.h"
#include "RPGGame.h"

namespace NGame
{
namespace
{
enum class TouchAction { BodyPart, Fire, Use, Open, Close, PickUp, Disarm, Cancel };
using NUI::SPoint;
using NUI::SRect;

bool HasKnownTrap(IMission& mission, CObjectBase* target)
{
	list<CPtr<CObjectBase>> traps;
	mission.GetActivePlayer()->GetPlayer()->GetTrappedObjectsList(&traps);
	return find(traps.begin(), traps.end(), target) != traps.end();
}

bool IsFirearm(NWorld::CUnit& unit)
{
	switch (unit.GetState())
	{
	case NWorld::CUnit::ST_NORMAL_PISTOL:
	case NWorld::CUnit::ST_NORMAL_RIFLE:
	case NWorld::CUnit::ST_NORMAL_SUB_MACHINE_GUN:
	case NWorld::CUnit::ST_NORMAL_HAND_MACHINE_GUN:
	case NWorld::CUnit::ST_MACHINE_GUN:
	case NWorld::CUnit::ST_SNIPE:
		return true;
	default:
		return false;
	}
}

class CTouchContextPanel : public NUI::CWindow
{
	OBJECT_NOCOPY_METHODS(CTouchContextPanel)

	struct SButton
	{
		SRect rect;
		TouchAction action;
		NAI::EHitLocation location;
		wstring caption;
		CObj<NUI::CTextDraw> text;
		bool enabled = true;
	};

	bool russian = false;
	bool aiming = false;
	CPtr<NWorld::CUnit> shooter;
	CPtr<CObjectBase> target;
	NAI::EHitLocation location = NAI::HL_BODY;
	vector<SButton> buttons;
	CObj<NUI::CTextDraw> title, details;

	const wchar_t* Text(const wchar_t* en, const wchar_t* ru) const { return russian ? ru : en; }
	void Center() { SetPosition(SPoint((S2UI::Width() - GetSize().x) / 2, (S2UI::Height() - GetSize().y) / 2)); }

	void AddButton(const SRect& rect, TouchAction action, const wchar_t* caption,
		NAI::EHitLocation part = NAI::HL_ANY)
	{
		buttons.push_back({rect, action, part, caption, new NUI::CTextDraw()});
	}

	NWorld::CCmd* MakeCommand(IMission& mission, TouchAction action) const
	{
		if (!IsValid(target))
			return nullptr;
		switch (action)
		{
		case TouchAction::Fire:
			return new NWorld::CCmdShootObject(target, 0, location);
		case TouchAction::Use:
			return CStateUse::MakeTargetCommand(target);
		case TouchAction::Open:
		case TouchAction::Close:
			{
				const auto* door = dynamic_cast<const NWorld::IWindowDoor*>(target.GetPtr());
				return door && door->IsOpen() == (action == TouchAction::Close)
					? CStateUse::MakeTargetCommand(target) : nullptr;
			}
		case TouchAction::PickUp:
			return CStatePickItem::MakeTargetCommand(&mission, target);
		case TouchAction::Disarm:
			return HasKnownTrap(mission, target) ? new NWorld::CCmdUntrapObject(target) : nullptr;
		default:
			return nullptr;
		}
	}

	static bool CanExecute(TouchAction action, const SActionInfo& info)
	{
		if (action == TouchAction::Fire)
			return info.eResult == NWorld::UCR_OK;
		return info.bOk && info.bEnoughAPToStart;
	}

	wstring Cost(const SActionInfo& info) const
	{
		return info.nMinAP >= 0
			? std::to_wstring(info.nMinAP) + Text(L" AP", L" ОД")
			: Text(L"— AP", L"— ОД");
	}

	void Refresh(IMission& mission)
	{
		for (SButton& button : buttons)
		{
			wstring caption = button.caption;
			if (button.action != TouchAction::Cancel && button.action != TouchAction::BodyPart)
			{
				SActionInfo info;
				CObj<NWorld::CCmd> cmd = MakeCommand(mission, button.action);
				if (IsValid(cmd))
					mission.CanDoCommand(cmd, false, &info);
				button.enabled = CanExecute(button.action, info);
				if (button.action == TouchAction::Fire)
				{
					const int chance = info.bOk ? mission.GetWorld()->GetGame()->GetCompositeToHit(
						shooter, target, location, false) : -1;
					const wstring hit = chance >= 0 ? std::to_wstring(chance) + L"%" : L"—";
					SetCaption(details, Text(L"Hit chance  ", L"Попадание  ") + hit + L"   |   " + Cost(info),
						info.bOk ? L"white" : L"#bcb6a5");
				}
				else if (info.nMinAP >= 0)
					caption += L"  ·  " + Cost(info);
				else if (info.eResult == NWorld::UCR_DOOR_LOCKED)
					caption += Text(L"  ·  Locked", L"  ·  Заперто");
			}
			SetCaption(button.text, caption, button.enabled ? L"white" : L"#bcb6a5");
		}
	}

	static void SetCaption(NUI::CTextDraw* text, const wstring& caption, const wchar_t* color)
	{
		const wstring markup = wstring(L"<font face=Arial size=23pt><center><color=") + color + L">" + caption;
		if (text->GetText() != markup)
			text->SetText(markup);
	}

	void DrawCaption(NUI::CTextDraw* text, const SRect& rect, const STime& time, NGScene::I2DGameView* view)
	{
		text->SetSize(SPoint(rect.Width(), -1));
		text->SetPosition(SPoint(rect.x1, rect.y1 + (rect.Height() - text->GetSize(view).y) / 2));
		text->Draw(this, time, view);
	}

public:
	CTouchContextPanel() = default;
	CTouchContextPanel(IMission& mission, NWorld::CUnit* unit, CObjectBase* object, bool aim)
		: CWindow(NUI::SWindowInfo(nullptr, SPoint(0, 0), SPoint(696, 370), "touchContext", NUI::STYLE_VISIBLE)),
		  aiming(aim), shooter(unit), target(object),
		  title(new NUI::CTextDraw()), details(new NUI::CTextDraw())
	{
		for (wchar_t c : NUI::GetDBString(4382))
			russian = russian || (c >= 0x400 && c <= 0x4ff);
		SetCaption(title, Text(L"Aim point", L"Точка прицеливания"), L"white");
		if (aiming)
		{
			AddButton(SRect(136, 22, 246, 91), TouchAction::BodyPart, Text(L"Head", L"Голова"), NAI::HL_HEAD);
			AddButton(SRect(120, 97, 262, 188), TouchAction::BodyPart, Text(L"Torso", L"Корпус"), NAI::HL_BODY);
			// The target faces the viewer, so its left is on the screen's right.
			AddButton(SRect(16, 97, 114, 188), TouchAction::BodyPart, Text(L"R. arm", L"П. рука"), NAI::HL_RHAND);
			AddButton(SRect(268, 97, 366, 188), TouchAction::BodyPart, Text(L"L. arm", L"Л. рука"), NAI::HL_LHAND);
			AddButton(SRect(86, 194, 188, 348), TouchAction::BodyPart, Text(L"R. leg", L"П. нога"), NAI::HL_RLEG);
			AddButton(SRect(194, 194, 296, 348), TouchAction::BodyPart, Text(L"L. leg", L"Л. нога"), NAI::HL_LLEG);
			AddButton(SRect(392, 178, 678, 251), TouchAction::Fire, Text(L"Fire", L"Огонь"));
			AddButton(SRect(392, 270, 678, 340), TouchAction::Cancel, Text(L"Cancel", L"Отмена"));
		}
		else
		{
			SetCaption(title, Text(L"Actions", L"Действия"), L"white");
			int top = 76;
			auto addAction = [&](TouchAction action, const wchar_t* caption) {
				AddButton(SRect(16, top, 432, top + 64), action, caption);
				top += 76;
			};
			if (HasKnownTrap(mission, target))
				addAction(TouchAction::Disarm, Text(L"Disarm trap", L"Обезвредить"));
			CObj<NWorld::CCmd> pickup = CStatePickItem::MakeTargetCommand(&mission, target);
			CObj<NWorld::CCmd> use = CStateUse::MakeTargetCommand(target);
			if (IsValid(pickup))
				addAction(TouchAction::PickUp, Text(L"Pick up", L"Подобрать"));
			if (IsValid(use))
			{
				const auto* door = dynamic_cast<const NWorld::IWindowDoor*>(object);
				if (door && door->IsOpen())
					addAction(TouchAction::Close, Text(L"Close", L"Закрыть"));
				else if (door)
					addAction(TouchAction::Open, Text(L"Open", L"Открыть"));
				else
					addAction(TouchAction::Use, Text(L"Use", L"Использовать"));
			}
			if (buttons.empty())
				return;
			addAction(TouchAction::Cancel, Text(L"Cancel", L"Отмена"));
			SetSize(SPoint(448, top + 4));
		}
		Center();
		Refresh(mission);
	}

	bool HasActions() const { return !buttons.empty(); }

	bool Tap(IMission& mission, const SPoint& screenPoint)
	{
		Center();
		const SPoint point = screenPoint - GetPosition();
		for (const auto& button : buttons)
		{
			const auto& rect = button.rect;
			if (point.x < rect.x1 || point.x >= rect.x2 || point.y < rect.y1 || point.y >= rect.y2)
				continue;
			if (button.action == TouchAction::Cancel)
				return true;
			if (button.action == TouchAction::BodyPart)
			{
				location = button.location;
				Refresh(mission);
				return false;
			}
			vector<CPtr<IUnitTracker>> units;
			mission.GetSelectedUnits(&units);
			if (!IsValid(shooter) || units.size() != 1 || units.front()->GetUnit() != shooter || !IsValid(target))
				return true;
			if (aiming)
			{
				auto* enemy = dynamic_cast<NWorld::CUnit*>(target.GetPtr());
				list<CPtr<NWorld::CUnit>> visible;
				mission.GetActivePlayer()->GetPlayer()->GetVisible(&visible);
				if (!enemy || enemy->IsDead() || enemy->IsUnconscious() || !IsFirearm(*shooter) ||
					mission.GetActivePlayer()->GetUnitDiplomacy(enemy) != NDb::DS_ENEMY ||
					find(visible.begin(), visible.end(), enemy) == visible.end())
					return true;
			}
			if (!button.enabled)
			{
				Refresh(mission);
				return false;
			}
			CObj<NWorld::CCmd> cmd = MakeCommand(mission, button.action);
			if (!IsValid(cmd))
				return true;
			SActionInfo info;
			mission.CanDoCommand(cmd, false, &info);
			if (!CanExecute(button.action, info))
			{
				Refresh(mission);
				return false;
			}
			mission.Command(shooter, cmd);
			return true;
		}
		return point.x < 0 || point.y < 0 || point.x >= GetSize().x || point.y >= GetSize().y;
	}

	void Draw(const STime& time, NGScene::I2DGameView* view) override
	{
		Center();
		SPoint origin;
		SRect window;
		if (!ClientToScreen(&origin, &window))
			return;
		CTPoint<float> pos;
		CTRect<float> clip;
		VirtualToScreen(&origin, &pos);
		VirtualToScreen(&window, &clip);
		const float scale = S2UI::Scale();
		CRectLayout layout;
		const CTRect<float> uv(0, 0, 1, 1);
		auto fill = [&](const SRect& rect, const NGfx::SPixel8888& color) {
			layout.AddRect(rect.x1 * scale, rect.y1 * scale, rect.Width() * scale, rect.Height() * scale, uv, color);
		};
		fill(SRect(0, 0, GetSize().x, GetSize().y), NGfx::SPixel8888(151, 121, 79, 255));
		fill(SRect(2, 2, GetSize().x - 2, GetSize().y - 2), NGfx::SPixel8888(33, 31, 28, 252));
		for (const auto& button : buttons)
		{
			const bool selected = button.action == TouchAction::BodyPart && button.location == location;
			const bool fire = button.action == TouchAction::Fire;
			fill(button.rect, selected ? NGfx::SPixel8888(157, 105, 48, 255)
				: !button.enabled ? NGfx::SPixel8888(59, 56, 51, 255)
				: fire ? NGfx::SPixel8888(141, 59, 41, 255) : NGfx::SPixel8888(84, 75, 59, 255));
		}
		view->CreateDynamicRects(static_cast<NDb::CTexture*>(nullptr), layout, pos, clip);
		DrawCaption(title, aiming ? SRect(378, 22, 688, 85) : SRect(0, 8, 448, 70), time, view);
		if (aiming)
			DrawCaption(details, SRect(378, 90, 688, 161), time, view);
		for (const auto& button : buttons)
			DrawCaption(button.text, button.rect, time, view);
	}
};
} // namespace

bool CMission::CanOpenTouchContext()
{
	return IsReady() && IsPlayerTurn() && !IsSequence() && !pWorld->IsSequence() &&
		!bHideInterface && !bSpecialHideInterface && (IsRealTime() || !IsActionExecuted()) &&
		pState->GetType() == IState::UPDATED;
}

bool CMission::OpenTouchContext()
{
	if (!CanOpenTouchContext())
		return false;
	vector<CPtr<IUnitTracker>> units;
	GetSelectedUnits(&units);
	if (units.size() != 1)
		return false;

	// Labels can be offset from their item and are easier to hold than the model.
	const CVec2 pos = GetCursor()->GetPos();
	auto* hud = dynamic_cast<NUI::CMissionUI*>(GetDesktop());
	CPtr<CObjectBase> target = hud ? hud->HitItemLabel(S2UI::FromPixel(pos.x), S2UI::FromPixel(pos.y)) : nullptr;
	if (!IsValid(target))
	{
		TraceCursor();
		target = sTraceResult.pObject;
	}
	if (!IsValid(target))
		return false;
	auto* unit = units.front()->GetUnit();
	auto* enemy = dynamic_cast<NWorld::CUnit*>(target.GetPtr());
	const bool aim = enemy && !enemy->IsDead() && !enemy->IsUnconscious() && IsFirearm(*unit) &&
		GetActivePlayer()->GetUnitDiplomacy(enemy) == NDb::DS_ENEMY;
	CObj<CTouchContextPanel> panel = new CTouchContextPanel(*this, unit, target, aim);
	if (!panel->HasActions())
		return false;
	pInterface->SetTouchOverlay(panel);
	return true;
}

bool CMission::ProcessTouchContext(const NInput::SEvent& event)
{
	auto* panel = static_cast<CTouchContextPanel*>(pInterface->GetTouchOverlay());
	const auto& msg = event.mMessage;
	static const int primary = NInput::GetControlID("MOUSE_BUTTON0");
	if (!CanOpenTouchContext() || bindCancel.ProcessEvent(event) || bindCancelAction.ProcessEvent(event) ||
		msg.cType == NInput::CT_TOUCH_CONTEXT)
		pInterface->SetTouchOverlay(nullptr);
	else if (msg.hasPointer && msg.cType == NInput::CT_KEY && msg.nAction == primary && !msg.bState)
	{
		const SPoint point(S2UI::FromPixel(msg.pointerX), S2UI::FromPixel(msg.pointerY));
		if (panel->Tap(*this, point))
			pInterface->SetTouchOverlay(nullptr);
	}
	return true;
}
} // namespace NGame
