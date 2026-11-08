#include "StdAfx.h"
// Weapon-tooltip idea: Gunnarguy/Silent-Storm-Mac, commit 2e9abd05.
// Adapted to our RPG implementation using detached, read-only shot previews.
#include "GameplayConvenience.h"
#include "Interface.h"
#include "RPGItemSet.h"
#include "RPGUnit.h"
#include "RPGUnitMission.h"
#include "aiPosition.h"
#include "wUnitServer.h"
#include "wMain.h"
#include "RPGToHit.h"
#include "RPGGame.h"
#include "rpgPerkConstants.h"
#include "rpgPerk.h"
#include "../DBFormat/DataRPG.h"
#include "../MiscDll/Commands.h"
namespace NRPG { NAI::EPose GetBestPose(const NDb::CRPGWeaponType&); }

namespace NUI {
bool ConvenienceRussian() {
    for(wchar_t c:GetDBString(4382)) if(c>=0x400 && c<=0x4ff) return true;
    return false;
}
namespace {
const wchar_t* Text(bool ru,const wchar_t* en,const wchar_t* russian) {return ru?russian:en;}
// The preview owns a detached weapon wrapper, sharing only its read-only ammo.
// No equipment, shoot mode, familiarity, ammunition or random state is changed.
class CComparisonHit: public NRPG::CToHitCalcer {
    OBJECT_NOCOPY_METHODS(CComparisonHit);
    CPtr<NRPG::CWeaponItem> original;
public:
    CComparisonHit() {}
    CComparisonHit(NWorld::CUnitServer* unit,NRPG::CWeaponItem* preview,NRPG::CWeaponItem* source,int distance,int bullet):
        NRPG::CToHitCalcer(unit,unit->GetPose(),distance,unit->GetPosition().GetCP(),0,0,1.f,false,
            unit->GetWorld()->GetGame()->IsNight(),CVec3(1,1,1),bullet,false),original(source) {pWeaponItem=preview;}
    float GetStance() override {
        auto& type=*pWeaponItem->GetDBWeapon()->pWeaponType;
        NAI::EPose pose=eCurPose;
        if(pUnitMission->HasPerk(NRPG::N_PERK_BEST_SHOOTING_POSE)) pose=NRPG::GetBestPose(type);
        return pose==NAI::CROUCH?int(type.fCrouchBonus):pose==NAI::CRAWL?int(type.fCrawlBonus):0;
    }
    float GetAllAdd() override {
        return NRPG::CToHitCalcer::GetAllAdd()+
            pUnitMission->GetRPGUnit()->GetWeaponAdaptation(original)-pUnitMission->GetWeaponAdaptation();
    }
};
struct Shot {float value=0;int hit=0;};
Shot BestShot(NWorld::CUnitServer* unit,NRPG::CWeaponItem* gun,int distance) {
    Shot best;
    NRPG::SWeaponInfo info;gun->GetInfo(&info);
    // An unloaded or broken gun cannot produce damage in its present state.
    if(!gun->HasAmmo() || !gun->IsWorking()) return best;
    for(auto mode:{NDb::SM_Snap,NDb::SM_Aimed}) {
        CObj<NRPG::CWeaponItem> preview=gun->CreateComparisonPreview(mode);
        if(!preview) continue;
        int bullets=1,ap=unit->GetUnitRPG()->GetRPGUnit()->GetWeaponAP(preview);
        bullets=Min(Max(1,bullets),gun->GetAmmoQuantity());
        float hits=0;int first=0;
        for(int b=0;b<bullets;++b) {
            CObj<CComparisonHit> calc=new CComparisonHit(unit,preview,gun,distance,b);
            const int chance=calc->GetToHit();if(!b)first=chance;hits+=chance*.01f;
        }
        const float value=(info.nDmgMin+info.nDmgMax)*.5f*hits/Max(1,ap);
        if(value>best.value) {best.value=value;best.hit=first;}
    }
    return best;
}
wstring Row(const wchar_t* label,float candidate,float held,bool high,bool integer=true) {
    wchar_t text[256];const float diff=candidate-held;
    const wchar_t* color=diff==0?L"beige":((diff>0)==high?L"FF7FDA7F":L"FFE87060");
    if(integer) swprintf(text,256,L"<br><color=darkyellow>%ls: <color=beige>%d / %d <color=%ls>(%+d)",label,int(candidate),int(held),color,int(diff));
    else swprintf(text,256,L"<br><color=darkyellow>%ls: <color=beige>%.1f / %.1f <color=%ls>(%+.1f)",label,candidate,held,color,diff);
    return text;
}
}
wstring WeaponComparisonText(NWorld::CUnit* unit,NRPG::IWeaponItemInfo* candidate,bool details) {
    if(!NGlobal::GetVar("ui_weapon_comparison",1).GetInt() || !IsValid(unit) || !candidate || !unit->GetRPG()) return L"";
    auto* inv=unit->GetRPG()->GetInventoryInfo();if(!inv)return L"";
    CDynamicCast<NRPG::IWeaponItemInfo> held(inv->GetActive());
    if(!held || !held->GetDBWeapon() || !candidate->GetDBWeapon() || !held->GetDBWeapon()->pWeaponType ||
        !candidate->GetDBWeapon()->pWeaponType || held.GetPtr()==candidate) return L"";
    const bool ru=ConvenienceRussian();
    NRPG::CReadOnlyPerkQueries readOnlyPerks;
    NRPG::SWeaponInfo a,b;candidate->GetInfo(&a);held->GetInfo(&b);
    // Item names are already localized and formatted by the game's string table.
    wstring out=L"<br><br><color=darkyellow>"+wstring(Text(ru,L"Compared with ",L"\u0421\u0440\u0430\u0432\u043d\u0435\u043d\u0438\u0435 \u0441 "))+GetDBString(held->GetDBItem()->pName);
    out+=Row(Text(ru,L"Damage",L"\u0423\u0440\u043e\u043d"),(a.nDmgMin+a.nDmgMax)*.5f,(b.nDmgMin+b.nDmgMax)*.5f,true,false);
    out+=Row(Text(ru,L"Range",L"\u0414\u0430\u043b\u044c\u043d\u043e\u0441\u0442\u044c"),float(a.nMaxRange),float(b.nMaxRange),true);
    out+=Row(Text(ru,L"Base shot AP",L"\u0411\u0430\u0437\u043e\u0432\u044b\u0435 \u041e\u0414"),float(a.nShotAP),float(b.nShotAP),false);
    if(details) {
        out+=Row(Text(ru,L"Penetration",L"\u041f\u0440\u043e\u0431\u0438\u0442\u0438\u0435"),float(a.nArmorPiercingAbility),float(b.nArmorPiercingAbility),true);
        out+=Row(Text(ru,L"Recoil",L"\u041e\u0442\u0434\u0430\u0447\u0430"),float(a.nRecoil),float(b.nRecoil),false);
        out+=Row(Text(ru,L"Familiarity",L"\u041f\u0440\u0438\u0432\u044b\u0447\u043a\u0430"),unit->GetRPG()->GetRPGUnit()->GetWeaponAdaptation(candidate),unit->GetRPG()->GetRPGUnit()->GetWeaponAdaptation(held),true);
    }
    int rounds=candidate->GetAmmoQuantity();
    for(const auto& item:inv->GetItems()) {
        CDynamicCast<NRPG::IClipItem> clip(item.pItem.GetPtr());
        if(clip && candidate->CanLoad(clip)) rounds+=clip->GetIncQuantity();
    }
    wchar_t text[256];swprintf(text,256,Text(ru,L"<br><color=beige>Compatible rounds carried: %d",L"<br><color=beige>\u041f\u043e\u0434\u0445\u043e\u0434\u044f\u0449\u0438\u0445 \u043f\u0430\u0442\u0440\u043e\u043d\u043e\u0432 \u0441 \u0441\u043e\u0431\u043e\u0439: %d"),rounds);out+=text;
    CDynamicCast<NWorld::CUnitServer> server(unit);
    CDynamicCast<NRPG::CWeaponItem> newGun(candidate),oldGun(held.GetPtr());
    if(server && newGun && oldGun && newGun->GetWeaponType()!=NDb::WT_RLAUNCHER && oldGun->GetWeaponType()!=NDb::WT_RLAUNCHER) {
        out+=Text(ru,L"<br><color=darkyellow>Estimated damage/AP, exposed target:",L"<br><color=darkyellow>\u041e\u0446\u0435\u043d\u043a\u0430 \u0443\u0440\u043e\u043d\u0430/\u041e\u0414 \u043f\u043e \u043e\u0442\u043a\u0440\u044b\u0442\u043e\u0439 \u0446\u0435\u043b\u0438:");
        for(int distance:{10,25,45}) {
            const Shot newer=BestShot(server,newGun,distance),older=BestShot(server,oldGun,distance);
            swprintf(text,256,L"<br><color=beige>%d: %.2f / %.2f",distance,newer.value,older.value);out+=text;
        }
        out+=Text(ru,L"<br><color=grey>Snap / aimed; no cover or armor.",L"<br><color=grey>\u041d\u0430\u0432\u0441\u043a\u0438\u0434\u043a\u0443 / \u043f\u0440\u0438\u0446\u0435\u043b\u044c\u043d\u043e; \u0431\u0435\u0437 \u0443\u043a\u0440\u044b\u0442\u0438\u0439 \u0438 \u0431\u0440\u043e\u043d\u0438.");
    }
    if(!details) out+=Text(ru,L"<br><color=grey>Hold Shift for details",L"<br><color=grey>\u0423\u0434\u0435\u0440\u0436\u0438\u0432\u0430\u0439\u0442\u0435 Shift \u0434\u043b\u044f \u043f\u043e\u0434\u0440\u043e\u0431\u043d\u043e\u0441\u0442\u0435\u0439");
    return out;
}
}
