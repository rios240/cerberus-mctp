#include "platform.h"
#include "testing.h"
#include "common/unused.h"
#include "mctp/mctp_interface.h"
#include "firmware_update.h"

TEST_SUITE_LABEL ("pldm_fw_update");

static void pldm_fw_update_test_place_holder(CuTest *test)
{
    TEST_START;

    CuAssertIntEquals(test, 0, 0);
}

static void pldm_fw_update_test_ua_send_request_update(CuTest *test) {
    

}

TEST_SUITE_START (pldm_fw_update);

TEST (pldm_fw_update_test_place_holder);
TEST (pldm_fw_update_test_ua_send_request_update);

TEST_SUITE_END;