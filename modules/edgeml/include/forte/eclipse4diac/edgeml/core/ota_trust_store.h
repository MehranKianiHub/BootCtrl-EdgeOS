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

#include <string>

namespace forte::eclipse4diac::edgeml {

  enum class EOtaTrustStoreStatus {
    kOk,
    kInvalidPath,
    kInvalidAnchorId,
    kNotFound,
    kIoError,
    kParseError,
  };

  class OtaTrustStore {
    public:
      static EOtaTrustStoreStatus putAnchor(const std::string &paPath, const std::string &paAnchorId,
                                            const std::string &paAnchorMaterial);
      static EOtaTrustStoreStatus removeAnchor(const std::string &paPath, const std::string &paAnchorId);
      static EOtaTrustStoreStatus getAnchor(const std::string &paPath, const std::string &paAnchorId,
                                            std::string &paAnchorMaterial);
  };

} // namespace forte::eclipse4diac::edgeml
