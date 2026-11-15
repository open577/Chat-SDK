#include <httplib.h>

void Hello(const httplib::Request &request, httplib::Response &respense)
{
    std::cout << request.method << std::endl;
    std::cout << request.path << std::endl;
    std::cout << request.body << std::endl;
    for (auto obj : request.headers)
    {
        std::cout << obj.first << " = " << obj.second << std::endl;
    }

    std::string html_body = "<html><body><h1>Hello World</h1></body></html>";
    respense.set_content(html_body, "text/html");
    respense.status = 200;
    return;
}
int main()
{
    httplib::Server ser;
    ser.Get("/hello", Hello);
    ser.Get(R"(/numbers/(\d+))", [](const httplib::Request &request, httplib::Response &respense)
            {
        std::cout << request.method << std::endl;
    std::cout << request.path << std::endl;
    std::cout << request.body << std::endl;
    for (auto obj : request.headers)
    {
        std::cout << obj.first << " = " << obj.second << std::endl;
    } });
    ser.listen("0.0.0.0", 9090);
    return 0;
}