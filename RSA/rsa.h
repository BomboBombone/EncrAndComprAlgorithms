#pragma once
#include "euclidean.h"

struct PubKey {
	struct BigInt* n;
	unsigned int e;
};
struct PrivKey {
	struct BigInt* n;
	struct BigInt* d;
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
struct BigInt* RSACrypt(struct RSA* rsa, char* mes);
char* RSADecrypt(struct RSA* rsa, struct BigInt* mes);