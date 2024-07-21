
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cppjieba/Jieba.hpp"
#include "example/add.hpp"
#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

const char *const DICT_PATH = "../../dict/jieba.dict.utf8";
const char *const HMM_PATH = "../../dict/hmm_model.utf8";
const char *const USER_DICT_PATH = "../../dict/user.dict.utf8";
const char *const IDF_PATH = "../../dict/idf.utf8";
const char *const STOP_WORD_PATH = "../../dict/stop_words.utf8";

std::unordered_set<std::string> LoadStopWords(const std::string &stopWordPath) {
  std::unordered_set<std::string> stopWords;
  std::ifstream file(stopWordPath);
  std::string line;
  while (file >> line) {
    if (!line.empty()) {
      /*std::cout << "stop = |" << line << "|\n";*/
      stopWords.insert(line);
    } else {
      break;
    }
  }
  return stopWords;
}

std::vector<std::string> splitUTF8(const std::string &str) {
  std::vector<std::string> result;
  std::string charBuffer;

  for (std::size_t i = 0; i < str.size();) {
    // 获取当前字节的值
    unsigned char lead = str[i];

    // 单字节字符(ASCII)
    if (lead <= 0x7F) {
      result.push_back(std::string(1, str[i]));
      ++i;  // 移动到下一个字节
    } else {
      // 根据UTF-8编码规则确定字符的字节数
      int charSize = 0;
      if ((lead >> 5) == 0x6) {  // 110xxxxx
        charSize = 2;
      } else if ((lead >> 4) == 0xE) {  // 1110xxxx
        charSize = 3;
      } else if ((lead >> 3) == 0x1E) {  // 11110xxx
        charSize = 4;
      }
      // 根据确定的字节数提取字符
      charBuffer.clear();
      for (int j = 0; j < charSize; ++j) {
        charBuffer += str[i + j];
      }
      result.push_back(charBuffer);
      i += charSize;  // 移动到下一个字符的开始位置
    }
  }

  return result;
}

int min(int a, int b) {
  if (a < b) {
    return a;
  } else {
    return b;
  }
}

uint64_t shortest_edit_distance(std::vector<std::string> target,
                                std::vector<std::string> str) {
  int target_len = target.size();
  int str_len = str.size();

  int dp[100][100];

  for (int i = 0; i < target_len; i++) {
    dp[i][0] = i;
  }

  for (int i = 0; i <= str_len; i++) {
    dp[0][i] = i;
  }

  for (int i = 1; i <= target_len; i++) {
    for (int j = 1; j <= str_len; j++) {
      dp[i][j] = min(dp[i - 1][j] + 1, dp[i][j - 1] + 1);
      dp[i][j] =
          min(dp[i][j], dp[i - 1][j - 1] + (target[i - 1] != str[j - 1]));
    }
  }

  return dp[target_len][str_len];
}

class Compare {
 public:
  Compare(std::vector<std::string> user) : m_user_input(std::move(user)){};

  bool operator()(std::vector<std::string> &a, std::vector<std::string> &b) {
    int a_sed = shortest_edit_distance(m_user_input, a);
    int b_sed = shortest_edit_distance(m_user_input, b);

    return a_sed > b_sed;
  }

 private:
  uint64_t shortest_edit_distance(std::vector<std::string> &target,
                                  std::vector<std::string> &str) {
    int target_len = target.size();
    int str_len = str.size();

    int dp[100][100];

    for (int i = 0; i < target_len; i++) {
      dp[i][0] = i;
    }

    for (int i = 0; i <= str_len; i++) {
      dp[0][i] = i;
    }

    for (int i = 1; i <= target_len; i++) {
      for (int j = 1; j <= str_len; j++) {
        dp[i][j] = min(dp[i - 1][j] + 1, dp[i][j - 1] + 1);
        dp[i][j] =
            min(dp[i][j], dp[i - 1][j - 1] + (target[i - 1] != str[j - 1]));
      }
    }

    return dp[target_len][str_len];
  }

 private:
  std::vector<std::string> m_user_input;
};

int main() {
  /*
  // 快速日志示例
  spdlog::info("Welcome to spdlog!");
  spdlog::error("Some error message with arg: {}", 1);

  // 创建日志器并使用
  auto my_logger = spdlog::stdout_color_mt("my_logger");
  my_logger->info("This message is logged by my_logger");
  std::string name = "World";
  fmt::print("Hello, {}!", name);

  int x = 0;
  for (int i = 0; i < 10; i++) {
    std::cout << "Hello, World!" << std::endl;
  }

  if (x == 0) {
    std::cout << "x is zero" << std::endl;
  }

  std::cout << "add:" << add(1, 1);
  */

  cppjieba::Jieba jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH,
                        STOP_WORD_PATH);

  std::vector<std::string> words;

  std::string s;
  std::string result;

  std::unordered_set<std::string> stopWords = LoadStopWords(STOP_WORD_PATH);

  s = "我来到北京清华大学，北平爷爷，西京大学，南京大学, 京城";
  //切分词

  jieba.Cut(s, words, true);

  //去除空格和停用词
  for (auto it = words.begin(); it != words.end();) {
    if (stopWords.find(*it) != stopWords.end() || (*it).empty() ||
        ((*it) == " ")) {
      words.erase(it);
    } else {
      ++it;
    }
  }

  //生成每个词和它所对应的词频
  std::unordered_map<std::string, uint64_t> words_and_its_frequency;
  //生成每个字符和它所出现的单词
  std::unordered_map<std::string, std::set<std::string>> char_and_its_words;

  for (auto &word : words) {
    std::vector<std::string> charactors = splitUTF8(word);
    ++words_and_its_frequency[word];
    for (auto &charactor : charactors) {
      char_and_its_words[charactor].insert(word);
    }
  }

  std::string user_input = "西京";
  //对用户输入的单词进行切分，对用户输入单词所含的字，
  //挑出这些字所包含的所有单词

  std::vector<std::string> char_of_user_input = splitUTF8(user_input);

  std::set<std::string> all_words;

  for (auto &chara : char_of_user_input) {
    std::set<std::string> temp;
    std::set_union(
        all_words.begin(), all_words.end(), char_and_its_words[chara].begin(),
        char_and_its_words[chara].end(), std::inserter(temp, temp.begin()));
    all_words = std::move(temp);
  }

  std::priority_queue<std::vector<std::string>,
                      std::vector<std::vector<std::string>>, Compare>
      que{Compare(char_of_user_input)};

  for (auto &word : all_words) {
    que.push(splitUTF8(word));
  }

  while (!que.empty()) {
    fmt::print("word = ");
    for (auto &ele : que.top()) {
      fmt::print("{}", ele);
    }
    fmt::print("\n");
    que.pop();
  }

  return 0;
}
