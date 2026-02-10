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

#include <string>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml::test {

  struct ML_OTAUpdate_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_OTAUpdate_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_OTAUpdate"_STRID) {
        setInputData({&mCommand, &mModelId, &mVersion, &mExpectedSize, &mChunk});
        setOutputData({&mState, &mProgress, &mActiveModelId, &mStagedModelId, &mStagedSize, &mRollbackAvailable,
                       &mSuccess, &mError, &mErrorCode});
        setup();
      }

      CIEC_USINT mCommand;
      CIEC_STRING mModelId;
      CIEC_STRING mVersion;
      CIEC_UDINT mExpectedSize;
      CIEC_STRING mChunk;

      CIEC_USINT mState;
      CIEC_USINT mProgress;
      CIEC_STRING mActiveModelId;
      CIEC_STRING mStagedModelId;
      CIEC_UDINT mStagedSize;
      CIEC_BOOL mRollbackAvailable;
      CIEC_BOOL mSuccess;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
  };

  BOOST_FIXTURE_TEST_SUITE(ML_OTAUpdateTests, ML_OTAUpdate_TestFixture)

  BOOST_AUTO_TEST_CASE(beginChunkCommitFlow) {
    const std::string modelId = "mock.ota.v1";

    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(modelId);
    mVersion = CIEC_STRING(std::string("1.0.0"));
    mExpectedSize = CIEC_UDINT(3U);
    mChunk = CIEC_STRING(std::string(""));
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mState));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mProgress));
    BOOST_TEST(modelId == mStagedModelId.getStorage());
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mSuccess));

    mCommand = CIEC_USINT(1U);
    mChunk = CIEC_STRING(std::string("abc"));
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mState));
    BOOST_TEST(3U == static_cast<CIEC_UDINT::TValueType>(mStagedSize));
    BOOST_TEST(99U == static_cast<CIEC_USINT::TValueType>(mProgress));

    mCommand = CIEC_USINT(2U);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mState));
    BOOST_TEST(100U == static_cast<CIEC_USINT::TValueType>(mProgress));
    BOOST_TEST(modelId == mActiveModelId.getStorage());
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mRollbackAvailable));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(rollbackRestoresPreviousActiveMarker) {
    const std::string modelA = "mock.ota.rollback.a";
    const std::string modelB = "mock.ota.rollback.b";

    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(modelA);
    mVersion = CIEC_STRING(std::string("1.0.0"));
    mExpectedSize = CIEC_UDINT(1U);
    mChunk = CIEC_STRING(std::string(""));
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(1U);
    mChunk = CIEC_STRING(std::string("a"));
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    mCommand = CIEC_USINT(2U);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    BOOST_TEST(modelA == mActiveModelId.getStorage());

    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(modelB);
    mVersion = CIEC_STRING(std::string("2.0.0"));
    mExpectedSize = CIEC_UDINT(1U);
    mChunk = CIEC_STRING(std::string(""));
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    mCommand = CIEC_USINT(1U);
    mChunk = CIEC_STRING(std::string("b"));
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    mCommand = CIEC_USINT(2U);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    BOOST_TEST(modelB == mActiveModelId.getStorage());
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mRollbackAvailable));

    mCommand = CIEC_USINT(4U);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(3U == static_cast<CIEC_USINT::TValueType>(mState));
    BOOST_TEST(modelA == mActiveModelId.getStorage());
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mRollbackAvailable));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
  }

  BOOST_AUTO_TEST_CASE(invalidStateTransitionChunkWithoutBegin) {
    mCommand = CIEC_USINT(1U);
    mModelId = CIEC_STRING(std::string("mock.ota.invalid"));
    mVersion = CIEC_STRING(std::string("1.0.0"));
    mExpectedSize = CIEC_UDINT(1U);
    mChunk = CIEC_STRING(std::string("x"));
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(beginWithEmptyModelId) {
    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(std::string(""));
    mVersion = CIEC_STRING(std::string("1.0.0"));
    mExpectedSize = CIEC_UDINT(10U);
    mChunk = CIEC_STRING(std::string(""));
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(3U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(abortClearsStaging) {
    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(std::string("mock.ota.abort"));
    mVersion = CIEC_STRING(std::string("1.0.0"));
    mExpectedSize = CIEC_UDINT(4U);
    mChunk = CIEC_STRING(std::string(""));
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(1U);
    mChunk = CIEC_STRING(std::string("ab"));
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(3U);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mState));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mProgress));
    BOOST_TEST("" == mStagedModelId.getStorage());
    BOOST_TEST(0U == static_cast<CIEC_UDINT::TValueType>(mStagedSize));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mSuccess));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
