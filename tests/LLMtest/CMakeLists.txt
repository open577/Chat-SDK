cmake_minimum_required(VERSION 3.10)

# 该目录可独立构建，也可被顶层工程通过 add_subdirectory 引入
project(LLMtest LANGUAGES CXX)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 依赖的源文件（相对于本 CMakeLists.txt 所在目录）
set(LLMTEST_SOURCES
    LLMtest.cpp
    ../../src/DeepSeekProvider.cc
    ../../src/mylog.cxx
    ../../src/ChatGPTProvider.cc
    ../../src/OllamaProvider.cc
    ../../src/LLMManager.cc
    ../../src/DataManager.cc
    ../../src/SessionManager.cc
    ../../src/ChatSDK.cc
)

add_executable(llmtest ${LLMTEST_SOURCES})

# 头文件搜索路径（源码里用的是 "../../include/xxx.h" 相对包含，
# 这里额外把源文件目录加入，便于编译器处理）
target_include_directories(llmtest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# 链接库（对应 makefile 中 -lpthread -lfmt -lgtest -ljsoncpp -lcurl -lssl -lcrypto -lsqlite3）
find_package(Threads REQUIRED)
target_link_libraries(llmtest PRIVATE Threads::Threads)

# 其余第三方库直接按库名链接
target_link_libraries(llmtest PRIVATE
    fmt
    gtest
    jsoncpp
    curl
    ssl
    crypto
    sqlite3
)