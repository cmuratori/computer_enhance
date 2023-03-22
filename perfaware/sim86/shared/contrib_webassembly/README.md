# Webassembly Bindings

To use the javascript wrapper add `lib.js` to your page. Make sure that `sim8086.wasm`
is also reachable at the same relative path as `./sim8086.wasm`.

```
<script src="./lib.js" async></script>
```

If you wish to use the C++ code in your own webassembly module, simply modify `contrib_webassembly/sim8086.cpp` and use `contrib_webassembly/build.bat` to build. 

For an example on how to run the webassembly module check `contrib_webassembly/index.html`.  

If you want to run it locally, go to the `contrib_webassembly` directory and use `live-server` by running `npx live-server` (this assumes you have nodejs installed on your system). After this any changes you make to `main.js`, `lib.js` or `sim8086.cpp` (and then run `build.bat`) will live reload your page at `localhost:8080/index.html`. Alternatively use any static file server to serve from the `contrib_webassembly` folder.

Check a deployed version [here](https://gautam1168.github.io/Part4-8086/index.html)

## Building
If you have clang installed you can just run `build.bat` on a windows machine to build the `sim8086.wasm`. The command inside `build.bat` will also work in a `.sh` file if you copy it there. Make sure you have the latest version of clang installed.

If you want to just get a built version of `sim8086.wasm` you can download it from [here](https://gautam1168.github.io/Part4-8086/sim8086.wasm). Linking to this directly in your `index.html` may or may not work since it is a github page and I cannot control cors headers. 

## Functions provided through lib.js
The interface of the shared library is exposed as javascript functions as follows:
1. `Sim86_GetVersion(): number`- Returns version from the compiled c++ library
2. `Sim86_Decode8086Instruction(bytes: Uint8Array): Instruction` - Returns the first instruction from the bytes provided as argument
3. `Sim86_RegisterNameFromOperand(registerAccess: { Index: number; Offset: number; Count: number}): string` - Returns the register name as a string
4. `Sim86_MnemonicFromOperationType(type: number): string` - Returns the mnemonic for an Op 
5. `Sim86_Get8086InstructionTable(): { Encodings: Uint32Array; EncodingCount: number; MaxInstructionBytecount: number; getEncoding: (index) => Instruction }` - Returns a wrapper around the instruction table used in the simulator

You can run these in the console of [this page](https://gautam1168.github.io/Part4-8086/index.html) if you want to check that its working correctly.
```
window.clib.Sim86_GetVersion()

window.clib.Sim86_Decode8086Instruction(new Uint8Array([0b10001001, 0b11011110]))

window.clib.Sim86_RegisterNameFromOperand({ Count: 2, Index: 7, Offset: 0 })

window.clib.Sim86_MnemonicFromOperationType(1)

window.clib.Sim86_Get8086InstructionTable()
```
