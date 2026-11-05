#pragma once

#include <cmath>
#include <cstdint>
#include <iterator>
#include <map>

namespace NInput {
class TouchGestures {
public:
  struct Point { float x = 0, y = 0; };
  enum class Phase { Down, Move, Up, Cancel };
  enum class Action { None, Aim, Tap, Context, Camera };
  struct Gesture {
    Action action = Action::None;
    Point point{};
    Point pan{};
    float zoom = 0;
    float yaw = 0;
  };

  void Reset() { fingers.clear(); }

  // Points are measured in UI units, with the same scale on both axes.
  Gesture Update(Phase phase, std::uint64_t finger, Point point, std::uint64_t milliseconds,
                 bool cameraAllowed) {
    if (phase == Phase::Cancel) {
      Reset();
      return {};
    }
    if (phase == Phase::Down) {
      if (fingers.empty()) {
        start = point;
        pressedAt = milliseconds;
        startedOnMap = cameraAllowed;
        moved = multiple = false;
      } else {
        multiple = true;
      }
      fingers.emplace(finger, point);
      return multiple ? Gesture{} : Gesture{Action::Aim, point};
    }
    auto contact = fingers.find(finger);
    if (contact == fingers.end()) return {};
    if (phase == Phase::Up) {
      fingers.erase(contact);
      if (multiple || moved || Distance(point, start) >= dragThreshold) return {};
      const bool held = milliseconds >= pressedAt && milliseconds - pressedAt >= holdMilliseconds;
      return {held && startedOnMap && cameraAllowed ? Action::Context : Action::Tap, point};
    }

    const Point previous = contact->second;
    contact->second = point;
    if (!startedOnMap || !cameraAllowed || fingers.size() > 2) {
      moved = moved || Distance(point, start) >= dragThreshold;
      return {};
    }
    if (fingers.size() == 2) {
      const auto other = contact == fingers.begin() ? std::next(contact) : fingers.begin();
      const Point before = Delta(previous, other->second);
      const Point after = Delta(point, other->second);
      const float oldDistance = std::hypot(before.x, before.y);
      const float newDistance = std::hypot(after.x, after.y);
      Gesture gesture{Action::Camera, point, {(point.x - previous.x) / 2, (point.y - previous.y) / 2}};
      if (oldDistance >= dragThreshold && newDistance >= dragThreshold) {
        gesture.zoom = std::log(newDistance / oldDistance);
        gesture.yaw = std::atan2(before.x * after.y - before.y * after.x,
                                 before.x * after.x + before.y * after.y);
      }
      return gesture;
    }
    // After a pinch, the remaining finger cannot initiate a tap or a new drag.
    if (multiple) return {};
    if (!moved && Distance(point, start) < dragThreshold) return {};
    const Point pan = Delta(point, moved ? previous : start);
    moved = true;
    return {Action::Camera, point, pan};
  }

private:
  static constexpr float dragThreshold = 12;
  static constexpr std::uint64_t holdMilliseconds = 450;
  static Point Delta(Point a, Point b) { return {a.x - b.x, a.y - b.y}; }
  static float Distance(Point a, Point b) { return std::hypot(a.x - b.x, a.y - b.y); }

  std::map<std::uint64_t, Point> fingers;
  Point start;
  std::uint64_t pressedAt = 0;
  bool startedOnMap = false;
  bool moved = false;
  bool multiple = false;
};
}
