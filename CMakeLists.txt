IF(APPLE)
    set(CMAKE_CXX_COMPILER /usr/local/Cellar/gcc/11.1.0_1/bin/g++-11)
    set(CMAKE_C_COMPILER /usr/local/Cellar/gcc/11.1.0_1/bin/gcc-11)
ELSE()
    set(CMAKE_CXX_COMPILER /usr/bin/clang++)
    set(CMAKE_C_COMPILER /usr/bin/clang)
ENDIF()

project(ning)
cmake_minimum_required(VERSION 3.10)
set(CXX_STANDARD 14)
aux_source_directory(src SOURCE)
aux_source_directory(src/frontend SOURCE)
aux_source_directory(src/backend SOURCE)
aux_source_directory(src/global SOURCE)
aux_source_directory(src/IR SOURCE)
add_executable(ning ${SOURCE})
