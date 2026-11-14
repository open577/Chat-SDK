#pragma once
#include <vector>
#include <string>
#include <ctime>

namespace model_config
{
    struct Message
    {
        std::string _messageid; // 消息ID
        std::string _role;      // 角色
        std::string _content;   // 消息内容
        std::time_t _time;      // 发消息时间

        Message(std::string &role, std::string &content) : _role(role), _content(content) {}
    };

    struct Model_Basic_Config
    {
        std::string _model_name;     // 模型名字
        float _model_tempture = 0.7; // 模型温度
        int _maxtoken = 2048;        // 一次生成的最大token
        std::string _modeldescrib;   // 模型描述
        std::string _modelurl;       // 模型URL
        Model_Basic_Config(std::string &modelname, std::string &modeldiscrib, std::string &modelurl)
            : _model_name(modelname), _modeldescrib(modeldiscrib), _modelurl(modelurl)
        {
        }

        virtual ~Model_Basic_Config() {}
    };

    struct Cloud_Config : public Model_Basic_Config
    {
        std::string _apikey; // 云端模型apikey
        Cloud_Config(std::string &modelname, std::string &modeldiscrib, std::string &modelurl, std::string apikey)
            : Model_Basic_Config(modelname, modeldiscrib, modelurl),
              _apikey(apikey)
        {
        }
    };

    struct Ollama_Config : public Model_Basic_Config
    {
        Ollama_Config(std::string &modelname, std::string &modeldiscrib, std::string &modelurl)
            : Model_Basic_Config(modelname, modeldiscrib, modelurl)

        {
        }
    };

    struct Model_Info : public Model_Basic_Config
    {
        bool _isavailable;     // 模型是否可以运行
        std::string _provider; // 模型提供商
        Model_Info(std::string &modelname, std::string &modeldiscrib, std::string &modelurl, std::string &provider)
            : Model_Basic_Config(modelname, modeldiscrib, modelurl), _provider(provider), _isavailable(false)
        {
        }
    };

    struct Session
    {
        std::string _sessionid;         // 回话ID
        std::string _modelname;         // 会话使用的模型名称
        std::vector<Message> _messages; // 消息队列
        std::time_t _createdAt;         // 会话创建时间戳
        std::time_t _updatedAt;         // 会话更新时间
        Session(const std::string &modelname = "") : _modelname(modelname)
        {
        }
    };

}