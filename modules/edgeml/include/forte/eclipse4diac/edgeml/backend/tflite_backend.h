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

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/model.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace forte::eclipse4diac::edgeml {

  class TFLiteBackend final : public IModelBackend {
    public:
      TFLiteBackend() = default;
      ~TFLiteBackend() override;

      EEdgeMLError loadModel(const ModelMetadata &paMetadata,
                             const std::vector<std::uint8_t> &paModelBinary) override;
      EEdgeMLError unloadModel(std::string_view paModelId) override;
      EEdgeMLError infer(std::string_view paModelId,
                         std::span<const float> paInput,
                         std::span<float> paOutput,
                         InferenceStats &paStats) override;

    private:
      struct ModelInstance {
        std::vector<std::uint8_t> binary;
        std::unique_ptr<tflite::FlatBufferModel> model;
        std::unique_ptr<tflite::Interpreter> interpreter;
      };

      std::unordered_map<std::string, ModelInstance> mLoadedModels;
  };

} // namespace forte::eclipse4diac::edgeml
