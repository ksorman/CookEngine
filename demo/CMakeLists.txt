cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME Demo)
set(COOK_ENGINE_SRC_PATH ${CMAKE_CURRENT_LIST_DIR}/../engine/src)
include_directories(${COOK_ENGINE_SRC_PATH})

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ./*.c??)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ./*.h)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SRC_FILES})

target_link_libraries(Demo CookEngine)

if(WIN32)
    set(DXC_BIN_DIR "${Vulkan_INCLUDE_DIR}/../Bin")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(DXC_BIN_DIR "${Vulkan_INCLUDE_DIR}/../Bin32")
    endif()
    find_file(DXC_RUNTIME_DLL NAMES dxcompiler.dll PATHS "${DXC_BIN_DIR}" NO_DEFAULT_PATH)
    if(NOT DXC_RUNTIME_DLL)
        message(FATAL_ERROR "dxcompiler.dll with SPIR-V support is required from the Vulkan SDK")
    endif()
    # The Windows SDK DLL found via PATH does not support SPIR-V.
    add_custom_command(TARGET Demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DXC_RUNTIME_DLL}" "$<TARGET_FILE_DIR:Demo>"
        VERBATIM)
endif()
