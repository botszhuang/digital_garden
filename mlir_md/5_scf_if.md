# My MLIR Track #5 - Structure Control Flow - if

By Botsz on April 8, 2026

**Disclaimer** : This is a documentation of my learning process only. Following these steps does not guarantee identical results.

This example demonstrates if-structure control flow whixh implemented in C++. 
The logic is compiled into a shared library and then called via Python which executing native machine code rather than relying on JIT complilation. 
This approch provides a relative simple and clean interface to control multiple complex funcstions.
The complete source code is available in [mlir_5_scf_if](./mlir_5_scf_if/).

The example of if-structure control flow in MLIR is equivalent to, in C++,

```cpp
bool control_flow_demo(int value) {
    bool res = 0 ;
    if (value > 42 ) { res = true ;
    } else {           res = false ;
    }
    return res ;
}
```
---
## 1. Resgistration & Definition of the Type System
First, we must register the ```SCFDialect``` to ensure structured control flow operations are available within the MLIR context:
```cpp
dialect_registry.insert<mlir::scf::SCFDialect>();
```
Secondly, the function requires an ```i32``` integer as input and returns a **boolean** (i1) as the output.
```cpp
// Create Types -----------------------------------------
const unsigned bitWidth32 = 32 ;
auto i32Type = mlir::IntegerType::get(&context, bitWidth32) ;
auto i1Type  = mlir::IntegerType::get(&context, 1) ;
```
## 2. Initialization of Arguments and Constants
Of course, we retrieve the ```i32``` input argument and define the constants needed for the comparision and return value as follow:
```cpp
// Get the input argument0
mlir::Value inputArg0 = entryBlock->getArgument(0);

// Create the Constants
auto fortyTwo = b.create<mlir::arith::ConstantIntOp>(42, bitWidth32);    
auto yes  = b.create<mlir::arith::ConstantOp>(b.getBoolAttr(true));  
auto no   = b.create<mlir::arith::ConstantOp>(b.getBoolAttr(false)); 
```
**Note:** An ```mlir::Value``` is a ```value-type handle``` (similar to a ```std::shared_ptr``` but much lighter).
While the handle itself is stored in the variable's local memory space, it acts as a gateway to the IR graph (operation).
It includes that 
* ```Type```: ```v.getType()``` tells what the data is (the shape, the element type, etc.). 
* ```Def```: ```v.getDefineingOp``` looks "upstream" to find the Operation that produced the value.
* ```Used```: ```v.getUsers``` looks "downstream" to find the "Used" list—the operations consuming this data.


## 3. If-structure Control Flow
We must claculate the condition before initializing the ```if``` operation. With the ```arith``` dialect, we compare the input against the constant (42) to generate a boolean result.
Then, we use ```scf::IfOp``` to handle the control flow logic.
Unlike the standard C++ ```if```, the MLIR can **yield** a value and we capture to use as the final function return.
```cpp
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
```
Now we have the MLIR modul as follow:
```bash
MLIR Module:
module {
  func.func @get_forty_two(%arg0: i32) -> i1 {
    %c42_i32 = arith.constant 42 : i32
    %true = arith.constant true
    %false = arith.constant false
    %0 = arith.cmpi sgt, %arg0, %c42_i32 : i32
    %1 = scf.if %0 -> (i1) {
      scf.yield %true : i1
    } else {
      scf.yield %false : i1
    }
    return %1 : i1
   }
}
```
## 4. Traslation form MLIR to LLVM IR
For the final steps, the MLIR module is translated to the LLVM dialect and then lowered to LLVM IR as usual. 
However, **it is impossible for the structured blocks like ```scf.if``` to jump to LLVM backend directly**. 
We must first lower these high-level operations into the ControlFlow (```cf```) dialect.
```createSCFToControlFlowPass``` transforms ```scf.if``` into standard branches (```cf.cond_br```).
```createConvertControlFlowToLLVMPass``` converts those branches into LLVM-compatible IR (```llvm.cond_br```).
```cpp
// Convert SCF to ControlFlow (creates cf.cond_br)
pass_manager.addPass( mlir::createSCFToControlFlowPass());
    
// Convert ControlFlow to LLVM (creates llvm.cond_br) - MANDATORY
pass_manager.addPass( mlir::createConvertControlFlowToLLVMPass());
```
## 5. Execution with Python
By following this pipeline—registering dialects, defining types, and implementing structured control flow—we can generate highly optimized logic that bridges the gap between high-level Python and low-level hardware. 
Once the MLIR logic is lowered to LLVM IR and compiled into a shared library, it is a simple and clean backend that Python can control with ease.
```python
import ctypes

module = ctypes.CDLL("./libmyFunc.so")

module.get_forty_two.argtypes = [ctypes.c_int32]
module.get_forty_two.restype = ctypes.c_bool

input_val = 50
result = module.get_forty_two(input_val)

if result > 0:
    print(f"Result: {input_val} > 42")
else:
    print(f"Result: {input_val} <= 42")
```
## Refernce

[1] [MLIR. (n.d.). 'func' Dialect. MLIR Documentation.](https://mlir.llvm.org/docs/Dialects/Func/)

[2] [MLIR. (n.d.). 'arith' Dialect. MLIR Documentation.](https://mlir.llvm.org/docs/Dialects/ArithOps/)

[3] [MLIR. (n.d.). 'scf' Dialect. MLIR Documentation.](https://mlir.llvm.org/docs/Dialects/SCFDialect/)

[4] [Introduction to MLIR](https://www.stephendiehl.com/posts/mlir_introduction/)
