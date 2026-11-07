cmake_minimum_required(VERSION 3.12)
set(PROJECT_NAME ProjectEngine)
project (${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (${PROJECT_NAME})

set(SPDLOG_NO_EXCEPTIONS ON CACHE BOOL "Build spdlog without exceptions" FORCE)

add_subdirectory(engine)
add_subdirectory(demo)

# Keep Assimp's internal exception handling separate from the engine policy.
if(MSVC)
    target_compile_options(CookEngine PUBLIC /EHs-c- /we4530 /we4577 /GR-)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(CookEngine PUBLIC /clang:-fno-exceptions)
    endif()
else()
    target_compile_options(CookEngine PUBLIC -fno-exceptions -fno-rtti)
endif()
if(WIN32)
    target_compile_definitions(CookEngine PUBLIC _HAS_EXCEPTIONS=0 _ATL_NO_EXCEPTIONS)
endif()

