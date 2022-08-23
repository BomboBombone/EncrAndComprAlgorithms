
#include "CppUnitTest.h"

extern "C" {
#include "RSA/rsa.h"
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(BigIntTests)
	{
	public:
		TEST_METHOD(Subtraction)
		{
            auto type = BI_256;
            uint64_t bigint1[] = {
                0xd0, 0x50, 0x86, 0x55, 0xf4, 0xa4, 0xc7, 0xd3,
                0x9b, 0x56, 0x79, 0xf6, 0x78, 0x47, 0x5c, 0x98,
                0xf6, 0x63, 0x52, 0x2b, 0x80, 0x5d, 0x21, 0xac,
                0x6c, 0x4d, 0xc5, 0xc1, 0xf5, 0x70, 0x36, 0xb0
            };
            uint64_t bigint2[] = {
                0xd0, 0x50, 0x86, 0x55, 0xf4, 0xa4, 0xc7, 0xd3,
                0x9b, 0x56, 0x79, 0xf6, 0x78, 0x47, 0x5c, 0x98,
                0xf6, 0x63, 0x52, 0x2b, 0x80, 0x5d, 0x21, 0xac,
                0x6c, 0x4d, 0xc5, 0xc1, 0xf5, 0x70, 0x35, 0xb2
            };

            BigInt* BI1 = (BigInt*)BigIntFactoryBlocks(type, bigint1);
            BigInt* BI2 = (BigInt*)BigIntFactoryBlocks(type, bigint2);

            Assert::IsTrue(SubBigInt(BI1, BI2) &&
                *(uint64_t*)BI1->data == UINT64_MAX - 1);

            free(BI1);
            free(BI2);
		}
        TEST_METHOD(Addition)
        {
            auto type = BI_256;
            uint64_t bigint1[] = {
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, UINT64_MAX - 1
            };
            uint64_t result[] = {
                0xb0,0x36,0x70,0xf5,0xc1,0xc5,0x4d,0x6c,
                0xac,0x21,0x5d,0x80,0x2b,0x52,0x63,0xf6,
                0x98,0x5c,0x47,0x78,0xf6,0x79,0x56,0x9b,
                0xd3,0xc7,0xa4,0xf4,0x55,0x86,0x50,0xd0
            };
            uint64_t bigint2[] = {
                0xd0, 0x50, 0x86, 0x55, 0xf4, 0xa4, 0xc7, 0xd3,
                0x9b, 0x56, 0x79, 0xf6, 0x78, 0x47, 0x5c, 0x98,
                0xf6, 0x63, 0x52, 0x2b, 0x80, 0x5d, 0x21, 0xac,
                0x6c, 0x4d, 0xc5, 0xc1, 0xf5, 0x70, 0x35, 0xb2
            };

            BigInt* BI1 = (BigInt*)BigIntFactoryBlocks(type, bigint1);
            BigInt* BI2 = (BigInt*)BigIntFactoryBlocks(type, bigint2);

            Assert::IsTrue(AddBigInt(BI1, BI2) &&
                !memcmp(BI1->data, result, type == BI_128 ? 128 : 256));

            free(BI1);
            free(BI2);
        }
        TEST_METHOD(Multiplication) {
            BigInt* a = BigIntFactoryBlocks(BI_256, 0);
            a->data[1] = 8;
            BigInt* b = BigIntFactoryBlocks(BI_256, 0);
            b->data[0] = 2;

            BigInt* out = BigIntFactoryBlocks(BI_512, 0);
            MulBigInt256(a, b, (uint32_t*)out->data);

            Assert::IsTrue(out->data[1] == 16);

            free(a);
            free(b);
            free(out);
        }
        TEST_METHOD(Division) {
            unsigned int dividend = 0x24f7f4;
            unsigned int divisor = 0xc37;

            BigInt* a = BigIntFactoryBlocks(BI_256, 0);
            ((unsigned int*)a->data)[0] = dividend;
            BigInt* b = BigIntFactoryBlocks(BI_256, 0);
            ((unsigned int*)b->data)[0] = divisor;

            BigInt* q = BigIntFactoryBlocks(BI_512, 0);
            BigInt* r = BigIntFactoryBlocks(BI_512, 0);

            DivBigInt(a, b, q, r);

            Assert::IsTrue(((unsigned int*)q->data)[0] == dividend / divisor && ((unsigned int*)r->data)[0] == dividend % divisor);

            free(a);
            free(b);
            free(q);
            free(r);
        }
        TEST_METHOD(Mod) {
            BigInt* base = BigIntFactoryBlocks(BI_256, 0);
            BigInt* mod = BigIntFactoryBlocks(BI_256, 0);
            BigInt* out = BigIntFactoryBlocks(BI_512, 0);

            ((unsigned int*)base->data)[0] = 0x0024f7f4;
            ((unsigned int*)mod->data)[0] = 0x00000c37;

            ModBigIntRef(base, mod, out);

            Assert::IsTrue(((unsigned int*)out->data)[0] == 0x24f7f4 % 0xc37);

            free(mod);
            free(out);
        }
        TEST_METHOD(Powm) {
            unsigned char test_val = 0x59;
            unsigned int e = 0x10001;
            unsigned int res = 1;
            for (unsigned int i = 1; i < test_val * test_val; i++)
                res *= test_val;

            BigInt* mod = BigIntFactoryBlocks(BI_256, 0);
            BigInt* out = BigIntFactoryBlocks(BI_512, 0);

            ((unsigned int*)mod->data)[0] = res - 1;

            PowmBigIntChar(test_val, mod, e, out);

            Assert::IsTrue(out->data[0] == test_val);

            free(mod);
            free(out);
        }
        TEST_METHOD(LeftShift) {
            int shift = 1;
            int data = 0x96;

            BigInt* b = BigIntFactoryBlocks(BI_256, 0);
            b->data[0] = data;

            ShiftLeftBigInt(b, shift);
            Assert::IsTrue(((unsigned int*)b->data)[shift / 8] == data << (shift % 8));

            free(b);
        }
        TEST_METHOD(RightShift) {
            int shift = 1;
            int data = 0x96;

            BigInt* b = BigIntFactoryBlocks(BI_256, 0);
            b->data[0] = data;

            ShiftRightBigInt(b, shift);
            Assert::IsTrue(((unsigned int*)b->data)[0] == 0x4b);

            free(b);
        }
        TEST_METHOD(BitSet) {
            int shift = 4;
            BigInt* base = BigIntFactory(BI_256, 0);
            SetBitBigInt(base, shift, 1);
            Assert::IsTrue(base->data[0] = 16);

            free(base);
        }
        TEST_METHOD(BitGet) {
            int shift = 4;
            BigInt* base = BigIntFactory(BI_256, 0);
            base->data[0] = 16;
            Assert::IsTrue(GetBitBigInt(base, shift) && LenBitBigInt(base) == shift + 1);

            free(base);
        }

	};
    TEST_CLASS(EuclideanTests)
    {
    public:
        TEST_METHOD(TestExEuclidean) {
            BigInt* a = BigIntFactoryBlocks(BI_256, 0);
            BigInt* b = BigIntFactoryBlocks(BI_256, 0);
            BigInt* gcd = BigIntFactoryBlocks(BI_256, 0);
            BigInt* x = BigIntFactoryBlocks(BI_256, 0);
            BigInt* y = BigIntFactoryBlocks(BI_256, 0);

            a->data[0] = 99;
            b->data[0] = 78;

            EuclideanGCD(gcd, a, b, x, y);

            Assert::IsTrue(*(int*)x->data == 0xfffffff5 /*This is the "equivalent" of -11*/
                && *(int*)y->data == 0xe);

            free(a);
            free(b);
            free(gcd);
            free(x);
            free(y);
        }
    };
    TEST_CLASS(RSATests)
    {
    public:
        TEST_METHOD(TestRSACryptDecrypt) {
            BigInt* p = BigIntFactoryBlocks(BI_256, 0);
            BigInt* q = BigIntFactoryBlocks(BI_256, 0);

            p->data[0] = 53;
            q->data[0] = 59;
            unsigned int e = 3;

            RSA* rsa = RSAFactory(p, q, e, RSA_256);

            char mes[512] = { 0 };
            mes[0] = 89;

            BigInt* res = RSACrypt(rsa, mes);
            char* out = RSADecrypt(rsa, res);

            Assert::IsTrue(!(memcmp(mes, out, 256)));

            free(q);
            free(p);
        }
    };
}
