cmake_minimum_required(VERSION 3.0)
set(TARGET_NAME printer)
project(${TARGET_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")

option(USE_SDL "Build with SDL2" false)

if (USE_SDL)
  find_package(SDL2 REQUIRED)
  include_directories(${SDL2_INCLUDE_DIRS})
  add_definitions(-DUSE_SDL)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(${TARGET_NAME} ${SOURCES})

set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD_REQUIRED 11)

if (USE_SDL)
  if (WIN32)
    target_link_libraries(${TARGET_NAME} ${MINGW32_LIBRARY}
                                         ${SDL2MAIN_LIBRARY}
                                         ${SDL2_LIBRARY})
  else()
    target_link_libraries(${TARGET_NAME} SDL2)
  endif()
else()
  if (WIN32)
    target_link_libraries(${TARGET_NAME} pdcurses)
  else()
    target_link_libraries(${TARGET_NAME} ncurses)
  endif()
endif()
