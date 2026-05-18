#!/bin/bash

llvm_dir=../../llvm-project/build/
exe_name=mlir_cpp_example

rm -rf build && mkdir build && cd build

cmake .. \
  -DEXE_NAME=${exe_name} \
  -DMLIR_DIR=${llvm_dir}lib/cmake/mlir \
  -DLLVM_DIR=${llvm_dir}lib/cmake/llvm \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=gold"

cmake --build . 

if [ ! -f ${exe_name} ]; then
    echo "Error: ${exe_name} not found."
    exit 1
fi

chmod +x ./${exe_name}

./${exe_name}
