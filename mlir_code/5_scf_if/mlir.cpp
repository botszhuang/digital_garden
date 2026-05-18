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
    auto i1Type  = mlir::IntegerType::get(&context, 1) ;

    // Create Operations ------------------------------------
    mlir::OpBuilder builder(&context);
    mlir::ImplicitLocOpBuilder b(loc, builder );

    b.setInsertionPointToEnd(module.getBody());

    // Create the Function: control_flow_demo ( i32 ) -> i1
    // i32: input argument type
    // i1: return type
    auto funcType = b.getFunctionType({i32Type}, {i1Type});
    auto funcOp = b.create<mlir::func::FuncOp>("control_flow_demo", funcType);
    
    // Create the Body Block
    auto entryBlock = funcOp.addEntryBlock();
    b.setInsertionPointToStart(entryBlock);

    // Get the input argument0
    mlir::Value inputArg0 = entryBlock->getArgument(0);

    // Create the Constants
    auto fortyTwo = b.create<mlir::arith::ConstantIntOp>(42, bitWidth32);    
    auto yes  = b.create<mlir::arith::ConstantOp>(b.getBoolAttr(true));  
    auto no   = b.create<mlir::arith::ConstantOp>(b.getBoolAttr(false)); 

    // Create the condition: inputArg0 > fortyTwo(42)
    auto condition = b.create<mlir::arith::CmpIOp>(mlir::arith::CmpIPredicate::sgt, inputArg0 , fortyTwo ); 
    auto ifOp = b.create<mlir::scf::IfOp>( i1Type, condition, true );
    
    // if condition is true, yield yes
    b.setInsertionPointToStart(ifOp.thenBlock());
    b.create<mlir::scf::YieldOp>(yes.getResult()); 
    
    // if condition is false, yield no
    b.setInsertionPointToStart(ifOp.elseBlock());
    b.create<mlir::scf::YieldOp>(no.getResult()); 
    
    // save the result of the ifOp to a variable
    b.setInsertionPointAfter(ifOp);
    auto ret_value = ifOp.getResult(0) ; 

    // return the result of function:get_forty_two
    b.create<mlir::func::ReturnOp>(ret_value); 

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