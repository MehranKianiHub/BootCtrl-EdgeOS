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
#include "forte/datatypes/forte_string.h"
#include "forte/datatypes/forte_udint.h"
#include "forte/datatypes/forte_usint.h"

#include <limits>
#include <string>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml::test {

  struct ML_Inference_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_Inference_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_Inference"_STRID) {
        setInputData({&mModelId, &mIn0, &mIn1, &mIn2, &mIn3, &mOutSize});
        setOutputData({&mOut0, &mOut1, &mOut2, &mOut3, &mValid, &mError, &mErrorCode, &mOutputCount, &mInferenceUs});
        setup();
      }

      CIEC_STRING mModelId;
      CIEC_REAL mIn0;
      CIEC_REAL mIn1;
      CIEC_REAL mIn2;
      CIEC_REAL mIn3;
      CIEC_USINT mOutSize;

      CIEC_REAL mOut0;
      CIEC_REAL mOut1;
      CIEC_REAL mOut2;
      CIEC_REAL mOut3;
      CIEC_BOOL mValid;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
      CIEC_USINT mOutputCount;
      CIEC_UDINT mInferenceUs;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_InferenceTests, ML_Inference_TestFixture)

  BOOST_AUTO_TEST_CASE(invalidOutputSize) {
    mModelId = CIEC_STRING(std::string("mock.default"));
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(2.0F);
    mIn2 = CIEC_REAL(3.0F);
    mIn3 = CIEC_REAL(4.0F);
    mOutSize = CIEC_USINT(3U);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mOutputCount));
  }

  BOOST_AUTO_TEST_CASE(nonFiniteInputValidation) {
    mModelId = CIEC_STRING(std::string("mock.default"));
    mIn0 = CIEC_REAL(std::numeric_limits<float>::quiet_NaN());
    mIn1 = CIEC_REAL(2.0F);
    mIn2 = CIEC_REAL(3.0F);
    mIn3 = CIEC_REAL(4.0F);
    mOutSize = CIEC_USINT(4U);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(5U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(modelNotLoaded) {
    mModelId = CIEC_STRING(std::string("mock.missing.model"));
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(2.0F);
    mIn2 = CIEC_REAL(3.0F);
    mIn3 = CIEC_REAL(4.0F);
    mOutSize = CIEC_USINT(4U);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(3U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(successfulInferenceOnDefaultMockModel) {
    mModelId = CIEC_STRING(std::string("mock.default"));
    mIn0 = CIEC_REAL(1.0F);
    mIn1 = CIEC_REAL(2.0F);
    mIn2 = CIEC_REAL(3.0F);
    mIn3 = CIEC_REAL(4.0F);
    mOutSize = CIEC_USINT(4U);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
    BOOST_TEST(4U == static_cast<CIEC_USINT::TValueType>(mOutputCount));
    BOOST_TEST(1.0F == static_cast<CIEC_REAL::TValueType>(mOut0));
    BOOST_TEST(2.0F == static_cast<CIEC_REAL::TValueType>(mOut1));
    BOOST_TEST(3.0F == static_cast<CIEC_REAL::TValueType>(mOut2));
    BOOST_TEST(4.0F == static_cast<CIEC_REAL::TValueType>(mOut3));
    BOOST_TEST(0U == static_cast<CIEC_UDINT::TValueType>(mInferenceUs));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
