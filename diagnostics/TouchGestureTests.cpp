#include "../Input/TouchGestures.h"
#include <cstdio>
#include <cstdlib>

namespace {
void Check(bool condition, const char* message) {
  if (condition) return;
  std::fprintf(stderr, "%s\n", message);
  std::exit(1);
}
}

int main() {
  NInput::TouchGestures touch;
  using Phase = NInput::TouchGestures::Phase;
  using Action = NInput::TouchGestures::Action;

  Check(touch.Update(Phase::Down, 1, {10, 20}, 100, true).action == Action::Aim,
        "The first contact should position the pointer without clicking.");
  Check(touch.Update(Phase::Up, 1, {11, 20}, 150, true).action == Action::Tap,
        "A short tap should issue a primary click.");

  touch.Update(Phase::Down, 1, {10, 20}, 200, true);
  Check(touch.Update(Phase::Up, 1, {10, 21}, 700, true).action == Action::Context,
        "A stationary hold on the map should open the context menu.");

  touch.Update(Phase::Down, 1, {10, 20}, 800, false);
  Check(touch.Update(Phase::Up, 1, {10, 21}, 1400, true).action == Action::Tap,
        "Holding an interface control should issue a primary click.");

  touch.Update(Phase::Down, 1, {10, 20}, 1500, true);
  const auto pan = touch.Update(Phase::Move, 1, {40, 20}, 1510, true);
  Check(pan.action == Action::Camera && pan.pan.x == 30,
        "Dragging should pan the camera.");
  Check(touch.Update(Phase::Up, 1, {10, 20}, 1600, true).action == Action::None,
        "Dragging back to the start should never issue a click.");

  touch.Update(Phase::Down, 1, {10, 20}, 1700, true);
  touch.Update(Phase::Down, 2, {50, 20}, 1710, true);
  touch.Update(Phase::Move, 1, {0, 10}, 1720, true);
  const auto pinch = touch.Update(Phase::Move, 2, {75, 30}, 1720, true);
  Check(pinch.action == Action::Camera && pinch.zoom > 0 && pinch.yaw > 0 && pinch.tilt == 0,
        "A spreading and rotating pair should zoom and turn without tilting.");
  Check(touch.Update(Phase::Up, 2, {75, 30}, 1730, true).action == Action::None &&
        touch.Update(Phase::Up, 1, {0, 10}, 1740, true).action == Action::None,
        "Releasing either finger after a pinch should never issue a click.");

  touch.Update(Phase::Down, 1, {10, 20}, 1800, true);
  touch.Update(Phase::Cancel, 1, {10, 20}, 1810, true);
  Check(touch.Update(Phase::Up, 1, {10, 20}, 1850, true).action == Action::None,
        "Cancellation should discard pending clicks.");

  touch.Update(Phase::Down, 1, {10, 90}, 2000, true);
  touch.Update(Phase::Down, 2, {90, 90}, 2020, true);
  Check(touch.Update(Phase::Move, 1, {12, 65}, 2050, true).action == Action::None,
        "An early move from only one finger should wait for the pair.");
  const auto tilt = touch.Update(Phase::Move, 2, {92, 65}, 2050, true);
  Check(tilt.action == Action::Camera && tilt.tilt == -25 &&
        tilt.pan.x == 0 && tilt.pan.y == 0 && tilt.zoom == 0 && tilt.yaw == 0,
        "Sliding both fingers up should tilt without panning, zooming or turning.");
  const auto reverse = touch.Update(Phase::Move, 2, {96, 80}, 2100, true);
  Check(reverse.tilt > 0 && reverse.zoom == 0 && reverse.yaw == 0,
        "A recognized tilt should keep its mode when one finger leads.");
  Check(touch.Update(Phase::Up, 1, {12, 65}, 2120, true).action == Action::None &&
        touch.Update(Phase::Up, 2, {96, 80}, 2130, true).action == Action::None,
        "Releasing a tilt should never click on the map.");

  touch.Update(Phase::Down, 1, {20, 40}, 2200, true);
  touch.Update(Phase::Down, 2, {60, 40}, 2210, true);
  touch.Update(Phase::Move, 2, {80, 40}, 2220, true);
  const auto anchoredPinch = touch.Update(Phase::Move, 2, {100, 40}, 2280, true);
  Check(anchoredPinch.zoom > 0 && anchoredPinch.tilt == 0,
        "An anchored pinch should still work when only one finger moves.");
}
