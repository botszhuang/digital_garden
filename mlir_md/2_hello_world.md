# My MLIR Track #2 - hello world

By Botsz on April 2, 2026

**Disclaimer** : This is a documentation of my learning process only. Following these steps does not guarantee identical results.

- **Write a LLVM IR**

Write a simple LLVM IR that returns an exit signal, compile it into a shared library, and interface with it using C and Python.

```bash
# File: simple_llvm.ll

define i32 @smaple_llvm() {
    ret i32 30
}
```
This simple LLVM module is equivalent to:
```cpp
// File: sample_c.c
int sample_c() {
    return 30 ;
}
```
<span><!--more--></span>

In the following section, we compile the MLIR module and sample_c.c separately to compare their resulting LLVM IR.

## Compiling ```sample_c.c```

This compiles ```sample_c.c``` to LLVM IR by ```clang```

```bash
$ clang -S -emit-llvm sample_c.c -o sample_c.ll
```
- ```clang``` : the front-end compiler

- ```-S``` : ouput assembly

- ```-emit-llvm</code>``` : Specifies the output as LLVM IR

Then, the output is saved in ```sample_c.ll```.
```llvm
#File: sample_c.ll
; ModuleID = 'sample_c.c'
source_filename = "sample_c.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @sample_c() #0 {
  ret i32 30
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
```  
It works; however, the generated sample_c.ll is target-specific rather than platform-independent.

## Compiling this LLVM module 
```bash
#Translates platform-independent LLVM module into target-specific machine code.
$ llc -filetype=obj --relocation-model=pic sample_c.ll    -o sample_c.o
$ llc -filetype=obj --relocation-model=pic simple_llvm.ll -o sample_llvm.o
```
- ```llc``` : The LLVM backend complier to trnalates ```.ll``` into native assembly or object code.

- ```-filetype=obj``` : output the binary object file (```.o```)

- ```--relocation-model=pic``` : Enable position Independent Code. This is required for shared libary to Loaded at an arbitrary memory address

- ```simple_llvm.ll -o simple_llvm.o``` : Takes the LLVM IR input (```simple_llvm.ll```) and defines the final output path for the machine code object file.

## Linking a Shared Library
```bash
$ clang -shared -fPIC sample_c.o    -o libsample_c.so
$ clang -shared -fPIC sample_llvm.o -o libsample_llvm.so
```

- `clang` : It acts as a linker wrapper.
- `-shared` : To produce a shared object (`.so`) instead of a standalone executable.
- `-fPIC` : For Position Independent Code, it ensures the linker for a shared library environment.
- `example.o` : The input binary object file (the machine code).
- `-o libexample.so` : The output filename.

## Linking into an Executable file

### Solution 1 ### 
Here, by **C** language the shared libaray are called by ```main.c``` as a function.
```cpp
#include <stdio.h>
extern int sample_c() ;
extern int sample_llvm() ;
int main() {
    printf("sample_c: %d\n", sample_c());
    printf("sample_llvm: %d\n", sample_llvm());
    return 0 ;
}
```
Here, I execute the main program and retrieve the computational results.

```bash
$ clang main.c -Lout -lsample_c -lsample_llvm -Wl,-rpath,. -o main.exe
$./main.exe
sample_c: 30
sample_llvm: 30
```
### Solution 2 ### 
I use **Python** to run shared library by **ctypes**,
```python
import ctypes

module_c = ctypes.CDLL("./out/libsample_c.so")
module_llvm = ctypes.CDLL("./out/libsample_llvm.so")

# Setup and call the C function
module_c.sample_c.argtypes = []
module_c.sample_c.restype = ctypes.c_int
print(f"sample_c result: {module_c.sample_c()}")

# Setup and call the LLVM function
module_llvm.sample_llvm.argtypes = []
module_llvm.sample_llvm.restype = ctypes.c_int
print(f"sample_llvm result: {module_llvm.sample_llvm()}")
```
If the Python call the shared library successfully, it will print the number 30.
```bash 
# Execute 
$ python3 run.py

Running main.exe...
sample_c: 30
sample_llvm: 30

Running with python...
sample_c result: 30
sample_llvm result: 30
```



Check out [the full code for the example](mlir_2_hello_world)

## Reference
[1] [LLVM Language Reference Manual](https://llvm.org/docs/LangRef.html)

[2] [編譯器 LLVM 淺淺玩:以動手實作來認識 The LLVM Compiler Infrastructure](https://medium.com/@zetavg/%E7%B7%A8%E8%AD%AF%E5%99%A8-llvm-%E6%B7%BA%E6%B7%BA%E7%8E%A9-42a58c7a7309#e1a9)

[3] [Introduction to MLIR](https://www.stephendiehl.com/posts/mlir_introduction/)

## 純粹抱怨、碎碎念
>忙了一整天好累喔。沒有 hello world 的 hello world for LLVM IR : ) 
