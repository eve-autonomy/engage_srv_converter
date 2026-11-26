// Copyright 2026 eve autonomy inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License

#ifndef ENGAGE_SRV_CONVERTER__DOUBLE_BUTTON_JUDGE_HPP_
#define ENGAGE_SRV_CONVERTER__DOUBLE_BUTTON_JUDGE_HPP_

#include <algorithm>
#include <utility>
#include <vector>

namespace engage_srv_converter
{

// No default values are defined here: every value comes from config/engage_srv_converter.param.yaml.
struct DoubleButtonParams
{
  // [A] Valid press time: min_press_sec <= duration <= max_press_sec
  //     ([C] shorter than min_press_sec / [B] longer than max_press_sec are invalid)
  double min_press_sec;
  double max_press_sec;
  // Minimum time both buttons must be ON at the same time (TBD: not decided yet)
  double min_overlap_sec;
};

struct DoubleButtonVerdict
{
  // true when a judgement was made at this call (both buttons became OFF)
  bool judged{false};
  // true when the engage is accepted
  bool accepted{false};
  // overlapped time of the accepted pair
  double overlap_sec{0.0};
};

/**
 * Judge for the double button engage (engage button + delivery reservation button).
 *
 * A "session" starts when one of the buttons becomes ON and ends when both buttons are OFF.
 * The judgement is made at the end of a session: the engage is accepted if the ON periods
 * of the two buttons overlap and the duration of each press is within [min, max] sec.
 *
 * This class does not depend on ROS, time is injected by the caller.
 */
class DoubleButtonJudge
{
public:
  enum Button {ENGAGE = 0, RESERVATION = 1};

  explicit DoubleButtonJudge(const DoubleButtonParams & params)
  : params_(params)
  {
  }

  DoubleButtonVerdict onButton(const Button button, const bool pressed, const double now_sec)
  {
    DoubleButtonVerdict verdict;
    State & s = state_[button];

    if (pressed && !s.pressed) {
      s.pressed = true;
      s.start = now_sec;
    } else if (!pressed && s.pressed) {
      s.pressed = false;
      s.intervals.emplace_back(s.start, now_sec);
    } else {
      return verdict;  // no change
    }

    if (!state_[ENGAGE].pressed && !state_[RESERVATION].pressed) {
      verdict = judge();
      state_[ENGAGE].intervals.clear();
      state_[RESERVATION].intervals.clear();
    }
    return verdict;
  }

private:
  using Interval = std::pair<double, double>;
  struct State
  {
    bool pressed{false};
    double start{0.0};
    std::vector<Interval> intervals;
  };

  bool isValidDuration(const Interval & i) const
  {
    const double d = i.second - i.first;
    return d >= params_.min_press_sec && d <= params_.max_press_sec;
  }

  DoubleButtonVerdict judge() const
  {
    DoubleButtonVerdict verdict;
    verdict.judged = true;
    for (const auto & e : state_[ENGAGE].intervals) {
      for (const auto & r : state_[RESERVATION].intervals) {
        if (!isValidDuration(e) || !isValidDuration(r)) {
          continue;
        }
        const double overlap = std::min(e.second, r.second) - std::max(e.first, r.first);
        if (overlap > 0.0 && overlap >= params_.min_overlap_sec) {
          verdict.accepted = true;
          verdict.overlap_sec = std::max(verdict.overlap_sec, overlap);
        }
      }
    }
    return verdict;
  }

  DoubleButtonParams params_;
  State state_[2];
};

}  // namespace engage_srv_converter

#endif  // ENGAGE_SRV_CONVERTER__DOUBLE_BUTTON_JUDGE_HPP_
