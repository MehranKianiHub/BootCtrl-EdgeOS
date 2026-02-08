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

  class FORTE_ML_Scale final : public CFunctionBlock {
      DECLARE_FIRMWARE_FB(FORTE_ML_Scale)

    public:
      static constexpr CIEC_USINT::TValueType scmErrorOk = 0;
      static constexpr CIEC_USINT::TValueType scmErrorNonFiniteInput = 1;
      static constexpr CIEC_USINT::TValueType scmErrorInvalidInputRange = 2;

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
      FORTE_ML_Scale(StringId paInstanceNameId, CFBContainer &paContainer);

      CIEC_REAL var_IN;
      CIEC_REAL var_IN_MIN;
      CIEC_REAL var_IN_MAX;
      CIEC_REAL var_OUT_MIN;
      CIEC_REAL var_OUT_MAX;
      CIEC_BOOL var_CLAMP;

      CIEC_REAL var_OUT;
      CIEC_BOOL var_VALID;
      CIEC_BOOL var_ERROR;
      CIEC_USINT var_ERROR_CODE;

      CEventConnection conn_CNF;

      CDataConnection *conn_IN;
      CDataConnection *conn_IN_MIN;
      CDataConnection *conn_IN_MAX;
      CDataConnection *conn_OUT_MIN;
      CDataConnection *conn_OUT_MAX;
      CDataConnection *conn_CLAMP;

      COutDataConnection<CIEC_REAL> conn_OUT;
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
