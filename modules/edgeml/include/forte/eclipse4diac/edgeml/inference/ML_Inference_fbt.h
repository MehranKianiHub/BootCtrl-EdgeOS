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

#include "forte/datatypes/forte_any_variant.h"
#include "forte/datatypes/forte_array.h"
#include "forte/datatypes/forte_array_common.h"
#include "forte/datatypes/forte_array_fixed.h"
#include "forte/datatypes/forte_array_variable.h"
#include "forte/datatypes/forte_bool.h"
#include "forte/datatypes/forte_real.h"
#include "forte/datatypes/forte_string.h"
#include "forte/datatypes/forte_udint.h"
#include "forte/datatypes/forte_usint.h"
#include "forte/funcbloc.h"

#include <cstddef>

namespace forte::eclipse4diac::edgeml {

  class FORTE_ML_Inference final : public CFunctionBlock {
      DECLARE_FIRMWARE_FB(FORTE_ML_Inference)

    public:
      static constexpr CIEC_USINT::TValueType scmErrorOk = 0;
      static constexpr CIEC_USINT::TValueType scmErrorEmptyModelId = 1;
      static constexpr CIEC_USINT::TValueType scmErrorInvalidInputVector = 2;
      static constexpr CIEC_USINT::TValueType scmErrorInvalidOutputCapacity = 3;
      static constexpr CIEC_USINT::TValueType scmErrorModelNotLoaded = 4;
      static constexpr CIEC_USINT::TValueType scmErrorBackendFailure = 5;
      static constexpr CIEC_USINT::TValueType scmErrorNonFiniteInput = 6;
      static constexpr CIEC_USINT::TValueType scmErrorOutputTooSmall = 7;

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
      FORTE_ML_Inference(StringId paInstanceNameId, CFBContainer &paContainer);

      CIEC_STRING var_MODEL_ID;
      CIEC_ANY_VARIANT var_IN_VALUES;
      CIEC_UDINT var_OUT_CAPACITY;

      CIEC_ANY_VARIANT var_OUT_VALUES;
      CIEC_BOOL var_VALID;
      CIEC_BOOL var_ERROR;
      CIEC_USINT var_ERROR_CODE;
      CIEC_UDINT var_OUTPUT_COUNT;
      CIEC_UDINT var_INFERENCE_US;

      CEventConnection conn_CNF;

      CDataConnection *conn_MODEL_ID;
      CDataConnection *conn_IN_VALUES;
      CDataConnection *conn_OUT_CAPACITY;

      COutDataConnection<CIEC_ANY_VARIANT> conn_OUT_VALUES;
      COutDataConnection<CIEC_BOOL> conn_VALID;
      COutDataConnection<CIEC_BOOL> conn_ERROR;
      COutDataConnection<CIEC_USINT> conn_ERROR_CODE;
      COutDataConnection<CIEC_UDINT> conn_OUTPUT_COUNT;
      COutDataConnection<CIEC_UDINT> conn_INFERENCE_US;

      CIEC_ANY *getDI(size_t paIndex) override;
      CIEC_ANY *getDO(size_t paIndex) override;
      CEventConnection *getEOConUnchecked(TPortId paIndex) override;
      CDataConnection **getDIConUnchecked(TPortId paIndex) override;
      CDataConnection *getDOConUnchecked(TPortId paIndex) override;

    private:
      void clearOutputVector();
  };

} // namespace forte::eclipse4diac::edgeml
