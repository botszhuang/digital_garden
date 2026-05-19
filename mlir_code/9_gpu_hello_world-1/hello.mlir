module attributes { gpu.container_module } {
  gpu.module @kernels {
    gpu.func @hello() kernel {
      %tid = gpu.thread_id x
      gpu.printf "Hello from thread %d\n", %tid : index
      gpu.return 
    }
  }

  func.func @main() {
    %c1 = arith.constant 1 : index
    %cN = arith.constant 16 : index
    gpu.launch_func @kernels::@hello
        blocks in (%c1,%c1,%c1)
        threads in (%cN,%c1,%c1)
    return 
  }
}

