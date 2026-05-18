import ctypes

module = ctypes.CDLL("./libmyFunc.so")

func = module.control_flow_demo
func.argtypes = [ctypes.c_int32]
func.restype = ctypes.c_bool

input_val = 50
result = func(input_val)

if result > 0:
    print(f"Result: {input_val} > 42")
else:
    print(f"Result: {input_val} <= 42")
