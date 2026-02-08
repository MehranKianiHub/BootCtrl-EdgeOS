/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/core/model_registry.h"

namespace forte::eclipse4diac::edgeml {

  bool ModelRegistry::registerModel(const ModelMetadata &paMetadata) {
    if (!paMetadata.isValid()) {
      return false;
    }

    const auto [it, inserted] = mModels.emplace(paMetadata.id, paMetadata);
    if (!inserted) {
      it->second = paMetadata;
    }
    return inserted;
  }

  bool ModelRegistry::unregisterModel(const std::string_view paModelId) {
    return 0 != mModels.erase(std::string(paModelId));
  }

  bool ModelRegistry::hasModel(const std::string_view paModelId) const {
    return mModels.contains(std::string(paModelId));
  }

  std::optional<ModelMetadata> ModelRegistry::findModel(const std::string_view paModelId) const {
    const auto iterator = mModels.find(std::string(paModelId));
    if (mModels.end() == iterator) {
      return std::nullopt;
    }
    return iterator->second;
  }

  std::vector<ModelMetadata> ModelRegistry::listModels() const {
    std::vector<ModelMetadata> result;
    result.reserve(mModels.size());
    for (const auto &entry : mModels) {
      result.push_back(entry.second);
    }
    return result;
  }

  std::size_t ModelRegistry::size() const {
    return mModels.size();
  }

  void ModelRegistry::clear() {
    mModels.clear();
  }

} // namespace forte::eclipse4diac::edgeml
