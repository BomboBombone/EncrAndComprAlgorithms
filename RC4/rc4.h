#pragma once

struct RC4
{
	unsigned char SBox[256];
	char Key[256];
	int keylen;
};

struct RC4 RC4Factory(const char* key, int _keylen);
void EncryptDecrypt(struct RC4* rc4, unsigned char* mes, unsigned char* ciph, const int size);