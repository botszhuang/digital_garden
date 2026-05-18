# My MLIR Track #1 - Installation

By Botsz on March 31, 2026

**Disclaimer** : This is a documentation of my learning process only. Following these steps does not guarantee identical results.

In the beginning, LLVM was developed to investigate dynamic compilation techniques for both static and dynamic programming languages. However, now day LLVM severs as portable, high-level assembly language, called language-independent intermediate representation (IR) to optimize the computation [1].

MLIR (Multi-Level Intermediate Representation ) was developed in 2018 by Google and released as a sub-project of LLVM in 2019. It is developed to manage the complexity of heterogeneous (hybrid) architectures, enabling precise orchestration of computation loading and memory usage across diverse hardware targets[2].

In order to delicately manage heterogeneous architectures, working on the compiler becomes now an essential pain to truly orchestrate data movement and workload balance.

## Installing MLIR

It needs some dependencies to install the MLIR.
```bash
# Ubuntu
sudo apt update
apt-get update
apt-get install -y cmake ninja-build clang lld ccache git python3-pip python3-dev
```
Next, It has to clone the LLVM repository. MLIR is built directly within it, and the download may take a while due to the repository’s size.
```bash
git clone https://github.com/llvm/llvm-project
```
or
```bash
git clone --depth 1 --branch llvmorg-22.1.0 https://github.com/llvm/llvm-project.git
```
```--depth 1``` All we need is the latest code file for version 22.1.0; all previous historical records are unnecessary.
 
```bash
mkdir llvm-project/build
cd llvm-project/build
```

## CMake Configuration Flags
```bash
cmake -G Ninja ../llvm \
   -DLLVM_ENABLE_PROJECTS=mlir \
   -DLLVM_BUILD_EXAMPLES=ON \
   -DLLVM_TARGETS_TO_BUILD="Native;ARM;X86;NVPTX" \
   -DCMAKE_BUILD_TYPE=Release \
   -DLLVM_ENABLE_ASSERTIONS=ON \
   -DCMAKE_C_COMPILER=clang\
   -DCMAKE_CXX_COMPILER=clang++ \
   -DLLVM_USE_LINKER=lld \
   -DLLVM_CCACHE_BUILD=ON \
   -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
   -DMLIR_ENABLE_CUDA_CONVERSIONS=ON
```

- ```-G Ninja``` : Faster build system selection. Chooses the Ninja build system for faster compilation.

- ```-DLLVM_ENABLE_PROJECTS=mlir``` : Compiles MLIR along with LLVM.

- ```-DLLVM_BUILD_EXAMPLES=ON``` : Builds official MLIR/LLVM examples for reference.

- ```-DLLVM_TARGETS_TO_BUILD="Native;ARM;X86;NVPTX"``` : Target CPU support. Specifies target architectures: ‘Native’ for the host CPU, plus ARM and X86, and and NVIDIA GPUs.

- ```-DCMAKE_BUILD_TYPE=Release``` : Release optimized mode for performance and much smaller in size

- ```-DLLVM_ENABLE_ASSERTIONS=ON``` : If any error while running, it will print a error massage.

- ```-DCMAKE_C_COMPILER=clang``` and ``` -DCMAKE_CXX_COMPILER=clang++``` : Choose Clang to bild the entire system

- ```-DLLVM_USE_LINKER=lld``` : ```LLVM LLD``` is the high-speed linker designed specifically for modern multi-core processors. It is typically 4 to 20 times faster than traditional BFDs. Switching to LLD saves the lot of time to wait "stuck in the linking stage".

If any error like
```
collect2: fatal error: cannot find 'ld.lld' compilation terminated.
```
, install ```lld``` before executing CMake. 
```bash
sudo apt-get update && sudo apt-get install -y lld
```

- ```-DLLVM_CCACHE_BUILD=ON``` : With ccache enabled, LLVM/MLIR will remember the already-finished parts, making it much faster if you ever need to recompile.

- ```-DCMAKE_BUILD_WITH_INSTALL_RPATH=ON``` : Ninja builds the porjects and save to the output files to the current directory, llvm-project/build

- ```-DMLIR_ENABLE_CUDA_CONVERSIONS=ON``` : The key word in MLIR to enable CUDA-related dialect (Dialects) transformation and compilation support.

## Verify the Build.
It take hours depending on the native CPU.

```bash
cmake --build . --target check-mlir
sudo cmake --build . --target install
```

- ```cmake -- build . -- target check-mlir``` : Verifying the Build and it take 5–10 minutes depending on the native CPU.

- ```cmake -- build . -- target install ``` : It copies the finished executable files, headers, and libraries from the local build directory to ```/usr/local/bin``` the path specified by ```CMAKE_INSTALL_PREFIX```.

