#include "crypto.h"
int Crypto::generateKeyPair(const char* privateKeyPath, const char* publicKeyPath) {

    RSA* keyPair = RSA_generate_key(RSA_KEY_LENGTH, PUB_EXP, NULL, NULL);

    //Creation of file keys.
    FILE *rsaPublicKeyFile = fopen(publicKeyPath, "w");
    FILE *rsaPrivateKeyFile = fopen(privateKeyPath, "w");

    if(rsaPublicKeyFile == NULL || rsaPrivateKeyFile == NULL)
        return -1;

    //Writing keyPair generated to files.
    if(PEM_write_RSAPublicKey(rsaPublicKeyFile, keyPair) == 0) {
        fclose(rsaPublicKeyFile);
        return -2;
    }

    fflush(rsaPublicKeyFile);
    fclose(rsaPublicKeyFile);

    if(PEM_write_RSAPrivateKey(rsaPrivateKeyFile, keyPair, NULL, NULL, 0, 0, NULL) == 0) {
        fclose(rsaPrivateKeyFile);
        return -2;
    }

    fflush(rsaPrivateKeyFile);
    fclose(rsaPrivateKeyFile);

    free(keyPair);
    return 0;
}

RSA* Crypto::getPrivateKey(const char* priKeyFile){
    FILE *rsaPrivateKeyFile = fopen(priKeyFile, "r");
    if(rsaPrivateKeyFile == NULL)
        printf("Error reading private file.");

    RSA *rsaPrivateKey = NULL;
    if (PEM_read_RSAPrivateKey(rsaPrivateKeyFile, &rsaPrivateKey, NULL, NULL) == NULL) {
        printf("Error reading private key.");
        ERR_print_errors_fp(stderr);
    } //key read

    fclose(rsaPrivateKeyFile);
    return rsaPrivateKey;
}


RSA* Crypto::getPublicKey(const char* pubKeyFile){
    RSA *rsaPublicKey = NULL;

    FILE *rsaPublicKeyFile = fopen(pubKeyFile,"r");
    if(rsaPublicKeyFile == NULL) printf("Error cannot open file!\n");

    //read public key
    if (PEM_read_RSAPublicKey(rsaPublicKeyFile, &rsaPublicKey, NULL, NULL) == NULL) {
        printf("Error cannot Read file!\n");
        ERR_print_errors_fp(stderr);
    }

    fclose(rsaPublicKeyFile);
    return rsaPublicKey;
}

int Crypto::encrypt(RSA* rsaPublicKey, const char* msg, char* encrypted) {
    int encryptLen;
    char *err = (char *)malloc(130);
    if((encryptLen = RSA_public_encrypt(strlen(msg) + 1, (unsigned char*)msg,
        (unsigned char*)encrypted, rsaPublicKey, RSA_PKCS1_OAEP_PADDING)) == -1) {
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        //fprintf(stderr, "Error encrypting message: %s\n", err);
        return -3;
    }
    free(err);
    return encryptLen;
}

int Crypto::encrypt(char* rsaPublicKey, const char* msg, char* encrypted) {
  RSA *publicKey = NULL;
  BIO *publicKeyBio = BIO_new_mem_buf(rsaPublicKey, -1);
  publicKey = PEM_read_bio_RSAPublicKey(publicKeyBio, &publicKey, NULL, NULL);
  BIO_free(publicKeyBio);
  return encrypt(publicKey, msg, encrypted);
}



int Crypto::decrypt(RSA* rsaPrivateKey, const char* msg, char* decrypted) {
    char *err = (char *)malloc(130);
    if(RSA_private_decrypt(RSA_size(rsaPrivateKey), (unsigned char*)msg, (unsigned char*)decrypted,
        rsaPrivateKey, RSA_PKCS1_OAEP_PADDING) == -1) {
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        //fprintf(stderr, "Error decrypting message: %s\n", err);
    }
    free(err);
    return 0;
}

int Crypto::decrypt(char* rsaPrivateKey, const char* msg, char* decrypted) {
  RSA *privateKey = NULL;
  BIO *privateKeyBio = BIO_new_mem_buf(rsaPrivateKey, -1);
  privateKey = PEM_read_bio_RSAPrivateKey(privateKeyBio, NULL, NULL, NULL);
  BIO_free(privateKeyBio);
  return decrypt(privateKey, msg, decrypted);
}

int Crypto::md5Hash(char *msg, char *hash) {
  unsigned char c[MD5_DIGEST_LENGTH];

  MD5_CTX mdContext;
  int bytes;
  unsigned char data[1024];


  MD5_Init (&mdContext);
  MD5_Update (&mdContext, msg, strlen(msg));
  MD5_Final (c, &mdContext);
  int charCount = 0;
  for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
    charCount = sprintf(hash, "%s%02x", hash, c[i]);
  }

  return charCount;
}

/*int Crypto::md5Hash(char *fileName, char *hash) {
  unsigned char c[MD5_DIGEST_LENGTH];
  int i;
  FILE *inFile = fopen (fileName, "rb");
  MD5_CTX mdContext;
  int bytes;
  unsigned char data[1024];

  if (inFile == NULL) {
   printf ("%s can't be opened.\n", fileName);
   return 0;
  }

  MD5_Init (&mdContext);
  while ((bytes = fread (data, 1, 1024, inFile)) != 0)
    MD5_Update (&mdContext, data, bytes);
  MD5_Final (c,&mdContext);
  for(i = 0; i < MD5_DIGEST_LENGTH; i++) {

    sprintf(hash, "%s%02x", hash, c[i]);
  }
  fclose (inFile);
  return 0;
}*/
