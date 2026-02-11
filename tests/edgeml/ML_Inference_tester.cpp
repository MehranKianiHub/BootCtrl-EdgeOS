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
#include "forte/datatypes/forte_any_variant.h"
#include "forte/datatypes/forte_array.h"
#include "forte/datatypes/forte_array_common.h"
#include "forte/datatypes/forte_array_fixed.h"
#include "forte/datatypes/forte_array_variable.h"
#include "forte/datatypes/forte_bool.h"
#include "forte/datatypes/forte_real.h"
#include "forte/datatypes/forte_string.h"
#include "forte/datatypes/forte_udint.h"
#include "forte/datatypes/forte_usint.h"

#include <limits>
#include <string>
#include <variant>
#include <vector>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml::test {
  namespace {
    void setInputVector(CIEC_ANY_VARIANT &paTarget, const std::vector<float> &paValues) {
      CIEC_ARRAY_VARIABLE<CIEC_REAL> array;
      if (paValues.empty()) {
        array.setBounds(0, -1);
      } else {
        array.setBounds(0, static_cast<intmax_t>(paValues.size()) - 1);
        for (std::size_t i = 0; i < paValues.size(); ++i) {
          array[static_cast<intmax_t>(i)] = CIEC_REAL(paValues[i]);
        }
      }
      paTarget.setValue(array);
    }

    std::vector<float> extractOutputVector(const CIEC_ANY_VARIANT &paSource) {
      if (!std::holds_alternative<CIEC_ANY_UNIQUE_PTR<CIEC_ARRAY>>(paSource)) {
        return {};
      }

      const auto &array = *std::get<CIEC_ANY_UNIQUE_PTR<CIEC_ARRAY>>(paSource);
      std::vector<float> output;
      output.reserve(array.size());
      for (intmax_t i = array.getLowerBound(), end = array.getUpperBound(); i <= end; ++i) {
        output.push_back(static_cast<CIEC_REAL::TValueType>(static_cast<const CIEC_REAL &>(array[i])));
      }
      return output;
    }
  } // namespace

  struct ML_Inference_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_Inference_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_Inference"_STRID) {
        setInputData({&mModelId, &mInValues, &mOutCapacity});
        setOutputData({&mOutValues, &mValid, &mError, &mErrorCode, &mOutputCount, &mInferenceUs});
        setup();
      }

      CIEC_STRING mModelId;
      CIEC_ANY_VARIANT mInValues;
      CIEC_UDINT mOutCapacity;

      CIEC_ANY_VARIANT mOutValues;
      CIEC_BOOL mValid;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
      CIEC_UDINT mOutputCount;
      CIEC_UDINT mInferenceUs;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_InferenceTests, ML_Inference_TestFixture)

  BOOST_AUTO_TEST_CASE(emptyInputVectorValidation) {
    mModelId = CIEC_STRING(std::string("mock.default"));
    setInputVector(mInValues, {});
    mOutCapacity = CIEC_UDINT(4U);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(nonFiniteInputValidation) {
    mModelId = CIEC_STRING(std::string("mock.default"));
    setInputVector(mInValues, {1.0F, std::numeric_limits<float>::quiet_NaN(), 3.0F});
    mOutCapacity = CIEC_UDINT(3U);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(6U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(outputCapacityTooSmall) {
    mModelId = CIEC_STRING(std::string("mock.default"));
    setInputVector(mInValues, {1.0F, 2.0F, 3.0F, 4.0F, 5.0F});
    mOutCapacity = CIEC_UDINT(3U);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(7U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(modelNotLoaded) {
    mModelId = CIEC_STRING(std::string("mock.missing.model"));
    setInputVector(mInValues, {1.0F, 2.0F});
    mOutCapacity = CIEC_UDINT(2U);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(4U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(successfulVariableShapeInference) {
    mModelId = CIEC_STRING(std::string("mock.default"));
    setInputVector(mInValues, {1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F});
    mOutCapacity = CIEC_UDINT(6U);

    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mValid));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
    BOOST_TEST(6U == static_cast<CIEC_UDINT::TValueType>(mOutputCount));

    const auto output = extractOutputVector(mOutValues);
    BOOST_TEST(6U == output.size());
    BOOST_TEST(1.0F == output[0]);
    BOOST_TEST(2.0F == output[1]);
    BOOST_TEST(3.0F == output[2]);
    BOOST_TEST(4.0F == output[3]);
    BOOST_TEST(5.0F == output[4]);
    BOOST_TEST(6.0F == output[5]);
    BOOST_TEST(0U == static_cast<CIEC_UDINT::TValueType>(mInferenceUs));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
