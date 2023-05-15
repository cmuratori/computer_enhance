clang++ -g --target=wasm32 -mbulk-memory -nostartfiles --no-standard-libraries -Wl,--import-memory -Wl,--export-all -Wl,--no-entry -o sim8086.wasm sim8086.cpp

clang++ -g --target=wasm32 -mbulk-memory -nostartfiles --no-standard-libraries -Wl,--import-memory -Wl,--export-all -Wl,--no-entry -o sim8086-test.wasm sim8086_test.cpp
