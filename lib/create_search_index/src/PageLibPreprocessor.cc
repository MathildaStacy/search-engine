#include "create_search_index/PageLibPreprocessor.h"
#include "simhash/Simhasher.hpp"
#include "spdlog/spdlog.h"
#include "tinyxml2/tinyxml2.h"
#include "config/config.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <cmath>
#include <cctype>
#include <nlohmann/json.hpp>

simhash::Simhasher simhasher(DICT_PATH, HMM_PATH, IDF_PATH, STOP_WORD_PATH);

//工具函数：取前25个汉字
std::vector<std::string> getFirst25Elements(const std::vector<std::string>& words) {
    // 如果原始向量的大小小于或等于25，则直接返回原向量
    if (words.size() <= 25) {
        return words;
    } else {
        // 否则，返回包含前25个元素的新向量
        // 使用迭代器或下标来实现
        return std::vector<std::string>(words.begin(), words.begin() + 25);
    }
}

bool operator<(const RstArticle& one, const RstArticle& other) {
        double me_cos = 0.0;
        double other_cos = 0.0;

        me_cos = one.cos();
        other_cos = other.cos();

        return me_cos < other_cos;

}



double RstArticle::cos() const {
    double numerator = 0.0;
    double denominator = 0.0;

    for(uint64_t i = 0; i < base_weight_vector.size(); ++i) {
        numerator += base_weight_vector[i] * article_weight_vector[i]; 
    }

    double mo1 = 0.0;
    double mo2 = 0.0;

    for(uint64_t i = 0; i < base_weight_vector.size(); ++i) {
        mo1 += base_weight_vector[i] * base_weight_vector[i];
    }

    mo1 = sqrt(mo1);
    mo1 = fabs(mo1);

    for(uint64_t i = 0; i < article_weight_vector.size(); ++i) {
        mo2 += article_weight_vector[i] * article_weight_vector[i];
    }

    mo2 = sqrt(mo2);
    mo2 = fabs(mo2);

    denominator = mo1 * mo2;

    return numerator / denominator;


}

bool operator==(const WebPage& w1, const WebPage& w2) {
    size_t topN = 5;
    uint64_t u641 = 0;
    uint64_t u642 = 0;

    std::vector<std::pair<std::string, double>> res; //单词-该单词的权重
    simhasher.extract(w1.content, res, topN);
    simhasher.make(w1.content, topN, u641);

    res.clear();

    simhasher.extract(w2.content, res, topN);
    simhasher.make(w2.content, topN, u642);


    return simhash::Simhasher::isEqual(u641, u642);
}


bool operator!=(const WebPage& w1, const WebPage& w2) {
    return !(w1 == w2);
}

namespace std {
    template<>
    struct hash<WebPage> {
        size_t operator()(const WebPage& page) const {
            
            //一个组合哈希
            return std::hash<uint64_t>()(page.docid) ^
                   std::hash<std::string>()(page.title) ^
                   std::hash<std::string>()(page.url) ^
                   std::hash<std::string>()(page.content);
        }
    };
}


PageLibPreprocessor::PageLibPreprocessor() :m_jieba_ptr(new cppjieba::Jieba(DICT_PATH,
        HMM_PATH,
        USER_DICT_PATH,
        IDF_PATH,
        STOP_WORD_PATH)) 
        {
             m_stop_words = LoadStopWords(STOP_WORD_PATH);
        }



PageLibPreprocessor::~PageLibPreprocessor() {
    delete m_jieba_ptr;
}


void PageLibPreprocessor::setLogger(std::shared_ptr<spdlog::logger> my_logger) {
    m_my_logger = my_logger;
}


//移除content中多余的HTML标签
std::string removeHtmlTags(const std::string& input) {
    // 定义正则表达式，用于匹配HTML标签
    std::regex tagRegex("<[^>]*>");

    // 使用正则表达式替换功能，将所有匹配的HTML标签替换为空字符串
    std::string output = std::regex_replace(input, tagRegex, "");

    return output;
}

