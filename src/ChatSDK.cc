#include "../include/ChatSDK.h"
#include "../include/mylog.h"
#include "../include/DeepSeekProvider.h"
#include "../include/ChatGPTProvider.h"
#include "../include/OllamaProvider.h"

#include <unordered_set>

using namespace liulang;

namespace ChatSDK
{
    // 初始化模型
    bool ChatSDK::initModels(const std::vector<std::shared_ptr<Model_Basic_Config>> &configs)
    {
        // deepseek
        if (!_llmManager.isModelavailable("deepseek-chat"))
        {
            auto deepseek = std::make_unique<DeepSeekProvider::DeepSeekProvider>();
            _llmManager.registerModel("deepseek-chat", std::move(deepseek));
            INF("deepseek模型注册成功");
        }

        // chatgpt
        if (!_llmManager.isModelavailable("openai/gpt-6.1-terra-discount"))
        {
            auto chatgpt = std::make_unique<ChatGPTProvider::ChatGPTProvider>();
            _llmManager.registerModel("openai/gpt-6.1-terra-discount", std::move(chatgpt));
            INF("openai/gpt-6.1-terra-discount模型注册成功");
        }

        // ollama
        std::unordered_set<std::string> modelnames;
        for (auto model : configs)
        {
            auto newpoint = std::dynamic_pointer_cast<Ollama_Config>(model);

            if (newpoint)
            {
                auto modelname = newpoint->_model_name;

                if (modelnames.find(modelname) == modelnames.end())
                {
                    // 是ollama模型 已经转化成功
                    modelnames.insert(newpoint->_model_name);

                    if (!_llmManager.isModelavailable(newpoint->_model_name))
                    {
                        auto ollama = std::make_unique<OllamaProvider::OllamaProvider>();

                        _llmManager.registerModel(newpoint->_model_name, std::move(ollama));
                        INF("ollama本地模型 {} 模型已经注册成功");
                    }
                }
            }
        }
        return true;
    }
    // 创建会话
    std::string ChatSDK::createSession(const std::string &modelName)
    {
    }
    // 获取指定会话
    std::shared_ptr<Session> ChatSDK::getSession(const std::string &sessionId)
    {
    }
    // 获取所有会话列表
    std::vector<std::string> ChatSDK::getSessionLists() const
    {
    }
    // 删除指定会话
    bool ChatSDK::deleteSession(const std::string &sessionId)
    {
    }
    // 获取可用的模型信息
    std::vector<Model_Info> ChatSDK::getAvailableModels() const
    {
    }

    // 发送消息 - 全量返回
    std::string ChatSDK::sendMessage(const std::string &sessionId, const std::string &message)
    {
    }
    // 发送消息 - 增量返回 - 流式响应
    std::string ChatSDK::sendMessageStream(const std::string &sessionId, const std::string &message,
                                           std::function<void(const std::string &, bool)> callback)
    {
    }

    // 注册所支持的模型
    void ChatSDK::registerAllProvider(const std::vector<std::shared_ptr<Model_Basic_Config>> &configs)
    {
    }
    // 初始化所支持的模型
    void ChatSDK::initProviders(const std::vector<std::shared_ptr<Model_Basic_Config>> &configs)
    {
        for (auto conf : configs)
        {
            if (auto modelconf = std::dynamic_pointer_cast<Cloud_Config>(conf)) // 云服务厂商
            {
                if (conf->_model_name == "deepseek-chat" || conf->_model_name == "openai/gpt-6.1-terra-discount")
                {
                    initAPIModelProviders(conf->_model_name, modelconf);
                }
                else
                {
                    ERR("{}模型暂不支持", conf->_model_name);
                }
            }

            if (auto modelconf = std::dynamic_pointer_cast<Ollama_Config>(conf))
            {
                initOllamaModelProviders(conf->_model_name, modelconf);
            }

            else
            {
                ERR("ollama {}模型暂不支持", conf->_model_name);
            }
        }
    }
    // 初始化模型提供者 - API模型提供者
    bool ChatSDK::initAPIModelProviders(const std::string &modelName, const std::shared_ptr<Cloud_Config> &apiConfig)
    {
    }
    // 初始化模型提供者 - Ollama模型提供者
    bool ChatSDK::initOllamaModelProviders(const std::string &modelName, const std::shared_ptr<Ollama_Config> &config)
    {
    }

}