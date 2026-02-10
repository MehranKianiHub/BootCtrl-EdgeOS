/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/postprocessing/ML_ArgMax_fbt.h"

#include <array>
#include <cmath>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames = std::array{"IN_0"_STRID, "IN_1"_STRID, "IN_2"_STRID, "IN_3"_STRID};
    const auto cDataOutputNames = std::array{"INDEX"_STRID, "VALUE"_STRID, "VALID"_STRID, "ERROR"_STRID,
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

    bool isFinite(const CIEC_REAL &paValue) {
      return std::isfinite(static_cast<CIEC_REAL::TValueType>(paValue));
    }
  } // namespace

  DEFINE_FIRMWARE_FB(FORTE_ML_ArgMax, "eclipse4diac::edgeml::ML_ArgMax"_STRID)

  FORTE_ML_ArgMax::FORTE_ML_ArgMax(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_IN_0(nullptr),
      conn_IN_1(nullptr),
      conn_IN_2(nullptr),
      conn_IN_3(nullptr),
      conn_INDEX(*this, 0, var_INDEX),
      conn_VALUE(*this, 1, var_VALUE),
      conn_VALID(*this, 2, var_VALID),
      conn_ERROR(*this, 3, var_ERROR),
      conn_ERROR_CODE(*this, 4, var_ERROR_CODE) {
    setInitialValues();
  }

  void FORTE_ML_ArgMax::setInitialValues() {
    var_IN_0 = CIEC_REAL(0.0F);
    var_IN_1 = CIEC_REAL(0.0F);
    var_IN_2 = CIEC_REAL(0.0F);
    var_IN_3 = CIEC_REAL(0.0F);

    var_INDEX = CIEC_USINT(0U);
    var_VALUE = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_ArgMax::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_INDEX = CIEC_USINT(0U);
    var_VALUE = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
  }

  void FORTE_ML_ArgMax::clearError() {
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_ArgMax::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    if (!isFinite(var_IN_0) || !isFinite(var_IN_1) || !isFinite(var_IN_2) || !isFinite(var_IN_3)) {
      setError(scmErrorNonFiniteInput);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    const std::array<CIEC_REAL::TValueType, 4> values{static_cast<CIEC_REAL::TValueType>(var_IN_0),
                                                       static_cast<CIEC_REAL::TValueType>(var_IN_1),
                                                       static_cast<CIEC_REAL::TValueType>(var_IN_2),
                                                       static_cast<CIEC_REAL::TValueType>(var_IN_3)};

    std::uint8_t maxIndex = 0;
    auto maxValue = values[0];
    for (std::uint8_t i = 1; i < values.size(); ++i) {
      if (values[i] > maxValue) {
        maxValue = values[i];
        maxIndex = i;
      }
    }

    var_INDEX = CIEC_USINT(maxIndex);
    var_VALUE = CIEC_REAL(maxValue);
    clearError();

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_ArgMax::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_IN_0, conn_IN_0);
        readData(1, var_IN_1, conn_IN_1);
        readData(2, var_IN_2, conn_IN_2);
        readData(3, var_IN_3, conn_IN_3);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_ArgMax::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_INDEX, conn_INDEX);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_VALUE, conn_VALUE);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_VALID, conn_VALID);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 4, var_ERROR_CODE, conn_ERROR_CODE);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_ArgMax::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_IN_0;
      case 1: return &var_IN_1;
      case 2: return &var_IN_2;
      case 3: return &var_IN_3;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_ArgMax::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_INDEX;
      case 1: return &var_VALUE;
      case 2: return &var_VALID;
      case 3: return &var_ERROR;
      case 4: return &var_ERROR_CODE;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_ArgMax::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_ArgMax::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_IN_0;
      case 1: return &conn_IN_1;
      case 2: return &conn_IN_2;
      case 3: return &conn_IN_3;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_ArgMax::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_INDEX;
      case 1: return &conn_VALUE;
      case 2: return &conn_VALID;
      case 3: return &conn_ERROR;
      case 4: return &conn_ERROR_CODE;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
