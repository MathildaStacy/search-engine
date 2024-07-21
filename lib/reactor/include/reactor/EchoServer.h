#ifndef __ECHOSERVER_H__
#define __ECHOSERVER_H__

#include "ThreadPool.h"
#include "TcpServer.h"

#include "keyword_recom_preprocess/DictProducer.h"
#include "create_search_index/PageLibPreprocessor.h"
#include "cache/keywordCacheManager.h"
#include "cache/urlCacheManager.h"
#include "nlohmann/json.hpp"
#include <string>
#include <unordered_map>


class MyTask
{
public:
    MyTask(const TcpConnectionPtr &con, std::unordered_map<std::string, std::string> request, 
           DictProducer& pro, PageLibPreprocessor& pag, KeywordCacheManager& keyword_cachemanager,
           UrlCacheManager& url_cache_manager);
    void process();

    void setMsg(std::string msg);
private:
    std::string _msg;
    TcpConnectionPtr _con;
    std::unordered_map<std::string, std::string> m_request;
    DictProducer& m_pro;
    PageLibPreprocessor& m_pag;
    KeywordCacheManager& m_keyword_cache_manager;
    UrlCacheManager& m_url_cache_manager;
};

class EchoServer
{
public:
    EchoServer(size_t threadNum, size_t queSize
               , const string &ip, 
               unsigned short port,
               DictProducer &pro,
               PageLibPreprocessor &pag,
               KeywordCacheManager& keyword_cachemanager,
               UrlCacheManager& url_cache_manager);
    ~EchoServer();

    void start();
    void stop();

    void setLogger(std::shared_ptr<spdlog::logger> my_logger);
    void setCacheManager(KeywordCacheManager& cachemanager);

    void onNewConnection(const TcpConnectionPtr &con);
    void onMessage(const TcpConnectionPtr &con);
    void onClose(const TcpConnectionPtr &con);



private:
    ThreadPool _pool;
    TcpServer _server;
    DictProducer & _pro;
    PageLibPreprocessor & _pag;
    std::shared_ptr<spdlog::logger> my_logger_ptr;

    KeywordCacheManager& m_keyword_cache_manager;
    UrlCacheManager& m_url_cache_manager;

};

#endif
