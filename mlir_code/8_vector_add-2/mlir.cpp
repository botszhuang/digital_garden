#include "my_library.h"

template <typename myT> mlir::Value myAddOp( mlir::ImplicitLocOpBuilder * b, mlir::Value vA , mlir::Value vB , myT type ){

  if ( mlir::isa<mlir::FloatType>(type) ) {
    return b->create<mlir::arith::AddFOp>(vA, vB);
  } else if ( mlir::isa<mlir::IntegerType>(type) ){
    return b->create<mlir::arith::AddIOp>(vA, vB);
  } else {
    std::cout << "error: Unsupported type for myAddOp." << std::endl; 
    exit(1);
  }

}

void subf_addI ( mlir::ImplicitLocOpBuilder * b ,
              mlir::LLVM::LLVMPointerType llvmPtrType ,
              mlir::IntegerType elementType ,
              mlir::VectorType vectorType ,
              mlir::ValueRange indices ,
              mlir::Value arrayA ,
              mlir::Value arrayB ,
              mlir::Value arrayC  ,
              unsigned alignVal ){

  // operations
  auto ptrA = b->create<mlir::LLVM::GEPOp>( llvmPtrType, elementType, arrayA , indices );
  auto ptrB = b->create<mlir::LLVM::GEPOp>( llvmPtrType, elementType, arrayB , indices );
  auto ptrC = b->create<mlir::LLVM::GEPOp>( llvmPtrType, elementType, arrayC , indices );

  auto vA = b->create<mlir::LLVM::LoadOp>( vectorType , ptrA , alignVal );
  auto vB = b->create<mlir::LLVM::LoadOp>( vectorType , ptrB , alignVal);

  auto resV = myAddOp(b , vA, vB , elementType );

  b->create<mlir::LLVM::StoreOp>(resV,ptrC , alignVal );
 
}
void f_main(mlir::ModuleOp module , 
            mlir::ImplicitLocOpBuilder * b ,
            mlir::LLVM::LLVMPointerType llvmPtrType,
            mlir::IntegerType elementType,
            mlir::VectorType vectorType,
            mlir::Type gpeIndexType ,
            unsigned alignVal ){

  b->setInsertionPointToEnd(module.getBody());

  auto indexType = b->getIndexType();
  
  // func
  auto FuncType = b->getFunctionType({llvmPtrType, 
                                                            llvmPtrType, 
                                                            llvmPtrType, 
                                                            indexType, 
                                                            indexType                                          
                                                          }, {});
  auto FuncOp = b->create<mlir::func::FuncOp>( "f_main", FuncType ) ;

  auto EntryBlock=FuncOp.addEntryBlock();
  mlir::OpBuilder::InsertionGuard guard(*b);
  b->setInsertionPointToStart(EntryBlock);     
  
  // arguemnts
  int i = 0 ;
  auto arrayA = EntryBlock->getArgument(i++) ;
  auto arrayB = EntryBlock->getArgument(i++) ;
  auto arrayC = EntryBlock->getArgument(i++) ;
  auto lowerBound = EntryBlock->getArgument(i++);
  auto UpperBound = EntryBlock->getArgument(i++);
  auto step = b->create<mlir::arith::ConstantIndexOp>(vectorType.getShape()[0]);

  // operation
  auto loop = b->create<mlir::scf::ForOp>( lowerBound , UpperBound , step );
  b->setInsertionPointToStart(loop.getBody());
  {
    auto iv = loop.getInductionVar();
    auto i_idx = b->create<mlir::arith::IndexCastOp>( gpeIndexType , iv);
 
    subf_addI ( b , llvmPtrType , elementType , vectorType , mlir::ValueRange {i_idx},  arrayA , arrayB , arrayC , alignVal ) ; 
  }
  b->setInsertionPointAfter ( loop );
  
  b->create<mlir::func::ReturnOp>();
}
void f_rem ( mlir::ModuleOp module , 
             mlir::ImplicitLocOpBuilder * b ,
             mlir::LLVM::LLVMPointerType llvmPtrType,
             mlir::IntegerType elementType,
             mlir::VectorType vectorType ,
             mlir::Type gpeIndexType ,
             unsigned alignVal ){
 
  b->setInsertionPointToEnd(module.getBody());

  auto indexType = b->getIndexType();
  auto FuncType = b->getFunctionType({llvmPtrType, llvmPtrType, llvmPtrType, indexType, indexType}, {});
  auto FuncOp = b->create<mlir::func::FuncOp>( "f_rem", FuncType );
  auto EntryBlock = FuncOp.addEntryBlock();
  
  mlir::OpBuilder::InsertionGuard guard(*b);
  b->setInsertionPointToStart(EntryBlock);     
  
  int i = 0 ;
  auto arrayA = EntryBlock->getArgument(i++);
  auto arrayB = EntryBlock->getArgument(i++);
  auto arrayC = EntryBlock->getArgument(i++);
  auto offsetIdx = EntryBlock->getArgument(i++);
  auto totalSizeIdx = EntryBlock->getArgument(i++);

  auto remainCountIdx = b->create<mlir::arith::SubIOp>(totalSizeIdx, offsetIdx);
 
  auto offset_val = b->create<mlir::arith::IndexCastOp>(gpeIndexType, offsetIdx);
  auto remain_val = b->create<mlir::arith::IndexCastOp>(gpeIndexType, remainCountIdx);

  auto pA = b->create<mlir::LLVM::GEPOp>(llvmPtrType, elementType, arrayA, mlir::ValueRange{offset_val});
  auto pB = b->create<mlir::LLVM::GEPOp>(llvmPtrType, elementType, arrayB, mlir::ValueRange{offset_val});
  auto pC = b->create<mlir::LLVM::GEPOp>(llvmPtrType, elementType, arrayC, mlir::ValueRange{offset_val}); 

  const unsigned vecLength = vectorType.getShape()[0] ;
  auto maskType = mlir::VectorType::get({vecLength}, b->getI1Type());
  auto dynamicMask = b->create<mlir::vector::CreateMaskOp>(maskType, mlir::ValueRange{remain_val});
  auto passThrough = b->create<mlir::LLVM::ConstantOp>(vectorType, b->getZeroAttr(vectorType));

  auto maskedVA = b->create<mlir::LLVM::MaskedLoadOp>(
    vectorType,    
    pA,        
    dynamicMask,    
    passThrough,   
    alignVal       
  );

  auto maskedVB = b->create<mlir::LLVM::MaskedLoadOp>(
    vectorType,    
    pB,        
    dynamicMask,    
    passThrough,   
    alignVal       
  );
  
  auto resV = b->create<mlir::arith::AddIOp>(maskedVA, maskedVB);

  b->create<mlir::LLVM::MaskedStoreOp>(
    resV, 
    pC, 
    dynamicMask, 
    alignVal
  );

  b->create<mlir::func::ReturnOp>();
}
int main() {

  mlir::DialectRegistry dialect_registry;
  dialect_registry.insert<mlir::arith::ArithDialect>();
  dialect_registry.insert<mlir::func::FuncDialect>();
  dialect_registry.insert<mlir::scf::SCFDialect>();
  dialect_registry.insert<mlir::cf::ControlFlowDialect>();
  dialect_registry.insert<mlir::memref::MemRefDialect>();
  dialect_registry.insert<mlir::vector::VectorDialect>();
  dialect_registry.insert<mlir::LLVM::LLVMDialect>();

  mlir::registerBuiltinDialectTranslation(dialect_registry);
  mlir::registerLLVMDialectTranslation(dialect_registry);
  mlir::registerAllToLLVMIRTranslations(dialect_registry);

  // Setup Context and Module
  mlir::MLIRContext context(dialect_registry);
  context.loadAllAvailableDialects();

  auto loc = mlir::UnknownLoc::get(&context);
  auto module = mlir::ModuleOp::create(loc);

  // Create Types -----------------------------------------
  const unsigned bitWidth32 = 32;
  const unsigned vectorLength = 32;
  auto elementType = mlir::IntegerType::get(&context, bitWidth32);
  auto vectorType = mlir::VectorType::get({vectorLength}, elementType);
  auto llvmPtrType = mlir::LLVM::LLVMPointerType::get( &context ) ;

  // Create Operations ------------------------------------
  mlir::OpBuilder builder(&context);
  mlir::ImplicitLocOpBuilder b(loc, builder);

  auto dataLayout = mlir::DataLayout::closest(module);
  auto indexBitwidth = dataLayout.getTypeSizeInBits(b.getIndexType());
  auto gpeIndexType = b.getIntegerType(indexBitwidth);
  auto alignVal = dataLayout.getTypeABIAlignment(elementType);
  b.setInsertionPointToStart(module.getBody());

  f_main ( module , & b , llvmPtrType , elementType , vectorType , gpeIndexType , alignVal ) ;
  f_rem  ( module , & b , llvmPtrType , elementType , vectorType , gpeIndexType , alignVal ) ;

  // Create the Function: vector_add
  auto funcType = b.getFunctionType({llvmPtrType, elementType, llvmPtrType, llvmPtrType }, {});
  auto funcOp = b.create<mlir::func::FuncOp>("vector_add", funcType);

  // Create Labels:
  auto entryBlock = funcOp.addEntryBlock() ;

  // Labels for if
  auto thenBlock = b.createBlock(&funcOp.getRegion());
  auto endIfBlock = b.createBlock(&funcOp.getRegion());
  //--------------------------------------------------------

  b.setInsertionPointToStart(entryBlock );

  int i = 0 ;
  auto arrayA = entryBlock->getArgument(i++) ;
  auto A_Size = entryBlock->getArgument(i++) ;
  auto arrayB = entryBlock->getArgument(i++) ;
  auto arrayC = entryBlock->getArgument(i++) ;

  auto totalSizeIdx = b.create<mlir::arith::IndexCastOp>( b.getIndexType() , A_Size ) ;
  auto vecLengthIdx = b.create<mlir::arith::ConstantIndexOp>(vectorLength) ;
  
  auto remainCountIdx = b.create<mlir::arith::RemUIOp>( totalSizeIdx->getResult(0) , vecLengthIdx );
  auto mainUpperBoundIdx = b.create<mlir::arith::SubIOp>( totalSizeIdx , remainCountIdx );
  auto lowerBoundIdx = b.create<mlir::arith::ConstantIndexOp>(0);

  auto zeroIdx = b.create<mlir::arith::ConstantIndexOp>(0);
 
  llvm::SmallVector<mlir::Value, 5> f_main_operands = { arrayA, arrayB, arrayC, lowerBoundIdx, mainUpperBoundIdx };
  auto call_f_main = b.create<mlir::func::CallOp>("f_main", mlir::ValueRange{}, f_main_operands ) ;                                                                       

  // if ( remainCountIdx > 0) { f_rme(); }                                                       
  // Create the condition: rem > 0
  auto if_rem = b.create<mlir::arith::CmpIOp>(mlir::arith::CmpIPredicate::sgt, remainCountIdx, zeroIdx);
  b.create<mlir::cf::CondBranchOp>( if_rem , thenBlock, endIfBlock);
  b.setInsertionPointToStart(thenBlock);   // if condition is true,
  
    llvm::SmallVector<mlir::Value, 5> f_rem_operands = { arrayA, arrayB, arrayC, mainUpperBoundIdx, totalSizeIdx } ;
    auto call_f_rem = b.create<mlir::func::CallOp>("f_rem", mlir::ValueRange{}, f_rem_operands ) ;
  
  b.create<mlir::cf::BranchOp>(endIfBlock);
  // end of if
  b.setInsertionPointToStart(endIfBlock);

  b.create<mlir::func::ReturnOp>();

  verify_mlir_module(module);
  print_module(module, "MLIR Module:");
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

  // LLVM IR --> shared library
  llvm_ir_to_so("myFunc.ll") ;

  // Clean up and Exit
  // the MLIRContext will automatically clean up all MLIR objects when it goes
  // out of scope.

  return EXIT_SUCCESS;
}