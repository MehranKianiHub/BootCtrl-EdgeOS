/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/management/ML_OTAUpdate_fbt.h"

#include "forte/eclipse4diac/edgeml/core/model_metadata.h"
#include "forte/eclipse4diac/edgeml/core/ota_persistence.h"
#include "forte/eclipse4diac/edgeml/core/ota_security.h"
#include "forte/eclipse4diac/edgeml/core/ota_trust_store.h"
#include "forte/eclipse4diac/edgeml/core/runtime_context.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    constexpr auto cDefaultStatePath = "./edgeml_ota_state.dat";
    constexpr auto cDefaultTrustStorePath = "./edgeml_trust_store.dat";

    const auto cDataInputNames = std::array{"COMMAND"_STRID,
                                             "MODEL_ID"_STRID,
                                             "VERSION"_STRID,
                                             "EXPECTED_SIZE"_STRID,
                                             "EXPECTED_SHA256"_STRID,
                                             "SIGNATURE"_STRID,
                                             "TRUST_ANCHOR"_STRID,
                                             "TRUST_ANCHOR_ID"_STRID,
                                             "TRUST_STORE_PATH"_STRID,
                                             "SOURCE_URI"_STRID,
                                             "TRANSPORT_SECURE"_STRID,
                                             "NONCE"_STRID,
                                             "STATE_PATH"_STRID,
                                             "CHUNK"_STRID};
    const auto cDataOutputNames = std::array{"STATE"_STRID,
                                              "PROGRESS"_STRID,
                                              "ACTIVE_MODEL_ID"_STRID,
                                              "STAGED_MODEL_ID"_STRID,
                                              "STAGED_SIZE"_STRID,
                                              "ROLLBACK_AVAILABLE"_STRID,
                                              "SUCCESS"_STRID,
                                              "ERROR"_STRID,
                                              "ERROR_CODE"_STRID};
    const auto cEventInputNames = std::array{"REQ"_STRID};
    const auto cEventInputTypeIds = std::array{"Event"_STRID};
    const auto cEventOutputNames = std::array{"CNF"_STRID};
    const auto cEventOutputTypeIds = std::array{"Event"_STRID};

    const SFBInterfaceSpec cFBInterfaceSpec = {
        .mEINames = cEventInputNames,
        .mEITypeNames = cEventInputTypeIds,
        .mEONames = cEventOutputNames,
        .mEOTypeNames = cEventOutputTypeIds,
        .mDINames = cDataInputNames,
        .mDONames = cDataOutputNames,
        .mDIONames = {},
        .mSocketNames = {},
        .mPlugNames = {},
    };

    CIEC_USINT::TValueType mapApplyError(const EEdgeMLError paStatus) {
      switch (paStatus) {
        case EEdgeMLError::kBackendUnavailable: return FORTE_ML_OTAUpdate::scmErrorBackendUnavailable;
        case EEdgeMLError::kInvalidModelId:
        case EEdgeMLError::kInvalidInput: return FORTE_ML_OTAUpdate::scmErrorInvalidPayload;
        default: return FORTE_ML_OTAUpdate::scmErrorApplyFailed;
      }
    }

    CIEC_ARRAY_VARIABLE<CIEC_BYTE> makeEmptyByteArray() {
      CIEC_ARRAY_VARIABLE<CIEC_BYTE> array;
      array.setBounds(0, -1);
      return array;
    }

    bool appendChunkBytes(std::vector<std::uint8_t> &paDestination, const CIEC_ANY_VARIANT &paChunkVariant) {
      if (!std::holds_alternative<CIEC_ANY_UNIQUE_PTR<CIEC_ARRAY>>(paChunkVariant)) {
        return false;
      }

      const auto &chunk = *std::get<CIEC_ANY_UNIQUE_PTR<CIEC_ARRAY>>(paChunkVariant);
      if (CIEC_ANY::e_BYTE != chunk.getElementDataTypeID()) {
        return false;
      }

      paDestination.reserve(paDestination.size() + chunk.size());
      for (intmax_t index = chunk.getLowerBound(), end = chunk.getUpperBound(); index <= end; ++index) {
        paDestination.push_back(static_cast<CIEC_BYTE::TValueType>(static_cast<const CIEC_BYTE &>(chunk[index])));
      }
      return true;
    }

    bool hasSecureScheme(const std::string_view paUri) {
      constexpr std::array<std::string_view, 5> cSecureSchemes = {"https://", "mqtts://", "coaps://", "ftps://",
                                                                   "sftp://"};
      for (const auto scheme : cSecureSchemes) {
        if (paUri.starts_with(scheme)) {
          return true;
        }
      }
      return false;
    }

    bool isValidNonceValue(const std::string &paNonce) {
      if (paNonce.empty() || paNonce.size() > 128U) {
        return false;
      }

      for (const auto raw : paNonce) {
        const auto character = static_cast<unsigned char>(raw);
        if (std::isalnum(character) != 0 || '_' == raw || '-' == raw || '.' == raw || ':' == raw) {
          continue;
        }
        return false;
      }
      return true;
    }

    CIEC_USINT::TValueType mapTrustStoreError(const EOtaTrustStoreStatus paStatus) {
      switch (paStatus) {
        case EOtaTrustStoreStatus::kNotFound: return FORTE_ML_OTAUpdate::scmErrorTrustAnchorUnavailable;
        case EOtaTrustStoreStatus::kInvalidAnchorId:
        case EOtaTrustStoreStatus::kInvalidPath: return FORTE_ML_OTAUpdate::scmErrorInvalidPayload;
        default: return FORTE_ML_OTAUpdate::scmErrorTrustStoreFailure;
      }
    }
  } // namespace

  DEFINE_FIRMWARE_FB(FORTE_ML_OTAUpdate, "eclipse4diac::edgeml::ML_OTAUpdate"_STRID)

  FORTE_ML_OTAUpdate::FORTE_ML_OTAUpdate(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_COMMAND(nullptr),
      conn_MODEL_ID(nullptr),
      conn_VERSION(nullptr),
      conn_EXPECTED_SIZE(nullptr),
      conn_EXPECTED_SHA256(nullptr),
      conn_SIGNATURE(nullptr),
      conn_TRUST_ANCHOR(nullptr),
      conn_TRUST_ANCHOR_ID(nullptr),
      conn_TRUST_STORE_PATH(nullptr),
      conn_SOURCE_URI(nullptr),
      conn_TRANSPORT_SECURE(nullptr),
      conn_NONCE(nullptr),
      conn_STATE_PATH(nullptr),
      conn_CHUNK(nullptr),
      conn_STATE(*this, 0, var_STATE),
      conn_PROGRESS(*this, 1, var_PROGRESS),
      conn_ACTIVE_MODEL_ID(*this, 2, var_ACTIVE_MODEL_ID),
      conn_STAGED_MODEL_ID(*this, 3, var_STAGED_MODEL_ID),
      conn_STAGED_SIZE(*this, 4, var_STAGED_SIZE),
      conn_ROLLBACK_AVAILABLE(*this, 5, var_ROLLBACK_AVAILABLE),
      conn_SUCCESS(*this, 6, var_SUCCESS),
      conn_ERROR(*this, 7, var_ERROR),
      conn_ERROR_CODE(*this, 8, var_ERROR_CODE),
      mExpectedSize(0U),
      mRollbackAvailable(false) {
    setInitialValues();
  }

  void FORTE_ML_OTAUpdate::setInitialValues() {
    var_COMMAND = CIEC_USINT(scmCommandBegin);
    var_MODEL_ID = CIEC_STRING(std::string(""));
    var_VERSION = CIEC_STRING(std::string(""));
    var_EXPECTED_SIZE = CIEC_UDINT(0U);
    var_EXPECTED_SHA256 = CIEC_STRING(std::string(""));
    var_SIGNATURE = CIEC_STRING(std::string(""));
    var_TRUST_ANCHOR = CIEC_STRING(std::string(""));
    var_TRUST_ANCHOR_ID = CIEC_STRING(std::string(""));
    var_TRUST_STORE_PATH = CIEC_STRING(std::string(cDefaultTrustStorePath));
    var_SOURCE_URI = CIEC_STRING(std::string(""));
    var_TRANSPORT_SECURE = CIEC_BOOL(false);
    var_NONCE = CIEC_STRING(std::string(""));
    var_STATE_PATH = CIEC_STRING(std::string(cDefaultStatePath));
    var_CHUNK.setValue(makeEmptyByteArray());

    var_STATE = CIEC_USINT(scmStateIdle);
    var_PROGRESS = CIEC_USINT(0U);
    var_ACTIVE_MODEL_ID = CIEC_STRING(std::string("mock.default"));
    var_STAGED_MODEL_ID = CIEC_STRING(std::string(""));
    var_STAGED_SIZE = CIEC_UDINT(0U);
    var_ROLLBACK_AVAILABLE = CIEC_BOOL(false);
    var_SUCCESS = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);

    mStagedModelId.clear();
    mStagedVersion.clear();
    mStagedExpectedSha256.clear();
    mStagedSignature.clear();
    mStagedTrustAnchor.clear();
    mStagedTrustAnchorId.clear();
    mStagedSourceUri.clear();
    mStagedNonce.clear();
    mStagedBlob.clear();
    mActiveModelId = "mock.default";
    mPreviousActiveModelId.clear();
    mStatePath = cDefaultStatePath;
    mTrustStorePath = cDefaultTrustStorePath;
    mLastAppliedNonce.clear();
    mExpectedSize = 0U;
    mRollbackAvailable = false;

    [[maybe_unused]] const auto recovered = recoverState();
  }

  void FORTE_ML_OTAUpdate::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_SUCCESS = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
  }

  void FORTE_ML_OTAUpdate::clearError() {
    var_SUCCESS = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_OTAUpdate::setState(const CIEC_USINT::TValueType paState) {
    var_STATE = CIEC_USINT(paState);
  }

  void FORTE_ML_OTAUpdate::clearStaging() {
    mStagedModelId.clear();
    mStagedVersion.clear();
    mStagedExpectedSha256.clear();
    mStagedSignature.clear();
    mStagedTrustAnchor.clear();
    mStagedTrustAnchorId.clear();
    mStagedSourceUri.clear();
    mStagedNonce.clear();
    mStagedBlob.clear();
    mExpectedSize = 0U;

    var_STAGED_MODEL_ID = CIEC_STRING(std::string(""));
    var_STAGED_SIZE = CIEC_UDINT(0U);
    var_PROGRESS = CIEC_USINT(0U);
  }

  void FORTE_ML_OTAUpdate::updateProgress() {
    if (0U == mExpectedSize) {
      var_PROGRESS = CIEC_USINT(0U);
      return;
    }

    const auto stagedSize = static_cast<CIEC_UDINT::TValueType>(mStagedBlob.size());
    const auto scaled = static_cast<CIEC_USINT::TValueType>((stagedSize * 100U) / mExpectedSize);
    const auto bounded = static_cast<CIEC_USINT::TValueType>(std::min<CIEC_USINT::TValueType>(99U, scaled));
    var_PROGRESS = CIEC_USINT(bounded);
  }

  bool FORTE_ML_OTAUpdate::validateTransportPolicy() const {
    if (!static_cast<CIEC_BOOL::TValueType>(var_TRANSPORT_SECURE)) {
      return false;
    }

    const auto sourceUri = var_SOURCE_URI.getStorage();
    if (sourceUri.empty()) {
      return false;
    }

    std::string canonicalUri(sourceUri);
    std::transform(canonicalUri.begin(), canonicalUri.end(), canonicalUri.begin(),
                   [](const unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return hasSecureScheme(canonicalUri);
  }

  bool FORTE_ML_OTAUpdate::validateNonce(std::string &paNonceOut) {
    paNonceOut = var_NONCE.getStorage();
    if (!isValidNonceValue(paNonceOut)) {
      return false;
    }
    if (!mLastAppliedNonce.empty() && mLastAppliedNonce == paNonceOut) {
      setError(scmErrorReplayDetected);
      return false;
    }
    return true;
  }

  bool FORTE_ML_OTAUpdate::persistState() {
    OtaPersistentState persistedState{
        .activeModelId = mActiveModelId,
        .previousActiveModelId = mPreviousActiveModelId,
        .stagedModelId = mStagedModelId,
        .stagedVersion = mStagedVersion,
        .lastAppliedNonce = mLastAppliedNonce,
        .rollbackAvailable = mRollbackAvailable,
        .state = static_cast<std::uint8_t>(var_STATE),
        .expectedSize = static_cast<std::uint32_t>(mExpectedSize),
        .stagedSize = static_cast<std::uint32_t>(mStagedBlob.size()),
    };
    return EOtaPersistenceStatus::kOk == OtaPersistence::save(persistedState, mStatePath);
  }

  void FORTE_ML_OTAUpdate::applyRecoveredState() {
    var_ACTIVE_MODEL_ID = CIEC_STRING(mActiveModelId);
    var_STAGED_MODEL_ID = CIEC_STRING(mStagedModelId);
    var_STAGED_SIZE = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(mStagedBlob.size()));
    var_ROLLBACK_AVAILABLE = CIEC_BOOL(mRollbackAvailable);

    const auto state = static_cast<CIEC_USINT::TValueType>(var_STATE);
    if (scmStateCommitted == state) {
      var_PROGRESS = CIEC_USINT(100U);
    } else if (scmStateStaging == state) {
      updateProgress();
    } else {
      var_PROGRESS = CIEC_USINT(0U);
    }
  }

  bool FORTE_ML_OTAUpdate::recoverState() {
    OtaPersistentState recoveredState{};
    const auto status = OtaPersistence::load(mStatePath, recoveredState);
    if (EOtaPersistenceStatus::kNotFound == status) {
      applyRecoveredState();
      return true;
    }
    if (EOtaPersistenceStatus::kOk != status) {
      return false;
    }

    mActiveModelId = recoveredState.activeModelId.empty() ? "mock.default" : recoveredState.activeModelId;
    mPreviousActiveModelId = recoveredState.previousActiveModelId;
    mStagedModelId = recoveredState.stagedModelId;
    mStagedVersion = recoveredState.stagedVersion;
    mStagedExpectedSha256.clear();
    mStagedSignature.clear();
    mStagedTrustAnchor.clear();
    mStagedTrustAnchorId.clear();
    mStagedSourceUri.clear();
    mStagedNonce.clear();
    mLastAppliedNonce = recoveredState.lastAppliedNonce;
    mExpectedSize = static_cast<CIEC_UDINT::TValueType>(recoveredState.expectedSize);
    mStagedBlob.clear();
    mRollbackAvailable = recoveredState.rollbackAvailable;

    const auto recoveredStateValue = static_cast<CIEC_USINT::TValueType>(recoveredState.state);
    if (recoveredStateValue <= scmStateRolledBack) {
      setState(recoveredStateValue);
    } else {
      setState(scmStateIdle);
    }

    auto &runtime = EdgeMLRuntime::instance();
    if (!runtime.hasModel(mActiveModelId)) {
      mActiveModelId = "mock.default";
      mPreviousActiveModelId.clear();
      mRollbackAvailable = false;
      setState(scmStateIdle);
    } else if (mRollbackAvailable && (mPreviousActiveModelId.empty() || !runtime.hasModel(mPreviousActiveModelId))) {
      mRollbackAvailable = false;
    }

    if (scmStateStaging == static_cast<CIEC_USINT::TValueType>(var_STATE)) {
      clearStaging();
      setState(scmStateIdle);
    }

    applyRecoveredState();
    [[maybe_unused]] const auto persisted = persistState();
    return true;
  }

  void FORTE_ML_OTAUpdate::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    clearError();
    if (!var_STATE_PATH.empty()) {
      mStatePath = var_STATE_PATH.getStorage();
    }
    if (!var_TRUST_STORE_PATH.empty()) {
      mTrustStorePath = var_TRUST_STORE_PATH.getStorage();
    }
    var_ROLLBACK_AVAILABLE = CIEC_BOOL(mRollbackAvailable);

    auto &runtime = EdgeMLRuntime::instance();
    const auto command = static_cast<CIEC_USINT::TValueType>(var_COMMAND);
    switch (command) {
      case scmCommandBegin: {
        if (scmStateStaging == static_cast<CIEC_USINT::TValueType>(var_STATE)) {
          setError(scmErrorInvalidStateTransition);
          break;
        }

        if (var_MODEL_ID.empty()) {
          setError(scmErrorEmptyModelId);
          break;
        }

        const auto &modelId = var_MODEL_ID.getStorage();
        if (!runtime.backendAvailableForModel(modelId)) {
          setError(scmErrorBackendUnavailable);
          break;
        }

        if (!validateTransportPolicy()) {
          setError(scmErrorTransportPolicyViolation);
          break;
        }

        std::string nonce;
        if (!validateNonce(nonce)) {
          if (scmErrorOk == static_cast<CIEC_USINT::TValueType>(var_ERROR_CODE)) {
            setError(scmErrorInvalidPayload);
          }
          break;
        }

        mStagedModelId = modelId;
        mStagedVersion = var_VERSION.getStorage();
        mStagedExpectedSha256 = var_EXPECTED_SHA256.getStorage();
        mStagedSignature = var_SIGNATURE.getStorage();
        mStagedTrustAnchor = var_TRUST_ANCHOR.getStorage();
        mStagedTrustAnchorId = var_TRUST_ANCHOR_ID.getStorage();
        mStagedSourceUri = var_SOURCE_URI.getStorage();
        mStagedNonce = nonce;
        mStagedBlob.clear();
        mExpectedSize = static_cast<CIEC_UDINT::TValueType>(var_EXPECTED_SIZE);

        var_STAGED_MODEL_ID = CIEC_STRING(mStagedModelId);
        var_STAGED_SIZE = CIEC_UDINT(0U);
        var_PROGRESS = CIEC_USINT(0U);
        setState(scmStateStaging);
        if (!persistState()) {
          setError(scmErrorPersistenceFailed);
        }
        break;
      }

      case scmCommandChunk: {
        if (scmStateStaging != static_cast<CIEC_USINT::TValueType>(var_STATE)) {
          setError(scmErrorInvalidStateTransition);
          break;
        }

        if (!appendChunkBytes(mStagedBlob, var_CHUNK)) {
          setError(scmErrorInvalidPayload);
          break;
        }

        if (0U != mExpectedSize && mStagedBlob.size() > mExpectedSize) {
          setError(scmErrorInvalidPayload);
          break;
        }

        var_STAGED_SIZE = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(mStagedBlob.size()));
        updateProgress();
        if (!persistState()) {
          setError(scmErrorPersistenceFailed);
        }
        break;
      }

      case scmCommandCommit: {
        if (scmStateStaging != static_cast<CIEC_USINT::TValueType>(var_STATE)) {
          setError(scmErrorInvalidStateTransition);
          break;
        }

        if (mStagedModelId.empty()) {
          setError(scmErrorEmptyModelId);
          break;
        }

        auto modelBinary = mStagedBlob;
        if (modelBinary.empty()) {
          if (EdgeMLRuntime::isMockModelId(mStagedModelId)) {
            modelBinary.emplace_back(0x00U);
          } else {
            setError(scmErrorInvalidPayload);
            break;
          }
        }

        if (0U != mExpectedSize && modelBinary.size() != mExpectedSize) {
          setError(scmErrorInvalidPayload);
          break;
        }

        const auto expectedDigest = OtaSecurity::normalizeHexDigest(mStagedExpectedSha256);
        if (expectedDigest.empty()) {
          setError(scmErrorInvalidPayload);
          break;
        }

        const auto payloadDigest = OtaSecurity::computeSha256Hex(modelBinary);
        if (!OtaSecurity::constantTimeEqual(payloadDigest, expectedDigest)) {
          setError(scmErrorHashMismatch);
          break;
        }

        if (mStagedSignature.empty()) {
          setError(scmErrorInvalidPayload);
          break;
        }

        std::string trustAnchor = mStagedTrustAnchor;
        if (!mStagedTrustAnchorId.empty()) {
          const auto trustStatus = OtaTrustStore::getAnchor(mTrustStorePath, mStagedTrustAnchorId, trustAnchor);
          if (EOtaTrustStoreStatus::kOk != trustStatus) {
            setError(mapTrustStoreError(trustStatus));
            break;
          }
        }
        if (trustAnchor.empty()) {
          setError(scmErrorTrustAnchorUnavailable);
          break;
        }

        const auto normalizedSignature = OtaSecurity::normalizeHexDigest(mStagedSignature);
        const auto expectedSignature = OtaSecurity::deriveSignature(payloadDigest, trustAnchor);
        if (normalizedSignature.empty() || expectedSignature.empty()) {
          setError(scmErrorInvalidPayload);
          break;
        }
        if (!OtaSecurity::constantTimeEqual(normalizedSignature, expectedSignature)) {
          setError(scmErrorSignatureMismatch);
          break;
        }

        if (!mLastAppliedNonce.empty() && mLastAppliedNonce == mStagedNonce) {
          setError(scmErrorReplayDetected);
          break;
        }

        const auto metadataSize = 0U == mExpectedSize ? modelBinary.size() : static_cast<std::size_t>(mExpectedSize);
        const ModelMetadata metadata{mStagedModelId, mStagedVersion, metadataSize, "sha256:" + payloadDigest};
        const auto status = runtime.loadModel(metadata, modelBinary);
        if (EEdgeMLError::kOk != status && EEdgeMLError::kModelAlreadyExists != status) {
          setError(mapApplyError(status));
          break;
        }

        mPreviousActiveModelId = mActiveModelId;
        mActiveModelId = mStagedModelId;
        mLastAppliedNonce = mStagedNonce;
        mRollbackAvailable = !mPreviousActiveModelId.empty() && mPreviousActiveModelId != mActiveModelId;

        var_ACTIVE_MODEL_ID = CIEC_STRING(mActiveModelId);
        var_ROLLBACK_AVAILABLE = CIEC_BOOL(mRollbackAvailable);
        var_STAGED_SIZE = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(mStagedBlob.size()));
        var_PROGRESS = CIEC_USINT(100U);
        setState(scmStateCommitted);
        if (!persistState()) {
          setError(scmErrorPersistenceFailed);
        }
        break;
      }

      case scmCommandAbort: {
        if (scmStateStaging != static_cast<CIEC_USINT::TValueType>(var_STATE)) {
          setError(scmErrorInvalidStateTransition);
          break;
        }

        clearStaging();
        setState(scmStateIdle);
        if (!persistState()) {
          setError(scmErrorPersistenceFailed);
        }
        break;
      }

      case scmCommandRollback: {
        if (!mRollbackAvailable || mPreviousActiveModelId.empty()) {
          setError(scmErrorRollbackUnavailable);
          break;
        }

        mActiveModelId = mPreviousActiveModelId;
        mRollbackAvailable = false;

        var_ACTIVE_MODEL_ID = CIEC_STRING(mActiveModelId);
        var_ROLLBACK_AVAILABLE = CIEC_BOOL(false);
        clearStaging();
        setState(scmStateRolledBack);
        if (!persistState()) {
          setError(scmErrorPersistenceFailed);
        }
        break;
      }

      case scmCommandRecover: {
        if (!recoverState()) {
          setError(scmErrorRecoveryFailed);
        }
        break;
      }

      case scmCommandProvisionAnchor: {
        if (var_TRUST_ANCHOR_ID.empty() || var_TRUST_ANCHOR.empty()) {
          setError(scmErrorInvalidPayload);
          break;
        }

        const auto status = OtaTrustStore::putAnchor(mTrustStorePath, var_TRUST_ANCHOR_ID.getStorage(),
                                                     var_TRUST_ANCHOR.getStorage());
        if (EOtaTrustStoreStatus::kOk != status) {
          setError(mapTrustStoreError(status));
        }
        break;
      }

      case scmCommandRemoveAnchor: {
        if (var_TRUST_ANCHOR_ID.empty()) {
          setError(scmErrorInvalidPayload);
          break;
        }

        const auto status = OtaTrustStore::removeAnchor(mTrustStorePath, var_TRUST_ANCHOR_ID.getStorage());
        if (EOtaTrustStoreStatus::kOk != status) {
          setError(mapTrustStoreError(status));
        }
        break;
      }

      default: {
        setError(scmErrorInvalidCommand);
        break;
      }
    }

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_OTAUpdate::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_COMMAND, conn_COMMAND);
        readData(1, var_MODEL_ID, conn_MODEL_ID);
        readData(2, var_VERSION, conn_VERSION);
        readData(3, var_EXPECTED_SIZE, conn_EXPECTED_SIZE);
        readData(4, var_EXPECTED_SHA256, conn_EXPECTED_SHA256);
        readData(5, var_SIGNATURE, conn_SIGNATURE);
        readData(6, var_TRUST_ANCHOR, conn_TRUST_ANCHOR);
        readData(7, var_TRUST_ANCHOR_ID, conn_TRUST_ANCHOR_ID);
        readData(8, var_TRUST_STORE_PATH, conn_TRUST_STORE_PATH);
        readData(9, var_SOURCE_URI, conn_SOURCE_URI);
        readData(10, var_TRANSPORT_SECURE, conn_TRANSPORT_SECURE);
        readData(11, var_NONCE, conn_NONCE);
        readData(12, var_STATE_PATH, conn_STATE_PATH);
        readData(13, var_CHUNK, conn_CHUNK);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_OTAUpdate::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_STATE, conn_STATE);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_PROGRESS, conn_PROGRESS);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_ACTIVE_MODEL_ID, conn_ACTIVE_MODEL_ID);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_STAGED_MODEL_ID, conn_STAGED_MODEL_ID);
        writeData(cFBInterfaceSpec.getNumDIs() + 4, var_STAGED_SIZE, conn_STAGED_SIZE);
        writeData(cFBInterfaceSpec.getNumDIs() + 5, var_ROLLBACK_AVAILABLE, conn_ROLLBACK_AVAILABLE);
        writeData(cFBInterfaceSpec.getNumDIs() + 6, var_SUCCESS, conn_SUCCESS);
        writeData(cFBInterfaceSpec.getNumDIs() + 7, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 8, var_ERROR_CODE, conn_ERROR_CODE);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_OTAUpdate::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_COMMAND;
      case 1: return &var_MODEL_ID;
      case 2: return &var_VERSION;
      case 3: return &var_EXPECTED_SIZE;
      case 4: return &var_EXPECTED_SHA256;
      case 5: return &var_SIGNATURE;
      case 6: return &var_TRUST_ANCHOR;
      case 7: return &var_TRUST_ANCHOR_ID;
      case 8: return &var_TRUST_STORE_PATH;
      case 9: return &var_SOURCE_URI;
      case 10: return &var_TRANSPORT_SECURE;
      case 11: return &var_NONCE;
      case 12: return &var_STATE_PATH;
      case 13: return &var_CHUNK;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_OTAUpdate::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_STATE;
      case 1: return &var_PROGRESS;
      case 2: return &var_ACTIVE_MODEL_ID;
      case 3: return &var_STAGED_MODEL_ID;
      case 4: return &var_STAGED_SIZE;
      case 5: return &var_ROLLBACK_AVAILABLE;
      case 6: return &var_SUCCESS;
      case 7: return &var_ERROR;
      case 8: return &var_ERROR_CODE;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_OTAUpdate::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_OTAUpdate::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_COMMAND;
      case 1: return &conn_MODEL_ID;
      case 2: return &conn_VERSION;
      case 3: return &conn_EXPECTED_SIZE;
      case 4: return &conn_EXPECTED_SHA256;
      case 5: return &conn_SIGNATURE;
      case 6: return &conn_TRUST_ANCHOR;
      case 7: return &conn_TRUST_ANCHOR_ID;
      case 8: return &conn_TRUST_STORE_PATH;
      case 9: return &conn_SOURCE_URI;
      case 10: return &conn_TRANSPORT_SECURE;
      case 11: return &conn_NONCE;
      case 12: return &conn_STATE_PATH;
      case 13: return &conn_CHUNK;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_OTAUpdate::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_STATE;
      case 1: return &conn_PROGRESS;
      case 2: return &conn_ACTIVE_MODEL_ID;
      case 3: return &conn_STAGED_MODEL_ID;
      case 4: return &conn_STAGED_SIZE;
      case 5: return &conn_ROLLBACK_AVAILABLE;
      case 6: return &conn_SUCCESS;
      case 7: return &conn_ERROR;
      case 8: return &conn_ERROR_CODE;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
