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
#include "forte/eclipse4diac/edgeml/core/ota_security.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml::test {
  namespace {
    std::vector<std::uint8_t> bytesFromString(const std::string &paValue) {
      return std::vector<std::uint8_t>(paValue.begin(), paValue.end());
    }

    void setChunkFromString(CIEC_ANY_VARIANT &paTarget, const std::string &paValue) {
      CIEC_ARRAY_VARIABLE<CIEC_BYTE> chunk;
      if (paValue.empty()) {
        chunk.setBounds(0, -1);
      } else {
        chunk.setBounds(0, static_cast<intmax_t>(paValue.size()) - 1);
        for (std::size_t i = 0; i < paValue.size(); ++i) {
          chunk[static_cast<intmax_t>(i)] = CIEC_BYTE(static_cast<CIEC_BYTE::TValueType>(paValue[i]));
        }
      }
      paTarget.setValue(chunk);
    }

    std::string makeUniqueStatePath() {
      static std::atomic<std::uint64_t> sequence{0U};
      const auto timePart = static_cast<std::uint64_t>(
          std::chrono::steady_clock::now().time_since_epoch().count());
      return "/tmp/edgeml_ota_state_" + std::to_string(timePart) + "_" +
             std::to_string(sequence.fetch_add(1U, std::memory_order_relaxed)) + ".dat";
    }

    void cleanupStateFile(const std::string &paPath) {
      std::error_code error;
      std::filesystem::remove(paPath, error);
      std::filesystem::remove(paPath + ".tmp", error);
    }
  } // namespace

  struct ML_OTAUpdate_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_OTAUpdate_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_OTAUpdate"_STRID) {
        setInputData({&mCommand,
                      &mModelId,
                      &mVersion,
                      &mExpectedSize,
                      &mExpectedSha256,
                      &mSignature,
                      &mTrustAnchor,
                      &mStatePath,
                      &mChunk});
        setOutputData({&mState, &mProgress, &mActiveModelId, &mStagedModelId, &mStagedSize, &mRollbackAvailable,
                       &mSuccess, &mError, &mErrorCode});
        setup();
      }

      CIEC_USINT mCommand;
      CIEC_STRING mModelId;
      CIEC_STRING mVersion;
      CIEC_UDINT mExpectedSize;
      CIEC_STRING mExpectedSha256;
      CIEC_STRING mSignature;
      CIEC_STRING mTrustAnchor;
      CIEC_STRING mStatePath;
      CIEC_ANY_VARIANT mChunk;

      CIEC_USINT mState;
      CIEC_USINT mProgress;
      CIEC_STRING mActiveModelId;
      CIEC_STRING mStagedModelId;
      CIEC_UDINT mStagedSize;
      CIEC_BOOL mRollbackAvailable;
      CIEC_BOOL mSuccess;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;

      void fire() {
        triggerEvent(0);
      }

      bool hasSingleCnf() {
        return checkForSingleOutputEventOccurence(0);
      }
  };

  BOOST_FIXTURE_TEST_SUITE(ML_OTAUpdateTests, ML_OTAUpdate_TestFixture)

  BOOST_AUTO_TEST_CASE(beginChunkCommitFlow) {
    const std::string statePath = makeUniqueStatePath();
    cleanupStateFile(statePath);

    const std::string modelId = "mock.ota.v1";
    const std::string payload = "abc";
    const std::string trustAnchor = "factory-key-v1";
    const auto payloadHash = OtaSecurity::computeSha256Hex(bytesFromString(payload));
    const auto signature = OtaSecurity::deriveSignature(payloadHash, trustAnchor);

    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(modelId);
    mVersion = CIEC_STRING(std::string("1.0.0"));
    mExpectedSize = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(payload.size()));
    mExpectedSha256 = CIEC_STRING(std::string("sha256:") + payloadHash);
    mSignature = CIEC_STRING(signature);
    mTrustAnchor = CIEC_STRING(trustAnchor);
    mStatePath = CIEC_STRING(statePath);
    setChunkFromString(mChunk, "");
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mState));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mProgress));
    BOOST_TEST(modelId == mStagedModelId.getStorage());
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mSuccess));

    mCommand = CIEC_USINT(1U);
    setChunkFromString(mChunk, payload);
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

    cleanupStateFile(statePath);
  }

  BOOST_AUTO_TEST_CASE(hashMismatchRejectsCommit) {
    const std::string statePath = makeUniqueStatePath();
    cleanupStateFile(statePath);

    const std::string payload = "abc";
    const std::string modelId = "mock.ota.hash.fail";
    const std::string trustAnchor = "factory-key-v1";
    const auto wrongHash = OtaSecurity::computeSha256Hex(bytesFromString("abd"));
    const auto signature = OtaSecurity::deriveSignature(wrongHash, trustAnchor);

    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(modelId);
    mVersion = CIEC_STRING(std::string("1.0.0"));
    mExpectedSize = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(payload.size()));
    mExpectedSha256 = CIEC_STRING(std::string("sha256:") + wrongHash);
    mSignature = CIEC_STRING(signature);
    mTrustAnchor = CIEC_STRING(trustAnchor);
    mStatePath = CIEC_STRING(statePath);
    setChunkFromString(mChunk, "");
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(1U);
    setChunkFromString(mChunk, payload);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(2U);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(8U == static_cast<CIEC_USINT::TValueType>(mErrorCode));

    cleanupStateFile(statePath);
  }

  BOOST_AUTO_TEST_CASE(signatureMismatchRejectsCommit) {
    const std::string statePath = makeUniqueStatePath();
    cleanupStateFile(statePath);

    const std::string payload = "abc";
    const std::string modelId = "mock.ota.signature.fail";
    const std::string trustAnchor = "factory-key-v1";
    const auto payloadHash = OtaSecurity::computeSha256Hex(bytesFromString(payload));
    const auto wrongSignature = OtaSecurity::deriveSignature(payloadHash, "different-trust-anchor");

    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(modelId);
    mVersion = CIEC_STRING(std::string("1.0.0"));
    mExpectedSize = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(payload.size()));
    mExpectedSha256 = CIEC_STRING(std::string("sha256:") + payloadHash);
    mSignature = CIEC_STRING(wrongSignature);
    mTrustAnchor = CIEC_STRING(trustAnchor);
    mStatePath = CIEC_STRING(statePath);
    setChunkFromString(mChunk, "");
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(1U);
    setChunkFromString(mChunk, payload);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(2U);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(9U == static_cast<CIEC_USINT::TValueType>(mErrorCode));

    cleanupStateFile(statePath);
  }

  BOOST_AUTO_TEST_CASE(recoverRestoresCommittedMarkers) {
    const std::string statePath = makeUniqueStatePath();
    cleanupStateFile(statePath);
    const std::string trustAnchor = "factory-key-v1";

    const std::string modelA = "mock.ota.rollback.a";
    const std::string payloadA = "a";
    const auto hashA = OtaSecurity::computeSha256Hex(bytesFromString(payloadA));
    const auto signatureA = OtaSecurity::deriveSignature(hashA, trustAnchor);

    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(modelA);
    mVersion = CIEC_STRING(std::string("1.0.0"));
    mExpectedSize = CIEC_UDINT(1U);
    mExpectedSha256 = CIEC_STRING(std::string("sha256:") + hashA);
    mSignature = CIEC_STRING(signatureA);
    mTrustAnchor = CIEC_STRING(trustAnchor);
    mStatePath = CIEC_STRING(statePath);
    setChunkFromString(mChunk, "");
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(1U);
    setChunkFromString(mChunk, payloadA);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(2U);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(modelA == mActiveModelId.getStorage());

    const std::string modelB = "mock.ota.rollback.b";
    const std::string payloadB = "b";
    const auto hashB = OtaSecurity::computeSha256Hex(bytesFromString(payloadB));
    const auto signatureB = OtaSecurity::deriveSignature(hashB, trustAnchor);

    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(modelB);
    mVersion = CIEC_STRING(std::string("2.0.0"));
    mExpectedSize = CIEC_UDINT(1U);
    mExpectedSha256 = CIEC_STRING(std::string("sha256:") + hashB);
    mSignature = CIEC_STRING(signatureB);
    mTrustAnchor = CIEC_STRING(trustAnchor);
    setChunkFromString(mChunk, "");
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(1U);
    setChunkFromString(mChunk, payloadB);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(2U);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(modelB == mActiveModelId.getStorage());
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mRollbackAvailable));

    ML_OTAUpdate_TestFixture recoveredFixture;
    recoveredFixture.mCommand = CIEC_USINT(5U);
    recoveredFixture.mStatePath = CIEC_STRING(statePath);
    recoveredFixture.fire();

    BOOST_TEST(recoveredFixture.hasSingleCnf());
    BOOST_TEST(modelB == recoveredFixture.mActiveModelId.getStorage());
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(recoveredFixture.mRollbackAvailable));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(recoveredFixture.mState));

    recoveredFixture.mCommand = CIEC_USINT(4U);
    recoveredFixture.fire();

    BOOST_TEST(recoveredFixture.hasSingleCnf());
    BOOST_TEST(3U == static_cast<CIEC_USINT::TValueType>(recoveredFixture.mState));
    BOOST_TEST(modelA == recoveredFixture.mActiveModelId.getStorage());
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(recoveredFixture.mRollbackAvailable));

    cleanupStateFile(statePath);
  }

  BOOST_AUTO_TEST_CASE(recoverFromStagingFallsBackToIdle) {
    const std::string statePath = makeUniqueStatePath();
    cleanupStateFile(statePath);

    const std::string modelId = "mock.ota.staging";
    const std::string payload = "ab";
    const std::string trustAnchor = "factory-key-v1";
    const auto payloadHash = OtaSecurity::computeSha256Hex(bytesFromString(payload));
    const auto signature = OtaSecurity::deriveSignature(payloadHash, trustAnchor);

    mCommand = CIEC_USINT(0U);
    mModelId = CIEC_STRING(modelId);
    mVersion = CIEC_STRING(std::string("1.0.0"));
    mExpectedSize = CIEC_UDINT(2U);
    mExpectedSha256 = CIEC_STRING(std::string("sha256:") + payloadHash);
    mSignature = CIEC_STRING(signature);
    mTrustAnchor = CIEC_STRING(trustAnchor);
    mStatePath = CIEC_STRING(statePath);
    setChunkFromString(mChunk, "");
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(1U);
    setChunkFromString(mChunk, payload);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mState));

    ML_OTAUpdate_TestFixture recoveredFixture;
    recoveredFixture.mCommand = CIEC_USINT(5U);
    recoveredFixture.mStatePath = CIEC_STRING(statePath);
    recoveredFixture.fire();

    BOOST_TEST(recoveredFixture.hasSingleCnf());
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(recoveredFixture.mSuccess));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(recoveredFixture.mState));
    BOOST_TEST("" == recoveredFixture.mStagedModelId.getStorage());
    BOOST_TEST(0U == static_cast<CIEC_UDINT::TValueType>(recoveredFixture.mStagedSize));

    cleanupStateFile(statePath);
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
