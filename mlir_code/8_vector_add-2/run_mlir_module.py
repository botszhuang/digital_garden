import ctypes
import numpy as np

# 載入函式庫
lib = ctypes.CDLL("./libmyFunc.so")

lib.vector_add.argtypes = [
    ctypes.c_void_p, # arrayA
    ctypes.c_int32,  # A_Size
    ctypes.c_void_p, # arrayB
    ctypes.c_void_p  # arrayC
]

# Define the return type (void)
lib.vector_add.restype = None

# Create input data
N = 100 # Total size
a = np.random.randint(0, 10, size=N).astype(np.int32)
b = np.random.randint(0, 10, size=N).astype(np.int32)
c = np.zeros(N, dtype=np.int32)

# Get pointers to the underlying data
# Use .ctypes.data to get the memory address for the shared library
ptr_a = a.ctypes.data
ptr_b = b.ctypes.data
ptr_c = c.ctypes.data

# Call the function
lib.vector_add(ptr_a, N, ptr_b, ptr_c)

# Verify results
expected = a + b
if np.array_equal(c, expected):
    print("Success! Vector addition is correct.")
else:
    print("Mismatch found.")

np.set_printoptions(formatter={'int': lambda x: f"{x:3d}"})

print("Array A (Input 1):", a[90:])
print("Array B (Input 2):", b[90:])
print("Array C (MLIR  ): ", c[90:])
print("Array C (Python): ", expected[90:])

