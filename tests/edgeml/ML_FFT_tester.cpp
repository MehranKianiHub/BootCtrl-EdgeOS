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

  struct ML_FFT_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_FFT_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_FFT"_STRID) {
        setInputData({&mIn0, &mIn1, &mIn2, &mIn3});
        setOutputData({&mMag0, &mMag1, &mMag2, &mMag3, &mValid, &mError, &mErrorCode});
        setup();
      }

      CIEC_REAL mIn0;
      CIEC_REAL mIn1;
      CIEC_REAL mIn2;
      CIEC_REAL mIn3;

      CIEC_REAL mMag0;
      CIEC_REAL mMag1;
      CIEC_REAL mMag2;
      CIEC_REAL mMag3;
      CIEC_BOOL mValid;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_FFTTests, ML_FFT_TestFixture)

  BOOST_AUTO_TEST_CASE(constantSignalProducesOnlyDcComponent) {
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(1.0F);
    mIn2 = CIEC_REAL(1.0F);
    mIn3 = CIEC_REAL(1.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mMag0) == 4.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mMag1) == 0.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mMag2) == 0.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mMag3) == 0.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
  }

  BOOST_AUTO_TEST_CASE(impulseSignalHasFlatSpectrumMagnitude) {
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(0.0F);
    mIn2 = CIEC_REAL(0.0F);
    mIn3 = CIEC_REAL(0.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mMag0) == 1.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mMag1) == 1.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mMag2) == 1.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mMag3) == 1.0F, boost::test_tools::tolerance(0.0001F));
  }

  BOOST_AUTO_TEST_CASE(nonFiniteInput) {
    mIn0 = CIEC_REAL(std::numeric_limits<float>::quiet_NaN());
    mIn1 = CIEC_REAL(0.0F);
    mIn2 = CIEC_REAL(0.0F);
    mIn3 = CIEC_REAL(0.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
