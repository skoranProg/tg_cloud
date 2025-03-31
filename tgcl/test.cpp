#include "test.h"
#include "../tgfs/tgfs_data.h"

#include <random>

bool Test_file::CompareFiles(const std::string &path1, const std::string &path2) {
    std::ifstream ifs1(path1, std::ios::binary|std::ifstream::ate);
    std::ifstream ifs2(path2, std::ios::binary|std::ifstream::ate);

    if (ifs1.fail() || ifs2.fail() || (ifs1.tellg() != ifs2.tellg())) {
        return false;
    }

    ifs1.seekg(0, std::ios_base::beg);
    ifs2.seekg(0, std::ios_base::beg);
    return std::equal(std::istreambuf_iterator<char>(ifs1.rdbuf()), std::istreambuf_iterator<char>() ,std::istreambuf_iterator<char>(ifs2.rdbuf()));
}

std::string Test_file::LoadFile() const {
    td_client_->SendFile(td_client_->GetMainChatId(), path_);
    std::filesystem::rename(path_, path_ + ".tst");
    auto mes = td_client_->GetLastMessage(td_client_->GetMainChatId());
    return DownloadFileFromMes(td_client_, std::move(mes))->local_->path_;
}

void RunTest(TdClass *td_class, const std::vector<std::shared_ptr<Test>> &tests) {
    std::cout << "Running Tests: " << std::endl;
    for (auto &test : tests) {
        test->print_test();
        if (test->test()) {
            std::cout << "Success" << std::endl;
        } else {
            std::cout << "Failed" << std::endl;
        }
    }
}

void TestFileUpload(TdClass *td_class, const std::vector<std::string> &paths)  {
    std::vector<std::shared_ptr<Test>> tests;
    for (auto &path : paths) {
        tests.push_back(std::make_shared<Test_file>(path, td_class));
    }
    RunTest(td_class, tests);
}

std::unordered_map<fuse_ino_t, uint64_t> GenerateRandomTable(const size_t size) {
    std::mt19937_64 rnd{142};
    std::unordered_map<fuse_ino_t, uint64_t> table;
    for (auto i = 0; i < size; i++) {
        table.insert(std::make_pair(rnd(), rnd()));
    }
    return table;
}

bool Test_table::test() {
    auto tb = GenerateRandomTable(table_size_);
    TableTdUpdater tu(td_client_);
    tu.upload_table(tb);
    const auto res = tu.get_table();
    return res == tb;
}

void TestFileTableUpload(TdClass *td_class, const std::vector<size_t> &sizes) {
    std::vector<std::shared_ptr<Test>> tests;
    for (auto &sz : sizes) {
        tests.push_back(std::make_shared<Test_table>(sz, td_class));
    }
    RunTest(td_class, tests);
}
