// ============================================================================
// NEGATIVE CONTROL for compile_gate.py's tier 2.
//
// This file MUST fail to compile. If it compiles clean, the gate is not
// running the compiler and every green verdict it has ever produced is
// vacuous — which is exactly what happened (tale §977) and went unnoticed
// because a vacuous PASS is indistinguishable from a real PASS.
//
// Do not "fix" this file. Its failure IS the test.
// ============================================================================
#include <cstddef>

static_assert(sizeof(int) == 1,
              "NEGATIVE CONTROL: this assert MUST fail. If you are reading a "
              "clean compile of this file, the gate is not compiling anything.");
