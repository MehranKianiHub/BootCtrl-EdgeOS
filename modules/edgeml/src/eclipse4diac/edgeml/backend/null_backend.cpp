/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/backend/null_backend.h"

#include <algorithm>
#include <mutex>

namespace forte::eclipse4diac::edgeml {

  EEdgeMLError NullBackend::loadModel(const ModelMetadata &paMetadata,
                                      const std::vector<std::uint8_t> & /*paModelBinary*/) {
    if (!paMetadata.isValid()) {
      return EEdgeMLError::kInvalidModelId;
    }

    std::lock_guard lock(mMutex);
    const auto inserted = mLoadedModels.emplace(paMetadata.id).second;
    return inserted ? EEdgeMLError::kOk : EEdgeMLError::kModelAlreadyExists;
  }

  EEdgeMLError NullBackend::unloadModel(const std::string_view paModelId) {
    std::lock_guard lock(mMutex);
    return 0 != mLoadedModels.erase(std::string(paModelId)) ? EEdgeMLError::kOk : EEdgeMLError::kModelNotFound;
  }

  EEdgeMLError NullBackend::infer(const std::string_view paModelId,
                                  const std::span<const float> paInput,
                                  const std::span<float> paOutput,
                                  InferenceStats &paStats) {
    std::lock_guard lock(mMutex);
    if (!mLoadedModels.contains(std::string(paModelId))) {
      return EEdgeMLError::kModelNotLoaded;
    }

    if (paOutput.empty()) {
      return EEdgeMLError::kOutputTooSmall;
    }

    const std::size_t elementsToCopy = std::min(paInput.size(), paOutput.size());
    std::copy_n(paInput.begin(), elementsToCopy, paOutput.begin());

    paStats.inferenceTimeUs = 0;
    paStats.outputElements = static_cast<std::uint32_t>(elementsToCopy);

    if (paOutput.size() < paInput.size()) {
      return EEdgeMLError::kOutputTooSmall;
    }

    return EEdgeMLError::kOk;
  }

} // namespace forte::eclipse4diac::edgeml
