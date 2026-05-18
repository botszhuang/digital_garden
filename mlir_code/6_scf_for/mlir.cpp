#include "my_library.h"

int main() {
    
    mlir::DialectRegistry dialect_registry;
    dialect_registry.insert<mlir::arith::ArithDialect>();
    dialect_registry.insert<mlir::func::FuncDialect>();
    dialect_registry.insert<mlir::scf::SCFDialect>();
    dialect_registry.insert<mlir::LLVM::LLVMDialect>();  

    mlir::registerBuiltinDialectTranslation(dialect_registry);
    mlir::registerLLVMDialectTranslation(dialect_registry);
    mlir::registerAllToLLVMIRTranslations(dialect_registry);    

    // Setup Context and Module
    mlir::MLIRContext context (dialect_registry);
    context.loadAllAvailableDialects();
  
    auto loc = mlir::UnknownLoc::get(&context);
    auto module = mlir::ModuleOp::create(loc);

    // Create Types -----------------------------------------
    const unsigned bitWidth32 = 32 ;
    auto i32Type = mlir::IntegerType::get(&context, bitWidth32) ;

    // Create Operations ------------------------------------
    mlir::OpBuilder builder(&context);
    mlir::ImplicitLocOpBuilder b(loc, builder );
    b.setInsertionPointToStart(module.getBody());

    // Create the Function: control_flow_for ( i32 ) -> i32
    auto funcType = b.getFunctionType({i32Type}, {i32Type});
    auto funcOp = b.create<mlir::func::FuncOp>("control_flow_for", funcType);   
    auto entryBlock = funcOp.addEntryBlock();
    b.setInsertionPointToStart(entryBlock);

    // 'forLowBound', 'forUpBound', and 'forStep' define the range.
    // 'sum_init' is the initial value for the loop-carried variable.
    auto forLowBound = b.create<mlir::arith::ConstantIndexOp>(0);
    auto forUpBound  = b.create<mlir::arith::IndexCastOp>( b.getIndexType(),entryBlock->getArgument(0));
    auto forStep     = b.create<mlir::arith::ConstantIndexOp>(1);
    auto sum_init    = b.create<mlir::arith::ConstantIntOp>( i32Type, 0);

    // for loop: for (i = 0; i < inputArg0; i++) { ... }  
    auto loop = b.create<mlir::scf::ForOp>( 
        forLowBound , // Lower bound
        forUpBound ,  // Upper bound
        forStep ,     // Step
        mlir::ValueRange{ sum_init } //initial value summation
    ) ;
    b.setInsertionPointToStart( loop.getBody() );
 
    // get value from mlir::ValueRange
    // or get value from yield of the previous iteration
    auto currentSum = loop.getRegionIterArg(0);  
    
    // ivI32 = (i32) index
    auto ivI32 = b.create<mlir::arith::IndexCastOp>(i32Type, loop.getInductionVar() );
    
    // nextSum = currentSum + ivI32
    auto nextSum = b.create<mlir::arith::AddIOp>( ivI32, currentSum );     
    
    // for : yield
    b.create<mlir::scf::YieldOp>(nextSum.getResult());
    
    // for : end
    b.setInsertionPointAfter(loop);    
    mlir::Value finalSum = loop.getResult(0); // get value from yield of the last iteration

    b.create<mlir::func::ReturnOp>(finalSum); 

    // -----------------------------------------------------
    verify_mlir_module(module);
    print_module(module, "MLIR Module:") ;
    save_module_to_file(module, "myFunc.mlir" ) ;

    // MLIR module --> LLVM module
    translate_mlir_module_to_llvm( &context , module) ;
    print_module( module, "LLVM Module:" ) ;
    save_module_to_file ( module, "myFunc_module.tex" ) ;
  
    // LLVM module --> LLVM IR
    llvm::LLVMContext llvmContext  ;
    auto llvmModule = translate_llvm_module_to_llvm_ir( module , &llvmContext ) ;
    print_llvm_ir( llvmModule.get() , "LLVM IR:" ) ;
    save_llvm_ir_to_file( llvmModule.get(), "myFunc.ll" ) ;

    llvm_ir_to_so("myFunc.ll") ;

    // Clean up and Exit 
    // the MLIRContext will automatically clean up all MLIR objects when it goes out of scope.

    return EXIT_SUCCESS;
}