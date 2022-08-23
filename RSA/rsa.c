#include "rsa.h"

//Calculates phi(n) = (p-1)(q-1)
void PHI(struct BigInt* p, struct BigInt* q, struct BigInt* res) {
    struct BigInt one;
    BigIntFactoryRef(&one);
    one.data[0] = 1;

    SubBigInt(p, &one);
    SubBigInt(q, &one);

    MulBigInt256(p, q, res);
}

//Finds d such that e*d % phi(n) == 1
void PrivKey(struct BigInt* p, struct BigInt* q, unsigned int e, struct BigInt* out) {
    struct BigInt one;
    one.type = p->type;
    ZeroMemory(one.data, 512);
    one.data[0] = 1;

    struct BigInt* a = BigIntFactoryBlocks(BI_256, 0);
    struct BigInt* b = BigIntFactoryBlocks(BI_256, 0);
    struct BigInt* gcd = BigIntFactoryBlocks(BI_256, 0);
    struct BigInt* x = BigIntFactoryBlocks(BI_256, 0);
    struct BigInt* y = BigIntFactoryBlocks(BI_256, 0);

    PHI(p, q, a);
    *(unsigned int*)b->data = e;

    EuclideanGCD(gcd, a, b, x, y);

    ZeroMemory(x->data, 512);
    x->data[8] = 1;

    SubBigInt(x, y);
    SubBigInt(a, x);

    memcpy(out, a, sizeof out);

    free(a);
    free(b);
    free(gcd);
    free(x);
    free(y);
}

struct RSA* RSAFactory(struct BigInt* p, struct BigInt* q, unsigned int e, rsaKeySize sz)
{
    struct RSA* rsa = malloc(sizeof rsa);
    if (!rsa) goto end;

    rsa->pubKey = malloc(sizeof rsa->pubKey);
    rsa->privKey = malloc(sizeof rsa->privKey);
    if (!(rsa->pubKey && rsa->privKey))
        goto end;

    rsa->pubKey->e = e;
    rsa->pubKey->n = BigIntFactory(sz, 0);
    rsa->privKey->n = BigIntFactory(sz, 0);
    rsa->privKey->d = BigIntFactory(sz, 0);

    MulBigInt256(p, q, rsa->pubKey->n);

    memcpy(rsa->privKey->n, rsa->pubKey->n, sizeof rsa->privKey->n);
    PrivKey(p, q, rsa->pubKey->e, rsa->privKey->d);

end:
    return rsa;
}

struct BigInt* RSACrypt(struct RSA* rsa, char* mes) {
    struct BigInt* buf = BigIntFactory(BI_512, mes);
    struct BigInt* out = BigIntFactory(BI_512, 0);
    struct BigInt* e = BigIntFactory(BI_256, 0);

    *(unsigned int*)e->data = rsa->pubKey->e;

    PowmBigInt(buf, rsa->pubKey->n, e, out);

    free(buf);
    free(e);

    return out;
}

char* RSADecrypt(struct RSA* rsa, struct BigInt* mes) {
    char* out = malloc(512);
    ZeroMemory(out, 512);

    PowmBigInt(mes, rsa->pubKey->n, rsa->privKey->d, out);

    return out;
}