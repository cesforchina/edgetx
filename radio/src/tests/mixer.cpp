/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "gtests.h"
#include "hal/adc_driver.h"

class TrimsTest : public OpenTxTest {};
class MixerTest : public OpenTxTest {};

#define CHECK_NO_MOVEMENT(channel, value, duration) \
    for (int i=1; i<=(duration); i++) { \
      evalFlightModeMixes(e_perout_mode_normal, 1); \
      GTEST_ASSERT_EQ((value), chans[(channel)]); \
    }

#define CHECK_SLOW_MOVEMENT(channel, sign, duration) \
    do { \
    for (int i=1; i<=(duration); i++) { \
      evalFlightModeMixes(e_perout_mode_normal, 1); \
      lastAct = lastAct + (sign) * (1<<19)/500; /* 100 on ARM */ \
      GTEST_ASSERT_EQ(256 * (lastAct >> 8), chans[(channel)]); \
    } \
    } while (0)

#define CHECK_DELAY(channel, duration) \
    do { \
      int32_t value = chans[(channel)]; \
      for (int i=1; i<=(duration); i++) { \
        evalFlightModeMixes(e_perout_mode_normal, 1); \
        GTEST_ASSERT_EQ(chans[(channel)], value); \
      } \
    } while (0)

#define CHECK_FLIGHT_MODE_TRANSITION(channel, duration, initValue, endValue) \
    do { \
      uint32_t delta = 0xffff / duration; \
      int32_t weightInit = 0xffff; \
      int32_t weightEnd = 0; \
      for (int i = 0; i <= (duration); i++) { \
        evalMixes(1); \
        GTEST_ASSERT_LE( abs(((initValue) * weightInit + (endValue) * weightEnd) / 0xffff - channelOutputs[(channel)]), 1); \
        weightInit = weightInit - delta; \
        weightEnd = weightEnd + delta; \
      } \
      for (int i = 0; i < 100; i++) { /* be sure the transition is finished*/ \
        evalMixes(1); \
      } \
    } while (0)

TEST_F(TrimsTest, throttleTrim)
{
  g_model.thrTrim = 1;
  // stick max + trim max
  anaSetFiltered(THR_STICK, +1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  // stick max + trim min
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  // stick min + trim max
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+512);
  // stick min + trim mid
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+256);
  // stick min + trim min
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024);

  // now the same tests with extended Trims
  g_model.extendedTrims = 1;
  // stick max + trim max
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  // stick max + trim min
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  // stick min + trim max
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+2048);
  // stick min + trim mid
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+1024);
  // stick min + trim min
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024);

}

TEST_F(TrimsTest, invertedThrottlePlusThrottleTrim)
{
  g_model.throttleReversed = 1;
  g_model.thrTrim = 1;
  // stick max + trim max
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024);
  // stick max + trim mid
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+256);
  // stick max + trim min
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+512);
  // stick min + trim max
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], +1024);
  // stick min + trim min
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], +1024);

  // now the same tests with extended Trims
  g_model.extendedTrims = 1;
  // stick max + trim max
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024);
  // stick max + trim mid
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+1024);
  // stick max + trim min
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+2048);
  // stick min + trim max
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], +1024);
  // stick min + trim min
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], +1024);
}

