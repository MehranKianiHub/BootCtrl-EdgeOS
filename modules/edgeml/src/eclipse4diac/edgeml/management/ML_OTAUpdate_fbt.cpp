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
#include "forte/eclipse4diac/edgeml/core/runtime_context.h"

#include <algorithm>
#include <array>
#include <vector>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames = std::array{"COMMAND"_STRID, "MODEL_ID"_STRID, "VERSION"_STRID,
                                             "EXPECTED_SIZE"_STRID, "CHUNK"_STRID};
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
  } // namespace

  DEFINE_FIRMWARE_FB(FORTE_ML_OTAUpdate, "eclipse4diac::edgeml::ML_OTAUpdate"_STRID)

  FORTE_ML_OTAUpdate::FORTE_ML_OTAUpdate(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_COMMAND(nullptr),
      conn_MODEL_ID(nullptr),
      conn_VERSION(nullptr),
      conn_EXPECTED_SIZE(nullptr),
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
    var_CHUNK = CIEC_STRING(std::string(""));

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
    mStagedBlob.clear();
    mActiveModelId = "mock.default";
    mPreviousActiveModelId.clear();
    mExpectedSize = 0U;
    mRollbackAvailable = false;
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

  void FORTE_ML_OTAUpdate::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    clearError();
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

        mStagedModelId = modelId;
        mStagedVersion = var_VERSION.getStorage();
        mStagedBlob.clear();
        mExpectedSize = static_cast<CIEC_UDINT::TValueType>(var_EXPECTED_SIZE);

        var_STAGED_MODEL_ID = CIEC_STRING(mStagedModelId);
        var_STAGED_SIZE = CIEC_UDINT(0U);
        var_PROGRESS = CIEC_USINT(0U);
        setState(scmStateStaging);
        break;
      }

      case scmCommandChunk: {
        if (scmStateStaging != static_cast<CIEC_USINT::TValueType>(var_STATE)) {
          setError(scmErrorInvalidStateTransition);
          break;
        }

        mStagedBlob.append(var_CHUNK.getStorage());
        var_STAGED_SIZE = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(mStagedBlob.size()));
        updateProgress();
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

        std::vector<std::uint8_t> modelBinary(mStagedBlob.begin(), mStagedBlob.end());
        if (modelBinary.empty()) {
          if (EdgeMLRuntime::isMockModelId(mStagedModelId)) {
            modelBinary.emplace_back(0x00);
          } else {
            setError(scmErrorInvalidPayload);
            break;
          }
        }

        const auto metadataSize = 0U == mExpectedSize ? modelBinary.size() : static_cast<std::size_t>(mExpectedSize);
        const ModelMetadata metadata{
            mStagedModelId, mStagedVersion, metadataSize, "sha256:ota:" + std::to_string(metadataSize)};
        const auto status = runtime.loadModel(metadata, modelBinary);
        if (EEdgeMLError::kOk != status && EEdgeMLError::kModelAlreadyExists != status) {
          setError(mapApplyError(status));
          break;
        }

        mPreviousActiveModelId = mActiveModelId;
        mActiveModelId = mStagedModelId;
        mRollbackAvailable = !mPreviousActiveModelId.empty() && mPreviousActiveModelId != mActiveModelId;

        var_ACTIVE_MODEL_ID = CIEC_STRING(mActiveModelId);
        var_ROLLBACK_AVAILABLE = CIEC_BOOL(mRollbackAvailable);
        var_STAGED_SIZE = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(mStagedBlob.size()));
        var_PROGRESS = CIEC_USINT(100U);
        setState(scmStateCommitted);
        break;
      }

      case scmCommandAbort: {
        if (scmStateStaging != static_cast<CIEC_USINT::TValueType>(var_STATE)) {
          setError(scmErrorInvalidStateTransition);
          break;
        }

        clearStaging();
        setState(scmStateIdle);
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
        readData(4, var_CHUNK, conn_CHUNK);
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
      case 4: return &var_CHUNK;
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
      case 4: return &conn_CHUNK;
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