## Check the installation of MLIR

```bash
mlir-opt --version
```
Then. it shows the version of LLVM.
```bash
LLVM (http://llvm.org/):
  LLVM version 23.0.0git
  Optimized build with assertions.
```  
Here the LLVM and MLIR installation is done and it is ready to go programming with MLIR.

## Reference

[1] [LLVM Language Reference Manual](https://llvm.org/docs/LangRef.html)

[2] Lattner, C., Pienaar, J., et al. (2020). “MLIR: A Compiler Infrastructure for the End of Moore’s Law.” Published in the 2021 IEEE/ACM International Symposium on Code Generation and Optimization (CGO).[arXiv:2002.11054](https://arxiv.org/abs/2002.11054)

[3] [Introduction to MLIR]( https://www.stephendiehl.com/posts/mlir_introduction/)

## 純粹抱怨、碎碎念
>因為老黃不想遵守大家一起訂出來的工業標準，所以現在大家為了GPU通解得降到更底層的編譯器 / 組合語言去工作。這不是我們愛找麻煩，是環境逼我們的。

## ( Update, May 18 2026 ) The CMake Configuration Flags for cloud server

By skipping ```-DCMAKE_BUILD_WITH_INSTALL_RPATH=ON``` in the CMake setup, the ```llvm-project``` remains decoupled from the root system. This approach is highly convenient for developing and validating builds on cloud servers.

```bash
cd /workspace/llvm-project/build

rm -rf build && mkdir build && cd build

cmake -G Ninja ../llvm \
  -DCMAKE_MAKE_PROGRAM=$(which ninja) \
  -DCMAKE_C_COMPILER=$(which clang) \
  -DCMAKE_CXX_COMPILER=$(which clang++) \
  -DCMAKE_ASM_COMPILER=$(which clang) \
  -DLLVM_USE_LINKER=lld \
  -DLLVM_ENABLE_PROJECTS="mlir" \
  -DLLVM_TARGETS_TO_BUILD="X86;NVPTX" \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_ASSERTIONS=ON \
  -DLLVM_CCACHE_BUILD=ON \
  -DMLIR_ENABLE_CUDA_CONVERSIONS=ON 

ninja check-mlir
```

### Why skipping RPATH makes cloud server development more convenient:

- #### Environment Decoupling & Isolation 
    Instead of installing to the root system (e.g., ```/usr/local```), all bin and lib files remain strictly within their respective build directories. This allows you to maintain multiple separate workspaces on the same server simultaneously (e.g., ```llvm-project-feature-A``` and ```llvm-project-feature-B```) without any path conflicts.

- #### Rapid Iteration & Validation
    After modifying your MLIR code, you can immediately validate your changes by running ```./bin/mlir-opt``` directly from the build directory or executing ninja check-mlir. This eliminates the need for a tedious make install step to overwrite system files.

- #### Permission Conflict Avoidance
    Cloud environments—especially shared corporate or academic servers—rarely grant developers sudo or root write permissions. Keeping the build independent of the system path ensures you can complete the entire development loop entirely within your personal home directory.

## ( Update, May 19 2026 ) The Incremental build for NV gpu
```bash
#install nvcc 
apt-get update && apt-get install -y nvidia-cuda-toolkit

nvcc --version
```
output for reference:
```
nvcc: NVIDIA (R) Cuda compiler driver
Copyright (c) 2005-2023 NVIDIA Corporation
Built on Fri_Jan__6_16:45:21_PST_2023
Cuda compilation tools, release 12.0, V12.0.140
Build cuda_12.0.r12.0/compiler.32267302_0
```
```bash
#incremental build
cmake ../llvm -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DMLIR_ENABLE_CUDA_RUNNER=ON \
  -DMLIR_ENABLE_NVPTXCOMPILER=ON

ninja mlir-opt mlir-runner
```

## ( Update, May 19 2026 ) Alternative Solution: Release Binaries
## :star: :star: :star: Highly recommended solution—saves a lot of time and effort. :star: :star: :star:
```bash
cd /workspace

# 1. donwload the Release Binaries
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-22.1.5/LLVM-22.1.5-Linux-X64.tar.xz

# 2. decompress
tar -xvf  LLVM-22.1.5-Linux-X64.tar.xz --no-same-owner

mv LLVM-22.1.5-Linux-X64 llvm-bin

# 3. validation
./llvm-bin/bin/mlir-opt --version
```

- The ```--no-same-owner``` flag tells tar to make you (the user running the command) as the owner of the extracted files, rather than trying to preserve the original ownership from whoever created the archive.