int PageLibPreprocessor::WriteUpPageLib() {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile((PAGE_PATH + std::string("auto.xml")).c_str()) != tinyxml2::XML_SUCCESS) {
        m_my_logger->error("加载RSS文件失败！");
        //cout << "加载RSS文件失败！" << endl;
        return 1;
    }

    tinyxml2::XMLElement* root = doc.RootElement();
    if (root == nullptr) {
        m_my_logger->error("找不到根元素！");
        //cout << "找不到根元素！" << endl;
        return 1;
    }

    tinyxml2::XMLElement* channel = root->FirstChildElement("channel");
    if (channel == nullptr) {
        m_my_logger->error("找不到<channel>元素！");
        //cout << "找不到<channel>元素！" << endl;
        return 1;
    }

    // 准备写入新的XML文件和偏移量文件
    std::ofstream newXmlFile((DATA_PATH + std::string("ripePage.dat")));
    std::ofstream offsetFile((DATA_PATH + std::string("newOffSet.dat")));
    int docId = 1; // 初始化docId

    newXmlFile << "<root>" << std::endl; // 写入根元素

    for (tinyxml2::XMLElement* item = channel->FirstChildElement("item"); item != nullptr; item = item->NextSiblingElement("item")) {
        std::stringstream ss;
        // 构建新的<doc>元素
        ss << "  <doc>" << std::endl;
        ss << "    <docid>" << docId << "</docid>" << std::endl;

        tinyxml2::XMLElement* title = item->FirstChildElement("title");
        if (title != nullptr) {
            ss << "    <title>" << title->GetText() << "</title>" << std::endl;
        }

        tinyxml2::XMLElement* link = item->FirstChildElement("link");
        if (link != nullptr) {
            ss << "    <url>" << link->GetText() << "</url>" << std::endl;
        }

        tinyxml2::XMLElement* description = item->FirstChildElement("description");
        if (description != nullptr) {
            ss << "    <content>" << removeHtmlTags(description->GetText()) << "</content>" << std::endl;
        }

        ss << "  </doc>" << std::endl;

        // 获取<doc>开始和结束的偏移量
        long startOffset = newXmlFile.tellp();
        newXmlFile << ss.str();
        long endOffset = long(newXmlFile.tellp()) - 1; // 减去1是为了定位到</doc>的>字符

        // 将偏移量写入文件
        offsetFile << "docid: " << docId << ", startOffset: " << startOffset << ", endOffset: " << endOffset << std::endl;

        docId++; // 递增docId
    }

    newXmlFile << "</root>" << std::endl; // 关闭根元素

    newXmlFile.close();
    offsetFile.close();

    return 0;
}

bool PageLibPreprocessor::readDocById(const uint64_t docId, WebPage &webpage) {
    std::string offsetFilePath = (DATA_PATH + std::string("newOffSet.dat"));
    std::string xmlFilePath = (DATA_PATH + std::string("ripePage.dat"));
    


    std::ifstream offsetFile(offsetFilePath);
    std::string line;
    long startOffset = -1;
    long endOffset = -1;
    
    // 在偏移量文件中查找对应的docId的偏移量
    while (getline(offsetFile, line)) {
        if (line.find("docid: " + std::to_string(docId)) != std::string::npos) {
            size_t startOffsetPos = line.find("startOffset: ") + 13;
            size_t endOffsetPos = line.find(", endOffset: ");
            startOffset = std::stol(line.substr(startOffsetPos, endOffsetPos - startOffsetPos));
            endOffset = std::stol(line.substr(endOffsetPos + 12));
            break;
        }
    }
    offsetFile.close();
    
    if (startOffset == -1 || endOffset == -1) {
        m_my_logger->error("未找到docId: ");
        //cout << "未找到docId: " << docId << endl;
        return false;
    }

    //m_my_logger->debug("{}", line);

    // 根据偏移量从XML文件中读取<doc>内容
    std::ifstream xmlFile(xmlFilePath);
    xmlFile.seekg(startOffset);
    std::vector<char> buffer(endOffset - startOffset + 1);
    xmlFile.read(&buffer[0], endOffset - startOffset + 1);
    std::string docContent(buffer.begin(), buffer.end());
    xmlFile.close();

    //m_my_logger->debug("|{}|", docContent);

    // 使用TinyXML-2解析<doc>内容
    tinyxml2::XMLDocument doc;
    doc.Parse(docContent.c_str());

    tinyxml2::XMLElement* docElement = doc.FirstChildElement("doc");
    if (docElement) {
        tinyxml2::XMLElement* docIdElement = docElement->FirstChildElement("docid");
        if (docIdElement) {
            webpage.docid = std::stoull(docIdElement->GetText());
        }
        tinyxml2::XMLElement* titleElement = docElement->FirstChildElement("title");
        if (titleElement) {
            webpage.title = titleElement->GetText() ? titleElement->GetText() : "";
        }
        tinyxml2::XMLElement* urlElement = docElement->FirstChildElement("url");
        if (urlElement) {
            webpage.url = urlElement->GetText() ? urlElement->GetText() : "";
        }
        tinyxml2::XMLElement* contentElement = docElement->FirstChildElement("content");
        if (contentElement) {
            webpage.content = contentElement->GetText() ? contentElement->GetText() : "11";
        }
    }

    return true;
}

