#ifndef __DX83_API_H__
#define __DX83_API_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------
//  Error Code definition
//--------------------------------------------------------------
#define   COMMAND_ERROR         0x01
#define   DATA_LEN_ERROR        0x02
#define   PARAMETER_EEROR       0x03
#define   CRCD_ERROR            0x04
#define   CRCA_ERROR            0x05
#define   CONFIG_HAVE_SET       0x06
#define   COUNTER_HAVE_SET      0x07
#define   COUNTER_IS_ZERO       0x08
#define   FUSE_IS_LOCKED        0x09
#define   SLA_OVER_TIMES        0x0a
#define   SN_NOT_INIT           0x0b
#define   ZONEKEY_NOT_INIT      0x0c
#define   SRVPBK_NOT_INIT       0x0d
#define   KPA_NOT_INIT          0x0e
#define   SYMKEY_NOT_INIT       0x0f
#define   NOT_SWAP_NONCE        0x10
#define   NOT_VERIFY_PIN        0x11
#define   NOT_VERIFY_ZONE       0x12
#define   PIN_ERROR             0x13
#define   ZONEKEY_ERROR         0x14
#define   GEN_KPA_ERROR         0x15
#define   SIGNATURE_ERROR       0x16
#define   ECDH_ERROR            0x17
#define   SM2_KEYAGREE_ERROR    0x18
#define   PUBLIC_ENCRYPT_ERROR  0x19
#define   PRIVATE_DECRYPT_ERROR 0x1a
#define   NOT_SUPPORT           0x1b

#define   SM9KEY_NOT_INIT       0x1c
#define   SM9_GEN_USER_ERROR    0x1d
#define   SM9_SIGNATURE_ERROR   0x1e
#define   SM9_KEYENC_ERROR      0x1f
#define   SM9_KEYDEC_ERROR      0x20
#define   SM9_ENCRYPT_ERROR     0x21
#define   SM9_DECRYPT_ERROR     0x22
#define   SM9_GEN_TMPKEY_ERROR  0x23
#define   SM9_KEYAGREE_ERROR    0x24

#define   PACKET_LEN_ERROR      0xa0
#define   I2C_BUS_ERROR         0xa1
#define   CMD_NOT_SUPPORT       0xa2
#define   I2C_STS_CRC_ERROR     0xa3
#define   RESPONSE_LEN_ERROR    0xa4
#define   POLLING_OVERTIME      0xa5

#define   I2C_DATA_CRCA_ERROR   0xb0
#define   I2C_DATA_CRCD_ERROR   0xb1
#define   VERIFY_KEY_ERROR      0xb2
#define   ENCRYPT_LEN_ERROR     0xb3
//--------------------------------------------------------------
//  Algorithm definition
//--------------------------------------------------------------
#define ALG0_ECC256R1    0
#define ALG0_ECC256K1    1
#define ALG0_SM2         2

#define ALG1_SHA256      0x00
#define ALG1_SM3         0x01
#define ALG1_HMAC_SHA256 0x10
#define ALG1_HMAC_SM3    0x11

#define ALG2_AES128_ECB  0x00
#define ALG2_AES128_CBC  0x01
#define ALG2_AES128_CFB  0x02
#define ALG2_AES128_OFB  0x03
#define ALG2_AES128_CTR  0x04

#define ALG2_AES256_ECB  0x10
#define ALG2_AES256_CBC  0x11
#define ALG2_AES256_CFB  0x12
#define ALG2_AES256_OFB  0x13
#define ALG2_AES256_CTR  0x14

#define ALG2_SM4_ECB     0x20
#define ALG2_SM4_CBC     0x21
#define ALG2_SM4_CFB     0x22
#define ALG2_SM4_OFB     0x23
#define ALG2_SM4_CTR     0x24

#define ALG3_SHA256      0x00
#define ALG3_SM3         0x01
#define ALG3_SHAHD       0x02
#define ALG3_HMAC_SHA256 0x03
#define ALG3_HMAC_SM3    0x04
#define ALG3_HMAC_SHAHD  0x05

