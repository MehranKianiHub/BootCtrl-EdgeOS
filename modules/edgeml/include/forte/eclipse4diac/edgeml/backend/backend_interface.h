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

#include "forte/eclipse4diac/edgeml/core/edgeml_status.h"
#include "forte/eclipse4diac/edgeml/core/model_metadata.h"

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace forte::eclipse4diac::edgeml {

  class IModelBackend {
    public:
      virtual ~IModelBackend() = default;

      virtual EEdgeMLError loadModel(const ModelMetadata &paMetadata,
                                     const std::vector<std::uint8_t> &paModelBinary) = 0;
      virtual EEdgeMLError unloadModel(std::string_view paModelId) = 0;
      virtual EEdgeMLError infer(std::string_view paModelId,
                                 std::span<const float> paInput,
                                 std::span<float> paOutput,
                                 InferenceStats &paStats) = 0;
  };

} // namespace forte::eclipse4diac::edgeml
