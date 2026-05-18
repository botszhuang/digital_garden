#include <cstdlib>

#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/Builders.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/Verifier.h>
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>

using namespace mlir;
using namespace mlir::arith;
using namespace mlir::func;

int main() {
    
    // 1.Registration of Dialects
    DialectRegistry dialect_registry;

    dialect_registry.insert<ArithDialect>();
    dialect_registry.insert<FuncDialect>();

    // 2. setup MLIRContext & ModuleOp
    MLIRContext context (dialect_registry);
    context.loadAllAvailableDialects();

    auto loc = UnknownLoc::get(&context);
    auto module = ModuleOp::create(loc);

    // 3. Create Types
    auto i32Type = IntegerType::get(&context, 32) ;

    // 4. Create OpBuilders
    OpBuilder builder(&context);
    ImplicitLocOpBuilder b(loc, builder );

    b.setInsertionPointToEnd(module.getBody());

    // 5-1. Create Function Operator  
    // Create the Function: () -> i32
    auto funcType = b.getFunctionType({}, {i32Type});
    auto funcOp = b.create<FuncOp>("get_forty_two", funcType);
    
    builder.insert(funcOp);

    // 5-2. Create the Body Block
    auto entryBlock = funcOp.addEntryBlock();
    b.setInsertionPointToStart(entryBlock);

    // Create the Constant and Return
    auto fortyTwo = b.create<ConstantIntOp>(42, 32); // %c42_i32 = arith.constant 42 : i32
    b.create<ReturnOp>(fortyTwo.getResult()); // return %c42_i32 : i32

    // 6-1.Verify the module
    if ( module.verify().failed() ) {
        llvm::errs() << "Module verification failed! " ;
        return EXIT_FAILURE ;
    }

    // 6-2.Print the MLIR Module
    printf ("Generated MLIR Module:\n");
    module->print(llvm::outs());
    puts("-----------------------------------");

    // 7. Clean up and Exit 
    // the MLIRContext will automatically clean up all MLIR objects when it goes out of scope.
    

    return EXIT_SUCCESS;
}
