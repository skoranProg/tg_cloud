#include "AES.h"
#include <iostream>

using namespace CryptoPP;

int AES_file_encryptor::encrypt(const std::string &path, const std::string &output) {
    try
    {
        GCM< AES >::Encryption e;
        e.SetKeyWithIV( keys_->key, keys_->key.size(), keys_->iv, keys_->iv.size() );

        FileSource(path.c_str(), true,
                   new AuthenticatedEncryptionFilter( e,
                                                     new FileSink(output.c_str(), true  ), false, TAG_SIZE
                                                     )
        );
    }
    catch( CryptoPP::Exception& e )
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}

int AES_file_encryptor::decrypt(const std::string &path, const std::string &output) {
    try
    {
        GCM< AES >::Decryption d;
        d.SetKeyWithIV( keys_->key, keys_->key.size(), keys_->iv, keys_->iv.size() );

        AuthenticatedDecryptionFilter df( d,
                                         new FileSink( output.c_str(), true   ),
                                         AuthenticatedDecryptionFilter::DEFAULT_FLAGS, TAG_SIZE
        );
        FileSource( path.c_str(), true,
                   new Redirector(df)
        );

        if(!df.GetLastResult()) {
            std::cerr << "Corrupt data's integrity " << std::endl;
            return 2;
        }
    }
    catch( CryptoPP::Exception& e )
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}

int Encryption_Keys::LoadIntoFile(const std::string &path) {
    try {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open key file" << std::endl;
            return 1;
        }
        file.write(reinterpret_cast<const char*>(key.BytePtr()), key.size());
        file.write(reinterpret_cast<const char*>(iv.BytePtr()), iv.size());
        return 0;
    } catch (Exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

int Encryption_Keys::LoadFromFile(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (size != FILE_SIZE_) {
        std::cerr << "Incorrect key file: wrong size" << std::endl;
        return 1;
    }
    file.read(reinterpret_cast<char*>(key.begin()), AES::DEFAULT_KEYLENGTH);
    file.read(reinterpret_cast<char*>(iv.begin()), size - AES::DEFAULT_KEYLENGTH);
    return 0;
}

int Encryption_Keys::GenerateKeys() {
    AutoSeededRandomPool prng;
    prng.GenerateBlock( key, key.size() );
    prng.GenerateBlock( iv, iv.size() );
    return 0;
}
