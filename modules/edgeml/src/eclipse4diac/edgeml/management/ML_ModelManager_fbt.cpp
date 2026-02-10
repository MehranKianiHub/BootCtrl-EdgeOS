/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/management/ML_ModelManager_fbt.h"

#include "forte/eclipse4diac/edgeml/core/model_metadata.h"
#include "forte/eclipse4diac/edgeml/core/runtime_context.h"

#include <array>
#include <string>
#include <vector>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames =
        std::array{"COMMAND"_STRID, "MODEL_ID"_STRID, "VERSION"_STRID, "SIZE_BYTES"_STRID, "SHA256"_STRID,
                    "MODEL_BLOB"_STRID};
    const auto cDataOutputNames = std::array{"SUCCESS"_STRID,        "ERROR"_STRID,      "ERROR_CODE"_STRID,
                                              "MODEL_COUNT"_STRID,    "INFO_MODEL_ID"_STRID, "INFO_VERSION"_STRID,
                                              "INFO_SIZE_BYTES"_STRID, "INFO_SHA256"_STRID};
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

    CIEC_USINT::TValueType mapBackendStatus(const EEdgeMLError paStatus) {
      switch (paStatus) {
        case EEdgeMLError::kInvalidModelId:
        case EEdgeMLError::kInvalidInput: return FORTE_ML_ModelManager::scmErrorInvalidInput;
        case EEdgeMLError::kModelNotFound:
        case EEdgeMLError::kModelNotLoaded: return FORTE_ML_ModelManager::scmErrorModelNotFound;
        case EEdgeMLError::kBackendUnavailable: return FORTE_ML_ModelManager::scmErrorBackendUnavailable;
        default: return FORTE_ML_ModelManager::scmErrorBackendFailure;
      }
    }
  } // namespace

  DEFINE_FIRMWARE_FB(FORTE_ML_ModelManager, "eclipse4diac::edgeml::ML_ModelManager"_STRID)

  FORTE_ML_ModelManager::FORTE_ML_ModelManager(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_COMMAND(nullptr),
      conn_MODEL_ID(nullptr),
      conn_VERSION(nullptr),
      conn_SIZE_BYTES(nullptr),
      conn_SHA256(nullptr),
      conn_MODEL_BLOB(nullptr),
      conn_SUCCESS(*this, 0, var_SUCCESS),
      conn_ERROR(*this, 1, var_ERROR),
      conn_ERROR_CODE(*this, 2, var_ERROR_CODE),
      conn_MODEL_COUNT(*this, 3, var_MODEL_COUNT),
      conn_INFO_MODEL_ID(*this, 4, var_INFO_MODEL_ID),
      conn_INFO_VERSION(*this, 5, var_INFO_VERSION),
      conn_INFO_SIZE_BYTES(*this, 6, var_INFO_SIZE_BYTES),
      conn_INFO_SHA256(*this, 7, var_INFO_SHA256) {
  }

  void FORTE_ML_ModelManager::setInitialValues() {
    var_COMMAND = CIEC_USINT(scmCommandList);
    var_MODEL_ID = CIEC_STRING(std::string(""));
    var_VERSION = CIEC_STRING(std::string(""));
    var_SIZE_BYTES = CIEC_UDINT(0U);
    var_SHA256 = CIEC_STRING(std::string(""));
    var_MODEL_BLOB = CIEC_STRING(std::string(""));

    var_SUCCESS = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
    var_MODEL_COUNT = CIEC_UDINT(0U);
    clearInfo();
  }

  void FORTE_ML_ModelManager::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_SUCCESS = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
  }

  void FORTE_ML_ModelManager::clearError() {
    var_SUCCESS = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_ModelManager::clearInfo() {
    var_INFO_MODEL_ID = CIEC_STRING(std::string(""));
    var_INFO_VERSION = CIEC_STRING(std::string(""));
    var_INFO_SIZE_BYTES = CIEC_UDINT(0U);
    var_INFO_SHA256 = CIEC_STRING(std::string(""));
  }

  void FORTE_ML_ModelManager::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    auto &runtime = EdgeMLRuntime::instance();
    clearInfo();

    const auto command = static_cast<CIEC_USINT::TValueType>(var_COMMAND);
    switch (command) {
      case scmCommandLoad: {
        if (var_MODEL_ID.empty()) {
          setError(scmErrorEmptyModelId);
          break;
        }

        const auto &modelId = var_MODEL_ID.getStorage();
        std::vector<std::uint8_t> modelBinary(var_MODEL_BLOB.getStorage().begin(), var_MODEL_BLOB.getStorage().end());
        if (EdgeMLRuntime::isMockModelId(modelId) && modelBinary.empty()) {
          modelBinary.emplace_back(0x00);
        } else if (!EdgeMLRuntime::isMockModelId(modelId) && modelBinary.empty()) {
          setError(scmErrorInvalidInput);
          break;
        }

        const auto declaredSize = static_cast<CIEC_UDINT::TValueType>(var_SIZE_BYTES);
        const auto metadataSize = 0U == declaredSize ? modelBinary.size() : static_cast<std::size_t>(declaredSize);
        const ModelMetadata metadata{modelId, var_VERSION.getStorage(), metadataSize, var_SHA256.getStorage()};

        const auto status = runtime.loadModel(metadata, modelBinary);
        if (EEdgeMLError::kOk == status || EEdgeMLError::kModelAlreadyExists == status) {
          clearError();
        } else {
          setError(mapBackendStatus(status));
        }
        break;
      }

      case scmCommandUnload: {
        if (var_MODEL_ID.empty()) {
          setError(scmErrorEmptyModelId);
          break;
        }

        const auto status = runtime.unloadModel(var_MODEL_ID.getStorage());
        if (EEdgeMLError::kOk == status) {
          clearError();
        } else {
          setError(mapBackendStatus(status));
        }
        break;
      }

      case scmCommandList: {
        const auto models = runtime.listModels();
        if (!models.empty()) {
          const auto &first = models.front();
          var_INFO_MODEL_ID = CIEC_STRING(first.id);
          var_INFO_VERSION = CIEC_STRING(first.version);
          var_INFO_SIZE_BYTES = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(first.sizeBytes));
          var_INFO_SHA256 = CIEC_STRING(first.sha256);
        }
        clearError();
        break;
      }

      case scmCommandInfo: {
        if (var_MODEL_ID.empty()) {
          setError(scmErrorEmptyModelId);
          break;
        }

        const auto info = runtime.findModel(var_MODEL_ID.getStorage());
        if (!info.has_value()) {
          setError(scmErrorModelNotFound);
          break;
        }

        var_INFO_MODEL_ID = CIEC_STRING(info->id);
        var_INFO_VERSION = CIEC_STRING(info->version);
        var_INFO_SIZE_BYTES = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(info->sizeBytes));
        var_INFO_SHA256 = CIEC_STRING(info->sha256);
        clearError();
        break;
      }

      default: {
        setError(scmErrorInvalidCommand);
        break;
      }
    }

    const auto modelCount = runtime.listModels().size();
    var_MODEL_COUNT = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(modelCount));

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_ModelManager::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_COMMAND, conn_COMMAND);
        readData(1, var_MODEL_ID, conn_MODEL_ID);
        readData(2, var_VERSION, conn_VERSION);
        readData(3, var_SIZE_BYTES, conn_SIZE_BYTES);
        readData(4, var_SHA256, conn_SHA256);
        readData(5, var_MODEL_BLOB, conn_MODEL_BLOB);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_ModelManager::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_SUCCESS, conn_SUCCESS);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_ERROR_CODE, conn_ERROR_CODE);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_MODEL_COUNT, conn_MODEL_COUNT);
        writeData(cFBInterfaceSpec.getNumDIs() + 4, var_INFO_MODEL_ID, conn_INFO_MODEL_ID);
        writeData(cFBInterfaceSpec.getNumDIs() + 5, var_INFO_VERSION, conn_INFO_VERSION);
        writeData(cFBInterfaceSpec.getNumDIs() + 6, var_INFO_SIZE_BYTES, conn_INFO_SIZE_BYTES);
        writeData(cFBInterfaceSpec.getNumDIs() + 7, var_INFO_SHA256, conn_INFO_SHA256);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_ModelManager::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_COMMAND;
      case 1: return &var_MODEL_ID;
      case 2: return &var_VERSION;
      case 3: return &var_SIZE_BYTES;
      case 4: return &var_SHA256;
      case 5: return &var_MODEL_BLOB;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_ModelManager::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_SUCCESS;
      case 1: return &var_ERROR;
      case 2: return &var_ERROR_CODE;
      case 3: return &var_MODEL_COUNT;
      case 4: return &var_INFO_MODEL_ID;
      case 5: return &var_INFO_VERSION;
      case 6: return &var_INFO_SIZE_BYTES;
      case 7: return &var_INFO_SHA256;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_ModelManager::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_ModelManager::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_COMMAND;
      case 1: return &conn_MODEL_ID;
      case 2: return &conn_VERSION;
      case 3: return &conn_SIZE_BYTES;
      case 4: return &conn_SHA256;
      case 5: return &conn_MODEL_BLOB;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_ModelManager::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_SUCCESS;
      case 1: return &conn_ERROR;
      case 2: return &conn_ERROR_CODE;
      case 3: return &conn_MODEL_COUNT;
      case 4: return &conn_INFO_MODEL_ID;
      case 5: return &conn_INFO_VERSION;
      case 6: return &conn_INFO_SIZE_BYTES;
      case 7: return &conn_INFO_SHA256;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
