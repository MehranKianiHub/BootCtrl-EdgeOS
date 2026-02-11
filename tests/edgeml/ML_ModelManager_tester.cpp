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
#include "forte/datatypes/forte_byte.h"
#include "forte/datatypes/forte_bool.h"
#include "forte/datatypes/forte_string.h"
#include "forte/datatypes/forte_udint.h"
#include "forte/datatypes/forte_usint.h"

#include <string>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml::test {
  namespace {
    void setBlobFromString(CIEC_ANY_VARIANT &paTarget, const std::string &paValue) {
      CIEC_ARRAY_VARIABLE<CIEC_BYTE> blob;
      if (paValue.empty()) {
        blob.setBounds(0, -1);
      } else {
        blob.setBounds(0, static_cast<intmax_t>(paValue.size()) - 1);
        for (std::size_t i = 0; i < paValue.size(); ++i) {
          blob[static_cast<intmax_t>(i)] = CIEC_BYTE(static_cast<CIEC_BYTE::TValueType>(paValue[i]));
        }
      }
      paTarget.setValue(blob);
    }
  } // namespace

  struct ML_ModelManager_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_ModelManager_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_ModelManager"_STRID) {
        setInputData({&mCommand, &mModelId, &mVersion, &mSizeBytes, &mSha256, &mModelBlob});
        setOutputData({&mSuccess, &mError, &mErrorCode, &mModelCount, &mInfoModelId, &mInfoVersion, &mInfoSizeBytes,
                       &mInfoSha256});
        setup();
      }

      CIEC_USINT mCommand;
      CIEC_STRING mModelId;
      CIEC_STRING mVersion;
      CIEC_UDINT mSizeBytes;
      CIEC_STRING mSha256;
      CIEC_ANY_VARIANT mModelBlob;

      CIEC_BOOL mSuccess;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
      CIEC_UDINT mModelCount;
      CIEC_STRING mInfoModelId;
      CIEC_STRING mInfoVersion;
      CIEC_UDINT mInfoSizeBytes;
      CIEC_STRING mInfoSha256;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_ModelManagerTests, ML_ModelManager_TestFixture)

  BOOST_AUTO_TEST_CASE(loadAndInfoMockModel) {
    const std::string modelId = "mock.modelmanager.load";

    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(modelId);
    mVersion = CIEC_STRING(std::string("1.2.3"));
    mSizeBytes = CIEC_UDINT(3U);
    mSha256 = CIEC_STRING(std::string("sha256:test"));
    setBlobFromString(mModelBlob, "abc");
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
    BOOST_TEST(static_cast<CIEC_UDINT::TValueType>(mModelCount) >= 1U);

    mCommand = CIEC_USINT(3U);
    mModelId = CIEC_STRING(modelId);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(modelId == mInfoModelId.getStorage());
    BOOST_TEST("1.2.3" == mInfoVersion.getStorage());
    BOOST_TEST(3U == static_cast<CIEC_UDINT::TValueType>(mInfoSizeBytes));
    BOOST_TEST("sha256:test" == mInfoSha256.getStorage());
  }

  BOOST_AUTO_TEST_CASE(listAndUnloadModel) {
    const std::string modelId = "mock.modelmanager.unload";

    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(modelId);
    mVersion = CIEC_STRING(std::string("0.0.1"));
    mSizeBytes = CIEC_UDINT(1U);
    mSha256 = CIEC_STRING(std::string("sha256:remove"));
    setBlobFromString(mModelBlob, "x");
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mSuccess));
    const auto countAfterLoad = static_cast<CIEC_UDINT::TValueType>(mModelCount);

    mCommand = CIEC_USINT(2U);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(static_cast<CIEC_UDINT::TValueType>(mModelCount) >= countAfterLoad);

    mCommand = CIEC_USINT(1U);
    mModelId = CIEC_STRING(modelId);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(static_cast<CIEC_UDINT::TValueType>(mModelCount) + 1U <= countAfterLoad);
  }

  BOOST_AUTO_TEST_CASE(unloadMissingModel) {
    mCommand = CIEC_USINT(1U);
    mModelId = CIEC_STRING(std::string("mock.modelmanager.missing"));
    mVersion = CIEC_STRING(std::string(""));
    mSizeBytes = CIEC_UDINT(0U);
    mSha256 = CIEC_STRING(std::string(""));
    setBlobFromString(mModelBlob, "");
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(3U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
