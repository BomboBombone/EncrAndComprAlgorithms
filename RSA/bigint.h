#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <Shared/macros.h>

typedef unsigned char byte;

#define UINT64_MAX 0xFFFFFFFFFFFFFFFF

typedef enum bigintsize {
	BI_128,
	BI_256,
	BI_512
} BigIntSize;

struct BigInt {
	byte data[512];
	BigIntSize type;
	byte sign;
};

//Data should be organized with Big Endian convention, ex:
//1234567887654321 ==> {12345678, 87654321}
struct BigInt* BigIntFactoryBlocks(BigIntSize sz, uint64_t* _data);
struct BigInt* BigIntFactoryRef(struct BigInt* p);
struct BigInt* BigIntFactory(BigIntSize sz, const char* _data);
struct BigInt* BigIntFactoryMove(struct BigInt* p);
void CopyBigInt(struct BigInt* dst, struct BigInt* src);

byte AddBigInt(struct BigInt* lhs, struct BigInt* rhs);
byte SubBigInt(struct BigInt* lhs, struct BigInt* rhs);
void MulBigInt256(struct BigInt* lhs, struct BigInt* rhs, uint32_t* out);
void DivBigInt(struct BigInt* a, struct BigInt* b, struct BigInt* q, struct BigInt* r);
void ModBigInt(struct BigInt* base, struct BigInt* mod);
void ModBigIntRef(struct BigInt* base, struct BigInt* mod, struct BigInt* out);
unsigned int LenBigInt(struct BigInt* p);
byte IsBiggerBigInt(struct BigInt* lhs, struct BigInt* rhs);
void InvertSign(struct BigInt* p);
void MakePositive(struct BigInt* p);
byte EqualsBigInt(struct BigInt* lhs, struct BigInt* rhs);
void ZeroBigInt(struct BigInt* p);
byte IsZeroBigInt(struct BigInt* rhs);

//Bit operations
void ShiftRightBigInt(struct BigInt* base, unsigned int amount);
void ShiftLeftBigInt(struct BigInt* base, unsigned int amount);
unsigned int LenBitBigInt(struct BigInt* p);
void SetBitBigInt(struct BigInt* p, unsigned int index, byte mask);
byte GetBitBigInt(struct BigInt* p, unsigned int index);
void GetBitsBigInt(struct BigInt* in, unsigned int start, unsigned int end, struct BigInt* out);

//Should be a rough equivalent of boost's powm method
void PowmBigInt(struct BigInt* base, struct BigInt* mod, struct BigInt* power, struct BigInt* out);
void PowmBigIntUInt(struct BigInt* base, struct BigInt* mod, unsigned int power, struct BigInt* out);
void PowmBigIntChar(unsigned char base, struct BigInt* mod, unsigned int power, struct BigInt* out);
