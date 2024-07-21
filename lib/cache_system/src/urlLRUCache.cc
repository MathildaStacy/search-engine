#include "cache/urlLRUCache.h"



UrlLRUCache::UrlLRUCache(int capacity) : lru_size(capacity) {}


UrlLRUCache::UrlLRUCache(const UrlLRUCache& other) {
    std::lock_guard<std::mutex> lock(other.mtx); // 锁定原对象以进行安全拷贝
    lru_cache = other.lru_cache;
    lru_size = other.lru_size;

    // 重建unordered_map，因为迭代器指向的是新list中的元素
    for (auto it = lru_cache.begin(); it != lru_cache.end(); ++it) {
        lru[std::get<0>(*it)] = it;
    }
}


// 拷贝赋值运算符
UrlLRUCache& UrlLRUCache::operator=(const UrlLRUCache& other) {
    if (this == &other) return *this; // 检查自赋值

    std::lock(mtx, other.mtx); // 锁定两个对象以防止死锁
    std::lock_guard<std::mutex> self_lock(mtx, std::adopt_lock);
    std::lock_guard<std::mutex> other_lock(other.mtx, std::adopt_lock);

    lru_cache = other.lru_cache;
    lru_size = other.lru_size;

    // 清除旧的unordered_map并重建，因为迭代器指向的是新list中的元素
    lru.clear();
    for (auto it = lru_cache.begin(); it != lru_cache.end(); ++it) {
        lru[std::get<0>(*it)] = it;
    }

    return *this;
}


std::vector<std::vector<std::string>> UrlLRUCache::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx); // 上锁
    if (lru.find(key) != lru.end()) {
        auto it = lru[key];

        auto temp = *it;
        std::vector<std::vector<std::string>> value = std::get<1>(*it);

        lru_cache.erase(it);

        lru_cache.emplace_front(key, value);
        lru[key] = lru_cache.begin();

        return value;
    } else {
        return {}; // 返回一个空的vector
    }
}

void UrlLRUCache::put(const std::string& key, const std::vector<std::vector<std::string>>& value) {
    std::lock_guard<std::mutex> lock(mtx); // 上锁
    if (lru.find(key) != lru.end()) {
        auto it = lru[key];

        lru_cache.erase(it);

        lru_cache.emplace_front(key, value);
        lru[key] = lru_cache.begin();
    } else {
        lru_cache.emplace_front(key, value);
        lru[key] = lru_cache.begin();

        if (lru_cache.size() > lru_size) {
            auto tuple1 = lru_cache.back();
            lru_cache.pop_back();

            lru.erase(std::get<0>(tuple1));
        }
    }
}

std::vector<std::tuple<std::string, std::vector<std::vector<std::string>>>> UrlLRUCache::getAllPairs() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::tuple<std::string, std::vector<std::vector<std::string>>>> pairs;
    for (const auto& item : lru_cache) {
        pairs.push_back(item);
    }
    return pairs;
}

size_t UrlLRUCache::get_size() {
    return lru_size;
}