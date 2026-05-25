#!/bin/bash

set -e

#Example: If you built LLVM in ~/llvm-project/build
llvm_dir="/workspace/llvm-bin"
exe_name="mlir_cpp_example"

cd /workspace/mlir_project/9_gpu_hello_world-2

echo "🔨 Cleaning old build..."
rm -rf build 

echo "📦 Configuring with CMake..."
mkdir build && cd build

cmake .. \
  -DEXE_NAME=${exe_name} \
  -DLLVM_DIR=${llvm_dir}/lib/cmake/llvm \
  -DMLIR_DIR=${llvm_dir}/lib/cmake/mlir \
  -DCMAKE_BUILD_TYPE=Release \
  || { echo "CMake failed"; exit 1; }

echo "===== Compiling ====="
make -j$(nproc) || { echo "Compilation failed"; exit 1; }

echo "===== Checking for compiled executable ====="
if [ ! -f "./${exe_name}" ]; then
    echo "Error: Compilation failed. ${exe_name} not found."
    exit 1
fi
chmod +x ./${exe_name}


echo "===== Build successful! ====="
echo "===== Running executable ====="
./${exe_name}
echo "===== Done! ====="