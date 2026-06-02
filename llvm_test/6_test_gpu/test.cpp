#include <cstdint>
#include <cstdlib>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Constants.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Triple.h>

#include <iostream>
#include <memory>
#include <vector>
#include <string>


// CodeGen 與 Target 相關標頭檔（輸出 PTX 必需）
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/TargetSelect.h>

// 初始化 NVPTX 後端所需的標頭檔
#include <llvm/InitializePasses.h>
#include <llvm/IR/LegacyPassManager.h> // 修正：PassManager 必需的標頭檔
/*
__global__ void test() {

L_EENTRY:
    return ;
}
*/

// ====== CUDA Driver API 執行 PTX 相關函式宣告 =====

#include <cuda.h> // 必須引入 CUDA Driver API 標頭檔
// Check CUDA API calls
#define CHECK_CUDA(res) \
    if (res != CUDA_SUCCESS) { \
        const char* errStr = nullptr; \
        cuGetErrorString(res, &errStr); \
        std::cerr << "FILE: " << __FILE__ << ", LINE: " << __LINE__ << ", CUDA error: " << errStr << std::endl; \
        return EXIT_FAILURE; \
    }

typedef struct {
    //Target Machine 參數
    std::string cpu; 
    std::string features ;
    llvm::TargetOptions options;
} generate_ptx_parameter_Struct;
std::string generate_ptx(llvm::Module &module , const generate_ptx_parameter_Struct &targetMachineParams) {
    // 1. 初始化 LLVM 的 NVPTX 目標架構
    LLVMInitializeNVPTXTargetInfo();
    LLVMInitializeNVPTXTarget();
    LLVMInitializeNVPTXTargetMC();
    LLVMInitializeNVPTXAsmPrinter();

    // 2. 取得 Target Triple (修正：型態改為 llvm::Triple)
    llvm::Triple targetTriple = module.getTargetTriple();
    std::string error;
    
    // 修正：lookupTarget 直接傳入 targetTriple 物件，消除 Deprecated 警告
    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    
    if (!target) {
        std::cerr << "找不到對應的 Target: " << error << "\n";
        return "";
    }

    // 3. 設定 Target Machine 參數
    std::unique_ptr<llvm::TargetMachine> targetMachine(
        target->createTargetMachine(
            targetTriple, 
            targetMachineParams.cpu, 
            targetMachineParams.features, 
            targetMachineParams.options,  
            llvm::Reloc::Static, 
            llvm::CodeModel::Small, 
            llvm::CodeGenOptLevel::Aggressive
        )
    );

    if (!targetMachine) {
        std::cerr << "無法建立 TargetMachine\n";
        return "";
    }

    // 設定 Module 的 Data Layout
    module.setDataLayout(targetMachine->createDataLayout());

    // 4. 建立輸出流並讓 TargetMachine 產生代碼
    llvm::SmallString<4096> ptxBuffer;
    llvm::raw_svector_ostream os(ptxBuffer);
    
    llvm::legacy::PassManager pm; // 修正：現在有了 LegacyPassManager.h，這行可以正確編譯了
    
    if (targetMachine->addPassesToEmitFile(pm, os, nullptr, llvm::CodeGenFileType::AssemblyFile)) {
        std::cerr << "NVPTX 後端不支援直接輸出此檔案類型\n";
        return "";
    }

    // 執行 CodeGen Pass 序列
    pm.run(module);

    // print ptx code as string 
    std::cout << "\n--- Generated PTX Code ---\n" << ptxBuffer.str().str() << std::endl;
    
    // return PTX code as string
    return std::string(ptxBuffer.str());
}
void nv_metadata(llvm::Function &Func){
    llvm::Module *module = Func.getParent();
    llvm::LLVMContext &ctx = module->getContext();

    llvm::NamedMDNode *md = module->getOrInsertNamedMetadata("nvvm.annotations");
    llvm::Metadata *mdVals[] = {
        llvm::ValueAsMetadata::get(&Func),
        llvm::MDString::get(ctx, "kernel"),
        llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1))
    };
    md->addOperand(llvm::MDNode::get(ctx, mdVals));    
}
// === CUDA Driver API 執行 PTX 相關函式 ===

