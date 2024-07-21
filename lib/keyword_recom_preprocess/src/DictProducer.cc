#include "keyword_recom_preprocess/DictProducer.h"
#include <vector>
#include <fstream>
#include <string>

int min(int a, int b) {
    if(a < b) {
        return a;
    }
    else 
    {
        return b;
    }
}

void DictProducer::setLogger(std::shared_ptr<spdlog::logger> my_logger)
{
    m_my_logger = my_logger;
}



std::string DictProducer::readFileIntoString(const std::string& path) {
    std::ifstream input_file(path);
    
    // 检查文件是否成功打开
    if (!input_file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    input_file.seekg(0, std::ios::end);
    size_t size = input_file.tellg();
    std::string buffer;
    if (size != static_cast<size_t>(-1)) {
        buffer.resize(size);
        input_file.seekg(0, std::ios::beg);
        input_file.read(&buffer[0], size);
    }

    return buffer;
}


bool isEnglish(const std::string& word) {
    if(word.empty()) {
        return false;
    }

    if(word[0] >= 'a' && word[0] <= 'z') {
        if(word[0] >= 'A' && word[0] <= 'Z') {
            return true;
        }
    }

    return false;
}

class Compare {
public:
  Compare(std::vector<std::string> user) : m_user_input(std::move(user)) {};

  bool operator()(std::vector<std::string>& a, std::vector<std::string>& b) {
    int a_sed = shortest_edit_distance(m_user_input, a);
    int b_sed = shortest_edit_distance(m_user_input, b);

    return a_sed > b_sed;
  }


private:
  uint64_t shortest_edit_distance(std::vector<std::string>& target, std::vector<std::string>& str) {
  int target_len = target.size();
  int str_len = str.size();

  int dp[100][100];

  for(int i = 0; i < target_len; i++) {
    dp[i][0] = i;
  }

  for(int i = 0; i <= str_len; i++) {
    dp[0][i] = i;
  }

  for(int i = 1; i <= target_len; i++) {
    for(int j = 1; j <= str_len; j++) {
      dp[i][j] = min(dp[i - 1][j] + 1, dp[i][j - 1] + 1);
      dp[i][j] = min(dp[i][j], dp[i - 1][j - 1] + (target[i - 1] != str[j - 1]));
    }
  }

  return dp[target_len][str_len];
}

private:

  std::vector<std::string> m_user_input;
};



DictProducer::DictProducer() : m_jieba_ptr(new cppjieba::Jieba(DICT_PATH,
        HMM_PATH,
        USER_DICT_PATH,
        IDF_PATH,
        STOP_WORD_PATH)) {}


DictProducer::~DictProducer() {
    delete m_jieba_ptr;
}


std::vector<std::string> DictProducer::splitUTF8(const std::string& str) {
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


std::unordered_set<std::string> DictProducer::LoadStopWords(const std::string& stopWordPath) {
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

void DictProducer::removeStopWords(std::vector<std::string>& words) {
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

void DictProducer::prepare() {
    //std::string s = "我来到北京清华大学，北平爷爷，西京大学，南京大学, 京城";
    //std::string s = "In the heart of an ancient forest, where the sun barely pierces through the dense canopy, a small, crystal-clear stream winds its way over smooth stones, whispering tales of old. Here, nature speaks in hushed tones, inviting all who wander to listen to the stories of the earth, woven through time.";
    std::string s = readFileIntoString(READING_PAGE_PATH + std::string("yuliao.txt"));
    m_stop_words = LoadStopWords(STOP_WORD_PATH);
    m_jieba_ptr->Cut(s, m_words, true);

    //私货
    //m_words.push_back("北大京");


    /*
    //去除前 测试
    for(const auto& ele: m_words) {
      m_my_logger->debug("{} ", ele);
    }
    m_my_logger->debug("\n");
    */

    //去除停止词
    removeStopWords(m_words);

    /*
    //去除后 测试
    for(const auto& ele: m_words) {
      m_my_logger->debug("{} ", ele);
    }
    m_my_logger->debug("\n");
    */

    for(auto& word: m_words) {
      if(isEnglish(word)) {
        std::vector<std::string> charactors = splitUTF8(word);
        ++m_english_words_and_its_frequency[word];
        for(auto& charactor: charactors) {
          m_english_char_and_its_words[charactor].insert(word);
        }
      }
      else {
        std::vector<std::string> charactors = splitUTF8(word);
        ++m_chinese_words_and_its_frequency[word];
        for(auto& charactor: charactors) {
          m_chinese_char_and_its_words[charactor].insert(word);
        }
      }
  }

  
}

std::vector<std::string> DictProducer::run(const std::string& user_input) {
  if(isEnglish(user_input)) {
    return english_run(user_input);
  }
  else {
    return chinese_run(user_input);
  }
}

std::vector<std::string> DictProducer::chinese_run(const std::string& user_input) {
    std::vector<std::string> char_of_user_input = splitUTF8(user_input);

  std::set<std::string> all_words;

  for(auto& chara: char_of_user_input) {
    std::set<std::string> temp;
    std::set_union(all_words.begin(), all_words.end(), m_chinese_char_and_its_words[chara].begin(), m_chinese_char_and_its_words[chara].end(), std::inserter(temp, temp.begin()));
    all_words = std::move(temp);
  }

  
  

  std::priority_queue<std::vector<std::string>, std::vector<std::vector<std::string>>, Compare> que{Compare(char_of_user_input)};

  for(auto& word: all_words) {
    que.push(splitUTF8(word));
  }

  std::vector<std::string> rst;

  while(!que.empty()) {
    std::string tmp;
    for(auto& ele: que.top()) {
      tmp += ele;
    }

    rst.push_back(tmp);
    que.pop();
  }

  return rst;
}

std::vector<std::string> DictProducer::english_run(const std::string& user_input) {
    std::vector<std::string> char_of_user_input = splitUTF8(user_input);

  std::set<std::string> all_words;

  for(auto& chara: char_of_user_input) {
    std::set<std::string> temp;
    std::set_union(all_words.begin(), all_words.end(), m_chinese_char_and_its_words[chara].begin(), m_chinese_char_and_its_words[chara].end(), std::inserter(temp, temp.begin()));
    all_words = std::move(temp);
  }

  
  

  std::priority_queue<std::vector<std::string>, std::vector<std::vector<std::string>>, Compare> que{Compare(char_of_user_input)};

  for(auto& word: all_words) {
    que.push(splitUTF8(word));
  }

  std::vector<std::string> rst;

  while(!que.empty()) {
    std::string tmp;
    for(auto& ele: que.top()) {
      tmp += ele;
    }

    rst.push_back(tmp);
    que.pop();
  }

  return rst;
}