TEST_F(TrimsTest, throttleTrimWithZeroWeightOnThrottle)
{
  g_model.thrTrim = 1;
  // the input already exists
  ExpoData *expo = expoAddress(THR_STICK);
  expo->weight = 0;
  // stick max + trim max
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 256);
  // stick max + trim mid
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_LE(abs(channelOutputs[2] - 128), 1);  //can't use precise comparison here because of lower precision math on 9X
  // stick max + trim min
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);
  // stick min + trim max
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 256);
  // stick min + trim mid
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_LE(abs(channelOutputs[2] - 128), 1);
  // stick min + trim min
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);

  // now some tests with extended Trims
  g_model.extendedTrims = 1;
  // trim min + various stick positions = should always be same value
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  anaSetFiltered(THR_STICK,  -1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);
  anaSetFiltered(THR_STICK,  -300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);
  anaSetFiltered(THR_STICK,  +300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);
  anaSetFiltered(THR_STICK,  +1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);

  // trim max + various stick positions = should always be same value
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  anaSetFiltered(THR_STICK,  -1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  anaSetFiltered(THR_STICK,  -300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  anaSetFiltered(THR_STICK,  +300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  anaSetFiltered(THR_STICK,  +1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
}

TEST_F(TrimsTest, invertedThrottlePlusthrottleTrimWithZeroWeightOnThrottle)
{
  g_model.throttleReversed = 1;
  g_model.thrTrim = 1;
  // the input already exists
  ExpoData *expo = expoAddress(THR_STICK);
  expo->weight = 0;
  // stick max + trim max
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);
  // stick max + trim mid
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_LE(abs(channelOutputs[2] - 128), 1);
  // stick max + trim min
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 256);
  // stick min + trim max
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);
  // stick min + trim mid
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, 0);
  evalMixes(1);
  EXPECT_LE(abs(channelOutputs[2] - 128), 1);
  // stick min + trim min
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, THR_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 256);

  // now some tests with extended Trims
  g_model.extendedTrims = 1;
  // trim min + various stick positions = should always be same value
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MIN);
  anaSetFiltered(THR_STICK,  -1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  anaSetFiltered(THR_STICK,  -300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  anaSetFiltered(THR_STICK,  +300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  anaSetFiltered(THR_STICK,  +1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);

  // trim max + various stick positions = should always be same value
  setTrimValue(0, THR_STICK, TRIM_EXTENDED_MAX);
  anaSetFiltered(THR_STICK,  -1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);
  anaSetFiltered(THR_STICK,  -300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);
  anaSetFiltered(THR_STICK,  +300);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);
  anaSetFiltered(THR_STICK,  +1024);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 0);
}

TEST_F(TrimsTest, CopyTrimsToOffset)
{
  setTrimValue(0, ELE_STICK, -100); // -100 on elevator
  evalFunctions(g_model.customFn, modelFunctionsContext); // it disables all safety channels
  copyTrimsToOffset(1);
  EXPECT_EQ(getTrimValue(0, ELE_STICK), -100); // unchanged
  EXPECT_EQ(g_model.limitData[1].offset, -195);
}

TEST_F(TrimsTest, CopySticksToOffset)
{
  anaSetFiltered(ELE_STICK, -100);
  evalMixes(1);
  copySticksToOffset(1);
#if defined(STICK_DEAD_ZONE)
  EXPECT_EQ(g_model.limitData[1].offset, -93);
#else
  EXPECT_EQ(g_model.limitData[1].offset, -97);
#endif
}

TEST_F(TrimsTest, MoveTrimsToOffsets)
{
  // No trim idle only
  g_model.thrTrim = 0;
  anaSetFiltered(THR_STICK,  0);
  setTrimValue(0, MIXSRC_TrimThr - MIXSRC_FIRST_TRIM, 100);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, -100);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 200);  // THR output value is reflecting 100 trim
  moveTrimsToOffsets();
  EXPECT_EQ(getTrimValue(0, MIXSRC_TrimThr - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(g_model.limitData[2].offset, 195); // value transferred
  EXPECT_EQ(getTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(g_model.limitData[1].offset, -195); // value transferred
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 200); // THR output value is still reflecting 100 trim
}

TEST_F(TrimsTest, MoveTrimsToOffsetsWithTrimIdle)
{
  // Trim idle only
  g_model.thrTrim = 1;
  anaSetFiltered(THR_STICK,  -1024);  // Min stick
  g_model.limitData[2].offset = 0;
  g_model.limitData[1].offset = 0;
  setTrimValue(0, MIXSRC_TrimThr - MIXSRC_FIRST_TRIM, 100);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, -100);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -568);  // THR output value is reflecting 100 trim idle
  moveTrimsToOffsets();

  // Trim affecting Throttle should not be affected
  EXPECT_EQ(getTrimValue(0, MIXSRC_TrimThr - MIXSRC_FIRST_TRIM), 100);  // unchanged
  EXPECT_EQ(g_model.limitData[2].offset, 0); // unchanged

  // Other trims should
  EXPECT_EQ(getTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(g_model.limitData[1].offset, -195); // value transferred
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -568);  // THR output value is still reflecting 100 trim idle
}

