/*******************************************************************************
 * Copyright (c) 2026 BootCtrl
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *******************************************************************************/

#include "../../4diacFORTE/tests/core/fbtests/fbtestfixture.h"
#include "../../4diacFORTE/tests/forte_boost_output_support.h"
#include "forte/datatypes/forte_bool.h"
#include "forte/datatypes/forte_string.h"
#include "forte/datatypes/forte_udint.h"
#include "forte/datatypes/forte_usint.h"

#include <boost/test/tools/floating_point_comparison.hpp>
#include <cstdio>
#include <string>

using namespace forte::literals;

namespace forte::eclipse4diac::edgeml::test {

  struct ML_Monitoring_TestFixture : public forte::test::CFBTestFixtureBase {

      ML_Monitoring_TestFixture() : CFBTestFixtureBase("eclipse4diac::edgeml::ML_Monitoring"_STRID) {
        setInputData({&mCommand, &mInferenceUs, &mHasError, &mInErrorCode, &mPersistPath, &mAppend});
        setOutputData({&mTotalInferences, &mTotalErrors, &mAvgInferenceUs, &mMaxInferenceUs, &mLastErrorCode,
                       &mHealthScore, &mSuccess, &mError, &mErrorCode, &mPersisted, &mPersistBytes});
        setup();
      }

      CIEC_USINT mCommand;
      CIEC_UDINT mInferenceUs;
      CIEC_BOOL mHasError;
      CIEC_USINT mInErrorCode;
      CIEC_STRING mPersistPath;
      CIEC_BOOL mAppend;

      CIEC_UDINT mTotalInferences;
      CIEC_UDINT mTotalErrors;
      CIEC_UDINT mAvgInferenceUs;
      CIEC_UDINT mMaxInferenceUs;
      CIEC_USINT mLastErrorCode;
      CIEC_REAL mHealthScore;
      CIEC_BOOL mSuccess;
      CIEC_BOOL mError;
      CIEC_USINT mErrorCode;
      CIEC_BOOL mPersisted;
      CIEC_UDINT mPersistBytes;

      void sendReport(const CIEC_UDINT::TValueType paInferenceUs,
                      const bool paHasError,
                      const CIEC_USINT::TValueType paErrorCode = 0U) {
        mCommand = CIEC_USINT(0U);
        mInferenceUs = CIEC_UDINT(paInferenceUs);
        mHasError = CIEC_BOOL(paHasError);
        mInErrorCode = CIEC_USINT(paErrorCode);
        mPersistPath = CIEC_STRING(std::string(""));
        mAppend = CIEC_BOOL(true);
        triggerEvent(0);
        BOOST_TEST(checkForSingleOutputEventOccurence(0));
      }
  };

  BOOST_FIXTURE_TEST_SUITE(ML_MonitoringTests, ML_Monitoring_TestFixture)

