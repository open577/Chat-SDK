#include "../include/mylog.h"
#include "../include/OllamaProvider.h"
#include <jsoncpp/json/json.h>
#include <httplib.h>
using namespace liulang;


namespace OllamaProvider
{
    // 初始化模型
    bool OllamaProvider::initModel(std::map<std::string, std::string> &modelconfig)
    {
        //找模型名字
        if (modelconfig.find("model_name") == modelconfig.end())
        {
            ERR("没有找到模型名字");
            return false;
        }
        _modelName = modelconfig.at("model_name");

        //找模型描述
        if (modelconfig.find("model_des") == modelconfig.end())
        {
            ERR("没有找到模型描述字段");
            return false;
        }
        _modelDes = modelconfig.at("model_des");

        //找url
        if(modelconfig.find("url")==modelconfig.end())
        {
            ERR("url无效 请检查你的URL是否设置成功");
            return false;
        }
        _url=modelconfig.at("url");


        //将模型状态设置为可运行
        _isavailable=true;
        return true;
    }
    // 测试模型是否有效
    bool OllamaProvider::IsAvailable()
    {
        return _isavailable;
    }
    // 获取模型名字
    std::string OllamaProvider::GetModelName()
    {
        return _modelName;
    }
    // 获取模型描述
    std::string OllamaProvider::GetModelDes()
    {
        return _modelDes;
    }
    // 全量响应
    std::string OllamaProvider::SendMessage(const std::vector<Message> &messages,
                                            const std::map<std::string, std::string> &requestParam)
    {
        //发送消息前先查看模型是否可以运行
        if(!IsAvailable())
        {
            ERR("模型暂时不可运行");
            return "";
        }

        //设置温度 最大token
        float temperature=0.7;
        int maxtokens=1024;
        if(requestParam.find("temperature")==requestParam.end())
        {
            ERR("模型配置中,在查找模型温度时没有找到温度配置");
            return "";
        }
        temperature=std::stod(requestParam.at("temperature"));

        if(requestParam.find("max_tokens")==requestParam.end())
        {
            ERR("模型配置中,在查找模型最大token时没有找到最大token配置");
            return "";
        }
        maxtokens=std::stoi(requestParam.at("max_tokens"));
        
        //构建消息
        Json::Value Message;
        for(auto message:messages)
        {
            Json::Value mes;
            mes["role"]=message._role;
            mes["content"]=message._content;
            Message.append(mes);
        }

        //构建请求体
        Json::Value options;
        options["temperature"]=temperature;
        options["num_ctx"]=maxtokens;

        Json::Value RequestBody;
        RequestBody["model"]=_modelName;
        RequestBody["stream"]=false;
        RequestBody["messages"]=Message;
        RequestBody["options"]=options;

        //将请求序列化
        Json::StreamWriterBuilder writebulid;
        std::string jsonreqest =Json::writeString (writebulid,RequestBody);
        
        //设置客户端请求参数
        httplib::Client client(_url);
        client.set_connection_timeout(30,0);
        client.set_read_timeout(30,0);

        //构建客户端请求头
        httplib::Headers headers=
        {
            {"Content-Type","application/json"}
        };

        //发送客户端请求
        auto response=client.Post("/api/chat",headers,jsonreqest,"application/json");
        if(!response)
        {
            ERR("客户端发送消息失败 状态码为{}",to_string(response.error()));
            return "";
        }
        if(response->status!=200)
        {
            ERR("应答状态错误{}",response->status);
            return "";
        }

        //对应答进行反序列化
        Json::CharReaderBuilder readbuild;
        Json::Value responsebody;
        std::istringstream sstream(response->body);
        std::string errmes;
        if(!Json::parseFromStream(readbuild,sstream,&responsebody,&errmes))
        {
            ERR("应答解析失败");
            return "";
        }
        //提取字节流
        std::string data;//存放解析后的正文
        if(responsebody.isMember("message")&&responsebody["message"].isMember("content"))
        {
            data+=responsebody["message"]["content"].asString();
            return data;
        }
        ERR("模型内容解析失败{}",responsebody);
        return "";
    }
    // 增量响应
    std::string OllamaProvider::SendMessageStream(const std::vector<Message> &messages,
                                                  const std::map<std::string, std::string> &requestParam,
                                                  std::function<void(std::string, bool)> callback)
    {
    }
}