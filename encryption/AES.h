#ifndef TG_CLOUD_AES_H
#define TG_CLOUD_AES_H

#include "encrypt_file.h"
#include <cryptopp/cryptlib.h>
#include <cryptopp/filters.h>
#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>
#include <cryptopp/osrng.h>
#include <cryptopp/files.h>

#include <utility>

using namespace CryptoPP;

/// TODO: Good testing for this class tho

class Encryption_Keys {
 public:
    Encryption_Keys() : key(AES::DEFAULT_KEYLENGTH), iv(AES::BLOCKSIZE * 16) {
    }

    int GenerateKeys();

    int LoadIntoFile(const std::string &path);

    int LoadFromFile(const std::string &path);

    SecByteBlock key;

    SecByteBlock iv;
 private:
    const int FILE_SIZE_ = AES::DEFAULT_KEYLENGTH + AES::BLOCKSIZE * 16;
};

class AES_file_encryptor : file_encryptor {
 public:
    AES_file_encryptor(Encryption_Keys* keys) : keys_(keys) {
    }

    int encrypt(const std::string &path, const std::string &output) override;

    int decrypt(const std::string &path, const std::string &output) override;

 private:
    const int TAG_SIZE = 12;

    Encryption_Keys* keys_;
};



#endif  // TG_CLOUD_AES_H
