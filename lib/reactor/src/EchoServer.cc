#include "reactor/EchoServer.h"
#include "cache/keywordCacheManager.h"
#include "cache/urlCacheManager.h"
#include "keyword_recom_preprocess/DictProducer.h"
#include "reactor/EventLoop.h"
#include "reactor/TcpConnection.h"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include <iostream>
#include <vector>

using std::cout;
using std::endl;

MyTask::MyTask(const TcpConnectionPtr &con, std::unordered_map<std::string, std::string> request, 
           DictProducer& pro, PageLibPreprocessor& pag, KeywordCacheManager& keyword_cachemanager,
           UrlCacheManager& url_cache_manager):
 _con(con)
, m_request(std::move(request))
, m_pro(pro)
, m_pag(pag)
, m_keyword_cache_manager(keyword_cachemanager)
, m_url_cache_manager(url_cache_manager)
{

}
void MyTask::process()
{
    std::string rst;

    


    //1.做业务逻辑的处理
    auto json = nlohmann::json::parse(_msg);

    // 将解析后的json对象转换为 std::unordered_map<std::string, std::string>
    std::unordered_map<std::string, std::string> request = json.get<std::unordered_map<std::string, std::string>>();
    
    

    if(request["search-type"] == "keyword") {
        //先查询缓存
        std::vector<std::string> rst_vec;
        if((rst_vec = m_keyword_cache_manager.query(local_id, request["body"])).empty())
        {
            spdlog::debug("\n\nKeyword缓存未触发\n\n");
            rst_vec = m_pro.run(request["body"]);
            m_keyword_cache_manager.insert(local_id, request["body"], rst_vec);
            for(auto& ele: rst_vec) {
                fmt::print("{} ", ele);
            }
        }

        nlohmann::json j = rst_vec;
        rst = j.dump(4);
    }
    else {
        //处理查询url的情况
        std::vector<std::vector<std::string>> rst_vec;

        if((rst_vec = m_url_cache_manager.query(local_id, request["body"])).empty())
        {
            spdlog::debug("\n\nUrl缓存未触发\n\n");

            
            rst_vec = m_pag.QueryPage(request["body"]);
            m_url_cache_manager.insert(local_id, request["body"], rst_vec);

        }
        

        nlohmann::json j = rst_vec;
        rst = j.dump(4);
        
    }
    //在此处，需要将接受到的数据msg打包给线程池进行处理
    //

    _msg = rst;
    _con->sendInLoop(_msg);
}

void MyTask::setMsg(std::string msg) {
    _msg = msg;
}

EchoServer::EchoServer(size_t threadNum, size_t queSize
                       , const string &ip
                       , unsigned short port,
                       DictProducer &pro,
                       PageLibPreprocessor &pag,
                       KeywordCacheManager& keyword_cachemanager,
                       UrlCacheManager& url_cache_manager)
: _pool(threadNum, queSize)
, _server(ip, port)
, _pro(pro)
, _pag(pag)
, m_keyword_cache_manager(keyword_cachemanager)
, m_url_cache_manager(url_cache_manager)
{

}

void EchoServer::setLogger(std::shared_ptr<spdlog::logger> my_logger) {
    my_logger_ptr = my_logger;
}

EchoServer::~EchoServer()
{

}

void EchoServer::start()
{
    _pool.start();
    using namespace  std::placeholders;
    _server.setAllCallback(std::bind(&EchoServer::onNewConnection, this, _1)
                           , std::bind(&EchoServer::onMessage, this, _1)
                           , std::bind(&EchoServer::onClose, this, _1));
    _server.start();
}

void EchoServer::stop()
{
    _pool.stop();
    _server.stop();
}

void EchoServer::onNewConnection(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has connected!" << endl;
}

void EchoServer::onMessage(const TcpConnectionPtr &con)
{
    string msg = con->receive();
    string rst;
    cout << ">>recv client msg = " << msg << endl;

    
    //1.做业务逻辑的处理
    auto json = nlohmann::json::parse(msg);

    // 将解析后的json对象转换为 std::unordered_map<std::string, std::string>
    std::unordered_map<std::string, std::string> request = json.get<std::unordered_map<std::string, std::string>>();
    
    /*
    if(request["search-type"] == "keyword") {
        std::vector<std::string> rst_vec = _pro.run(request["body"]);
        for(auto& ele: rst_vec) {
            fmt::print("{} ", ele);
        }
        nlohmann::json j = rst_vec;
        rst = j.dump(4);
    }
    else {
        //处理查询url的情况
        std::vector<std::vector<std::string>> rst_vec = _pag.QueryPage(request["body"]);

        nlohmann::json j = rst_vec;
        rst = j.dump(4);
        
    }
    */
    //在此处，需要将接受到的数据msg打包给线程池进行处理
    //
    MyTask task(con, request, _pro, _pag, m_keyword_cache_manager, m_url_cache_manager);
    task.setMsg(msg);
    _pool.addTask(std::bind(&MyTask::process, task));
}

void EchoServer::onClose(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has closed!" << endl;
}


void EchoServer::setCacheManager(KeywordCacheManager& cachemanager) {
    m_keyword_cache_manager = cachemanager;
}