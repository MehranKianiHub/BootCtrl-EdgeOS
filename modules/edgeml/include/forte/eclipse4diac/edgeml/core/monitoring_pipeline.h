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
#include <cstdint>
#include <string>

namespace forte::eclipse4diac::edgeml {

  struct MonitoringPersistentMetrics {
      std::uint64_t totalInferenceTimeUs;
      std::uint32_t totalInferences;
      std::uint32_t totalErrors;
      std::uint32_t maxInferenceUs;
      std::uint8_t lastErrorCode;
  };

  enum class EMonitoringPersistenceStatus {
    kOk,
    kInvalidPath,
    kIoError,
    kParseError,
  };

  class MonitoringPipeline {
    public:
      static EMonitoringPersistenceStatus exportCsv(const MonitoringPersistentMetrics &paMetrics,
                                                    const std::string &paPath,
                                                    bool paAppend,
                                                    std::size_t &paWrittenBytes);

      static EMonitoringPersistenceStatus importLatestCsv(const std::string &paPath,
                                                          MonitoringPersistentMetrics &paMetrics);
  };

} // namespace forte::eclipse4diac::edgeml
