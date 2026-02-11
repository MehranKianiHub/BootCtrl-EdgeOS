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

#include <cstdint>
#include <string>

namespace forte::eclipse4diac::edgeml {

  struct OtaPersistentState {
      std::string activeModelId;
      std::string previousActiveModelId;
      std::string stagedModelId;
      std::string stagedVersion;
      std::string lastAppliedNonce;
      bool rollbackAvailable;
      std::uint8_t state;
      std::uint32_t expectedSize;
      std::uint32_t stagedSize;
  };

  enum class EOtaPersistenceStatus {
    kOk,
    kInvalidPath,
    kIoError,
    kParseError,
    kNotFound,
  };

  class OtaPersistence {
    public:
      static EOtaPersistenceStatus save(const OtaPersistentState &paState, const std::string &paPath);
      static EOtaPersistenceStatus load(const std::string &paPath, OtaPersistentState &paState);
  };

} // namespace forte::eclipse4diac::edgeml
