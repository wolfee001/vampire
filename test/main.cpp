#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    std::cerr.setf(std::ios::fixed, std::ios::floatfield);
    std::cerr.precision(6);
    std::cerr.width(6);
    return RUN_ALL_TESTS();
}
