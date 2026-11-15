#include <gtest/gtest.h>
#include <unordered_map>
#include <iostream>


//全局环境
std::unordered_map<int, int> _map;

class GlobalEnvironment : public testing::Environment
{
public:
    virtual void SetUp() override
    {
        std::cout << "配置全局环境" << std::endl;
    }

    virtual void TearDown() override
    {
        std::cout << "销毁环境变量" << std::endl;
        _map.clear();
    }

private:
};

TEST(GlobalEnvironment, insert)
{
    _map.insert(std::make_pair(1, 1));
    _map.insert(std::make_pair(2, 2));
    _map.insert(std::make_pair(3, 3));
    ASSERT_EQ(_map.size(), 3);
}

TEST(unordered_map_erase, erase)
{
    _map.erase(1);
    ASSERT_EQ(_map.size(), 2);
}

TEST(unordered_map_num, num)
{
    _map.erase(1);
    ASSERT_EQ(_map.size(), 3);
}
int main()
{
    testing::AddGlobalTestEnvironment(new GlobalEnvironment);
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
    return 0;
}