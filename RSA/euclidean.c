#include "euclidean.h"

void EuclideanGCD(struct BigInt* gcd, struct BigInt* lhs, struct BigInt* rhs, struct BigInt* x, struct BigInt* y) {
	struct BigInt remainderBuf;
	struct BigInt old_s, old_r, s, r;
	BigIntFactoryRef(&s);
	BigIntFactoryRef(&old_s);
	CopyBigInt(&r, rhs);
	CopyBigInt(&old_r, lhs);
	
	old_s.data[0] = 1;

	while (!IsZeroBigInt(&r)) {
		struct BigInt q;
		struct BigInt subBuf;
		struct BigInt mulBuf;
		struct BigInt oldBuf;
		CopyBigInt(&subBuf, &old_r);
		BigIntFactoryRef(&q);
		BigIntFactoryRef(&mulBuf);
		BigIntFactoryRef(&oldBuf);

		ZeroBigInt(&remainderBuf);
		//quotient := old_r div r
		DivBigInt(&old_r, &r, &q, &remainderBuf);
		//(old_r, r) := (r, old_r − quotient × r)
		CopyBigInt(&oldBuf, &r);
		CopyBigInt(&r, &remainderBuf);
		CopyBigInt(&old_r, &oldBuf);
		//(old_s, s) := (s, old_s − quotient × s)
		CopyBigInt(&subBuf, &old_s);
		ZeroBigInt(&mulBuf);
		ZeroBigInt(&oldBuf);
		CopyBigInt(&oldBuf, &s);
		MulBigInt256(&q, &s, &mulBuf);
		SubBigInt(&subBuf, &mulBuf);
		CopyBigInt(&s, &subBuf);
		CopyBigInt(&old_s, &oldBuf);
	}

	struct BigInt bez;
	BigIntFactoryRef(&bez);
	if (!IsZeroBigInt(&rhs)) {
		//bez := (old_r − old_s × a) div b
		struct BigInt buf;
		BigIntFactoryRef(&buf);
		MulBigInt256(&old_s, lhs, &buf);

		CopyBigInt(&bez, &old_r);
		SubBigInt(&bez, &buf);
		ZeroBigInt(&buf);

		ZeroBigInt(&remainderBuf);
		DivBigInt(&bez, rhs, &buf, &remainderBuf);
		CopyBigInt(&bez, &buf);
	}

	CopyBigInt(x, &old_s);
	CopyBigInt(y, &bez);
	CopyBigInt(gcd, &old_r);
}