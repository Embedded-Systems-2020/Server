#include <rsa.h>

int padding = RSA_PKCS1_PADDING;

// char privKey[] = "-----BEGIN RSA PRIVATE KEY-----\n"\
// "MIIEpgIBAAKCAQEA1JmCh5ltimuTMBrw7Zvt0aHZUTCwy8av3Ceo9BmFVbBthA/I\n"\
// "rFGs9LRChb441ucoe7JRC2P6xB3TDtW23ZyVuBK7qAHV2WCA0LpAXWd4EPSiae10\n"\
// "8jJiH/INUHPdn6UguYJWD0ywctLcEOs2gVNQsxcqdLlhCsMO5pjkTlhhJa9bkFzm\n"\
// "CyaybJ3yXidTyteAn4wDADfhWoi9vjbo4cMkrbxQ95cbHpuwjVQjx3nq5w6wvt6x\n"\
// "RyHuzRWFzDvCmkG7R+XD+Fd95nMu09E29wQrulbzNCHeDioqZJscJrCyIWtaWNQW\n"\
// "olycb6BkaVgkN8XsM1vcibS5efdiL5drtzyBEwIDAQABAoIBAQDCJrbHstwQjxFJ\n"\
// "2zCvLblPumjF9pteXUJ44VcwJKZZjNdWxTtT8vrqNjyR+xfZWhzkagINTu7RbxMx\n"\
// "9wYa+EbaZXEuiw6Kh9DbzVu+cSF7Co3QAuTL0is+VUTHdiitBJYQE/EX48c39Uy3\n"\
// "vrZ2AGowQzWohysSMvRGrtXAnoGJcSMnZ8Pn5f/sL+YUK3HqgXChcWXBytMjxPSj\n"\
// "AKfYmyVB8lC3tWspmD/W4FFtU1rSaziSWogei11NjkVd7bzy+JJyCxNPgaGbd88S\n"\
// "Ej13avTINFsapDvhJl9NW76CvIQifmmvHhRTQ9dRBDA9tzc1WBKlfYZUpbk5O1Dh\n"\
// "Omm03i2xAoGBAOqGETGAH6lSUqLnMUDoh4DsCNdfAH4gzXzi3GIsM7v6VaMqhBGl\n"\
// "0LJ4ohV/SQrd448YIRyaQ1gJnL1btzVoTYZp38qb/EPsKAByNN2vyg0qclCh9gK/\n"\
// "JB6S9EHXdWbdBqUygf6lvWTkVco5Lc6HoujtzEqCTbQEhVmSHL+mQOLVAoGBAOgR\n"\
// "ek5F2Iqab6QjlsYY2gHcniE43QdWM38ra5bYuXKNGEoG+w3srX1cmOdTSHW6LrzA\n"\
// "DswYwVl7bzBuCm3dXe2hJmYzMhCgx5Kz+FBfzYzyfWzgljK7ZMvbQRYdDcVHYGvC\n"\
// "wNhx9TB9yR8I6PszzjfrWb6xP2NKmeGNTiQMozhHAoGBANSNP6Xybj+iqdyU1pHy\n"\
// "bWUOB5FSu/qUNkj6vyyGzTiQUO85yErKUpR+OsBrBUNBCU4WW2Q199UhmRoRtsFD\n"\
// "0AeDi11w9DiHH1YQLBPDj8dQ8hxcPcANhmnjXFcUNIZOyggM975YboezXoGSQw7M\n"\
// "5XxgkWfyIwSFmGouv7l+r44lAoGBALmdERf94X6qm1TLalKzmdsXXBryYkb1+WZj\n"\
// "kkAqw9kNZToAb1jpzrGQGHQdNr2C1tL9QliYDD3H7lzsBsds4S2pUQ85L3gBQN7j\n"\
// "t16B4eQ0J4Gn+DpHjA1JJrvLLTJ3LnXCZYA5VYUddnmuJLqW6v/q6/MLzcla581x\n"\
// "ueg3e20NAoGBAMas0vhaFI1Sd87J1gVAWpcWIgkMdG+3kIMqDHgKKc+NvSGKD6lh\n"\
// "eYVTAiBZquQXyWG2bnLg27WouGb61IZ/0bO0hyOxyWO+0nmnHpzWB5RJcTAem3JJ\n"\
// "lGzrdcmjbo7AWn3bgqsl6k1ZbfRxCLBEhnQuK8f6WGkz3G5soVNBc0Zv\n"\
// "-----END RSA PRIVATE KEY-----\n";

RSA * createRSA(unsigned char * key,int public) {
    RSA *rsa= NULL;
    BIO *keybio ;
    keybio = BIO_new_mem_buf(key, -1);
    if (keybio==NULL) {
        printf( "Failed to create key BIO");
        return 0;
    }
    if(public) {
        rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
    }
    else {
        rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa,NULL, NULL);
    }

    if(rsa == NULL) {
        printf( "Failed to create RSA");
    }

    return rsa;
}

/**
 * Encrypt data with public key
 *
 * @param  data      Data to encrypt
 * @param  data_len  Length of data
 * @param  key       Public key
 * @param  encrypted Encryption output
 * @return           0 if successful, -1 in case of error
 */
int public_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted) {
    RSA * rsa = createRSA(key,1);
    int result = RSA_public_encrypt(data_len,data,encrypted,rsa,padding);
    return result;
}

/**
 * Decrypt data with the private key
 *
 * @param  enc_data  Encrypted data
 * @param  data_len  Length of encrypted data (256bits for 2048bits key)
 * @param  key       Private key
 * @param  decrypted Decrypted data
 * @return           0 if successful, -1 in case of error
 */
int private_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted) {
    RSA * rsa = createRSA(key,0);
    int  result = RSA_private_decrypt(data_len,enc_data,decrypted,rsa,padding);
    return result;
}

/**
 * Encrypt data with the private key
 *
 * @param  data      Data to encrypt
 * @param  data_len  Length of data
 * @param  key       Private key
 * @param  encrypted Encrypted data
 * @return           0 if successful, -1 in case of error
 */
int private_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted) {
    RSA * rsa = createRSA(key,0);
    int result = RSA_private_encrypt(data_len,data,encrypted,rsa,padding);
    return result;
}

/**
 * Decrypt data with public key
 *
 * @param  enc_data  Encrypted data
 * @param  data_len  Length of encrypted data
 * @param  key       Public key
 * @param  decrypted Decrypted data
 * @return           0 if successful, -1 in case of error
 */
int public_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted) {

    RSA * rsa = createRSA(key,1);
    int  result = RSA_public_decrypt(data_len,enc_data,decrypted,rsa,padding);
    return result;
}

/**
 * Loads a key from a file to a char array
 * 
 * @param  pFile File path
 * @return       Char array with key
 */
unsigned char * loadKey(char *pFile){
    FILE *keyFD;
    long size;
    unsigned char *keyBuffer;

    keyFD = fopen(pFile, "r");
    if(keyFD == NULL){
        return "1";
    }

    fseek(keyFD, 0L, SEEK_END);
    size = ftell(keyFD);
    rewind(keyFD);

    //privateKey = calloc(1, size+1);
    keyBuffer = (unsigned char *)malloc(size);
    if(!keyBuffer){
        fclose(keyFD);
        return "2";
    }

    //fread(buffer, fileLen, sizeof(unsigned char), file);
    if( 1 != fread(keyBuffer, size, sizeof(unsigned char), keyFD)){
        fclose(keyFD);
        free(keyBuffer);
        return "3";
    }

    fclose(keyFD);
    return keyBuffer;
}
