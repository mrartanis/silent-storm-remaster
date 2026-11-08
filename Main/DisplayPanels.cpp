#include "StdAfx.h"
#include "Interface.h"
#include "DisplayLayout.h"
#include "DisplayOptions.h"
#include "GameplayConvenience.h"
#include "../Input/Bind.h"
#include "GInit.h"
#include "../MiscDll/Commands.h"
#include <cwctype>

namespace NUI {
class CConveniencePanel: public CWindow {
  OBJECT_BASIC_METHODS(CConveniencePanel);
public:
  CConveniencePanel() {}
  explicit CConveniencePanel(const SWindowInfo& info):CWindow(info) {}
  bool ProcessMessage(const SEvent& event) override {
    if(event.nEvent==EVENT_NOTIFY && GetInterface()->HandleConvenienceMessage(event))return true;
    return CWindow::ProcessMessage(event);
  }
};
namespace {
bool RussianDisplayText() {
  for(wchar_t c:GetDBString(4382)) if(c>=0x400 && c<=0x4ff) return true;
  return false;
}
CWindow* Panel(CInterface* ui, const char* id, int height) {
  CWindow* old = ui->GetChildByID(id);
  if (IsValid(old)) { old->ShowWindow(SWTYPE_SHOW); return old; }
  const SWindowInfo info(ui,SPoint(240,(768-height)/2),SPoint(544,height),id,
      STYLE_ENABLED | STYLE_VISIBLE | STYLE_TOPMOST | STYLE_MODAL);
  CWindow* panel=std::string(id)=="convenience_settings" || std::string(id)=="keyboard_help" ?
      static_cast<CWindow*>(new CConveniencePanel(info)):new CWindow(info);
  panel->EnableAdaptiveLayout();
  auto* background = new CImage(SWindowInfo(panel, SPoint(0,0), SPoint(544,height), "background",
      STYLE_ENABLED | STYLE_VISIBLE | STYLE_TRANSPARENT | STYLE_BOTTOMMOST));
  const bool confirmation=std::string(id)=="display_confirmation" || std::string(id)=="convenience_settings" || std::string(id)=="keyboard_help";
  background->SetColor(confirmation?NGfx::SPixel8888(216,203,175,255):NGfx::SPixel8888(12,16,20,245));
  if(confirmation) {
    const SPoint positions[]={SPoint(0,0),SPoint(0,height-3),SPoint(0,3),SPoint(541,3)};
    const SPoint sizes[]={SPoint(544,3),SPoint(544,3),SPoint(3,height-6),SPoint(3,height-6)};
    for(int i=0;i<4;++i) {
      auto* border=new CImage(SWindowInfo(panel,positions[i],sizes[i],"border",STYLE_ENABLED|STYLE_VISIBLE|STYLE_TRANSPARENT));
      border->SetColor(NGfx::SPixel8888(89,58,39,255));
    }
  }
  panel->ShowWindow(SWTYPE_SHOW);
  return panel;
}
void Label(CWindow* parent, const char* id, int y, const wstring& text) {
  CText* label = dynamic_cast<CText*>(parent->GetChildByID(id));
  if (!IsValid(label)) label = new CText(SWindowInfo(parent, SPoint(16,y), SPoint(512,id==std::string("countdown")?64:32),id,
      STYLE_VISIBLE | STYLE_ENABLED | STYLE_TRANSPARENT));
  if (label->GetText()!=text) label->SetText(text);
}
void Button(CWindow* parent, const char* id, int y, const wstring& text) {
  CButton* button = dynamic_cast<CButton*>(parent->GetChildByID(id));
  if (!IsValid(button)) {
    button = new CButton(SWindowInfo(parent,SPoint(16,y),SPoint(512,34),id));
    CText* label = new CText(SWindowInfo(button,SPoint(8,0),SPoint(496,34),"caption",
        STYLE_VISIBLE | STYLE_ENABLED | STYLE_TRANSPARENT));
    label->SetText(text);
  } else {
    CText* label = dynamic_cast<CText*>(button->GetChildByID("caption"));
    if (IsValid(label) && label->GetText()!=text) label->SetText(text);
  }
}
void NativeButton(CWindow* parent, const char* id, int y, const wstring& text) {
  if(IsValid(parent->GetChildByID(id))) return;
  auto* button=new CButton(SWindowInfo(parent,SPoint(16,y),SPoint(512,36),id));
  button->AddTextState(0,GetDBString(4404)+L"<center>"+text);
  button->AddTextState(1,GetDBString(4405)+L"<center>"+text);
  button->AddTextState(2,GetDBString(4405)+L"<center>"+text);
}
}
void CInterface::ShowDisplayOptions() {
  ResetMouseCapture();
  Panel(this,"display_settings",300);
  UpdateDisplayPanels();
}
void CInterface::UpdateDisplayPanels() {
  CWindow* settings=GetChildByID("display_settings");
  if (IsValid(settings) && settings->GetStyle(STYLE_VISIBLE)) {
    BringWindowToTop(settings);
    Label(settings,"title",12,L"<font size=22pt>Display and interface");
    const int mode=NGlobal::GetVar("gfx_fullscreen",1).GetInt();
    Button(settings,"display_mode",58,mode==2?L"<font size=18pt>Window mode: Borderless":mode==1?L"<font size=18pt>Window mode: Fullscreen":L"<font size=18pt>Window mode: Window");
    WCHAR text[256];
    const float percent=NGlobal::GetVar("ui_scale",0).GetFloat();
    const auto& m=S2Platform::Display();
    if (percent<=0) swprintf(text,256,L"<font size=18pt>UI scale: Auto (applied %.0f%%)",m.uiScale/m.displayScale*100);
    else swprintf(text,256,L"<font size=18pt>UI scale: %.0f%% (applied %.0f%%)",percent,m.uiScale/m.displayScale*100);
    Button(settings,"display_scale",102,text);
    Button(settings,"display_fonts",146,NGlobal::GetVar("ui_vector_fonts",1).GetFloat()!=0 ? L"<font size=18pt>Fonts: Vector" : L"<font size=18pt>Fonts: Original bitmap");
    Label(settings,"hint",193,L"<font size=14pt>Click a setting to change it. Scale is limited to fit the UI.");
    Button(settings,"display_close",246,L"<font size=18pt><center>Close");
  }
  const int seconds=NGScene::DisplayChangeSeconds();
  CWindow* confirmation=GetChildByID("display_confirmation");
  if(seconds>0) {
    confirmation=Panel(this,"display_confirmation",220);
    BringWindowToTop(confirmation);
    const bool russian=RussianDisplayText();
    WCHAR text[256]; swprintf(text,256,russian?
      L"\u0421\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c \u0432\u044b\u0431\u0440\u0430\u043d\u043d\u044b\u0439 \u0440\u0435\u0436\u0438\u043c?<br>\u0412\u043e\u0437\u0432\u0440\u0430\u0442 \u0447\u0435\u0440\u0435\u0437 %d \u0441\u0435\u043a.":
      L"Keep this display mode?<br>Reverting in %d seconds.",seconds);
    Label(confirmation,"countdown",20,GetDBString(4402)+L"<center>"+text);
    NativeButton(confirmation,"display_keep",92,russian?L"\u0421\u041e\u0425\u0420\u0410\u041d\u0418\u0422\u042c (ENTER)":L"KEEP (ENTER)");
    NativeButton(confirmation,"display_revert",140,russian?L"\u0412\u0415\u0420\u041d\u0423\u0422\u042c (ESCAPE)":L"REVERT (ESCAPE)");
  } else if(IsValid(confirmation)) confirmation->SetStyle(STYLE_VISIBLE,false);
}
bool CInterface::HandleDisplayMessage(const SEvent& event) {
  if(event.nEvent==EVENT_WINKEY) {
    if(NGScene::DisplayChangeSeconds()>0 && (event.nVal==13 || event.nVal==27)) {
      if(event.nVal==13) NGScene::ConfirmDisplayChange(); else NGScene::RevertDisplayChange();
      UpdateDisplayPanels(); return true;
    }
    CWindow* settings=GetChildByID("display_settings");
    if(event.nVal==27 && IsValid(settings) && settings->GetStyle(STYLE_VISIBLE)) {
      settings->SetStyle(STYLE_VISIBLE,false); return true;
    }
  }
  if(event.nEvent!=EVENT_NOTIFY) return false;
  const auto& id=event.szID;
  if(id=="display_keep") NGScene::ConfirmDisplayChange();
  else if(id=="display_revert") NGScene::RevertDisplayChange();
  else if(id=="display_close") { CWindow* panel=GetChildByID("display_settings"); if(IsValid(panel)) panel->SetStyle(STYLE_VISIBLE,false); }
  else if(id=="display_mode") {
    NGScene::BeginDisplayChange();
    NGlobal::SetVar("gfx_fullscreen",(NGlobal::GetVar("gfx_fullscreen",1).GetInt()+1)%3);
    if(!NGScene::SetModeFromConfig(false)) NGScene::RevertDisplayChange();
  } else if(id=="display_scale") {
    const int choices[]={0,75,100,125,150,200};
    const int old=static_cast<int>(NGlobal::GetVar("ui_scale",0).GetFloat());
    int next=0; for(int i=0;i<6;++i) if(choices[i]==old) next=choices[(i+1)%6];
    NGlobal::SetVar("ui_scale",static_cast<float>(next));
    S2Platform::UpdateDisplay(static_cast<float>(next));
  } else if(id=="display_fonts") NGlobal::SetVar("ui_vector_fonts",NGlobal::GetVar("ui_vector_fonts",1).GetFloat()!=0?0.0f:1.0f);
  else return false;
  UpdateDisplayPanels(); return true;
}

namespace {
struct HelpCommand {const char* command;const wchar_t* english;const wchar_t* russian;};
const HelpCommand helpCommands[]={
 {"attack",L"Attack",L"\u0410\u0442\u0430\u043a\u043e\u0432\u0430\u0442\u044c"},{"move",L"Move",L"\u041f\u0435\u0440\u0435\u043c\u0435\u0449\u0435\u043d\u0438\u0435"},
 {"firstaid",L"First aid",L"\u041f\u0435\u0440\u0432\u0430\u044f \u043f\u043e\u043c\u043e\u0449\u044c"},{"usetool",L"Use tool",L"\u0418\u0441\u043f\u043e\u043b\u044c\u0437\u043e\u0432\u0430\u0442\u044c \u0438\u043d\u0441\u0442\u0440\u0443\u043c\u0435\u043d\u0442"},
 {"setmine",L"Set mine",L"\u0423\u0441\u0442\u0430\u043d\u043e\u0432\u0438\u0442\u044c \u043c\u0438\u043d\u0443"},{"dropcorpse",L"Drop carried body",L"\u041f\u043e\u043b\u043e\u0436\u0438\u0442\u044c \u043f\u0435\u0440\u0435\u043d\u043e\u0441\u0438\u043c\u043e\u0435 \u0442\u0435\u043b\u043e"},
 {"showitems",L"Highlight items",L"\u041f\u043e\u0434\u0441\u0432\u0435\u0442\u0438\u0442\u044c \u043f\u0440\u0435\u0434\u043c\u0435\u0442\u044b"},{"show_info",L"Show information",L"\u041f\u043e\u043a\u0430\u0437\u0430\u0442\u044c \u0438\u043d\u0444\u043e\u0440\u043c\u0430\u0446\u0438\u044e"},
 {"hide",L"Hide",L"\u0421\u043a\u0440\u044b\u0442\u043d\u043e\u0441\u0442\u044c"},{"focus_unit",L"Focus soldier",L"\u041a\u0430\u043c\u0435\u0440\u0430 \u043d\u0430 \u0431\u043e\u0439\u0446\u0430"},
 {"center",L"Center camera",L"\u0426\u0435\u043d\u0442\u0440\u0438\u0440\u043e\u0432\u0430\u0442\u044c \u043a\u0430\u043c\u0435\u0440\u0443"},{"character",L"Character sheet",L"\u0425\u0430\u0440\u0430\u043a\u0442\u0435\u0440\u0438\u0441\u0442\u0438\u043a\u0438 \u0431\u043e\u0439\u0446\u0430"},
 {"clues",L"Clues",L"\u0423\u043b\u0438\u043a\u0438"},{"objectives",L"Objectives",L"\u0426\u0435\u043b\u0438 \u0437\u0430\u0434\u0430\u043d\u0438\u044f"},
 {"nextenemy",L"Next enemy",L"\u0421\u043b\u0435\u0434\u0443\u044e\u0449\u0438\u0439 \u043f\u0440\u043e\u0442\u0438\u0432\u043d\u0438\u043a"},{"slot_next",L"Switch held item",L"\u0421\u043c\u0435\u043d\u0438\u0442\u044c \u043f\u0440\u0435\u0434\u043c\u0435\u0442 \u0432 \u0440\u0443\u043a\u0430\u0445"},
 {"gamemenu",L"Game menu",L"\u0418\u0433\u0440\u043e\u0432\u043e\u0435 \u043c\u0435\u043d\u044e"},{"loadmenu",L"Load game",L"\u0417\u0430\u0433\u0440\u0443\u0437\u0438\u0442\u044c \u0438\u0433\u0440\u0443"},
 {"pose_crawl",L"Prone",L"\u041b\u0435\u0447\u044c"},{"pose_crouch",L"Crouch",L"\u041f\u0440\u0438\u0441\u0435\u0441\u0442\u044c"},
 {"pose_normal",L"Stand",L"\u0412\u0441\u0442\u0430\u0442\u044c"},{"pose_run",L"Run",L"\u0411\u0435\u0436\u0430\u0442\u044c"},{"pose_strafe",L"Strafe",L"\u0414\u0432\u0438\u0433\u0430\u0442\u044c\u0441\u044f \u0431\u043e\u043a\u043e\u043c"},
 {"select_all",L"Select all",L"\u0412\u044b\u0431\u0440\u0430\u0442\u044c \u0432\u0441\u0435\u0445"},{"select_all_units",L"Select all soldiers",L"\u0412\u044b\u0431\u0440\u0430\u0442\u044c \u0432\u0441\u0435\u0445 \u0431\u043e\u0439\u0446\u043e\u0432"},
 {"deselect",L"Clear selection",L"\u0421\u043d\u044f\u0442\u044c \u0432\u044b\u0434\u0435\u043b\u0435\u043d\u0438\u0435"},{"continue",L"Continue",L"\u041f\u0440\u043e\u0434\u043e\u043b\u0436\u0438\u0442\u044c"},
 {"skip",L"Skip",L"\u041f\u0440\u043e\u043f\u0443\u0441\u0442\u0438\u0442\u044c"},{"skipScript",L"Skip scene",L"\u041f\u0440\u043e\u043f\u0443\u0441\u0442\u0438\u0442\u044c \u0441\u0446\u0435\u043d\u0443"},{"skipScriptPart",L"Skip scene part",L"\u041f\u0440\u043e\u043f\u0443\u0441\u0442\u0438\u0442\u044c \u0447\u0430\u0441\u0442\u044c \u0441\u0446\u0435\u043d\u044b"},
 {"floor_slider",L"Change visible floor",L"\u0418\u0437\u043c\u0435\u043d\u0438\u0442\u044c \u0432\u0438\u0434\u0438\u043c\u044b\u0439 \u044d\u0442\u0430\u0436"},{"unit_rotate",L"Turn soldier",L"\u041f\u043e\u0432\u0435\u0440\u043d\u0443\u0442\u044c \u0431\u043e\u0439\u0446\u0430"},
 {"cursor_x",L"Pointer horizontal",L"\u0423\u043a\u0430\u0437\u0430\u0442\u0435\u043b\u044c \u043f\u043e \u0433\u043e\u0440\u0438\u0437\u043e\u043d\u0442\u0430\u043b\u0438"},{"cursor_y",L"Pointer vertical",L"\u0423\u043a\u0430\u0437\u0430\u0442\u0435\u043b\u044c \u043f\u043e \u0432\u0435\u0440\u0442\u0438\u043a\u0430\u043b\u0438"},
 {"scroll",L"Scroll",L"\u041f\u0440\u043e\u043a\u0440\u0443\u0442\u043a\u0430"},{"leftbutton_down",L"Select / act",L"\u0412\u044b\u0431\u0440\u0430\u0442\u044c / \u0432\u044b\u043f\u043e\u043b\u043d\u0438\u0442\u044c"},{"rightbutton_down",L"Alternate action",L"\u0414\u043e\u043f\u043e\u043b\u043d\u0438\u0442\u0435\u043b\u044c\u043d\u043e\u0435 \u0434\u0435\u0439\u0441\u0442\u0432\u0438\u0435"},
 {"hero_next",L"Next soldier",L"\u0421\u043b\u0435\u0434\u0443\u044e\u0449\u0438\u0439 \u0431\u043e\u0435\u0446"},{"hero_prev",L"Previous soldier",L"\u041f\u0440\u0435\u0434\u044b\u0434\u0443\u0449\u0438\u0439 \u0431\u043e\u0435\u0446"},
 {"endofturn",L"End turn",L"\u0417\u0430\u0432\u0435\u0440\u0448\u0438\u0442\u044c \u0445\u043e\u0434"},{"startofturn",L"Start turn",L"\u041d\u0430\u0447\u0430\u0442\u044c \u0445\u043e\u0434"},
 {"inventory",L"Inventory",L"\u0418\u043d\u0432\u0435\u043d\u0442\u0430\u0440\u044c"},{"pause",L"Pause",L"\u041f\u0430\u0443\u0437\u0430"},
 {"next_floor",L"Floor up",L"\u042d\u0442\u0430\u0436 \u0432\u044b\u0448\u0435"},{"prev_floor",L"Floor down",L"\u042d\u0442\u0430\u0436 \u043d\u0438\u0436\u0435"},
 {"hitlocation_head",L"Aim at head",L"\u0426\u0435\u043b\u0438\u0442\u044c\u0441\u044f \u0432 \u0433\u043e\u043b\u043e\u0432\u0443"},{"hitlocation_body",L"Aim at torso",L"\u0426\u0435\u043b\u0438\u0442\u044c\u0441\u044f \u0432 \u043a\u043e\u0440\u043f\u0443\u0441"},
 {"hitlocation_larm",L"Aim at left arm",L"\u0426\u0435\u043b\u0438\u0442\u044c\u0441\u044f \u0432 \u043b\u0435\u0432\u0443\u044e \u0440\u0443\u043a\u0443"},{"hitlocation_rarm",L"Aim at right arm",L"\u0426\u0435\u043b\u0438\u0442\u044c\u0441\u044f \u0432 \u043f\u0440\u0430\u0432\u0443\u044e \u0440\u0443\u043a\u0443"},
 {"hitlocation_lleg",L"Aim at left leg",L"\u0426\u0435\u043b\u0438\u0442\u044c\u0441\u044f \u0432 \u043b\u0435\u0432\u0443\u044e \u043d\u043e\u0433\u0443"},{"hitlocation_rleg",L"Aim at right leg",L"\u0426\u0435\u043b\u0438\u0442\u044c\u0441\u044f \u0432 \u043f\u0440\u0430\u0432\u0443\u044e \u043d\u043e\u0433\u0443"},
 {"weapon_reload",L"Reload",L"\u041f\u0435\u0440\u0435\u0437\u0430\u0440\u044f\u0434\u0438\u0442\u044c"},{"weapon_snapshot",L"Snap shot",L"\u0412\u044b\u0441\u0442\u0440\u0435\u043b \u043d\u0430\u0432\u0441\u043a\u0438\u0434\u043a\u0443"},
 {"weapon_aimedshot",L"Aimed shot",L"\u041f\u0440\u0438\u0446\u0435\u043b\u044c\u043d\u044b\u0439 \u0432\u044b\u0441\u0442\u0440\u0435\u043b"},{"weapon_carefulshot",L"Careful shot",L"\u0422\u043e\u0447\u043d\u044b\u0439 \u0432\u044b\u0441\u0442\u0440\u0435\u043b"},
 {"weapon_shortburst",L"Short burst",L"\u041a\u043e\u0440\u043e\u0442\u043a\u0430\u044f \u043e\u0447\u0435\u0440\u0435\u0434\u044c"},{"weapon_longburst",L"Long burst",L"\u0414\u043b\u0438\u043d\u043d\u0430\u044f \u043e\u0447\u0435\u0440\u0435\u0434\u044c"},
 {"weapon_snipeshot",L"Sniping",L"\u0421\u043d\u0430\u0439\u043f\u0435\u0440\u0441\u043a\u0438\u0439 \u0432\u044b\u0441\u0442\u0440\u0435\u043b"},{"weapon_prevmode",L"Previous fire mode",L"\u041f\u0440\u0435\u0434\u044b\u0434\u0443\u0449\u0438\u0439 \u0440\u0435\u0436\u0438\u043c \u043e\u0433\u043d\u044f"},
 {"weapon_nextmode",L"Next fire mode",L"\u0421\u043b\u0435\u0434\u0443\u044e\u0449\u0438\u0439 \u0440\u0435\u0436\u0438\u043c \u043e\u0433\u043d\u044f"},{"save",L"Quick save",L"\u0411\u044b\u0441\u0442\u0440\u043e\u0435 \u0441\u043e\u0445\u0440\u0430\u043d\u0435\u043d\u0438\u0435"},
 {"load",L"Quick load",L"\u0411\u044b\u0441\u0442\u0440\u0430\u044f \u0437\u0430\u0433\u0440\u0443\u0437\u043a\u0430"},{"savemenu",L"Save game",L"\u0421\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c \u0438\u0433\u0440\u0443"},
 {"loadgame",L"Load game",L"\u0417\u0430\u0433\u0440\u0443\u0437\u0438\u0442\u044c \u0438\u0433\u0440\u0443"},{"mainmenu",L"Game menu",L"\u0418\u0433\u0440\u043e\u0432\u043e\u0435 \u043c\u0435\u043d\u044e"},
 {"camera_zoom",L"Camera zoom",L"\u041f\u0440\u0438\u0431\u043b\u0438\u0436\u0435\u043d\u0438\u0435 \u043a\u0430\u043c\u0435\u0440\u044b"},{"camera_forward",L"Camera forward/back",L"\u041a\u0430\u043c\u0435\u0440\u0430 \u0432\u043f\u0435\u0440\u0451\u0434/\u043d\u0430\u0437\u0430\u0434"},
 {"camera_strafe",L"Camera left/right",L"\u041a\u0430\u043c\u0435\u0440\u0430 \u0432\u043b\u0435\u0432\u043e/\u0432\u043f\u0440\u0430\u0432\u043e"},{"camera_rotate",L"Rotate camera",L"\u0412\u0440\u0430\u0449\u0435\u043d\u0438\u0435 \u043a\u0430\u043c\u0435\u0440\u044b"},
 {"camera_pitch",L"Camera tilt",L"\u041d\u0430\u043a\u043b\u043e\u043d \u043a\u0430\u043c\u0435\u0440\u044b"},{"confirm",L"Confirm",L"\u041f\u043e\u0434\u0442\u0432\u0435\u0440\u0434\u0438\u0442\u044c"},
 {"camera_fps_pitch",L"First-person camera tilt",L"\u041d\u0430\u043a\u043b\u043e\u043d \u043a\u0430\u043c\u0435\u0440\u044b \u043e\u0442 \u043f\u0435\u0440\u0432\u043e\u0433\u043e \u043b\u0438\u0446\u0430"},
 {"camera_fps_rotate",L"First-person camera rotation",L"\u041f\u043e\u0432\u043e\u0440\u043e\u0442 \u043a\u0430\u043c\u0435\u0440\u044b \u043e\u0442 \u043f\u0435\u0440\u0432\u043e\u0433\u043e \u043b\u0438\u0446\u0430"},
 {"cancel",L"Cancel / close",L"\u041e\u0442\u043c\u0435\u043d\u0430 / \u0437\u0430\u043a\u0440\u044b\u0442\u044c"},{"exitgame",L"Quit",L"\u0412\u044b\u0445\u043e\u0434"},
 {"console",L"Console",L"\u041a\u043e\u043d\u0441\u043e\u043b\u044c"},{"screenshot",L"Screenshot",L"\u0421\u043d\u0438\u043c\u043e\u043a \u044d\u043a\u0440\u0430\u043d\u0430"},{"showfps",L"Show FPS",L"\u041f\u043e\u043a\u0430\u0437\u0430\u0442\u044c FPS"}
};
wstring HelpCaption(const string& command,bool russian) {
 for(const auto& entry:helpCommands) if(command==entry.command) return russian?entry.russian:entry.english;
 if(command.size()==6 && command.substr(0,5)=="hero_") return wstring(russian?L"\u0411\u043e\u0435\u0446 ":L"Soldier ")+wchar_t(command.back());
 wstring text(command.begin(),command.end());for(auto& c:text) if(c==L'_')c=L' ';
 if(!text.empty())text[0]=std::towupper(text[0]);return text;
}
wstring HelpKey(const string& key,bool ru) {
 if(key=="LCTRL" || key=="RCTRL")return L"Ctrl";
 if(key=="LSHIFT" || key=="RSHIFT")return L"Shift";
 if(key=="LALT" || key=="RALT")return L"Alt";
 if(key.substr(0,4)=="NUM_")return L"Numpad "+wstring(key.begin()+4,key.end());
 if(key=="MOUSE_AXIS_Z")return ru?L"\u041a\u043e\u043b\u0435\u0441\u043e \u043c\u044b\u0448\u0438":L"Mouse wheel";
 if(key=="MOUSE_AXIS_X")return ru?L"\u041c\u044b\u0448\u044c \u043f\u043e \u0433\u043e\u0440\u0438\u0437\u043e\u043d\u0442\u0430\u043b\u0438":L"Mouse horizontal";
 if(key=="MOUSE_AXIS_Y")return ru?L"\u041c\u044b\u0448\u044c \u043f\u043e \u0432\u0435\u0440\u0442\u0438\u043a\u0430\u043b\u0438":L"Mouse vertical";
 if(key=="MOUSE_BUTTON0")return ru?L"\u041b\u041a\u041c":L"Left mouse";
 if(key=="MOUSE_BUTTON1")return ru?L"\u041f\u041a\u041c":L"Right mouse";
 if(key=="MOUSE_BUTTON2")return ru?L"\u0421\u0440\u0435\u0434\u043d\u044f\u044f \u043a\u043d\u043e\u043f\u043a\u0430 \u043c\u044b\u0448\u0438":L"Middle mouse";
 return wstring(key.begin(),key.end());
}
bool VisiblePanel(CInterface* ui,const char* id) {auto* panel=ui->GetChildByID(id);return IsValid(panel)&&panel->GetStyle(STYLE_VISIBLE);}
void HidePanel(CInterface* ui,const char* id) {auto* panel=ui->GetChildByID(id);if(IsValid(panel))panel->SetStyle(STYLE_VISIBLE,false);}
}
void CInterface::ShowConvenienceOptions() {
 ResetMouseCapture();Panel(this,"convenience_settings",400);UpdateConveniencePanels();
}
void CInterface::ShowKeyboardHelp() {
 ResetMouseCapture();keyboardHelpPage=0;Panel(this,"keyboard_help",680);UpdateConveniencePanels();
}
void CInterface::UpdateConveniencePanels() {
 const bool ru=ConvenienceRussian();
 if(VisiblePanel(this,"convenience_settings")) {
  CWindow* panel=GetChildByID("convenience_settings");BringWindowToTop(panel);
  Label(panel,"title",12,GetDBString(4402)+L"<color=FF392619><left>"+(ru?L"<center>\u0423\u0434\u043e\u0431\u0441\u0442\u0432\u0430":L"<center>Convenience"));
  const auto toggle=[&](const char* id,int y,const char* variable,const wchar_t* en,const wchar_t* russian) {
   wstring text=ru?russian:en;text+=NGlobal::GetVar(variable,1).GetInt()?(ru?L": \u0432\u043a\u043b.":L": on"):(ru?L": \u0432\u044b\u043a\u043b.":L": off");
   Button(panel,id,y,GetDBString(4402)+L"<color=FF392619><left>"+text);
  };
  toggle("convenience_compare",58,"ui_weapon_comparison",L"Compare weapons",L"\u0421\u0440\u0430\u0432\u043d\u0435\u043d\u0438\u0435 \u043e\u0440\u0443\u0436\u0438\u044f");
  toggle("convenience_help",102,"ui_keyboard_help",L"F1 keyboard reference",L"\u0421\u043f\u0440\u0430\u0432\u043a\u0430 \u043f\u043e \u043a\u043b\u0430\u0432\u0438\u0448\u0430\u043c F1");
  int speed=NGlobal::GetVar("game_other_turn_speed",1).GetInt();
  wchar_t text[160];swprintf(text,160,ru?L"\u0427\u0443\u0436\u0438\u0435 \u0445\u043e\u0434\u044b: %ls":L"Other turns: %ls",speed==2?L"\u00d7" L"2":speed==3?L"\u00d7" L"3":speed==4?L"\u00d7" L"4":ru?L"\u043e\u0431\u044b\u0447\u043d\u0430\u044f \u0441\u043a\u043e\u0440\u043e\u0441\u0442\u044c":L"normal speed");
  Button(panel,"convenience_speed",146,GetDBString(4402)+L"<color=FF392619><left>"+text);
  Label(panel,"speed_hint",186,GetDBString(4402)+L"<color=FF392619><left>"+(ru?L"\u0422\u043e\u043b\u044c\u043a\u043e \u043e\u0434\u0438\u043d\u043e\u0447\u043d\u0430\u044f \u043f\u043e\u0448\u0430\u0433\u043e\u0432\u0430\u044f \u0438\u0433\u0440\u0430.":L"Single-player turn-based play only."));
#if defined(__APPLE__)
  toggle("convenience_numpad",220,"input_mac_numpad",L"Command + digits as numpad",L"Command + \u0446\u0438\u0444\u0440\u044b \u0432\u043c\u0435\u0441\u0442\u043e numpad");
#endif
  Button(panel,"convenience_keys",264,GetDBString(4402)+L"<color=FF392619><left>"+(ru?L"\u041a\u043b\u0430\u0432\u0438\u0448\u0438 \u0443\u043f\u0440\u0430\u0432\u043b\u0435\u043d\u0438\u044f\u2026":L"Keyboard reference\u2026"));
  Button(panel,"convenience_close",344,GetDBString(4402)+L"<color=FF392619><left>"+(ru?L"<center>\u0417\u0430\u043a\u0440\u044b\u0442\u044c":L"<center>Close"));
 }
 if(!VisiblePanel(this,"keyboard_help"))return;
 CWindow* panel=GetChildByID("keyboard_help");BringWindowToTop(panel);
 vector<string> commands;NInput::GetBoundCommands(&commands);
 vector<pair<wstring,wstring>> rows;
 for(const auto& command:commands) {
  list<NInput::SBind> binds;NInput::GetBind(command,&binds);
  for(const auto& bind:binds) {
   if(!bind.szSection.empty() && bind.szSection!="game" && bind.szSection!="menu")continue;
   if(bind.eType==NInput::MTYPE_EVENT_UP) continue; // release half of the same key
   wstring keys;
   for(const auto& key:bind.controlsSet) {if(!keys.empty()) keys+=L" + ";keys+=HelpKey(key,ru);}
   if(bind.eType==NInput::MTYPE_SLIDER_MINUS)keys=L"\u2212 "+keys;
   else if(bind.eType==NInput::MTYPE_SLIDER)keys=L"+ "+keys;
   const wstring caption=HelpCaption(command,ru);
   if(std::find(rows.begin(),rows.end(),make_pair(caption,keys))==rows.end())rows.emplace_back(caption,keys);
  }
 }
 const int perPage=12,pages=Max(1,(int(rows.size())+perPage-1)/perPage);
 keyboardHelpPage=Clamp(keyboardHelpPage,0,pages-1);
 wchar_t title[160];swprintf(title,160,ru?L"<center>\u041a\u043b\u0430\u0432\u0438\u0448\u0438 \u0443\u043f\u0440\u0430\u0432\u043b\u0435\u043d\u0438\u044f \u2014 %d/%d":L"<center>Keyboard reference \u2014 %d/%d",keyboardHelpPage+1,pages);
 Label(panel,"title",12,GetDBString(4402)+L"<color=FF392619><left>"+title);
 for(int i=0;i<perPage;++i) {
  char id[32];sprintf(id,"key_row_%d",i);const int index=keyboardHelpPage*perPage+i;
  const wstring text=index<int(rows.size())?rows[index].first+L": <color=FF7A321F>"+rows[index].second:L"";
  CText* line=dynamic_cast<CText*>(panel->GetChildByID(id));
  if(!line)line=new CText(SWindowInfo(panel,SPoint(16,57+i*38),SPoint(512,38),id,STYLE_ENABLED|STYLE_VISIBLE|STYLE_TRANSPARENT));
  line->SetText(GetDBString(4402)+L"<color=FF392619><left>"+L"<font size=14pt>"+text);
 }
#if defined(__APPLE__)
 Label(panel,"mac_hint",518,GetDBString(4402)+L"<color=FF392619><left>"+L"<font size=14pt>Command + 0\u20269 \u2192 numpad; Command + = / \u2212 \u2192 + / \u2212");
#else
 Label(panel,"hint",518,GetDBString(4402)+L"<color=FF392619><left>"+(ru?L"\u041f\u043e\u043a\u0430\u0437\u0430\u043d\u044b \u0442\u0435\u043a\u0443\u0449\u0438\u0435 \u043d\u0430\u0437\u043d\u0430\u0447\u0435\u043d\u0438\u044f \u043a\u043b\u0430\u0432\u0438\u0448.":L"Shows the current key bindings."));
#endif
 Button(panel,"keys_previous",562,GetDBString(4402)+L"<color=FF392619><left>"+(ru?L"<center>\u041f\u0440\u0435\u0434\u044b\u0434\u0443\u0449\u0430\u044f \u0441\u0442\u0440\u0430\u043d\u0438\u0446\u0430":L"<center>Previous page"));
 Button(panel,"keys_next",600,GetDBString(4402)+L"<color=FF392619><left>"+(ru?L"<center>\u0421\u043b\u0435\u0434\u0443\u044e\u0449\u0430\u044f \u0441\u0442\u0440\u0430\u043d\u0438\u0446\u0430":L"<center>Next page"));
 Button(panel,"keys_close",638,GetDBString(4402)+L"<color=FF392619><left>"+(ru?L"<center>\u0417\u0430\u043a\u0440\u044b\u0442\u044c (Esc / F1)":L"<center>Close (Esc / F1)"));
}
bool CInterface::HandleConvenienceMessage(const SEvent& event) {
 if(event.nEvent==EVENT_WINKEY) {
  if(VisiblePanel(this,"keyboard_help") && (event.nVal==27 || event.nVal==112)) {HidePanel(this,"keyboard_help");return true;}
  if(VisiblePanel(this,"convenience_settings") && event.nVal==27) {HidePanel(this,"convenience_settings");return true;}
  if(event.nVal==112 && NGlobal::GetVar("ui_keyboard_help",1).GetInt()) {
   // Respect a player's own F1 binding. The reference is also in Settings.
   vector<string> commands;NInput::GetBoundCommands(&commands);
   for(const auto& command:commands) {list<NInput::SBind> binds;NInput::GetBind(command,&binds);
    for(const auto& bind:binds) if(std::find(bind.controlsSet.begin(),bind.controlsSet.end(),string("F1"))!=bind.controlsSet.end())return false;}
   ShowKeyboardHelp();return true;
  }
 }
 if(event.nEvent!=EVENT_NOTIFY)return false;
 const auto& id=event.szID;
 const char* variable=id=="convenience_compare"?"ui_weapon_comparison":id=="convenience_help"?"ui_keyboard_help":id=="convenience_numpad"?"input_mac_numpad":nullptr;
 if(variable)NGlobal::SetVar(variable,NGlobal::GetVar(variable,1).GetInt()?0:1);
 else if(id=="convenience_speed") {const int speed=NGlobal::GetVar("game_other_turn_speed",1).GetInt();NGlobal::SetVar("game_other_turn_speed",speed>=1&&speed<4?speed+1:1);}
 else if(id=="convenience_keys")ShowKeyboardHelp();
 else if(id=="convenience_close")HidePanel(this,"convenience_settings");
 else if(id=="keys_close")HidePanel(this,"keyboard_help");
 else if(id=="keys_previous")--keyboardHelpPage;
 else if(id=="keys_next")++keyboardHelpPage;
 else return false;
 if(variable || id=="convenience_speed")NGlobal::SaveConfig(".\\cfg\\config.cfg");
 UpdateConveniencePanels();return true;
}
}

using NUI::CConveniencePanel;
REGISTER_SAVELOAD_CLASS(0x53324350,CConveniencePanel)
