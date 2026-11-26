#include "../../include/DeepSeekProvider.h"
#include "../../include/ChatGPTProvider.h"
#include "../../include/OllamaProvider.h"
#include "../../include/ChatGPTProvider.h"
#include "../../include/ChatSDK.h"
#include "../../include/mylog.h"
#include <gtest/gtest.h>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <iostream>

using namespace liulang;

// TEST(LLM_Deepseek_test, test)
// {

//     std::shared_ptr<Logger> _logger = std::make_shared<Logger>();
//     // _logger->logger_init("异步日志","./log", spdlog::level::err);
//     _logger->logger_init("异步日志", "stdout", spdlog::level::debug);
//     // auto model = std::make_shared<DeepSeekProvider::DeepSeekProvider>();
//     // std::cout<<model;
//     // ASSERT_TRUE(model!=nullptr);

//     // std::map<std::string, std::string> model_basic;
//     // model_basic["api_key"] = std::getenv("deepseek_api_key");
//     // bool init = model->initModel(model_basic);
//     // ASSERT_TRUE(init);
//     // // std::cout<<model->GetModelDes();
//     // std::map<std::string, std::string> model_config_deep;
//     // model_config_deep["temperature"] = "0.7";
//     // model_config_deep["max_token"] = "2048";

//     // std::cout << 2 << std::endl;
//     // std::vector<model_config::Message> mes;
//     // mes.push_back(Message("user", "你是谁"));
//     // std::string res = model->SendMessage(mes, model_config_deep);
//     // ASSERT_TRUE(res != "");
//     // std::cout << res;

//     auto model = std::make_shared<DeepSeekProvider::DeepSeekProvider>();
//     ASSERT_TRUE(model);
//     std::map<std::string, std::string> model_basic;
//     std::string key = std::getenv("deepseek_api_key");

//     model_basic["api_key"] = key;
//     std::cout << model->GetModelName();
//     bool init = model->initModel(model_basic);
//     ASSERT_TRUE(init);
//     std::cout << model->GetModelDes();

//     std::map<std::string, std::string> model_config_deep;
//     model_config_deep["temperature"] = "0.7";
//     model_config_deep["max_tokens"] = "2048";

//     std::vector<model_config::Message> mes;
//     mes.push_back(Message("user", "今天星期几"));
//     std::string res = model->SendMessageStream(mes, model_config_deep, [&](const std::string &mes, bool end) -> void
//                                                {
//         if(end)
//         {
//             INF("{}",mes);
//         }

//         else
//         INF("[DONE]"); });
//     ASSERT_TRUE(res != "");
//     std::cout << res << std::endl;
// }

TEST(ChatGPTProvider_test, test)
{
    std::shared_ptr<Logger> _logger = std::make_shared<Logger>();
    // _logger->logger_init("异步日志","./log", spdlog::level::err);
    _logger->logger_init("异步日志", "stdout", spdlog::level::debug);

    auto chatgpt = std::make_unique<ChatGPTProvider::ChatGPTProvider>();
    ASSERT_TRUE(chatgpt != nullptr);

    std::map<std::string, std::string> conf;
    conf["api_key"] = std::getenv("openai_api_key");
    chatgpt->GetModelName();
    bool init = chatgpt->initModel(conf);
    ASSERT_TRUE(init);
    ASSERT_TRUE(chatgpt->IsAvailable());
    std::vector<Message> mes;
    mes.push_back(Message("user", "你是谁"));
    mes.push_back(Message("user", "你具体是哪个版本的模型"));
    std::map<std::string, std::string> model_config;
    model_config["temperature"] = "0.7";
    model_config["max_token"] = "2048";
    std::string res = chatgpt->SendMessage(mes, model_config);
    std::cout << res << std::endl;

    // auto model = std::make_shared<ChatGPTProvider::ChatGPTProvider>();
    // ASSERT_TRUE(model!= nullptr );
    // std::map<std::string, std::string> model_basic;
    // std::string key = std::getenv("openai_api_key");

    // model_basic["api_key"] = key;
    // std::cout << model->GetModelName()<<std::endl;
    // bool init = model->initModel(model_basic);
    // ASSERT_TRUE(init);
    // std::cout << model->GetModelDes()<<std::endl;

    // std::map<std::string, std::string> model_config_deep;
    // model_config_deep["temperature"] = "0.7";
    // model_config_deep["max_tokens"] = "2048";

    // std::vector<model_config::Message> mes;
    // mes.push_back(Message("user", "你是哪个国家的模型"));
    // std::string res = model->SendMessageStream(mes, model_config_deep, [&](const std::string &mes, bool end) -> void
    //                                            {
    //     if(end)
    //     {
    //         INF("{}",mes);
    //     }

    //     else
    //     INF("[DONE]"); });

    // // ASSERT_TRUE(res != "");
    // std::cout << res << std::endl;
}

