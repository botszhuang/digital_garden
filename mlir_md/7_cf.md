# My MLIR Track #7 - Control Flow

By Botsz on May 19, 2026

**Disclaimer** : This is a documentation of my learning process only. Following these steps does not guarantee identical results.
```mlir
MLIR Module:
module {
  func.func @control_flow_demo(%arg0: i32) -> i1 {
    %c42_i32 = arith.constant 42 : i32
    %true = arith.constant true
    %false = arith.constant false
    %0 = arith.cmpi sgt, %arg0, %c42_i32 : i32
    cf.cond_br %0, ^bb1, ^bb2
  ^bb1:  // pred: ^bb0
    cf.br ^bb3(%true : i1)
  ^bb2:  // pred: ^bb0
    cf.br ^bb3(%false : i1)
  ^bb3(%1: i1):  // 2 preds: ^bb1, ^bb2
    return %1 : i1
  }
}
```