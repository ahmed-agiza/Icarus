#ifndef CRYPTO_H
#define CRYPTO_H

#include <string.h>
#include <stdint.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/md5.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/bio.h>
#include "network_exceptions.h"

//should go in settings file
#define RSA_KEY_LENGTH  2048
#define PUB_EXP         3


#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66

/*#define PRIVFILE  "./RSAPrivateKeyFile.bin"
#define PUBFILE   "./rsaPublicKey_file.bin"*/

#define ERROR_SMALL_BUFFER -1
#define ERROR_INVALID_INPUT -2

class Crypto{
public:
  static int generateKeyPair(const char* privateKeyPath, const char* publicKeyPath);
  static int encrypt(RSA* rsaPublicKey, const char* msg, char* encrypt);
  static int decrypt(RSA* rsaPrivateKey, const char* msg, char* decrypt);
  static int encrypt(char* rsaPublicKey, const char* msg, char* encrypt);
  static int decrypt(char* rsaPrivateKey, const char* msg, char* decrypt);
  static int sign(RSA* rsaPrivateKey, const char* msg, char* encrypt);
  static int unsign(RSA* rsaPublicKey, const char* msg, char* decrypt);
  static int sign(char* rsaPrivateKey, const char* msg, char* encrypt);
  static int unsign(char* rsaPublicKey, const char* msg, char* decrypt);
  static int base64Encode(const void* msg, size_t msgLength, char* encoded, size_t encodedLength);
  static int base64Decode(char *msg, size_t messageLength, unsigned char *decoded, size_t *decodedLength = 0);
  static void generateRandomString(char *buf, int length);
  static size_t base64Len(const char* b64input);
  static int md5Hash(char *msg, char *hash);
  static int md5HashFile(char *filename, char *hash);
  static RSA* getPublicKey(const char* pubKeyFile);
  static RSA* getPrivateKey(const char* priKeyFile);
};

#endif //CRYPTO_H