int PageLibPreprocessor::remove_duplicates_and_rewrite_pageLib() {
    tinyxml2::XMLDocument doc;
    if(doc.LoadFile((DATA_PATH + std::string("ripePage.dat")).c_str()))
    {
        m_my_logger->error("加载ripePage.dat文件失败！");
        //cout << "加载RSS文件失败！" << endl;
        return 1;
    }

    tinyxml2::XMLElement *root = doc.RootElement();
    if(root == nullptr) {
        m_my_logger->error("找不到根元素！");
        return 1;
    }

    std::vector<WebPage> webpages;

    for(tinyxml2::XMLElement* docElement = root->FirstChildElement("doc"); docElement != nullptr; docElement = docElement->NextSiblingElement("doc")) {
        WebPage webpage;

        tinyxml2::XMLElement* docIdElement = docElement->FirstChildElement("docid");
        if (docIdElement && docIdElement->GetText()) {
            webpage.docid = std::stoull(docIdElement->GetText());
        }

        tinyxml2::XMLElement* titleElement = docElement->FirstChildElement("title");
        if (titleElement && titleElement->GetText()) {
            webpage.title = titleElement->GetText();
        }

        tinyxml2::XMLElement* urlElement = docElement->FirstChildElement("url");
        if (urlElement && urlElement->GetText()) {
            webpage.url = urlElement->GetText();
        }

        tinyxml2::XMLElement* contentElement = docElement->FirstChildElement("content");
        if (contentElement && contentElement->GetText()) {
            webpage.content = contentElement->GetText();
        }

        webpages.push_back(webpage); // 将解析的WebPage添加到vector中
    }


    //对WebPage的Content字段进行去重
    std::vector<WebPage> mySet;

    for(auto &ele: webpages) {
        insertIfUnique(mySet, ele);
    }


    // 准备写入新的XML文件和偏移量文件
    std::ofstream newXmlFile((DATA_PATH + std::string("newRipePage.dat")));
    std::ofstream offsetFile((DATA_PATH + std::string("newNewOffSet.dat")));
    


    newXmlFile << "<root>" << std::endl; // 写入根元素

    for(const auto& ele: mySet) {
        std::stringstream ss;

        // 构建新的<doc>元素
        ss << "  <doc>" << std::endl;
        ss << "    <docid>" << ele.docid << "</docid>" << std::endl;
        ss << "    <title>" << ele.title << "</title>" << std::endl;
        ss << "    <url>"   << ele.url   << "</url>"   << std::endl;
        ss << "    <content>" << ele.content << "</content>" << std::endl;
        ss << "  </doc>" << std::endl;

        // 获取<doc>开始和结束的偏移量
        long startOffset = newXmlFile.tellp();
        newXmlFile << ss.str();
        long endOffset = long(newXmlFile.tellp()) - 1;

        // 将偏移量写入文件
        offsetFile << "docid: " << ele.docid << ", startOffset: " << startOffset << ", endOffset: " << endOffset << std::endl;

    }

    newXmlFile << "</root>" << std::endl; // 关闭根元素


    newXmlFile.close();
    offsetFile.close();

    return 0;
}

