cmake_minimum_required(VERSION 2.8.12)

option(TOOLBOXCPP_TESTS "Add compilation of toolboxcpp's own unittests" OFF)

project(toolboxcpp)

# Works with CMake 3.1 and above
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD          11)
# Anyway, adjust compiler flags depending on its ID
# Underscores are used to prevent literals from being treated as variable names
if("_${CMAKE_CXX_COMPILER_ID}_" STREQUAL "_GNU_")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")
elseif("_${CMAKE_CXX_COMPILER_ID}_" STREQUAL "_Clang_")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")
elseif("_${CMAKE_CXX_COMPILER_ID}_" STREQUAL "_MSVC_")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
endif()

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
    add_subdirectory(catch2)
    set(UNITTESTS Log)
    
    foreach(I ${UNITTESTS})
        add_executable(${I}_unittest test/${I}.cpp)
        target_link_libraries(${I}_unittest toolboxcpp Catch2::Catch)
        add_test(NAME "${I}-unittest" COMMAND $<TARGET_FILE:${I}_unittest>)
        set_tests_properties("${I}-unittest" PROPERTIES DEPENDS ${I}_unittest)
    endforeach()
endif()
