#include <gtest/gtest.h>
#include <iostream>
#include <unordered_map>
#include <memory>

TEST(unordered_map_test,find)
{
    std::unordered_map<int,int> _map;
    _map.insert(std::make_pair(1,1));
    _map.insert(std::make_pair(2,2));
    _map.insert(std::make_pair(3,3));
    ASSERT_EQ(_map.size(),2);
    ASSERT_EQ(_map.find(1)->second,1);
}
int main(int argc,char *argv[])
{
    // testing::InitGoogleTest(&argc,argv);
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

// #include <iostream>
// #include <gtest/gtest.h>
// int abs(int x)
// {
//     return x > 0 ? x : -x;
// }
// TEST(abs_test, test1)
// {
//     ASSERT_TRUE(abs(1) == 1) << "abs(1)=1";
//     ASSERT_TRUE(abs(-1) == 1);
//     ASSERT_FALSE(abs(-2) == -2);
//     ASSERT_EQ(abs(1), abs(-1));
//     ASSERT_NE(abs(-1), 0);
//     ASSERT_LT(abs(-1), 2);
// }

// int main(int argc, char *argv[])
// {
//     将命令⾏参数传递给gtest
//     testing::InitGoogleTest(&argc, argv);
//     运⾏所有测试案例
//     return RUN_ALL_TESTS();
// }