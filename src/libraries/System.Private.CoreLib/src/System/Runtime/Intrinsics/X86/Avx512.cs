// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.CompilerServices;

namespace System.Runtime.Intrinsics.X86
{
    /// <summary>
    /// This class provides access to Intel AVX2 hardware instructions via intrinsics
    /// </summary>
    [Intrinsic]
    [CLSCompliant(false)]
    public abstract class Avx512 : Avx
    {
        internal Avx512() { }

        public static new bool IsSupported { get => IsSupported; }

        [Intrinsic]
        public new abstract class X64 : Avx.X64
        {
            internal X64() { }

            public static new bool IsSupported { get => IsSupported; }
        }

        public static Vector512<int> Add(Vector512<int> left, Vector512<int> right) => Add(left, right);
        public static Vector512<float> Add(Vector512<float> left, Vector512<float> right) => Add(left, right);

        public static Vector128<float> ReciprocalSqrt128(Vector128<float> vals) => ReciprocalSqrt128(vals);
        public static Vector256<float> ReciprocalSqrt256(Vector256<float> vals) => ReciprocalSqrt256(vals);
        public static Vector512<float> ReciprocalSqrt512(Vector512<float> vals) => ReciprocalSqrt512(vals);
    }
}
