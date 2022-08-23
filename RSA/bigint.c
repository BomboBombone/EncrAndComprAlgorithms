#include "bigint.h"

#define true 1;
#define false 0;

struct BigInt* BigIntFactoryBlocks(BigIntSize sz, uint64_t* _data)
{
	size_t size = 128;

	if (sz == BI_256) {
		size = 256;
	}
	else if (sz == BI_512) {
		size = 512;
	}

	struct BigInt* obj = malloc(sizeof(struct BigInt));
	if (!obj) goto end;

	obj->type = sz;
	obj->sign = 0;

	ZeroMemory(obj, 512);
	if (_data) {
		byte dwords = size / 8;

		for (byte i = 0; i < dwords; i++) {
			((uint64_t*)obj->data)[i] = _data[dwords - 1 - i];
		}
	}

end:
	return obj;
}

struct BigInt* BigIntFactory(BigIntSize sz, const char* _data) {
	size_t size = 128;

	if (sz == BI_256) {
		size = 256;
	}
	else if (sz == BI_512) {
		size = 512;
	}

	struct BigInt* obj = malloc(sizeof(struct BigInt));
	if (!obj) goto end;

	obj->type = sz;
	obj->sign = 0;

	ZeroMemory(obj, 512);
	if (_data) {
		for (unsigned int i = 0; i < size; i++) {
			obj->data[i] = _data[i];
		}
	}

end:
	return obj;
}

struct BigInt* BigIntFactoryMove(struct BigInt* p)
{
	size_t size = 128;

	if (p->type == BI_256) {
		size = 256;
	}
	else if (p->type == BI_512) {
		size = 512;
	}

	struct BigInt* obj = malloc(sizeof(struct BigInt));
	if (!obj) goto end;
	obj->type = p->type;
	obj->sign = p->sign;

	ZeroMemory(obj, 512);
	for (unsigned int i = 0; i < size; i++) {
		obj->data[i] = p->data[i];
	}

end:
	return obj;
}

struct BigInt* BigIntFactoryRef(struct BigInt* p)
{
	size_t size = 512;

	if (!p) goto end;
	p->type = BI_512;
	p->sign = 0;

	for (unsigned int i = 0; i < size; i++) {
		p->data[i] = 0;
	}

end:
	return p;
}

byte AddBigInt(struct BigInt* lhs, struct BigInt* rhs) {
	if (rhs->sign) {
		rhs->sign = 0;
		SubBigInt(lhs, rhs);
		rhs->sign = 1;

		return;
	}

	byte carry = 0;
	unsigned int size = lhs->type == BI_128 ? 128 : 256;
	byte dwords = size / 8;

	if (lhs->sign) {
		struct BigInt buf;
		((uint64_t*)buf.data)[dwords] = 1;

		lhs->sign = 0;
		SubBigInt(&buf, lhs);
		CopyBigInt(lhs, &buf);

		CopyBigInt(&buf, rhs);
		SubBigInt(&buf, lhs);
		CopyBigInt(lhs, &buf);
		lhs->sign = 1;
	}
	else {
		for (byte i = dwords - 1; i < dwords; i--) {
			uint64_t res = ((uint64_t*)lhs)[i] + ((uint64_t*)rhs)[i];

			if (res < ((uint64_t*)lhs)[i] || res < ((uint64_t*)rhs)[i]) {
				for (byte j = i + 1; j < dwords; j++) {
					if (((uint64_t*)lhs)[j] == UINT64_MAX) {
						((uint64_t*)lhs)[j] = 0;
						continue;
					}
					else {
						((uint64_t*)lhs)[j]++;
						break;
					}
				}
			}

			((uint64_t*)lhs)[i] = res;
		}
	}

	return true;
}

byte SubBigInt(struct BigInt* lhs, struct BigInt* rhs)
{
	if (rhs->sign) {
		rhs->sign = 0;
		AddBigInt(lhs, rhs);
		rhs->sign = 1;

		return;
	}

	unsigned int lSz = 128;
	unsigned int rSz = 128;

	switch (lhs->type)
	{
	case BI_256:
		lSz = 256;
		break;
	case BI_512:
		lSz = 512;
		break;
	}
	switch (rhs->type)
	{
	case BI_256:
		rSz = 256;
		break;
	case BI_512:
		rSz = 512;
		break;
	}
	byte lDwords = lSz / 8;
	byte rDwords = rSz / 8;

	for (byte i = lDwords - 1; i < lDwords; i--) {
		uint64_t res = ((uint64_t*)lhs)[i] - ((uint64_t*)rhs)[i];
		if (((uint64_t*)lhs)[i] < ((uint64_t*)rhs)[i]) {
			if (i == rDwords - 1)
				return false;

			for (byte j = (i + 1); j < rDwords; j++) {
				if (((uint64_t*)lhs)[j]) {
					((uint64_t*)lhs)[j]--;
					break;
				}
				else if (!j) {
					((uint64_t*)lhs)[j] = UINT64_MAX;
					lhs->sign = 1;
					return true;
				}
				else {
					((uint64_t*)lhs)[j] = UINT64_MAX;
					continue;
				}
			}
		}

		((uint64_t*)lhs)[i] = res;
	}

	return true;
}

