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

#include <gtest/gtest.h>

#include <utility>

#include "engage_srv_converter/double_button_judge.hpp"

using engage_srv_converter::DoubleButtonJudge;
using engage_srv_converter::DoubleButtonVerdict;

namespace
{
constexpr auto E = DoubleButtonJudge::ENGAGE;
constexpr auto R = DoubleButtonJudge::RESERVATION;

struct Press
{
  DoubleButtonJudge::Button button;
  double on;
  double off;
};

// Same values as config/engage_srv_converter.param.yaml
engage_srv_converter::DoubleButtonParams testParams()
{
  engage_srv_converter::DoubleButtonParams params;
  params.min_press_sec = 1.0;
  params.max_press_sec = 15.0;
  params.min_overlap_sec = 0.0;
  return params;
}

// Plays the given presses in time order and returns the verdict of the last event.
DoubleButtonVerdict play(const std::vector<Press> & presses)
{
  struct Ev {double t; DoubleButtonJudge::Button b; bool p;};
  std::vector<Ev> evs;
  for (const auto & p : presses) {
    evs.push_back({p.on, p.button, true});
    evs.push_back({p.off, p.button, false});
  }
  std::stable_sort(evs.begin(), evs.end(), [](const Ev & a, const Ev & b) {return a.t < b.t;});
  DoubleButtonJudge judge(testParams());
  DoubleButtonVerdict last;
  for (const auto & e : evs) {
    const auto v = judge.onButton(e.b, e.p, e.t);
    if (v.judged) {
      last = v;
    }
  }
  return last;
}
}  // namespace

// T-5: both 3 s
TEST(DoubleButtonJudge, BothPressedAccepted) {EXPECT_TRUE(play({{E, 0, 3}, {R, 0, 3}}).accepted);}
// T-6 / T-7: boundary values (A includes 1 s and 15 s)
TEST(DoubleButtonJudge, Boundary1s) {EXPECT_TRUE(play({{E, 0, 1.0}, {R, 0, 1.0}}).accepted);}
TEST(DoubleButtonJudge, Boundary15s) {EXPECT_TRUE(play({{E, 0, 15.0}, {R, 0, 15.0}}).accepted);}
// T-8 / T-9 (C)
TEST(DoubleButtonJudge, Short05s) {EXPECT_FALSE(play({{E, 0, 0.5}, {R, 0, 0.5}}).accepted);}
TEST(DoubleButtonJudge, Short09s) {EXPECT_FALSE(play({{E, 0, 0.9}, {R, 0, 0.9}}).accepted);}
// T-10 (B)
TEST(DoubleButtonJudge, Long16s) {EXPECT_FALSE(play({{E, 0, 16}, {R, 0, 16}}).accepted);}
// T-11..T-16
TEST(DoubleButtonJudge, EngageTooLong) {EXPECT_FALSE(play({{E, 0, 20}, {R, 5, 10}}).accepted);}
TEST(DoubleButtonJudge, ReservationTooLong) {EXPECT_FALSE(play({{E, 5, 10}, {R, 0, 20}}).accepted);}
TEST(DoubleButtonJudge, BothTooLong) {EXPECT_FALSE(play({{E, 0, 20}, {R, 0, 20}}).accepted);}
TEST(DoubleButtonJudge, EngageTooShort) {EXPECT_FALSE(play({{E, 4.5, 5.0}, {R, 0, 5}}).accepted);}
TEST(DoubleButtonJudge, ReservationTooShort) {EXPECT_FALSE(play({{E, 0, 5}, {R, 4.5, 5.0}}).accepted);}
TEST(DoubleButtonJudge, BothTooShort) {EXPECT_FALSE(play({{E, 0, 0.5}, {R, 0, 0.5}}).accepted);}
// T-2 / T-3: a single button
TEST(DoubleButtonJudge, EngageOnly) {EXPECT_FALSE(play({{E, 0, 3}}).accepted);}
TEST(DoubleButtonJudge, ReservationOnly) {EXPECT_FALSE(play({{R, 0, 3}}).accepted);}
// T-17 / T-18: the ON periods do not overlap
TEST(DoubleButtonJudge, EngageThenReservation) {EXPECT_FALSE(play({{E, 0, 3}, {R, 4, 7}}).accepted);}
TEST(DoubleButtonJudge, ReservationThenEngage) {EXPECT_FALSE(play({{R, 0, 3}, {E, 4, 7}}).accepted);}
TEST(DoubleButtonJudge, TouchingButNotOverlapping) {EXPECT_FALSE(play({{E, 0, 3}, {R, 3, 6}}).accepted);}
// T-19: start timing is shifted, but the ON periods overlap
TEST(DoubleButtonJudge, ShiftedStartAccepted)
{
  const auto v = play({{E, 0, 6}, {R, 1, 6}});
  EXPECT_TRUE(v.accepted);
  EXPECT_NEAR(v.overlap_sec, 5.0, 1e-9);
}
// T-21: the second engage is judged again
TEST(DoubleButtonJudge, SecondJudgement)
{
  DoubleButtonJudge j(testParams());
  j.onButton(E, true, 0); j.onButton(R, true, 0);
  j.onButton(E, false, 3);
  EXPECT_TRUE(j.onButton(R, false, 3).accepted);
  j.onButton(E, true, 100);
  const auto v = j.onButton(E, false, 103);
  EXPECT_TRUE(v.judged);
  EXPECT_FALSE(v.accepted);
}
// The judgement is made only when both buttons are OFF
TEST(DoubleButtonJudge, NotJudgedWhileOneButtonIsOn)
{
  DoubleButtonJudge j(testParams());
  j.onButton(E, true, 0); j.onButton(R, true, 0);
  EXPECT_FALSE(j.onButton(E, false, 3).judged);
  EXPECT_TRUE(j.onButton(R, false, 4).judged);
}
// A stuck engage button (shorted) never forms a valid pair
TEST(DoubleButtonJudge, StuckEngageButtonNeverAccepts)
{
  DoubleButtonJudge j(testParams());
  j.onButton(E, true, 0);
  j.onButton(R, true, 50);
  const auto v = j.onButton(R, false, 53);
  EXPECT_FALSE(v.judged);
  EXPECT_FALSE(j.onButton(E, false, 60).accepted);
}
