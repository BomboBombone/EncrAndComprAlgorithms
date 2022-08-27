#pragma once
#include "bigint.h"

// lhs * x + rhs * y = gcd(lhs, rhs)
void EuclideanGCD(struct BigInt* gcd, struct BigInt* lhs, struct BigInt* rhs, struct BigInt* x, struct BigInt* y);