#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "crypto/ecc.h"
#include "crypto/ecc_mbedtls.h"
#include "crypto/aes_mbedtls.h"
#include "mbedtls/ecp.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/error.h"
#include "crypto/rng_mbedtls.h"
#include "crypto/pit_crypto.h"
#include <stdbool.h>
#include "pit/pit.h"
#include <arpa/inet.h>
#include "i2c/pit_i2c.h"


uint8_t *shared_secret;
int shared_length;
struct ecc_private_key priv_key;
struct ecc_public_key pub_key;
uint8_t class_OTPs [128];
int state;

int lock(uint8_t *secret){

  size_t keysize = (256 / 8);

  int key_stat = pit_keygenstate(keysize, &priv_key, &pub_key, &state);
  if(key_stat != 1){
    return PIT_KEY_GEN_FAILURE;
  }

  struct ecc_engine_mbedtls engine;
  ecc_mbedtls_init (&engine);
  struct ecc_public_key pub_key_serv;
  shared_length = engine.base.get_shared_secret_max_length(&engine.base, &priv_key);
  shared_secret = malloc( 8 * shared_length);

  uint8_t *pub_der = NULL;
  size_t der_length;
  engine.base.get_public_key_der (&engine.base, &pub_key, &pub_der, &der_length);

  uint8_t buffer[der_length];
  bzero(buffer, der_length);

  keyexchangestate(pub_der, der_length, buffer);
  
  engine.base.init_public_key(&engine.base, buffer, der_length, &pub_key_serv);
  ecc_mbedtls_release (&engine);
  key_stat = pit_secretkey(&priv_key, &pub_key_serv, secret, &state);

  if(key_stat != 1){
    return PIT_SECRET_KEY_GEN_FAILURE;
  }

  memcpy(shared_secret, secret, shared_length);
  state = 0;
  return SUCESS;
}

int unlock(){
  int my_state;



  int product_id_size = 16; //Move to same spot as PID
  //uint8_t PID[16] = "ABCDEFGHIJKLMNOP";

  uint8_t ePID[16];
  uint8_t ePID_tag[16];
  bool isValidPID = false;

  receive_product_info(ePID, ePID_tag, product_id_size);

  int pid_status = pit_OTPvalidation(shared_secret, shared_length, ePID_tag, ePID, sizeof(ePID), (unsigned char *)PRODUCT_ID, &isValidPID, &my_state);
  printf("Did pid_status work? 0 is no, 1 is yes. %d\n", pid_status);



  //End of changes

  int otp_size = 128;
  uint8_t OTP_tag[16];
  uint8_t OTP[otp_size];
  uint8_t OTPs[otp_size];

  int status = pit_OTPgen(shared_secret, shared_length, OTP_tag, OTP, otp_size, OTPs, &my_state);
  memcpy(class_OTPs, OTPs, otp_size);
  if(status != 1){
    return PIT_OTP_GENERATION_FAILURE;
  }



  //Send OTPs to server
  uint8_t serv_enc[128];
  uint8_t server_tag[16];
  send_unlock_info(OTPs, sizeof(OTPs), OTP_tag, serv_enc, server_tag);



  bool isValid = false;
  pit_OTPvalidation(shared_secret, shared_length, server_tag, serv_enc, sizeof(serv_enc), OTP, &isValid, &my_state);
  printf("Did Final OTP val work? 0 is no, 1 is yes. %d", isValid);
  exit(20);
  if(isValid){
    state = 7; 
    return SUCESS;
  }
  return PIT_UNLOCK_NOT_VALID;

}

int get_state(){
  return state;
}

int get_OTPs(uint8_t *OTPs){
  memcpy(OTPs, class_OTPs, 128);  //Size of OTPs is always 128
  return 1;
}



