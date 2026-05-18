#include <cstdlib>

#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/Builders.h>
#include <mlir/IR/Verifier.h>

#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/LLVMIR/LLVMDialect.h>

#include <mlir/Target/LLVMIR/Export.h>
#include <mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h>
#include <mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h>

#include <mlir/Conversion/ArithToLLVM/ArithToLLVM.h>
#include <mlir/Conversion/FuncToLLVM/ConvertFuncToLLVMPass.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>

#include <mlir/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>


using namespace mlir;
using namespace mlir::arith;
using namespace mlir::func ;

int main() {
    
    DialectRegistry dialect_registry;

    dialect_registry.insert<arith::ArithDialect>();
    dialect_registry.insert<func::FuncDialect>();
    dialect_registry.insert<LLVM::LLVMDialect>();  
    
    registerBuiltinDialectTranslation(dialect_registry);
    registerLLVMDialectTranslation(dialect_registry);

    // Setup Context and Module
    MLIRContext context (dialect_registry);
    context.loadAllAvailableDialects();

    // 1-3. Initialize LLVM targets for the JIT
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    auto loc = UnknownLoc::get(&context);
    auto module = ModuleOp::create(loc);

    // Create Types
    auto i32Type = IntegerType::get(&context, 32) ;

    // Create Operations
    OpBuilder builder(&context);
    ImplicitLocOpBuilder b(loc, builder );

    b.setInsertionPointToEnd(module.getBody());

    // Create the Function: () -> i32
    auto funcType = b.getFunctionType({}, {i32Type});
    auto funcOp = b.create<FuncOp>("get_forty_two", funcType);
    
    //builder.insert(funcOp);

    // Create the Body Block
    auto entryBlock = funcOp.addEntryBlock();
    b.setInsertionPointToStart(entryBlock);

    // Create the Constant and Return
    auto fortyTwo = b.create<ConstantIntOp>(42, 32); // %c42_i32 = arith.constant 42 : i32
    b.create<ReturnOp>(fortyTwo.getResult()); // return %c42_i32 : i32

    // Verify the module
    if ( module.verify().failed() ) {
        llvm::errs() << "Module verification failed! " ;
        return EXIT_FAILURE ;
    }

    // Print the mlir Module
    printf ("Generated MLIR Module:\n");
    module->print(llvm::outs());
    puts("-----------------------------------");

    // Clean up and Exit 
    // the MLIRContext will automatically clean up all MLIR objects when it goes out of scope.
 
    // 2. Run the Pass Manager
    PassManager pass_manager(&context);
    pass_manager.addPass(createArithToLLVMConversionPass());
    pass_manager.addPass(createConvertFuncToLLVMPass());
    if (pass_manager.run(module).failed()) {
        llvm::errs() << "Pass manager failed to run! " ;
        return EXIT_FAILURE ;
    }

    // Print the llvm Module
    printf ("Generated LLVM Module:\n");
    module->print(llvm::outs());
    puts("-----------------------------------");

    // 3. Transforms MLIR to LLVM IR
    llvm ::LLVMContext llvmContext = llvm::LLVMContext();
    auto llmvModule = mlir::translateModuleToLLVMIR(module, llvmContext);
    if (!llmvModule) {
        llvm::errs() << "Failed to translate MLIR module to LLVM IR! " ;
        return EXIT_FAILURE ;
    }
    // Print the LLVM IR
    printf ("Generated LLVM IR:\n");
    llmvModule->print(llvm::outs(), nullptr);   
    puts("-----------------------------------");

    // 4.JIT Execution
    auto maybeEngine = ExecutionEngine::create(module);
    if ( !maybeEngine ){
        llvm::errs() << "Failed to create execution engine! " ;
        return EXIT_FAILURE ;
    }
    
    auto &engine = maybeEngine.get();
    auto funcPtr = engine->lookup("get_forty_two") ;
    if (!funcPtr) {
        llvm::errs() << "Failed to lookup function in execution engine! " ;
        return EXIT_FAILURE ;
    }
    
    // Call the function and print the result
    auto ans = reinterpret_cast <int(*)()>(*funcPtr) ;
    if (!ans) {
        llvm::errs() << "Failed to lookup function in execution engine! " ;
        return EXIT_FAILURE ;
    }
    llvm::outs() << "Execution result: " << ans() << "\n";

    return EXIT_SUCCESS;
}