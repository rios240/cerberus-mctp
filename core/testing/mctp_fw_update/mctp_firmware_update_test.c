// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "platform.h"
#include "testing.h"
#include "cmd_interface/cmd_interface.h"
#include "cmd_interface/cerberus_protocol_master_commands.h"
#include "cmd_interface/cmd_interface_system.h"
#include "mctp/mctp_interface.h"
#include "mctp/mctp_base_protocol.h"
#include "mctp/mctp_control_protocol.h"
#include "mctp/mctp_control_protocol_commands.h"
#include "common/unused.h"
#include "crypto/checksum.h"
#include "testing/mock/cmd_interface/cmd_interface_mock.h"
#include "testing/mock/cmd_interface/cmd_channel_mock.h"


TEST_SUITE_LABEL ("mctp_firmware_update");

/**
 * Length of the MCTP header.
 */
#define	MCTP_HEADER_LENGTH		7

/**
 * Length of an MCTP error message.
 */
#define	MCTP_ERROR_MSG_LENGTH	(MCTP_HEADER_LENGTH + sizeof (struct cerberus_protocol_error) + 1)


/**
 * Dependencies for testing the MCTP interface.
 */
struct mctp_interface_testing {
	struct cmd_channel_mock channel;				/**< Command channel mock instance. */
	struct cmd_interface_mock cmd_cerberus;			/**< Cerberus protocol command interface mock instance. */
	struct cmd_interface_mock cmd_mctp;				/**< MCTP control protocol command interface mock instance. */
	struct device_manager device_mgr;				/**< Device manager. */
	struct mctp_interface mctp;						/**< MCTP interface instance */
};

/**
 * Response callback context.
 */
struct mctp_interface_test_callback_context {
	struct mctp_interface_testing *testing;			/**< Testing instances to utilize. */
	struct cmd_packet *rsp_packet;					/**< Response packet to send back. */
	CuTest *test;									/**< Test framework. */
	int expected_status;							/**< Expected process_packet completion status. */
};


/**
 * Helper function to setup the MCTP interface to use mock instances
 *
 * @param test The test framework.
 * @param mctp The instances to initialize for testing.
 */
static void setup_mctp_interface_with_interface_mock_test (CuTest *test,
	struct mctp_interface_testing *mctp)
{
	struct device_manager_full_capabilities capabilities;
	int status;

	status = cmd_interface_mock_init (&mctp->cmd_cerberus);
	CuAssertIntEquals (test, 0, status);

	status = cmd_interface_mock_init (&mctp->cmd_mctp);
	CuAssertIntEquals (test, 0, status);

	status = device_manager_init (&mctp->device_mgr, 2, DEVICE_MANAGER_AC_ROT_MODE,
		DEVICE_MANAGER_SLAVE_BUS_ROLE);
	CuAssertIntEquals (test, 0, status);

	status = device_manager_update_device_entry (&mctp->device_mgr, 0,
		MCTP_BASE_PROTOCOL_PA_ROT_CTRL_EID, 0x5D);
	CuAssertIntEquals (test, 0, status);

	status = device_manager_update_device_entry (&mctp->device_mgr, 1,
		MCTP_BASE_PROTOCOL_BMC_EID, 0);
	CuAssertIntEquals (test, 0, status);

	device_manager_get_device_capabilities (&mctp->device_mgr, 0, &capabilities);
	capabilities.request.hierarchy_role = DEVICE_MANAGER_PA_ROT_MODE;

	status = device_manager_update_device_capabilities (&mctp->device_mgr, 0, &capabilities);
	CuAssertIntEquals (test, 0, status);

	status = mctp_interface_init (&mctp->mctp, &mctp->cmd_cerberus.base, &mctp->cmd_mctp.base,
		&mctp->device_mgr);
	CuAssertIntEquals (test, 0, status);

	status = cmd_channel_mock_init (&mctp->channel, 0);
	CuAssertIntEquals (test, 0, status);
}

/**
 * Helper function to complete MCTP test
 *
 * @param test The test framework.
 * @param mctp The instances to release.
 */
static void complete_mctp_interface_with_interface_mock_test (CuTest *test,
	struct mctp_interface_testing *mctp)
{
	int status;

	status = cmd_interface_mock_validate_and_release (&mctp->cmd_cerberus);
	CuAssertIntEquals (test, 0, status);

	status = cmd_interface_mock_validate_and_release (&mctp->cmd_mctp);
	CuAssertIntEquals (test, 0, status);

	status = cmd_channel_mock_validate_and_release (&mctp->channel);
	CuAssertIntEquals (test, 0, status);

	device_manager_release (&mctp->device_mgr);
	mctp_interface_deinit (&mctp->mctp);
}

/**
 * Callback function which sends an MCTP response message to process_packet
 *
 * @param expected The expectation that is being used to validate the current call on the mock.
 * @param called The context for the actual call on the mock.
 *
 * @return This function always returns 0
 */
static intptr_t mctp_interface_testing_process_packet_callback (const struct mock_call *expected,
	const struct mock_call *called)
{
	struct mctp_interface_test_callback_context *context = expected->context;
	struct cmd_message *tx;
	int status;

	UNUSED (called);

	status = mctp_interface_process_packet (&context->testing->mctp, context->rsp_packet, &tx);
	CuAssertIntEquals (context->test, context->expected_status, status);

	return 0;
}

/**
 * Helper function that generates an MCTP request and calls issue_request.
 *
 * @param test The test framework.
 * @param mctp The testing instances to utilize.
 * @param context Callback context to utilize.
 * @param issue_request_status Expected issue_request completion status.
 * @param msg_type Message type to use in request.
 */
