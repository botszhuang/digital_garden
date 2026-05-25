cmake_minimum_required(VERSION 3.22)
project(MLIRGpuHello LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Handle dynamic executable name passed from the bash script
if(NOT EXE_NAME)
  set(EXE_NAME "mlir_gpu_hello")
endif()

find_package(MLIR REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS} ${MLIR_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

if(NOT LLVM_ENABLE_RTTI)
  if(MSVC)
    add_compile_options(/GR-)
  else()
    add_compile_options(-fno-rtti)
  endif()
endif()

# Use your main file name (Ensure your source file is actually named mlir.cpp or main.cpp)
add_executable(${EXE_NAME} mlir.cpp)

llvm_map_components_to_libnames(llvm_libs 
  native
  nativecodegen
  asmprinter
  core
  support
)

target_link_libraries(${EXE_NAME}
  PRIVATE
  # --- MLIR Translations ---
  MLIRBuiltinToLLVMIRTranslation
  MLIRLLVMToLLVMIRTranslation
  
  # --- MLIR Dialects ---
  MLIRArithDialect
  MLIRFuncDialect
  MLIRGPUDialect
  MLIRLLVMDialect
  
  # --- MLIR Core ---
  MLIRIR
  MLIRSupport
  ${llvm_libs}
)


