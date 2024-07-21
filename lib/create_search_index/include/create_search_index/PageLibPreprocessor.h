#pragma once


#include <cstdint>
#include <string>
#include <vector>
#include "spdlog/common.h"
#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "simhash/Simhasher.hpp"
#include "config/config.hpp"
#include "cppjieba/Jieba.hpp"



struct WebPage {
    uint64_t docid;
    std::string title;
    std::string url;
    std::string content;
};

struct RstArticle {
    std::vector<std::string> words;
    std::vector<double> base_weight_vector;
    std::vector<double> article_weight_vector;
    int articleId;

    

    double cos() const;
};

class PageLibPreprocessor {
public:
    PageLibPreprocessor();

    virtual ~PageLibPreprocessor();

    void setLogger(std::shared_ptr<spdlog::logger> my_logger);

    bool readDocById(const uint64_t docId, WebPage& webpage);

    int remove_duplicates_and_rewrite_pageLib();



    int WriteUpPageLib();

    int createInvertedIndex();

    int loadInvertedIndex();

    std::vector<std::vector<std::string>> QueryPage(const std::string & sentence);


    std::vector<std::string> splitUTF8(const std::string& str);
private:

    void insertIfUnique(std::vector<WebPage>& pages, const WebPage& page);

    std::unordered_set<std::string> LoadStopWords(const std::string& stopWordPath);

    std::unordered_map<int, std::map<std::string, double>> transform(const std::unordered_map<std::string, std::map<int, double>>& original);

    //去除停用词，空字符串和空格

    void removeStopWords(std::vector<std::string>& words);

private:
    std::shared_ptr<spdlog::logger> m_my_logger;

    cppjieba::Jieba *m_jieba_ptr;

    //停用词
    std::unordered_set<std::string> m_stop_words;

    std::unordered_map<std::string, std::map<int, double>> m_inverted_index;

    std::unordered_map<int, std::map<std::string, double>> m_transformed_inverted_index;

    //simhash::Simhasher *m_simhash_ptr;

};