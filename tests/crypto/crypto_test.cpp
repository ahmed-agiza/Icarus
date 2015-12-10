#include <stdio.h>
#include "crypto.h"

int main(int argc, char const *argv[]) {
  //char msg[1024];
  //memset(msg, 0, 1024);
  //Crypto::md5Hash("rsa_pubkey_file.bin", msg);
  //printf("%s\n", msg);
  //Crypto::generateKeyPair("pri.bin", "pub.bin");
  RSA *privateKey = Crypto::getPrivateKey("rsa_privkey_file.bin");
  RSA *publicKey = Crypto::getPublicKey("rsa_pubkey_file.bin");
  char p[5000], pr[5000];
  memset(p, 0, 5000);
  memset(pr, 0, 5000);
  FILE *file = fopen("rsa_pubkey_file.bin", "r");
  if (!file)
    throw -1;
  int s = fread (p, 1, 1024, file);
  printf("%d\n\n", s);
  fclose(file);
  printf("%s\n", p);

  FILE *priFile = fopen("rsa_privkey_file.bin", "r");
  if (!priFile)
    throw -1;
  int sp = fread (pr, 1, 5000, priFile);
  printf("%d\n\n", sp);
  fclose(priFile);
  printf("%s\n", pr);



  //char encrypted[1024];
  //char decrypted[1024];
  char *encrypted = (char *) malloc(RSA_size(publicKey));
  char *decrypted = (char *) malloc(RSA_size(privateKey));
  int x = Crypto::encrypt(p, "Hellooooooooo!", encrypted);
  printf("Encrypted: %s\n%d\n", encrypted, (int)strlen(encrypted));
  // Crypto::decrypt(privateKey, encrypted, decrypted);
  Crypto::decrypt(pr, encrypted, decrypted);
  printf("Decrypted: %s\n%d\n", decrypted, (int)strlen(decrypted));
  free(encrypted);
  free(decrypted);
  return 0;
}
