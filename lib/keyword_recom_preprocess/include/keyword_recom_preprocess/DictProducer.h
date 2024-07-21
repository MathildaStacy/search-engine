#pragma once

#include <cstdint>
#include <iostream>
#include <iterator>
#include <queue>
#include <set>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "example/add.hpp"
#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "cppjieba/Jieba.hpp"
#include "config/config.hpp"



class DictProducer {
public:
    DictProducer();

    void setLogger(std::shared_ptr<spdlog::logger> my_logger);

    virtual ~DictProducer();

    void prepare();

    std::vector<std::string> run(const std::string& user_input);


private:
    std::unordered_set<std::string> LoadStopWords(const std::string& stopWordPath);

    
    std::string readFileIntoString(const std::string& path);


    std::vector<std::string> splitUTF8(const std::string& str);

    std::vector<std::string> chinese_run(const std::string& user_input);

    std::vector<std::string> english_run(const std::string& user_input);

    //去除停用词
    void removeStopWords(std::vector<std::string>& words);
private:
    std::shared_ptr<spdlog::logger> m_my_logger;
    cppjieba::Jieba *m_jieba_ptr;
    std::vector<std::string> m_words;
    
    //停用词
    std::unordered_set<std::string> m_stop_words;

    //生成每个词和它所对应的词频 中文
    std::unordered_map<std::string, uint64_t> m_chinese_words_and_its_frequency;
    //生成每个字符和它所出现的单词  中文
    std::unordered_map<std::string, std::set<std::string>> m_chinese_char_and_its_words;

    //生成每个词和它所对应的词频 英文
    std::unordered_map<std::string, uint64_t> m_english_words_and_its_frequency;
    //生成每个字符和它所出现的单词  英文
    std::unordered_map<std::string, std::set<std::string>> m_english_char_and_its_words;

};