#define HKDF_SHA256      0
#define HKDF_SM3         1
//--------------------------------------------------------------
//  KID definition
//  Using: KID_PBK_SRV+x, x is the group number
//--------------------------------------------------------------
#define KID_PIN          0x10   // 0x10:      1  group pin
#define KID_PBK_SRV      0x20   // 0x20-0x28: 9  groups server's public keys, 0x24 is ram temp public key
#define KID_PBK_KPAIR    0x30   // 0x30-0x39: 10 groups kpair's public keys, 0x38/0x39 is ram temp public key
#define KID_KPAIR        0x30   // 0x30-0x39: 10 groups key pairs, 0x38/0x39 is ram temp kpair
#define KID_SYM          0x40   // 0x40-0x48: 9  groups symmetric keys, 0x48 is ram temp session key
#define KID_ZONE         0x50   // 0x50-0x57: 8  groups zone keys
//--------------------------------------------------------------
//  Hash Update parameter definition
//--------------------------------------------------------------
#define OPT_NONE         0x00
#define OPT_SN           0x80
#define OPT_UID          0x40
#define OPT_MOTP         0x20
#define OPT_NONCE        0x10
#define OPT_COUNTER1     0x08
#define OPT_COUNTER0     0x04
#define OPT_ZONE         0x02
//--------------------------------------------------------------
//  Struct definition
//--------------------------------------------------------------
typedef struct {
	unsigned char sla;      // slave address, high 7 bits active
	unsigned char ver[8];   // version
	unsigned char sn[8];
	unsigned char uid[8];
	unsigned char fuse[10];
	unsigned int  stream;
	unsigned char dbuf[256];
	unsigned long counter0;
	unsigned long counter1;
	unsigned char motp[64];
	unsigned char h_nonce[32];
	unsigned char c_nonce[32];
	unsigned char encflag;
	unsigned char getflag;
} DX83_CHIP;

//-----------------------------------------------------------------------------
//  external variable and function
//-----------------------------------------------------------------------------
extern unsigned char TransferPacket(unsigned char *buf, unsigned char txlen);

//-----------------------------------------------------------------------------
//  system function
//-----------------------------------------------------------------------------
unsigned char DX83_SoftReset(DX83_CHIP *dx83);
unsigned char DX83_Sleep(DX83_CHIP *dx83);

unsigned char DX83_ReadFlash(DX83_CHIP *dx83, unsigned short address, unsigned char *buf, unsigned char len);
unsigned char DX83_WriteSN(DX83_CHIP *dx83, unsigned char *sn); // sn: 8 bytes

unsigned char DX83_GetChipInfo(DX83_CHIP *dx83); // 更新结构体信息
unsigned char DX83_GetRandom(DX83_CHIP *dx83, unsigned char *random, unsigned char len);
unsigned char DX83_SwapNonce(DX83_CHIP *dx83, unsigned char *txbuf, unsigned char *rxbuf); // txbuf/rxbuf: 32 bytes

// index = 0 -- 7, verify zone
// index = 8, verify pin
unsigned char DX83_VerifyKey(DX83_CHIP *dx83, unsigned char index, unsigned char *seed, unsigned char *pin_zkey); // seed/pin_zkey: 32 bytes

unsigned char DX83_WriteUID(DX83_CHIP *dx83, unsigned char *uid);   // uid: 8 bytes
unsigned char DX83_WriteSlaveAddress(DX83_CHIP *dx83, unsigned char slave);
unsigned char DX83_ReadMOTP(DX83_CHIP *dx83, unsigned char *motp);  // motp: 64 bytes
unsigned char DX83_WriteMOTP(DX83_CHIP *dx83, unsigned char *motp); // motp: 64 bytes
unsigned char DX83_ReadSM2ID(DX83_CHIP *dx83, unsigned char *sm2id, unsigned char *len);  // sm2id: 128 bytes
unsigned char DX83_WriteSM2ID(DX83_CHIP *dx83, unsigned char *sm2id, unsigned char len);
unsigned char DX83_LockZoneWrite(DX83_CHIP *dx83, unsigned char zone);
unsigned char DX83_LockZoneRead(DX83_CHIP *dx83, unsigned char zone);

unsigned char DX83_SetCounterKid(DX83_CHIP *dx83, unsigned char index, unsigned char kid_kpair_sym);
unsigned char DX83_SetCounterValue(DX83_CHIP *dx83, unsigned char index, unsigned int counter);
unsigned char DX83_GetCounterValue(DX83_CHIP *dx83, unsigned char index);
unsigned char DX83_DecCounterValue(DX83_CHIP *dx83, unsigned char index);
unsigned char DX83_ClearCounterValue(DX83_CHIP *dx83, unsigned char index);

