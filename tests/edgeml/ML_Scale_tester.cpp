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

  struct ML_Scale_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_Scale_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_Scale"_STRID) {
        setInputData({&mIn, &mInMin, &mInMax, &mOutMin, &mOutMax, &mClamp});
        setOutputData({&mOut, &mValid, &mError, &mErrorCode});
        setup();
      }

      CIEC_REAL mIn;
      CIEC_REAL mInMin;
      CIEC_REAL mInMax;
      CIEC_REAL mOutMin;
      CIEC_REAL mOutMax;
      CIEC_BOOL mClamp;

      CIEC_REAL mOut;
      CIEC_BOOL mValid;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_ScaleTests, ML_Scale_TestFixture)

  BOOST_AUTO_TEST_CASE(validScaleMapping) {
    mIn = CIEC_REAL(5.0F);
    mInMin = CIEC_REAL(0.0F);
    mInMax = CIEC_REAL(10.0F);
    mOutMin = CIEC_REAL(-1.0F);
    mOutMax = CIEC_REAL(1.0F);
    mClamp = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(0.0F == static_cast<CIEC_REAL::TValueType>(mOut));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(invalidInputRange) {
    mIn = CIEC_REAL(5.0F);
    mInMin = CIEC_REAL(1.0F);
    mInMax = CIEC_REAL(1.0F);
    mOutMin = CIEC_REAL(0.0F);
    mOutMax = CIEC_REAL(1.0F);
    mClamp = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(nonFiniteInput) {
    mIn = CIEC_REAL(std::numeric_limits<float>::quiet_NaN());
    mInMin = CIEC_REAL(0.0F);
    mInMax = CIEC_REAL(1.0F);
    mOutMin = CIEC_REAL(0.0F);
    mOutMax = CIEC_REAL(1.0F);
    mClamp = CIEC_BOOL(true);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(extrapolationWhenClampDisabled) {
    mIn = CIEC_REAL(12.0F);
    mInMin = CIEC_REAL(0.0F);
    mInMax = CIEC_REAL(10.0F);
    mOutMin = CIEC_REAL(0.0F);
    mOutMax = CIEC_REAL(100.0F);
    mClamp = CIEC_BOOL(false);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(120.0F == static_cast<CIEC_REAL::TValueType>(mOut));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
