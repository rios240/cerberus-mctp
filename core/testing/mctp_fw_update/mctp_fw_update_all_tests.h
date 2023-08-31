// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef MCTP_FW_UPDATE_ALL_TESTS_H_
#define MCTP_FW_UPDATE_ALL_TESTS_H_

#include "testing.h"
#include "platform_all_tests.h"
#include "common/unused.h"


/**
 * Add all tests for components in the 'mctp' directory.
 *
 * Be sure to keep the test suites in alphabetical order for easier management.
 *
 * @param suite Suite to add the tests to.
 */

static void add_all_mctp_fw_update_test (CuSuite *suite) {
    /* This is unused when no tests will be executed. */
	UNUSED (suite);

    TESTING_RUN_SUITE(mctp_firmware_update);
}

#endif /* MCTP_FW_UPDATE_ALL_TESTS_H_ */