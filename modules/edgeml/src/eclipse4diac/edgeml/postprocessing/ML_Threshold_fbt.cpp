/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/postprocessing/ML_Threshold_fbt.h"

#include <array>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames = std::array{"IN"_STRID, "THRESHOLD"_STRID, "INCLUSIVE"_STRID};
    const auto cDataOutputNames = std::array{"EXCEEDS"_STRID, "MARGIN"_STRID};
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
  } // namespace

  DEFINE_FIRMWARE_FB(FORTE_ML_Threshold, "eclipse4diac::edgeml::ML_Threshold"_STRID)

  FORTE_ML_Threshold::FORTE_ML_Threshold(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_IN(nullptr),
      conn_THRESHOLD(nullptr),
      conn_INCLUSIVE(nullptr),
      conn_EXCEEDS(*this, 0, var_EXCEEDS),
      conn_MARGIN(*this, 1, var_MARGIN) {
  }

  void FORTE_ML_Threshold::setInitialValues() {
    var_IN = CIEC_REAL(0.0F);
    var_THRESHOLD = CIEC_REAL(0.5F);
    var_INCLUSIVE = CIEC_BOOL(true);
    var_EXCEEDS = CIEC_BOOL(false);
    var_MARGIN = CIEC_REAL(-0.5F);
  }

  void FORTE_ML_Threshold::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    const auto inputValue = static_cast<CIEC_REAL::TValueType>(var_IN);
    const auto thresholdValue = static_cast<CIEC_REAL::TValueType>(var_THRESHOLD);

    var_EXCEEDS = func_ML_Threshold(var_IN, var_THRESHOLD, var_INCLUSIVE);
    var_MARGIN = CIEC_REAL(inputValue - thresholdValue);

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_Threshold::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_IN, conn_IN);
        readData(1, var_THRESHOLD, conn_THRESHOLD);
        readData(2, var_INCLUSIVE, conn_INCLUSIVE);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_Threshold::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_EXCEEDS, conn_EXCEEDS);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_MARGIN, conn_MARGIN);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_Threshold::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_IN;
      case 1: return &var_THRESHOLD;
      case 2: return &var_INCLUSIVE;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_Threshold::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_EXCEEDS;
      case 1: return &var_MARGIN;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_Threshold::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_Threshold::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_IN;
      case 1: return &conn_THRESHOLD;
      case 2: return &conn_INCLUSIVE;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_Threshold::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_EXCEEDS;
      case 1: return &conn_MARGIN;
    }
    return nullptr;
  }

  CIEC_BOOL func_ML_Threshold(const CIEC_REAL &paIN, const CIEC_REAL &paThreshold, const CIEC_BOOL &paInclusive) {
    const auto inputValue = static_cast<CIEC_REAL::TValueType>(paIN);
    const auto thresholdValue = static_cast<CIEC_REAL::TValueType>(paThreshold);
    const auto inclusive = static_cast<CIEC_BOOL::TValueType>(paInclusive);
    return CIEC_BOOL(inclusive ? (inputValue >= thresholdValue) : (inputValue > thresholdValue));
  }

} // namespace forte::eclipse4diac::edgeml