static void mctp_interface_testing_generate_and_issue_request (CuTest *test,
	struct mctp_interface_testing *mctp, struct mctp_interface_test_callback_context *context, uint8_t *buf,
	int issue_request_status, uint8_t msg_type)
{
 	uint8_t msg_buf[MCTP_BASE_PROTOCOL_MAX_MESSAGE_LEN] = {0};
	struct cmd_packet tx_packet;
	struct mctp_base_protocol_transport_header *header =
		(struct mctp_base_protocol_transport_header*) tx_packet.data;
	int status;

	buf[0] = msg_type;

	memset (&tx_packet, 0, sizeof (tx_packet));

	header->cmd_code = SMBUS_CMD_CODE_MCTP;
	header->byte_count = mctp_protocol_packet_len (sizeof(buf)) - MCTP_BASE_PROTOCOL_SMBUS_OVERHEAD;
	header->source_addr = 0xBB;
	header->rsvd = 0;
	header->header_version = 1;
	header->destination_eid = MCTP_BASE_PROTOCOL_BMC_EID;
	header->source_eid = MCTP_BASE_PROTOCOL_PA_ROT_CTRL_EID;
	header->som = 1;
	header->eom = 1;
	header->tag_owner = MCTP_BASE_PROTOCOL_TO_REQUEST;
	header->msg_tag = 0x00;
	header->packet_seq = 0;

	memcpy (&tx_packet.data[7], buf, sizeof (buf));

	tx_packet.data[7 + sizeof (buf)] = checksum_crc8 (0xAA, tx_packet.data, 7 + sizeof (buf));
	tx_packet.pkt_size = mctp_protocol_packet_len (sizeof(buf));
	tx_packet.state = CMD_VALID_PACKET;
	tx_packet.dest_addr = 0x55;
	tx_packet.timeout_valid = false;

	status = mock_expect (&mctp->channel.mock, mctp->channel.base.send_packet, &mctp->channel, 0,
		MOCK_ARG_VALIDATOR_TMP (cmd_channel_mock_validate_packet, &tx_packet, sizeof (tx_packet)));
	CuAssertIntEquals (test, 0, status);

	status = mock_expect_external_action (&mctp->channel.mock,
		mctp_interface_testing_process_packet_callback, context);
	CuAssertIntEquals (test, 0, status);

	status = mctp_interface_issue_request (&mctp->mctp, &mctp->channel.base, 0x55,
		MCTP_BASE_PROTOCOL_BMC_EID, buf, sizeof (buf), msg_buf, sizeof (msg_buf), msg_type);
	CuAssertIntEquals (test, issue_request_status, status);
}


/*******************
 * Test cases
 *******************/

static void user_agent_test_send_request_update(CuTest *test) {
	struct mctp_interface_testing mctp;
	struct mctp_interface_test_callback_context context;
	struct cmd_packet rx;
	struct mctp_base_protocol_transport_header *header =
		(struct mctp_base_protocol_transport_header*) rx.data;
	uint8_t data[10];
	struct cmd_interface_msg response;
	int status;

	memset (&rx, 0, sizeof (rx));

	header->cmd_code = SMBUS_CMD_CODE_MCTP;
	header->byte_count = mctp_protocol_packet_len (sizeof(data)) - MCTP_BASE_PROTOCOL_SMBUS_OVERHEAD;
	header->source_addr = 0xAB;
	header->rsvd = 0;
	header->header_version = 1;
	header->destination_eid = MCTP_BASE_PROTOCOL_PA_ROT_CTRL_EID;
	header->source_eid = MCTP_BASE_PROTOCOL_BMC_EID;
	header->som = 1;
	header->eom = 1;
	header->tag_owner = MCTP_BASE_PROTOCOL_TO_RESPONSE;
	header->msg_tag = 0;
	header->packet_seq = 0;

	rx.data[7] = MCTP_BASE_PROTOCOL_MSG_TYPE_VENDOR_DEF;
	rx.data[8] = 0x00;
	rx.data[9] = 0x00;
	rx.data[10] = 0x00;
	rx.data[11] = 0x01;
	rx.data[12] = 0x02;
	rx.data[13] = 0x03;
	rx.data[14] = 0x04;
	rx.data[15] = 0x05;
	rx.data[16] = 0x06;
	rx.data[17] = checksum_crc8 (0xBA, rx.data, 17);
	rx.pkt_size = mctp_protocol_packet_len (sizeof(data));
	rx.dest_addr = 0x5D;
	rx.timeout_valid = true;
	platform_init_timeout (10, &rx.pkt_timeout);

	response.data = data;
	response.length = sizeof (data);
	memcpy (response.data, &rx.data[7], response.length);
	response.source_eid = MCTP_BASE_PROTOCOL_BMC_EID;
	response.target_eid = MCTP_BASE_PROTOCOL_PA_ROT_CTRL_EID;
	response.crypto_timeout = false;
	response.channel_id = 0;
	response.max_response = 0;

	TEST_START;

	setup_mctp_interface_with_interface_mock_test (test, &mctp);

	status = mock_expect (&mctp.cmd_cerberus.mock, mctp.cmd_cerberus.base.process_response,
		&mctp.cmd_cerberus, 0,
		MOCK_ARG_VALIDATOR_DEEP_COPY (cmd_interface_mock_validate_request, &response,
			sizeof (response), cmd_interface_mock_save_request, cmd_interface_mock_free_request));
	CuAssertIntEquals (test, 0, status);

	context.expected_status = 0;
	context.rsp_packet = &rx;
	context.test = test;
	context.testing = &mctp;

	mctp_interface_testing_generate_and_issue_request (test, &mctp, &context, 0, pldm,
		MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM);

	complete_mctp_interface_with_interface_mock_test (test, &mctp);
	
}








TEST_SUITE_START (mctp_firmware_update);

TEST (user_agent_test_send_request_update);

TEST_SUITE_END;