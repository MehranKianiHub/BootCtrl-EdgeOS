/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/preprocessing/ML_Normalize_fbt.h"

#include <algorithm>
#include <array>
#include <cmath>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames = std::array{"IN"_STRID, "METHOD"_STRID, "MIN"_STRID, "MAX"_STRID,
                                             "MEAN"_STRID, "STDDEV"_STRID, "CLAMP"_STRID};
    const auto cDataOutputNames = std::array{"OUT"_STRID, "VALID"_STRID, "ERROR"_STRID, "ERROR_CODE"_STRID};
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

  DEFINE_FIRMWARE_FB(FORTE_ML_Normalize, "eclipse4diac::edgeml::ML_Normalize"_STRID)

  FORTE_ML_Normalize::FORTE_ML_Normalize(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_IN(nullptr),
      conn_METHOD(nullptr),
      conn_MIN(nullptr),
      conn_MAX(nullptr),
      conn_MEAN(nullptr),
      conn_STDDEV(nullptr),
      conn_CLAMP(nullptr),
      conn_OUT(*this, 0, var_OUT),
      conn_VALID(*this, 1, var_VALID),
      conn_ERROR(*this, 2, var_ERROR),
      conn_ERROR_CODE(*this, 3, var_ERROR_CODE) {
  }

  void FORTE_ML_Normalize::setInitialValues() {
    var_IN = CIEC_REAL(0.0F);
    var_METHOD = CIEC_USINT(scmMethodMinMax);
    var_MIN = CIEC_REAL(0.0F);
    var_MAX = CIEC_REAL(1.0F);
    var_MEAN = CIEC_REAL(0.0F);
    var_STDDEV = CIEC_REAL(1.0F);
    var_CLAMP = CIEC_BOOL(true);

    var_OUT = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_Normalize::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_OUT = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
  }

  void FORTE_ML_Normalize::clearError() {
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_Normalize::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    if (!isFinite(var_IN)) {
      setError(scmErrorNonFiniteInput);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    const auto method = static_cast<CIEC_USINT::TValueType>(var_METHOD);
    const auto inValue = static_cast<CIEC_REAL::TValueType>(var_IN);

    if (scmMethodMinMax == method) {
      if (!isFinite(var_MIN) || !isFinite(var_MAX)) {
        setError(scmErrorNonFiniteInput);
        sendOutputEvent(scmEventCNFID, paECET);
        return;
      }

      const auto minValue = static_cast<CIEC_REAL::TValueType>(var_MIN);
      const auto maxValue = static_cast<CIEC_REAL::TValueType>(var_MAX);
      if (!(minValue < maxValue)) {
        setError(scmErrorInvalidMinMaxRange);
        sendOutputEvent(scmEventCNFID, paECET);
        return;
      }

      auto normalized = (inValue - minValue) / (maxValue - minValue);
      if (static_cast<CIEC_BOOL::TValueType>(var_CLAMP)) {
        normalized = std::clamp(normalized, 0.0F, 1.0F);
      }

      var_OUT = CIEC_REAL(normalized);
      clearError();
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    if (scmMethodZScore == method) {
      if (!isFinite(var_MEAN) || !isFinite(var_STDDEV)) {
        setError(scmErrorNonFiniteInput);
        sendOutputEvent(scmEventCNFID, paECET);
        return;
      }

      const auto meanValue = static_cast<CIEC_REAL::TValueType>(var_MEAN);
      const auto stdDevValue = static_cast<CIEC_REAL::TValueType>(var_STDDEV);
      if (!(stdDevValue > 0.0F)) {
        setError(scmErrorInvalidStdDev);
        sendOutputEvent(scmEventCNFID, paECET);
        return;
      }

      var_OUT = CIEC_REAL((inValue - meanValue) / stdDevValue);
      clearError();
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    setError(scmErrorUnknownMethod);
    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_Normalize::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_IN, conn_IN);
        readData(1, var_METHOD, conn_METHOD);
        readData(2, var_MIN, conn_MIN);
        readData(3, var_MAX, conn_MAX);
        readData(4, var_MEAN, conn_MEAN);
        readData(5, var_STDDEV, conn_STDDEV);
        readData(6, var_CLAMP, conn_CLAMP);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_Normalize::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_OUT, conn_OUT);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_VALID, conn_VALID);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_ERROR_CODE, conn_ERROR_CODE);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_Normalize::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_IN;
      case 1: return &var_METHOD;
      case 2: return &var_MIN;
      case 3: return &var_MAX;
      case 4: return &var_MEAN;
      case 5: return &var_STDDEV;
      case 6: return &var_CLAMP;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_Normalize::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_OUT;
      case 1: return &var_VALID;
      case 2: return &var_ERROR;
      case 3: return &var_ERROR_CODE;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_Normalize::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_Normalize::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_IN;
      case 1: return &conn_METHOD;
      case 2: return &conn_MIN;
      case 3: return &conn_MAX;
      case 4: return &conn_MEAN;
      case 5: return &conn_STDDEV;
      case 6: return &conn_CLAMP;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_Normalize::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_OUT;
      case 1: return &conn_VALID;
      case 2: return &conn_ERROR;
      case 3: return &conn_ERROR_CODE;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
