#include <stdio.h>
#include "steganography.h"

int main(int argc, char const *argv[]) {
  /*int rc = Steganography::encryptImage("user_id/steg", "image.jpg", "cover.jpg", "encrypted.jpg", "123456");
  int rc2 = Steganography::decryptImage("user_id/steg", "user_id/steg/encrypted.jpg", "decrypted.jpg", "123456");
  printf("%d\n%d\n", rc, rc2);*/
  char extraDataBuff[1024];
  char extraDataBuff2[1024];
  memset(extraDataBuff, 0, 1024);
  memset(extraDataBuff2, 0, 1024);
  int rc = Steganography::encryptImage("user_id/steg", "image.jpg", "cover.jpg", "encrypted.jpg", "Some extra data!", "123456");
  int rc2 = Steganography::getImageData("user_id/steg", "user_id/steg/encrypted.jpg", extraDataBuff, sizeof(extraDataBuff), "123456");
  int rc3 = Steganography::updateImageData("user_id/steg", "user_id/steg/encrypted.jpg", "Updated data!", "123456");
  int rc4 = Steganography::getImageData("user_id/steg", "user_id/steg/encrypted.jpg", extraDataBuff2, sizeof(extraDataBuff2), "123456");
  int rc5 = Steganography::decryptImage("user_id/steg", "user_id/steg/encrypted.jpg", "decrypted.jpg", "123456");
  printf("%s\n%s\n%d\n%d\n%d\n%d\n%d\n", extraDataBuff, extraDataBuff2, rc, rc2, rc3, rc4, rc5);
  return 0;
}
