#include <jsoncpp/json/json.h>
#include <string>
#include <iostream>
int main()
{
    Json::Value val;
    val["name"]="zhangsan";
    val["age"]=18;
    val["score"].append(88.2);
    val["score"].append(88.2);
    val["score"].append(88.2);

    std::string str=val.toStyledString();
    std::cout<<str<<std::endl;

    std::string name=val["name"].asString();
    int age= val["age"].asInt();
    float score1=val["socre"][0].asDouble();
    float score2=val["socre"][1].asDouble();
    float score3=val["socre"][2].asDouble();


    std::cout<<name<<" "<<age<<" "<<score1<<" "<<score2<<" "<<score3<<std::endl;

    return 0;
}