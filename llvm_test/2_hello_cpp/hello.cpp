#include <iostream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

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

    // Create a string constant for "Hello, World!"
    llvm::Value * helloStr = b.CreateGlobalString("Hello, World!");

    // puts(helloStr);
    llvm::FunctionType* putsType = llvm::FunctionType::get(int32Type, true);
    llvm::Function* putsFunc = llvm::Function::Create(putsType, llvm::Function::ExternalLinkage, "puts", module);
    b.CreateCall(putsFunc, {helloStr});

    // Return 0 from main
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