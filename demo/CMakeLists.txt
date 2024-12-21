cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME Demo)
set(COOK_ENGINE_SRC_PATH ${CMAKE_CURRENT_LIST_DIR}/../engine/src)
include_directories(${COOK_ENGINE_SRC_PATH})

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(Demo CookEngine)