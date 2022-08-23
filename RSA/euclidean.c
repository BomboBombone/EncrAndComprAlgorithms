#include "euclidean.h"

byte EuclideanGCD(struct BigInt* gcd, struct BigInt* lhs, struct BigInt* rhs, struct BigInt* x, struct BigInt* y)
{
	struct BigInt* amodb = BigIntFactory(BI_512, 0);
	struct BigInt* rBuf = BigIntFactory(BI_512, 0);

	if (IsZeroBigInt(rhs)) {
		memcpy(gcd->data, lhs->data, 512);
		ZeroMemory(x, 512);
		ZeroMemory(y, 512);
		x->data[0] = 1;
		return;
	}

	struct BigInt* x1 = BigIntFactory(x->type, 0);

	struct BigInt* y1 = BigIntFactory(y->type, 0);

	BigIntFactoryRef(amodb);
	ModBigIntRef(lhs, rhs, amodb);
	byte sign = EuclideanGCD(gcd, rhs, amodb, x1, y1);

	memcpy(x, y1, sizeof x);

	struct BigInt* x2 = BigIntFactoryMove(x1);
	struct BigInt* res = BigIntFactory(y->type, 0);
	struct BigInt* res1 = BigIntFactory(y->type, 0);

	DivBigInt(lhs, rhs, res, rBuf);
	free(rBuf);

	MulBigInt256(res, y1, res1);

	SubBigInt(x2, res1);

	memcpy(y, x2, sizeof y);

	free(x1);
	free(y1);
	free(res);
	free(res1);
}
