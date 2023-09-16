#include <string.h>
#include "platform.h"
#include "testing.h"
#include "common/unused.h"
#include "mctp/mctp_interface.h"
#include "firmware_update.h"
#include "utils.h"
#include "base.h"
#include "logging/logging_buffers.h"



TEST_SUITE_LABEL ("pldm_fw_update");

static void pldm_fw_update_test_place_holder(CuTest *test)
{
    TEST_START;

    CuAssertIntEquals(test, 0, 0);
}

static void pldm_fw_update_test_ua_send_request_update(CuTest *test) {
    uint8_t instanceId = 1;
    uint32_t maxTransferSize = 512;
    uint16_t numOfComp = 3;
    uint8_t maxOutstandingTransferReq = 2;
    uint16_t pkgDataLen = 0x1234;
    const char* compImgSetVerStr = "0penBmcv1.0";
    uint8_t compImgSetVerStrLen = strlen(compImgSetVerStr);
    struct variable_field compImgSetVerStrInfo;
    compImgSetVerStrInfo.ptr = (const uint8_t*)compImgSetVerStr;
    compImgSetVerStrInfo.length = compImgSetVerStrLen;
    int status;

    uint8_t pldmBuf[sizeof(struct pldm_msg_hdr) + sizeof(struct pldm_request_update_req) + compImgSetVerStrLen];
    struct pldm_msg *pldmMsg = (struct pldm_msg*) pldmBuf;

    TEST_START;

    status = encode_request_update_req(
        instanceId, maxTransferSize, numOfComp, maxOutstandingTransferReq,
        pkgDataLen, PLDM_STR_TYPE_ASCII, compImgSetVerStrLen,
        &compImgSetVerStrInfo, pldmMsg,
        sizeof(struct pldm_request_update_req) + compImgSetVerStrLen);

    CuAssertIntEquals(test, PLDM_SUCCESS, status);

    logBuffer(pldmBuf, sizeof (pldmBuf));

}

TEST_SUITE_START (pldm_fw_update);

TEST (pldm_fw_update_test_place_holder);
TEST (pldm_fw_update_test_ua_send_request_update);

TEST_SUITE_END;