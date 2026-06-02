llvm_dir := ../../llvm-bin

TARGET := test
SRC    := ${TARGET}.cpp

CUDA_PATH := /usr/local/cuda-12.8/targets/x86_64-linux
CUDA_INC  := -I$(CUDA_PATH)/include
CUDA_LIB  := -L$(CUDA_PATH)/lib -L$(CUDA_PATH)/lib/stubs

CLANG := $(llvm_dir)/bin/clang++
LLVM_CONFIG := $(llvm_dir)/bin/llvm-config

CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags) $(CUDA_INC) -std=c++17 -O3
LDFLAGS  := $(shell $(LLVM_CONFIG) --ldflags --libs core support native nvptx --system-libs) $(CUDA_LIB) -lcuda

all: $(TARGET)

$(TARGET): $(SRC)
	@echo "Compiling LLVM API Code ($<) using built Clang..."
	$(CLANG) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	@echo "Cleaning up..."
	rm -f $(TARGET)

.PHONY: all clean