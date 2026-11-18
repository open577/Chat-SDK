#include "../../include/DeepSeekProvider.h"
#include "../../include/ChatGPTProvider.h"
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

    // auto chatgpt = std::make_unique<ChatGPTProvider::ChatGPTProvider>();
    // ASSERT_TRUE(chatgpt != nullptr);

    // std::map<std::string, std::string> conf;
    // conf["api_key"] = std::getenv("openai_api_key");
    // chatgpt->GetModelName();
    // bool init = chatgpt->initModel(conf);
    // ASSERT_TRUE(init);
    // ASSERT_TRUE(chatgpt->IsAvailable());
    // std::vector<Message> mes;
    // mes.push_back(Message("user", "你是谁"));
    // mes.push_back(Message("user", "你具体是哪个版本的模型"));
    // std::map<std::string, std::string> model_config;
    // model_config["temperature"] = "0.7";
    // model_config["max_token"] = "2048";
    // std::string res = chatgpt->SendMessage(mes, model_config);
    // std::cout << res << std::endl;

    auto model = std::make_shared<ChatGPTProvider::ChatGPTProvider>();
    ASSERT_TRUE(model!= nullptr );
    std::map<std::string, std::string> model_basic;
    std::string key = std::getenv("openai_api_key");

    model_basic["api_key"] = key;
    std::cout << model->GetModelName()<<std::endl;
    bool init = model->initModel(model_basic);
    ASSERT_TRUE(init);
    std::cout << model->GetModelDes()<<std::endl;

    std::map<std::string, std::string> model_config_deep;
    model_config_deep["temperature"] = "0.7";
    model_config_deep["max_tokens"] = "2048";

    std::vector<model_config::Message> mes;
    mes.push_back(Message("user", "你是哪个国家的模型"));
    std::string res = model->SendMessageStream(mes, model_config_deep, [&](const std::string &mes, bool end) -> void
                                               {
        if(end)
        {
            INF("{}",mes);
        }

        else
        INF("[DONE]"); });
    
    // ASSERT_TRUE(res != "");
    std::cout << res << std::endl;
}

int main(int argc, char *argv[])
{

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    return 0;
}