cmake_minimum_required(VERSION 3.13)
project(MLIRExample CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED EXE_NAME)
    set(exe_name "mlir_cpp_example")
else()
    set(exe_name ${EXE_NAME})
endif()

find_package(LLVM REQUIRED CONFIG)
find_package(MLIR REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(AddLLVM)
include(AddMLIR)

include_directories(${LLVM_INCLUDE_DIRS} ${MLIR_INCLUDE_DIRS})

add_executable(${exe_name} mlir.cpp)

# Link ONLY essentials — fastest possible link time
target_link_libraries(${exe_name}
    PRIVATE
    MLIRIR
    MLIRSupport
    MLIRFuncDialect
    MLIRArithDialect
    LLVMSupport
)

