#pragma once



#include <list>
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <mutex> // 引入互斥量

class UrlLRUCache {
public:
    UrlLRUCache(int capacity);

    UrlLRUCache(const UrlLRUCache& other);

    // 拷贝赋值运算符
    UrlLRUCache& operator=(const UrlLRUCache& other);



public:
    std::vector<std::vector<std::string>> get(const std::string& key);
    void put(const std::string& key, const std::vector<std::vector<std::string>>& value);
    std::vector<std::tuple<std::string, std::vector<std::vector<std::string>>>> getAllPairs();

    size_t get_size();

private:
    std::list<std::tuple<std::string, std::vector<std::vector<std::string>>>> lru_cache;
    std::unordered_map<std::string, std::list<std::tuple<std::string, std::vector<std::vector<std::string>>>>::iterator> lru;
    size_t lru_size;
    mutable std::mutex mtx; // 互斥量，用于同步对LRU缓存的访问
};


