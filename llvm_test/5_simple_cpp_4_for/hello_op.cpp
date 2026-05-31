#include <cstdlib>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
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
    const std::string& funcName, 
    int argument ) {

    std::cout << "--- execute JIT function: " << funcName << "---"<< std::endl;    

    // 3. Look up the "main" function
    auto MainSymExpect = JIT.lookup("main");
    if (!MainSymExpect) {
        llvm::errs() << "Cannot find main function: " << MainSymExpect.takeError() << "\n";
        return -1;
    }
   
    // 4. Convert the symbol to a function pointer 
    // the original type :  int main( int a ) 
    int (*ResultMain)(int) = MainSymExpect->toPtr<int(*)(int)>();
    
    // call main() and get the return value, which should be 0
    int exitCode = ResultMain(argument);

    std::cout << "argument: " << argument << std::endl ;
    
    std::cout << "JIT is done and return value is: " << exitCode << "\n";
    
    return exitCode ;
}

/* LLVM IR
; ModuleID = 'simple_module'
source_filename = "simple_module"

define i32 @main ( i32 %a ){
L_entry:

    ; Compare %a to 10
    %cmp = icmp sgt i32 %a , 42

    ; Branchless selection: If %cmp is true, pick 0. If false, pick 1.
    %retval = select i1 %cmp, i32 0, i32 1

    ret i32 %retval
}

*/
int main() {
    
    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("simple_module", *ctx);
    llvm::IRBuilder<> b(*ctx);

    // type
    auto *Int32Ty = b.getInt32Ty();
        
    // LLVM IR: define i32 @main( i32 %a ) 
    std::vector<llvm::Type*> mainArgs = {Int32Ty};
    auto *FuncTy = llvm::FunctionType::get(Int32Ty, mainArgs, false);

    // Add main into module
    auto *MainFunc = llvm::Function::Create(
        FuncTy,
        llvm::Function::ExternalLinkage,
        "main",
        *module
    );

    // variables
    auto * a = MainFunc->getArg(0); a->setName("a") ;
    auto * Ten = b.getInt32(10);
    auto * Zero = b.getInt32(0);
    auto * One = b.getInt32(1);

    //Labels
    auto * L_entry = llvm::BasicBlock::Create(*ctx, "L_entry", MainFunc);
  
    b.SetInsertPoint(L_entry);

    // LLVM IR: %cmp = icmp sgt i32 %a , 10
    auto cmp = b.CreateICmpSGT( a , Ten , "cmp" ) ;

    // LLVM IR: %retval = select i1 %cmp, i32 0, i32 1
    auto * RetVal = b.CreateSelect(cmp,Zero,One);
   
    // LLVM IR: ret i32 42
    b.CreateRet(RetVal);           

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
  
    int result1 = executeJITFunctions( * myJIT , "main", 1 );
    int result2 = executeJITFunctions( * myJIT , "main", 15 );

    return EXIT_SUCCESS ;
}