import ctypes

module = ctypes.CDLL("./libmyFunc.so")

func = module.control_flow_for
func.argtypes = [ctypes.c_int32]
func.restype = ctypes.c_int32

input_val = 50
result = func(input_val)

print(f"Result: {result}")
