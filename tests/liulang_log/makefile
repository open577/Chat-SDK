mylog.exe:my_log_test.cc log.cc
	g++ $^ -o $@ -lfmt -lpthread -lgflags -lspdlog
.PHONY:clean
clean:
	rm mylog.exe