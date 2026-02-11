/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/core/ota_persistence.h"

#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <unordered_map>

namespace forte::eclipse4diac::edgeml {
  namespace {
    constexpr std::uint32_t cFileSchemaVersion = 2U;
    constexpr std::uint32_t cLegacySchemaVersion = 1U;

    std::string stringifyBool(const bool paValue) {
      return paValue ? "1" : "0";
    }

    bool parseUnsigned(const std::string &paValue, std::uint64_t &paOut) {
      if (paValue.empty()) {
        return false;
      }

      std::size_t consumed = 0U;
      try {
        const auto parsed = std::stoull(paValue, &consumed, 10);
        if (consumed != paValue.size()) {
          return false;
        }
        paOut = parsed;
        return true;
      } catch (...) {
        return false;
      }
    }

    bool parseBool(const std::string &paValue, bool &paOut) {
      if ("1" == paValue || "true" == paValue) {
        paOut = true;
        return true;
      }
      if ("0" == paValue || "false" == paValue) {
        paOut = false;
        return true;
      }
      return false;
    }

    EOtaPersistenceStatus parseState(const std::unordered_map<std::string, std::string> &paKv,
                                     OtaPersistentState &paState) {
      const auto versionIt = paKv.find("schema_version");
      if (paKv.end() == versionIt) {
        return EOtaPersistenceStatus::kParseError;
      }

      std::uint64_t parsedVersion = 0U;
      if (!parseUnsigned(versionIt->second, parsedVersion) ||
          (cLegacySchemaVersion != parsedVersion && cFileSchemaVersion != parsedVersion)) {
        return EOtaPersistenceStatus::kParseError;
      }

      const auto activeIt = paKv.find("active_model_id");
      const auto previousIt = paKv.find("previous_active_model_id");
      const auto stagedIdIt = paKv.find("staged_model_id");
      const auto stagedVersionIt = paKv.find("staged_version");
      const auto nonceIt = paKv.find("last_applied_nonce");
      const auto rollbackIt = paKv.find("rollback_available");
      const auto stateIt = paKv.find("state");
      const auto expectedSizeIt = paKv.find("expected_size");
      const auto stagedSizeIt = paKv.find("staged_size");
      if (paKv.end() == activeIt || paKv.end() == previousIt || paKv.end() == stagedIdIt || paKv.end() == stagedVersionIt ||
          paKv.end() == rollbackIt || paKv.end() == stateIt || paKv.end() == expectedSizeIt || paKv.end() == stagedSizeIt) {
        return EOtaPersistenceStatus::kParseError;
      }

      if (cFileSchemaVersion == parsedVersion && paKv.end() == nonceIt) {
        return EOtaPersistenceStatus::kParseError;
      }

      bool rollbackAvailable = false;
      if (!parseBool(rollbackIt->second, rollbackAvailable)) {
        return EOtaPersistenceStatus::kParseError;
      }

      std::uint64_t parsedState = 0U;
      std::uint64_t parsedExpectedSize = 0U;
      std::uint64_t parsedStagedSize = 0U;
      if (!parseUnsigned(stateIt->second, parsedState) || !parseUnsigned(expectedSizeIt->second, parsedExpectedSize) ||
          !parseUnsigned(stagedSizeIt->second, parsedStagedSize) || parsedState > 255U ||
          parsedExpectedSize > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()) ||
          parsedStagedSize > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())) {
        return EOtaPersistenceStatus::kParseError;
      }

      paState.activeModelId = activeIt->second;
      paState.previousActiveModelId = previousIt->second;
      paState.stagedModelId = stagedIdIt->second;
      paState.stagedVersion = stagedVersionIt->second;
      paState.lastAppliedNonce = paKv.end() == nonceIt ? "" : nonceIt->second;
      paState.rollbackAvailable = rollbackAvailable;
      paState.state = static_cast<std::uint8_t>(parsedState);
      paState.expectedSize = static_cast<std::uint32_t>(parsedExpectedSize);
      paState.stagedSize = static_cast<std::uint32_t>(parsedStagedSize);
      return EOtaPersistenceStatus::kOk;
    }
  } // namespace

  EOtaPersistenceStatus OtaPersistence::save(const OtaPersistentState &paState, const std::string &paPath) {
    if (paPath.empty()) {
      return EOtaPersistenceStatus::kInvalidPath;
    }

    const std::filesystem::path destination(paPath);
    const auto parent = destination.parent_path();
    if (!parent.empty()) {
      std::error_code mkdirError;
      std::filesystem::create_directories(parent, mkdirError);
      if (mkdirError) {
        return EOtaPersistenceStatus::kIoError;
      }
    }

    const auto temporaryPath = destination.string() + ".tmp";
    {
      std::ofstream output(temporaryPath, std::ios::out | std::ios::trunc);
      if (!output.is_open()) {
        return EOtaPersistenceStatus::kIoError;
      }

      output << "schema_version=" << cFileSchemaVersion << '\n';
      output << "active_model_id=" << paState.activeModelId << '\n';
      output << "previous_active_model_id=" << paState.previousActiveModelId << '\n';
      output << "staged_model_id=" << paState.stagedModelId << '\n';
      output << "staged_version=" << paState.stagedVersion << '\n';
      output << "last_applied_nonce=" << paState.lastAppliedNonce << '\n';
      output << "rollback_available=" << stringifyBool(paState.rollbackAvailable) << '\n';
      output << "state=" << static_cast<std::uint32_t>(paState.state) << '\n';
      output << "expected_size=" << paState.expectedSize << '\n';
      output << "staged_size=" << paState.stagedSize << '\n';
      output.flush();
      if (!output.good()) {
        return EOtaPersistenceStatus::kIoError;
      }
    }

    std::error_code removeError;
    std::filesystem::remove(destination, removeError);
    std::error_code renameError;
    std::filesystem::rename(temporaryPath, destination, renameError);
    if (renameError) {
      std::error_code cleanupError;
      std::filesystem::remove(temporaryPath, cleanupError);
      return EOtaPersistenceStatus::kIoError;
    }
    return EOtaPersistenceStatus::kOk;
  }

  EOtaPersistenceStatus OtaPersistence::load(const std::string &paPath, OtaPersistentState &paState) {
    if (paPath.empty()) {
      return EOtaPersistenceStatus::kInvalidPath;
    }

    std::ifstream input(paPath);
    if (!input.is_open()) {
      if (std::filesystem::exists(paPath)) {
        return EOtaPersistenceStatus::kIoError;
      }
      return EOtaPersistenceStatus::kNotFound;
    }

    std::unordered_map<std::string, std::string> kv;
    std::string line;
    while (std::getline(input, line)) {
      if (line.empty()) {
        continue;
      }

      const auto separator = line.find('=');
      if (std::string::npos == separator || 0U == separator) {
        return EOtaPersistenceStatus::kParseError;
      }

      const auto key = line.substr(0U, separator);
      const auto value = line.substr(separator + 1U);
      kv[key] = value;
    }

    return parseState(kv, paState);
  }

} // namespace forte::eclipse4diac::edgeml
