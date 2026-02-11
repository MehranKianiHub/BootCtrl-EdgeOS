/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "forte/eclipse4diac/edgeml/backend/null_backend.h"
#include "forte/eclipse4diac/edgeml/core/model_registry.h"
#include "forte/eclipse4diac/edgeml/core/runtime_context.h"

#include <boost/test/unit_test.hpp>
#include <atomic>
#include <array>
#include <string>
#include <thread>
#include <vector>

namespace forte::eclipse4diac::edgeml::test {

  BOOST_AUTO_TEST_SUITE(ModelRegistryTests)

  BOOST_AUTO_TEST_CASE(registerAndFindModel) {
    ModelRegistry registry;
    ModelMetadata metadata{"motor_health", "1.0.0", 1024, "abc123"};

    BOOST_TEST(registry.registerModel(metadata));
    BOOST_TEST(registry.hasModel("motor_health"));
    BOOST_TEST(registry.size() == 1U);

    const auto entry = registry.findModel("motor_health");
    BOOST_REQUIRE(entry.has_value());
    BOOST_TEST(entry->version == "1.0.0");
    BOOST_TEST(entry->sizeBytes == 1024U);
  }

  BOOST_AUTO_TEST_CASE(updateExistingModelKeepsSingleEntry) {
    ModelRegistry registry;

    BOOST_TEST(registry.registerModel({"classifier", "1.0.0", 100, "hashA"}));
    BOOST_TEST(!registry.registerModel({"classifier", "1.0.1", 120, "hashB"}));

    const auto entry = registry.findModel("classifier");
    BOOST_REQUIRE(entry.has_value());
    BOOST_TEST(entry->version == "1.0.1");
    BOOST_TEST(registry.size() == 1U);
  }

  BOOST_AUTO_TEST_CASE(nullBackendLoadInferAndUnload) {
    NullBackend backend;
    InferenceStats stats{};

    const auto loadStatus = backend.loadModel({"binary_model", "1.0.0", 3, "hash"}, {1U, 2U, 3U});
    BOOST_TEST(static_cast<int>(EEdgeMLError::kOk) == static_cast<int>(loadStatus));

    std::array<float, 3> input{1.0F, 2.0F, 3.0F};
    std::array<float, 3> output{0.0F, 0.0F, 0.0F};
    const auto inferStatus = backend.infer("binary_model", input, output, stats);
    BOOST_TEST(static_cast<int>(EEdgeMLError::kOk) == static_cast<int>(inferStatus));
    BOOST_TEST(output[0] == 1.0F);
    BOOST_TEST(output[1] == 2.0F);
    BOOST_TEST(output[2] == 3.0F);
    BOOST_TEST(stats.outputElements == 3U);

    const auto unloadStatus = backend.unloadModel("binary_model");
    BOOST_TEST(static_cast<int>(EEdgeMLError::kOk) == static_cast<int>(unloadStatus));
  }

  BOOST_AUTO_TEST_CASE(modelRegistryConcurrentAccess) {
    ModelRegistry registry;
    constexpr int workerCount = 8;
    constexpr int iterationsPerWorker = 200;
    std::vector<std::thread> workers;
    workers.reserve(workerCount);

    for (int worker = 0; worker < workerCount; ++worker) {
      workers.emplace_back([&registry, worker]() {
        for (int i = 0; i < iterationsPerWorker; ++i) {
          const std::string modelId = "registry.concurrent." + std::to_string(worker) + "." + std::to_string(i % 16);
          const ModelMetadata metadata{modelId, "1.0.0", 1U, "hash"};
          [[maybe_unused]] const auto inserted = registry.registerModel(metadata);
          [[maybe_unused]] const auto exists = registry.hasModel(modelId);
          [[maybe_unused]] const auto model = registry.findModel(modelId);
          if (0 == (i % 3)) {
            [[maybe_unused]] const auto removed = registry.unregisterModel(modelId);
          }
          [[maybe_unused]] const auto currentSize = registry.size();
        }
      });
    }

    for (auto &worker : workers) {
      worker.join();
    }

    BOOST_TEST(registry.size() <= static_cast<std::size_t>(workerCount * 16));
  }

  BOOST_AUTO_TEST_CASE(edgeMLRuntimeConcurrentLoadInferUnload) {
    auto &runtime = EdgeMLRuntime::instance();
    constexpr int workerCount = 4;
    constexpr int iterationsPerWorker = 80;
    std::atomic_bool sawUnexpectedFailure{false};
    std::vector<std::thread> workers;
    workers.reserve(workerCount);

    for (int worker = 0; worker < workerCount; ++worker) {
      workers.emplace_back([&runtime, &sawUnexpectedFailure, worker]() {
        for (int i = 0; i < iterationsPerWorker; ++i) {
          const std::string modelId = "mock.runtime.concurrent." + std::to_string(worker) + "." + std::to_string(i);
          const ModelMetadata metadata{modelId, "1.0.0", 1U, "hash"};
          const auto loadStatus = runtime.loadModel(metadata, {0x01});
          if (EEdgeMLError::kOk != loadStatus && EEdgeMLError::kModelAlreadyExists != loadStatus) {
            sawUnexpectedFailure = true;
            continue;
          }

          const std::array<float, 4> input{1.0F, 2.0F, 3.0F, 4.0F};
          std::array<float, 4> output{0.0F, 0.0F, 0.0F, 0.0F};
          InferenceStats stats{};
          const auto inferStatus = runtime.inferModel(modelId, input, output, stats);
          if (EEdgeMLError::kOk != inferStatus) {
            sawUnexpectedFailure = true;
          }

          const auto unloadStatus = runtime.unloadModel(modelId);
          if (EEdgeMLError::kOk != unloadStatus) {
            sawUnexpectedFailure = true;
          }
        }
      });
    }

    for (auto &worker : workers) {
      worker.join();
    }

    BOOST_TEST(!sawUnexpectedFailure.load());
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
