extern "C" {
#include "rsa.h"
}

#include <iostream>
#include <stdio.h>
#include <string.h>

int main()
{
    uint64_t P[] = {       
        0xdd3343cbc08f31db, 0xd011d17f3a80ab71, 0xb7120951b5d8ea73, 0xba8bb9ecac1664b8, 
        0x23e569890c3615da, 0x127735cd68db492d, 0xbcf5561a2442f1bd, 0x5450753679960cfa, 
        0x2a0919997b38c44c, 0xf319f80962d8a5c9, 0x056a061fa6714c59, 0x2c34e7481270ac99,
        0x3932e73c4a5d9675, 0x42f3c951b5be3225, 0xb5fa2d50bf4a17bc, 0x2d513a9b4b033825
    };

    uint64_t Q[] = {
        0xc0b264accb673bca, 0xb4e7876368e0d242, 0x2cee9a7b7c25542d, 0xb01574ccc81fb06d,
        0x702deeae76affb03, 0xa4e5b3d241893dcd, 0x2c458dbc38488c39, 0x99a1a16abb07aae3, 
        0x219d8518afc3f791, 0xd295800cd0a865ea, 0x92ce4e88fbffd383, 0x6b186a6a23fcb87e,
        0x87812de03a877866, 0x58cd6390fa7d0fb1, 0xf62dd2a4edde05ba, 0x22ab58ccb4ffd08b
    };

    BigInt* p = BigIntFactoryBlocks(BI_128, P);
    BigInt* q = BigIntFactoryBlocks(BI_128, Q);

    unsigned int e = 0x10001;

    RSA* rsa = RSAFactory(p, q, e, RSA_256);

    char mes[512] = { 0 };
    memcpy(mes, "T", 2);

    //char mes[] = "XKhXTtNVbka98rysyfIZHMC5zn7m15vqDIvbOp5X4KYxkNnVICs4L8iorvF7azzB2Tr5wYRjsY2gWsFKk1Grh85NbdBofcmxGzQjO61W9Dnd7x84eV3PB9R9RSZe9BGO81OtbzcTnZH4COuXbbhi4QQ07dH8wxE2cEtMzxtSFR2uUK9xJ4euC3jNB2SD7OLoJn0kc11J5SPMQQNzthsuxKubwYpQXZLqKJxRoCAKRIqcznOCjL43hpy4JdITT2F";

    BigInt* res = RSACrypt(rsa, mes);
    char* out = RSADecrypt(rsa, res);

    if (!(memcmp(mes, out, 256))) {
        std::cout << "RSA test passed!\n";
    }
    else {
        std::cout << "RSA test failed!\n";
    }
    return 0;
}