void PageLibPreprocessor::insertIfUnique(std::vector<WebPage>& container, const WebPage& page) {
    for(const auto& ele: container) {
        if(ele == page) {
            return ;
        }
    }

    container.push_back(page);
}

std::unordered_set<std::string> PageLibPreprocessor::LoadStopWords(const std::string& stopWordPath) {
    std::unordered_set<std::string> stopWords;
    std::ifstream file(stopWordPath);
    std::string line;
    while (file >> line) {
        if(!line.empty())
        {
          /*std::cout << "stop = |" << line << "|\n";*/
          stopWords.insert(line);
        }
        else {
          break;
        }
    }
    return stopWords;
}

void PageLibPreprocessor::removeStopWords(std::vector<std::string>& words) {
    for(auto it = words.begin(); it != words.end();) {
            if(m_stop_words.find(*it) != m_stop_words.end()) {
                words.erase(it);
            }
            else if((*it).empty()) {
                words.erase(it);
                
            }
            else if((*it)[0] == ' ') {
                words.erase(it);
            }
            else {
                ++it;
            }
         }
}



int PageLibPreprocessor::createInvertedIndex() {

    //1.加载网页库
    tinyxml2::XMLDocument doc;
    if(doc.LoadFile((DATA_PATH + std::string("newRipePage.dat")).c_str()))
    {
        m_my_logger->error("加载ripePage.dat文件失败！");
        //cout << "加载RSS文件失败！" << endl;
        return 1;
    }

    tinyxml2::XMLElement *root = doc.RootElement();
    if(root == nullptr) {
        m_my_logger->error("找不到根元素！");
        return 1;
    }

    //2.对每一个分词，存储它在每个文章中出现的频率和它是否在本文章中出现过
    //2. step1 建立存储中间过程的数据结构，同时记录一共有多少个文章
    std::unordered_map<std::string, std::unordered_map<uint64_t, std::pair<uint64_t, bool>>>
        temp;
    
    uint64_t articleNum = 0;

    

    //2. step2 加载每一个文章，先切分词，更新该数据结构

    for(tinyxml2::XMLElement* docElement = root->FirstChildElement("doc"); docElement != nullptr; docElement = docElement->NextSiblingElement("doc")) {
        //文章数自增
        ++articleNum;

        //加载文章内容和该文章的id

        uint64_t docid;
        tinyxml2::XMLElement* docIdElement = docElement->FirstChildElement("docid");
        if (docIdElement && docIdElement->GetText()) {
            docid = std::stoull(docIdElement->GetText());
        }

        std::string content;
        tinyxml2::XMLElement* contentElement = docElement->FirstChildElement("content");
        if (contentElement && contentElement->GetText()) {
            content = contentElement->GetText();
        }

        //切分词
        std::vector<std::string> words;
        m_jieba_ptr->Cut(content, words, true);


        //去除停用词
        removeStopWords(words);

        //对于每个词，更新它的信息数据结构
        for(const auto& word: words) {
            (temp[word][docid].first)++;
            (temp[word][docid].second) = true;

        } 
    }

    std::unordered_map<std::string, std::map<int, double>> rst;

    //计算未归一化的结果

    for(const auto& str_to_map: temp) {
        uint64_t DF = str_to_map.second.size();

        for(const auto& ele2: str_to_map.second) {
            uint64_t TF = ele2.second.first;
            double IDF = log2(articleNum / (DF + 1));

            rst[str_to_map.first][ele2.first] = TF * IDF;
        }
    }
    

    //计算归一化的结果

    std::unordered_map<int, std::map<std::string, double>> docid_to_word;

    /*
    对于我这个数据结构，它的意义是第一个string是一个单词，它所对应的map的第一个int是文章id，
    第二个double是这个单词在该文章中的权重。transform的意义是，把它转换成这种数据结构：
    std::unordered_map<int, std::map<std::string, double>>，它的含义是第一个int是文章id，
    它所对应的map的第一个元素是这个文章中包含的某一个单词，double这个单词在该文章中的权重。
    这样方便后续的转换
    */
    docid_to_word = transform(rst);

    std::unordered_map<std::string, std::map<int, double>> ultra_rst;

    for(const auto& ele: docid_to_word) {
        double total = 0.0;

        for(const auto& weight: ele.second) {
            total += weight.second * weight.second;
        }

        total = std::sqrt(total);

        for(const auto& word: ele.second) {
            ultra_rst[word.first][ele.first] = word.second / total;
        }
    }


    m_inverted_index = ultra_rst;


    nlohmann::json j = ultra_rst;

    std::ofstream file(DATA_PATH + std::string("InvertedIndex.json"));

    file << j.dump(4);
    file.close();

    return 0;
}


