/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/core/ota_security.h"

#include <array>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

namespace forte::eclipse4diac::edgeml {
  namespace {
    constexpr std::array<std::uint32_t, 64> cSha256RoundConstants = {
        0x428A2F98U, 0x71374491U, 0xB5C0FBCFU, 0xE9B5DBA5U, 0x3956C25BU, 0x59F111F1U, 0x923F82A4U, 0xAB1C5ED5U,
        0xD807AA98U, 0x12835B01U, 0x243185BEU, 0x550C7DC3U, 0x72BE5D74U, 0x80DEB1FEU, 0x9BDC06A7U, 0xC19BF174U,
        0xE49B69C1U, 0xEFBE4786U, 0x0FC19DC6U, 0x240CA1CCU, 0x2DE92C6FU, 0x4A7484AAU, 0x5CB0A9DCU, 0x76F988DAU,
        0x983E5152U, 0xA831C66DU, 0xB00327C8U, 0xBF597FC7U, 0xC6E00BF3U, 0xD5A79147U, 0x06CA6351U, 0x14292967U,
        0x27B70A85U, 0x2E1B2138U, 0x4D2C6DFCU, 0x53380D13U, 0x650A7354U, 0x766A0ABBU, 0x81C2C92EU, 0x92722C85U,
        0xA2BFE8A1U, 0xA81A664BU, 0xC24B8B70U, 0xC76C51A3U, 0xD192E819U, 0xD6990624U, 0xF40E3585U, 0x106AA070U,
        0x19A4C116U, 0x1E376C08U, 0x2748774CU, 0x34B0BCB5U, 0x391C0CB3U, 0x4ED8AA4AU, 0x5B9CCA4FU, 0x682E6FF3U,
        0x748F82EEU, 0x78A5636FU, 0x84C87814U, 0x8CC70208U, 0x90BEFFFAU, 0xA4506CEBU, 0xBEF9A3F7U, 0xC67178F2U};

    constexpr std::array<std::uint32_t, 8> cSha256InitialState = {
        0x6A09E667U, 0xBB67AE85U, 0x3C6EF372U, 0xA54FF53AU, 0x510E527FU, 0x9B05688CU, 0x1F83D9ABU, 0x5BE0CD19U};

    constexpr std::uint32_t rotateRight(const std::uint32_t paValue, const std::uint32_t paBits) {
      return (paValue >> paBits) | (paValue << (32U - paBits));
    }

    std::vector<std::uint8_t> withSha256Padding(std::span<const std::uint8_t> paData) {
      std::vector<std::uint8_t> message(paData.begin(), paData.end());
      const auto bitLength = static_cast<std::uint64_t>(message.size()) * 8ULL;

      message.push_back(static_cast<std::uint8_t>(0x80U));
      while ((message.size() % 64U) != 56U) {
        message.push_back(static_cast<std::uint8_t>(0x00U));
      }

      for (int index = 7; index >= 0; --index) {
        message.push_back(static_cast<std::uint8_t>((bitLength >> (static_cast<std::uint32_t>(index) * 8U)) & 0xFFU));
      }

      return message;
    }