void generate_llvm_code(llvm::Module &module) {

    auto &ctx = module.getContext();
    llvm::IRBuilder<> b(ctx);

    // type
    auto * voidTy = b.getVoidTy();

    // defin i32 array_add( i32 * a , i32* b , i32 *c , int size ) 
    char funcName [] = "test" ;
    std::vector<llvm::Type*> Args = {};
    auto *FuncTy = llvm::FunctionType::get(voidTy, Args, false);

    auto *Func = llvm::Function::Create(
        FuncTy,
        llvm::Function::ExternalLinkage,
        funcName,
        module
    );

    Func->setCallingConv(llvm::CallingConv::PTX_Kernel);
    
    // lebel
    auto * L_Entry = llvm::BasicBlock::Create(ctx, "L_ENTRY", Func);

    b.SetInsertPoint(L_Entry);
    b.CreateRetVoid();  
    
}
void verify_module(llvm::Module &module) {
    if (llvm::verifyModule(module, &llvm::errs())) {
        std::cerr << "LLVM Module failed @ " << __FILE__ << ":" << __LINE__ << std::endl;
        exit(EXIT_FAILURE);
    }
}
void print_module(llvm::Module &module) {
    std::cout << "--- Print LLVM IR ---\n";
    module.print(llvm::outs(), nullptr);
}

int main() {
    
    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("gpu_module", *ctx);
    
    module->setTargetTriple(llvm::Triple("nvptx64-nvidia-cuda"));
    generate_llvm_code(*module);
    nv_metadata(*module->getFunction("test"));
            
    verify_module(*module);
    print_module(*module);

    // ======= CUDA Driver API to Execute PTX =======
    // set Target Machine parameters for PTX generation
    generate_ptx_parameter_Struct Target_Machine_Params;
    Target_Machine_Params.cpu = "sm_89";
    Target_Machine_Params.features = "";
    Target_Machine_Params.options = llvm::TargetOptions();

    std::string ptxCode = generate_ptx(*module , Target_Machine_Params);

    // ===== RUN CUDA Driver API =====

    std::cout << "\n--- 開始在 GPU 上執行 ---\n";

    // 1. 初始化 CUDA 驅動程式
    CHECK_CUDA(cuInit(0));

    // 2. 獲取第一個 GPU 裝置 (Device 0)
    CUdevice device;
    CHECK_CUDA(cuDeviceGet(&device, 0));

    // 3. 建立 CUDA Context (相當於 GPU 的處理程序環境)
    CUcontext context;
    CHECK_CUDA(cuCtxCreate(&context, 0, device));

    // 4. 編譯 PTX 代碼成 CUDA 模組
    CUmodule cuModule;
    CHECK_CUDA(cuModuleLoadData(&cuModule, ptxCode.c_str()));

    // 5. kernel 函數
    CUfunction kernel;
    CHECK_CUDA(cuModuleGetFunction(&kernel, cuModule, "test"));

    // 6. setup kernel parameters (這裡沒有參數，所以傳入 nullptr)
    // void *Args[] = { &param1, &param2, ... }; // 如果 kernel 有參數，這裡需要傳入指向參數的指標
    void* kernelArgs[] = {};

    // 7. 執行 kernel (使用 1 個 block 和 1 個 thread)
    std::cout << "Launching a kernel...\n";
    CHECK_CUDA(cuLaunchKernel(kernel,
                             1, 1, 1, // Blocks in a Grid
                             1, 1, 1, // Threads in a Block
                             0, //shared memory size
                             nullptr, // stream
                             kernelArgs, // kernel parameters
                             nullptr // extra
                             )); 
    // 8.Synchronize to wait for kernel execution to finish
    CHECK_CUDA(cuCtxSynchronize());
    std::cout << "--- GPU 上的執行完成 ---" << std::endl;
    
    //9. Cleanup
    CHECK_CUDA(cuModuleUnload(cuModule));
    CHECK_CUDA(cuCtxDestroy(context));

    return EXIT_SUCCESS;

}

