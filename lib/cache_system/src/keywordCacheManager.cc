#include "cache/keywordCacheManager.h"
#include "cache/keywordLRUCache.h"
#include <cstddef>
#include <functional>
#include <string>
#include <vector>



KeywordCacheManager::KeywordCacheManager(size_t num, size_t capacity) {
    for(size_t i = 0; i < num; ++i) {
        m_caches.emplace_back(capacity);
    }
}

std::vector<std::reference_wrapper<KeywordLRUCache>> KeywordCacheManager::getEvenIndexedCaches() {
    std::vector<std::reference_wrapper<KeywordLRUCache>> evenIndexedCaches;
    for (size_t i = 2; i < m_caches.size(); i += 2) {
        evenIndexedCaches.push_back(std::ref(m_caches[i]));
    }
    return evenIndexedCaches;
}

KeywordLRUCache KeywordCacheManager::mergeLRUCaches(const std::vector<std::reference_wrapper<KeywordLRUCache>>& caches) {
    int newCapacity = caches[0].get().get_size();
    KeywordLRUCache mergedCache(newCapacity);
    for (const auto& cache : caches) {
        auto pairs = cache.get().getAllPairs(); // 假设已经修改了LRUCache以支持这个操作
        for (const auto& pair : pairs) {
            mergedCache.put(std::get<0>(pair), std::get<1>(pair));
        }
    }
    return mergedCache;
}

// 把所有奇数下标的LRUCache，拷贝到它们紧邻的偶数下标
void KeywordCacheManager::updateAllEvenLRUCache() {
    for (size_t i = 1; i < m_caches.size(); i += 2) {
        // 检查是否存在紧邻的偶数下标缓存
        if (i + 1 < m_caches.size()) {
            m_caches[i + 1] = m_caches[i];
        }
    }
}


void KeywordCacheManager::synchronizeAllOddLRUCache() {
    updateAllEvenLRUCache();
    std::vector<std::reference_wrapper<KeywordLRUCache>> rst = getEvenIndexedCaches();

    KeywordLRUCache temp = mergeLRUCaches(rst);


    for(size_t i = 1; i < m_caches.size(); i += 2) {
        m_caches[i] = temp;
    }

}


std::vector<std::string> KeywordCacheManager::query(size_t index, const std::string& key) {
    return m_caches[index].get(key);
}


void KeywordCacheManager::insert(size_t index, const std::string& key, std::vector<std::string> &value) {
    m_caches[index].put(key, value);
}