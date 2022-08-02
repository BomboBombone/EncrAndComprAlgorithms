#include "rc4.h"

struct RC4 RC4Factory(const char* key, int _keylen)
{
	struct RC4 rc4;
    rc4.keylen = _keylen;
	for (int i = 0; i < rc4.keylen; i++) {
		rc4.Key[i] = key[i];
	}
	return rc4;
}

void KSA(struct RC4* rc4)
{
	for (unsigned int i = 0; i < 256; i++) {
		rc4->SBox[i] = i;
	}

	unsigned int j = 0;
	for (unsigned int i = 0; i < 256; i++) {
		j = (j + rc4->SBox[i] + rc4->Key[i % rc4->keylen]) % 256;

		//std::swap(SBox[i], SBox[j])
		unsigned char temp = rc4->SBox[i];
		rc4->SBox[i] = rc4->SBox[j];
		rc4->SBox[j] = temp;
	}
}

void EncryptDecrypt(struct RC4* rc4, unsigned char* mes, unsigned char* ciph, const int size)
{
	KSA(rc4);
	unsigned int i = 0, j = 0;
	for (unsigned int k = 0; k < size; k++) {
		i = (i + 1) % 256;
		j = (j + rc4->SBox[i]) % 256;

		//std::swap(SBox[i], SBox[j])
		unsigned char temp = rc4->SBox[i];
		rc4->SBox[i] = rc4->SBox[j];
		rc4->SBox[j] = temp;

		ciph[k] = rc4->SBox[(rc4->SBox[i] + rc4->SBox[j]) % 256] ^ mes[k];
	}
}
