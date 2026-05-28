---
title: "hello.cpp for gpu #1 string"
author: "Botsz"
date: 2026-05-29
---
## 1. Define the string `hello world\n\0` and place it in the constant memory `addrspace(4)`
During the compiling process , build a global read-only string constant and store it in **Constant Memory** (Address Space 4).

### 1.1 Build a String(Data)
---  
This decides the string's context and type, and it nned to include the header file.
```c
#include "llvm/IR/Constants.h"
```
```c
std::string StrVal = "hello world\n"; 
Constant *StrConstant = ConstantDataArray::getString(Context, StrVal, true);
```
- `ConstantDataArray::getString(...)` creates a 'character array constant' within LLVM.
- `Context`: LLVM context
- `StrVal`: Set the string here, "hello world\n".
- `true`: **AddNull**. Add the '\0'(Null Terminator) in the terminal automatically.

### 1.2 Build a Container
---
It is a global variable and specify its physical attributes such as Address Space, alignment, etc.[2].
```c
GlobalVariable *GV = new GlobalVariable(
    *TheModule,             // 1. Module
    StrConstant->getType(), // 2. Data type ( e.g., [13 x i8])
    true,                   // 3. isConstant = true
    GlobalValue::PrivateLinkage, // 4. Linkage Type
    StrConstant,            // 5. Initializer
    ".str",                 // 6. Variable name
    nullptr,                // 7. Insert before a specific variable (usually nullptr)
    GlobalValue::NotThreadLocal, // 8. Thread Local Storage(TLS) attribute
    4                       // 9. Address Space ID！
);
```
4. **Linkage Type** (`GlobalValue::PrivateLinkage`): Specifies the symbol's visibility. 
    - `PrivateLinkage` means the symbol is only visible within the current translation unit and will not appear in the global symbol table.
    - `ExternalLinkage` means globally visible. Any other file in the project can reference and use this variable or function.

5. **Initializer** (`StrConstant`): The initial value assigned to the global variable. For a global array, this is typically an ConstantDataArray or ConstantExpr.

6. **Variable Name** (`.str`): The name of the variable in the LLVM IR.  `.str` is commonly used for compiler-generated internal strings.

7. **Insert Before** (`nullptr`): A pointer to another global variable if you want to insert this new one before an existing one in the module's global list. Passing `nullptr` simply appends it to the end of the list.

| Variable | | Linkage | UnnamedAddr | AddrSpace | isConstant | Type | Initializer | | Alignment |
| :--- | :--- |:--- | :--- | :--- | :--- | :--- | :--- | :---|:--- |
| `@.str` | = | `private` | `unnamed_addr` | `addrspace(4)` | `constant` | `[13 x i8]` | `c"hello world\0A\00"` | , |`align 1` |

### 1.3 Properties of a Global Variable
---
#### 1.3.1 Address
```c
    GV->setUnnamedAddr( llvm::GlobalValue::UnnamedAddr::Local); // private
```
This tells the LLVM optimizer that the ***address*** of this global variable is not important, only its content matters. ???

If two different global variables have the exact same constant data (for example, two identical string literals), and both are marked `Local` unnamed address, the LLVM Linker-Constant-Merge pass can merge them into a single memory location to save space.???

|Enumerator||
|:---|:---|
|None||
|Local| If a local_unnamed_addr attribute is attached to a global, the address is known to be insignificant within the module [3].|
|Global||
#### 1.3.1 Padding
```c
    GV->setAlignment(llvm::Align(1));
```
An alignment of 1 means the variable has zero padding wastage in the final binary[4].


## Reference
[1] llvm::ConstantDataArray Class Reference https://llvm.org/doxygen/classllvm_1_1ConstantDataArray.html

[2] llvm::GlobalValue Class Reference https://llvm.org/doxygen/classllvm_1_1GlobalValue.html#aedfa75f0c85c4aa85b257f066fbea57c

[3] IR: Introduce local_unnamed_addr attribute. https://reviews.llvm.org/D20348

[4] https://llvm.org/doxygen/classllvm_1_1GlobalVariable.html