int PageLibPreprocessor::loadInvertedIndex() {
    const std::string& filename = DATA_PATH + std::string("InvertedIndex.json");

    m_inverted_index.clear();

    

    // 打开文件
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        spdlog::error("无法打开文件: {}", filename);
        //std::cerr << "无法打开文件: " << filename << std::endl;
        return -1; // 返回错误
    }

    // 使用nlohmann::json直接从输入流解析JSON
    nlohmann::json jsonData;
    inputFile >> jsonData;

    // 直接使用.get<>()转换JSON数据到目标类型
    m_inverted_index = jsonData.get<std::unordered_map<std::string, std::map<int, double>>>();
    m_transformed_inverted_index = transform(m_inverted_index);
    return 0;
}

std::vector<std::vector<std::string>> PageLibPreprocessor::QueryPage(const std::string & sentence) {
    std::vector<std::string> sentence_words;


    spdlog::debug("File PageLibPreprocessor.cc line 581");
    m_jieba_ptr->Cut(sentence, sentence_words, true);
    removeStopWords(sentence_words);

    spdlog::debug("File PageLibPreprocessor.cc line 585");
    //先对用户输入的句子（看作一个文章），计算每个分词在文章中的归一权重
    //step1 记录中间结果
    std::unordered_map<std::string, std::unordered_map<uint64_t, std::pair<uint64_t, bool>>>
        temp;
    
    // step2 对于每个词，更新它的信息数据结构 (复用上面的)
    for(const auto& word: sentence_words) {
        (temp[word][0].first)++;
        (temp[word][0].second) = true;
    } 

    spdlog::debug("File PageLibPreprocessor.cc line 597");
    std::unordered_map<std::string, std::map<int, double>> rst;

    // step3 计算未归一化的结果
    uint64_t articleNum = 0;
    for(const auto& str_to_map: temp) {
        uint64_t DF = str_to_map.second.size();

        for(const auto& ele2: str_to_map.second) {
            uint64_t TF = ele2.second.first;
            double IDF = log2(articleNum / (DF + 1));

            rst[str_to_map.first][ele2.first] = TF * IDF;
        }
    }

    spdlog::debug("File PageLibPreprocessor.cc line 613");

    std::unordered_map<int, std::map<std::string, double>> docid_to_word;

    // step4 先转换数据结构
    docid_to_word = transform(rst);
    
    spdlog::debug("File PageLibPreprocessor.cc line 620");

    //计算归一化的结果
    std::unordered_map<std::string, std::map<int, double>> sentence_temp_rst;

    for(const auto& ele: docid_to_word) {
        double total = 0.0;

        for(const auto& weight: ele.second) {
            total += weight.second * weight.second;
        }

        total = std::sqrt(total);

        for(const auto& word: ele.second) {
            sentence_temp_rst[word.first][ele.first] = word.second / total;
        }
    }

    spdlog::debug("File PageLibPreprocessor.cc line 637");

    std::unordered_map<int, std::map<std::string, double>> sentence_temp2_rst;

    sentence_temp2_rst = transform(sentence_temp_rst);

    std::vector<double> sentence_base_vector;
    std::vector<std::string> sentence_base_words;

    spdlog::debug("File PageLibPreprocessor.cc line 646");
    for(auto& single: sentence_temp2_rst) {
        for(auto& pair: single.second) {
            sentence_base_words.push_back(pair.first);
            sentence_base_vector.push_back(pair.second);
        }
    }


    spdlog::debug("File PageLibPreprocessor.cc line 655");
    //计算完毕

    std::priority_queue<RstArticle> articles;


    std::set<int> articleIds;

    for(const auto& word: sentence_words) {
        for(auto &pair: m_inverted_index[word]) {
            articleIds.insert(pair.first);
        }
    }

    spdlog::debug("File PageLibPreprocessor.cc line 669");

    for(const auto& id: articleIds) {
        RstArticle art;
        art.articleId = id;
        art.words = sentence_base_words;
        art.base_weight_vector = sentence_base_vector;
        for(const auto& word: art.words) {
            art.article_weight_vector.push_back(m_transformed_inverted_index[id][word]);
        }

        articles.push(art);
    }

    spdlog::debug("File PageLibPreprocessor.cc line 683");
    //优先队列构建完毕，开始依据优先队列构造数据

    std::vector<std::vector<std::string>> query_rst;

    int size = 5;
    while( size > 0 && !articles.empty() ) {
        RstArticle art = articles.top();
        WebPage page;
        readDocById(art.articleId, page);

        std::vector<std::string> words = splitUTF8(page.content);

        spdlog::debug("File PageLibPreprocessor.cc line 698 words size = {}", words.size());

        words = getFirst25Elements(words);
        spdlog::debug("File PageLibPreprocessor.cc line 701 words size = {}", words.size());

        std::string content;

        for(const auto& word: words) {
            content += word;
        }

        query_rst.push_back(std::vector<std::string>{page.title, page.url, content});
        articles.pop();
        size--;
    }

    return query_rst;


}

