#!/bin/bash

BUILD_DIR=build


pushd ${BUILD_DIR}/Tests

rm -rf test-results && mkdir -p test-results

# Run the tests with ctest
GTEST_OUTPUT="xml:test-results/" GTEST_COLOR=1 ctest -j"$(nproc)" --verbose #--repeat until-fail:10
# Parse the results and summarize
../../Tests/ci_check_test_results.py --allow ../../Tests/known_failures_linux.txt "test-results/*.xml"

popd
