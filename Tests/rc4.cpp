#include "CppUnitTest.h"

extern "C" {
#include "RC4/rc4.h"
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace RC4Tests {
	TEST_CLASS(RC4Encryption) {
	public:
		TEST_METHOD(Encrypt) {
			const char expected[] = {
				0x52, 0x42, 0x04, 0xe8, 
				0x6d, 0x6d, 0x65, 0x5a, 
				0x27, 0x04, 0xee, 0x31, 
				0xc8, 0xbf, 0xbf, 0xd0, 
				0x35, 0xe2, 0xf5, 0x65
			};

			const char* mykey = "mypersonalkey";
			const int keylen = 14;

			unsigned char* mymes = (unsigned char*)"This is my Message!";
			const int mymeslen = 20;

			RC4 algorithm = RC4Factory(mykey, keylen);

			unsigned char* encr = (unsigned char*)malloc(mymeslen);

			EncryptDecrypt(&algorithm, mymes, encr, mymeslen);

			Assert::IsTrue(!memcmp(encr, expected, mymeslen));

			free(encr);
		}

		TEST_METHOD(Decrypt) {
			unsigned char encrypted[] = {
				0x9f, 0xfb, 0x2f, 0xd3,
				0x5a, 0x76, 0xaa, 0x30, 
				0x93, 0xa1, 0xd6, 0x35, 
				0x6a, 0xcf, 0xc4, 0xbf, 
				0xb7, 0xec, 0xc1, 0x65
			};

			const char* mykey = "mysecondpersonalkey";
			const int keylen = 20;

			unsigned char* mymes = (unsigned char*)"This is my Message!";
			const int mymeslen = 20;

			RC4 algorithm = RC4Factory(mykey, keylen);

			unsigned char* decr = (unsigned char*)malloc(mymeslen);

			EncryptDecrypt(&algorithm, encrypted, decr, mymeslen);

			Assert::IsTrue(!memcmp(decr, mymes, mymeslen));

			free(decr);
		}

		TEST_METHOD(EncrDecr) {
			const char* mykey = "myveryspecialkey";
			const int keylen = strlen(mykey);

			unsigned char* mymes = (unsigned char*)"This is my Message!";
			const int mymeslen = 20;

			RC4 algorithm = RC4Factory(mykey, keylen);

			unsigned char* encr = (unsigned char*)malloc(mymeslen);
			unsigned char* decr = (unsigned char*)malloc(mymeslen);

			EncryptDecrypt(&algorithm, mymes, encr, mymeslen);
			EncryptDecrypt(&algorithm, encr, decr, mymeslen);

			Assert::IsTrue(!memcmp(decr, mymes, mymeslen));

			free(encr);
			free(decr);
		}
	};
}