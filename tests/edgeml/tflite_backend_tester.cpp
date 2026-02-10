/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "../../4diacFORTE/tests/forte_boost_output_support.h"

#include <boost/test/unit_test.hpp>

#if defined(FORTE_EDGEML_BACKEND_TFLITE)
  #include "forte/eclipse4diac/edgeml/backend/tflite_backend.h"

#include <array>

namespace forte::eclipse4diac::edgeml::test {

  BOOST_AUTO_TEST_SUITE(TFLiteBackendTests)

  BOOST_AUTO_TEST_CASE(inferFailsWhenModelNotLoaded) {
    TFLiteBackend backend;
    std::array<float, 4> input{1.0F, 2.0F, 3.0F, 4.0F};
    std::array<float, 4> output{0.0F, 0.0F, 0.0F, 0.0F};
    InferenceStats stats{};

    const auto status = backend.infer("missing.tflite", input, output, stats);
    BOOST_TEST(EEdgeMLError::kModelNotLoaded == status);
  }

  BOOST_AUTO_TEST_CASE(loadFailsOnEmptyBinary) {
    TFLiteBackend backend;
    const ModelMetadata metadata{"demo.tflite", "1.0.0", 0U, ""};
    const auto status = backend.loadModel(metadata, {});

    BOOST_TEST(EEdgeMLError::kInvalidInput == status);
  }

  BOOST_AUTO_TEST_CASE(unloadMissingModel) {
    TFLiteBackend backend;
    const auto status = backend.unloadModel("unknown");

    BOOST_TEST(EEdgeMLError::kModelNotFound == status);
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test

#endif
