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

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace forte::eclipse4diac::edgeml {

  class OtaSecurity {
    public:
      static std::string computeSha256Hex(std::span<const std::uint8_t> paData);
      static std::string computeSha256Hex(const std::vector<std::uint8_t> &paData);
      static std::string normalizeHexDigest(std::string_view paValue);
      static bool constantTimeEqual(std::string_view paLeft, std::string_view paRight);
      static std::string deriveSignature(std::string_view paDigestHex, std::string_view paTrustAnchor);
  };

} // namespace forte::eclipse4diac::edgeml
