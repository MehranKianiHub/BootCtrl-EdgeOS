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

#include "forte/eclipse4diac/edgeml/core/model_metadata.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace forte::eclipse4diac::edgeml {

  class ModelRegistry {
    public:
      [[nodiscard]] bool registerModel(const ModelMetadata &paMetadata);
      [[nodiscard]] bool unregisterModel(std::string_view paModelId);
      [[nodiscard]] bool hasModel(std::string_view paModelId) const;
      [[nodiscard]] std::optional<ModelMetadata> findModel(std::string_view paModelId) const;
      [[nodiscard]] std::vector<ModelMetadata> listModels() const;
      [[nodiscard]] std::size_t size() const;
      void clear();

    private:
      std::unordered_map<std::string, ModelMetadata> mModels;
  };

} // namespace forte::eclipse4diac::edgeml
