#include <iostream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

// JIT execution engine headers
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/TargetSelect.h>

int JITEXcute( std::unique_ptr<llvm::Module> module ) {

    std::cout << "\n--- JIT processing ---\n";

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    // 1. buid LLJIT instance
    auto JITExpect = llvm::orc::LLJITBuilder().create();
    if (!JITExpect) {
        llvm::errs() << "No JIT Engine: " << JITExpect.takeError() << "\n";
        return 1;
    }
    auto JIT = std::move(*JITExpect);

    // 2. set the module in a pipline and hand it over to JIT
    auto TSM = llvm::orc::ThreadSafeModule(std::move(module), std::make_unique<llvm::LLVMContext>());
    
    // Note: now JIT takes full ownership of the module's lifecycle,
    // so no `delete module` at the end.
    if (auto Err = JIT->addIRModule(std::move(TSM))) {
        llvm::errs() << "Cannot add module to JIT: " << std::move(Err) << "\n";
        return 1;
    }

    // 3. Look up the "main" function
    auto MainSymExpect = JIT->lookup("main");
    if (!MainSymExpect) {
        llvm::errs() << "Cannot find main function: " << MainSymExpect.takeError() << "\n";
        return 1;
    }
    
    // 4. Convert the symbol to a function pointer 
    // the original type :  int main() --> int(*)()
    int (*ResultMain)() = MainSymExpect->toPtr<int(*)()>();
    
    // call main() and get the return value, which should be 0
    // puts("Hello, World!")
    int exitCode = ResultMain(); 
    
    std::cout << "\nJIT is done and return value is: " << exitCode << "\n";
    
    return 0 ;
}

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
    std::cout << "\n--- Generated LLVM IR ---\n";
    module->print(llvm::outs(), nullptr);

    int ret =  JITEXcute( std::unique_ptr<llvm::Module>(module) )  ;

    //delete module;

    return ret ;
}