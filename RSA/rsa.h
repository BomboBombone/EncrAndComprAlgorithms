#pragma once
#include "euclidean.h"

struct PubKey {
	struct BigInt* n;
	unsigned int e;
};
struct PrivKey {
	struct BigInt* phin;
	struct BigInt* d;
	struct BigInt* dP;
	struct BigInt* dQ;
	struct BigInt* qInv;
	struct BigInt* p;
	struct BigInt* q;
};
struct RSA
{
	struct PrivKey* privKey;
	struct PubKey* pubKey;
};

typedef enum RsaKeySize {
	RSA_128,
	RSA_256
} rsaKeySize;

typedef enum RsaKeyType {
	PUB_KEY,
	PRIV_KEY
} rsaKeyType;

struct RSA* RSAFactory(struct BigInt* p, struct BigInt* q, unsigned int e, rsaKeySize sz);
void RSADispose(struct RSA* p);
void PubKeyDispose(struct PubKey* p);
void PrivKeyDispose(struct PrivKey* p);

struct BigInt* RSACrypt(struct RSA* rsa, char* mes);
struct BigInt* RSADecrypt(struct RSA* rsa, struct BigInt* mes);
char* RSADecrypt2(struct RSA* rsa, struct BigInt* mes);