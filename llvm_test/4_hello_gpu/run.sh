#!/bin/bash

LLC=/workspace/llvm-bin/bin/llc

${LLC} -march=nvptx64 -mcpu=sm_89 hello.ll -o llvm_hello.ptx

nvcc -ptx hello.cu -o cuda_hello.ptx --gpu-architecture=sm_89

python text_ptx.py

# --- CU ---
# Hello World from GPU!
# ------------------
# --- LLVM ---
# hello world
# ------------------
