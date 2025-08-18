#include "tgfs_table.h"
#pragma mmap_size = 268435456;

template <class K, class V> int tgfs_table<K, V>::update() {
    // TODO
    return 0;
}

template <class K, class V> int tgfs_table<K, V>::upload() {
    // TODO
    return 0;
}

template <class K, class V> tgfs_table<K, V>::tgfs_table(std::string path) {}

template <class K, class V> V tgfs_table<K, V>::at(K key) {}

template <class K, class V> bool tgfs_table<K, V>::contains(K key) {
    update();
    // TODO
    return true;
}

template <class K, class V> int tgfs_table<K, V>::set(K key, V value) {
    // TODO
    return 0;
}
