cmake_minimum_required(VERSION 3.1)
project(VarInt CXX)

set (CMAKE_CXX_STANDARD 11)
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

include_directories(include)
file(GLOB VARINT_SRC src/*.cpp src/*.c include/*.hpp include/*.h)
add_executable(${PROJECT_NAME} ${VARINT_SRC})
