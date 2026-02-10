/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/preprocessing/ML_Scale_fbt.h"

#include <algorithm>
#include <array>
#include <cmath>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames = std::array{"IN"_STRID, "IN_MIN"_STRID, "IN_MAX"_STRID,
                                             "OUT_MIN"_STRID, "OUT_MAX"_STRID, "CLAMP"_STRID};
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

  DEFINE_FIRMWARE_FB(FORTE_ML_Scale, "eclipse4diac::edgeml::ML_Scale"_STRID)

  FORTE_ML_Scale::FORTE_ML_Scale(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_IN(nullptr),
      conn_IN_MIN(nullptr),
      conn_IN_MAX(nullptr),
      conn_OUT_MIN(nullptr),
      conn_OUT_MAX(nullptr),
      conn_CLAMP(nullptr),
      conn_OUT(*this, 0, var_OUT),
      conn_VALID(*this, 1, var_VALID),
      conn_ERROR(*this, 2, var_ERROR),
      conn_ERROR_CODE(*this, 3, var_ERROR_CODE) {
    setInitialValues();
  }

  void FORTE_ML_Scale::setInitialValues() {
    var_IN = CIEC_REAL(0.0F);
    var_IN_MIN = CIEC_REAL(0.0F);
    var_IN_MAX = CIEC_REAL(1.0F);
    var_OUT_MIN = CIEC_REAL(0.0F);
    var_OUT_MAX = CIEC_REAL(1.0F);
    var_CLAMP = CIEC_BOOL(true);

    var_OUT = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_Scale::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_OUT = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
  }

  void FORTE_ML_Scale::clearError() {
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_Scale::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    if (!isFinite(var_IN) || !isFinite(var_IN_MIN) || !isFinite(var_IN_MAX) || !isFinite(var_OUT_MIN) ||
        !isFinite(var_OUT_MAX)) {
      setError(scmErrorNonFiniteInput);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    const auto inMin = static_cast<CIEC_REAL::TValueType>(var_IN_MIN);
    const auto inMax = static_cast<CIEC_REAL::TValueType>(var_IN_MAX);
    if (!(inMin < inMax)) {
      setError(scmErrorInvalidInputRange);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    auto inValue = static_cast<CIEC_REAL::TValueType>(var_IN);
    if (static_cast<CIEC_BOOL::TValueType>(var_CLAMP)) {
      inValue = std::clamp(inValue, inMin, inMax);
    }

    const auto outMin = static_cast<CIEC_REAL::TValueType>(var_OUT_MIN);
    const auto outMax = static_cast<CIEC_REAL::TValueType>(var_OUT_MAX);

    const auto normalized = (inValue - inMin) / (inMax - inMin);
    const auto scaled = outMin + normalized * (outMax - outMin);

    var_OUT = CIEC_REAL(scaled);
    clearError();

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_Scale::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_IN, conn_IN);
        readData(1, var_IN_MIN, conn_IN_MIN);
        readData(2, var_IN_MAX, conn_IN_MAX);
        readData(3, var_OUT_MIN, conn_OUT_MIN);
        readData(4, var_OUT_MAX, conn_OUT_MAX);
        readData(5, var_CLAMP, conn_CLAMP);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_Scale::writeOutputData(const TEventID paEIID) {
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

  CIEC_ANY *FORTE_ML_Scale::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_IN;
      case 1: return &var_IN_MIN;
      case 2: return &var_IN_MAX;
      case 3: return &var_OUT_MIN;
      case 4: return &var_OUT_MAX;
      case 5: return &var_CLAMP;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_Scale::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_OUT;
      case 1: return &var_VALID;
      case 2: return &var_ERROR;
      case 3: return &var_ERROR_CODE;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_Scale::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_Scale::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_IN;
      case 1: return &conn_IN_MIN;
      case 2: return &conn_IN_MAX;
      case 3: return &conn_OUT_MIN;
      case 4: return &conn_OUT_MAX;
      case 5: return &conn_CLAMP;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_Scale::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_OUT;
      case 1: return &conn_VALID;
      case 2: return &conn_ERROR;
      case 3: return &conn_ERROR_CODE;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
