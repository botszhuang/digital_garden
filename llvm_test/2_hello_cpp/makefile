#llvm_dir := /workspace/llvm-bin
llvm_dir := ../../llvm-bin

CLANG := $(llvm_dir)/bin/clang++
LLVM_CONFIG := $(llvm_dir)/bin/llvm-config

TARGET := hello
SRC    := hello.cpp

CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags)
LDFLAGS  := $(shell $(LLVM_CONFIG) --ldflags --libs core support --system-libs)

all: $(TARGET)

$(TARGET): $(SRC)
	@echo "Compiling LLVM API Code ($<) using built Clang..."
	$(CLANG) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	@echo "Cleaning up..."
	rm -f $(TARGET)

.PHONY: all clean