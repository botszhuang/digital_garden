#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "my_library.h"

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
  const unsigned vectorLength = 8;
  auto i32Type = mlir::IntegerType::get(&context, bitWidth32);
  auto memRefI32Type = mlir::MemRefType::get({mlir::ShapedType::kDynamic}, i32Type);
  auto vectorType = mlir::VectorType::get({vectorLength}, i32Type);

  // Create Operations ------------------------------------
  mlir::OpBuilder builder(&context);
  mlir::ImplicitLocOpBuilder b(loc, builder);
  b.setInsertionPointToStart(module.getBody());

  // Create the Function: vector_add
  auto funcType = b.getFunctionType({memRefI32Type, memRefI32Type, memRefI32Type}, {});
  auto funcOp = b.create<mlir::func::FuncOp>("vector_add", funcType);
  funcOp->setAttr("llvm.emit_c_interface", b.getUnitAttr());
  
  auto entryBlock = funcOp.addEntryBlock();
  b.setInsertionPointToStart(entryBlock);

  mlir::Value arrayA = entryBlock->getArgument(0);
  mlir::Value arrayB = entryBlock->getArgument(1);
  mlir::Value arrayC = entryBlock->getArgument(2);

  auto lowerBound = b.create<mlir::arith::ConstantIndexOp>(0) ;
  auto upperBound = b.create<mlir::memref::DimOp>(arrayA, lowerBound) ;
  auto step = b.create<mlir::arith::ConstantIndexOp>(vectorLength) ;

  auto loop = b.create<mlir::scf::ForOp>( lowerBound , upperBound , step ) ;

  auto paddingValueType = memRefI32Type.getElementType();
  auto paddingValue = b.create<mlir::arith::ConstantOp>( b.getZeroAttr( paddingValueType ));

  b.setInsertionPointToStart( loop.getBody() );
  {
    auto iv = loop.getInductionVar() ;

    // Vector: Represents data in hardware SIMD registers.
    // If the target hardware supports vector instructions (e.g., AVX, NEON), 
    // this allows data-level parallelism, significantly speeding up computation 
    // by processing multiple elements (vectorLength) in a single instruction.

    auto aVec = b.create<mlir::vector::TransferReadOp>( vectorType , arrayA , iv , paddingValue ) ;
    auto bVec = b.create<mlir::vector::TransferReadOp>( vectorType , arrayB , iv , paddingValue ) ;
    auto cVec = b.create<mlir::arith::AddIOp>( aVec , bVec ) ;
    b.create<mlir::vector::TransferWriteOp>( cVec , arrayC , iv) ; 
  }

  b.setInsertionPointAfter ( loop );
  b.create<mlir::func::ReturnOp>();

  // -----------------------------------------------------
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