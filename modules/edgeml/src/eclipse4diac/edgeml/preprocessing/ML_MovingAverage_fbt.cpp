/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/preprocessing/ML_MovingAverage_fbt.h"

#include <array>
#include <cmath>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames =
        std::array{"IN_0"_STRID, "IN_1"_STRID, "IN_2"_STRID, "IN_3"_STRID, "WINDOW"_STRID, "RESET"_STRID};
    const auto cDataOutputNames =
        std::array{"OUT_0"_STRID, "OUT_1"_STRID, "OUT_2"_STRID, "OUT_3"_STRID, "SAMPLE_COUNT"_STRID,
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
  } // namespace

  DEFINE_FIRMWARE_FB(FORTE_ML_MovingAverage, "eclipse4diac::edgeml::ML_MovingAverage"_STRID)

  FORTE_ML_MovingAverage::FORTE_ML_MovingAverage(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_IN_0(nullptr),
      conn_IN_1(nullptr),
      conn_IN_2(nullptr),
      conn_IN_3(nullptr),
      conn_WINDOW(nullptr),
      conn_RESET(nullptr),
      conn_OUT_0(*this, 0, var_OUT_0),
      conn_OUT_1(*this, 1, var_OUT_1),
      conn_OUT_2(*this, 2, var_OUT_2),
      conn_OUT_3(*this, 3, var_OUT_3),
      conn_SAMPLE_COUNT(*this, 4, var_SAMPLE_COUNT),
      conn_VALID(*this, 5, var_VALID),
      conn_ERROR(*this, 6, var_ERROR),
      conn_ERROR_CODE(*this, 7, var_ERROR_CODE),
      mHistory{},
      mWriteIndex(0U),
      mStoredSamples(0U) {
    setInitialValues();
  }

  void FORTE_ML_MovingAverage::setInitialValues() {
    var_IN_0 = CIEC_REAL(0.0F);
    var_IN_1 = CIEC_REAL(0.0F);
    var_IN_2 = CIEC_REAL(0.0F);
    var_IN_3 = CIEC_REAL(0.0F);
    var_WINDOW = CIEC_USINT(3U);
    var_RESET = CIEC_BOOL(false);

    var_OUT_0 = CIEC_REAL(0.0F);
    var_OUT_1 = CIEC_REAL(0.0F);
    var_OUT_2 = CIEC_REAL(0.0F);
    var_OUT_3 = CIEC_REAL(0.0F);
    var_SAMPLE_COUNT = CIEC_USINT(0U);
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);

    clearHistory();
  }

  void FORTE_ML_MovingAverage::clearHistory() {
    for (auto &sample : mHistory) {
      sample = {0.0F, 0.0F, 0.0F, 0.0F};
    }
    mWriteIndex = 0U;
    mStoredSamples = 0U;
  }

  void FORTE_ML_MovingAverage::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_OUT_0 = CIEC_REAL(0.0F);
    var_OUT_1 = CIEC_REAL(0.0F);
    var_OUT_2 = CIEC_REAL(0.0F);
    var_OUT_3 = CIEC_REAL(0.0F);
    var_SAMPLE_COUNT = CIEC_USINT(0U);
    var_VALID = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
  }

  void FORTE_ML_MovingAverage::clearError() {
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_MovingAverage::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    const auto window = static_cast<CIEC_USINT::TValueType>(var_WINDOW);
    if (0U == window || window > scmMaxWindow) {
      setError(scmErrorInvalidWindow);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    if (!isFinite(var_IN_0) || !isFinite(var_IN_1) || !isFinite(var_IN_2) || !isFinite(var_IN_3)) {
      setError(scmErrorNonFiniteInput);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    if (static_cast<CIEC_BOOL::TValueType>(var_RESET)) {
      clearHistory();
    }

    mHistory[mWriteIndex] = {static_cast<CIEC_REAL::TValueType>(var_IN_0), static_cast<CIEC_REAL::TValueType>(var_IN_1),
                             static_cast<CIEC_REAL::TValueType>(var_IN_2), static_cast<CIEC_REAL::TValueType>(var_IN_3)};
    mWriteIndex = static_cast<CIEC_USINT::TValueType>((mWriteIndex + 1U) % scmMaxWindow);
    if (mStoredSamples < scmMaxWindow) {
      ++mStoredSamples;
    }

    const auto activeSamples = static_cast<CIEC_USINT::TValueType>(mStoredSamples < window ? mStoredSamples : window);
    std::array<CIEC_REAL::TValueType, scmVectorWidth> sums{0.0F, 0.0F, 0.0F, 0.0F};
    for (CIEC_USINT::TValueType s = 0U; s < activeSamples; ++s) {
      const auto idx = static_cast<CIEC_USINT::TValueType>((mWriteIndex + scmMaxWindow - 1U - s) % scmMaxWindow);
      for (std::size_t i = 0; i < scmVectorWidth; ++i) {
        sums[i] += mHistory[idx][i];
      }
    }

    const auto divisor = static_cast<CIEC_REAL::TValueType>(activeSamples);
    var_OUT_0 = CIEC_REAL(sums[0] / divisor);
    var_OUT_1 = CIEC_REAL(sums[1] / divisor);
    var_OUT_2 = CIEC_REAL(sums[2] / divisor);
    var_OUT_3 = CIEC_REAL(sums[3] / divisor);
    var_SAMPLE_COUNT = CIEC_USINT(activeSamples);
    clearError();

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_MovingAverage::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_IN_0, conn_IN_0);
        readData(1, var_IN_1, conn_IN_1);
        readData(2, var_IN_2, conn_IN_2);
        readData(3, var_IN_3, conn_IN_3);
        readData(4, var_WINDOW, conn_WINDOW);
        readData(5, var_RESET, conn_RESET);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_MovingAverage::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_OUT_0, conn_OUT_0);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_OUT_1, conn_OUT_1);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_OUT_2, conn_OUT_2);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_OUT_3, conn_OUT_3);
        writeData(cFBInterfaceSpec.getNumDIs() + 4, var_SAMPLE_COUNT, conn_SAMPLE_COUNT);
        writeData(cFBInterfaceSpec.getNumDIs() + 5, var_VALID, conn_VALID);
        writeData(cFBInterfaceSpec.getNumDIs() + 6, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 7, var_ERROR_CODE, conn_ERROR_CODE);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_MovingAverage::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_IN_0;
      case 1: return &var_IN_1;
      case 2: return &var_IN_2;
      case 3: return &var_IN_3;
      case 4: return &var_WINDOW;
      case 5: return &var_RESET;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_MovingAverage::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_OUT_0;
      case 1: return &var_OUT_1;
      case 2: return &var_OUT_2;
      case 3: return &var_OUT_3;
      case 4: return &var_SAMPLE_COUNT;
      case 5: return &var_VALID;
      case 6: return &var_ERROR;
      case 7: return &var_ERROR_CODE;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_MovingAverage::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_MovingAverage::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_IN_0;
      case 1: return &conn_IN_1;
      case 2: return &conn_IN_2;
      case 3: return &conn_IN_3;
      case 4: return &conn_WINDOW;
      case 5: return &conn_RESET;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_MovingAverage::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_OUT_0;
      case 1: return &conn_OUT_1;
      case 2: return &conn_OUT_2;
      case 3: return &conn_OUT_3;
      case 4: return &conn_SAMPLE_COUNT;
      case 5: return &conn_VALID;
      case 6: return &conn_ERROR;
      case 7: return &conn_ERROR_CODE;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
