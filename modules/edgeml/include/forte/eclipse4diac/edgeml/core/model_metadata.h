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

#include <cstddef>
#include <string>

namespace forte::eclipse4diac::edgeml {

  struct ModelMetadata {
    std::string id;
    std::string version;
    std::size_t sizeBytes{0};
    std::string sha256;

    [[nodiscard]] bool isValid() const {
      return !id.empty();
    }
  };

} // namespace forte::eclipse4diac::edgeml
