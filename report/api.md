# Expose AVX10 converged ISA

## Background and motivation

Intel's latest `AVX` offering, `AVX10`, introduces a converged vector ISA that will be supported on all Intel processors. Please find full details in the [AVX10 technical paper](https://cdrdv2.intel.com/v1/dl/getContent/784343) and [AVX10 architectural specification](https://cdrdv2.intel.com/v1/dl/getContent/784267).

For brevity, we highlight the following relevant features from the technical paper and specification:

1. `AVX10` will be a _versioned_ ISA, where a version N will include all instructions/features of version N-1, N-2 etc. 

2. `AVX10` supports _multiple vector lengths_, where CPUID exposes the max vector length available for a processor.

3. `AVX10.1` will support `AVX512` and the following extensions for vector lengths 128, 256, and 512 bits: 

- `AVX512F`, `AVX512BW`, `AVX512DQ`, and `AVX512CD`

- `AVX512_VBMI`, `AVX512_IFMA`

- `AVX512_VNNI`

- `AVX512_BF16`

- `AVX512_VPOPCNTDQ,`, `AVX512_VBMI2`, `VAES`, `GFNI`, `VPCLMULQDQ`, `AVX512_BITALG`

- `AVX512_FP16`

4. On future P-core (performance) processors, the maximum vector length will be 512-bit; One future E-core (energy) processors it will be 256-bit.

## API Proposal

### AVX10 Versioning

As part of `AVX10` versioning, developers can expect that all features and capabilities of version `N` will be included in version `N+1`. As such, we propose that we capture this incremental versioning via class inheritance with the nomenclature `Avx10vN`: 

```C#
class Avx10v1 
{
  
}

class Avx10v2 : Avx10v1 
{
  
}

class Avx10v3 : Avx10v2 
{
  
}
```

Developers will be able to continue to check for ISA support via `Avx10vN.IsSupported` as is currently done for specific .NET ISA APIs.

### AVX10 Vector Length

As `AVX10` allows implementations to support different maximum vector lengths, we propose the following nested class structure, one for each supported vector length:


```C#
class Avx10 
{
  class V128 
  { 
    public static Vector128<ulong> Abs(Vector128<long> v);
  }

  class V256 
  { 
    public static Vector256<ulong> Abs(Vector256<long> v);
  }

  class V512 
  {
    public static Vector512<ulong> Abs(Vector512<long> v);
  }

}
```

`Avx10.V128.IsSupported()` returns `true` if 128-bit vectors are enabled via the CPUID vector length bits for `AVX10`. As the `AVX10` architecture specification states that the highest enumerated vector length implies all smaller vector lengths are supported, a developer may check `Avx10.V256.IsSupported()` and safely use `Avx10.V128` methods.

## AVX10 and AVX512 

`AVX10.1` represents a pre-enabling step for `AVX10`, and is the convergence of the previously listed `AVX512` instruction sets --- there is overlap with existing exposed .NET `AVX512` ISA APIs. We propose that if `AVX10.V512.IsSupported()` returns true, then the corresponding `AVX512` APIs for the aforementioned extensions can safely be used:

```C#
Vector512<long> v1 = Vector512.Create((long)someParam);
if (Avx10.V512.IsSupported()) {
  Vector512<ulong> v2 = Avx512F.Abs(v1);
  Vector512<double> v3 = Avx512DQ.ConvertToVector512Double(v2);
  // etc
}
```

For an `AVX10/256` implementation, the subset of `VL` instructions for existing `AVX512` instruction sets will be available _without the presence of `512-bit` support. As existing `AVX512VL` (and the associated nested `VL` classes in .NET APIs) implies 512-bit, we will expose the `V128` and `V256` nested classes which will contain existing and future `VL` instructions that operate on the associated vector length. For clarity, it is possible to have `AVX10.V256.IsSupported() == true` but `AVX512F.IsSupported() == false` on `AVX10/256` implementation.

### Avx10v1 API

Given the aforementioned API discussion, we propose the following API, where all current `AVX512VL` family instructions are consolidated under the `V128` and `V256` nested classes: 

```C#
class Avx10v1 
{

    class V128 
    {
      /// From AVX512F VL
      public static Vector128<ulong> Abs(Vector128<long> value) => Abs(value);

      /// From AVX512BW VL
      public static Vector128<byte> CompareGreaterThan(Vector128<byte> left, Vector128<byte> right) => CompareGreaterThan(left, right);

      /// From AVX512DQ VL
      public static Vector128<float> Range(Vector128<float> left, Vector128<float> right, [ConstantExpected(Max = (byte)(0x0F))] byte control) => Range(left, right, control);
    }

    class V256 
    {
      /// From AVX512F VL
      public static Vector256<ulong> Abs(Vector256<long> value) => Abs(value);

      /// From AVX512BW VL
      public static Vector256<byte> CompareGreaterThan(Vector256<byte> left, Vector256<byte> right) => CompareGreaterThan(left, right);

      /// From AVX512DQ VL
      public static Vector256<float> Range(Vector256<float> left, Vector256<float> right, [ConstantExpected(Max = (byte)(0x0F))] byte control) => Range(left, right, control);
    }

    class V512 
    {
      // no changes, place holder for future versions
    }

}
```

This includes the `VL` subsets for the following extensions:

- `AVX512F`, `AVX512BW`, `AVX512DQ`, and `AVX512CD`

- `AVX512_VBMI`, `AVX512_IFMA`

- `AVX512_VNNI`

- `AVX512_BF16`

- `AVX512_VPOPCNTDQ,`, `AVX512_VBMI2`, `VAES`, `GFNI`, `VPCLMULQDQ`, `AVX512_BITALG`

- `AVX512_FP16`


## Alternative Designs

### Alternative Versioning

One alternative design we are considering is to expose all `AVX10`` methods under a single class and provide a form of versioning --- defined via method attributes --- on the API:

```C#
class Avx10 
{
    public static bool VersionIsAtLeast(ulong version);

    class V128 
    {
      [SupportedAvx10Version(1)]
      public static Vector128<ulong> Method1(Vector128<long> v);

      [SupportedAvx10Version(2)]
      public static Vector128<ulong> Method2(Vector128<long> v);
    }

    class V256 
    {
      [SupportedAvx10Version(1)]
      public static Vector256<ulong> Method1(Vector256<long> v);

      [SupportedAvx10Version(2)]
      public static Vector256<ulong> Method2(Vector256<long> v);
    }

    class V512 
    {
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
  v1 = Avx10.V256.Method1(v1);
  v1 = Avx10.V256.Method2(v1);
}
```

To help developers ensure they are using the API correctly, we propose to create an analyzer that will ensure that a proper version check is in place for all methods used, and flag a warning if a method is used outside of a proper version check.


### V512 Surface Area

For developer ease-of-us, one alternative design is to duplicate the `AVX512` 512-bit API surface in the `V512` class, so the developer does not have to explicitly reference existing `AVX512` APIs. Note that this requires duplicating a large amount of API surface.

## Risks