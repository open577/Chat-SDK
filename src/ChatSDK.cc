#include "../include/ChatSDK.h"
#include "../include/mylog.h"
#include "../include/DeepSeekProvider.h"
#include "../include/ChatGPTProvider.h"
#include "../include/OllamaProvider.h"

#include <unordered_set>
#include <map>

using namespace liulang;

namespace ChatSDK
{
    // 初始化模型
    bool ChatSDK::initModels(const std::vector<std::shared_ptr<Model_Basic_Config>> &configs)
    {
        // 注册所支持的模型
        registerAllProvider(configs);

        // 初始化所支持的模型
        initProviders(configs);

        _initialized = true;
        return true;
    }
    // 创建会话
    std::string ChatSDK::createSession(const std::string &modelName)
    { // 检测SDK是否初始化成功
        if (!_initialized)
        {
            ERR("ChatSDK::createSession: SDK is not initialized");
            return "";
        }

        // 通过sessionManager来创建会话
        auto sessionId = _sessionManager.createSession(modelName);
        if (sessionId.empty())
        {
            ERR("ChatSDK::createSession: create session failed");
            return "";
        }
        INF("ChatSDK::createSession: create session {} successed", sessionId);
        return sessionId;
    }

    // 获取指定会话
    std::shared_ptr<Session> ChatSDK::getSession(const std::string &sessionId)
    {
        if (!_initialized)
        {
            ERR("ChatSDK::getSession: SDK is not initialized");
            return nullptr;
        }

        // 通过sessionManager来获取会话
        auto session = _sessionManager.getSession(sessionId);
        if (!session)
        {
            ERR("ChatSDK::getSession: session {} not found", sessionId);
            return nullptr;
        }
        return session;
    }
    // 获取所有会话列表
    std::vector<std::string> ChatSDK::getSessionLists() const
    {
        if (!_initialized)
        {
            ERR("ChatSDK::getSessionLists: SDK is not initialized");
            return {};
        }

        // 通过sessionManager来获取所有会话
        return _sessionManager.getSessionLists();
    }
    // 删除指定会话
    bool ChatSDK::deleteSession(const std::string &sessionId)
    {
        if (!_initialized)
        {
            ERR("ChatSDK::deleteSession: SDK is not initialized");
            return false;
        }

        // 通过sessionManager来删除会话
        bool result = _sessionManager.deleteSession(sessionId);
        if (!result)
        {
            ERR("ChatSDK::deleteSession: delete session {} failed", sessionId);
            return false;
        }
        INF("ChatSDK::deleteSession: delete session {} successed", sessionId);
        return true;
    }
    // 获取可用的模型信息
    std::vector<Model_Info> ChatSDK::getAvailableModels() const
    {
        return _llmManager.getavailModel(); // 主要和模型进行交互
    }

