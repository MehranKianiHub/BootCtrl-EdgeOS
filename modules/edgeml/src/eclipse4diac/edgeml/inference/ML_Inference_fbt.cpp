/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/inference/ML_Inference_fbt.h"

#include "forte/eclipse4diac/edgeml/core/runtime_context.h"

#include <algorithm>
#include <array>
#include <cmath>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames = std::array{"MODEL_ID"_STRID, "IN_0"_STRID, "IN_1"_STRID, "IN_2"_STRID,
                                             "IN_3"_STRID, "OUT_SIZE"_STRID};
    const auto cDataOutputNames = std::array{"OUT_0"_STRID, "OUT_1"_STRID, "OUT_2"_STRID, "OUT_3"_STRID,
                                              "VALID"_STRID, "ERROR"_STRID, "ERROR_CODE"_STRID,
                                              "OUTPUT_COUNT"_STRID, "INFERENCE_US"_STRID};
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

    bool isFinite(const CIEC_REAL &paValue) {
      return std::isfinite(static_cast<CIEC_REAL::TValueType>(paValue));
    }
  } // namespace

  DEFINE_FIRMWARE_FB(FORTE_ML_Inference, "eclipse4diac::edgeml::ML_Inference"_STRID)

  FORTE_ML_Inference::FORTE_ML_Inference(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_MODEL_ID(nullptr),
      conn_IN_0(nullptr),
      conn_IN_1(nullptr),
      conn_IN_2(nullptr),
      conn_IN_3(nullptr),
      conn_OUT_SIZE(nullptr),
      conn_OUT_0(*this, 0, var_OUT_0),
      conn_OUT_1(*this, 1, var_OUT_1),
      conn_OUT_2(*this, 2, var_OUT_2),
      conn_OUT_3(*this, 3, var_OUT_3),
      conn_VALID(*this, 4, var_VALID),
      conn_ERROR(*this, 5, var_ERROR),
      conn_ERROR_CODE(*this, 6, var_ERROR_CODE),
      conn_OUTPUT_COUNT(*this, 7, var_OUTPUT_COUNT),
      conn_INFERENCE_US(*this, 8, var_INFERENCE_US) {
    setInitialValues();
  }

  void FORTE_ML_Inference::setInitialValues() {
    var_MODEL_ID = CIEC_STRING(std::string("mock.default"));
    var_IN_0 = CIEC_REAL(0.0F);
    var_IN_1 = CIEC_REAL(0.0F);
    var_IN_2 = CIEC_REAL(0.0F);
    var_IN_3 = CIEC_REAL(0.0F);
    var_OUT_SIZE = CIEC_USINT(scmVectorWidth);

    var_OUT_0 = CIEC_REAL(0.0F);
    var_OUT_1 = CIEC_REAL(0.0F);
    var_OUT_2 = CIEC_REAL(0.0F);
    var_OUT_3 = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
    var_OUTPUT_COUNT = CIEC_USINT(0U);
    var_INFERENCE_US = CIEC_UDINT(0U);
  }

  void FORTE_ML_Inference::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_OUT_0 = CIEC_REAL(0.0F);
    var_OUT_1 = CIEC_REAL(0.0F);
    var_OUT_2 = CIEC_REAL(0.0F);
    var_OUT_3 = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
    var_OUTPUT_COUNT = CIEC_USINT(0U);
    var_INFERENCE_US = CIEC_UDINT(0U);
  }

  void FORTE_ML_Inference::clearError() {
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_Inference::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    if (var_MODEL_ID.empty()) {
      setError(scmErrorEmptyModelId);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    if (scmVectorWidth != static_cast<CIEC_USINT::TValueType>(var_OUT_SIZE)) {
      setError(scmErrorInvalidOutputSize);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    if (!isFinite(var_IN_0) || !isFinite(var_IN_1) || !isFinite(var_IN_2) || !isFinite(var_IN_3)) {
      setError(scmErrorNonFiniteInput);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    auto &runtime = EdgeMLRuntime::instance();
    if (!runtime.backendAvailableForModel(var_MODEL_ID.getStorage())) {
      setError(scmErrorBackendFailure);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    std::array<float, scmVectorWidth> input{
        static_cast<CIEC_REAL::TValueType>(var_IN_0), static_cast<CIEC_REAL::TValueType>(var_IN_1),
        static_cast<CIEC_REAL::TValueType>(var_IN_2), static_cast<CIEC_REAL::TValueType>(var_IN_3)};
    std::array<float, scmVectorWidth> output{};
    InferenceStats stats{};

    auto &backend = runtime.backendForModel(var_MODEL_ID.getStorage());
    const auto status = backend.infer(var_MODEL_ID.getStorage(), input, output, stats);
    if (EEdgeMLError::kModelNotLoaded == status) {
      setError(scmErrorModelNotLoaded);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    if (EEdgeMLError::kOk != status) {
      setError(scmErrorBackendFailure);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    var_OUT_0 = CIEC_REAL(output[0]);
    var_OUT_1 = CIEC_REAL(output[1]);
    var_OUT_2 = CIEC_REAL(output[2]);
    var_OUT_3 = CIEC_REAL(output[3]);
    var_OUTPUT_COUNT = CIEC_USINT(static_cast<CIEC_USINT::TValueType>(
        std::min<std::uint32_t>(stats.outputElements, static_cast<std::uint32_t>(scmVectorWidth))));
    var_INFERENCE_US = CIEC_UDINT(stats.inferenceTimeUs);
    clearError();

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_Inference::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_MODEL_ID, conn_MODEL_ID);
        readData(1, var_IN_0, conn_IN_0);
        readData(2, var_IN_1, conn_IN_1);
        readData(3, var_IN_2, conn_IN_2);
        readData(4, var_IN_3, conn_IN_3);
        readData(5, var_OUT_SIZE, conn_OUT_SIZE);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_Inference::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_OUT_0, conn_OUT_0);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_OUT_1, conn_OUT_1);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_OUT_2, conn_OUT_2);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_OUT_3, conn_OUT_3);
        writeData(cFBInterfaceSpec.getNumDIs() + 4, var_VALID, conn_VALID);
        writeData(cFBInterfaceSpec.getNumDIs() + 5, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 6, var_ERROR_CODE, conn_ERROR_CODE);
        writeData(cFBInterfaceSpec.getNumDIs() + 7, var_OUTPUT_COUNT, conn_OUTPUT_COUNT);
        writeData(cFBInterfaceSpec.getNumDIs() + 8, var_INFERENCE_US, conn_INFERENCE_US);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_Inference::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_MODEL_ID;
      case 1: return &var_IN_0;
      case 2: return &var_IN_1;
      case 3: return &var_IN_2;
      case 4: return &var_IN_3;
      case 5: return &var_OUT_SIZE;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_Inference::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_OUT_0;
      case 1: return &var_OUT_1;
      case 2: return &var_OUT_2;
      case 3: return &var_OUT_3;
      case 4: return &var_VALID;
      case 5: return &var_ERROR;
      case 6: return &var_ERROR_CODE;
      case 7: return &var_OUTPUT_COUNT;
      case 8: return &var_INFERENCE_US;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_Inference::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_Inference::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_MODEL_ID;
      case 1: return &conn_IN_0;
      case 2: return &conn_IN_1;
      case 3: return &conn_IN_2;
      case 4: return &conn_IN_3;
      case 5: return &conn_OUT_SIZE;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_Inference::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_OUT_0;
      case 1: return &conn_OUT_1;
      case 2: return &conn_OUT_2;
      case 3: return &conn_OUT_3;
      case 4: return &conn_VALID;
      case 5: return &conn_ERROR;
      case 6: return &conn_ERROR_CODE;
      case 7: return &conn_OUTPUT_COUNT;
      case 8: return &conn_INFERENCE_US;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
