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
#include "forte/datatypes/forte_udint.h"
#include "forte/datatypes/forte_usint.h"
#include "forte/funcbloc.h"

#include <cstddef>
#include <cstdint>

namespace forte::eclipse4diac::edgeml {

  class FORTE_ML_Monitoring final : public CFunctionBlock {
      DECLARE_FIRMWARE_FB(FORTE_ML_Monitoring)

    public:
      static constexpr CIEC_USINT::TValueType scmCommandReport = 0;
      static constexpr CIEC_USINT::TValueType scmCommandSnapshot = 1;
      static constexpr CIEC_USINT::TValueType scmCommandReset = 2;

      static constexpr CIEC_USINT::TValueType scmErrorOk = 0;
      static constexpr CIEC_USINT::TValueType scmErrorInvalidCommand = 1;

    private:
      static const TEventID scmEventREQID = 0;
      static const TEventID scmEventCNFID = 0;

      void executeEvent(TEventID paEIID, CEventChainExecutionThread *const paECET) override;
      void readInputData(TEventID paEIID) override;
      void writeOutputData(TEventID paEIID) override;
      void setInitialValues() override;

      void setError(CIEC_USINT::TValueType paErrorCode);
      void clearError();
      void clearMetrics();
      void publishMetrics();

    public:
      FORTE_ML_Monitoring(StringId paInstanceNameId, CFBContainer &paContainer);

      CIEC_USINT var_COMMAND;
      CIEC_UDINT var_INFERENCE_US;
      CIEC_BOOL var_HAS_ERROR;
      CIEC_USINT var_IN_ERROR_CODE;

      CIEC_UDINT var_TOTAL_INFERENCES;
      CIEC_UDINT var_TOTAL_ERRORS;
      CIEC_UDINT var_AVG_INFERENCE_US;
      CIEC_UDINT var_MAX_INFERENCE_US;
      CIEC_USINT var_LAST_ERROR_CODE;
      CIEC_REAL var_HEALTH_SCORE;
      CIEC_BOOL var_SUCCESS;
      CIEC_BOOL var_ERROR;
      CIEC_USINT var_ERROR_CODE;

      CEventConnection conn_CNF;

      CDataConnection *conn_COMMAND;
      CDataConnection *conn_INFERENCE_US;
      CDataConnection *conn_HAS_ERROR;
      CDataConnection *conn_IN_ERROR_CODE;

      COutDataConnection<CIEC_UDINT> conn_TOTAL_INFERENCES;
      COutDataConnection<CIEC_UDINT> conn_TOTAL_ERRORS;
      COutDataConnection<CIEC_UDINT> conn_AVG_INFERENCE_US;
      COutDataConnection<CIEC_UDINT> conn_MAX_INFERENCE_US;
      COutDataConnection<CIEC_USINT> conn_LAST_ERROR_CODE;
      COutDataConnection<CIEC_REAL> conn_HEALTH_SCORE;
      COutDataConnection<CIEC_BOOL> conn_SUCCESS;
      COutDataConnection<CIEC_BOOL> conn_ERROR;
      COutDataConnection<CIEC_USINT> conn_ERROR_CODE;

      CIEC_ANY *getDI(size_t paIndex) override;
      CIEC_ANY *getDO(size_t paIndex) override;
      CEventConnection *getEOConUnchecked(TPortId paIndex) override;
      CDataConnection **getDIConUnchecked(TPortId paIndex) override;
      CDataConnection *getDOConUnchecked(TPortId paIndex) override;

    private:
      std::uint64_t mTotalInferenceTimeUs;
      CIEC_UDINT::TValueType mTotalInferences;
      CIEC_UDINT::TValueType mTotalErrors;
      CIEC_UDINT::TValueType mMaxInferenceUs;
      CIEC_USINT::TValueType mLastErrorCode;
  };

} // namespace forte::eclipse4diac::edgeml
