## Fuzzing

This fuzzer should reach a reasonable coverage (>85%) of `essence.c` with no test cases to start with and in a very short time, but this could always be improved by starting from a real testcase.

### On Linux:

- `cmake -Bbuild -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++`

Fuzzing (from `./build/`):

- `./essence_fuzzer ../corpus/`

Running coverage:

- `./essence_fuzzer_coverage ../corpus/*`

### On Windows:

- `cmake -Bbuild -GNinja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++`

Fuzzing (from `./build/`):

- `.\essence_fuzzer.exe ../corpus/`

Running coverage:
- `.\essence_fuzzer_coverage.exe $(ls ../corpus/* | % {$_.FullName})`

## Monitoring coverage

```
llvm-profdata merge -sparse *.profraw -o default.profdata 
llvm-cov report essence_fuzzer_coverage -instr-profile="default.profdata"
llvm-cov show essence_fuzzer_coverage -instr-profile="default.profdata" --format=html > report.html
```

Will output a file `report.html` containing coverage information by line in the source file. 
