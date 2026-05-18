# My MLIR Track #6 - Structure Control Flow - for

By Botsz on April 12, 2026

**Disclaimer** : This is a documentation of my learning process only. Following these steps does not guarantee identical results.

This example demonstrates if-structure control flow whixh implemented in C++. 
The logic is compiled into a shared library and then called via Python which executing native machine code rather than relying on JIT complilation. 
This approch provides a relative simple and clean interface to control multiple complex funcstions.
The complete source code is available in [mlir_6_scf_if](./mlir_6_scf_for/).

The example of for-structure control flow in MLIR is equivalent to, in C++,

```cpp
int control_flow_for(int value) {
    int sum = 0 ;
    for ( int i = 0 ; i < value ; i++ )
        sum = sum + i ;
    }
    return sum ;
}
```
---
## 1. Set up the arguments for the ```for``` loop
In MLIR, ```iter_args``` are not restricted to the index type; they simply must match the data type of the value being yielded (such as i32, f32, or index). 
However, the loop's **induction variable** (the counter) is always an ```index``` type to maintain platform-independent memory addressing and loop bounds. 
Consequently, for compatibility, the **lower bound**, **upper bound**, and **step** must also be of the index type.
- ```forLowBound``` and ```forStep``` : Use ```ConstantIndexOp``` to obtain the **index** values for loop control.
- ```forUpBound``` : Use ```IndexCastOp``` to cast the first argument of this function to the **index** type.
- ```sum_init```: This remains a standard **integer** (i32) to serve as the initial value for the accumulation.

```cpp
// 'forLowBound', 'forUpBound', and 'forStep' define the range.
// 'sum_init' is the initial value for the loop-carried variable.
auto forLowBound = b.create<mlir::arith::ConstantIndexOp>(0);
auto forUpBound  = b.create<mlir::arith::IndexCastOp>( b.getIndexType(),entryBlock->getArgument(0));
auto forStep     = b.create<mlir::arith::ConstantIndexOp>(1);
auto sum_init    = b.create<mlir::arith::ConstantIntOp>( i32Type, 0);
```
## 2. Initialize the ```scf::ForOp```
The loop is created by four primary inputs to control the **Bounds** and **Step** to control the loop number. 
The ```mlir::ValueRange{ sum_init }``` defines the initial value of the "loop-carried" variable. 
This allows the sum value to persist and update across iterations.
```cpp
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
```
Once the loop is created, the builder's insertion point is moved inside the loop's body block:
- ```currentSum```: This retrieves the currnt value of accumulator for the current iteration. On the first run, this means ```currentSum = sum_init```; on he subsequent run, it is the value yielded by the previous iteration, ```currentSum = nextSum.getResult()```. 
- **Casting the Induction Variable**: The induction variable (```i```) of the loop is an ```mlir::index``` type. Therefore, it must be cast to ```i32``` via an ```IndexCastOp``` before the ```mlir::arith::AddIOp``` operation.
- **Addition**: ```nextSum``` is obtianed by adding the casted induction variable (```ivI32```) to the ```currentSum```.
- **Yield the Result**: The ```scf::YieldOp``` is used to pass the updated total to the subsequent iteration. It is necessary to call ```nextSum.getResult()``` because the destination of the yield operation is the ```mlir::Value``` currentSum (the ```iter_arg```) of the next iteration.

## 3. Execution with Python
By lowering the MLIR logic to LLVM IR and compiling it into a native shared library, we create a high-performance backend that Python can orchestrate with a clean, simple interface. This method successfully bridges the gap between flexible scripting and low-level machine code, providing a scalable way to manage complex functions.
```bash
$ ./run.sh
-----------------------------------
Computer successfully generated libmyFunc.so
Result: 1225
```
The complete source code is available in [mlir_6_scf_for](./mlir_6_scf_for/).

## Refernce

[1] [MLIR. (n.d.). 'func' Dialect. MLIR Documentation.](https://mlir.llvm.org/docs/Dialects/Func/)

[2] [MLIR. (n.d.). 'arith' Dialect. MLIR Documentation.](https://mlir.llvm.org/docs/Dialects/ArithOps/)

[3] [MLIR. (n.d.). 'scf' Dialect. MLIR Documentation.](https://mlir.llvm.org/docs/Dialects/SCFDialect/)