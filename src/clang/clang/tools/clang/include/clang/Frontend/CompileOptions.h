//===--- CompileOptions.h ---------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the CompileOptions interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_FRONTEND_COMPILEOPTIONS_H
#define LLVM_CLANG_FRONTEND_COMPILEOPTIONS_H

#include <string>
#include <vector>

namespace clang {

/// CompileOptions - Track various options which control how the code
/// is optimized and passed to the backend.
class CompileOptions {
public:
  unsigned OptimizationLevel : 3; /// The -O[0-4] option specified.
  unsigned OptimizeSize      : 1; /// If -Os is specified.
  unsigned DebugInfo         : 1; /// Should generate deubg info (-g).
  unsigned UnitAtATime       : 1; /// Unused. For mirroring GCC
                                  /// optimization selection.
  unsigned InlineFunctions   : 1; /// Should functions be inlined?
  unsigned SimplifyLibCalls  : 1; /// Should standard library calls be
                                  /// treated specially.
  unsigned UnrollLoops       : 1; /// Control whether loops are unrolled.
  unsigned VerifyModule      : 1; /// Control whether the module
                                  /// should be run through the LLVM Verifier.
  unsigned TimePasses        : 1; /// Set when -ftime-report is enabled.
  unsigned NoCommon          : 1; /// Set when -fno-common or C++ is enabled.

  /// CPU - An optional CPU to target.
  std::string CPU;

  /// Features - A list of subtarget features to pass to the code
  /// generator.
  std::vector<std::string> Features;

public:
  CompileOptions() {
    OptimizationLevel = 0;
    OptimizeSize = 0;
    DebugInfo = 0;
    UnitAtATime = 1;
    InlineFunctions = SimplifyLibCalls = UnrollLoops = 0;
    VerifyModule = 1;
    TimePasses = 0;
    NoCommon = 0;
  }  
};

}  // end namespace clang

#endif
