#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>  // for close

#include <cstdint>
#include <cstring>  // for memset
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "nlohmann/json.hpp"
#include "spdlog/common.h"
#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

struct train_t {
  uint64_t length;
  std::string data;
};

int main() {
  // 创建socket
  std::cout << "start\n";
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    std::cerr << "创建socket失败" << std::endl;
    return -1;
  }

  std::cout << "line 26\n";

  // 定义服务器地址
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(8888);                    // 设置端口
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");  // 设置服务器IP

  std::cout << "line 35\n";
  // 连接服务器
  if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) <
      0) {
    std::cerr << "连接失败" << std::endl;
    close(sock);
    return -1;
  }

  std::cout << "line 43\n";
  //第一次测试
  // 发送数据
  // const char* message = "Hello, Server!";

  std::unordered_map<std::string, std::string> message_map;
  message_map["search-type"] = "keyword";
  message_map["body"] = "人民";

  std::cout << "line 51\n";

  train_t train;

  nlohmann::json j = message_map;
  std::cout << "line 56\n";
  std::string msg = j.dump(4);

  train.length = msg.size();
  train.data = msg;

  std::cout << "size = " << train.length << "\n";

  if (send(sock, &(train.length), sizeof(train.length), 0) < 0) {
    std::cerr << "发送失败" << std::endl;
    close(sock);
    return -1;
  }

  if (send(sock, train.data.c_str(), train.length, 0) < 0) {
    std::cerr << "发送失败" << std::endl;
    close(sock);
    return -1;
  }

  // 接收回复
  uint64_t rst_size = 0;
  ssize_t bytesRead = recv(sock, &rst_size, sizeof(rst_size), MSG_WAITALL);
  if (bytesRead < 0) {
    std::cerr << "接收失败" << std::endl;
    close(sock);
    return -1;
  }

  char buffer[65535] = {0};
  bytesRead = recv(sock, buffer, rst_size, MSG_WAITALL);
  if (bytesRead < 0) {
    std::cerr << "接收失败" << std::endl;
    close(sock);
    return -1;
  }

  std::string reply = buffer;
  auto json = nlohmann::json::parse(reply);

  std::vector<std::string> reply_vec = json.get<std::vector<std::string>>();

  for (auto &ele : reply_vec) {
    std::cout << ele << " ";
  }

  //第二次测试
  message_map["search-type"] = "url";
  message_map["body"] = "人民有希望，国家有力量";

  std::cout << "line 51\n";

  train_t train1;

  nlohmann::json j2 = message_map;
  std::cout << "line 56\n";
  std::string msg1 = j2.dump(4);

  train1.length = msg1.size();
  train1.data = msg1;

  std::cout << "size = " << train1.length << "\n";

  if (send(sock, &(train1.length), sizeof(train1.length), 0) < 0) {
    std::cerr << "发送失败" << std::endl;
    close(sock);
    return -1;
  }

  if (send(sock, train1.data.c_str(), train1.length, 0) < 0) {
    std::cerr << "发送失败" << std::endl;
    close(sock);
    return -1;
  }

  //接收回复
  // 接收回复
  uint64_t rst_size1 = 0;
  ssize_t bytesRead2 = recv(sock, &rst_size1, sizeof(rst_size1), MSG_WAITALL);
  if (bytesRead2 < 0) {
    std::cerr << "接收失败" << std::endl;
    close(sock);
    return -1;
  }

  char buffer2[999999] = {0};
  bytesRead2 = recv(sock, buffer2, rst_size1, MSG_WAITALL);
  if (bytesRead2 < 0) {
    std::cerr << "接收失败" << std::endl;
    close(sock);
    return -1;
  }

  std::string reply2 = buffer2;
  auto json2 = nlohmann::json::parse(reply2);
  std::vector<std::vector<std::string>> reply_vec2 =
      json2.get<std::vector<std::vector<std::string>>>();

  for (const auto &row : reply_vec2) {
    for (const auto &ele : row) {
      fmt::print("{}\n", ele);
      fmt::print("||\n||\n");
    }

    fmt::println("|||||||||||||||||||||||||||||||\n");
    fmt::println("|||||||||||||||||||||||||||||||\n");
  }

  // 关闭socket
  close(sock);

  return 0;
}
