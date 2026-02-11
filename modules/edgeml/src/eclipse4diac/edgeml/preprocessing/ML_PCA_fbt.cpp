/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/preprocessing/ML_PCA_fbt.h"

#include <array>
#include <cmath>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml {
  namespace {
    const auto cDataInputNames = std::array{
        "IN_0"_STRID, "IN_1"_STRID, "IN_2"_STRID, "IN_3"_STRID, "MEAN_0"_STRID,  "MEAN_1"_STRID,
        "MEAN_2"_STRID, "MEAN_3"_STRID, "COMP1_0"_STRID, "COMP1_1"_STRID, "COMP1_2"_STRID, "COMP1_3"_STRID,
        "COMP2_0"_STRID, "COMP2_1"_STRID, "COMP2_2"_STRID, "COMP2_3"_STRID};
    const auto cDataOutputNames =
        std::array{"OUT_0"_STRID, "OUT_1"_STRID, "VALID"_STRID, "ERROR"_STRID, "ERROR_CODE"_STRID};
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

  DEFINE_FIRMWARE_FB(FORTE_ML_PCA, "eclipse4diac::edgeml::ML_PCA"_STRID)

  FORTE_ML_PCA::FORTE_ML_PCA(const StringId paInstanceNameId, CFBContainer &paContainer) :
      CFunctionBlock(paContainer, cFBInterfaceSpec, paInstanceNameId),
      conn_CNF(*this, 0),
      conn_IN_0(nullptr),
      conn_IN_1(nullptr),
      conn_IN_2(nullptr),
      conn_IN_3(nullptr),
      conn_MEAN_0(nullptr),
      conn_MEAN_1(nullptr),
      conn_MEAN_2(nullptr),
      conn_MEAN_3(nullptr),
      conn_COMP1_0(nullptr),
      conn_COMP1_1(nullptr),
      conn_COMP1_2(nullptr),
      conn_COMP1_3(nullptr),
      conn_COMP2_0(nullptr),
      conn_COMP2_1(nullptr),
      conn_COMP2_2(nullptr),
      conn_COMP2_3(nullptr),
      conn_OUT_0(*this, 0, var_OUT_0),
      conn_OUT_1(*this, 1, var_OUT_1),
      conn_VALID(*this, 2, var_VALID),
      conn_ERROR(*this, 3, var_ERROR),
      conn_ERROR_CODE(*this, 4, var_ERROR_CODE) {
    setInitialValues();
  }

  void FORTE_ML_PCA::setInitialValues() {
    var_IN_0 = CIEC_REAL(0.0F);
    var_IN_1 = CIEC_REAL(0.0F);
    var_IN_2 = CIEC_REAL(0.0F);
    var_IN_3 = CIEC_REAL(0.0F);

    var_MEAN_0 = CIEC_REAL(0.0F);
    var_MEAN_1 = CIEC_REAL(0.0F);
    var_MEAN_2 = CIEC_REAL(0.0F);
    var_MEAN_3 = CIEC_REAL(0.0F);

    var_COMP1_0 = CIEC_REAL(1.0F);
    var_COMP1_1 = CIEC_REAL(0.0F);
    var_COMP1_2 = CIEC_REAL(0.0F);
    var_COMP1_3 = CIEC_REAL(0.0F);

    var_COMP2_0 = CIEC_REAL(0.0F);
    var_COMP2_1 = CIEC_REAL(1.0F);
    var_COMP2_2 = CIEC_REAL(0.0F);
    var_COMP2_3 = CIEC_REAL(0.0F);

    var_OUT_0 = CIEC_REAL(0.0F);
    var_OUT_1 = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_PCA::setError(const CIEC_USINT::TValueType paErrorCode) {
    var_OUT_0 = CIEC_REAL(0.0F);
    var_OUT_1 = CIEC_REAL(0.0F);
    var_VALID = CIEC_BOOL(false);
    var_ERROR = CIEC_BOOL(true);
    var_ERROR_CODE = CIEC_USINT(paErrorCode);
  }

  void FORTE_ML_PCA::clearError() {
    var_VALID = CIEC_BOOL(true);
    var_ERROR = CIEC_BOOL(false);
    var_ERROR_CODE = CIEC_USINT(scmErrorOk);
  }

  void FORTE_ML_PCA::executeEvent(const TEventID paEIID, CEventChainExecutionThread *const paECET) {
    if (scmEventREQID != paEIID) {
      return;
    }

    const std::array<CIEC_REAL, 16> allInputs{var_IN_0,   var_IN_1,   var_IN_2,   var_IN_3,
                                               var_MEAN_0, var_MEAN_1, var_MEAN_2, var_MEAN_3,
                                               var_COMP1_0, var_COMP1_1, var_COMP1_2, var_COMP1_3,
                                               var_COMP2_0, var_COMP2_1, var_COMP2_2, var_COMP2_3};
    for (const auto &value : allInputs) {
      if (!isFinite(value)) {
        setError(scmErrorNonFiniteInput);
        sendOutputEvent(scmEventCNFID, paECET);
        return;
      }
    }

    const std::array<CIEC_REAL::TValueType, 4> in{static_cast<CIEC_REAL::TValueType>(var_IN_0),
                                                   static_cast<CIEC_REAL::TValueType>(var_IN_1),
                                                   static_cast<CIEC_REAL::TValueType>(var_IN_2),
                                                   static_cast<CIEC_REAL::TValueType>(var_IN_3)};
    const std::array<CIEC_REAL::TValueType, 4> mean{static_cast<CIEC_REAL::TValueType>(var_MEAN_0),
                                                     static_cast<CIEC_REAL::TValueType>(var_MEAN_1),
                                                     static_cast<CIEC_REAL::TValueType>(var_MEAN_2),
                                                     static_cast<CIEC_REAL::TValueType>(var_MEAN_3)};
    const std::array<CIEC_REAL::TValueType, 4> comp1{static_cast<CIEC_REAL::TValueType>(var_COMP1_0),
                                                      static_cast<CIEC_REAL::TValueType>(var_COMP1_1),
                                                      static_cast<CIEC_REAL::TValueType>(var_COMP1_2),
                                                      static_cast<CIEC_REAL::TValueType>(var_COMP1_3)};
    const std::array<CIEC_REAL::TValueType, 4> comp2{static_cast<CIEC_REAL::TValueType>(var_COMP2_0),
                                                      static_cast<CIEC_REAL::TValueType>(var_COMP2_1),
                                                      static_cast<CIEC_REAL::TValueType>(var_COMP2_2),
                                                      static_cast<CIEC_REAL::TValueType>(var_COMP2_3)};

    CIEC_REAL::TValueType comp1NormSq = 0.0F;
    CIEC_REAL::TValueType comp2NormSq = 0.0F;
    for (std::size_t i = 0; i < 4; ++i) {
      comp1NormSq += comp1[i] * comp1[i];
      comp2NormSq += comp2[i] * comp2[i];
    }
    if (comp1NormSq <= 0.0F || comp2NormSq <= 0.0F) {
      setError(scmErrorInvalidComponent);
      sendOutputEvent(scmEventCNFID, paECET);
      return;
    }

    CIEC_REAL::TValueType projection0 = 0.0F;
    CIEC_REAL::TValueType projection1 = 0.0F;
    for (std::size_t i = 0; i < 4; ++i) {
      const auto centered = in[i] - mean[i];
      projection0 += centered * comp1[i];
      projection1 += centered * comp2[i];
    }

    var_OUT_0 = CIEC_REAL(projection0);
    var_OUT_1 = CIEC_REAL(projection1);
    clearError();

    sendOutputEvent(scmEventCNFID, paECET);
  }

  void FORTE_ML_PCA::readInputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventREQID: {
        readData(0, var_IN_0, conn_IN_0);
        readData(1, var_IN_1, conn_IN_1);
        readData(2, var_IN_2, conn_IN_2);
        readData(3, var_IN_3, conn_IN_3);
        readData(4, var_MEAN_0, conn_MEAN_0);
        readData(5, var_MEAN_1, conn_MEAN_1);
        readData(6, var_MEAN_2, conn_MEAN_2);
        readData(7, var_MEAN_3, conn_MEAN_3);
        readData(8, var_COMP1_0, conn_COMP1_0);
        readData(9, var_COMP1_1, conn_COMP1_1);
        readData(10, var_COMP1_2, conn_COMP1_2);
        readData(11, var_COMP1_3, conn_COMP1_3);
        readData(12, var_COMP2_0, conn_COMP2_0);
        readData(13, var_COMP2_1, conn_COMP2_1);
        readData(14, var_COMP2_2, conn_COMP2_2);
        readData(15, var_COMP2_3, conn_COMP2_3);
        break;
      }
      default: break;
    }
  }

  void FORTE_ML_PCA::writeOutputData(const TEventID paEIID) {
    switch (paEIID) {
      case scmEventCNFID: {
        writeData(cFBInterfaceSpec.getNumDIs() + 0, var_OUT_0, conn_OUT_0);
        writeData(cFBInterfaceSpec.getNumDIs() + 1, var_OUT_1, conn_OUT_1);
        writeData(cFBInterfaceSpec.getNumDIs() + 2, var_VALID, conn_VALID);
        writeData(cFBInterfaceSpec.getNumDIs() + 3, var_ERROR, conn_ERROR);
        writeData(cFBInterfaceSpec.getNumDIs() + 4, var_ERROR_CODE, conn_ERROR_CODE);
        break;
      }
      default: break;
    }
  }

  CIEC_ANY *FORTE_ML_PCA::getDI(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_IN_0;
      case 1: return &var_IN_1;
      case 2: return &var_IN_2;
      case 3: return &var_IN_3;
      case 4: return &var_MEAN_0;
      case 5: return &var_MEAN_1;
      case 6: return &var_MEAN_2;
      case 7: return &var_MEAN_3;
      case 8: return &var_COMP1_0;
      case 9: return &var_COMP1_1;
      case 10: return &var_COMP1_2;
      case 11: return &var_COMP1_3;
      case 12: return &var_COMP2_0;
      case 13: return &var_COMP2_1;
      case 14: return &var_COMP2_2;
      case 15: return &var_COMP2_3;
    }
    return nullptr;
  }

  CIEC_ANY *FORTE_ML_PCA::getDO(const size_t paIndex) {
    switch (paIndex) {
      case 0: return &var_OUT_0;
      case 1: return &var_OUT_1;
      case 2: return &var_VALID;
      case 3: return &var_ERROR;
      case 4: return &var_ERROR_CODE;
    }
    return nullptr;
  }

  CEventConnection *FORTE_ML_PCA::getEOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_CNF;
    }
    return nullptr;
  }

  CDataConnection **FORTE_ML_PCA::getDIConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_IN_0;
      case 1: return &conn_IN_1;
      case 2: return &conn_IN_2;
      case 3: return &conn_IN_3;
      case 4: return &conn_MEAN_0;
      case 5: return &conn_MEAN_1;
      case 6: return &conn_MEAN_2;
      case 7: return &conn_MEAN_3;
      case 8: return &conn_COMP1_0;
      case 9: return &conn_COMP1_1;
      case 10: return &conn_COMP1_2;
      case 11: return &conn_COMP1_3;
      case 12: return &conn_COMP2_0;
      case 13: return &conn_COMP2_1;
      case 14: return &conn_COMP2_2;
      case 15: return &conn_COMP2_3;
    }
    return nullptr;
  }

  CDataConnection *FORTE_ML_PCA::getDOConUnchecked(const TPortId paIndex) {
    switch (paIndex) {
      case 0: return &conn_OUT_0;
      case 1: return &conn_OUT_1;
      case 2: return &conn_VALID;
      case 3: return &conn_ERROR;
      case 4: return &conn_ERROR_CODE;
    }
    return nullptr;
  }

} // namespace forte::eclipse4diac::edgeml
