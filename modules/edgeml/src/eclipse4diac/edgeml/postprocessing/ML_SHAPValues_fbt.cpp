/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/postprocessing/ML_SHAPValues_fbt.h"

#include <array>
#include <cmath>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames =
        std::array{"IN_0"_STRID,      "IN_1"_STRID,       "IN_2"_STRID,   "IN_3"_STRID,
                   "BASELINE_0"_STRID, "BASELINE_1"_STRID, "BASELINE_2"_STRID, "BASELINE_3"_STRID,
                   "WEIGHT_0"_STRID,   "WEIGHT_1"_STRID,    "WEIGHT_2"_STRID,   "WEIGHT_3"_STRID,
                   "NORMALIZE"_STRID};
    const auto cDataOutputNames = std::array{"SHAP_0"_STRID, "SHAP_1"_STRID, "SHAP_2"_STRID, "SHAP_3"_STRID,
                                              "TOTAL_CONTRIBUTION"_STRID, "VALID"_STRID, "ERROR"_STRID,
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

  DEFINE_FIRMWARE_FB(FORTE_ML_SHAPValues, "eclipse4diac::edgeml::ML_SHAPValues"_STRID)

  FORTE_ML_SHAPValues::FORTE_ML_SHAPValues(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_IN_0(nullptr),
      conn_IN_1(nullptr),
      conn_IN_2(nullptr),
      conn_IN_3(nullptr),
      conn_BASELINE_0(nullptr),
      conn_BASELINE_1(nullptr),
      conn_BASELINE_2(nullptr),
      conn_BASELINE_3(nullptr),
      conn_WEIGHT_0(nullptr),
      conn_WEIGHT_1(nullptr),
      conn_WEIGHT_2(nullptr),
      conn_WEIGHT_3(nullptr),
      conn_NORMALIZE(nullptr),
      conn_SHAP_0(*this, 0, var_SHAP_0),
      conn_SHAP_1(*this, 1, var_SHAP_1),
      conn_SHAP_2(*this, 2, var_SHAP_2),
      conn_SHAP_3(*this, 3, var_SHAP_3),
      conn_TOTAL_CONTRIBUTION(*this, 4, var_TOTAL_CONTRIBUTION),
      conn_VALID(*this, 5, var_VALID),
      conn_ERROR(*this, 6, var_ERROR),
      conn_ERROR_CODE(*this, 7, var_ERROR_CODE) {
    setInitialValues();
  }

  void FORTE_ML_SHAPValues::setInitialValues() {
    var_IN_0 = CIEC_REAL(0.0F);
    var_IN_1 = CIEC_REAL(0.0F);
    var_IN_2 = CIEC_REAL(0.0F);
    var_IN_3 = CIEC_REAL(0.0F);
    var_BASELINE_0 = CIEC_REAL(0.0F);
    var_BASELINE_1 = CIEC_REAL(0.0F);
    var_BASELINE_2 = CIEC_REAL(0.0F);
    var_BASELINE_3 = CIEC_REAL(0.0F);
    var_WEIGHT_0 = CIEC_REAL(1.0F);
    var_WEIGHT_1 = CIEC_REAL(1.0F);
    var_WEIGHT_2 = CIEC_REAL(1.0F);
    var_WEIGHT_3 = CIEC_REAL(1.0F);
    var_NORMALIZE = CIEC_BOOL(false);

    var_SHAP_0 = CIEC_REAL(0.0F);
    var_SHAP_1 = CIEC_REAL(0.0F);
    var_SHAP_2 = CIEC_REAL(0.0F);
    var_SHAP_3 = CIEC_REAL(0.0F);
    var_TOTAL_CONTRIBUTION = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_SHAPValues::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_SHAP_0 = CIEC_REAL(0.0F);
    var_SHAP_1 = CIEC_REAL(0.0F);
    var_SHAP_2 = CIEC_REAL(0.0F);
    var_SHAP_3 = CIEC_REAL(0.0F);
    var_TOTAL_CONTRIBUTION = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
  }

  void FORTE_ML_SHAPValues::clearError() {
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_SHAPValues::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    const std::array<CIEC_REAL, 12> numericInputs{
        var_IN_0,       var_IN_1,       var_IN_2,       var_IN_3,       var_BASELINE_0, var_BASELINE_1,
        var_BASELINE_2, var_BASELINE_3, var_WEIGHT_0,   var_WEIGHT_1,   var_WEIGHT_2,   var_WEIGHT_3,
    };
    for (const auto &value : numericInputs) {
      if (!isFinite(value)) {
        setError(scmErrorNonFiniteInput);
        sendOutputEvent(scmEventCNFID, paECET);
        return;
      }
    }

    std::array<CIEC_REAL::TValueType, 4> shap{0.0F, 0.0F, 0.0F, 0.0F};
    const std::array<CIEC_REAL::TValueType, 4> in{static_cast<CIEC_REAL::TValueType>(var_IN_0),
                                                   static_cast<CIEC_REAL::TValueType>(var_IN_1),
                                                   static_cast<CIEC_REAL::TValueType>(var_IN_2),
                                                   static_cast<CIEC_REAL::TValueType>(var_IN_3)};
    const std::array<CIEC_REAL::TValueType, 4> baseline{static_cast<CIEC_REAL::TValueType>(var_BASELINE_0),
                                                         static_cast<CIEC_REAL::TValueType>(var_BASELINE_1),
                                                         static_cast<CIEC_REAL::TValueType>(var_BASELINE_2),
                                                         static_cast<CIEC_REAL::TValueType>(var_BASELINE_3)};
    const std::array<CIEC_REAL::TValueType, 4> weight{static_cast<CIEC_REAL::TValueType>(var_WEIGHT_0),
                                                       static_cast<CIEC_REAL::TValueType>(var_WEIGHT_1),
                                                       static_cast<CIEC_REAL::TValueType>(var_WEIGHT_2),
                                                       static_cast<CIEC_REAL::TValueType>(var_WEIGHT_3)};

    for (std::size_t i = 0; i < 4; ++i) {
      shap[i] = (in[i] - baseline[i]) * weight[i];
    }

    if (static_cast<CIEC_BOOL::TValueType>(var_NORMALIZE)) {
      CIEC_REAL::TValueType sumAbs = 0.0F;
      for (const auto value : shap) {
        sumAbs += std::fabs(value);
      }
      if (sumAbs > 0.0F) {
        for (auto &value : shap) {
          value /= sumAbs;
        }
      }
    }

    CIEC_REAL::TValueType total = 0.0F;
    for (const auto value : shap) {
      total += value;
    }

    var_SHAP_0 = CIEC_REAL(shap[0]);
    var_SHAP_1 = CIEC_REAL(shap[1]);
    var_SHAP_2 = CIEC_REAL(shap[2]);
    var_SHAP_3 = CIEC_REAL(shap[3]);
    var_TOTAL_CONTRIBUTION = CIEC_REAL(total);
    clearError();

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_SHAPValues::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_IN_0, conn_IN_0);
        readData(1, var_IN_1, conn_IN_1);
        readData(2, var_IN_2, conn_IN_2);
        readData(3, var_IN_3, conn_IN_3);
        readData(4, var_BASELINE_0, conn_BASELINE_0);
        readData(5, var_BASELINE_1, conn_BASELINE_1);
        readData(6, var_BASELINE_2, conn_BASELINE_2);
        readData(7, var_BASELINE_3, conn_BASELINE_3);
        readData(8, var_WEIGHT_0, conn_WEIGHT_0);
        readData(9, var_WEIGHT_1, conn_WEIGHT_1);
        readData(10, var_WEIGHT_2, conn_WEIGHT_2);
        readData(11, var_WEIGHT_3, conn_WEIGHT_3);
        readData(12, var_NORMALIZE, conn_NORMALIZE);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_SHAPValues::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_SHAP_0, conn_SHAP_0);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_SHAP_1, conn_SHAP_1);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_SHAP_2, conn_SHAP_2);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_SHAP_3, conn_SHAP_3);
        writeData(cFBInterfaceSpec.getNumDIs() + 4, var_TOTAL_CONTRIBUTION, conn_TOTAL_CONTRIBUTION);
        writeData(cFBInterfaceSpec.getNumDIs() + 5, var_VALID, conn_VALID);
        writeData(cFBInterfaceSpec.getNumDIs() + 6, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 7, var_ERROR_CODE, conn_ERROR_CODE);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_SHAPValues::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_IN_0;
      case 1: return &var_IN_1;
      case 2: return &var_IN_2;
      case 3: return &var_IN_3;
      case 4: return &var_BASELINE_0;
      case 5: return &var_BASELINE_1;
      case 6: return &var_BASELINE_2;
      case 7: return &var_BASELINE_3;
      case 8: return &var_WEIGHT_0;
      case 9: return &var_WEIGHT_1;
      case 10: return &var_WEIGHT_2;
      case 11: return &var_WEIGHT_3;
      case 12: return &var_NORMALIZE;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_SHAPValues::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_SHAP_0;
      case 1: return &var_SHAP_1;
      case 2: return &var_SHAP_2;
      case 3: return &var_SHAP_3;
      case 4: return &var_TOTAL_CONTRIBUTION;
      case 5: return &var_VALID;
      case 6: return &var_ERROR;
      case 7: return &var_ERROR_CODE;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_SHAPValues::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_SHAPValues::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_IN_0;
      case 1: return &conn_IN_1;
      case 2: return &conn_IN_2;
      case 3: return &conn_IN_3;
      case 4: return &conn_BASELINE_0;
      case 5: return &conn_BASELINE_1;
      case 6: return &conn_BASELINE_2;
      case 7: return &conn_BASELINE_3;
      case 8: return &conn_WEIGHT_0;
      case 9: return &conn_WEIGHT_1;
      case 10: return &conn_WEIGHT_2;
      case 11: return &conn_WEIGHT_3;
      case 12: return &conn_NORMALIZE;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_SHAPValues::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_SHAP_0;
      case 1: return &conn_SHAP_1;
      case 2: return &conn_SHAP_2;
      case 3: return &conn_SHAP_3;
      case 4: return &conn_TOTAL_CONTRIBUTION;
      case 5: return &conn_VALID;
      case 6: return &conn_ERROR;
      case 7: return &conn_ERROR_CODE;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
