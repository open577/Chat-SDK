#include "../include/LLMManager.h"
#include "../include/mylog.h"

namespace LLMManager
{
    // 注册模型
    bool LLMManager::registerModel(const std::string modelname, std::unique_ptr<LLMProvider::LLMProvider> model)
    {
        if (!model)
        {
            INF("无法注册模型 {}", modelname);
            return false;
        }

        _provider[modelname] = std::move(model);
        _modelinfo[modelname] = model_config::Model_Info(modelname);
        INF("模型注册成功 modelName: {}", modelname);
        return true;
    }

    // 初始刷模型
    bool LLMManager::initModel(const std::string modelname, std::map<std::string, std::string> modelpara)
    {
        auto it = _provider.find(modelname);
        if (it == _provider.end())
        {
            ERR("模型还未注册成功");
            return false;
        }

        bool issuccess = it->second->initModel(modelpara);
        if (!issuccess)
        {
            ERR("模型初始化失败{}", modelname);
            return false;
        }

        _modelinfo[modelname]._modeldescrib = it->second->GetModelDes();
        _modelinfo[modelname]._model_name = it->second->GetModelName();
        _modelinfo[modelname]._isavailable = true;
        return true;
    }
    // 获取可用模型
    std::vector<model_config::Model_Info> LLMManager::getavailModel() const
    {
        std::vector<model_config::Model_Info> models;
        for (auto it : _modelinfo)
        {
            if (it.second._isavailable)
            {
                models.push_back(it.second);
            }
        }

        return models;
    }
    // 查看模型是否可以用
    bool LLMManager::isModelavailable(const std::string modelname) const
    {
        auto model = _modelinfo.find(modelname);
        if (model == _modelinfo.end())
        {
            INF("模型 {} 还未初始化成功 暂不可用", modelname);
            return false;
        }

        return model->second._isavailable;
    }

    // 给模型发送消息 全量返回
    std::string LLMManager::sendMessage(const std::string modelname, const std::vector<model_config::Message> message,
                                        const std::map<std::string, std::string> &requestParam)
    {
        // 检测模型是否注册 是否可以运行
        auto it = _modelinfo.find(modelname);
        if (it == _modelinfo.end())
        {
            ERR("模型没有注册 暂不可发消息 请先注册模型 {}", modelname);
            return "";
        }

        if (!it->second._isavailable)
        {
            ERR("模型暂不可运行 {}", modelname);
            return "";
        }

        return _provider[modelname]->SendMessage(message, requestParam);
    }
    // 给模型发送消息 流式响应
    std::string LLMManager::sendMessageStream(const std::string modelname, const std::vector<model_config::Message> message,
                                  const std::map<std::string, std::string> &requestParam, std::function<void(const std::string &, bool)> callback)
    {
        auto it = _modelinfo.find(modelname);
        if (it == _modelinfo.end())
        {
            ERR("模型没有注册 暂不可发消息 请先注册模型 {}", modelname);
            return "";
        }

        if (!it->second._isavailable)
        {
            ERR("模型暂不可运行 {}", modelname);
            return "";
        }

       return _provider[modelname]->SendMessageStream(message, requestParam,callback);
    }

}