cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD          11)

option(LOG_FACADE_TESTS "Add compilation of log_facade's own unittests" OFF)

project(log_facade)

set(SOURCES
    include/log_facade/Log.hpp
    
    include/log_facade/logger/Logger.hpp
    include/log_facade/logger/Combinators.hpp
    include/log_facade/logger/Sinks.hpp
    include/log_facade/logger/DefaultFmt.hpp
    
    include/log_facade/util/FuncRef.hpp
    include/log_facade/util/FoldTuple.hpp
    include/log_facade/util/SourceLocation.hpp
    
    src/LogFacade.cpp
)

source_group(include\\log_facade FILES
    include/log_facade/Log.hpp
)

source_group(include\\log_facade\\logger FILES    
    include/log_facade/logger/Logger.hpp
    include/log_facade/logger/Combinators.hpp
    include/log_facade/logger/Sinks.hpp
    include/log_facade/logger/DefaultFmt.hpp
)

source_group(include\\log_facade\\util FILES
    include/log_facade/util/FuncRef.hpp
    include/log_facade/util/FoldTuple.hpp
    include/log_facade/util/SourceLocation.hpp
)

source_group(src FILES src/LogFacade.cpp)

add_library(log_facade          STATIC ${SOURCES})
target_include_directories(log_facade PUBLIC include PRIVATE src)

add_library(log_facade_shared   SHARED ${SOURCES})
target_include_directories(log_facade_shared PUBLIC include PRIVATE src)

if(LOG_FACADE_TESTS)
    enable_testing()
    option(gtest_force_shared_crt "" ON)
    add_subdirectory(googletest/googletest)
    add_executable(log_facade_unittest src/unittest.cpp)
    target_link_libraries(log_facade_unittest gtest_main log_facade)
    add_test(NAME log_facade_unittest COMMAND $<TARGET_FILE:log_facade_unittest>)
endif()