  BOOST_AUTO_TEST_CASE(reportAndSnapshot) {
    sendReport(100U, false);
    BOOST_TEST(1U == static_cast<CIEC_UDINT::TValueType>(mTotalInferences));
    BOOST_TEST(0U == static_cast<CIEC_UDINT::TValueType>(mTotalErrors));
    BOOST_TEST(100U == static_cast<CIEC_UDINT::TValueType>(mAvgInferenceUs));
    BOOST_TEST(100U == static_cast<CIEC_UDINT::TValueType>(mMaxInferenceUs));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mHealthScore) == 1.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mPersisted));

    sendReport(300U, true, 7U);
    BOOST_TEST(2U == static_cast<CIEC_UDINT::TValueType>(mTotalInferences));
    BOOST_TEST(1U == static_cast<CIEC_UDINT::TValueType>(mTotalErrors));
    BOOST_TEST(200U == static_cast<CIEC_UDINT::TValueType>(mAvgInferenceUs));
    BOOST_TEST(300U == static_cast<CIEC_UDINT::TValueType>(mMaxInferenceUs));
    BOOST_TEST(7U == static_cast<CIEC_USINT::TValueType>(mLastErrorCode));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mHealthScore) == 0.5F, boost::test_tools::tolerance(0.0001F));

    mCommand = CIEC_USINT(1U);
    mInferenceUs = CIEC_UDINT(0U);
    mHasError = CIEC_BOOL(false);
    mInErrorCode = CIEC_USINT(0U);
    mPersistPath = CIEC_STRING(std::string(""));
    mAppend = CIEC_BOOL(true);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(2U == static_cast<CIEC_UDINT::TValueType>(mTotalInferences));
    BOOST_TEST(1U == static_cast<CIEC_UDINT::TValueType>(mTotalErrors));
  }

  BOOST_AUTO_TEST_CASE(resetClearsMetrics) {
    sendReport(123U, true, 3U);

    mCommand = CIEC_USINT(2U);
    mInferenceUs = CIEC_UDINT(0U);
    mHasError = CIEC_BOOL(false);
    mInErrorCode = CIEC_USINT(0U);
    mPersistPath = CIEC_STRING(std::string(""));
    mAppend = CIEC_BOOL(true);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(0U == static_cast<CIEC_UDINT::TValueType>(mTotalInferences));
    BOOST_TEST(0U == static_cast<CIEC_UDINT::TValueType>(mTotalErrors));
    BOOST_TEST(0U == static_cast<CIEC_UDINT::TValueType>(mAvgInferenceUs));
    BOOST_TEST(0U == static_cast<CIEC_UDINT::TValueType>(mMaxInferenceUs));
    BOOST_TEST(0U == static_cast<CIEC_USINT::TValueType>(mLastErrorCode));
    BOOST_TEST(static_cast<CIEC_REAL::TValueType>(mHealthScore) == 1.0F, boost::test_tools::tolerance(0.0001F));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mError));
  }

  BOOST_AUTO_TEST_CASE(exportAndImportRoundTrip) {
    const std::string path = "./ML_Monitoring_roundtrip.temp.csv";
    std::remove(path.c_str());

    sendReport(100U, false);
    sendReport(300U, true, 7U);

    mCommand = CIEC_USINT(3U);
    mInferenceUs = CIEC_UDINT(0U);
    mHasError = CIEC_BOOL(false);
    mInErrorCode = CIEC_USINT(0U);
    mPersistPath = CIEC_STRING(path);
    mAppend = CIEC_BOOL(false);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mPersisted));
    BOOST_TEST(static_cast<CIEC_UDINT::TValueType>(mPersistBytes) > 0U);

    mCommand = CIEC_USINT(2U);
    mPersistPath = CIEC_STRING(std::string(""));
    mAppend = CIEC_BOOL(true);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(0U == static_cast<CIEC_UDINT::TValueType>(mTotalInferences));

    mCommand = CIEC_USINT(4U);
    mPersistPath = CIEC_STRING(path);
    mAppend = CIEC_BOOL(true);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mPersisted));
    BOOST_TEST(2U == static_cast<CIEC_UDINT::TValueType>(mTotalInferences));
    BOOST_TEST(1U == static_cast<CIEC_UDINT::TValueType>(mTotalErrors));
    BOOST_TEST(200U == static_cast<CIEC_UDINT::TValueType>(mAvgInferenceUs));
    BOOST_TEST(300U == static_cast<CIEC_UDINT::TValueType>(mMaxInferenceUs));
    BOOST_TEST(7U == static_cast<CIEC_USINT::TValueType>(mLastErrorCode));

    std::remove(path.c_str());
  }

  BOOST_AUTO_TEST_CASE(importReadsLatestAppendedSnapshot) {
    const std::string path = "./ML_Monitoring_append.temp.csv";
    std::remove(path.c_str());

    sendReport(100U, false);

    mCommand = CIEC_USINT(3U);
    mInferenceUs = CIEC_UDINT(0U);
    mHasError = CIEC_BOOL(false);
    mInErrorCode = CIEC_USINT(0U);
    mPersistPath = CIEC_STRING(path);
    mAppend = CIEC_BOOL(false);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mPersisted));

    sendReport(200U, true, 2U);

    mCommand = CIEC_USINT(3U);
    mInferenceUs = CIEC_UDINT(0U);
    mHasError = CIEC_BOOL(false);
    mInErrorCode = CIEC_USINT(0U);
    mPersistPath = CIEC_STRING(path);
    mAppend = CIEC_BOOL(true);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mPersisted));

    mCommand = CIEC_USINT(2U);
    mPersistPath = CIEC_STRING(std::string(""));
    mAppend = CIEC_BOOL(true);
    triggerEvent(0);
    BOOST_TEST(checkForSingleOutputEventOccurence(0));

    mCommand = CIEC_USINT(4U);
    mPersistPath = CIEC_STRING(path);
    mAppend = CIEC_BOOL(true);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mPersisted));
    BOOST_TEST(2U == static_cast<CIEC_UDINT::TValueType>(mTotalInferences));
    BOOST_TEST(1U == static_cast<CIEC_UDINT::TValueType>(mTotalErrors));
    BOOST_TEST(150U == static_cast<CIEC_UDINT::TValueType>(mAvgInferenceUs));
    BOOST_TEST(200U == static_cast<CIEC_UDINT::TValueType>(mMaxInferenceUs));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mLastErrorCode));

    std::remove(path.c_str());
  }

  BOOST_AUTO_TEST_CASE(invalidPathOnExport) {
    mCommand = CIEC_USINT(3U);
    mInferenceUs = CIEC_UDINT(0U);
    mHasError = CIEC_BOOL(false);
    mInErrorCode = CIEC_USINT(0U);
    mPersistPath = CIEC_STRING(std::string(""));
    mAppend = CIEC_BOOL(true);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(2U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(missingFileOnImport) {
    const std::string path = "./ML_Monitoring_missing.temp.csv";
    std::remove(path.c_str());

    mCommand = CIEC_USINT(4U);
    mInferenceUs = CIEC_UDINT(0U);
    mHasError = CIEC_BOOL(false);
    mInErrorCode = CIEC_USINT(0U);
    mPersistPath = CIEC_STRING(path);
    mAppend = CIEC_BOOL(true);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(3U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_CASE(invalidCommand) {
    mCommand = CIEC_USINT(99U);
    mInferenceUs = CIEC_UDINT(1U);
    mHasError = CIEC_BOOL(false);
    mInErrorCode = CIEC_USINT(0U);
    mPersistPath = CIEC_STRING(std::string(""));
    mAppend = CIEC_BOOL(true);
    triggerEvent(0);

    BOOST_TEST(checkForSingleOutputEventOccurence(0));
    BOOST_TEST(!static_cast<CIEC_BOOL::TValueType>(mSuccess));
    BOOST_TEST(static_cast<CIEC_BOOL::TValueType>(mError));
    BOOST_TEST(1U == static_cast<CIEC_USINT::TValueType>(mErrorCode));
  }

  BOOST_AUTO_TEST_SUITE_END()

} // namespace forte::eclipse4diac::edgeml::test
