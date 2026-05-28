---
title: "Generating my First LLVM IR: A Minimal Example"
auther: "Botsz"
date: 2026-05-29
---
This tutorial demonstrates how to use the LLVM C++ API to programmatically generate a simple LLVM Intermediate Representation (IR) module. Our goal is to build a program that replicates a basic C function returning a constant.

## The C Equivalence
If this were written in C, the code would look like this:
```c
int main() {

    return 42;
}
```
## About the Rquired Header Files
To interact with the LLVM infrastructure, we need to include the core IR management headers:
```cpp
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
```
- `llvm/IR/LLVMContext.h` provids the definition for the `llvm::LLVMContext` class, which serves as the top-level container for managing global state and core data within the LLVM infrastructure[1].
- `llvm::Module.h` class is the top-level container for all other LLVM Intermediate Representation (IR) objects. It holds the globals variables, functions, libraries, a symbol table, and various information about the target's characteristics[2].
- `llvm/IR/IRBuilder.h` provides a uniform API for creating instructions and inserting them into a basic block: either at the end of a BasicBlock, or at a specific iterator location in a block[3].
- `llvm/IR/Function.h` 
- `llvm/IR/Verifier.h` provides a verifier interface to valify the input to the system and the transformations[4].
- `llvm/Support/raw_ostream` implements an extremely fast bulk output stream to the system and **only output only**[5].

## Setting Context, Module & IRBuilder
Every LLVM program requires a core environment setup before generating any code:
```cpp
    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("simple_module", *ctx);
    llvm::IRBuilder<> b(*ctx);
```
 - The **Context** acts as the global container of all LLVM state , including unique symbols and operation definitions. **Context** holds the definitions and rules that make the code valid. Therefore, the necessary Dialects must be explicitly loaded for the context to recognize specific operations.

- A **Module** (```module```) is initialized as the top-level container. While the context provides the rules and vocabulary, the Module acts as the structural root that physically holds the functions and arithmetic operations defined by the developer.    

- An **IRBuilder** is utilized as a cursor to manage the insertion of functions and operations into the Module. 

## Defining the Function Prototype
Next, we must explicitly declare the main function signature (int main()) and register it with our module.
```c
// Equivalent LLVM IR: declare i32 @main()

// 1. Get the return type (32-bit integer)
auto *Int32Ty = b.getInt32Ty();

// 2. Define the function type: returns Int32, takes no arguments (false)
auto *FuncTy = llvm::FunctionType::get(Int32Ty, false);

// 3. Insert the main function into our module
auto *MainFunc = llvm::Function::Create(
        FuncTy,
        llvm::Function::ExternalLinkage, // Visible outside the module
        "main",
        *module
    );
```
## Implementing Main Logic

LLVM IR requires instructions to live inside an explicit Basic Block. A basic block is a straight-line sequence of execution containing a single entry point and a single exit point.
```c
// 1. Create the 'entry' basic block inside MainFunc
auto *EntryBB = llvm::BasicBlock::Create(*ctx, "entry", MainFunc);

// 2. Tell the IRBuilder to start inserting instructions here
b.SetInsertPoint(EntryBB);

// 3. Generate the return statement (Equivalent LLVM IR: ret i32 42)
auto *ReturnValue = b.getInt32(42); // Generate a constant 42
b.CreateRet(ReturnValue);           // Generate the return instruction       
```
If this were written in C, the code would look like this:
```c
int main() {

    return 42;
}
```
## Verifying and Printing the LLVM IR

Before outputting our generated structure, it is critical to run LLVM's internal verifier to guarantee that we haven't violated any IR structural rules.
```c
// Verify the module for consistency errors
if (llvm::verifyModule(*module, &llvm::errs())) {
    std::cerr << "LLVM Module verification failed!\n";
    return EXIT_FAILURE;
}

// Print the generated IR to stdout
std::cout << "--- Printed LLVM IR ---\n";
module->print(llvm::outs(), nullptr);

return EXIT_SUCCESS;
```

### Expected Output:

When you run the compiled C++ generator program, it will seamlessly output the valid LLVM assembly text representing the function:
```
--- Print LLVM IR ---
; ModuleID = 'simple_module'
source_filename = "simple_module"

define i32 @main() {
entry:
  ret i32 42
}
```

## Reference
[1] llvm::LLVMContext Class Reference https://llvm.org/doxygen/classllvm_1_1LLVMContext.html

[2] llvm::Module Class Reference https://llvm.org/doxygen/classllvm_1_1Module.html

[3] llvm::IRBuilder< FolderTy, InserterTy > Class Template Reference https://llvm.org/doxygen/classllvm_1_1IRBuilder.html#details

[4] Verifier.h https://llvm.org/doxygen/Verifier_8h_source.html

[5] llvm::raw_ostream Class Reference https://llvm.org/doxygen/classllvm_1_1raw__ostream.html