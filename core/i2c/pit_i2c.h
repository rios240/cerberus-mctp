#include "mbedtls/ecdh.h"
#include "crypto/ecc_mbedtls.h"

/**
* Initiate a connection to desired server. Dependent on implementation (Socket vs i2c)
* @param desired_port The desired port to connect to the server on (Need not implement if using i2c)
* @return A file descriptor pointing to the socket (server implementation)
*/

int pit_connect(int desired_port);

/**
 * On success, keyexchangestate should initialize pubkey_serv with the server's public key
 * @param pubkey_cli The initialized public key for the client (Machine using cerberus)
 * @param pubkey_serv An uninitialized public key to store the server's public key into
 * @return 1 on success
*/
int keyexchangestate(uint8_t *pubkey_cli, size_t pubkey_der_length, uint8_t *pubkey_serv);


/**
 * Sends OTPs, AES IV, and the AES-GCM Tag for OTP encryption to the server, receives the server's encrypted message and tag for that message back
 * @param OTPs The Encrypted OTP to send
 * @param OTPs_size Size (in bytes) of the OTPs
 * @param unlock_aes_iv The AES IV used to encrypt the OTP into OTPs
 * @param unlock_aes_iv_size Size (in bytes) of the unlock_aes_iv param
 * @param OTP_tag The AES-GCM Tag generated when encrypting OTP into OTPs
 * @param server_encrypted_message An empty buffer to hold the server's response message (which will then be validated in the unlock API)
 * @param server_tag The AES-GCM tag for the server's encrypted message
 * @return 1 on success
*/
int send_unlock_info(uint8_t *OTPs, size_t OTPs_size, uint8_t *unlock_aes_iv, size_t unlock_aes_iv_size, uint8_t *OTP_tag, uint8_t *server_encrypted_message, uint8_t *server_tag);