TEST_F(TrimsTest, MoveTrimsToOffsetsWithCrossTrims)
{
  // No trim idle only
  // Cross trims
  g_model.thrTrim = 0;
  g_model.limitData[2].offset = 0;
  g_model.limitData[1].offset = 0;
  g_model.thrTrimSw = MIXSRC_TrimEle - MIXSRC_FIRST_TRIM;
  ExpoData *expo = expoAddress(THR_STICK);
  expo->trimSource = TRIM_ELE;
  expo = expoAddress(ELE_STICK);
  expo->trimSource = TRIM_THR;

  anaSetFiltered(THR_STICK,  0);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, 100);
  setTrimValue(0, MIXSRC_TrimThr - MIXSRC_FIRST_TRIM, -100);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 200);  // THR output value is reflecting 100 Ele trim
  moveTrimsToOffsets();
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 200);  // THR output value remains unchanged
  EXPECT_EQ(getTrimValue(0, MIXSRC_TrimThr - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(g_model.limitData[2].offset, 195); // value transferred
  EXPECT_EQ(getTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(g_model.limitData[1].offset, -195); // value transferred
}

TEST_F(TrimsTest, MoveTrimsToOffsetsWithCrosstrimsAndTrimIdle)
{
  // Trim idle only
  // Cross trims
  g_model.limitData[2].offset = 0;
  g_model.limitData[1].offset = 0;
  g_model.thrTrim = 1;
  g_model.thrTrimSw = MIXSRC_TrimEle - MIXSRC_FIRST_TRIM;
  ExpoData *expo = expoAddress(THR_STICK);
  expo->trimSource = TRIM_ELE;
  expo = expoAddress(ELE_STICK);
  expo->trimSource = TRIM_THR;

  anaSetFiltered(THR_STICK,  -1024);  // Min stick
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, 100);
  setTrimValue(0, MIXSRC_TrimThr - MIXSRC_FIRST_TRIM, -100);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -568);  // THR output value is reflecting 100 ele trim idle
  moveTrimsToOffsets();

  // Trim affecting Throttle (now Ele because of crosstrims) should not be affected
  EXPECT_EQ(getTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM), 100);  // unchanged
  EXPECT_EQ(g_model.limitData[2].offset, 0); // THR chan offset unchanged

  // Other trims should
  EXPECT_EQ(getTrimValue(0, MIXSRC_TrimThr - MIXSRC_FIRST_TRIM), 0);  // back to neutral
  EXPECT_EQ(g_model.limitData[1].offset, -195); // Ele chan offset transfered
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -568);  // THR output value is still reflecting 100 trim idle
}

TEST_F(TrimsTest, InstantTrim)
{
  anaSetFiltered(AIL_STICK, 50);
  instantTrim();
#if defined(STICK_DEAD_ZONE)
  EXPECT_EQ(23, getTrimValue(0, AIL_STICK));
#else
  EXPECT_EQ(25, getTrimValue(0, AIL_STICK));
#endif
}

TEST_F(TrimsTest, InstantTrimNegativeCurve)
{
  ExpoData *expo = expoAddress(AIL_STICK);
  expo->curve.type = CURVE_REF_CUSTOM;
  expo->curve.value = 1;
  g_model.points[0] = -100;
  g_model.points[1] = -75;
  g_model.points[2] = -50;
  g_model.points[3] = -25;
  g_model.points[4] = 0;
  anaSetFiltered(AIL_STICK, 512);
  instantTrim();
#if defined(STICK_DEAD_ZONE)
  EXPECT_EQ(127, getTrimValue(0, AIL_STICK));
#else
  EXPECT_EQ(128, getTrimValue(0, AIL_STICK));
#endif
}

TEST(Curves, LinearIntpol)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  for (int8_t i=-2; i<=2; i++) {
    g_model.points[2+i] = 50*i;
  }
  EXPECT_EQ(applyCustomCurve(-1024, 0), -1024);
  EXPECT_EQ(applyCustomCurve(0, 0), 0);
  EXPECT_EQ(applyCustomCurve(1024, 0), 1024);
  EXPECT_EQ(applyCustomCurve(-192, 0), -192);
}



TEST_F(MixerTest, InfiniteRecursiveChannels)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].srcRaw = MIXSRC_FIRST_CH + 1;
  g_model.mixData[0].weight = 100;
  g_model.mixData[1].destCh = 1;
  g_model.mixData[1].srcRaw = MIXSRC_FIRST_CH + 2;
  g_model.mixData[1].weight = 100;
  g_model.mixData[2].destCh = 2;
  g_model.mixData[2].srcRaw = MIXSRC_FIRST_CH;
  g_model.mixData[2].weight = 100;
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[2], 0);
  EXPECT_EQ(chans[1], 0);
  EXPECT_EQ(chans[0], 0);
}

