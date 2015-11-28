#ifndef CRYPTO_H
#define CRYPTO_H

#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

//should go in settings file
#define RSA_KEY_LENGTH  2048
#define PUB_EXP         3

/*#define PRIVFILE  "./rsa_privkey_file.bin"
#define PUBFILE   "./rsa_pubkey_file.bin"*/

class Crypto{
public:
  static int generateKey(const char* privFile, const char* pubFile);
  static int encrypt(RSA* rsa_pubkey,const char* msg, char* encrypt);
  static int decrypt(RSA* rsa_prikey, const char* msg, char* decrypt);
  static RSA* getPubKey(const char* pubKeyFile);
  static RSA* getPriKey(const char* priKeyFile);
};

#endif //CRYPTO_H
