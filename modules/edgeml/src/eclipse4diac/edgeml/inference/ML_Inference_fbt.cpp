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
#include <cstdint>
#include <variant>
#include <vector>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames = std::array{"MODEL_ID"_STRID, "IN_VALUES"_STRID, "OUT_CAPACITY"_STRID};
    const auto cDataOutputNames = std::array{"OUT_VALUES"_STRID, "VALID"_STRID, "ERROR"_STRID, "ERROR_CODE"_STRID,
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

    CIEC_ARRAY_VARIABLE<CIEC_REAL> makeEmptyRealArray() {
      CIEC_ARRAY_VARIABLE<CIEC_REAL> array;
      array.setBounds(0, -1);
      return array;
    }
  } // namespace

  DEFINE_FIRMWARE_FB(FORTE_ML_Inference, "eclipse4diac::edgeml::ML_Inference"_STRID)

  FORTE_ML_Inference::FORTE_ML_Inference(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_MODEL_ID(nullptr),
      conn_IN_VALUES(nullptr),
      conn_OUT_CAPACITY(nullptr),
      conn_OUT_VALUES(*this, 0, var_OUT_VALUES),
      conn_VALID(*this, 1, var_VALID),
      conn_ERROR(*this, 2, var_ERROR),
      conn_ERROR_CODE(*this, 3, var_ERROR_CODE),
      conn_OUTPUT_COUNT(*this, 4, var_OUTPUT_COUNT),
      conn_INFERENCE_US(*this, 5, var_INFERENCE_US) {
    setInitialValues();
  }

  void FORTE_ML_Inference::setInitialValues() {
    var_MODEL_ID = CIEC_STRING(std::string("mock.default"));
    var_IN_VALUES.setValue(makeEmptyRealArray());
    var_OUT_CAPACITY = CIEC_UDINT(4U);

    clearOutputVector();
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
    var_OUTPUT_COUNT = CIEC_UDINT(0U);
    var_INFERENCE_US = CIEC_UDINT(0U);
  }

  void FORTE_ML_Inference::clearOutputVector() {
    var_OUT_VALUES.setValue(makeEmptyRealArray());
  }

  void FORTE_ML_Inference::setError(const CIEC_USINT::TValueType paErrorCode) {
    clearOutputVector();
    var_VALID = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
    var_OUTPUT_COUNT = CIEC_UDINT(0U);
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

    if (!std::holds_alternative<CIEC_ANY_UNIQUE_PTR<CIEC_ARRAY>>(var_IN_VALUES)) {
      setError(scmErrorInvalidInputVector);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    const auto &inputArray = *std::get<CIEC_ANY_UNIQUE_PTR<CIEC_ARRAY>>(var_IN_VALUES);
    if (CIEC_ANY::e_REAL != inputArray.getElementDataTypeID()) {
      setError(scmErrorInvalidInputVector);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    const auto inputSize = inputArray.size();
    if (0U == inputSize) {
      setError(scmErrorInvalidInputVector);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    const auto outputCapacity = static_cast<CIEC_UDINT::TValueType>(var_OUT_CAPACITY);
    if (0U == outputCapacity) {
      setError(scmErrorInvalidOutputCapacity);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    std::vector<float> input;
    input.reserve(inputSize);
    for (intmax_t index = inputArray.getLowerBound(), end = inputArray.getUpperBound(); index <= end; ++index) {
      const auto &value = static_cast<const CIEC_REAL &>(inputArray[index]);
      if (!isFinite(value)) {
        setError(scmErrorNonFiniteInput);
        sendOutputEvent(scmEventCNFID, paECET);
        return;
      }
      input.push_back(static_cast<CIEC_REAL::TValueType>(value));
    }

    std::vector<float> output(outputCapacity, 0.0F);
    InferenceStats stats{};

    auto &runtime = EdgeMLRuntime::instance();
    const auto status = runtime.inferModel(var_MODEL_ID.getStorage(), input, output, stats);
    if (EEdgeMLError::kModelNotLoaded == status) {
      setError(scmErrorModelNotLoaded);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    if (EEdgeMLError::kOutputTooSmall == status) {
      setError(scmErrorOutputTooSmall);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    if (EEdgeMLError::kOk != status) {
      setError(scmErrorBackendFailure);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    const auto outputCount = static_cast<CIEC_UDINT::TValueType>(
        std::min<std::uint32_t>(stats.outputElements, static_cast<std::uint32_t>(output.size())));
    CIEC_ARRAY_VARIABLE<CIEC_REAL> outArray = makeEmptyRealArray();
    if (0U < outputCount) {
      outArray.setBounds(0, static_cast<intmax_t>(outputCount) - 1);
      for (CIEC_UDINT::TValueType i = 0U; i < outputCount; ++i) {
        outArray[static_cast<intmax_t>(i)] = CIEC_REAL(output[i]);
      }
    }
    var_OUT_VALUES.setValue(outArray);

    var_OUTPUT_COUNT = CIEC_UDINT(outputCount);
    var_INFERENCE_US = CIEC_UDINT(stats.inferenceTimeUs);
    clearError();

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_Inference::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_MODEL_ID, conn_MODEL_ID);
        readData(1, var_IN_VALUES, conn_IN_VALUES);
        readData(2, var_OUT_CAPACITY, conn_OUT_CAPACITY);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_Inference::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_OUT_VALUES, conn_OUT_VALUES);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_VALID, conn_VALID);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_ERROR_CODE, conn_ERROR_CODE);
        writeData(cFBInterfaceSpec.getNumDIs() + 4, var_OUTPUT_COUNT, conn_OUTPUT_COUNT);
        writeData(cFBInterfaceSpec.getNumDIs() + 5, var_INFERENCE_US, conn_INFERENCE_US);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_Inference::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_MODEL_ID;
      case 1: return &var_IN_VALUES;
      case 2: return &var_OUT_CAPACITY;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_Inference::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_OUT_VALUES;
      case 1: return &var_VALID;
      case 2: return &var_ERROR;
      case 3: return &var_ERROR_CODE;
      case 4: return &var_OUTPUT_COUNT;
      case 5: return &var_INFERENCE_US;
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
      case 1: return &conn_IN_VALUES;
      case 2: return &conn_OUT_CAPACITY;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_Inference::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_OUT_VALUES;
      case 1: return &conn_VALID;
      case 2: return &conn_ERROR;
      case 3: return &conn_ERROR_CODE;
      case 4: return &conn_OUTPUT_COUNT;
      case 5: return &conn_INFERENCE_US;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