TEST_F(MixerTest, BlockingChannel)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].srcRaw = MIXSRC_FIRST_CH;
  g_model.mixData[0].weight = 100;
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], 0);
}

TEST_F(MixerTest, RecursiveAddChannel)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = 50;
  g_model.mixData[1].destCh = 0;
  g_model.mixData[1].mltpx = MLTPX_ADD;
  g_model.mixData[1].srcRaw = MIXSRC_FIRST_CH + 1;
  g_model.mixData[1].weight = 100;
  g_model.mixData[2].destCh = 1;
  g_model.mixData[2].srcRaw = MIXSRC_FIRST_STICK;
  g_model.mixData[2].weight = 100;
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], CHANNEL_MAX/2);
  EXPECT_EQ(chans[1], 0);
}

TEST_F(MixerTest, RecursiveAddChannelAfterInactivePhase)
{
  g_model.flightModeData[1].swtch = SWSRC_FIRST_SWITCH + 1;
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_FIRST_CH + 1;
  g_model.mixData[0].flightModes = 0b11110;
  g_model.mixData[0].weight = 50;
  g_model.mixData[1].destCh = 0;
  g_model.mixData[1].mltpx = MLTPX_ADD;
  g_model.mixData[1].srcRaw = MIXSRC_MAX;
  g_model.mixData[1].flightModes = 0b11101;
  g_model.mixData[1].weight = 50;
  g_model.mixData[2].destCh = 1;
  g_model.mixData[2].srcRaw = MIXSRC_MAX;
  g_model.mixData[2].weight = 100;
  simuSetSwitch(3, -1);
  evalMixes(1);
  EXPECT_EQ(chans[0], CHANNEL_MAX/2);
  EXPECT_EQ(chans[1], CHANNEL_MAX);
  simuSetSwitch(3, 0);
  evalMixes(1);
  EXPECT_EQ(chans[0], CHANNEL_MAX/2);
  EXPECT_EQ(chans[1], CHANNEL_MAX);
}


TEST_F(MixerTest, SlowOnPhase)
{
  g_model.flightModeData[1].swtch = TR(SWSRC_THR, SWSRC_SA0);
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = 100;
  g_model.mixData[0].flightModes = 0x2 + 0x4 + 0x8 + 0x10 /*only enabled in phase 0*/;
  g_model.mixData[0].speedUp = 50;
  g_model.mixData[0].speedDown = 50;

  s_mixer_first_run_done = true;
  mixerCurrentFlightMode = 0;
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], 0);

  CHECK_SLOW_MOVEMENT(0, +1, 250);

  mixerCurrentFlightMode = 1;
  CHECK_SLOW_MOVEMENT(0, -1, 250);
}

TEST_F(MixerTest, SlowOnSwitchSource)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_eeGeneral.switchConfig = 0x03;
  g_model.mixData[0].srcRaw = MIXSRC_FIRST_SWITCH;
  int switchIndex = 0;
  g_model.mixData[0].weight = 100;
  g_model.mixData[0].speedUp = 50;
  g_model.mixData[0].speedDown = 50;

  s_mixer_first_run_done = true;

  simuSetSwitch(switchIndex, -1);
  CHECK_SLOW_MOVEMENT(0, -1, 250);
  EXPECT_EQ(chans[0], -CHANNEL_MAX);

  simuSetSwitch(switchIndex, 1);
  CHECK_SLOW_MOVEMENT(0, +1, 500);
}

TEST_F(MixerTest, SlowDisabledOnStartup)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = 100;
  g_model.mixData[0].speedUp = 50;
  g_model.mixData[0].speedDown = 50;

  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], CHANNEL_MAX);
}

TEST_F(MixerTest, DelayOnSwitch)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = 100;
#if defined(PCBFRSKY)
  g_model.mixData[0].swtch = SWSRC_SA2;
  int switch_index = 0;
#else
  g_model.mixData[0].swtch = SWSRC_THR;
  int switch_index = 1;
#endif
  g_model.mixData[0].delayUp = 50;
  g_model.mixData[0].delayDown = 50;

  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], 0);

  simuSetSwitch(switch_index, 1);
  CHECK_DELAY(0, 500);

  evalFlightModeMixes(e_perout_mode_normal, 1);
  EXPECT_EQ(chans[0], CHANNEL_MAX);

  simuSetSwitch(switch_index, 0);
  CHECK_DELAY(0, 500);

  evalFlightModeMixes(e_perout_mode_normal, 1);
  EXPECT_EQ(chans[0], 0);
}

