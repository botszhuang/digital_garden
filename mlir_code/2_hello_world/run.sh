#!/bin/bash


llvm_file="sample_llvm"
c_file="sample_c"

mkdir -p out

clang -S -emit-llvm -c ${c_file}.c -o out/${c_file}.ll

llc -filetype=obj --relocation-model=pic ${llvm_file}.ll   -o out/${llvm_file}.o
llc -filetype=obj --relocation-model=pic out/${c_file}.ll  -o out/${c_file}.o

clang -shared -fPIC out/${llvm_file}.o -o out/lib${llvm_file}.so
clang -shared -fPIC out/${c_file}.o    -o out/lib${c_file}.so

clang main.c -Lout -lsample_c -lsample_llvm -Wl,-rpath,./out/ -o out/main.exe

echo "Running main.exe..."
./out/main.exe

echo -e "\nRunning with python..."
python3 run.py

echo -e "\nDone."



