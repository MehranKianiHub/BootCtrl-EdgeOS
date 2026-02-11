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
#include <cstdint>
#include <string>
#include <vector>

namespace forte::eclipse4diac::edgeml {

  class FORTE_ML_OTAUpdate final : public CFunctionBlock {
      DECLARE_FIRMWARE_FB(FORTE_ML_OTAUpdate)

    public:
      static constexpr CIEC_USINT::TValueType scmCommandBegin = 0;
      static constexpr CIEC_USINT::TValueType scmCommandChunk = 1;
      static constexpr CIEC_USINT::TValueType scmCommandCommit = 2;
      static constexpr CIEC_USINT::TValueType scmCommandAbort = 3;
      static constexpr CIEC_USINT::TValueType scmCommandRollback = 4;
      static constexpr CIEC_USINT::TValueType scmCommandRecover = 5;
      static constexpr CIEC_USINT::TValueType scmCommandProvisionAnchor = 6;
      static constexpr CIEC_USINT::TValueType scmCommandRemoveAnchor = 7;

      static constexpr CIEC_USINT::TValueType scmStateIdle = 0;
      static constexpr CIEC_USINT::TValueType scmStateStaging = 1;
      static constexpr CIEC_USINT::TValueType scmStateCommitted = 2;
      static constexpr CIEC_USINT::TValueType scmStateRolledBack = 3;

      static constexpr CIEC_USINT::TValueType scmErrorOk = 0;
      static constexpr CIEC_USINT::TValueType scmErrorInvalidCommand = 1;
      static constexpr CIEC_USINT::TValueType scmErrorInvalidStateTransition = 2;
      static constexpr CIEC_USINT::TValueType scmErrorEmptyModelId = 3;
      static constexpr CIEC_USINT::TValueType scmErrorInvalidPayload = 4;
      static constexpr CIEC_USINT::TValueType scmErrorBackendUnavailable = 5;
      static constexpr CIEC_USINT::TValueType scmErrorApplyFailed = 6;
      static constexpr CIEC_USINT::TValueType scmErrorRollbackUnavailable = 7;
      static constexpr CIEC_USINT::TValueType scmErrorHashMismatch = 8;
      static constexpr CIEC_USINT::TValueType scmErrorSignatureMismatch = 9;
      static constexpr CIEC_USINT::TValueType scmErrorPersistenceFailed = 10;
      static constexpr CIEC_USINT::TValueType scmErrorRecoveryFailed = 11;
      static constexpr CIEC_USINT::TValueType scmErrorTrustAnchorUnavailable = 12;
      static constexpr CIEC_USINT::TValueType scmErrorTrustStoreFailure = 13;
      static constexpr CIEC_USINT::TValueType scmErrorTransportPolicyViolation = 14;
      static constexpr CIEC_USINT::TValueType scmErrorReplayDetected = 15;

    private:
      static const TEventID scmEventREQID = 0;
      static const TEventID scmEventCNFID = 0;

      void executeEvent(TEventID paEIID, CEventChainExecutionThread *const paECET) override;
      void readInputData(TEventID paEIID) override;
      void writeOutputData(TEventID paEIID) override;
      void setInitialValues() override;

      void setError(CIEC_USINT::TValueType paErrorCode);
      void clearError();
      void clearStaging();
      void setState(CIEC_USINT::TValueType paState);
      void updateProgress();
      bool persistState();
      bool recoverState();
      void applyRecoveredState();
      bool validateTransportPolicy() const;
      bool validateNonce(std::string &paNonceOut);

    public:
      FORTE_ML_OTAUpdate(StringId paInstanceNameId, CFBContainer &paContainer);

      CIEC_USINT var_COMMAND;
      CIEC_STRING var_MODEL_ID;
      CIEC_STRING var_VERSION;
      CIEC_UDINT var_EXPECTED_SIZE;
      CIEC_STRING var_EXPECTED_SHA256;
      CIEC_STRING var_SIGNATURE;
      CIEC_STRING var_TRUST_ANCHOR;
      CIEC_STRING var_TRUST_ANCHOR_ID;
      CIEC_STRING var_TRUST_STORE_PATH;
      CIEC_STRING var_SOURCE_URI;
      CIEC_BOOL var_TRANSPORT_SECURE;
      CIEC_STRING var_NONCE;
      CIEC_STRING var_STATE_PATH;
      CIEC_ANY_VARIANT var_CHUNK;

      CIEC_USINT var_STATE;
      CIEC_USINT var_PROGRESS;
      CIEC_STRING var_ACTIVE_MODEL_ID;
      CIEC_STRING var_STAGED_MODEL_ID;
      CIEC_UDINT var_STAGED_SIZE;
      CIEC_BOOL var_ROLLBACK_AVAILABLE;
      CIEC_BOOL var_SUCCESS;
      CIEC_BOOL var_ERROR;
      CIEC_USINT var_ERROR_CODE;

      CEventConnection conn_CNF;

      CDataConnection *conn_COMMAND;
      CDataConnection *conn_MODEL_ID;
      CDataConnection *conn_VERSION;
      CDataConnection *conn_EXPECTED_SIZE;
      CDataConnection *conn_EXPECTED_SHA256;
      CDataConnection *conn_SIGNATURE;
      CDataConnection *conn_TRUST_ANCHOR;
      CDataConnection *conn_TRUST_ANCHOR_ID;
      CDataConnection *conn_TRUST_STORE_PATH;
      CDataConnection *conn_SOURCE_URI;
      CDataConnection *conn_TRANSPORT_SECURE;
      CDataConnection *conn_NONCE;
      CDataConnection *conn_STATE_PATH;
      CDataConnection *conn_CHUNK;

      COutDataConnection<CIEC_USINT> conn_STATE;
      COutDataConnection<CIEC_USINT> conn_PROGRESS;
      COutDataConnection<CIEC_STRING> conn_ACTIVE_MODEL_ID;
      COutDataConnection<CIEC_STRING> conn_STAGED_MODEL_ID;
      COutDataConnection<CIEC_UDINT> conn_STAGED_SIZE;
      COutDataConnection<CIEC_BOOL> conn_ROLLBACK_AVAILABLE;
      COutDataConnection<CIEC_BOOL> conn_SUCCESS;
      COutDataConnection<CIEC_BOOL> conn_ERROR;
      COutDataConnection<CIEC_USINT> conn_ERROR_CODE;

      CIEC_ANY *getDI(size_t paIndex) override;
      CIEC_ANY *getDO(size_t paIndex) override;
      CEventConnection *getEOConUnchecked(TPortId paIndex) override;
      CDataConnection **getDIConUnchecked(TPortId paIndex) override;
      CDataConnection *getDOConUnchecked(TPortId paIndex) override;

    private:
      std::string mStagedModelId;
      std::string mStagedVersion;
      std::string mStagedExpectedSha256;
      std::string mStagedSignature;
      std::string mStagedTrustAnchor;
      std::string mStagedTrustAnchorId;
      std::string mStagedSourceUri;
      std::string mStagedNonce;
      std::vector<std::uint8_t> mStagedBlob;
      std::string mActiveModelId;
      std::string mPreviousActiveModelId;
      std::string mStatePath;
      std::string mTrustStorePath;
      std::string mLastAppliedNonce;
      CIEC_UDINT::TValueType mExpectedSize;
      bool mRollbackAvailable;
  };

} // namespace forte::eclipse4diac::edgeml
