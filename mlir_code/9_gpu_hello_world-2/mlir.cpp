#include <cstdlib>

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallVector.h> // Added missing header
#include <llvm/Support/TargetSelect.h>

// ── MLIR Core ──────────────────────────────────────────────────────────────
#include <mlir/IR/Diagnostics.h>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/Builders.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/BuiltinDialect.h>
#include <mlir/IR/ValueRange.h>
#include <mlir/IR/Verifier.h>

// ── MLIR Dialects ──────────────────────────────────────────────────────────
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Conversion/GPUCommon/GPUCommonPass.h"
#include <mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h>

 
void registerDialects( mlir::MLIRContext &context) {

    mlir::DialectRegistry registry ;// = context.getDialectRegistry();

    registry.insert<mlir::BuiltinDialect>();
    registry.insert<mlir::arith::ArithDialect>();
    registry.insert<mlir::func::FuncDialect>();
    registry.insert<mlir::gpu::GPUDialect>();
  
    mlir::registerBuiltinDialectTranslation(registry);
  
    context.appendDialectRegistry(registry);
    context.loadAllAvailableDialects();

}

mlir::gpu::GPUFuncOp gpu_Hello_kernel( mlir::ImplicitLocOpBuilder &b , mlir::gpu::GPUModuleOp &gpuModuleOp ) {
    
    mlir::OpBuilder::InsertionGuard guard(b);
    b.setInsertionPointToStart(gpuModuleOp.getBody());

    auto helloFuncType = b.getFunctionType({}, {});

    //gpu.func @hello() kernel 
    auto helloFuncOp = b.create<mlir::gpu::GPUFuncOp>("hello", helloFuncType);
    helloFuncOp->setAttr("gpu.kernel", b.getUnitAttr() ) ;

    //auto &helloBlock = helloFuncOp.getBody().emplaceBlock();

    // Safely fetch the existing entry block or create it if missing (cross-version safe)
    auto * helloBlock = helloFuncOp.getBody().empty()
                      ? helloFuncOp.addEntryBlock()
                      : &helloFuncOp.getBody().front() ;
    b.setInsertionPointToStart(helloBlock);

    //%tid = gpu.thread_id x
    auto threadIdXOp = b.create<mlir::gpu::ThreadIdOp>(b.getIndexType(), mlir::gpu::Dimension::x);
    
    //gpu.printf "Hello from thread %d\n", %tid : index
    llvm::SmallVector<mlir::Value,1> printfArgs = { threadIdXOp.getResult() };
    b.create<mlir::gpu::PrintfOp>(b.getStringAttr("Hello from thread %d\n"), printfArgs);

    //gpu.return 
    b.create<mlir::gpu::ReturnOp>();  

    return helloFuncOp ;
}

int main() {

    mlir::MLIRContext context ;
    registerDialects(context);

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    auto loc = mlir::UnknownLoc::get(&context);
    mlir::OpBuilder builder(&context);
    mlir::ImplicitLocOpBuilder b(loc, builder );

    auto moduleOp = mlir::ModuleOp::create(loc);

    // GPU module ======================================================
    // module attributes { gpu.container_module } 
    moduleOp->setAttr("gpu.container_module", builder.getUnitAttr() ) ;
    b.setInsertionPointToStart(moduleOp.getBody());
    
    // gpu.module @kernels {
    auto gpuModuleOp = b.create<mlir::gpu::GPUModuleOp>("kernels");

    gpu_Hello_kernel(b , gpuModuleOp) ;

    // Host ======================================================
    // Restore the insertion point back to host module
    b.setInsertionPointToEnd(moduleOp.getBody());

    auto mainFuncType = b.getFunctionType({}, {});
    auto mainFuncOp = b.create<mlir::func::FuncOp>("main", mainFuncType);

    auto &mainBlock = mainFuncOp.getBody().emplaceBlock();
    b.setInsertionPointToStart(&mainBlock);  

    auto c1 = b.create<mlir::arith::ConstantIndexOp>(1);
    auto cN = b.create<mlir::arith::ConstantIndexOp>(16);

    mlir::gpu::KernelDim3 gridShape  { c1.getResult(), c1.getResult() , c1.getResult() };
    mlir::gpu::KernelDim3 blockShape { cN.getResult(), c1.getResult() , c1.getResult() };

    auto helloKernelRef = mlir::SymbolRefAttr::get(&context, "hello");   
    auto kernelRef = mlir::SymbolRefAttr::get(&context, "kernels", {helloKernelRef});

    // gpu.launch @kernels.hello grid(%c1, %c1, %c1) block(%c16, %c1, %c1)
    b.create<mlir::gpu::LaunchFuncOp>( 
        kernelRef, 
        gridShape,  
        blockShape, 
        mlir::Value(), 
        mlir::ValueRange()     
    );

    b.create<mlir::func::ReturnOp>();
    
    // 6-1.Verify the module
    if ( moduleOp.verify().failed() ) {
        llvm::errs() << "Module verification failed! " ;
        return EXIT_FAILURE ;
    }
 
    // 6-2.Print the MLIR Module
 
    printf ("Generated MLIR Module:\n");
    moduleOp->print(llvm::outs());
    puts("-----------------------------------");
 
    // 7. Clean up and Exit 
    // the MLIRContext will automatically clean up all MLIR objects when it goes out of scope.
    

    return EXIT_SUCCESS;
}
