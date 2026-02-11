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

#include "forte/datatypes/forte_array.h"
#include "forte/datatypes/forte_array_common.h"
#include "forte/datatypes/forte_array_fixed.h"
#include "forte/datatypes/forte_array_variable.h"
#include "forte/datatypes/forte_any_variant.h"
#include "forte/datatypes/forte_byte.h"
#include "forte/datatypes/forte_bool.h"
#include "forte/datatypes/forte_string.h"
#include "forte/datatypes/forte_udint.h"
#include "forte/datatypes/forte_usint.h"
#include "forte/funcbloc.h"

#include <cstddef>

namespace forte::eclipse4diac::edgeml {

  class FORTE_ML_ModelManager final : public CFunctionBlock {
      DECLARE_FIRMWARE_FB(FORTE_ML_ModelManager)

    public:
      static constexpr CIEC_USINT::TValueType scmCommandLoad = 0;
      static constexpr CIEC_USINT::TValueType scmCommandUnload = 1;
      static constexpr CIEC_USINT::TValueType scmCommandList = 2;
      static constexpr CIEC_USINT::TValueType scmCommandInfo = 3;

      static constexpr CIEC_USINT::TValueType scmErrorOk = 0;
      static constexpr CIEC_USINT::TValueType scmErrorInvalidCommand = 1;
      static constexpr CIEC_USINT::TValueType scmErrorEmptyModelId = 2;
      static constexpr CIEC_USINT::TValueType scmErrorModelNotFound = 3;
      static constexpr CIEC_USINT::TValueType scmErrorBackendUnavailable = 4;
      static constexpr CIEC_USINT::TValueType scmErrorBackendFailure = 5;
      static constexpr CIEC_USINT::TValueType scmErrorInvalidInput = 6;

    private:
      static const TEventID scmEventREQID = 0;
      static const TEventID scmEventCNFID = 0;

      void executeEvent(TEventID paEIID, CEventChainExecutionThread *const paECET) override;
      void readInputData(TEventID paEIID) override;
      void writeOutputData(TEventID paEIID) override;
      void setInitialValues() override;

      void setError(CIEC_USINT::TValueType paErrorCode);
      void clearError();
      void clearInfo();

    public:
      FORTE_ML_ModelManager(StringId paInstanceNameId, CFBContainer &paContainer);

      CIEC_USINT var_COMMAND;
      CIEC_STRING var_MODEL_ID;
      CIEC_STRING var_VERSION;
      CIEC_UDINT var_SIZE_BYTES;
      CIEC_STRING var_SHA256;
      CIEC_ANY_VARIANT var_MODEL_BLOB;

      CIEC_BOOL var_SUCCESS;
      CIEC_BOOL var_ERROR;
      CIEC_USINT var_ERROR_CODE;
      CIEC_UDINT var_MODEL_COUNT;
      CIEC_STRING var_INFO_MODEL_ID;
      CIEC_STRING var_INFO_VERSION;
      CIEC_UDINT var_INFO_SIZE_BYTES;
      CIEC_STRING var_INFO_SHA256;

      CEventConnection conn_CNF;

      CDataConnection *conn_COMMAND;
      CDataConnection *conn_MODEL_ID;
      CDataConnection *conn_VERSION;
      CDataConnection *conn_SIZE_BYTES;
      CDataConnection *conn_SHA256;
      CDataConnection *conn_MODEL_BLOB;

      COutDataConnection<CIEC_BOOL> conn_SUCCESS;
      COutDataConnection<CIEC_BOOL> conn_ERROR;
      COutDataConnection<CIEC_USINT> conn_ERROR_CODE;
      COutDataConnection<CIEC_UDINT> conn_MODEL_COUNT;
      COutDataConnection<CIEC_STRING> conn_INFO_MODEL_ID;
      COutDataConnection<CIEC_STRING> conn_INFO_VERSION;
      COutDataConnection<CIEC_UDINT> conn_INFO_SIZE_BYTES;
      COutDataConnection<CIEC_STRING> conn_INFO_SHA256;

      CIEC_ANY *getDI(size_t paIndex) override;
      CIEC_ANY *getDO(size_t paIndex) override;
      CEventConnection *getEOConUnchecked(TPortId paIndex) override;
      CDataConnection **getDIConUnchecked(TPortId paIndex) override;
      CDataConnection *getDOConUnchecked(TPortId paIndex) override;
  };

} // namespace forte::eclipse4diac::edgeml
