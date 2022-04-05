// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.Intrinsics;

namespace System.Runtime.Intrinsics.X86
{
    /// <summary>
    /// This class provides access to Intel AVX2 hardware instructions via intrinsics
    /// </summary>
    [CLSCompliant(false)]
    public abstract class Avx512 : Avx
    {
        internal Avx2() { }

        public static new bool IsSupported { [Intrinsic] get { return false; } }

        public new abstract class X64 : Avx.X64
        {
            internal X64() { }

            public static new bool IsSupported { [Intrinsic] get { return false; } }
        }

        public static Vector512<int> Add(Vector512<int> left, Vector512<int> right) { throw new PlatformNotSupportedException(); }
        public static Vector512<int> Add(Vector512<float> left, Vector512<float> right) { throw new PlatformNotSupportedException(); }

        public static Vector128<int> MaskAdd128(Vector128<int> left, Vector128<int> right, KMask<ushort> mask) { throw new PlatformNotSupportedException(); }
        public static Vector256<int> MaskAdd256(Vector256<int> left, Vector256<int> right, KMask<ushort> mask) { throw new PlatformNotSupportedException(); }
        public static Vector512<int> MaskAdd512(Vector512<int> left, Vector512<int> right, KMask<ushort> mask) { throw new PlatformNotSupportedException(); }

        public static Vector128<float> ReciprocalSqrt128(Vector128<float> vals) { throw new PlatformNotSupportedException(); }
        public static Vector256<float> ReciprocalSqrt256(Vector256<float> vals) { throw new PlatformNotSupportedException(); }
        public static Vector512<float> ReciprocalSqrt512(Vector512<float> vals) { throw new PlatformNotSupportedException(); }
    }
}