/*
    对于我这个数据结构，它的意义是第一个string是一个单词，它所对应的map的第一个int是文章id，
    第二个double是这个单词在该文章中的权重。transform的意义是，把它转换成这种数据结构：
    std::unordered_map<int, std::map<std::string, double>>，它的含义是第一个int是文章id，
    它所对应的map的第一个元素是这个文章中包含的某一个单词，double这个单词在该文章中的权重。
    这样方便后续的转换
*/

std::unordered_map<int, std::map<std::string, double>> 
    PageLibPreprocessor::transform(const std::unordered_map<std::string, std::map<int, double>>& original) {

        std::unordered_map<int, std::map<std::string, double>> target;
    
    // 遍历原始数据结构
    for (const auto& word_pair : original) {
        const auto& word = word_pair.first; // 获取单词
        const auto& id_weight_map = word_pair.second; // 获取对应的文章ID和权重的map
        
        // 遍历文章ID和权重的map
        for (const auto& id_weight_pair : id_weight_map) {
            const auto& id = id_weight_pair.first; // 获取文章ID
            const auto& weight = id_weight_pair.second; // 获取权重
            
            // 在目标数据结构中插入或更新数据
            target[id][word] = weight;
        }
    }
    
    return target;
}

std::vector<std::string> PageLibPreprocessor::splitUTF8(const std::string& str) {
    std::vector<std::string> result;
    std::string charBuffer;

    for (std::size_t i = 0; i < str.size();) {
        // 获取当前字节的值
        unsigned char lead = str[i];

        // 单字节字符(ASCII)
        if (lead <= 0x7F) {
            result.push_back(std::string(1, str[i]));
            ++i; // 移动到下一个字节
        } else {
            // 根据UTF-8编码规则确定字符的字节数
            int charSize = 0;
            if ((lead >> 5) == 0x6) { // 110xxxxx
                charSize = 2;
            } else if ((lead >> 4) == 0xE) { // 1110xxxx
                charSize = 3;
            } else if ((lead >> 3) == 0x1E) { // 11110xxx
                charSize = 4;
            }
            // 根据确定的字节数提取字符
            charBuffer.clear();
            for (int j = 0; j < charSize; ++j) {
                charBuffer += str[i + j];
            }
            result.push_back(charBuffer);
            i += charSize; // 移动到下一个字符的开始位置
        }
    }

    return result;
}

