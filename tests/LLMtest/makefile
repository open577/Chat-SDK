llmtest.exe:LLMtest.cpp ../../src/DeepSeekProvider.cc ../../src/mylog.cxx ../../src/ChatGPTProvider.cc
	g++ $^ -o $@ -std=c++17 -lpthread -lfmt -lgtest -ljsoncpp -lcurl -lssl -lcrypto
.PHONY:clean
clean:
	rm *exe