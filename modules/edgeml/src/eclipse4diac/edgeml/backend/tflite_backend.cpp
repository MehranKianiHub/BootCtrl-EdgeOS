/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/backend/tflite_backend.h"

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/interpreter_builder.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>

#include <algorithm>
#include <chrono>
#include <mutex>
#include <utility>

namespace forte::eclipse4diac::edgeml {
  namespace {
    std::size_t tensorElementCount(const TfLiteTensor &paTensor) {
      if (nullptr == paTensor.dims || paTensor.dims->size <= 0) {
        return 0;
      }

      std::size_t elementCount = 1;
      for (int i = 0; i < paTensor.dims->size; ++i) {
        elementCount *= static_cast<std::size_t>(paTensor.dims->data[i]);
      }
      return elementCount;
    }
  } // namespace

  TFLiteBackend::~TFLiteBackend() = default;

  EEdgeMLError TFLiteBackend::loadModel(const ModelMetadata &paMetadata,
                                        const std::vector<std::uint8_t> &paModelBinary) {
    if (!paMetadata.isValid()) {
      return EEdgeMLError::kInvalidModelId;
    }
    if (paModelBinary.empty()) {
      return EEdgeMLError::kInvalidInput;
    }

    std::lock_guard lock(mMutex);
    if (mLoadedModels.contains(paMetadata.id)) {
      return EEdgeMLError::kModelAlreadyExists;
    }

    auto binaryCopy = paModelBinary;
    auto model = tflite::FlatBufferModel::BuildFromBuffer(reinterpret_cast<const char *>(binaryCopy.data()),
                                                          binaryCopy.size());
    if (!model) {
      return EEdgeMLError::kInvalidInput;
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model, resolver);
    std::unique_ptr<tflite::Interpreter> interpreter;
    if (kTfLiteOk != builder(&interpreter) || !interpreter) {
      return EEdgeMLError::kBackendUnavailable;
    }

    if (kTfLiteOk != interpreter->AllocateTensors()) {
      return EEdgeMLError::kBackendUnavailable;
    }

    mLoadedModels.emplace(paMetadata.id,
                          ModelInstance{std::move(binaryCopy), std::move(model), std::move(interpreter)});
    return EEdgeMLError::kOk;
  }

  EEdgeMLError TFLiteBackend::unloadModel(const std::string_view paModelId) {
    std::lock_guard lock(mMutex);
    return 0 != mLoadedModels.erase(std::string(paModelId)) ? EEdgeMLError::kOk : EEdgeMLError::kModelNotFound;
  }

  EEdgeMLError TFLiteBackend::infer(const std::string_view paModelId,
                                    const std::span<const float> paInput,
                                    const std::span<float> paOutput,
                                    InferenceStats &paStats) {
    std::lock_guard lock(mMutex);
    const auto loadedModel = mLoadedModels.find(std::string(paModelId));
    if (loadedModel == mLoadedModels.end()) {
      return EEdgeMLError::kModelNotLoaded;
    }

    auto *interpreter = loadedModel->second.interpreter.get();
    if (nullptr == interpreter || interpreter->inputs().empty() || interpreter->outputs().empty()) {
      return EEdgeMLError::kBackendUnavailable;
    }

    const auto inputTensorIndex = interpreter->inputs()[0];
    const auto *inputTensor = interpreter->tensor(inputTensorIndex);
    if (nullptr == inputTensor || kTfLiteFloat32 != inputTensor->type) {
      return EEdgeMLError::kInvalidInput;
    }

    const auto inputElements = tensorElementCount(*inputTensor);
    if (0 == inputElements || paInput.size() != inputElements) {
      return EEdgeMLError::kInvalidInput;
    }

    auto *inputBuffer = interpreter->typed_input_tensor<float>(0);
    if (nullptr == inputBuffer) {
      return EEdgeMLError::kBackendUnavailable;
    }
    std::copy_n(paInput.begin(), inputElements, inputBuffer);

    const auto start = std::chrono::steady_clock::now();
    if (kTfLiteOk != interpreter->Invoke()) {
      return EEdgeMLError::kBackendUnavailable;
    }
    const auto end = std::chrono::steady_clock::now();

    const auto outputTensorIndex = interpreter->outputs()[0];
    const auto *outputTensor = interpreter->tensor(outputTensorIndex);
    if (nullptr == outputTensor || kTfLiteFloat32 != outputTensor->type) {
      return EEdgeMLError::kBackendUnavailable;
    }

    const auto outputElements = tensorElementCount(*outputTensor);
    if (0 == outputElements) {
      return EEdgeMLError::kBackendUnavailable;
    }
    if (paOutput.size() < outputElements) {
      return EEdgeMLError::kOutputTooSmall;
    }

    const auto *outputBuffer = interpreter->typed_output_tensor<float>(0);
    if (nullptr == outputBuffer) {
      return EEdgeMLError::kBackendUnavailable;
    }
    std::copy_n(outputBuffer, outputElements, paOutput.begin());

    paStats.inferenceTimeUs =
        static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    paStats.outputElements = static_cast<std::uint32_t>(outputElements);
    return EEdgeMLError::kOk;
  }

} // namespace forte::eclipse4diac::edgeml
