// 2023 - Jeremy English jhe@jeremyenglish.org
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
//
// This is a node.js addon to wrap the Sim86 shared library provided by Casey
// Muratori.  I'm sure improvements can be made to make it more
// "javascript-ty".  If you are a javascript guru please make improvements.
//
//
//-----------------------------------------------------------------------------
//
// BUILDING ON WINDOWS
//
// To build this you will need a couple of things (this only works on windows)
//
// 1. Download the sim86_shared.dll and sim86_shared.h files
// https://github.com/cmuratori/computer_enhance/tree/main/perfaware/sim86/shared
//
// 2. Place those in a folder with this file.  You will also need the following
// files in the same directory
//
//    - sim8086_addon.cc
//    - sim8086_disassemble.js
//    - sim8086_example.js
//    - sim86_shared.h
//    - package-lock.json
//    - package.json
//    - binding.gyp
//    - sim86_shared.dll
//
// 3. Start a shell that has a C++ compiler available.
//
//    I do this by running vcvarsall.bat.  This has been test with "Microsoft
//    (R) C/C++ Optimizing Compiler Version 19.29.30145 for x64"
//
// 4. You need npm and node
//
//    I've test this with npm version 6.14.14 and node version 14.17.5
//
// 5. From this directory run `npm install`
//
// 6. To test the build run `node ./`
//
// 7. For the full disassembler see sim8086_disassemble.js
//
//-----------------------------------------------------------------------------
//
// BUILDING ON LINUX
//
// 1. Build the shared library:
//
//   a. cd to `$SRC/computer_enhance/perfaware/sim86`
//
//   b. clang++ -c -fPIC -o sim86_shared.o sim86_lib.cpp
//
//   c. clang++ -shared sim86_shared.o -o libsim86.so
//
// 2. Copy the library to /usr/local/lib (gyp will look in that folder)
//
//   a. sudo cp libsim86.so /usr/local/lib/libsim86.so
//
// 3. cd shared/contrib_nodejs
//
// 4. copy sim86_shared.h to current folder
//
//   a. cp ../sim86_shared.h .
//
// 5. Run npm install (I make sure that it is using clang)
//
//   a. env V=1 CC=clang CXX=clang++ npm install
//
// 6. node ./
//
// 7. Test the disassembler
//
//   a. node ./sim8086_disassemble.js ../../../part1/listing_0042_completionist_decode
//
//-----------------------------------------------------------------------------
//
// For more information see:
//
//   https://www.computerenhance.com
//


#include <napi.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#include <stdio.h>
#include "sim86_shared.h"

#define internal static


typedef u32 (*sim86_get_version_t)(void); 
typedef void (*sim86_decode8086instruction_t)(u32 SourceSize, u8 *Source, instruction *Dest);
typedef char const * (*sim86_registernamefromoperand_t)(register_access *RegAccess);
typedef char const * (*sim86_mnemonicfromoperationtype_t)(operation_type Type);
typedef void (*sim86_get8086instructiontable_t)(instruction_table *Dest);

internal sim86_get_version_t Dll_Sim86_GetVersion;
internal sim86_decode8086instruction_t Dll_Sim86_Decode8086Instruction;
internal sim86_registernamefromoperand_t Dll_Sim86_RegisterNameFromOperand;
internal sim86_mnemonicfromoperationtype_t Dll_Sim86_MnemonicFromOperationType;
internal sim86_get8086instructiontable_t Dll_Sim86_Get8086InstructionTable;



internal void 
LoadSim86(Napi::Env env) {

#if defined(_WIN32)

  HMODULE sim86Library = LoadLibrary("sim86_shared.dll");
  if(sim86Library) {

    Dll_Sim86_GetVersion = 
      (sim86_get_version_t)GetProcAddress(sim86Library, "Sim86_GetVersion");

    Dll_Sim86_Decode8086Instruction =
      (sim86_decode8086instruction_t)GetProcAddress(sim86Library, "Sim86_Decode8086Instruction");

    Dll_Sim86_RegisterNameFromOperand =
      (sim86_registernamefromoperand_t)GetProcAddress(sim86Library, "Sim86_RegisterNameFromOperand");

    Dll_Sim86_MnemonicFromOperationType =
      (sim86_mnemonicfromoperationtype_t)GetProcAddress(sim86Library, "Sim86_MnemonicFromOperationType");

    Dll_Sim86_Get8086InstructionTable =
      (sim86_get8086instructiontable_t)GetProcAddress(sim86Library, "Sim86_Get8086InstructionTable");

  } else {
    Napi::TypeError::New(env, "Could not load sim86_shared.dll").ThrowAsJavaScriptException();
  }

#else

    Dll_Sim86_GetVersion = Sim86_GetVersion;
    Dll_Sim86_Decode8086Instruction = Sim86_Decode8086Instruction;
    Dll_Sim86_RegisterNameFromOperand = Sim86_RegisterNameFromOperand;
    Dll_Sim86_MnemonicFromOperationType = Sim86_MnemonicFromOperationType;
    Dll_Sim86_Get8086InstructionTable = Sim86_Get8086InstructionTable;

#endif
}



