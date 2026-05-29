#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <memory>

// JIT
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/TargetSelect.h>

int JITEXcute( std::unique_ptr<llvm::Module> module ,
               std::unique_ptr<llvm::LLVMContext> context ) {

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
    auto TSM = llvm::orc::ThreadSafeModule(std::move(module), 
                            std::move(context) );
    
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

/* LLVM IR
; ModuleID = 'simple_module'
source_filename = "simple_module"

define i32 @main() {
entry:
  ret i32 42
}
*/
int main() {
    
    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("simple_module", *ctx);
    llvm::IRBuilder<> b(*ctx);

    // LLVM IR: defin i32 main() 
    auto *Int32Ty = b.getInt32Ty();
    auto *FuncTy = llvm::FunctionType::get(Int32Ty, false);

    // 3. Add main into module
    auto *MainFunc = llvm::Function::Create(
        FuncTy,
        llvm::Function::ExternalLinkage,
        "main",
        *module
    );

    // LLVM IR: entry:
    auto *EntryBB = llvm::BasicBlock::Create(*ctx, "entry", MainFunc);
    b.SetInsertPoint(EntryBB);

    // LLVM IR: ret i32 42
    auto *ReturnValue = b.getInt32(42); // produce a constant, 42
    b.CreateRet(ReturnValue);           

    // --------------------------------------------------------------
    // 6. verify
    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "LLVM Module failed！\n";
        return EXIT_FAILURE ;
    }

    std::cout << "--- Print LLVM IR ---\n";
    module->print(llvm::outs(), nullptr);

    int ret =  JITEXcute( std::move(module) , std::move(ctx ) )  ;

    return ret ;
}