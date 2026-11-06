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
  touch.Update(Phase::Move, 1, {10, 21}, 300, true);
  Check(!touch.PollHold(649), "A hold should wait for the threshold.");
  const auto hold = touch.PollHold(650);
  Check(hold && hold->x == 10 && hold->y == 21,
        "A hold should open the menu at the finger before release.");
  Check(!touch.PollHold(690) && touch.Update(Phase::Move, 1, {40, 21}, 700, true).action == Action::None,
        "A recognized hold should fire once and consume the rest of the gesture.");
  Check(touch.Update(Phase::Up, 1, {40, 21}, 710, true).action == Action::None,
        "Releasing the finger should not select a menu item.");

  touch.Update(Phase::Down, 1, {10, 20}, 720, true);
  Check(touch.Update(Phase::Up, 1, {10, 20}, 1190, true).action == Action::None && !touch.PollHold(1190),
        "If the input loop misses a hold, its release must not become a tap.");

  touch.Update(Phase::Down, 1, {10, 20}, 1200, false);
  Check(!touch.PollHold(1660) && touch.Update(Phase::Up, 1, {10, 21}, 1700, true).action == Action::Tap,
        "Holding an interface control should issue a primary click.");

  touch.Update(Phase::Down, 1, {10, 20}, 1500, true);
  const auto pan = touch.Update(Phase::Move, 1, {40, 20}, 1510, true);
  Check(pan.action == Action::Camera && pan.pan.x == 30,
        "Dragging should pan the camera.");
  Check(!touch.PollHold(1960), "A drag must never become a context hold.");
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

  int movie = 0, dialogue = 0;
  touch.Reset();
  touch.Update(Phase::Down, 1, {40, 60}, 3000, false, &movie);
  Check(!touch.PollHold(3450) && !touch.PollSkip(3999, &movie),
        "A movie hold should wait two seconds and never open the map menu.");
  touch.Update(Phase::Move, 1, {41, 60}, 4000, false, &movie);
  const auto progress = touch.GetSkipProgress(4000, &movie);
  Check(progress && progress->fraction == 0.5f && progress->point.x == 41 && progress->point.y == 60,
        "The progress ring should follow small finger movements and report elapsed time.");
  Check(!touch.PollSkip(4999, &movie) && touch.PollSkip(5000, &movie) &&
        !touch.PollSkip(5001, &movie) && !touch.GetSkipProgress(5001, &movie),
        "A skip should fire at two seconds, once per hold, and hide the ring.");
  Check(touch.Update(Phase::Up, 1, {41, 60}, 5010, false, &movie).action == Action::None,
        "Releasing a completed skip should not click.");

  touch.Update(Phase::Down, 1, {40, 60}, 6000, false, &dialogue);
  Check(touch.Update(Phase::Up, 1, {40, 60}, 6100, false, &dialogue).action == Action::Tap,
        "A short tap should still operate dialogue buttons.");
  touch.Update(Phase::Down, 1, {40, 60}, 6200, false, &movie);
  Check(touch.Update(Phase::Up, 1, {40, 60}, 8250, false, &movie).action == Action::None,
        "A late release after a stalled frame should not click.");

  touch.Update(Phase::Down, 1, {40, 60}, 8300, false, &movie);
  Check(!touch.PollSkip(9000, &dialogue) && !touch.GetSkipProgress(9500, &movie) &&
        touch.Update(Phase::Up, 1, {40, 60}, 9600, false, &dialogue).action == Action::None,
        "A screen transition should cancel the hold and consume its release.");

  touch.Update(Phase::Down, 1, {40, 60}, 9700, false, &dialogue);
  touch.Update(Phase::Move, 1, {60, 60}, 9800, false, &dialogue);
  Check(!touch.GetSkipProgress(10000, &dialogue) && !touch.PollSkip(11700, &dialogue),
        "Dragging should dismiss the skip ring and cancel the hold.");
  touch.Reset();
  touch.Update(Phase::Down, 1, {40, 60}, 11800, false, &dialogue);
  touch.Update(Phase::Down, 2, {65, 60}, 11900, false, &dialogue);
  touch.Update(Phase::Up, 2, {65, 60}, 12000, false, &dialogue);
  Check(!touch.GetSkipProgress(12500, &dialogue) && !touch.PollSkip(13800, &dialogue),
        "A second finger should cancel the skip even after it leaves.");
  touch.Reset();
  Check(!touch.GetSkipProgress(14000, &dialogue) && !touch.PollSkip(14000, &dialogue),
        "Losing focus should discard the skip and its progress.");
}
