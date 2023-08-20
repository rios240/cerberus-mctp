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
#include "i2c/mctp_i2c.h"
#include "logging/logging_mctp_buffers.h"


TEST_SUITE_LABEL ("pldm_over_mctp_binding");


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



/*******************
 * Test cases
 *******************/

static void mctp_interface_test_issue_request_no_response (CuTest *test)
{
	struct mctp_interface_testing mctp;
 	uint8_t buf[6] = {0};
 	uint8_t msg_buf[MCTP_BASE_PROTOCOL_MAX_MESSAGE_LEN] = {0};
	struct cmd_packet tx_packet;
	struct mctp_base_protocol_transport_header *header;
	int status;

	buf[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_VENDOR_DEF;

	memset (&tx_packet, 0, sizeof (tx_packet));

	header = (struct mctp_base_protocol_transport_header*) tx_packet.data;

	header->cmd_code = SMBUS_CMD_CODE_MCTP;
	header->byte_count = 11;
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

	tx_packet.data[13] = checksum_crc8 (0xAA, tx_packet.data, 13);
	tx_packet.pkt_size = 14;
	tx_packet.state = CMD_VALID_PACKET;
	tx_packet.dest_addr = 0x55;
	tx_packet.timeout_valid = false;

	TEST_START;

	setup_mctp_interface_with_interface_mock_test (test, &mctp);

	status = mock_expect (&mctp.channel.mock, mctp.channel.base.send_packet, &mctp.channel, 0,
		MOCK_ARG_VALIDATOR (cmd_channel_mock_validate_packet, &tx_packet, sizeof (tx_packet)));

	CuAssertIntEquals (test, 0, status);

	status = mctp_interface_issue_request (&mctp.mctp, &mctp.channel.base, 0x55,
		MCTP_BASE_PROTOCOL_BMC_EID, buf, sizeof (buf), msg_buf, sizeof (msg_buf), 1);
	CuAssertIntEquals (test, MCTP_BASE_PROTOCOL_RESPONSE_TIMEOUT, status);

	complete_mctp_interface_with_interface_mock_test (test, &mctp); 



}


static void mctp_interface_test_issue_request_set_state_effecter_states_no_response (CuTest *test)
{
	struct mctp_interface_testing mctp;
 	uint8_t buf[23] = {0};
 	uint8_t msg_buf[MCTP_BASE_PROTOCOL_MAX_MESSAGE_LEN] = {0};
	struct cmd_packet tx_packet;
	struct mctp_base_protocol_transport_header *header;
	int status;

	buf[0] = MCTP_BASE_PROTOCOL_MSG_TYPE_PLDM;
	int bytes_received = socket_receive_pldm_message(buf + 1, sizeof (buf) - 1);
	CuAssertIntEquals (test, 22, bytes_received);
	

	memset (&tx_packet, 0, sizeof (tx_packet));

	header = (struct mctp_base_protocol_transport_header*) tx_packet.data;

	header->cmd_code = SMBUS_CMD_CODE_MCTP;
	header->byte_count = mctp_protocol_packet_len(sizeof (buf)) - MCTP_BASE_PROTOCOL_SMBUS_OVERHEAD;
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
	tx_packet.pkt_size = mctp_protocol_packet_len(sizeof (buf));
	tx_packet.state = CMD_VALID_PACKET;
	tx_packet.dest_addr = 0x55;
	tx_packet.timeout_valid = false;

	TEST_START;

	setup_mctp_interface_with_interface_mock_test (test, &mctp);

	status = mock_expect (&mctp.channel.mock, mctp.channel.base.send_packet, &mctp.channel, 0,
		MOCK_ARG_VALIDATOR (cmd_channel_mock_validate_packet, &tx_packet, sizeof (tx_packet)));

	CuAssertIntEquals (test, 0, status);

	status = mctp_interface_issue_request (&mctp.mctp, &mctp.channel.base, 0x55,
		MCTP_BASE_PROTOCOL_BMC_EID, buf, sizeof (buf), msg_buf, sizeof (msg_buf), 1);
	CuAssertIntEquals (test, MCTP_BASE_PROTOCOL_RESPONSE_TIMEOUT, status);

	//uint8_t packet_buf[sizeof (tx_packet)];
	//memcpy(packet_buf, &tx_packet, sizeof (tx_packet));

	//logBufferToFile(packet_buf, msg_buf, sizeof (packet_buf), sizeof (msg_buf));

	complete_mctp_interface_with_interface_mock_test (test, &mctp);

	status = socket_send_mctp_packet(msg_buf, tx_packet.pkt_size);
	CuAssertIntEquals (test, 0, status);
}


TEST_SUITE_START (pldm_over_mctp_binding);

TEST (mctp_interface_test_issue_request_set_state_effecter_states_no_response);
TEST (mctp_interface_test_issue_request_no_response);

TEST_SUITE_END;