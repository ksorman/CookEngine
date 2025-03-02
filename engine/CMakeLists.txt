cmake_minimum_required(VERSION 3.12)

set(LIB_NAME CookEngine)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c??)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.h)

add_subdirectory(externals/glm)
add_subdirectory(externals/spdlog)
add_subdirectory(externals/glfw)

add_library(${LIB_NAME} STATIC ${HEADER_FILES} ${SRC_FILES})
target_include_directories(CookEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/externals/stb ${CMAKE_CURRENT_SOURCE_DIR}/externals/glm ${CMAKE_CURRENT_SOURCE_DIR}/externals/spdlog/include ${CMAKE_CURRENT_SOURCE_DIR}/externals/glfw/include ${CMAKE_CURRENT_SOURCE_DIR}/externals/stb)
target_link_libraries(CookEngine glm glfw Vulkan::Vulkan spdlog::spdlog)
