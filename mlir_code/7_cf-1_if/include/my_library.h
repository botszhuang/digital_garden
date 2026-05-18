// my_library.h 
// developer : botsz
// Date : 2026-04-06
// description : This header file declares utility functions for MLIR module verification, printing, and emission. It also includes necessary MLIR and LLVM headers for working with MLIR modules, pass management, and LLVM IR translation.

#pragma once

#include <cstdlib>
#include <iostream>

#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/Builders.h>
#include <mlir/IR/Verifier.h>
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Dialect/SCF/IR/SCF.h>
#include <mlir/Dialect/ControlFlow/IR/ControlFlowOps.h>
#include <mlir/Dialect/LLVMIR/LLVMDialect.h>
#include <mlir/Target/LLVMIR/Dialect/All.h>
#include <mlir/Target/LLVMIR/Export.h>
#include <mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h>
#include <mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h>
#include <mlir/Conversion/ArithToLLVM/ArithToLLVM.h>
#include <mlir/Conversion/FuncToLLVM/ConvertFuncToLLVMPass.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
#include "mlir/Dialect/SCF/IR/SCF.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <llvm/Support/FileSystem.h>

void verify_mlir_module ( mlir::ModuleOp module ) ; 
void print_module       ( mlir::ModuleOp module, std::string Tag      ) ;
void save_module_to_file( mlir::ModuleOp module, std::string filename ) ;

void translate_mlir_module_to_llvm ( mlir::MLIRContext * contextPtr, mlir::ModuleOp module ) ;  
std::unique_ptr<llvm::Module> translate_llvm_module_to_llvm_ir ( mlir::ModuleOp module , llvm::LLVMContext * llvmContext ) ;
void print_llvm_ir( llvm::Module * llvmModule, std::string Tag ) ;
void save_llvm_ir_to_file( llvm::Module * llvmModule, std::string filename ) ;
void llvm_ir_to_so( std::string llFile ) ;
