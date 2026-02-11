/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/management/ML_Monitoring_fbt.h"

#include "forte/eclipse4diac/edgeml/core/monitoring_pipeline.h"

#include <algorithm>
#include <array>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames = std::array{"COMMAND"_STRID,
                                             "INFERENCE_US"_STRID,
                                             "HAS_ERROR"_STRID,
                                             "IN_ERROR_CODE"_STRID,
                                             "PERSIST_PATH"_STRID,
                                             "APPEND"_STRID};
    const auto cDataOutputNames = std::array{"TOTAL_INFERENCES"_STRID,
                                              "TOTAL_ERRORS"_STRID,
                                              "AVG_INFERENCE_US"_STRID,
                                              "MAX_INFERENCE_US"_STRID,
                                              "LAST_ERROR_CODE"_STRID,
                                              "HEALTH_SCORE"_STRID,
                                              "SUCCESS"_STRID,
                                              "ERROR"_STRID,
                                              "ERROR_CODE"_STRID,
                                              "PERSISTED"_STRID,
                                              "PERSIST_BYTES"_STRID};
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

    CIEC_USINT::TValueType mapPersistenceStatus(const EMonitoringPersistenceStatus paStatus) {
      switch (paStatus) {
        case EMonitoringPersistenceStatus::kInvalidPath: return FORTE_ML_Monitoring::scmErrorInvalidPath;
        case EMonitoringPersistenceStatus::kIoError: return FORTE_ML_Monitoring::scmErrorPersistenceIo;
        case EMonitoringPersistenceStatus::kParseError: return FORTE_ML_Monitoring::scmErrorPersistenceParse;
        case EMonitoringPersistenceStatus::kOk:
        default: return FORTE_ML_Monitoring::scmErrorOk;
      }
    }
  } // namespace

  DEFINE_FIRMWARE_FB(FORTE_ML_Monitoring, "eclipse4diac::edgeml::ML_Monitoring"_STRID)

  FORTE_ML_Monitoring::FORTE_ML_Monitoring(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_COMMAND(nullptr),
      conn_INFERENCE_US(nullptr),
      conn_HAS_ERROR(nullptr),
      conn_IN_ERROR_CODE(nullptr),
      conn_PERSIST_PATH(nullptr),
      conn_APPEND(nullptr),
      conn_TOTAL_INFERENCES(*this, 0, var_TOTAL_INFERENCES),
      conn_TOTAL_ERRORS(*this, 1, var_TOTAL_ERRORS),
      conn_AVG_INFERENCE_US(*this, 2, var_AVG_INFERENCE_US),
      conn_MAX_INFERENCE_US(*this, 3, var_MAX_INFERENCE_US),
      conn_LAST_ERROR_CODE(*this, 4, var_LAST_ERROR_CODE),
      conn_HEALTH_SCORE(*this, 5, var_HEALTH_SCORE),
      conn_SUCCESS(*this, 6, var_SUCCESS),
      conn_ERROR(*this, 7, var_ERROR),
      conn_ERROR_CODE(*this, 8, var_ERROR_CODE),
      conn_PERSISTED(*this, 9, var_PERSISTED),
      conn_PERSIST_BYTES(*this, 10, var_PERSIST_BYTES),
      mTotalInferenceTimeUs(0U),
      mTotalInferences(0U),
      mTotalErrors(0U),
      mMaxInferenceUs(0U),
      mLastErrorCode(0U) {
    setInitialValues();
  }

  void FORTE_ML_Monitoring::setInitialValues() {
    var_COMMAND = CIEC_USINT(scmCommandSnapshot);
    var_INFERENCE_US = CIEC_UDINT(0U);
    var_HAS_ERROR = CIEC_BOOL(false);
    var_IN_ERROR_CODE = CIEC_USINT(0U);
    var_PERSIST_PATH = CIEC_STRING(std::string(""));
    var_APPEND = CIEC_BOOL(true);

    var_TOTAL_INFERENCES = CIEC_UDINT(0U);
    var_TOTAL_ERRORS = CIEC_UDINT(0U);
    var_AVG_INFERENCE_US = CIEC_UDINT(0U);
    var_MAX_INFERENCE_US = CIEC_UDINT(0U);
    var_LAST_ERROR_CODE = CIEC_USINT(0U);
    var_HEALTH_SCORE = CIEC_REAL(1.0F);
    var_SUCCESS = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
    clearPersistenceOutputs();

    clearMetrics();
  }

  void FORTE_ML_Monitoring::clearMetrics() {
    mTotalInferenceTimeUs = 0U;
    mTotalInferences = 0U;
    mTotalErrors = 0U;
    mMaxInferenceUs = 0U;
    mLastErrorCode = 0U;
  }

  void FORTE_ML_Monitoring::clearPersistenceOutputs() {
    var_PERSISTED = CIEC_BOOL(false);
    var_PERSIST_BYTES = CIEC_UDINT(0U);
  }

  void FORTE_ML_Monitoring::publishMetrics() {
    var_TOTAL_INFERENCES = CIEC_UDINT(mTotalInferences);
    var_TOTAL_ERRORS = CIEC_UDINT(mTotalErrors);
    var_MAX_INFERENCE_US = CIEC_UDINT(mMaxInferenceUs);
    var_LAST_ERROR_CODE = CIEC_USINT(mLastErrorCode);

    if (0U == mTotalInferences) {
      var_AVG_INFERENCE_US = CIEC_UDINT(0U);
      var_HEALTH_SCORE = CIEC_REAL(1.0F);
      return;
    }

    const auto avg = static_cast<CIEC_UDINT::TValueType>(mTotalInferenceTimeUs / mTotalInferences);
    var_AVG_INFERENCE_US = CIEC_UDINT(avg);

    const auto ratio = static_cast<float>(mTotalErrors) / static_cast<float>(mTotalInferences);
    const auto health = std::clamp(1.0F - ratio, 0.0F, 1.0F);
    var_HEALTH_SCORE = CIEC_REAL(health);
  }

  void FORTE_ML_Monitoring::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_SUCCESS = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
  }

  void FORTE_ML_Monitoring::clearError() {
    var_SUCCESS = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_Monitoring::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    clearPersistenceOutputs();

    const auto command = static_cast<CIEC_USINT::TValueType>(var_COMMAND);
    switch (command) {
      case scmCommandReport: {
        ++mTotalInferences;

        const auto inferenceUs = static_cast<CIEC_UDINT::TValueType>(var_INFERENCE_US);
        mTotalInferenceTimeUs += inferenceUs;
        if (inferenceUs > mMaxInferenceUs) {
          mMaxInferenceUs = inferenceUs;
        }

        if (static_cast<CIEC_BOOL::TValueType>(var_HAS_ERROR)) {
          ++mTotalErrors;
          mLastErrorCode = static_cast<CIEC_USINT::TValueType>(var_IN_ERROR_CODE);
        }

        clearError();
        break;
      }

      case scmCommandSnapshot: {
        clearError();
        break;
      }

      case scmCommandReset: {
        clearMetrics();
        clearError();
        break;
      }

      case scmCommandExport: {
        if (var_PERSIST_PATH.empty()) {
          setError(scmErrorInvalidPath);
          break;
        }

        const MonitoringPersistentMetrics metrics{mTotalInferenceTimeUs,
                                                  static_cast<std::uint32_t>(mTotalInferences),
                                                  static_cast<std::uint32_t>(mTotalErrors),
                                                  static_cast<std::uint32_t>(mMaxInferenceUs),
                                                  static_cast<std::uint8_t>(mLastErrorCode)};

        std::size_t writtenBytes = 0U;
        const auto status = MonitoringPipeline::exportCsv(metrics, var_PERSIST_PATH.getStorage(),
                                                          static_cast<CIEC_BOOL::TValueType>(var_APPEND), writtenBytes);
        if (EMonitoringPersistenceStatus::kOk != status) {
          setError(mapPersistenceStatus(status));
          break;
        }

        var_PERSISTED = CIEC_BOOL(true);
        var_PERSIST_BYTES = CIEC_UDINT(static_cast<CIEC_UDINT::TValueType>(writtenBytes));
        clearError();
        break;
      }

      case scmCommandImport: {
        if (var_PERSIST_PATH.empty()) {
          setError(scmErrorInvalidPath);
          break;
        }

        MonitoringPersistentMetrics metrics{};
        const auto status = MonitoringPipeline::importLatestCsv(var_PERSIST_PATH.getStorage(), metrics);
        if (EMonitoringPersistenceStatus::kOk != status) {
          setError(mapPersistenceStatus(status));
          break;
        }

        mTotalInferenceTimeUs = metrics.totalInferenceTimeUs;
        mTotalInferences = static_cast<CIEC_UDINT::TValueType>(metrics.totalInferences);
        mTotalErrors = static_cast<CIEC_UDINT::TValueType>(metrics.totalErrors);
        mMaxInferenceUs = static_cast<CIEC_UDINT::TValueType>(metrics.maxInferenceUs);
        mLastErrorCode = static_cast<CIEC_USINT::TValueType>(metrics.lastErrorCode);

        var_PERSISTED = CIEC_BOOL(true);
        clearError();
        break;
      }

      default: {
        setError(scmErrorInvalidCommand);
        break;
      }
    }

    publishMetrics();
    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_Monitoring::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_COMMAND, conn_COMMAND);
        readData(1, var_INFERENCE_US, conn_INFERENCE_US);
        readData(2, var_HAS_ERROR, conn_HAS_ERROR);
        readData(3, var_IN_ERROR_CODE, conn_IN_ERROR_CODE);
        readData(4, var_PERSIST_PATH, conn_PERSIST_PATH);
        readData(5, var_APPEND, conn_APPEND);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_Monitoring::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_TOTAL_INFERENCES, conn_TOTAL_INFERENCES);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_TOTAL_ERRORS, conn_TOTAL_ERRORS);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_AVG_INFERENCE_US, conn_AVG_INFERENCE_US);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_MAX_INFERENCE_US, conn_MAX_INFERENCE_US);
        writeData(cFBInterfaceSpec.getNumDIs() + 4, var_LAST_ERROR_CODE, conn_LAST_ERROR_CODE);
        writeData(cFBInterfaceSpec.getNumDIs() + 5, var_HEALTH_SCORE, conn_HEALTH_SCORE);
        writeData(cFBInterfaceSpec.getNumDIs() + 6, var_SUCCESS, conn_SUCCESS);
        writeData(cFBInterfaceSpec.getNumDIs() + 7, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 8, var_ERROR_CODE, conn_ERROR_CODE);
        writeData(cFBInterfaceSpec.getNumDIs() + 9, var_PERSISTED, conn_PERSISTED);
        writeData(cFBInterfaceSpec.getNumDIs() + 10, var_PERSIST_BYTES, conn_PERSIST_BYTES);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_Monitoring::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_COMMAND;
      case 1: return &var_INFERENCE_US;
      case 2: return &var_HAS_ERROR;
      case 3: return &var_IN_ERROR_CODE;
      case 4: return &var_PERSIST_PATH;
      case 5: return &var_APPEND;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_Monitoring::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_TOTAL_INFERENCES;
      case 1: return &var_TOTAL_ERRORS;
      case 2: return &var_AVG_INFERENCE_US;
      case 3: return &var_MAX_INFERENCE_US;
      case 4: return &var_LAST_ERROR_CODE;
      case 5: return &var_HEALTH_SCORE;
      case 6: return &var_SUCCESS;
      case 7: return &var_ERROR;
      case 8: return &var_ERROR_CODE;
      case 9: return &var_PERSISTED;
      case 10: return &var_PERSIST_BYTES;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_Monitoring::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_Monitoring::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_COMMAND;
      case 1: return &conn_INFERENCE_US;
      case 2: return &conn_HAS_ERROR;
      case 3: return &conn_IN_ERROR_CODE;
      case 4: return &conn_PERSIST_PATH;
      case 5: return &conn_APPEND;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_Monitoring::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_TOTAL_INFERENCES;
      case 1: return &conn_TOTAL_ERRORS;
      case 2: return &conn_AVG_INFERENCE_US;
      case 3: return &conn_MAX_INFERENCE_US;
      case 4: return &conn_LAST_ERROR_CODE;
      case 5: return &conn_HEALTH_SCORE;
      case 6: return &conn_SUCCESS;
      case 7: return &conn_ERROR;
      case 8: return &conn_ERROR_CODE;
      case 9: return &conn_PERSISTED;
      case 10: return &conn_PERSIST_BYTES;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
