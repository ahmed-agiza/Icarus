#include "crypto.h"
int Crypto::generateKey(const char* privFile, const char* pubFile) {
  RSA* keypair = RSA_generate_key(RSA_KEY_LENGTH, PUB_EXP, NULL, NULL);

  //creation of file keys.
  FILE *rsa_pubkey_file = fopen(pubFile,"w");
  FILE *rsa_privkey_file = fopen(privFile,"w");

  if(rsa_pubkey_file == NULL || rsa_privkey_file == NULL) return -1;

  //writing keypair generated to files.
  if(PEM_write_RSAPublicKey(rsa_pubkey_file, keypair) == 0) {
    fclose(rsa_pubkey_file);
    return -2;
  }

  fflush(rsa_pubkey_file);
  fclose(rsa_pubkey_file);

  if(PEM_write_RSAPrivateKey(rsa_privkey_file, keypair, NULL, NULL, 0, 0,NULL) == 0) {
    fclose(rsa_privkey_file);
    return -2;
  }

  fflush(rsa_privkey_file);
  fclose(rsa_privkey_file);

  free(keypair);
  return 0;
}

RSA* Crypto::getPriKey(const char* priKeyFile){
  FILE *rsa_privkey_file = fopen(priKeyFile,"r");
  if(rsa_privkey_file == NULL) printf("Error reading private file.");

  RSA *rsa_prikey = NULL;
  if (PEM_read_RSAPrivateKey(rsa_privkey_file, &rsa_prikey, NULL, NULL) == NULL) {
      printf("Error reading private key.");
      ERR_print_errors_fp(stderr);
  } //key read

  fclose(rsa_privkey_file);
  return rsa_prikey;
}

RSA* Crypto::getPubKey(const char* pubKeyFile){
  RSA *rsa_pubkey = NULL;

  FILE *rsa_pubkey_file = fopen(pubKeyFile,"r");
  if(rsa_pubkey_file == NULL) printf("Error cannot open file!\n");

  //read public key
  if (PEM_read_RSAPublicKey(rsa_pubkey_file, &rsa_pubkey, NULL, NULL) == NULL) {
      printf("Error cannot Read file !\n");
      ERR_print_errors_fp(stderr);
  }

  fclose(rsa_pubkey_file);
  return rsa_pubkey;
}

int Crypto::encrypt(RSA* rsa_pubkey, const char* msg, char* encrypt) {
  int encrypt_len;
  char *err = (char *)malloc(130);
  if((encrypt_len = RSA_public_encrypt(strlen(msg)+1, (unsigned char*)msg,
  (unsigned char*)encrypt, rsa_pubkey, RSA_PKCS1_OAEP_PADDING)) == -1) {
    ERR_load_crypto_strings();
    ERR_error_string(ERR_get_error(), err);
    //fprintf(stderr, "Error encrypting message: %s\n", err);
    return -3;
  }
  free(err);
  return 0;
}

int Crypto::decrypt(RSA* rsa_prikey, const char* msg, char* decrypt) {
  char *err = (char *)malloc(130);
  if(RSA_private_decrypt(strlen(msg), (unsigned char*)msg, (unsigned char*)decrypt,
  rsa_prikey, RSA_PKCS1_OAEP_PADDING) == -1) {
    ERR_load_crypto_strings();
    ERR_error_string(ERR_get_error(), err);
    //fprintf(stderr, "Error decrypting message: %s\n", err);
  }

  free(err);
  return 0;
}
