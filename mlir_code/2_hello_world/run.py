import ctypes

module_c = ctypes.CDLL("./out/libsample_c.so")
module_llvm = ctypes.CDLL("./out/libsample_llvm.so")

# Setup and call the C function
module_c.sample_c.argtypes = []
module_c.sample_c.restype = ctypes.c_int
print(f"sample_c result: {module_c.sample_c()}")

# Setup and call the LLVM function
module_llvm.sample_llvm.argtypes = []
module_llvm.sample_llvm.restype = ctypes.c_int
print(f"sample_llvm result: {module_llvm.sample_llvm()}")
