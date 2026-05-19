#!/bin/bash

llvm_dir=/workspace/llvm-bin

${llvm_dir}/bin/mlir-opt hello.mlir --pass-pipeline='builtin.module(gpu-kernel-outlining,gpu-lower-to-nvvm-pipeline,gpu-to-llvm)' | ${llvm_dir}/bin/mlir-runner --entry-point-result=void --shared-libs=${llvm_dir}/lib/libmlir_cuda_runtime.so --shared-libs=${llvm_dir}/lib/libmlir_runner_utils.so -e main