TEST_F(MixerTest, SlowOnMultiply)
{
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].weight = 100;
  g_model.mixData[1].destCh = 0;
  g_model.mixData[1].mltpx = MLTPX_MUL;
  g_model.mixData[1].srcRaw = MIXSRC_MAX;
  g_model.mixData[1].weight = 100;
  g_model.mixData[1].swtch = TR(SWSRC_THR, SWSRC_SA0);
  g_model.mixData[1].speedUp = 50;
  g_model.mixData[1].speedDown = 50;

  s_mixer_first_run_done = true;

  simuSetSwitch(0, 1);
  CHECK_SLOW_MOVEMENT(0, 1, 250);

  simuSetSwitch(0, -1);
  CHECK_NO_MOVEMENT(0, CHANNEL_MAX, 250);

  simuSetSwitch(0, 1);
  CHECK_NO_MOVEMENT(0, CHANNEL_MAX, 250);
}

TEST_F(TrimsTest, throttleTrimEle) {
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  g_eeGeneral.templateSetup = 17;
  applyDefaultTemplate();
  g_model.thrTrim = 1;
// checks ELE sticks are not affected by throttleTrim
// stick max + trim min
  anaSetFiltered(ELE_STICK, +1024);
  setTrimValue(0, ELE_STICK, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024 - 256);
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  g_eeGeneral.templateSetup = 0;
  applyDefaultTemplate();
}

#if defined(HELI)
TEST(Heli, BasicTest)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  g_model.swashR.collectiveSource = MIXSRC_Thr;
  g_model.swashR.elevatorSource = MIXSRC_Ele;
  g_model.swashR.aileronSource = MIXSRC_Ail;
  g_model.swashR.collectiveWeight = 100;
  g_model.swashR.elevatorWeight = 100;
  g_model.swashR.aileronWeight = 100;
  g_model.swashR.type = SWASH_TYPE_120;
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_CYC1;
  g_model.mixData[0].weight = 100;
  g_model.mixData[1].destCh = 1;
  g_model.mixData[1].mltpx = MLTPX_ADD;
  g_model.mixData[1].srcRaw = MIXSRC_CYC2;
  g_model.mixData[1].weight = 100;
  g_model.mixData[2].destCh = 2;
  g_model.mixData[2].mltpx = MLTPX_ADD;
  g_model.mixData[2].srcRaw = MIXSRC_CYC3;
  g_model.mixData[2].weight = 100;
  anaSetFiltered(ELE_STICK, 1024);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], -CHANNEL_MAX);
  EXPECT_EQ(chans[1], CHANNEL_MAX/2);
  EXPECT_EQ(chans[2], CHANNEL_MAX/2);
}

TEST(Heli, Mode2Test)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  g_eeGeneral.templateSetup = 2;
  applyDefaultTemplate();
  g_model.swashR.collectiveSource = MIXSRC_Thr;
  g_model.swashR.elevatorSource = MIXSRC_Ele;
  g_model.swashR.aileronSource = MIXSRC_Ail;
  g_model.swashR.collectiveWeight = 100;
  g_model.swashR.elevatorWeight = 100;
  g_model.swashR.aileronWeight = 100;
  g_model.swashR.type = SWASH_TYPE_120;
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_CYC1;
  g_model.mixData[0].weight = 100;
  g_model.mixData[1].destCh = 1;
  g_model.mixData[1].mltpx = MLTPX_ADD;
  g_model.mixData[1].srcRaw = MIXSRC_CYC2;
  g_model.mixData[1].weight = 100;
  g_model.mixData[2].destCh = 2;
  g_model.mixData[2].mltpx = MLTPX_ADD;
  g_model.mixData[2].srcRaw = MIXSRC_CYC3;
  g_model.mixData[2].weight = 100;
  anaSetFiltered(ELE_STICK, 1024);
  evalFlightModeMixes(e_perout_mode_normal, 0);
  EXPECT_EQ(chans[0], -CHANNEL_MAX);
  EXPECT_EQ(chans[1], CHANNEL_MAX/2);
  EXPECT_EQ(chans[2], CHANNEL_MAX/2);
  SYSTEM_RESET();
}
#endif

