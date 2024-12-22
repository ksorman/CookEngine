cmake_minimum_required(VERSION 3.12)

set(LIB_NAME CookEngine)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c??)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.h)
add_library(${LIB_NAME} STATIC ${HEADER_FILES} ${SRC_FILES})
target_link_libraries(CookEngine Vulkan::Vulkan)
