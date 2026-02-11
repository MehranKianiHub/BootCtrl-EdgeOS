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

  struct ML_PCA_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_PCA_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_PCA"_STRID) {
        setInputData({&mIn0, &mIn1, &mIn2, &mIn3, &mMean0, &mMean1, &mMean2, &mMean3, &mComp10, &mComp11,
                      &mComp12, &mComp13, &mComp20, &mComp21, &mComp22, &mComp23});
        setOutputData({&mOut0, &mOut1, &mValid, &mError, &mErrorCode});
        setup();
      }

      CIEC_REAL mIn0;
      CIEC_REAL mIn1;
      CIEC_REAL mIn2;
      CIEC_REAL mIn3;

      CIEC_REAL mMean0;
      CIEC_REAL mMean1;
      CIEC_REAL mMean2;
      CIEC_REAL mMean3;

      CIEC_REAL mComp10;
      CIEC_REAL mComp11;
      CIEC_REAL mComp12;
      CIEC_REAL mComp13;

      CIEC_REAL mComp20;
      CIEC_REAL mComp21;
      CIEC_REAL mComp22;
      CIEC_REAL mComp23;

      CIEC_REAL mOut0;
      CIEC_REAL mOut1;
      CIEC_BOOL mValid;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_PCATests, ML_PCA_TestFixture)

  BOOST_AUTO_TEST_CASE(projectsInputWithIdentityComponents) {
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(2.0F);
    mIn2 = CIEC_REAL(3.0F);
    mIn3 = CIEC_REAL(4.0F);

    mMean0 = CIEC_REAL(0.0F);
    mMean1 = CIEC_REAL(0.0F);
    mMean2 = CIEC_REAL(0.0F);
    mMean3 = CIEC_REAL(0.0F);

    mComp10 = CIEC_REAL(1.0F);
    mComp11 = CIEC_REAL(0.0F);
    mComp12 = CIEC_REAL(0.0F);
    mComp13 = CIEC_REAL(0.0F);

    mComp20 = CIEC_REAL(0.0F);
    mComp21 = CIEC_REAL(1.0F);
    mComp22 = CIEC_REAL(0.0F);
    mComp23 = CIEC_REAL(0.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(1.0F == static_cast<CIEC_REAL::TValueType>(mOut0));
    BOOST_TEST(2.0F == static_cast<CIEC_REAL::TValueType>(mOut1));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
  }

  BOOST_AUTO_TEST_CASE(appliesMeanCentering) {
    mIn0 = CIEC_REAL(5.0F);
    mIn1 = CIEC_REAL(9.0F);
    mIn2 = CIEC_REAL(0.0F);
    mIn3 = CIEC_REAL(0.0F);

    mMean0 = CIEC_REAL(2.0F);
    mMean1 = CIEC_REAL(4.0F);
    mMean2 = CIEC_REAL(0.0F);
    mMean3 = CIEC_REAL(0.0F);

    mComp10 = CIEC_REAL(1.0F);
    mComp11 = CIEC_REAL(0.0F);
    mComp12 = CIEC_REAL(0.0F);
    mComp13 = CIEC_REAL(0.0F);

    mComp20 = CIEC_REAL(0.0F);
    mComp21 = CIEC_REAL(1.0F);
    mComp22 = CIEC_REAL(0.0F);
    mComp23 = CIEC_REAL(0.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(3.0F == static_cast<CIEC_REAL::TValueType>(mOut0));
    BOOST_TEST(5.0F == static_cast<CIEC_REAL::TValueType>(mOut1));
  }

  BOOST_AUTO_TEST_CASE(invalidComponentVector) {
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(1.0F);
    mIn2 = CIEC_REAL(1.0F);
    mIn3 = CIEC_REAL(1.0F);

    mMean0 = CIEC_REAL(0.0F);
    mMean1 = CIEC_REAL(0.0F);
    mMean2 = CIEC_REAL(0.0F);
    mMean3 = CIEC_REAL(0.0F);

    mComp10 = CIEC_REAL(0.0F);
    mComp11 = CIEC_REAL(0.0F);
    mComp12 = CIEC_REAL(0.0F);
    mComp13 = CIEC_REAL(0.0F);

    mComp20 = CIEC_REAL(0.0F);
    mComp21 = CIEC_REAL(1.0F);
    mComp22 = CIEC_REAL(0.0F);
    mComp23 = CIEC_REAL(0.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(nonFiniteInput) {
    mIn0 = CIEC_REAL(std::numeric_limits<float>::quiet_NaN());
    mIn1 = CIEC_REAL(1.0F);
    mIn2 = CIEC_REAL(1.0F);
    mIn3 = CIEC_REAL(1.0F);

    mMean0 = CIEC_REAL(0.0F);
    mMean1 = CIEC_REAL(0.0F);
    mMean2 = CIEC_REAL(0.0F);
    mMean3 = CIEC_REAL(0.0F);

    mComp10 = CIEC_REAL(1.0F);
    mComp11 = CIEC_REAL(0.0F);
    mComp12 = CIEC_REAL(0.0F);
    mComp13 = CIEC_REAL(0.0F);

    mComp20 = CIEC_REAL(0.0F);
    mComp21 = CIEC_REAL(1.0F);
    mComp22 = CIEC_REAL(0.0F);
    mComp23 = CIEC_REAL(0.0F);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