    std::array<std::uint32_t, 8> computeSha256State(std::span<const std::uint8_t> paData) {
      auto state = cSha256InitialState;
      const auto padded = withSha256Padding(paData);

      std::array<std::uint32_t, 64> schedule{};
      for (std::size_t chunkOffset = 0U; chunkOffset < padded.size(); chunkOffset += 64U) {
        for (std::size_t word = 0U; word < 16U; ++word) {
          const auto byteOffset = chunkOffset + (word * 4U);
          schedule[word] = (static_cast<std::uint32_t>(padded[byteOffset]) << 24U) |
                           (static_cast<std::uint32_t>(padded[byteOffset + 1U]) << 16U) |
                           (static_cast<std::uint32_t>(padded[byteOffset + 2U]) << 8U) |
                           static_cast<std::uint32_t>(padded[byteOffset + 3U]);
        }
        for (std::size_t word = 16U; word < 64U; ++word) {
          const auto sigma0 = rotateRight(schedule[word - 15U], 7U) ^ rotateRight(schedule[word - 15U], 18U) ^
                              (schedule[word - 15U] >> 3U);
          const auto sigma1 = rotateRight(schedule[word - 2U], 17U) ^ rotateRight(schedule[word - 2U], 19U) ^
                              (schedule[word - 2U] >> 10U);
          schedule[word] = schedule[word - 16U] + sigma0 + schedule[word - 7U] + sigma1;
        }

        auto a = state[0];
        auto b = state[1];
        auto c = state[2];
        auto d = state[3];
        auto e = state[4];
        auto f = state[5];
        auto g = state[6];
        auto h = state[7];

        for (std::size_t round = 0U; round < 64U; ++round) {
          const auto sigma1 = rotateRight(e, 6U) ^ rotateRight(e, 11U) ^ rotateRight(e, 25U);
          const auto choice = (e & f) ^ ((~e) & g);
          const auto temp1 = h + sigma1 + choice + cSha256RoundConstants[round] + schedule[round];
          const auto sigma0 = rotateRight(a, 2U) ^ rotateRight(a, 13U) ^ rotateRight(a, 22U);
          const auto majority = (a & b) ^ (a & c) ^ (b & c);
          const auto temp2 = sigma0 + majority;

          h = g;
          g = f;
          f = e;
          e = d + temp1;
          d = c;
          c = b;
          b = a;
          a = temp1 + temp2;
        }

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
      }

      return state;
    }

    std::string stateToHex(const std::array<std::uint32_t, 8> &paState) {
      std::ostringstream stream;
      stream << std::hex << std::setfill('0');
      for (const auto word : paState) {
        stream << std::setw(8) << word;
      }
      return stream.str();
    }
  } // namespace

  std::string OtaSecurity::computeSha256Hex(const std::span<const std::uint8_t> paData) {
    return stateToHex(computeSha256State(paData));
  }

  std::string OtaSecurity::computeSha256Hex(const std::vector<std::uint8_t> &paData) {
    return computeSha256Hex(std::span<const std::uint8_t>(paData.data(), paData.size()));
  }

  std::string OtaSecurity::normalizeHexDigest(std::string_view paValue) {
    constexpr std::string_view cPrefix = "sha256:";
    if (paValue.starts_with(cPrefix)) {
      paValue.remove_prefix(cPrefix.size());
    }

    std::string normalized;
    normalized.reserve(paValue.size());
    for (const char character : paValue) {
      if (std::isspace(static_cast<unsigned char>(character)) != 0) {
        continue;
      }
      if (!std::isxdigit(static_cast<unsigned char>(character))) {
        return "";
      }
      normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
    }

    if (64U != normalized.size()) {
      return "";
    }
    return normalized;
  }

  bool OtaSecurity::constantTimeEqual(const std::string_view paLeft, const std::string_view paRight) {
    if (paLeft.size() != paRight.size()) {
      return false;
    }

    std::uint8_t diff = 0U;
    for (std::size_t i = 0U; i < paLeft.size(); ++i) {
      diff |= static_cast<std::uint8_t>(paLeft[i] ^ paRight[i]);
    }
    return 0U == diff;
  }

  std::string OtaSecurity::deriveSignature(const std::string_view paDigestHex, const std::string_view paTrustAnchor) {
    const auto normalizedDigest = normalizeHexDigest(paDigestHex);
    if (normalizedDigest.empty() || paTrustAnchor.empty()) {
      return "";
    }

    const auto signatureMaterial =
        std::string("edgeml-ota-signature-v1:") + normalizedDigest + ":" + std::string(paTrustAnchor);
    const auto bytes =
        std::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t *>(signatureMaterial.data()),
                                      signatureMaterial.size());
    return computeSha256Hex(bytes);
  }

} // namespace forte::eclipse4diac::edgeml
