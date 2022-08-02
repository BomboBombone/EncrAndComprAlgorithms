extern "C" {
#include "rc4.h"
}

#include <iostream>

int main() {
	const char* mykey = "mypersonalkey";
	const int keylen = 14;

	unsigned char* mymes = (unsigned char*) "This is my Message!";
	const int mymeslen = 20;

	RC4 algorithm = RC4Factory(mykey, keylen);

	unsigned char* encr = (unsigned char*) malloc(mymeslen);
	unsigned char* decr = (unsigned char*)malloc(mymeslen);

	EncryptDecrypt(&algorithm, mymes, encr, mymeslen);

	std::cout << encr << std::endl;

	EncryptDecrypt(&algorithm, encr, decr, mymeslen);
	std::cout << decr << std::endl;

	free(encr);
	free(decr);

	system("pause");
	return 0;
}