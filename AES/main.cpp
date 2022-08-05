extern "C" {
#include "aes.h"
}

#include <iostream>

int main()
{
    unsigned char plain[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                         0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
    unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                           0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    unsigned char right[] = { 0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
                             0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a };
    AES* aes = AESFactory(AES_128);
    auto out = EncryptECB(aes, plain, 16, key);
    if (!memcmp(right, out, 16)) {
        std::cout << "Match!" << std::endl;
    }
    else {
        std::cout << "Not match!" << std::endl;
    }
    system("pause");
    
    return 0;
}