Napi::Value Sim86GetVersion(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (Dll_Sim86_GetVersion == nullptr){
    Napi::TypeError::New(env, "External function Sim86_GetVersion is null").ThrowAsJavaScriptException();
    return env.Null();
  }

  u32 version = Dll_Sim86_GetVersion();
  Napi::Number ver = Napi::Number::New(env, version);
  return ver;
}



Napi::Object Sim86Get8086InstructionTable(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Object result = Napi::Object::New(env);

  if (Dll_Sim86_Get8086InstructionTable == nullptr){
    Napi::TypeError::New(env, "External function Dll_Sim86_Get8086InstructionTable is null").ThrowAsJavaScriptException();
    return result;
  }

  instruction_table table;
  Dll_Sim86_Get8086InstructionTable(&table);
  result.Set("EncodingCount", table.EncodingCount);
  result.Set("MaxInstructionByteCount", table.MaxInstructionByteCount);
  
  Napi::Array napi_encoding = Napi::Array::New(env, table.EncodingCount);
  for(u32 i = 0; i < table.EncodingCount; i++){

    Napi::Object napi_ins_encoding = Napi::Object::New(env);
    napi_ins_encoding.Set("Op", Napi::Number::New(env, table.Encodings[i].Op));

    Napi::Array napi_bits = Napi::Array::New(env, 16);

    for(int j = 0; j < 16; j++){
      Napi::Object temp = Napi::Object::New(env);
      temp.Set("Usage",    Napi::Number::New(env, table.Encodings[i].Bits[j].Usage));
      temp.Set("BitCount", table.Encodings[i].Bits[j].BitCount);
      temp.Set("Shift",    table.Encodings[i].Bits[j].Shift);
      temp.Set("Value",    table.Encodings[i].Bits[j].Value);
      napi_bits[j] = temp;
    }

    napi_ins_encoding.Set("Bits", napi_bits);
    napi_encoding[i] = napi_ins_encoding;
  }

  result.Set("Encodings", napi_encoding);

  return result;
}



