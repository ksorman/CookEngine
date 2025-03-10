cmake_minimum_required(VERSION 3.12)

set(LIB_NAME CookEngine)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c??)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.h)

if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

add_subdirectory(externals/glm)
add_subdirectory(externals/spdlog)
add_subdirectory(externals/glfw)
add_subdirectory(externals/assimp)
add_subdirectory(externals/vma)
add_subdirectory(externals/volk)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/shaders DESTINATION ${CMAKE_BINARY_DIR}/engine)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/engine)

add_library(${LIB_NAME} STATIC ${HEADER_FILES} ${SRC_FILES})

target_include_directories(CookEngine PUBLIC 
   ${CMAKE_CURRENT_SOURCE_DIR}/externals/stb 
   ${CMAKE_CURRENT_SOURCE_DIR}/externals/glm 
   ${CMAKE_CURRENT_SOURCE_DIR}/externals/spdlog/include 
   ${CMAKE_CURRENT_SOURCE_DIR}/externals/glfw/include 
   ${CMAKE_CURRENT_SOURCE_DIR}/externals/stb 
   ${CMAKE_CURRENT_SOURCE_DIR}/externals/assimp/include 
   ${CMAKE_CURRENT_SOURCE_DIR}/externals/vma/include)

   
target_link_libraries(CookEngine 
   glm 
   glfw 
   volk 
   Vulkan::Vulkan
   spdlog::spdlog
   assimp
   GPUOpen::VulkanMemoryAllocator)
