#include <gtest/gtest.h>
#include <log_facade/Log.hpp>
#include <log_facade/Backend.hpp>

TEST(LogFacade, BasicMacro)
{
    $log_info("Say Hi to the world!");
}
