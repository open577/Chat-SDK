#include "../../include/DeepSeekProvider.h"
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

//     auto model = std::make_shared<DeepSeekProvider::DeepSeekProvider>();
//     ASSERT_TRUE(model);

//     std::map<std::string, std::string> model_basic;
//     model_basic["api_key"] = std::getenv("deepseek_apikey");
//     bool init=model->initModel(model_basic);
//     ASSERT_TRUE(init);
//     // std::cout<<model->GetModelDes();
//     std::map<std::string, std::string> model_config_deep;
//     model_config_deep["temperature"] = "0.7";
//     model_config_deep["max_token"] = "2048";

//     std::cout<<2<<std::endl;
//     std::vector<model_config::Message> mes;
//     mes.push_back(Message("user", "你是谁"));
//     std::string res=model->SendMessage(mes, model_config_deep);
//     std::cout<<res;

// }

int main(int argc, char *argv[])
{
    std::shared_ptr<Logger> _logger = std::make_shared<Logger>();
    // _logger->logger_init("异步日志","./log", spdlog::level::err);
    _logger->logger_init("异步日志", "stdout", spdlog::level::debug);

    auto model = std::make_shared<DeepSeekProvider::DeepSeekProvider>();

    std::map<std::string, std::string> model_basic;
    std::string key = std::getenv("deepseek_api_key");  
    std::cout << key << std::endl;
    std::cout << 2 << std::endl;

    model_basic["api_key"] = key;
    std::cout << model->GetModelName();
    bool init = model->initModel(model_basic);
    std::cout << model->GetModelDes();

    std::map<std::string, std::string> model_config_deep;
    model_config_deep["temperature"] = "0.7";
    model_config_deep["max_tokens"] = "2048";

    std::vector<model_config::Message> mes;
    mes.push_back(Message("user", "你是谁"));
    std::string res = model->SendMessage(mes, model_config_deep);
    std::cout << res;

    // testing::InitGoogleTest(&argc, argv);
    // return RUN_ALL_TESTS();
    return 0;
}