unsigned char DX83_WriteZone(DX83_CHIP *dx83, unsigned char zone, unsigned short address, unsigned char *buf, unsigned char len);
unsigned char DX83_ReadZone(DX83_CHIP *dx83, unsigned char zone, unsigned short address, unsigned char *buf, unsigned char len);

unsigned char DX83_ImportKey(DX83_CHIP *dx83, unsigned char kid, unsigned char *key, unsigned char len);
unsigned char DX83_GenKeyPair(DX83_CHIP *dx83, unsigned char alg0, unsigned char kid_kpair, unsigned char *pbk); // pbk: 64 bytes
unsigned char DX83_GetPublicKey(DX83_CHIP *dx83, unsigned char kid_pbk, unsigned char *pbk);  // pbk: 64 bytes

// hash: 32 bytes, sign: 64 bytes
unsigned char DX83_Signature(DX83_CHIP *dx83, unsigned char alg0, unsigned char kid_kpair, unsigned char *hash, unsigned char *sign);

// hash: 32 bytes, sign: 64 bytes, result: 4bytes
unsigned char DX83_VerfiySignature(DX83_CHIP *dx83, unsigned char alg0, unsigned char kid_pbk, unsigned char *hash_sign, unsigned char *result);

// tmpbk: 64 bytes, sessionkey: 64 bytes
// need to gen temp kpair of kid = 0x38 before
// session key is also copyed to symmetric key of kid = 0x48
unsigned char DX83_EcdhAgreement(DX83_CHIP *dx83, unsigned char alg0, unsigned char kid_kpair, unsigned char *tmpbk, unsigned char *sessionkey);

// tmpbk: 64 bytes, sessionkey: klen bytes
// need to gen temp kpair of kid = 0x38 before
// need to write sm2ida before
// session key is also copyed to symmetric key of kid = 0x48
unsigned char DX83_SM2AgreementA(DX83_CHIP *dx83, unsigned char kid_kpair, unsigned char kid_pbk, unsigned char *tmpbk_id, unsigned char len, unsigned char *sessionkey, unsigned char klen);

// tmpbk: 64 bytes, sessionkey: klen bytes
// need to gen temp kpair of kid = 0x39 before
// need to write sm2idb before
// session key is also copyed to symmetric key of kid = 0x48
unsigned char DX83_SM2AgreementB(DX83_CHIP *dx83, unsigned char kid_kpair, unsigned char kid_pbk, unsigned char *tmpbk_id, unsigned char len, unsigned char *sessionkey, unsigned char klen);

unsigned char DX83_GenSessionKey(DX83_CHIP *dx83, unsigned char kid_sym, unsigned char hkdf, unsigned char *buf, unsigned char buf_len, unsigned char *sessionkey, unsigned char klen);

// Length(encrypt) = plaint_len + 96
unsigned char DX83_PublicEncrypt(DX83_CHIP *dx83, unsigned char alg0, unsigned char kid_pbk, unsigned char *plaint, unsigned char plaint_len, unsigned char *encrypt);

// Length(paint) = encrypt_len - 96
unsigned char DX83_PrivateDecrypt(DX83_CHIP *dx83, unsigned char alg0, unsigned char kid_kpair, unsigned char *encrypt, unsigned char encryt_len, unsigned char *plaint);

//-----------------------------------------------------------------------------
// option: | bit7 | bit6 | bit5 |  bit4 |   bit3   |  bit2    | bit1 |  bit0
//-----------------------------------------------------------------------------
//         |  SN  | UID  | MOTP | Nonce | Counter1 | Counter0 | Zone |  --- 
//-----------------------------------------------------------------------------
// result: 32 bytes
// digest: 32 bytes
unsigned char DX83_HmacAuthenticate(DX83_CHIP *dx83, unsigned char alg1, unsigned char kid_sym, unsigned char *buf, unsigned char len, unsigned char *result);
void          Lib_HmacAuthenticate(DX83_CHIP *dx83, unsigned char alg1, unsigned char *key, unsigned char *buf, unsigned char len, unsigned char *result);
unsigned char DX83_HashInit(DX83_CHIP *dx83, unsigned char alg1, unsigned char kid_sym);
unsigned char DX83_HashUpdate(DX83_CHIP *dx83, unsigned char option, unsigned char *buf, unsigned char len);
unsigned char DX83_HashUpdateZone(DX83_CHIP *dx83, unsigned char zone, unsigned short address, unsigned short len);
unsigned char DX83_HashFinal(DX83_CHIP *dx83, unsigned char *digest);

