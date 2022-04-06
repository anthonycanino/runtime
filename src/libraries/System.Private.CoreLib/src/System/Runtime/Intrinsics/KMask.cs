// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Diagnostics;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics.X86;

namespace System.Runtime.Intrinsics
{

    public static class KMask
    {
        internal const int Size = 8;

        /// <summary>Gets a value that indicates whether 512-bit vector operations are subject to hardware acceleration through JIT intrinsic support.</summary>
        /// <value><see langword="true" /> if 512-bit vector operations are subject to hardware acceleration; otherwise, <see langword="false" />.</value>
        /// <remarks>512-bit vector operations are subject to hardware acceleration on systems that support Single Instruction, Multiple Data (SIMD) instructions for 512-bit vectors and the RyuJIT just-in-time compiler is used to compile managed code.</remarks>
        public static bool IsHardwareAccelerated
        {
            [Intrinsic]
            get => false;
        }

        /// <summary>Creates a new <see cref="KMask{T}" /> instance with all elements initialized to the specified value.</summary>
        /// <param name="value">The value that all elements will be initialized to.</param>
        /// <typeparam name="T">The type of the elements in the vector.</typeparam>
        /// <returns>A new <see cref="KMask{T}" /> with all elements initialized to <paramref name="value" />.</returns>
        [Intrinsic]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static unsafe KMask<T> Create<T>(T value)
            where T : struct
        {
            if (typeof(T) == typeof(byte))
            {
                return Create((byte)(object)value).As<byte, T>();
            }
            else if (typeof(T) == typeof(ushort))
            {
                return Create((ushort)(object)value).As<ushort, T>();
            }
            else if (typeof(T) == typeof(uint))
            {
                return Create((uint)(object)value).As<uint, T>();
            }
            else if (typeof(T) == typeof(ulong))
            {
                return Create((ulong)(object)value).As<ulong, T>();
            }
            else
            {
                throw new NotSupportedException(SR.Arg_TypeNotSupported);
            }
        }

        /// <summary>Creates a new <see cref="KMask{Byte}" /> instance with all elements initialized to the specified value.</summary>
        /// <param name="value">The value that all elements will be initialized to.</param>
        /// <remarks>On x86, this method corresponds to __m512i _mm512_set1_epi8</remarks>
        /// <returns>A new <see cref="KMask{Byte}" /> with all elements initialized to <paramref name="value" />.</returns>
        [Intrinsic]
        public static unsafe KMask<byte> Create(byte value)
        {
            throw new Exception("not yet impl");
            /*
            if (Avx.IsSupported)
            {
                return Create(value);
            }

            throw new Exception("not yet impl");

            return SoftwareFallback(value);

            static KMask<byte> SoftwareFallback(byte value)
            {
                byte* pResult = stackalloc byte[1]
                {
                    value,
                };

                return Unsafe.AsRef<KMask<byte>>(pResult);
            }
            */
        }

        /// <summary>Creates a new <see cref="KMask{UInt16}" /> instance with all elements initialized to the specified value.</summary>
        /// <param name="value">The value that all elements will be initialized to.</param>
        /// <remarks>On x86, this method corresponds to __m512i _mm512_set1_epi16</remarks>
        /// <returns>A new <see cref="KMask{UInt16}" /> with all elements initialized to <paramref name="value" />.</returns>
        [Intrinsic]
        [CLSCompliant(false)]
        public static unsafe KMask<ushort> Create(ushort value)
        {
            throw new Exception("not yet impl");
            /*
            if (Avx.IsSupported)
            {
                return Create(value);
            }

            throw new Exception("not yet impl");

            return SoftwareFallback(value);

            static KMask<ushort> SoftwareFallback(ushort value)
            {
                ushort* pResult = stackalloc ushort[1]
                {
                    value,
                };

                return Unsafe.AsRef<KMask<ushort>>(pResult);
            }
            */
        }

        /// <summary>Creates a new <see cref="KMask{UInt32}" /> instance with all elements initialized to the specified value.</summary>
        /// <param name="value">The value that all elements will be initialized to.</param>
        /// <remarks>On x86, this method corresponds to __m512i _mm512_set1_epi32</remarks>
        /// <returns>A new <see cref="KMask{UInt32}" /> with all elements initialized to <paramref name="value" />.</returns>
        [Intrinsic]
        [CLSCompliant(false)]
        public static unsafe KMask<uint> Create(uint value)
        {
            throw new Exception("not yet impl");
            /*
            if (Avx.IsSupported)
            {
                return Create(value);
            }

            throw new Exception("not yet impl");

            /*
            return SoftwareFallback(value);

            static KMask<uint> SoftwareFallback(uint value)
            {
                uint* pResult = stackalloc uint[1]
                {
                    value,
                };

                return Unsafe.AsRef<KMask<uint>>(pResult);
            }
            */
        }

        /// <summary>Creates a new <see cref="KMask{UInt64}" /> instance with all elements initialized to the specified value.</summary>
        /// <param name="value">The value that all elements will be initialized to.</param>
        /// <remarks>On x86, this method corresponds to __m512i _mm512_set1_epi64x</remarks>
        /// <returns>A new <see cref="KMask{UInt64}" /> with all elements initialized to <paramref name="value" />.</returns>
        [Intrinsic]
        [CLSCompliant(false)]
        public static unsafe KMask<ulong> Create(ulong value)
        {
            throw new Exception("not yet impl");
            /*
            if (Sse2.X64.IsSupported && Avx.IsSupported)
            {
                return Create(value);
            }

            return SoftwareFallback(value);

            static KMask<ulong> SoftwareFallback(ulong value)
            {
                ulong* pResult = stackalloc ulong[1]
                {
                    value,
                };

                return Unsafe.AsRef<KMask<ulong>>(pResult);
            }
            */
        }

        /// <summary>Reinterprets a <see cref="KMask{T}" /> as a new <see cref="KMask{U}" />.</summary>
        /// <typeparam name="TFrom">The type of the input vector.</typeparam>
        /// <typeparam name="TTo">The type of the vector <paramref name="vector" /> should be reinterpreted as.</typeparam>
        /// <param name="vector">The vector to reinterpret.</param>
        /// <returns><paramref name="vector" /> reinterpreted as a new <see cref="KMask{U}" />.</returns>
        /// <exception cref="NotSupportedException">The type of <paramref name="vector" /> (<typeparamref name="TFrom" />) or the type of the target (<typeparamref name="TTo" />) is not supported.</exception>
        [Intrinsic]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static KMask<TTo> As<TFrom, TTo>(this KMask<TFrom> vector)
            where TFrom : struct
            where TTo : struct
        {
            ThrowHelper.ThrowForUnsupportedIntrinsicsKMaskBaseType<TFrom>();
            ThrowHelper.ThrowForUnsupportedIntrinsicsKMaskBaseType<TTo>();

            return Unsafe.As<KMask<TFrom>, KMask<TTo>>(ref vector);
        }

        [Intrinsic]
        [CLSCompliant(false)]
        public static KMask<ushort> Not(KMask<ushort> left)
        {
            throw  new Exception("not yet impl");
        }

        [Intrinsic]
        [CLSCompliant(false)]
        public static KMask<ushort> Or(KMask<ushort> left, KMask<ushort> right)
        {
            throw  new Exception("not yet impl");
        }

    }
}