// TEST(OllamaProvider_test, test)
// {
//         std::shared_ptr<Logger> _logger = std::make_shared<Logger>();
//         // _logger->logger_init("异步日志","./log", spdlog::level::err);
//         _logger->logger_init("异步日志", "stdout", spdlog::level::debug);
//         std::unique_ptr<OllamaProvider::OllamaProvider> model=std::make_unique<OllamaProvider::OllamaProvider>();
//         ASSERT_TRUE(model!=nullptr);
//         std::map<std::string,std::string> modelconf;
//         modelconf["model_name"]="deepseek-r1:1.5b";
//         modelconf["model_des"]="Ollama 本地模型真牛逼";
//         modelconf["url"]="http://127.0.0.1:11434";

//         ASSERT_TRUE(model->initModel(modelconf));

//         std::vector<model_config::Message> message;
//         message.push_back(Message("user","你是谁"));
//         // message.push_back(Message("user","给我写一篇小说在500字左右"));

//         std::map<std::string,std::string> request;
//         request["temperature"]="0.7";
//         request["max_tokens"]="1024";
//         // std::string res=model->SendMessage(message,request);
//         std::string res=model->SendMessageStream(message,request,[=](const std::string data,bool statu) ->void
//         {

//             if(statu)
//             {
//                 INF("{}",data);
//             }
//             else
//                 INF("DONE");
//             return;
//         });

//         std::cout<<res<<std::endl;

// }

// TEST(ChatSDK_test, test)
// {
    
//     std::shared_ptr<Logger> _logger = std::make_shared<Logger>();
//     // _logger->logger_init("异步日志","./log", spdlog::level::err);
//     _logger->logger_init("异步日志", "stdout", spdlog::level::debug);

//     std::unique_ptr<ChatSDK::ChatSDK> chat=std::make_unique<ChatSDK::ChatSDK> ();
//     std::vector<std::shared_ptr<Model_Basic_Config>> confs;
//     std::shared_ptr<Cloud_Config> deepseek = std::make_shared<Cloud_Config>();
//     deepseek->_apikey = std::getenv("deepseek_api_key");
//     ASSERT_FALSE(deepseek->_apikey.empty());
//     deepseek->_maxtoken = 1024;
//     deepseek->_model_name="deepseek-chat";
//     deepseek->_model_tempture = 0.7;
//     confs.push_back(deepseek);

//     std::shared_ptr<Cloud_Config> chatgpt = std::make_shared<Cloud_Config>();
//     chatgpt->_apikey = std::getenv("openai_api_key");
//     ASSERT_FALSE(chatgpt->_apikey.empty());
//     chatgpt->_maxtoken = 1024;
//     chatgpt->_model_name="openai/gpt-6.1-terra-discount";
//     chatgpt->_model_tempture = 0.7;
//     confs.push_back(chatgpt);

//     std::shared_ptr<Ollama_Config> ollama = std::make_shared<Ollama_Config>();
//     ollama->_maxtoken = 1024;
//     ollama->_model_tempture = 0.7;
//     ollama->_model_name = "deepseek-r1:1.5b";
//     ollama->_modelurl = "http://127.0.0.1:11434";
//     confs.push_back(ollama);

//     // 初始化模型
//     bool init = chat->initModels(confs);
//     ASSERT_TRUE(init == true);

//     // 创建会话
//     std::string createsession = chat->createSession(chatgpt->_model_name);
//     ASSERT_FALSE(createsession.empty());

//     //
//     std::string message;
//     std::cout << ">>> ";
//     std::getline(std::cin, message);
//     auto response = chat->sendMessage(createsession, message);
//     ASSERT_FALSE(response.empty());

//     std::cout << ">>> ";
//     std::getline(std::cin, message);
//     response = chat->sendMessage(createsession, message);
//     ASSERT_FALSE(response.empty());

//     auto messages = chat->_sessionManager.getSessionMessage(createsession);
//     for (const auto &msg : messages)
//     {
//         std::cout << msg._role << ": " << msg._content << std::endl;
//     }
//     ASSERT_FALSE(messages.empty());
// }

int main(int argc, char *argv[])
{

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}