TEST(Trainer, UnpluggedTest)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_ADD;
  g_model.mixData[0].srcRaw = MIXSRC_FIRST_TRAINER;
  g_model.mixData[0].weight = 100;
  g_model.mixData[0].delayUp = 50;
  g_model.mixData[0].delayDown = 50;
  ppmInputValidityTimer = 0;
  ppmInput[0] = 1024;
  CHECK_DELAY(0, 5000);
}

TEST_F(MixerTest, flightModeTransition)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  g_model.flightModeData[1].swtch = TR(SWSRC_ID2, SWSRC_SA2);
  g_model.flightModeData[0].fadeIn = 100;
  g_model.flightModeData[0].fadeOut = 100;
  g_model.flightModeData[1].fadeIn = 100;
  g_model.flightModeData[1].fadeOut = 100;
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_REPL;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].flightModes = 0b11110;
  g_model.mixData[0].weight = 100;
  g_model.mixData[1].destCh = 0;
  g_model.mixData[1].mltpx = MLTPX_REPL;
  g_model.mixData[1].srcRaw = MIXSRC_MAX;
  g_model.mixData[1].flightModes = 0b11101;
  g_model.mixData[1].weight = -10;
  evalMixes(1);
  simuSetSwitch(0, 1);
  CHECK_FLIGHT_MODE_TRANSITION(0, 1000, 1024, -102);
}

TEST_F(MixerTest, flightModeOverflow)
{
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  setModelDefaults();
  g_model.flightModeData[1].swtch = TR(SWSRC_ID2, SWSRC_SA2);
  g_model.flightModeData[0].fadeIn = 100;
  g_model.flightModeData[0].fadeOut = 100;
  g_model.mixData[0].destCh = 0;
  g_model.mixData[0].mltpx = MLTPX_REPL;
  g_model.mixData[0].srcRaw = MIXSRC_MAX;
  g_model.mixData[0].flightModes = 0;
  g_model.mixData[0].weight = 250;
  evalMixes(1);
  simuSetSwitch(0, 1);
  CHECK_FLIGHT_MODE_TRANSITION(0, 1000, 1024, 1024);
}

TEST_F(TrimsTest, throttleTrimWithCrossTrims)
{
  g_model.thrTrim = 1;
  g_model.thrTrimSw = MIXSRC_TrimEle - MIXSRC_FIRST_TRIM;
  ExpoData *expo = expoAddress(THR_STICK);
  expo->trimSource = TRIM_ELE;
  expo = expoAddress(ELE_STICK);
  expo->trimSource = TRIM_THR;

  // stick max + trim max
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick max + trim min
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick min + trim max
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024 + 512);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick min + trim mid
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024 + 256);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick min + trim min
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024);
  EXPECT_EQ(channelOutputs[1], 0);

  // now the same tests with extended Trims
  g_model.extendedTrims = 1;
  // stick max + trim max
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick max + trim min
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], 1024);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick min + trim max
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+2048);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick min + trim mid
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+1024);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick min + trim min
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024);
  EXPECT_EQ(channelOutputs[1], 0);
}

TEST_F(TrimsTest, invertedThrottlePlusThrottleTrimWithCrossTrims)
{
  g_model.throttleReversed = 1;
  g_model.thrTrim = 1;
  g_model.thrTrimSw = MIXSRC_TrimEle - MIXSRC_FIRST_TRIM;
  ExpoData *expo = expoAddress(THR_STICK);
  expo->trimSource = TRIM_ELE;
  expo = expoAddress(ELE_STICK);
  expo->trimSource = TRIM_THR;

  // stick max + trim max
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick max + trim mid
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+256);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick max + trim min
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+512);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick min + trim max
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], +1024);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick min + trim min
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], +1024);
  EXPECT_EQ(channelOutputs[1], 0);

  // now the same tests with extended Trims
  g_model.extendedTrims = 1;
  // stick max + trim max
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick max + trim mid
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, 0);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+1024);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick max + trim min
  anaSetFiltered(THR_STICK,  +1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], -1024+2048);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick min + trim max
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MAX);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], +1024);
  EXPECT_EQ(channelOutputs[1], 0);
  // stick min + trim min
  anaSetFiltered(THR_STICK,  -1024);
  setTrimValue(0, MIXSRC_TrimEle - MIXSRC_FIRST_TRIM, TRIM_EXTENDED_MIN);
  evalMixes(1);
  EXPECT_EQ(channelOutputs[2], +1024);
  EXPECT_EQ(channelOutputs[1], 0);
}
