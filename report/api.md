# Expose AVX10 converged ISA

## Background and motivation

AVX10 represents a converged vector ISA for SIMD programming on Intel processors.  Please find full details in the [AVX10 technical paper](https://cdrdv2.intel.com/v1/dl/getContent/784343) and [AVX10 architectural specification](https://cdrdv2.intel.com/v1/dl/getContent/784267).

The following feature highlights are relevant to this API proposal:

1. AVX10 will be a _versioned_ ISA, where a version N will include all instructions/features of version N-1, N-2 etc. 

2. AVX10 supports _multiple vector lengths_, where CPUID exposes the max vector length for a processor.

3. AVX10 version 1 will support `AVX512` and all of its extensions for vector lengths 128, 256, and 512 bits. On future P-core processors, maximum vector length will be 512-bit, while on E-core processors it will be 256-bit.

## API Proposal

### AVX10 Versioning

As part of `AVX10` versioning, developers can expect that all features and capabilities of version `N` will be included in version `N+1`. As such, we propose that we capture this incremental versioning via class inheritance with the nomenclature `Avx10vN`: 

```C#
class Avx10v1 {
  
}

class Avx10v2 : Avx10v1 {
  
}

class Avx10v3 : Avx10v2 {
  
}
```

Developers will be able to continue to check for ISA support via `Avx10vN.IsSupported` as is currently done for specific .NET ISA APIs.

### AVX10 Vector Length

Vector length is orthogonal to the aforementioned versioning choice, where we propose a nested inner class that represents each vector length available:

```C#
class Avx10 {
  class V256 { 
    public static Vector256<ulong> Abs(Vector256<long> v);

  }

  class V512 {
    public static Vector512<ulong> Abs(Vector512<long> v);
  }

}
```

## AVX10 and AVX512 

As `AVX10` represents a convergence of Intel's SIMD ISAs, there is overlap with existing `AVX512` functionality. With the presence of the two APIs, developers would have to write something along the lines of the following to properly program for both platforms:

```C#
Vector256<long> v1 = Vector256.Create((long)someParam);
if (Avx10.IsSupported()) {
  Vector256<ulong> v2 = Avx10.V256.Abs(v1);
  // ...

} else if (Avx512F.VL.IsSupported()) {
  Vector256<ulong> v2 = Avx512F.VL.Abs(v1);
  // ...

} else {
  // slower fallback  

}
```

To capture the spirit of the converged ISA, we propose a new `IsEmulated` method, which will return `true` if the JIT is capable of emulating `Avx10` APIs with `AVX512` APIs. This would allow developers to collapse the above code into:


```C#
Vector256<long> v1 = Vector256.Create((long)someParam);
if (Avx10v1.IsSupported() || Avx10v1.IsEmulated()) {
  Vector256<ulong> v2 = Avx10.V256.Abs(v1);
  // ...

} else {
  // slower fallback  

}
```

`IsSupported` continues the trend of reflecting the exact CPUID information read from the processor.

### Avx10v1 API

Given the aforementioned API discussion, we propose the following API, where all current `AVX512VL` family instructions are consolidated under the `V256` nested classes. 

```C#
class Avx10v1 {

    class V256 {
      /// From AVX512F VL
      public static Vector128<ulong> Abs(Vector128<long> value) => Abs(value);
      public static Vector256<ulong> Abs(Vector256<long> value) => Abs(value);

      /// From AVX512BW VL
      public static Vector128<byte> CompareGreaterThan(Vector128<byte> left, Vector128<byte> right) => CompareGreaterThan(left, right);
      public static Vector256<byte> CompareGreaterThan(Vector256<byte> left, Vector256<byte> right) => CompareGreaterThan(left, right);

      /// From AVX512DQ VL
      public static Vector128<float> Range(Vector128<float> left, Vector128<float> right, [ConstantExpected(Max = (byte)(0x0F))] byte control) => Range(left, right, control);
      public static Vector256<float> Range(Vector256<float> left, Vector256<float> right, [ConstantExpected(Max = (byte)(0x0F))] byte control) => Range(left, right, control);

      // etc 
    }

    class V512 {

    }

}
```

## Alternative Designs

One alternative design we are considering is to expose all Avx10 methods under a single class and provide a form of versioning --- defined via method attributes --- on the API:

```C#
class Avx10 
{

    public static new uint Version { get => Version; }

    class V256 
    {
      [SupportedAvx10Version(1)]
      public static Vector256<ulong> Method1(Vector256<long> v);

      [SupportedAvx10Version(2)]
      public static Vector256<ulong> Method2(Vector256<long> v);
    }

    class V512 
    {
      [SupportedAvx10Version(1)]
      public static Vector512<ulong> Method1(Vector512<long> v);

      [SupportedAvx10Version(2)]
      public static Vector512<ulong> Method2(Vector512<long> v);
    }
}
```

The developer may check for the specific `Avx10` version necessary and then may use it and all preceding version methods without having to explicitly refer to additional classes `Avx10v1`, `Avx10v2` etc:

```C#
Vector256<ulong> v1 = ...;
if (Avx10.VersionIsAtLeast(2))
{
  v1 = Avx10.V512.Method1(v1);
  v1 = Avx10.V512.Method2(v1);
}
```

To help developers ensure they are using the API correctly, we propose to create an analyzer that will ensure that a proper version check is in place for all methods used, and flag a warning if a method is used outside of a proper version check.

## Risks