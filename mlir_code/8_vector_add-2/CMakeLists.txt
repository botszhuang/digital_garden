cmake_minimum_required(VERSION 3.15)
project(mlir_cpp_test)

get_filename_component(MLIR_BUILD_INCLUDE_DIR "${MLIR_DIR}/../../../tools/mlir/include" ABSOLUTE)
get_filename_component(LLVM_BUILD_INCLUDE_DIR "${LLVM_DIR}/../../../tools/llvm/include" ABSOLUTE)

# 1. Set C++ Standard (MLIR requires 17+)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2.Find MLIR/LLVM
find_package(MLIR REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)
file(GLOB INCLUDE_SOURCES "include/*.cpp" )

set(SOURCES mlir.cpp ${INCLUDE_SOURCES} )

add_executable(mlir_cpp_example ${SOURCES})

target_include_directories( mlir_cpp_example PRIVATE 
    ${MLIR_INCLUDE_DIRS} 
    ${LLVM_INCLUDE_DIRS} 
    ${MLIR_BUILD_INCLUDE_DIR}  # 解決 .h.inc 找不到的問題
    ${LLVM_BUILD_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_compile_definitions(mlir_cpp_example PRIVATE ${LLVM_DEFINITIONS})

get_property(mlir_libs GLOBAL PROPERTY MLIR_ALL_LIBS)
get_property(llvm_libs GLOBAL PROPERTY LLVM_ALL_LIBS)

target_link_libraries(mlir_cpp_example PRIVATE
    ${mlir_libs}
    ${llvm_libs}
    )    
