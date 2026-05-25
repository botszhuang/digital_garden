; 1. 定義 "hello world\n" 字串，放在常數空間 (addrspace(4))
; 字串長度為 13 位元組（包含 \n 與 結尾的 \0）
@.str = private unnamed_addr addrspace(4) constant [13 x i8] c"hello world\0A\00", align 1

; 2. 宣告內建的 vprintf 函式
declare i32 @vprintf(ptr, ptr)

; 3. 定義 GPU Kernel
define void @hello() {
entry:
    ; 將格式化字串從常數空間轉為泛用指標 (Generic Pointer)
    ; %str_ptr = cast addrspace(4) ptr addrspace(4) @.str to ptr
    %str_ptr = addrspacecast ptr addrspace(4) @.str to ptr

    ; 因為不需要帶入任何數字或變數，第二個參數直接傳入 null
    %result = call i32 @vprintf(ptr %str_ptr, ptr null)

    ret void
}

; 標記此函式為 CUDA Kernel
!nvvm.annotations = !{!0}
;!0 = !{ptr @hello, i32 1}
!0 = !{ptr @hello, !"kernel", i32 1}
