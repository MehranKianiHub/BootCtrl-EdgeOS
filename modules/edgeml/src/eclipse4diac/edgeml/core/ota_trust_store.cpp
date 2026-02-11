/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/core/ota_trust_store.h"

#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

namespace forte::eclipse4diac::edgeml {
  namespace {
    constexpr std::uint32_t cFileSchemaVersion = 1U;

    bool isValidAnchorId(const std::string &paAnchorId) {
      if (paAnchorId.empty() || paAnchorId.size() > 64U) {
        return false;
      }

      for (const auto character : paAnchorId) {
        if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
            (character >= '0' && character <= '9') || '_' == character || '-' == character || '.' == character) {
          continue;
        }
        return false;
      }
      return true;
    }

    bool parseUnsignedHexByte(const std::string &paValue, const std::size_t paOffset, std::uint8_t &paOut) {
      if (paOffset + 2U > paValue.size()) {
        return false;
      }

      const auto nibble = [&](const char paCharacter, std::uint8_t &paNibble) {
        if (paCharacter >= '0' && paCharacter <= '9') {
          paNibble = static_cast<std::uint8_t>(paCharacter - '0');
          return true;
        }
        if (paCharacter >= 'a' && paCharacter <= 'f') {
          paNibble = static_cast<std::uint8_t>(10 + (paCharacter - 'a'));
          return true;
        }
        if (paCharacter >= 'A' && paCharacter <= 'F') {
          paNibble = static_cast<std::uint8_t>(10 + (paCharacter - 'A'));
          return true;
        }
        return false;
      };

      std::uint8_t high = 0U;
      std::uint8_t low = 0U;
      if (!nibble(paValue[paOffset], high) || !nibble(paValue[paOffset + 1U], low)) {
        return false;
      }
      paOut = static_cast<std::uint8_t>((high << 4U) | low);
      return true;
    }

    std::string encodeValue(const std::string &paValue) {
      static constexpr char cHex[] = "0123456789ABCDEF";
      std::string encoded;
      encoded.reserve(paValue.size());
      for (const auto raw : paValue) {
        const auto character = static_cast<unsigned char>(raw);
        if (std::isalnum(character) != 0 || '-' == raw || '_' == raw || '.' == raw) {
          encoded.push_back(raw);
        } else {
          encoded.push_back('%');
          encoded.push_back(cHex[(character >> 4U) & 0x0FU]);
          encoded.push_back(cHex[character & 0x0FU]);
        }
      }
      return encoded;
    }

    bool decodeValue(const std::string &paEncoded, std::string &paDecoded) {
      paDecoded.clear();
      paDecoded.reserve(paEncoded.size());
      for (std::size_t index = 0U; index < paEncoded.size(); ++index) {
        if ('%' != paEncoded[index]) {
          paDecoded.push_back(paEncoded[index]);
          continue;
        }

        std::uint8_t decoded = 0U;
        if (!parseUnsignedHexByte(paEncoded, index + 1U, decoded)) {
          return false;
        }
        paDecoded.push_back(static_cast<char>(decoded));
        index += 2U;
      }
      return true;
    }

    EOtaTrustStoreStatus loadStore(const std::string &paPath, std::map<std::string, std::string> &paAnchors) {
      paAnchors.clear();
      std::ifstream input(paPath);
      if (!input.is_open()) {
        if (std::filesystem::exists(paPath)) {
          return EOtaTrustStoreStatus::kIoError;
        }
        return EOtaTrustStoreStatus::kNotFound;
      }

      std::string line;
      bool hasSchema = false;
      while (std::getline(input, line)) {
        if (line.empty()) {
          continue;
        }

        const auto separator = line.find('=');
        if (std::string::npos == separator || 0U == separator) {
          return EOtaTrustStoreStatus::kParseError;
        }

        const auto key = line.substr(0U, separator);
        const auto value = line.substr(separator + 1U);
        if ("schema_version" == key) {
          std::size_t consumed = 0U;
          try {
            const auto parsedVersion = std::stoul(value, &consumed, 10);
            if (consumed != value.size() || cFileSchemaVersion != parsedVersion) {
              return EOtaTrustStoreStatus::kParseError;
            }
          } catch (...) {
            return EOtaTrustStoreStatus::kParseError;
          }
          hasSchema = true;
          continue;
        }

        if (!key.starts_with("anchor.")) {
          return EOtaTrustStoreStatus::kParseError;
        }

        const auto anchorId = key.substr(std::string("anchor.").size());
        if (!isValidAnchorId(anchorId)) {
          return EOtaTrustStoreStatus::kParseError;
        }

        std::string decoded;
        if (!decodeValue(value, decoded)) {
          return EOtaTrustStoreStatus::kParseError;
        }
        paAnchors[anchorId] = decoded;
      }

      if (!hasSchema) {
        return EOtaTrustStoreStatus::kParseError;
      }
      return EOtaTrustStoreStatus::kOk;
    }

