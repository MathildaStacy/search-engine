#pragma once

#include "cache/urlLRUCache.h"
#include <cstddef>
#include <vector>


class UrlCacheManager {
public:

    UrlCacheManager(size_t num, size_t capacity);

    //获取缓存数组中所有下标为奇数的缓存的引用
    std::vector<std::reference_wrapper<UrlLRUCache>> getEvenIndexedCaches();

    //给定一个LRUCache的数组，合并所有的LRUCache的内容为一个新的LRUCache，返回这个Cache
    UrlLRUCache mergeLRUCaches(const std::vector<std::reference_wrapper<UrlLRUCache>>& caches);

    // 把所有奇数下标的LRUCache，拷贝到它们紧邻的偶数下标
    void updateAllEvenLRUCache();


    //同步所有的奇数下标缓存
    void synchronizeAllOddLRUCache();


    //查询缓存
    std::vector<std::vector<std::string>> query(size_t index, const std::string& key);

    //插入缓存
    void insert(size_t index, const std::string& key, std::vector<std::vector<std::string>>& value);
private:
    std::vector<UrlLRUCache> m_caches; //从1开始，奇数是线程更新的缓存，偶数是主线程用来合并的temp缓存，每一个相邻的奇数和偶数是一对缓存

};