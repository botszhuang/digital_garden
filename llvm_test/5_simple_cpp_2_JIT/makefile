#llvm_dir := /workspace/llvm-bin
llvm_dir := ../../llvm-bin

CLANG := $(llvm_dir)/bin/clang++
LLVM_CONFIG := $(llvm_dir)/bin/llvm-config

TARGET := hello
SRC    := hello.cpp

CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags)
LDFLAGS  := $(shell $(LLVM_CONFIG) --ldflags --libs core support orcjit native --system-libs)
# --libs all linke in all llvm libraries
#LDFLAGS  := $(shell $(LLVM_CONFIG) --ldflags --libs all --system-libs)
#orcjit native: This explicitly pulls in the core engine, the LLJIT components (orcjit), 
#and the specific backend components for the machine you are currently compiling on (native, which resolves your X86 errors).

all: $(TARGET)

$(TARGET): $(SRC)
	@echo "Compiling LLVM API Code ($<) using built Clang..."
	$(CLANG) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	@echo "Cleaning up..."
	rm -f $(TARGET)

.PHONY: all clean