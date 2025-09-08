#pragma once

#include <filesystem>
#include <fstream>

#include "td_helpers.h"
#include "tdclient.h"

class Test {
 public:
    virtual ~Test() = default;

    virtual bool test() = 0;

    virtual void print_test() = 0;
};

class Test_file : public Test {
 public:
    explicit Test_file(const std::string &path, TdClass *td_client)
        : path_(path), td_client_(td_client) {}

    bool test() override {
        auto dw_path = LoadFile();
        bool answer = CompareFiles(path_ + ".tst", dw_path);
        std::filesystem::remove(dw_path);
        std::filesystem::rename(path_ + ".tst", path_);
        return true;
    }

    void print_test() override {
        std::cout << "Test: " << path_ << std::endl;
    }

 private:
    static bool CompareFiles(const std::string &path1,
                             const std::string &path2);

    std::string LoadFile() const;

    TdClass *td_client_;

    std::string path_;
};

std::unordered_map<fuse_ino_t, uint64_t> GenerateRandomTable(size_t size);

class Test_table : public Test {
 public:
    explicit Test_table(const size_t table_size, TdClass *td_client)
        : table_size_(table_size), td_client_(td_client) {}

    bool test() override;

    void print_test() override {
        std::cout << "Test table with size: " << table_size_ << std::endl;
    }

 private:
    size_t table_size_;

    TdClass *td_client_;
};

void RunTest(TdClass *td_class,
             const std::vector<std::shared_ptr<Test>> &tests);

void TestFileUpload(TdClass *td_class, const std::vector<std::string> &paths);

void TestFileTableUpload(TdClass *td_class, const std::vector<size_t> &sizes);
