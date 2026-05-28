---
title: "LLVM - hello.cpp"
auther: "Botsz"
date: 2026-05-28
---
This note demonstrates how to implement hello.md by transitioning from hand-crafted LLVM IR to C++ API-generated code.
```c
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
```
- `llvm/IR/LLVMContext.h` provids the definition for the `llvm::LLVMContext` class, which serves as the top-level container for managing global state and core data within the LLVM infrastructure[1].
- `llvm::Module.h` class is the top-level container for all other LLVM Intermediate Representation (IR) objects. It holds the globals variables, functions, libraries, a symbol table, and various information about the target's characteristics[2].
- `llvm/IR/IRBuilder.h` provides a uniform API for creating instructions and inserting them into a basic block: either at the end of a BasicBlock, or at a specific iterator location in a block[3].
- `llvm/IR/Function.h` 
- `llvm/IR/Verifier.h` provides a verifier interface to valify the input to the system and the transformations[5].
- `llvm/Support/raw_ostream` implements an extremely fast bulk output stream to the system and **only output only**[6].

# hello.cpp
```c
int main() {

    llvm::LLVMContext ctx;
    llvm::Module * module = new llvm::Module("hello_module", ctx);
    llvm::IRBuilder<> b(ctx);

    llvm::Type * int32Type = b.getInt32Ty();

    // Create the main function: int main()
    llvm::FunctionType * mainFuncType = llvm::FunctionType::get(int32Type, false);
    llvm::Function * mainFunc = llvm::Function::Create(mainFuncType, llvm::Function::ExternalLinkage, "main", module);

    // Create a basic block and set the insertion point
    llvm::BasicBlock * mainBlock = llvm::BasicBlock::Create(ctx, "mainEntry", mainFunc);
    b.SetInsertPoint(mainBlock);

    // Define a global constant string for "Hello, World!"
    // LLVM IR: @.str = private unnamed_addr constant [14 x i8] c"Hello, World!\00"
    // Create a string constant for "Hello, World!"
    // LLVM IR: %cast2str = getelementptr [14 x i8], [14 x i8]* @.str, i64 0, i64 0
    llvm::Value * helloStr = b.CreateGlobalString("Hello, World!");

    // puts(helloStr);
    // LLVM IR:  call i32 @puts(i8* %cast2str)
    llvm::FunctionType* putsType = llvm::FunctionType::get(int32Type, true);
    llvm::Function* putsFunc = llvm::Function::Create(putsType, llvm::Function::ExternalLinkage, "puts", module);
    b.CreateCall(putsFunc, {helloStr});

    // Return 0 from main
    // LLVM IR: ret i32 0
    b.CreateRet(b.getInt32(0));

    // Verify the module
    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "Error constructing function!\n";
        return 1;
    }

    // Print the generated LLVM IR
    module->print(llvm::outs(), nullptr);

    delete module;

    return 0;
}
```
## The C Equivalence
If this were written in C, the code would look like this:
```c
const char .str[14] = "Hello, World!\0"; // The global array

int main() {
    // This is what the GEP instruction does:
    const char* mystr = &.str[0]; 
    
    puts(mystr);
    return 0;
}
```

The complete example is available in [here](./2_hello_cpp/)

## Reference
[1] llvm::LLVMContext Class Reference https://llvm.org/doxygen/classllvm_1_1LLVMContext.html

[2] llvm::Module Class Reference https://llvm.org/doxygen/classllvm_1_1Module.html

[3] llvm::IRBuilder< FolderTy, InserterTy > Class Template Reference https://llvm.org/doxygen/classllvm_1_1IRBuilder.html#details

[4]

[5] Verifier.h https://llvm.org/doxygen/Verifier_8h_source.html

[6] llvm::raw_ostream Class Reference https://llvm.org/doxygen/classllvm_1_1raw__ostream.html