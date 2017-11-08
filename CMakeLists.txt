cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD          11)

project(log_facade)

set(SOURCES
    include/log_facade/Log.hpp
    include/log_facade/Backend.hpp
    include/log_facade/Combinators.hpp
    
    include/util/FuncRef.hpp
    include/util/FoldTuple.hpp
    
    src/LogFacade.cpp
)

add_library(log_facade          STATIC ${SOURCES})
target_include_directories(log_facade PUBLIC include PRIVATE src)

add_library(log_facade_shared   SHARED ${SOURCES})
target_include_directories(log_facade_shared PUBLIC include PRIVATE src)
