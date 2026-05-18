#!/bin/bash

llvm_dir=/workspace/llvm-project/build/bin

${llvm_dir}/mlir-opt gpu_hello.mlir \
  --pass-pipeline='builtin.module( \
    gpu-kernel-outlining, \
    convert-gpu-to-nvvm, \
    convert-nvvm-to-llvm, \
    reconcile-unrealized-casts)' \
  | ${llvm_dir}/mlir-runner \
  --shared-libs=${llvm_dir}/../lib/libmlir_cuda_runtime.so \
  --shared-libs=${llvm_dir}/../lib/libmlir_runner_utils.so \
  -e main

