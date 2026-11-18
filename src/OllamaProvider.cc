#include "../include/mylog.h"
#include "../include/OllamaProvider.h"

namespace OllamaProvider
{
    // 初始化模型
    bool OllamaProvider::initModel(std::map<std::string, std::string> &modelconfig)
    {
        
    }
    // 测试模型是否有效
    bool OllamaProvider::IsAvailable()
    {
        return _isavailable;
    }
    // 获取模型名字
    std::string OllamaProvider::GetModelName()
    {
        return "deepseek-r1:1.5b";
    }
    // 获取模型描述
    std::string OllamaProvider::GetModelDes()
    {
        return "Ollama 模型";
    }
    // 全量响应
    std::string OllamaProvider::SendMessage(const std::vector<Message> &messages,
                            const std::map<std::string, std::string> &requestParam)
                            {}
    // 增量响应
    std::string OllamaProvider::SendMessageStream(const std::vector<Message> &messages,
                                  const std::map<std::string, std::string> &requestParam,
                                  std::function<void(std::string, bool)> callback)
                                  {}
}