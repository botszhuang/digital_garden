#include "my_library.h"

void verify_mlir_module ( mlir::ModuleOp module ) {

    if ( module.verify().failed() ) {
        llvm::errs() << "Module verification failed! " ;
        exit(EXIT_FAILURE) ;
    }

}

void print_module ( mlir::ModuleOp module , std::string Tag ) {
    puts("-----------------------------------");
    std::cout << Tag << std::endl ;
    module->print(llvm::outs());
    puts("-----------------------------------");
}

void save_module_to_file( mlir::ModuleOp module, std::string filename ) {

    std::error_code ec;
    
    llvm::raw_fd_ostream mlirfile(filename, ec, llvm::sys::fs::OF_None);

    if (ec) {
        llvm::errs() << "Failed to open file for writing! " << ec.message() << "\n";
        exit(EXIT_FAILURE);
    }

    module.print(mlirfile);    
    mlirfile.close() ;
    std::cout << "MLIR module emitted to file: " << filename << std::endl ;

}

void translate_mlir_module_to_llvm ( mlir::MLIRContext * contextPtr, mlir::ModuleOp module ) {

    mlir::PassManager pass_manager( contextPtr );
    // Convert SCF to ControlFlow (creates cf.cond_br)
    pass_manager.addPass( mlir::createSCFToControlFlowPass());
    
    // Convert ControlFlow to LLVM (creates llvm.cond_br) - MANDATORY
    pass_manager.addPass( mlir::createConvertControlFlowToLLVMPass());

    pass_manager.addPass( mlir::createArithToLLVMConversionPass());
    pass_manager.addPass( mlir::createConvertFuncToLLVMPass());

    pass_manager.addPass(mlir::createReconcileUnrealizedCastsPass());


    if (pass_manager.run(module).failed()) {
        llvm::errs() << "Pass manager failed to run! " ;
        exit(EXIT_FAILURE) ;
    }

}

std::unique_ptr<llvm::Module> translate_llvm_module_to_llvm_ir ( mlir::ModuleOp module , llvm::LLVMContext * llvmContext ) {

    //llvm ::LLVMContext llvmContext = llvm::LLVMContext();
    auto llmvModule = mlir::translateModuleToLLVMIR( module, *llvmContext );
    if (!llmvModule) {
        llvm::errs() << "Failed to translate MLIR module to LLVM IR! " ;
        exit(EXIT_FAILURE) ;
    }

    return llmvModule ;

}

void print_llvm_ir( llvm::Module * llvmModule, std::string Tag ) {
    puts("-----------------------------------");
    std::cout << Tag << std::endl ;
    llvmModule->print(llvm::outs(), nullptr);
    puts("-----------------------------------");
}

void save_llvm_ir_to_file( llvm::Module * llvmModule, std::string filename ) {

    std::error_code ec;
   
    llvm::raw_fd_ostream llvmfile(filename, ec, llvm::sys::fs::OF_None);

    if (ec) {
        llvm::errs() << "Failed to open file for writing! " << ec.message() << "\n";
        exit(EXIT_FAILURE);
    }

    llvmModule->print(llvmfile, nullptr);    
    llvmfile.close() ;
    std::cout << "LLVM IR emitted to file: " << filename << std::endl ;
}
void llvm_ir_to_so( std::string llFile ) {
    
    // 2. AUTOMATIC COMPILATION: 
    // Define filenames
    std::string objFile = llFile.substr(0, llFile.find_last_of('.')) + ".o";
    std::string soFile = "lib" + objFile.substr(0, objFile.find_last_of('.')) + ".so";

    // 1. Automatically translate LLVM IR to Object file
    std::string llcCmd = "llc -filetype=obj --relocation-model=pic " + llFile + " -o " + objFile;
    std::system(llcCmd.c_str());

    // 2. Automatically link Object file to Shared Object
    std::string linkCmd = "clang -shared -fPIC " + objFile + " -o " + soFile;
    std::system(linkCmd.c_str());

    std::cout << "Computer successfully generated " << soFile << std::endl;
}
