#include "my_library.h"

int main() {
    
    mlir::DialectRegistry dialect_registry;
    dialect_registry.insert<mlir::arith::ArithDialect>();
    dialect_registry.insert<mlir::func::FuncDialect>();
    dialect_registry.insert<mlir::scf::SCFDialect>();
    dialect_registry.insert<mlir::cf::ControlFlowDialect>();
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

    auto forLowBound = b.create<mlir::arith::ConstantIntOp>( i32Type, 0 );
    auto forUpBound = entryBlock->getArgument(0);
    auto forStep     = b.create<mlir::arith::ConstantIntOp>( i32Type, 1 );
    auto sum_init    = b.create<mlir::arith::ConstantIntOp>( i32Type, 0 );

    llvm::SmallVector<mlir::Location, 2> argLocs(2, loc);
    llvm::SmallVector<mlir::Location, 1> exitLocs(1, loc);
    auto loopHeader = b.createBlock(&funcOp.getRegion(), funcOp.getRegion().end(), {i32Type, i32Type} , argLocs );
    auto loopExit   = b.createBlock(&funcOp.getRegion(), funcOp.getRegion().end(), {i32Type} , exitLocs );
    auto loopBody   = b.createBlock(&funcOp.getRegion(), loopExit->getIterator(), {i32Type, i32Type} , argLocs );   
    b.setInsertionPointToEnd(entryBlock);
    
    b.create<mlir::cf::BranchOp>(loopHeader, mlir::ValueRange{forLowBound, sum_init});

    b.setInsertionPointToStart( loopHeader );
    
    auto current_iv = loopHeader->getArgument(0);
    auto currentSum = loopHeader->getArgument(1);
    
    auto cond = b.create<mlir::arith::CmpIOp>(mlir::arith::CmpIPredicate::slt, current_iv, forUpBound);
    b.create<mlir::cf::CondBranchOp>( cond , 
        loopBody, mlir::ValueRange{ current_iv , currentSum } ,
        loopExit, mlir::ValueRange{ currentSum }
    );
    b.setInsertionPointToStart(loopBody);
    
    auto bodyIv = loopBody->getArgument(0);
    auto bodySum = loopBody->getArgument(1);
    auto nextSum = b.create<mlir::arith::AddIOp>( bodyIv, bodySum );     
    auto next_iv = b.create<mlir::arith::AddIOp>( bodyIv, forStep );

    b.create<mlir::cf::BranchOp>( loopHeader, mlir::ValueRange{next_iv, nextSum});

    b.setInsertionPointToStart ( loopExit ) ;
    mlir::Value finalSum = loopExit->getArgument(0); 
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