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

#include "forte/eclipse4diac/edgeml/backend/backend_interface.h"

#include <mutex>
#include <string>
#include <unordered_set>

namespace forte::eclipse4diac::edgeml {

  class NullBackend final : public IModelBackend {
    public:
      EEdgeMLError loadModel(const ModelMetadata &paMetadata,
                             const std::vector<std::uint8_t> &paModelBinary) override;
      EEdgeMLError unloadModel(std::string_view paModelId) override;
      EEdgeMLError infer(std::string_view paModelId,
                         std::span<const float> paInput,
                         std::span<float> paOutput,
                         InferenceStats &paStats) override;

    private:
      mutable std::mutex mMutex;
      std::unordered_set<std::string> mLoadedModels;
  };

} // namespace forte::eclipse4diac::edgeml