    EOtaTrustStoreStatus saveStore(const std::string &paPath, const std::map<std::string, std::string> &paAnchors) {
      if (paPath.empty()) {
        return EOtaTrustStoreStatus::kInvalidPath;
      }

      const std::filesystem::path destination(paPath);
      const auto parent = destination.parent_path();
      if (!parent.empty()) {
        std::error_code mkdirError;
        std::filesystem::create_directories(parent, mkdirError);
        if (mkdirError) {
          return EOtaTrustStoreStatus::kIoError;
        }
      }

      const auto temporaryPath = destination.string() + ".tmp";
      {
        std::ofstream output(temporaryPath, std::ios::out | std::ios::trunc);
        if (!output.is_open()) {
          return EOtaTrustStoreStatus::kIoError;
        }

        output << "schema_version=" << cFileSchemaVersion << '\n';
        for (const auto &[anchorId, anchorMaterial] : paAnchors) {
          output << "anchor." << anchorId << '=' << encodeValue(anchorMaterial) << '\n';
        }
        output.flush();
        if (!output.good()) {
          return EOtaTrustStoreStatus::kIoError;
        }
      }

      std::error_code removeError;
      std::filesystem::remove(destination, removeError);
      std::error_code renameError;
      std::filesystem::rename(temporaryPath, destination, renameError);
      if (renameError) {
        std::error_code cleanupError;
        std::filesystem::remove(temporaryPath, cleanupError);
        return EOtaTrustStoreStatus::kIoError;
      }
      return EOtaTrustStoreStatus::kOk;
    }
  } // namespace

  EOtaTrustStoreStatus OtaTrustStore::putAnchor(const std::string &paPath, const std::string &paAnchorId,
                                                const std::string &paAnchorMaterial) {
    if (paPath.empty()) {
      return EOtaTrustStoreStatus::kInvalidPath;
    }
    if (!isValidAnchorId(paAnchorId)) {
      return EOtaTrustStoreStatus::kInvalidAnchorId;
    }

    std::map<std::string, std::string> anchors;
    const auto loadStatus = loadStore(paPath, anchors);
    if (EOtaTrustStoreStatus::kOk != loadStatus && EOtaTrustStoreStatus::kNotFound != loadStatus) {
      return loadStatus;
    }

    anchors[paAnchorId] = paAnchorMaterial;
    return saveStore(paPath, anchors);
  }

  EOtaTrustStoreStatus OtaTrustStore::removeAnchor(const std::string &paPath, const std::string &paAnchorId) {
    if (paPath.empty()) {
      return EOtaTrustStoreStatus::kInvalidPath;
    }
    if (!isValidAnchorId(paAnchorId)) {
      return EOtaTrustStoreStatus::kInvalidAnchorId;
    }

    std::map<std::string, std::string> anchors;
    const auto loadStatus = loadStore(paPath, anchors);
    if (EOtaTrustStoreStatus::kNotFound == loadStatus) {
      return EOtaTrustStoreStatus::kNotFound;
    }
    if (EOtaTrustStoreStatus::kOk != loadStatus) {
      return loadStatus;
    }

    if (0U == anchors.erase(paAnchorId)) {
      return EOtaTrustStoreStatus::kNotFound;
    }

    return saveStore(paPath, anchors);
  }

  EOtaTrustStoreStatus OtaTrustStore::getAnchor(const std::string &paPath, const std::string &paAnchorId,
                                                std::string &paAnchorMaterial) {
    paAnchorMaterial.clear();
    if (paPath.empty()) {
      return EOtaTrustStoreStatus::kInvalidPath;
    }
    if (!isValidAnchorId(paAnchorId)) {
      return EOtaTrustStoreStatus::kInvalidAnchorId;
    }

    std::map<std::string, std::string> anchors;
    const auto loadStatus = loadStore(paPath, anchors);
    if (EOtaTrustStoreStatus::kOk != loadStatus) {
      return loadStatus;
    }

    const auto it = anchors.find(paAnchorId);
    if (anchors.end() == it) {
      return EOtaTrustStoreStatus::kNotFound;
    }

    paAnchorMaterial = it->second;
    return EOtaTrustStoreStatus::kOk;
  }

} // namespace forte::eclipse4diac::edgeml
