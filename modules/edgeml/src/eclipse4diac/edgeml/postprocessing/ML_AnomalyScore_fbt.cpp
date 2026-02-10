/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/postprocessing/ML_AnomalyScore_fbt.h"

#include <array>
#include <cmath>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames = std::array{"IN"_STRID, "REFERENCE"_STRID, "THRESHOLD"_STRID, "SCALE"_STRID};
    const auto cDataOutputNames = std::array{"SCORE"_STRID, "IS_ANOMALY"_STRID, "VALID"_STRID, "ERROR"_STRID,
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

  DEFINE_FIRMWARE_FB(FORTE_ML_AnomalyScore, "eclipse4diac::edgeml::ML_AnomalyScore"_STRID)

  FORTE_ML_AnomalyScore::FORTE_ML_AnomalyScore(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_IN(nullptr),
      conn_REFERENCE(nullptr),
      conn_THRESHOLD(nullptr),
      conn_SCALE(nullptr),
      conn_SCORE(*this, 0, var_SCORE),
      conn_IS_ANOMALY(*this, 1, var_IS_ANOMALY),
      conn_VALID(*this, 2, var_VALID),
      conn_ERROR(*this, 3, var_ERROR),
      conn_ERROR_CODE(*this, 4, var_ERROR_CODE) {
    setInitialValues();
  }

  void FORTE_ML_AnomalyScore::setInitialValues() {
    var_IN = CIEC_REAL(0.0F);
    var_REFERENCE = CIEC_REAL(0.0F);
    var_THRESHOLD = CIEC_REAL(1.0F);
    var_SCALE = CIEC_REAL(1.0F);

    var_SCORE = CIEC_REAL(0.0F);
    var_IS_ANOMALY = CIEC_BOOL(false);
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_AnomalyScore::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_SCORE = CIEC_REAL(0.0F);
    var_IS_ANOMALY = CIEC_BOOL(false);
    var_VALID = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
  }

  void FORTE_ML_AnomalyScore::clearError() {
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_AnomalyScore::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    if (!isFinite(var_IN) || !isFinite(var_REFERENCE) || !isFinite(var_THRESHOLD) || !isFinite(var_SCALE)) {
      setError(scmErrorNonFiniteInput);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    const auto scale = static_cast<CIEC_REAL::TValueType>(var_SCALE);
    if (scale < 0.0F) {
      setError(scmErrorInvalidScale);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    const auto input = static_cast<CIEC_REAL::TValueType>(var_IN);
    const auto reference = static_cast<CIEC_REAL::TValueType>(var_REFERENCE);
    const auto threshold = static_cast<CIEC_REAL::TValueType>(var_THRESHOLD);
    const auto score = std::abs(input - reference) * scale;
    if (!std::isfinite(score)) {
      setError(scmErrorNonFiniteInput);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    var_SCORE = CIEC_REAL(score);
    var_IS_ANOMALY = CIEC_BOOL(score >= threshold);
    clearError();

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_AnomalyScore::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_IN, conn_IN);
        readData(1, var_REFERENCE, conn_REFERENCE);
        readData(2, var_THRESHOLD, conn_THRESHOLD);
        readData(3, var_SCALE, conn_SCALE);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_AnomalyScore::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_SCORE, conn_SCORE);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_IS_ANOMALY, conn_IS_ANOMALY);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_VALID, conn_VALID);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 4, var_ERROR_CODE, conn_ERROR_CODE);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_AnomalyScore::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_IN;
      case 1: return &var_REFERENCE;
      case 2: return &var_THRESHOLD;
      case 3: return &var_SCALE;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_AnomalyScore::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_SCORE;
      case 1: return &var_IS_ANOMALY;
      case 2: return &var_VALID;
      case 3: return &var_ERROR;
      case 4: return &var_ERROR_CODE;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_AnomalyScore::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_AnomalyScore::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_IN;
      case 1: return &conn_REFERENCE;
      case 2: return &conn_THRESHOLD;
      case 3: return &conn_SCALE;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_AnomalyScore::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_SCORE;
      case 1: return &conn_IS_ANOMALY;
      case 2: return &conn_VALID;
      case 3: return &conn_ERROR;
      case 4: return &conn_ERROR_CODE;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
