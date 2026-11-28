#include "ChatServer.h"
#include <string>
#include <gflags/gflags.h>
#include "mylog.h"
#include <string>
#include "../include/mylog.h"

DEFINE_string(host, "0.0.0.0", "服务器监听地址");
DEFINE_int32(port, 9090, "服务器端口");
DEFINE_string(log_level, "INF", "日志级别");
DEFINE_double(temperature, 0.7, "模型温度");
DEFINE_int32(maxtoken, 2048, "最大token数");
DEFINE_string(config_file, "./config", "配置文件路径");
DEFINE_string(ollama_model, "", "ollama模型名");
DEFINE_string(ollama_des, "", "ollama模型描述");
DEFINE_string(ollama_url, "", "ollama模型url");

std::string VERSION = "1.0.0";

// 获取apikey
std::string GetEvn(const std::string &key)
{
    if (!key.empty())
        return std::getenv(key.c_str());

    ERR("api_key获取失败 请检查环境变量");
    return "";
}

// 检查配置
bool checkconfig(ChatServer::ServerConfig &config)
{
    if (config.temperature < 0.0 || config.temperature > 2.0)
    {
        ERR("模型温度设置不合 {}", config.temperature);
        return false;
    }

    if (config.maxtoken <= 0)
    {
        ERR("模型token设置不合法 {}", config.maxtoken);
        return false;
    }

    if (config.deepseekapi.empty() && config.chatgptapi.empty())
    {
        ERR("云模型api暂未设置 请先设置apikey");
        return false;
    }

    if (config.ollamades.empty() || config.ollamamodelname.empty() || config.ollamaurl.empty())
    {
        ERR("ollama模型配置不全 请检查");
        return false;
    }

    return true;
}

void showAPIInfo()
{
    std::cout << "\nChatServer API接口说明:\n";
    std::cout << "  POST   /api/session              - 创建新会话\n";
    std::cout << "  GET    /api/sessions             - 获取所有会话列表\n";
    std::cout << "  GET    /api/models               - 获取可用模型列表\n";
    std::cout << "  DELETE /api/session/{session_id} - 删除指定会话\n";
    std::cout << "  GET    /api/session/{session_id}/history - 获取会话历史消息\n";
    std::cout << "  POST   /api/message              - 发送消息(全量返回)\n";
    std::cout << "  POST   /api/message/async        - 发送消息(流式返回)\n";
    std::cout << "\n使用示例:\n";
    std::cout << "  # 基本启动\n";
    std::cout << "  ./chatserver.exe\n";
    std::cout << "\n  # 指定端口启动\n";
    std::cout << "  ./chatserver.exe --port=9000\n";
    std::cout << "\n  # 使用指定配置文件\n";
    std::cout << "  ./chatserver.exe --config_file=my_config.conf\n";
    std::cout << "\n  # 设置环境变量后启动\n";
    std::cout << "  export DEEPSEEK_API_KEY=your_api_key\n";
    std::cout << "  ./chatserver.exe\n";
}

int main(int argc, char *argv[])
{
    try
    {
        // 显示帮助信息（当使用-h或--help时，gflags会自动显示帮助信息并退出）
        // 这里我们额外显示API接口说明
        if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help"))
        {
            showAPIInfo();
            return 0;
        }

        // 解析命令行参数
        gflags::SetUsageMessage("ChatServer - AI聊天服务器\n\n使用方法: ./AIChatServer [options]");
        gflags::ParseCommandLineFlags(&argc, &argv, true);
        gflags::SetVersionString(VERSION);
        std::ifstream file(FLAGS_config_file);
        if (file)
        {
            gflags::SetCommandLineOption("flagfile", FLAGS_config_file.c_str());
        }

        // 构建ServerConfig
        ChatServer::ServerConfig config;
        config.host = FLAGS_host;
        config.port = FLAGS_port;
        config.loglevel = FLAGS_log_level;
        config.temperature = FLAGS_temperature;
        config.maxtoken = FLAGS_maxtoken;

        // 从环境变量获取API密钥
        config.deepseekapi = GetEvn("deepseek_api_key");
        config.chatgptapi = GetEvn("openai_api_key");
        // 从命令行参数获取Ollama配置
        config.ollamamodelname = FLAGS_ollama_model;
        config.ollamades = FLAGS_ollama_des;
        config.ollamaurl = FLAGS_ollama_url;

        // 验证配置参数
        if (!checkconfig(config))
        {
            ERR("配置验证失败，请检查参数设置");
            return 1;
        }

        // 设置日志级别
        spdlog::level::level_enum logLevel = spdlog::level::info; // 默认INFO级别
        if (config.loglevel == "TRACE")
            logLevel = spdlog::level::trace;
        else if (config.loglevel == "DEBUG")
            logLevel = spdlog::level::debug;
        else if (config.loglevel == "INFO")
            logLevel = spdlog::level::info;
        else if (config.loglevel == "WARN" || config.loglevel == "WARNING")
            logLevel = spdlog::level::warn;
        else if (config.loglevel == "ERROR")
            logLevel = spdlog::level::err;
        else if (config.loglevel == "CRITICAL")
            logLevel = spdlog::level::critical;

        // 初始化日志组件
        liulang::Logger::logger_init("ChatServer", "stdout", logLevel);

        // 显示当前配置
        INF("AIChatServer 启动配置:");
        INF("  版本: {}", VERSION);
        INF("  主机: {}", config.host);
        INF("  端口: {}", config.port);
        INF("  日志级别: {}", config.loglevel);
        INF("  温度值: {}", config.temperature);
        INF("  最大Token: {}", config.maxtoken);
        INF("  DeepSeek API Key: {}", (config.deepseekapi.empty() ? "未设置" : "已设置"));
        INF("  ChatGPT API Key: {}", (config.chatgptapi.empty() ? "未设置" : "已设置"));
        INF("  Ollama 模型: {}", (config.ollamamodelname.empty() ? "未设置" : config.ollamamodelname));
        INF("  Ollama 模型描述: {}", (config.ollamades.empty() ? "未设置" : config.ollamades));
        INF("  Ollama 端点: {}", (config.ollamaurl.empty() ? "未设置" : config.ollamaurl));

        // 创建并启动ChatServer
        ChatServer::ChatServer server(config);
        if (server.start())
        {
            INF("ChatServer 启动成功!");
            INF("服务器地址: http://{}:{}", config.host, config.port);

            // 主线程等待，让服务器在单独线程中运行
            while (server.isRunning())
            {
                std::this_thread::sleep_for(std::chrono::seconds(100));
            }
        }
        else
        {
            ERR("ChatServer 启动失败!");
            return 1;
        }

        return 0;
    }
    catch (const std::exception &e)
    {
        ERR("发生异常: {}", e.what());
        return 1;
    }
    catch (...)
    {
        ERR("发生未知异常");
        return 1;
    }
}