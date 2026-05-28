import pycuda.driver as cuda
import pycuda.autoinit

def run_test( ptx_file , func_name , label ):
    # 1. Load the PTX file
    ptx_module = cuda.module_from_file(ptx_file)
    hello_fn = ptx_module.get_function(func_name)

    # 2. Run Function on GPU
    print(f"--- {label} ---")
    hello_fn(block=(1, 1, 1), grid=(1, 1))
    cuda.Context.synchronize() # Wait for the GPU to finish printing before Python exits
    print("------------------")

if __name__ == "__main__":

    run_test("cuda_hello.ptx", "hello", "CU")     

    run_test("llvm_hello.ptx", "hello", "LLVM")