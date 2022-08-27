#include "rsa.h"

//Calculates phi(n) = (p-1)(q-1)
void PHI(struct BigInt* p, struct BigInt* q, struct BigInt* res) {
    struct BigInt one;
    BigIntFactoryRef(&one);
    one.data[0] = 1;

    SubBigInt(p, &one);
    SubBigInt(q, &one);

    MulBigInt256(p, q, res);

    AddBigInt(p, &one);
    AddBigInt(q, &one);
}

//Finds d such that e*d % phi(n) == 1
void PrivKey(struct BigInt* p, struct BigInt* q, unsigned int e, struct BigInt* out) {
    struct BigInt* a = BigIntFactoryBlocks(BI_256, 0);
    struct BigInt* b = BigIntFactoryBlocks(BI_256, 0);
    struct BigInt* gcd = BigIntFactoryBlocks(BI_256, 0);
    struct BigInt* x = BigIntFactoryBlocks(BI_256, 0);
    struct BigInt* y = BigIntFactoryBlocks(BI_256, 0);

    PHI(p, q, a);
    *(unsigned int*)b->data = e;

    EuclideanGCD(gcd, a, b, x, y);

    if (y->sign) {
        AddBigInt(a, y);
    }


    memcpy(out, a, sizeof(struct BigInt));

    free(a);
    free(b);
    free(gcd);
    free(x);
    free(y);
}

struct RSA* RSAFactory(struct BigInt* p, struct BigInt* q, unsigned int e, rsaKeySize sz)
{
    struct BigInt buf;
    struct BigInt one;
    BigIntFactoryRef(&one);
    one.data[0] = 1;

    if (IsBiggerBigInt(q, p)) {
        struct BigInt* temp = p;
        p = q;
        q = temp;
    }

    struct RSA* rsa = malloc(sizeof(struct BigInt));
    if (!rsa) goto end;

    rsa->pubKey = malloc(sizeof(struct PubKey));
    rsa->privKey = malloc(sizeof(struct PrivKey));
    if (!(rsa->pubKey && rsa->privKey))
        goto end;

    rsa->pubKey->e = e;
    rsa->pubKey->n = BigIntFactory(sz, 0);
    rsa->privKey->phin = BigIntFactory(sz, 0);
    rsa->privKey->d = BigIntFactory(sz, 0);
    rsa->privKey->p = BigIntFactory(sz, 0);
    rsa->privKey->q = BigIntFactory(sz, 0);
    rsa->privKey->dP = BigIntFactory(sz, 0);
    rsa->privKey->dQ = BigIntFactory(sz, 0);
    rsa->privKey->qInv = BigIntFactory(sz, 0);

    MulBigInt256(p, q, rsa->pubKey->n->data);

    PHI(p, q, rsa->privKey->phin);
    PrivKey(p, q, rsa->pubKey->e, rsa->privKey->d);

    CopyBigInt(rsa->privKey->q, q);
    CopyBigInt(rsa->privKey->p, p);
    
    SubBigInt(q, &one);
    SubBigInt(p, &one);

    CopyBigInt(rsa->privKey->dP, rsa->privKey->d);
    CopyBigInt(rsa->privKey->dQ, rsa->privKey->d);

    ModBigInt(rsa->privKey->dP, p);
    ModBigInt(rsa->privKey->dQ, q);

    AddBigInt(q, &one);
    AddBigInt(p, &one);

    ZeroBigInt(rsa->privKey->qInv);
    EuclideanGCD(&buf, q, p, rsa->privKey->qInv, &buf);
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

char* RSADecrypt2(struct RSA* rsa, struct BigInt* mes) {
    struct BigInt out;
    BigIntFactoryRef(&out);
    char* outChars = malloc(512);
    ZeroMemory(outChars, 512);

    PowmBigInt(mes, rsa->pubKey->n, rsa->privKey->d, &out);

    memcpy(outChars, out.data, 256);

    return outChars;
}

struct BigInt* RSADecrypt(struct RSA* rsa, struct BigInt* mes) {
    struct BigInt m1;
    BigIntFactoryRef(&m1);
    struct BigInt m2;
    BigIntFactoryRef(&m2);
    struct BigInt h;
    BigIntFactoryRef(&h);
    struct BigInt mesP;
    CopyBigInt(&mesP, mes);
    ModBigInt(&mesP, rsa->privKey->p);
    struct BigInt mesQ;
    CopyBigInt(&mesQ, mes);
    ModBigInt(&mesQ, rsa->privKey->q);

    PowmBigInt(&mesQ, rsa->privKey->q, rsa->privKey->dQ, &m2);
    PowmBigInt(&mesP, rsa->privKey->p, rsa->privKey->dP, &m1);
    //CopyBigInt(&m1, &m2);
    struct BigInt* out = BigIntFactoryMove(&m1);
    SubBigInt(out, &m2);
    MulBigInt256(rsa->privKey->qInv, out, &h);
    ModBigInt(&h, rsa->privKey->p);

    ZeroBigInt(out);
    MulBigInt256(rsa->privKey->q, &h, out);
    AddBigInt(out, &m2);

    return out;
}

void RSADispose(struct RSA* p) {
    PubKeyDispose(p->pubKey);
    PrivKeyDispose(p->privKey);
    free(p);
}

void PubKeyDispose(struct PubKey* p) {
    free(p->n);
    free(p);
}

void PrivKeyDispose(struct PrivKey* p) {
    free(p->d);
    free(p->dP);
    free(p->dQ);
    free(p->phin);
    free(p->p);
    free(p->q);
    free(p->qInv);
    free(p);
}