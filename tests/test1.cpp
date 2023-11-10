#include <gtest/gtest.h>

#include "../src/libsat355.h"

TEST(libsat355, helloworld)
{
    int result = HelloWorld();
     ASSERT_NEAR(result, 0, 1.0e-11);
}