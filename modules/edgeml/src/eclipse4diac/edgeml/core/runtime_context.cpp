/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/core/runtime_context.h"

#include <mutex>
#include <vector>

namespace forte::eclipse4diac::edgeml {

  EdgeMLRuntime &EdgeMLRuntime::instance() {
    static EdgeMLRuntime runtime;
    return runtime;
  }

  bool EdgeMLRuntime::isMockModelId(const std::string_view paModelId) {
    return paModelId.starts_with("mock.");
  }

  bool EdgeMLRuntime::backendAvailableForModel(const std::string_view paModelId) const {
    if (isMockModelId(paModelId)) {
      return true;
    }
#ifdef FORTE_EDGEML_BACKEND_TFLITE
    return true;
#else
    (void) paModelId;
    return false;
#endif
  }

  IModelBackend &EdgeMLRuntime::backendForModel(const std::string_view paModelId) {
#ifdef FORTE_EDGEML_BACKEND_TFLITE
    if (!isMockModelId(paModelId)) {
      return mTfliteBackend;
    }
#else
    (void) paModelId;
#endif
    return mNullBackend;
  }

  EEdgeMLError EdgeMLRuntime::loadModel(const ModelMetadata &paMetadata, const std::vector<std::uint8_t> &paModelBinary) {
    if (!paMetadata.isValid()) {
      return EEdgeMLError::kInvalidModelId;
    }
    if (!backendAvailableForModel(paMetadata.id)) {
      return EEdgeMLError::kBackendUnavailable;
    }

    std::lock_guard lock(mRuntimeMutex);
    auto &backend = backendForModel(paMetadata.id);
    const auto loadStatus = backend.loadModel(paMetadata, paModelBinary);
    if (EEdgeMLError::kOk == loadStatus || EEdgeMLError::kModelAlreadyExists == loadStatus) {
      [[maybe_unused]] const auto inserted = mRegistry.registerModel(paMetadata);
    }
    return loadStatus;
  }

  EEdgeMLError EdgeMLRuntime::unloadModel(const std::string_view paModelId) {
    if (!backendAvailableForModel(paModelId)) {
      return EEdgeMLError::kBackendUnavailable;
    }

    std::lock_guard lock(mRuntimeMutex);
    auto &backend = backendForModel(paModelId);
    const auto unloadStatus = backend.unloadModel(paModelId);
    if (EEdgeMLError::kOk == unloadStatus) {
      [[maybe_unused]] const auto removed = mRegistry.unregisterModel(paModelId);
    }
    return unloadStatus;
  }

  EEdgeMLError EdgeMLRuntime::inferModel(const std::string_view paModelId,
                                         const std::span<const float> paInput,
                                         const std::span<float> paOutput,
                                         InferenceStats &paStats) {
    if (paModelId.empty()) {
      return EEdgeMLError::kInvalidModelId;
    }
    if (!backendAvailableForModel(paModelId)) {
      return EEdgeMLError::kBackendUnavailable;
    }

    std::lock_guard lock(mRuntimeMutex);
    auto &backend = backendForModel(paModelId);
    return backend.infer(paModelId, paInput, paOutput, paStats);
  }

  bool EdgeMLRuntime::hasModel(const std::string_view paModelId) const {
    return mRegistry.hasModel(paModelId);
  }

  std::optional<ModelMetadata> EdgeMLRuntime::findModel(const std::string_view paModelId) const {
    return mRegistry.findModel(paModelId);
  }

  std::vector<ModelMetadata> EdgeMLRuntime::listModels() const {
    return mRegistry.listModels();
  }

  EdgeMLRuntime::EdgeMLRuntime() {
    const std::vector<std::uint8_t> binary{0x00};
    const ModelMetadata metadata{"mock.default", "0.1.0", binary.size(), "sha256:mock.default"};
    [[maybe_unused]] const auto status = loadModel(metadata, binary);
  }

} // namespace forte::eclipse4diac::edgeml
