#include <memory>
#include <thread>
#include <vector>

#include "cache/keywordCacheManager.h"
#include "cache/urlCacheManager.h"
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
  std::shared_ptr<spdlog::logger> my_logger =
      spdlog::stdout_color_mt("my_logger");
  spdlog::set_level(spdlog::level::debug);

  my_logger->info("This message is logged by my_logger");
  my_logger->set_level(spdlog::level::debug);

  DictProducer pro;
  pro.setLogger(my_logger);
  pro.prepare();

  // my_logger->info("main.cc line 28");

  PageLibPreprocessor pag;
  pag.setLogger(my_logger);
  // pag.WriteUpPageLib();

  // my_logger->info("main.cc line 34");

  // pag.remove_duplicates_and_rewrite_pageLib();

  // my_logger->info("main.cc line 38");
  // pag.createInvertedIndex();

  my_logger->info("main.cc line 42");
  pag.loadInvertedIndex();

  KeywordCacheManager keyword_manager(9, 50);
  UrlCacheManager url_manager(9, 50);
  std::thread backStageUpdateCache([&keyword_manager, &url_manager]() {
    while (1) {
      sleep(3);
      spdlog::info("\n\n缓存已同步\n\n");
      keyword_manager.synchronizeAllOddLRUCache();
      url_manager.synchronizeAllOddLRUCache();
    }
  });
  EchoServer server(4, 10, "127.0.0.1", 8888, pro, pag, keyword_manager,
                    url_manager);
  server.setLogger(my_logger);

  server.start();

  /*

      // 假设有5个缓存，每个缓存的容量都是10
      UrlCacheManager cacheManager(5, 10);

      // 往奇数下标的cache（例如下标为1的cache）插入键值对
      std::string key = "exampleKey";
      std::vector<std::vector<std::string>> value = {{"exampleValue1"},
     {"exampleValue2"}}; cacheManager.insert(1, key, value);

      // 尝试从同一个奇数下标的cache获取键值对
      auto retrievedValue = cacheManager.query(1, key);
      if (!retrievedValue.empty()) {
          std::cout << "Successfully retrieved from cache at index 1: " <<
     retrievedValue[0][0] << std::endl; } else { std::cout << "Failed to
     retrieve from cache at index 1." << std::endl;
      }

      // 执行同步操作
      cacheManager.synchronizeAllOddLRUCache();

      // 尝试从另一个奇数下标的cache（例如下标为3的cache）获取相同的键值对
      retrievedValue = cacheManager.query(3, key);
      if (!retrievedValue.empty()) {
          std::cout << "Successfully retrieved from cache at index 3 after
     synchronization: " << retrievedValue[0][0] << std::endl; } else { std::cout
     << "Failed to retrieve from cache at index 3 after synchronization." <<
     std::endl;
      }

  */

  return 0;
}