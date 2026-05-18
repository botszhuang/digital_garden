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

```bash
# 1. 在大硬碟建立快取資料夾
mkdir -p /workspace/.ccache

# 2. 把快取路徑寫入系統環境變數，指定到大硬碟
echo 'export CCACHE_DIR=/workspace/.ccache' >> /root/.bashrc

# 3. 讓設定立刻生效
source /root/.bashrc

# 4. 限制快取最大上限為 30GB
ccache -M 30G
```

```bash
cd /workspace/llvm-project/build

rm -rf build && mkdir build && cd build

cmake -G Ninja ../llvm \
  -DLLVM_ENABLE_PROJECTS="mlir" \
  -DLLVM_TARGETS_TO_BUILD="X86;NVPTX" \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_ASSERTIONS=ON \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DLLVM_USE_LINKER=lld \
  -DLLVM_CCACHE_BUILD=ON \
  -DMLIR_ENABLE_CUDA_CONVERSIONS=ON

ninja check-mlir
```