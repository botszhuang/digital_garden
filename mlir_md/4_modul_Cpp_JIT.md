# My MLIR Track #4 - Simple MLIR Example With JIT

By Botsz on April 5, 2026

**Disclaimer** : This is a documentation of my learning process only. Following these steps does not guarantee identical results.

Following the previous section [My MLIR Track #3](mlir_3_modul_by_Cpp.md), the MLIR module is now ready. In this section, we add the MLIR operations: registration, lowering, translation, and **JIT** (Just-In-Time) execution. **The complete source code is located in the [mlir_4_cpp_JIT](mlir_4_cpp_JIT) directory.** 

## 1. Registration of Dialects
Before MLIR can process any operations, in the MLIR ecosystem, the system also requires to register translation registration. 

The function of ```registerBuiltinDialectTranslation``` take the translation from the builtin dialect to the LLVM IR in the given registry and ```registerLLVMDialectTranslation``` is to take MLIR's LLVM Dialect operations and turn them into real LLVM IR instructions.

By calling ```InitializeNativeTarget()``` and ```InitializeNativeTargetAsmPrinter()```, LLVM is instructed to prepare its backend specifically for the current CPU. 
This ensures the JIT compiler produces machine instructions that the processor understands, allowing the code to execute successfully on the local hardware.
```cpp
dialect_registry.insert<LLVM::LLVMDialect>();

registerBuiltinDialectTranslation(dialect_registry);
registerLLVMDialectTranslation(dialect_registry);

// Translation to LLVM IR
registerAllToLLVMIRTranslations(dialect_registry);

// Setup Context and Module
MLIRContext context (dialect_registry);
context.loadAllAvailableDialects();

// **Initialize LLVM targets for the JIT**
llvm::InitializeNativeTarget();
llvm::InitializeNativeTargetAsmPrinter();

......
```

## 2. Run the Pass Manager 
The Pass Manager acts as the transformation engine to convert high-level code into lower-level representations.
By ```createArithToLLVMConversionPass```, the math-realted operations are mapped to the LLVM equivalents.
Similarly, ```createConvertFuncToLLVMPass``` transforms standard function structures to adhere to the LLVM calling convention.
This pipeline effectively translates the entire MLIR module into the compatible LLVM IR.
```cpp
    PassManager pass_manager(&context);
    pass_manager.addPass(createArithToLLVMConversionPass());
    pass_manager.addPass(createConvertFuncToLLVMPass());
    if (pass_manager.run(module).failed()) {
        llvm::errs() << "Pass manager failed to run! " ;
        return EXIT_FAILURE ;
    }
```

## 3. Transforms MLIR to LLVM IR
After the lowering passes are complete, the module exists in the MLIR LLVM Dialect, which is still an MLIR-based representation.
To move beyond the MLIR framework, ```mlir::translateModuleToLLVMIR``` acts as the final bridge, converting the MLIR operations into a true ```llvm::Module```. 

This process transforms MLIR's operation-based structure into standard LLVM IR, consisting of LLVM instructions and metadata.  
```cpp
llvm ::LLVMContext llvmContext = llvm::LLVMContext();
auto llmvModule = mlir::translateModuleToLLVMIR(module, llvmContext);
if (!llmvModule) {
    llvm::errs() << "Failed to translate MLIR module to LLVM IR! " ;
    return EXIT_FAILURE ;
}
```

## 4. JIT Execution
The **ExecutionEngine** serves the lowered MLIR module and compile it into executable machine code in memory.
By calling ```ExecutionEngine::create(module)```, the system invokes the LLVM JIT (Just-In-Time) compiler to transform the IR into a binary format specific to the host architecture. 
Once the engine is initialized, ```engine->lookup("get_forty_two")``` is used to search the newly compiled binary for the memory address of the target function.
This step transitions the code from a static representation into a live, callable symbol, setting the stage for direct execution.
```cpp

    // JIT Execution
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
```
Then, the terminal output shows the transformation levels of the module/IR. It begins with the Generated MLIR Module, where logic is written in high-level, hardware-independent dialects like arith and func.

Once the Pass Manager runs, the output transitions to the Generated LLVM Module. 
Here, operations are lowered into the llvm dialect. This is still MLIR, but it is now modeled specifically to map to LLVM’s internal structure.

The final stage, Generated LLVM IR, shows the result of the translation bridge. 
The MLIR wrappers are replaced by standard LLVM assembly (define i32 @get_forty_two()), which is the universal format LLVM backends consume. 
Then, ```Execution result: 42 ```confirms that the JIT successfully compiled this IR into machine code and executed it to return the expected integer.
```bash
$ ./run.sh 

Generated MLIR Module:
module {
  func.func @get_forty_two() -> i32 {
    %c42_i32 = arith.constant 42 : i32
    return %c42_i32 : i32
  }
}
-----------------------------------
Generated LLVM Module:
module {
  llvm.func @get_forty_two() -> i32 {
    %0 = llvm.mlir.constant(42 : i32) : i32
    llvm.return %0 : i32
  }
}
-----------------------------------
Generated LLVM IR:
; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

define i32 @get_forty_two() {
  ret i32 42
}

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
-----------------------------------
Execution result: 42
```

Welcome to check out [the full code for the example](4_mlir_cpp_JIT)

## Reference
[1] [MLIR. (n.d.). 'func' Dialect. MLIR Documentation.](https://mlir.llvm.org/docs/Dialects/Func/)

[2] [MLIR. (n.d.). 'arith' Dialect. MLIR Documentation.](https://mlir.llvm.org/docs/Dialects/ArithOps/)

[3] [LLVM Language Reference Manual](https://llvm.org/docs/LangRef.html)

[4] [Introduction to MLIR](https://www.stephendiehl.com/posts/mlir_introduction/)

