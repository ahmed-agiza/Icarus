#include "crypto.h"

#define PRIVFILE  "./rsa_privkey_file.bin"
#define PUBFILE   "./rsa_pubkey_file.bin"

int main(){

  /*char hash[128], hash2[128];
  memset(hash, 0, 128);
  memset(hash2, 0, 128);
  Crypto::md5Hash("Hello!", hash);
  printf("%s\n", hash);
  Crypto::md5Hash("Hello!", hash2);
  printf("%s\n", hash2);*/
  //int z = Crypto::generateKeyPair(PRIVFILE, PUBFILE);

  RSA* pub = Crypto::getPublicKey(PUBFILE);
  RSA* pri = Crypto::getPrivateKey(PRIVFILE);

  //char* encrypt = (char*)malloc(RSA_size(pri));
  //char* decrypt = (char *)malloc(RSA_size(pub));
  char encrypt[2048];
  char decrypt[2048];
  memset(encrypt, 0, 2048);
  memset(decrypt, 0, 2048);
  /*int x = Crypto::encrypt(pub, "Helloo-------", encrypt);

  int y = Crypto::decrypt(pri, encrypt, decrypt);

  printf("%d = %d\n", x, strlen(encrypt));

  printf("Encrypted: %s\n", encrypt);
  printf("Decrypted: %s\n", decrypt);*/
  int x = Crypto::sign(pri, "Helloo-------", encrypt);

  int y = Crypto::unsign(pub, encrypt, decrypt);

  printf("%d = %d\n", x, strlen(encrypt));

  printf("Encrypted: %s\n", encrypt);
  printf("Decrypted: %s\n", decrypt);

  return 0;
}
