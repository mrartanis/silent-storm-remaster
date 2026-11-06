#include "StdAfx.h"
#include "AgentPlay.h"
#include "../MiscDll/Commands.h"
#include "../Main/Interface.h"
#include "../Main/iMission.h"
#include "../Main/iMissionInternal.h"
#include "../Main/iMain.h"
#include "../Main/NetworkWorld.h"
#include "../Main/wUnitServer.h"
#include "../Main/wUnitCommands.h"
#include "../Main/RPGUnitMission.h"
#include "../Main/RPGItem.h"
#include "../Main/RPGGame.h"
#include "../Main/DisplayLayout.h"
#include "../Main/Camera.h"
#include "../Main/GView.h"
#include "../Main/aiGrid.h"
#include "../DBFormat/DataRPG.h"
#include "../DBFormat/DataMap.h"
#include <sstream>
#include <iomanip>
#include <locale>
#include <cmath>
#include <cstdio>

namespace NMainLoop { bool MakeScreenShot(); }

namespace S2Agent {
// Access stays here: no new public world/debug API and no saved fields.
struct Controller {
  static std::shared_ptr<S2Net::NetworkSession> Session(NGame::CMission* m) { return m->networkSession; }
  static std::shared_ptr<S2Net::NetworkWorld> World(NGame::CMission* m) { return m->networkWorld; }
};
namespace {
std::shared_ptr<S2Net::NetworkSession> session;
std::map<CPtr<NWorld::CUnit>,unsigned> contacts;
unsigned nextContact=1;

std::string Quote(const std::string& text) {
  std::ostringstream out;out << '"';
  for(unsigned char c:text) {
    if(c=='"' || c=='\\')out << '\\' << char(c);
    else if(c<32)out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << unsigned(c) << std::dec;
    else out << char(c);
  }
  out << '"';return out.str();
}
unsigned Contact(NWorld::CUnit* unit) {
  auto key=CPtr<NWorld::CUnit>(unit);auto found=contacts.find(key);
  if(found!=contacts.end())return found->second;
  return contacts[key]=nextContact++;
}
void Position(std::ostream& out,NWorld::CUnit* unit) {
  const auto& p=unit->GetPosition();const auto point=p.GetCP();
  out << "\"grid\":[" << p.pos.p.GetX() << ',' << p.pos.p.GetY()
      << "],\"position\":[" << point.x << ',' << point.y << ',' << point.z << "],\"pose\":" << int(p.GetPose());
}
void Item(std::ostream& out,NRPG::IInventoryItem* item) {
  out << "\"db\":" << item->GetDBItem()->GetRecordID() << ",\"quantity\":" << item->GetQuantity();
  const char* kind="item";
  if(dynamic_cast<NRPG::IWeaponItemInfo*>(item))kind="weapon";
  else if(dynamic_cast<NRPG::IGrenadeItemInfo*>(item))kind="grenade";
  else if(dynamic_cast<NRPG::IFirstAidItem*>(item))kind="medical";
  else if(dynamic_cast<NRPG::IClipItem*>(item))kind="clip";
  else if(dynamic_cast<NRPG::IMineItem*>(item))kind="mine";
  else if(dynamic_cast<NRPG::IToolItem*>(item))kind="tool";
  out << ",\"kind\":" << Quote(kind);
  if(auto* container=dynamic_cast<NRPG::IItemContainerInfo*>(item))out << ",\"charges\":" << container->GetIncQuantity();
}
std::string Observe(NGame::CMission* mission) {
  std::ostringstream out;out.imbue(std::locale::classic());out << std::fixed << std::setprecision(3);
  if(session) {
    auto state=session->State();
    if(state==S2Net::SessionState::Finished || state==S2Net::SessionState::Failed || state==S2Net::SessionState::Finishing) {
      out << "{\"phase\":\"finished\",\"status\":" << Quote(session->Status()) << '}';return out.str();
    }
  }
  if(!mission || !Controller::Session(mission))return "{\"phase\":\"connecting\"}";
  auto* player=mission->GetActivePlayer()->GetPlayer();
  auto* world=mission->GetWorld();
  const int local=player->GetScenarioPlayerID();
  auto network=Controller::World(mission);
  auto* map=NDb::GetTemplate(network->Match().templateID);
  out << "{\"phase\":" << Quote(network->IsPreparing()?"preparing":"playing") << ",\"side\":" << local;
  out << ",\"starting_level\":" << network->Preset().startingLevel << ",\"supply_level\":" << network->Preset().supplyLevel;
  if(network->IsPreparing())out << ",\"ready\":" << (network->IsReady(local)?"true":"false")
    << ",\"opponent_ready\":" << (network->IsReady(1-local)?"true":"false");
  if(map)out << ",\"map\":{\"width\":" << map->nWidth << ",\"height\":" << map->nHeight << '}';
  out << ",\"turn\":"
      << (world->GetCurrentPlayer()?world->GetCurrentPlayer()->GetScenarioPlayerID():-1)
      << ",\"executing\":" << (world->IsExecuting()?"true":"false") << ",\"units\":[";
  NWorld::IPlayer::CUnitSet units;player->GetUnits(&units);
  for(size_t i=0;i<units.size();++i) {
    auto* u=static_cast<NWorld::CUnitServer*>(units[i].GetPtr());auto* gun=u->GetUnitRPG()->GetWeaponItem();
    if(i)out << ',';
    out << "{\"id\":" << i << ',';Position(out,u);
    out << ",\"hp\":" << u->GetUnitRPG()->GetTotalVP() << ",\"ap\":" << u->GetAP()
        << ",\"can_fight\":" << (u->CanFight()?"true":"false") << ",\"command\":" << (u->HasCommand()?"true":"false")
        << ",\"ammo\":" << (gun?gun->GetAmmoQuantity():-1) << ",\"shoot_mode\":" << (gun?int(gun->GetShootMode()):-1)
        << ",\"supported_modes\":[";
    bool firstMode=true;
    for(int mode=0;gun && mode<NDb::SM_MAXVALUE;++mode)if(gun->IsShootModeSupported(static_cast<NDb::EShootMode>(mode))) {
      if(!firstMode)out << ',';firstMode=false;out << mode;
    }
    auto* inventory=u->GetUnitRPG()->GetInventory();
    out << "],\"active_slot\":" << inventory->GetActiveSlot() << ",\"slots\":[";
    bool firstSlot=true;
    for(int slot=0;slot<NDb::N_SLOTS;++slot)if(auto* item=inventory->Get(static_cast<NDb::ESlot>(slot))) {
      if(!firstSlot)out << ',';firstSlot=false;out << "{\"slot\":" << slot << ',';Item(out,item);out << '}';
    }
    out << "],\"backpack\":[";
    const auto& items=inventory->GetItems();
    for(size_t item=0;item<items.size();++item) {
      if(item)out << ',';out << "{\"item\":" << item << ',';Item(out,items[item].pItem);out << '}';
    }
    out << "]}";
  }
  out << "],\"visible_enemies\":[";
  list<CPtr<NWorld::CUnit>> visible;player->GetVisible(&visible);bool first=true;
  // Never enumerate the opponent's roster or retain last-known positions here.
  for(auto u:visible)if(u && world->GetDiplomacyState(player,u->GetPlayer())==NDb::DS_ENEMY && !u->IsDead()) {
    if(!first)out << ',';first=false;
    out << "{\"contact\":" << Contact(u) << ',';Position(out,u);
    NWorld::SEnemyInfo info;player->GetEnemyUnitInfo(u,&info);
    if(info.bUnitInfo)out << ",\"condition\":" << int(info.eUnitCondition);
    if(info.bCanSeeUnitHP)out << ",\"hp\":" << info.nUnitHP;
    out << '}';
  }
  out << "]}";return out.str();
}
bool Execute(NGame::CMission* mission,std::istringstream& input,const std::string& verb,std::string& result,bool& quit) {
  auto end=[&](){std::string extra;return !(input>>extra);};
  if(verb=="quit" && end()){quit=true;result="\"status\":\"closing\"";return true;}
  if(verb=="observe" && end())return true;
  if(verb=="screenshot" && end()) {
    if(!mission)return false;
    return NMainLoop::MakeScreenShot();
  }
  if(!mission || !Controller::Session(mission))return false;
  auto* player=mission->GetActivePlayer()->GetPlayer();auto* world=mission->GetWorld();
  if(verb=="ready" && end()) {
    auto network=Controller::World(mission);if(!network->IsPreparing() || network->IsStartingBattle())return false;
    Controller::Session(mission)->Submit(network->PreparationCommand(S2Net::PreparationAction::Ready,0,1));return true;
  }
  if(verb=="hover" || verb=="click" || verb=="right_click") {
    int x=-1,y=-1;const auto screen=mission->GetScene()->GetScreenRect();
    if(!(input>>x>>y) || !end() || x<0 || y<0 || x>=screen.x || y>=screen.y)return false;
    mission->GetCursor()->SetPos(CVec2(float(x),float(y)));
    x=int(S2UI::FromPixel(float(x)));y=int(S2UI::FromPixel(float(y)));
    auto* ui=mission->GetInterface();ui->ProcessMessage(NUI::SEvent(NUI::EVENT_MOUSEMOVE,x,y));
    if(verb!="hover") {
      ui->ProcessMessage(NUI::SEvent(verb=="click"?NUI::EVENT_LBUTTONDOWN:NUI::EVENT_RBUTTONDOWN,x,y));
      ui->ProcessMessage(NUI::SEvent(verb=="click"?NUI::EVENT_LBUTTONUP:NUI::EVENT_RBUTTONUP,x,y));
    }
    return true;
  }
  if(verb=="camera") {
    float dx=0,dy=0,yaw=0,zoom=0;
    if(!(input>>dx>>dy>>yaw>>zoom) || !end() || !std::isfinite(dx) || !std::isfinite(dy) || !std::isfinite(yaw)
      || !std::isfinite(zoom) || std::abs(dx)>100 || std::abs(dy)>100 || std::abs(yaw)>6.3f || std::abs(zoom)>100)return false;
    auto* camera=mission->GetCamera();if(camera->IsCameraFrozen())return false;
    camera->ScrollAnchor(CVec3(dx,dy,0),true,true);
    ICamera::SCameraPos position;ICamera::SCameraLimits limits;
    camera->GetPlacement(&position);camera->GetLimits(&limits);position.fYaw+=yaw;
    position.fRod=(std::max)(limits.fMinRod,(std::min)(limits.fMaxRod,position.fRod+zoom));
    camera->SetPlacement(position);return true;
  }
  if(verb=="floor") {
    int floor=0;if(!(input>>floor) || floor<-3 || floor>4 || !end())return false;
    mission->SetCutFloor(floor);return true;
  }
  if(verb=="end_turn" && end()) {
    if(world->IsExecuting() || world->GetCurrentPlayer()!=player)return false;
    mission->Command(new NWorld::CCmdEndOfTurn);return true;
  }
  if(verb=="stop" && end()) {
    if(world->GetCurrentPlayer()!=player)return false;
    mission->StopAction();return true;
  }
  int index=-1;if(!(input>>index))return false;
  NWorld::IPlayer::CUnitSet units;player->GetUnits(&units);
  if(index<0 || index>=int(units.size()))return false;
  auto* unit=static_cast<NWorld::CUnitServer*>(units[index].GetPtr());
  if(verb=="focus" && end()){mission->Select(unit,false);mission->FocusCameraOnUnit(unit);return true;}
  if(world->GetCurrentPlayer()!=player || world->IsExecuting() || !unit->CanFight())return false;
  CObj<NWorld::CCmd> action;bool preview=false;NWorld::CUnit* attackTarget=nullptr;
  std::string actionVerb=verb;
  if(verb.compare(0,8,"preview_")==0){preview=true;actionVerb=verb.substr(8);}
  if(actionVerb=="move" || actionVerb=="look") {
    int x=0,y=0;if(!(input>>x>>y) || x<0 || y<0 || x>4095 || y>4095 || !end())return false;
    auto destination=unit->GetPosition().pos;destination.p.SetXY(x,y);destination.p.SetFinal(0);
    if(actionVerb=="move")action=new NWorld::CCmdPath(destination);
    else action=new NWorld::CCmdLook(destination);
  } else if(actionVerb=="attack") {
    unsigned id=0;if(!(input>>id) || !end())return false;
    list<CPtr<NWorld::CUnit>> visible;player->GetVisible(&visible);NWorld::CUnit* target=nullptr;
    for(auto u:visible)if(u && world->GetDiplomacyState(player,u->GetPlayer())==NDb::DS_ENEMY && !u->IsDead() && Contact(u)==id)target=u;
    if(!target)return false;
    attackTarget=target;
    action=new NWorld::CCmdShootObject(target,0,NAI::HL_ANY);
  } else if(actionVerb=="shoot") {
    float x=0,y=0,z=0;if(!(input>>x>>y>>z) || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)
      || std::abs(x)>4096 || std::abs(y)>4096 || std::abs(z)>128 || !end())return false;
    action=new NWorld::CCmdShootTile(CVec3(x,y,z));
  } else if(actionVerb=="reload" && end())action=new NWorld::CCmdReload;
  else if(actionVerb=="continue" && end())action=new NWorld::CCmdContinue;
  else if(actionVerb=="mode") {
    int mode=-1;if(!(input>>mode) || mode<0 || mode>=NDb::SM_MAXVALUE || !end())return false;
    action=new NWorld::CCmdShootMode(static_cast<NDb::EShootMode>(mode));
  } else if(actionVerb=="pose") {
    int pose=-1;if(!(input>>pose) || pose<0 || pose>2 || !end())return false;
    auto destination=unit->GetPosition();destination.SetPose(static_cast<NAI::EPose>(pose));
    action=new NWorld::CCmdPath(destination.pos,NAI::PF_USE_POSEDIR);
  } else if(actionVerb=="hide") {
    int state=-1;if(!(input>>state) || state<0 || state>1 || !end())return false;
    action=new NWorld::CCmdHide(state!=0);
  } else if(actionVerb=="slot") {
    int slot=-1;if(!(input>>slot) || slot<0 || slot>=NDb::N_SLOTS || !end())return false;
    action=new NWorld::CCmdSetActiveItem(slot);
  } else if(actionVerb=="equip") {
    int item=-1,slot=-1;
    const auto& items=unit->GetUnitRPG()->GetInventory()->GetItems();
    if(!(input>>item>>slot) || item<0 || item>=int(items.size()) || slot<0 || slot>=NDb::N_SLOTS || !end())return false;
    const auto& source=items[item];
    action=new NWorld::CCmdExchangeInventoryItems(NWorld::SItem(unit,NWorld::SItem::BACKPACK,source.sPos,source.pItem),
      NWorld::SItem(unit,NWorld::SItem::SLOT,slot),true,slot);
  } else if(actionVerb=="heal") {
    int other=-1;if(!(input>>other) || other<0 || other>=int(units.size()) || !end())return false;
    action=new NWorld::CCmdHeal(units[other]);
  } else return false;
  mission->Select(unit,false);
  if(preview) {
    int start=-1,full=-1;auto code=mission->CanDoCommand(action,false,&start,&full);
    std::ostringstream out;out << "\"result\":" << int(code) << ",\"start_ap\":" << start << ",\"full_ap\":" << full;
    if(attackTarget)out << ",\"hit_chance\":" << world->GetGame()->GetCompositeToHit(unit,attackTarget,NAI::HL_ANY,false);
    result=out.str();
  } else {mission->Command(unit,action);result="\"status\":\"submitted\"";}
  return true;
}
}

