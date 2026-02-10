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

#include <boost/test/tools/floating_point_comparison.hpp>
#include <limits>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml::test {

  struct ML_AnomalyScore_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_AnomalyScore_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_AnomalyScore"_STRID) {
        setInputData({&mIn, &mReference, &mThreshold, &mScale});
        setOutputData({&mScore, &mIsAnomaly, &mValid, &mError, &mErrorCode});
        setup();
      }

      CIEC_REAL mIn;
      CIEC_REAL mReference;
      CIEC_REAL mThreshold;
      CIEC_REAL mScale;

      CIEC_REAL mScore;
      CIEC_BOOL mIsAnomaly;
      CIEC_BOOL mValid;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_AnomalyScoreTests, ML_AnomalyScore_TestFixture)

  BOOST_AUTO_TEST_CASE(detectsAnomalyAboveThreshold) {
    mIn = CIEC_REAL(10.0F);
    mReference = CIEC_REAL(8.0F);
    mThreshold = CIEC_REAL(1.0F);
    mScale = CIEC_REAL(1.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(2.0F == static_cast<CIEC_REAL::TValueType>(mScore));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mIsAnomaly));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(belowThresholdIsNotAnomaly) {
    mIn = CIEC_REAL(10.0F);
    mReference = CIEC_REAL(9.6F);
    mThreshold = CIEC_REAL(1.0F);
    mScale = CIEC_REAL(1.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mScore) == 0.4F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mIsAnomaly));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
  }

  BOOST_AUTO_TEST_CASE(scaleIsAppliedToDifference) {
    mIn = CIEC_REAL(8.0F);
    mReference = CIEC_REAL(7.5F);
    mThreshold = CIEC_REAL(1.0F);
    mScale = CIEC_REAL(2.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mScore) == 1.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mIsAnomaly));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
  }

  BOOST_AUTO_TEST_CASE(invalidScale) {
    mIn = CIEC_REAL(10.0F);
    mReference = CIEC_REAL(8.0F);
    mThreshold = CIEC_REAL(1.0F);
    mScale = CIEC_REAL(-1.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(nonFiniteInput) {
    mIn = CIEC_REAL(std::numeric_limits<float>::quiet_NaN());
    mReference = CIEC_REAL(0.0F);
    mThreshold = CIEC_REAL(1.0F);
    mScale = CIEC_REAL(1.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
