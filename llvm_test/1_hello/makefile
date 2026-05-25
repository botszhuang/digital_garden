#LLVM_PROJECT_DIR := /workspace/llvm-bin
LLVM_PROJECT_DIR := ../../llvm-bin

LLC := $(LLVM_PROJECT_DIR)/bin/llc
CLANG := $(LLVM_PROJECT_DIR)/bin/clang

TARGET := hello
SRC    := hello.ll

all: $(TARGET)

$(TARGET): $(SRC)
	@echo "Compiling LLVM IR ($<) using built Clang..."
	$(CLANG) $(TARGET).ll -o $(TARGET)

clean:
	@echo "Cleaning up..."
	rm -f $(TARGET)

.PHONY: all clean