bool Poll() {
  auto* mission=dynamic_cast<NGame::CMission*>(NMainLoop::GetCurrentInterfaceForHarness());
  if(mission && Controller::Session(mission))session=Controller::Session(mission);
  FILE* file=fopen("_agent_request.txt","rb");if(!file)return true;
  char buffer[1025]={};size_t count=fread(buffer,1,sizeof(buffer)-1,file);fclose(file);remove("_agent_request.txt");
  std::istringstream input(std::string(buffer,count));input.imbue(std::locale::classic());
  unsigned long long id=0;std::string verb,result;bool quit=false;
  bool ok=count<sizeof(buffer)-1 && (input>>id>>verb) && Execute(mission,input,verb,result,quit);
  std::string response="{\"id\":"+std::to_string(id)+",\"ok\":"+(ok?"true":"false");
  if(!ok)response+=",\"error\":\"invalid_or_unavailable_action\"";
  if(!result.empty())response+=","+result;
  response+=",\"observation\":"+Observe(mission)+"}\n";
  if(FILE* output=fopen("_agent_response.tmp","wb")) {
    fwrite(response.data(),1,response.size(),output);fclose(output);
    remove("_agent_response.json");rename("_agent_response.tmp","_agent_response.json");
  }
  return !quit;
}
void Done() {
  contacts.clear();session.reset();nextContact=1;
}
}
