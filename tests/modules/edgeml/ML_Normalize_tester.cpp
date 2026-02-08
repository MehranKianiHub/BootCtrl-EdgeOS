/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "../../core/fbtests/fbtestfixture.h"
#include "forte_boost_output_support.h"
#include "forte/datatypes/forte_usint.h"

#include <limits>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml::test {

  struct ML_Normalize_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_Normalize_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_Normalize"_STRID) {
        setInputData({&mIn, &mMethod, &mMin, &mMax, &mMean, &mStdDev, &mClamp});
        setOutputData({&mOut, &mValid, &mError, &mErrorCode});
        setup();
      }

      CIEC_REAL mIn;
      CIEC_USINT mMethod;
      CIEC_REAL mMin;
      CIEC_REAL mMax;
      CIEC_REAL mMean;
      CIEC_REAL mStdDev;
      CIEC_BOOL mClamp;

      CIEC_REAL mOut;
      CIEC_BOOL mValid;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_NormalizeTests, ML_Normalize_TestFixture)

  BOOST_AUTO_TEST_CASE(minMaxNormalization) {
    mIn = CIEC_REAL(25.0F);
    mMethod = CIEC_USINT(0U);
    mMin = CIEC_REAL(0.0F);
    mMax = CIEC_REAL(100.0F);
    mMean = CIEC_REAL(0.0F);
    mStdDev = CIEC_REAL(1.0F);
    mClamp = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(0.25F == static_cast<CIEC_REAL::TValueType>(mOut));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(zScoreNormalization) {
    mIn = CIEC_REAL(14.0F);
    mMethod = CIEC_USINT(1U);
    mMin = CIEC_REAL(0.0F);
    mMax = CIEC_REAL(1.0F);
    mMean = CIEC_REAL(10.0F);
    mStdDev = CIEC_REAL(2.0F);
    mClamp = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(2.0F == static_cast<CIEC_REAL::TValueType>(mOut));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(minMaxDivisionByZeroProtection) {
    mIn = CIEC_REAL(10.0F);
    mMethod = CIEC_USINT(0U);
    mMin = CIEC_REAL(1.0F);
    mMax = CIEC_REAL(1.0F);
    mMean = CIEC_REAL(0.0F);
    mStdDev = CIEC_REAL(1.0F);
    mClamp = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(zScoreDivisionByZeroProtection) {
    mIn = CIEC_REAL(10.0F);
    mMethod = CIEC_USINT(1U);
    mMin = CIEC_REAL(0.0F);
    mMax = CIEC_REAL(1.0F);
    mMean = CIEC_REAL(10.0F);
    mStdDev = CIEC_REAL(0.0F);
    mClamp = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(3U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(nonFiniteInput) {
    mIn = CIEC_REAL(std::numeric_limits<float>::quiet_NaN());
    mMethod = CIEC_USINT(0U);
    mMin = CIEC_REAL(0.0F);
    mMax = CIEC_REAL(1.0F);
    mMean = CIEC_REAL(0.0F);
    mStdDev = CIEC_REAL(1.0F);
    mClamp = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(unknownMethod) {
    mIn = CIEC_REAL(0.0F);
    mMethod = CIEC_USINT(99U);
    mMin = CIEC_REAL(0.0F);
    mMax = CIEC_REAL(1.0F);
    mMean = CIEC_REAL(0.0F);
    mStdDev = CIEC_REAL(1.0F);
    mClamp = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(4U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
