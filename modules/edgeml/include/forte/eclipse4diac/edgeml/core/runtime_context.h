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
#include "forte/eclipse4diac/edgeml/backend/null_backend.h"
#include "forte/eclipse4diac/edgeml/core/model_registry.h"

#ifdef FORTE_EDGEML_BACKEND_TFLITE
  #include "forte/eclipse4diac/edgeml/backend/tflite_backend.h"
#endif

#include <optional>
#include <string_view>
#include <vector>

namespace forte::eclipse4diac::edgeml {

  class EdgeMLRuntime final {
    public:
      static EdgeMLRuntime &instance();

      static bool isMockModelId(std::string_view paModelId);

      [[nodiscard]] bool backendAvailableForModel(std::string_view paModelId) const;
      IModelBackend &backendForModel(std::string_view paModelId);

      EEdgeMLError loadModel(const ModelMetadata &paMetadata, const std::vector<std::uint8_t> &paModelBinary);
      EEdgeMLError unloadModel(std::string_view paModelId);

      [[nodiscard]] bool hasModel(std::string_view paModelId) const;
      [[nodiscard]] std::optional<ModelMetadata> findModel(std::string_view paModelId) const;
      [[nodiscard]] std::vector<ModelMetadata> listModels() const;

    private:
      EdgeMLRuntime();

      ModelRegistry mRegistry;
      NullBackend mNullBackend;
#ifdef FORTE_EDGEML_BACKEND_TFLITE
      TFLiteBackend mTfliteBackend;
#endif
  };

} // namespace forte::eclipse4diac::edgeml
