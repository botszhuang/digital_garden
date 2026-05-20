# My MLIR Track #9 - GPU Hello World

By Botsz on May 20, 2026

**Disclaimer** : This is a documentation of my learning process only. Following these steps does not guarantee identical results.

This code implements a simple 'Hello World' program using the MLIR GPU dialect and demonstrates how to call it from the main host program. 

## 1. Module Attributes
```mlir
module attributes { gpu.container_module } { ... }
```
- ```module```: This is the outermost container, represented by the Builtin Dialect's ModuleOp. It defines a top-level operation that holds a single graph region and a single block, serving as an isolation boundary for all the operations it contains.

- ```attributes { gpu.container_module }```: This attribute tells the compiler that the module acts as a container enclosing code destined for the GPU[1-2].

**[Note] Metadata/Attribute**

- **The lable to Split Host and Device Code** : For heterogeneous computing (CPU + GPU), the compiler separates the process into Host code and Device code.
  - Host code : run on CPU and launch kernels.
  - Device code : Performs the functions on the GPU.

- **Container**(gpu.container_module): This is the top-level, host-side module in the CPU code. It acts as a parent container that encapsulates one or more nested submodules (marked with gpu.module) destined for the GPU.

## 2. GPU Module
```mlir
module attributes { gpu.container_module } {
  gpu.module @kernels {
    gpu.func @hello() kernel {
      %tid = gpu.thread_id x
      gpu.printf "Hello from thread %d\n", %tid : index
      gpu.return 
    }
  }
```
- ```gpu.module @kernels```: This defines a GPU-only kernel module named ```kernels```, which compiles directly into GPU backend code.

- ```gpu.func @hello() kernel```: This defines a GPU function, ```@hello```, where the kernel keyword indicates it serves as an entry point from the **Host**.

- ```%tid = gpu.thread_id x```: Gets the current thread ID in the X dimension. 

- ```gpu.printf```: Executes a formatted print inside the GPU for the current thread ID. **Note**: **Uses the** ```index``` **type**.

- ```gpu.return```: End the GPU function

## 3. Main Function & Launch GPU Kernel
```mlir
func.func @main() {
  %c1 = arith.constant 1 : index
  %cN = arith.constant 16 : index
  gpu.launch_func @kernels::@hello
      blocks in (%c1,%c1,%c1)
      threads in (%cN,%c1,%c1)
  return 
}
```
This part executes the logic in CPU (Host).
- ```%c1 = arith.constant 1 : index``` : Define a constant ```1```.
- ```%c1 = arith.constant 16 : index``` : Define a constant ```16```.
- ```gpu.launch_func @kernels::@hello```: Launch the function ```@hello```, defined in the ```@kernels``` module.

- ```blocks in (%c1,%c1,%c1)```: Set the **Grid** shape ```( x, y, z )``` and the total **Block** size = **1x1x1 = 1** in a **Grid**.
- ```threads in (%cN,%c1,%c1)```: Set the **Bolck** shape ```( x, y, z )``` and the toal **Thread** size = **16x1x1 = 16** in a **Bolck**.

## 4. The Complete code

```mlir
module attributes { gpu.container_module } {
  gpu.module @kernels {
    gpu.func @hello() kernel {
      %tid = gpu.thread_id x
      gpu.printf "Hello from thread %d\n", %tid : index
      gpu.return 
    }
  }

  func.func @main() {
    %c1 = arith.constant 1 : index
    %cN = arith.constant 16 : index
    gpu.launch_func @kernels::@hello
        blocks in (%c1,%c1,%c1)
        threads in (%cN,%c1,%c1)
    return 
  }
}
```
1. Host Request: The CPU (Host) uses gpu.launch_func to trigger the execution on the GPU (Device).
2. Allocation: The GPU hardware scheduler maps the request to its internal cores (SMs), creating 1 Block and 16 Threads as defined by your parameters.
3. Parallel Execution: Since there are 16 threads, the code inside @hello runs 16 times in parallel, but each thread has a unique ```%tid``` (Thread ID), allowing them to identify themselves.**

```bash
#output
Hello from thread 0
Hello from thread 1
Hello from thread 2
Hello from thread 3
Hello from thread 4
Hello from thread 5
Hello from thread 6
Hello from thread 7
Hello from thread 8
Hello from thread 9
Hello from thread 10
Hello from thread 11
Hello from thread 12
Hello from thread 13
Hello from thread 14
Hello from thread 15
```

The complete example is available in [mlir_code/9_gpu_hello_world-1](../mlir_code/9_gpu_hello_world-1/).

## Reference

[1] Dialect Specification: Check the MLIR GPU Dialect documentation https://mlir.llvm.org/docs/Dialects/GPU/

[2] 'gpu' Dialect https://github.com/llvm/llvm-project/blob/main/mlir/docs/Dialects/GPU.md