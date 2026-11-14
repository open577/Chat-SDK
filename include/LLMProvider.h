#include <map>
#include <string>
#include "common.h"
#include <functional>
using namespace model_config;
namespace LLMProvider
{
    class LLMProvider
    {
    public:
        // 初始化模型
        virtual bool initModel(std::map<std::string &, std::string &> &modelconfig) = 0;
        // 测试模型是否有效
        virtual bool IsAvailable() = 0;
        // 获取模型名字
        virtual std::string GetModelName() = 0;
        // 获取模型描述
        virtual std::string GetModelDes() = 0;
        // 全量响应
        virtual std::string SendMessage(const std::vector<Message> &messages,
                                        const std::map<std::string, std::string> &requestParam) = 0;
        // 增量响应
        virtual std::string SendMessageStream(const std::vector<Message> &messages,
                                              const std::map<std::string, std::string> &requestParam,
                                              std::function<void(std::string, bool)> callback) = 0;

    private:
        bool _isavilable;
        std::string _apikey; // apikey
        std::string _url;    // url
    };
}