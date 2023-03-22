async function loadLibrary() {
  const wasmMemory = new WebAssembly.Memory({ initial: 160 });
  const view = new Uint8Array(wasmMemory.buffer);
  const importObject = {
    env: {
      memory: wasmMemory
    }
  };

  const { instance } = await WebAssembly.instantiateStreaming(
    fetch("./sim8086.wasm"), importObject
  );

  const MaxMemory = BigInt(view.length - instance.exports.__heap_base);

  function Sim86_GetVersion() {
    return instance.exports.Sim86_GetVersion();
  }

  function Sim86_Decode8086Instruction(sourceBytes) {
    // encode
    if (!(sourceBytes instanceof Uint8Array)) {
      throw new Error("sourceBytes must be provided as a Uint8Array");
    }

    for (let i = 0; i < sourceBytes.length; ++i) {
      view[instance.exports.__heap_base + i] = sourceBytes[i];
    }

    const offset = instance.exports.__heap_base;
    const resultOffset = offset + (1 << 20);

    // call
    instance.exports.Sim86_Decode8086Instruction(sourceBytes.length, offset, resultOffset);

    // decode
    const wordExtractor = new Uint32Array(view.buffer, resultOffset);

    const result = {
      Address: wordExtractor[0], // u32
      Size: wordExtractor[1], //u32
      Op: wordExtractor[2],
      Flags: wordExtractor[3], //u32
      Operands: [
        {
          Type: 0, // 0: None, 1: Register, 2: Memory, 3: Immediate
          Immediate: null,
          Register: null,
          Address: null
        }, 
        {
          Type: 0,
          Immediate: null,
          Register: null,
          Address: null
        }
      ],
      SegmentOverride: null
    };

    let nextWordIndex = 4;
    for (let i = 0; i < 2; ++i) {
      const Operand = result.Operands[i];
      const OperandType = wordExtractor[nextWordIndex++];
      Operand.Type = OperandType;
      if (OperandType == 1) {
        Operand.Register = {
          Index: wordExtractor[nextWordIndex++],
          Offset: wordExtractor[nextWordIndex++],
          Count: wordExtractor[nextWordIndex++],
        };
        nextWordIndex += 8;
      } else if (OperandType == 2) {
        Operand.Address = {
          ExplicitSegment: wordExtractor[nextWordIndex++],
          Displacement: wordExtractor[nextWordIndex++],
          Flags: wordExtractor[nextWordIndex++],
          Terms: [
            {
              Scale: wordExtractor[nextWordIndex++],
              Register: {
                Index: wordExtractor[nextWordIndex++],
                Offset: wordExtractor[nextWordIndex++],
                Count: wordExtractor[nextWordIndex++],
              }
            },
            {
              Scale: wordExtractor[nextWordIndex++],
              Register: {
                Index: wordExtractor[nextWordIndex++],
                Offset: wordExtractor[nextWordIndex++],
                Count: wordExtractor[nextWordIndex++],
              }
            }
          ]
        };
      } else if (OperandType == 3) {
        Operand.Immediate = {
          Value: wordExtractor[nextWordIndex++],
          Flags: wordExtractor[nextWordIndex++],
        };
        nextWordIndex += 9;
      }
    }

    result.SegmentOverride = wordExtractor[nextWordIndex];
    return result;
  }

  function readStringFromOffset(view, offset) {
    const result = String.fromCharCode.apply(null, 
      view.slice(offset, view.indexOf(0, offset))
    );

    return result;
  }

  function Sim86_RegisterNameFromOperand({ Index, Offset, Count }) {
    const wordView = new Uint32Array(view.buffer, instance.exports.__heap_base);
    wordView[0] = Index;
    wordView[1] = Offset;
    wordView[2] = Count;
    const resultOffset = instance.exports.Sim86_RegisterNameFromOperand(
      instance.exports.__heap_base
    );

    const result = readStringFromOffset(view, resultOffset);
    return result;
  }

  function Sim86_MnemonicFromOperationType(Type) {
    const resultOffset = instance.exports.Sim86_MnemonicFromOperationType(Type);   

    const result = readStringFromOffset(view, resultOffset);
    return result;
  }

  function Sim86_Get8086InstructionTable() {
    instance.exports.Sim86_Get8086InstructionTable(instance.exports.__heap_base);		
    const wordView = new Uint32Array(view.buffer, instance.exports.__heap_base);
    const EncodingsOffset = wordView[0];
    const EncodingCount = wordView[1];
    const MaxInstructionByteCount = wordView[2];

    const getEncoding = function (index) {
      const Encoding = new Uint8Array(this.Encodings.buffer, index * (4 + 64));
      const result = {
        Op: Encoding[0],
        Bits: new Array(16)
      };
      
      let BitsUsageIndex = (index * (4 + 64)) + 4;
      for (let i = 0; i < 16; ++i) {
        result.Bits[i] = {
          Usage: Encoding[BitsUsageIndex++],
          BitCount: Encoding[BitsUsageIndex++],
          Shift: Encoding[BitsUsageIndex++],
          Value: Encoding[BitsUsageIndex++]
        };
      }

      return result;
    }

    return {
      Encodings: view.slice(EncodingsOffset, EncodingsOffset + (EncodingCount * (4 + 16 * 4))),
      EncodingCount,
      MaxInstructionByteCount,
      getEncoding
    };
  }

  return {
    Sim86_GetVersion,
    Sim86_Decode8086Instruction,
    Sim86_RegisterNameFromOperand,
    Sim86_MnemonicFromOperationType,
    Sim86_Get8086InstructionTable,
  };
}

