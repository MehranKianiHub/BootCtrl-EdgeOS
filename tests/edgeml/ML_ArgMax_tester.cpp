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

  struct ML_ArgMax_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_ArgMax_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_ArgMax"_STRID) {
        setInputData({&mIn0, &mIn1, &mIn2, &mIn3});
        setOutputData({&mIndex, &mValue, &mValid, &mError, &mErrorCode});
        setup();
      }

      CIEC_REAL mIn0;
      CIEC_REAL mIn1;
      CIEC_REAL mIn2;
      CIEC_REAL mIn3;

      CIEC_USINT mIndex;
      CIEC_REAL mValue;
      CIEC_BOOL mValid;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_ArgMaxTests, ML_ArgMax_TestFixture)

  BOOST_AUTO_TEST_CASE(selectHighestValue) {
    mIn0 = CIEC_REAL(0.1F);
    mIn1 = CIEC_REAL(5.0F);
    mIn2 = CIEC_REAL(2.0F);
    mIn3 = CIEC_REAL(4.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mIndex));
    BOOST_TEST(5.0F == static_cast<CIEC_REAL::TValueType>(mValue));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(tieKeepsFirstOccurrence) {
    mIn0 = CIEC_REAL(3.0F);
    mIn1 = CIEC_REAL(3.0F);
    mIn2 = CIEC_REAL(2.0F);
    mIn3 = CIEC_REAL(3.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mIndex));
    BOOST_TEST(3.0F == static_cast<CIEC_REAL::TValueType>(mValue));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(nonFiniteInput) {
    mIn0 = CIEC_REAL(0.0F);
    mIn1 = CIEC_REAL(1.0F);
    mIn2 = CIEC_REAL(std::numeric_limits<float>::quiet_NaN());
    mIn3 = CIEC_REAL(2.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
