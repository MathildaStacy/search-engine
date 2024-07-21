#include <memory>

#include "create_search_index/PageLibPreprocessor.h"
#include "keyword_recom_preprocess/DictProducer.h"
#include "reactor/EchoServer.h"
#include "spdlog/common.h"
#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

int main() {
  // EchoServer server(4, 10, "127.0.0.1", 8888);
  // server.start();

  std::shared_ptr<spdlog::logger> my_logger =
      spdlog::stdout_color_mt("my_logger");
  spdlog::set_level(spdlog::level::debug);

  my_logger->info("This message is logged by my_logger");
  my_logger->set_level(spdlog::level::debug);

  DictProducer pro;
  pro.setLogger(my_logger);
  // pro.prepare();

  my_logger->info("main.cc line 28");

  PageLibPreprocessor pag;
  // pag.setLogger(my_logger);
  // pag.WriteUpPageLib();

  my_logger->info("main.cc line 34");

  // pag.remove_duplicates_and_rewrite_pageLib();

  my_logger->info("main.cc line 38");
  // pag.createInvertedIndex();

  my_logger->info("main.cc line 42");
  pag.loadInvertedIndex();

  my_logger->info("main.cc line 45");

  std::vector<std::vector<std::string>> rst =
      pag.QueryPage("人民有希望，国家有力量");

  for (const auto &row : rst) {
    for (const auto &ele : row) {
      fmt::print("{}\n", ele);
      fmt::print("||\n||\n");
    }

    fmt::println("|||||||||||||||||||||||||||||||\n");
    fmt::println("|||||||||||||||||||||||||||||||\n");
  }

  /*
  EchoServer server(4, 10, "127.0.0.1", 8888, pro, pag);
  server.setLogger(my_logger);

  server.start();
  */

  return 0;
}