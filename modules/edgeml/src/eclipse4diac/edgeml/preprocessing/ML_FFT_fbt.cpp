/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/preprocessing/ML_FFT_fbt.h"

#include <array>
#include <cmath>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames = std::array{"IN_0"_STRID, "IN_1"_STRID, "IN_2"_STRID, "IN_3"_STRID};
    const auto cDataOutputNames = std::array{"MAG_0"_STRID, "MAG_1"_STRID, "MAG_2"_STRID, "MAG_3"_STRID,
                                              "VALID"_STRID, "ERROR"_STRID, "ERROR_CODE"_STRID};
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

    constexpr double cPi = 3.14159265358979323846;
  } // namespace

  DEFINE_FIRMWARE_FB(FORTE_ML_FFT, "eclipse4diac::edgeml::ML_FFT"_STRID)

  FORTE_ML_FFT::FORTE_ML_FFT(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_IN_0(nullptr),
      conn_IN_1(nullptr),
      conn_IN_2(nullptr),
      conn_IN_3(nullptr),
      conn_MAG_0(*this, 0, var_MAG_0),
      conn_MAG_1(*this, 1, var_MAG_1),
      conn_MAG_2(*this, 2, var_MAG_2),
      conn_MAG_3(*this, 3, var_MAG_3),
      conn_VALID(*this, 4, var_VALID),
      conn_ERROR(*this, 5, var_ERROR),
      conn_ERROR_CODE(*this, 6, var_ERROR_CODE) {
    setInitialValues();
  }

  void FORTE_ML_FFT::setInitialValues() {
    var_IN_0 = CIEC_REAL(0.0F);
    var_IN_1 = CIEC_REAL(0.0F);
    var_IN_2 = CIEC_REAL(0.0F);
    var_IN_3 = CIEC_REAL(0.0F);

    var_MAG_0 = CIEC_REAL(0.0F);
    var_MAG_1 = CIEC_REAL(0.0F);
    var_MAG_2 = CIEC_REAL(0.0F);
    var_MAG_3 = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_FFT::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_MAG_0 = CIEC_REAL(0.0F);
    var_MAG_1 = CIEC_REAL(0.0F);
    var_MAG_2 = CIEC_REAL(0.0F);
    var_MAG_3 = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
  }

  void FORTE_ML_FFT::clearError() {
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_FFT::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    if (!isFinite(var_IN_0) || !isFinite(var_IN_1) || !isFinite(var_IN_2) || !isFinite(var_IN_3)) {
      setError(scmErrorNonFiniteInput);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    const std::array<double, scmVectorWidth> input{static_cast<CIEC_REAL::TValueType>(var_IN_0),
                                                    static_cast<CIEC_REAL::TValueType>(var_IN_1),
                                                    static_cast<CIEC_REAL::TValueType>(var_IN_2),
                                                    static_cast<CIEC_REAL::TValueType>(var_IN_3)};

    std::array<double, scmVectorWidth> magnitudes{};
    for (std::size_t k = 0; k < scmVectorWidth; ++k) {
      double real = 0.0;
      double imag = 0.0;
      for (std::size_t n = 0; n < scmVectorWidth; ++n) {
        const double angle = (2.0 * cPi * static_cast<double>(k) * static_cast<double>(n)) /
                             static_cast<double>(scmVectorWidth);
        real += input[n] * std::cos(angle);
        imag -= input[n] * std::sin(angle);
      }
      magnitudes[k] = std::sqrt(real * real + imag * imag);
    }

    var_MAG_0 = CIEC_REAL(static_cast<CIEC_REAL::TValueType>(magnitudes[0]));
    var_MAG_1 = CIEC_REAL(static_cast<CIEC_REAL::TValueType>(magnitudes[1]));
    var_MAG_2 = CIEC_REAL(static_cast<CIEC_REAL::TValueType>(magnitudes[2]));
    var_MAG_3 = CIEC_REAL(static_cast<CIEC_REAL::TValueType>(magnitudes[3]));
    clearError();

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_FFT::readInputData(const TEventID paEIID) {
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

  void FORTE_ML_FFT::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_MAG_0, conn_MAG_0);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_MAG_1, conn_MAG_1);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_MAG_2, conn_MAG_2);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_MAG_3, conn_MAG_3);
        writeData(cFBInterfaceSpec.getNumDIs() + 4, var_VALID, conn_VALID);
        writeData(cFBInterfaceSpec.getNumDIs() + 5, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 6, var_ERROR_CODE, conn_ERROR_CODE);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_FFT::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_IN_0;
      case 1: return &var_IN_1;
      case 2: return &var_IN_2;
      case 3: return &var_IN_3;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_FFT::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_MAG_0;
      case 1: return &var_MAG_1;
      case 2: return &var_MAG_2;
      case 3: return &var_MAG_3;
      case 4: return &var_VALID;
      case 5: return &var_ERROR;
      case 6: return &var_ERROR_CODE;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_FFT::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_FFT::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_IN_0;
      case 1: return &conn_IN_1;
      case 2: return &conn_IN_2;
      case 3: return &conn_IN_3;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_FFT::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_MAG_0;
      case 1: return &conn_MAG_1;
      case 2: return &conn_MAG_2;
      case 3: return &conn_MAG_3;
      case 4: return &conn_VALID;
      case 5: return &conn_ERROR;
      case 6: return &conn_ERROR_CODE;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