byte IsBiggerOrEqBigInt(struct BigInt* lhs, struct BigInt* rhs) {
	unsigned int size = ((struct BigInt*)lhs)->type == ((struct BigInt*)rhs)->type == BI_128 ? 128 : 256;

	if (lhs->type == BI_512 || rhs->type == BI_512) {
		for (unsigned int i = 511; i > size - 1; i--) {
			int res = lhs->data[i] - rhs->data[i];
			if (res > 0) {
				return true;
			}
			else if (res < 0)
				return false;
		}
	}

	for (int i = size - 1; i >= 0; i--) {
		if (lhs->data[i] > rhs->data[i]) {
			return true;
		}
		else if (lhs->data[i] == rhs->data[i]) {
			if (!i)
				return true;
			continue;
		}
		else {
			return false;
		}
	}
	return false;
}

byte IsBiggerBigInt(struct BigInt* lhs, struct BigInt* rhs) {
	unsigned int size = ((struct BigInt*)lhs)->type == ((struct BigInt*)rhs)->type == BI_128 ? 128 : 256;

	if (lhs->type == BI_512 || rhs->type == BI_512) {
		for (unsigned int i = 511; i > size - 1; i--) {
			int res = lhs->data[i] - rhs->data[i];
			if (res > 0) {
				return true;
			}
			else if (res < 0)
				return false;
		}
	}

	for (int i = size - 1; i >= 0; i--) {
		if (lhs->data[i] > rhs->data[i]) {
			return true;
		}
		else if (lhs->data[i] == rhs->data[i]) {
			continue;
		}
		else {
			return false;
		}
	}
	return false;
}

//Multiply 2 BigInts up to 256 bytes
void MulBigInt256(struct BigInt* lhs, struct BigInt* rhs, uint32_t* out) {
	unsigned int words = 512 / 4;
	
	for (unsigned int i = 0; i < words; i++) {
		for (unsigned int  j = 0; j < words; j++) {
			uint64_t r = ((uint32_t*)rhs->data)[i];
			uint64_t l = ((uint32_t*)lhs->data)[j];

			uint64_t res = r * l;
			if (i + j < words) {
				out[i + j] += res;
				if (((uint32_t*)&res)[1] && i + j + 1 < words)
					out[i + j + 1] += ((uint32_t*)&res)[1];
			}
		}
	}
}

void CopyBigInt(struct BigInt* dst, struct BigInt* src) {
	memcpy(dst, src, sizeof (struct BigInt));
}

void DivBigInt(struct BigInt* a, struct BigInt* b, struct BigInt* q, struct BigInt* r) {
	if (IsBiggerBigInt(b, a)) {
		CopyBigInt(r, a);
		return;
	}
	unsigned int aLen = LenBitBigInt(a);
	unsigned int bLen = LenBitBigInt(b);

	struct BigInt* buf = BigIntFactory(BI_512, 0);
	GetBitsBigInt(a, aLen - bLen, aLen, buf);

	for (int i = aLen - bLen; i >= 0; i--) {
		if (!IsBiggerOrEqBigInt(buf, b)) {
			SetBitBigInt(q, i, 0);
			goto put_down;
		}
		SubBigInt(buf, b);
		SetBitBigInt(q, i, 1);

	put_down:
		if (i) {
			ShiftLeftBigInt(buf, 1);
			if (GetBitBigInt(a, i - 1)) {
				SetBitBigInt(buf, 0, 1);
			}
		}
	}

	CopyBigInt(r, buf);

	free(buf);
}

void ModBigIntRef(struct BigInt* base, struct BigInt* mod, struct BigInt* out) {
	memcpy(out, base, sizeof(struct BigInt));
	ModBigInt(out, mod);
}

void ModBigInt(struct BigInt* base, struct BigInt* mod) {
	struct BigInt r;
	struct BigInt q;
	BigIntFactoryRef(&r);
	BigIntFactoryRef(&q);

	DivBigInt(base, mod, &q, &r);

	memcpy(base, &r, sizeof base);
}

void ShiftRightBigInt(struct BigInt* base, unsigned int amount) {
	if (!amount)
		return;
	int bytes = amount / 8;
	byte bits = amount % 8;

	size_t size = LenBigInt(base);

	if (bytes) {
		for (int i = bytes; i < size; i++) {
			base->data[i - bytes] = base->data[i];
			base->data[i] = 0;
		}
	}

	byte carry = 0;
	for (int i = size - 1 - bytes; i >= 0; i--) {
		byte old = base->data[i];
		base->data[i] = (base->data[i] >> bits) | carry;
		carry = old << (8 - bits);
	}
}

