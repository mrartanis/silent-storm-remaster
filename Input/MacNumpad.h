#pragma once
#include <SDL3/SDL_scancode.h>
#include <array>
namespace NInput {
// Only numpad aliases; no platform-wide save/load/menu shortcuts.
inline SDL_Scancode MacNumpadAlias(SDL_Scancode scan,bool command,bool enabled) {
    if(!command || !enabled)return scan;
    if(scan>=SDL_SCANCODE_1 && scan<=SDL_SCANCODE_9)
        return SDL_Scancode(SDL_SCANCODE_KP_1+scan-SDL_SCANCODE_1);
    switch(scan) {
        case SDL_SCANCODE_0:return SDL_SCANCODE_KP_0;
        case SDL_SCANCODE_EQUALS:return SDL_SCANCODE_KP_PLUS;
        case SDL_SCANCODE_MINUS:return SDL_SCANCODE_KP_MINUS;
        case SDL_SCANCODE_PERIOD:return SDL_SCANCODE_KP_PERIOD;
        default:return scan;
    }
}
class MacNumpadState {
    std::array<SDL_Scancode,SDL_SCANCODE_COUNT> routes{};
    std::array<unsigned,SDL_SCANCODE_COUNT> held{};
public:
    void Reset() {routes.fill(SDL_SCANCODE_UNKNOWN);held.fill(0);}
    bool Route(SDL_Scancode physical,bool down,bool command,bool enabled,bool repeat,SDL_Scancode* mapped) {
        if(physical<=SDL_SCANCODE_UNKNOWN || physical>=SDL_SCANCODE_COUNT)return false;
        // Text edits still receive repeated virtual-key events. Repeats never
        // acquire another physical hold or change an established alias.
        if(repeat) {*mapped=routes[physical];return down && *mapped!=SDL_SCANCODE_UNKNOWN;}
        if(down) {
            if(routes[physical]!=SDL_SCANCODE_UNKNOWN)return false;
            *mapped=MacNumpadAlias(physical,command,enabled);routes[physical]=*mapped;
            return held[*mapped]++==0;
        }
        *mapped=routes[physical];routes[physical]=SDL_SCANCODE_UNKNOWN;
        return *mapped!=SDL_SCANCODE_UNKNOWN && held[*mapped] && --held[*mapped]==0;
    }
};
}
