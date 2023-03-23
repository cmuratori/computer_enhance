// 2023 - Jeremy English jhe@jeremyenglish.org
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
//
// This is an implementation of Casey Muratori's 8086 disassembler from
// Computer Enhance.  
//
//   https://www.computerenhance.com
//
//
// The is a demonstration of using the node.js wrapper for the shared library
// that is provided with the course.
//
// See sim8086_addon.cc for the node addon build instructions.  Once the
// node.js addon is installed run this script from the command line:
//
//   node.js sim8086_diassemble.js listing_0042_completionist_decode >
//   output.asm
//
//   It also can take a "-v" argument to set the verbosity.  This will print
//   details about each of the operands in the byte stream.  You will not be
//   able to assemble the output when using the "-v" argument
//
//
// For further information on the original C++ implementation see:
//
//   https://github.com/cmuratori/computer_enhance/blob/main/perfaware/sim86/sim86_text.cpp
// 
// 


const isRegister  = 1;
const isAddress   = 2;
const isImmediate = 3;

const instLock               = 0x1;
const instRep                = 0x2;
const instWide               = 0x8;
const instSegment            = 0x4;
const instFar                = 0x10;

const addressExplicitSegment            = 0x1;
const immediateRelativeJumpDisplacement = 0x1;

if (process.argv.length < 3){
    console.log("usage: node sim8086_disassemble.js <filename> [-v]");
    console.log("\t-v verbose");
    return;
}

let verbose = false;
let filename = "";
for(let i = 2; i < process.argv.length; i++){
    if (process.argv[i] == "-v")
        verbose = true;
    else
        filename = process.argv[i];            
}

vlog(filename);

let fs = require('fs');
let path = require('path');    
let sim86 = require('bindings')('sim8086');

let buf;

fs.stat(filename, function(err, stats){
    vlog("file size", stats.size);
    buf = Buffer.alloc(stats.size);
});

fs.open(filename, 'r', function(err, fd){
    fs.read(fd, buf, 0, buf.length,
        0, function(err, bytes){
            if (err) {
                console.log(err);
            } else {
                //wtf?
                let data = [];
                for(let i = 0; i < bytes; i++){
                    data[i] = buf.readUInt8(i);
                }
                disassemble(data);
            }
    });
});

function vlog(...s){
    if (verbose){
        s.unshift("info:");
        console.log(...s);
    }
}

function disassemble(data){
   console.log("bits 16");
    let offset = 0;
    while (offset < data.length){
        let instruction = sim86.decode8086Instruction(data.slice(offset));
        vlog("Instruction", instruction);

        if (instruction.Type != 0){
            offset += instruction.Size;
            console.log(disassembleInstruction(instruction));
        } else {
            console.log("Unrecognized instruction") 
        }

        vlog("\n\n");
    }
}

function disassembleInstruction(instruction){
    let op = sim86.getMnemonicFromOperationType(instruction.Op);
    let args = [];
    let result = "";
    let w = instruction.Flags & instWide;

    if (instruction.Flags & instLock){
        if (op == "xchg") {
            let temp = instruction.Operands[0];
            instruction.Operands[0] = instruction.Operands[1];
            instruction.Operands[1] = temp;
        }
        result += "lock ";
    }

    let suffix = "";
    if (instruction.Flags & instRep){
        result += "rep ";
        suffix += w ? "w" : "b";
    }

    for (let i = 0; i < instruction.Operands.length; i++){
        if (instruction.Operands[i].Type == isRegister){
            vlog("\t", "Register", instruction.Operands[i].Register);
            args.push(sim86.getRegisterNameFromOperand(instruction.Operands[i].Register));
        } else if (instruction.Operands[i].Type == isAddress){
            vlog("\t", "Address", instruction.Operands[i].Address);
            args.push(getAddressDetails(instruction.Operands[i].Address, instruction));
        } else if (instruction.Operands[i].Type == isImmediate){
            vlog("\t", "Immediate", instruction.Operands[i].Immediate);
            let immediate = instruction.Operands[i].Immediate;
            if (immediate.Flags & immediateRelativeJumpDisplacement){
                let val = (immediate.Value + instruction.Size);
                let prefix = "$";

                if (val >= 0)
                    prefix += "+";
                
                args.push(prefix + val);
            } else {
                args.push(instruction.Operands[i].Immediate.Value);
            }
        }
    }

    result += op + suffix + " " + args.join(",");
    vlog(result);

    return result;
}

function getEffectiveAddress(address){
    details = []

    for(let j = 0; j < address.Terms.length; j++){
        let term = address.Terms[j];
        if (term){
            vlog("\t\t", "Term", term);
            reg = sim86.getRegisterNameFromOperand(term.Register);
            if (reg !== "")
                details.push(reg);
        }
    }

    if (address.Displacement != 0){
        details.push(address.Displacement);
    }

    return details;
}

function getAddressDetails(address, instruction){
    let result = "";
    let w = instruction.Flags & instWide;

    if (instruction.Flags & instFar){
        result += "far ";
    }

    if (address.Flags & addressExplicitSegment){
        result += address.ExplicitSegment + ":" + address.Displacement;
    } else {

        if (instruction.Operands[0].Type != isRegister){
            result += w ? "word " : "byte ";            
        }

        if (instruction.Flags & instSegment){
            let segReg = {"Index": instruction.SegmentOverride, "Offset": 0, "Count": 2};
            let reg = sim86.getRegisterNameFromOperand(segReg);
            result += reg + ":";
        }

        result += "[" + getEffectiveAddress(address).join("+") + "]";
    }
    return result.replace("+-", "-");
}
