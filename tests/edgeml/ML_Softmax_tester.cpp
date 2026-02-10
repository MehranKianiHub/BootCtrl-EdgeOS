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

  struct ML_Softmax_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_Softmax_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_Softmax"_STRID) {
        setInputData({&mIn0, &mIn1, &mIn2, &mIn3, &mTemperature});
        setOutputData({&mOut0, &mOut1, &mOut2, &mOut3, &mValid, &mError, &mErrorCode});
        setup();
      }

      CIEC_REAL mIn0;
      CIEC_REAL mIn1;
      CIEC_REAL mIn2;
      CIEC_REAL mIn3;
      CIEC_REAL mTemperature;

      CIEC_REAL mOut0;
      CIEC_REAL mOut1;
      CIEC_REAL mOut2;
      CIEC_REAL mOut3;
      CIEC_BOOL mValid;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_SoftmaxTests, ML_Softmax_TestFixture)

  BOOST_AUTO_TEST_CASE(distributionSumsToOne) {
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(2.0F);
    mIn2 = CIEC_REAL(3.0F);
    mIn3 = CIEC_REAL(4.0F);
    mTemperature = CIEC_REAL(1.0F);

    triggerEvent(0);

    const auto out0 = static_cast<CIEC_REAL::TValueType>(mOut0);
    const auto out1 = static_cast<CIEC_REAL::TValueType>(mOut1);
    const auto out2 = static_cast<CIEC_REAL::TValueType>(mOut2);
    const auto out3 = static_cast<CIEC_REAL::TValueType>(mOut3);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
    BOOST_TEST(out3 > out2);
    BOOST_TEST(out2 > out1);
    BOOST_TEST(out1 > out0);
    BOOST_TEST((out0 + out1 + out2 + out3) == 1.0F, boost::test_tools::tolerance(0.0001F));
  }

  BOOST_AUTO_TEST_CASE(highTemperatureApproachesUniformDistribution) {
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(2.0F);
    mIn2 = CIEC_REAL(3.0F);
    mIn3 = CIEC_REAL(4.0F);
    mTemperature = CIEC_REAL(100.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mOut0) == 0.25F, boost::test_tools::tolerance(0.02F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mOut1) == 0.25F, boost::test_tools::tolerance(0.02F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mOut2) == 0.25F, boost::test_tools::tolerance(0.02F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mOut3) == 0.25F, boost::test_tools::tolerance(0.02F));
  }

  BOOST_AUTO_TEST_CASE(invalidTemperature) {
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(2.0F);
    mIn2 = CIEC_REAL(3.0F);
    mIn3 = CIEC_REAL(4.0F);
    mTemperature = CIEC_REAL(0.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(nonFiniteInput) {
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(std::numeric_limits<float>::quiet_NaN());
    mIn2 = CIEC_REAL(3.0F);
    mIn3 = CIEC_REAL(4.0F);
    mTemperature = CIEC_REAL(1.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
