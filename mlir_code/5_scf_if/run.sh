#!/bin/bash

current_dir=$(pwd)
#Example: If you built LLVM in ~/llvm-project/build
llvm_dir=../../llvm-project/build/

rm -rf build 
mkdir build 
cd build

cmake .. \
  -DMLIR_DIR=$(llvm_dir)/lib/cmake/mlir \
  -DLLVM_DIR=$(llvm_dir)/lib/cmake/llvm

cmake --build .

./mlir_cpp_example

python ${current_dir}/run_mlir_module.py 