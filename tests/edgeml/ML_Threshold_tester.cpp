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

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml::test {

  struct ML_Threshold_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_Threshold_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_Threshold"_STRID) {
        setInputData({&mIn, &mThreshold, &mInclusive});
        setOutputData({&mExceeds, &mMargin});
        setup();
      }

      CIEC_REAL mIn;
      CIEC_REAL mThreshold;
      CIEC_BOOL mInclusive;

      CIEC_BOOL mExceeds;
      CIEC_REAL mMargin;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_ThresholdTests, ML_Threshold_TestFixture)

  BOOST_AUTO_TEST_CASE(greaterExclusive) {
    mIn = CIEC_REAL(1.5F);
    mThreshold = CIEC_REAL(1.0F);
    mInclusive = CIEC_BOOL(false);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mExceeds));
    BOOST_TEST(0.5F == static_cast<CIEC_REAL::TValueType>(mMargin));
  }

  BOOST_AUTO_TEST_CASE(equalExclusive) {
    mIn = CIEC_REAL(1.0F);
    mThreshold = CIEC_REAL(1.0F);
    mInclusive = CIEC_BOOL(false);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mExceeds));
    BOOST_TEST(0.0F == static_cast<CIEC_REAL::TValueType>(mMargin));
  }

  BOOST_AUTO_TEST_CASE(equalInclusive) {
    mIn = CIEC_REAL(1.0F);
    mThreshold = CIEC_REAL(1.0F);
    mInclusive = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mExceeds));
    BOOST_TEST(0.0F == static_cast<CIEC_REAL::TValueType>(mMargin));
  }

  BOOST_AUTO_TEST_CASE(lowerInput) {
    mIn = CIEC_REAL(-1.0F);
    mThreshold = CIEC_REAL(1.0F);
    mInclusive = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mExceeds));
    BOOST_TEST(-2.0F == static_cast<CIEC_REAL::TValueType>(mMargin));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
