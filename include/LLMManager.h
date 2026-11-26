#pragma once
#include "LLMProvider.h"
#include <map>
#include <memory>

namespace LLMManager
{
    class LLMManager
    {
    public:
        // 注册模型
        bool registerModel(const std::string modelname, std::unique_ptr<LLMProvider::LLMProvider> model);
        // 初始刷模型
        bool initModel(const std::string modelname, const std::map<std::string, std::string> modelpara);
        // 获取可用模型
        std::vector<model_config::Model_Info> getavailModel() const;
        // 查看模型是否可以用
        bool isModelavailable(const std::string modelName) const;
        // 给模型发送消息 全量返回
        std::string sendMessage(const std::string modelName, const std::vector<model_config::Message> message, const std::map<std::string, std::string> &requestParam);
        // 给模型发送消息 流式响应
        std::string sendMessageStream(const std::string modelname, const std::vector<model_config::Message> message, const std::map<std::string, std::string> &requestParam, std::function<void(const std::string &, bool)> callback);

    private:
        // 模型提供器 key 为模型名称 v具体实现 ————多态
        std::map<std::string, std::unique_ptr<LLMProvider::LLMProvider>> _provider;
        // 模型信息 key 为模型名称 v具体模型信息
        std::map<std::string, model_config::Model_Info> _modelinfo;
    };
}