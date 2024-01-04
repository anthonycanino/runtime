# AVX10 Enablement and Optimizations in .NET Runtime

The following is a rough breakdown of high level tasks.

### 1. Make runtime and JIT aware of AVX10 ISA

Task: 

- Create `AVX10_1` and `AVX10_2` etc. ISA entries in `InstructionSetDesc.txt`. These will roughly correspond to the public API intrinsics for AVX10.

- Logic for new CPUID detection for AVX10, i.e., check vector length available, check AVX10 version, and read from the CPUID tree.

Opens:

1. JIT currently encodes an ISA (and available instructions) as a singular unit, i.e., if `AVX512VL` is available, an internal variable is set `X86_AVX512VL_ISA` to which all dependencies are check (can I use `AVX512VL` when compling `Vector`, can I use an `AVX512VL` instruction in emit etc.). `AVX10` decouples vector length from available instructions.

- We need to either introduce some feature that lets one query for available vector length, or we will have to have separate `AVX10_1_256_ISA`, `AVX10_1_512_ISA` to encode what is available for the compiler to target. 

2. Will we need to create a dependency on AVX512VL etc? I believe `AVX10` is meant to expose same instruction but exist in isolation from an ISA availability perspective.

Summary: No major blockers, would be straightforward but require more work than a new `AVX512` ISA due to the CPUID changes.

### 2. AVX10 intrinsic API proposal

Task:

- MSFT will want to expose a public API surface for working with AVX10. We can let them drive this, as they did for AVX512. But we will need to have a rough idea of the design as we will have to match the C# classes with the internal ISAs.

Open:

1. The decoupling of vector length from ISA, and its solution, will likely have to be reflected in the API as well. Something like this will probably be the answer

```C#
class Avx10 
{
  public bool IsSupported;

  class V256 
  {
    public bool IsSupported;
  }

  class V512 
  {
    public bool IsSupported;
  }
}
```

which means that we would represent `AVX10_V256` and `AVX10_V512` in the `InstructionSetDesc.txt` above.

Summary: No major blockers, but this will require a public API proposal and review process.

### 3. AVX10 intrinsic API codegen

Task:

- Each intrinsic will need to be mapped to the instruction it generates in the intrinsic tables. Given that most of `AVX10.2` overlaps with `AVX512VL`, this will likely be a copy paste effort and is very straightforward.

Open:

1. MSFT did the majority of this work last time, hopefully we can get them to contribute again.

Summary: No major blockers.

### 4. Allow additional 16 YMM registers for AVX10/Vector256

Task:

- Limit the set of new `YMM` registers to _only_ V256 if AVX10/256 is available, and the proper set for AVX10/512 (to be answered, see opens) in `lsra.cpp`, `lsrabuild.cpp`, and `lsraxarch.cpp`. The set of available registers (at runtime) is defined in the main compiler structure as `rbmAllFlt` and is set in `compiler.cpp`.

  - Right now, the JIT compiler will add `MM16` - `MM32` to the available set of float registers if `AVX512` is present. For `AVX10.2`, we cant simply add to this register set, as V128 would get assigned registers that are not available to be encoded.

  - This might hint that V256 should actually be transformed into AVX10_2 intrinsics, not AVX intrinsics. 

Open:

1. Will AVX10.2 on PCore have AVX512VL, or will it be limited to AVX10.2 512 and AVX10.1 256? 

Summary:

This will depend on the open question above, but regardless, will require a bit of work as the set of available registers on AVX10/256 is limited to 256 vector instructions only, which means we will have to somehow limit this.

### 5. Enhance `Vector256` codegen with `AVX10` instructions.

Task:

- (Map Vector256 to AVX10_2_256 intrinsics instead of AVX2 intrinsics)

### General Questions 


(Edits finish here)

======








0. Hardware running environment for MSFT to accept changes into repo when hardware not available.
4. Adjust codegen checks for AVX512 to check for AVX10.1/AVX10.2
9. KMask register lack of compatibility
6. Allow embedded rounding for YMM                       
7. Convert remaining AVX2 implementations to Vector256   
8. Allow AVX512 optimizations for AVX10 (ymm)            


#