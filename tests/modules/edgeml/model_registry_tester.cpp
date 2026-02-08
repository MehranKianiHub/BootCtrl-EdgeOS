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

#include <boost/test/unit_test.hpp>
#include <array>

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
    BOOST_TEST(EEdgeMLError::kOk == loadStatus);

    std::array<float, 3> input{1.0F, 2.0F, 3.0F};
    std::array<float, 3> output{0.0F, 0.0F, 0.0F};
    const auto inferStatus = backend.infer("binary_model", input, output, stats);
    BOOST_TEST(EEdgeMLError::kOk == inferStatus);
    BOOST_TEST(output[0] == 1.0F);
    BOOST_TEST(output[1] == 2.0F);
    BOOST_TEST(output[2] == 3.0F);
    BOOST_TEST(stats.outputElements == 3U);

    const auto unloadStatus = backend.unloadModel("binary_model");
    BOOST_TEST(EEdgeMLError::kOk == unloadStatus);
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
