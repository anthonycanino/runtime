// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
//
// #JITEEVersionIdentifier
//
// This GUID represents the version of the JIT/EE interface. Any time the interface between the JIT and
// the EE changes (by adding or removing methods to any interface shared between them), this GUID should
// be changed. This is the identifier verified by ICorJitCompiler::getVersionIdentifier().
//
// You can use src/coreclr/tools/Common/JitInterface/ThunkGenerator/gen.bat (or .sh on Unix) to update this file.
//
// Note that this file is parsed by some tools, namely superpmi.py, so make sure the first line is exactly
// of the form:
//
//   constexpr GUID JITEEVersionIdentifier = { /* 1776ab48-edfa-49be-a11f-ec216b28174c */
//
// (without the leading slashes or spaces).
//
// See docs/project/updating-jitinterface.md for details
//
// **** NOTE TO INTEGRATORS:
//
// If there is a merge conflict here, because the version changed in two different places, you must
// create a **NEW** GUID, not simply choose one or the other!
//
// NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#ifndef JIT_EE_VERSIONING_GUID_H
#define JIT_EE_VERSIONING_GUID_H

<<<<<<< HEAD
#include <minipal/guid.h>

constexpr GUID JITEEVersionIdentifier = { /* f4059fe5-346c-4294-981d-a81b06f9f402 */
    0xf4059fe5,
    0x346c,
    0x4294,
    {0x98, 0x1d, 0xa8, 0x1b, 0x06, 0xf9, 0xf4, 0x02}
=======
constexpr GUID JITEEVersionIdentifier = { /* 6e4908f2-5bed-45f3-8ca0-1a47a8d102b9 */
    0x6e4908f2,
    0x5bed,
    0x45f3,
    {0x8c, 0xa0, 0x1a, 0x47, 0xa8, 0xd1, 0x02, 0xb9}
>>>>>>> 3a7b0284e7e (Hacking in a Half/AVX512FP16 ISA and intrinsic.)
  };

#endif // JIT_EE_VERSIONING_GUID_H
