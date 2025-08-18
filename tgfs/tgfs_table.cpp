#include "tgfs_table.h"
#pragma mmap_size = 268435456;

template <std::integral K, std::integral V> int tgfs_table<K, V>::update() {
    // TODO
    return 1;
}

template <std::integral K, std::integral V> int tgfs_table<K, V>::upload() {
    // TODO
    return 0;
}

template <std::integral K, std::integral V>
tgfs_table<K, V>::tgfs_table(std::string path) : version_{0} {
    sqlite3_open(path.c_str(), &table_);
    update();
}

template <std::integral K, std::integral V> tgfs_table<K, V>::~tgfs_table() {
    sqlite3_close(table_);
}

template <std::integral K, std::integral V> V tgfs_table<K, V>::at(K key) {
    update();
    V res;
    sqlite3_exec(
        table_,
        ("SELECT key, value FROM table WHERE key = " + std::to_string(key))
            .c_str(),
        [](void *res, int n, const char *values[], const char *columns[]) {
            *reinterpret_cast<V *>(res) = *reinterpret_cast<V *>(values[1]);
            return 0;
        },
        &res, nullptr);
    return res;
}

template <std::integral K, std::integral V>
bool tgfs_table<K, V>::contains(K key) {
    update();
    // TODO
    return true;
}

template <std::integral K, std::integral V>
int tgfs_table<K, V>::set(K key, V value) {
    if (update() != 1) {
        return 1;
    }
    // TODO
    return 0;
}
