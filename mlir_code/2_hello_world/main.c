#include <stdio.h>
extern int sample_c() ;
extern int sample_llvm() ;
int main() {
    printf("sample_c: %d\n", sample_c());
    printf("sample_llvm: %d\n", sample_llvm());
    return 0 ;
}


