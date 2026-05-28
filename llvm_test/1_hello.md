---
title: "LLVM - hello.ll"
auther: "Botsz"
date: 2026-05-26
---
## 1. Installation
Simply create a second `build` folder inside the llvm-project repository.
```bash
# Create a new, separate build directory
mkdir build-llvm && cd build-llvm

# Configure for a standard LLVM + Clang build
cmake -G Ninja ../llvm \
  -DLLVM_ENABLE_PROJECTS="clang" \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_TARGETS_TO_BUILD="Host"

# Build it
ninja
```
**Tip to save time and disk space**: If LLVM is only needed to compile code for the current machine, including `-DLLVM_TARGETS_TO_BUILD="host"` prevents the compilation of backends for ARM, AArch64, MIPS, PowerPC, etc., significantly cutting down build time.

## 2. Hello Woeld!
```llvm
; Define the main function --> int main ( )
define i32 @main() {

    ; Get a pointer to the first element of our string array
    %mystr = getelementptr [14 x i8], [14 x i8]* @.str, i64 0, i64 0
    
    ; Call the 'puts' function with the string pointer
    call i32 @puts(i8* %mystr)
    
    ; Return 0 from main
    ret i32 0
}
```
### 2.1 Define Function 
```llvm
define i32 @main()
```
- `define`: This tells LLVM that a function is being defined here (as opposed to `declare`, which just references an external function defined elsewhere).
- `i32`: a 32-bit signed integer
- `@main`: The name of the function. 
- `@`: This signifies a **global** identifier. All functions and global variables in LLVM IR begin with `@`.
- `()`: a parameter list

Result: `define i32 @main()` : It defines a global function `main`, taking no arguments, and return a 32-bit signed integer.

### 2.2. The `getelementptr` (GEP) Instruction
```llvm
%mystr = getelementptr [14 x i8], [14 x i8]* @.str, i64 0, i64 0
```
- `%mystr`: A **local** identifier (indicated by %). It is a temporary register to holds the calculated address.
- `getelementptr`: The instruction gets the address of a subelement from a structural type (like an array or a struct).
- `[14 x i8]`, [14 x i8]*: This specifies the base type being stepped through ([14 x i8], an array of 14 8-bit integers/characters). 
- `[14 x i8]*`: This argument provides the type and address of the global string variable (@.str).
- **The First Index (`i64 0`)**: This steps through the pointer itself. Because `@.str` is a pointer to the array, index `0` means **stay at the beginning of the memory block pointed to by @.str**.
- **The Second Index (`i64 0`)**: This steps into the array structure. Index 0 means **give me the address of the very first element (index `0`) inside that 14-byte array**.

Result: `%mystr` now holds a simple `i8*` (a pointer to a character, or a char* in C syntax), which points directly to the first letter of the string.

```c
const char .str[14] = "Hello, World!\0"; // The global array
const char* mystr = &.str[0];
```
### 2.3. Calling the External Function

```llvm
call i32 @puts(i8* %mystr)
```
- `call`: Executes a function.
- `i32`: Indicates that the function being called returns a 32-bit integer as.
- `puts` is from the standard C library (libc).
- `(i8* %mystr)`: The argument passed to the function. 

### 2.4. Returning from Main
```
ret i32 0
```
- `ret`: a return instruction. 
- `i32 0`: a return value. 
  - signals that the program executed successfully without errors.

## The C Equivalence
If this were written in C, the code would look like this:
```c
const char .str[14] = "Hello, World!\0"; // The global array

int main() {
    // This is what the GEP instruction does:
    const char* mystr = &.str[0]; 
    
    puts(mystr);
    return 0;
}
```

The complete example is available in [here](./1_hello/)

## Reference
[1] Getting Started with the LLVM System https://llvm.org/docs/GettingStarted.html#getting-the-source-code-and-building-llvm