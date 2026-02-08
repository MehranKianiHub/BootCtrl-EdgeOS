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

namespace forte::eclipse4diac::edgeml {

  enum class EEdgeMLError : std::uint8_t {
    kOk = 0,
    kInvalidModelId,
    kModelAlreadyExists,
    kModelNotFound,
    kModelNotLoaded,
    kInvalidInput,
    kOutputTooSmall,
    kBackendUnavailable,
  };

  struct InferenceStats {
    std::uint32_t inferenceTimeUs{0};
    std::uint32_t outputElements{0};
  };

} // namespace forte::eclipse4diac::edgeml
