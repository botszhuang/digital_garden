import ctypes
import numpy as np

# 載入函式庫
lib = ctypes.CDLL("./libmyFunc.so")

# 定義正確的 MemRef 結構體
class memRefDescriptor(ctypes.Structure):
    _fields_ = [
        ("allocated", ctypes.c_void_p),
        ("aligned", ctypes.c_void_p),
        ("offset", ctypes.c_longlong),
        ("size", ctypes.c_longlong),
        ("stride", ctypes.c_longlong)
    ]

def create_memRef(np_array):
    ptr = np_array.ctypes.data_as(ctypes.c_void_p)
    ref = memRefDescriptor()
    ref.allocated = ptr # 這裡會正確寫入結構體
    ref.aligned = ptr
    ref.offset = 0
    ref.size = np_array.size
    ref.stride = 1
    return ref

data_size = 1024
a = np.array([1] * data_size, dtype=np.int32)
b = np.array([2] * data_size, dtype=np.int32)
c = np.zeros(data_size, dtype=np.int32)

m_a = create_memRef(a)
m_b = create_memRef(b)
m_c = create_memRef(c)

# 呼叫 MLIR 產生的函式
func = lib._mlir_ciface_vector_add

# 呼叫函式並傳遞結構體指標
func(ctypes.byref(m_a), ctypes.byref(m_b), ctypes.byref(m_c))


# 顯示前 10 個結果
print(f"Result C (first 10): {c[:10]}")

