#include <iostream>
#include <string>
#include <aes.h>
#include <filters.h>
#include <modes.h>
#include <hex.h>

int main() {
    using namespace CryptoPP;

    // Secret Key and Initialization Vector (IV) setup
    byte key[AES::DEFAULT_KEYLENGTH], iv[AES::BLOCKSIZE];
    memset(key, 0x01, AES::DEFAULT_KEYLENGTH);
    memset(iv, 0x01, AES::BLOCKSIZE);

    // Message to be encrypted
    std::string plainText = "Hello, Crypto++!";
    std::string cipherText, recoveredText;

    // Encryption
    try {
        ECB_Mode< AES >::Encryption e;
        e.SetKey(key, sizeof(key));

        // The StreamTransformationFilter removes
        //  padding as required.
        StringSource ss1(plainText, true, 
            new StreamTransformationFilter(e,
                new StringSink(cipherText)
            ) // StreamTransformationFilter      
        ); // StringSource
    }
    catch(const CryptoPP::Exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    // Decryption
    try {
        ECB_Mode< AES >::Decryption d;
        d.SetKey(key, sizeof(key));

        // The StreamTransformationFilter removes
        //  padding as required.
        StringSource ss2(cipherText, true, 
            new StreamTransformationFilter(d,
                new StringSink(recoveredText)
            ) // StreamTransformationFilter
        ); // StringSource

        std::cout << "Plain Text: " << plainText << std::endl;
        std::cout << "Cipher Text: " << cipherText << std::endl;
        std::cout << "Recovered Text: " << recoveredText << std::endl;
    }
    catch(const CryptoPP::Exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    return 0;
}
