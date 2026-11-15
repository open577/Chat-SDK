#include <gtest/gtest.h>
#include <unordered_map>
#include <iostream>

class LocalEnvironment : public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "局部环境变量" << std::endl;
    }

    virtual void TearDown() override
    {
        std::cout << "局部环境变量销毁" << std::endl;
        _map.clear();
    }
    std::unordered_map<int, int> _map;

private:
};

TEST_F(LocalEnvironment, insert)
{
    _map.insert(std::make_pair(1, 1));
    _map.insert(std::make_pair(2, 1));
    _map.insert(std::make_pair(3, 1));

    ASSERT_EQ(_map.size(),3);
}

TEST_F(LocalEnvironment, erase)
{
    _map.insert(std::make_pair(1, 1));
    _map.insert(std::make_pair(2, 1));
    _map.insert(std::make_pair(3, 1));

    _map.erase(1);
    ASSERT_EQ(_map.size(),2);
}

int main()
{
    testing::InitGoogleTest();
    RUN_ALL_TESTS();
    return 0;
}