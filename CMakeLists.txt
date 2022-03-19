cmake_minimum_required(VERSION 3.16)
project(engine)

find_package(Git REQUIRED)

if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
    message(STATUS "Updating submodules")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive 
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
endif()

if(NOT (EXISTS "${PROJECT_SOURCE_DIR}/deps/glfw/CMakeLists.txt" OR 
        EXISTS "${PROJECT_SOURCE_DIR}/deps/glew/CMakeLists.txt"))
    message(FATAL_ERROR "The submodules were not downloaded! Please update submodules and try again.")
endif()

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Debug)
set(EXECUTABLE_OUTPUT_PATH "bin")
file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

add_subdirectory(${PROJECT_SOURCE_DIR}/deps/glfw EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/deps/glew EXCLUDE_FROM_ALL)

include_directories(${PROJECT_SOURCE_DIR}/src)

target_link_libraries(${CMAKE_PROJECT_NAME} glfw libglew_static -static)