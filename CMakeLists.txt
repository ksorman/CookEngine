cmake_minimum_required(VERSION 3.12)

macro(SETUP_GROUPS src_files)
	foreach(FILE ${src_files})
		get_filename_component(PARENT_DIR "${FILE}" PATH)

		# skip src or include and changes /'s to \\'s
		set(GROUP "${PARENT_DIR}")
		string(REPLACE "/" "\\" GROUP "${GROUP}")

		source_group("${GROUP}" FILES "${FILE}")
	endforeach()
endmacro()

macro(SET_OUTPUT_NAMES projname)
	set_target_properties(${projname} PROPERTIES OUTPUT_NAME_DEBUG ${projname}_Debug)
	set_target_properties(${projname} PROPERTIES OUTPUT_NAME_RELEASE ${projname}_Release)
	set_target_properties(${projname} PROPERTIES OUTPUT_NAME_RELWITHDEBINFO ${projname}_ReleaseDebInfo)
endmacro()

set(PROJECT_NAME CookEngine)

project(${PROJECT_NAME} CXX)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c??)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.h)

file(GLOB_RECURSE OPENGL_SRC_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} opengl/*.c??)
file(GLOB_RECURSE OPENGL_HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} opengl/*.h)

include_directories(src)
add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES} ${OPENGL_SRC_FILES} ${OPENGL_HEADER_FILES})

add_definitions(-D_CONSOLE)

SETUP_GROUPS("${SRC_FILES}")
SETUP_GROUPS("${HEADER_FILES}")

SET_OUTPUT_NAMES(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

set(EXTERNALS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals")

set(GLFW_DIR ${EXTERNALS_DIR}/glfw)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory(${GLFW_DIR})
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_DIR}/include)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

set(GLAD_DIR ${EXTERNALS_DIR}/glad)
add_library(glad ${GLAD_DIR}/src/gl.c)
target_include_directories(glad PRIVATE ${GLAD_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLAD_DIR}/include)
target_link_libraries(${PROJECT_NAME} glad ${CMAKE_DL_LIBS})

set(GLM_DIR ${EXTERNALS_DIR}/glm)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_DIR})

set(OPENGL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/opengl)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENGL_DIR})