Napi::String Sim86GetMnemonicFromOperationType(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (Dll_Sim86_MnemonicFromOperationType == nullptr){
    Napi::TypeError::New(env, "External function Dll_Sim86_MnemonicFromOperationType is null").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  if (info.Length() != 1){
    Napi::TypeError::New(env, "Wrong number of arguments. Expecting operation_type").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  if (!info[0].IsNumber()){
    Napi::TypeError::New(env, "Wrong argument. Expecting operation_type").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  //TODO I need to do some bounds checking on this or convert from string to enum
  operation_type type = (operation_type)info[0].As<Napi::Number>().Int32Value();

  const char *mnemonic = Dll_Sim86_MnemonicFromOperationType(type);
  Napi::String ver = Napi::String::New(env, mnemonic);
  return ver;
}



Napi::String Sim86GetRegisterNameFromOperand(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (Dll_Sim86_RegisterNameFromOperand == nullptr){
    Napi::TypeError::New(env, "External function Dll_Sim86_RegisterNameFromOperand is null").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  if (info.Length() != 1){
    Napi::TypeError::New(env, "Wrong number of arguments. Expecting register_access").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  if (!info[0].IsObject()){
    Napi::TypeError::New(env, "Wrong argument. Expecting register_access").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  Napi::Object napi_regAccess = info[0].As<Napi::Object>();
  if (!(napi_regAccess.Has("Index") && napi_regAccess.Has("Offset") && napi_regAccess.Has("Count"))){
    Napi::TypeError::New(env, "Wrong argument. Expecting register_access").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  register_access regAccess;
  regAccess.Index = napi_regAccess.Get("Index").As<Napi::Number>().Uint32Value();
  regAccess.Offset = napi_regAccess.Get("Offset").As<Napi::Number>().Uint32Value();
  regAccess.Count = napi_regAccess.Get("Count").As<Napi::Number>().Uint32Value();

  const char *registerName = Dll_Sim86_RegisterNameFromOperand(&regAccess);
  Napi::String result = Napi::String::New(env, registerName);
  return result;
}



Napi::Object Sim86Decode8086Instruction(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Object result = Napi::Object::New(env);

  if (Dll_Sim86_Decode8086Instruction == nullptr){
    Napi::TypeError::New(env, "External function Dll_Sim86_Decode8086Instruction is null").ThrowAsJavaScriptException();
    return result;
  }

  if (info.Length() != 1){
    Napi::TypeError::New(env, "Wrong number of arguments. Expecting Source").ThrowAsJavaScriptException();
    return result;
  }

  if (!info[0].IsArray()){
    Napi::TypeError::New(env, "Wrong argument. Expecting source array of bytes").ThrowAsJavaScriptException();
    return result;
  }

  Napi::Array napi_source = info[0].As<Napi::Array>();

  instruction dest;

  //TODO Do I want to malloc here?  Example code uses a unique pointer to a
  //vector

  u8 *source = (u8*)calloc(napi_source.Length(), sizeof(u8));

  for(int i = 0; i < napi_source.Length(); i++){
    Napi::Value v = napi_source[i];
    if (!v.IsNumber()){
      Napi::Error::New(info.Env(), "Expected a Number as Array Item")
          .ThrowAsJavaScriptException();
      return result;
    } 

    u32 n = v.As<Napi::Number>().Uint32Value();
    if (0 <= n && n <= 255){
      source[i] = (u8)n;
    } else {
      Napi::Error::New(info.Env(),
                       "Array Item Number value is out of range [0..255]")
          .ThrowAsJavaScriptException();
      return result;
    }
  }

  Dll_Sim86_Decode8086Instruction(napi_source.Length(), source, &dest);

  free(source);

  result.Set("Address", dest.Address);
  result.Set("Size", dest.Size);
  result.Set("Op", Napi::Number::New(env, dest.Op));
  result.Set("Flags", dest.Flags);
  result.Set("SegmentOverride", dest.SegmentOverride);

  Napi::Array operands = Napi::Array::New(env, 2);
  for(int i = 0; i < 2; i++){

    Napi::Object insOp = Napi::Object::New(env);
    insOp.Set("Type", Napi::Number::New(env, dest.Operands[i].Type));

    if (dest.Operands[i].Type == Operand_Register){

      Napi::Object reg = Napi::Object::New(env); 
      reg.Set("Index", dest.Operands[i].Register.Index);
      reg.Set("Offset", dest.Operands[i].Register.Offset);
      reg.Set("Count", dest.Operands[i].Register.Count);
      insOp.Set("Register", reg);

    } else if (dest.Operands[i].Type == Operand_Memory){

      Napi::Object addr = Napi::Object::New(env);
      addr.Set("ExplicitSegment", dest.Operands[i].Address.ExplicitSegment); 
      addr.Set("Displacement", dest.Operands[i].Address.Displacement); 
      addr.Set("Flags", dest.Operands[i].Address.Flags); 
      Napi::Array terms = Napi::Array::New(env, 2);
      for(int j = 0; j < 2; j++){
        Napi::Object term = Napi::Object::New(env);
        Napi::Object term_reg = Napi::Object::New(env);
        term.Set("Scale", dest.Operands[i].Address.Terms[j].Scale);
        term_reg.Set("Index", dest.Operands[i].Address.Terms[j].Register.Index);
        term_reg.Set("Offset", dest.Operands[i].Address.Terms[j].Register.Offset);
        term_reg.Set("Count", dest.Operands[i].Address.Terms[j].Register.Count);
        term.Set("Register", term_reg);
        terms[j] = term;
      }
      addr.Set("Terms", terms);
      insOp.Set("Address", addr);

    } else if (dest.Operands[i].Type == Operand_Immediate){
    
      Napi::Object imm = Napi::Object::New(env);
      imm.Set("Value", dest.Operands[i].Immediate.Value);
      imm.Set("Flags", dest.Operands[i].Immediate.Flags);
      insOp.Set("Immediate", imm);
    }
  
    operands[i] = insOp;
  }

  result.Set("Operands", operands);

  return result;
}



Napi::Object Init(Napi::Env env, Napi::Object exports) {
  LoadSim86(env);
  //TODO Where is the destructor?
  //else { FreeLibrary((HMODULE)h);

  exports.Set(Napi::String::New(env, "getVersion"),
      Napi::Function::New(env, Sim86GetVersion));

  exports.Set(Napi::String::New(env, "get8086InstructionTable"),
      Napi::Function::New(env, Sim86Get8086InstructionTable));

  exports.Set(Napi::String::New(env, "getMnemonicFromOperationType"),
      Napi::Function::New(env, Sim86GetMnemonicFromOperationType));

  exports.Set(Napi::String::New(env, "getRegisterNameFromOperand"),
      Napi::Function::New(env, Sim86GetRegisterNameFromOperand));

  exports.Set(Napi::String::New(env, "decode8086Instruction"),
      Napi::Function::New(env, Sim86Decode8086Instruction));


  return exports;
}

NODE_API_MODULE(sim8086, Init)
