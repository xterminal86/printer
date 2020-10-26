cmake_minimum_required(VERSION 2.6)
set(TARGET_NAME printer)
project(${TARGET_NAME})
find_package(SDL2 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")

option(USE_SDL "Build with SDL2" false)

if (USE_SDL)
  add_definitions(-DUSE_SDL)
endif()

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(${TARGET_NAME} ${SOURCES})

if (USE_SDL)
  target_link_libraries(${TARGET_NAME} ${SDL2_LIBRARIES} SDL2_image)
else()
  target_link_libraries(${TARGET_NAME} ncurses)
endif()
