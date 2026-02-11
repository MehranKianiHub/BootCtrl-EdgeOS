/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "../../4diacFORTE/tests/core/fbtests/fbtestfixture.h"
#include "../../4diacFORTE/tests/forte_boost_output_support.h"
#include "forte/datatypes/forte_bool.h"
#include "forte/datatypes/forte_usint.h"

#include <limits>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml::test {

  struct ML_MovingAverage_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_MovingAverage_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_MovingAverage"_STRID) {
        setInputData({&mIn0, &mIn1, &mIn2, &mIn3, &mWindow, &mReset});
        setOutputData({&mOut0, &mOut1, &mOut2, &mOut3, &mSampleCount, &mValid, &mError, &mErrorCode});
        setup();
      }

      CIEC_REAL mIn0;
      CIEC_REAL mIn1;
      CIEC_REAL mIn2;
      CIEC_REAL mIn3;
      CIEC_USINT mWindow;
      CIEC_BOOL mReset;

      CIEC_REAL mOut0;
      CIEC_REAL mOut1;
      CIEC_REAL mOut2;
      CIEC_REAL mOut3;
      CIEC_USINT mSampleCount;
      CIEC_BOOL mValid;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_MovingAverageTests, ML_MovingAverage_TestFixture)

  BOOST_AUTO_TEST_CASE(averagesOverWindowTwo) {
    mWindow = CIEC_USINT(2U);
    mReset = CIEC_BOOL(false);

    mIn0 = CIEC_REAL(2.0F);
    mIn1 = CIEC_REAL(4.0F);
    mIn2 = CIEC_REAL(6.0F);
    mIn3 = CIEC_REAL(8.0F);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mSampleCount));

    mIn0 = CIEC_REAL(4.0F);
    mIn1 = CIEC_REAL(6.0F);
    mIn2 = CIEC_REAL(8.0F);
    mIn3 = CIEC_REAL(10.0F);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(3.0F == static_cast<CIEC_REAL::TValueType>(mOut0));
    BOOST_TEST(5.0F == static_cast<CIEC_REAL::TValueType>(mOut1));
    BOOST_TEST(7.0F == static_cast<CIEC_REAL::TValueType>(mOut2));
    BOOST_TEST(9.0F == static_cast<CIEC_REAL::TValueType>(mOut3));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mSampleCount));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
  }

  BOOST_AUTO_TEST_CASE(resetClearsHistory) {
    mWindow = CIEC_USINT(3U);
    mReset = CIEC_BOOL(false);

    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(1.0F);
    mIn2 = CIEC_REAL(1.0F);
    mIn3 = CIEC_REAL(1.0F);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mIn0 = CIEC_REAL(2.0F);
    mIn1 = CIEC_REAL(2.0F);
    mIn2 = CIEC_REAL(2.0F);
    mIn3 = CIEC_REAL(2.0F);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mReset = CIEC_BOOL(true);
    mIn0 = CIEC_REAL(10.0F);
    mIn1 = CIEC_REAL(11.0F);
    mIn2 = CIEC_REAL(12.0F);
    mIn3 = CIEC_REAL(13.0F);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(10.0F == static_cast<CIEC_REAL::TValueType>(mOut0));
    BOOST_TEST(11.0F == static_cast<CIEC_REAL::TValueType>(mOut1));
    BOOST_TEST(12.0F == static_cast<CIEC_REAL::TValueType>(mOut2));
    BOOST_TEST(13.0F == static_cast<CIEC_REAL::TValueType>(mOut3));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mSampleCount));
  }

  BOOST_AUTO_TEST_CASE(invalidWindow) {
    mWindow = CIEC_USINT(0U);
    mReset = CIEC_BOOL(false);
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(1.0F);
    mIn2 = CIEC_REAL(1.0F);
    mIn3 = CIEC_REAL(1.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(nonFiniteInput) {
    mWindow = CIEC_USINT(3U);
    mReset = CIEC_BOOL(false);
    mIn0 = CIEC_REAL(std::numeric_limits<float>::quiet_NaN());
    mIn1 = CIEC_REAL(1.0F);
    mIn2 = CIEC_REAL(1.0F);
    mIn3 = CIEC_REAL(1.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
