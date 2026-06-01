#include <cstdlib>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Alignment.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <memory>

// JIT
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <ostream>
#include <vector>

std::unique_ptr<llvm::orc::LLJIT> initJITAndAddModule( 
    std::unique_ptr<llvm::Module> module ,
    std::unique_ptr<llvm::LLVMContext> context ) {

    std::cout << "\n--- JIT processing ---\n";

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    // 1. buid LLJIT instance
    auto JITExpect = llvm::orc::LLJITBuilder().create();
    if (!JITExpect) {
        llvm::errs() << "No JIT Engine: " << JITExpect.takeError() << "\n";
        return nullptr;
    }
    auto JIT = std::move(*JITExpect);

    // 2. set the module in a pipline and hand it over to JIT
    auto TSM = llvm::orc::ThreadSafeModule(std::move(module), 
                            std::move(context) );
    
    // Note: now JIT takes full ownership of the module's lifecycle,
    // so no `delete module` at the end.
    if (auto Err = JIT->addIRModule(std::move(TSM))) {
        llvm::errs() << "Cannot join the module to JIT: " << std::move(Err) << "\n";
        return nullptr;
    }

    return  JIT ;
}
int executeJITFunctions ( 
    llvm::orc::LLJIT & JIT,
    const std::string& funcName ) {

    std::cout << "--- execute JIT function: " << funcName << "---"<< std::endl;    

    // 3. Look up the "main" function
    auto MainSymExpect = JIT.lookup("main");
    if (!MainSymExpect) {
        llvm::errs() << "Cannot find main function: " << MainSymExpect.takeError() << "\n";
        return -1;
    }
   
    // 4. Convert the symbol to a function pointer 
    // the original type :  int main( int a ) 
    int (*ResultMain)() = MainSymExpect->toPtr<int(*)()>();
    
    // call main() and get the return value, which should be 0
    int exitCode = ResultMain();
    
    std::cout << "JIT is done and return value is: " << exitCode << "\n";
    
    return exitCode ;
}

int main() {
    
    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("simple_module", *ctx);
    llvm::IRBuilder<> b(*ctx);

    // type
    auto *Int32Ty = b.getInt32Ty();

    // number
    auto * Ten = b.getInt32(10);
    auto * Tewnty = b.getInt32(20);
    auto * Zero = b.getInt32(0);
    auto * One = b.getInt32(1);

    // LLVM IR: define i32 @main( i32 %a ) 
    std::vector<llvm::Type*> mainArgs = {};
    auto *FuncTy = llvm::FunctionType::get(Int32Ty, mainArgs, false);

    // Add main into module
    auto *MainFunc = llvm::Function::Create(
        FuncTy,
        llvm::Function::ExternalLinkage,
        "main",
        *module
    );

    //Labels
    auto * L_Entry = llvm::BasicBlock::Create(*ctx, "L_Entry", MainFunc);
    auto * L_CMP = llvm::BasicBlock::Create(*ctx, "L_CMP", MainFunc ) ;
    auto * L_LOGICS = llvm::BasicBlock::Create(*ctx, "L_LOGICS", MainFunc ) ;
    auto * L_ret = llvm::BasicBlock::Create(*ctx, "L_ret", MainFunc);



    // L_Entry:
    b.SetInsertPoint(L_Entry);
        // br label %L_CMP
        b.CreateBr( L_CMP );

    b.SetInsertPoint( L_CMP);
        // %i_val   = phi i32 [ 0 , %L_Entry] [%new_i   , %L_IPP]
        auto * i_val = b.CreatePHI(Int32Ty, 2 ) ; 
        auto * sum_val = b.CreatePHI(Int32Ty, 2 ) ;
        auto * cmp = b.CreateICmpSLT( i_val, Tewnty);    
        b.CreateCondBr( cmp , L_LOGICS, L_ret ) ;

    b.SetInsertPoint(L_LOGICS);
        auto * new_sum = b.CreateAdd( sum_val, One, "new_sum", false , true ) ;
        auto * new_i = b.CreateAdd ( i_val , One , "new_i" , false , true ) ;
        b.CreateBr(L_CMP);

    b.SetInsertPoint(L_ret);
        b.CreateRet(sum_val);       

    i_val->addIncoming(Zero, L_Entry);
    i_val->addIncoming(new_i, L_LOGICS);
    sum_val->addIncoming(Ten, L_Entry);
    sum_val->addIncoming(new_sum, L_LOGICS);

    // --------------------------------------------------------------
    // 6. verify
    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "LLVM Module failed！\n";
        return EXIT_FAILURE ;
    }

    std::cout << "--- Print LLVM IR ---\n";
    module->print(llvm::outs(), nullptr);

    auto myJIT = initJITAndAddModule(std::move(module), std::move(ctx));
    if ( ! myJIT ) { return EXIT_FAILURE ; }
  
    int result1 = executeJITFunctions( * myJIT , "main" );

    return EXIT_SUCCESS ;
}
