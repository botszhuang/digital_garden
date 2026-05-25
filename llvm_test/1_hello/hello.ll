; Declare the external C standard library function 'puts'
declare i32 @puts(i8*)

; Define a global constant string for "Hello, World!"
@.str = private unnamed_addr constant [14 x i8] c"Hello, World!\00"

; Define the main function
define i32 @main() {
    ; Get a pointer to the first element of our string array
    %cast2str = getelementptr [14 x i8], [14 x i8]* @.str, i64 0, i64 0
    
    ; Call the 'puts' function with the string pointer
    call i32 @puts(i8* %cast2str)
    
    ; Return 0 from main
    ret i32 0
}