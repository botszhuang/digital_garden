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

/* LLVM IR
; ModuleID = 'simple_module'
source_filename = "simple_module"

define i32 @main (){
    L_Entry:
        %sum = alloca i32, align 4
        %i   = alloca i32, align 4
        store i32 10, i32* %sum, align 4
        store i32 0,  i32* %i,   align 4
        br label %L_CMP

    L_CMP:
        %ivalCMP = load i32, i32* %i, align 4
        %cmp = icmp slt i32 %ivalCMP, 20 
        br i1 %cmp, label %L_LOGICS , label %L_ret

    L_LOGICS:
        %old_sum = load i32, i32* %sum, align 4
        %new_sum = add nsw i32 %old_sum, 1
        store i32 %new_sum, i32* %sum, align 4
        br label %L_IPP
        
    L_IPP:
        %old_i = load i32, i32* %i, align 4
        %new_i = add nsw i32 %old_i, 1 
        store i32 %new_i, i32* %i, align 4
        br label %L_CMP

    L_ret:
        %sum_val = load i32, i32* %sum, align 4 
        ret i32 %sum_val 
}    

*/
int main() {
    
    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("simple_module", *ctx);
    llvm::IRBuilder<> b(*ctx);

    // type
    auto *Int32Ty = b.getInt32Ty();
    auto align_4 = llvm::Align(4);

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
    auto * L_IPP = llvm::BasicBlock::Create(*ctx, "L_IPP", MainFunc ) ;
    auto * L_ret = llvm::BasicBlock::Create(*ctx, "L_ret", MainFunc);

    // L_Entry:
    b.SetInsertPoint(L_Entry);

        // %sum = alloca i32, align 4
        auto * sumPtr = b.CreateAlloca( Int32Ty, nullptr , "sumPtr" ) ; 
        sumPtr->setAlignment( align_4 );
        
        // %i = alloca i32, align 4
        auto * iPtr = b.CreateAlloca( Int32Ty, nullptr , "iPtr" ) ; 
        iPtr->setAlignment( align_4 );

        // store i32 10, i32* %sum, align 4
        auto * storeSumPtr = b.CreateStore(Ten, sumPtr);
        storeSumPtr->setAlignment(align_4);

        // store i32 0,  i32* %i,   align 4
        auto * storeIPtr = b.CreateStore(Zero, iPtr);
        storeIPtr->setAlignment(align_4);

        // br label %L_CMP
        b.CreateBr(L_CMP);


    b.SetInsertPoint( L_CMP);
        //%ivalCMP = load i32, i32* %i, align 4
        auto * ivalCMP = b.CreateLoad(Int32Ty, iPtr);
        ivalCMP->setAlignment(align_4);
        
        //%cmp = icmp slt i32 %ivalCMP, 20
        auto * cmpVal = b.CreateICmp(llvm::CmpInst::ICMP_SLT, ivalCMP, Tewnty ) ;
        
        //br i1 %cmp, label %L_LOGICS , label %L_ret
        b.CreateCondBr( cmpVal , L_LOGICS , L_ret ) ;


    b.SetInsertPoint(L_LOGICS);

        // %old_sum = load i32, i32* %sum, align 4
        auto * old_sumVal = b.CreateLoad(Int32Ty,sumPtr);
        
        // %new_sum = add nsw i32 %old_sum, 1
        auto * new_sumVal = b.CreateAdd(old_sumVal, One, "new_sum" ,false, true);
        
        // store i32 %new_sum, i32* %sum, align 4
        auto * storeNew_SumVal = b.CreateStore( new_sumVal, sumPtr);
        storeNew_SumVal->setAlignment(align_4);

        // br label %L_IPP
        b.CreateBr(L_IPP);


    b.SetInsertPoint(L_IPP) ;
        // %old_i = load i32, i32* %i, align 4
        auto * old_iVal = b.CreateLoad(Int32Ty, iPtr);
        old_iVal->setAlignment(align_4);
        
        // %new_i = add nsw i32 %old_i, 1 
        auto * new_iVal = b.CreateAdd( old_iVal, One, "new_i", false, true ); 
        
        // store i32 %new_i, i32* %i, align 4
        auto * storeNew_i = b.CreateStore(new_iVal, iPtr);
        storeNew_i->setAlignment(align_4);

        // br label %L_CMP
        b.CreateBr( L_CMP ) ;

    b.SetInsertPoint(L_ret);

        // %sum_val = load i32, i32* %sum, align 4 
        auto * retVal = b.CreateLoad(Int32Ty,sumPtr);    
        // ret i32 %sum_val 
        b.CreateRet(retVal);           

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