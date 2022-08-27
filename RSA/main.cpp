extern "C" {
#include "rsa.h"
}

#include <iostream>
#include <stdio.h>
#include <string.h>

int main()
{
    //dd3343cbc08f31dbd011d17f3a80ab71b7120951b5d8ea73ba8bb9ecac1664b823e569890c3615da127735cd68db492dbcf5561a2442f1bd5450753679960cfa2a0919997b38c44cf319f80962d8a5c9056a061fa6714c592c34e7481270ac993932e73c4a5d967542f3c951b5be3225b5fa2d50bf4a17bc2d513a9b4b033825
    //c0b264accb673bcab4e7876368e0d2422cee9a7b7c25542db01574ccc81fb06d702deeae76affb03a4e5b3d241893dcd2c458dbc38488c3999a1a16abb07aae3219d8518afc3f791d295800cd0a865ea92ce4e88fbffd3836b186a6a23fcb87e87812de03a87786658cd6390fa7d0fb1f62dd2a4edde05ba22ab58ccb4ffd08b
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

    //uint64_t P[] = {
    //    0, 0, 0, 0,
    //    0, 0, 0, 0,
    //    0, 0, 0, 0,
    //    0, 0, 0, 0x9c5db1fa679f6aeb
    //};
    //
    //uint64_t Q[] = {
    //    0, 0, 0, 0,
    //    0, 0, 0, 0,
    //    0, 0, 0, 0,
    //    0, 0, 0, 0x95a8f09bc8de881f
    //};

    BigInt* p = BigIntFactoryBlocks(BI_128, P);
    BigInt* q = BigIntFactoryBlocks(BI_128, Q);

    unsigned int e = 0xb;

    RSA* rsa = RSAFactory(p, q, e, RSA_256);

    char mes[512] = { 0 };
    ((unsigned int*)mes)[0] = 0x7cafff;

    //char mes[] = "XKhXTtNVbka98rysyfIZHMC5zn7m15vqDIvbOp5X4KYxkNnVICs4L8iorvF7azzB2Tr5wYRjsY2gWsFKk1Grh85NbdBofcmxGzQjO61W9Dnd7x84eV3PB9R9RSZe9BGO81OtbzcTnZH4COuXbbhi4QQ07dH8wxE2cEtMzxtSFR2uUK9xJ4euC3jNB2SD7OLoJn0kc11J5SPMQQNzthsuxKubwYpQXZLqKJxRoCAKRIqcznOCjL43hpy4JdITT2F";

    BigInt* res = RSACrypt(rsa, mes);
    BigInt* out = RSADecrypt(rsa, res);

    if (!(memcmp(mes, out->data, 256))) {
        std::cout << "RSA test passed!\n";
    }
    else {
        std::cout << "RSA test failed!\n";
    }

    free(p);
    free(q);
    free(out);
    RSADispose(rsa);
    return 0;
}