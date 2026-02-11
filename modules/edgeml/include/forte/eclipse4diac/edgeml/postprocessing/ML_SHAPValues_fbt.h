/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#pragma once

#include "forte/datatypes/forte_bool.h"
#include "forte/datatypes/forte_real.h"
#include "forte/datatypes/forte_usint.h"
#include "forte/funcbloc.h"

#include <cstddef>

namespace forte::eclipse4diac::edgeml {

  class FORTE_ML_SHAPValues final : public CFunctionBlock {
      DECLARE_FIRMWARE_FB(FORTE_ML_SHAPValues)

    public:
      static constexpr CIEC_USINT::TValueType scmErrorOk = 0;
      static constexpr CIEC_USINT::TValueType scmErrorNonFiniteInput = 1;

    private:
      static const TEventID scmEventREQID = 0;
      static const TEventID scmEventCNFID = 0;

      void executeEvent(TEventID paEIID, CEventChainExecutionThread *const paECET) override;
      void readInputData(TEventID paEIID) override;
      void writeOutputData(TEventID paEIID) override;
      void setInitialValues() override;

      void setError(CIEC_USINT::TValueType paErrorCode);
      void clearError();

    public:
      FORTE_ML_SHAPValues(StringId paInstanceNameId, CFBContainer &paContainer);

      CIEC_REAL var_IN_0;
      CIEC_REAL var_IN_1;
      CIEC_REAL var_IN_2;
      CIEC_REAL var_IN_3;
      CIEC_REAL var_BASELINE_0;
      CIEC_REAL var_BASELINE_1;
      CIEC_REAL var_BASELINE_2;
      CIEC_REAL var_BASELINE_3;
      CIEC_REAL var_WEIGHT_0;
      CIEC_REAL var_WEIGHT_1;
      CIEC_REAL var_WEIGHT_2;
      CIEC_REAL var_WEIGHT_3;
      CIEC_BOOL var_NORMALIZE;

      CIEC_REAL var_SHAP_0;
      CIEC_REAL var_SHAP_1;
      CIEC_REAL var_SHAP_2;
      CIEC_REAL var_SHAP_3;
      CIEC_REAL var_TOTAL_CONTRIBUTION;
      CIEC_BOOL var_VALID;
      CIEC_BOOL var_ERROR;
      CIEC_USINT var_ERROR_CODE;

      CEventConnection conn_CNF;

      CDataConnection *conn_IN_0;
      CDataConnection *conn_IN_1;
      CDataConnection *conn_IN_2;
      CDataConnection *conn_IN_3;
      CDataConnection *conn_BASELINE_0;
      CDataConnection *conn_BASELINE_1;
      CDataConnection *conn_BASELINE_2;
      CDataConnection *conn_BASELINE_3;
      CDataConnection *conn_WEIGHT_0;
      CDataConnection *conn_WEIGHT_1;
      CDataConnection *conn_WEIGHT_2;
      CDataConnection *conn_WEIGHT_3;
      CDataConnection *conn_NORMALIZE;

      COutDataConnection<CIEC_REAL> conn_SHAP_0;
      COutDataConnection<CIEC_REAL> conn_SHAP_1;
      COutDataConnection<CIEC_REAL> conn_SHAP_2;
      COutDataConnection<CIEC_REAL> conn_SHAP_3;
      COutDataConnection<CIEC_REAL> conn_TOTAL_CONTRIBUTION;
      COutDataConnection<CIEC_BOOL> conn_VALID;
      COutDataConnection<CIEC_BOOL> conn_ERROR;
      COutDataConnection<CIEC_USINT> conn_ERROR_CODE;

      CIEC_ANY *getDI(size_t paIndex) override;
      CIEC_ANY *getDO(size_t paIndex) override;
      CEventConnection *getEOConUnchecked(TPortId paIndex) override;
      CDataConnection **getDIConUnchecked(TPortId paIndex) override;
      CDataConnection *getDOConUnchecked(TPortId paIndex) override;
  };

} // namespace forte::eclipse4diac::edgeml
