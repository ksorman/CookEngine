cmake_minimum_required(VERSION 3.12)
set(PROJECT_NAME ProjectEngine)
project (${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (${PROJECT_NAME})

add_subdirectory(demo)
add_subdirectory(engine)