void ShiftLeftBigInt(struct BigInt* base, unsigned int amount) {
	if (!amount)
		return;
	int bytes = amount / 8;
	byte bits = amount % 8;

	size_t size = LenBigInt(base);

	if (bytes) {
		for (int i = size - 1; i >= 0; i--) {
			base->data[i + bytes] = base->data[i];
			base->data[i] = 0;
		}
	}
	byte carry = 0;
	for (int i = bytes; i <= size + bytes && i < 512/8; i++) {
		byte old = base->data[i];
		base->data[i] = (base->data[i] << bits) | carry;
		carry = old >> (8 - bits);
	}
}

void SetBitBigInt(struct BigInt* p, unsigned int index, byte mask) {
	unsigned int bytes = index / 8;
	byte bits = index % 8;

	if (mask) {
		p->data[bytes] |= 1UL << bits;
	}
	else {
		p->data[bytes] &= ~(1UL << bits);
	}
}

byte GetBitBigInt(struct BigInt* p, unsigned int index) {
	unsigned int bytes = index / 8;
	byte bits = index % 8;

	return (p->data[bytes] >> bits) << 7;
}

void GetBitsBigInt(struct BigInt* in, unsigned int start, unsigned int end, struct BigInt* out) {
	unsigned int inLen = LenBitBigInt(in);
	unsigned int startBytes = start / 8;
	byte startBits = start % 8;
	unsigned int endBytes = end / 8;
	byte endBits = end % 8;

	if (start > end) {
		unsigned int temp = end;
		end = start;
		start = temp;
	}

	CopyBigInt(out, in);
	ShiftLeftBigInt(out, inLen - end);
	ShiftRightBigInt(out, start);
}

unsigned int LenBigInt(struct BigInt* p) {
	size_t size = 128;

	if (p->type == BI_256) {
		size = 256;
	}
	else if (p->type == BI_512) {
		size = 512;
	}

	for (int i = size - 1; i >= 0; i--) {
		if (p->data[i])
			return i + 1;
	}
	return 0;
}

unsigned int LenBitBigInt(struct BigInt* p) {
	unsigned int bytes = LenBigInt(p);
	byte msd = p->data[bytes - 1];
	bytes = (bytes - 1) * 8;
	byte bits = 0;
	for (bits = 1; bits <= 8 && msd; bits++) {
		msd >>= 1;
	}
	bytes += bits - 1;
}

void PowmBigInt(struct BigInt* base, struct BigInt* mod, struct BigInt* power, struct BigInt* out) {
	struct BigInt _b;
	struct BigInt _c;
	struct BigInt* b = &_b;
	struct BigInt* c = &_c;
	struct BigInt zero;
	BigIntFactoryRef(&zero);
	BigIntFactoryRef(c);
	CopyBigInt(b, base);

	out->data[0] = 1;

	struct BigInt* e = BigIntFactoryMove(power);

	while (IsBiggerBigInt(e, &zero)) {
		if (e->data[0] & 1) {
			MulBigInt256(out, b, c);
			ModBigInt(c, mod);
			memcpy(out, c, b->type == BI_128 ? 128 : 256);

			ZeroMemory(c->data, 512);
		}
		MulBigInt256(b, b, c);
		ModBigInt(c, mod);
		memcpy(b, c, b->type == BI_128 ? 128 : 256);
		ZeroMemory(c->data, 512);

		ShiftRightBigInt(e, 1);
	}

	ModBigInt(out, mod);

	free(e);
}

void PowmBigIntUInt(struct BigInt* base, struct BigInt* mod, unsigned int power, struct BigInt* out)
{
	struct BigInt e;
	ZeroMemory(e.data, 512);
	e.sign = 0;
	*(unsigned int*)(&e)->data = power;

	PowmBigInt(base, mod, &e, out);
}

void PowmBigIntChar(unsigned char base, struct BigInt* mod, unsigned int power, struct BigInt* out)
{
	struct BigInt _b;
	struct BigInt* b = &_b;
	ZeroMemory(b->data, 512);
	_b.type = mod->type;
	_b.sign = 0;
	b->data[0] = base;

	PowmBigIntUInt(b, mod, power, out);
}

byte IsZeroBigInt(struct BigInt* rhs)
{
	size_t size = 128;

	if (rhs->type == BI_256) {
		size = 256;
	}
	else if (rhs->type == BI_512) {
		size = 512;
	}
	for (int i = size - 1; i >= 0; i--) {
		if (rhs->data[i])
			return false;
	}

	return true;
}
