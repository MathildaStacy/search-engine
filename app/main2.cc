#include <memory>

#include "create_search_index/PageLibPreprocessor.h"
#include "keyword_recom_preprocess/DictProducer.h"
#include "spdlog/common.h"
#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

int main() {
  std::shared_ptr<spdlog::logger> my_logger =
      spdlog::stdout_color_mt("my_logger");

  my_logger->info("This message is logged by my_logger");
  my_logger->set_level(spdlog::level::debug);

  /*
  DictProducer pro;
  pro.setLogger(my_logger);
  pro.prepare();
  std::vector<std::string> rst = pro.run("的人民");
  for(auto& ele: rst) {
      fmt::print("{} ", ele);
  }
  */

  PageLibPreprocessor pag;
  pag.setLogger(my_logger);
  // pag.WriteUpPageLib();
  WebPage wp;

  pag.readDocById(4, wp);

  fmt::println("docid = {}", wp.docid);
  fmt::println("title = {}", wp.title);
  fmt::println("url = {}", wp.url);
  fmt::println("content = {}", wp.content);

  pag.remove_duplicates_and_rewrite_pageLib();

  pag.createInvertedIndex();
}