#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include <memory>
#include <iostream>

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

    return EXIT_SUCCESS ;
}