project(ning)
cmake_minimum_required(VERSION 3.10)
set(CXX_STANDARD 14)
find_package(LLVM REQUIRED)
aux_source_directory(. SOURCE)
#LIST(REMOVE_ITEM SOURCE example1)
#LIST(REMOVE_ITEM SOURCE example2)
#LIST(REMOVE_ITEM SOURCE Makefile)

include_directories(${LLVM_INCLUDE_DIR})
add_executable(ning ${SOURCE})

#target_link_libraries(tony ${LLVM_LABRARIES})
#execute_process(COMMAND llvm-config --libs all
#        OUTPUT_VARIABLE llvm_libs)
target_link_libraries(ning ${llvm_libs})
