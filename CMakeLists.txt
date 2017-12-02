cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD          11)

option(TOOLBOXCPP_TESTS "Add compilation of toolboxcpp's own unittests" OFF)

project(toolboxcpp)

set(SOURCES
    include/toolboxcpp/log/Log.hpp
    include/toolboxcpp/log/Logger.hpp
    include/toolboxcpp/log/Combinators.hpp
    include/toolboxcpp/log/Sinks.hpp
    include/toolboxcpp/log/DefaultFmt.hpp
    
    src/log/Logger.cpp

    include/toolboxcpp/util/FuncRef.hpp
    include/toolboxcpp/util/FoldTuple.hpp
    include/toolboxcpp/util/SourceLocation.hpp
)

source_group(include\\toolboxcpp\\log FILES    
    include/toolboxcpp/log/Log.hpp
    include/toolboxcpp/log/Logger.hpp
    include/toolboxcpp/log/Combinators.hpp
    include/toolboxcpp/log/Sinks.hpp
    include/toolboxcpp/log/DefaultFmt.hpp
)

source_group(include\\toolboxcpp\\util FILES
    include/toolboxcpp/util/FuncRef.hpp
    include/toolboxcpp/util/FoldTuple.hpp
    include/toolboxcpp/util/SourceLocation.hpp
)

source_group(src\\log FILES
    src/log/Logger.cpp
)

add_library(toolboxcpp          STATIC EXCLUDE_FROM_ALL ${SOURCES})
target_include_directories(toolboxcpp        PUBLIC include PRIVATE src)

add_library(toolboxcpp_shared   SHARED EXCLUDE_FROM_ALL ${SOURCES})
target_include_directories(toolboxcpp_shared PUBLIC include PRIVATE src)

if(TOOLBOXCPP_TESTS)
    enable_testing()
    option(gtest_force_shared_crt "" ON)
    add_subdirectory(googletest/googletest)
    set(UNITTESTS Log)
    
    foreach(I ${UNITTESTS})
        add_executable(${I}_unittest test/${I}.cpp)
        target_link_libraries(${I}_unittest gtest_main toolboxcpp)
        add_test(NAME "${I}-unittest" COMMAND $<TARGET_FILE:${I}_unittest>)
        set_tests_properties("${I}-unittest" PROPERTIES DEPENDS ${I}_unittest)
    endforeach()
endif()
