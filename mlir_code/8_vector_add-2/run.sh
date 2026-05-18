#!/bin/bash

current_dir=$(pwd)
#Example: If you built LLVM in ~/llvm-project/build
#llvm_dir=/usr/lib/llvm-20
llvm_dir=~/git/llvm-project/build

rm -rf build 
mkdir build 
cd build

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. \
  -DMLIR_DIR=${llvm_dir}/lib/cmake/mlir \
  -DLLVM_DIR=${llvm_dir}/lib/cmake/llvm

cmake --build .

ln -sf $(pwd)/compile_commands.json ${current_dir}/compile_commands.json

./mlir_cpp_example

python ${current_dir}/run_mlir_module.py 