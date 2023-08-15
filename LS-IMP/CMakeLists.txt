PROJECT(LS_NIA)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# SET(CMAKE_CXX_FLAGS "-g -Ofast -static")
# close -static on mac
# SET(CMAKE_CXX_FLAGS "-Wall -g -Ofast -fsanitize=address")
# SET(CMAKE_CXX_FLAGS "-Wall -g -pg -Ofast")
SET(CMAKE_CXX_FLAGS "-Wall -g -Ofast")

INCLUDE_DIRECTORIES(src)

FILE(GLOB cpp_files "src/*.cpp")

ADD_EXECUTABLE(solver ${cpp_files})
target_compile_options(solver PRIVATE -pg)
