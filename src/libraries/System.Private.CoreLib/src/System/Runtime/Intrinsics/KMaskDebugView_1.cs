// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.


using System.Runtime.CompilerServices;

namespace System.Runtime.Intrinsics
{
    internal readonly struct KMaskDebugView<T>
        where T : struct
    {
        private readonly KMask<T> _value;

        public KMaskDebugView(KMask<T> value)
        {
            _value = value;
        }

    }
}
