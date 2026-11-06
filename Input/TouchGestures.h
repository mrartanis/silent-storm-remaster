#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <map>
#include <optional>

namespace NInput {
class TouchGestures {
public:
  struct Point { float x = 0, y = 0; };
  struct SkipProgress { Point point; float fraction; };
  enum class Phase { Down, Move, Up, Cancel };
  enum class Action { None, Aim, Tap, Camera };
  struct Gesture {
    Action action = Action::None;
    Point point{};
    Point pan{};
    float zoom = 0;
    float yaw = 0;
    float tilt = 0;
  };

  void Reset() { fingers.clear(); }

  std::optional<Point> PollHold(std::uint64_t milliseconds) {
    if (fingers.size() != 1 || !startedOnMap || moved || multiple || holdSent ||
        Elapsed(milliseconds) < contextHoldMilliseconds)
      return {};
    holdSent = true;
    return fingers.begin()->second;
  }

  std::optional<SkipProgress> GetSkipProgress(std::uint64_t milliseconds, const void* target) const {
    if (!target || target != skipTarget || fingers.size() != 1 || moved || multiple || holdSent)
      return {};
    const float fraction = static_cast<float>(Elapsed(milliseconds)) / skipHoldMilliseconds;
    return SkipProgress{fingers.begin()->second, (std::min)(fraction, 1.0f)};
  }

  bool PollSkip(std::uint64_t milliseconds, const void* target) {
    if (skipTarget && target != skipTarget)
      holdSent = true;  // The gesture belongs to the screen that received the press.
    const auto progress = GetSkipProgress(milliseconds, target);
    if (!progress || Elapsed(milliseconds) < skipHoldMilliseconds)
      return false;
    holdSent = true;
    return true;
  }

  // Points are measured in UI units, with the same scale on both axes.
  Gesture Update(Phase phase, std::uint64_t finger, Point point, std::uint64_t milliseconds,
                 bool cameraAllowed, const void* target = nullptr) {
    if (phase == Phase::Cancel) {
      Reset();
      return {};
    }
    if (phase == Phase::Down) {
      if (fingers.empty()) {
        start = point;
        pressedAt = milliseconds;
        skipTarget = target;
        startedOnMap = cameraAllowed && !skipTarget;
        moved = multiple = holdSent = false;
      } else {
        multiple = true;
        startedOnMap = startedOnMap && cameraAllowed;
      }
      fingers.emplace(finger, point);
      if (fingers.size() == 2) BeginPair();
      return multiple ? Gesture{} : Gesture{Action::Aim, point};
    }
    auto contact = fingers.find(finger);
    if (contact == fingers.end()) return {};
    if (phase == Phase::Up) {
      fingers.erase(contact);
      if (fingers.size() == 2) BeginPair();
      // An overdue hold may have been released while the game missed a frame.
      // Even then, it must never turn into a primary click on the map.
      if (multiple || moved || holdSent || Distance(point, start) >= dragThreshold ||
          (startedOnMap && Elapsed(milliseconds) >= contextHoldMilliseconds) ||
          (skipTarget && (target != skipTarget || Elapsed(milliseconds) >= skipHoldMilliseconds))) return {};
      return {Action::Tap, point};
    }

    if (holdSent) return {};
    const Point previous = contact->second;
    contact->second = point;
    if (!startedOnMap || !cameraAllowed || fingers.size() > 2) {
      moved = moved || Distance(point, start) >= dragThreshold;
      return {};
    }
    if (fingers.size() == 2)
      return UpdatePair(point, milliseconds);
    // After a pinch, the remaining finger cannot initiate a tap or a new drag.
    if (multiple) return {};
    if (!moved && Distance(point, start) < dragThreshold) return {};
    const Point pan = Delta(point, moved ? previous : start);
    moved = true;
    return {Action::Camera, point, pan};
  }

private:
  using Pair = std::array<Point, 2>;
  enum class PairMode { Pending, Tilt, Transform };
  static constexpr float dragThreshold = 12;
  static constexpr std::uint64_t contextHoldMilliseconds = 450;
  static constexpr std::uint64_t skipHoldMilliseconds = 2000;
  static constexpr std::uint64_t pairDecisionMilliseconds = 48;
  static Point Delta(Point a, Point b) { return {a.x - b.x, a.y - b.y}; }
  static float Distance(Point a, Point b) { return std::hypot(a.x - b.x, a.y - b.y); }
  Pair CurrentPair() const { return {fingers.begin()->second, fingers.rbegin()->second}; }
  std::uint64_t Elapsed(std::uint64_t milliseconds) const {
    return milliseconds >= pressedAt ? milliseconds - pressedAt : 0;
  }

  void BeginPair() {
    pair = CurrentPair();
    pairMode = PairMode::Pending;
    firstPairMotion = 0;
  }

  Gesture UpdatePair(Point point, std::uint64_t milliseconds) {
    const Pair current = CurrentPair();
    const Point a = Delta(current[0], pair[0]);
    const Point b = Delta(current[1], pair[1]);
    const Point center{(a.x + b.x) / 2, (a.y + b.y) / 2};
    if (pairMode == PairMode::Pending) {
      const float aTravel = Distance(a, {});
      const float bTravel = Distance(b, {});
      if ((std::max)(aTravel, bTravel) < dragThreshold) return {};
      if (!firstPairMotion) firstPairMotion = milliseconds;

      // SDL reports a separate move for each finger. Decide using both fingers'
      // displacement from touch-down so a vertical drag cannot start as a twist.
      const float vertical = std::abs(center.y);
      if (vertical >= dragThreshold && std::abs(center.x) < 0.65f * vertical &&
          Distance(a, b) < vertical) {
        pairMode = PairMode::Tilt;
      } else if ((std::min)(aTravel, bTravel) >= dragThreshold / 2 &&
                 (a.y * b.y <= 0 || std::abs(center.x) >= vertical)) {
        pairMode = PairMode::Transform;
      } else if (milliseconds - firstPairMotion >= pairDecisionMilliseconds) {
        pairMode = PairMode::Transform;
      } else {
        return {};
      }
    }

    Gesture gesture{Action::Camera, point};
    if (pairMode == PairMode::Tilt) {
      gesture.tilt = center.y;
    } else {
      gesture.pan = center;
      const Point before = Delta(pair[1], pair[0]);
      const Point after = Delta(current[1], current[0]);
      const float oldDistance = Distance(before, {});
      const float newDistance = Distance(after, {});
      if (oldDistance >= dragThreshold && newDistance >= dragThreshold) {
        gesture.zoom = std::log(newDistance / oldDistance);
        gesture.yaw = std::atan2(before.x * after.y - before.y * after.x,
                                 before.x * after.x + before.y * after.y);
      }
    }
    pair = current;
    return gesture;
  }

  std::map<std::uint64_t, Point> fingers;
  Pair pair{};
  PairMode pairMode = PairMode::Pending;
  std::uint64_t firstPairMotion = 0;
  Point start;
  std::uint64_t pressedAt = 0;
  const void* skipTarget = nullptr;
  bool startedOnMap = false;
  bool moved = false;
  bool multiple = false;
  bool holdSent = false;
};
}