// iv: 16 bytes
unsigned char DX83_CipherInit(DX83_CHIP *dx83, unsigned char alg2, unsigned char kid_sym, unsigned char *iv);
unsigned char DX83_CipherEncrypt(DX83_CHIP *dx83, unsigned char *plaint, unsigned char len, unsigned char *encrypt);
unsigned char DX83_CipherDecrypt(DX83_CHIP *dx83, unsigned char *encrypt, unsigned char len, unsigned char *plaint);

//-----------------------------------------------------------------------------------------------------------------------------------------
//  SM9 related Function
//-----------------------------------------------------------------------------------------------------------------------------------------
unsigned char DX83_SM9ImportKey(DX83_CHIP *dx83, unsigned char kid, unsigned char *key, unsigned char len);
unsigned char DX83_SM9GetPublicKey(DX83_CHIP *dx83, unsigned char kid_pbk, unsigned char *pbk);

unsigned char DX83_SM9GenG1MasterKeyPair(DX83_CHIP *dx83, unsigned char kid_kpair, unsigned char *pbk);
unsigned char DX83_SM9GenG2MasterKeyPair(DX83_CHIP *dx83, unsigned char kid_kpair, unsigned char *pbk);

// need to write sm9id before
unsigned char DX83_SM9GenG1UserPrivateKey(DX83_CHIP *dx83, unsigned char mkid, unsigned char ukid, unsigned char hid, unsigned char *smid, unsigned char idlen);
// need to write sm9id before
unsigned char DX83_SM9GenG2UserPrivateKey(DX83_CHIP *dx83, unsigned char mkid, unsigned char ukid, unsigned char hid, unsigned char *smid, unsigned char idlen);

unsigned char DX83_SM9Signature(DX83_CHIP *dx83, unsigned char mkid, unsigned char ukid, unsigned char *message, unsigned char mlen, unsigned char *sign);
// need to write the Signer sm9id before
unsigned char DX83_SM9VerifySignature(DX83_CHIP *dx83, unsigned char mkid, unsigned char hid, unsigned char *message, unsigned char mlen, unsigned char *sign, unsigned char *result);

// need to write receiver sm9id before
unsigned char DX83_SM9KeyEncapsulation(DX83_CHIP *dx83, unsigned char mkid, unsigned char hid, unsigned char klen, unsigned char *c, unsigned char *sessionkey);
// need to write receiver sm9id before
unsigned char DX83_SM9KeyDecapsulation(DX83_CHIP *dx83, unsigned char ukid, unsigned char klen, unsigned char *c, unsigned char *sessionkey);

// need to write receiver sm9id before
// Length(encrypt) = plaint_len + 96
unsigned char DX83_SM9Encrypt(DX83_CHIP *dx83, unsigned char mkid, unsigned char hid, unsigned char *plaint, unsigned char plaint_len, unsigned char *encrypt);
// need to write receiver sm9id before
// Length(paint) = encrypt_len - 96
unsigned char DX83_SM9Decrypt(DX83_CHIP *dx83, unsigned char ukid, unsigned char *encrypt, unsigned char encrypt_len, unsigned char *plaint);

// need to write the other side's sm9id before
unsigned char DX83_SM9GenTempKeyPair(DX83_CHIP *dx83, unsigned char mkid, unsigned char ukid, unsigned char hid, unsigned char *smid, unsigned char idlen, unsigned char *tmpbk);
// tmpbk: 64 bytes, sessionkey: klen bytes
// need to gen temp kpair of kid = 0x68 before
// need to write sm9ida before
// session key is also copyed to symmetric key of kid = 0x48
unsigned char DX83_SM9AgreementA(DX83_CHIP *dx83, unsigned char mkid, unsigned char ukid, unsigned char *tmpbk_id, unsigned char len,unsigned char *sessionkey, unsigned char klen);
// tmpbk: 64 bytes, sessionkey: klen bytes
// need to gen temp kpair of kid = 0x69 before
// need to write sm9idb before
// session key is also copyed to symmetric key of kid = 0x48
unsigned char DX83_SM9AgreementB(DX83_CHIP *dx83, unsigned char mkid, unsigned char ukid, unsigned char *tmpbk_id, unsigned char len,unsigned char *sessionkey, unsigned char klen);


#ifdef __cplusplus
}
#endif

#endif

