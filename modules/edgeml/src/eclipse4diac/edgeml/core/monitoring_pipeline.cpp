/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/core/monitoring_pipeline.h"

#include <array>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>

namespace forte::eclipse4diac::edgeml {
  namespace {
    constexpr auto cCsvHeader = "total_inferences,total_errors,total_inference_time_us,max_inference_us,last_error_code\n";

    bool parseUnsigned(const std::string &paToken, std::uint64_t &paValue) {
      if (paToken.empty()) {
        return false;
      }

      std::size_t consumed = 0;
      try {
        const auto parsed = std::stoull(paToken, &consumed, 10);
        if (consumed != paToken.size()) {
          return false;
        }
        paValue = parsed;
        return true;
      } catch (...) {
        return false;
      }
    }

    std::string toCsvLine(const MonitoringPersistentMetrics &paMetrics) {
      std::ostringstream stream;
      stream << paMetrics.totalInferences << ',' << paMetrics.totalErrors << ',' << paMetrics.totalInferenceTimeUs << ','
             << paMetrics.maxInferenceUs << ',' << static_cast<std::uint32_t>(paMetrics.lastErrorCode) << '\n';
      return stream.str();
    }
  } // namespace

  EMonitoringPersistenceStatus MonitoringPipeline::exportCsv(const MonitoringPersistentMetrics &paMetrics,
                                                             const std::string &paPath,
                                                             const bool paAppend,
                                                             std::size_t &paWrittenBytes) {
    paWrittenBytes = 0U;
    if (paPath.empty()) {
      return EMonitoringPersistenceStatus::kInvalidPath;
    }

    bool needsHeader = true;
    if (paAppend) {
      std::ifstream existing(paPath);
      if (existing.good()) {
        existing.peek();
        needsHeader = existing.eof();
      }
    }

    const auto mode = paAppend ? (std::ios::out | std::ios::app) : (std::ios::out | std::ios::trunc);
    std::ofstream output(paPath, mode);
    if (!output.is_open()) {
      return EMonitoringPersistenceStatus::kIoError;
    }

    if (!paAppend || needsHeader) {
      output << cCsvHeader;
      paWrittenBytes += std::char_traits<char>::length(cCsvHeader);
    }

    const auto line = toCsvLine(paMetrics);
    output << line;
    output.flush();
    if (!output.good()) {
      return EMonitoringPersistenceStatus::kIoError;
    }

    paWrittenBytes += line.size();
    return EMonitoringPersistenceStatus::kOk;
  }

  EMonitoringPersistenceStatus MonitoringPipeline::importLatestCsv(const std::string &paPath,
                                                                   MonitoringPersistentMetrics &paMetrics) {
    if (paPath.empty()) {
      return EMonitoringPersistenceStatus::kInvalidPath;
    }

    std::ifstream input(paPath);
    if (!input.is_open()) {
      return EMonitoringPersistenceStatus::kIoError;
    }

    std::string latestDataLine;
    std::string line;
    while (std::getline(input, line)) {
      if (line.empty()) {
        continue;
      }
      if (line.starts_with("total_inferences,")) {
        continue;
      }
      latestDataLine = line;
    }

    if (latestDataLine.empty()) {
      return EMonitoringPersistenceStatus::kParseError;
    }

    std::array<std::string, 5> tokens{};
    std::stringstream parser(latestDataLine);
    for (auto &token : tokens) {
      if (!std::getline(parser, token, ',')) {
        return EMonitoringPersistenceStatus::kParseError;
      }
    }

    std::uint64_t parsedTotalInferences = 0U;
    std::uint64_t parsedTotalErrors = 0U;
    std::uint64_t parsedTotalInferenceTime = 0U;
    std::uint64_t parsedMaxInferenceUs = 0U;
    std::uint64_t parsedLastErrorCode = 0U;

    if (!parseUnsigned(tokens[0], parsedTotalInferences) || !parseUnsigned(tokens[1], parsedTotalErrors) ||
        !parseUnsigned(tokens[2], parsedTotalInferenceTime) || !parseUnsigned(tokens[3], parsedMaxInferenceUs) ||
        !parseUnsigned(tokens[4], parsedLastErrorCode)) {
      return EMonitoringPersistenceStatus::kParseError;
    }

    if (parsedTotalInferences > std::numeric_limits<std::uint32_t>::max() ||
        parsedTotalErrors > std::numeric_limits<std::uint32_t>::max() ||
        parsedMaxInferenceUs > std::numeric_limits<std::uint32_t>::max() ||
        parsedLastErrorCode > std::numeric_limits<std::uint8_t>::max()) {
      return EMonitoringPersistenceStatus::kParseError;
    }

    paMetrics.totalInferences = static_cast<std::uint32_t>(parsedTotalInferences);
    paMetrics.totalErrors = static_cast<std::uint32_t>(parsedTotalErrors);
    paMetrics.totalInferenceTimeUs = parsedTotalInferenceTime;
    paMetrics.maxInferenceUs = static_cast<std::uint32_t>(parsedMaxInferenceUs);
    paMetrics.lastErrorCode = static_cast<std::uint8_t>(parsedLastErrorCode);
    return EMonitoringPersistenceStatus::kOk;
  }

} // namespace forte::eclipse4diac::edgeml
