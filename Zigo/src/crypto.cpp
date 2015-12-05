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

size_t Crypto::base64Len(const char* decoded) {
	size_t len = strlen(decoded),
	padding = 0;

	if (decoded[len-1] == '=' && decoded[len-2] == '=') //last two chars are =
	padding = 2;
	else if (decoded[len-1] == '=') //last char is =
	padding = 1;

	return (len*3)/4 - padding;
}
int Crypto::base64Encode(const void* msg, size_t msgLength, char* encoded, size_t encodedLength) {
	static const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const uint8_t *data = (const uint8_t *)msg;
	size_t resultIndex = 0;
	uint32_t dataInt = 0;
	int padCount = msgLength % 3;
	uint8_t firstByte, secondByte, thirdByte, fourthByte;

	for (size_t i = 0; i < msgLength; i += 3) {
		/* these three 8-bit (ASCII) characters become one 24-bit number */
		dataInt = ((uint32_t)data[i]) << 16; //parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0

		if((i + 1) < msgLength)
		dataInt += ((uint32_t)data[i+1]) << 8;//parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0

		if((i + 2) < msgLength)
		dataInt += data[i+2];

		firstByte = (uint8_t)(dataInt >> 18) & 63;
		secondByte = (uint8_t)(dataInt >> 12) & 63;
		thirdByte = (uint8_t)(dataInt >> 6) & 63;
		fourthByte = (uint8_t)dataInt & 63;

		if(resultIndex >= encodedLength)
			return ERROR_SMALL_BUFFER;   /* indicate failure: buffer too small */
		encoded[resultIndex++] = base64chars[firstByte];
    if(resultIndex >= encodedLength)
      return ERROR_SMALL_BUFFER;   /* indicate failure: buffer too small */
		encoded[resultIndex++] = base64chars[secondByte];


		if((i + 1) < msgLength) {
			if(resultIndex >= encodedLength)
        return ERROR_SMALL_BUFFER;   /* indicate failure: buffer too small */
			encoded[resultIndex++] = base64chars[thirdByte];
		}

		/*
		* if we have all three bytes available, then their encoding is spread
		* decoded over four characters
		*/
		if((i + 2) < msgLength) {
			if(resultIndex >= encodedLength)
        return ERROR_SMALL_BUFFER;   /* indicate failure: buffer too small */
			encoded[resultIndex++] = base64chars[fourthByte];
		}
	}

	/*
	* create and add padding that is required if we did not have a multiple of 3
	* number of characters available
	*/
	if (padCount > 0) {
		for (; padCount < 3; padCount++) {
			if(resultIndex >= encodedLength)
        return ERROR_SMALL_BUFFER;   /* indicate failure: buffer too small */
			encoded[resultIndex++] = '=';
		}
	}

	if(resultIndex >= encodedLength)
    return ERROR_SMALL_BUFFER;   /* indicate failure: buffer too small */
	encoded[resultIndex] = 0;
	return 0;   /* indicate success */
}

int Crypto::base64Decode(char *msg, size_t messageLength, unsigned char *decoded, size_t *decodedLength) {
	static const unsigned char ascii[] = {
		66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
		54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
		29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66
	};
	char *end = msg + messageLength;
	char iter = 0;
	size_t buf = 0, len = 0;

	while (msg < end) {
		unsigned char c = ascii[(size_t)(*msg++)];

		switch (c) {
			case WHITESPACE:
        continue;   /* skip whitespace */
			case INVALID:
        return ERROR_INVALID_INPUT;   /* invalid input, return error */
			case EQUALS:                 /* pad character, end of data */
			  msg = end;
			  continue;
			default:
  			buf = buf << 6 | c;
  			iter++; // increment the number of iteration
  			/* If the buffer is full, split it into bytes */
  			if (iter == 4) {
  				if ((len += 3) > *decodedLength)
            return ERROR_SMALL_BUFFER; /* buffer overflow */
  				*(decoded++) = (buf >> 16) & 255;
  				*(decoded++) = (buf >> 8) & 255;
  				*(decoded++) = buf & 255;
  				buf = 0; iter = 0;

  			}
		}
	}

	if (iter == 3) {
		if ((len += 2) > *decodedLength)
      return ERROR_SMALL_BUFFER; /* buffer overflow */
		*(decoded++) = (buf >> 10) & 255;
		*(decoded++) = (buf >> 2) & 255;
	} else if (iter == 2) {
		if (++len > *decodedLength)
      return ERROR_SMALL_BUFFER; /* buffer overflow */
		*(decoded++) = (buf >> 4) & 255;
	}

	*decodedLength = len;
	return 0;
}

void Crypto::generateRandomString(char *buf, int length) {
  static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  if (length) {
    for (int i = 0; i < length; i++) {
      int key = rand() % (int)(sizeof(charset) -1);
      buf[i] = charset[key];
    }
    buf[length] = '\0';
  }
}

int Crypto::md5Hash(char *msg, char *hash) {
  unsigned char c[MD5_DIGEST_LENGTH];

  MD5_CTX mdContext;


  MD5_Init (&mdContext);
  MD5_Update (&mdContext, msg, strlen(msg));
  MD5_Final (c, &mdContext);

  int charCount = 0;

  for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
    charCount = sprintf(hash, "%s%02x", hash, c[i]);
  }

  return charCount;
}
