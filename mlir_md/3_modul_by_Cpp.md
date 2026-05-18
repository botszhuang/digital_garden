# My MLIR Track #3 - Simple MLIR Example by C++

By Botsz on April 3, 2026

**Disclaimer** : This is a documentation of my learning process only. Following these steps does not guarantee identical results.

Generating an MLIR module using the C++ API—typically via OpBuilder—is generally more efficient and robust than writing it by hand. In a programmatic environment, maintaining the code becomes significantly easier; for instance, if you need to update a data type from ```f32``` to ```f16``` across an entire module, changing a single variable in C++ is much faster and more reliable than performing a "find and replace" in a large text file.

Furthermore, the C++ API provides a crucial layer of safety by allowing the compiler to check the code at compile-time. When using ODS (Operation Definition Specification) generated wrappers, the C++ compiler can flag errors such as passing a float to a function expecting an int before running the generator. 

While we generated the module by hand in the previous section, we are now going to transition to constructing a simple MLIR module using ```C++``` and it is equivalent to 
```cpp
int mian(){
  return 42 ;
}
```

---

**The complete source code is located in the [mlir_3_modul_by_Cpp](mlir_3_modul_by_Cpp) directory.** 
Constructing an MLIR module via the C++ API involves a specific integration order for its core components:

### 1.Registration of Dialects
The registration of Dialects is required to ensure MLIR engine to recognize and utilize specific operation sets, such as Arith for mathematical computations and Func for function definitions.
```cpp
DialectRegistry dialect_registry;
dialect_registry.insert<ArithDialect>();
dialect_registry.insert<FuncDialect>();
```
### 2. Setup MLIRContext & ModuleOp
The **MLIRContext** acts as the global container of all MLIR state , including unique symbols and operation definitions. **MLIRContext** holds the definitions and rules that make the code valid. Therefore, the necessary Dialects must be explicitly loaded for the context to recognize specific operations.
Once the context is prepared, **Location** (```loc```) must be established. This location is registered within the context and serves as a mandatory metadata tag for every operation created thereafter. 
Then, a **ModuleOp** (```module```) is initialized as the top-level container. While the context provides the rules and vocabulary, the ModuleOp acts as the structural root that physically holds the functions and arithmetic operations defined by the developer.
```cpp
MLIRContext context (dialect_registry);
context.loadAllAvailableDialects();
auto loc = UnknownLoc::get(&context);
auto module = ModuleOp::create(loc);
```

### 3.Create Types
A variable like ```i32Type``` represents a **Type Object** that is owned and managed by the MLIRContext.
Once ```i32Type``` is defined, it cannot be "changed" into an i64 or any other format.
If a different type is needed, the developer must simply request a new one from the context.
```cpp     
auto i32Type = IntegerType::get(&context, 32) ;
```

### 4. Create OpBuilder & ImplicitLocOpBuilders
An **OpBuilder** is utilized as a cursor to manage the insertion of functions and operations into the ModuleOp. 
By passing the established Location and Type Objects to the builder, the developer programmatically assembles the SSA (Static Single Assignment) graph that forms the logic of the MLIR module.
Forthemore, an **ImplicitLocOpBuilder** is emplyed to simplify the programming process.
An ImplicitLocOpBuilder internally stores a default location, which reduces the overhead of manually managing and passing the location argument to every builder call.
```cpp
OpBuilder builder(&context);
ImplicitLocOpBuilder b(loc, builder );
b.setInsertionPointToEnd(module.getBody());
```

### 5.Function Operator & Entry Block
To construct a **Function Operator**(```funcOp```), the ```i32Type``` is used to define the function's input and output types. 
Then, the OpBuilder's insertion point is set to the function's body to allow for the addition of further operations.
```cpp
auto funcType = b.getFunctionType({}, {i32Type});
auto funcOp = b.create<FuncOp>("get_forty_two", funcType);
builder.insert(funcOp);
```

An **entry block** is added to the funcOp to provide the structural scope for the function's logic.
By calling ```setInsertionPointToStart```, the builder is moved inside this block as a "cursor", effectively setting the stage for operation placement.
From this position, the builder creates a           ```ConstantIntOp``` to define a literal value and inserts it into the function.
Finally, a ```ReturnOp``` is created and inserted to terminate the function. 
This sequence ensures that the operations are correctly nested and linked within the function's scope.
```cpp    
auto entryBlock = funcOp.addEntryBlock();
b.setInsertionPointToStart(entryBlock);

auto fortyTwo = b.create<ConstantIntOp>(42, 32); // %c42_i32 = arith.constant 42 : i32
b.create<ReturnOp>(fortyTwo.getResult()); // return %c42_i32 : i32
```

### 6.Verify and Print the Module
Before processing the IR further, it is critical to call ```module.verify()```. This internal check ensures that the C++ construction phase are correct.

Once verified, the module can be serialized into its human-readable text format. By calling ```module->print(llvm::outs())```, the internal data structures are converted back into the standard MLIR assembly language. 
It allows the developer to inspect the final SSA graph and ensure the logic matches the intended design.
```cpp
// Verify the module
if ( module.verify().failed() ) {
    llvm::errs() << "Module verification failed! " ;
    return EXIT_FAILURE ;
}

// Print the mlir Module
printf ("Generated MLIR Module:\n");
module->print(llvm::outs());

// Clean up and Exit 
// the MLIRContext will automatically clean up all MLIR objects when it goes out of scope.

```
The MLIRContext owns all attributes, types, and operation definitions and it will automatically clean up these objects when it goes out of scope.


## Reference
[1] [MLIR. (n.d.). 'func' Dialect. MLIR Documentation.](https://mlir.llvm.org/docs/Dialects/Func/)

[2] [MLIR. (n.d.). 'arith' Dialect. MLIR Documentation.](https://mlir.llvm.org/docs/Dialects/ArithOps/)

[3] [Introduction to MLIR](https://www.stephendiehl.com/posts/mlir_introduction/)

