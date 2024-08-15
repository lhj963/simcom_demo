//
// Created by xf on 2021/7/30.
//

#ifndef TESTMAKEFILE_MAIN_H
#define TESTMAKEFILE_MAIN_H

#define bool int
#define true 1
#define false 0
enum KeyType
{
    EncryptMasterPublicKey,
    SignMasterPublicKey,
    EncryptUserPrivateKey,
    SignUserPrivateKey,
    ChangeMasterPublicKey,
    SM4Key,
    UserPublicKey_UserID
};

size_t padding(unsigned char *src, size_t srclen);

size_t unPadding(unsigned char *src, size_t srclen);

int genRandom();

int getKeys(const char *vinCode);

int handshake();

unsigned char SM3_Hash(unsigned char *text, int len, unsigned char *res);

unsigned char SM4_Decrypt(unsigned char *cipher, int c_len, unsigned char *text);

unsigned char SM4_Encrypt(unsigned char *text, int t_len, unsigned char *cipher);

unsigned char SM9_init();

unsigned char SM9_importKey(char *key, enum KeyType keyType);

unsigned char SM9_sign(unsigned char *message, int message_len, unsigned char *sign);

unsigned char SM9_verifySignature(unsigned char *message, int message_len, unsigned char *sign, unsigned char *res);

unsigned char SM9_decrypt(unsigned char *cipher, int cipher_len, unsigned char *dectext);

unsigned char SM9_encrypt(unsigned char *plaint, int plaint_len, unsigned char *cipher);

#endif //TESTMAKEFILE_MAIN_H
