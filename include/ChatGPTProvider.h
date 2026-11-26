#pragma once
#include "common.h"
#include "LLMProvider.h"

using namespace LLMProvider;
namespace ChatGPTProvider
{
    class ChatGPTProvider : public LLMProvider
    {
    public:
        // 初始化模型
        bool initModel(std::map<std::string, std::string> &modelconfig);
        // 测试模型是否有效
        bool IsAvailable();

        // 获取模型名字
        std::string GetModelName();
        // 获取模型描述
        std::string GetModelDes();
        // 全量响应
        std::string SendMessage(const std::vector<Message> &messages,
                                const std::map<std::string, std::string> &requestParam);
        // 增量响应
        std::string SendMessageStream(const std::vector<Message> &messages,
                                      const std::map<std::string, std::string> &requestParam,
                                      std::function<void(const std::string &, bool)> callback);
    };
}