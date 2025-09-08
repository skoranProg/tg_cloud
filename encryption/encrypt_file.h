#ifndef TG_CLOUD_ENCRYPT_FILE_H
#define TG_CLOUD_ENCRYPT_FILE_H

#include <string>

class file_encryptor {
 public:
    virtual int encrypt(const std::string &path, const std::string &output) = 0;

    virtual int decrypt(const std::string &path, const std::string &output) = 0;
};

#endif  // TG_CLOUD_ENCRYPT_FILE_H
