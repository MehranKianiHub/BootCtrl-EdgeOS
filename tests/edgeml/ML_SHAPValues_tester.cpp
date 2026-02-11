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

  struct ML_SHAPValues_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_SHAPValues_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_SHAPValues"_STRID) {
        setInputData({&mIn0, &mIn1, &mIn2, &mIn3, &mBaseline0, &mBaseline1, &mBaseline2, &mBaseline3,
                      &mWeight0, &mWeight1, &mWeight2, &mWeight3, &mNormalize});
        setOutputData({&mShap0, &mShap1, &mShap2, &mShap3, &mTotalContribution, &mValid, &mError, &mErrorCode});
        setup();
      }

      CIEC_REAL mIn0;
      CIEC_REAL mIn1;
      CIEC_REAL mIn2;
      CIEC_REAL mIn3;
      CIEC_REAL mBaseline0;
      CIEC_REAL mBaseline1;
      CIEC_REAL mBaseline2;
      CIEC_REAL mBaseline3;
      CIEC_REAL mWeight0;
      CIEC_REAL mWeight1;
      CIEC_REAL mWeight2;
      CIEC_REAL mWeight3;
      CIEC_BOOL mNormalize;

      CIEC_REAL mShap0;
      CIEC_REAL mShap1;
      CIEC_REAL mShap2;
      CIEC_REAL mShap3;
      CIEC_REAL mTotalContribution;
      CIEC_BOOL mValid;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_SHAPValuesTests, ML_SHAPValues_TestFixture)

  BOOST_AUTO_TEST_CASE(computesLinearContributions) {
    mIn0 = CIEC_REAL(2.0F);
    mIn1 = CIEC_REAL(4.0F);
    mIn2 = CIEC_REAL(6.0F);
    mIn3 = CIEC_REAL(8.0F);

    mBaseline0 = CIEC_REAL(1.0F);
    mBaseline1 = CIEC_REAL(1.0F);
    mBaseline2 = CIEC_REAL(1.0F);
    mBaseline3 = CIEC_REAL(1.0F);

    mWeight0 = CIEC_REAL(1.0F);
    mWeight1 = CIEC_REAL(0.5F);
    mWeight2 = CIEC_REAL(0.0F);
    mWeight3 = CIEC_REAL(-1.0F);

    mNormalize = CIEC_BOOL(false);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mShap0) == 1.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mShap1) == 1.5F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mShap2) == 0.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mShap3) == -7.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mTotalContribution) == -4.5F,
               boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
  }

  BOOST_AUTO_TEST_CASE(normalizesByAbsoluteContribution) {
    mIn0 = CIEC_REAL(2.0F);
    mIn1 = CIEC_REAL(4.0F);
    mIn2 = CIEC_REAL(6.0F);
    mIn3 = CIEC_REAL(8.0F);

    mBaseline0 = CIEC_REAL(0.0F);
    mBaseline1 = CIEC_REAL(0.0F);
    mBaseline2 = CIEC_REAL(0.0F);
    mBaseline3 = CIEC_REAL(0.0F);

    mWeight0 = CIEC_REAL(1.0F);
    mWeight1 = CIEC_REAL(1.0F);
    mWeight2 = CIEC_REAL(1.0F);
    mWeight3 = CIEC_REAL(1.0F);

    mNormalize = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mShap0) == 0.1F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mShap1) == 0.2F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mShap2) == 0.3F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mShap3) == 0.4F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mTotalContribution) == 1.0F,
               boost::test_tools::tolerance(0.0001F));
  }

  BOOST_AUTO_TEST_CASE(nonFiniteInput) {
    mIn0 = CIEC_REAL(std::numeric_limits<float>::quiet_NaN());
    mIn1 = CIEC_REAL(1.0F);
    mIn2 = CIEC_REAL(1.0F);
    mIn3 = CIEC_REAL(1.0F);

    mBaseline0 = CIEC_REAL(0.0F);
    mBaseline1 = CIEC_REAL(0.0F);
    mBaseline2 = CIEC_REAL(0.0F);
    mBaseline3 = CIEC_REAL(0.0F);

    mWeight0 = CIEC_REAL(1.0F);
    mWeight1 = CIEC_REAL(1.0F);
    mWeight2 = CIEC_REAL(1.0F);
    mWeight3 = CIEC_REAL(1.0F);

    mNormalize = CIEC_BOOL(false);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
