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
#include "forte/funcbloc.h"

#include <cstddef>

namespace forte::eclipse4diac::edgeml {

  class FORTE_ML_Threshold final : public CFunctionBlock {
      DECLARE_FIRMWARE_FB(FORTE_ML_Threshold)

    private:
      static const TEventID scmEventREQID = 0;
      static const TEventID scmEventCNFID = 0;

      void executeEvent(TEventID paEIID, CEventChainExecutionThread *const paECET) override;
      void readInputData(TEventID paEIID) override;
      void writeOutputData(TEventID paEIID) override;
      void setInitialValues() override;

    public:
      FORTE_ML_Threshold(StringId paInstanceNameId, CFBContainer &paContainer);

      CIEC_REAL var_IN;
      CIEC_REAL var_THRESHOLD;
      CIEC_BOOL var_INCLUSIVE;

      CIEC_BOOL var_EXCEEDS;
      CIEC_REAL var_MARGIN;

      CEventConnection conn_CNF;

      CDataConnection *conn_IN;
      CDataConnection *conn_THRESHOLD;
      CDataConnection *conn_INCLUSIVE;

      COutDataConnection<CIEC_BOOL> conn_EXCEEDS;
      COutDataConnection<CIEC_REAL> conn_MARGIN;

      CIEC_ANY *getDI(size_t paIndex) override;
      CIEC_ANY *getDO(size_t paIndex) override;
      CEventConnection *getEOConUnchecked(TPortId paIndex) override;
      CDataConnection **getDIConUnchecked(TPortId paIndex) override;
      CDataConnection *getDOConUnchecked(TPortId paIndex) override;

      void evt_REQ(const CIEC_REAL &paIN,
                   const CIEC_REAL &paTHRESHOLD,
                   const CIEC_BOOL &paINCLUSIVE,
                   CAnyBitOutputParameter<CIEC_BOOL> paEXCEEDS,
                   COutputParameter<CIEC_REAL> paMARGIN) {
        COutputGuard guard_paEXCEEDS(paEXCEEDS);
        COutputGuard guard_paMARGIN(paMARGIN);
        var_IN = paIN;
        var_THRESHOLD = paTHRESHOLD;
        var_INCLUSIVE = paINCLUSIVE;
        executeEvent(scmEventREQID, nullptr);
        *paEXCEEDS = var_EXCEEDS;
        *paMARGIN = var_MARGIN;
      }
  };

  CIEC_BOOL func_ML_Threshold(const CIEC_REAL &paIN, const CIEC_REAL &paThreshold, const CIEC_BOOL &paInclusive);

} // namespace forte::eclipse4diac::edgeml