    // 发送消息 - 全量返回
    std::string ChatSDK::sendMessage(const std::string &sessionId, const std::string &message)
    {
        if (!_initialized)
        {
            ERR("ChatSDK::deleteSession: SDK is not initialized");
            return "";
        }

        // 获取sessionid对应的session对象
        auto session = _sessionManager.getSession(sessionId);
        if (!session)
        {
            ERR("ChatSDK::sendMessage: session {} not found", sessionId);
            return "";
        }

        // 构建消息对象
        model_config::Message usermessage;
        usermessage._role = "user";
        usermessage._content = message;
        _sessionManager.addMessage(sessionId, usermessage);
        auto historymess = _sessionManager.getSessionMessage(sessionId);

        // 构建请求参数
        std::map<std::string, std::string> param;
        param["temperature"] = "0.7";
        param["max_tokens"] = "1024";
        // 发送消息
        std::string response = _llmManager.sendMessage(session->_modelname, historymess, param);
        if (response.empty())
        {
            ERR("ChatSDK::sendMessage: send message to model {} failed", session->_modelname);
            return "";
        }

        // 添加助手消息并更新会话时间
        Message assistantMessage("assistant", response);
        _sessionManager.addMessage(sessionId, assistantMessage);
        _sessionManager.updateSessionTimestamp(sessionId);
        INF("ChatSDK::sendMessage: send message to model {} successed", session->_modelname);
        return response;
    }
    // 发送消息 - 增量返回 - 流式响应
    std::string ChatSDK::sendMessageStream(const std::string &sessionId, const std::string &message,
                                           std::function<void(const std::string &, bool)> callback)
    {
        if (!_initialized)
        {
            ERR("ChatSDK::sendMessageStream: SDK is not initialized");
            return "";
        }

        // 获取sessionId对应的session对象
        auto session = _sessionManager.getSession(sessionId);
        if (!session)
        {
            ERR("ChatSDK::sendMessageStream: session {} not found", sessionId);
            return "";
        }

        // 构造历史消息
        Message userMessage("user", message);
        _sessionManager.addMessage(sessionId, userMessage);
        auto historyMessages = _sessionManager.getSessionMessage(sessionId);

        // 构建请求参数
        auto it = _modelConfigs.find(session->_modelname);
        if (it == _modelConfigs.end())
        {
            ERR("ChatSDK::sendMessageStream: model {} not found", session->_modelname);
            return "";
        }
        std::map<std::string, std::string> requestParam;
        requestParam["temperature"] = std::to_string(it->second->_model_tempture);
        requestParam["max_tokens"] = std::to_string(it->second->_maxtoken);

        // 调用LLMManager发送消息
        auto response = _llmManager.sendMessageStream(session->_modelname, historyMessages, requestParam, callback);
        if (response.empty())
        {
            ERR("ChatSDK::sendMessageStream: send message to model {} failed", session->_modelname);
            return "";
        }

        // 添加助手消息并更新会话时间
        Message assistantMessage("assistant", response);
        _sessionManager.addMessage(sessionId, assistantMessage);
        _sessionManager.updateSessionTimestamp(sessionId);
        INF("ChatSDK::sendMessageStream: send message to model {} successed", session->_modelname);
        return response;
    }

    // 注册所支持的模型
    void ChatSDK::registerAllProvider(const std::vector<std::shared_ptr<Model_Basic_Config>> &configs)
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
    { // 参数检测
        if (modelName.empty())
        {
            ERR("ChatSDK::initAPIModelProviders: modelName is empty");
            return false;
        }

        if (!apiConfig || apiConfig->_apikey.empty())
        {
            ERR("ChatSDK::initAPIModelProviders: apiKey is empty");
            return false;
        }

        // 初始化模型提供者
        if (_llmManager.isModelavailable(modelName))
        {
            INF("ChatSDK::initAPIModelProviders: model {} is already available", modelName);
            return true;
        }

        // 初始化模型
        std::map<std::string, std::string> modelParams;
        modelParams["api_key"] = apiConfig->_apikey;
        if (!_llmManager.initModel(modelName, modelParams))
        {
            ERR("ChatSDK::initAPIModelProviders: init model {} failed", modelName);
            return false;
        }

        // 模型配置
        _modelConfigs[modelName] = apiConfig;
        INF("ChatSDK::initAPIModelProviders: model {} init successed", modelName);
        return true;
    }
    // 初始化模型提供者 - Ollama模型提供者
    bool ChatSDK::initOllamaModelProviders(const std::string &modelName, const std::shared_ptr<Ollama_Config> &config)
    {
        // 参数检测
        if (modelName.empty())
        {
            ERR("ChatSDK::initOllamaModelProviders: modelName is empty");
            return false;
        }
        if (!config || config->_modelurl.empty())
        {
            ERR("ChatSDK::initOllamaModelProviders: endpoint is empty");
            return false;
        }

        // 初始化模型提供者
        if (_llmManager.isModelavailable(modelName))
        {
            INF("ChatSDK::initOllamaModelProviders: model {} is already available", modelName);
            return true;
        }

        // 初始化模型
        std::map<std::string, std::string> modelParams;
        modelParams["model_name"] = modelName;
        modelParams["model_desc"] = config->_modeldescrib;
        modelParams["endpoint"] = config->_modelurl;
        if (!_llmManager.initModel(modelName, modelParams))
        {
            ERR("ChatSDK::initOllamaModelProviders: init model {} failed", modelName);
            return false;
        }

        // 模型配置
        _modelConfigs[modelName] = config;
        INF("ChatSDK::initOllamaModelProviders: model {} init successed", modelName);
        return true;
    }

}