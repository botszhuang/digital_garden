#include <cstdint>
#include <cstdlib>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
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
    auto MainSymExpect = JIT.lookup(funcName);
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

typedef struct {
    llvm::BasicBlock * Entry ;
    llvm::BasicBlock * CMP ;
    llvm::BasicBlock * RUN ;
    llvm::BasicBlock * END ;
} For_LOOP_LABEL;

int main() {
    
    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("simple_module", *ctx);
    llvm::IRBuilder<> b(*ctx);

    // LLVM IR: defin i64 main() 
    auto *IntTy = b.getInt32Ty();
    auto *FuncTy = llvm::FunctionType::get(IntTy, false);

    // 3. Add main into module
    auto *MainFunc = llvm::Function::Create(
        FuncTy,
        llvm::Function::ExternalLinkage,
        "main",
        *module
    );

    // Label
    For_LOOP_LABEL L ;
    L.Entry = llvm::BasicBlock::Create(*ctx, "L_Entry", MainFunc);
    L.CMP = llvm::BasicBlock::Create(*ctx, "L_CMP", MainFunc);
    L.RUN = llvm::BasicBlock::Create(*ctx, "L_FOR_RUN", MainFunc);
    L.END = llvm::BasicBlock::Create(*ctx, "L_FOR_END", MainFunc);
 
    uint64_t three_C_int = 3 ;
    auto * ArraySize = b.getInt32( three_C_int ) ;
    auto * One = b.getInt32(1);
    auto * Zero = b.getInt32(0);
    auto * Ten = b.getInt32(10);

    // ==================== 1. L_Entry ====================
    b.SetInsertPoint(L.Entry);
        auto * ArrayTy = llvm::ArrayType::get(IntTy, three_C_int ) ;
        // %arr = alloca [ 3 * i64 ], align 8
        auto * ArrayAlloca = b.CreateAlloca( ArrayTy, nullptr, "arr" );
        b.CreateBr(L.CMP);
    
    // ==================== 2. L_CMP ====================    
    b.SetInsertPoint(L.CMP);
        auto * i_val = b.CreatePHI( IntTy, 2);
        auto * cmp = b.CreateICmpSLT(i_val, ArraySize);
        b.CreateCondBr(cmp,L.RUN,L.END);
    
        i_val->addIncoming(Zero, L.Entry); 

    // ==================== 3. L_FOR_RUN ====================    
    b.SetInsertPoint(L.RUN);      
        // val = i_val * Ten
        auto * Val = b.CreateMul(i_val, Ten , "Val");
        // array [i]
        std::vector<llvm::Value*> indices = { Zero, i_val };
        auto * element_ptr = b.CreateInBoundsGEP( ArrayTy, ArrayAlloca , indices) ;   
        // store
        b.CreateStore(Val, element_ptr);

        // i = i +1
        auto * new_i = b.CreateAdd( i_val, One, "new_i", false ,true) ;
        i_val->addIncoming(new_i,L.RUN);
        b.CreateBr(L.CMP);    
    
    // ==================== 4. L_FOR_END ====================
    b.SetInsertPoint(L.END);

    std::vector<llvm::Value*> retIndices = { Zero, b.getInt32(1) };
    auto *retPtr = b.CreateInBoundsGEP(ArrayTy, ArrayAlloca, retIndices, "retPtr");
    auto * retVal = b.CreateLoad(IntTy,retPtr);    
    b.CreateRet(retVal);           

    // --------------------------------------------------------------
    // 6. verify
    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "LLVM Module failed！\n";
        return EXIT_FAILURE ;
    }

    std::cout << "--- Print LLVM IR ---\n";
    module->print(llvm::outs(), nullptr);

  auto JIT = initJITAndAddModule(std::move(module), std::move(ctx));
    if (!JIT) {
        return EXIT_FAILURE;
    }

    // 執行名為 "main" 的函式
    int finalResult = executeJITFunctions(*JIT, "main");

    return EXIT_SUCCESS ;
}